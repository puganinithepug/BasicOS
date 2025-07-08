#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include <time.h>
#include <dirent.h>
#include <string.h>

#include "varstore.h"
#include "shell.h"
#include "scheduler.h"

#define MAX_ARGS_SIZE 7

int badcommand(){
    printf("Unknown Command\n");
    return 1;
}

int badcommandMsg(char *error_msg){
    printf("Bad command: %s\n", error_msg);
    return 2;
}

// For run command only
int badcommandFileDoesNotExist(){
    printf("Bad command: File not found\n");
    return 3;
}

int help();
int quit();
int set(char* var, char *value[], int value_length);
int print(char* var);
int echo(char* str);
int run(char* script);
int exec(char* scripts[], size_t n_scripts, enum Policy policy);
int my_ls(const char* dirname);
int badcommandFileDoesNotExist();

// Interpret commands and their arguments
int interpreter(char* command_args[], int args_size) {
    int i;

    if (args_size < 1) {
        return badcommand();
    } else if (args_size > MAX_ARGS_SIZE) {
        return badcommandMsg("Too many tokens");
    }

    for (i = 0; i < args_size; i++) { // terminate args at newlines
        command_args[i][strcspn(command_args[i], "\r\n")] = 0;
    }

    if (strcmp(command_args[0], "help") == 0){
        //help
        if (args_size != 1) {return badcommand();}
        return help();
    } else if (strcmp(command_args[0], "quit") == 0) {
        //quit
        if (args_size != 1) {return badcommand();}
        return quit();
    } else if (strcmp(command_args[0], "set") == 0) {
        //set
        if (args_size < 3) {return badcommand();}	
        if (args_size > 7) {return badcommandMsg("Too many tokens");}
        return set(command_args[1], &command_args[2], args_size - 2);
    } else if (strcmp(command_args[0], "echo") == 0) {
    	//echo
        if (args_size != 2) {return badcommand();}
        return echo(command_args[1]);
    } else if (strcmp(command_args[0], "print") == 0) {
        //print
        if (args_size != 2) {return badcommand();}
        return print(command_args[1]);
    } else if (strcmp(command_args[0], "run") == 0) {
        //run
        if (args_size != 2) {return badcommand();}
        return run(command_args[1]);
    } else if (strcmp(command_args[0], "exec") == 0) {
        //exec
        if (args_size < 3 || args_size > 5) {return badcommand();}

        // Determine the execution policy
        enum Policy policy;
        // TODO hash table, or anything but this
        if (strcmp(command_args[args_size - 1], "RR") == 0) { policy = RR; }
        else if (strcmp(command_args[args_size - 1], "RR30") == 0) { policy = RR30; }
        else { return badcommandMsg("Invalid scheduling policy.\n"); }

        return exec(
            &command_args[1],
            args_size - 2,
            policy
        );
    } else if (strcmp(command_args[0], "my_ls") == 0) {
            //my_ls
        //if no directory specified list contents
        if (args_size < 1 || args_size > 2) {return badcommand();}
    
        char *dir;
        if (args_size == 1) {
            dir = ".";
        } else if (args_size == 2) {
            dir = command_args[1];
        }
        return my_ls(dir);
    } else if (strcmp(command_args[0], "my_mkdir") == 0) {
        //my_mkdir
        if (args_size != 2) {return badcommand();}
        char* dir = command_args[1];
        //check if it exists
        if (dir[0] == '$') {
            //already exist, check memory
            char* dirname = mem_get_value(dir+1);
            if (dirname == NULL) {return badcommandMsg("my_mkdir");}
            dir = dirname;
        }
        if (strchr(dir, ' ') != NULL) {return badcommandMsg("my_mkdir");}
        return mkdir(dir, 0777);
    } else if (strcmp(command_args[0], "my_touch") == 0) {
	    //my_touch
	    for (int i = 1; i < args_size; i++) {
            FILE *fp;
            fp = fopen(command_args[i], "w");
            fclose(fp);
        }
        return 0;
    } else if (strcmp(command_args[0], "my_cd") == 0) {
        //my_cd
        if (args_size<2) {return badcommand();}
        char* dir = command_args[1];
        
        if(chdir(dir) != 0){
            return badcommandMsg("my_cd");
        } else {
            return 0;
        }
    } else {
        return badcommand();
    }
}

