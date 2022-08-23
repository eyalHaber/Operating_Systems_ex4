//Eyal Haber 203786298
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wait.h>

void clean() {
    char to_srv[] = "to_srv.txt";
    // if the file exists remove it:
    if (access(to_srv, F_OK) == 0) {
        remove(to_srv);
    }
}

const char *get_result(int n1, int n2, int operator)
{
    static char result[100];

    if(operator == 1){
        sprintf(result, "%d", (n1 + n2));
    }
    if(operator == 2){
        sprintf(result, "%d", (n1 - n2));
    }
    if(operator == 3){
        sprintf(result, "%d", (n1 * n2));
    }
    if(operator == 4){
        if(n2==0){
            return "CANNOT_DIVIDE_BY_ZERO\n";
        } else {
            sprintf(result, "%d", (n1 / n2));
        }
    }
    return result;
}

void client_handler(int sig) {
    // should open new fork() for the new client
    pid_t pid = fork(); // create child process (int)
    if (pid == -1) { // fork() failed
        exit(1);
    }
    if (pid == 0) // current client process
    {
        char to_srv[] = "to_srv.txt";
        // open the "to_srv.txt" file:
        if (access(to_srv, F_OK) != 0) {
            // file doesn't exist
            printf("ERROR_FROM_EX4\n"); //  (to_srv.txt doesn't exist)
        } else {
            // file exist, so we can read it and calculate the result
            FILE *fp_r = fopen(to_srv, "r");
            if (fp_r == NULL) {  // failed to open file
                printf("ERROR_FROM_EX4\n"); //  (failed to open o_srv.txt)
                //kill(getpid(), SIGTERM); // terminate current process
                exit(1);
            }

            // create "to_client_X.txt" file:
            char client_file[100] = "to_client_";
            char txt[100] = ".txt";
            //char curr_client_file[100];
            // read the "to_srv.txt" file lines:
            char line[100];
            // lines will be:
            char client_pid[100]; // 1 -> name f the file: "to_client_"
            char num1[100]; // 2 -> first number
            char op[2]; // 3 -> Calculation code: 1='+', 2='-', 3='*', 4='/'
            char num2[100]; // 4 -> second number
            int i = 1; // number of line to read
            // scan lines:
            fscanf(fp_r, "%99[^\n]\n%99[^\n]\n%2[^\n]\n%99[^\n] ", client_pid, num1, op, num2);
            // remove the to_srv file as soon as possible so a new client can create it!
            fclose(fp_r);
            remove(to_srv);

            strcat(client_file, client_pid);
            strcat(client_file, txt);

            if (access(client_file, F_OK) == 0) {
                // file exist
                printf("ERROR_FROM_EX4\n"); //  (client_file with that pid already exist)
                exit(1);
            } else {
                // file doesn't exist, so we can make it
                FILE *fp_w = fopen(client_file, "w");
                if (fp_w == NULL) {  // failed to open file
                    printf("ERROR_FROM_EX4\n"); //  (failed to open client_file)
                    exit(1);
                }
                // CALCULATE:
                int n1 = atoi(num1);
                int n2 = atoi(num2);
                int operator = atoi(op);
                const char *answer = get_result(n1, n2, operator);
                fputs(answer, fp_w);
                // close file
                fclose(fp_w); // the client will remove this file
                // sent signal to the client that his answer file is ready:
                kill(atoi(client_pid), SIGUSR1);
                // finished handling this client:
                exit(0);
            }
        }
    }
}

void alarm_handler(int sig)
{
    printf("The server was closed because no service request was received for the last 60 seconds\n");
    clean(); // clean "to_srv.txt" file
    exit(0);
}


int main()
{

    // handle alarm signal
    signal(SIGALRM, alarm_handler);
    // get signal from client and handle him:
    signal(SIGUSR1, client_handler);
    // prevent zombies:
    signal(SIGCHLD, SIG_IGN);

    // to_srv.txt should be removed before clients created:
    clean();

    // endless loop (until this server is closed) in order to keep receiving new clients:
    while(1) {
        // wait for clients:
        alarm(60);
        pause();
    }
}




