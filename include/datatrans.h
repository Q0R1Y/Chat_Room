#ifndef _DATATRANS_H_
#define _DATATRANS_H_
#include<string>

#define SIGLE_THREAD_MAX_DATA_SIZE (500*(1<<20))

struct Data_trans
{
	char src[100];
	char des[100];
	int begin;
	int size;
};

// pass struct Data_trans
void *data_trans_thread(void *args);

// copy form file "src" to file "des" from "begin" with bolck of "size"
// return 0 if success
int data_from_to(std::string src, std::string des, int begin, int size);

// 0 for success
int del_file(std::string src);

// 1 for exist
int file_exist(std::string src,std::string dir);

// 0 for success
int cp_file(std::string src,std::string des);


// sync src from des
// flag=1: copy
// flag=0: sync
void sync_dir(std::string src,std::string des,int flag);
#endif
