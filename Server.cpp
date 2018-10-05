/*
CPSC 441 Assignment 1 Bad Spelling Web Proxy
The majority of the code is written in c and uses c++ compiler
Skeleton code of the socket programming gained from CPSC 441 TA, Simpy. Code Client.c and Server.c from https://d2l.ucalgary.ca/d2l/le/content/235059/Home
By: Thomas Vy 
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <string.h>

#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>

#define NUMBEROFERRORS 15 // The number of errors on a webpage
#define BUFF_SIZE 10000 //The buffer size of the receiving array

int main(int args0, char ** args1)
{
	//Setting the address of the listening socket.
	struct sockaddr_in proxy_address;
	int portNumber = 12345; //Sets up on port 12345
	memset(&proxy_address, 0, sizeof(proxy_address));
	proxy_address.sin_family = AF_INET;
	proxy_address.sin_port = htons(portNumber);
	proxy_address.sin_addr.s_addr = htonl(INADDR_ANY);

	//Sets up the listening socket
	int client_listeningSocket;
	client_listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(client_listeningSocket == -1)
	{
		printf("Could not create socket... Terminating program...");
		exit(1);
	}
	printf("Successful creation of listening Socket");

	//Binds the listening socket to the address set for the listening socket
	int status;
	status = bind(client_listeningSocket, (struct sockaddr *) &proxy_address, sizeof(struct sockaddr_in));
	if( status == -1)
	{
		printf("Error in binding\n");
		exit(1);
	}
	printf("Binding complete\n");

	//listen for clients in a queue that holds up to 5 people  
	status = listen(client_listeningSocket, 5);
	if(status == -1)
	{
		printf("Error setting up listening\n");
		exit(1);
	}
	printf("Listening for connection requests...\n");

	//While loop for the listening to clients requests
	while(1){
		//Connects to client
		int client_connected_socket;
		client_connected_socket = accept(client_listeningSocket, NULL, NULL);
		if(client_connected_socket == -1)
		{
			printf("Error in accepting connection\n");
			exit(1);
		}
		printf("Connection established.\n");

		//The request from client held inside receive_msg
		char receive_msg[BUFF_SIZE];
		memset(receive_msg, 0, BUFF_SIZE);
		int count = recv(client_connected_socket, receive_msg, BUFF_SIZE, 0);
		if (count <0) {
			printf("Error in recv()\n");
		} else {
			printf("Client said: %s\n", receive_msg);
		}
		//If the length of the request is 0 quit the connection with current client and get a new client
		if(strlen(receive_msg) ==0)
		{
			close(client_connected_socket);
			continue;
		}
		//If  request is connect, ignore the request and get a new client
		if(!strncmp("CONNECT",receive_msg, 7))
		{
			close(client_connected_socket);
			continue;
		}
		//Grabs the host of the website
		char * host = strtok(strstr(receive_msg, "Host: ")+6, "\r\n");
		//Parse the request so we can send it to the server
		char request[(int)strlen(receive_msg)+4];
		strcpy(request, receive_msg); 
		strcat(request, "\r\n\r\n");

		printf("Host: %s\nPort: %d\n", host, 80);

		//Initializing Address for the server we are trying to connect to 
		struct sockaddr_in webServerAddress;
		struct hostent * address;
		memset(&webServerAddress, 0, sizeof(webServerAddress));
		address= gethostbyname(host);
		webServerAddress.sin_family = AF_INET;
		webServerAddress.sin_port = htons(80);
		bcopy((char *) address->h_addr, (char *) &webServerAddress.sin_addr.s_addr, address->h_length);

		//Creating Proxy to Server socket
		int proxyServerSocket;
		proxyServerSocket = socket(AF_INET, SOCK_STREAM, 0);
		if (proxyServerSocket < 0) {
			printf("Error in socket() while creating lstn_sock\n");
			exit(-1);
		}

		//Connect to the server
		status = connect(proxyServerSocket, (struct sockaddr *) &webServerAddress,
				sizeof(struct sockaddr_in));
		if (status < 0) {
			printf("Error in connect()\n");
			close(client_connected_socket);
			close(proxyServerSocket);
			continue;
		} else {
			printf("Connected.\n");
		}

		/* Send data to the server*/
		printf("HTTP request: %s", request);
		count = send(proxyServerSocket, request, sizeof(request), 0);
		if (count < 0) {
			printf("Error in send()\n");
			close(client_connected_socket);
			close(proxyServerSocket);
			continue;
		}
		printf("Successful sending\n");

		/* Receive data from the server*/
		char recv_buffer[BUFF_SIZE];
		memset(recv_buffer, 0, BUFF_SIZE);
		int bytes_read = 0, temp =0;
		//Let it keep recevining until the server is done sending 
		//Keep receving until the bytes that were received is 0
		while((temp = recv(proxyServerSocket, &recv_buffer[bytes_read], sizeof recv_buffer-temp, 0))>0)
		{
			bytes_read  += temp;
		}
		if(bytes_read == -1)
		{
			printf("Error in receving");
			exit(1);
		}
		printf("%s", recv_buffer);
		//Seperate HTTP Response from content
		char * endHTTPResponse = strstr(recv_buffer, "\r\n\r\n") + 4; //beginning of the content
		char * end = recv_buffer + (int)strlen(recv_buffer); //end of the content
		char * code = recv_buffer +9; //HTTP reponse code:200 or 304 or etc.
		printf("%s", endHTTPResponse);
		
		char content[end - endHTTPResponse +1]; //content array
		strcpy(content, endHTTPResponse);
		srand(time(NULL));
		if(!strncmp(code, "200", 3))// OK response
		{
			//Checks the content type
			char * contentTypePointer = strstr(recv_buffer, "Content-Type:")+14;
			int length = (end - endHTTPResponse);
			//Checks if the content is an html file
			if(!strncmp("text/html",contentTypePointer,9))
			{
				//html file
				char allowed[length]; //An array that tells us which character is allowed to change
				memset(allowed, 1, length);
				bool enteredTag = false;
				//If '<' is encountered flag it and everything forward of it as an unchangeable charcter
				//until a '>' character is encountered
				for(int i=0;i<length;i++)
				{
					if(recv_buffer[i+ (endHTTPResponse - recv_buffer)] == '<')
					{
						enteredTag = true;
					}
					if(enteredTag == true)
					{
							allowed[i] = 0;
					}
					if(recv_buffer[i+(endHTTPResponse - recv_buffer)] == '>')
						enteredTag = false;
				}
				//Choose a random index in the recv-buffer and check if it a legal changeable character.
				//If legal get a random character and change the corresponding index position in recv_buffer
				//If illegal choose another index.
				for(int i= 0; i<NUMBEROFERRORS; i++)
				{
					int index = rand() % (end - endHTTPResponse)+ (endHTTPResponse - recv_buffer);
					if(allowed[index -(endHTTPResponse - recv_buffer)] == 1 && recv_buffer[index] != '\n' && recv_buffer[index] != ' ' && recv_buffer[index] != '\r')
						recv_buffer[index] = rand() % 94 +33;
					else
						i--;
				}
			}
			//content type is a plain text file
			else if (!strncmp("text/plain",contentTypePointer,9))
			{
				//Choose a random index from recv_buffer and get a random character and replace it.
				for(int i= 0; i<NUMBEROFERRORS; i++)
				{
					int index = rand() % (end - endHTTPResponse) + (endHTTPResponse - recv_buffer);
					if(recv_buffer[index] != '\n' && recv_buffer[index] != ' ' && recv_buffer[index] != '\r')
						recv_buffer[index] = rand() % 94 +33;
				}
			}
		}
		//if it is a not 200 response code then just forward the response to the client
		printf("%s", recv_buffer);
		//Send the recv_buffer that contains the response to the client
		count = send(client_connected_socket, recv_buffer, bytes_read, 0);
		if (count < 0) {
			printf("Error in send()\n");
		}
		printf("Successful sending to Client, bytes read: %d\n", bytes_read);
		//Close sockets and get new client
		close(client_connected_socket);
		close(proxyServerSocket);	
	}
	/* Close the listening for clients socket*/
	close(client_listeningSocket);
	return 0;
}