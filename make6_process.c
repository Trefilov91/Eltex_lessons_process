#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define FILE_NAME "./new_file"
#define MSG_END_SIMBOL '*'

void get_proc_bin_name(char *bin_name);
void get_proc_bin_path(char *bin_path, char *bin_name);

int main()
{
    int fd;
    pid_t pid, ppid, child_pid[2] = {0};
    char pid_str[20] = {0};
    char *file_pid_str;
    char *file_ppid_str;
    char bin_name[50];
    char bin_path[512];
    size_t bytes_counter;
    int status[2];
    char in;

    get_proc_bin_name(bin_name);
    get_proc_bin_path(bin_path, bin_name);

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
                execl(bin_path, bin_name, NULL);
                exit(EXIT_SUCCESS);
            }
            else {
                if(pid == atol(file_pid_str)) {
                    child_pid[1] = fork();
                    if(child_pid[0] == 0) {
                        execl(bin_path, bin_name, NULL);
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
            execl(bin_path, bin_name, NULL);
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
                execl(bin_path, bin_name, NULL);
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

void get_proc_bin_name(char *bin_name)
{
    pid_t pid = getpid();
    char proc_path[100];
    char buf[512];
    char *token = NULL;
    int fd, readed_bytes;

    sprintf(proc_path, "/proc/%ld/stat", (long)pid);
    fd = open(proc_path, O_RDONLY, S_IRWXO | S_IRWXU | S_IRWXG);
    if(fd < 0) {
        printf("can not open file %s\n", proc_path);
        close(fd);
        return;
    }
    readed_bytes = read(fd, buf, sizeof(buf));
    if(readed_bytes < 0) {
        printf("can not read file %s\n", proc_path);
        close(fd);
        return;
    }

    token = strtok(buf, " ()");
    token = strtok(NULL, " ()");
    strcpy(bin_name, token);
}

void get_proc_bin_path(char *bin_path, char *bin_name)
{
    char dir_path[500];
    getcwd(dir_path, sizeof(dir_path));
    sprintf(bin_path, "%s/%s", dir_path, bin_name);
}