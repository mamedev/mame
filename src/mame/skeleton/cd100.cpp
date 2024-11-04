// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Rowe CD-100 jukebox series.

    Communication between the Central Control Computer and the Mech
    Controller is handled through an unemulated serial link (Z180 emulation
    improvements are needed to handle this).

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6801.h"
#include "cpu/z180/z180.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"


namespace {

class cd100_state : public driver_device
{
public:
	cd100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mechcpu(*this, "mechcpu")
		, m_switches(*this, "IN%d", 0U)
		, m_misc(*this, "MISC")
		, m_link(*this, "LINK")
		, m_strobe(0x3f)
		, m_mech_p1(0)
	{
	}

	void cd100b(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u8 ccc_in_r();
	void ccc_out0_w(u8 data);
	void ccc_out1_w(u8 data);
	u8 mech_p1_r();
	void mech_p1_w(u8 data);
	u8 mech_p2_r();
	void mech_p2_w(u8 data);
	void mech_latch_w(u8 data);

	void ccc_mem_map(address_map &map) ATTR_COLD;
	void ccc_io_map(address_map &map) ATTR_COLD;
	void mc6803_map(address_map &map) ATTR_COLD;

	required_device<z180_device> m_maincpu;
	required_device<m6803_cpu_device> m_mechcpu;
	required_ioport_array<6> m_switches;
	required_ioport m_misc;
	required_ioport m_link;

	u8 m_strobe;
	u8 m_mech_p1;
};


void cd100_state::machine_start()
{
	save_item(NAME(m_strobe));
}

u8 cd100_state::ccc_in_r()
{
	u8 ret = 0x0f;

	for (int i = 0; i < 6; i++)
		if (BIT(m_strobe, i))
			ret &= m_switches[i]->read();

	return ret | m_misc->read() << 4;
}

void cd100_state::ccc_out0_w(u8 data)
{
	m_strobe = data & 0x3f;

	// D6 (STROBE6): Not used
	// D7 (STROBE7): Background music active
}

void cd100_state::ccc_out1_w(u8 data)
{
	// D0 (STROBE8): Display reset
	// D1 (STROBE9): Speed info to motor chip
	// D2 (STROBE10): Direction info to motor chip
	// D3: Mute
	// D4: RoweLink Tx/Rx select
	// D5: System error LED
	// D6: Board error LED
	// D7: Watchdog strobe
}

u8 cd100_state::mech_p1_r()
{
	u8 ret = 0;

	// P10: IIC latched data Rxed
	// P11: IIC bit trapped
	// P12: 74HCT151 Y return (Z12)
	// P13: 74HCT151 Y return (Z17)
	if (BIT(m_link->read(), (m_mech_p1 & 0xe0) >> 5))
		ret |= 0x08;

	return ret;
}

void cd100_state::mech_p1_w(u8 data)
{
	// P14: Board error LED
	// P15: 74HCT151 A select (Z12, Z17)
	// P16: 74HCT151 B select (Z12, Z17)
	// P17: 74HCT151 C select (Z12, Z17)
	m_mech_p1 = data;
}

u8 cd100_state::mech_p2_r()
{
	// P21: +5VDC
	// P22: GND
	// P23: System Rx
	return 0x02;
}

void cd100_state::mech_p2_w(u8 data)
{
	// P20: IIC enable
	// P24: System Tx
}

void cd100_state::mech_latch_w(u8 data)
{
	// Q0: Money counter
	// Q1: Play counter
	// Q2: Magazine motor
	// Q3: Transfer motor
	// Q4: Detent solenoid
	// Q5: System Tx/Rx
	// Q6: Audio mute
	// Q7: IIC data Tx
}

void cd100_state::ccc_mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom().region("ccc_program", 0);
	map(0x20000, 0x21fff).ram().share("nvram");
}

void cd100_state::ccc_io_map(address_map &map)
{
	map(0x0000, 0x003f).noprw(); // internal registers
	map(0x2000, 0x200f).rw("rtc", FUNC(rtc72421_device::read), FUNC(rtc72421_device::write));
	map(0x6000, 0x6000).w(FUNC(cd100_state::ccc_out1_w));
	map(0x8000, 0x8000).w(FUNC(cd100_state::ccc_out0_w));
	map(0xa000, 0xa000).r(FUNC(cd100_state::ccc_in_r));
}

void cd100_state::mc6803_map(address_map &map)
{
	map(0x2000, 0x27ff).ram();
	map(0x4000, 0x4000).w(FUNC(cd100_state::mech_latch_w));
	// TODO: CD-100B has more peripheral stuff in the 60XX range, not on CD-100A schematics?
	map(0x8000, 0xffff).rom().region("mech_program", 0);
}


static INPUT_PORTS_START(cd100b)
	PORT_START("IN0")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_COIN1) // 5¢
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_COIN2) // 10¢
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_COIN3) // 25¢
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_COIN4) // 50¢

	PORT_START("IN1")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN) // Tile Disp Limit
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN) // Tile Disp Index
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNKNOWN) // Reserved
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN) // UK Defaults

	PORT_START("IN2")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN) // Key 0
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN) // Key 1
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNKNOWN) // Key 2
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN) // Key 3

	PORT_START("IN3")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN) // Key 4
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN) // Key 5
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNKNOWN) // Key 6
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN) // Key 7

	PORT_START("IN4")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN) // Key 8
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN) // Key 9
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN) // Audit report start

	PORT_START("IN5")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN) // "POPULAR"
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN) // "RESET"
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_UNKNOWN) // "OUT"
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN) // "IN"

	PORT_START("MISC")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_UNKNOWN) // "Cancel Switch"
	PORT_BIT(0x2, IP_ACTIVE_LOW, IPT_UNKNOWN) // "Display Attention"
	PORT_BIT(0x4, IP_ACTIVE_LOW, IPT_SERVICE)
	PORT_BIT(0x8, IP_ACTIVE_LOW, IPT_UNKNOWN) // "Bad Battery"

	PORT_START("LINK")
	PORT_DIPNAME(0xe0, 0xe0, "RoweLink Address") PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(0x00, "0")
	PORT_DIPSETTING(0x20, "1")
	PORT_DIPSETTING(0x40, "2")
	PORT_DIPSETTING(0x60, "3")
	PORT_DIPSETTING(0x80, "4")
	PORT_DIPSETTING(0xa0, "5")
	PORT_DIPSETTING(0xc0, "6")
	PORT_DIPSETTING(0xe0, "7")
