// Author: Logan Tillman

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "bonding.h"
#include "dllist.h"

/* Struct for holding the global information */
typedef struct {
    pthread_mutex_t *lock;
    Dllist waitHydrogen;
    Dllist waitOxygen;
} GlobalInfo;

/* Struct for holding the molecule information */
typedef struct {
    pthread_cond_t *cond;
    int tid;
    int h1;
    int h2;
    int o;
} Molecule;

/* Initializing the global struct */
void *initialize_v(char *verbosity) {
    GlobalInfo *g = (GlobalInfo *) malloc(sizeof(GlobalInfo));
    g->lock = new_mutex();
    g->waitHydrogen = new_dllist();
    g->waitOxygen = new_dllist();

    return (void *)g;
}

/* Function for handling a hydrogen thread */
void *hydrogen(void *arg) {

    /* Initializing the molecule */
    Molecule *molecule = (Molecule *) malloc(sizeof(Molecule));
    molecule->cond = new_cond();
    molecule->h1 = -1;
    molecule->h2 = -1;
    molecule->o = -1;

    struct bonding_arg *a = (struct bonding_arg *) arg;
    GlobalInfo *g = (GlobalInfo *) a->v;

    /* Setting the current thread id */
    molecule->tid = a->id;

    /* Locking the mutex */
    pthread_mutex_lock(g->lock);

    /* If there's not an available hydrogen or oxygen, we add the molecule to the list and wait */
    if (dll_empty(g->waitHydrogen) || dll_empty(g->waitOxygen)) {
        dll_append(g->waitHydrogen, new_jval_v(molecule));
        pthread_cond_wait(molecule->cond, g->lock);
    }
    else {

        /* Once we wake, we grab the other molecules */
        Molecule *hydrogen = (Molecule *) g->waitHydrogen->flink->val.v;
        Molecule *oxygen = (Molecule *) g->waitOxygen->flink->val.v;

        /* Set the thread id of all 3 molecules for each molecule */
        molecule->h1 = a->id;
        molecule->h2 = hydrogen->tid;
        molecule->o = oxygen->tid;

        hydrogen->h1 = a->id;
        hydrogen->h2 = hydrogen->tid;
        hydrogen->o = oxygen->tid;

        oxygen->h1 = a->id;
        oxygen->h2 = hydrogen->tid;
        oxygen->o = oxygen->tid;

        /* Delete the nodes from their lists */
        dll_delete_node(g->waitHydrogen->flink);
        dll_delete_node(g->waitOxygen->flink);

        /* Wake the other threads */
        pthread_cond_signal(hydrogen->cond);
        pthread_cond_signal(oxygen->cond);
    }

    /* Unlock the mutex */
    pthread_mutex_unlock(g->lock);

    /* Bond the molecules */
    char *rv = Bond(molecule->h1, molecule->h2, molecule->o);

    /* Free up our memory */
    free(molecule->cond);
    free(molecule);

    return (void *) rv;
}

/* Function for handling an oxygen thread */
void *oxygen(void *arg) {

    /* Initializing the molecule */
    Molecule *molecule = (Molecule *) malloc(sizeof(Molecule));
    molecule->cond = new_cond();
    molecule->h1 = -1;
    molecule->h2 = -1;
    molecule->o = -1;

    struct bonding_arg *a = (struct bonding_arg *) arg;
    GlobalInfo *g = (GlobalInfo *) a->v;

    /* Setting the thread id */
    molecule->tid = a->id;

    /* Locking the mutex */
    pthread_mutex_lock(g->lock);

    int numHydrogen = 0;
    Dllist tmp;

    /* Checking to see if we have 2 available hydrogen threads */
    dll_traverse(tmp, g->waitHydrogen) {
        if (numHydrogen == 2) {
            break;
        }
        numHydrogen++;
    }

    /* If we don't have 2 available hydrogen threads, we add the current thread to the list and wait */
    if (numHydrogen < 2) {
        dll_append(g->waitOxygen, new_jval_v(molecule));
        pthread_cond_wait(molecule->cond, g->lock);
    }
    else {

        /* If there are 2 available hydrogen threads, grab them */
        Molecule *h1 = (Molecule *) g->waitHydrogen->flink->val.v;
        Molecule *h2 = (Molecule *) dll_next(g->waitHydrogen->flink)->val.v;

        /* Set the thread ids for each molecule */
        molecule->h1 = h1->tid;
        molecule->h2 = h2->tid;
        molecule->o = a->id;

        h1->h1 = h1->tid;
        h1->h2 = h2->tid;
        h1->o = a->id;

        h2->h1 = h1->tid;
        h2->h2 = h2->tid;
        h2->o = a->id;

        /* Delete the molecules from the list */
        dll_delete_node(g->waitHydrogen->flink);
        dll_delete_node(g->waitHydrogen->flink);

        /* Wake the other threads */
        pthread_cond_signal(h1->cond);
        pthread_cond_signal(h2->cond);
    }

    /* Unlock the mutex */
    pthread_mutex_unlock(g->lock);

    /* Bond the molecules */
    char *rv = Bond(molecule->h1, molecule->h2, molecule->o);

    /* Free the memory */
    free(molecule->cond);
    free(molecule);

    return (void *) rv;
}
