#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define MAX_PARAM_LENGTH 256
#define SERVER_FIFO_NAME "./server_fifo"
#define CLIENT_FIFO_PREFIX "./client_fifo_"

// Useful for debugging
void pid_printf(const char* format, ...) {
    pid_t pid = getpid();
    va_list args;
    va_start(args, format);
    fprintf(stdout, "[PID %d] - ", pid);
    vfprintf(stdout, format, args);
    va_end(args);
}


int main() {
    // Declaring variables
    int system_call_number;
    int num_params;
    int size_params;
    int case_number;
    char optional_param[MAX_PARAM_LENGTH];
    char empty_string[] = "";
    int loop_continue = 1;



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
// TODO: Yannie edit replace/here
pid_printf("Connection to server established... \n");

    // Inifite loop to ask user what to do next
    while (loop_continue)
    {
        printf("What to do now?\n");
        printf("1. Send a request to server\n2. Exit\n3. Terminate\n");
        scanf("%d", &case_number);

        //Switch between user choices
        switch (case_number)
        {
            case 1:
                pid_printf("Client request chosen... \n");

                //Send request to server
                printf("Please choose what system call to do: \n");
                printf("1. Connect to server\n2. Convert number to text \n3. Convert Text to Number\n");
                printf("4. Store\n5. Recall\n0. Exit\n-1. Terminate \n");

                // Read system call information from the user
                printf("Enter system call number: \n");
                scanf("%d", &system_call_number);
                printf("Enter number of parameters [0 or 1]: \n");
                scanf("%d", &num_params);

                // According to project spec we only require one or zero parameters right now
                if (num_params != 0 && num_params > 1) {
                    // Wrong value, start loop over
                    pid_printf("Error: Number of parameters must equal 0 or 1.");
                    continue;
                }

                if (num_params == 1) {
                    // read in user input and store in optional param as string
                    // can change the data type on server side if it is an int
                    printf("Enter parameter value: \n");
                    fflush(stdout); // Flush the output buffer to ensure prompt is displayed
                    //fgets(optional_param, MAX_PARAM_LENGTH, stdin);
                    scanf("%s", optional_param);
                   

                    // fgets adds a newline, so replace it with string termination null char
                   optional_param[strcspn(optional_param, "\n")] = '\0';
                    printf("You entered: %s\n", optional_param);
                }

                if (num_params == 0) {
                    // if the client doesn't want to enter a param just make it equal empty string
                    sprintf(optional_param, "%s", empty_string);
                }

                // PID print is ugly here so not using it
                printf("----------------------------------------------------\n");
                printf("Client pid: %d\n", pid);
                printf("System call requested: %d\n", system_call_number);
                printf("with %d ", num_params);
                printf("parameter: %s\n", optional_param);
                printf("size of params: %d\n", size_params);
                printf("----------------------------------------------------\n");

                // Write the system call information to the request FIFO queue
                write(server_fifo_fd, &pid, sizeof(pid_t));
                write(server_fifo_fd, &system_call_number, sizeof(int));
                write(server_fifo_fd, &num_params, sizeof(int));
                write(server_fifo_fd, &size_params, sizeof(int));
                write(server_fifo_fd, &optional_param, sizeof(optional_param));


                pid_printf("Sent client request to server... \n");
                
                break;

            // EXIT - indicates THIS client does not want to issue more requests to the server,
            // it should send a “EXIT” system call to the server, 
            // close its client specific FIFO, delete it and exit.
            case 2:
                // Exit, this client doesnt want to issue more requests
                pid_printf("Exit request chosen... \n");

                //Hard coded vars for exit request
                sprintf(optional_param, "%s", empty_string);
                system_call_number = 0;
                num_params = 0;
                size_params = sizeof(optional_param);
                
                // Hard coded call
                write(server_fifo_fd, &pid, sizeof(pid_t));
                write(server_fifo_fd, &system_call_number, sizeof(int));
                write(server_fifo_fd, &num_params, sizeof(int));
                write(server_fifo_fd, &size_params, sizeof(int));
                write(server_fifo_fd, &optional_param, sizeof(optional_param));
                pid_printf("Sent exit request to server... \n");

                // exit the loop via var
                // break here just ends the switch, which is needed to avoid default behavior
                loop_continue = 0;
                break;

            // TERMINATE - indicates THIS client does not want to issue more requests to the server, 
            // and is flagging the server to also exit. it should send a “TERMINATE” system call to 
            // the server, close its client specific FIFO, delete it and exit.
            case 3:
                // Terminate, this client doesnt want to issue more requests and signals server to term
                pid_printf("Terminate request chosen... \n");

                //Hard coded vars for term request
                sprintf(optional_param, "%s", empty_string);
                system_call_number = -1;
                num_params = 0;
                size_params = sizeof(optional_param);
                
                // Hard coded call
                write(server_fifo_fd, &pid, sizeof(pid_t));
                write(server_fifo_fd, &system_call_number, sizeof(int));
                write(server_fifo_fd, &num_params, sizeof(int));
                write(server_fifo_fd, &size_params, sizeof(int));
                write(server_fifo_fd, &optional_param, sizeof(optional_param));
                pid_printf("Sent terminate request to server... \n");

                // exit the loop via var
                // break here just ends the switch, which is needed to avoid default behavior
                loop_continue = 0;
                break;

            default:
                pid_printf("Error: Incorrect case selection.\n");
                continue;
        }      
    }

// ------------------------------

    // Close FIFO and unlink because we are done
    close(client_fifo_fd);
    unlink(client_fifo_name);

    // If I unlink the server here it seems to close it for everyone, so not sure how to handle it yet
    close(server_fifo_fd);

    // Exit main
    return 0;
}