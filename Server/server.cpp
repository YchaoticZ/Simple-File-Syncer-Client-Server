#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <openssl/sha.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <errno.h>
#include <dirent.h>
#include <inttypes.h>
#include <time.h>
#include <sys/stat.h>

enum message_type{
	SERVER_MESSAGE = 65,
	SERVER_FILE_TRANSFER = 66,
	SERVER_FILE_REQ = 80,
	SERVER_FILE_SEND = 81
};

void handleRequest(int sock);
void error(const char *msg);
bool quit = false;


void sig_handler(const int signum);
struct fileInfo* checkDir();
//void checkDir();
void sha256(char *string, int length, unsigned char *outputBuffer);
void printHash(unsigned char *hash);

void print_time();
void transfer(const int sockfd, char readBuffer[], struct fileInfo *dirFP);
struct fileInfo* checkDir();

struct fileInfo{
    char fileName[256];
	uint32_t size;
    unsigned char hash[32];
};

//struct fileInfo *dirPtr;
int numManFiles = 0;
int dirs = 1;
extern int errno;

//Temp here move to session.hpp later
class Session{
private:
	int sockfd;
	uint32_t user_id;
	
public:
	Session(const int newsock_fd);
	
	inline uint32_t get_user_id();
	inline int get_sockfd();
};

struct __attribute__ ((__packed__)) message_header{
	char type; //65
    char fileName[256];
    uint8_t hash[32];
};

struct __attribute__ ((__packed__)) file_req{
	char type; //Msg Type
    char fileName[256];
};

struct __attribute__ ((__packed__)) file_send{
	char type; //Msg Type
    char fileName[256]; 
	uint32_t fileSize;
};

int connect();

struct fileInfo* list_dir (const char *dir_name);

