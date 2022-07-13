#include <stdio.h>
#include<string.h>
#include <unordered_map> 
#include <pthread.h>
#include <fstream>
//#include <string>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <iostream>
#include <dirent.h>
#include<unistd.h> 
#include "ReaderWriterLock.h"
#include "persistence_hashmap.h"

using namespace std;

Lockfile locks_arr[number_of_files];

int PersistentStorage::hash_calc(string str)
{
   int hash=0, len = str.length();
   for(int i=0;i<len;++i)
   {
   	hash=hash+str[i]*i*prime_no;
   	hash=hash%number_of_files;
   	//var=var*prime_no;
   	//printf("%d\n",hash);

   }
   //printf("%d",hash);
   return hash;
}

int PersistentStorage::load_hashmap()
{
	struct dirent *de;
	char s[100];
	DIR *dr = opendir(folder_name.c_str());
	if (dr == NULL)  // opendir returns NULL if couldn't open directory 
    { 
        printf("Could not open current directory" ); 
        return 0; 
    } 


    FILE *fp;
    chdir(folder_name.c_str());

    while ((de = readdir(dr)) != NULL) 
    	{    if ((strcmp(de->d_name,".")==0) || (strcmp(de->d_name,"..")==0))
                 continue;
            printf("%s\n", getcwd(s, 100)); 
            printf("%s\n", de->d_name);
            int pos=0;
            fp=fopen(de->d_name,"rb");
            struct FileKeyValue input;
            while(fread(&input, sizeof(struct FileKeyValue ), 1,fp)) 
            {
              printf("Current values :: flag = %d key = %s value = %s\n", input.is_deleted, 
             input.key, input.value);
             int pos= ftell(fp);
             printf("%d\n",pos-write_size );
             string key = string(input.key);
             int hash_val= hash_calc(key);

             files[hash_val][key]={pos};
            
             }


       fclose(fp);


    }
  
    closedir(dr); 
    chdir("..");    
    return 0; 
} 


void PersistentStorage::clean_files()
{

	struct dirent *de;
	char s[100];
	DIR *dr = opendir(folder_name.c_str());
	if (dr == NULL)  // opendir returns NULL if couldn't open directory 
    { 
        printf("Could not open current directory" ); 
        return;
        
    } 

    


    FILE *fp,*fp2;
    chdir(folder_name.c_str());

    while ((de = readdir(dr)) != NULL) 
    	{    if ((strcmp(de->d_name,".")==0) || (strcmp(de->d_name,"..")==0))
                 continue;
            printf("%s\n", getcwd(s, 100)); 
            printf("%s\n", de->d_name);
            int pos=0;
            fp=fopen(de->d_name,"rb");
            fp2= fopen("temp.dat", "wb"); 
            struct FileKeyValue input;
            while(fread(&input, sizeof(struct FileKeyValue ), 1,fp)) 
            {
              printf("Current values :: flag = %d key = %s value = %s\n", input.is_deleted, 
             input.key, input.value);
              if(input.is_deleted==0)
                fwrite (&input, sizeof(FileKeyValue), 1, fp2);
               else
               	continue;
             }
       remove(de->d_name);
       rename("temp.dat",de->d_name);
       fclose(fp2);
       fclose(fp);


    }
    closedir(dr);
    chdir("..");      
    
    
}






const char* PersistentStorage::file_path(int hash_val)
{
    string *str = new string(folder_name + "/" + to_string(hash_val) + ".dat");
    return str->c_str();
}


