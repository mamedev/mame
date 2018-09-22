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

DEFINE_DEVICE_TYPE(DP83932C_BE, dp83932c_be_device, "dp83932c_be", "National Semiconductor DP83932C SONIC (big)")
DEFINE_DEVICE_TYPE(DP83932C_LE, dp83932c_le_device, "dp83932c_le", "National Semiconductor DP83932C SONIC (little)")

dp83932c_device::dp83932c_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, endianness_t endian)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_network_interface(mconfig, *this, 10.0f)
	, m_space_config("shared", endian, 32, 32)
	, m_out_int(*this)
{
}

dp83932c_be_device::dp83932c_be_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dp83932c_device(mconfig, DP83932C_BE, tag, owner, clock, ENDIANNESS_BIG)
{
}

dp83932c_le_device::dp83932c_le_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: dp83932c_device(mconfig, DP83932C_LE, tag, owner, clock, ENDIANNESS_LITTLE)
{
}

void dp83932c_device::map(address_map &map)
{
/*
    // command and status registers
    map(0x00, 0x03).rw(FUNC(dp83932c_device::cr_r), FUNC(dp83932c_device::cr_w));
    map(0x04, 0x07).rw(FUNC(dp83932c_device::dcr_r), FUNC(dp83932c_device::dcr_w));
    map(0x08, 0x0b).rw(FUNC(dp83932c_device::rcr_r), FUNC(dp83932c_device::rcr_w));
    map(0x0c, 0x0f).rw(FUNC(dp83932c_device::tcr_r), FUNC(dp83932c_device::tcr_w));
    map(0x10, 0x13).rw(FUNC(dp83932c_device::imr_r), FUNC(dp83932c_device::imr_w));
    map(0x14, 0x17).rw(FUNC(dp83932c_device::isr_r), FUNC(dp83932c_device::isr_w));

    // transmit registers
    map(0x18, 0x1b).rw(FUNC(dp83932c_device::utda_r), FUNC(dp83932c_device::utda_w));
    map(0x1c, 0x1f).rw(FUNC(dp83932c_device::ctda_r), FUNC(dp83932c_device::ctda_w));

    // tps
    // tfc
    // tsa0
    // tsa1
    // tfs

    // receive registers
    map(0x34, 0x37).rw(FUNC(dp83932c_device::urda_r), FUNC(dp83932c_device::urda_w));
    map(0x38, 0x3b).rw(FUNC(dp83932c_device::crda_r), FUNC(dp83932c_device::crda_w));

    // crba0
    // crba1
    // rbwc0
    // rbwc1
    // eobc
    // urra
    // rsa
    // rea
    // rrp
    // rwp
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
    // ce
    // cdp
    // cdc
    // sr
    // wt0
    // wt1
    // rsc
    // crct
    // faet
    // mpt
    // mdt

    // 30-3e internal use registers
*/
}

void dp83932c_device::device_start()
{
	m_space = &space(0);
	m_out_int.resolve();
}

void dp83932c_device::device_reset()
{
}

void dp83932c_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
}

device_memory_interface::space_config_vector dp83932c_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
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
