#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
 
#define MAX_BUFFER_SIZE 8192
 
void main_func(char *input, char *output) {

    int fd_input, fd_output;
    ssize_t bytes_read;
    char buffer[MAX_BUFFER_SIZE];
 
    if ((fd_input = open(input, O_RDONLY)) < 0) {
        printf("Error while opening file for read\n");
        exit(EXIT_FAILURE);
    }
 	
    // Создаем именованные каналы для передачи данных между процессами
    const char* pipe1 = "pipe1.fifo";
    const char* pipe2 = "pipe2.fifo";
    mknod(pipe1, S_IFIFO | 0666, 0);
    mknod(pipe2, S_IFIFO | 0666, 0);
    int fd1, fd2;
    if ((fd_input = open(input, O_RDONLY, 0666)) < 0) {
        printf("Error while opening a file");
        exit(EXIT_FAILURE);
    }
    bytes_read = read(fd_input, buffer, MAX_BUFFER_SIZE);
    if (close(fd_input) < 0) {
        printf("Error while closing an input file");
    }
    if ((fd1 = open(pipe1, O_WRONLY)) < 0) {
        printf("Error while opening pipe");
        exit(EXIT_FAILURE);
    }        
    bytes_read = write(fd1, buffer, MAX_BUFFER_SIZE);
    if (close(fd1) < 0) {
        printf("Error while closing pipe");
        exit(EXIT_FAILURE);
    }
    if ((fd2 = open(pipe2, O_RDONLY)) < 0) {
        exit(EXIT_FAILURE);
    }
    bytes_read = read(fd2, buffer, MAX_BUFFER_SIZE);
    if (close(fd2) < 0) {
        printf("Error while closing a pipe");
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
 
int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Console run wrong format\n");
		return 0;
	}
	main_func(argv[1], argv[2]);
	return 0;
}
