#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/sha.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <errno.h>

void dostuff(int);
void error(const char *msg);
bool quit = false;

//Catchs exit signal to close connections before program closes 
void sig_handler(const int signum) { 
    puts("\nGot interrupt signal."); 
    if (signum == SIGINT) 
    { 
        //Closes all connections before exit 
        printf("SIGINT Received, Closing\n"); 
        quit = true; 
    } 
} 

void sha256(char *string, char outputBuffer[65])
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, string, strlen(string));
    SHA256_Final(hash, &sha256);
    int i = 0;
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++){
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = 0;
}

int main(int argc, char *argv[])
{
    using namespace std;
    int sockfd, newsockfd, portno, pid; // These two variables store the values returned by the socket system call and the accept system call.
    socklen_t clilen; // stores size of client address
    char buffer[256]; // buffer
    struct sockaddr_in serv_addr, cli_addr; // server addr and client addr
    int n; //
    
     //Sig Handler Error Check 
    if (signal(SIGINT, sig_handler) == SIG_ERR) 
    printf("Can't catch SIGINT\n"); 
    
    if (argc < 2) {// standard error check
    fprintf(stderr,"ERROR, no port provided\n");
    exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //establishes new socket
    if (sockfd < 0)
          error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr)); //clears serv_addr buffer to 0
    portno = atoi(argv[1]); 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno); //host byte order to network byte order
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd,5); //process to listen for socket for cons, 5 pending max in queue
    clilen = sizeof(cli_addr); //

    //Cache file manifest here
    FILE *fp, *fpFile, *test;
    fp = fopen("manifest.txt", "r+");
    
    if(fp == NULL){
        error("Cannot open manifest");
    }
    else{
        std::cout<<"Sucessfully opened manifest!" << std::endl;
    }
    
    //read each file in slot and give sha256 of file.
    int numfiles = 0;
    char myString[100];
    fgets(myString, 100, fp);
    
    cout << "Here1" << endl;
    std::cout << "Length" << strlen(myString)-1 << std::endl;
    for (size_t n=0 ; n < strlen(myString)-1; n++){ 
        if (!isdigit(myString[n])){
            std::cerr << "Manifest file is incorrect..." << std::endl;
            return -1;
        }
    }
    numfiles = std::atoi(myString);
    
    std::cout << "Numfiles: " << numfiles << std::endl;
    cout << "Here2" << endl;
    long filesize = 0;

    for(size_t m=0; m<numfiles; m++){
        fgets(myString,100,fp);
        myString[strlen(myString)-1] = 0;
        cout << "Trying to open:" << myString << endl;
        fpFile = fopen(myString, "r");
        if (fpFile == NULL){
            cout << "Error #:" << strerror(errno) << endl;
            cout << "Error Opening" << endl;
        }
        else {
            cout <<"Sucessfully Opened!" << endl;
        }
        fseek( fpFile, 0 , SEEK_END);
        cout << "Seeked!" << endl;
        filesize = ftell ( fpFile );
        cout << "FSized!" << endl;
        fclose (fpFile);
        fprintf(fp," %d",filesize);
        return 0;
    }

    return 0; // Debug end program here


    while(1){//main accept loop
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);//blocks until a client connects

        if (newsockfd < 0) 
            error("ERROR on accept");
        pid = fork();
        if (pid < 0)
            error("ERROR on fork");
        if (pid == 0){
            close(sockfd);
            dostuff(newsockfd);
            exit(0);
        }
        else
            close(newsockfd);
    }

    close(sockfd);
    return 0;
}

void error(const char *msg)
{
        perror(msg);
            exit(1);
}

void dostuff (int sock)
{
    int n;
    char buffer[256];
                
    bzero(buffer,256);
    n = read(sock,buffer,255);
    if (n < 0) error("ERROR reading from socket");
    printf("Here is the message: %s\n",buffer);
    n = write(sock,"I got your message",18);
    if (n < 0) error("ERROR writing to socket");
}
