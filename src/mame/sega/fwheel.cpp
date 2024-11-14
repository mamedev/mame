// license:BSD-3-Clause
// copyright-holders:

/*
Fortune Wheel, Sega 1984?

This game came on (at least) 3 PCBs:

1)
IC BOARD M4J MAIN, one '837-6553' sticker, one '837-6899 FORTUNE WHEEL' sticker
NEC D780C-1 main CPU
SEGA 315-5124 custom
SEGA 315-5297 custom
27C512 ROM
3x NEC D4168C-15-SG RAM (1 near the CPU, 2 near the 315-5124)
10.7386 MHz XTAL

2)
one '837-6900 FORTUNE WHEEL' sticker, one '880324 0508 H' sticker
NEC D780C-1 audio CPU
2x YM2413
D4364CX RAM
4.000 MHz XTAL
27512 ROM

3)
MADE IN JAPAN @SEGA 1984, one '837-6898 FORTUNE WHEEL' sticker, one '880322 0311 H' sticker
NEC D780C-1 CPU (wheel?)
OKI M2128-15 RAM
9x4 NEC 2501
2x 27128 ROMs
8-dip bank
4-dip bank
4.000 MHz XTAL
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ymopl.h"
#include "video/315_5124.h"

#include "speaker.h"

namespace {

class fwheel_state : public driver_device
{
public:
	fwheel_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_bank(*this, "bank")
		, m_ipl(*this, "ipl")
		, m_vdp(*this, "vdp")
	{}

	void fwheel(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_memory_bank m_bank;
	required_memory_region m_ipl;
	required_device<sega315_5124_device> m_vdp;

	void main_io_map(address_map &map) ATTR_COLD;
	void main_prg_map(address_map &map) ATTR_COLD;
	void sound_prg_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void wheel_prg_map(address_map &map) ATTR_COLD;
};


void fwheel_state::main_prg_map(address_map &map)
{
	map(0x0000, 0x7fff).bankr(m_bank);
	map(0xc000, 0xdfff).ram();
}

void fwheel_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x3e, 0x3e).lw8(
		NAME([this] (u8 data) {
			// TODO: unconfirmed, may be either bit 3 or 5
			m_bank->set_entry(BIT(data, 3));
			if (data != 0xe0 && data != 0xc8)
				logerror("I/O $3e -> %02x\n", data);
		})
	);
	map(0xbe, 0xbe).rw(m_vdp, FUNC(sega315_5124_device::data_read), FUNC(sega315_5124_device::data_write));
	map(0xbf, 0xbf).rw(m_vdp, FUNC(sega315_5124_device::control_read), FUNC(sega315_5124_device::control_write));
	map(0xdc, 0xdc).portr("IN0"); // TODO: comms from another CPU
}

void fwheel_state::sound_prg_map(address_map &map)
{
	map(0x0000, 0x8fff).rom();
	map(0xf800, 0xffff).ram();
}

void fwheel_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x00, 0x01).w("ym1", FUNC(ym2413_device::write));
	map(0x40, 0x41).w("ym2", FUNC(ym2413_device::write));
}

void fwheel_state::wheel_prg_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
}


static INPUT_PORTS_START( fwheel )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1") // 4 dip bank
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

void fwheel_state::machine_start()
{
	m_bank->configure_entries(0, 2, m_ipl->base(), 0x8000);
}

void fwheel_state::machine_reset()
{
	m_bank->set_entry(0);
}

void fwheel_state::fwheel(machine_config &config)
{
	// on 837-6899 PCB
	Z80(config, m_maincpu, XTAL(10'738'000) / 3); // divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &fwheel_state::main_prg_map);
	m_maincpu->set_addrmap(AS_IO, &fwheel_state::main_io_map);

	config.set_maximum_quantum(attotime::from_hz(60));

	SPEAKER(config, "mono").front_center();

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(10'738'000) / 2,
			sega315_5124_device::WIDTH , sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH - 2, sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH + 256 + 10,
			sega315_5124_device::HEIGHT_NTSC, sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT, sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_224_TBORDER_HEIGHT + 224);
	m_screen->set_refresh_hz(XTAL(10'738'000) / 2 / (sega315_5124_device::WIDTH * sega315_5124_device::HEIGHT_NTSC));
	m_screen->set_screen_update(m_vdp, FUNC(sega315_5246_device::screen_update));

	SEGA315_5124(config, m_vdp, XTAL(10'738'000)); // not verified
	m_vdp->set_screen(m_screen);
	m_vdp->set_is_pal(false);
	m_vdp->n_int().set_inputline(m_maincpu, 0);
	m_vdp->n_nmi().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_vdp->add_route(ALL_OUTPUTS, "mono", 1.00);

	// on 837-6900 PCB
	z80_device &audiocpu(Z80(config, "audiocpu", 4_MHz_XTAL));
	audiocpu.set_addrmap(AS_PROGRAM, &fwheel_state::sound_prg_map);
	audiocpu.set_addrmap(AS_IO, &fwheel_state::sound_io_map);

	YM2413(config, "ym1", 4_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.30);

	YM2413(config, "ym2", 4_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 0.30);

	// on 837-6898 PCB
	z80_device &wheelcpu(Z80(config, "wheelcpu", 4_MHz_XTAL));
	wheelcpu.set_addrmap(AS_PROGRAM, &fwheel_state::wheel_prg_map);
}


ROM_START( fwheel )
	ROM_REGION( 0x10000, "ipl", 0 )
	ROM_LOAD( "epr-12198.ic2", 0x00000, 0x10000, CRC(673e9aac) SHA1(62bec711b86a70988b80a74f440768b1460e46ac) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "epr-12199.ic2", 0x00000, 0x10000, CRC(e5f31387) SHA1(20269210031880b971ab46a194f16e42027d3d6a) )

	ROM_REGION( 0x8000, "wheelcpu", 0) // function unverified
	ROM_LOAD( "epr-12196.ic28", 0x0000, 0x4000, CRC(3a072a1e) SHA1(ede2b481930ba87a50f738941c9ceaffc09ea4bd) )
	ROM_LOAD( "epr-12197.ic21", 0x4000, 0x4000, CRC(289b3c75) SHA1(55a099633063f80b7d40469c2cc777401794a0bd) )
ROM_END

} // Anonymous namespace


GAME( 1989, fwheel, 0, fwheel, fwheel, fwheel_state, empty_init, ROT270, "Sega", "Fortune Wheel", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
