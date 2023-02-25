#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

// Server Fifo
#define REQUEST_FIFO "request.fifo"

// declarations
char my_fifo_name [128];
int connect_to_server(int, int, int);
int client_request(int, int, int);
void exit_request(int, int, int);
void terminate_request(int, int, int);

// main method
int main()
{
    int case_number;
    int loop_counter= 1;
    int request_fifo_fd;  // server fifo
    int response_fifo_fd; // client fifo

    pid_t pid;

    int system_call_number;
    int num_params;
    int size_params;

    // Get the process ID
    pid = getpid();

    // Connect to server FIFO, create client FIFO, and send connection info to server
    if (connect_to_server(request_fifo_fd, response_fifo_fd, pid) != 0)
    {
        perror("Connection failure");
        return 1;
    }

    // Inifite loop to ask user what to do next
    while (loop_counter)
    {
        printf("What to do now?\n");
        printf("1. Send a request to server\n2. Exit\n3. Terminate\n");
        scanf("%d", &case_number);
        //Switch between user choices
        switch (case_number)
        {
            case 1:
                client_request(request_fifo_fd, response_fifo_fd, pid);
                break;

            case 2:
                // Exit, this client doesnt want to issue more requests
                exit_request(request_fifo_fd, response_fifo_fd, pid);
                loop_counter = 0;
                break;

            case 3:
                // Terminate, this client doesnt want to issue more requests and signals server to term
                terminate_request(request_fifo_fd, response_fifo_fd, pid);

                loop_counter = 0;
                break;

            default:
                printf("Error: Incorrect selection. What to do now?");
                printf("1. Send a request to server  2. Exit  3. Terminate");
                scanf("%d", &case_number);
                break;
        }      
    }
    // Close the request & response FIFO queue
    close(request_fifo_fd);
    unlink(REQUEST_FIFO);
    close(response_fifo_fd);
    unlink(my_fifo_name);

    return 0;
}

// function definitions

// Create the client-specific FIFO using an appropriate name (e.g., “./ClientNfifo”), 
// where N is the client number and send the initial “connect system call” to the server 
// including client number and name of the client-specific FIFO.
int connect_to_server(int request_fifo_fd, int response_fifo_fd, int pid)
{
    // hard-coded because this is first request by client ("connect system call")
    int system_call_number = 1;
    int num_params = 1;
    int size_params;

    // Create client fifo utilizing process id
    sprintf(my_fifo_name, "./client%dfifo", pid);
    if (mkfifo (my_fifo_name, 0666) == -1)
    {  
        perror ("mkfifo");
    }
    printf("Created client FIFO... \n");

    // Open the request FIFO queue for writing
    request_fifo_fd = open(REQUEST_FIFO, O_WRONLY);
    if (request_fifo_fd < 0)
    {
        perror("open");
        return 1;
    }
    printf("Opened server FIFO to write... \n");

    // Calculate the size of the rest of the message
    size_params = sizeof(my_fifo_name)*2;

    //Hard coded 1 system call
    write(request_fifo_fd, &pid, sizeof(pid_t));
    write(request_fifo_fd, &system_call_number, sizeof(int));
    write(request_fifo_fd, &num_params, sizeof(int));
    write(request_fifo_fd, &size_params, sizeof(int));
    write(request_fifo_fd, &my_fifo_name, sizeof(char));
    printf("Sent initial request containing client FIFO [%s]]... \n", my_fifo_name);
    

    printf("Opening client FIFO for reading... \n");
    // Open the response/client FIFO queue for reading
    response_fifo_fd = open(my_fifo_name, O_RDONLY);
    if (response_fifo_fd < 0)
    {
        perror("open");
        return 1;
    }

    printf("Connection to server established... \n");

    return 0;
}

// TODO: <copy requirements from homework doc here>
int client_request(int request_fifo_fd, int response_fifo_fd, int pid)
{
    int system_call_number;
    int num_params;
    int size_params;

    //Send request to server
    printf("Please choose what system call to do: \n");
    printf("1. Connect to server\n2. Convert number to text \n3. Convert Text to Number\n");
    printf("4. Store\n5. Recall\n0. Exit\n-1. Terminate \n");

    // Read system call information from the user
    printf("Enter system call number: \n");
    scanf("%d", &system_call_number);
    printf("Enter number of parameters: \n");
    scanf("%d", &num_params);

    // Calculate the size of the rest of the message
    size_params = num_params * sizeof(int);

    // Write the system call information to the request FIFO queue
    write(request_fifo_fd, &pid, sizeof(pid_t));
    write(request_fifo_fd, &system_call_number, sizeof(int));
    write(request_fifo_fd, &num_params, sizeof(int));
    write(request_fifo_fd, &size_params, sizeof(int));

    // Write the parameters to the request FIFO queue
    for (int i = 0; i < num_params; i++)
    {
        int param;
        printf("Enter parameter %d: \n", i + 1);
        scanf("%d", &param);
        write(request_fifo_fd, &param, sizeof(int));
    }
    printf("Sent request");

    // TODO: <how do we return system call information?>
    return 0;
}

// EXIT - indicates THIS client does not want to issue more requests to the server,
// it should send a “EXIT” system call to the server, 
// close its client specific FIFO, delete it and exit.
void exit_request(int request_fifo_fd, int response_fifo_fd, int pid)
{
    // hard-coded because this is exit request by client
    int system_call_number = 0;
    int num_params = 0;
    int size_params = 0;

    //Hard coded 1 system call
    write(request_fifo_fd, &pid, sizeof(pid_t));
    write(request_fifo_fd, &system_call_number, sizeof(int));
    write(request_fifo_fd, &num_params, sizeof(int));
    write(request_fifo_fd, &size_params, sizeof(int));
    write(request_fifo_fd, &my_fifo_name, sizeof(int));
    printf("Sent exit request to server... \n");
}

// TERMINATE - indicates THIS client does not want to issue more requests to the server, 
// and is flagging the server to also exit. it should send a “TERMINATE” system call to 
// the server, close its client specific FIFO, delete it and exit.
void terminate_request(int request_fifo_fd, int response_fifo_fd, int pid)
{
    // hard-coded because this is exit request by client
    int system_call_number = -1;
    int num_params = 0;
    int size_params = 0;

    //Hard coded 1 system call
    write(request_fifo_fd, &pid, sizeof(pid_t));
    write(request_fifo_fd, &system_call_number, sizeof(int));
    write(request_fifo_fd, &num_params, sizeof(int));
    write(request_fifo_fd, &size_params, sizeof(int));
    write(request_fifo_fd, &my_fifo_name, sizeof(int));
    printf("Sent terminate request to server... \n");
}