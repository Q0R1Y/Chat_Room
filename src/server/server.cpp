#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

#include<string>
#include<map>

#include<sys/socket.h>
#include<sys/types.h>

#include<netinet/in.h>
#include<arpa/inet.h>

#include<dbg.h>

std::map<int,std::string> fd_id;

FILE *log_file;

fd_set fds,sfds,mute_fds;
int max_fd;

volatile int q_flag=0;

char buf[1024];

void *room_thread(void *args)
{
	struct sockaddr_in saddr;
	socklen_t saddr_len=sizeof(saddr);

	int sfd=*(int*)args,cfd;
	while(1)
	{
		sfds=fds;
		memcpy(&sfds,&fds,sizeof(fds));
		if(q_flag)
			break;
		//		LOG("%s\tserver\tselect() begin",__TIME__);
		if(select(max_fd+1,&sfds,NULL,&mute_fds,NULL)<0)
		{
			LOG("%s\tserver\tselect() error",__TIME__);
			return NULL;
		}
		//		LOG("%s\tserver\tselect() success",__TIME__);
		for(int i=0;i<=max_fd;++i)
		{
			if(FD_ISSET(i,&sfds))
			{
				if(i==sfd)
				{
					if((cfd=accept(sfd,(struct sockaddr*)&saddr,&saddr_len))<0)
					{
						LOG("%s\tserver\taccept() error",__TIME__);
						return NULL;
					}
					FD_SET(cfd,&fds);
					max_fd=max_fd<cfd?cfd:max_fd;
					int len=recv(cfd,buf,sizeof(buf)-1,0);
					buf[len]=0;
					fd_id[cfd]=std::string(buf);
					LOG("%d %s",cfd,buf);
				}
				else
				{
					int len=1;
					if((len=recv(i,buf,sizeof(buf)-1,0))<0)
					{
						LOG("%s\tserver\t%d recv() error",__TIME__,i);
						FD_CLR(i,&fds);
						close(i);
					}
					else
					{
						buf[len]=0;
						LOG("%s\tserver\t%d:%s",__TIME__,i,buf);
						if(!strcmp(buf,"quit\n"))
						{
							if(send(i,buf,len,0)<0)
							{
								LOG("%s\tserver\t%d to %d send() error",__TIME__,i,i);
							}
							FD_CLR(i,&fds);
							close(i);
						}
						else
						{
							for(int j=0;j<=max_fd;++j)
							{
								if(FD_ISSET(j,&fds)&&j!=sfd&&j!=i)
								{
									if(send(j,buf,len,0)<0)
									{
										LOG("%s\tserver\t%d to %d send() error",__TIME__,i,j);
									}
									LOG("%s\tserver\t%d to %d send() success",__TIME__,i,j);
								}
							}
						}
					}
				}
			}
		}
	}

	return NULL;
}
void o_menu()
{
	printf("0.\tquit\n");
	printf("1.\tmute\n");
	printf("2.\tunmute\n");
	printf("3.\tkick\n");
	printf("4.\tcheck_online_user\n");
	printf("5.\tfile_trans\n");
}

#include<iostream>
void mute()
{
	printf("Input ID:\n");
	std::string a;
	std::cin>>a;
	for(auto it=fd_id.begin();it!=fd_id.end();++it)
		if(a==it->second)
		{
			send(it->first,"You have been mute\n",strlen("You have been mute\n"),0);
			FD_SET(it->first,&mute_fds);
			break;
		}

}


void unmute()
{
	printf("Input ID:\n");
	std::string a;
	std::cin>>a;
	for(auto it=fd_id.begin();it!=fd_id.end();++it)
		if(a==it->second&&FD_ISSET(it->first,&mute_fds))
		{
			send(it->first,"You have been unmute\n",strlen("You have been unmute\n"),0);
			FD_CLR(it->first,&mute_fds);
			break;
		}
}

void kick()
{
	printf("Input ID:\n");
	std::string a;
	std::cin>>a;
	int op=-1;
	for(auto it=fd_id.begin();it!=fd_id.end();++it)
		if(a==it->second)
		{
			send(it->first,"You have been kick\n",strlen("You have been kick\n"),0);
			op=it->first;
			break;
		}
	FD_CLR(op,&fds);
	close(op);
}

void check_users()
{
	int cnt=0;
	for(int i=0;i<max_fd;++i)
	{
		if(FD_ISSET(i,&fds))
		{
			printf("%s\n",fd_id[i].c_str());
			cnt++;
		}
	}
	printf("total:%d\n",cnt);
}

void *control_thread(void *args)
{
	while(1)
	{
		o_menu();
		int op;
		scanf("%d",&op);
		switch(op)
		{
		case 0:
			q_flag=1;
			return NULL;
		case 1:
			mute();
			break;
		case 2:
			unmute();
			break;
		case 3:
			kick();
			break;
		case 4:
			check_users();
			break;
		case 5:
//			file_trans();
			break;
		default:
			printf("INVALID INPUT\n");
			break;
		}
	}

	return NULL;
}

void *j_flag(void *args)
{
	while(!q_flag)
		sleep(1);
	return NULL;
}

int main(int argc,char **argv)
{
	log_file=fopen(S_LOG_PATH,S_LOG_MODE);
	if(argc!=2)
	{
		LOG("%s\tserver\targc error",__TIME__);
		return 1;
	}

	int sfd=socket(AF_INET,SOCK_STREAM,0);
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

	if(bind(sfd,(struct sockaddr*)&saddr,saddr_len)<0)
	{
		LOG("%s\tserver\tbind() error",__TIME__);
		return 1;
	}

	if(listen(sfd,10)<0)
	{
		LOG("%s\tserver\tlisten() error",__TIME__);
		return 1;
	}

	FD_ZERO(&fds);
	FD_ZERO(&sfds);

	FD_SET(sfd,&fds);

	max_fd=sfd;

	pthread_t rthread,cthread;

	pthread_create(&rthread,NULL,room_thread,&sfd);
	pthread_create(&cthread,NULL,control_thread,NULL);

	pthread_t jthread;
	pthread_create(&jthread,NULL,j_flag,NULL);
	pthread_join(jthread,NULL);
	
	pthread_join(rthread,NULL);
	pthread_join(cthread,NULL);
	
	fclose(log_file);
	close(sfd);

	return 0;
}
