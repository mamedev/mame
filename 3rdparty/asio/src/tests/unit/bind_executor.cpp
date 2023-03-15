//
// bind_executor.cpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include "asio/bind_executor.hpp"

#include "asio/io_context.hpp"
#include "asio/steady_timer.hpp"
#include "unit_test.hpp"

#if defined(ASIO_HAS_BOOST_DATE_TIME)
# include "asio/deadline_timer.hpp"
#else // defined(ASIO_HAS_BOOST_DATE_TIME)
# include "asio/steady_timer.hpp"
#endif // defined(ASIO_HAS_BOOST_DATE_TIME)

#if defined(ASIO_HAS_BOOST_BIND)
# include <boost/bind/bind.hpp>
#else // defined(ASIO_HAS_BOOST_BIND)
# include <functional>
#endif // defined(ASIO_HAS_BOOST_BIND)

using namespace asio;

#if defined(ASIO_HAS_BOOST_BIND)
namespace bindns = boost;
#else // defined(ASIO_HAS_BOOST_BIND)
namespace bindns = std;
#endif

#if defined(ASIO_HAS_BOOST_DATE_TIME)
typedef deadline_timer timer;
namespace chronons = boost::posix_time;
#elif defined(ASIO_HAS_CHRONO)
typedef steady_timer timer;
namespace chronons = asio::chrono;
#endif // defined(ASIO_HAS_BOOST_DATE_TIME)

void increment(int* count)
{
  ++(*count);
}

void bind_executor_to_function_object_test()
{
  io_context ioc1;
  io_context ioc2;

  int count = 0;

  timer t(ioc1, chronons::seconds(1));
  t.async_wait(
      bind_executor(
        ioc2.get_executor(),
        bindns::bind(&increment, &count)));

  ioc1.run();

  ASIO_CHECK(count == 0);

  ioc2.run();

  ASIO_CHECK(count == 1);
}

struct incrementer_token
{
  explicit incrementer_token(int* c) : count(c) {}
  int* count;
};

namespace asio {

template <>
class async_result<incrementer_token, void(asio::error_code)>
{
public:
  typedef void return_type;

#if defined(ASIO_HAS_VARIADIC_TEMPLATES)

  template <typename Initiation, typename... Args>
  static void initiate(Initiation initiation,
      incrementer_token token, ASIO_MOVE_ARG(Args)... args)
  {
    initiation(bindns::bind(&increment, token.count),
        ASIO_MOVE_CAST(Args)(args)...);
  }

#else // defined(ASIO_HAS_VARIADIC_TEMPLATES)

  template <typename Initiation>
  static void initiate(Initiation initiation, incrementer_token token)
  {
    initiation(bindns::bind(&increment, token.count));
  }

#define ASIO_PRIVATE_INITIATE_DEF(n) \
  template <typename Initiation, ASIO_VARIADIC_TPARAMS(n)> \
  static return_type initiate(Initiation initiation, \
      incrementer_token token, ASIO_VARIADIC_MOVE_PARAMS(n)) \
  { \
    initiation(bindns::bind(&increment, token.count), \
        ASIO_VARIADIC_MOVE_ARGS(n)); \
  } \
  /**/
  ASIO_VARIADIC_GENERATE(ASIO_PRIVATE_INITIATE_DEF)
#undef ASIO_PRIVATE_INITIATE_DEF

#endif // defined(ASIO_HAS_VARIADIC_TEMPLATES)
};

} // namespace asio

void bind_executor_to_completion_token_test()
{
  io_context ioc1;
  io_context ioc2;

  int count = 0;

  timer t(ioc1, chronons::seconds(1));
  t.async_wait(
      bind_executor(
        ioc2.get_executor(),
        incrementer_token(&count)));

  ioc1.run();

  ASIO_CHECK(count == 0);

  ioc2.run();

  ASIO_CHECK(count == 1);
}

ASIO_TEST_SUITE
(
  "bind_executor",
  ASIO_TEST_CASE(bind_executor_to_function_object_test)
  ASIO_TEST_CASE(bind_executor_to_completion_token_test)
)
