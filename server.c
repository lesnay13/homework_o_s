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

int connect_system(pid_t, int);
void num_to_text(char*);
void text_to_num();
void store(char*,int);
void recall();
void terminate_connection();

int main()
{
    int request_fifo_fd;
    int *client_fifo_name;
    int response_fifo_fd;
    pid_t client_pid;
    int system_call_number;
    int num_params;
    int size_params;
    char *params;
    char *num;

    // Create the well-known request FIFO queue
    mkfifo(REQUEST_FIFO, 0666);

    // Open the request FIFO queue for reading
    request_fifo_fd = open(REQUEST_FIFO, O_RDONLY);
    if (request_fifo_fd < 0)
    {
        perror("open");
        return 1;
    }

    // Enter an infinite loop to process client requests
    while (1)
    {

        printf("Received request from client in well-known FIFO... \n");
        // Read the client PID, system call number, and number of parameters
        read(request_fifo_fd, &client_pid, sizeof(pid_t));
        read(request_fifo_fd, &system_call_number, sizeof(int));
        read(request_fifo_fd, &num_params, sizeof(int));
        read(request_fifo_fd, &size_params, sizeof(int));

        // Allocate memory for the parameters
        params = (char *)malloc(size_params*2);

        printf("Trying to read params...\n");
        read(request_fifo_fd, &params, sizeof(int));
        printf("Received %ls \n", &params);
        // Read the parameters
        /*for (int i = 0; i < num_params; i++)
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
                terminate_connection();
                break;
            case 0:
                exit(0);
                break;
            case 1:
                connect_system(client_pid, response_fifo_fd);
                break;
            case 2:
                num_to_text(num);
                break;
            case 3:
                text_to_num();
                break;
            case 4:
                //store(params, num_params);
                break;
            case 5:
                recall();
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
        free(num);
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

void text_to_num()
{
    // int size= sizeof(text);
    // int numConverted = atoi(text);

    // printf("Converting text to number:  %d\n", numConverted);
}

void store(char *params, int num_params)
{
    /*for (int i = 0; i < num_params; i++)
    {
        params[i] = params[i] * 2;
    }*/
}
void recall()
{

}

void terminate_connection()
{

}