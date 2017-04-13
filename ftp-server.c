#include <sys/types.h>			//定义基本系统数据类型
#include <sys/socket.h>			//套接字接口

#include <netinet/in.h>			//INTERNET地址族
#include <netdb.h>				//网络数据库操作

#include <sys/stat.h>			//文件状态
#include <sys/fcntl.h>			//文件控制
#include <unistd.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>			//INTERNET定义
#include <dirent.h>				//for DIR

#define MAX_LINE 1024			//缓冲区上限

char currentDirPath[200];                 //当前工作目录的绝对路径
char currentDirName[30];                  //当前目录的名称
char helpMessage[]="get:\tdownload a file from server\nput:\tupload a file to server\npwd:\tdisplay the current directory of server\ndir:\tdisplay the files in the current directory of server\ncd:\tchange the directory of server\n?:\tdisplay the whole command which equals 'help'\nquit:\treturn\n";
char cdempty[]="error:no such directory!";

/*---------------------------------函数声明---------------------------------------*/
char *getDirName(char *dirPathName);			//根据当前工作目录的绝对路径得到当前目录名称
void handle_get(int sock,char*fileName);		//处理get命令，获取远方的一个文件；
void handle_put(int sock,char *fileName);		//处理put命令，传送给远方一个文件；
void handle_pwd(int sock);				//处理pwd命令，显示远方的当前目录；
void handle_dir(int sock);                          	//处理dir命令，列出远方当前目录下的内容；
void handle_cd(int sock,char *dirName);             	//处理cd命令，改变远方的当前目录；
void handle_help(int sock);                         	//处理?命令，显示可以提供的命令，即help；


/*---------------------------------函数定义---------------------------------------*/
/*
*getDirName 得到当前工作目录名称
*参数：dirPathName 当前工作目录的绝对地址
*/
char *getDirName(char *dirPathName){
	int i, pos, len;
	char *dirName;			//当前工作目录名称
	if(dirPathName == NULL){
		printf("absoult directory path is NULL.\n");
		return NULL;
	}	
	
	len=strlen(dirPathName);
	for(i = len - 1;i >= 0;i --){    //找到最后一个/号，之后的字符串即为当前工作目录名称
		if(dirPathName[i] == '/'){  
			pos=i;
			break;
		}
	}
	
	dirName = (char *)malloc(len - pos + 1);
	for(i = pos + 1;i <= len;i ++)
		dirName[i - pos - 1] = dirPathName[i];	
	
	return dirName;
}

void handle_pwd(int sock){
	int len; 
	char *savePointer = getDirName(currentDirPath); //得到当前工作目录名称
	strcpy(currentDirName,savePointer);	
	len = strlen(currentDirName) + 1;
	write(sock,currentDirName,len);                 //发到客户端
}

void handle_dir(int sock){

	DIR *pdir; 
	char fileMsg[50];
	int i, fileCnt = 0, len;
	struct dirent *pent;
	struct stat fileStat;
	char filePath[200];
	
	pdir = opendir(currentDirPath);				//打开目录句柄。如果opendir成功运行，将返回一组目录流（一组目录字符串）
	
	while((pent = readdir(pdir))!= NULL){		//计算文件及目录数。readdir用来读取目录，返回是dirent结构体指针
		fileCnt ++;
	}

	write(sock,&fileCnt,sizeof(int));
	closedir(pdir);

	if(fileCnt <= 0)
		return;
	else{
		pdir=opendir(currentDirPath);
		for(i = 0; i < fileCnt;i ++){
			pent = readdir(pdir);
			memset(fileMsg,0,sizeof(fileMsg));
			char *fileName = pent->d_name;
			
			memset(filePath,0,sizeof(filePath));
			strcpy(filePath,currentDirPath);
			strcat(filePath,"/");
			strcat(filePath, fileName);
			int op = open(filePath, O_RDONLY, S_IREAD);
			
			fstat(op,&fileStat);			//fstat 返回关于打开文件的信息
			if(S_ISDIR(fileStat.st_mode)){	//检查是否为目录
				strcat(fileMsg,"dir\t");
				strcat(fileMsg,fileName);
			}
			else
			{	
				strcat(fileMsg,"file\t");
				strcat(fileMsg,fileName);
			}
			write(sock,fileMsg,sizeof(fileMsg));
		}
		closedir(pdir);
	}	
}

