// Common/DynamicBuffer.h

#ifndef __COMMON_DYNAMIC_BUFFER_H
#define __COMMON_DYNAMIC_BUFFER_H

#include "Buffer.h"

template <class T> class CDynamicBuffer: public CBuffer<T>
{
  void GrowLength(size_t size)
  {
    size_t delta;
    if (this->_capacity > 64)
      delta = this->_capacity / 4;
    else if (this->_capacity > 8)
      delta = 16;
    else
      delta = 4;
    delta = MyMax(delta, size);
    size_t newCap = this->_capacity + delta;
    if (newCap < delta)
      newCap = this->_capacity + size;
    SetCapacity(newCap);
  }
public:
  CDynamicBuffer(): CBuffer<T>() {};
  CDynamicBuffer(const CDynamicBuffer &buffer): CBuffer<T>(buffer) {};
  CDynamicBuffer(size_t size): CBuffer<T>(size) {};
  CDynamicBuffer& operator=(const CDynamicBuffer &buffer)
  {
    this->Free();
    if (buffer._capacity > 0)
    {
      SetCapacity(buffer._capacity);
      memmove(this->_items, buffer._items, buffer._capacity * sizeof(T));
    }
    return *this;
  }
  void EnsureCapacity(size_t capacity)
  {
    if (this->_capacity < capacity)
      GrowLength(capacity - this->_capacity);
  }
};

typedef CDynamicBuffer<char> CCharDynamicBuffer;
typedef CDynamicBuffer<wchar_t> CWCharDynamicBuffer;
typedef CDynamicBuffer<unsigned char> CByteDynamicBuffer;

#endif
