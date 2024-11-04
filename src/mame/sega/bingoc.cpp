// license:BSD-3-Clause
// copyright-holders:Angelo Salese, David Haywood
/*******************************************************************************************

Bingo Circus (c) 1989 Sega

A Bingo machine with a terminal for each player, maximum 8 players can play together.

preliminary driver by David Haywood & Angelo Salese

TODO:
-a cabinet snap is here -> http://www.system16.com/hardware.php?id=840&page=1#2743,
 every player should have his own screen.
-inconsistent (likely wrong) sound banking.

============================================================================================
BINGO CIRCUS (MAIN PCB)
(c)SEGA

CPU   : MAIN 68000 SOUND Z-80
SOUND : YM2151 uPD7759C

12635A.EPR  ; MAIN PROGRAM
12636A.EPR  ;  /
12637.EPR   ; VOICE DATA
12638.EPR   ;  /
12639.EPR   ; SOUND PRG


BINGO CIRCUS (TERMINAL PCB)
(c)SEGA

CPU   : MAIN Z-80 SOUND Z-80
SOUND : 2 x ASSP 5C68A

12646.ic20  ; MAIN PROGRAM
12647.ic24  ; SOUND PRG + DATA
12648.ic25  ;  /

*******************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "315_5338a.h"
#include "bingoct.h"
#include "machine/gen_latch.h"
#include "machine/i8251.h"
#include "sound/rf5c68.h"
#include "sound/upd7759.h"
#include "sound/ymopm.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class bingoc_state : public driver_device
{
public:
	bingoc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_upd7759(*this, "upd"),
		m_soundlatch(*this, "soundlatch") { }


	void bingoc(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	//uint8_t m_x;
	[[maybe_unused]] void main_sound_latch_w(uint8_t data);
	void sound_play_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<upd7759_device> m_upd7759;
	required_device<generic_latch_8_device> m_soundlatch;
	void main_map(address_map &map) ATTR_COLD;
	void sound_io(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

#define SOUND_TEST 0

void bingoc_state::video_start()
{
}

uint32_t bingoc_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

#if SOUND_TEST
/*dirty code to test z80 + bgm/sfx*/
/*
0x00-0x7f controls u7759 samples (command 0xff->n)
0x80-0x85 ym2151 bgm
0x90-0x9b ym2151 sfx
*/
uint8_t bingoc_state::sound_test_r()
{
	if(machine().input().code_pressed_once(KEYCODE_Z))
		m_x++;

	if(machine().input().code_pressed_once(KEYCODE_X))
		m_x--;

	if(machine().input().code_pressed_once(KEYCODE_A))
		return 0xff;

	popmessage("%02x",m_x);
	return m_x;
}
#else
void bingoc_state::main_sound_latch_w(uint8_t data)
{
	m_soundlatch->write(data);
	m_soundcpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}
#endif

void bingoc_state::sound_play_w(uint8_t data)
{
	/*
	---- --x- sound rom banking
	---- ---x start-stop sample
	*/
	uint8_t *upd = memregion("upd")->base();
	memcpy(&upd[0x00000], &upd[0x20000 + (((data & 2)>>1) * 0x20000)], 0x20000);
	m_upd7759->start_w(data & 1);
//  printf("%02x\n",data);
}

void bingoc_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x100003).rw("uart1", FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0x100008, 0x10000b).rw("uart2", FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0x100010, 0x100013).rw("uart3", FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0x100018, 0x10001b).rw("uart4", FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0x100020, 0x100023).rw("uart5", FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0x100028, 0x10002b).rw("uart6", FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0x100030, 0x100033).rw("uart7", FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0x100038, 0x10003b).rw("uart8", FUNC(i8251_device::read), FUNC(i8251_device::write)).umask16(0x00ff);
	map(0x100070, 0x100071).noprw();
	map(0x180000, 0x18001f).rw("io", FUNC(sega_315_5338a_device::read), FUNC(sega_315_5338a_device::write)).umask16(0x00ff); //lamps?
#if 0 // !SOUND_TEST
	map(0x180010, 0x180011).w(FUNC(bingoc_state::main_sound_latch_w)); //WRONG there...
