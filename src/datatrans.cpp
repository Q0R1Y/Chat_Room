#include <datatrans.h>
#include <scanfile.h>
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

std::string SRC_DIR;
std::string BACKUP_DIR;

int data_from_to(std::string src, std::string des, int begin, int size)
{
	int fds, fdd;
	umask(0);
	if ((fds = open(src.c_str(), O_RDONLY)) == -1 || (fdd = open(des.c_str(), O_WRONLY | O_CREAT, 0644)) == -1)
		return perror("32 open() error"), 1;

	if (lseek(fds, begin, SEEK_SET) == -1 || lseek(fdd, begin, SEEK_SET) == -1)
		return perror("35 lseek() error"), 1;

	int len;
	char buf[1007];
	for (int i = 0; i < size / (int)sizeof(buf) + 1; ++i)
	{
		if ((len = read(fds, buf, sizeof(buf))) == -1)
			return 1;
		if (write(fdd, buf, len) == -1)
			return 1;
	}

	close(fds);
	close(fdd);

	return 0;
}

void *data_trans_thread(void *args)
{
	void *re = malloc(sizeof(int));
	*(int *)re = data_from_to(((Data_trans *)args)->src, ((Data_trans *)args)->des,
			((Data_trans *)args)->begin, ((Data_trans *)args)->size);
	return re;
}

int del_file(std::string src)
{
	DIR *path = opendir(src.c_str());
	if (path == NULL)
		return remove(src.c_str());
	else
	{
		struct dirent *dir_info = NULL;
		while ((dir_info = readdir(path)))
		{
			if (strcmp(dir_info->d_name, ".") == 0 || strcmp(dir_info->d_name, ".."))
				continue;
			std::string temp = (src + "/") + dir_info->d_name;
			del_file(temp);
		}
		remove(src.c_str());
	}
	if (closedir(path) == -1)
	{
		perror("closedir error");
		exit(EXIT_FAILURE);
	}
	return 0;
}

// src is a relative path of a file.
// dir is a relative path of a dir.
int file_exist(std::string src, std::string dir)
{
	std::vector<std::string> src_split_path;
	size_t prepos = 0, nowpos = 0;
	while ((nowpos = src.find("/", nowpos)) != std::string::npos) // split src by '/'
	{
		src_split_path.push_back(src.substr(prepos, nowpos - prepos));
		prepos = ++nowpos;
	}
	src_split_path.push_back(src.substr(prepos, nowpos - prepos));

	std::string rela_path = dir + "/"; // relative path of src in dir

	int flag = 0;
	int size = src_split_path.size();
	for (int i = 0; i < size; ++i)
	{
		if (flag)
			rela_path = rela_path + src_split_path[i] + "/";
		if (src_split_path[i] == SRC_DIR.substr(SRC_DIR.rfind("/") + 1))
			flag = 1;
	}
	rela_path.pop_back(); // pop_back() to pop '/'
	if (access(rela_path.c_str(), F_OK) == 0)
		return 1;
	return 0;
}

