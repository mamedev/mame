// Common/MyVector.cpp

#include "StdAfx.h"

#include <string.h>

#include "MyVector.h"

CBaseRecordVector::~CBaseRecordVector() { ClearAndFree(); }

void CBaseRecordVector::ClearAndFree()
{
  Clear();
  delete []((unsigned char *)_items);
  _capacity = 0;
  _size = 0;
  _items = 0;
}

void CBaseRecordVector::Clear() { DeleteFrom(0); }
void CBaseRecordVector::DeleteBack() { Delete(_size - 1); }
void CBaseRecordVector::DeleteFrom(int index) { Delete(index, _size - index); }

void CBaseRecordVector::ReserveOnePosition()
{
  if (_size != _capacity)
    return;
  unsigned delta = 1;
  if (_capacity >= 64)
    delta = (unsigned)_capacity / 4;
  else if (_capacity >= 8)
    delta = 8;
  Reserve(_capacity + (int)delta);
}

void CBaseRecordVector::Reserve(int newCapacity)
{
  // if (newCapacity <= _capacity)
  if (newCapacity == _capacity)
    return;
  if ((unsigned)newCapacity >= ((unsigned)1 << (sizeof(unsigned) * 8 - 1)))
    throw 1052353;
  size_t newSize = (size_t)(unsigned)newCapacity * _itemSize;
  if (newSize / _itemSize != (size_t)(unsigned)newCapacity)
    throw 1052354;
  unsigned char *p = NULL;
  if (newSize > 0)
  {
    p = new unsigned char[newSize];
    if (p == 0)
      throw 1052355;
    int numRecordsToMove = (_size < newCapacity ? _size : newCapacity);
    memcpy(p, _items, _itemSize * numRecordsToMove);
  }
  delete [](unsigned char *)_items;
  _items = p;
  _capacity = newCapacity;
}

void CBaseRecordVector::ReserveDown()
{
  Reserve(_size);
}

void CBaseRecordVector::MoveItems(int destIndex, int srcIndex)
{
  memmove(((unsigned char *)_items) + destIndex * _itemSize,
    ((unsigned char  *)_items) + srcIndex * _itemSize,
    _itemSize * (_size - srcIndex));
}

void CBaseRecordVector::InsertOneItem(int index)
{
  ReserveOnePosition();
  MoveItems(index + 1, index);
  _size++;
}

void CBaseRecordVector::Delete(int index, int num)
{
  TestIndexAndCorrectNum(index, num);
  if (num > 0)
  {
    MoveItems(index, index + num);
    _size -= num;
  }
}
