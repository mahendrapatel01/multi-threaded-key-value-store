#include <stdio.h>
#include <unordered_map> 
#include <pthread.h>
#include <fstream>
#include<string.h>
//#include <string>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <iostream>
#include <dirent.h>
#include<unistd.h> 
#include "persistence_hashmap.h"
#include "cache.h"

const int cache_entries= 1000;
using namespace std;

unordered_map <string,int> cache_meta_data;
pthread_mutex_t lock_cache[cache_entries];


PersistentStorage storage;


/*
struct cache_block {
   char key[256];
   char value[256];
   int reference_count;
   bool dirty_bit;
   bool lock_bit;

     };
*/
struct cache_block cache_array[cache_entries];

int no_elements_curr_cache=0;

void cache_initialize()
{
     for(int i=0;i<cache_entries;++i)
    {
       cache_array[i].reference_count=0;
       cache_array[i].dirty_bit=0;
       cache_array[i].lock_bit=0;
       


    }
}

void cache_insert(string key,string value) 
{
	if (cache_meta_data.find(key)==cache_meta_data.end())  //key not in cache so write it in cache with lru
    {
          int min_lfu_index=0;
          int min_lfu=cache_array[0].reference_count;
           for(int i=0;i<cache_entries;++i)
           {
           	  
           	   pthread_mutex_lock(&lock_cache[i]);
           	   if(cache_array[i].reference_count <= min_lfu and cache_array[i].lock_bit==0)
           	   {
		       	   	cache_array[min_lfu_index].lock_bit==0;
		            cache_array[i].lock_bit==1;
		            min_lfu_index=i;
		            min_lfu=cache_array[i].reference_count;
		            if(min_lfu==0)
		              break;

           	   }
           	   pthread_mutex_unlock(&lock_cache[i]);
           }
           cache_array[min_lfu_index].lock_bit==0;
           //here write this index key ,value in persistance storage
		   pthread_mutex_lock(&lock_cache[min_lfu_index]);
		   if(cache_array[min_lfu_index].dirty_bit==1)
		   {
		      string key_insert= cache_array[min_lfu_index].key;
		      string value_insert=cache_array[min_lfu_index].value;
		      storage.insert_key(key_insert,value_insert);
		   }
		         
		 
		   cache_meta_data.erase(cache_array[min_lfu_index].key);    //erase key from hashmap which need to be replaced

		   
		   strcpy(cache_array[min_lfu_index].key,key.c_str());           //update cache
		   strcpy(cache_array[min_lfu_index].value,value.c_str());
		   cache_array[min_lfu_index].reference_count=1;
		   cache_array[min_lfu_index].dirty_bit=1;
		   pthread_mutex_unlock(&lock_cache[min_lfu_index]);


		   cache_meta_data[key]=min_lfu_index;                       //inserting new in cache

     }

    else                                                    //key in cache modify it
    {
         int index = cache_meta_data[key];
         pthread_mutex_lock(&lock_cache[index]);
         strcpy(cache_array[index].value,value.c_str());
         cache_array[index].reference_count++;
         cache_array[index].dirty_bit=1;
         pthread_mutex_unlock(&lock_cache[index]);
     
     }

}



void cache_delete(string key)
{
  if (cache_meta_data.find(key)==cache_meta_data.end())
  {
    storage.delete_key(key);

  }

  else
  {
    int index = cache_meta_data[key];
    pthread_mutex_lock(&lock_cache[index]);
    cache_array[index].reference_count=0;
    cache_array[index].dirty_bit=0;
     pthread_mutex_unlock(&lock_cache[index]);
     storage.delete_key(key);

}



	
}


string cache_find(string key)
{

  if (cache_meta_data.find(key)==cache_meta_data.end())
  {
      string value=storage.find_value(key);
      int min_lfu_index =0;
      int min_lfu=cache_array[0].reference_count;
      
      
      pthread_mutex_lock(&lock_cache[i]);
      
      for(int i=0;i<cache_entries;++i)
        {
              pthread_mutex_lock(&lock_cache[i]);
               if(cache_array[i].reference_count <= min_lfu)
               {
                min_lfu_index=i;
                min_lfu=cache_array[i].reference_count;
                if(min_lfu==0)
                  break;
               }
        }
        cache_array[min_lfu_index].reference_count++;
        pthread_mutex_unlock(&lock_cache[i]);
        
        
        if(cache_array[min_lfu_index].dirty_bit==1)
       {
        string key_insert= cache_array[min_lfu_index].key;
        string value_insert=cache_array[min_lfu_index].value;
        storage.insert_key(key_insert,value_insert);
       }

        cache_meta_data.erase(cache_array[min_lfu_index].key);    //erase key from hashmap which need to be replaced

        pthread_mutex_lock(&lock_cache[min_lfu_index]);
        strcpy(cache_array[min_lfu_index].key,key.c_str());           //update cache
        strcpy(cache_array[min_lfu_index].value,value.c_str());
     //   cache_array[min_lfu_index].reference_count=1;
        cache_array[min_lfu_index].dirty_bit=1;
        pthread_mutex_unlock(&lock_cache[min_lfu_index]);


        cache_meta_data[key]=min_lfu_index;   

        return value;



  }

  else
  {
    
    int index= cache_meta_data[key];
    pthread_mutex_lock(&lock_cache[index]);
    if(cache_array[index].reference_count==0)
    {
      pthread_mutex_unlock(&lock_cache[index]);
      return "";
    }
    else
    {
    cache_array[index].reference_count=1;
    cache_array[index].dirty_bit=0;
    string value = cache_array[index].value;
    pthread_mutex_unlock(&lock_cache[index]);
    return value;
    }

  }


}











