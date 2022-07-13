#pragma once

// To access Sync::readlock()
namespace Sync {

const int number_of_files=10;

struct Lockfile
{
	int read_count;
	int write_count;
	int waiting_reader;
	int waiting_writer;
	pthread_mutex_t lock;
	pthread_cond_t cond_read;
	pthread_cond_t cond_write;
};

void readlock(struct Lockfile var);


void readunlock(struct Lockfile var) ;


  void writelock(struct Lockfile var);
  
    void writeunlock(struct Lockfile var);

}
  
