#include <pthread.h>
#include "ReaderWriterLock.h"

namespace Sync {

void readlock(struct Lockfile var)
{
	pthread_mutex_lock(&(var.lock));

	if((var.write_count == 1) || (var.waiting_writer >0))
		   {
		   	  var.waiting_reader++;

		   	  pthread_cond_wait(&(var.cond_read),&(var.lock));

		   	  var.waiting_reader--;
		   }


		   var.read_count++;
		   pthread_mutex_unlock(&(var.lock));
		   pthread_cond_broadcast(&(var.cond_read));
}


void readunlock(struct Lockfile var) 
    { 
  
         
        pthread_mutex_lock(&var.lock); 
  
        if (var.read_count == 0) 
            pthread_cond_signal(&var.cond_write); 
  
        pthread_mutex_unlock(&var.lock); 
    } 


  void writelock(struct Lockfile var) 
    { 
        pthread_mutex_lock(&var.lock); 
  
        
        if (var.waiting_writer == 1 || var.waiting_reader > 0) { 
            var.write_count++; 
            pthread_cond_wait(&var.cond_write, &var.lock); 
            var.write_count--; 
        } 
        var.write_count = 1; 
        pthread_mutex_unlock(&var.lock); 
    } 
  
    void writeunlock(struct Lockfile var) 
    { 
        pthread_mutex_lock(&var.lock); 
        var.write_count = 0; 
  
        if (var.waiting_reader > 0) 
            pthread_cond_signal(&var.cond_read); 
        else
            pthread_cond_signal(&var.cond_write); 
        pthread_mutex_unlock(&var.lock); 
    } 

}
  
