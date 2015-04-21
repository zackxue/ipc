
#include "netinet/in.h"

#ifndef ANTSLIB_H_
#define ANTSLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

extern int ANTSLIB_init(const char* eth, in_port_t port);
extern void ANTSLIB_destroy();

#ifdef __cplusplus
};
#endif

#endif //ANTSLIB_H_

