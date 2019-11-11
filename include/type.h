//
//  etype.h
//  ertos
//
//  Created by fridy on 2019/10/13.
//  Copyright © 2019 fridy. All rights reserved.
//

#ifndef _etype_h
#define _etype_h

#include "stdio.h"
#include "stdlib.h"
//#include "limits.h"

#ifdef __cpulsplus
extern "C" {
#endif
    
#define OPEN_ETYPE_TEST
    
#ifdef OPEN_TYPE_TEST
#define TYPE_TEST \
do {    \
printf("int8_t :%lu, int16 :%lu, int32, :%lu int64 :%lu (Byte)\r\n",    \
sizeof(int8_t), sizeof(int16_t),sizeof(int32_t), sizeof(int64_t));  \
}while(0);
#else
#define TYPE_TEST
#endif

//#define NULL      (void *)0

typedef  signed char int8_t;
typedef  unsigned char uint8_t;

typedef  signed short int16_t;
typedef  unsigned short uint16_t;

typedef  signed int int32_t;
typedef  unsigned int uint32_t;

typedef  signed long long int64_t;
typedef  unsigned long long uint64_t;

typedef  signed char int8;
typedef  unsigned char uint8;

typedef  signed short int16;
typedef  unsigned short uint16;

typedef  signed int int32;
typedef  unsigned int uint32;

typedef  signed long long int64;
typedef  unsigned long long uint64;
    
#ifdef __cpulsplus
}
#endif

#endif /* etype_h */
