#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> 
#include "main.h"
#include <string.h>
#include <sys/resource.h> 
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>
#include "array_list.h" 
#define NUM_ARGS 3
#define FILE_IDX 1

void limit_fork(rlim_t max_procs) { 
    struct rlimit rl; 

    if (getrlimit(RLIMIT_NPROC, &rl)) { 
        perror("getrlimit"); 
	exit(-1); 
    } 
    rl.rlim_cur = max_procs; 
    if (setrlimit(RLIMIT_NPROC, &rl)) { 
        perror("setrlimit"); 
	exit(-1); 
    } 
}

void remove_trailing_newline(char *str) { 
    if (str == NULL) {
        return; 
    } 

    int len = strlen(str); 
    if (str[len-1] == '\n') {
        str[len-1] = '\0'; 
    } 
} 

int is_num(char argv[]) { 
    if (isdigit(argv[0])) { 
        return 1; 
    } 

    return 0; 
} 

int validate_args(int argc, char *argv[]) { 
     FILE *file; 

     if (argc == NUM_ARGS) { 
          if ((!is_num(argv[1])) & (is_num(argv[2]))) { 
	      if ((file = fopen(argv[1], "r"))) {
                  fclose(file); 
		  if (atoi(argv[2]) > 0) { 
                      return 1; 
	          } 
		  fprintf(stderr, "ERROR: not a valid positive integer\n");
		  exit(1);  
	      } 
	      fprintf(stderr, "ERROR: file does not exist\n"); 
	      exit(1);  
          } 
	  fprintf(stderr, "ERROR: invalid command line arg types\n"); 
	  exit(1); 
     } 
     fprintf(stderr, "ERROR: invalid amount of command line args\n"); 
     exit(1); 
}

FILE* open_file(char *argv[]) {
    FILE* my_file = NULL;

    my_file = fopen(argv[FILE_IDX], "r");

    if (my_file) {
        return my_file;
    } else {
        fprintf(stderr, "ERROR: File does not exist\n");
        exit(1);
    }
} 

void end_process(int *length, struct array_list al) { 
    int status;
    int num; 
    pid_t fin; 

    fin = wait(&status);
    if (fin > -1) {
        for (int i = 0; i < *length; i++) { 
            if (al.array[i].child == fin) {
	        num = al.array[i].line_num; 
		break; 
            } 
	}
        if (WIFEXITED(status)) { 
            if (WEXITSTATUS(status) == 0) { 
                fprintf(stdout, "END: process %i processing line #%i\n", fin, num); 
	    } else { 
                fprintf(stdout, "ABNORMAL TERMINATION: process %i processing line #%i\n", fin, num); 
	    }
	}	
    }  
} 	

void read_file(FILE *fd, int max_proc, int line_num, int count, struct array_list al, int *length) {
    char *line = NULL;
    char *dup = NULL;
    char *output_name = NULL;
    char *url = NULL;
    char *max_sec = NULL;
    size_t size = 128;  
    pid_t child;  

    limit_fork(300);
    if (getline(&line, &size, fd) > 0) {
	if (strlen(line) > 1) {
            dup = line;
            output_name = strsep(&dup, " ");
	    if (!strchr(dup, ' ')) {
                url = dup;
	    } else {
               url = strsep(&dup, " ");
	       max_sec = dup;
	    }

	    remove_trailing_newline(output_name);
	    remove_trailing_newline(url);
	    remove_trailing_newline(max_sec);
	    if (max_proc <= count) { 
                end_process(length, al); 
	        count--; 
	    } 

	    count++; 
	    if ((child = fork()) < 0) {
                fprintf(stderr, "ERROR: fork failure\n");
                exit(1);
            } else if (child == 0) {
		if (max_sec != NULL) { 
		    execlp("curl", "curl", "-m", max_sec, "-o", output_name, "-s", url, NULL); 
		    fprintf(stderr, "ERROR: exec failure\n"); 
	        } else { 
                    execlp("curl", "curl", "-o", output_name, "-s", url, NULL);
                    fprintf(stderr, "ERROR: exec failure\n");
                    exit(1);
	        }
            } else {
		(*length)++; 
		line_num++;
	        fprintf(stdout, "START: process %i processing line #%i\n", child, line_num); 	
		struct dict d = dict_new(child, line_num); 
		al = array_list_add_to_end(al, d);
                read_file(fd, max_proc, line_num, count, al, length);
		end_process(length, al); 
            }
	} else {
            read_file(fd, max_proc, line_num, count, al, length);
	}
    }

   free(line);
}
 
int main (int argc, char *argv[]) { 
    int max_proc;
    int line_num = 0; 
    int count = 0; 
    int len = 0; 
    int *length = &len; 
    FILE *fd;   
    struct array_list al = array_list_new(); 
 
    validate_args(argc, argv);  
    fd = open_file(argv);
    max_proc = atoi(argv[2]); 
    read_file(fd, max_proc, line_num, count, al, length);
 
    free_array_list(al); 
    fclose(fd);     
    return 0; 
} 
