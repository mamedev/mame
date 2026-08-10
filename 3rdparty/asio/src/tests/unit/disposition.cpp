//
// disposition.cpp
// ~~~~~~~~~~~~~~~
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
#include "asio/disposition.hpp"

#include "asio/error.hpp"

#include "unit_test.hpp"

void no_error_test()
{
  using asio::no_error;

  ASIO_CHECK(no_error == no_error);
  ASIO_CHECK(!(no_error != no_error));
}

void error_code_disposition_test()
{
  using asio::no_error;

  asio::error_code ec1;

  ASIO_CHECK(ec1 == no_error);
  ASIO_CHECK(no_error == ec1);
  ASIO_CHECK(!(ec1 != no_error));
  ASIO_CHECK(!(no_error != ec1));

  std::exception_ptr ep1 = asio::to_exception_ptr(ec1);
  ASIO_CHECK(ep1 == nullptr);

  asio::error_code ec2 = asio::error::eof;

  ASIO_CHECK(!(ec2 == no_error));
  ASIO_CHECK(!(no_error == ec2));
  ASIO_CHECK(ec2 != no_error);
  ASIO_CHECK(no_error != ec2);

#if !defined(ASIO_NO_EXCEPTIONS)
  bool caught;
  try
  {
    asio::throw_exception(ec2);
    caught = false;
  }
  catch (const asio::system_error& ex)
  {
    caught = true;
    ASIO_CHECK(ex.code() == asio::error::eof);
  }
  ASIO_CHECK(caught);
#endif // !defined(ASIO_NO_EXCEPTIONS)

  std::exception_ptr ep2 = asio::to_exception_ptr(ec2);
  ASIO_CHECK(ep2 != nullptr);
}

void exception_ptr_disposition_test()
{
  using asio::no_error;

  std::exception_ptr ep1;

  ASIO_CHECK(ep1 == no_error);
  ASIO_CHECK(no_error == ep1);
  ASIO_CHECK(!(ep1 != no_error));
  ASIO_CHECK(!(no_error != ep1));

  std::exception_ptr ep2 = asio::to_exception_ptr(ep1);
  ASIO_CHECK(ep1 == nullptr);

  std::exception_ptr ep3 = std::make_exception_ptr(
      asio::system_error(asio::error::eof));

  ASIO_CHECK(!(ep3 == no_error));
  ASIO_CHECK(!(no_error == ep3));
  ASIO_CHECK(ep3 != no_error);
  ASIO_CHECK(no_error != ep3);

#if !defined(ASIO_NO_EXCEPTIONS)
  bool caught;
  try
  {
    asio::throw_exception(ep3);
    caught = false;
  }
  catch (const asio::system_error& ex)
  {
    caught = true;
    ASIO_CHECK(ex.code() == asio::error::eof);
  }
  ASIO_CHECK(caught);
#endif // !defined(ASIO_NO_EXCEPTIONS)

  std::exception_ptr ep4 = asio::to_exception_ptr(ep3);
  ASIO_CHECK(ep4 != nullptr);
}

ASIO_TEST_SUITE
(
  "disposition",
  ASIO_TEST_CASE(no_error_test)
  ASIO_TEST_CASE(error_code_disposition_test)
  ASIO_TEST_CASE(exception_ptr_disposition_test)
)
