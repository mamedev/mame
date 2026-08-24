//
// buffer.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2026 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include "asio/buffer.hpp"

#include <array>
#include <cstring>
#include "unit_test.hpp"

#if defined(ASIO_HAS_BOOST_ARRAY)
# include <boost/array.hpp>
#endif // defined(ASIO_HAS_BOOST_ARRAY)

//------------------------------------------------------------------------------

// buffer_compile test
// ~~~~~~~~~~~~~~~~~~~
// The following test checks that all overloads of the buffer function compile
// and link correctly. Runtime failures are ignored.

namespace buffer_compile {

using namespace asio;

template <typename T>
class mutable_contiguous_container
{
public:
  typedef T value_type;
  typedef T* iterator;
  typedef const T* const_iterator;
  typedef T& reference;
  typedef const T& const_reference;

  mutable_contiguous_container() {}
  std::size_t size() const { return 0; }
  iterator begin() { return 0; }
  const_iterator begin() const { return 0; }
  iterator end() { return 0; }
  const_iterator end() const { return 0; }
};

template <typename T>
class const_contiguous_container
{
public:
  typedef const T value_type;
  typedef const T* iterator;
  typedef const T* const_iterator;
  typedef const T& reference;
  typedef const T& const_reference;

  const_contiguous_container() {}
  std::size_t size() const { return 0; }
  iterator begin() { return 0; }
  const_iterator begin() const { return 0; }
  iterator end() { return 0; }
  const_iterator end() const { return 0; }
};

template <typename T, std::size_t Extent>
struct span
{
  T* data() const { return 0; }
  std::size_t size() const { return 0; }
  friend span<const T, Extent> as_bytes(const span&) { return {}; }
  span<T, static_cast<std::size_t>(-1)>
    subspan(std::size_t, std::size_t = 0) const { return {}; }
};

template <typename T, std::size_t Extent>
struct span<const T, Extent>
{
  T* data() const { return 0; }
  std::size_t size() const { return 0; }
  span<const T, static_cast<std::size_t>(-1)>
    subspan(std::size_t, std::size_t = 0) const { return {}; }
};

void test()
{
  try
  {
    char raw_data[1024];
    const char const_raw_data[1024] = "";
    void* void_ptr_data = raw_data;
    const void* const_void_ptr_data = const_raw_data;
    span<char, 1> span_1;
    span<const char, 1> span_2;
#if defined(ASIO_HAS_BOOST_ARRAY)
    boost::array<char, 1024> array_data;
    const boost::array<char, 1024>& const_array_data_1 = array_data;
    boost::array<const char, 1024> const_array_data_2 = { { 0 } };
#endif // defined(ASIO_HAS_BOOST_ARRAY)
    std::array<char, 1024> std_array_data;
    const std::array<char, 1024>& const_std_array_data_1 = std_array_data;
    std::array<const char, 1024> const_std_array_data_2 = { { 0 } };
    std::vector<char> vector_data(1024);
    const std::vector<char>& const_vector_data = vector_data;
    std::string string_data(1024, ' ');
    const std::string const_string_data(1024, ' ');
    std::vector<mutable_buffer> mutable_buffer_sequence;
    std::vector<const_buffer> const_buffer_sequence;
#if defined(ASIO_HAS_STD_STRING_VIEW)
    std::string_view string_view_data(string_data);
#elif defined(ASIO_HAS_STD_EXPERIMENTAL_STRING_VIEW)
    std::experimental::string_view string_view_data(string_data);
#endif // defined(ASIO_HAS_STD_EXPERIMENTAL_STRING_VIEW)
    mutable_contiguous_container<char> mutable_contiguous_data;
    const mutable_contiguous_container<char> const_mutable_contiguous_data;
    const_contiguous_container<char> const_contiguous_data;
    const const_contiguous_container<char> const_const_contiguous_data;

    // mutable_buffer constructors.

    mutable_buffer mb1;
    mutable_buffer mb2(void_ptr_data, 1024);
    mutable_buffer mb3(span_1);
    (void)mb3;
    mutable_buffer mb4(mb1);
    (void)mb4;

    // mutable_buffer functions.

    void* ptr1 = mb1.data();
    (void)ptr1;

    std::size_t n1 = mb1.size();
    (void)n1;

    // mutable_buffer operators.

    mb1 += 128;
    mb1 = mb2 + 128;
    mb1 = 128 + mb2;

    // const_buffer constructors.

    const_buffer cb1;
    const_buffer cb2(const_void_ptr_data, 1024);
    const_buffer cb3(span_1);
    (void)cb3;
    const_buffer cb4(span_2);
    (void)cb4;
    const_buffer cb5(cb1);
    (void)cb5;
    const_buffer cb6(mb1);
    (void)cb6;

    // const_buffer functions.

    const void* ptr2 = cb1.data();
    (void)ptr2;

    std::size_t n2 = cb1.size();
    (void)n2;

    // const_buffer operators.

    cb1 += 128;
    cb1 = cb2 + 128;
    cb1 = 128 + cb2;

    // buffer_size function overloads.

    std::size_t size1 = buffer_size(mb1);
    (void)size1;
    std::size_t size2 = buffer_size(cb1);
    (void)size2;
    std::size_t size3 = buffer_size(mutable_buffer_sequence);
    (void)size3;
    std::size_t size4 = buffer_size(const_buffer_sequence);
    (void)size4;

    // buffer function overloads.

    mb1 = buffer(mb2);
    mb1 = buffer(mb2, 128);
    cb1 = buffer(cb2);
    cb1 = buffer(cb2, 128);
    mb1 = buffer(void_ptr_data, 1024);
    cb1 = buffer(const_void_ptr_data, 1024);
    mb1 = buffer(raw_data);
    mb1 = buffer(raw_data, 1024);
    cb1 = buffer(const_raw_data);
    cb1 = buffer(const_raw_data, 1024);
    mb1 = buffer(span_1);
    mb1 = buffer(span_1, 128);
    cb1 = buffer(span_2);
    cb1 = buffer(span_2, 128);
#if defined(ASIO_HAS_BOOST_ARRAY)
    mb1 = buffer(array_data);
    mb1 = buffer(array_data, 1024);
    cb1 = buffer(const_array_data_1);
    cb1 = buffer(const_array_data_1, 1024);
    cb1 = buffer(const_array_data_2);
    cb1 = buffer(const_array_data_2, 1024);
#endif // defined(ASIO_HAS_BOOST_ARRAY)
    mb1 = buffer(std_array_data);
    mb1 = buffer(std_array_data, 1024);
    cb1 = buffer(const_std_array_data_1);
    cb1 = buffer(const_std_array_data_1, 1024);
    cb1 = buffer(const_std_array_data_2);
    cb1 = buffer(const_std_array_data_2, 1024);
    mb1 = buffer(vector_data);
    mb1 = buffer(vector_data, 1024);
    cb1 = buffer(const_vector_data);
    cb1 = buffer(const_vector_data, 1024);
    mb1 = buffer(string_data);
    mb1 = buffer(string_data, 1024);
    cb1 = buffer(const_string_data);
    cb1 = buffer(const_string_data, 1024);
#if defined(ASIO_HAS_STRING_VIEW)
    cb1 = buffer(string_view_data);
    cb1 = buffer(string_view_data, 1024);
#endif // defined(ASIO_HAS_STRING_VIEW)
    mb1 = buffer(mutable_contiguous_data);
    mb1 = buffer(mutable_contiguous_data, 1024);
    cb1 = buffer(const_mutable_contiguous_data);
    cb1 = buffer(const_mutable_contiguous_data, 1024);
    cb1 = buffer(const_contiguous_data);
    cb1 = buffer(const_contiguous_data, 1024);
    cb1 = buffer(const_const_contiguous_data);
    cb1 = buffer(const_const_contiguous_data, 1024);

    // buffer_copy function overloads.

    std::size_t size5 = buffer_copy(mb1, cb2);
    (void)size5;
    std::size_t size6 = buffer_copy(mb1, mb2);
    (void)size6;
    std::size_t size7 = buffer_copy(mb1, const_buffer_sequence);
    (void)size7;
    std::size_t size8 = buffer_copy(mutable_buffer_sequence, cb2);
    (void)size8;
    std::size_t size9 = buffer_copy(mutable_buffer_sequence, mb2);
    (void)size9;
    std::size_t size10 = buffer_copy(
        mutable_buffer_sequence, const_buffer_sequence);
    (void)size10;
    std::size_t size11 = buffer_copy(mb1, cb2, 128);
    (void)size11;
    std::size_t size12 = buffer_copy(mb1, mb2, 128);
    (void)size12;
    std::size_t size13 = buffer_copy(mb1, const_buffer_sequence, 128);
    (void)size13;
    std::size_t size14 = buffer_copy(mutable_buffer_sequence, cb2, 128);
    (void)size14;
    std::size_t size15 = buffer_copy(mutable_buffer_sequence, mb2, 128);
    (void)size15;
    std::size_t size16 = buffer_copy(
        mutable_buffer_sequence, const_buffer_sequence, 128);
    (void)size16;

    // dynamic_buffer function overloads.

    dynamic_string_buffer<char, std::string::traits_type,
      std::string::allocator_type> db1 = dynamic_buffer(string_data);
    (void)db1;
    dynamic_string_buffer<char, std::string::traits_type,
      std::string::allocator_type> db2 = dynamic_buffer(string_data, 1024);
    (void)db2;
    dynamic_vector_buffer<char, std::allocator<char> >
      db3 = dynamic_buffer(vector_data);
    (void)db3;
    dynamic_vector_buffer<char, std::allocator<char> >
      db4 = dynamic_buffer(vector_data, 1024);
    (void)db4;

    // dynamic_buffer member functions.

    std::size_t size17 = db1.size();
    (void)size17;
    std::size_t size18 = db3.size();
    (void)size18;

    std::size_t size19 = db1.max_size();
    (void)size19;
    std::size_t size20 = db3.max_size();
    (void)size20;

#if !defined(ASIO_NO_DYNAMIC_BUFFER_V1)
    dynamic_string_buffer<char, std::string::traits_type,
      std::string::allocator_type>::const_buffers_type
        cb7 = db1.data();
    (void)cb7;
    dynamic_vector_buffer<char, std::allocator<char> >::const_buffers_type
      cb8 = db3.data();
    (void)cb8;

    dynamic_string_buffer<char, std::string::traits_type,
      std::string::allocator_type>::mutable_buffers_type mb5
        = db1.prepare(1024);
    (void)mb5;
    dynamic_vector_buffer<char, std::allocator<char> >::mutable_buffers_type
      mb6 = db3.prepare(1024);
    (void)mb6;

    db1.commit(1024);
    db3.commit(1024);
#endif // !defined(ASIO_NO_DYNAMIC_BUFFER_V1)

    dynamic_string_buffer<char, std::string::traits_type,
      std::string::allocator_type>::mutable_buffers_type
        mb7 = db1.data(0, 1);
    (void)mb7;
    dynamic_vector_buffer<char, std::allocator<char> >::mutable_buffers_type
      mb8 = db3.data(0, 1);
    (void)mb8;

    dynamic_string_buffer<char, std::string::traits_type,
      std::string::allocator_type>::const_buffers_type
        cb9 = static_cast<const dynamic_string_buffer<char,
          std::string::traits_type,
            std::string::allocator_type>&>(db1).data(0, 1);
    (void)cb9;
    dynamic_vector_buffer<char, std::allocator<char> >::const_buffers_type
      cb10 = static_cast<const dynamic_vector_buffer<char,
        std::allocator<char> >&>(db3).data(0, 1);
    (void)cb10;

    db1.grow(1024);
    db3.grow(1024);

    db1.shrink(1024);
    db3.shrink(1024);

    db1.consume(0);
    db3.consume(0);
  }
  catch (std::exception&)
  {
  }
}

} // namespace buffer_compile

