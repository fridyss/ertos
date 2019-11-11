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


#ifdef __cplusplus
}
#endif


#endif /*__ERRNO_H*/
