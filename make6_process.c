#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define EXE_BIN "./make6_process"
#define FILE_NAME "./new_file"
#define MSG_END_SIMBOL '*'

int main()
{
    int fd;
    pid_t pid, ppid, child_pid[2] = {0};
    char pid_str[20] = {0};
    char *file_pid_str;
    char *file_ppid_str;
    size_t bytes_counter;
    int status[2];
    char in;

    fd = open(FILE_NAME, O_CREAT | O_RDWR | O_EXCL, S_IRWXO | S_IRWXU | S_IRWXG);
    if(fd < 0) {
        fd = open(FILE_NAME, O_RDWR, S_IRWXO | S_IRWXU | S_IRWXG);

        pid = getpid(); 
        ppid = getppid();

        while(1) {
            memset(pid_str, 0, sizeof(pid_str));
            bytes_counter = lseek(fd, 0, SEEK_END);
            lseek(fd, 0, SEEK_SET);
            bytes_counter = read(fd, pid_str, bytes_counter);
            if(pid_str[bytes_counter - 1] == MSG_END_SIMBOL) 
                break;
        }           
        file_ppid_str = strtok (pid_str,"\n");
        file_pid_str = strtok (NULL,"\n");

        if(ppid == atol(file_ppid_str)) {
            child_pid[0] = fork();
            if(child_pid[0] == 0) {
                execl(EXE_BIN, EXE_BIN, NULL);
                exit(EXIT_SUCCESS);
            }
            else {
                if(pid == atol(file_pid_str)) {
                    child_pid[1] = fork();
                    if(child_pid[0] == 0) {
                        execl(EXE_BIN, EXE_BIN, NULL);
                        exit(EXIT_SUCCESS);
                    }              
                }
                if(child_pid[0] != 0){
                    wait(&status[0]);
                }
                if(child_pid[1] != 0){
                    wait(&status[0]);
                }               
            }
        }            
    }
    else {
        printf("Create file %s\n", FILE_NAME);
        printf("Press 'q' to end process\n");
        fflush(stdout);

        pid = getpid();        
        sprintf(pid_str, "%lu\n", (unsigned long)pid);
        fflush(stdout);
        bytes_counter = write(fd, (void*)pid_str, strlen(pid_str));
        if(bytes_counter < 0){
            printf("Can't write to file\n");
            close(fd);
            return 1;
        }
        sync();

        child_pid[0] = fork();
        if(child_pid[0] == 0) {
            execl(EXE_BIN, EXE_BIN, NULL);
            exit(EXIT_SUCCESS);
        }
        else {
            memset(pid_str, 0, sizeof(pid_str));
            sprintf(pid_str, "%lu\n%C", (unsigned long)child_pid[0], MSG_END_SIMBOL);
            do {
                bytes_counter = write(fd, (void*)pid_str, strlen(pid_str));
            }while(bytes_counter <= 0);            
            sync();

            child_pid[1] = fork();
            if(child_pid[1] == 0) {
                execl(EXE_BIN, EXE_BIN, NULL);
                exit(EXIT_SUCCESS);
            }
            else{
                if(child_pid[0] != 0){
                    wait(&status[0]);                    
                }
                if(child_pid[1] != 0){
                    wait(&status[1]);
                }
            }
        }        
    }

    while(1) {
        in = getc(stdin);
        if(in == 'q') break; 
    }
    printf("process end\n");
    fflush(stdout);

    close(fd);
    remove(FILE_NAME);
    exit(EXIT_SUCCESS);
    return 0;
}