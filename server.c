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
#define MAX_BUFFFER 1000

//Useful for debugging
void pid_printf(const char* format, ...) {
    pid_t pid = getpid();
    va_list args;
    va_start(args, format);
    fprintf(stdout, "[PID %d] - ", pid);
    vfprintf(stdout, format, args);
    va_end(args);
}

void text_to_num(char *num, int response_fifo_fd, pid_t client_pid)
{
  // Define lookup table for text-to-number conversion
    const char* number_strings[] = {"zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};
    const int num_numbers = sizeof(number_strings) / sizeof(char*);
    const int default_value = -1;
    
    // Search for the corresponding numerical value in the lookup table
    int number_value = default_value;
    for (int i = 0; i < num_numbers; i++) {
        if (strcmp(num, number_strings[i]) == 0) {
            number_value = i;
            break;
        }
    }
    
    // Send response message back to the client
    //edit
    int response_size = sizeof(number_value);
    char* response_buffer = (char*) malloc(response_size);
    memcpy(response_buffer, &number_value, response_size);
    write(response_fifo_fd, response_buffer, response_size);
    free(response_buffer);
    
    // Print message to the screen indicating the system call received
    printf("Client pid: %d\nSystem Call Requested: 3 with 1 parameter which is:\nParam1=%s result=%d\n", client_pid, num, number_value);
}

void num_to_text(char* num)
{
    int len = strlen(num); // Get number of digits in given number
 
    /* Base cases */
    if (len == 0) {
        fprintf(stderr, "empty string\n");
        return;
    }
    if (len > 4) {
        fprintf(stderr,
                "Length more than 4 is not supported\n");
        return;
    }
 
    /* The first string is not used, it is to make
        array indexing simple */
    char* single_digits[]
        = { "zero", "one", "two",   "three", "four",
            "five", "six", "seven", "eight", "nine" };
 
    /* The first string is not used, it is to make
        array indexing simple */
    char* two_digits[]
        = { "",          "ten",      "eleven",  "twelve",
            "thirteen",  "fourteen", "fifteen", "sixteen",
            "seventeen", "eighteen", "nineteen" };
 
    /* The first two string are not used, they are to make
        array indexing simple*/
    char* tens_multiple[] = { "",       "",        "twenty",
                              "thirty", "forty",   "fifty",
                              "sixty",  "seventy", "eighty",
                              "ninety" };
 
    char* tens_power[] = { "hundred", "thousand" };
 
    /* Used for debugging purpose only */
    printf("\n%s: ", num);
 
    /* For single digit number */
    if (len == 1) {
        printf("%s\n", single_digits[*num - '0']);
        return;
    }
 
    /* Iterate while num is not '\0' */
    while (*num != '\0') {
 
        /* Code path for first 2 digits */
        if (len >= 3) {
            if (*num - '0' != 0) {
                printf("%s ", single_digits[*num - '0']);
                printf("%s ",tens_power[len - 3]); // here len can be 3 or 4
            }
*/
// ------------------------------


            // Done talking to client so close FIFO and exit fork/child here
            close(client_fifo_fd);
            unlink(client_fifo_name);
            exit(0);
        }
        else {
            /* Need to explicitly handle 10-19. Sum of the
            two digits is used as index of "two_digits"
            array of strings */
            if (*num == '1') {
                int sum = *num - '0' + *(num + 1) - '0';
                printf("%s\n", two_digits[sum]);
                return;
            }
 
            /* Need to explicitly handle 20 */
            else if (*num == '2' && *(num + 1) == '0') {
                printf("twenty\n");
                return;
            }
 
            /* Rest of the two digit numbers i.e., 21 to 99
             */
            else {
                int i = *num - '0';
                printf("%s ", i ? tens_multiple[i] : "");
                ++num;
                if (*num != '0')
                    printf("%s ",
                           single_digits[*num - '0']);
            }
        }
        ++num;
    }
}

int main() {
    // Variables to print buffer request
    char request_bufffer[MAX_BUFFFER] = {0};
    int print_position=0;

    // Make server FIFO
    pid_printf("Creating server FIFO\n");
    if (mkfifo(SERVER_FIFO_NAME, 0666) == -1) {
        perror ("Error making server FIFO\n");
    }

    // Open server FIFO to start reading for requests
    pid_printf("Opening server FIFO\n");
    int server_fifo_fd = open(SERVER_FIFO_NAME, O_RDONLY);
    if (server_fifo_fd == -1) {
        perror("Server fifo error\n");
        return 1;
    }
    pid_printf("Server started. Waiting for client requests...\n");

    // Loop to read from well-known server FIFO
    while (1) {
        
        // Declaring variables
        char optional_param[MAX_PARAM_LENGTH];
        int system_call_number;
        int num_params;
        int size_params;
        pid_t client_pid;
        ssize_t bytes_read;
        int num_clients;
        int value;

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
        read(server_fifo_fd, &optional_param, MAX_PARAM_LENGTH);
        pid_printf("Parsed client request successfully! \n");

        // PID print is ugly here so not using it
        pid_printf("----------------------------------------------------\n");
        print_position += snprintf(request_bufffer + print_position, MAX_BUFFFER - print_position, "Client pid: %d\n", client_pid);
        print_position += snprintf(request_bufffer + print_position, MAX_BUFFFER - print_position, "System call requested: %d\n", system_call_number);
        print_position += snprintf(request_bufffer + print_position, MAX_BUFFFER - print_position, "with %d ", num_params);
        print_position += snprintf(request_bufffer + print_position, MAX_BUFFFER - print_position, "parameter: %s\n", optional_param);
        print_position += snprintf(request_bufffer + print_position, MAX_BUFFFER - print_position, "size of params: %d", size_params);
        printf("%s\n", request_bufffer);
        pid_printf("----------------------------------------------------\n");

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


// ------------------------------
// TODO: Yannie edit here. I didn't change anything so you'll have to adapt it to this program.

            // Perform the requested system call (in this case, just convert and store the parameters)
            switch (system_call_number)
            {
                case -1:
                    printf("Client pid: %d\nSystem Call Requested: Terminate\n", client_pid);
                    exit(0);
                    break;
                case 0:
                    printf("Client pid: %d requested to exit.\n", client_pid);
                    close(server_fifo_fd); // close client-specific FIFO
                    exit(0); // terminate server program
                    break;
                case 1:
                    // Verify the num of params =  1 
                    if(num_params != 1){
                        printf("Invalid request from client %d: system call %d with parameters %d\n", client_pid,system_call_number,num_params);
                        return -1;
                    }

                    //Opening client fifo 
                    pid_printf("Opening client FIFO: %s\n", client_fifo_name);
                    int client_fifo_fd = open(client_fifo_name, O_WRONLY);
                    //Verify client fifo openned 
                    if(client_fifo_fd ==-1){
                        printf("Error opening client!\n");
                        return -1;
                    }
                    break;
                case 2:
                    //Verify client request
                    if(system_call_number !=2){
                        printf("Invalid request from client %d: system call %d with parameters %d\n",client_pid,system_call_number,num_params);
                        return-1;
                    }

                    num_to_text(optional_param);

                    break;
                case 3:
                    //Verify client request
                    if(num_params !=1){
                        printf("Invalid request from clien %d: system call %d with parameters %d\n",client_pid,system_call_number,num_params);
                        return-1;
                    }
                    //send request to function
                    text_to_num(optional_param, client_fifo_fd,client_pid);
                    break;
                case 4:
                    //store(params,client_pid, client_fifo_fd, request_fifo_fd);
                    break;
                case 5:
                    //recall(client_pid, response_fifo_fd, request_fifo_fd);
                    break;

                default:
                    break;
            }
// ------------------------------


            // Done talking to client so close FIFO and exit fork/child here
            pid_printf("---!!!--- END CLIENT REQUEST ---!!!---\n");
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

