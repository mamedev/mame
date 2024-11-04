// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
/***************************************************************************

Karate Champ - (c) 1984 Data East

driver by Ernesto Corvi


Changes:
(1999/11/11 Takahiro Nogi)
Changed "karatedo" to "karatevs".
Supported "karatedo".
Corrected DIPSW settings.(kchamp/karatedo)

Currently supported sets:
Karate Champ - 1 Player Version (kchamp)
Karate Champ - VS Version (kchampvs)
Karate Champ - 1 Player Version Japanese (karatedo)
Karate Champ - VS Version Japanese (karatevs)

VS Version Info:
---------------
Memory Map:
Main CPU
0000-bfff ROM (encrypted)
c000-cfff RAM
d000-d3ff char videoram
d400-d7ff color videoram
d800-d8ff sprites
e000-ffff ROM (encrypted)

Sound CPU
0000-5fff ROM
6000-6300 RAM

IO Ports:
Main CPU
INPUT  00 = Player 1 Controls - ( ACTIVE LOW )
INPUT  40 = Player 2 Controls - ( ACTIVE LOW )
INPUT  80 = Coins and Start Buttons - ( ACTIVE LOW )
INPUT  C0 = Dip Switches - ( ACTIVE LOW )
OUTPUT 00 = Screen Flip
OUTPUT 01 = CPU Control
                bit 0 = external nmi enable
OUTPUT 02 = Sound Reset
OUTPUT 40 = Sound latch write

Sound CPU
INPUT  01 = Sound latch read
OUTPUT 00 = AY8910 #1 data write
OUTPUT 01 = AY8910 #1 control write
OUTPUT 02 = AY8910 #2 data write
OUTPUT 03 = AY8910 #2 control write
OUTPUT 04 = MSM5205 write
OUTPUT 05 = CPU Control
                bit 0 = MSM5205 trigger
                bit 1 = external nmi enable

1P Version Info:
---------------
Same as VS version but with a DAC instead of a MSM5205. Also some minor
IO ports and memory map changes. Dip switches differ too.

***************************************************************************/

#include "emu.h"
#include "kchamp.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "screen.h"
#include "speaker.h"


/********************
* VS Version        *
********************/

void kchamp_state::nmi_enable_w(int state)
{
	m_nmi_enable = state;
	if (!m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void kchamp_state::sound_reset_w(int state)
{
	m_audiocpu->set_input_line(INPUT_LINE_RESET, state ? CLEAR_LINE : ASSERT_LINE);
	if (!state)
	{
		m_ay[0]->reset();
		m_ay[1]->reset();
		sound_control_w(0);
	}
}

void kchamp_state::sound_control_w(u8 data)
{
	m_msm->reset_w(!(data & 1));
	m_sound_nmi_enable = ((data >> 1) & 1);
	if (!m_sound_nmi_enable)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void kchamp_state::kchampvs_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xcfff).ram();
	map(0xd000, 0xd3ff).ram().w(FUNC(kchamp_state::kchamp_videoram_w)).share("videoram");
	map(0xd400, 0xd7ff).ram().w(FUNC(kchamp_state::kchamp_colorram_w)).share("colorram");
	map(0xd800, 0xd8ff).ram().share("spriteram");
	map(0xd900, 0xdfff).ram();
	map(0xe000, 0xffff).rom();
}

void kchamp_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0xffff).rom().share("decrypted_opcodes");
}

void kchamp_state::kchampvs_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("P1");
	map(0x00, 0x07).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0x40, 0x40).portr("P2").w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x80, 0x80).portr("SYSTEM");
	map(0xc0, 0xc0).portr("DSW");
}

void kchamp_state::kchampvs_sound_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0xffff).ram();
}

void kchamp_state::kchampvs_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("ay1", FUNC(ay8910_device::data_address_w));
	map(0x01, 0x01).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x02, 0x03).w("ay2", FUNC(ay8910_device::data_address_w));
	map(0x04, 0x04).w(m_adpcm_select, FUNC(ls157_device::ab_w));
	map(0x05, 0x05).w(FUNC(kchamp_state::sound_control_w));
}


/********************
* 1 Player Version  *
********************/

uint8_t kchamp_state::sound_reset_r()
{
	if (!machine().side_effects_disabled())
		m_audiocpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
	return 0;
}

void kchamp_state::kc_sound_control_w(offs_t offset, uint8_t data)
{
	if (offset == 0)
	{
		m_sound_nmi_enable = BIT(data, 7);
		if (!m_sound_nmi_enable)
			m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
	else
		m_dac->set_output_gain(0, BIT(data, 0) ? 1.0 : 0);
}

void kchamp_state::kchamp_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe3ff).ram().w(FUNC(kchamp_state::kchamp_videoram_w)).share("videoram");
	map(0xe400, 0xe7ff).ram().w(FUNC(kchamp_state::kchamp_colorram_w)).share("colorram");
	map(0xea00, 0xeaff).ram().share("spriteram");
	map(0xeb00, 0xffff).ram();
}

void kchamp_state::kchamp_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x80, 0x80).portr("DSW");
	map(0x80, 0x87).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0x90, 0x90).portr("P1");
	map(0x98, 0x98).portr("P2");
	map(0xa0, 0xa0).portr("SYSTEM");
	map(0xa8, 0xa8).r(FUNC(kchamp_state::sound_reset_r)).w(m_soundlatch, FUNC(generic_latch_8_device::write));
}

void kchamp_state::kchamp_sound_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();
	map(0xe000, 0xe2ff).ram();
}

void kchamp_state::kchamp_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x01).w("ay1", FUNC(ay8910_device::data_address_w));
	map(0x02, 0x03).w("ay2", FUNC(ay8910_device::data_address_w));
	map(0x04, 0x04).w(m_dac, FUNC(dac_byte_interface::data_w));
	map(0x05, 0x05).w(FUNC(kchamp_state::kc_sound_control_w));
	map(0x06, 0x06).r(m_soundlatch, FUNC(generic_latch_8_device::read));
}

static INPUT_PORTS_START( kchampvs )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_4WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_4WAY PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/********************
* 1 Player Version  *
********************/

static INPUT_PORTS_START( kchamp )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_4WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_4WAY PORT_PLAYER(2)

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )
INPUT_PORTS_END

static const gfx_layout tilelayout =
{
	8,8,    // tile size
	256*8,  // number of tiles
	2,  // bits per pixel
	{ 0x4000*8, 0 }, // plane offsets
	{ 0,1,2,3,4,5,6,7 }, // x offsets
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 }, // y offsets
	8*8 // offset to next tile
};

static const gfx_layout spritelayout =
{
	16,16,  // tile size
	512,    // number of tiles
	2,  // bits per pixel
	{ 0xC000*8, 0 }, // plane offsets
	{ 0,1,2,3,4,5,6,7,
		0x2000*8+0,0x2000*8+1,0x2000*8+2,0x2000*8+3,
		0x2000*8+4,0x2000*8+5,0x2000*8+6,0x2000*8+7 }, // x offsets
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8,
	8*8,9*8,10*8,11*8,12*8,13*8,14*8, 15*8 }, // y offsets
	16*8    // ofset to next tile
};

static GFXDECODE_START( gfx_kchamp )
	GFXDECODE_ENTRY( "gfx1", 0x00000, tilelayout,   32*4, 32 )
	GFXDECODE_ENTRY( "gfx2", 0x08000, spritelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x04000, spritelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x00000, spritelayout, 0, 16 )
GFXDECODE_END


void kchamp_state::vblank_irq(int state)
{
	if (state && m_nmi_enable)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void kchamp_state::msmint(int state)
{
	if (!state)
		return;

	m_msm_play_lo_nibble = !m_msm_play_lo_nibble;
	m_adpcm_select->select_w(m_msm_play_lo_nibble);

	if (m_msm_play_lo_nibble && m_sound_nmi_enable)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

/********************
* 1 Player Version  *
********************/

INTERRUPT_GEN_MEMBER(kchamp_state::sound_int)
{
	if (m_sound_nmi_enable)
		device.execute().set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}


MACHINE_START_MEMBER(kchamp_state,kchamp)
{
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_sound_nmi_enable));
}

MACHINE_START_MEMBER(kchamp_state,kchampvs)
{
	MACHINE_START_CALL_MEMBER(kchamp);

	save_item(NAME(m_msm_play_lo_nibble));
}

void kchamp_state::machine_reset()
{
	m_nmi_enable = 0;
	m_sound_nmi_enable = 0;
}

