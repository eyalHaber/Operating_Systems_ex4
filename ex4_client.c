//Eyal Haber 203786298
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>

void file_handler(int sig)
{
    char client_file[] = "to_client_";
    pid_t pid = getpid(); // int
    char mypid[6];
    sprintf(mypid, "%d", pid);
    strcat(client_file, mypid);
    char txt[] = ".txt";
    strcat(client_file, txt); // client_file = "client_file_'getpid()'.txt"
    // copy the client_file in order ro later remove him:
    char curr_client_file[100];
    //printf("client_file:      %s\n", client_file);
    strcpy(curr_client_file, client_file);
    // open the current client file:
    FILE *fp = fopen(curr_client_file, "r");
    if(fp == NULL) { // failed to open file
        printf("ERROR_FROM_EX4\n");
        exit(0);
    }
    char answer[100];
    // reads answer in text until newline is encountered
    fscanf(fp, "%[^\n]", answer);
    printf("%s\n", answer);
    fclose(fp); // close file
    // remove the client_file:
    remove(curr_client_file);
    exit(0);
}

void alarm_handler(int sig)
{
    printf("Client closed because no response was received from the server for 30 seconds\n");
    remove("to_srv.txt"); // release this file name to other clients
    exit(0);
}

int main(int argc,char* argv[])
{
    if(argc < 5) { // not enough parameters
        printf("ERROR_FROM_EX4\n");
        exit(1); // number between 1 and 4 should be entered
    }
    // 4 parameters
    char *P1 = argv[1]; // server PID (send signal to this SERVER's PID)
    char *P2 = argv[2]; // first num (second argument in "to_srv")
    char *P3 = argv[3]; // Calculation code: 1='+', 2='-', 3='*', 4='/' (third argument in "to_srv")
    if(atoi(P3)<1 || atoi(P3)>4){
        printf("ERROR_FROM_EX4\n");
        exit(1); // number between 1 and 4 should be entered
    }
    char *P4 = argv[4]; // second num (fourth argument in "to_srv")

    pid_t server = atoi(P1);
    pid_t pid = getpid(); // get this client's PID (first argument in "to_srv")
    char client_pid[6];  // create a char pid to be converted to
    sprintf(client_pid, "%d", pid);  // convert the pid to string

    // signal handler calls
    signal(SIGUSR1, file_handler);
    signal(SIGALRM, alarm_handler);

    // create the "to_srv.txt" file for the server to read
    int success_flag = 0;
	int i;
    for (i = 0; i < 10; i++) {  // try 10 times to create the file
        srand(time(NULL));  // sead the randomization
        int rand_num = (rand() % 5) + 1; // generate random number between 1 to 5 seconds
        sleep(rand_num); // sleep for rand_num seconds
        if (access("to_srv.txt", F_OK) == 0) {
            // file exists and being used by the server from other client, so we try again if we can
        } else {
            // file doesn't exist, so we can create it and connect the server
            FILE *fp = fopen("to_srv.txt", "w");
            if(fp == NULL) {  // failed to open file
                printf("ERROR_FROM_EX4\n");
                exit(1);
            }
            fputs(client_pid, fp);
            fputs("\n", fp);
            fputs(P2, fp);
            fputs("\n", fp);
            fputs(P3, fp);
            fputs("\n", fp);
            fputs(P4, fp);
            // close file
            fclose(fp);
            // change success flag
            success_flag = 1;
            // file was successfully created, so now we send signal to server with the P1 pid
            kill(server, SIGUSR1); // sent signal to the server
            alarm(30); // wait 30 seconds for the server's response
            pause(); // wait for the alarm or a signal from the server
        }
    }
    if (success_flag == 0) { // finished 10 tries to create a file without success
        printf("ERROR_FROM_EX4\n");
        exit(0);
    }
}