void handle_cd(int sock,char *dirName){
	DIR *pdir;
	struct dirent *pent;
	char filename[30];
	int i,fileCnt=0;
	int flag=0;

	pdir=opendir(currentDirPath);
	pent=readdir(pdir);
	while(pent!=NULL){
		fileCnt++;
		pent=readdir(pdir);
	}
	closedir(pdir);

	if(fileCnt<=0)
		return;
	else{
		pdir=opendir(currentDirPath);
		for(i=0;i<fileCnt;i++){
			pent=readdir(pdir);
			
			if(strcmp(pent->d_name,dirName)==0){
				printf("pent->type:%d",pent->d_type);
				if(pent->d_type!=4){
					continue;				
				}
				if(strcmp(dirName,"..") == 0) {
					int i,pos = 0;
					int len=strlen(currentDirPath);
					for(i = len - 1;i >= 0;i --){
						if(currentDirPath[i] == '/'){
							pos=i;
							break;
						}
					}
					currentDirPath[pos]='\0';
					
				}
				else{	
					strcat(currentDirPath,"/");
					strcat(currentDirPath,dirName);
				}
				char *savePointer = getDirName(currentDirPath);
				strcpy(currentDirName,savePointer);
				flag=1;
				break;
			}
		}
		if(flag==1){
			write(sock,currentDirPath,sizeof(currentDirPath));
                }else{
			write(sock,cdempty,sizeof(cdempty));
		}
		closedir(pdir);
	}
}

void handle_help(int sock){
	int len=strlen(helpMessage)+1;
	write(sock,helpMessage,len);
}

void handle_get(int sock,char*fileName){
	struct stat fileStat;
	long fileSize;
	char filePath[200], buff[MAX_LINE];
	
	memset(filePath,0,sizeof(filePath));//内存空间初始化
	strcpy(filePath,currentDirPath);
	strcat(filePath,"/");
	strcat(filePath,fileName);//获取文件的真实路径

	int op=open(filePath,O_RDONLY,S_IREAD);//只读模式，可读取权限
	if(op != -1){//打开成功
		fstat(op,&fileStat);			//fstat 返回关于打开文件的信息
		fileSize=(long) fileStat.st_size;
		write(sock,&fileSize,sizeof(long));
		memset(buff,0,MAX_LINE);
		while(fileSize > MAX_LINE){
			read(op,buff,MAX_LINE);
			write(sock,buff,MAX_LINE);
			fileSize=fileSize-MAX_LINE;
		}
		
		read(op,buff,fileSize);
		write(sock,buff,fileSize);
		close(op);
		printf("'get' completed.\n");
	}
	else //打开失败
		printf("open file %s failed.\n",filePath);
}

void handle_put(int sock,char *fileName){	

	long fileSize;
	
	char filePath[200], buff[MAX_LINE];
	
	//memset(filePath,0,sizeof(filePath));//内存空间初始化
	strcpy(filePath,currentDirPath);
	strcat(filePath,"/");
	strcat(filePath,fileName);
	
	//打开文件，参见“Linux C语言中open函数”，头文件：sys/types.h,sys/stat.h,fcntl.h
	//O_RDWR 以可读写方式打开文件，O_CREAT 若不存在则建立该文件；S_IREAD(S_IWRITE)，该文件所有者具有可读(写)的权限
	int op=open(filePath,O_RDWR|O_CREAT, S_IREAD|S_IWRITE);
	if(op!=-1){	
		memset(buff,0,MAX_LINE);
		read(sock,&fileSize,sizeof(long));
		while(fileSize>MAX_LINE){
			read(sock,buff,MAX_LINE);
			write(op,buff,MAX_LINE);
			fileSize=fileSize-MAX_LINE;
		}
		read(sock,buff,fileSize);
		write(op,buff,fileSize);
		close(op);
		printf("'put' completed.\n");
	}
	else
		printf("open file %s failed.\n",filePath);
}

/*---------------------------------main函数---------------------------------------*/

