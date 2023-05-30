#include<scanfile.h>
#include<fcntl.h>
#include<vector>
#include<unistd.h>
#include<dirent.h>
#include<sys/stat.h>

void get_files_name(std::string name,std::deque<std::string> &stor)
{
	DIR *dir=NULL;
	if((dir=opendir(name.c_str()))==NULL)
	{
		stor.push_back(name);
		return ;
	}

	struct dirent *info=NULL;

	while((info=readdir(dir)))
	{
		if(info->d_name[0]=='.')
			continue;
		get_files_name(name+"/"+info->d_name,stor);
	}

}


int total_size(const std::deque<std::string> &files)
{
	int res=0;
	for(int i=0;i<(int)files.size();++i)
		res+=lseek(open(files[i].c_str(),O_WRONLY),0,SEEK_END);
	
	return res;
}


void* progress_bar(void* args)
{
	extern std::string SRC_DIR;
	extern std::string BACKUP_DIR;

	for(int i=0;i<5;++i)
	{
		int j;
		for(j=0;j<i*10;++j)
			printf("-");
		for(;j<50;++j)
			printf(" ");
		printf("%3d%%",i*20);
		fflush(stdout);
		usleep(5000);
		for(int j=0;j<54;++j)
			printf("\b");
	}
	sleep(1);
	for(int i=0;i<50;++i)
		printf("-");
	printf("%3d%%\n",100);
	return NULL;
}
