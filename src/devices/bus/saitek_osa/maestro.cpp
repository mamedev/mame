// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Saitek OSA Module: Kasparov Maestro/Analyst (1987-1990)
This is for the newer versions. For Maestro A, see maestroa.*

The hardware and chess engine is similar to the Stratos/Turbo King series.

Version B is compatible with the 1st EGR expansion ROM, version C/D uses
EGR II. Versions D+ and D++ are post-production improvements, they were not
sold officially.

Hardware notes:
- CPU: see notes below
- 64KB ROM (2*27C256)
- 16KB RAM (2*HY62C64P-70), one of them is optional
- 1 ROM socket for EGR expansion ROM

Analyst adds a HD44780A00H and a small 16-char LCD screen.

Both were sold at 4MHz, 6MHz, 8MHz speeds, Maestro also had a 10MHz version.
CPUs used were: R65C02P4, RP65C02F, W65C02P-8. In almost all cases with the
higher speed versions, they overclocked the CPU. And if the CPU couldn't
handle the overclock well enough, they went for a slightly lower speed XTAL.

TODO:
- cpu clock divider after writing to 0x2000/0x2200

*******************************************************************************/

#include "emu.h"
#include "maestro.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/m6502/r65c02.h"
#include "video/hd44780.h"

#include "softlist_dev.h"

namespace {

//-------------------------------------------------
//  initialization
//-------------------------------------------------

// Maestro / shared

class saitekosa_maestro_device : public device_t, public device_saitekosa_expansion_interface
{
public:
	// construction/destruction
	saitekosa_maestro_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	DECLARE_INPUT_CHANGED_MEMBER(change_cpu_freq);

	// from host
	virtual u8 data_r() override;
	virtual void nmi_w(int state) override;
	virtual void ack_w(int state) override;

protected:
	saitekosa_maestro_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	memory_share_creator<u8> m_banked_ram;
	required_memory_bank m_rambank;
	required_memory_bank m_rombank;
	required_device<generic_slot_device> m_extrom;

	u8 m_latch = 0xff;
	bool m_latch_enable = false;
	u8 m_extrom_bank = 0;

	virtual void main_map(address_map &map) ATTR_COLD;

	u8 extrom_r(offs_t offset);
	template <int N> void stall_w(u8 data = 0);
	u8 rts_r();
	u8 xdata_r();
	void xdata_w(u8 data);
	u8 ack_r();
	void control_w(u8 data);
};

saitekosa_maestro_device::saitekosa_maestro_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_saitekosa_expansion_interface(mconfig, *this),
	m_maincpu(*this, "maincpu"),
	m_banked_ram(*this, "banked_ram", 0x2000, ENDIANNESS_LITTLE),
	m_rambank(*this, "rambank"),
	m_rombank(*this, "rombank"),
	m_extrom(*this, "extrom")
{ }

saitekosa_maestro_device::saitekosa_maestro_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	saitekosa_maestro_device(mconfig, OSA_MAESTRO, tag, owner, clock)
{ }


// Analyst

class saitekosa_analyst_device : public saitekosa_maestro_device
{
public:
	saitekosa_analyst_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;

	static auto parent_rom_device_type() { return &OSA_MAESTRO; }

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<hd44780_device> m_lcd;

