#ifndef __MNET_H__
#define __MNET_H__

#ifdef __Cplusplus
extern "C"{
#endif

extern int mnet_check_connectble(char *ipaddr, int port, int timeout);
extern int mnet_setnoblock(int fd);

#ifdef __Cplusplus
}
#endif

#endif