int main(int argc, char *argv[]) //format is ./server port
{
    using namespace std;
    int sockfd, portno; // These two variables store the values returned by the socket system call and the accept system call.
    //socklen_t clilen; // stores size of client address
    struct sockaddr_in serv_addr;// ,cli_addr; // server addr and client addr

    //Sig Handler Error Check 
    if (signal(SIGINT, sig_handler) == SIG_ERR) 
		printf("Can't catch SIGINT\n"); 
    
    if (argc < 2) {// standard error check
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
    }
	//Valid Ports are 0 to 65535.
	if (atoi(argv[1]) < 0 || atoi(argv[1]) > 65535){
		printf("Invalid Port. Use 0-65535\n");
		exit(1);
	}
	
    /// you want to set your socket as nonblocking here
    sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0); //establishes new socket
    if (sockfd < 0){
		error("ERROR opening socket");
	}
	
	bzero((char *) &serv_addr, sizeof(serv_addr)); //clears serv_addr buffer to 0
	portno = atoi(argv[1]); 
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno); //host byte order to network byte order
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
        error("ERROR on binding");
	}

    listen(sockfd,10); //process to listen for socket for cons, 5 pending max in queue
    //clilen = sizeof(cli_addr); //
	
	//Start prototype for the thing
    // DIR *d;
    // struct dirent *dir;

	const char *dir1 = "/home/danny/fileproj/files";

    // Cache file manifest here
    // FILE *fp;
	// int fileSize = 0;
    // char fBuffer[10000], fileName[256],readBuffer[10000];
	// unsigned char hash[32];
	// bzero(fBuffer,10000);
	// bzero(readBuffer,10000);
	// bzero(fileName,100);
    struct fileInfo *filePointer;
	// size_t bRead = -1;
	// struct stat sb;
	// char *filePath;
	// char search[5] = ".bin";

	filePointer = list_dir(dir1);

	printf("Done2\n");
	/*
	for (int i =0; i<numManFiles;i++){
		printf("Contents---Name:%s ---FileSize:%i ---Hash:",filePointer2[i].fileName,filePointer2[i].size);
		printHash(filePointer2[i].hash);
	}
	
	return 0;
	
    if (d)
    {   
        while ((dir = readdir(d)) != NULL){ // find how many .txt files in dir
			filePath = (char *)malloc(sizeof(dir->d_name)+27);
			snprintf(filePath,sizeof(dir->d_name)+27,"/home/danny/fileproj/files/%s",dir->d_name);
			printf("Checking:%s\n",filePath);
			if (stat(filePath, &sb) == 0 && S_ISDIR(sb.st_mode)){
				// is a Directory
				if (strncmp(dir->d_name,"..",2) == 0 || strncmp(dir->d_name,".",1) == 0){
					printf("Ignoring %s\n",filePath);
				}
				else{
					printf("%s is a directory \n",filePath);
					dirs++;
				}
			}
			else if (stat(filePath, &sb) == 0 && S_ISREG(sb.st_mode)){
				// is a file
				numManFiles++;
			}
			free(filePath);
        }

		rewinddir(d); //reset dir to first index
        printf("Number of files in directory: %i\n",numManFiles);
		filePointer2 = (struct fileInfo*)malloc(sizeof(struct fileInfo) * numManFiles);
    
        int dirIndex = 0;
        while ((dir = readdir(d)) != NULL)
        {   
            if(strstr(dir->d_name, search) != NULL){
                snprintf(fileName, sizeof(fileName), "files/%s",dir->d_name);
                printf("Reading file: %s\n", fileName);
                fp = fopen(fileName, "rb");
                if(fp == NULL){
                    printf("Error opening: %s\n",dir->d_name);
                    return 0;
                }
                fseek(fp, 0, SEEK_END);
                fileSize = ftell(fp);
				printf("Size of binary file:%i\n",fileSize);
                rewind(fp);
                bRead = fread(fBuffer, sizeof(fBuffer), 1, fp);
				printf("Bytes Read:%zu Buffer Contents:%s\n",bRead,fBuffer);
                sha256(fBuffer,fileSize,hash);
                printf("Hash of file: %s is:",dir->d_name);
				printHash(hash);
				printf("\n");
                fclose(fp);
                strncpy(filePointer2[dirIndex].fileName,dir->d_name,sizeof(filePointer2[dirIndex].fileName));
                filePointer2[dirIndex].size = fileSize;
                // strncpy(filePointer2[dirIndex].hash,hash,(char *)sizeof(filePointer2[dirIndex].hash));
				memcpy(filePointer2[dirIndex].hash,hash,sizeof(filePointer2[dirIndex].hash));
                dirIndex++;
				bzero(fBuffer,10000);
            }
        }
		dirIndex = 0;
        closedir(d);
        printf("Error checking structs:\n");
        for (int i = 0; i < numManFiles; i++){
            printf("1: %s ,2: %i ,3:",filePointer2[i].fileName,filePointer2[i].size);
			printHash(filePointer2[i].hash);
        }
        printf("\n\n"); //formatting
    }
	free((char *)dir1);
	*/ //Replace this
	printf("Exiting Test Block\n");
	
	
	int numConnections = 0, connectionFD = -1;
	
	Session **session;

    while(1){//main accept loop

		connectionFD = accept(sockfd, (struct sockaddr*) NULL, NULL);//blocks until a client connects
		if (connectionFD >= 0){ // No error codes{
			Session **ptr = (Session**)malloc(sizeof(Session*)*(numConnections+1));
			Session *newSession = new Session(connectionFD);
			if (!ptr || !newSession){
				puts("Can't accept new session");
				free(ptr);
				free(newSession);
				close(connectionFD);
			}
			else{
				memcpy(ptr, session, sizeof(Session*)*numConnections);
				if (session != NULL){
					free(session);
				}
				session = ptr;
				session[numConnections] = newSession;
				numConnections++;
				printf("Accepted new connection #%i\n",numConnections);
				printf("%i Clients Currently Connected\n",numConnections);
				print_time();

                filePointer = checkDir();
                     
                int n;
				printf("NumManFiles: %i\n",numManFiles);
                for (int i = 0; i < numManFiles; i++){
                    struct message_header msg;
					msg.type = SERVER_MESSAGE;
                    //len = snprintf(writeBuffer,1024,"%s %i %s ",filePointer[i].fileName,filePointer[i].size,filePointer[i].hash);
					strncpy(msg.fileName,filePointer[i].fileName,sizeof(filePointer[i].fileName));
					memcpy(msg.hash,filePointer[i].hash,32);
                    n = write(connectionFD,&msg,sizeof(msg));
                    if (n < 0){
                        error("Error writing to socket\n");
                    }
                    else{
                        printf("Wrote:%i bytes\n", n);
						printf("Contents MSG.TYPE:%c\n",msg.type);
						printf("Contents MSG.FILENAME:%s\n",msg.fileName);
						printf("Contents MSG.HASH:");
						printHash(msg.hash);
                    }
                }
                printf("Done\n\n");
				fflush(stdout);
			}
		}
		else {
			char buffer[10000];
			for (int x = 0; x < numConnections; ++x) {
				const int len = recv(session[x]->get_sockfd(), (void*)buffer, sizeof(buffer), MSG_DONTWAIT);
				//int sessNum = x;
				if (len > 0) {
					int remaining_len = len;
					printf("Message of size %i Received: %s\n",len,buffer); 
					while(remaining_len > 0){
						if (buffer[0] == SERVER_FILE_REQ){
							struct file_req req;
							if (remaining_len >= 257){
								memmove(buffer,&buffer[1],remaining_len);
								memcpy(req.fileName,buffer,256);
								memmove(buffer,&buffer[256],256);
								remaining_len-=256;
								//find file name
								bool found = false;
								for (int i = 0; i < numManFiles;i++){
									if (strncmp(filePointer[i].fileName,req.fileName,256) == 0){
										printf("File found! Matches #%i: %s\n",i,filePointer[i].fileName);
										found = true;
										FILE *readFP;
										struct file_send send;
										uint32_t bWritten,bRead;
										char prepend[28] = "/home/danny/fileproj/files/";
										char formattedName[284];
										snprintf(formattedName,284,"%s%s",prepend,filePointer[i].fileName);
										printf("Opening file:%s\n",formattedName);
										readFP = fopen(formattedName,"rb");
										if (readFP == NULL){
											error("Error opening file\n");
										}
										else{
											printf("Opened %s properly!\n",formattedName);
										}
										//Gets to here okay
										char *fileContents = (char *)malloc(filePointer[i].size);
										printf("Size of fileContents %lu, fileSize %u\n",sizeof(fileContents),filePointer[i].size);
										bRead = fread(fileContents,filePointer[i].size,1,readFP);
										if (bRead == 0){
											 printf("Oh dear, something went wrong with read()! %s\n", strerror(errno));
										}
										fclose(readFP);		
										printf("Contents of fileContents:%s\n",fileContents);

										send.type = SERVER_FILE_SEND;
										memcpy(send.fileName,filePointer[i].fileName,sizeof(filePointer[i].fileName));
										send.fileSize = filePointer[i].size;

										printf("Doing Malloc for MSG Here. FileSize:%u\n",filePointer[i].size);
										char *newMsg = (char *)malloc(sizeof(send)+filePointer[i].size);
										memcpy(newMsg,&send,sizeof(send));
										
										printf("Struct Contents:");
										for (uint32_t k = 0; k< sizeof(file_send); k++){
											printf("%c",newMsg[k]);
										}
										
										printf("\n");
										memcpy(&newMsg[sizeof(send)],fileContents,filePointer[i].size);
										bWritten = write(session[x]->get_sockfd(),newMsg,sizeof(send)+filePointer[i].size);
										//sizeofsend is 261, filesize is 53, should send 314 total
										printf("Wrote #bytes to socket:%u, %lu\n",bWritten,sizeof(send));
										printf("Message written to socket:");
										for (uint32_t j = 0; j< bWritten; j++){
											printf("%c",newMsg[j]);
										}
										printf("\n");
										if (bWritten == (sizeof(send)+filePointer[i].size)){ //good
											printf("Wrote %i bytes to the socket as header!\n",bWritten);
										}
										else {//socket was full or error?
											printf("Error\n");
										}
										free(fileContents);
										free(newMsg);
									}
								}
								if (found == false){
									error("Wrong filename sent\n");
								}
								else{
									printf("Completed File Transfer\n");
								}
							}
						}
					}
				} 
				else if (len == 0) {
					// socket disconnected properly	
					delete session[x];
					numConnections--;
					session[x] = session[numConnections]; // last connection replaces deleted connection
					printf("Connection #%i disconnected\n",x+1);
					printf("%i Clients Currently Connected\n",numConnections);
				}
			}
		}
		printf("Waiting\n");
		sleep(1);
		sched_yield();
    }

    close(sockfd);
    return 0;
}

