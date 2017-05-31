#ifndef __PRINT_UTIL_H__
#define __PRINT_UTIL_H__

#ifdef __Cplusplus
extern .C{
#endif

#ifndef print_log
#define print_log(sparam,arg...) \
do {\
    printf("[%s:%d %s] "sparam"\n", __FILE__, __LINE__, __FUNCTION__, ##arg);\
}while(0);

#endif

#ifdef __Cplusplus
}
#endif

#endif