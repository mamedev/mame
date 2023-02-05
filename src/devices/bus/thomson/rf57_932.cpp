// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Thomson RF 57-932 RS-232-C extension interface

***************************************************************************/

#include "emu.h"
#include "rf57_932.h"

#include "bus/rs232/rs232.h"
#include "machine/mos6551.h"

// device type definition
DEFINE_DEVICE_TYPE(RF57_932, rf57_932_device, "rf57_932", "Thomson RF 57-932 RS-232-C Interface")

//-------------------------------------------------
//  rf57_932_device - constructor
//-------------------------------------------------

rf57_932_device::rf57_932_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, RF57_932, tag, owner, clock)
	, thomson_extension_interface(mconfig, *this)
{
}

void rf57_932_device::rom_map(address_map &map)
{
}

void rf57_932_device::io_map(address_map &map)
{
	map(0x28, 0x2b).rw("acia", FUNC(mos6551_device::read), FUNC(mos6551_device::write));
}

void rf57_932_device::device_add_mconfig(machine_config &config)
{
	mos6551_device &acia(MOS6551(config, "acia", DERIVED_CLOCK(1, 1)));
	acia.set_xtal(1.8432_MHz_XTAL);
	acia.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	acia.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	acia.irq_handler().set(FUNC(rf57_932_device::irq_w));
	acia.write_dcd(0);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("acia", FUNC(mos6551_device::write_rxd));
	rs232.dsr_handler().set("acia", FUNC(mos6551_device::write_dsr));
	rs232.cts_handler().set("acia", FUNC(mos6551_device::write_cts));
	rs232.rxc_handler().set("acia", FUNC(mos6551_device::write_rxc));
}

void rf57_932_device::device_start()
{
}