void error(const char *msg)
{
        perror(msg);
            exit(1);
}

//Catchs exit signal to close connections before program closes 
void sig_handler(const int signum) { 
    puts("\nGot interrupt signal."); 
    if (signum == SIGINT) 
    { 
        //Closes all connections before exit 
        printf("SIGINT Received, Closing\n"); 
        exit(1);
    } 
} 

void sha256(char *string, int length, unsigned char *outputBuffer)
{
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, string, length);
    SHA256_Final((unsigned char*)outputBuffer, &sha256);
	printHash(outputBuffer);
	printf("\n");
}

void printHash(unsigned char *hash){
    int i = 0;
    for(i = 0; i < 32; i++){;
        unsigned int a = hash[i];
        printf("%02x", a);
    }
    printf("\n");
}


//Time Function
void print_time (){
    struct timeval tv;
    struct tm* ptm;
    char time_string[40];
    long milliseconds;

    /* Obtain the time of day, and convert it to a tm struct. */
    gettimeofday (&tv, NULL);
    ptm = localtime (&tv.tv_sec); //gets local time
    ptm->tm_hour -= 9; //Offset European time to Pacific
    /* Format the date and time, down to a single second. */
    strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", ptm);
    /* Compute milliseconds from microseconds. */
    milliseconds = tv.tv_usec / 1000;
    /* Print the formatted time, in seconds, followed by a decimal point
    and the milliseconds. */
	printf ("%s.%03ld\n", time_string, milliseconds);
}

