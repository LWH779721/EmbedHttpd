#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h> 

int mnet_setnoblock(int fd)
{
    int opts;
    
    opts = fcntl(fd, F_GETFL);
    if (opts < 0) {
        perror("fcntl(F_GETFL)");
        return -1;
    }
    
    opts = (opts | O_NONBLOCK);
    if (fcntl(fd, F_SETFL, opts) < 0) {
        perror("fcntl(F_SETFL)");
        return -1;
    } 
    
    return 0;       
}

int mnet_check_connectble(char *ipaddr, int port, int timeout)
{
    struct sockaddr_in remote;
    struct timeval timeo = {3, 0};
    socklen_t len;
    int client, ret = -1;
    
    if (0 > (client = socket(AF_INET, SOCK_STREAM, 0)))
	{
		printf("create socker err\n");
		return -1;
	} 
	
	if (timeout)
	{
	    timeo.tv_sec = timeout;    
	}
	
	setsockopt(client, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof timeo);     
	memset(&remote, 0, sizeof remote);
	remote.sin_family = AF_INET;
	remote.sin_port = htons(port);
    remote.sin_addr.s_addr = inet_addr(ipaddr);
	len = sizeof(remote);

	if (0 > connect(client,(struct sockaddr *)&remote, len))
	{	
	    perror("connect err");
		goto fail_label;
	}
	
	ret = 0;
fail_label:
    close(client);	
	return ret;
}

long mnet_tcp_server(int port)
{
	struct sockaddr_in server;
	socklen_t len;
	int server_fd, ret;

	if (0 > (server_fd = socket(AF_INET, SOCK_STREAM, 0)))
	{
		printf("create socker err\n");
		return -1;
	} 	
	
    // 获取sockClient1对应的内核接收缓冲区大小  
    int optVal = 0;  
    int optLen = sizeof(optVal);  
    getsockopt(server_fd, SOL_SOCKET, SO_RCVBUF, (char*)&optVal, &optLen);  
    printf("sockClient1, recv buf is %d\n", optVal); // 8192 
    
    //optVal = 0;  
    //setsockopt(*httpd, SOL_SOCKET, SO_RCVBUF, (char*)&optVal, optLen); 
    
    optVal = 1;
    ret = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&optVal, sizeof(optVal) );
    
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;//300*1000;

    //接受时限
    setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    
    mnet_setnoblock(server_fd);
    
    bzero(&server,sizeof server);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
	len = sizeof(server);

	if (-1 == bind(server_fd,(struct sockaddr *)&server, len))
	{
		printf("bind err\n");
		goto fail_label;
	}

	if (-1 == listen(server_fd, 25))
	{
		printf("listen err\n");
		goto fail_label;
	}
 	
    return server_fd;
fail_label:
	close(server_fd);
	return -1;	
}