// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/**************************************************************************************************

'Nichibutsu LD' HW (c) 1990? Nichibutsu

TODO:
- ldquiz4: spins on "memory test error 13", implying a missing ROM dump
  (other GFXs will return further errors if missing, returning the label there).
  To bypass: bp 18d,1,{hl=34bf;g}
  or alternatively patch location $40 in ROM with a 0x00 (which looks a debug switch)
- Unknown LaserDisc type;
- Unknown irq vector for LaserDisc strobe, unless it's really supposed to execute code with trg0
  irq service (which spins for nothing in both games)
- V9938 has issues with layer clears, has an hard time sending a vblank irq (the only one enabled)
  at the right time. Removing the invert() from the int_cb will "fix" it at the expense of being
  excruciatingly slow.
- Document meaning of remaining DIP switches

Notes:
- In service mode, press KAN/PON for the sound test and CHI/REACH for the voice test
- Push START to continue after the RGB test screen is shown

===================================================================================================

1 x TMPZ84C011AF-6 main CPU
1 x 12.000MHz OSC
1 x 21.47727MHz OSC
1 x Z0840004PSC audio CPU
1 x 4.000MHz OSC
1 x Yamaha V9938
1 x uPC1352C (NTSC decoder)
1 x Yamaha YM3812
2 x 8 dip switch banks

A red/white RCA connector near the uPC, labeled video/audio respectively
A LDC labeled 2 pin connector
6 x potentiometers for LD decoder section, 5 of them aligned as VR1-VR5,
    the 6th one (VR6) is near LDC
1 x potentiometer labeled VR7, near the sound section
1 x VOL for LD decoder section

**************************************************************************************************/


#include "emu.h"

#include "nbmjctrl.h"

#include "cpu/z80/tmpz84c011.h"
#include "machine/74166.h"
#include "machine/gen_latch.h"
#include "sound/dac.h"
#include "sound/ymopl.h"
#include "video/v9938.h"

#include "screen.h"
#include "speaker.h"


namespace {

#define MAIN_CLOCK XTAL(12'000'000)
#define VDP_CLOCK XTAL(21'477'272)
#define SOUND_CLOCK XTAL(4'000'000)

class nichild_state : public driver_device
{
public:
	nichild_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_v9938(*this, "v9938")
		, m_gfxrom(*this, "gfx")
		, m_keys{ { *this, "P1_KEY%u", 0U }, { *this, "P2_KEY%u", 0U } }
		, m_dsw(*this, "DSW%c", 'A')
		, m_dsw_shifter(*this, "ttl166_%u", 1U)
		, m_sound_rom(*this, "audiorom")
		, m_soundbank(*this, "soundbank")
		, m_soundlatch(*this, "soundlatch")
	{
	}

	void nichild(machine_config &config);

private:
	required_device<tmpz84c011_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<v9938_device> m_v9938;
	required_region_ptr<uint8_t> m_gfxrom;

	required_ioport_array<5> m_keys[2];
	required_ioport_array<2> m_dsw;
	required_device_array<ttl166_device, 2> m_dsw_shifter;

	required_region_ptr<uint8_t> m_sound_rom;
	required_memory_bank m_soundbank;
	required_device<generic_latch_8_device> m_soundlatch;

	uint8_t gfx_r(offs_t offset);
	uint8_t p1_keymatrix_r();
	uint8_t p2_keymatrix_r();
	void key_select_w(uint8_t data);
	uint8_t porta_r();
	void porta_w(uint8_t data);
	void gfxbank_w(uint8_t data);

	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	void soundbank_w(uint8_t data);

