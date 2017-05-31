#ifndef __HTTPD_H__
#define __HTTPD_H__

#ifdef __Cplusplus
extern .C{
#endif

struct request
{
    char methon[5];
    char url[200];  
};

#ifdef __Cplusplus
}
#endif

#endif