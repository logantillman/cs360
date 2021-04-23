// Author: Logan Tillman

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "bonding.h"
#include "dllist.h"

typedef struct {
    pthread_mutex_t *lock;
    Dllist waitHydrogen;
    Dllist waitOxygen;
} GlobalInfo;

typedef struct {
    pthread_cond_t *cond;
    pthread_t *tid;
    pthread_t *mol1;
    pthread_t *mol2;
    pthread_t *mol3;
} Molecule;

void *initialize_v(char *verbosity) {
  return NULL;
}

void *hydrogen(void *arg) {
  return NULL;
}

void *oxygen(void *arg) {
  return NULL;
}
