// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for 80186-based VMEbus controller by Colex, Inc.

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
//#include "bus/vme/vme.h"
#include "machine/74259.h"
#include "machine/m3002.h"
#include "machine/z80scc.h"


namespace {

class clxvme186_state : public driver_device
{
public:
	clxvme186_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void clxvme186(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void unknown_w(u16 data);
	u8 sasi_status_r();

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<i80186_cpu_device> m_maincpu;
};


void clxvme186_state::machine_start()
{
}


void clxvme186_state::unknown_w(u16 data)
{
}

u8 clxvme186_state::sasi_status_r()
{
	return 0;
}

void clxvme186_state::mem_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram(); // On-board RAM
	map(0x40000, 0xbffff).unmaprw(); // Off-board memory
	map(0xc0000, 0xc7fff).unmapr(); // Vector interrupt input
	map(0xc8000, 0xcffff).unmaprw(); // VMEbus short I/O
	map(0xf0000, 0xf7fff).mirror(0x8000).rom().region("eprom", 0); // On-board EPROM
}

void clxvme186_state::io_map(address_map &map)
{
	map(0xe000, 0xe001).unmaprw(); // SASI data port (PCS0)
	map(0xe080, 0xe087).rw("scc", FUNC(scc8530_device::ab_dc_r), FUNC(scc8530_device::ab_dc_w)).umask16(0x00ff); // Serial I/O ports A & B (PCS1)
	map(0xe100, 0xe100).rw("rtc", FUNC(m3000_device::read), FUNC(m3000_device::write)); // Real time clock (PCS2)
	map(0xe180, 0xe181).portr("TTL"); // TTL input port (PCS3)
	map(0xe180, 0xe181).w(FUNC(clxvme186_state::unknown_w));
	map(0xe190, 0xe190).r(FUNC(clxvme186_state::sasi_status_r)); // SASI status port (PCS3)
	map(0xe190, 0xe19f).w("ctrllatch", FUNC(ls259_device::write_d0)).umask16(0x00ff); // Control ports (PCS3)
	map(0xe200, 0xe207).unmapw(); // Register files (PCS4)
	map(0xe280, 0xe281).unmapw(); // Printer data port (PCS5)
}


static INPUT_PORTS_START(clxvme186)
	PORT_START("TTL")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN) // P6 X data
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN) // P6 Y data
	PORT_BIT(0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN) // P6 M data
	PORT_BIT(0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN) // P6 D data
	PORT_BIT(0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN) // P6 G data
	PORT_BIT(0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN) // Printer busy
	PORT_BIT(0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN) // Printer paper empty
	PORT_BIT(0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN) // /SYSFAIL
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("rtc", m3000_device, busy_r)
	PORT_BIT(0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN) // Colex use only
	PORT_BIT(0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN) // Colex use only
	PORT_BIT(0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN) // /ACFAIL
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN) // TTL input bit P2 pin 7c
	PORT_BIT(0xe000, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


void clxvme186_state::clxvme186(machine_config &config)
{
	I80186(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &clxvme186_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &clxvme186_state::io_map);

	SCC8530N(config, "scc", 3.6864_MHz_XTAL);

	M3000(config, "rtc", 32.768_kHz_XTAL);

	LS259(config, "ctrllatch");
}


ROM_START(clxvme186)
	ROM_REGION16_LE(0x8000, "eprom", 0)
	ROM_LOAD16_BYTE("clxu37.bin", 0x0000, 0x4000, CRC(9308130c) SHA1(a0230f8e9943a0c15cac365d4c58614e7befde59))
	ROM_LOAD16_BYTE("clxu53.bin", 0x0001, 0x4000, CRC(66bf7cc5) SHA1(cfe8df9260b565840249256a30631a34e931c886))
	ROM_IGNORE(0x4000)
ROM_END

} // anonymous namespace


COMP(1983, clxvme186, 0, 0, clxvme186, clxvme186, clxvme186_state, empty_init, "Colex", "Colex VME-80186", MACHINE_IS_SKELETON)
