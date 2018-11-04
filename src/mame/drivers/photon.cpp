// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, Miodrag Milanovic
/*
    Photon System

    Uses PK8000 emulation by Miodrag Milanovic
    Imported to MAME by Mariusz Wojcieszek

    Russian arcade system based on PK8000 home computer, created by unknown manufacturer
    in late 1980s or early 1990s.

    Following games were produced for this system:
    - Tetris
    - Python
    - Klad/Labyrinth

    Use joystick left and right in Klad/Labyrinth attract mode to select a game to play.
*/

#include "emu.h"
#include "includes/pk8000.h"

#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "sound/spkrdev.h"
#include "screen.h"
#include "speaker.h"


class photon_state : public pk8000_base_state
{
public:
	photon_state(const machine_config &mconfig, device_type type, const char *tag)
		: pk8000_base_state(mconfig, type, tag),
		m_speaker(*this, "speaker") { }

	DECLARE_WRITE8_MEMBER(_80_porta_w);
	DECLARE_READ8_MEMBER(_80_portb_r);
	DECLARE_WRITE8_MEMBER(_80_portc_w);

	virtual void machine_reset() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(interrupt);
	IRQ_CALLBACK_MEMBER(irq_callback);
	void set_bank(uint8_t data);

	required_device<speaker_sound_device> m_speaker;
	void photon(machine_config &config);
	void pk8000_io(address_map &map);
	void pk8000_mem(address_map &map);
};


void photon_state::set_bank(uint8_t data)
{
	uint8_t *rom = memregion("maincpu")->base();
	uint8_t *ram = memregion("maincpu")->base();
	uint8_t block1 = data & 3;
	uint8_t block2 = (data >> 2) & 3;
	uint8_t block3 = (data >> 4) & 3;
	uint8_t block4 = (data >> 6) & 3;

	switch(block1) {
		case 0:
				membank("bank1")->set_base(rom + 0x10000);
				membank("bank5")->set_base(ram);
				break;
		case 1: break;
		case 2: break;
		case 3:
				membank("bank1")->set_base(ram);
				membank("bank5")->set_base(ram);
				break;
	}

	switch(block2) {
		case 0:
				membank("bank2")->set_base(rom + 0x14000);
				membank("bank6")->set_base(ram + 0x4000);
				break;
		case 1: break;
		case 2: break;
		case 3:
				membank("bank2")->set_base(ram + 0x4000);
				membank("bank6")->set_base(ram + 0x4000);
				break;
	}
	switch(block3) {
		case 0:
				membank("bank3")->set_base(rom + 0x18000);
				membank("bank7")->set_base(ram + 0x8000);
				break;
		case 1: break;
		case 2: break;
		case 3:
				membank("bank3")->set_base(ram + 0x8000);
				membank("bank7")->set_base(ram + 0x8000);
				break;
	}
	switch(block4) {
		case 0:
				membank("bank4")->set_base(rom + 0x1c000);
				membank("bank8")->set_base(ram + 0xc000);
				break;
		case 1: break;
		case 2: break;
		case 3:
				membank("bank4")->set_base(ram + 0xc000);
				membank("bank8")->set_base(ram + 0xc000);
				break;
	}
}
WRITE8_MEMBER(photon_state::_80_porta_w)
{
	set_bank(data);
}

READ8_MEMBER(photon_state::_80_portb_r)
{
	return 0xff;
}

WRITE8_MEMBER(photon_state::_80_portc_w)
{
	m_speaker->level_w(BIT(data,7));
}

ADDRESS_MAP_START(photon_state::pk8000_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x3fff ) AM_READ_BANK("bank1") AM_WRITE_BANK("bank5")
	AM_RANGE( 0x4000, 0x7fff ) AM_READ_BANK("bank2") AM_WRITE_BANK("bank6")
	AM_RANGE( 0x8000, 0xbfff ) AM_READ_BANK("bank3") AM_WRITE_BANK("bank7")
	AM_RANGE( 0xc000, 0xffff ) AM_READ_BANK("bank4") AM_WRITE_BANK("bank8")
ADDRESS_MAP_END

