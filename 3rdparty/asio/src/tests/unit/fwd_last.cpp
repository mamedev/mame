//
// fwd_last.cpp
// ~~~~~~~~~~~~
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

// Test that full declarations are pulled in first.
#include "asio.hpp"

// Test that the forward declaration header doesn't conflict when included
// after the full declarations.
#include "asio/fwd.hpp"

#include "unit_test.hpp"

ASIO_TEST_SUITE
(
  "fwd_last",
  ASIO_TEST_CASE(null_test)
)
