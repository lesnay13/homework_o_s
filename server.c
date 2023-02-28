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
    int *client_fifo_name;
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
            params[i] = malloc(size_params);
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

int connect_system(pid_t client_pid, int response_fifo_fd)
{
    // Open the response FIFO queue for writing
    char response_fifo_name[200];
    sprintf(response_fifo_name, "%d.%s", client_pid, my_fifo_name);
    response_fifo_fd = open(response_fifo_name, O_WRONLY);
    if (response_fifo_fd < 0)
    {
        perror("open");
        return 1;
    }
    printf("Opened client FIFO for writing...\n");
    return 0;
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
                printf("%s ",
                       tens_power[len - 3]); // here len can
                                             // be 3 or 4
            }
            --len;
        }
 
        /* Code path for last 2 digits */
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
    free(num);
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

void store(char *params, pid_t client_pid, int response_fifo_fd, int request_fifo_fd)
{
    printf("Client pid: %d\n", client_pid);
    printf("System Call Requested: 4 with 1 parameter which is:\nParam1=%s\n", params);
    
    // Store the parameter value in a variable
    int storedValue = atoi(params);
    
    // Return the stored value to the client
    sprintf(request_fifo_fd, "%d", storedValue);
    int n = strlen(request_fifo_fd) + 1;
    write(response_fifo_fd, &n, sizeof(int));
    write(response_fifo_fd, request_fifo_fd, n);
}
void recall(pid_t client_pid, int response_fifo_fd, int request_fifo_fd)
{
    printf("Client pid: %d\nSystem Call Requested: 5 with 0 parameters\n", client_pid);
    int stored_value;
    if (read(response_fifo_fd, &stored_value, sizeof(int)) == -1) {
        perror("Error reading stored value");
        return 1;
    }
    if (write(request_fifo_fd, &stored_value, sizeof(int)) == -1) {
        perror("Error writing to client FIFO");
        return 1;
    }
}