#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>  
#include "httpd.h"
#include "mlanguage.h"
#include "mlog.h"

#define HTTP_PORT 80
#define MAX_CONNECT 20

extern int errno;

long start_up(int *httpd)
{
	struct sockaddr_in server;
	socklen_t len;
	int ret;

	if (0 > (*httpd = socket(AF_INET,SOCK_STREAM, 0)))
	{
		printf("create socker err\n");
		return -1;
	} 	
	
    // 获取sockClient1对应的内核接收缓冲区大小  
    int optVal = 0;  
    int optLen = sizeof(optVal);  
    getsockopt(*httpd, SOL_SOCKET, SO_RCVBUF, (char*)&optVal, &optLen);  
    printf("sockClient1, recv buf is %d\n", optVal); // 8192 
    
    //optVal = 0;  
    //setsockopt(*httpd, SOL_SOCKET, SO_RCVBUF, (char*)&optVal, optLen); 
    
    optVal = 1;
    ret = setsockopt( *httpd, SOL_SOCKET, SO_REUSEADDR, (char*)&optVal, sizeof(optVal) );
    
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;//300*1000;

    //接受时限
    setsockopt(*httpd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout,sizeof(timeout));
    
    bzero(&server,sizeof server);
	server.sin_family = AF_INET;
	server.sin_port = htons(HTTP_PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
	len = sizeof(server);

	if (-1 == bind(*httpd,(struct sockaddr *)&server,len))
	{
		printf("bind err\n");
		goto fail_label;
	}

	if (-1 == listen(*httpd,MAX_CONNECT))
	{
		printf("listen err\n");
		goto fail_label;
	}
 	
	ret = 0;
    return ret;
fail_label:
	close(*httpd);
	ret = -1;
	return ret;	
}

struct request *request_parse(char *buffer, long len)
{
    struct request *req;
    char *p = buffer;
    //int len;
    int i;
    
    if (NULL == (req = malloc(sizeof *req)))
    {
        printf("malloc failed\n");
        return NULL;
    }
    
    sscanf(p, "%s%n", req->methon, &i);
    if (i > 7
        || i <= 0) 
    {
        printf("get methon err\n");
        goto fail;
    }
    req->methon[i] = 0;
    //printf("%s \n",req->methon);
    
    p += i;
    p ++;
    sscanf(p, "%s%n", req->url, &i);
    if (i > 200
        || i <= 0) 
    {
        printf("get url err\n");
        goto fail;
    }
    req->url[i] = 0;
    //printf("%s \n",req->url);
    
    for (i = 0; i < len; ++i)
    {
        printf("%c",buffer[i]);
    }
    fflush(stdout);        
    return req;
fail:
    free(req);
    return NULL;
}

long execute_cgi(char *url, int client)
{
    char c,buf[100];
    int i;
    
    //sprintf(buf, "HTTP/1.0 200 OK\r\n");
    //send(client, buf, strlen(buf), 0);
 
    /*for (i = 0; i < 11; i++) {
        recv(client, &c, 1, 0);
        printf("%c",c);
        //write(cgi_input[1], &c, 1);
    }
    
    i = recv(client, buf, 100, 0);
    if (i > 0)
        printf("%s",buf);*/
        
    return 0; 
}

long send_file(char *url, int client) 
{
    FILE *fp;
    char buffer[1024], *p = NULL;
    int len;
    
    if (access(url, F_OK) != F_OK)
    {
        send(client, "HTTP/1.1 404 OK\r\n", strlen("HTTP/1.1 404 OK\r\n"), 0); 
        return 0;   
    }
    
    int filefd = open(url, O_RDONLY);
    
    if (NULL == (fp = fdopen(filefd,"rb")))
    {
        mlog("err fail when open file", NULL);
        return -1;
    }
    
    fseek(fp, 0, SEEK_END);  
    len = ftell(fp);
    rewind(fp);
    
    send(client, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"), 0);
    send(client, "Date: Wed May 24 09:43:45 CST 2017\r\n", strlen("Date: Wed May 24 09:43:45 CST 2017\r\n"), 0);
    
    if (NULL == (p = strrchr(url, '.')))
    {
        printf("err fail when get ends\n");
        fclose(fp);
        return -1;    
    }
        
    if (strcmp(p, ".htm") == 0
        || strcmp(p, ".html") == 0)
    {
        send(client, "Content-type: text/html\r\n", strlen("Content-type: text/html\r\n"), 0);
    }
    else if (strcmp(p, ".ico") == 0)
    {
        send(client, "Content-type: img/ico\r\n", strlen("Content-type: img/ico\r\n"), 0);
    }
    else if (strcmp(p, ".js") == 0) 
    {
        send(client, "Content-type: application/x-javascript\r\n", strlen("Content-type: application/x-javascript\r\n"), 0);
    }   
    
    sprintf(buffer, "Content-length: %d\r\n\r\n", len);
    send(client, buffer, strlen(buffer), 0);
#if 0    
    while ((len = fread(buffer, 1, 1024, fp)) > 0)
    {
        send(client, buffer, len, 0); 
    }
#else
    int ret, left = len;
    mlog("file fd : %d\n", filefd);
    while (left)
    {
        ret = sendfile(client, filefd, NULL, len);
        if (ret == 0)
        {
            mlog("err : %s ---------", strerror(errno));        
        }
        mlog("err : %s ---------", strerror(errno));       
        left -= ret;
    }
        
    mlog("ret : %d, len : %d-----------", ret, len);
#endif    
    fclose(fp);
    return 0;      
}

long do_request(struct request *req, int client)
{
    int cgi = 0;
    
    if (NULL == req)
    {
        printf("request is null\n");
        return -1;
    }
    
    if (memcmp(req->methon, "GET", 3) == 0)
    {
        if (strcmp(req->url, "/") == 0)
        {
            memcpy(req->url, "/index.htm", 15);
        }
    }
    else if (memcmp(req->methon, "POST", 4) == 0)
    {
        cgi = 1;
    }
    
    if (cgi)
    {
        execute_cgi(req->url, client);    
    }
    else
    {
        send_file(req->url, client);
    }
        
    free(req);
    return 0;
} 

void *client_thread(void *args)
{
	char buffer[1024] = {0};
	int len;
	int client = *(int *)args;
    
    pthread_detach(pthread_self());
	while(1)
    {
        len = recv(client, buffer, sizeof(buffer),0);
        if (len > 0)
        {   
            //struct request * req = request_parse(buffer, len);
            do_request(request_parse(buffer, len), client);         
            //break;
        }
        else if (len == 0)
        {
            mlog("client socket close");
            break;
        } 
        else
        {
            mlog("errno : %s",strerror(errno));
            break;
        }
    }
	
	close(client);
}

void httpd_destory(int status, void *arg)
{
    int httpd = *(int *)arg;
    
    close(httpd);    
}

int main(int argc,char ** args)
{
    int httpd,client;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	pthread_t client_id;
    
    //set_log_file("./log.txt");
    chroot("./web/");
    on_exit(httpd_destory, (void *)&httpd);

	if (start_up(&httpd))
	{
		printf("start up server err\n");
		return -1;
	}
	
	bzero(&client_addr,sizeof client_addr);
	while (1)
	{
		if (0 > (client = accept(httpd,(struct sockaddr *)&client_addr,&len)))
		{
			//printf("accept err\n");
			//goto fail_label;
			continue;
		}
		else
		{ 
		    mlog("accept client socket : %d",client); 
		    //printf("",client_addr.);   
		}
		
		if (pthread_create(&client_id,NULL,client_thread,&client))
		{
			printf("create thread err\n");
			continue;
		}
	}
	
fail_label:
	close(httpd);
	return 0;
}
