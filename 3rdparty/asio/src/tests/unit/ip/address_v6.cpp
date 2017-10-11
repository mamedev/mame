//
// address_v6.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2016 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Disable autolinking for unit tests.
#if !defined(BOOST_ALL_NO_LIB)
#define BOOST_ALL_NO_LIB 1
#endif // !defined(BOOST_ALL_NO_LIB)

// Test that header file is self-contained.
#include "asio/ip/address_v6.hpp"

#include "../unit_test.hpp"
#include <sstream>

//------------------------------------------------------------------------------

// ip_address_v6_compile test
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks that all public member functions on the class
// ip::address_v6 compile and link correctly. Runtime failures are ignored.

namespace ip_address_v6_compile {

void test()
{
  using namespace asio;
  namespace ip = asio::ip;

  try
  {
    asio::error_code ec;

    // address_v6 constructors.

    ip::address_v6 addr1;
    const ip::address_v6::bytes_type const_bytes_value = { { 0 } };
    ip::address_v6 addr2(const_bytes_value);

    // address_v6 functions.

    unsigned long scope_id = addr1.scope_id();
    addr1.scope_id(scope_id);

    bool b = addr1.is_unspecified();
    (void)b;

    b = addr1.is_loopback();
    (void)b;

    b = addr1.is_multicast();
    (void)b;

    b = addr1.is_link_local();
    (void)b;

    b = addr1.is_site_local();
    (void)b;

    b = addr1.is_v4_mapped();
    (void)b;

#if !defined(ASIO_NO_DEPRECATED)
    b = addr1.is_v4_compatible();
    (void)b;
#endif // !defined(ASIO_NO_DEPRECATED)

    b = addr1.is_multicast_node_local();
    (void)b;

    b = addr1.is_multicast_link_local();
    (void)b;

    b = addr1.is_multicast_site_local();
    (void)b;

    b = addr1.is_multicast_org_local();
    (void)b;

    b = addr1.is_multicast_global();
    (void)b;

    ip::address_v6::bytes_type bytes_value = addr1.to_bytes();
    (void)bytes_value;

    std::string string_value = addr1.to_string();
#if !defined(ASIO_NO_DEPRECATED)
    string_value = addr1.to_string(ec);
#endif // !defined(ASIO_NO_DEPRECATED)

#if !defined(ASIO_NO_DEPRECATED)
    ip::address_v4 addr3 = addr1.to_v4();
#endif // !defined(ASIO_NO_DEPRECATED)

    // address_v6 static functions.

#if !defined(ASIO_NO_DEPRECATED)
    addr1 = ip::address_v6::from_string("0::0");
    addr1 = ip::address_v6::from_string("0::0", ec);
    addr1 = ip::address_v6::from_string(string_value);
    addr1 = ip::address_v6::from_string(string_value, ec);
#endif // !defined(ASIO_NO_DEPRECATED)

    addr1 = ip::address_v6::any();

    addr1 = ip::address_v6::loopback();

#if !defined(ASIO_NO_DEPRECATED)
    addr1 = ip::address_v6::v4_mapped(addr3);

    addr1 = ip::address_v6::v4_compatible(addr3);
#endif // !defined(ASIO_NO_DEPRECATED)

    // address_v6 comparisons.

    b = (addr1 == addr2);
    (void)b;

    b = (addr1 != addr2);
    (void)b;

    b = (addr1 < addr2);
    (void)b;

    b = (addr1 > addr2);
    (void)b;

    b = (addr1 <= addr2);
    (void)b;

    b = (addr1 >= addr2);
    (void)b;

    // address_v6 creation functions.

    addr1 = ip::make_address_v6(const_bytes_value, scope_id);
    addr1 = ip::make_address_v6("0::0");
    addr1 = ip::make_address_v6("0::0", ec);
    addr1 = ip::make_address_v6(string_value);
    addr1 = ip::make_address_v6(string_value, ec);
#if defined(ASIO_HAS_STD_STRING_VIEW)
# if defined(ASIO_HAS_STD_EXPERIMENTAL_STRING_VIEW)
    std::experimental::string_view string_view_value("0::0");
# else // defined(ASIO_HAS_STD_EXPERIMENTAL_STRING_VIEW)
    std::string_view string_view_value("0::0");
# endif // defined(ASIO_HAS_STD_EXPERIMENTAL_STRING_VIEW)
    addr1 = ip::make_address_v6(string_view_value);
    addr1 = ip::make_address_v6(string_view_value, ec);
#endif // defined(ASIO_HAS_STD_STRING_VIEW)

    // address_v6 IPv4-mapped conversion.
#if defined(ASIO_NO_DEPRECATED)
    ip::address_v4 addr3;
#endif // defined(ASIO_NO_DEPRECATED)
    addr1 = ip::make_address_v6(ip::v4_mapped, addr3);
    addr3 = ip::make_address_v4(ip::v4_mapped, addr1);

    // address_v6 I/O.

    std::ostringstream os;
    os << addr1;

#if !defined(BOOST_NO_STD_WSTREAMBUF)
    std::wostringstream wos;
    wos << addr1;
#endif // !defined(BOOST_NO_STD_WSTREAMBUF)
  }
  catch (std::exception&)
  {
  }
}

} // namespace ip_address_v6_compile

