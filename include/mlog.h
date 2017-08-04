#ifndef __MLOG_H__
#define __MLOG_H__

#ifdef __Cplusplus
extern "C"{
#endif

#ifndef mlog
#define mlog(sparam,arg...) \
do {\
    printf("[%s %s:%d %s] "sparam"\n", mlog_time(), __FILE__, __LINE__, __FUNCTION__, ##arg);\
}while(0);
#endif

extern long mlog_set_ofile(char *file);

#ifdef __Cplusplus
}
#endif
#endif