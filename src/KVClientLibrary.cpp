#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string.h>
#include <stdlib.h>
#include <unistd.h>
#include<arpa/inet.h>
#include "KVClientLibrary.h"

static Message *request_message=NULL;

static Message *response_message=NULL;

void print_error(char* er_msg)
{
	perror(er_msg);
	exit(0);
}

int connect_server(int port_no,char* host_name)
{
	int socket_fd;
	struct sockaddr_in server_address;
	struct hostent *server;

	socket_fd = socket(AF_INET,SOCK_STREAM,0);
	if(socket_fd < 0)
		print_error("error while opening socket!!!");

	server = gethostbyname(host_name);
	if(server==NULL)
	{
		print_error("no such host present!!!");
	}

	bzero((char*)&server_address,sizeof(server_address));
	server_address.sin_family = AF_INET;
	bcopy((char*)server->h_addr,
		(char*)&server_address.sin_addr.s_addr,server->h_length);

	server_address.sin_port = htons(port_no);
	if(connect(socket_fd,(struct sockaddr*)&server_address,sizeof(server_address)) < 0)
		print_error("error in connect");

	return socket_fd;
}

//send the key value pair in struct format
int add_kv_pair(int socket_fd,char* key,char* value)
{
	Message* msg = (Message*)malloc(sizeof(Message));
	msg->status_code = '2';
	strcpy(msg->key,key);
	strcpy(msg->value,value);
	int wt = write(socket_fd,(void*)msg,BUFFER_SIZE);
	printf("wt : %d\n",wt);
	if(wt<=0)
	{
		printf("error in write!!!");
		return -1;
	}

	int rd = read(socket_fd,(void*)msg,BUFFER_SIZE);
	if(rd<=0)
	{
		printf("error in read!!!");
		return -1;
	}
	msg = (Message*)msg;
	if((int)msg->status_code==200)
	{
		return 1;
	}
	return 0;
}

//get the key value pair in struct format from server
Message* get_kv_pair(int socket_fd,char* key)
{
	Message* msg = (Message*)malloc(sizeof(Message));
	msg->status_code = '1';
	strcpy(msg->key,key);

	int wt = write(socket_fd,(void*)msg,BUFFER_SIZE);
	if(wt<=0)
	{
		printf("error in write!!!");
		return NULL;
	}

	int rd = read(socket_fd,(void*)msg,BUFFER_SIZE);
	if(rd<=0)
	{
		printf("error in read!!!");
		return NULL;
	}
	msg = (Message*)msg;
	if((int)msg->status_code==200)
	{
		return msg;
	}
	return NULL;
}


//delete the KV value pair
int delete_kv_pair(int socket_fd,char* key)
{
	Message* msg = (Message*)malloc(sizeof(Message));
	msg->status_code = '3';
	strcpy(msg->key,key);

	int wt = write(socket_fd,(void*)msg,BUFFER_SIZE);
	if(wt<=0)
	{
		printf("error in write!!!");
		return -1;
	}

	int rd = read(socket_fd,(void*)msg,BUFFER_SIZE);
	if(rd<=0)
	{
		printf("error in read!!!");
		return -1;
	}
	msg = (Message*)msg;
	if((int)msg->status_code==200)
	{
		return 1;
	}
	return 0;
}

void print_message(struct Message* msg)
{
	printf("status : %d",(int)msg->status_code);
	printf("key : %s",msg->key);
	printf("value : %s",msg->value);
}


void * send_message_to_server(int socket_fd,char statuscode,char* key,char* value)
{

    if(request_message==NULL)
    {
        request_message=(Message*)malloc(sizeof(Message));
    }

    request_message->status_code=statuscode;
    strcpy(request_message->key,key);
    if(status_code=='2')
    	strcpy(request_message->value,value);

    int wt = write(socket_fd,(void*)request_message,BUFFER_SIZE);
	if(wt<=0)
	{
		printf("error in write!!!");
		return NULL;
	}

    return NULL;

}

void * recv_response_from_server(int socket_fd)
{
    if(response_message==NULL)
    {
        response_message=(Message*)malloc(sizeof(Message));
    }

	int rd = read(socket_fd,(void*)response_message,BUFFER_SIZE);
	if(rd<=0)
	{
		printf("error in read!!!");
		return NULL;
	}

	printf("Response got from server\n");

	if((int)response_message->status_code==200)
	{
		return response_message;
	}
	return NULL;


}


void RemoveExtraSpaces(char *str)
{

	size_t len=strlen(str);

	int i=0;
	int j=-1;

	while(++j<len && str[j]==' ');

	bool isSpaceExist=false;

	while(j<=len)
	{
		if(str[j]!=' ')
		{
			if((str[j]==';'||str[j]==','||str[j]=='\0'||str[j]=='|'||str[j]=='<'||str[j]=='>')&&i>0&&str[i-1]==' ')
			{
				str[i-1]=str[j];
				j++;
			}
			else
			{
				str[i++]=str[j++];
			}

			isSpaceExist=false;
		}
		else
		{
			if(isSpaceExist||(!isSpaceExist&&i>0&&(str[i-1]==';'||str[i-1]==','||str[i-1]=='|'||str[i-1]=='<'||str[i-1]=='>')))
			{
				j++;
			}
			else
			{
				str[i++]=str[j++];
				isSpaceExist=true;
			}
		}
	}

}