//------------------------------------------------------------------------------

namespace buffer_copy_runtime {

using namespace asio;
using namespace std;

void test()
{
  char dest_data[256];
  char source_data[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

  memset(dest_data, 0, sizeof(dest_data));
  mutable_buffer mb1 = asio::buffer(dest_data);
  mutable_buffer mb2 = asio::buffer(source_data);
  std::size_t n = buffer_copy(mb1, mb2);
  ASIO_CHECK(n == sizeof(source_data));
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mb1 = asio::buffer(dest_data);
  const_buffer cb1 = asio::buffer(source_data);
  n = buffer_copy(mb1, cb1);
  ASIO_CHECK(n == sizeof(source_data));
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mb1 = asio::buffer(dest_data);
  std::vector<mutable_buffer> mv1;
  mv1.push_back(asio::buffer(source_data, 5));
  mv1.push_back(asio::buffer(source_data) + 5);
  n = buffer_copy(mb1, mv1);
  ASIO_CHECK(n == sizeof(source_data));
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mb1 = asio::buffer(dest_data);
  std::vector<const_buffer> cv1;
  cv1.push_back(asio::buffer(source_data, 6));
  cv1.push_back(asio::buffer(source_data) + 6);
  n = buffer_copy(mb1, cv1);
  ASIO_CHECK(n == sizeof(source_data));
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mv1.clear();
  mv1.push_back(asio::buffer(dest_data, 7));
  mv1.push_back(asio::buffer(dest_data) + 7);
  cb1 = asio::buffer(source_data);
  n = buffer_copy(mv1, cb1);
  ASIO_CHECK(n == sizeof(source_data));
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mv1.clear();
  mv1.push_back(asio::buffer(dest_data, 7));
  mv1.push_back(asio::buffer(dest_data) + 7);
  cv1.clear();
  cv1.push_back(asio::buffer(source_data, 8));
  cv1.push_back(asio::buffer(source_data) + 8);
  n = buffer_copy(mv1, cv1);
  ASIO_CHECK(n == sizeof(source_data));
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mb1 = asio::buffer(dest_data);
  mb2 = asio::buffer(source_data);
  n = buffer_copy(mb1, mb2, 10);
  ASIO_CHECK(n == 10);
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mb1 = asio::buffer(dest_data);
  cb1 = asio::buffer(source_data);
  n = buffer_copy(mb1, cb1, 10);
  ASIO_CHECK(n == 10);
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mb1 = asio::buffer(dest_data);
  mv1.clear();
  mv1.push_back(asio::buffer(source_data, 5));
  mv1.push_back(asio::buffer(source_data) + 5);
  n = buffer_copy(mb1, mv1, 10);
  ASIO_CHECK(n == 10);
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mb1 = asio::buffer(dest_data);
  cv1.clear();
  cv1.push_back(asio::buffer(source_data, 6));
  cv1.push_back(asio::buffer(source_data) + 6);
  n = buffer_copy(mb1, cv1, 10);
  ASIO_CHECK(n == 10);
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mv1.clear();
  mv1.push_back(asio::buffer(dest_data, 7));
  mv1.push_back(asio::buffer(dest_data) + 7);
  cb1 = asio::buffer(source_data);
  n = buffer_copy(mv1, cb1, 10);
  ASIO_CHECK(n == 10);
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);

  memset(dest_data, 0, sizeof(dest_data));
  mv1.clear();
  mv1.push_back(asio::buffer(dest_data, 7));
  mv1.push_back(asio::buffer(dest_data) + 7);
  cv1.clear();
  cv1.push_back(asio::buffer(source_data, 8));
  cv1.push_back(asio::buffer(source_data) + 8);
  n = buffer_copy(mv1, cv1, 10);
  ASIO_CHECK(n == 10);
  ASIO_CHECK(memcmp(dest_data, source_data, n) == 0);
}

} // namespace buffer_copy_runtime

