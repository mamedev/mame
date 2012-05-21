/* Sort.c -- Sort functions
2010-09-17 : Igor Pavlov : Public domain */

#include "Sort.h"

#define HeapSortDown(p, k, size, temp) \
  { for (;;) { \
    UInt32 s = (k << 1); \
    if (s > size) break; \
    if (s < size && p[s + 1] > p[s]) s++; \
    if (temp >= p[s]) break; \
    p[k] = p[s]; k = s; \
  } p[k] = temp; }

void HeapSort(UInt32 *p, UInt32 size)
{
  if (size <= 1)
    return;
  p--;
  {
    UInt32 i = size / 2;
    do
    {
      UInt32 temp = p[i];
      UInt32 k = i;
      HeapSortDown(p, k, size, temp)
    }
    while (--i != 0);
  }
  /*
  do
  {
    UInt32 k = 1;
    UInt32 temp = p[size];
    p[size--] = p[1];
    HeapSortDown(p, k, size, temp)
  }
  while (size > 1);
  */
  while (size > 3)
  {
    UInt32 temp = p[size];
    UInt32 k = (p[3] > p[2]) ? 3 : 2;
    p[size--] = p[1];
    p[1] = p[k];
    HeapSortDown(p, k, size, temp)
  }
  {
    UInt32 temp = p[size];
    p[size] = p[1];
    if (size > 2 && p[2] < temp)
    {
      p[1] = p[2];
      p[2] = temp;
    }
    else
      p[1] = temp;
  }
}

/*
#define HeapSortRefDown(p, vals, n, size, temp) \
  { UInt32 k = n; UInt32 val = vals[temp]; for (;;) { \
    UInt32 s = (k << 1); \
    if (s > size) break; \
    if (s < size && vals[p[s + 1]] > vals[p[s]]) s++; \
    if (val >= vals[p[s]]) break; \
    p[k] = p[s]; k = s; \
  } p[k] = temp; }

void HeapSortRef(UInt32 *p, UInt32 *vals, UInt32 size)
{
  if (size <= 1)
    return;
  p--;
  {
    UInt32 i = size / 2;
    do
    {
      UInt32 temp = p[i];
      HeapSortRefDown(p, vals, i, size, temp);
    }
    while (--i != 0);
  }
  do
  {
    UInt32 temp = p[size];
    p[size--] = p[1];
    HeapSortRefDown(p, vals, 1, size, temp);
  }
  while (size > 1);
}
*/