	void audio_map(address_map &map) ATTR_COLD;
	void audio_io(address_map &map) ATTR_COLD;

	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t m_gfx_bank = 0;
	uint8_t m_key_select = 0;
	uint8_t m_soundlatch_ack = 0;
	int m_dsw_data = 0;
};


uint8_t nichild_state::gfx_r(offs_t offset)
{
	uint32_t gfx_offset;

	gfx_offset  = ((offset & 0x007f) << 8);
	gfx_offset |= ((offset & 0xff00) >> 8);
	gfx_offset |= m_gfx_bank;

	//logerror("%08x %02x\n",gfx_offset,m_gfx_bank);

	return m_gfxrom[gfx_offset];
}

uint8_t nichild_state::porta_r()
{
	// 7-------  dipswitch 74166 qh
	// -6543210  output (see below)

	return m_dsw_data << 7;
}

void nichild_state::porta_w(uint8_t data)
{
	// 7-------  input (see above)
	// -6------  dipswitch 74166 clock
	// --5-----  dipswitch 74166 shift/load
	// ---43210  unknown

	logerror("PORTA %02x\n",data);

	m_dsw_shifter[0]->shift_load_w(BIT(data, 5));
	m_dsw_shifter[1]->shift_load_w(BIT(data, 5));
	m_dsw_shifter[0]->clock_w(BIT(data, 6));
	m_dsw_shifter[1]->clock_w(BIT(data, 6));
}

void nichild_state::gfxbank_w(uint8_t data)
{
	// TODO: ldquiz4 checks up to 0x30, what for?
	m_gfx_bank = data * 0x8000;
}

uint8_t nichild_state::p1_keymatrix_r()
{
	uint8_t result = 0xff;

	for (unsigned i = 0; i < 5; i++)
		if (BIT(m_key_select, i) == 0)
			result &= m_keys[0][i]->read();

	return result;
}

uint8_t nichild_state::p2_keymatrix_r()
{
	uint8_t result = 0xff;

	for (unsigned i = 0; i < 5; i++)
		if (BIT(m_key_select, i) == 0)
			result &= m_keys[1][i]->read();

	return result;
}

void nichild_state::key_select_w(uint8_t data)
{
	// 7-------  unknown (always 0?)
	// -6------  coin counter
	// --5-----  unknown (sometimes toggles 0/1)
	// ---43210  key row select

	m_key_select = (data & 0x1f);

	machine().bookkeeping().coin_counter_w(0, BIT(data, 6));
}

void nichild_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("ipl", 0x0000);
	map(0x8000, 0x9fff).rom().region("ipl", 0x8000);
	map(0xe000, 0xffff).ram();
}

void nichild_state::main_io(address_map &map)
{
//  map.global_mask(0xff);
	map(0x60, 0x60).mirror(0xff00).w(FUNC(nichild_state::key_select_w));
	map(0x64, 0x64).mirror(0xff00).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	// shabdama accesses 0x70-0x73, ldquiz4 0x7c-0x7f
	map(0x70, 0x73).mirror(0xff0c).rw(m_v9938, FUNC(v9938_device::read), FUNC(v9938_device::write));
	map(0x80, 0xff).select(0xff00).r(FUNC(nichild_state::gfx_r));
}

void nichild_state::soundbank_w(uint8_t data)
{
	m_soundbank->set_entry(data & 0x03);
	// 1 -> 0 -> 1 transitions clears the soundlatch
	if (!BIT(data, 7) && BIT(m_soundlatch_ack, 7))
		m_soundlatch->clear_w();

	m_soundlatch_ack = data & 0x80;

	if (data & 0x7c)
		logerror("soundbank_w: %02x\n", data);
}

// Sound design looks a link between armedf.cpp and nichisnd
void nichild_state::audio_map(address_map &map)
{
	map(0x0000, 0x77ff).rom().region("audiorom", 0);
	map(0x7800, 0x7fff).ram();
	map(0x8000, 0xffff).bankr(m_soundbank);
}

void nichild_state::audio_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r(m_soundlatch, FUNC(generic_latch_8_device::read)).nopw();
	map(0x02, 0x02).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x04, 0x04).w(FUNC(nichild_state::soundbank_w));
	map(0x06, 0x06).nopw(); // irq ack
	map(0x80, 0x81).w("ymsnd", FUNC(ym3812_device::write));
}


static INPUT_PORTS_START( nichild_mj )
	PORT_INCLUDE(nbmjctrl)

	PORT_START("PORTD")
	PORT_DIPNAME( 0x01, 0x01, "PORTD" )
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
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWA")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSWA:1") // those three are probably difficulty
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSWA:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSWA:3")
	PORT_DIPNAME(0x08, 0x08, DEF_STR( Coinage )) PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(0x08, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(0x00, DEF_STR( 1C_2C ))
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSWA:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSWA:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSWA:7")
	PORT_DIPNAME( 0x80, 0x80, "Background" ) PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, "Green" )
	PORT_DIPSETTING(    0x00, "Video Playback" )

	PORT_START("DSWB")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSWB:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSWB:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSWB:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSWB:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSWB:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSWB:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSWB:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSWB:8")
INPUT_PORTS_END

