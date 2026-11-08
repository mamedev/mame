// license:BSD-3-Clause
// copyright-holders:Carl

#include "netdev_common.h"

#include "util/hashing.h"

#include <algorithm>


namespace osd {

namespace {

// Ethernet minimum frame length
constexpr int ETHERNET_MIN_FRAME = 64;

} // anonymous namespace

network_device_base::network_device_base(network_handler &handler)
	: m_handler(handler)
	, m_stopped(true)
{
}

network_device_base::~network_device_base()
{
}

void network_device_base::start()
{
	m_stopped = false;
}

void network_device_base::stop()
{
	m_stopped = true;
}

void network_device_base::poll()
{
	uint8_t *buf;
	int len;
	while (!m_stopped && (len = recv_dev(&buf)))
	{
		m_handler.recv_cb(buf, len);
	}
}

int network_device_base::finalise_frame(uint8_t buf[], int length)
{
	/*
	 * The host network driver receives frames which are shorter than the Ethernet
	 * minimum. Partly this is because it can't see the frame check sequence
	 * bytes, but mainly it's because the OS expects the lower level device
	 * to add the required padding.
	 *
	 * We do the equivalent padding here (i.e. pad with zeroes to the
	 * minimum Ethernet length minus FCS), so that devices which check
	 * for this will not reject these packets.
	 */
	if (length < ETHERNET_MIN_FRAME - 4)
	{
		std::fill_n(&buf[length], ETHERNET_MIN_FRAME - length - 4, 0);

		length = ETHERNET_MIN_FRAME - 4;
	}

	// compute and append the frame check sequence
	const u32 fcs = util::crc32_creator::simple(buf, length);

	buf[length++] = (fcs >> 0) & 0xff;
	buf[length++] = (fcs >> 8) & 0xff;
	buf[length++] = (fcs >> 16) & 0xff;
	buf[length++] = (fcs >> 24) & 0xff;

	return length;
}

} // namespace osd