//------------------------------------------------------------------------------

namespace buffer_sequence {

using namespace asio;
using namespace std;

struct valid_const_a
{
  typedef const_buffer* const_iterator;
  typedef const_buffer value_type;
  const_buffer* begin() const { return 0; }
  const_buffer* end() const { return 0; }
};

struct valid_const_b
{
  const_buffer* begin() const { return 0; }
  const_buffer* end() const { return 0; }
};

struct valid_mutable_a
{
  typedef mutable_buffer* const_iterator;
  typedef mutable_buffer value_type;
  mutable_buffer* begin() const { return 0; }
  mutable_buffer* end() const { return 0; }
};

struct valid_mutable_b
{
  mutable_buffer* begin() const { return 0; }
  mutable_buffer* end() const { return 0; }
};

struct custom_const_buffer_sequence
{
  operator const_buffer() const { return const_buffer(0, 0); }
};

struct custom_mutable_buffer_sequence
{
  operator mutable_buffer() const { return mutable_buffer(0, 0); }
  operator const_buffer() const { return mutable_buffer(0, 0); }
};

struct invalid_const_a
{
  typedef int value_type;
  int* begin() const { return 0; }
  const_buffer* end() const { return 0; }
};

struct invalid_const_b
{
  typedef const_buffer value_type;
  const_buffer* begin() const { return 0; }
};

struct invalid_const_c
{
  typedef const_buffer value_type;
  const_buffer* end() const { return 0; }
};

struct invalid_const_d
{
  int* begin() const { return 0; }
  const_buffer* end() const { return 0; }
};

struct invalid_const_e
{
  const_buffer* begin() const { return 0; }
};

struct invalid_const_f
{
  const_buffer* end() const { return 0; }
};

struct invalid_mutable_a
{
  typedef int value_type;
  int* begin() const { return 0; }
  mutable_buffer* end() const { return 0; }
};

struct invalid_mutable_b
{
  typedef mutable_buffer value_type;
  mutable_buffer* begin() const { return 0; }
};

struct invalid_mutable_c
{
  typedef mutable_buffer value_type;
  mutable_buffer* end() const { return 0; }
};

struct invalid_mutable_d
{
  int* begin() const { return 0; }
  mutable_buffer* end() const { return 0; }
};

struct invalid_mutable_e
{
  mutable_buffer* begin() const { return 0; }
};

struct invalid_mutable_f
{
  mutable_buffer* end() const { return 0; }
};

void test()
{
  ASIO_CHECK(is_const_buffer_sequence<const_buffer>::value);
  ASIO_CHECK(!is_mutable_buffer_sequence<const_buffer>::value);

  const_buffer b1;
  ASIO_CHECK(buffer_sequence_begin(b1) == &b1);
  ASIO_CHECK(buffer_sequence_end(b1) == &b1 + 1);

  ASIO_CHECK(is_const_buffer_sequence<mutable_buffer>::value);
  ASIO_CHECK(is_mutable_buffer_sequence<mutable_buffer>::value);

  mutable_buffer b2;
  ASIO_CHECK(buffer_sequence_begin(b2) == &b2);
  ASIO_CHECK(buffer_sequence_end(b2) == &b2 + 1);

  ASIO_CHECK(is_const_buffer_sequence<vector<const_buffer> >::value);
  ASIO_CHECK(!is_mutable_buffer_sequence<vector<const_buffer> >::value);

  vector<const_buffer> b3;
  ASIO_CHECK(buffer_sequence_begin(b3) == b3.begin());
  ASIO_CHECK(buffer_sequence_end(b3) == b3.end());

  ASIO_CHECK(is_const_buffer_sequence<vector<mutable_buffer> >::value);
  ASIO_CHECK(is_mutable_buffer_sequence<vector<mutable_buffer> >::value);

  vector<mutable_buffer> b4;
  ASIO_CHECK(buffer_sequence_begin(b4) == b4.begin());
  ASIO_CHECK(buffer_sequence_end(b4) == b4.end());

  ASIO_CHECK(is_const_buffer_sequence<valid_const_a>::value);
  ASIO_CHECK(!is_mutable_buffer_sequence<valid_const_a>::value);

  valid_const_a b5;
  ASIO_CHECK(buffer_sequence_begin(b5) == b5.begin());
  ASIO_CHECK(buffer_sequence_end(b5) == b5.end());

  ASIO_CHECK(is_const_buffer_sequence<valid_const_b>::value);
  ASIO_CHECK(!is_mutable_buffer_sequence<valid_const_b>::value);

  valid_const_b b6;
  ASIO_CHECK(buffer_sequence_begin(b6) == b6.begin());
  ASIO_CHECK(buffer_sequence_end(b6) == b6.end());

  ASIO_CHECK(is_const_buffer_sequence<valid_mutable_a>::value);
  ASIO_CHECK(is_mutable_buffer_sequence<valid_mutable_a>::value);

  valid_mutable_a b7;
  ASIO_CHECK(buffer_sequence_begin(b7) == b7.begin());
  ASIO_CHECK(buffer_sequence_end(b7) == b7.end());

  ASIO_CHECK(is_const_buffer_sequence<valid_mutable_b>::value);
  ASIO_CHECK(is_mutable_buffer_sequence<valid_mutable_b>::value);

  valid_mutable_b b8;
  ASIO_CHECK(buffer_sequence_begin(b8) == b8.begin());
  ASIO_CHECK(buffer_sequence_end(b8) == b8.end());

  ASIO_CHECK(is_const_buffer_sequence<
      custom_const_buffer_sequence>::value);
  ASIO_CHECK(!is_mutable_buffer_sequence<
      custom_const_buffer_sequence>::value);

  custom_const_buffer_sequence b11;
  ASIO_CHECK(buffer_sequence_begin(b11) == &b11);
  ASIO_CHECK(buffer_sequence_end(b11) == &b11 + 1);

  ASIO_CHECK(is_const_buffer_sequence<
      custom_mutable_buffer_sequence>::value);
  ASIO_CHECK(is_mutable_buffer_sequence<
      custom_mutable_buffer_sequence>::value);

  custom_mutable_buffer_sequence b12;
  ASIO_CHECK(buffer_sequence_begin(b12) == &b12);
  ASIO_CHECK(buffer_sequence_end(b12) == &b12 + 1);

  ASIO_CHECK(!is_const_buffer_sequence<invalid_const_a>::value);
  ASIO_CHECK(!is_mutable_buffer_sequence<invalid_const_a>::value);

  ASIO_CHECK(!is_const_buffer_sequence<invalid_const_b>::value);
  ASIO_CHECK(!is_mutable_buffer_sequence<invalid_const_b>::value);

  ASIO_CHECK(!is_const_buffer_sequence<invalid_const_c>::value);
  ASIO_CHECK(!is_mutable_buffer_sequence<invalid_const_c>::value);

  ASIO_CHECK(!is_const_buffer_sequence<invalid_const_d>::value);
  ASIO_CHECK(!is_mutable_buffer_sequence<invalid_const_d>::value);

  ASIO_CHECK(!is_const_buffer_sequence<invalid_const_e>::value);
  ASIO_CHECK(!is_mutable_buffer_sequence<invalid_const_e>::value);

  ASIO_CHECK(!is_const_buffer_sequence<invalid_const_f>::value);
  ASIO_CHECK(!is_mutable_buffer_sequence<invalid_const_f>::value);

  ASIO_CHECK(!is_mutable_buffer_sequence<invalid_mutable_a>::value);
  ASIO_CHECK(!is_mutable_buffer_sequence<invalid_mutable_a>::value);

  ASIO_CHECK(!is_mutable_buffer_sequence<invalid_mutable_b>::value);
  ASIO_CHECK(!is_mutable_buffer_sequence<invalid_mutable_b>::value);

  ASIO_CHECK(!is_mutable_buffer_sequence<invalid_mutable_c>::value);
  ASIO_CHECK(!is_mutable_buffer_sequence<invalid_mutable_c>::value);

  ASIO_CHECK(!is_mutable_buffer_sequence<invalid_mutable_d>::value);
  ASIO_CHECK(!is_mutable_buffer_sequence<invalid_mutable_d>::value);

  ASIO_CHECK(!is_mutable_buffer_sequence<invalid_mutable_e>::value);
  ASIO_CHECK(!is_mutable_buffer_sequence<invalid_mutable_e>::value);

  ASIO_CHECK(!is_mutable_buffer_sequence<invalid_mutable_f>::value);
  ASIO_CHECK(!is_mutable_buffer_sequence<invalid_mutable_f>::value);
}

} // namespace buffer_sequence