int main(int argc,char *argv[]){
	int sock;                               //服务器用于监听的数据通道
	int sockmsg;                            //服务器用于监听的命令通道
  
	char client_cmd[10];                    //客户端出发的命令
	char cmd_arg[20];                       //客户端输入的文件或目录名，用在cd,put,get命令中
	struct sockaddr_in server;              //服务器数据通道的信息
	struct sockaddr_in servermsg;           //服务器命令通道的信息
	int datasock;                           //用于通信的数据通道
	int msgsock;                            //用于通信的命令通道
	pid_t child;                            //client子进程

    sock=socket(AF_INET,SOCK_STREAM,0);       	//创建用于传数据的套接字   
	sockmsg=socket(AF_INET,SOCK_STREAM,0);	//创建用于传消息的套接字   
	int opt = 1,opt2 = 1;
	setsockopt(sock , SOL_SOCKET , SO_REUSEADDR , &opt , sizeof(opt));		//实现sock的重用
	setsockopt(sockmsg , SOL_SOCKET , SO_REUSEADDR , &opt2 , sizeof(opt2));	//实现sockmsg的重用

	if (sock < 0 || sockmsg < 0){                                             	//socket创建失败
		perror("opening stream socket\n");
		exit(1);
	}

	memset(&server,0,sizeof(server));
	server.sin_family=AF_INET;						//设置协议族
	server.sin_addr.s_addr=htonl(INADDR_ANY);				//监听所有地址
	server.sin_port=htons(45000);						//端口设为45000

	memset(&servermsg,0,sizeof(servermsg));
	servermsg.sin_family=AF_INET;						//设置协议族
	servermsg.sin_addr.s_addr=htonl(INADDR_ANY);				//监听所有地址
	servermsg.sin_port=htons(45001);                                        //端口设为45001
   
	if (bind(sock,(struct sockaddr *) &server,sizeof(server)) < 0 || bind(sockmsg,(struct sockaddr *) &servermsg,sizeof(servermsg)) < 0){
		perror("binding stream socket\n");
		exit(1);
	}
  
	int length = sizeof(server);
	int lengthmsg = sizeof(servermsg);

	if (getsockname(sock,(struct sockaddr *) &server,&length) < 0 || getsockname(sockmsg,(struct sockaddr *) &servermsg,&lengthmsg) < 0){//得到套接字的本地名字
		perror("getting socket name\n");
		exit(1);
	}

	printf("Socket port # %d  %d\n",ntohs(server.sin_port),ntohs(servermsg.sin_port));

	memset(currentDirPath,0,sizeof(currentDirPath));
	getcwd(currentDirPath,sizeof(currentDirPath));				//将当前工作目录的绝对路径复制到currentDirPath中

	listen(sock,2);								//监听数据通道
	listen(sockmsg,2);                                     			//监听命令通道
	while(1){
		datasock = accept(sock,(struct sockaddr*)0,(int*)0);      	//与客户端的数据通道连接
		msgsock = accept(sockmsg,(struct sockaddr*)0,(int*)0);    	//与客户端的命令通道连接
		if (datasock == -1 || msgsock==-1)
			perror("accept");
		else{
			if((child = fork()) == -1)                            	//出错
				printf("Fork Error!\n");
			if(child == 0){                                       	//子进程
				printf("connection accepted! new client.\n");
				write(datasock,helpMessage,strlen(helpMessage) + 1);
				
				while(1){
					memset(client_cmd,0,sizeof(client_cmd));
					int rval = 0;
					rval = read(msgsock,client_cmd,sizeof(client_cmd));//读命令
					printf("%d\n",rval);
					if(rval < 0)
						perror("reading command failed.\n");
					else if(rval == 0){                             //连接关闭
						printf("connection closed.\n");
						close(datasock);
						close(msgsock);
					}
					else{
						if(strcmp(client_cmd,"pwd") == 0){      	//为pwd命令
							printf("command 'pwd'\n");
							handle_pwd(datasock);
							printf("done.\n\n");
						}
						else if(strcmp(client_cmd,"dir") == 0){		//为dir命令
							printf("command 'dir'\n");
							handle_dir(datasock);
							printf("done.\n\n");
						}
						else if(strcmp(client_cmd,"cd") == 0){		//为cd命令
							printf("command 'cd'\n");
							memset(cmd_arg,0,sizeof(cmd_arg));
							read(msgsock,cmd_arg,sizeof(cmd_arg));
							handle_cd(datasock,cmd_arg);
							printf("done.\n\n");
						}
						
						else if(strcmp(client_cmd,"get") == 0){		//为get命令
							printf("command 'get'\n");
							memset(cmd_arg,0,sizeof(cmd_arg));
							read(msgsock,cmd_arg,sizeof(cmd_arg));
							handle_get(datasock,cmd_arg);
							printf("done.\n\n");
						}
						else if(strcmp(client_cmd,"put") == 0){		//为put命令
							printf("command 'put'\n");
							memset(cmd_arg,0,sizeof(cmd_arg));
							read(msgsock,cmd_arg,sizeof(cmd_arg));
							handle_put(datasock,cmd_arg);
							printf("done.n\n");
						}
						else if(strcmp(client_cmd,"?") == 0){		//为?命令
							printf("command '?'\n");
							handle_help(datasock);
							printf("done.\n\n");
						}
						else if(strcmp(client_cmd,"quit") == 0){	//为quit命令
							printf("command 'quit'\n");
							break;
						}
						else {                                          //非法输入
							printf("illegal command!\n");
						}
					}
				}//while(1)
				//用户退出
				printf("connection closed.\n");
				close(datasock);
				close(msgsock);
				exit(1);
			}//if(child == 0)
		}//else
	}//while(1)
	exit(0);
}
