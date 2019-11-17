// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

QTY     Type    clock   position    function
2x  2636            Programmable Video Interface
1x  2650    OSC/2 = 1.7897725 MHz       8-bit Microprocessor - main
1x  oscillator  3.579545 MHz

ROMs
QTY     Type    position    status
4x  2708    6F,6H,6L,6N     dumped
1x  N82S115     2B  dumped

RAMs
QTY     Type    position
2x  2101

*/

#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "machine/s2636.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class subhuntr_state : public driver_device
{
public:
	subhuntr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void subhuntr(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	INTERRUPT_GEN_MEMBER(subhuntr_interrupt);

	void subhuntr_palette(palette_device &palette) const;
	uint32_t screen_update_subhuntr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void subhuntr_data_map(address_map &map);
	void subhuntr_io_map(address_map &map);
	void subhuntr_map(address_map &map);

	required_device<s2650_device> m_maincpu;
};


/***************************************************************************

  Video

***************************************************************************/

void subhuntr_state::subhuntr_palette(palette_device &palette) const
{
}

uint32_t subhuntr_state::screen_update_subhuntr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void subhuntr_state::video_start()
{
}


/***************************************************************************

  Memory Maps, I/O

***************************************************************************/

void subhuntr_state::subhuntr_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x1c00, 0x1fff).ram();
}

void subhuntr_state::subhuntr_io_map(address_map &map)
{
}

void subhuntr_state::subhuntr_data_map(address_map &map)
{
//  map(S2650_CTRL_PORT, S2650_CTRL_PORT).rw(FUNC(subhuntr_state::), FUNC(subhuntr_state::));
//  map(S2650_DATA_PORT, S2650_DATA_PORT).rw(FUNC(subhuntr_state::), FUNC(subhuntr_state::));
}

/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( subhuntr )
INPUT_PORTS_END


/***************************************************************************

  Machine Config/Interface

***************************************************************************/

void subhuntr_state::machine_start()
{
}

void subhuntr_state::machine_reset()
{
}

INTERRUPT_GEN_MEMBER(subhuntr_state::subhuntr_interrupt)
{
	device.execute().set_input_line_and_vector(0, HOLD_LINE, 0x03); // S2650
}

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	8*8
};

static GFXDECODE_START( gfx_subhuntr )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout, 0, 1 )
GFXDECODE_END


void subhuntr_state::subhuntr(machine_config &config)
{
	/* basic machine hardware */
	S2650(config, m_maincpu, 14318180/4/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &subhuntr_state::subhuntr_map);
	m_maincpu->set_addrmap(AS_IO, &subhuntr_state::subhuntr_io_map);
	m_maincpu->set_addrmap(AS_DATA, &subhuntr_state::subhuntr_data_map);
	m_maincpu->set_vblank_int("screen", FUNC(subhuntr_state::subhuntr_interrupt));
	m_maincpu->sense_handler().set("screen", FUNC(screen_device::vblank));

	//s2636_device &s2636(S2636(config, "s2636", 0));
	//s2636.set_offsets(3, -21);
	//s2636.add_route(ALL_OUTPUTS, "mono", 0.10);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_video_attributes(VIDEO_ALWAYS_UPDATE);
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(256, 256);
	screen.set_visarea(1*8, 29*8-1, 2*8, 32*8-1);
	screen.set_screen_update(FUNC(subhuntr_state::screen_update_subhuntr));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_subhuntr);
	PALETTE(config, "palette", FUNC(subhuntr_state::subhuntr_palette), 26);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	/* discrete sound */
}



/******************************************************************************/

ROM_START( subhuntr )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "mr21.6f",  0x0000, 0x0400, CRC(27847939) SHA1(e6b41b511fefac1e1e207eff2dac8c2963d47c5c) )
	ROM_LOAD( "mr22.6g",  0x0400, 0x0400, CRC(e9af1ee8) SHA1(451e88407a120444377a58b06b65152c57503533) )
	ROM_LOAD( "mr25.6l",  0x0800, 0x0400, CRC(8271c975) SHA1(c7192658b50d781ab1b94c2e8cb75c5be3539820) )
	ROM_LOAD( "mr24.6n",  0x0c00, 0x0400, CRC(385c4944) SHA1(84050b0356c9a3a36528dba768f2684e28c6c7c4) )

	ROM_REGION( 0x0200, "gfx1", 0 )
	ROM_LOAD( "82s115.2b",   0x0000, 0x0200, CRC(6946c9de) SHA1(956b4bebe6960a73609deb75e1493c4127fd7f77) ) // ASCII, not much else
ROM_END

GAME(1979, subhuntr,  0,        subhuntr, subhuntr, subhuntr_state, empty_init, ROT0, "Model Racing", "Sub Hunter (Model Racing)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
