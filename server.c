#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) //used to print errors.
{
    perror(msg);
    exit(1);
}

void handle_request(int socket, char requestBuffer[]){
        int n = read(socket, requestBuffer, 255); //reads in the values from the client
        if (n < 0)
            error("ERROR reading from socket");
        printf("Here is the message: %s\n", requestBuffer); //prints the values

        n = write(socket, "I got your message", 18); //writes to the client
        if (n < 0)
            error("ERROR writing to socket");
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
