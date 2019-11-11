#ifndef __LOG2_H
#define __LOG2_H

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif 


#define log2(n)   \
(	\
	(n) < 1 ? 0 : \
	(n) & (1ul << 15) ? (15) : \
	(n) & (1ul << 14) ? (14) : \
	(n) & (1ul << 13) ? (13) : \
	(n) & (1ul << 12) ? (12) : \
	(n) & (1ul << 11) ? (11) : \
	(n) & (1ul << 10) ? (10) : \
	(n) & (1ul <<  9) ? ( 9) : \
	(n) & (1ul <<  8) ? ( 8) : \
	(n) & (1ul <<  7) ? ( 7) : \
	(n) & (1ul <<  6) ? ( 6) : \
	(n) & (1ul <<  5) ? ( 5) : \
	(n) & (1ul <<  4) ? ( 4) : \
	(n) & (1ul <<  3) ? ( 3) : \
	(n) & (1ul <<  2) ? ( 2) : \
	(n) & (1ul <<  1) ? ( 1) : \
	(n) & (1ul <<  0) ? ( 0) : \
     0 \
)


#define roundup_pow_of_two(n)  \
(	\
		(n == 1) ? (1) : (1ul << (log2( (n) - 1 ) + 1))		\
)

#define rounddown_pow_of_two(n) \
( \
		(n == 1) ? (1) : (1ul << ( log2( (n) ) ))	\
)


#ifdef __cplusplus
}
#endif

#endif /*__LOG2_H*/

