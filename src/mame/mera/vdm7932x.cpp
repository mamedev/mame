// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Mera-Elzab VDM 79321/79322 terminals.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "machine/z80ctc.h"
#include "machine/z80sio.h"


namespace {

class vdm7932x_state : public driver_device
{
public:
	vdm7932x_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
	{
	}

	void vdm7932x(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;

	void scan_w(offs_t offset, u8 data);
	u8 i8031_p3_r();
	void ppi1_pc_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void sub_map(address_map &map) ATTR_COLD;
	void subx_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<i8031_device> m_subcpu;
	bool m_obfa = false;
};


void vdm7932x_state::machine_start()
{
	m_obfa = false;
	save_item(NAME(m_obfa));
}

void vdm7932x_state::scan_w(offs_t offset, u8 data)
{
}

u8 vdm7932x_state::i8031_p3_r()
{
	return m_obfa ? 0xfd : 0xff;
}

void vdm7932x_state::ppi1_pc_w(u8 data)
{
	m_obfa = !BIT(data, 7);
}

void vdm7932x_state::mem_map(address_map &map)
{
	map(0x0000, 0xbfff).rom().region("maincpu", 0);
	map(0xc000, 0xdfff).ram();
}

void vdm7932x_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x40).nopw(); // ?
	map(0x44, 0x47).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x48, 0x4b).w("pit", FUNC(pit8253_device::write));
	map(0x54, 0x57).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x58, 0x5b).rw("sio", FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
	map(0x80, 0x83).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void vdm7932x_state::sub_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("subcpu", 0);
}

void vdm7932x_state::subx_map(address_map &map)
{
	map(0x00, 0x00).select(0xff00).w(FUNC(vdm7932x_state::scan_w));
	map(0x49, 0x49).mirror(0xff00).r("ppi1", FUNC(i8255_device::acka_r));
}


static INPUT_PORTS_START(vdm7932x)
INPUT_PORTS_END


static const z80_daisy_config daisy_chain[] =
{
	{ "sio" },
	{ "ctc" },
	{ nullptr }
};

void vdm7932x_state::vdm7932x(machine_config &config) // all clocks unverified
{
	Z80(config, m_maincpu, 24.0734_MHz_XTAL / 8); // UA880D
	m_maincpu->set_addrmap(AS_PROGRAM, &vdm7932x_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &vdm7932x_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	I8031(config, m_subcpu, 24.0734_MHz_XTAL / 4); // Intel P8031AH (for keyboard?)
	m_subcpu->port_in_cb<3>().set(FUNC(vdm7932x_state::i8031_p3_r));
	m_subcpu->set_addrmap(AS_PROGRAM, &vdm7932x_state::sub_map);
	m_subcpu->set_addrmap(AS_IO, &vdm7932x_state::subx_map);

	PIT8253(config, "pit", 0); // UM8253-5

	z80ctc_device &ctc(Z80CTC(config, "ctc", 24.0734_MHz_XTAL / 8)); // UA857D
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80sio_device &sio(Z80SIO(config, "sio", 24.0734_MHz_XTAL / 8)); // UA8560D
	sio.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	i8255_device &ppi1(I8255A(config, "ppi1")); // КР580ВВ55А (on separate card)
	ppi1.out_pc_callback().set(FUNC(vdm7932x_state::ppi1_pc_w));

	I8255A(config, "ppi2"); // КР580ВВ55А (on separate card)
}


ROM_START( vdm79322 ) // 8k ram // b&w (amber)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27512_m322.bin",      0x00000, 0x10000, CRC(24573079) SHA1(b81c17e99493302054d78fbee2e416ab6493b5f3) )

	ROM_REGION( 0x04000, "subcpu", 0 )
	ROM_LOAD( "27128_w322-3700.bin", 0x00000, 0x04000, CRC(e5e76ca2) SHA1(bb18c9fa29ef9fa0563aa07d2b856cf6594fc020) )
ROM_END

} // anonymous namespace


COMP(1992, vdm79322, 0, 0, vdm7932x, vdm7932x, vdm7932x_state, empty_init, "Mera-Elzab", "VDM 79322/CM 7233", MACHINE_IS_SKELETON)
