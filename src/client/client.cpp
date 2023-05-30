#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

#include<pthread.h>

#include<sys/socket.h>
#include<sys/types.h>

#include<netinet/in.h>
#include<arpa/inet.h>

#include<dbg.h>

FILE *log_file;


// read from server
void *read_thread(void *args)
{
	int sfd=*(int*)args;
	char buf[1024];
	while(1)
	{
		int len=1;
		if((len=recv(sfd,buf,sizeof(buf)-1,0))<0)
		{
			LOG("%s\tclient\tread_thread recv() error",__TIME__);
			break;
		}
		if(!strcmp(buf,"quit\n"))
		{
			break;
		}
		printf("%d\n",len);
		buf[len]=0;
		if(len!=0) LOG("%s",buf);
		printf("%s\n",buf);
	}
	LOG("%s\tclient\tread_thread quit",__TIME__);
	return NULL;
}

// write to server
void *write_thread(void *args)
{
	int sfd=*(int*)args;
	char *buf;
	size_t buf_len;
	int len;
	while(1)
	{
		if((len=getline(&buf,&buf_len,stdin))<0)
		{
			LOG("%s\tclient\twrite_thread getline()",__TIME__);
			break;
		}
		send(sfd,buf,len,0);
		if(!strcmp(buf,"quit\n"))
		{
			break;
		}
	}
	free(buf);
	LOG("%s\tclient\twrite_thread quit",__TIME__);
	return NULL;
}


int main(int argc,char **argv)
{
	log_file=fopen(C_LOG_PATH,C_LOG_MODE);
	if(argc!=3)
	{
		LOG("%s\tclient\targc error",__TIME__);
		return 1;
	}

	int cfd;
	if((cfd=socket(AF_INET,SOCK_STREAM,0))<0)
	{
		LOG("%s\tclient\tsocket() error",__TIME__);
		return 1;
	}
	
	struct sockaddr_in saddr;
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family=AF_INET;
	inet_pton(AF_INET,argv[1],&saddr.sin_addr.s_addr);
	saddr.sin_port=htons(atoi(argv[2]));

	if(connect(cfd,(struct sockaddr*)&saddr,sizeof(saddr))<0)
	{
		LOG("%s\tclient\tconnect() error",__TIME__);
		return 1;
	}
	LOG("%s\tclient\tconnect() success",__TIME__);


	pthread_t rthread,wthread;

	pthread_create(&rthread,NULL,read_thread,&cfd);
	pthread_create(&wthread,NULL,write_thread,&cfd);
	
	pthread_join(wthread,NULL);
	pthread_join(rthread,NULL);
	
	close(cfd);

	return 0;
}
