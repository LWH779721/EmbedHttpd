#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

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

void * client_thread(void *args)
{
	char buffer[1024];
	int len;
	int client = *(int *)args;

	while(1)
        {
                memset(buffer,0,1024);
                len = recv(client, buffer, sizeof(buffer),0);
                if(strcmp(buffer,"exit\n")==0)
                        break;
                fputs(buffer, stdout);
                send(client, buffer, len, 0);
        }
	
	close(client);
}

int main(int argc,char ** args)
{
	int httpd,client;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	pthread_t client_id;

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
			goto fail_label;
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
