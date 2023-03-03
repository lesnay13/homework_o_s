#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define MAX_PARAM_LENGTH 128
#define SERVER_FIFO_NAME "./server_fifo"
#define CLIENT_FIFO_PREFIX "./client_fifo_"

// Not entirely sure how this function works but it is useful for debugging
void pid_printf(const char* format, ...) {
    pid_t pid = getpid();
    va_list args;
    va_start(args, format);
    fprintf(stdout, "[PID %d] - ", pid);
    vfprintf(stdout, format, args);
    va_end(args);
}


int main() {

    // Make server FIFO
    pid_printf("Creating server FIFO\n");
    if (mkfifo(SERVER_FIFO_NAME, 0666) == -1) {
        perror ("Error making server FIFO\n");
    }

    // Open server FIFO to start watching for requests
    pid_printf("Opening server FIFO\n");
    int server_fifo_fd = open(SERVER_FIFO_NAME, O_RDONLY);
    if (server_fifo_fd == -1) {
        perror("Server fifo error\n");
        return 1;
    }
    pid_printf("Server started. Waiting for client requests...\n");

    // Loop to read from well-known server FIFO
    while (1) {
        
        // if I set these in the loop does it stop? 
        // (Answer was no but they are fine here as they fit the scope of the loop)
        char optional_param[MAX_PARAM_LENGTH];
        int system_call_number;
        int num_params;
        int size_params;
        pid_t client_pid;
        ssize_t bytes_read;

        // Read client requests as they come in and check for EOF lines so we can avoid infinite loop.
        bytes_read = read(server_fifo_fd, &client_pid, sizeof(pid_t));
        if (bytes_read == -1) {
            perror("Read request");
            exit(1);
        }
        // Found end of file so don't fork infinitely. Client has closed its end of the FIFO.
        if (bytes_read == 0) {
            // Continue listening for new client requests
            continue;
        }

        // Client request found so lets read it in. If there is no request it will just block here.
        pid_printf("---!!!--- NEW CLIENT REQUEST ---!!!---\n");
        read(server_fifo_fd, &system_call_number, sizeof(system_call_number));
        read(server_fifo_fd, &num_params, sizeof(num_params));
        read(server_fifo_fd, &size_params, sizeof(size_params));
        read(server_fifo_fd, optional_param, MAX_PARAM_LENGTH);
        pid_printf("Parsed client request successfully! \n");

        // PID print is ugly here so not using it
        printf("----------------------------------------------------\n");
        printf("Client pid: %d\n", client_pid);
        printf("System call requested: %d\n", system_call_number);
        printf(" with %d", num_params);
        printf(" parameter: %s\n", optional_param);
        printf(" size of params: %d\n", size_params);
        printf("----------------------------------------------------\n");

        // Fork so that server can handle multiple clients
        pid_t check_pid = fork();
        pid_printf("Fork pid value: %d\n", check_pid);

        // Check if we are child or parent
        if (check_pid < 0) {
            perror("Fork error");
            return 1;
        }
        else if (check_pid == 0) {

            // We are in the child process if fork returns 0
            pid_printf("This is the child process, let's do things with the client request...\n");
            
            // Read in the "optional_param" which is the client FIFO name in this case
            // So we are going to handle it in a specific way to construct client FIFO name
            pid_printf("Client request is: %s\n", optional_param);
            char client_fifo_name[MAX_PARAM_LENGTH * 2];
            sprintf(client_fifo_name, "%s", optional_param);

            // Open the client FIFO
            pid_printf("Opening client FIFO: %s\n", client_fifo_name);
            int client_fifo_fd = open(client_fifo_name, O_WRONLY);

            // Right now just respond with client pid so we know it is passing info back and forth
            pid_printf("Responding via client FIFO: %s\n", client_fifo_name);
            char server_response[MAX_PARAM_LENGTH];
            sprintf(server_response, "Hello Client %d", client_pid);
            write(client_fifo_fd, server_response, sizeof(server_response));

            // Write some terminal feedback and close connection
            pid_printf("Sent 'hello' response to client (pid=%d)\n", client_pid);
            pid_printf("---!!!--- END CLIENT REQUEST ---!!!---\n");


// ------------------------------
/* TODO: Yannie edit here. I didn't change anything so you'll have to adapt it to this program.

            // Perform the requested system call (in this case, just convert and store the parameters)
            switch (system_call_number)
            {
                case -1:
                    printf("Client pid: %d\nSystem Call Requested: Terminate\n", client_pid);
                    exit(0);
                    break;
                case 0:
                    printf("Client pid: %d requested to exit.\n", client_pid);
                    close(request_fifo_fd); // close client-specific FIFO
                    exit(0); // terminate server program
                    break;
                case 1:
                    connect_system(client_pid, response_fifo_fd);
                    break;
                case 2:
                    num_to_text(params);
                    break;
                case 3:
                    text_to_num(params, response_fifo_fd,client_pid);
                    break;
                case 4:
                    store(params,client_pid, response_fifo_fd, request_fifo_fd);
                    break;
                case 5:
                    recall(client_pid, response_fifo_fd, request_fifo_fd);
                    break;

                default:
                    break;
            }
*/
// ------------------------------


            // Done talking to client so close FIFO and exit fork/child here
            close(client_fifo_fd);
            unlink(client_fifo_name);
            exit(0);
        }
        else {

            // Parent process because pid return is greater than zero
            // Continue listening for new client requests
            pid_printf("This is the parent process, continuing loop to wait for new client requests...\n");
            continue; 
        }

    }

    // Close FIFO and unlink because we are done
    pid_printf("We are done, close the server FIFO\n");
    close(server_fifo_fd);
    unlink(SERVER_FIFO_NAME);

    // Exit main
    return 0;
}