void kchamp_state::kchampvs(machine_config &config)
{
	// Basic machine hardware
	Z80(config, m_maincpu, XTAL(12'000'000)/4);     // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &kchamp_state::kchampvs_map);
	m_maincpu->set_addrmap(AS_IO, &kchamp_state::kchampvs_io_map);
	m_maincpu->set_addrmap(AS_OPCODES, &kchamp_state::decrypted_opcodes_map);

	Z80(config, m_audiocpu, XTAL(12'000'000)/4);    // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &kchamp_state::kchampvs_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &kchamp_state::kchampvs_sound_io_map);
	// IRQs triggered from main CPU
	// NMIs from MSM5205

	ls259_device &mainlatch(LS259(config, "mainlatch")); // 8C
	mainlatch.q_out_cb<0>().set(FUNC(kchamp_state::flip_screen_set));
	mainlatch.q_out_cb<1>().set(FUNC(kchamp_state::nmi_enable_w));
	mainlatch.q_out_cb<2>().set(FUNC(kchamp_state::sound_reset_w));

	MCFG_MACHINE_START_OVERRIDE(kchamp_state,kchampvs)

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.10); // verified on PCB
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(kchamp_state::screen_update_kchampvs));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(kchamp_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_kchamp);
	PALETTE(config, m_palette, FUNC(kchamp_state::kchamp_palette), 256);

	// Sound hardware
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);

	AY8910(config, m_ay[0], XTAL(12'000'000)/8).add_route(ALL_OUTPUTS, "speaker", 0.3);    // verified on PCB
	AY8910(config, m_ay[1], XTAL(12'000'000)/8).add_route(ALL_OUTPUTS, "speaker", 0.3);    // verified on PCB

	LS157(config, m_adpcm_select, 0); // at 4C
	m_adpcm_select->out_callback().set("msm", FUNC(msm5205_device::data_w));

	MSM5205(config, m_msm, 375000);  // verified on PCB, discrete circuit clock
	m_msm->vck_callback().set(FUNC(kchamp_state::msmint));  // interrupt function
	m_msm->set_prescaler_selector(msm5205_device::S96_4B);  // 1 / 96 = 3906.25Hz playback
	m_msm->add_route(ALL_OUTPUTS, "speaker", 1.0);
}

/********************
* 1 Player Version  *
********************/

void kchamp_state::kchamp(machine_config &config)
{
	// Basic machine hardware
	Z80(config, m_maincpu, XTAL(12'000'000)/4);     // 12MHz / 4 = 3.0 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &kchamp_state::kchamp_map);
	m_maincpu->set_addrmap(AS_IO, &kchamp_state::kchamp_io_map);

	Z80(config, m_audiocpu, XTAL(12'000'000)/4);    // 12MHz / 4 = 3.0 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &kchamp_state::kchamp_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &kchamp_state::kchamp_sound_io_map);
	m_audiocpu->set_periodic_int(FUNC(kchamp_state::sound_int), attotime::from_hz(125)); // Hz
	// IRQs triggered from main CPU
	// NMIs from 125Hz clock

	ls259_device &mainlatch(LS259(config, "mainlatch")); // IC71
	mainlatch.q_out_cb<0>().set(FUNC(kchamp_state::flip_screen_set));
	mainlatch.q_out_cb<1>().set(FUNC(kchamp_state::nmi_enable_w));

	MCFG_MACHINE_START_OVERRIDE(kchamp_state,kchamp)

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(kchamp_state::screen_update_kchamp));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(kchamp_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_kchamp);
	PALETTE(config, m_palette, FUNC(kchamp_state::kchamp_palette), 256);

	// Sound hardware
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, 0);

	AY8910(config, m_ay[0], XTAL(12'000'000)/12).add_route(ALL_OUTPUTS, "speaker", 0.3); // Guess based on actual PCB recordings of karatedo
	AY8910(config, m_ay[1], XTAL(12'000'000)/12).add_route(ALL_OUTPUTS, "speaker", 0.3); // Guess based on actual PCB recordings of karatedo

	DAC08(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.3); // IC11
}

void kchamp_state::kchamp_arfyc(machine_config &config)
{
	kchamp(config);

	m_audiocpu->set_clock(XTAL(8'867'238)/2); // 8.867238 MHz xtal / 2, measured on real PCB
	m_ay[0]->set_clock(XTAL(8'867'238)/8);    // 8.867238 MHz xtal / 8, measured on real PCB
	m_ay[1]->set_clock(XTAL(8'867'238)/8);    // 8.867238 MHz xtal / 8, measured on real PCB
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( kchamp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "b014.bin", 0x0000, 0x2000, CRC(0000d1a0) SHA1(0c584096825e1d3cc718e0bda1abb897a7f4d2df) )
	ROM_LOAD( "b015.bin", 0x2000, 0x2000, CRC(03fae67e) SHA1(3b6a30f39f5ad512415e3b8ba6e07f3118e28d9e) )
	ROM_LOAD( "b016.bin", 0x4000, 0x2000, CRC(3b6e1d08) SHA1(ecf7d2b0f31f04c0be7d5a1f450b9c95d9f54d80) )
	ROM_LOAD( "b017.bin", 0x6000, 0x2000, CRC(c1848d1a) SHA1(eb5f85d88e170e864d0cd4c372be2a193935caa2) )
	ROM_LOAD( "b018.bin", 0x8000, 0x2000, CRC(b824abc7) SHA1(4a30fec025150e889600a78497700155e743c99e) )
	ROM_LOAD( "b019.bin", 0xa000, 0x2000, CRC(3b487a46) SHA1(7837e480fd4648e0d3f792b79fa0019e063abdc6) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "b026.bin", 0x0000, 0x2000, CRC(999ed2c7) SHA1(f01e4ee81f8f7b0d1cf001d3ec01a9210f8109b4) )
	ROM_LOAD( "b025.bin", 0x2000, 0x2000, CRC(33171e07) SHA1(55ee74c9f1d86ec13d92ea0d1b700bbe24b79def) ) // ADPCM
	ROM_LOAD( "b024.bin", 0x4000, 0x2000, CRC(910b48b9) SHA1(c6a2f8266ff1f14b462b92d47a4a86542df77cdd) ) // ADPCM
	ROM_LOAD( "b023.bin", 0x6000, 0x2000, CRC(47f66aac) SHA1(484802cfbff7c5f51dd97cb3b2321e137b03481c) )
	ROM_LOAD( "b022.bin", 0x8000, 0x2000, CRC(5928e749) SHA1(a4dbd6f2a6a7aa9597875dfd781e55b0fb14d49b) )
	ROM_LOAD( "b021.bin", 0xa000, 0x2000, CRC(ca17e3ba) SHA1(91a3ccd6045dcef5f3293d669fe5a4df59cd954b) )
	ROM_LOAD( "b020.bin", 0xc000, 0x2000, CRC(ada4f2cd) SHA1(15a4ed7497cb6c06f523ebe38bc4eb6dbcd09549) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "b000.bin", 0x00000, 0x2000, CRC(a4fa98a1) SHA1(33b9a1a56d72ffa5f4e16b69e6e19af5a2882b2c) )  // plane0, tiles
	ROM_LOAD( "b001.bin", 0x04000, 0x2000, CRC(fea09f7c) SHA1(174fc8022c455438538e6a3b67c7effc857ae634) )  // plane1, tiles

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "b013.bin", 0x00000, 0x2000, CRC(eaad4168) SHA1(f31b05ffb86677157f3a44cdcf0f9a729e0ab259) )  // Top, plane0, sprites
	ROM_LOAD( "b004.bin", 0x02000, 0x2000, CRC(10a47e2d) SHA1(97fe2de3ce2b8dc017dceffce494be18695708d2) )  // Bot, plane0, sprites
	ROM_LOAD( "b012.bin", 0x04000, 0x2000, CRC(b4842ea9) SHA1(471475f65edbd292b9162ad50e5cb0c7144845b0) )  // Top, plane0, sprites
	ROM_LOAD( "b003.bin", 0x06000, 0x2000, CRC(8cd166a5) SHA1(4b623c4c0025d75b3ed9746f8b6730bf3e65d85a) )  // Bot, plane0, sprites
	ROM_LOAD( "b011.bin", 0x08000, 0x2000, CRC(4cbd3aa3) SHA1(a9a683dcc4f52b18450659a20434a4d2a7b411d9) )  // Top, plane0, sprites
	ROM_LOAD( "b002.bin", 0x0a000, 0x2000, CRC(6be342a6) SHA1(0b8ac7ef7c6a6464fbc027a9fd17fa7ce1ffd962) )  // Bot, plane0, sprites
	ROM_LOAD( "b007.bin", 0x0c000, 0x2000, CRC(cb91d16b) SHA1(bf32a03e7882b74280b29fa004429b77ad52e5ee) )  // Top, plane1, sprites
	ROM_LOAD( "b010.bin", 0x0e000, 0x2000, CRC(489c9c04) SHA1(d920ef4f03e1b2e871df0cb2d672689c89febe96) )  // Bot, plane1, sprites
	ROM_LOAD( "b006.bin", 0x10000, 0x2000, CRC(7346db8a) SHA1(d2b2c1700ae0ff9c614a9981a3da3d69879e9f25) )  // Top, plane1, sprites
	ROM_LOAD( "b009.bin", 0x12000, 0x2000, CRC(b78714fc) SHA1(4df7f15c37d56a9d66d0049aad65b32063e5c29a) )  // Bot, plane1, sprites
	ROM_LOAD( "b005.bin", 0x14000, 0x2000, CRC(b2557102) SHA1(ec4285029fc3ee1ad0adb05f363b234c67f8903d) )  // Top, plane1, sprites
	ROM_LOAD( "b008.bin", 0x16000, 0x2000, CRC(c85aba0e) SHA1(4be21b38623c2a8ae7f1e7397fb002e4cb9e4614) )  // Bot, plane1, sprites

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "br27", 0x0000, 0x0100, CRC(f683c54a) SHA1(92893990456b92f04a2be98b8e9626e97b7a2562) ) // Red
	ROM_LOAD( "br26", 0x0100, 0x0100, CRC(3ddbb6c4) SHA1(0eca5594d6812bc79f8b78f83fe003877d20c973) ) // Green
	ROM_LOAD( "br25", 0x0200, 0x0100, CRC(ba4a5651) SHA1(77e81bd64ab59a7466d20eabdff4be241e963c52) ) // Blue
