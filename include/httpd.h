#ifndef __HTTPD_H__
#define __HTTPD_H__

#ifdef __cplusplus
extern .C{
#endif

enum methon{get, post}; 
//enum hv{1.0,1.1,2.0}; // http version

struct request
{
    char methon[5];
    char url[200];    
    //enum methon m;  
    //char *url;
    //enum hv;  
};

typedef struct {
    int port;
} http_conf;

extern void httpd_destory(int status, void *arg);

#ifdef __cplusplus
}
#endif
#endif