int cp_file(std::string src, std::string des)
{
	umask(0);

	std::vector<std::string> des_split_path;
	size_t prepos = 0, nowpos = 0;
	while ((nowpos = src.find("/", nowpos)) != std::string::npos) // split src by '/'
	{
		des_split_path.push_back(des.substr(prepos, nowpos - prepos));
		prepos = ++nowpos;
	}
	des_split_path.push_back(des.substr(prepos, nowpos - prepos));

	std::string des_s=des.substr(0,des.find("/", 0));
	for(int i=1;i+1<(int)des_split_path.size();++i)
		mkdir((des_s+"/"+des_split_path[i]).c_str(),0777),des_s=des_s+"/"+des_split_path[i];

	int fd_src, fd_des;
	if ((fd_src = open(src.c_str(), O_RDONLY)) == -1)
		return perror("src open fail"), 1;
	if ((fd_des = open(des.c_str(), O_WRONLY | O_CREAT, 0777)) == -1)
		return perror("src open fail"), 1;

	int size = lseek(fd_src, 0, SEEK_END);

	pthread_t thread = 0;
	std::vector<Data_trans *> args;
	std::vector<pthread_t> threads;

	if (size > SIGLE_THREAD_MAX_DATA_SIZE)
	{
		for (int i = 0; i < size / SIGLE_THREAD_MAX_DATA_SIZE; ++i)
		{
			threads.push_back(thread);
			Data_trans *t = (Data_trans *)malloc(sizeof(Data_trans));
			t->begin = i * SIGLE_THREAD_MAX_DATA_SIZE;
			t->size = SIGLE_THREAD_MAX_DATA_SIZE;
			strcpy(t->src, src.c_str());
			strcpy(t->des, des.c_str());
			args.push_back(t);
			if (pthread_create(&threads[i], NULL, data_trans_thread, args[i]) != 0)
				return perror("pthread_create error"), 1;
		}
		if (size % SIGLE_THREAD_MAX_DATA_SIZE)
		{
			threads.push_back(thread);
			Data_trans *t = (Data_trans *)malloc(sizeof(Data_trans));
			;
			t->begin = size / SIGLE_THREAD_MAX_DATA_SIZE * SIGLE_THREAD_MAX_DATA_SIZE;
			t->size = size - size / SIGLE_THREAD_MAX_DATA_SIZE * SIGLE_THREAD_MAX_DATA_SIZE;
			strcpy(t->src, src.c_str());
			strcpy(t->des, des.c_str());
			args.push_back(t);
			if (pthread_create(&threads[threads.size() - 1], NULL, data_trans_thread, args[size / SIGLE_THREAD_MAX_DATA_SIZE]) != 0)
				return perror("pthread_create error"), 1;
		}

		for (int i = 0; i < size / SIGLE_THREAD_MAX_DATA_SIZE; ++i)
		{
			void *re;
			if (pthread_join(threads[i], &re))
				perror("166 pthread_join error");
			if (*(int *)re != 0)
				return 1;
		}
		if (size % SIGLE_THREAD_MAX_DATA_SIZE)
		{
			void *re;
			if (pthread_join(threads[size / SIGLE_THREAD_MAX_DATA_SIZE], &re))
				perror("175 pthread_join error");
			if (*(int *)re != 0)
				return 1;
		}
	}
	else
	{
		data_from_to(src, des, 0, size);
	}
	return 0;
}

void sync_dir(std::string src,std::string des,int flag)
{
	umask(0);
	if(flag==1)
	{
		del_file(des);
		mkdir(des.c_str(),0777);
	}

	std::deque<std::string> files;
	get_files_name(src,files);

	key_t key=ftok("./",2);
	int sem_id;
	if((sem_id=semget(key,1,IPC_CREAT|0777))==-1&&errno!=EEXIST)
		perror("semget error:");

	semctl(sem_id,0,SETVAL,MAX_PROCESS_NU);

	struct sembuf V[1]{{0,+1,0}};
	struct sembuf P[1]{{0,-1,0}};
	int nu=files.size();
	while(files.size())
	{
		pid_t pid;

		if((pid=fork())==-1)
			perror("fork error:");
		else if(pid!=0)
		{
			if(semop(sem_id,P,1)==-1)
				perror("semop error:");
		}
		else
		{
			std::string from=files.front();
			std::string to=des.substr(0,from.find("/",0))+"/"+from.substr(from.find("/",0)+1);
			if(file_exist(from,des.substr(0,from.find("/",0))))
			{
				struct stat finfo,tinfo;
				stat(from.c_str(),&finfo);
				stat(to.c_str(),&tinfo);
				if(finfo.st_mtim.tv_sec>tinfo.st_mtim.tv_sec)
					cp_file(from,to);
			}
			else
				cp_file(from,to);
			if(semop(sem_id,V,1)==-1)
				perror("semop error:");
			exit(0);
		}
		files.pop_front();
	}
	while(nu--) wait(NULL);
	semctl(sem_id,0,IPC_RMID);
}