ROM_END

ROM_START( kchamp2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "karate_champ_bs14-8.ic80",  0x00000, 0x2000, CRC(e9f93e89) SHA1(ae7fd8eab57049a1b6c30f7141e670c4137995c9) )
	ROM_LOAD( "karate_champ_bs15-8.ic74",  0x02000, 0x2000, CRC(572961f2) SHA1(c72df0c18d6b3f86eb96f1e5c08702058d7a2104) )
	ROM_LOAD( "karate_champ_bs16-8.ic66",  0x04000, 0x2000, CRC(651a02b9) SHA1(69a8eae7387d46f7c1e751a1b91180ec7279e037) )
	ROM_LOAD( "karate_champ_bs17-8.ic60",  0x06000, 0x2000, CRC(523968a4) SHA1(640d51561e479fd3554763cf8a793dc23af061da) )
	ROM_LOAD( "karate_champ_bs18-8.ic56",  0x08000, 0x2000, CRC(49a18c37) SHA1(0ffd330e890cf57110b04adda4c3a4961bde54cb) )
	ROM_LOAD( "karate_champ_bs19-8.ic49",  0x0a000, 0x2000, CRC(9a8c9311) SHA1(04ef9ebcee26945de0f13e0b110dee7f4af35e06) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "karate_champ_bo26-8.ic39",  0x00000, 0x2000, CRC(645650de) SHA1(254a865f0c49bae88efd000c948a9c12eaed5eaf) )
	ROM_LOAD( "karate_champ_bo25-8.ic44",  0x02000, 0x2000, CRC(33171e07) SHA1(55ee74c9f1d86ec13d92ea0d1b700bbe24b79def) ) // ADPCM
	ROM_LOAD( "karate_champ_bs24-8.ic50",  0x04000, 0x2000, CRC(910b48b9) SHA1(c6a2f8266ff1f14b462b92d47a4a86542df77cdd) ) // ADPCM
	ROM_LOAD( "karate_champ_bs23-8.ic57",  0x06000, 0x2000, CRC(47f66aac) SHA1(484802cfbff7c5f51dd97cb3b2321e137b03481c) )
	ROM_LOAD( "karate_champ_bs22-8.ic61",  0x08000, 0x2000, CRC(5928e749) SHA1(a4dbd6f2a6a7aa9597875dfd781e55b0fb14d49b) )
	ROM_LOAD( "karate_champ_bs21-8.ic67",  0x0a000, 0x2000, CRC(ca17e3ba) SHA1(91a3ccd6045dcef5f3293d669fe5a4df59cd954b) )
	ROM_LOAD( "karate_champ_bs20-8.ic75",  0x0c000, 0x2000, CRC(ada4f2cd) SHA1(15a4ed7497cb6c06f523ebe38bc4eb6dbcd09549) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "karate_champ_bs00-8.ic89",  0x00000, 0x2000, CRC(a4fa98a1) SHA1(33b9a1a56d72ffa5f4e16b69e6e19af5a2882b2c) ) // plane0, tiles
	ROM_LOAD( "karate_champ_bs01-8.ic91",  0x04000, 0x2000, CRC(fea09f7c) SHA1(174fc8022c455438538e6a3b67c7effc857ae634) ) // plane1, tiles

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "karate_champ_bs13-8.ic128", 0x00000, 0x2000, CRC(eaad4168) SHA1(f31b05ffb86677157f3a44cdcf0f9a729e0ab259) ) // Top, plane0, sprites
	ROM_LOAD( "karate_champ_bs04-8.ic116", 0x02000, 0x2000, CRC(10a47e2d) SHA1(97fe2de3ce2b8dc017dceffce494be18695708d2) ) // Bot, plane0, sprites
	ROM_LOAD( "karate_champ_bs12-8.ic127", 0x04000, 0x2000, CRC(b4842ea9) SHA1(471475f65edbd292b9162ad50e5cb0c7144845b0) ) // Top, plane0, sprites
	ROM_LOAD( "karate_champ_bs03-8.ic115", 0x06000, 0x2000, CRC(8cd166a5) SHA1(4b623c4c0025d75b3ed9746f8b6730bf3e65d85a) ) // Bot, plane0, sprites
	ROM_LOAD( "karate_champ_bs11-8.ic126", 0x08000, 0x2000, CRC(4cbd3aa3) SHA1(a9a683dcc4f52b18450659a20434a4d2a7b411d9) ) // Top, plane0, sprites
	ROM_LOAD( "karate_champ_bs02-8.ic114", 0x0a000, 0x2000, CRC(6be342a6) SHA1(0b8ac7ef7c6a6464fbc027a9fd17fa7ce1ffd962) ) // Bot, plane0, sprites
	ROM_LOAD( "karate_champ_bs07-8.ic120", 0x0c000, 0x2000, CRC(cb91d16b) SHA1(bf32a03e7882b74280b29fa004429b77ad52e5ee) ) // Top, plane1, sprites
	ROM_LOAD( "karate_champ_bs10-8.ic124", 0x0e000, 0x2000, CRC(489c9c04) SHA1(d920ef4f03e1b2e871df0cb2d672689c89febe96) ) // Bot, plane1, sprites
	ROM_LOAD( "karate_champ_bs06-8.ic119", 0x10000, 0x2000, CRC(7346db8a) SHA1(d2b2c1700ae0ff9c614a9981a3da3d69879e9f25) ) // Top, plane1, sprites
	ROM_LOAD( "karate_champ_bs09-8.ic123", 0x12000, 0x2000, CRC(b78714fc) SHA1(4df7f15c37d56a9d66d0049aad65b32063e5c29a) ) // Bot, plane1, sprites
	ROM_LOAD( "karate_champ_bs05-8.ic118", 0x14000, 0x2000, CRC(b2557102) SHA1(ec4285029fc3ee1ad0adb05f363b234c67f8903d) ) // Top, plane1, sprites
	ROM_LOAD( "karate_champ_bs08-8.ic122", 0x16000, 0x2000, CRC(c85aba0e) SHA1(4be21b38623c2a8ae7f1e7397fb002e4cb9e4614) ) // Bot, plane1, sprites

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "be27.ic34", 0x000, 0x100, CRC(f05bda76) SHA1(0842092e28a15de3b4c198e48f650dbbe3cc95ce) ) // Red
	ROM_LOAD( "be28.ic24", 0x100, 0x100, CRC(d26d6fa9) SHA1(72a30bdc3410f2704539a32aeef69ee803e39f82) ) // Green
	ROM_LOAD( "be29.ic18", 0x200, 0x100, CRC(111ccb15) SHA1(4928a7ccc8649520cad3b119ff0b0ca1276c95ae) ) // Blue
ROM_END

