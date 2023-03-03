#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

// Server Fifo
#define REQUEST_FIFO "request.fifo"
#define MAX_PARAMS 10
#define MAX_MSG_SIZE 256

// declarations
char my_fifo_name [128];

int connect_system(pid_t, int);
void num_to_text(char*);
void text_to_num(char*,int, pid_t);
void store(char*, pid_t, int, int);
void recall(pid_t, int, int);

int main()
{
    int request_fifo_fd;
    int response_fifo_fd;
    pid_t client_pid;
    char buffer[MAX_MSG_SIZE];
    int system_call_number;
    int num_params;
    int size_params;
    char *params;

    // Create the well-known request FIFO queue
    if(mkfifo(REQUEST_FIFO, 0666)==-1)
    {
        perror("Error creating server FIFO");
        exit(EXIT_FAILURE);
    }

    // Open the request FIFO queue for reading
    request_fifo_fd = open(REQUEST_FIFO, O_RDONLY);
    if (request_fifo_fd < 0)
    {
        perror("Error openining server FIFO");
        return 1;
    }

    // Enter an infinite loop to process client requests
    while (1)
    {
        //Verify that the client fifo can be read
        if(read(response_fifo_fd,buffer,sizeof(buffer))==-1)
        {
            perror("Error reading from client");
            continue;
        }

        printf("Received request from client in well-known FIFO... \n");
       
        // Read the client PID, system call number, and number of parameters
       /* read(request_fifo_fd, &client_pid, sizeof(pid_t));
        read(request_fifo_fd, &system_call_number, sizeof(int));
        read(request_fifo_fd, &num_params, sizeof(int));
        read(request_fifo_fd, &size_params, sizeof(int));*/

        memcpy(&client_pid, buffer, sizeof(client_pid));
        memcpy(&system_call_number, buffer + sizeof(client_pid), sizeof(system_call_number));
        memcpy(&num_params, buffer + sizeof(client_pid) + sizeof(system_call_number), sizeof(num_params));
        memcpy(&size_params, buffer + sizeof(client_pid) + sizeof(system_call_number) + sizeof(num_params), sizeof(size_params));
        printf("Trying to read params...\n");
        for (int i = 0; i < num_params; i++) {
            params[i] = (char *)malloc(size_params);
            memcpy(params[i], buffer + sizeof(client_pid) + sizeof(system_call_number) + sizeof(num_params) + sizeof(size_params) + i * size_params, size_params);
        }
        printf("Received %ls \n", &params);
        // Allocate memory for the parameters
        /*params = (char *)malloc(size_params*2);

        printf("Trying to read params...\n");
        read(request_fifo_fd, &params, sizeof(int));
        printf("Received %ls \n", &params);
        // Read the parameters
        for (int i = 0; i < num_params; i++)
        {
            printf("Loop 1\n");

            read(request_fifo_fd, &params[i], sizeof(int));
            printf("Received %s \n", &params[i]);
        }*/
    

        printf("System call received from client is: %d\n", system_call_number);
        printf("Client pid: %d\n", client_pid);
        printf("System call requested: %d\n", system_call_number);
        printf(" with %d", num_params);
        printf(" parameters which are: \n");
        printf(" size of params: %d\n", size_params);



        // Perform the requested system call (in this case, just convert and store the parameters)
        switch (system_call_number)
        {
            case -1:
                printf("Client pid: %d\nSystem Call Requested: Terminate\n", client_pid);
                close(request_fifo_fd);
                close(response_fifo_fd);
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
        // Write the converted parameters back to the client
        write(response_fifo_fd, &num_params, sizeof(int));
        write(response_fifo_fd, &size_params, sizeof(int));
        write(response_fifo_fd, params, size_params);

        // Close the response FIFO queue
        close(response_fifo_fd);
        unlink(my_fifo_name);

        // Free the memory for the parameters
        free(params);
    }

    // Close the request FIFO queue
    close(request_fifo_fd);
    unlink(REQUEST_FIFO);
    return 0;
}