int help() {
    // note the literal tab characters here for alignment
    char help_string[] = (
"COMMAND                 DESCRIPTION\n \
help                   Displays all the commands\n \
quit                   Exits / terminates the shell with “Bye!”\n \
set VAR STRING         Assigns a value to shell memory\n \
print VAR              Displays the STRING assigned to VAR\n \
run SCRIPT.TXT         Executes the file SCRIPT.TXT\n "
);
    printf("%s\n", help_string);
    return 0;
}

int quit() {
    printf("Bye!\n");
    exit(0);
}

int set(char *var, char *value[], int value_length) {
    char buffer[MAX_USER_INPUT];
    char *p = buffer;
    char *delim = " ";

    // Copy the tokenized values into the buffer, separated by delim
    p = stpcpy(p, value[0]);
    for (int i = 1; i < value_length; i++) {
        p = stpcpy(p, delim);
        p = stpcpy(p, value[i]);
    }

    mem_set_value(var, buffer);

	return 0;
}

int print(char *var) {
    printf("%s\n", mem_get_value(var)); 
    return 0;
}

int echo(char *str) {
    if(str == NULL) {
        badcommand();
    }
    if(str[0] == '$') {
        char *value = mem_get_value(str + 1);
        if (value == NULL) {
            printf("Variable does not exist");
        } else {
            printf("%s\n", mem_get_value(str+1));
        }
    }
    else {
        printf("%s\n", str);
    }
    return 0;
}

int run(char *script) {
    // Check memory limits
    if (N_FRAMES < 2) {
        printf("Error: The shell memory is not large enough to support this command. Please rebuild with enough memory (need a minimum of two pages).\n");
        return 1;
    }

    FILE *p = fopen(script, "rt");
    if (p == NULL) { return badcommandFileDoesNotExist(); }

    struct pcb *proc = new_process(p, script);  // create a new process

    fclose(p);

    // Initialize the scheduler
    struct Scheduler *sch = scheduler_get(RR30);

    // Run the process
    scheduler_add(sch, proc);
    scheduler_run(sch);

    return 0;
}

int exec(char *scripts[], size_t n_scripts, enum Policy policy) {
    // Check memory limits
    if (N_FRAMES < n_scripts * 2) {
        printf("Error: The shell memory is not large enough to support this command. Please rebuild with enough memory (need a minimum of %zu pages), or rerun the command with fewer scripts.\n", 2 * n_scripts);
        return 1;
    }

    // Initialize the scheduler
    struct Scheduler *sch = scheduler_get(policy);

    // Populate scheduler with jobs
    int processed_by_lookahead[3] = {0};      // keep track of which scripts have been processed (maximum of three)
    for (int i = 0; i < n_scripts; i++) {
        if (processed_by_lookahead[i]) {
            // script was already processed by lookahead
            continue;
        }

        FILE *p = fopen(scripts[i], "rt");
        if (p == NULL) { return badcommandFileDoesNotExist(); }
        
        struct pcb *proc = new_process(p, scripts[i]);  // create a new process
        scheduler_add(sch, proc);            // add the process to the scheduler
    
        fclose(p);

        // Look ahead for duplicates
        for (int j = i + 1; j < n_scripts; j++) {
            if (strcmp(scripts[i], scripts[j]) == 0) {
                // script is mentioned again at index j, share memory
                struct pcb *proc_cpy = pcb_new(
                    generate_pid(),
                    proc->page_tbl,
                    scripts[i]
                ); // new pid, same page table
                scheduler_add(sch, proc_cpy);
                processed_by_lookahead[j] = 1;
            }
        }
    }

    // Run the scheduler (unless it's already running)
    if (!sch->running) {
        scheduler_run(sch);
    }

    return 0;
}

// Comparing for sorting entries alphabetically
int comparebyAlpha(const struct dirent **a, const struct dirent **b) {
    return strcmp((*a)->d_name, (*b)->d_name);
}

//my_ls implementation
int my_ls(const char* dir) {
    struct dirent **namelist;
    int n;

    // Use scandir to read and sort directory entries
    n = scandir(dir, &namelist, NULL, comparebyAlpha);
    if (n < 0) {
        perror("Unable to open directory");
        return errno; // Return the error code
    }

    // Print sorted entries
    for (int i = 0; i < n; i++) {
        // Don't print hidden directories (beginning with .)
        if (namelist[i]->d_name[0] != '.') {
            printf("%s\n", namelist[i]->d_name);
        }
    }
    free(namelist); // Free the array holding the entries

    return 0; // Success
}
