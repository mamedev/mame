// SortUtils.cpp

#include "StdAfx.h"

#include "SortUtils.h"
#include "Common/Wildcard.h"

static int CompareStrings(const int *p1, const int *p2, void *param)
{
  const UStringVector &strings = *(const UStringVector *)param;
  return CompareFileNames(strings[*p1], strings[*p2]);
}

void SortFileNames(const UStringVector &strings, CIntVector &indices)
{
  indices.Clear();
  int numItems = strings.Size();
  indices.Reserve(numItems);
  for(int i = 0; i < numItems; i++)
    indices.Add(i);
  indices.Sort(CompareStrings, (void *)&strings);
}
