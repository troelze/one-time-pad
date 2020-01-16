/* OTP_DEC
* Programmed by: Tres Oelze
* Description: This program connects to otp_dec_d, and asks it to perform a one-time pad style decryption.
* Course Name: CS 372 - 400
* Last Modified: 3/17/2019
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <fcntl.h>
#include <ctype.h>

void error(const char *msg) { perror(msg); exit(2); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[255];
    bzero(buffer, 255);
   
    
	if (argc < 4) { fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); exit(0); } // Check usage & args

    //check if bad characters in KEY, count number of chars, and if file valid!
    FILE *KEY_verify;
    char c1;
    KEY_verify = fopen(argv[2], "r");
    if(KEY_verify == NULL)
    {
        fprintf(stderr, "error opening key file: %s\n", argv[2]);
        exit(1);
    }
    unsigned int KEY_count = 0;
    while((c1 = getc(KEY_verify)) != EOF)
    {
       if (c1 >= 65 && c1 <= 90) {
        
            KEY_count++;
            continue;
        } else if (c1 == 32 || c1 == '\n') {
            KEY_count++;
            continue;
        } else {
            fprintf(stderr, "bad character (%c) in key file: %s\n", c1, argv[2]);
            exit(1);
        }
    }
    fclose(KEY_verify);
    //check if bad characters in PT, count number of chars and if file valid!
    FILE *PT_verify;
    char c;
    PT_verify = fopen(argv[1], "r");
    if(PT_verify == NULL)
    {
        fprintf(stderr, "error opening file: %s\n", argv[1]);
        exit(1);
    }
    unsigned int PT_count = 0;
    while((c = getc(PT_verify)) != EOF)
    {
        if (c >= 65 && c <= 90) {
        
            PT_count++;
            continue;
        } else if (c == 32 || c == '\n') {
            PT_count++;
            continue;
        } else {
            fprintf(stderr, "bad character (%c) in file: %s\n", c, argv[1]);
            exit(1);
        }

    }
    fclose(PT_verify);
    //compare PT char count to KEY char count
    if (PT_count > KEY_count)
    {
        fprintf(stderr, "key %s must be at least as long as plain text %s\n", argv[2], argv[1]);
        exit(1);
    }

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

    //send verification code
    char verify[5];
    bzero(verify, 5);
    write(socketFD, "aloha", 5);
    read(socketFD, verify, 5);
    if (strncmp(verify, "valid", 5) != 0) //exit if vrerification code was invalid
    {
        fprintf(stderr,"\n");
        exit(2); // Check 
    }
    

	//Read from plaintext file and send contents to server
    int plaintextFD;
    plaintextFD = open(argv[1], O_RDONLY);
     if (plaintextFD == -1)
    {
        printf("open() failed on \"%s\"\n", argv[1]);
        exit(1);
    }

    while (1) {
        // Read data into buffer.  
        // store how many bytes were actually read in bytes_read.
        int bytes_read = read(plaintextFD, buffer, sizeof(buffer));
        if (bytes_read == 0) // We're done reading from the file
            break;

        if (bytes_read < 0) {
            // handle errors
        }

        //write will return how many bytes were written. p keeps
        // track of where in the buffer we are, while we decrement bytes_read
        // to keep track of how many bytes are left to write.
        void *p = buffer;
        while (bytes_read > 0) {
            int bytes_written = write(socketFD, p, bytes_read);
            if (bytes_written <= 0) {
                // handle errors
            }
            bytes_read -= bytes_written;
            p += bytes_written;
        }
    }

    close(plaintextFD);
    write(socketFD, "&", 1);

    int keyFD;
    keyFD = open(argv[2], O_RDONLY);
    if (keyFD == -1)
    {
        printf("open() failed on \"%s\"\n", argv[2]);
        exit(1);
    }

    while (1) {
        // Read data into buffer.  
        // store how many bytes were actually read in bytes_read.
        int bytes_read = read(keyFD, buffer, sizeof(buffer));
        if (bytes_read == 0) // We're done reading from the file
            break;

        if (bytes_read < 0) {
            // handle errors
        }

        // write will return how many bytes were written. p keeps
        // track of where in the buffer we are, while we decrement bytes_read
        // to keep track of how many bytes are left to write.
        void *p = buffer;
        while (bytes_read > 0) {
            int bytes_written = write(socketFD, p, bytes_read);
            if (bytes_written <= 0) {
                // handle errors
            }
            bytes_read -= bytes_written;
            p += bytes_written;
        }
    }
   
    close(keyFD);

    close(socketFD);

    //start listening on new socket
    int listenSocketFD, establishedConnectionFD;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in clientAddress;

	// Set up the address struct for this process (the transfer)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = 12005; // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(12005); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");

    listen(listenSocketFD, 1); // Flip the socket on - it can now receive up to 1 connections

    // Accept a connection, blocking if one is not available until one connects
    sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
    establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
    if (establishedConnectionFD < 0) error("ERROR on accept");


    //printf("waiting for message from client...\n");
    char input_buffer[255];
    bzero(input_buffer, 255);

    //read uncyphered data received from otp_dec_d back to stdout
    int read_return;
    do {
        read_return = read(establishedConnectionFD, input_buffer, 255);
        if (read_return > 0)
        {
            write(1, input_buffer, read_return);
        }
    } while (read_return > 0);

	return 0;
}
