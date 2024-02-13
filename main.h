#ifndef TREE_H
#define TRR_H
#include <stdio.h> 
#include <stdlib.h> 
#include <sys/resource.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include "array_list.h" 

void limit_fork(rlim_t max_procs);

void remove_trailing_newline(char *str); 

int is_num(char argv[]); 

int validate_args(int argc, char *argv[]);

FILE* open_file(char *argv[]);  

void end_process(int *length, struct array_list al); 
//void make_child(char *output_name, char *url, char *max_sec, int line); 

void read_file(FILE *fd, int max_proc, int line_num, int count, struct array_list al, int *length); 

#endif