namespace buffer_literals {

void test()
{
#if defined(ASIO_HAS_USER_DEFINED_LITERALS)
  using namespace asio::buffer_literals;
  using namespace std; // For memcmp.

  asio::const_buffer b1 = ""_buf;
  ASIO_CHECK(b1.size() == 0);

  asio::const_buffer b2 = "hello"_buf;
  ASIO_CHECK(b2.size() == 5);
  ASIO_CHECK(memcmp(b2.data(), "hello", 5) == 0);

  asio::const_buffer b3 = 0x00_buf;
  ASIO_CHECK(b3.size() == 1);
  ASIO_CHECK(memcmp(b3.data(), "\x00", 1) == 0);

  asio::const_buffer b4 = 0X01_buf;
  ASIO_CHECK(b4.size() == 1);
  ASIO_CHECK(memcmp(b4.data(), "\x01", 1) == 0);

  asio::const_buffer b5 = 0xaB_buf;
  ASIO_CHECK(b5.size() == 1);
  ASIO_CHECK(memcmp(b5.data(), "\xab", 1) == 0);

  asio::const_buffer b6 = 0xABcd_buf;
  ASIO_CHECK(b6.size() == 2);
  ASIO_CHECK(memcmp(b6.data(), "\xab\xcd", 2) == 0);

  asio::const_buffer b7 = 0x01ab01cd01ef01ba01dc01fe_buf;
  ASIO_CHECK(b7.size() == 12);
  ASIO_CHECK(memcmp(b7.data(),
        "\x01\xab\x01\xcd\x01\xef\x01\xba\x01\xdc\x01\xfe", 12) == 0);

  asio::const_buffer b8 = 0b00000000_buf;
  ASIO_CHECK(b8.size() == 1);
  ASIO_CHECK(memcmp(b8.data(), "\x00", 1) == 0);

  asio::const_buffer b9 = 0B00000001_buf;
  ASIO_CHECK(b9.size() == 1);
  ASIO_CHECK(memcmp(b9.data(), "\x01", 1) == 0);

  asio::const_buffer b10 = 0B11111111_buf;
  ASIO_CHECK(b10.size() == 1);
  ASIO_CHECK(memcmp(b10.data(), "\xFF", 1) == 0);

  asio::const_buffer b11 = 0b1111000000001111_buf;
  ASIO_CHECK(b11.size() == 2);
  ASIO_CHECK(memcmp(b11.data(), "\xF0\x0F", 2) == 0);
#endif // (defined(ASIO_HAS_USER_DEFINED_LITERALS)
}

} // namespace buffer_literals

//------------------------------------------------------------------------------

ASIO_TEST_SUITE
(
  "buffer",
  ASIO_COMPILE_TEST_CASE(buffer_compile::test)
  ASIO_TEST_CASE(buffer_copy_runtime::test)
  ASIO_TEST_CASE(buffer_sequence::test)
  ASIO_TEST_CASE(buffer_literals::test)
)
