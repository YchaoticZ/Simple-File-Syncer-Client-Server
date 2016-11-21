#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <openssl/sha.h>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

void sha256(char *string, int length, unsigned char *outputBuffer);
void error(const char *msg);

struct fileInfo* checkDir();

enum message_type{
	CLIENT_MESSAGE = 65,
	CLIENT_FILE_TRANSFER = 66,
	CLIENT_FILE_REQ = 80,
	CLIENT_FILE_RECV = 81
};

void printHash(unsigned char *hash);


struct fileInfo{
    char fileName[256];
    uint32_t size;
    unsigned char hash[32];
};

struct __attribute__ ((__packed__)) message_header{
	char type; //Msg Type
    char fileName[256]; 
    unsigned char hash[32];
};

struct __attribute__ ((__packed__)) file_req{
	char type; //Msg Type
    char fileName[256];
};

struct __attribute__ ((__packed__)) file_recv{
	char type; //Msg Type
    char fileName[256]; 
	uint32_t fileSize;
};


int numManFiles = 0;

/*

6:04 PM - honest to god brain genius: static library build
6:05 PM - honest to god brain genius: output as .o file
6:05 PM - honest to god brain genius: you dont use a main
6:05 PM - honest to god brain genius: for your libraries
6:05 PM - honest to god brain genius: because the entry point is in another program
6:05 PM - honest to god brain genius: basically instead of -o client
6:05 PM - honest to god brain genius: youd do whatever
6:05 PM - honest to god brain genius: the convention is .o
6:05 PM - honest to god brain genius: then you use ar to make it into a static lib
6:06 PM - @nevinyrral #colors: AR??????????
6:06 PM - honest to god brain genius: ar rcs libxyz.a (all the objects)
6:06 PM - honest to god brain genius: and thats it
6:06 PM - honest to god brain genius: know how you use -lssl

*/

