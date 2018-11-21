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

	void photon(machine_config &config);

private:
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

void photon_state::pk8000_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x3fff).bankr("bank1").bankw("bank5");
	map(0x4000, 0x7fff).bankr("bank2").bankw("bank6");
	map(0x8000, 0xbfff).bankr("bank3").bankw("bank7");
	map(0xc000, 0xffff).bankr("bank4").bankw("bank8");
}

void photon_state::pk8000_io(address_map &map)
{
	map.unmap_value_high();
	map(0x80, 0x83).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x84, 0x87).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x88, 0x88).rw(FUNC(photon_state::video_color_r), FUNC(photon_state::video_color_w));
	map(0x8c, 0x8c).portr("JOY1");
	map(0x8d, 0x8d).portr("JOY2");
	map(0x90, 0x90).rw(FUNC(photon_state::text_start_r), FUNC(photon_state::text_start_w));
	map(0x91, 0x91).rw(FUNC(photon_state::chargen_start_r), FUNC(photon_state::chargen_start_w));
	map(0x92, 0x92).rw(FUNC(photon_state::video_start_r), FUNC(photon_state::video_start_w));
	map(0x93, 0x93).rw(FUNC(photon_state::color_start_r), FUNC(photon_state::color_start_w));
	map(0xa0, 0xbf).rw(FUNC(photon_state::color_r), FUNC(photon_state::color_w));
}

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
	MCFG_DEVICE_ADD("maincpu",I8080, 1780000)
	MCFG_DEVICE_PROGRAM_MAP(pk8000_mem)
	MCFG_DEVICE_IO_MAP(pk8000_io)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", photon_state, interrupt)
	MCFG_DEVICE_IRQ_ACKNOWLEDGE_DRIVER(photon_state, irq_callback)

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

	i8255_device &ppi1(I8255(config, "ppi8255_1"));
	ppi1.out_pa_callback().set(FUNC(photon_state::_80_porta_w));
	ppi1.in_pb_callback().set(FUNC(photon_state::_80_portb_r));
	ppi1.out_pc_callback().set(FUNC(photon_state::_80_portc_w));

	i8255_device &ppi2(I8255(config, "ppi8255_2"));
	ppi2.in_pa_callback().set(FUNC(pk8000_base_state::_84_porta_r));
	ppi2.out_pa_callback().set(FUNC(pk8000_base_state::_84_porta_w));
	ppi2.out_pc_callback().set(FUNC(pk8000_base_state::_84_portc_w));

	/* audio hardware */
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("speaker", SPEAKER_SOUND)
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

GAME( 19??,  phtetris, 0,      photon, photon, photon_state, empty_init, ROT0, "<unknown>", "Tetris (Photon System)",           0 )
GAME( 1989?, phpython,  0,     photon, photon, photon_state, empty_init, ROT0, "<unknown>", "Python (Photon System)",           0 )
GAME( 19??,  phklad,   0,      photon, photon, photon_state, empty_init, ROT0, "<unknown>", "Klad / Labyrinth (Photon System)", 0 )
