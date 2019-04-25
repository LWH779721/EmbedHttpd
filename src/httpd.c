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

extern int errno;

http_conf *load_conf()
{
    
}

struct request *request_parse(char *buffer, long len)
{
    struct request *req;
    char *p = buffer;
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
    printf("methon : %s \n",req->methon);
    
    p += i;
    p ++;
    
    snprintf(req->url, "%s", "/home/lwh/workspace/http/web");
    printf("url : %s \n",req->url);
    sscanf(p, "%s%n", req->url+28, &i);
    if (i > 200
        || i <= 0) 
    {
        printf("get url err\n");
        goto fail;
    }
    req->url[28 + i] = 0;
    printf("url : %s \n",req->url);
  
    return req;
fail:
    free(req);
    return NULL;
}

void not_found(int client)
{
    char buf[1024];

    sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, SERVER_STRING);
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type: text/html\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "your request because the resource specified\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "is unavailable or nonexistent.\r\n");
    send(client, buf, strlen(buf), 0);
    sprintf(buf, "</BODY></HTML>\r\n");
    send(client, buf, strlen(buf), 0);
}

long execute_cgi(char *url, int client)
{
    char c,buf[100];
    int i;
    
    if (access(url, F_OK) != F_OK)
    {
        not_found(client);
        return 0;   
    }
    
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    send(client, buf, strlen(buf), 0);
    send(client, "Content-type: text/html\r\n", strlen("Content-type: text/html\r\n"), 0);
    
    sprintf(buf, "Content-length: %d\r\n\r\n", 5);
    send(client, buf, strlen(buf), 0);
    
    strcpy(buf, "hello");
    send(client, buf, strlen(buf), 0);
        
    return 0; 
}

long send_file(char *url, int client) 
{
    char buffer[1024], *p = NULL;
    int len, ret;
    struct stat filestat;
    
    if (access(url, F_OK) != F_OK)
    {
        printf("url not exist: %s\n", url);
        not_found(client); 
        return 0;   
    }
    
    int filefd = open(url, O_RDONLY);
    stat(url, &filestat);
    len = filestat.st_size;
    
    send(client, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"), 0);
    send(client, "Date: Wed May 24 09:43:45 CST 2017\r\n", strlen("Date: Wed May 24 09:43:45 CST 2017\r\n"), 0);
    
    if (NULL == (p = strrchr(url, '.')))
    {
        printf("err fail when get ends\n");
        close(filefd);
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
    
    send(client, "Content-type: text/html\r\n", strlen("Content-type: text/html\r\n"), 0);
    send(client, "Content-Encoding: gzip\r\n", strlen("Content-Encoding: gzip\r\n"), 0);

    sprintf(buffer, "Content-length: %d\r\n\r\n", len);
    send(client, buffer, strlen(buffer), 0);

    while (len > 0)
    {
        ret = sendfile(client, filefd, NULL, len);
        if (ret < 0)
        {
            mlog_err("%s ---------", strerror(errno));        
        }
     
        len -= ret;
    }

    close(filefd);
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
    
    strcat(req->url, ".gz");
    printf("%d, url: %s\n", __LINE__, req->url);
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

void httpd_destory(int status, void *arg)
{
    int httpd = *(int *)arg;
    
    close(httpd);    
}

int recv_dump(int fd)
{
    FILE *fp;
    char buf[1024];
    
    if (NULL == (fp = fdopen(fd, "r+")))
    {
        printf("failed when fdopen\n");
        goto failed_label;
    }
    
    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        mlog_err("--%s", buf);
    }
    
    fclose(fp);
    return 0;
    
failed_label:
    close(fd);
    return -1;
}
