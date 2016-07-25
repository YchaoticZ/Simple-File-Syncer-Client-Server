#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <openssl/sha.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main (int argc, char *argv[]){
   int sockfd, newsockfd, portno, clilen, n; //inits
   char buffer[256]; //buffer for data
   struct sockaddr_in serv_addr, cli_addr; //struct for server and client addr
   struct hostent *server;

   portno = atoi(argv[2]); //converts 3rd argument to port number
   sockfd = socket(AF_INET,SOCK_STREAM,0);
   if (sockfd < 0){ 
	error("Error");
   }
   server = gethostbyname(argv[1]); //gets host name by args
   if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
   }
   bzero((char *) &serv_addr, sizeof(serv_addr)); // sets buffer to zero
   //initialize server_addr values
   serv_addr.sin_family = AF_INET; //ipv4
   serv_addr.sin_port = htons(portno); //saves port in network byte order
   bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
   if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        error("ERROR connecting");
	}

    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",buffer);
    close(sockfd);


    return 0;
}
