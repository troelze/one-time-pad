/* OTP_ENC_D
* Programmed by: Tres Oelze
* Description: This program connects to otp_enc, and performs a one-time pad style encryption.
* Course Name: CS 372 - 400
* Last Modified: 3/17/2019
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <netdb.h> 


void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");

    int childExitMethod = -5;
    
    while(1)
    {
        listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

        // Accept a connection, blocking if one is not available until one connects
        sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
        establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
        if (establishedConnectionFD < 0) error("ERROR on accept");

        int childExitStatus = -5;
        //fork new process
        pid_t spawnpid = -5;
        
        
        spawnpid = fork();
        switch(spawnpid)
        {
            case -1:
                perror("Error creating fork!");
                exit(1);
                break;
            case 0:{ //child process reads data sent from otp_enc

                char buffer[255];
                bzero(buffer, 255);


                char verify[5];
                bzero(verify, 5);
                read(establishedConnectionFD, verify, 5);               //send socket verification code - if it doesn't match up with theirs then exit!
                if (strncmp(verify, "hello", 5) ==  0)
                {
                    write(establishedConnectionFD, "valid", 5);
                } else {
                    write(establishedConnectionFD, "notin", 5);
                    fprintf(stderr, "Attempted to encrypt, denied access.\n");
                    exit(1);
                }

                
                //attempt to open plain text file and report if this fails              
                int read_return;
                char file[] = "tempPT.txt";
                int tempPT;
                tempPT = open(file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                if (tempPT == -1)
                {
                    printf("open() failed on \"%s\"\n", file);
                    exit(1);
                }
                
                //read in from socket the plain text file and save data in temp file
                do {
                    read_return = read(establishedConnectionFD, buffer, 255);
                    if (read_return > 0)
                    {
                        write(tempPT, buffer, read_return);
                    }
                } while (read_return > 0);

                close(tempPT);

                //open temp file again
                FILE *fp;
                fpos_t position;
                char c;
                char c2;
                char c3;
                fp = fopen("tempPT.txt", "r");
                if(fp == NULL)
                {
                    fprintf(stderr, "error opening file: tempPT.txt");
                    exit(1);
                }
                //this takes the first have of the temp file and counts up the chars in the first half
                unsigned int count = 0;
                while((c = getc(fp)) != '&')
                {
                    
                   count++;
                }
                FILE *fp2;
                fpos_t position2;
                fp2 = fopen("tempKEY.txt", "w+");
                
                //write later half of temp file to new temp file (to store separate key file)
                while((c = getc(fp)) != EOF)
                {
                    
                    fputc(c, fp2);
                    
                }
                
                count = count-1;
                
                rewind(fp);
                rewind(fp2);

                FILE *fp3;
                fpos_t position3;
                fp3 = fopen("cyphered.txt", "w+");
                //write to third temp file the cyphered message using first two temp files
                int g;
                for(g = 0; g < count; g++)
                {
                    c = getc(fp2);
                
                    if (c == ' ')
                    {
                        c = 91;
                    }
                    c = c - 'A';

                    c2 = getc(fp);
                    if (c2 == ' ')
                    {
                        c2 = 91;
                    }
                    c2 = c2 - 'A';

                    c3 = (c2+c)%27;   //this is the cypher
                    if (c3 == 26)
                    {
                        c3 = -33;
                    }

                    c3 = c3+65;
                    fputc(c3, fp3);
                    

                }

                fputc('\n', fp3);


                fclose(fp3);
                fflush(fp3);
                fclose(fp2);
                fclose(fp);

                exit(0);
                break;
            }
            default:{ //back in parent, wait for child to finish to send cyphered message back
                pid_t result_pid;
                result_pid = waitpid(spawnpid, &childExitStatus, 0);
                
                //if wasn't able to connect, then skip to end of loop and go back to listening
                if (result_pid > 0)
                {
                    if(WIFEXITED(childExitStatus))
                    {
                        int exit_status = WEXITSTATUS(childExitStatus);
                        if (exit_status != 0)
                        {
                            break;
                        }
                    }
                    
                }
                //start up new socket to send cyphered data
                int socketFD;
                struct sockaddr_in server2Address;
                struct hostent* serverHostInfo2;
               
                memset((char*)&server2Address, '\0', sizeof(server2Address)); // Clear out the address struct
	            portNumber = 12005; // Get the port number, convert to an integer from a string
	            server2Address.sin_family = AF_INET; // Create a network-capable socket
	            server2Address.sin_port = htons(portNumber); // Store the port number
	            serverHostInfo2 = gethostbyname("localhost"); // Convert the machine name into a special form of address
	            if (serverHostInfo2 == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	            memcpy((char*)&server2Address.sin_addr.s_addr, (char*)serverHostInfo2->h_addr, serverHostInfo2->h_length); // Copy in the address

	            // Set up the socket
                socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
                if (socketFD < 0) error("CLIENT: ERROR opening socket");
                
                // Connect to server
                if (connect(socketFD, (struct sockaddr*)&server2Address, sizeof(server2Address)) < 0) // Connect socket to address
                    error("CLIENT: ERROR connecting");
                
                char read_buff[255];
                bzero(read_buff, 255);

                int cypheredFD;
                cypheredFD = open("cyphered.txt", O_RDONLY);
                if (cypheredFD == -1)
                {
                    printf("open() failed on transfer");
                    exit(1);
                }
            
                while (1) {
                    // Read data into buffer. 
                    // store how many bytes were actually read in bytes_read.
                    int bytes_read = read(cypheredFD, read_buff, sizeof(read_buff));
                    if (bytes_read == 0) // We're done reading from the file
                        break;

                    if (bytes_read < 0) {
                        // handle errors
                    }

                    //  write will return how many bytes were written. p keeps
                    // track of where in the buffer we are, while we decrement bytes_read
                    // to keep track of how many bytes are left to write.
                    void *p = read_buff;
                    while (bytes_read > 0) {
                        int bytes_written = write(socketFD, p, bytes_read);
                        fflush(stdout);
                        if (bytes_written <= 0) {
                            // handle errors
                        }
                        bytes_read -= bytes_written;
                        p += bytes_written;
                    }
                }
                close(cypheredFD);

                close(socketFD);
                remove("tempPT.txt");
                remove("tempKEY.txt");
                remove("cyphered.txt");

                break;
            }
        }  
    }

    close(listenSocketFD); // Close the listening socket
	return 0; 
}
