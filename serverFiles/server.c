#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/stat.h>


void error(const char *msg) //used to print errors.
{
    perror(msg);
    exit(1);
}

void handle_request(int socket, char requestBuffer[]) {
    int n = read(socket, requestBuffer, 255); // Reads in the values from the client
    if (n < 0) error("ERROR reading from socket");

    char* endFlag = strstr(requestBuffer, "<END>");
    if (endFlag != NULL) {
        *endFlag = '\0'; // Replace the start of "<END>" with a null terminator
    }
    printf("Here is the message: %s\n", requestBuffer); // Prints the values

    // Open the requested file
    FILE *file = fopen(requestBuffer, "rb"); // Open in binary mode
    if (file == NULL) {
        n = write(socket, "ERROR: File not found ", 22);
        error("ERROR opening file");
    }

    // Get file size
    struct stat st;
    stat(requestBuffer, &st);
    long fileSize = st.st_size;
    printf("The file size is %ld bytes\n", fileSize);

    // Allocate memory for file buffer
    char *fileBuffer = (char *)malloc(fileSize);
    if (fileBuffer == NULL) {
        error("ERROR allocating memory");
    }

    // Read file into buffer
    fread(fileBuffer, sizeof(char), fileSize, file);
    if (ferror(file)) {
        free(fileBuffer);
        fclose(file);
        error("ERROR reading file");
    }
    fclose(file);
    char fileSizeMsg[256]; // Buffer for the file size message
    snprintf(fileSizeMsg, sizeof(fileSizeMsg), "%ld<END>", fileSize);

    n = write(socket, fileSizeMsg, strlen(fileSizeMsg));
    if (n < 0) {
        error("ERROR writing to socket");
    }
        // Send file to the client in chunks
    long bytesSent = 0;
    while (bytesSent < fileSize) {
        n = write(socket, fileBuffer + bytesSent, fileSize - bytesSent);
        if (n < 0) {
            free(fileBuffer);
            error("ERROR writing to socket");
        }
        bytesSent += n;
    }

    free(fileBuffer); // Free the allocated memory
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    if (argc < 2) //checks for argument length
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0); //creates a socket
    if (sockfd < 0)
        error("ERROR opening socket");

    bzero((char *)&serv_addr, sizeof(serv_addr)); //trashes garbage values
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;         //IPv4
    serv_addr.sin_addr.s_addr = INADDR_ANY; //allowing any IP address to access this server
    serv_addr.sin_port = htons(portno);     //sets port number

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) //checks for error
        error("ERROR on binding");

    listen(sockfd, 5); //listens on the port
    clilen = sizeof(cli_addr);

    while (1)
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen); //accepts 1 request from a new client
        if (newsockfd < 0)
            error("ERROR on accept");

        handle_request(newsockfd, buffer);

        close(newsockfd); //closes the new socket
    }

    close(sockfd); //closes the port
    return 0;
}
