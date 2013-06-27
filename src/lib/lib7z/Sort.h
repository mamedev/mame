/* Sort.h -- Sort functions
2009-02-07 : Igor Pavlov : Public domain */

#ifndef __7Z_SORT_H
#define __7Z_SORT_H

#include "Types.h"

#ifdef __cplusplus
extern "C" {
#endif

void HeapSort(UInt32 *p, UInt32 size);
/* void HeapSortRef(UInt32 *p, UInt32 *vals, UInt32 size); */

#ifdef __cplusplus
}
#endif

#endif
