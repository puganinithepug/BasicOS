#ifndef stdin
#include <stdio.h>
#endif

int parseInput(char inp[]);
int run_shell(FILE *input_stream);
void execute_line(char *line);
