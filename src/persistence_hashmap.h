#pragma once
#include <stdio.h>
#include <unordered_map> 
#include <pthread.h>
#include <fstream>
#include <string>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <iostream>
#include <dirent.h>
#include<unistd.h> 
#include "ReaderWriterLock.h"

using namespace std;
using namespace Sync;

struct Position
{
  long offset;
};

struct FileKeyValue
{
	bool is_deleted;      // 1 if soft deleted otherwise 0
	char key[256];
	char value[256];
};

class PersistentStorage {
  public :

PersistentStorage() : prime_no(97), write_size(513) {}
int insert_key(string key, string value);
 int delete_key(string key);
string find_value(string key);
int load_hashmap(); 
void clean_files();

  private :
  unordered_map <int, unordered_map <string,Position> > files;

const int prime_no;
const int write_size;

string folder_name = "Kvstore";
int hash_calc(string str);
const char* file_path(int hash_val);
void makefolder()
{	
	mkdir(folder_name.c_str(),0777); 
}

};