ROM_START( karatedo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "be14", 0x0000, 0x2000, CRC(44e60aa0) SHA1(6d007d7082c15182832f947444b00b7feb0e7738) )
	ROM_LOAD( "be15", 0x2000, 0x2000, CRC(a65e3793) SHA1(bf1e8fbc6755e85414eb7629e6fab3bf154f6546) )
	ROM_LOAD( "be16", 0x4000, 0x2000, CRC(151d8872) SHA1(1bb27142fdb33e3aeaf95c7a0ad7e8c258bbcb66) )
	ROM_LOAD( "be17", 0x6000, 0x2000, CRC(8f393b6a) SHA1(f246a6e069a2f562c5b7de05a2b8a6a09c1f4d1b) )
	ROM_LOAD( "be18", 0x8000, 0x2000, CRC(a09046ad) SHA1(665973bffc38e36b8b0f6bc79e10db280be0613e) )
	ROM_LOAD( "be19", 0xa000, 0x2000, CRC(0cdc4da9) SHA1(405454deda311abb8badd58a47529e42ddce5f6a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "be26", 0x0000, 0x2000, CRC(999ab0a3) SHA1(6aa545cee7a261f3dc5774dea2066f1412f63f49) )
	ROM_LOAD( "be25", 0x2000, 0x2000, CRC(253bf0da) SHA1(33bae6401003dc57deaa14bf7f6a7ebad5b7efe3) ) // ADPCM
	ROM_LOAD( "be24", 0x4000, 0x2000, CRC(e2c188af) SHA1(b7a0801a4c634694f1556873fd21f7e13441be17) ) // ADPCM
	ROM_LOAD( "be23", 0x6000, 0x2000, CRC(25262de1) SHA1(6264cd82756be9e1cdcd9ad3c3dfc6fef78dab8f) )
	ROM_LOAD( "be22", 0x8000, 0x2000, CRC(38055c48) SHA1(8406a52aaa7e56093a8d8552e928988b6fdd6c95) )
	ROM_LOAD( "be21", 0xa000, 0x2000, CRC(5f0efbe7) SHA1(f831efd02c917adac827fe6db8449ca8707b3d44) )
	ROM_LOAD( "be20", 0xc000, 0x2000, CRC(cbe8a533) SHA1(04cb41c487c2f951417628ed2888e04d59a39d29) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "be00", 0x0000, 0x2000, CRC(cec020f2) SHA1(07c501cc24797000f369fd98a26efe13875107bb) )  // plane0, tiles
	ROM_LOAD( "be01", 0x4000, 0x2000, CRC(cd96271c) SHA1(bcc71010e5489b19ad1553141c7b2e366bbbc68f) )  // plane1, tiles

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "be13",     0x00000, 0x2000, CRC(fb358707) SHA1(37124f1f545787723fecf466d8dcd31b88cdd75d) )  // Top, plane0, sprites
	ROM_LOAD( "be04",     0x02000, 0x2000, CRC(48372bf8) SHA1(28231b3bdb1d7226d7856554ba667b6d61f4fe22) )  // Bot, plane0, sprites
	ROM_LOAD( "b012.bin", 0x04000, 0x2000, CRC(b4842ea9) SHA1(471475f65edbd292b9162ad50e5cb0c7144845b0) )  // Top, plane0, sprites
	ROM_LOAD( "b003.bin", 0x06000, 0x2000, CRC(8cd166a5) SHA1(4b623c4c0025d75b3ed9746f8b6730bf3e65d85a) )  // Bot, plane0, sprites
	ROM_LOAD( "b011.bin", 0x08000, 0x2000, CRC(4cbd3aa3) SHA1(a9a683dcc4f52b18450659a20434a4d2a7b411d9) )  // Top, plane0, sprites
	ROM_LOAD( "b002.bin", 0x0a000, 0x2000, CRC(6be342a6) SHA1(0b8ac7ef7c6a6464fbc027a9fd17fa7ce1ffd962) )  // Bot, plane0, sprites
	ROM_LOAD( "be07",     0x0c000, 0x2000, CRC(40f2b6fb) SHA1(8d9ee04d917a8e143bd00fa7582990213bfa42d3) )  // Top, plane1, sprites
	ROM_LOAD( "be10",     0x0e000, 0x2000, CRC(325c0a97) SHA1(0159536ff0ebac8ccf65aac1a524a30b3fca3418) )  // Bot, plane1, sprites
	ROM_LOAD( "b006.bin", 0x10000, 0x2000, CRC(7346db8a) SHA1(d2b2c1700ae0ff9c614a9981a3da3d69879e9f25) )  // Top, plane1, sprites
	ROM_LOAD( "b009.bin", 0x12000, 0x2000, CRC(b78714fc) SHA1(4df7f15c37d56a9d66d0049aad65b32063e5c29a) )  // Bot, plane1, sprites
	ROM_LOAD( "b005.bin", 0x14000, 0x2000, CRC(b2557102) SHA1(ec4285029fc3ee1ad0adb05f363b234c67f8903d) )  // Top, plane1, sprites
	ROM_LOAD( "b008.bin", 0x16000, 0x2000, CRC(c85aba0e) SHA1(4be21b38623c2a8ae7f1e7397fb002e4cb9e4614) )  // Bot, plane1, sprites

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "br27", 0x0000, 0x0100, CRC(f683c54a) SHA1(92893990456b92f04a2be98b8e9626e97b7a2562) ) // Red
	ROM_LOAD( "br26", 0x0100, 0x0100, CRC(3ddbb6c4) SHA1(0eca5594d6812bc79f8b78f83fe003877d20c973) ) // Green
	ROM_LOAD( "br25", 0x0200, 0x0100, CRC(ba4a5651) SHA1(77e81bd64ab59a7466d20eabdff4be241e963c52) ) // Blue
ROM_END

// Bootleg from Tecfri
ROM_START( kchamptec )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "13.bin", 0x0000, 0x2000, CRC(0000d1a0) SHA1(0c584096825e1d3cc718e0bda1abb897a7f4d2df) )
	ROM_LOAD( "12.bin", 0x2000, 0x2000, CRC(03fae67e) SHA1(3b6a30f39f5ad512415e3b8ba6e07f3118e28d9e) )
	ROM_LOAD( "11.bin", 0x4000, 0x2000, CRC(3b6e1d08) SHA1(ecf7d2b0f31f04c0be7d5a1f450b9c95d9f54d80) )
	ROM_LOAD( "10.bin", 0x6000, 0x2000, CRC(53036ef7) SHA1(aeb56a7ab5f76907c83d250d8693d8532d515fb6) )
	ROM_LOAD( "9.bin",  0x8000, 0x2000, CRC(b824abc7) SHA1(4a30fec025150e889600a78497700155e743c99e) )
	ROM_LOAD( "8.bin",  0xa000, 0x2000, CRC(18cd5a0c) SHA1(0b4c3d2f3487db0a487b9d882d8abaeeb2464539) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "1.bin", 0x0000, 0x2000, CRC(999ed2c7) SHA1(f01e4ee81f8f7b0d1cf001d3ec01a9210f8109b4) )
	ROM_LOAD( "2.bin", 0x2000, 0x2000, CRC(33171e07) SHA1(55ee74c9f1d86ec13d92ea0d1b700bbe24b79def) ) // ADPCM
	ROM_LOAD( "3.bin", 0x4000, 0x2000, CRC(d5501588) SHA1(3177a6bdf9408be25c5763cb0a7d8ea0acc80f89) ) // ADPCM
	ROM_LOAD( "4.bin", 0x6000, 0x2000, CRC(47f66aac) SHA1(484802cfbff7c5f51dd97cb3b2321e137b03481c) )
	ROM_LOAD( "5.bin", 0x8000, 0x2000, BAD_DUMP CRC(5928e749) SHA1(a4dbd6f2a6a7aa9597875dfd781e55b0fb14d49b) ) // Bad EPROM
	ROM_LOAD( "6.bin", 0xa000, 0x2000, CRC(ca17e3ba) SHA1(91a3ccd6045dcef5f3293d669fe5a4df59cd954b) )
	ROM_LOAD( "7.bin", 0xc000, 0x2000, CRC(ada4f2cd) SHA1(15a4ed7497cb6c06f523ebe38bc4eb6dbcd09549) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "27.bin", 0x00000, 0x2000, CRC(a4fa98a1) SHA1(33b9a1a56d72ffa5f4e16b69e6e19af5a2882b2c) )  // plane0, tiles
	ROM_LOAD( "26.bin", 0x04000, 0x2000, CRC(fea09f7c) SHA1(174fc8022c455438538e6a3b67c7effc857ae634) )  // plane1, tiles

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "14.bin", 0x00000, 0x2000, CRC(eaad4168) SHA1(f31b05ffb86677157f3a44cdcf0f9a729e0ab259) )  // Top, plane0, sprites
	ROM_LOAD( "23.bin", 0x02000, 0x2000, CRC(10a47e2d) SHA1(97fe2de3ce2b8dc017dceffce494be18695708d2) )  // Bot, plane0, sprites
	ROM_LOAD( "15.bin", 0x04000, 0x2000, CRC(b4842ea9) SHA1(471475f65edbd292b9162ad50e5cb0c7144845b0) )  // Top, plane0, sprites
	ROM_LOAD( "24.bin", 0x06000, 0x2000, CRC(8cd166a5) SHA1(4b623c4c0025d75b3ed9746f8b6730bf3e65d85a) )  // Bot, plane0, sprites
	ROM_LOAD( "16.bin", 0x08000, 0x2000, CRC(4cbd3aa3) SHA1(a9a683dcc4f52b18450659a20434a4d2a7b411d9) )  // Top, plane0, sprites
	ROM_LOAD( "25.bin", 0x0a000, 0x2000, CRC(6be342a6) SHA1(0b8ac7ef7c6a6464fbc027a9fd17fa7ce1ffd962) )  // Bot, plane0, sprites
	ROM_LOAD( "20.bin", 0x0c000, 0x2000, CRC(cb91d16b) SHA1(bf32a03e7882b74280b29fa004429b77ad52e5ee) )  // Top, plane1, sprites
	ROM_LOAD( "17.bin", 0x0e000, 0x2000, CRC(489c9c04) SHA1(d920ef4f03e1b2e871df0cb2d672689c89febe96) )  // Bot, plane1, sprites
	ROM_LOAD( "21.bin", 0x10000, 0x2000, CRC(7346db8a) SHA1(d2b2c1700ae0ff9c614a9981a3da3d69879e9f25) )  // Top, plane1, sprites
	ROM_LOAD( "18.bin", 0x12000, 0x2000, CRC(b78714fc) SHA1(4df7f15c37d56a9d66d0049aad65b32063e5c29a) )  // Bot, plane1, sprites
	ROM_LOAD( "22.bin", 0x14000, 0x2000, CRC(b2557102) SHA1(ec4285029fc3ee1ad0adb05f363b234c67f8903d) )  // Top, plane1, sprites
	ROM_LOAD( "19.bin", 0x16000, 0x2000, CRC(c85aba0e) SHA1(4be21b38623c2a8ae7f1e7397fb002e4cb9e4614) )  // Bot, plane1, sprites

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "br27.1", 0x0000,  0x0100, CRC(f683c54a) SHA1(92893990456b92f04a2be98b8e9626e97b7a2562) ) // Red
	ROM_LOAD( "br26.2", 0x0100,  0x0100, CRC(3ddbb6c4) SHA1(0eca5594d6812bc79f8b78f83fe003877d20c973) ) // Green
	ROM_LOAD( "br25.3", 0x0200,  0x0100, CRC(ba4a5651) SHA1(77e81bd64ab59a7466d20eabdff4be241e963c52) ) // Blue
ROM_END

