#include<iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include<string.h>
#include <netinet/in.h>
#include<sys/time.h>
#include<pthread.h>
#include<sys/epoll.h>
#include <errno.h>
#include<vector>
#include<algorithm>
#define MAX_EVENTS 10
#include "KVClientLibrary.h"
#include "config.h"
#include "cache.h"

using namespace std;

pthread_cond_t *worker_cond_var=NULL;

pthread_mutex_t m;

vector<int> *tfdsets=NULL;



void ProcessRequest(Message *input)
{

    char status_code=input->status_code;

    char *key= input->key;

    char *val=input->value;

    if(status_code=='1')
    {

        //TODO:Need to call get request

        //find_
        cache_find(key);

    }

    else if(status_code=='2')
    {
         //TODO:Need to call PUT request
         printf("got 2 : %s %s ",key,val);
         cache_insert(key,val);
    }

    else
    {
        //TODO:Need to call Delete request
        cache_delete(key);
    }


}


void * process_conn(void *args)
{
    int my_rank=*(int *)args;

    printf("Myrank=%d\n",my_rank);

    struct epoll_event ev,events[MAX_EVENTS];

    int epfd=epoll_create1(0);

    Message* msg = (Message*)malloc(sizeof(Message));

     if(epfd == -1)
  {
    fprintf(stderr, "Failed to create epoll file descriptor\n");
    exit(1);
  }
    while(1)
    {
        pthread_mutex_lock(&m);

        while(tfdsets[my_rank].size()==0)
        {
        pthread_cond_wait(&worker_cond_var[my_rank],&m);
        }

        pthread_mutex_unlock(&m);

        for (int i = 0; i < tfdsets[my_rank].size(); i++)
        {
            ev.data.fd=tfdsets[my_rank][i];
            ev.events=EPOLLIN | EPOLLET;

         if (epoll_ctl(epfd, EPOLL_CTL_ADD, tfdsets[my_rank][i], &ev) == -1&&errno!=EEXIST)
         {
            perror("epoll_ctl: error while adding");
            exit(1);
        }
        }

       int nfds= epoll_wait(epfd, events, MAX_EVENTS, -1);

       for(int i=0;i<nfds;i++)
       {

           int c= read(events[i].data.fd,(void *)msg,BUFFER_SIZE);
            if(c!=0)
            {

				ProcessRequest((Message*)msg);
                send(events[i].data.fd,"I am server",sizeof("I am server"),0);

            }

           else if(c==0)
           {
            //Deleting client fd if it is closed
            if(epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL) == -1)
            {

                perror("epoll_ctl: Error while deleting");
                exit(1);
            }

            tfdsets[my_rank].erase(remove(tfdsets[my_rank].begin(), tfdsets[my_rank].end(), events[i].data.fd), tfdsets[my_rank].end());

           }
       }


    }

}

int main()
{

	worker_cond_var=(pthread_cond_t *)malloc(sizeof(pthread_cond_t)*THREAD_POOL_SIZE);

	if(worker_cond_var==NULL)
	{
		perror("malloc error");
			exit(1);
	}

	tfdsets=(vector<int> *)malloc(sizeof(vector<int>)*THREAD_POOL_SIZE);

	if(tfdsets==NULL)
	{
		perror("malloc error");
			exit(1);
	}

	pthread_t *wthreads=(pthread_t *)malloc(sizeof(pthread_t)*THREAD_POOL_SIZE);

	if(wthreads==NULL)
	{
	  perror("malloc error");
			exit(1);
	}

	int *rankarr=(int *)malloc(sizeof(int)*THREAD_POOL_SIZE);

	if(rankarr==NULL)
	{
	  perror("malloc error");
			exit(1);
	}

	struct sockaddr_in addr;

	int sfd;

	socklen_t peer_addr_size;

	sfd=socket(AF_INET,SOCK_STREAM,0);

	if(sfd<0)
	{
	   printf("Socket creation error \n");
		 exit(1);
	}
	memset(&addr,0,sizeof(struct sockaddr));

	addr.sin_port=htons(PORT_NUMBER);

	addr.sin_family=AF_INET;

	 memset(&addr.sin_addr,0,sizeof(addr.sin_addr));

	addr.sin_addr.s_addr=INADDR_ANY;

	if(bind(sfd,(struct sockaddr*)&addr ,sizeof(addr))<0)
	{
	    perror("bind failed\n");
		exit(1);
	}

	if(listen(sfd,3)<0)
	{
		perror("listen failed\n");
		exit(1);
	}

	peer_addr_size=sizeof(addr);

	int counter=0;

	int i;

	for(i=0;i<THREAD_POOL_SIZE;i++)
	{

	rankarr[i]=i;

	pthread_create(&wthreads[i],NULL,process_conn,(void *)&rankarr[i]);

	pthread_detach(wthreads[i]);
	}

	while(1)
	{
		int cfd;

		int index=(counter)%THREAD_POOL_SIZE;

		if((cfd=accept(sfd,(struct sockaddr *)&addr,&peer_addr_size))<0)
		{
			perror("accept\n");
			exit(1);
		}

		tfdsets[index].push_back(cfd);

		pthread_cond_signal(&worker_cond_var[index]);

		counter++;
	}

return 0;

}
