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
#include "persistence_hashmap.h"

struct cache_block {
   char key[256];
   char value[256];
   int reference_count;
   bool dirty_bit;
   bool lock_bit;

     };



void cache_insert(string key,string value);
void cache_delete(string key);
string cache_find(string key);
void cache_initialize();