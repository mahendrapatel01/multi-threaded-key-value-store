#include<string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include<arpa/inet.h>
#include "KVClientLibrary.h"

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int  portno, n;
	char key[256],value[256];
	char status_code;
    char buffer[513];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    printf("%s %s\n",argv[1],argv[2]);
    portno = atoi(argv[2]);

    struct sockaddr_in serv_addr;

    memset(&serv_addr,0,sizeof(struct sockaddr_in));

    int sfd=0;

if((sfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		 printf("\n Socket creation error \n");
		 exit(1);
	}
serv_addr.sin_port=htons(portno);
serv_addr.sin_family=AF_INET;

memset(serv_addr.sin_zero,0,sizeof(serv_addr.sin_zero));

memset(&serv_addr.sin_addr,0,sizeof(serv_addr.sin_addr));

if(inet_pton(AF_INET,argv[1],&(serv_addr.sin_addr))<=0)
	{
		 printf("\nInvalid address/ Address not supported \n");
        	exit(1);
	}

	if(connect(sfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0)
	{
		 printf("\nConnection Failed \n");
		 exit(1);
	}

     size_t inputlen;
     size_t size=0;
	char *input=NULL;
   Message *msg ;
   char k[512];
   char *inputCommands[3];
   while(1)
   {
		printf("enter the Request :");

		inputlen=getline(&input,&size,stdin);

		input[inputlen-1]='\0';

		RemoveExtraSpaces(input);

        int commandsCount=0;
        char *token=strtok(input," ");
		while(token!=NULL)
		{
			inputCommands[commandsCount++]=token;
			token=strtok(NULL," ");
		}
		if(inputCommands[0][0]=="2")
    		send_message_to_server(sfd,inputCommands[0][0],inputCommands[1],inputCommands[2]);
    	else
    		send_message_to_server(sfd,inputCommands[0][0],inputCommands[1],NULL);
    	recv_response_from_server(sfd);
   }

    return 0;
}
