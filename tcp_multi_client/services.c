#include "service.h" 
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h> 
#include <string.h> 
#include <sys/wait.h>
#include <unistd.h> 

void ListDirectories(const char *path, char *buff, size_t buff_size) {
    int pid = 0;
    int p[2]; // Pipe file descriptors: p[0] for reading, p[1] for writing

    // Initialize the pipe
    if (pipe(p) == -1) {
        perror("Error creating pipe");
        return;
    }

    pid = fork();
    if (pid < 0) {
        perror("Error forking");
        return;
    }

    if (pid == 0) { // Child process
        close(p[0]); // Close unused read end of the pipe
        dup2(p[1], STDOUT_FILENO); // Redirect stdout to the pipe's write end
        close(p[1]); // Close the original write end of the pipe

        char *args[] = {"ls", (char *)path, NULL};
        execv("/bin/ls", args); // Execute 'ls'

        // If execv fails
        perror("execv failed");
        exit(EXIT_FAILURE);
    } else { // Parent process
        close(p[1]); // Close unused write end of the pipe

        // Read from the pipe into the buffer
        ssize_t bytes_read = read(p[0], buff, buff_size - 1);
        if (bytes_read < 0) {
            perror("Error reading from pipe");
        } else {
            buff[bytes_read] = '\0'; // Null-terminate the buffer
        }

        close(p[0]); // Close the read end of the pipe

        // Wait for the child process to finish
        // wait(NULL);
    }
}
bool Authenticate(const char* username, const char* psswd){ 
    const char* us = "mok" ; 
    const char* ps = "mok" ; 
    if( strcmp(us,username) == 0  && strcmp(ps,psswd) == 0   ){ 
        return true; 
    }else { 
        return false; 
    }
 
}

void getDateTime(char* _time, size_t time_size, char* _date, size_t date_size) { 
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    
    // Format the date as YYYY-MM-DD
    snprintf(_time, time_size, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    
    // Format the time as HH:MM:SS
    snprintf(_date, date_size, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void getFileContent(char* filename, char* buff, size_t buff_size){ 
  int pid = 0;
    int p[2]; // Pipe file descriptors: p[0] for reading, p[1] for writing

    // Initialize the pipe
    if (pipe(p) == -1) {
        perror("Error creating pipe");
        return;
    }

    pid = fork();
    if (pid < 0) {
        perror("Error forking");
        return;
    }

    if (pid == 0) { // Child process
        close(p[0]); // Close unused read end of the pipe
        dup2(p[1], STDOUT_FILENO); // Redirect stdout to the pipe's write end
        close(p[1]); // Close the original write end of the pipe

        char *args[] = {"cat", (char *)filename, NULL};
        execv("/bin/cat", args); // Execute 'ls'

        // If execv fails
        perror("execv failed");
        exit(EXIT_FAILURE);
    } else { // Parent process
        close(p[1]); // Close unused write end of the pipe

        // Read from the pipe into the buffer
        ssize_t bytes_read = read(p[0], buff, buff_size - 1);
        if (bytes_read < 0) {
            perror("Error reading from pipe");
        } else {
            buff[bytes_read] = '\0'; // Null-terminate the buffer
        }

        close(p[0]); // Close the read end of the pipe

        // Wait for the child process to finish
        wait(NULL);
    }
} 


void getElapsedTime( bool fcapture ,  double* elapsed ){ 
    static struct timespec start_time, end_time;
    if ( fcapture ){ 
    // Get current time (start time)
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    }else { 
    // Get current time (end time)
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    // Calculate elapsed time in seconds
    double tmp  =(double) (end_time.tv_sec - start_time.tv_sec) +(end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    *elapsed  = tmp ; 
    }

}