#endif
	map(0xff8000, 0xffffff).ram();
}

void bingoc_state::sound_map(address_map &map)
{
	map(0x0000, 0x4fff).rom();
	map(0xf800, 0xffff).ram();
}

void bingoc_state::sound_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x40, 0x40).w(FUNC(bingoc_state::sound_play_w));
	map(0x80, 0x80).w(m_upd7759, FUNC(upd7759_device::port_w));
#if !SOUND_TEST
	map(0xc0, 0xc0).r(m_soundlatch, FUNC(generic_latch_8_device::read));
#else
	map(0xc0, 0xc0).r(FUNC(bingoc_state::sound_test_r));
#endif
}


static INPUT_PORTS_START( bingoc )
INPUT_PORTS_END


void bingoc_state::bingoc(machine_config &config)
{
	M68000(config, m_maincpu, 8000000);      /* ? MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &bingoc_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(bingoc_state::irq2_line_hold));

	Z80(config, m_soundcpu, 4000000);        /* ? MHz */
	m_soundcpu->set_addrmap(AS_PROGRAM, &bingoc_state::sound_map);
	m_soundcpu->set_addrmap(AS_IO, &bingoc_state::sound_io);
#if SOUND_TEST
	m_soundcpu->set_vblank_int("screen", FUNC(bingoc_state::nmi_line_pulse));
#endif

	I8251(config, "uart1", 4000000); // unknown
	I8251(config, "uart2", 4000000); // unknown
	I8251(config, "uart3", 4000000); // unknown
	I8251(config, "uart4", 4000000); // unknown
	I8251(config, "uart5", 4000000); // unknown
	I8251(config, "uart6", 4000000); // unknown
	I8251(config, "uart7", 4000000); // unknown
	I8251(config, "uart8", 4000000); // unknown

	SEGA_315_5338A(config, "io", 0); // ?

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(bingoc_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x100);


	SPEAKER(config, "lspeaker").front_left(); //might just be mono...
	SPEAKER(config, "rspeaker").front_right();

	GENERIC_LATCH_8(config, m_soundlatch);

	YM2151(config, "ymsnd", 7159160/2).add_route(0, "lspeaker", 1.0).add_route(1, "rspeaker", 1.0);

	UPD7759(config, m_upd7759);
	m_upd7759->add_route(ALL_OUTPUTS, "lspeaker", 1.0);
	m_upd7759->add_route(ALL_OUTPUTS, "rspeaker", 1.0);

	// terminals
	BINGOCT(config, "term1");
#if 0
	BINGOCT(config, "term2");
	BINGOCT(config, "term3");
	BINGOCT(config, "term4");
	BINGOCT(config, "term5");
	BINGOCT(config, "term6");
	BINGOCT(config, "term7");
	BINGOCT(config, "term8");
#endif
}


ROM_START( bingoc )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "12636a.epr", 0x00000, 0x20000, CRC(ef8dccff) SHA1(9eb6e55e2000b252647fc748cbbeedf4f119aed7) )
	ROM_LOAD16_BYTE( "12635a.epr", 0x00001, 0x20000, CRC(a94cd74e) SHA1(0c3e157a5ddf34f4f1a2d30b9758bf067896371c) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "12639.epr", 0x00000, 0x10000, CRC(4307f6ba) SHA1(f568930191cd31a2112ef8d4cf5ff340826d5877) )

	ROM_REGION( 0x60000, "upd", 0 )
	ROM_LOAD( "12637.epr", 0x40000, 0x20000, CRC(164ac43f) SHA1(90160df8e927a25ea08badedb3fcd818c314b388) )
	ROM_LOAD( "12638.epr", 0x20000, 0x20000, CRC(ef52ab73) SHA1(d14593ef88ac2acd00daaf522008405f65f67548) )
	ROM_COPY( "upd",       0x20000, 0x00000, 0x20000 )
ROM_END


} // Anonymous namespace


GAME( 1989, bingoc,  0, bingoc,  bingoc, bingoc_state,  empty_init, ROT0, "Sega", "Bingo Circus (Rev. A 891001)", MACHINE_NOT_WORKING )
