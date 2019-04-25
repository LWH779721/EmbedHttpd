#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>  
#include <sys/epoll.h>
#include <signal.h>
#include <stdint.h>

#include "mnet.h"
#include "httpd.h"
#include "mlanguage.h"
#include "mlog.h"

#define HTTP_PORT 8080
#define MAX_CONNECT 20

#define SERVER_STRING "Server: httpd/0.1.0\r\n"

int main(int argc, char **args)
{
    int httpd,client;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	int epfd,nfds, i, fd, n, nread;
    struct epoll_event ev,events[MAX_CONNECT];
    char buf[2048];
    int ret = -1;
    
    signal(SIGPIPE, SIG_IGN);
    //set_log_file("./log.txt");
    
    ret = chroot("web");
    if (ret)
    {
        perror("chroot err");
		//return -1;    
    }
    
    on_exit(httpd_destory, (void *)&httpd);
    bzero(&client_addr,sizeof client_addr);
    	
	if (0 > (httpd = mnet_tcp_server(HTTP_PORT)))
	{
		printf("start up server err\n");
		return -1;
	}
       
    epfd = epoll_create(MAX_CONNECT);
    if (epfd == -1)
    {
        printf("err when epoll create\n");
        return -1;
    }
   
    ev.data.fd = httpd;
    ev.events = EPOLLIN|EPOLLET;//监听读状态同时设置ET模式
    epoll_ctl(epfd, EPOLL_CTL_ADD, httpd, &ev);//注册epoll事件
    while(1)
    {
        nfds = epoll_wait(epfd, events, MAX_CONNECT, -1);
        if (nfds == -1)
        {
            printf("failed when epoll wait\n");
            continue;
        }
                
        for (i = 0; i < nfds; i++)
        {
            fd = events[i].data.fd;
            if (fd == httpd)
            {
                while ((client = accept(httpd, (struct sockaddr *) &client_addr, (size_t *)&len)) > 0) 
                {
                    if (mnet_setnoblock(client))
                    {
                        mlog_err("failed when set client nonlock");
                        continue;
                    }
                    
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = client;
                    
                    if (epoll_ctl(epfd, EPOLL_CTL_ADD, client, &ev) == -1)
                    {
                        perror("epoll add client err");
                        continue;
                    }
                    
                    mlog_info("accept fd : %d", client);
                }
                
                if (errno != EAGAIN && errno != ECONNABORTED 
                                && errno != EPROTO && errno != EINTR) 
                                    perror("accept");
                
                continue;
            }
            
            if (events[i].events & EPOLLIN) 
            {                            
                n = 0;
                while ((nread = read(fd, buf + n, BUFSIZ-1)) > 0) //ET下可以读就一直读
                {
                    n += nread;
                }
                
                if (n == 0)
                {
                    mlog_err("client: %d request over", fd);
                    close(fd);
                    continue;
                }
                
                buf[n] = 0;        
                if (nread == -1 && errno != EAGAIN) 
                {
                    perror("read error");
                }
                        
                mlog_info("read fd : %d, req: %s", fd, buf);

                //recv_dump(fd);              
                
                    #if 0    
                        ev.data.fd = fd;
                        ev.events = events[i].events | EPOLLOUT; //MOD OUT 
                        if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) == -1) {
                                perror("epoll_ctl: mod");
                        }
                   #else
                       do_request(request_parse(buf, len), fd); 
                   #endif     
            }          
        }
    }
    
fail_label:
	close(httpd);
	return 0;
}