// SortUtils.h

#ifndef __SORT_UTLS_H
#define __SORT_UTLS_H

#include "../../../Common/MyString.h"
#include <stdint.h>

void SortFileNames(const UStringVector &strings, CUIntVector &indices);
int CompareFileSizeCustom(uint64_t s1, uint64_t s2);

#endif
