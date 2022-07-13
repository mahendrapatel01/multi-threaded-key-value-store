#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 513

struct Message{
	char status_code;
	char key[256];
	char value[256];
};
typedef struct Message Message;

int connect_server(int port_no,char* host_name);

void* send_message_to_server(int socket_fd,char statuscode,char* key,char* value);

int add_kv_pair(int socket_fd,char* key,char* value);

Message* get_kv_pair(int socket_fd,char* key);

int delete_kv_pair(int socket_fd,char* key);

void print_error(char* er_msg);

void print_message(struct Message* msg);

void* recv_response_from_server(int socket_fd);

void RemoveExtraSpaces(char *str);
