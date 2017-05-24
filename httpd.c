#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>

#define HTTP_PORT 8888
#define MAX_CONNECT 20

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
    
    optVal = 0;  
    setsockopt(*httpd, SOL_SOCKET, SO_RCVBUF, (char*)&optVal, optLen); 
    
    getsockopt(*httpd, SOL_SOCKET, SO_RCVBUF, (char*)&optVal, &optLen);  
    printf("sockClient1, recv buf is %d\n", optVal); // 8192 
    
    /*struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 300*1000;

    //接受时限
    setsockopt(*httpd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout,sizeof(timeout));*/
    
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
	close(httpd);
	ret = -1;
	return ret;	
}

long send_ico(int client) 
{
    FILE *fp;
    char buffer[1024];
    int len;
    
    if (NULL == (fp = fopen("./favicon.ico","rb")))
    {
        printf("err fail when open file");
        return -1;
    }
       
    send(client, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"), 0);
    send(client, "Date: Wed May 24 09:43:45 CST 2017\r\n", strlen("Date: Wed May 24 09:43:45 CST 2017\r\n"), 0);
    send(client, "Content-type: image/ico\r\n", strlen("Content-type: image/ico\r\n"), 0);
    send(client, "Content-length: 5686\r\n\r\n", strlen("Content-length: 5686\r\n\r\n"), 0);
    while ((len = fread(buffer, 1, 1024, fp)) > 0)
    {
        printf("len : %d\n",len);
        send(client, buffer, len, 0); 
    }
    
    fclose(fp);
    return 0;       
}

long send_html(int client) 
{
    /*FILE *fp;
    char buffer[1024];
    int len;
    
    if (NULL == (fp = fopen("./web/a.htm","rb")))
    {
        printf("err fail when open file");
        return -1;
    }*/
       
    send(client, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"), 0);
    send(client, "Date: Wed May 24 09:43:45 CST 2017\r\n", strlen("Date: Wed May 24 09:43:45 CST 2017\r\n"), 0);
    send(client, "Content-type: text/html\r\n", strlen("Content-type: text/html\r\n"), 0);
    send(client, "Content-length: 30\r\n\r\n", strlen("Content-length: 30\r\n\r\n"), 0);
    send(client, "<html><body>test</body></html>", strlen("<html><body>test</body></html>"), 0);
    /*while ((len = fread(buffer, 1, 1024, fp)) > 0)
    {
        printf("len : %d\n",len);
        send(client, buffer, len, 0); 
    }*/
    
    //fclose(fp);
    return 0;       
}

void *client_thread(void *args)
{
	char buffer[1024];
	int len,i;
	int client = *(int *)args;

	while(1)
    {
        memset(buffer,0,1024);
        len = recv(client, buffer, sizeof(buffer),0);
        //if(strcmp(buffer,"exit")==0)
        //    break;
        if (len > 0)
        {    
            printf("len : %d\n",len);
            for (i = 0; i < len; i++)
            {
                if (isprint(buffer[i]))
                {    
                    printf("%c",buffer[i]);
                } 
                else
                {
                    printf("\n%d",buffer[i]);        
                }   
            }    
            fflush(stdout);  
            
            if (strstr(buffer, "favicon.ico"))
            {
                send_ico(client);        
            }
            else
            {
                send_html(client);  
            }
            
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
    
    on_exit(httpd_destory, (void *)&httpd);

	if (start_up(&httpd))
	{
		printf("start up server err\n");
		return -1;
	}
	
	while (1)
	{
		if (0 > (client = accept(httpd,(struct sockaddr *)&client_addr,&len)))
		{
			printf("accept err\n");
			//goto fail_label;
			continue;
		}
		else
		{
		    printf("accept client\n");    
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