int PersistentStorage::insert_key(string key, string value)
{
	int hash_val=hash_calc(key);

	printf( "%s ***\n\n " , file_path(hash_val));
	FILE *fp;
    struct FileKeyValue file_obj;
    file_obj.is_deleted = 0;
    strcpy(file_obj.key,key.c_str());
    strcpy(file_obj.value,value.c_str());
    Sync::writelock(locks_arr[hash_val]); //locking a file


     
    if(files.find(hash_val)==files.end())
    {
    	fp = fopen(file_path(hash_val),"wb");
    	if(fp==NULL)
    	{
    		printf("Error opening file\n");
        Sync::writeunlock(locks_arr[hash_val]);
        return 0;
    	}
      
    	fwrite(&file_obj, sizeof(FileKeyValue), 1, fp); 
        files[hash_val][key]={0 /*todo : add more value */};    	
     }

    else if(files[hash_val].find(key)==files[hash_val].end())
    {

 		printf("Inserting New Key\n"); /* todo - your map should have deleted entries also */ 
        fp = fopen(file_path(hash_val),"ab+");
        files[hash_val][key]={ftell(fp)};
    	fwrite (&file_obj, sizeof(FileKeyValue), 1, fp);
    	
    	
    }
    else
    {
       
       fp=fopen(file_path(hash_val),"rb+");
       int curr_ptr= ftell(fp);
       Position entry_pos=files[hash_val][key];
       int seek_pos=entry_pos.offset;

       printf("Modifying existing key\n");
       fseek(fp, seek_pos, SEEK_SET );
       struct FileKeyValue input;
       fread(&input, sizeof(FileKeyValue), 1, fp);
       printf("Current values :: flag = %d key = %s value = %s\n", input.is_deleted, 
             input.key, input.value); 
            
            strcpy(input.value,value.c_str());
    		input.is_deleted=0;
    		printf("Modified values :: flag = %d key = %s value = %s\n", input.is_deleted, 
             input.key, input.value); 
            fseek(fp,-1* sizeof(FileKeyValue),SEEK_CUR);
            fwrite (&input, sizeof(struct FileKeyValue), 1, fp);
    }
      fclose(fp);
      Sync::writeunlock(locks_arr[hash_val]);

    return 1;
}


 int PersistentStorage::delete_key(string key)
{
   int hash_val=hash_calc(key);
    Sync::writelock(locks_arr[hash_val]);
   FILE *fp=fopen(file_path(hash_val),"rb+");
   if(fp==NULL)
   {
   		printf("Error opening file or key doesn't exist\n");
      Sync::writeunlock(locks_arr[hash_val]);
   		return 0;
    	
   }

   else if(files[hash_val].find(key)==files[hash_val].end())
   { 
      Sync::writeunlock(locks_arr[hash_val]);
   	  printf("Key to be deleted doesn't exist\n");
   	  return 0;
   }

   else
    {
      struct Position entry_pos=files[hash_val][key];   		
      int seek_pos=entry_pos.offset;
      fseek( fp, seek_pos, SEEK_SET );
      struct FileKeyValue input;
      fread(&input, sizeof(struct FileKeyValue), 1, fp);
      printf ("flag = %d key = %s value = %s\n", input.is_deleted, 
              input.key, input.value); 
      
      if(input.is_deleted==1)
      {
        Sync::writeunlock(locks_arr[hash_val]);
      	printf("Key to be deleted doesn't exist");
      	return 0;
      }

      input.is_deleted=1;
      fseek(fp,-513,SEEK_CUR);
      int pos= ftell(fp);
      printf("%d\n",pos );
      fwrite (&input, sizeof(struct FileKeyValue), 1, fp);
      fread(&input, sizeof(struct FileKeyValue), 1, fp);
      printf ("flag = %d key = %s value = %s\n", input.is_deleted, 
          input.key, input.value); 


           fclose(fp);
        Sync::writeunlock(locks_arr[hash_val]);
    		return 1;
    	}
    Sync::writeunlock(locks_arr[hash_val]);
    return 1;
 }

string PersistentStorage::find_value(string key)
 {
	int hash_val=hash_calc(key); 

    string value;
    if(files.find(hash_val)==files.end())
     { 
     	value="";
    	printf("This key doesn't exist");
     	return value;
     }

    if(files[hash_val].find(key)==files[hash_val].end())
     {
     	value="";
     	printf("This key doesn't exist");
     	return value;
     }
  
 	struct FileKeyValue input;
    Sync::readlock(locks_arr[hash_val]);
    FILE *fp=fopen(file_path(hash_val),"rb");
     if(fp==NULL)
     	{
        Sync::readunlock(locks_arr[hash_val]);
    		printf("Error opening file\n");
     		return NULL;
    	}
     else
     	{
     		struct Position entry_pos=files[hash_val][key];
     		int seek_pos=entry_pos.offset;
     		fseek( fp, seek_pos, SEEK_SET );
    	
    		
     		fread(&input, sizeof(struct FileKeyValue), 1, fp);
    		printf ("flag = %d key = %s value = %s\n", input.is_deleted, 
            input.key, input.value);
           if(input.is_deleted==0)
           {
     		value=input.value;
     		}
     	  else
     	  {
     	  	value="";
     	  	printf("the value is deleted\n");
     	  }


            fclose(fp);
        Sync::readunlock(locks_arr[hash_val]);
    		return value;
    	}
      
      Sync::readunlock(locks_arr[hash_val]);
     	return value; 
     }

// int main()
// {

//   PersistentStorage storage;

//   //makefolder();
//   ifstream fin("input.txt");
//   typedef unordered_map<string, string>::iterator map_ite_t;
//   unordered_map<string, string> input_data;
//   string key, value;
//   while(fin) {
//       fin>>key;
//       fin>>value;
//       input_data.insert({key, value});
//       cout<<key << "    "<< value << '\n';

//   }
//   map_ite_t map_ite = input_data.begin();
//   while(map_ite != input_data.end()) {
//     storage.insert_key(map_ite->first, map_ite->second);
//     map_ite++;
//   }

//   // insert_key("ajay", "good boy");
//   // insert_key("Surbhi", "Jain");
//   string ans=storage.find_value("Surbhi");
//   storage.delete_key("ajay");
//   // insert_key("ajay", "bad boy");
//   cout<<storage.find_value("ajay");
//   printf("%s\n",ans.c_str() );
//   storage.load_hashmap();
//   storage.clean_files();
//   storage.load_hashmap();
//     return 0;
// }
