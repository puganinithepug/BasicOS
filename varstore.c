#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "limits.h"
#include "varstore.h"
#include "scheduler.h"

#define VAR_NULL "none"

struct var_memory_struct {
    int pid;
    char *var;
    char *value;
};

struct var_memory_struct varstore[MEM_SIZE];

// Helper functions
int match(char *model, char *var) {
    int i, len = strlen(var), matchCount = 0;
    for (i = 0; i < len; i++) {
        if (model[i] == var[i]) matchCount++;
    }
    if (matchCount == len) {
        return 1;
    } else return 0;
}

// Shell memory functions

void mem_init(){
    int i;
    for (i = 0; i < MEM_SIZE; i++){		
        varstore[i].var   = VAR_NULL;
        varstore[i].value = VAR_NULL;
    }
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++){
        if (
            varstore[i].pid == getspid() &&
            strcmp(varstore[i].var, var_in) == 0
        ) {
            varstore[i].value = strdup(value_in);
            return;
        }
    }

    // Value does not exist, need to find a free spot.
    for (i = 0; i < MEM_SIZE; i++){
        if (strcmp(varstore[i].var, VAR_NULL) == 0) {
            varstore[i].pid   = getspid();
            varstore[i].var   = strdup(var_in);
            varstore[i].value = strdup(value_in);
            return;
        } 
    }

    return;
}

//get value based on input key
char *mem_get_value(char *var_in) {
    int i;

    for (i = 0; i < MEM_SIZE; i++){
        if (
            varstore[i].pid == getspid() &&
            strcmp(varstore[i].var, var_in) == 0
        ) {
            return strdup(varstore[i].value);
        } 
    }
    return NULL;  // variable does not exist
}