INPUT_PORTS_END


void cd100_state::cd100b(machine_config &config)
{
	HD64180RP(config, m_maincpu, 12.288_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &cd100_state::ccc_mem_map);
	m_maincpu->set_addrmap(AS_IO, &cd100_state::ccc_io_map);
	// TODO: ASCI channel 0 for peripheral interface (RS232C, through MAX232)
	// TODO: ASCI channel 1 for RoweLink (RS485, through SN75176)
	// TODO: CSIO for display (RS422, through DS8923)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 6264/62256 + same battery as RTC

	RTC72421(config, "rtc", 32768).out_int_handler().set_inputline(m_maincpu, Z180_INPUT_LINE_IRQ2);

	M6803(config, m_mechcpu, 4.9152_MHz_XTAL);
	m_mechcpu->set_addrmap(AS_PROGRAM, &cd100_state::mc6803_map);
	m_mechcpu->in_p1_cb().set(FUNC(cd100_state::mech_p1_r));
	m_mechcpu->out_p1_cb().set(FUNC(cd100_state::mech_p1_w));
	m_mechcpu->in_p2_cb().set(FUNC(cd100_state::mech_p2_r));
	m_mechcpu->out_p2_cb().set(FUNC(cd100_state::mech_p2_w));
}


ROM_START(cd100b)
	ROM_REGION(0x20000, "ccc_program", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS(0, "v27", "Version 2.7")
	ROM_SYSTEM_BIOS(1, "v30", "Version 3.0")
	ROM_SYSTEM_BIOS(2, "v40", "Version 4.0")
	ROM_SYSTEM_BIOS(3, "v41", "Version 4.1")
	ROMX_LOAD("70039903.v27", 0x00000, 0x10000, CRC(2aa0c0dc) SHA1(f0be4f7c26b9e798790c169370c8dc2d2c2edbbb), ROM_BIOS(0))
	ROMX_LOAD("70042704.v30", 0x00000, 0x20000, CRC(3e1af9ac) SHA1(2ee4ff59963270fa34ff61f0428bd3d1a01a743a), ROM_BIOS(1))
	ROMX_LOAD("70042704.v40", 0x00000, 0x20000, CRC(66ba3ede) SHA1(22d9ee14fadef81b1ed7d95c1432d235bea5eba7), ROM_BIOS(2))
	ROMX_LOAD("jukeccc",      0x00000, 0x20000, CRC(416a9dd5) SHA1(83fcd73092792b1f0d391e80d81ab8791c4c8e39), ROM_BIOS(3))

	ROM_REGION(0x8000, "mech_program", 0)
	ROM_LOAD("70038325.31", 0x0000, 0x8000, CRC(6c46039a) SHA1(e4410180d94e9a60ddc8f42296ee60be20b28b5b)) // "CDM4 Mech Controller version 3.1"
ROM_END

} // anonymous namespace


SYST(1992, cd100b, 0, 0, cd100b, cd100b, cd100_state, empty_init, "Rowe International", "CD-100B LaserStar", MACHINE_IS_SKELETON_MECHANICAL)