/* Bootleg from the Spanish company "Automaticos Arfyc".
   Color PROMs like 'kchamp2p' and a few unique bytes on the K26/BE26 sound ROM.
   Frequencies measured on the PCB:
    Program Z80: 2.99684 MHz (12.000 MHz xtal / 4)
    Sound Z80:   4.43141 MHz (8.867238 MHz xtal / 2)
    SN74LS04N:   8.86277 MHz (8.867238 MHz xtal)
    2xAY-3-8910: 1.10785 MHz (8.867238 MHz xtal / 8)
*/
ROM_START( karateda )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "k-14_2764.d1",  0x0000, 0x2000, CRC(44e60aa0) SHA1(6d007d7082c15182832f947444b00b7feb0e7738) )
	ROM_LOAD( "k-15_2764.d2",  0x2000, 0x2000, CRC(a65e3793) SHA1(bf1e8fbc6755e85414eb7629e6fab3bf154f6546) )
	ROM_LOAD( "k-16_2764.d4",  0x4000, 0x2000, CRC(151d8872) SHA1(1bb27142fdb33e3aeaf95c7a0ad7e8c258bbcb66) )
	ROM_LOAD( "k-17_2764.d5",  0x6000, 0x2000, CRC(8f393b6a) SHA1(f246a6e069a2f562c5b7de05a2b8a6a09c1f4d1b) )
	ROM_LOAD( "k-18_2764.d6",  0x8000, 0x2000, CRC(a09046ad) SHA1(665973bffc38e36b8b0f6bc79e10db280be0613e) )
	ROM_LOAD( "k-19_2764.d7",  0xa000, 0x2000, CRC(0cdc4da9) SHA1(405454deda311abb8badd58a47529e42ddce5f6a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "k-26_2764.e10", 0x0000, 0x2000, CRC(645232ba) SHA1(97b78a9bb069d3fdbd348a4e83d5abdf8418e595) )
	ROM_LOAD( "k-25_2764.e9",  0x2000, 0x2000, CRC(253bf0da) SHA1(33bae6401003dc57deaa14bf7f6a7ebad5b7efe3) ) // ADPCM
	ROM_LOAD( "k-24_2764.e8",  0x4000, 0x2000, CRC(e2c188af) SHA1(b7a0801a4c634694f1556873fd21f7e13441be17) ) // ADPCM
	ROM_LOAD( "k-23_2764.e6",  0x6000, 0x2000, CRC(25262de1) SHA1(6264cd82756be9e1cdcd9ad3c3dfc6fef78dab8f) )
	ROM_LOAD( "k-22_2764.e5",  0x8000, 0x2000, CRC(38055c48) SHA1(8406a52aaa7e56093a8d8552e928988b6fdd6c95) )
	ROM_LOAD( "k-21_2764.e4",  0xa000, 0x2000, CRC(5f0efbe7) SHA1(f831efd02c917adac827fe6db8449ca8707b3d44) )
	ROM_LOAD( "k-20_2764.e2",  0xc000, 0x2000, CRC(cbe8a533) SHA1(04cb41c487c2f951417628ed2888e04d59a39d29) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "k-0_2764.h3",   0x00000, 0x2000, CRC(cec020f2) SHA1(07c501cc24797000f369fd98a26efe13875107bb) ) // Plane0, tiles
	ROM_LOAD( "k-1_2764.h6",   0x04000, 0x2000, CRC(cd96271c) SHA1(bcc71010e5489b19ad1553141c7b2e366bbbc68f) ) // Plane1, tiles

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "k-13_2764.k15", 0x00000, 0x2000, CRC(fb358707) SHA1(37124f1f545787723fecf466d8dcd31b88cdd75d) ) // Top, plane0, sprites
	ROM_LOAD( "k-4_2764.j15",  0x02000, 0x2000, CRC(48372bf8) SHA1(28231b3bdb1d7226d7856554ba667b6d61f4fe22) ) // Bot, plane0, sprites
	ROM_LOAD( "k-12_2764.k13", 0x04000, 0x2000, CRC(b4842ea9) SHA1(471475f65edbd292b9162ad50e5cb0c7144845b0) ) // Top, plane0, sprites
	ROM_LOAD( "k-3_2764.j13",  0x06000, 0x2000, CRC(8cd166a5) SHA1(4b623c4c0025d75b3ed9746f8b6730bf3e65d85a) ) // Bot, plane0, sprites
	ROM_LOAD( "k-11_2764.k12", 0x08000, 0x2000, CRC(4cbd3aa3) SHA1(a9a683dcc4f52b18450659a20434a4d2a7b411d9) ) // Top, plane0, sprites
	ROM_LOAD( "k-2_2764.j12",  0x0a000, 0x2000, CRC(6be342a6) SHA1(0b8ac7ef7c6a6464fbc027a9fd17fa7ce1ffd962) ) // Bot, plane0, sprites
	ROM_LOAD( "k-7_2764.k4",   0x0c000, 0x2000, CRC(40f2b6fb) SHA1(8d9ee04d917a8e143bd00fa7582990213bfa42d3) ) // Top, plane1, sprites
	ROM_LOAD( "k-10_2764.k9",  0x0e000, 0x2000, CRC(325c0a97) SHA1(0159536ff0ebac8ccf65aac1a524a30b3fca3418) ) // Bot, plane1, sprites
	ROM_LOAD( "k-6_2764.k2",   0x10000, 0x2000, CRC(7346db8a) SHA1(d2b2c1700ae0ff9c614a9981a3da3d69879e9f25) ) // Top, plane1, sprites
	ROM_LOAD( "k-9_2764.k8",   0x12000, 0x2000, CRC(b78714fc) SHA1(4df7f15c37d56a9d66d0049aad65b32063e5c29a) ) // Bot, plane1, sprites
	ROM_LOAD( "k-5_2764.k1",   0x14000, 0x2000, CRC(b2557102) SHA1(ec4285029fc3ee1ad0adb05f363b234c67f8903d) ) // Top, plane1, sprites
	ROM_LOAD( "k-8_2764.k7",   0x16000, 0x2000, CRC(c85aba0e) SHA1(4be21b38623c2a8ae7f1e7397fb002e4cb9e4614) ) // Bot, plane1, sprites

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "1_dm74s287_82s129.f11", 0x0000, 0x0100, CRC(f05bda76) SHA1(0842092e28a15de3b4c198e48f650dbbe3cc95ce) ) // Red
	ROM_LOAD( "2_dm74s287_82s129.f12", 0x0100, 0x0100, CRC(d26d6fa9) SHA1(72a30bdc3410f2704539a32aeef69ee803e39f82) ) // Green
	ROM_LOAD( "3_dm74s287_82s129.f13", 0x0200, 0x0100, CRC(111ccb15) SHA1(4928a7ccc8649520cad3b119ff0b0ca1276c95ae) ) // Blue
ROM_END