//temp here move to session.cpp later
Session::Session(int new_sockfd){
	user_id = 0;
	sockfd = new_sockfd;
}

inline int Session::get_sockfd(){
	return sockfd;
}

uint32_t Session::get_user_id(){
	return user_id;
}

struct fileInfo* checkDir(){
    DIR *d;
    struct dirent *dir;
    struct fileInfo *filePointer2;
    char search[5] = ".bin";
    d = opendir("/home/danny/fileproj/files");
    
    if (d == NULL){
        printf("Error opening directory!\n");
    }
     
    //Cache file manifest here
    FILE *fp;
    int fileSize = 0;
    char fBuffer[10000], fileName[100];
	unsigned char hash[32];
    numManFiles = 0;
	bzero(fBuffer,10000);
	bzero(fileName,100);
	
    if (d)
    {   
        while ((dir = readdir(d)) != NULL){
            if(strstr(dir->d_name, search) != NULL){
                numManFiles++;
            }
        }

        printf("Number of files in directory: %i\n",numManFiles);
        filePointer2 = (struct fileInfo*)malloc(sizeof(struct fileInfo) * numManFiles);
        rewinddir(d);
        int dirIndex = 0;
        
        while ((dir = readdir(d)) != NULL){   
            if(strstr(dir->d_name, search) != NULL){
                snprintf(fileName, sizeof(fileName), "files/%s",dir->d_name);
                printf("Reading file: %s\n", fileName);
                fp = fopen(fileName, "rb");
                if(fp == NULL){
                    printf("Error opening: %s\n",dir->d_name);
                    return 0;
                }
                fseek(fp, 0, SEEK_END);
                fileSize = ftell(fp);
                fseek(fp, 0, SEEK_SET);
                fread(fBuffer, sizeof(fBuffer), 1, fp);
                sha256(fBuffer,fileSize,hash);
                printf("Hash of file: %s is:",dir->d_name);
				printHash(hash);
                fclose(fp);
                strncpy(filePointer2[dirIndex].fileName,dir->d_name,sizeof(filePointer2[dirIndex].fileName));
                filePointer2[dirIndex].size = fileSize;
                //strncpy(filePointer2[dirIndex].hash,hash,(char *)sizeof(filePointer2[dirIndex].hash));
				memcpy(filePointer2[dirIndex].hash,hash,sizeof(filePointer2[dirIndex].hash));
                dirIndex++;
				bzero(fBuffer,10000);
            }
        }
    
        printf("Error Checking Files:\n");
        for (int i = 0; i < dirIndex; i++){
            printf("1: %s ,2: %i ,3:",filePointer2[i].fileName,filePointer2[i].size);
			printHash(filePointer2[i].hash);
        }
    }
    else if (d == NULL){
        printf("Error opening directory\n");
        return 0;
    }

    closedir(d); 
    return filePointer2;
}

struct fileInfo* list_dir (const char *dir_name){
    DIR *d;
	FILE *fp;
	static fileInfo *filePointer;
	unsigned char hash[32];
	uint32_t fileSize;
	char fileName[350];
	struct stat sb;
	struct fileInfo *temp;
	char fBuffer[10000];
	size_t bRead;
	