	virtual void main_map(address_map &map) override ATTR_COLD;
};

saitekosa_analyst_device::saitekosa_analyst_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	saitekosa_maestro_device(mconfig, OSA_ANALYST, tag, owner, clock),
	m_lcd(*this, "lcd")
{ }


void saitekosa_maestro_device::device_start()
{
	// init banks
	m_rombank->configure_entries(0, 2, memregion("maincpu")->base(), 0x8000);
	m_rambank->configure_entries(0, 2, m_banked_ram, 0x1000);

	// register for savestates
	save_item(NAME(m_latch_enable));
	save_item(NAME(m_latch));
	save_item(NAME(m_extrom_bank));
}

void saitekosa_maestro_device::device_reset()
{
	control_w(0);
}

INPUT_CHANGED_MEMBER(saitekosa_maestro_device::change_cpu_freq)
{
	static const XTAL xtal[6] = { 4_MHz_XTAL, 5.67_MHz_XTAL, 6_MHz_XTAL, 7.2_MHz_XTAL, 8_MHz_XTAL, 10_MHz_XTAL };
	m_maincpu->set_unscaled_clock(xtal[newval % 6]);
}


//-------------------------------------------------
//  host i/o
//-------------------------------------------------

u8 saitekosa_maestro_device::data_r()
{
	return m_latch_enable ? m_latch : 0xff;
}

void saitekosa_maestro_device::nmi_w(int state)
{
	m_maincpu->set_input_line(0, !state ? ASSERT_LINE : CLEAR_LINE);
}

void saitekosa_maestro_device::ack_w(int state)
{
	if (state != m_expansion->ack_state())
		machine().scheduler().perfect_quantum(attotime::from_usec(100));
}

u32 saitekosa_analyst_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0xffffff, cliprect);
	const u8 *render = m_lcd->render();

	// draw lcd characters
	for (int i = 0; i < 16; i++)
	{
		const u8 *src = render + 16 * ((i & 7) + BIT(i, 3) * 40);
		for (int y = 0; y < 8; y++)
			for (int x = 0; x < 5; x++)
				bitmap.pix(y + 4, i * 6 + x + 2) = (BIT(src[y], 4 - x) && m_expansion->pw_state()) ? 0x282828 : 0xe8e8e8;
	}

	return 0;
}


//-------------------------------------------------
//  internal i/o
//-------------------------------------------------

u8 saitekosa_maestro_device::extrom_r(offs_t offset)
{
	u16 bank = m_extrom_bank * 0x4000;
	return (m_extrom->exists()) ? m_extrom->read_rom(offset | bank) : 0xff;
}

template <int N> void saitekosa_maestro_device::stall_w(u8 data)
{
	// cpu clock divider
}

u8 saitekosa_maestro_device::rts_r()
{
	if (!machine().side_effects_disabled())
	{
		// strobe RTS-P
		m_expansion->rts_w(1);
		m_expansion->rts_w(0);
	}

	return 0xff;
}

void saitekosa_maestro_device::xdata_w(u8 data)
{
	// clock latch
	m_latch = data;
}

u8 saitekosa_maestro_device::xdata_r()
{
	return m_expansion->data_state();
}

void saitekosa_maestro_device::control_w(u8 data)
{
	// d0: main rom bank
	m_rombank->set_entry(data & 1);

	// d1: ext rom bank
	// d1: ram bank
	m_extrom_bank = BIT(data, 1);
	m_rambank->set_entry(m_extrom_bank);

	// d3: enable latch output
	m_latch_enable = bool(data & 8);

	// d2: STB-P
	m_expansion->stb_w(BIT(data, 2));
}

u8 saitekosa_maestro_device::ack_r()
{
	// d6: _Vcc
	// d7: ACK-P
	return m_expansion->ack_state() ? 0x80 : 0x00;
}

void saitekosa_maestro_device::main_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2000).mirror(0x01ff).w(FUNC(saitekosa_maestro_device::stall_w<0>));
	map(0x2200, 0x2200).mirror(0x01ff).rw(FUNC(saitekosa_maestro_device::rts_r), FUNC(saitekosa_maestro_device::stall_w<1>));
	map(0x2400, 0x2400).mirror(0x01ff).rw(FUNC(saitekosa_maestro_device::xdata_r), FUNC(saitekosa_maestro_device::xdata_w));
	map(0x2600, 0x2600).mirror(0x01ff).rw(FUNC(saitekosa_maestro_device::ack_r), FUNC(saitekosa_maestro_device::control_w));
	map(0x2800, 0x37ff).bankrw("rambank");
	map(0x4000, 0x7fff).r(FUNC(saitekosa_maestro_device::extrom_r));
	map(0x8000, 0xffff).bankr("rombank");
}

