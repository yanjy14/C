#include <arpa/inet.h>
struct Msg
{
	int length;		//长度
	char type;		//类型
	char data[4096];	//数据段
};
typedef struct User_Info_Table 		//客户信息表
{
	int fd; 						//套接字描述符
	int count;						//标志位
	unsigned long int secs;			//上次收到keeplive时间
	struct in_addr v4addr;			//服务器给客户端分配的IPV4地址
	struct in6_addr v6addr;			//客户端的IPV6地址
	struct User_Info_Table * pNext;	//链表下一个节点
} User_Info_Table;
struct IPADDR
{
	char addr[32];		//IP地址
	int status;			//标志位
};
struct IPADDR ipaddr[128];//全局变量的地址池