static INPUT_PORTS_START( nichild_quiz )
	// the quiz game(s) accesses the matrix with 0x00 writes, so that any of these works
	PORT_START("P1_KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("D Button") PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("C Button") PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B Button") PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A Button") PORT_PLAYER(1)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_KEY1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_KEY2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_KEY3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_KEY4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("D Button") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("C Button") PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B Button") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A Button") PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2_KEY4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PORTD")
	PORT_DIPNAME( 0x01, 0x01, "PORTD" )
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
	PORT_SERVICE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWA")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSWA:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSWA:2")
	PORT_DIPNAME(0x04, 0x04, DEF_STR( Coinage )) PORT_DIPLOCATION("DSWA:3")
	PORT_DIPSETTING(0x04, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(0x00, DEF_STR( 1C_2C ))
	PORT_DIPNAME(0x08, 0x08, DEF_STR( Lives )) PORT_DIPLOCATION("DSWA:4")
	PORT_DIPSETTING(0x08, "3")
	PORT_DIPSETTING(0x00, "5")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSWA:5")
	// at least for ldquiz4, to be verified for other games
	// (definitely don't affect sound in shabdama unless it expects attract mode audio from LD player)
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("DSWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Video Playback In Attract Mode" ) PORT_DIPLOCATION("DSWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "RGB Test Screen" ) PORT_DIPLOCATION("DSWA:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSWB:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSWB:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSWB:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSWB:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSWB:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSWB:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSWB:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSWB:8")
INPUT_PORTS_END


void nichild_state::machine_start()
{
	m_soundbank->configure_entries(0, 3, m_sound_rom + 0x8000, 0x8000);
	m_soundbank->set_entry(0);
}

void nichild_state::machine_reset()
{
}

static const z80_daisy_config daisy_chain_main[] =
{
	TMPZ84C011_DAISY_INTERNAL,
	{ nullptr }
};


void nichild_state::nichild(machine_config &config)
{
	TMPZ84C011(config, m_maincpu, MAIN_CLOCK/2);
	m_maincpu->set_daisy_config(daisy_chain_main);
	m_maincpu->set_addrmap(AS_PROGRAM, &nichild_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &nichild_state::main_io);
	m_maincpu->in_pa_callback().set(FUNC(nichild_state::porta_r));
	m_maincpu->in_pb_callback().set(FUNC(nichild_state::p1_keymatrix_r));
	m_maincpu->in_pc_callback().set(FUNC(nichild_state::p2_keymatrix_r));
	m_maincpu->in_pd_callback().set_ioport("PORTD");
	m_maincpu->out_pa_callback().set(FUNC(nichild_state::porta_w));
	m_maincpu->out_pe_callback().set(FUNC(nichild_state::gfxbank_w));

	Z80(config, m_audiocpu, SOUND_CLOCK);
	m_audiocpu->set_addrmap(AS_PROGRAM, &nichild_state::audio_map);
	m_audiocpu->set_addrmap(AS_IO, &nichild_state::audio_io);
	m_audiocpu->set_periodic_int(FUNC(nichild_state::irq0_line_hold), attotime::from_hz(XTAL(SOUND_CLOCK)/512)); // ?

	TTL166(config, m_dsw_shifter[0]);
	m_dsw_shifter[0]->data_callback().set([this]() { return bitswap<8>(m_dsw[1]->read(), 0, 1, 2, 3, 4, 5, 6, 7); }); // DSWB
	m_dsw_shifter[0]->qh_callback().set(m_dsw_shifter[1], FUNC(ttl166_device::serial_w));

	TTL166(config, m_dsw_shifter[1]);
	m_dsw_shifter[1]->data_callback().set([this]() { return bitswap<8>(m_dsw[0]->read(), 0, 1, 2, 3, 4, 5, 6, 7); }); // DSWA
	m_dsw_shifter[1]->qh_callback().set([this](int state) { m_dsw_data = state; });

	V9938(config, m_v9938, VDP_CLOCK);
	m_v9938->set_screen_ntsc("screen");
	m_v9938->set_vram_size(0x40000);
	m_v9938->int_cb().set(m_maincpu, FUNC(tmpz84c011_device::trg3)).invert();

	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	// TODO: mixing with LD player
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	YM3812(config, "ymsnd", SOUND_CLOCK).add_route(ALL_OUTPUTS, "speaker", 0.5);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 1.0); // unknown DAC
}


/***************************************************************************

  Machine driver(s)

***************************************************************************/

// NOTE: identical to shabdama below
ROM_START( ldmj1mbh )
	ROM_REGION( 0x10000, "ipl", ROMREGION_ERASE00 )
	ROM_LOAD( "1.bin",        0x000000, 0x010000, CRC(e49e3d73) SHA1(6d17d60e1b6f8aee96f7a09f45113030064d3bdb) )

	ROM_REGION( 0x20000, "audiorom", ROMREGION_ERASE00 )
	ROM_LOAD( "3.bin",        0x000000, 0x010000, CRC(e8233c6e) SHA1(fbfdb03dc9f4e3e80e161b8522b676485ffb1c95) )
	ROM_LOAD( "2.bin",        0x010000, 0x010000, CRC(3e0b5344) SHA1(eeae36fc4fca091065c1d51f05c2d11f44fe6d13) )

	ROM_REGION( 0x200000, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "4.bin",        0x000000, 0x010000, CRC(199e2127) SHA1(2514d51cb06438b312d1f328c72baa739280416a) )
	ROM_LOAD( "5.bin",        0x010000, 0x010000, CRC(0706386a) SHA1(29eee363775869dcc9c46285632e8bf745c9110b) )
	ROM_LOAD( "6.bin",        0x020000, 0x010000, CRC(0fece809) SHA1(1fe8436af8ead02a3b517b6306f9824cd64b2d26) )
	ROM_LOAD( "7.bin",        0x030000, 0x010000, CRC(7f08e3a6) SHA1(127018442183332175c9e1f558274cd2cb5f0147) )
	ROM_LOAD( "8.bin",        0x040000, 0x010000, CRC(3e75423e) SHA1(62e24849ddeb004ed8570d2884afa4ab257cdf07) )
	ROM_LOAD( "9.bin",        0x050000, 0x010000, CRC(1afdc5bf) SHA1(b07b32656ffc96b7f7d4bd242b2a6e0e105ab67a) )
	ROM_LOAD( "10.bin",       0x060000, 0x010000, CRC(5da10b82) SHA1(72974d083110fc6c583bfa1c22ce3abe02ba86f6) )

	ROM_REGION( 0x800, "plds", 0 ) // all protected
	ROM_LOAD( "pal16l8.0", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.1", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.2", 0x400, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.3", 0x600, 0x104, NO_DUMP )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "ldmj1mbh", 0, NO_DUMP )
ROM_END

ROM_START( shabdama )
	ROM_REGION( 0x10000, "ipl", ROMREGION_ERASE00 )
	ROM_LOAD( "1.bin",        0x000000, 0x010000, CRC(e49e3d73) SHA1(6d17d60e1b6f8aee96f7a09f45113030064d3bdb) )

	ROM_REGION( 0x20000, "audiorom", ROMREGION_ERASE00 )
	ROM_LOAD( "3.bin",        0x000000, 0x010000, CRC(e8233c6e) SHA1(fbfdb03dc9f4e3e80e161b8522b676485ffb1c95) )
	ROM_LOAD( "2.bin",        0x010000, 0x010000, CRC(3e0b5344) SHA1(eeae36fc4fca091065c1d51f05c2d11f44fe6d13) )

	ROM_REGION( 0x200000, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "4.bin",        0x000000, 0x010000, CRC(199e2127) SHA1(2514d51cb06438b312d1f328c72baa739280416a) )
	ROM_LOAD( "5.bin",        0x010000, 0x010000, CRC(0706386a) SHA1(29eee363775869dcc9c46285632e8bf745c9110b) )
	ROM_LOAD( "6.bin",        0x020000, 0x010000, CRC(0fece809) SHA1(1fe8436af8ead02a3b517b6306f9824cd64b2d26) )
	ROM_LOAD( "7.bin",        0x030000, 0x010000, CRC(7f08e3a6) SHA1(127018442183332175c9e1f558274cd2cb5f0147) )
	ROM_LOAD( "8.bin",        0x040000, 0x010000, CRC(3e75423e) SHA1(62e24849ddeb004ed8570d2884afa4ab257cdf07) )
	ROM_LOAD( "9.bin",        0x050000, 0x010000, CRC(1afdc5bf) SHA1(b07b32656ffc96b7f7d4bd242b2a6e0e105ab67a) )
	ROM_LOAD( "10.bin",       0x060000, 0x010000, CRC(5da10b82) SHA1(72974d083110fc6c583bfa1c22ce3abe02ba86f6) )

	ROM_REGION( 0x800, "plds", 0 ) // all protected
	ROM_LOAD( "pal16l8.0", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.1", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.2", 0x400, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.3", 0x600, 0x104, NO_DUMP )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "shabdama", 0, NO_DUMP )
ROM_END


// LD QUIZ 第4弾 答えたもん勝ち!

ROM_START( ldquiz4 )
	ROM_REGION( 0x10000, "ipl", 0 ) // 27512
	ROM_LOAD( "1.e3", 0x00000,  0x10000, CRC(49255f66) SHA1(bdd01987331c2aadea7f588d39c48c70cd43fc71) )

	ROM_REGION( 0x20000, "audiorom", 0 ) // 27512
	ROM_LOAD( "3.e7", 0x00000,  0x10000, CRC(b033eb6a) SHA1(2c11b2b998117f68a1fbbd110d3f67ab472e133d) )
	ROM_LOAD( "2.e6", 0x10000,  0x10000, CRC(6c83cad6) SHA1(c38f60fb4fdbda76ea3459644bf491cc305a7ae6) )

	ROM_REGION( 0x200000, "gfx", ROMREGION_ERASE00 ) // 27010
	ROM_LOAD( "4.k1",   0x000000, 0x20000, CRC(9cdf8114) SHA1(99e6b9bb43c6df320fdb1ea8599967b707f7f18d) )
	ROM_LOAD( "5.k2",   0x020000, 0x20000, CRC(7746a909) SHA1(c69a45159d15e8897a5999e57b519ed6fc0d9812) )
	ROM_LOAD( "6.k3",   0x040000, 0x20000, CRC(3b3e63ad) SHA1(92898ade77ad267978a469b03c9113f4e5a47288) )
	ROM_LOAD( "7.k4",   0x060000, 0x20000, CRC(cdcaae32) SHA1(bfc07524a9859592bf7c6397fd80570b4e5e15fc) )
	ROM_LOAD( "8.k5",   0x080000, 0x20000, CRC(c08b90e6) SHA1(add35812ea98ac44299b7f165efeee268aa57132) )
	ROM_LOAD( "9.k6",   0x0a0000, 0x20000, CRC(72c1a283) SHA1(8dbfc5892d719033dff82c70f13c1d7c63173240) )
	ROM_LOAD( "10.k8",  0x0c0000, 0x20000, CRC(c7437125) SHA1(55b161ce2432d04531ed0afab973f892b571ef88) )
	ROM_LOAD( "11.k9",  0x0e0000, 0x20000, CRC(6feeab93) SHA1(d77325c1eecb677c48d11bf8d5f73b238f2896e6) )
	ROM_LOAD( "12.k10", 0x100000, 0x20000, CRC(c7f9bf98) SHA1(103b78b0e126ea4249982bf114010f57e5ffa70a) )
	ROM_LOAD( "13",     0x180000, 0x08000, NO_DUMP )

	ROM_REGION( 0x800, "plds", 0 ) // all protected
	ROM_LOAD( "pal16l8.0", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.1", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.2", 0x400, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.3", 0x600, 0x104, NO_DUMP )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "ldquiz4", 0, NO_DUMP )
ROM_END

} // anonymous namespace

// 1990
// LD花札 花のクリスマスイブ (LD version of nbmj8891.cpp hnxmasev?)
// 1991
GAME( 1991, ldmj1mbh, 0,   nichild, nichild_mj,   nichild_state, empty_init, ROT0, "Nichibutsu / AV Japan", "LD Mahjong #1 Marine Blue no Hitomi (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND ) // LD麻雀 第1弾 マリンブルーの瞳
// LD麻雀 第2弾 マリンブルーの瞳2
// LD麻雀 第3弾 泊まりにおいでよ
GAME( 1991, shabdama, 0,   nichild, nichild_mj,   nichild_state, empty_init, ROT0, "Nichibutsu / AV Japan", "LD Mahjong #4 Shabon-Dama (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND ) // LD麻雀 第4弾 シャボン玉
// LDQUIZ クイズDEデート
// LDQUIZ ミラクルQ 日本物産
// LDQUIZ もう答えずにはいられない
// 1992
GAME( 1992, ldquiz4,  0,   nichild, nichild_quiz, nichild_state, empty_init, ROT0, "Nichibutsu / AV Japan", "LD Quiz dai 4-dan - Kotaetamon Gachi! (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND ) // LDQUIZ 答えたもん勝ち
// LD麻雀 第5弾 夜明けのカフェテラス (LD A8165)
// LD麻雀 第6弾 ティファニー
// 1993
// LD麻雀 第7弾 ジェラシー
// LD麻雀 第8弾 ブルセラ (LD A8190M)
// 1994
// LD麻雀 第9弾 ポケベル1919
// LD麻雀 第10弾 ボディコン総集編
// LD麻雀 第11弾 エロスの館
