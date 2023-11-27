// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/**************************************************************************************************

'Nichibutsu LD' HW (c) 199? Nichibutsu

TODO:
- ldquiz4: spins on "memory test error 13", implying a missing ROM dump (other GFXs will return further errors if missing, returning the label there);
- Unknown LaserDisc type;
- Unknown irq vector for LaserDisc strobe (ldquiz4 sets a flag at $f014,
  the only place it clears it is at snippet 0x0ED6);
- V9938 has issues with layer clears, implicitly cleared by superimposing or irq timing?
- Complete audio section, SFXs keeps ringing;
- Document meaning of DIP switches

===================================================================================================

    1 x TMPZ84C011AF-6 main CPU
    1 x 21.47727MHz OSC
    1 x Z0840004PSC audio CPU
    1 x 4.000MHz OSC
    1 x Yamaha V9938
    1 x uPC1352C
    1 x Yamaha YM3812
    2 x 8 dip switch banks

**************************************************************************************************/


#include "emu.h"

#include "cpu/z80/tmpz84c011.h"
#include "machine/74166.h"
#include "machine/gen_latch.h"
#include "sound/dac.h"
#include "sound/ymopl.h"
#include "video/v9938.h"

#include "screen.h"
#include "speaker.h"


namespace {

#define MAIN_CLOCK XTAL(21'477'272)
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
		, m_p1_keymatrix(*this, { "P1_KEY0", "P1_KEY1", "P1_KEY2", "P1_KEY3", "P1_KEY4" })
		, m_p2_keymatrix(*this, { "P2_KEY0", "P2_KEY1", "P2_KEY2", "P2_KEY3", "P2_KEY4" })
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

	required_ioport_array<5> m_p1_keymatrix;
	required_ioport_array<5> m_p2_keymatrix;
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
	void portb_w(uint8_t data);
	void portc_w(uint8_t data);
	void portd_w(uint8_t data);
	void gfxbank_w(uint8_t data);

	void main_map(address_map &map);
	void main_io(address_map &map);

	void soundbank_w(uint8_t data);

	void audio_map(address_map &map);
	void audio_io(address_map &map);

	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t m_gfx_bank = 0;
	uint8_t m_key_select = 0;
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

void nichild_state::portb_w(uint8_t data)
{
	logerror("PORTB %02x\n",data);
}

void nichild_state::portc_w(uint8_t data)
{
	logerror("PORTC %02x\n",data);
}

void nichild_state::portd_w(uint8_t data)
{
	logerror("PORTD %02x\n",data);
}

void nichild_state::gfxbank_w(uint8_t data)
{
	// TODO: ldquiz4 checks up to 0x30, what for?
	m_gfx_bank = data * 0x8000;
}

uint8_t nichild_state::p1_keymatrix_r()
{
	uint8_t result = 0xff;
	for (unsigned i = 0; m_p1_keymatrix.size() > i; ++i)
	{
		if (!BIT(m_key_select, i))
			result &= m_p1_keymatrix[i]->read();
	}
	return result;
}

uint8_t nichild_state::p2_keymatrix_r()
{
	uint8_t result = 0xff;
	for (unsigned i = 0; m_p2_keymatrix.size() > i; ++i)
	{
		if (!BIT(m_key_select, i))
			result &= m_p2_keymatrix[i]->read();
	}
	return result;
}

void nichild_state::key_select_w(uint8_t data)
{
	m_key_select = (data & 0x1f);
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
	// TODO: bit 7 used often with 0 -> 1 transitions
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


static INPUT_PORTS_START( nichild )
	// mahjong panels are virtually identical to nb1413m3
	PORT_START("P1_KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P1_KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2_KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2_KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2_KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2_KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2_KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

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
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSWA:8")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSWA:7")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSWA:6")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSWA:5")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSWA:4")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSWA:3")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSWA:2")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSWA:1")

	PORT_START("DSWB")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSWB:8")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSWB:7")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSWB:6")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSWB:5")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSWB:4")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSWB:3")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSWB:2")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSWB:1")
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
	TMPZ84C011(config, m_maincpu, MAIN_CLOCK/4);
	m_maincpu->set_daisy_config(daisy_chain_main);
	m_maincpu->set_addrmap(AS_PROGRAM, &nichild_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &nichild_state::main_io);
	m_maincpu->in_pa_callback().set(FUNC(nichild_state::porta_r));
	m_maincpu->in_pb_callback().set(FUNC(nichild_state::p1_keymatrix_r));
	m_maincpu->in_pc_callback().set(FUNC(nichild_state::p2_keymatrix_r));
	m_maincpu->in_pd_callback().set_ioport("PORTD");
	m_maincpu->out_pa_callback().set(FUNC(nichild_state::porta_w));
	m_maincpu->out_pb_callback().set(FUNC(nichild_state::portb_w));
	m_maincpu->out_pc_callback().set(FUNC(nichild_state::portc_w));
	m_maincpu->out_pd_callback().set(FUNC(nichild_state::portd_w));
	m_maincpu->out_pe_callback().set(FUNC(nichild_state::gfxbank_w));

	Z80(config, m_audiocpu, SOUND_CLOCK);
	m_audiocpu->set_addrmap(AS_PROGRAM, &nichild_state::audio_map);
	m_audiocpu->set_addrmap(AS_IO, &nichild_state::audio_io);
	m_audiocpu->set_periodic_int(FUNC(nichild_state::irq0_line_hold), attotime::from_hz(XTAL(SOUND_CLOCK)/512)); // ?

	TTL166(config, m_dsw_shifter[0]);
	m_dsw_shifter[0]->data_callback().set_ioport("DSWB");
	m_dsw_shifter[0]->qh_callback().set(m_dsw_shifter[1], FUNC(ttl166_device::serial_w));

	TTL166(config, m_dsw_shifter[1]);
	m_dsw_shifter[1]->data_callback().set_ioport("DSWA");
	m_dsw_shifter[1]->qh_callback().set([this](int state) { m_dsw_data = state; });

	V9938(config, m_v9938, MAIN_CLOCK);
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
	ROM_LOAD( "13",     0x120000, 0x20000, NO_DUMP )

	ROM_REGION( 0x800, "plds", 0 ) // all protected
	ROM_LOAD( "pal16l8.0", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.1", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.2", 0x400, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.3", 0x600, 0x104, NO_DUMP )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "ldquiz4", 0, NO_DUMP )
ROM_END

} // anonymous namespace


GAME( 1991, shabdama, 0,   nichild, nichild, nichild_state, empty_init, ROT0, "Nichibutsu / AV Japan", "LD Mahjong #4 Shabon-Dama (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
GAME( 1992, ldquiz4,  0,   nichild, nichild, nichild_state, empty_init, ROT0, "Nichibutsu", "LD Quiz dai 4-dan - Kotaetamon Gachi! (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
