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
    int system_call_number;
    int num_params;
    int size_params;


    // Get the process ID
    pid_t pid;
    pid = getpid();

    // Open server FIFO for writing
    pid_printf("Opening server FIFO: %s\n", SERVER_FIFO_NAME);
    int server_fifo_fd = open(SERVER_FIFO_NAME, O_WRONLY);
    if (server_fifo_fd == -1) {
        perror("Server fifo doesn't exist\n");
        return 1;
    }

    // Creating our FIFO so the server can attempt to open it when it reads the request
    pid_printf("Creating client FIFO\n");
    char client_fifo_name[MAX_PARAM_LENGTH];
    sprintf(client_fifo_name, CLIENT_FIFO_PREFIX"%d", pid);
    if (mkfifo (client_fifo_name, 0666) == -1)
    {  
        perror ("Error making client fifo\n");
    }

    // Calculate the size of the rest of the message
    // hardcode other params for connect request
    system_call_number = 1;
    num_params = 1;
    size_params = sizeof(client_fifo_name);

    // Writing the params for the connect request to the server FIFO
    pid_printf("Writing connect request to server FIFO\n");
    write(server_fifo_fd, &pid, sizeof(pid));
    write(server_fifo_fd, &system_call_number, sizeof(system_call_number));
    write(server_fifo_fd, &num_params, sizeof(num_params));
    write(server_fifo_fd, &size_params, sizeof(size_params));
    write(server_fifo_fd, &client_fifo_name, sizeof(client_fifo_name));

    // Open client FIFO for reading
    pid_printf("opening client FIFO: %s\n", client_fifo_name);
    int client_fifo_fd = open(client_fifo_name, O_RDONLY);
    if (client_fifo_fd == -1) {
        perror("client FIFO open error\n");
        return 1;
    }

    // Read server response from client FIFO
    pid_printf("Reading server response from client FIFO\n");
    char server_response[MAX_PARAM_LENGTH];
    read(client_fifo_fd, server_response, MAX_PARAM_LENGTH);
    pid_printf("Server response: '%s'\n", server_response);


// ------------------------------
/* TODO: Yannie edit replace/here

    // Create the client request
    printf("Creating client request\n");
    char client_request[MAX_PARAM_LENGTH];
    printf("Enter a parameter: \n");
    fgets(client_request, MAX_PARAM_LENGTH, stdin);
    client_request[strcspn(client_request, "\n")] = 0;  // Remove trailing newline
*/
// ------------------------------


    // Close FIFO and unlink because we are done
    close(client_fifo_fd);
    unlink(client_fifo_name);

    // If I unlink the server here it seems to close it for everyone, so not sure how to handle it yet
    close(server_fifo_fd);

    // Exit main
    return 0;
}