int main (int argc, char *argv[]){ // format is ./client website port

    if (argc != 3){
		fprintf(stderr,"ERROR, Syntax is ./client website port\n");
		exit(1);
    }
    int sockfd, portno; //inits
    char buffer[16384]; //buffer for data
	char msgBuffer[1028];
    struct sockaddr_in serv_addr;//, cli_addr; //struct for server and client addr
    struct hostent *server;
    bool restart = true;
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
	
	//
	//prototype for the thing
    DIR *d;
    struct dirent *dir;
    char search[5] = ".bin";
    d = opendir("/home/danny/fileproj/files");

    //Cache file manifest here
    FILE *fp;
	int fileSize = 0;
    char fBuffer[10000], fileName[100],readBuffer[10000];
	unsigned char hash[32];
	bzero(fBuffer,10000);
	bzero(readBuffer,10000);
	bzero(fileName,100);
    struct fileInfo *filePointer2;
	size_t bRead = -1;

    if (d)
    {   
        while ((dir = readdir(d)) != NULL){ // find how many .txt files in dir
            if(strstr(dir->d_name, search) != NULL){
                numManFiles++;
            }
        }

		rewinddir(d); //reset dir to first index
        printf("Number of files in directory: %i\n",numManFiles);
        /// this falls out of scope at the end of this function or if block
        /// need to malloc instead. you also need to use malloc because this could stack overflow on long lists
		filePointer2 = (struct fileInfo*)malloc(sizeof(struct fileInfo) * numManFiles);
    
        int dirIndex = 0;
        while ((dir = readdir(d)) != NULL)
        {   
            if(strstr(dir->d_name, search) != NULL){
                snprintf(fileName, sizeof(fileName), "files/%s",dir->d_name);
                //printf("Reading file: %s\n", fileName);
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
                //strncpy(filePointer2[dirIndex].hash,hash,(char *)sizeof(filePointer2[dirIndex].hash));
				memcpy(filePointer2[dirIndex].hash,hash,sizeof(filePointer2[dirIndex].hash));
                dirIndex++;
				bzero(fBuffer,10000);
            }
			/*TODO
			n = read(sockfd,readBuffer,sizeof(readBuffer));
			if (n < 0) error("ERROR reading from socket");
			else printf("Num of bytes read: %i\n",n);
			if (strcmp(readBuffer,"DNE") == 0){
				//copy whole file
				printf("A\n");
			}
			else if (strcmp(readBuffer,indexPointer->hash) == 0){
				n = write(sockfd,"File OK",7);
				printf("B\n");
			}
			else{
				printf("C\n");
				//initiate block check here
			}
			*/
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
	
	
	printf("Exiting Test Block\n");

	//

	bzero(msgBuffer,1028);
	while (restart == true){
		struct fileInfo *dirPtr;
		printf("File transfer starting...\n");

		dirPtr = checkDir();
		if (dirPtr == NULL){
			printf("Directory Opening Error\n");
			exit(1);
		}

		sleep(2);
		printf("Done sleeping\n");

		while(1){
			int len = recv(sockfd, msgBuffer, 16384, MSG_DONTWAIT);
			uint32_t remaining_len;
			if (len < 0){
				remaining_len = 0;
			}
			else{
				remaining_len = len;
			}
			char *bufIndex;
			struct message_header msg;
			bool found;
			while(remaining_len > 0 ){
				printf("There is a message remaining_len: %u > 0\n",remaining_len);
				printf("Whole Message:\n");
				for(uint32_t i = 0; i < remaining_len;i++){
					printf("%c",msgBuffer[i]);
				}
				printf("\n");
				bufIndex = msgBuffer;
				printf("Full msg received!\n");
				printf("RemainingLenStart: %u\n",remaining_len);
				printf("Header value:%i\n",msgBuffer[0]);
				if(msgBuffer[0] == CLIENT_MESSAGE){
					printf("Entering CLIENT MSG, Header Type Should be Chopped off\n");
					if (remaining_len >= 288){
						bufIndex = &msgBuffer[1];
						memmove(msgBuffer,bufIndex,remaining_len);
						bufIndex = msgBuffer;
						remaining_len--;
						printf("Whole Message(NoHeader):\n");
						for(int i = 0; i < len;i++){
							printf("%c",msgBuffer[i]);
						}
						printf("\n");

						memcpy(msg.fileName,bufIndex,256);
						bufIndex+=256;
						remaining_len-=256;
						
						memcpy(msg.hash,bufIndex,32);
						bufIndex+=32;
						remaining_len-=32;
						printf("Recv FName:%s\nHash:",msg.fileName);
						printHash(msg.hash);
						memmove(msgBuffer,bufIndex,remaining_len);
						found = false;
						for (int j = 0; j < numManFiles; j++){
							if (found == true){
								break;
							}
							printf("Checking filename...1:%s vs 2:%s\n",dirPtr[j].fileName,msg.fileName);
							
							if (strncmp(dirPtr[j].fileName,msg.fileName,256) == 0){
								printf("File found! Matches #%i: %s\n",j,dirPtr[j].fileName);
								found = true;
								printf("ClientHash:");
								printHash(dirPtr[j].hash);
								printf("ServerHash:");
								printHash(msg.hash);
								if (memcmp(dirPtr[j].hash,msg.hash,32) == 0){
									printf("Hash Matches Okay!\n");
								}
								else {
									printf("Hash Doesnt Match, Send File!\n");
									struct file_req req;
									int er;
									req.type = CLIENT_FILE_REQ;
									strncpy(req.fileName,msg.fileName,256);
									er = write(sockfd,&req,sizeof(req));
									if (er < 0) {
										error("ERROR writing to socket");
									}
									else{
										printf("Requested file %s be sent over!\n",msg.fileName);
									}
								}
							}
						}
						if (found == false){
							printf("File not found on client\n");
							struct file_req req;
							int er;
							req.type = CLIENT_FILE_REQ;
							strncpy(req.fileName,msg.fileName,256);
							er = write(sockfd,&req,sizeof(req));
							if (er < 0) {
								error("ERROR writing to socket");
							}
							else{
								printf("Wrote %i Bytes...Requested file %s be sent over!\n",er,msg.fileName);
							}
						}

					}
					else{ // not long enough
						printf("Shouldnt Enter Here Yet\n");
					}

					printf("One Loop Done. Remaining Length:%u\n\n",remaining_len);
					
				}
				else if (msgBuffer[0] == CLIENT_FILE_RECV){
					printf("Entered MSG Recv Block\n");
					bufIndex = &msgBuffer[1];
					memmove(msgBuffer,bufIndex,remaining_len);
					bufIndex = msgBuffer;
					remaining_len--;
					struct file_recv recv;
					if (remaining_len > (sizeof(file_recv) - 1)){
						memcpy(recv.fileName,bufIndex,sizeof(recv.fileName));
						bufIndex+=sizeof(recv.fileName);
						memcpy(&recv.fileSize,bufIndex,sizeof(recv.fileSize));
						bufIndex+=sizeof(recv.fileSize);
						memmove(msgBuffer,bufIndex,remaining_len);
						bufIndex = msgBuffer;
						remaining_len-=(sizeof(recv.fileName)+sizeof(recv.fileSize));
						printf("Remaining Len:%u\n",remaining_len);
						if (remaining_len >= recv.fileSize){
							printf("Reads the File Contents here after handling the name and size\n");
							uint32_t writeCheck;
							char *fileContents = (char *)malloc(recv.fileSize);
							memcpy(fileContents,bufIndex,recv.fileSize);
							bufIndex+=recv.fileSize;
							memmove(msgBuffer,bufIndex,remaining_len);
							bufIndex = msgBuffer;
							char *prependFileName = (char *)malloc(sizeof(recv.fileName)+28);
							snprintf(prependFileName,sizeof(recv.fileName)+28,"/home/danny/fileproj/files/%s",recv.fileName);
							FILE* writeFD = fopen(prependFileName,"wb");
							printf("Attempting to open:%s\n",prependFileName);
							if (writeFD != NULL){
								printf("Sucessfully opened %s!\n",prependFileName);
								writeCheck = fwrite(fileContents,recv.fileSize,1,writeFD);
								if (writeCheck != 1){
									printf("Error unable to write the %u bytes\n",recv.fileSize);
								}
								fclose(writeFD);
							}
							else{
								error("Error opening file");
							}
							free(fileContents);
							free(prependFileName);
							bufIndex+=sizeof(recv.fileSize);
							memmove(msgBuffer,bufIndex,remaining_len);
							bufIndex = msgBuffer;
							remaining_len-=recv.fileSize;
						}
					}
					else{
						printf("shouldnt happen\n");
					}
					printf("RemainingLen:%u\n",remaining_len);
					printf("Exiting MSG Recv Block\n\n");
				}
				else{
					printf("Not a proper header type, move to next char\n");
					bufIndex++;
					memmove(msgBuffer,bufIndex,remaining_len);
					bufIndex = msgBuffer;
					remaining_len--;
					//should never happen;
				}
				sleep(1);
			}
			printf("No more to read\n");
			sleep(2);
		}

		free(dirPtr);
		printf("Done Updating File Directory!\n");

		fgets(buffer,255,stdin);
		if (strncmp(buffer,"restart",7) == 0){
			restart = true;
		}
		else{
			restart = false;
		}

	}

	close(sockfd);
    return 0;
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

void error(const char *msg)
{
    perror(msg);
    exit(1);
}


struct fileInfo* checkDir(){
	DIR *d;
	struct dirent *dir;
	struct fileInfo *filePointer;
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
		filePointer = (struct fileInfo*)malloc(sizeof(struct fileInfo) * numManFiles);
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
                strncpy(filePointer[dirIndex].fileName,dir->d_name,sizeof(filePointer[dirIndex].fileName));
                filePointer[dirIndex].size = fileSize;
                //strncpy(filePointer[dirIndex].hash,hash,(char *)sizeof(filePointer[dirIndex].hash));
				memcpy(filePointer[dirIndex].hash,hash,sizeof(filePointer[dirIndex].hash));
				dirIndex++;
				bzero(fBuffer,10000);
			}
		}
	
		printf("Client Files:\n");
		for (int i = 0; i < dirIndex; i++){
			printf("1: %s ,2: %i ,3:",filePointer[i].fileName,filePointer[i].size);
			printHash(filePointer[i].hash);
		}
		printf("\n");
	}
	else if (d == NULL){
		printf("Error opening directory\n");
		return 0;
	}

	closedir(d); 
	return filePointer;
}