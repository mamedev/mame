// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the National Semiconductor DP83932C SONIC™ (Systems-
 * Oriented Network Interface Controller) device.
 *
 * References:
 *
 *   http://bitsavers.org/components/national/_dataBooks/1995_National_Ethernet_Databook.pdf
 *
 * TODO
 *   - everything (skeleton only)
 */

#include "emu.h"
#include "dp83932c.h"
#include "hashing.h"

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(DP83932C, dp83932c_device, "dp83932c", "National Semiconductor DP83932C SONIC")

dp83932c_device::dp83932c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DP83932C, tag, owner, clock)
	, device_network_interface(mconfig, *this, 10.0f)
	, m_ram(*this, finder_base::DUMMY_TAG)
	, m_out_int(*this)
{
}

void dp83932c_device::map(address_map &map)
{
	// datasheet uses unshifted register addresses
	int const shift = 1;

	// command and status registers
	map(0x00 << shift, (0x00 << shift) | 0x01).rw(FUNC(dp83932c_device::cr_r), FUNC(dp83932c_device::cr_w));
	//map(0x01 << shift, (0x01 << shift) | 0x01).rw(FUNC(dp83932c_device::dcr_r), FUNC(dp83932c_device::dcr_w));
	//map(0x02 << shift, (0x02 << shift) | 0x01).rw(FUNC(dp83932c_device::rcr_r), FUNC(dp83932c_device::rcr_w));
	map(0x03 << shift, (0x03 << shift) | 0x01).rw(FUNC(dp83932c_device::tcr_r), FUNC(dp83932c_device::tcr_w));
	//map(0x04 << shift, (0x04 << shift) | 0x01).rw(FUNC(dp83932c_device::imr_r), FUNC(dp83932c_device::imr_w));
	map(0x05 << shift, (0x05 << shift) | 0x01).rw(FUNC(dp83932c_device::isr_r), FUNC(dp83932c_device::isr_w));

	// transmit registers
	map(0x06 << shift, (0x06 << shift) | 0x01).rw(FUNC(dp83932c_device::utda_r), FUNC(dp83932c_device::utda_w));
	//map(0x07 << shift, (0x07 << shift) | 0x01).rw(FUNC(dp83932c_device::ctda_r), FUNC(dp83932c_device::ctda_w));

	// tps
	// tfc
	// tsa0
	// tsa1
	// tfs

	// receive registers
	//map(0x0d << shift, (0x0d << shift) | 0x01).rw(FUNC(dp83932c_device::urda_r), FUNC(dp83932c_device::urda_w));
	map(0x0e << shift, (0x0e << shift) | 0x01).rw(FUNC(dp83932c_device::crda_r), FUNC(dp83932c_device::crda_w));

	// crba0
	// crba1
	// rbwc0
	// rbwc1
	// eobc
	// urra
	// rsa
	// rea
	map(0x17 << shift, (0x17 << shift) | 0x01).rw(FUNC(dp83932c_device::rrp_r), FUNC(dp83932c_device::rrp_w));
	map(0x18 << shift, (0x18 << shift) | 0x01).rw(FUNC(dp83932c_device::rwp_r), FUNC(dp83932c_device::rwp_w));
	// trba0
	// trba1
	// tbwc0
	// tbwc1
	// addr0
	// addr1
	// llfa
	// ttda
	// cep
	// cap2
	// cap1
	// cap0
	map(0x25 << shift, (0x25 << shift) | 0x01).rw(FUNC(dp83932c_device::ce_r), FUNC(dp83932c_device::ce_w));
	// cdp
	// cdc
	// sr
	map(0x29 << shift, (0x29 << shift) | 0x01).rw(FUNC(dp83932c_device::wt0_r), FUNC(dp83932c_device::wt0_w));
	map(0x2a << shift, (0x2a << shift) | 0x01).rw(FUNC(dp83932c_device::wt1_r), FUNC(dp83932c_device::wt1_w));
	map(0x2b << shift, (0x2b << shift) | 0x01).rw(FUNC(dp83932c_device::rsc_r), FUNC(dp83932c_device::rsc_w));
	// crct
	map(0x2d << shift, (0x2d << shift) | 0x01).rw(FUNC(dp83932c_device::faet_r), FUNC(dp83932c_device::faet_w));
	// mpt
	// mdt

	// 30-3e internal use registers
	// dcr2
}

void dp83932c_device::device_start()
{
	m_out_int.resolve();
}

void dp83932c_device::device_reset()
{
	m_cr = RST | STP | RXDIS;
	m_tcr = NCRS | PTX;
	m_isr = 0;
	m_ce = 0;
	m_rsc = 0;
}

void dp83932c_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
}

void dp83932c_device::send_complete_cb(int result)
{
}

int dp83932c_device::recv_start_cb(u8 *buf, int length)
{
	return 0;
}

void dp83932c_device::recv_complete_cb(int result)
{
}
