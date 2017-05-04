#Socket 编程
##基本TCP套接字编程
###socket函数
```
#include <sys/socket.h>
int socket(int family, int type, int protocol)
成功返回非负描述符，出错则为-1.
```
###connect函数
```
#include <sys/socket.h>
int connect(int sockfd, const struct sockaddr *servaddr, soclan_t addrlen)
成功返回0，出错则为-1.
```
###accept函数
```
#include<sys/socket.h>
int accept(int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen)
成功返回非负描述符，师范返回-1
```
##iocrl函数
获取所在主机全部网络接口的信息
```
#include <unistd.h>
int ioctl (int fd, int request, ... /* void *arg */);
返回：成功则为0，若出错则为-1。
```
第三个参数的数据类型取决于 request的值。
类别  | request  | 说明 | 数据类型
-----|----------|------|---------
文件  | FIONREAD |获取接收缓冲区中的字节数 | int

##地址转换函数
###inet_pton函数和inet_ntop函数
p 和 n 分别表示**表达**(presentation)和**数值**(number)。
```
#include <arpa/inet.h>
int inet_pton(int family, const char *strptr, void *addrptr);
			成功返回1，输入不是有效格式返回0，出错返回-1。
const char *inet_ntop(int family, const void *addrptr, char *strptr, size_t len);
			成功返回指向结果的指针，出错则为NULL。
```