    /* Open the directory specified by "dir_name". */
    d = opendir (dir_name);

    /* Check it was opened. */
    if (! d) {
        fprintf (stderr, "Cannot open directory '%s': %s\n",
                 dir_name, strerror (errno));
        exit (EXIT_FAILURE);
    }
    while (1) {
        struct dirent *dir;
        char *d_name;

        /* "Readdir" gets subsequent entries from "d". */
        dir = readdir (d);
        if (! dir) {
            /* There are no more entries in this directory, so break
               out of the while loop. */
            break;
        }
        d_name = dir->d_name;
        /* Print the name of the file and directory. */
		if (strncmp(d_name,".",1) != 0 && strncmp(d_name,"..",2) != 0){
			printf ("1:%s/%s\n", dir_name, d_name);
			snprintf(fileName,350,"%s/%s",dir_name,d_name);
			memmove(fileName,&fileName[21],329);
			printf("2:Path:%s --> ",fileName);
			
			if (stat(fileName, &sb) == 0 && S_ISDIR(sb.st_mode))
			{
				printf("3:Its a directory!\n");
				
			}
			if (stat(fileName, &sb) == 0 && S_ISREG(sb.st_mode))
			{
				printf("3:Its a regular file!\n");
				if (numManFiles == 0){
					filePointer = (fileInfo *)malloc(sizeof(fileInfo));
					memcpy(filePointer[numManFiles].fileName,fileName,strnlen(fileName,sizeof(fileName)));
					printf("File %i Name:%s\n",numManFiles,filePointer[numManFiles].fileName);
					for (int i = 0; i<numManFiles; i++){
						printf("Opening:%s\n",filePointer[i].fileName);
					}
				}
				else{
					temp = (fileInfo *)malloc(sizeof(fileInfo)*(numManFiles+1));
					memcpy(temp,filePointer,sizeof(fileInfo)*numManFiles);
					memcpy(temp[numManFiles].fileName,fileName,strnlen(fileName,sizeof(fileName)));
					filePointer = temp;
					printf("File %i Name:%s\n",numManFiles,filePointer[numManFiles].fileName);
					for (int i = 0; i<numManFiles; i++){
						printf("Opening#%i:%s\n",i,filePointer[i].fileName);
					}
				}
				
				fp = fopen(fileName, "rb");
				if(fp == NULL){
					printf("Error opening: %s\n",fileName);			
				}
				
				printf("Opening:%s\n",filePointer[numManFiles].fileName);
				fp = fopen(filePointer[numManFiles].fileName,"rb");
				fseek(fp, 0, SEEK_END);
				fileSize = ftell(fp);
				printf("Size of binary file:%i\n",fileSize);
				rewind(fp);
				filePointer[numManFiles].size = fileSize;
				bRead = fread(fBuffer, sizeof(fBuffer), 1, fp);
				printf("Bytes Read:%zu Buffer Contents:%s\n\n",bRead,fBuffer);
				fclose(fp);
				sha256(fBuffer,fileSize,hash);
                printf("Hash of file: %s is:",dir->d_name);
				printHash(hash);
				memcpy(filePointer[numManFiles].hash,hash,sizeof(filePointer[numManFiles].hash));
				printf("\n");
				bzero(fBuffer,10000);
				numManFiles++;
			}
			printf("Loop Complete\n\n");
		}
		
		if (dir->d_type & DT_DIR) {

			/* Check that the directory is not "d" or d's parent. */
			if (strcmp (d_name, "..") != 0 &&
				strcmp (d_name, ".") != 0) {
				int path_length;
				char path[PATH_MAX];

				path_length = snprintf (path, PATH_MAX,
										"%s/%s", dir_name, d_name);
				printf ("Checking Inside:%s\n", path);
				if (path_length >= PATH_MAX) {
					fprintf (stderr, "Path length has got too long.\n");
					exit (EXIT_FAILURE);
				}
				/* Recursively call "list_dir" with the new path. */
				list_dir (path);
			}
		}
    }
    /* After going through all the entries, close the directory. */
    if (closedir (d)) {
        fprintf (stderr, "Could not close '%s': %s\n",
                 dir_name, strerror (errno));
        exit (EXIT_FAILURE);
    }
	
	return filePointer;
}
