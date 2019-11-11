#ifndef __CONDEF_H
#define __CONDEF_H

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define stack_type_t   uint32_t
//#define tick_type_t	  uint32_t
typedef uint32_t tick_type_t;

#define TRUE							( 1u )		
#define FALSE							( 0u )

#define MAX_24_BIT_NUMBER 				( 0xfffffful )

#ifdef __cplusplus
}
#endif


#endif /*__CONDEF_H*/

