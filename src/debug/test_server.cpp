#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

#include<sys/socket.h>
#include<sys/types.h>

#include<netinet/in.h>
#include<arpa/inet.h>

#include<dbg.h>

FILE *log_file;

char buf[1024];

int main(int argc,char **argv)
{
	log_file=fopen(S_LOG_PATH,S_LOG_MODE);
	if(argc!=2)
	{
		LOG("%s\tserver\targc error",__TIME__);
		return 1;
	}

	int sfd=socket(AF_INET,SOCK_STREAM,0),cfd;
	if(sfd<0)
	{
		LOG("%s\tserver\tsocket() error",__TIME__);
		return 1;
	}

	struct sockaddr_in saddr;
	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(atoi(argv[1]));

	socklen_t saddr_len=sizeof(saddr);

	if(bind(sfd,(struct sockaddr*)&saddr,sizeof(saddr))<0)
	{
		LOG("%s\tserver\tbind() error",__TIME__);
		return 1;
	}

	if(listen(sfd,10)<0)
	{
		LOG("%s\tserver\tlisten() error",__TIME__);
		return 1;
	}

	if((cfd=accept(sfd,(struct sockaddr*)&saddr,&saddr_len))<0)
	{
		LOG("%s\tserver\taccept() error",__TIME__);
		return 1;
	}
	int len=read(cfd,buf,sizeof(buf));
	buf[len]=0;
	printf("%s",buf);

	close(sfd);
	fclose(log_file);

	return 0;
}
