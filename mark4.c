#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
 
#define MAX_BUFFER_SIZE 8192

void upper_words(char request[]) {
    int in_word = 0;
    int has_upper = 0;
    int j = 0;
    int first_word = 1;
    for (int i = 0; i < MAX_BUFFER_SIZE; ++i) {
        if (request[i] == '\0') {
            request[j] = '\0';
            break;
        }
	if (isupper(request[i])) {
            if (in_word) {
		if (has_upper) {
                    request[j] = request[i];
                    ++j;
                }
            } else {
                if (first_word) {
                    in_word = 1;
                    request[j] = request[i];
                    ++j;
                    first_word = 0;
                    has_upper = 1;
                } else {
                    in_word = 1;
                    request[j] = ' ';
                    request[j + 1] = request[i];
                    j += 2;
                    has_upper = 1;
                }
            }
        } else if (isalpha(request[i])) {
            if (in_word) {
                if (has_upper) {
                    request[j] = request[i];
                    ++j;
                }
            } else {
                in_word = 1;
                has_upper = 0;
            }
        } else {
            in_word = 0;
            has_upper = 0;
        }
    }
}
 
void main_func(char *input, char *output) {
    int fd_input, fd_output;
    ssize_t bytes_read;
    char buffer[MAX_BUFFER_SIZE];
 
    if ((fd_input = open(input, O_RDONLY)) < 0) {
        printf("Error while opening file for read\n");
        exit(EXIT_FAILURE);
    }
 	
    // Создаем неименованные каналы для передачи данных между процессами
    int fd1[2], fd2[2];
    if (pipe(fd1) == -1) {
        printf("Error while creating a pipe\n");
        exit(EXIT_FAILURE);
    }
    if (pipe(fd2) == -1) {
        printf("Error while creating a pipe\n");
        exit(EXIT_FAILURE);
    }
    
    int result1 = fork(); // форк на 1 и 2 процессы

    if (result1 < 0) {
        printf("Error while fork");
        exit(EXIT_FAILURE);
    } else if (result1 > 0) {   // первый процесс
        if (close(fd1[0]) < 0) {
            printf("Error while closing read part of pipe");
            exit(EXIT_FAILURE);
        }
        if ((fd_input = open(input, O_RDONLY, 0666)) < 0) {
            printf("Error while opening a file");
            exit(EXIT_FAILURE);
        }
        bytes_read = read(fd_input, buffer, MAX_BUFFER_SIZE);
        if (close(fd_input) < 0) {
            printf("Error while closing an input file");
        }
        
        bytes_read = write(fd1[1], buffer, MAX_BUFFER_SIZE);
        if (close(fd1[1]) < 0) {
            printf("Error while closing write part of pipe");
            exit(EXIT_FAILURE);
        }
    } else {        // второй процесс от первого
        int result2 = fork(); // форк 3-го процесса от 2-го
        if (result2 < 0) {
            printf("Error while fork");
            exit(EXIT_FAILURE);
        } else if (result2 > 0) {   // второй процесс
            if (close(fd1[1]) < 0) {
                printf("Error while closing write part of pipe");
                exit(EXIT_FAILURE);
            }
            bytes_read = read(fd1[0], buffer, MAX_BUFFER_SIZE);
            if (close(fd1[0]) < 0) {
                printf("Error while closing a read side");
                exit(EXIT_FAILURE);
            }
            upper_words(buffer);
            bytes_read = write(fd2[1], buffer, MAX_BUFFER_SIZE);
            if (bytes_read != MAX_BUFFER_SIZE) {
                printf("Error while write");
                exit(EXIT_FAILURE);
            }
            if (close(fd2[1]) < 0) {
                printf("Error while closing a write side");
                exit(EXIT_FAILURE);
            }
        } else {               // третий процесс
            if (close(fd2[1]) < 0) {
                printf("Error while closing a write side");
                exit(EXIT_FAILURE);
            }
            bytes_read = read(fd2[0], buffer, MAX_BUFFER_SIZE);
            if (close(fd2[0]) < 0) {
                printf("Error while closing a read side");
                exit(EXIT_FAILURE);
            }
            if ((fd_output = open(output, O_WRONLY | O_CREAT, 0666)) < 0) {
                printf("Can`t open write file\n");
                exit(EXIT_FAILURE);
            }
            bytes_read = write(fd_output, buffer, strlen(buffer));
            if (close(fd_output) < 0) {
                printf("Error close write file");
            }
        }
        
    }
}
 
int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Console run wrong format\n");
		return 0;
	}
	main_func(argv[1], argv[2]);
	return 0;
}