void saitekosa_analyst_device::main_map(address_map &map)
{
	saitekosa_maestro_device::main_map(map);
	map(0x3800, 0x3801).mirror(0x07fe).rw(m_lcd, FUNC(hd44780_device::read), FUNC(hd44780_device::write));
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( maestro )
	PORT_START("CPU")
	PORT_CONFNAME( 0x07, 0x04, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, saitekosa_maestro_device, change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "4MHz" )
	PORT_CONFSETTING(    0x01, "5.67MHz" )
	PORT_CONFSETTING(    0x02, "6MHz" )
	PORT_CONFSETTING(    0x03, "7.2MHz" )
	PORT_CONFSETTING(    0x04, "8MHz" )
	PORT_CONFSETTING(    0x05, "10MHz" )
INPUT_PORTS_END

ioport_constructor saitekosa_maestro_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(maestro);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void saitekosa_maestro_device::device_add_mconfig(machine_config &config)
{
	// basic machine hardware
	R65C02(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &saitekosa_maestro_device::main_map);

	// extension rom
	GENERIC_SOCKET(config, "extrom", generic_plain_slot, "saitek_egr");
	SOFTWARE_LIST(config, "cart_list").set_original("saitek_egr");
}

void saitekosa_analyst_device::device_add_mconfig(machine_config &config)
{
	saitekosa_maestro_device::device_add_mconfig(config);

	// video hardware
	HD44780(config, m_lcd, 270'000); // OSC = 91K resistor
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( maestro )
	ROM_REGION(0x10000, "maincpu", 0)

	ROM_DEFAULT_BIOS("d1")

	// B (Maestro only)
	ROM_SYSTEM_BIOS(0, "b1", "Maestro B (set 1)")
	ROMX_LOAD("m6c_807e_u2.u2", 0x0000, 0x8000, CRC(a6b11715) SHA1(18e086353d9122034f78bcd75ef5b3462c5983ac), ROM_BIOS(0))
	ROMX_LOAD("b6c_721_u3.u3",  0x8000, 0x8000, CRC(b1e57023) SHA1(6cec5cdc0bf4f8ac88afb0397fcb4738136b0431), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "b2", "Maestro B (set 2)")
	ROMX_LOAD("m6c_807c_u2.u2", 0x0000, 0x8000, CRC(57c34b4d) SHA1(1f436687f90b1afd4646e90d5617cf05c4465c98), ROM_BIOS(1))
	ROMX_LOAD("b6c_721_u3.u3",  0x8000, 0x8000, CRC(b1e57023) SHA1(6cec5cdc0bf4f8ac88afb0397fcb4738136b0431), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "b3", "Maestro B (set 3)")
	ROMX_LOAD("m6c_530_u2.u2",  0x0000, 0x8000, CRC(a8be85d8) SHA1(a7fb2b6a185bd5b355b3ba78439332a25c456773), ROM_BIOS(2))
	ROMX_LOAD("b6c_528a_u3.u3", 0x8000, 0x8000, CRC(c03dfe60) SHA1(efbc9abd5e93f51d5c49376c3fd7c87ee4ede82b), ROM_BIOS(2))

	// C (C and above are shared Maestro/Analyst)
	ROM_SYSTEM_BIOS(3, "c1", "Maestro C (set 1)")
	ROMX_LOAD("m6l_b30d_u2.u2", 0x0000, 0x8000, CRC(bb10e15c) SHA1(7b0fb987c49da76a03b46c80d2b4eacaa785ee75), ROM_BIOS(3))
	ROMX_LOAD("b6c_721_u3.u3",  0x8000, 0x8000, CRC(b1e57023) SHA1(6cec5cdc0bf4f8ac88afb0397fcb4738136b0431), ROM_BIOS(3))

	ROM_SYSTEM_BIOS(4, "c2", "Maestro C (set 2)")
	ROMX_LOAD("m6l_b30b_u2.u2", 0x0000, 0x8000, CRC(4b5026d7) SHA1(9715a0220c1bd3456480104f1c7ae61cbf1a1d73), ROM_BIOS(4))
	ROMX_LOAD("b6c_721_u3.u3",  0x8000, 0x8000, CRC(b1e57023) SHA1(6cec5cdc0bf4f8ac88afb0397fcb4738136b0431), ROM_BIOS(4))

	ROM_SYSTEM_BIOS(5, "c3", "Maestro C (set 3)")
	ROMX_LOAD("m6l_b25_u2.u2",  0x0000, 0x8000, CRC(217ae56c) SHA1(27ec80d0f82723c2710e2ccb477705934a4c2119), ROM_BIOS(5))
	ROMX_LOAD("b6c_721_u3.u3",  0x8000, 0x8000, CRC(b1e57023) SHA1(6cec5cdc0bf4f8ac88afb0397fcb4738136b0431), ROM_BIOS(5))

	// D
	ROM_SYSTEM_BIOS(6, "d1", "Maestro D (set 1)")
	ROMX_LOAD("ma3_714a_u2.u2", 0x0000, 0x8000, CRC(435e1e30) SHA1(0d82df7c40443cb341dacebdf65f33c3e03bce70), ROM_BIOS(6))
	ROMX_LOAD("b6m_b15_u3.u3",  0x8000, 0x8000, CRC(6155de90) SHA1(bb5cdf061dde2d1dc7925d455891c3ade1d274e3), ROM_BIOS(6))

	ROM_SYSTEM_BIOS(7, "d2", "Maestro D (set 2)")
	ROMX_LOAD("ma3_714a_u2.u2", 0x0000, 0x8000, CRC(435e1e30) SHA1(0d82df7c40443cb341dacebdf65f33c3e03bce70), ROM_BIOS(7))
	ROMX_LOAD("b6m_629_u3.u3",  0x8000, 0x8000, CRC(15e7b1f1) SHA1(d2a757114f13c6141d74a15671aa06b675304b4a), ROM_BIOS(7))

	// D+
	ROM_SYSTEM_BIOS(8, "dp", "Maestro D+")
	ROMX_LOAD("m6m_625_u2.u2",  0x0000, 0x8000, CRC(aa7b5cfd) SHA1(e909108fdace633a519fecf0b9876fe6a46b2067), ROM_BIOS(8))
	ROMX_LOAD("b6m_614_u3.u3",  0x8000, 0x8000, CRC(eff75543) SHA1(d7c1b3824bc87d5ffada6f5c8c72a8b292ff3d46), ROM_BIOS(8))

	// D++
	ROM_SYSTEM_BIOS(9, "dpp", "Maestro D++")
	ROMX_LOAD("d++_u2.u2",      0x0000, 0x8000, CRC(48ef032c) SHA1(d336cb2096780b4d3bcceda0d2ed1246e780cd8d), ROM_BIOS(9))
	ROMX_LOAD("b6m_614_u3.u3",  0x8000, 0x8000, CRC(eff75543) SHA1(d7c1b3824bc87d5ffada6f5c8c72a8b292ff3d46), ROM_BIOS(9))
ROM_END

ROM_START( analyst )
	ROM_REGION(0x10000, "maincpu", 0)

	ROM_DEFAULT_BIOS("d1")

	// B (Analyst only)
	ROM_SYSTEM_BIOS(0, "b", "Analyst B")
	ROMX_LOAD("m6l_a15_u2.u2",  0x0000, 0x8000, CRC(91570897) SHA1(e6db36ffc87ce3941a3e12222678069cff9e47f6), ROM_BIOS(0))
	ROMX_LOAD("b6c_721_u3.u3",  0x8000, 0x8000, CRC(b1e57023) SHA1(6cec5cdc0bf4f8ac88afb0397fcb4738136b0431), ROM_BIOS(0))

	// C (C and above are shared Maestro/Analyst)
	ROM_SYSTEM_BIOS(1, "c1", "Analyst C (set 1)")
	ROMX_LOAD("m6l_b30d_u2.u2", 0x0000, 0x8000, CRC(bb10e15c) SHA1(7b0fb987c49da76a03b46c80d2b4eacaa785ee75), ROM_BIOS(1))
	ROMX_LOAD("b6c_721_u3.u3",  0x8000, 0x8000, CRC(b1e57023) SHA1(6cec5cdc0bf4f8ac88afb0397fcb4738136b0431), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "c2", "Analyst C (set 2)")
	ROMX_LOAD("m6l_b30b_u2.u2", 0x0000, 0x8000, CRC(4b5026d7) SHA1(9715a0220c1bd3456480104f1c7ae61cbf1a1d73), ROM_BIOS(2))
	ROMX_LOAD("b6c_721_u3.u3",  0x8000, 0x8000, CRC(b1e57023) SHA1(6cec5cdc0bf4f8ac88afb0397fcb4738136b0431), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "c3", "Analyst C (set 3)")
	ROMX_LOAD("m6l_b25_u2.u2",  0x0000, 0x8000, CRC(217ae56c) SHA1(27ec80d0f82723c2710e2ccb477705934a4c2119), ROM_BIOS(3))
	ROMX_LOAD("b6c_721_u3.u3",  0x8000, 0x8000, CRC(b1e57023) SHA1(6cec5cdc0bf4f8ac88afb0397fcb4738136b0431), ROM_BIOS(3))

	// D
	ROM_SYSTEM_BIOS(4, "d1", "Analyst D (set 1)")
	ROMX_LOAD("ma3_714a_u2.u2", 0x0000, 0x8000, CRC(435e1e30) SHA1(0d82df7c40443cb341dacebdf65f33c3e03bce70), ROM_BIOS(4))
	ROMX_LOAD("b6m_b15_u3.u3",  0x8000, 0x8000, CRC(6155de90) SHA1(bb5cdf061dde2d1dc7925d455891c3ade1d274e3), ROM_BIOS(4))

	ROM_SYSTEM_BIOS(5, "d2", "Analyst D (set 2)")
	ROMX_LOAD("ma3_714a_u2.u2", 0x0000, 0x8000, CRC(435e1e30) SHA1(0d82df7c40443cb341dacebdf65f33c3e03bce70), ROM_BIOS(5))
	ROMX_LOAD("b6m_629_u3.u3",  0x8000, 0x8000, CRC(15e7b1f1) SHA1(d2a757114f13c6141d74a15671aa06b675304b4a), ROM_BIOS(5))

	// D+
	ROM_SYSTEM_BIOS(6, "dp", "Analyst D+")
	ROMX_LOAD("m6m_625_u2.u2",  0x0000, 0x8000, CRC(aa7b5cfd) SHA1(e909108fdace633a519fecf0b9876fe6a46b2067), ROM_BIOS(6))
	ROMX_LOAD("b6m_614_u3.u3",  0x8000, 0x8000, CRC(eff75543) SHA1(d7c1b3824bc87d5ffada6f5c8c72a8b292ff3d46), ROM_BIOS(6))

	// D++
	ROM_SYSTEM_BIOS(7, "dpp", "Analyst D++")
	ROMX_LOAD("d++_u2.u2",      0x0000, 0x8000, CRC(48ef032c) SHA1(d336cb2096780b4d3bcceda0d2ed1246e780cd8d), ROM_BIOS(7))
	ROMX_LOAD("b6m_614_u3.u3",  0x8000, 0x8000, CRC(eff75543) SHA1(d7c1b3824bc87d5ffada6f5c8c72a8b292ff3d46), ROM_BIOS(7))
ROM_END

const tiny_rom_entry *saitekosa_maestro_device::device_rom_region() const
{
	return ROM_NAME(maestro);
}

const tiny_rom_entry *saitekosa_analyst_device::device_rom_region() const
{
	return ROM_NAME(analyst);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(OSA_MAESTRO, device_saitekosa_expansion_interface, saitekosa_maestro_device, "osa_maestro", "Saitek OSA Maestro B-D")
DEFINE_DEVICE_TYPE_PRIVATE(OSA_ANALYST, device_saitekosa_expansion_interface, saitekosa_analyst_device, "osa_analyst", "Saitek OSA Analyst")
