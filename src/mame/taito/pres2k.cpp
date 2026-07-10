// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Taito "Presenter 2000" プレゼンター2000

MSX-like Karaoke unit
Written as "PRESENTER - 2000" on main body stickers

TODO:
- Resets itself when coined in (or if it's in free play mode);
- Card slot;
- Video input, Karaoke sound input, external sound input jacks;
- Doesn't produce any sound with the SSG;
- Expansion connector;
- Remote control for START/STOP buttons (if not same as SSG port B);
- An user/operator manual scan;

**************************************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/v9938.h"

#include "screen.h"
#include "speaker.h"

namespace {

class pres2k_state : public driver_device
{
public:
	pres2k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppi(*this, "ppi")
		, m_ssg(*this, "ssg")
		, m_bios_bank(*this, "bios_bank")
	{ }

	void pres2k(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
//	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device<i8255_device> m_ppi;
	required_device<ym2149_device> m_ssg;
	required_memory_bank m_bios_bank;

	u8 ppi_pa_r();
	void ppi_pb_w(u8 data);
	u8 ppi_pc_r();

	void main_map(address_map &map) ATTR_COLD;

	u8 m_ay_address;
};

void pres2k_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rom().region("bios", 0);
	map(0x8000, 0xbfff).bankr("bios_bank");
	map(0xc000, 0xdfff).ram().share("nvram");

	map(0xf000, 0xf003).rw("v9938", FUNC(v9938_device::read), FUNC(v9938_device::write));
	// reads address then discards result, left-over?
	map(0xf004, 0xf004).lrw8(
		NAME([this] (offs_t offset) {
			return m_ay_address;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_ay_address = data;
			m_ssg->address_w(data);
		})
	);
	map(0xf005, 0xf005).rw(m_ssg, FUNC(ym2149_device::data_r), FUNC(ym2149_device::data_w));
	map(0xf008, 0xf00b).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
}

u8 pres2k_state::ppi_pa_r()
{
	// if PORTC bit 5 low then it scans for 0x55 value, not read from here
	return 0xff;
}

void pres2k_state::ppi_pb_w(u8 data)
{
	m_bios_bank->set_entry((data & 0x60) >> 5);
	machine().bookkeeping().coin_counter_w(0, BIT(data, 4));
	// TODO: bits 3-2
}

u8 pres2k_state::ppi_pc_r()
{
	// only bits 0-2 read from handler
	return 0xff;
}

static INPUT_PORTS_START( pres2k )
	PORT_START("SSG_PA")
	PORT_DIPNAME( 0x01, 0x01, "SSG_PA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	// Battery error if low
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SSG_PB")
	PORT_DIPNAME( 0x01, 0x00, "SSG_PB" ) // must be ON for "B" mode coin in to work
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	// TODO: unconfirmed which is which
	PORT_DIPNAME( 0x04, 0x04, "Mode Select" ) // back of unit
	PORT_DIPSETTING(    0x04, "A: Karaoke Link Mode" ) // カラオケ
	PORT_DIPSETTING(    0x00, "B: Lucky Draw Mode" ) // 福引き抽選
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 ) // actually has some use in service mode only
	PORT_SERVICE( 0x20, IP_ACTIVE_LOW )  // Configure Lottery
	// a START and STOP buttons, on main unit
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Start SW")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Stop SW")
INPUT_PORTS_END

void pres2k_state::machine_start()
{
	m_bios_bank->configure_entries(0, 4, memregion("bios")->base(), 0x4000);
	m_bios_bank->set_entry(2);
}

void pres2k_state::pres2k(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(21'477'272) / 6);
	m_maincpu->set_addrmap(AS_PROGRAM, &pres2k_state::main_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(pres2k_state::ppi_pa_r));
	m_ppi->out_pb_callback().set(FUNC(pres2k_state::ppi_pb_w));
	m_ppi->in_pc_callback().set(FUNC(pres2k_state::ppi_pc_r));

	v9938_device &v9938(V9938(config, "v9938", XTAL(21'477'272)));
	v9938.set_screen_ntsc("screen");
	v9938.set_vram_size(1024 * 128);
	v9938.int_cb().set_inputline("maincpu", 0);
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	YM2149(config, m_ssg, XTAL(21'477'272) / 6);
	// mixing unconfirmed
	m_ssg->add_route(0, "speaker", 0.5, 1);
	m_ssg->add_route(2, "speaker", 0.5, 1);
	m_ssg->add_route(1, "speaker", 0.5, 0);
	m_ssg->add_route(2, "speaker", 0.5, 0);
	m_ssg->port_a_read_callback().set_ioport("SSG_PA");
	m_ssg->port_b_read_callback().set_ioport("SSG_PB");
}


ROM_START( pres2k )
	ROM_REGION( 0x10000, "bios", 0 )
	ROM_LOAD( "a70_01-1.rom", 0x0000, 0x8000, CRC(16b68261) SHA1(465ef5215a9081cbdf22d342a3632ae4f280325c) )
	ROM_LOAD( "a70_02.rom",   0x8000, 0x8000, CRC(1f3f3617) SHA1(b244b079f199a28d7f31496439f6df033a5a3773) )
ROM_END

} // anonymous namespace


GAME( 1986?, pres2k, 0, pres2k, pres2k, pres2k_state, empty_init, ROT0, "Taito", "Presenter 2000", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // A70, fits between bigevglf (A67) and tokio (A71)
