/*
 ============================================================================
 Name        : normal_web_server.c
 Author      : brian
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>
#include <string.h>


int decode(const char *s, char *dec)
{
	char *o;
	const char *end = s + strlen(s);
	int c;

	for (o = dec; s <= end; o++) {
		c = *s++;
		if (c == '+') c = ' ';
		else if (c == '%' && (!ishex(*s++) || !ishex(*s++)	|| !sscanf(s - 2, "%2x", &c)))
			return -1;
		if (dec) *o = c;
	}

	return o - dec;
}

inline int ishex(int x)
{
	return	(x >= '0' && x <= '9')	||
		(x >= 'a' && x <= 'f')	||
		(x >= 'A' && x <= 'F');
}
int main(int argc, char* argv[]){

	char *port=argv[1];

	//char response_data[1024];
	//gets(response_data,1024,html_data);
	//strcat(http_header,response_data);
	//create a socket
	int server_socket;
	server_socket=socket(AF_INET,SOCK_STREAM,0);

	//define the address
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(atoi(port));
	//server_address.sin_port = htons(8001);
	server_address.sin_addr.s_addr = INADDR_ANY;

	bind(server_socket, (struct sockaddr *)& server_address, sizeof(server_address));

	listen(server_socket, 5);

	int client_socket;

	while(1){
		char http_header_Correct[10000]="HTTP/1.1 200 OK\r\n\n";
		char http_header_Error[2048]="HTTP/1.1 404 NOT FOUND\r\n\n";
		char server_receive[256];
		client_socket = accept(server_socket,NULL, NULL);
		//receive request from client
		recv(client_socket,&server_receive,sizeof(server_receive),0);

		//call system function
		char s[256]="";
		char *p;
		p = strtok(server_receive," ");
		char decodeString[256]="";
		int execStatus=0;
		while(p)
		{
		    char out[strlen(p) + 1];
		    puts(decode(p, out) < 0 ? "bad string" : out);
		    for(int i =0; i<6 ;i++){
		    	s[i]=out[i];
		    }
		    if(strcmp(s,"/exec/")==0){
		    	for(int i =6; i<strlen(out) ;i++){
		    		decodeString[i-6]=out[i];
		    	}
		    	execStatus++;
		    	break;
		    }
		    p=strtok(NULL," ");
		 }
		 if(execStatus==0){
			 send(client_socket,http_header_Error, strlen(http_header_Error),0);
		 }else{

			//URLDECODE

			// return command content
			FILE *fstream=NULL;
			char buff[1024];
			char commandContent[10000]="";
			char originalString[256]="";
			strcat(originalString,decodeString);
			memset(buff,0,sizeof(buff));
			int status=system(decodeString);
			if(NULL==(fstream=popen(originalString,"r")))
			{
				fprintf(stderr,"execute command failed: %s",strerror(errno));
				return -1;
			}
			while(NULL!=fgets(buff, sizeof(buff), fstream)) {
				 strcat(commandContent,buff);
				 printf("%s",buff);
			}
			pclose(fstream);

			if(status==-1){
				send(client_socket,http_header_Error, strlen(http_header_Error),0);
			}else{
				if(WIFEXITED(status))
				{
				   if(WEXITSTATUS(status) == 0){
					  strcat(http_header_Correct,commandContent);
					  send(client_socket,http_header_Correct, strlen(http_header_Correct),0);
				   }else{
					  send(client_socket,http_header_Error, strlen(http_header_Error),0);
				   }
				}
			}
		 }
		 close(client_socket);
	}
}

