#ifndef __ERRNO_H
#define __ERRNO_H

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ECOULD_NOT_ALLOCATE_REQUIRED_MEMORY		( -1 )
#define	EQUEUE_BLOCKED							( -4 )
#define EQUEUE_YIELD							( -5 )
#define ECOULD_NOT_FIND_PARAMETERS				( -6 )
#define EINVALID_PARAMETERS						( -7 )
#define EQUEUE_FULL								( -8 )
#define EQUEUE_EMPTY							( -9 )
#define EOK                          			(  0 )               
#define EERR                        			(  1 )               
#define ETIMEOUT                     			(  2 )              
#define EFULL                       			(  3 )              
#define EEMPTY                       			(  4 )              
#define ENOMEM                       			(  5 )              
#define ENOSYS                                  (  6 )              
#define EBUSY                       	 		(  7 )              
#define EIO                         			(  8 )              
#define EINTR                        			(  9 )              
#define EINVAL                       			(  10)              


#ifdef __cplusplus
}
#endif


#endif /*__ERRNO_H*/