ROM_START( kchampvs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bs24.d13", 0x00000, 0x02000, CRC(829da69b) SHA1(3266e7686e537f34ee5ce4cccc349eb12fc65038) )
	ROM_LOAD( "bs23.d11", 0x02000, 0x02000, CRC(091f810e) SHA1(283edb08ce106835185a1c2d6b88f7544d75f3b4) )
	ROM_LOAD( "bs22.d10", 0x04000, 0x02000, CRC(d4df2a52) SHA1(60d6cb1cb51c6f80a0f88913d4152ab8bda752d6) )
	ROM_LOAD( "bs21.d8",  0x06000, 0x02000, CRC(3d4ef0da) SHA1(228c8e47bb7123b69746506402edb875a43d7af5) )
	ROM_LOAD( "bs20.d7",  0x08000, 0x02000, CRC(623a467b) SHA1(5f150c67632f8e32769b75aa0615d0eb018afdc4) )
	ROM_LOAD( "bs19.d6",  0x0a000, 0x02000, CRC(43e196c4) SHA1(8029798ea0a560603c3dcde56db5a1ccde58c514) )
	ROM_CONTINUE(         0x0e000, 0x02000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "bs18.d4",  0x00000, 0x02000, CRC(eaa646eb) SHA1(cbd48f4d5d225b71c2dd0b14f420838561e3f83e) )
	ROM_LOAD( "bs17.d2",  0x02000, 0x02000, CRC(d71031ad) SHA1(b168f4ef4feb4195305404df699acecb731eab02) )
	ROM_LOAD( "bs16.d1",  0x04000, 0x02000, CRC(6f811c43) SHA1(1d33ac8129562ab709bd7396b4c2457b6db99277) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "bs12.k1",  0x00000, 0x02000, CRC(4c574ecd) SHA1(86914eef33da73463ba6261eecae75209d24fac1) )
	ROM_LOAD( "bs13.k3",  0x02000, 0x02000, CRC(750b66af) SHA1(c7824994b977d4e846f3ecadfcfc51331f52b6f4) )
	ROM_LOAD( "bs14.k5",  0x04000, 0x02000, CRC(9ad6227c) SHA1(708af5e70927040cf7f2ae6f792344c19099530c) )
	ROM_LOAD( "bs15.k6",  0x06000, 0x02000, CRC(3b6d5de5) SHA1(288fffcbc9369db5c75e7e0d6181612de6f12da3) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "bs00.a1",  0x00000, 0x02000, CRC(51eda56c) SHA1(31438e115e95c2a684ec65ed2bdb9125e3675226) )
	ROM_LOAD( "bs06.c1",  0x02000, 0x02000, CRC(593264cf) SHA1(866469f37b6c90afc65e53e6589b67ac4b25997e) )
	ROM_LOAD( "bs01.a3",  0x04000, 0x02000, CRC(b4842ea9) SHA1(471475f65edbd292b9162ad50e5cb0c7144845b0) )
	ROM_LOAD( "bs07.c3",  0x06000, 0x02000, CRC(8cd166a5) SHA1(4b623c4c0025d75b3ed9746f8b6730bf3e65d85a) )
	ROM_LOAD( "bs02.a5",  0x08000, 0x02000, CRC(4cbd3aa3) SHA1(a9a683dcc4f52b18450659a20434a4d2a7b411d9) )
	ROM_LOAD( "bs08.c5",  0x0a000, 0x02000, CRC(6be342a6) SHA1(0b8ac7ef7c6a6464fbc027a9fd17fa7ce1ffd962) )
	ROM_LOAD( "bs03.a6",  0x0c000, 0x02000, CRC(8dcd271a) SHA1(0abeaa46433a59c110815ecf188c7afd6fa387a4) )
	ROM_LOAD( "bs09.c6",  0x0e000, 0x02000, CRC(4ee1dba7) SHA1(717ce9a4e20f6e02adf678b1400af4aaecdbfb40) )
	ROM_LOAD( "bs04.a8",  0x10000, 0x02000, CRC(7346db8a) SHA1(d2b2c1700ae0ff9c614a9981a3da3d69879e9f25) )
	ROM_LOAD( "bs10.c8",  0x12000, 0x02000, CRC(b78714fc) SHA1(4df7f15c37d56a9d66d0049aad65b32063e5c29a) )
	ROM_LOAD( "bs05.a10", 0x14000, 0x02000, CRC(b2557102) SHA1(ec4285029fc3ee1ad0adb05f363b234c67f8903d) )
	ROM_LOAD( "bs11.c10", 0x16000, 0x02000, CRC(c85aba0e) SHA1(4be21b38623c2a8ae7f1e7397fb002e4cb9e4614) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "br27.k10", 0x0000, 0x0100, CRC(f683c54a) SHA1(92893990456b92f04a2be98b8e9626e97b7a2562) ) // Red
	ROM_LOAD( "br26.k9",  0x0100, 0x0100, CRC(3ddbb6c4) SHA1(0eca5594d6812bc79f8b78f83fe003877d20c973) ) // Green
	ROM_LOAD( "br25.k8",  0x0200, 0x0100, CRC(ba4a5651) SHA1(77e81bd64ab59a7466d20eabdff4be241e963c52) ) // Blue
ROM_END

ROM_START( kchampvs2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lt.d13",   0x00000, 0x02000, CRC(eef41aa8) SHA1(6d4e8159e9c3cd629337863c0397ff90b4c8d3fa) )
	ROM_LOAD( "lt.d11",   0x02000, 0x02000, CRC(091f810e) SHA1(283edb08ce106835185a1c2d6b88f7544d75f3b4) )
	ROM_LOAD( "lt.d10",   0x04000, 0x02000, CRC(d4df2a52) SHA1(60d6cb1cb51c6f80a0f88913d4152ab8bda752d6) )
	ROM_LOAD( "lt.d8",    0x06000, 0x02000, CRC(3d4ef0da) SHA1(228c8e47bb7123b69746506402edb875a43d7af5) )
	ROM_LOAD( "lt.d7",    0x08000, 0x02000, CRC(623a467b) SHA1(5f150c67632f8e32769b75aa0615d0eb018afdc4) )
	ROM_LOAD( "lt.d6",    0x0a000, 0x02000, CRC(c3bc6e46) SHA1(a7b9420592905b0df5ff00c392d887f40395179f) )
	ROM_CONTINUE(         0x0e000, 0x02000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "lt.d4",    0x00000, 0x02000, CRC(eaa646eb) SHA1(cbd48f4d5d225b71c2dd0b14f420838561e3f83e) )
	ROM_LOAD( "lt.d2",    0x02000, 0x02000, CRC(d71031ad) SHA1(b168f4ef4feb4195305404df699acecb731eab02) )
	ROM_LOAD( "lt.d1",    0x04000, 0x02000, CRC(6f811c43) SHA1(1d33ac8129562ab709bd7396b4c2457b6db99277) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "lt.k1",    0x00000, 0x02000, CRC(4c574ecd) SHA1(86914eef33da73463ba6261eecae75209d24fac1) )
	ROM_LOAD( "lt.k3",    0x02000, 0x02000, CRC(750b66af) SHA1(c7824994b977d4e846f3ecadfcfc51331f52b6f4) )
	ROM_LOAD( "lt.k5",    0x04000, 0x02000, CRC(9ad6227c) SHA1(708af5e70927040cf7f2ae6f792344c19099530c) )
	ROM_LOAD( "lt.k6",    0x06000, 0x02000, CRC(3b6d5de5) SHA1(288fffcbc9369db5c75e7e0d6181612de6f12da3) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "lt.a1",    0x00000, 0x02000, CRC(51eda56c) SHA1(31438e115e95c2a684ec65ed2bdb9125e3675226) )
	ROM_LOAD( "lt.c1",    0x02000, 0x02000, CRC(593264cf) SHA1(866469f37b6c90afc65e53e6589b67ac4b25997e) )
	ROM_LOAD( "lt.a3",    0x04000, 0x02000, CRC(b4842ea9) SHA1(471475f65edbd292b9162ad50e5cb0c7144845b0) )
	ROM_LOAD( "lt.c3",    0x06000, 0x02000, CRC(8cd166a5) SHA1(4b623c4c0025d75b3ed9746f8b6730bf3e65d85a) )
	ROM_LOAD( "lt.a5",    0x08000, 0x02000, CRC(4cbd3aa3) SHA1(a9a683dcc4f52b18450659a20434a4d2a7b411d9) )
	ROM_LOAD( "lt.c5",    0x0a000, 0x02000, CRC(6be342a6) SHA1(0b8ac7ef7c6a6464fbc027a9fd17fa7ce1ffd962) )
	ROM_LOAD( "lt.a6",    0x0c000, 0x02000, CRC(8dcd271a) SHA1(0abeaa46433a59c110815ecf188c7afd6fa387a4) )
	ROM_LOAD( "lt.c6",    0x0e000, 0x02000, CRC(4ee1dba7) SHA1(717ce9a4e20f6e02adf678b1400af4aaecdbfb40) )
	ROM_LOAD( "lt.a8",    0x10000, 0x02000, CRC(7346db8a) SHA1(d2b2c1700ae0ff9c614a9981a3da3d69879e9f25) )
	ROM_LOAD( "lt.c8",    0x12000, 0x02000, CRC(b78714fc) SHA1(4df7f15c37d56a9d66d0049aad65b32063e5c29a) )
	ROM_LOAD( "lt.a10",   0x14000, 0x02000, CRC(b2557102) SHA1(ec4285029fc3ee1ad0adb05f363b234c67f8903d) )
	ROM_LOAD( "lt.c10",   0x16000, 0x02000, CRC(c85aba0e) SHA1(4be21b38623c2a8ae7f1e7397fb002e4cb9e4614) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "lt.k10",   0x0000, 0x0100, CRC(f683c54a) SHA1(92893990456b92f04a2be98b8e9626e97b7a2562) ) // Red
	ROM_LOAD( "lt.k9",    0x0100, 0x0100, CRC(3ddbb6c4) SHA1(0eca5594d6812bc79f8b78f83fe003877d20c973) ) // Green
	ROM_LOAD( "lt.k8",    0x0200, 0x0100, CRC(ba4a5651) SHA1(77e81bd64ab59a7466d20eabdff4be241e963c52) ) // Blue
ROM_END

// This version has a mix of the "Karate Champ (US VS version, set 1)" ROMs and a few ROMs unique to "Taisen Karate Dou (Japan VS version)" along with 1 completely unique program ROM.
// The only difference is instead of "My hero" as found in the later US sets it says "My hero deserves the fair".
ROM_START( kchampvs3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bs24.d13", 0x00000, 0x02000, CRC(829da69b) SHA1(3266e7686e537f34ee5ce4cccc349eb12fc65038) )
	ROM_LOAD( "bs23.d11", 0x02000, 0x02000, CRC(091f810e) SHA1(283edb08ce106835185a1c2d6b88f7544d75f3b4) )
	ROM_LOAD( "bs22.d10", 0x04000, 0x02000, CRC(d4df2a52) SHA1(60d6cb1cb51c6f80a0f88913d4152ab8bda752d6) )
	ROM_LOAD( "bs21.d8",  0x06000, 0x02000, CRC(80839912) SHA1(4a3c6ad8eba756ae8e89faf42b72b41f85731e67) ) // only unique ROM
	ROM_LOAD( "bs20.d7",  0x08000, 0x02000, CRC(623a467b) SHA1(5f150c67632f8e32769b75aa0615d0eb018afdc4) )
	ROM_LOAD( "bs19.d6",  0x0a000, 0x02000, CRC(43e196c4) SHA1(8029798ea0a560603c3dcde56db5a1ccde58c514) )
	ROM_CONTINUE(         0x0e000, 0x02000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "bs18.d4",  0x00000, 0x02000, CRC(eaa646eb) SHA1(cbd48f4d5d225b71c2dd0b14f420838561e3f83e) )
	ROM_LOAD( "bs17.d2",  0x02000, 0x02000, CRC(d71031ad) SHA1(b168f4ef4feb4195305404df699acecb731eab02) )
	ROM_LOAD( "bs16.d1",  0x04000, 0x02000, CRC(6f811c43) SHA1(1d33ac8129562ab709bd7396b4c2457b6db99277) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "bs12.k1",  0x00000, 0x02000, CRC(4c574ecd) SHA1(86914eef33da73463ba6261eecae75209d24fac1) )
	ROM_LOAD( "bs13.k3",  0x02000, 0x02000, CRC(750b66af) SHA1(c7824994b977d4e846f3ecadfcfc51331f52b6f4) )
	ROM_LOAD( "bs14.k5",  0x04000, 0x02000, CRC(9ad6227c) SHA1(708af5e70927040cf7f2ae6f792344c19099530c) )
	ROM_LOAD( "bs15.k6",  0x06000, 0x02000, CRC(3b6d5de5) SHA1(288fffcbc9369db5c75e7e0d6181612de6f12da3) )

	ROM_REGION( 0x18000, "gfx2", 0 ) // 00, 06, 03 and 09 match the karatevs set
	ROM_LOAD( "bs00.a1",  0x00000, 0x02000, CRC(c46a8b88) SHA1(a47e56a6dc7f36b896b8156e77a1da7e8be2332e) )
	ROM_LOAD( "bs06.c1",  0x02000, 0x02000, CRC(cf8982ff) SHA1(aafb249503ad51f64b1f31ea2d869dfc0e065d19) )
	ROM_LOAD( "bs01.a3",  0x04000, 0x02000, CRC(b4842ea9) SHA1(471475f65edbd292b9162ad50e5cb0c7144845b0) )
	ROM_LOAD( "bs07.c3",  0x06000, 0x02000, CRC(8cd166a5) SHA1(4b623c4c0025d75b3ed9746f8b6730bf3e65d85a) )
	ROM_LOAD( "bs02.a5",  0x08000, 0x02000, CRC(4cbd3aa3) SHA1(a9a683dcc4f52b18450659a20434a4d2a7b411d9) )
	ROM_LOAD( "bs08.c5",  0x0a000, 0x02000, CRC(6be342a6) SHA1(0b8ac7ef7c6a6464fbc027a9fd17fa7ce1ffd962) )
	ROM_LOAD( "bs03.a6",  0x0c000, 0x02000, CRC(bde8a52b) SHA1(1a0800472caf8c79a15cc977dad1a7bc97c74b2b) )
	ROM_LOAD( "bs09.c6",  0x0e000, 0x02000, CRC(e9a5f945) SHA1(e6b21912bee97de06819c8ac85a45bbc70030f88) )
	ROM_LOAD( "bs04.a8",  0x10000, 0x02000, CRC(7346db8a) SHA1(d2b2c1700ae0ff9c614a9981a3da3d69879e9f25) )
	ROM_LOAD( "bs10.c8",  0x12000, 0x02000, CRC(b78714fc) SHA1(4df7f15c37d56a9d66d0049aad65b32063e5c29a) )
	ROM_LOAD( "bs05.a10", 0x14000, 0x02000, CRC(b2557102) SHA1(ec4285029fc3ee1ad0adb05f363b234c67f8903d) )
	ROM_LOAD( "bs11.c10", 0x16000, 0x02000, CRC(c85aba0e) SHA1(4be21b38623c2a8ae7f1e7397fb002e4cb9e4614) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "br27.k10", 0x0000, 0x0100, CRC(f683c54a) SHA1(92893990456b92f04a2be98b8e9626e97b7a2562) ) // Red
	ROM_LOAD( "br26.k9",  0x0100, 0x0100, CRC(3ddbb6c4) SHA1(0eca5594d6812bc79f8b78f83fe003877d20c973) ) // Green
	ROM_LOAD( "br25.k8",  0x0200, 0x0100, CRC(ba4a5651) SHA1(77e81bd64ab59a7466d20eabdff4be241e963c52) ) // Blue
ROM_END

// This version has a mix of the "Karate Champ (US VS version, set 1)" ROMs and a few ROMs unique to "Taisen Karate Dou (Japan VS version)".
// It displays the dialog balloon in red with a boat below it.
ROM_START( kchampvs4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bs24.d13", 0x00000, 0x02000, CRC(829da69b) SHA1(3266e7686e537f34ee5ce4cccc349eb12fc65038) )
	ROM_LOAD( "bs23.d11", 0x02000, 0x02000, CRC(091f810e) SHA1(283edb08ce106835185a1c2d6b88f7544d75f3b4) )
	ROM_LOAD( "bs22.d10", 0x04000, 0x02000, CRC(d4df2a52) SHA1(60d6cb1cb51c6f80a0f88913d4152ab8bda752d6) )
	ROM_LOAD( "bs21.d8",  0x06000, 0x02000, CRC(3d4ef0da) SHA1(228c8e47bb7123b69746506402edb875a43d7af5) )
	ROM_LOAD( "bs20.d7",  0x08000, 0x02000, CRC(623a467b) SHA1(5f150c67632f8e32769b75aa0615d0eb018afdc4) )
	ROM_LOAD( "bs19.d6",  0x0a000, 0x02000, CRC(43e196c4) SHA1(8029798ea0a560603c3dcde56db5a1ccde58c514) )
	ROM_CONTINUE(         0x0e000, 0x02000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "bs18.d4",  0x00000, 0x02000, CRC(eaa646eb) SHA1(cbd48f4d5d225b71c2dd0b14f420838561e3f83e) )
	ROM_LOAD( "bs17.d2",  0x02000, 0x02000, CRC(d71031ad) SHA1(b168f4ef4feb4195305404df699acecb731eab02) )
	ROM_LOAD( "bs16.d1",  0x04000, 0x02000, CRC(6f811c43) SHA1(1d33ac8129562ab709bd7396b4c2457b6db99277) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "bs12.k1",  0x00000, 0x02000, CRC(4c574ecd) SHA1(86914eef33da73463ba6261eecae75209d24fac1) )
	ROM_LOAD( "bs13.k3",  0x02000, 0x02000, CRC(750b66af) SHA1(c7824994b977d4e846f3ecadfcfc51331f52b6f4) )
	ROM_LOAD( "bs14.k5",  0x04000, 0x02000, CRC(9ad6227c) SHA1(708af5e70927040cf7f2ae6f792344c19099530c) )
	ROM_LOAD( "bs15.k6",  0x06000, 0x02000, CRC(3b6d5de5) SHA1(288fffcbc9369db5c75e7e0d6181612de6f12da3) )

	ROM_REGION( 0x18000, "gfx2", 0 ) // 00, 06, 03 and 09 match the karatevs set
	ROM_LOAD( "bs00.a1",  0x00000, 0x02000, CRC(c46a8b88) SHA1(a47e56a6dc7f36b896b8156e77a1da7e8be2332e) )
	ROM_LOAD( "bs06.c1",  0x02000, 0x02000, CRC(cf8982ff) SHA1(aafb249503ad51f64b1f31ea2d869dfc0e065d19) )
	ROM_LOAD( "bs01.a3",  0x04000, 0x02000, CRC(b4842ea9) SHA1(471475f65edbd292b9162ad50e5cb0c7144845b0) )
	ROM_LOAD( "bs07.c3",  0x06000, 0x02000, CRC(8cd166a5) SHA1(4b623c4c0025d75b3ed9746f8b6730bf3e65d85a) )
	ROM_LOAD( "bs02.a5",  0x08000, 0x02000, CRC(4cbd3aa3) SHA1(a9a683dcc4f52b18450659a20434a4d2a7b411d9) )
	ROM_LOAD( "bs08.c5",  0x0a000, 0x02000, CRC(6be342a6) SHA1(0b8ac7ef7c6a6464fbc027a9fd17fa7ce1ffd962) )
	ROM_LOAD( "bs03.a6",  0x0c000, 0x02000, CRC(bde8a52b) SHA1(1a0800472caf8c79a15cc977dad1a7bc97c74b2b) )
	ROM_LOAD( "bs09.c6",  0x0e000, 0x02000, CRC(e9a5f945) SHA1(e6b21912bee97de06819c8ac85a45bbc70030f88) )
	ROM_LOAD( "bs04.a8",  0x10000, 0x02000, CRC(7346db8a) SHA1(d2b2c1700ae0ff9c614a9981a3da3d69879e9f25) )
	ROM_LOAD( "bs10.c8",  0x12000, 0x02000, CRC(b78714fc) SHA1(4df7f15c37d56a9d66d0049aad65b32063e5c29a) )
	ROM_LOAD( "bs05.a10", 0x14000, 0x02000, CRC(b2557102) SHA1(ec4285029fc3ee1ad0adb05f363b234c67f8903d) )
	ROM_LOAD( "bs11.c10", 0x16000, 0x02000, CRC(c85aba0e) SHA1(4be21b38623c2a8ae7f1e7397fb002e4cb9e4614) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "br27.k10", 0x0000, 0x0100, CRC(f683c54a) SHA1(92893990456b92f04a2be98b8e9626e97b7a2562) ) // Red
	ROM_LOAD( "br26.k9",  0x0100, 0x0100, CRC(3ddbb6c4) SHA1(0eca5594d6812bc79f8b78f83fe003877d20c973) ) // Green
	ROM_LOAD( "br25.k8",  0x0200, 0x0100, CRC(ba4a5651) SHA1(77e81bd64ab59a7466d20eabdff4be241e963c52) ) // Blue
ROM_END

ROM_START( karatevs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "br24.d13", 0x00000, 0x02000, CRC(ea9cda49) SHA1(7d753a8d391418d0fe5231eb88b3627f7d3fd99e) )
	ROM_LOAD( "br23.d11", 0x02000, 0x02000, CRC(46074489) SHA1(5593f819b6893820ef0c0fece13cf3ca83e1ab85) )
	ROM_LOAD( "br22.d10", 0x04000, 0x02000, CRC(294f67ba) SHA1(45f13a7deb75bb167176c5405128de3ca76e22f0) )
	ROM_LOAD( "br21.d8",  0x06000, 0x02000, CRC(934ea874) SHA1(dbc139715a1598033beedbf4f8fec73703b016d6) )
	ROM_LOAD( "br20.d7",  0x08000, 0x02000, CRC(97d7816a) SHA1(e02f9306fc3539f4feaedfcabea66d172d09a510) )
	ROM_LOAD( "br19.d6",  0x0a000, 0x02000, CRC(dd2239d2) SHA1(0533d5abf8e25a4aeec2f7832b657eab56fd11f0) )
	ROM_CONTINUE(         0x0e000, 0x02000 )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Sound CPU
	ROM_LOAD( "br18.d4",  0x00000, 0x02000, CRC(00ccb8ea) SHA1(83d69684dc3ad37aca03c901fd23c7652134766f) )
	ROM_LOAD( "bs17.d2",  0x02000, 0x02000, CRC(d71031ad) SHA1(b168f4ef4feb4195305404df699acecb731eab02) )
	ROM_LOAD( "br16.d1",  0x04000, 0x02000, CRC(2512d961) SHA1(f0cd1be112b915d700e0587759606d48d115a83f) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "br12.k1",  0x00000, 0x02000, CRC(9ed6f00d) SHA1(3def985deb29a7644309ede3bd82c225b4ae23f8) )
	ROM_LOAD( "bs13.k3",  0x02000, 0x02000, CRC(750b66af) SHA1(c7824994b977d4e846f3ecadfcfc51331f52b6f4) )
	ROM_LOAD( "br14.k5",  0x04000, 0x02000, CRC(fc399229) SHA1(e8d633151b0d7fa49c455920c4b0588575a7084e) )
	ROM_LOAD( "bs15.k6",  0x06000, 0x02000, CRC(3b6d5de5) SHA1(288fffcbc9369db5c75e7e0d6181612de6f12da3) )

	ROM_REGION( 0x18000, "gfx2", 0 )
	ROM_LOAD( "br00.a1",  0x00000, 0x02000, CRC(c46a8b88) SHA1(a47e56a6dc7f36b896b8156e77a1da7e8be2332e) )
	ROM_LOAD( "br06.c1",  0x02000, 0x02000, CRC(cf8982ff) SHA1(aafb249503ad51f64b1f31ea2d869dfc0e065d19) )
	ROM_LOAD( "bs01.a3",  0x04000, 0x02000, CRC(b4842ea9) SHA1(471475f65edbd292b9162ad50e5cb0c7144845b0) )
	ROM_LOAD( "bs07.c3",  0x06000, 0x02000, CRC(8cd166a5) SHA1(4b623c4c0025d75b3ed9746f8b6730bf3e65d85a) )
	ROM_LOAD( "bs02.a5",  0x08000, 0x02000, CRC(4cbd3aa3) SHA1(a9a683dcc4f52b18450659a20434a4d2a7b411d9) )
	ROM_LOAD( "bs08.c5",  0x0a000, 0x02000, CRC(6be342a6) SHA1(0b8ac7ef7c6a6464fbc027a9fd17fa7ce1ffd962) )
	ROM_LOAD( "br03.a6",  0x0c000, 0x02000, CRC(bde8a52b) SHA1(1a0800472caf8c79a15cc977dad1a7bc97c74b2b) )
	ROM_LOAD( "br09.c6",  0x0e000, 0x02000, CRC(e9a5f945) SHA1(e6b21912bee97de06819c8ac85a45bbc70030f88) )
	ROM_LOAD( "bs04.a8",  0x10000, 0x02000, CRC(7346db8a) SHA1(d2b2c1700ae0ff9c614a9981a3da3d69879e9f25) )
	ROM_LOAD( "bs10.c8",  0x12000, 0x02000, CRC(b78714fc) SHA1(4df7f15c37d56a9d66d0049aad65b32063e5c29a) )
	ROM_LOAD( "bs05.a10", 0x14000, 0x02000, CRC(b2557102) SHA1(ec4285029fc3ee1ad0adb05f363b234c67f8903d) )
	ROM_LOAD( "bs11.c10", 0x16000, 0x02000, CRC(c85aba0e) SHA1(4be21b38623c2a8ae7f1e7397fb002e4cb9e4614) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "br27.k10", 0x0000, 0x0100, CRC(f683c54a) SHA1(92893990456b92f04a2be98b8e9626e97b7a2562) ) // Red
	ROM_LOAD( "br26.k9",  0x0100, 0x0100, CRC(3ddbb6c4) SHA1(0eca5594d6812bc79f8b78f83fe003877d20c973) ) // Green
	ROM_LOAD( "br25.k8",  0x0200, 0x0100, CRC(ba4a5651) SHA1(77e81bd64ab59a7466d20eabdff4be241e963c52) ) // Blue
ROM_END


void kchamp_state::decrypt_code()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int A = 0; A < 0x10000; A++)
		m_decrypted_opcodes[A] = (rom[A] & 0x55) | ((rom[A] & 0x88) >> 2) | ((rom[A] & 0x22) << 2);
}


void kchamp_state::init_kchampvs()
{
	decrypt_code();

	uint8_t *rom = memregion("maincpu")->base();

	/*
	    Note that the first 4 opcodes that the program
	    executes aren't encrypted for some obscure reason.
	    The address for the 2nd opcode (a jump) is encrypted too.
	    It's not clear what the 3rd and 4th opcode are supposed to do,
	    they just write to a RAM location. This write might be what
	    turns the encryption on, but this doesn't explain the
	    encrypted address for the jump.
	 */
	m_decrypted_opcodes[0] = rom[0];  // this is a jump
	int A = rom[1] + 256 * rom[2];
	m_decrypted_opcodes[A] = rom[A];  // fix opcode on first jump address (again, a jump)
	rom[A+1] ^= 0xee;                 // fix address of the second jump
	A = rom[A+1] + 256 * rom[A+2];
	m_decrypted_opcodes[A] = rom[A];  // fix third opcode (ld a,$xx)
	A += 2;
	m_decrypted_opcodes[A] = rom[A];  // fix fourth opcode (ld ($xxxx),a)
	// and from here on, opcodes are encrypted

	m_msm_play_lo_nibble = true;
}


void kchamp_state::init_kchampvs2()
{
	decrypt_code();
	m_msm_play_lo_nibble = true;
}

//    YEAR  NAME       PARENT  MACHINE       INPUT     CLASS          INIT            ROT    COMPANY                  FULLNAME                                FLAGS
GAME( 1984, kchamp,    0,      kchamp,       kchamp,   kchamp_state,  empty_init,     ROT90, "Data East USA",         "Karate Champ (US)",                    MACHINE_SUPPORTS_SAVE )
GAME( 1984, kchamp2p,  kchamp, kchamp,       kchampvs, kchamp_state,  empty_init,     ROT90, "Data East USA",         "Karate Champ (US, 2 players)",         MACHINE_SUPPORTS_SAVE )
GAME( 1984, karatedo,  kchamp, kchamp,       kchamp,   kchamp_state,  empty_init,     ROT90, "Data East Corporation", "Karate Dou (Japan)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1984, kchamptec, kchamp, kchamp,       kchamp,   kchamp_state,  empty_init,     ROT90, "bootleg (Tecfri)",      "Karate Champ (Tecfri bootleg)",        MACHINE_SUPPORTS_SAVE )
GAME( 1984, karateda,  kchamp, kchamp_arfyc, kchamp,   kchamp_state,  empty_init,     ROT90, "bootleg (Arfyc)",       "Karate Dou (Arfyc bootleg)",           MACHINE_SUPPORTS_SAVE )
GAME( 1984, kchampvs,  kchamp, kchampvs,     kchampvs, kchamp_state,  init_kchampvs,  ROT90, "Data East USA",         "Karate Champ (US VS version, set 1)",  MACHINE_SUPPORTS_SAVE )
GAME( 1984, kchampvs2, kchamp, kchampvs,     kchampvs, kchamp_state,  init_kchampvs2, ROT90, "Data East USA",         "Karate Champ (US VS version, set 2)",  MACHINE_SUPPORTS_SAVE )
GAME( 1984, kchampvs3, kchamp, kchampvs,     kchampvs, kchamp_state,  init_kchampvs,  ROT90, "Data East USA",         "Karate Champ (US VS version, set 3)",  MACHINE_SUPPORTS_SAVE )
GAME( 1984, kchampvs4, kchamp, kchampvs,     kchampvs, kchamp_state,  init_kchampvs,  ROT90, "Data East USA",         "Karate Champ (US VS version, set 4)",  MACHINE_SUPPORTS_SAVE )
GAME( 1984, karatevs,  kchamp, kchampvs,     kchampvs, kchamp_state,  init_kchampvs,  ROT90, "Data East Corporation", "Taisen Karate Dou (Japan VS version)", MACHINE_SUPPORTS_SAVE )
