#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg) // a method to print errors.
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n; // declares variables.
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3) // prints errors if there aren't enough arguments.
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // creats the socket connection.
    if (sockfd < 0)
        error("ERROR opening socket");

    server = gethostbyname(argv[1]); // Gets server address.
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr)); //erases garbage values
    serv_addr.sin_family = AF_INET;               // sets the address family to IPv4
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno); //sets the port number

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting"); //prints errors

    char tempBuffer[256];

    printf("Please enter the filename of the desired file: ");
    bzero(buffer, 256); // Clear buffer
    fgets(tempBuffer, 255, stdin);

    // Remove newline character from fgets, if present
    tempBuffer[strcspn(tempBuffer, "\n")] = 0;

    // Concatenate the filename and "<END>" flag
    snprintf(buffer, sizeof(buffer), "%s<END>", tempBuffer);

    n = write(sockfd, buffer, strlen(buffer));//attempts to write the message to the socket
    if (n < 0)
        error("ERROR writing to socket");

    bzero(buffer, 256);
    n = read(sockfd, buffer, 255); //reads the response from the server.
    if (n < 0)
        error("ERROR reading from socket");
    char* endFlag = strstr(buffer, "<END>");
    if (endFlag != NULL) {
        size_t beforeEndSize = endFlag - buffer;
        size_t afterEndSize = strlen(endFlag + strlen("<END>"));

        // Allocate memory for the new strings (+1 for null terminator)
        char* beforeEnd = (char*)malloc(beforeEndSize + 1);
        char* afterEnd = (char*)malloc(afterEndSize + 1);

        if (beforeEnd == NULL || afterEnd == NULL) {
            // Handle memory allocation error
        } else {
            // Copy the parts to new strings
            strncpy(beforeEnd, buffer, beforeEndSize);
            beforeEnd[beforeEndSize] = '\0'; // Null-terminate the string

            strcpy(afterEnd, endFlag + strlen("<END>")); // Copy the part after "<END>"

            // Use 'beforeEnd' and 'afterEnd' as needed
            // ...
            printf("File Size is: %s\n", beforeEnd);
            // printf("After <END>: %s\n", afterEnd);

            int fileSize = atoi(beforeEnd);
            int bytesReceived = strlen(afterEnd);
            printf("%s", afterEnd);
            while(bytesReceived < fileSize){
                n = read(sockfd, buffer, 255);
                printf("%s", buffer);
                bytesReceived += n;
                bzero(buffer,256);
                // printf("Bytes Received: %d\n", bytesReceived);
            }
            printf("\n");

            // Free the allocated memory
            free(beforeEnd);
            free(afterEnd);
        }
    }

    
    

    close(sockfd);
    return 0;
}