//------------------------------------------------------------------------------

// ip_address_v6_runtime test
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
// The following test checks that the various public member functions meet the
// necessary postconditions.

namespace ip_address_v6_runtime {

void test()
{
  using asio::ip::address_v6;

  address_v6 a1;
  ASIO_CHECK(a1.is_unspecified());
  ASIO_CHECK(a1.scope_id() == 0);

  address_v6::bytes_type b1 = {{ 1, 2, 3, 4, 5,
    6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 }};
  address_v6 a2(b1, 12345);
  ASIO_CHECK(a2.to_bytes()[0] == 1);
  ASIO_CHECK(a2.to_bytes()[1] == 2);
  ASIO_CHECK(a2.to_bytes()[2] == 3);
  ASIO_CHECK(a2.to_bytes()[3] == 4);
  ASIO_CHECK(a2.to_bytes()[4] == 5);
  ASIO_CHECK(a2.to_bytes()[5] == 6);
  ASIO_CHECK(a2.to_bytes()[6] == 7);
  ASIO_CHECK(a2.to_bytes()[7] == 8);
  ASIO_CHECK(a2.to_bytes()[8] == 9);
  ASIO_CHECK(a2.to_bytes()[9] == 10);
  ASIO_CHECK(a2.to_bytes()[10] == 11);
  ASIO_CHECK(a2.to_bytes()[11] == 12);
  ASIO_CHECK(a2.to_bytes()[12] == 13);
  ASIO_CHECK(a2.to_bytes()[13] == 14);
  ASIO_CHECK(a2.to_bytes()[14] == 15);
  ASIO_CHECK(a2.to_bytes()[15] == 16);
  ASIO_CHECK(a2.scope_id() == 12345);

  address_v6 a3;
  a3.scope_id(12345);
  ASIO_CHECK(a3.scope_id() == 12345);

  address_v6 unspecified_address;
  address_v6::bytes_type loopback_bytes = {{ 0 }};
  loopback_bytes[15] = 1;
  address_v6 loopback_address(loopback_bytes);
  address_v6::bytes_type link_local_bytes = {{ 0xFE, 0x80, 1 }};
  address_v6 link_local_address(link_local_bytes);
  address_v6::bytes_type site_local_bytes = {{ 0xFE, 0xC0, 1 }};
  address_v6 site_local_address(site_local_bytes);
  address_v6::bytes_type v4_mapped_bytes = {{ 0 }};
  v4_mapped_bytes[10] = 0xFF, v4_mapped_bytes[11] = 0xFF;
  v4_mapped_bytes[12] = 1, v4_mapped_bytes[13] = 2;
  v4_mapped_bytes[14] = 3, v4_mapped_bytes[15] = 4;
  address_v6 v4_mapped_address(v4_mapped_bytes);
  address_v6::bytes_type v4_compat_bytes = {{ 0 }};
  v4_compat_bytes[12] = 1, v4_compat_bytes[13] = 2;
  v4_compat_bytes[14] = 3, v4_compat_bytes[15] = 4;
  address_v6 v4_compat_address(v4_compat_bytes);
  address_v6::bytes_type mcast_global_bytes = {{ 0xFF, 0x0E, 1 }};
  address_v6 mcast_global_address(mcast_global_bytes);
  address_v6::bytes_type mcast_link_local_bytes = {{ 0xFF, 0x02, 1 }};
  address_v6 mcast_link_local_address(mcast_link_local_bytes);
  address_v6::bytes_type mcast_node_local_bytes = {{ 0xFF, 0x01, 1 }};
  address_v6 mcast_node_local_address(mcast_node_local_bytes);
  address_v6::bytes_type mcast_org_local_bytes = {{ 0xFF, 0x08, 1 }};
  address_v6 mcast_org_local_address(mcast_org_local_bytes);
  address_v6::bytes_type mcast_site_local_bytes = {{ 0xFF, 0x05, 1 }};
  address_v6 mcast_site_local_address(mcast_site_local_bytes);

  ASIO_CHECK(!unspecified_address.is_loopback());
  ASIO_CHECK(loopback_address.is_loopback());
  ASIO_CHECK(!link_local_address.is_loopback());
  ASIO_CHECK(!site_local_address.is_loopback());
  ASIO_CHECK(!v4_mapped_address.is_loopback());
  ASIO_CHECK(!v4_compat_address.is_loopback());
  ASIO_CHECK(!mcast_global_address.is_loopback());
  ASIO_CHECK(!mcast_link_local_address.is_loopback());
  ASIO_CHECK(!mcast_node_local_address.is_loopback());
  ASIO_CHECK(!mcast_org_local_address.is_loopback());
  ASIO_CHECK(!mcast_site_local_address.is_loopback());

  ASIO_CHECK(unspecified_address.is_unspecified());
  ASIO_CHECK(!loopback_address.is_unspecified());
  ASIO_CHECK(!link_local_address.is_unspecified());
  ASIO_CHECK(!site_local_address.is_unspecified());
  ASIO_CHECK(!v4_mapped_address.is_unspecified());
  ASIO_CHECK(!v4_compat_address.is_unspecified());
  ASIO_CHECK(!mcast_global_address.is_unspecified());
  ASIO_CHECK(!mcast_link_local_address.is_unspecified());
  ASIO_CHECK(!mcast_node_local_address.is_unspecified());
  ASIO_CHECK(!mcast_org_local_address.is_unspecified());
  ASIO_CHECK(!mcast_site_local_address.is_unspecified());

  ASIO_CHECK(!unspecified_address.is_link_local());
  ASIO_CHECK(!loopback_address.is_link_local());
  ASIO_CHECK(link_local_address.is_link_local());
  ASIO_CHECK(!site_local_address.is_link_local());
  ASIO_CHECK(!v4_mapped_address.is_link_local());
  ASIO_CHECK(!v4_compat_address.is_link_local());
  ASIO_CHECK(!mcast_global_address.is_link_local());
  ASIO_CHECK(!mcast_link_local_address.is_link_local());
  ASIO_CHECK(!mcast_node_local_address.is_link_local());
  ASIO_CHECK(!mcast_org_local_address.is_link_local());
  ASIO_CHECK(!mcast_site_local_address.is_link_local());

  ASIO_CHECK(!unspecified_address.is_site_local());
  ASIO_CHECK(!loopback_address.is_site_local());
  ASIO_CHECK(!link_local_address.is_site_local());
  ASIO_CHECK(site_local_address.is_site_local());
  ASIO_CHECK(!v4_mapped_address.is_site_local());
  ASIO_CHECK(!v4_compat_address.is_site_local());
  ASIO_CHECK(!mcast_global_address.is_site_local());
  ASIO_CHECK(!mcast_link_local_address.is_site_local());
  ASIO_CHECK(!mcast_node_local_address.is_site_local());
  ASIO_CHECK(!mcast_org_local_address.is_site_local());
  ASIO_CHECK(!mcast_site_local_address.is_site_local());

  ASIO_CHECK(!unspecified_address.is_v4_mapped());
  ASIO_CHECK(!loopback_address.is_v4_mapped());
  ASIO_CHECK(!link_local_address.is_v4_mapped());
  ASIO_CHECK(!site_local_address.is_v4_mapped());
  ASIO_CHECK(v4_mapped_address.is_v4_mapped());
  ASIO_CHECK(!v4_compat_address.is_v4_mapped());
  ASIO_CHECK(!mcast_global_address.is_v4_mapped());
  ASIO_CHECK(!mcast_link_local_address.is_v4_mapped());
  ASIO_CHECK(!mcast_node_local_address.is_v4_mapped());
  ASIO_CHECK(!mcast_org_local_address.is_v4_mapped());
  ASIO_CHECK(!mcast_site_local_address.is_v4_mapped());

#if !defined(ASIO_NO_DEPRECATED)
  ASIO_CHECK(!unspecified_address.is_v4_compatible());
  ASIO_CHECK(!loopback_address.is_v4_compatible());
  ASIO_CHECK(!link_local_address.is_v4_compatible());
  ASIO_CHECK(!site_local_address.is_v4_compatible());
  ASIO_CHECK(!v4_mapped_address.is_v4_compatible());
  ASIO_CHECK(v4_compat_address.is_v4_compatible());
  ASIO_CHECK(!mcast_global_address.is_v4_compatible());
  ASIO_CHECK(!mcast_link_local_address.is_v4_compatible());
  ASIO_CHECK(!mcast_node_local_address.is_v4_compatible());
  ASIO_CHECK(!mcast_org_local_address.is_v4_compatible());
  ASIO_CHECK(!mcast_site_local_address.is_v4_compatible());
#endif // !defined(ASIO_NO_DEPRECATED)

  ASIO_CHECK(!unspecified_address.is_multicast());
  ASIO_CHECK(!loopback_address.is_multicast());
  ASIO_CHECK(!link_local_address.is_multicast());
  ASIO_CHECK(!site_local_address.is_multicast());
  ASIO_CHECK(!v4_mapped_address.is_multicast());
  ASIO_CHECK(!v4_compat_address.is_multicast());
  ASIO_CHECK(mcast_global_address.is_multicast());
  ASIO_CHECK(mcast_link_local_address.is_multicast());
  ASIO_CHECK(mcast_node_local_address.is_multicast());
  ASIO_CHECK(mcast_org_local_address.is_multicast());
  ASIO_CHECK(mcast_site_local_address.is_multicast());

  ASIO_CHECK(!unspecified_address.is_multicast_global());
  ASIO_CHECK(!loopback_address.is_multicast_global());
  ASIO_CHECK(!link_local_address.is_multicast_global());
  ASIO_CHECK(!site_local_address.is_multicast_global());
  ASIO_CHECK(!v4_mapped_address.is_multicast_global());
  ASIO_CHECK(!v4_compat_address.is_multicast_global());
  ASIO_CHECK(mcast_global_address.is_multicast_global());
  ASIO_CHECK(!mcast_link_local_address.is_multicast_global());
  ASIO_CHECK(!mcast_node_local_address.is_multicast_global());
  ASIO_CHECK(!mcast_org_local_address.is_multicast_global());
  ASIO_CHECK(!mcast_site_local_address.is_multicast_global());

  ASIO_CHECK(!unspecified_address.is_multicast_link_local());
  ASIO_CHECK(!loopback_address.is_multicast_link_local());
  ASIO_CHECK(!link_local_address.is_multicast_link_local());
  ASIO_CHECK(!site_local_address.is_multicast_link_local());
  ASIO_CHECK(!v4_mapped_address.is_multicast_link_local());
  ASIO_CHECK(!v4_compat_address.is_multicast_link_local());
  ASIO_CHECK(!mcast_global_address.is_multicast_link_local());
  ASIO_CHECK(mcast_link_local_address.is_multicast_link_local());
  ASIO_CHECK(!mcast_node_local_address.is_multicast_link_local());
  ASIO_CHECK(!mcast_org_local_address.is_multicast_link_local());
  ASIO_CHECK(!mcast_site_local_address.is_multicast_link_local());

  ASIO_CHECK(!unspecified_address.is_multicast_node_local());
  ASIO_CHECK(!loopback_address.is_multicast_node_local());
  ASIO_CHECK(!link_local_address.is_multicast_node_local());
  ASIO_CHECK(!site_local_address.is_multicast_node_local());
  ASIO_CHECK(!v4_mapped_address.is_multicast_node_local());
  ASIO_CHECK(!v4_compat_address.is_multicast_node_local());
  ASIO_CHECK(!mcast_global_address.is_multicast_node_local());
  ASIO_CHECK(!mcast_link_local_address.is_multicast_node_local());
  ASIO_CHECK(mcast_node_local_address.is_multicast_node_local());
  ASIO_CHECK(!mcast_org_local_address.is_multicast_node_local());
  ASIO_CHECK(!mcast_site_local_address.is_multicast_node_local());

  ASIO_CHECK(!unspecified_address.is_multicast_org_local());
  ASIO_CHECK(!loopback_address.is_multicast_org_local());
  ASIO_CHECK(!link_local_address.is_multicast_org_local());
  ASIO_CHECK(!site_local_address.is_multicast_org_local());
  ASIO_CHECK(!v4_mapped_address.is_multicast_org_local());
  ASIO_CHECK(!v4_compat_address.is_multicast_org_local());
  ASIO_CHECK(!mcast_global_address.is_multicast_org_local());
  ASIO_CHECK(!mcast_link_local_address.is_multicast_org_local());
  ASIO_CHECK(!mcast_node_local_address.is_multicast_org_local());
  ASIO_CHECK(mcast_org_local_address.is_multicast_org_local());
  ASIO_CHECK(!mcast_site_local_address.is_multicast_org_local());

  ASIO_CHECK(!unspecified_address.is_multicast_site_local());
  ASIO_CHECK(!loopback_address.is_multicast_site_local());
  ASIO_CHECK(!link_local_address.is_multicast_site_local());
  ASIO_CHECK(!site_local_address.is_multicast_site_local());
  ASIO_CHECK(!v4_mapped_address.is_multicast_site_local());
  ASIO_CHECK(!v4_compat_address.is_multicast_site_local());
  ASIO_CHECK(!mcast_global_address.is_multicast_site_local());
  ASIO_CHECK(!mcast_link_local_address.is_multicast_site_local());
  ASIO_CHECK(!mcast_node_local_address.is_multicast_site_local());
  ASIO_CHECK(!mcast_org_local_address.is_multicast_site_local());
  ASIO_CHECK(mcast_site_local_address.is_multicast_site_local());

  ASIO_CHECK(address_v6::loopback().is_loopback());
}

} // namespace ip_address_v6_runtime

//------------------------------------------------------------------------------

ASIO_TEST_SUITE
(
  "ip/address_v6",
  ASIO_TEST_CASE(ip_address_v6_compile::test)
  ASIO_TEST_CASE(ip_address_v6_runtime::test)
)