ADDRESS_MAP_START(photon_state::pk8000_io)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x80, 0x83) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0x84, 0x87) AM_DEVREADWRITE("ppi8255_2", i8255_device, read, write)
	AM_RANGE(0x88, 0x88) AM_READWRITE(video_color_r, video_color_w)
	AM_RANGE(0x8c, 0x8c) AM_READ_PORT("JOY1")
	AM_RANGE(0x8d, 0x8d) AM_READ_PORT("JOY2")
	AM_RANGE(0x90, 0x90) AM_READWRITE(text_start_r, text_start_w)
	AM_RANGE(0x91, 0x91) AM_READWRITE(chargen_start_r, chargen_start_w)
	AM_RANGE(0x92, 0x92) AM_READWRITE(video_start_r, video_start_w)
	AM_RANGE(0x93, 0x93) AM_READWRITE(color_start_r, color_start_w)
	AM_RANGE(0xa0, 0xbf) AM_READWRITE(color_r, color_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( photon )
	PORT_START("JOY1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("JOY2")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

INTERRUPT_GEN_MEMBER(photon_state::interrupt)
{
	device.execute().set_input_line(0, HOLD_LINE);
}

IRQ_CALLBACK_MEMBER(photon_state::irq_callback)
{
	return 0xff;
}


void photon_state::machine_reset()
{
	set_bank(0);
}

void photon_state::video_start()
{
	save_item(NAME(m_text_start));
	save_item(NAME(m_chargen_start));
	save_item(NAME(m_video_start));
	save_item(NAME(m_color_start));
	save_item(NAME(m_video_mode));
	save_item(NAME(m_video_color));
	save_item(NAME(m_color));
	save_item(NAME(m_video_enable));
}

uint32_t photon_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return video_update(screen, bitmap, cliprect, memregion("maincpu")->base());
}

MACHINE_CONFIG_START(photon_state::photon)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8080, 1780000)
	MCFG_CPU_PROGRAM_MAP(pk8000_mem)
	MCFG_CPU_IO_MAP(pk8000_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", photon_state, interrupt)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(photon_state, irq_callback)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(256+32, 192+32)
	MCFG_SCREEN_VISIBLE_AREA(0, 256+32-1, 0, 192+32-1)
	MCFG_SCREEN_UPDATE_DRIVER(photon_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(pk8000_base_state, pk8000)

	MCFG_DEVICE_ADD("ppi8255_1", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(photon_state, _80_porta_w))
	MCFG_I8255_IN_PORTB_CB(READ8(photon_state, _80_portb_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(photon_state, _80_portc_w))

	MCFG_DEVICE_ADD("ppi8255_2", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(pk8000_base_state, _84_porta_r))
	MCFG_I8255_OUT_PORTA_CB(WRITE8(pk8000_base_state, _84_porta_w))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(pk8000_base_state, _84_portc_w))

	/* audio hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

/*
    Dump was made using custom adaptor, hence it is marked as bad dump.
    The real machine has following roms:
    0000...07FFh - ROM1 (D41)
    0800...0FFFh - ROM2 (D42)
    1000...17FFh - ROM3 (D43)
    1800...1FFFh - not populated (D44)
    2000...27FFh - ROM5 (D45)
    2800...2FFFh - ROM6 (D46)
    3000...37FFh - ROM7 (D47)
    3800...3FFFh - ROM8 (D48)
*/
ROM_START( phtetris )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "foton_tetris.bin", 0x10000, 0x4000, BAD_DUMP CRC(a8af10bb) SHA1(5e2ea9a5d38399cbe156638eea73a3d25c442f77) )
ROM_END

/*
    Dump was made using custom adaptor, hence it is marked as bad dump.
    The real machine has following roms:

    0000...07FFh - ROM1 (D41)
    0800...0FFFh - ROM2 (D42)
    1000...17FFh - ROM3 (D43)
*/
ROM_START( phpython )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "foton_piton.bin", 0x10000, 0x1800, BAD_DUMP CRC(4eac925a) SHA1(26f9a18c7aed31b7daacdc003bafb60a5e6d6300) )
ROM_END


/*
    Dump was made using custom adaptor, hence it is marked as bad dump.
*/
ROM_START( phklad )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "klad.bin", 0x10000, 0x4000, BAD_DUMP CRC(49cc7d65) SHA1(d966cfc1d973a533df8044a71fad37f7177da554) )
ROM_END

GAME( 19??,  phtetris, 0,      photon, photon, photon_state, 0, ROT0, "<unknown>", "Tetris (Photon System)",           0 )
GAME( 1989?, phpython,  0,     photon, photon, photon_state, 0, ROT0, "<unknown>", "Python (Photon System)",           0 )
GAME( 19??,  phklad,   0,      photon, photon, photon_state, 0, ROT0, "<unknown>", "Klad / Labyrinth (Photon System)", 0 )
