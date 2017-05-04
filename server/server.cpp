#include "server.h"
#include <pthread.h>
#define MAXBUF 1024
//struct IPADDR ipaddr[128];//全局变量的地址池
User_Manager *manager;
fd_set readfds, testfds;//select集合
static void keepalive();
int main(int argc, char const *argv[])
{
	//TODO:创建IPV6套接字，把该套接字加入Select模型字符集;
	int server_sockfd, client_sockfd, tun_fd;
	int server_len, client_len;
	struct sockaddr_in6 server_address;
	struct sockaddr_in6 client_address;
	int result;
	char buff[MAXBUF];

	server_sockfd = socket(AF_INET6, SOCK_STREAM, 0);//服务器监听socket
	memset(&server_address,0,sizeof(server_address));
	server_address.sin6_family = AF_INET6;
	server_address.sin6_addr.s6_addr = htonl(INADDR_ANY);//所有地址
	server_address.sin6_port = htons(9734);

	server_len = sizeof(server_address);
	
	bind(server_sockfd,(struct sockaddr *)&server_address,server_len);
	
	listen(server_sockfd, 5);
	//初始化select集合
	FD_ZERO(&readfds);
	FD_SET(server_sockfd, &readfds);
	//TODO:创建tun虚接口
	tun_fd = socket(AF_INET, SOCK_STREAM, 0);//tun 虚接口
	//TODO:创建客户信息表和地址池
	manager = new User_Manager;
	//TODO:获取服务器DNS地址
	FILE *fp; 
	system("cat /etc/resolv.conf | grep -i nameserver | cut -c 12-30 > dns.txt");
	char *dns;
	if(fp = fopen("dns.txt","r")){
		fscanf(fp,"%s",dns);
	}else{
		perror("open dns.txt failed");
	}
	fclose(fp);
	in_addr server_v4;
	inet_pton(AF_INET, dns, (void *)&server_v4);
	//TODO:创建keeplive线程
	pthread_t ka_id;
	//pthread_create(&ka_id, NULL, &keepalive, NULL);
	//TODO:创建读取虚接口线程
	pthread_t tun_id;
	//pthread_create(&tun_id, NULL, &readtun, NULL);
	//TODO：主进程中while循环中数据处理。
	while(1){
		char ch;
		int fd;
		int nread;
		testfds = readfds;
		printf("server waiting\n");

		result = select(FD_SETSIZE, &testfds, (fd_set *)0,(fd_set *)0,(struct timeval *)0);//非阻塞
		if (result < 1)
		{
			perror("server5");
			exit(1);
		}

		for(fd = 0; fd < FD_SETSIZE; fd++){
			if(FD_ISSET(FD,&testfds)){
				if(fd == server_sockfd){
					//connect request ----- put client into set
					memset(&client_address,0,sizeof(client_address));
					client_len = sizeof(client_address);   
                    client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len); 
                    FD_SET(client_sockfd, &readfds);//将客户端socket加入到集合中  
                    printf("adding client on fd %d/n", client_sockfd);
                    printf("connection from %s, port %d\n", inet_ntop(AF_INET6,&client_address.sin6_addr,buff,sizeof(buff)),ntohs(client_address.sin6_port));
                    // User_Info_Table temp;
                    // temp.init(client_sockfd,client_address);
                    // manager.add_user(temp);
				}else{
					//data request
					ioctl(fd, FIONREAD, &nread)；//获取接收缓冲区的字节数
					if (nread!=0){
						Msg recvmsg
						read(fd,&recvmsg,sizeof(recvmsg));
						printf("%s\n", &recvmsg);
					}


				}
			}
		}
	}
	return 0;
}

static void keepalive(){
	pthread_detach(pthread_self());
	sleep(1000);
	User_Info_Table *current = manager->head;
	while(current!=NULL){
		if((current->count = current->count - 1)==0){
			int fd = current -> fd;

			//通过fd发送104类型消息
			current->count = 20；
		}
		if(current->secs>60){
			manager->delete(current);//回收地址
			int fd = current -> fd;
			close(fd);
            FD_CLR(fd, &readfds);
		}
	}
	return;
} 
