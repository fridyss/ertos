#ifndef _DEBUG_H
#define _DEBUG_H

#include "type.h"
#include "service.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ASSERT(x)	\
do{ \
	if( (x) == 0 )	\
		printf("error :%s, %d \r\n", __FILE__, __LINE__);	\
}while(0) 	



#ifdef __cplusplus
}
#endif

#endif /*_DEBUG_H*/

