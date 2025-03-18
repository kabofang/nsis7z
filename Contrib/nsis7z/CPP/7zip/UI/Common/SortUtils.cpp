// SortUtils.cpp

#include "StdAfx.h"

#include "../../../Common/Wildcard.h"

#include "SortUtils.h"

static int CompareStrings(const unsigned *p1, const unsigned *p2, void *param)
{
  const UStringVector &strings = *(const UStringVector *)param;
  return CompareFileNames(strings[*p1], strings[*p2]);
}

void SortFileNames(const UStringVector &strings, CUIntVector &indices)
{
  const unsigned numItems = strings.Size();
  indices.ClearAndSetSize(numItems);
  if (numItems == 0)
    return;
  unsigned *vals = &indices[0];
  for (unsigned i = 0; i < numItems; i++)
    vals[i] = i;
  indices.Sort(CompareStrings, (void *)&strings);
}

int CompareFileSizeCustom(uint64_t s1, uint64_t s2)
{
	constexpr UInt64 threshold = 100 * 1024 * 1024;
	const bool isALarge = s1 > threshold;
	const bool isBLarge = s2 > threshold;
	if (isALarge || isBLarge) {
		return s1 > s2 ? -1 : 1;
	}
	return 0;
};
