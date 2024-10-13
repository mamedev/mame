// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Exidy 6502 hardware

    Games supported:
        * Side Trak
        * Targ
        * Spectar
        * Mouse Trap
        * Venture
        * Teeter Torture (prototype)
        * Pepper 2
        * Hard Hat
        * Fax and Fax 2

    TODO:
        * check whether games besides Venture have coin counter outputs

    Known bugs:
        * none at this time


'Universal' Game Board V2 (xxL logic, xxA audio)

Name                 Year  CPU    board/rom numbers

Side Trak            1979  6502   STL, STA
Targ                 1980  6502   HRL, HRA
Spectar              1980  6502   SPL, SPA
Mouse Trap           1981  6502   MTL, MTA
Venture              1981  6502   VEL, VEA
Teeter Torture       1982  6502   Prototype
Pepper II            1982  6502   77-0008,77-0007 PTL, PTA
Hard Hat             1982  6502   HHL, HHA
Fax                  1982  6502   FXL, FLA


****************************************************************************

    Exidy memory map

    0000-00FF R/W Zero Page RAM
    0100-01FF R/W Stack RAM
    0200-03FF R/W Scratchpad RAM
    0800-3FFF  R  Program ROM              (Targ, Spectar only)
    1A00       R  PX3 (Player 2 inputs)    (Fax only)
                  bit 4  D
                  bit 5  C
                  bit 6  B
                  bit 7  A
    1C00       R  PX2 (Player 1 inputs)    (Fax only)
                  bit 0  2 player start
                  bit 1  1 player start
                  bit 4  D
                  bit 5  C
                  bit 6  B
                  bit 7  A
    2000-3FFF  R  Banked question ROM      (Fax only)
    4000-43FF R/W Screen RAM
    4800-4FFF R/W Character Generator RAM (except Pepper II and Fax)
    5000       W  Motion Object 1 Horizontal Position Latch (sprite 1 X)
    5040       W  Motion Object 1 Vertical Position Latch   (sprite 1 Y)
    5080       W  Motion Object 2 Horizontal Position Latch (sprite 2 X)
    50C0       W  Motion Object 2 Vertical Position Latch   (sprite 2 Y)
    5100       R  Option Dipswitch Port
                  bit 0  coin 2 (NOT inverted) (must activate together with $5103 bit 5)
                  bit 1-2  bonus
                  bit 3-4  coins per play
                  bit 5-6  lives
                  bit 7  US/UK coins
    5100       W  Motion Objects Image Latch
                  Sprite number  bits 0-3 Sprite #1  4-7 Sprite #2
    5101       R  Control Inputs Port
                  bit 0  start 1
                  bit 1  start 2
                  bit 2  right
                  bit 3  left
                  bit 5  up
                  bit 6  down
                  bit 7  coin 1 (must activate together with $5103 bit 6)
    5101       W  Output Control Latch (not used in PEPPER II upright)
                  bit 7  Enable sprite #1
                  bit 6  Enable sprite #2
    5103       R  Interrupt Condition Latch
                  bit 0  LNG0 - supposedly a language DIP switch
                  bit 1  LNG1 - supposedly a language DIP switch
                  bit 2  different for each game, but generally a collision bit
                  bit 3  TABLE - supposedly a cocktail table DIP switch
                  bit 4  different for each game, but generally a collision bit
                  bit 5  coin 2 (must activate together with $5100 bit 0)
                  bit 6  coin 1 (must activate together with $5101 bit 7)
                  bit 7  L256 - VBlank?
    5213       R  IN2 (Mouse Trap)
                  bit 3  blue button
                  bit 2  free play
                  bit 1  red button
                  bit 0  yellow button
    52XX      R/W Audio/Color Board Communications
    6000-6FFF R/W Character Generator RAM (Pepper II, Fax only)
    8000-FFF9  R  Program memory space
    FFFA-FFFF  R  Interrupt and Reset Vectors

    Exidy Sound Board:
    0000-07FF R/W RAM (mirrored every 0x7f)
    0800-0FFF R/W 6532 Timer
    1000-17FF R/W 6520 PIA
    1800-1FFF R/W 8253 Timer
    2000-27FF bit 0 Channel 1 Filter 1 enable
              bit 1 Channel 1 Filter 2 enable
              bit 2 Channel 2 Filter 1 enable
              bit 3 Channel 2 Filter 2 enable
              bit 4 Channel 3 Filter 1 enable
              bit 5 Channel 3 Filter 2 enable
    2800-2FFF 6840 Timer
    3000      Bit 0..1 Noise select
    3001      Bit 0..2 Channel 1 Amplitude
    3002      Bit 0..2 Channel 2 Amplitude
    3003      Bit 0..2 Channel 3 Amplitude
    5800-7FFF ROM

    Targ:
    5200    Sound board control
            bit 0 Music
            bit 1 Shoot
            bit 2 unused
            bit 3 Swarn
            bit 4 Sspec
            bit 5 crash
            bit 6 long
            bit 7 game

    5201    Sound board control
            bit 0 note
            bit 1 upper

    MouseTrap:
    5101    W  MouseTrap P1/P2 LED States
               bit 2  Player 1 LED state
               bit 4  Player 2 LED state

    MouseTrap Digital Sound:
    0000-3FFF ROM

    IO:
        A7 = 0: R Communication from sound processor
        A6 = 0: R CVSD Clock State
        A5 = 0: W Busy to sound processor
        A4 = 0: W Data to CVSD

***************************************************************************/

#include "emu.h"
#include "exidysound.h"

#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "sound/dac.h"
#include "sound/samples.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

#define EXIDY_MASTER_CLOCK              (XTAL(11'289'000))
#define EXIDY_CPU_CLOCK                 (EXIDY_MASTER_CLOCK / 16)
#define EXIDY_PIXEL_CLOCK               (EXIDY_MASTER_CLOCK / 2)
#define EXIDY_HTOTAL                    (0x150)
#define EXIDY_HBEND                     (0x000)
#define EXIDY_HBSTART                   (0x100)
#define EXIDY_HSEND                     (0x140)
#define EXIDY_HSSTART                   (0x120)
#define EXIDY_VTOTAL                    (0x118)
#define EXIDY_VBEND                     (0x000)
#define EXIDY_VBSTART                   (0x100)
#define EXIDY_VSEND                     (0x108)
#define EXIDY_VSSTART                   (0x100)


class exidy_state : public driver_device
{
public:
	exidy_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_dsw(*this, "DSW"),
		m_in0(*this, "IN0"),
		m_led(*this, "led%u", 0U),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_color_latch(*this, "color_latch"),
		m_videoram(*this, "videoram"),
		m_sprite1_xpos(*this, "sprite1_xpos"),
		m_sprite1_ypos(*this, "sprite1_ypos"),
		m_sprite2_xpos(*this, "sprite2_xpos"),
		m_sprite2_ypos(*this, "sprite2_ypos"),
		m_spriteno(*this, "spriteno"),
		m_sprite_enable(*this, "sprite_enable"),
		m_characterram(*this, "characterram")
	{
	}

	void venture(machine_config &config);
	void mtrap(machine_config &config);
	void pepper2(machine_config &config);

	ioport_value intsource_coins_r();
	DECLARE_INPUT_CHANGED_MEMBER(coin_count_w);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_ioport m_dsw;
	required_ioport m_in0;
	output_finder<2> m_led;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_color_latch;

	void base(machine_config &config);

	void exidy_video_config(uint8_t _collision_mask, uint8_t _collision_invert, int _is_2bpp);

	uint8_t exidy_interrupt_r();

	void exidy_map(address_map &map) ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_sprite1_xpos;
	required_shared_ptr<uint8_t> m_sprite1_ypos;
	required_shared_ptr<uint8_t> m_sprite2_xpos;
	required_shared_ptr<uint8_t> m_sprite2_ypos;
	required_shared_ptr<uint8_t> m_spriteno;
	required_shared_ptr<uint8_t> m_sprite_enable;
	optional_shared_ptr<uint8_t> m_characterram;

	emu_timer *m_collision_timer[128];
	uint8_t m_collision_mask = 0;
	uint8_t m_collision_invert = 0;
	int m_is_2bpp = 0;
	uint8_t m_int_condition = 0;
	bitmap_ind16 m_background_bitmap;
	bitmap_ind16 m_motion_object_1_vid;
	bitmap_ind16 m_motion_object_2_vid;
	bitmap_ind16 m_motion_object_2_clip;

	void mtrap_ocl_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(exidy_vblank_interrupt);

	TIMER_CALLBACK_MEMBER(latch_collision);
	void latch_condition(int collision);
	void set_1_color(int index, int which);
	void set_colors();
	void draw_background();
	int sprite_1_enabled();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void check_collision();

	void venture_map(address_map &map) ATTR_COLD;
	void mtrap_map(address_map &map) ATTR_COLD;
	void pepper2_map(address_map &map) ATTR_COLD;
};


class spectar_state : public exidy_state
{
public:
	spectar_state(const machine_config &mconfig, device_type type, const char *tag) :
		exidy_state(mconfig, type, tag),
		m_dac(*this, "dac"),
		m_samples(*this, "samples")
	{
	}

	void sidetrac(machine_config &config);
	void spectar(machine_config &config);
	void rallys(machine_config &config);
	void phantoma(machine_config &config);

	ioport_value spectar_coins_r();
	ioport_value rallys_coin1_r();

	void init_sidetrac();
	void init_spectar();

protected:
	virtual void machine_start() override ATTR_COLD;

	void set_max_freq(int freq) { m_max_freq = freq; }

	void spectar_audio_1_w(uint8_t data);
	void spectar_audio_2_w(uint8_t data);

	void adjust_sample(uint8_t freq);
	bool RISING_EDGE(uint8_t data, uint8_t bit) const { return (data & bit) && !(m_port_1_last & bit); }
	bool FALLING_EDGE(uint8_t data, uint8_t bit) const { return !(data & bit) && (m_port_1_last & bit); }

private:
	required_device<dac_bit_interface> m_dac;
	required_device<samples_device> m_samples;

	// Targ and Spectar samples
	int m_max_freq = 0;
	uint8_t m_port_1_last = 0;
	uint8_t m_tone_freq = 0;
	uint8_t m_tone_active = 0;

	void sidetrac_map(address_map &map) ATTR_COLD;
	void spectar_map(address_map &map) ATTR_COLD;
	void rallys_map(address_map &map) ATTR_COLD;
	void phantoma_map(address_map &map) ATTR_COLD;
};


class targ_state : public spectar_state
{
public:
	targ_state(const machine_config &mconfig, device_type type, const char *tag) :
		spectar_state(mconfig, type, tag),
		m_sound_prom(*this, "targ")
	{
	}

	void targ(machine_config &config);

	void init_targ();

protected:
	virtual void machine_start() override ATTR_COLD;

	void targ_audio_1_w(uint8_t data);
	void targ_audio_2_w(uint8_t data);

private:
	required_region_ptr<uint8_t> m_sound_prom;

	uint8_t m_port_2_last = 0;
	uint8_t m_tone_pointer = 0;

	void targ_map(address_map &map) ATTR_COLD;
};


class teetert_state : public exidy_state
{
public:
	teetert_state(const machine_config &mconfig, device_type type, const char *tag) :
		exidy_state(mconfig, type, tag),
		m_dial(*this, "DIAL")
	{
	}

	void teetert(machine_config &config);

	ioport_value teetert_input_r();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_ioport m_dial;
	uint8_t m_last_dial = 0;
};


class fax_state : public exidy_state
{
public:
	fax_state(const machine_config &mconfig, device_type type, const char *tag) :
		exidy_state(mconfig, type, tag),
		m_rom_bank(*this, "bank1")
	{
	}

	void fax(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_memory_bank m_rom_bank;

	void fax_bank_select_w(uint8_t data);

	void fax_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Special inputs
 *
 *************************************/

ioport_value exidy_state::intsource_coins_r()
{
	uint8_t const dsw = m_dsw->read();
	uint8_t const in0 = m_in0->read();
	return (BIT(~in0, 7) << 1) | BIT(dsw, 0);
}

ioport_value spectar_state::spectar_coins_r()
{
	uint8_t const dsw = m_dsw->read();
	uint8_t const in0 = m_in0->read();
	return (BIT(~in0, 7) << 1) | BIT(~dsw, 0);
}

ioport_value spectar_state::rallys_coin1_r()
{
	return BIT(m_in0->read(), 7);
}

ioport_value teetert_state::teetert_input_r()
{
	uint8_t const dial = m_dial->read();

	int result = (dial != m_last_dial) << 4;
	if (result != 0)
	{
		if (((dial - m_last_dial) & 0xff) < 0x80)
		{
			result |= 1;
			m_last_dial++;
		}
		else
			m_last_dial--;
	}

	return result;
}


/*************************************
 *
 *  Special inputs
 *
 *************************************/

INPUT_CHANGED_MEMBER(exidy_state::coin_count_w)
{
	machine().bookkeeping().coin_counter_w(param, newval);
}


/*************************************
 *
 *  Bank switching
 *
 *************************************/

void fax_state::fax_bank_select_w(uint8_t data)
{
	m_rom_bank->set_entry(data & 0x1f);

	if ((data & 0x1f) > 0x17)
		logerror("Banking to unpopulated ROM bank %02X!\n", data & 0x1f);

}

void exidy_state::mtrap_ocl_w(uint8_t data) // Mouse Trap (possibly others) set P1 and P2 leds value at 5101, too.
{
	*m_sprite_enable = data;

	m_led[0] = !BIT(data, 2);
	m_led[1] = !BIT(data, 4);
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void exidy_state::exidy_map(address_map &map)
{
	map(0x0000, 0x03ff).ram();
	map(0x4000, 0x43ff).mirror(0x0400).ram().share("videoram");
	map(0x5000, 0x5000).mirror(0x003f).writeonly().share("sprite1_xpos");
	map(0x5040, 0x5040).mirror(0x003f).writeonly().share("sprite1_ypos");
	map(0x5080, 0x5080).mirror(0x003f).writeonly().share("sprite2_xpos");
	map(0x50c0, 0x50c0).mirror(0x003f).writeonly().share("sprite2_ypos");
	map(0x5100, 0x5100).mirror(0x00fc).portr("DSW");
	map(0x5100, 0x5100).mirror(0x00fc).writeonly().share("spriteno");
	map(0x5101, 0x5101).mirror(0x00fc).portr("IN0");
	map(0x5101, 0x5101).mirror(0x00fc).writeonly().share("sprite_enable");
	map(0x5103, 0x5103).mirror(0x00fc).r(FUNC(exidy_state::exidy_interrupt_r));
	map(0x5210, 0x5212).writeonly().share("color_latch");
	map(0x5213, 0x5213).portr("IN2");
}


void spectar_state::sidetrac_map(address_map &map)
{
	exidy_map(map);
	map(0x0800, 0x3fff).rom();
	map(0x4800, 0x4fff).rom();
	map(0x5200, 0x5200).w(FUNC(spectar_state::spectar_audio_1_w));
	map(0x5201, 0x5201).w(FUNC(spectar_state::spectar_audio_2_w));
	map(0xff00, 0xffff).rom().region("maincpu", 0x3f00);
}


void targ_state::targ_map(address_map &map)
{
	exidy_map(map);
	map(0x0800, 0x3fff).rom();
	map(0x4800, 0x4fff).ram().share("characterram");
	map(0x5200, 0x5200).w(FUNC(targ_state::targ_audio_1_w));
	map(0x5201, 0x5201).w(FUNC(targ_state::targ_audio_2_w));
	map(0xff00, 0xffff).rom().region("maincpu", 0x3f00);
}


void spectar_state::spectar_map(address_map &map)
{
	exidy_map(map);
	map(0x0800, 0x3fff).rom();
	map(0x4800, 0x4fff).ram().share("characterram");
	map(0x5200, 0x5200).w(FUNC(spectar_state::spectar_audio_1_w));
	map(0x5201, 0x5201).w(FUNC(spectar_state::spectar_audio_2_w));
	map(0xff00, 0xffff).rom().region("maincpu", 0x3f00);
}


void spectar_state::rallys_map(address_map &map)
{
	map(0x0000, 0x03ff).ram();
	map(0x0800, 0x3fff).rom();
	map(0x4000, 0x43ff).mirror(0x0400).ram().share("videoram");
	map(0x4800, 0x4fff).ram().share("characterram");
	map(0x5000, 0x5000).writeonly().share("sprite1_xpos");
	map(0x5001, 0x5001).writeonly().share("sprite1_ypos");
	map(0x5100, 0x5100).mirror(0x00fc).portr("DSW");
	map(0x5100, 0x5100).mirror(0x00fc).writeonly().share("spriteno");
	map(0x5101, 0x5101).mirror(0x00fc).portr("IN0");
	map(0x5101, 0x5101).mirror(0x00fc).writeonly().share("sprite_enable");
	map(0x5103, 0x5103).mirror(0x00fc).r(FUNC(spectar_state::exidy_interrupt_r));
	map(0x5200, 0x5200).w(FUNC(spectar_state::spectar_audio_1_w));
	map(0x5201, 0x5201).w(FUNC(spectar_state::spectar_audio_2_w));
	map(0x5210, 0x5212).writeonly().share("color_latch");
	map(0x5213, 0x5213).portr("IN2");
	map(0x5300, 0x5300).writeonly().share("sprite2_xpos");
	map(0x5301, 0x5301).writeonly().share("sprite2_ypos");
	map(0xff00, 0xffff).rom().region("maincpu", 0x3f00);
}


void spectar_state::phantoma_map(address_map &map)
{
	rallys_map(map);
	map(0xf800, 0xffff).rom().region("maincpu", 0xf800); // the ROM is actually mapped high
}


void exidy_state::venture_map(address_map &map)
{
	exidy_map(map);
	map(0x4800, 0x4fff).ram().share("characterram");
	map(0x5200, 0x520f).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8000, 0xffff).rom();
}


void exidy_state::pepper2_map(address_map &map)
{
	exidy_map(map);
	map(0x4800, 0x4fff).noprw();
	map(0x5200, 0x520f).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x6000, 0x6fff).ram().share("characterram");
	map(0x8000, 0xffff).rom();
}

void exidy_state::mtrap_map(address_map &map)
{
	venture_map(map);
	map(0x5101, 0x5101).w(FUNC(exidy_state::mtrap_ocl_w));
}

void fax_state::fax_map(address_map &map)
{
	exidy_map(map);
	map(0x0400, 0x07ff).ram();
	map(0x1a00, 0x1a00).portr("IN4");
	map(0x1c00, 0x1c00).portr("IN3");
	map(0x2000, 0x2000).w(FUNC(fax_state::fax_bank_select_w));
	map(0x2000, 0x3fff).bankr(m_rom_bank);
	map(0x5200, 0x520f).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x5213, 0x5217).nopw(); // empty control lines on color/sound board
	map(0x6000, 0x6fff).ram().share("characterram");
	map(0x8000, 0xffff).rom();
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( sidetrac )
	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	/* 0x0c same as 0x08 */
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, "Top Score Award" ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("INTSOURCE")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( targ )
	PORT_START("DSW")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_COIN2 ) /* upright/cocktail switch? */
	PORT_DIPNAME( 0x02, 0x00, "Pence Coinage" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, "10P/1P, 50P Coin/6P" )
	PORT_DIPSETTING(    0x02, "2x10P/1P, 50P Coin/3P" )
	PORT_DIPNAME( 0x04, 0x00, "Top Score Award" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "Credit" )
	PORT_DIPSETTING(    0x04, "Extended Play" )
	PORT_DIPNAME( 0x18, 0x08, "Quarter Coinage" ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, "1C/1C (no display)" )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x80, "Currency" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "Quarters" )
	PORT_DIPSETTING(    0x00, "Pence" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("INTSOURCE")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_CUSTOM )
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(spectar_state, spectar_coins_r)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/* identical to Targ, the only difference is the additional Language dip switch */
static INPUT_PORTS_START( spectar )
	PORT_INCLUDE(targ)

	PORT_MODIFY("INTSOURCE")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( French ) )
	PORT_DIPSETTING(    0x02, DEF_STR( German ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Spanish ) )
	PORT_BIT( 0x1c, IP_ACTIVE_HIGH, IPT_CUSTOM )
INPUT_PORTS_END

static INPUT_PORTS_START( spectarrf ) // default to Spanish since it's a Spanish bootleg
	PORT_INCLUDE(spectar)

	PORT_MODIFY("INTSOURCE")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( French ) )
	PORT_DIPSETTING(    0x02, DEF_STR( German ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Spanish ) )
INPUT_PORTS_END

static INPUT_PORTS_START( rallys )
	PORT_INCLUDE(spectar)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )        PORT_CONDITION("DSW", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "2C/1P, 50P Coin/3P" )    PORT_CONDITION("DSW", 0x80, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x01, "1C/1P, 50P Coin/6P" )    PORT_CONDITION("DSW", 0x80, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x02, "1C/2P, 50P Coin/12P" )   PORT_CONDITION("DSW", 0x80, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x03, "1C/3P, 50P Coin/18P" )   PORT_CONDITION("DSW", 0x80, NOTEQUALS, 0x00)
	PORT_DIPNAME( 0x04, 0x00, "Top Score Award" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x00, "Credit" )
	PORT_DIPSETTING(    0x04, "Extended Play" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x20, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x60, "5" )
	PORT_DIPNAME( 0x80, 0x00, "Coin Mode" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, "Mode 1" )
	PORT_DIPSETTING(    0x80, "Mode 2" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )

	PORT_MODIFY("INTSOURCE")
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_CUSTOM )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(spectar_state, rallys_coin1_r)
INPUT_PORTS_END

static INPUT_PORTS_START( phantoma )
	PORT_INCLUDE(rallys)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )        PORT_CONDITION("DSW", 0x80, EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, "2F/1P, 5F Coin/3P" )     PORT_CONDITION("DSW", 0x80, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x01, "1F/1P, 5F Coin/6P" )     PORT_CONDITION("DSW", 0x80, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x02, "1F/2P, 5F Coin/12P" )    PORT_CONDITION("DSW", 0x80, NOTEQUALS, 0x00)
	PORT_DIPSETTING(    0x03, "1F/3P, 5F Coin/18P" )    PORT_CONDITION("DSW", 0x80, NOTEQUALS, 0x00)
INPUT_PORTS_END


static INPUT_PORTS_START( mtrap )
	PORT_START("DSW")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x06, "30000" )
	PORT_DIPSETTING(    0x04, "40000" )
	PORT_DIPSETTING(    0x02, "50000" )
	PORT_DIPSETTING(    0x00, "60000" )
	PORT_DIPNAME( 0x98, 0x98, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:4,5,8")
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, "Coin A 2C/1C Coin B 1C/3C" )
	PORT_DIPSETTING(    0x98, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, "Coin A 1C/1C Coin B 1C/4C" )
	PORT_DIPSETTING(    0x18, "Coin A 1C/1C Coin B 1C/5C" )
	PORT_DIPSETTING(    0x88, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, "Coin A 1C/3C Coin B 2C/7C" )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Dog Button")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("INTSOURCE")
/*
    The schematics claim these exist, but there's nothing in
    the ROMs to support that claim (as far as I can see):

    PORT_DIPNAME( 0x03, 0x00, DEF_STR( Language ) )
    PORT_DIPSETTING(    0x00, DEF_STR( English ) )
    PORT_DIPSETTING(    0x01, DEF_STR( French ) )
    PORT_DIPSETTING(    0x02, DEF_STR( German ) )
    PORT_DIPSETTING(    0x03, DEF_STR( Spanish ) )
    PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
    PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
    PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
*/

	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_CUSTOM )
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(exidy_state, intsource_coins_r)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Yellow Button")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Red Button")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING( 0x04, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00, DEF_STR( On ) )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Blue Button")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( venture )
	PORT_START("DSW")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, exidy_state, coin_count_w, 1)
	PORT_DIPNAME( 0x06, 0x00, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPSETTING(    0x04, "40000" )
	PORT_DIPSETTING(    0x06, "50000" )
	PORT_DIPNAME( 0x98, 0x80, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:4,5,8")
	PORT_DIPSETTING(    0x88, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_1C ) )
	/*0x90 same as 0x80 */
	PORT_DIPSETTING(    0x98, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, "Pence: A 2C/1C B 1C/3C" )
	PORT_DIPSETTING(    0x18, "Pence: A 1C/1C B 1C/6C" )
	/*0x10 same as 0x00 */
	PORT_DIPNAME( 0x60, 0x20, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x60, "5" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, exidy_state, coin_count_w, 0)

	PORT_START("INTSOURCE")
/*
    The schematics claim these exist, but there's nothing in
    the ROMs to support that claim (as far as I can see):

    PORT_DIPNAME( 0x03, 0x00, DEF_STR( Language ) )
    PORT_DIPSETTING(    0x00, DEF_STR( English ) )
    PORT_DIPSETTING(    0x01, DEF_STR( French ) )
    PORT_DIPSETTING(    0x02, DEF_STR( German ) )
    PORT_DIPSETTING(    0x03, DEF_STR( Spanish ) )
    PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
    PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
    PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
*/
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_CUSTOM )
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(exidy_state, intsource_coins_r)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( teetert )
	PORT_START("DSW")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x06, "20000" )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x02, "40000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x98, 0x98, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:4,5,8")
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, "Pence: A 2C/1C B 1C/3C" )
	PORT_DIPSETTING(    0x98, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, "Pence: A 1C/1C B 1C/4C" )
	PORT_DIPSETTING(    0x18, "Pence: A 1C/1C B 1C/5C" )
	PORT_DIPSETTING(    0x88, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, "1C/3C, 2C/7C" )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x60, "2" )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x44, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(teetert_state, teetert_input_r)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("INTSOURCE")
/*
    The schematics claim these exist, but there's nothing in
    the ROMs to support that claim (as far as I can see):

    PORT_DIPNAME( 0x03, 0x00, "Language" )
    PORT_DIPSETTING(    0x00, "English" )
    PORT_DIPSETTING(    0x01, "French" )
    PORT_DIPSETTING(    0x02, "German" )
    PORT_DIPSETTING(    0x03, "Spanish" )
    PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
    PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
    PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
*/
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(exidy_state, intsource_coins_r)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DIAL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(5) PORT_KEYDELTA(30) PORT_REVERSE
INPUT_PORTS_END


static INPUT_PORTS_START( pepper2 )
	PORT_START("DSW")
	PORT_BIT ( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x06, "40000 and 80000" )
	PORT_DIPSETTING(    0x04, "50000 and 100000" )
	PORT_DIPSETTING(    0x02, "70000 and 140000" ) // 1st Edition manual lists 60000
	PORT_DIPSETTING(    0x00, "90000 and 180000" ) // 1st Edition manual lists 70000
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x60, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x98, 0x98, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:4,5,8")
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, "Coin A 2C/1C Coin B 1C/3C" )
	PORT_DIPSETTING(    0x98, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, "Coin A 1C/1C Coin B 1C/4C" )
	PORT_DIPSETTING(    0x18, "Coin A 1C/1C Coin B 1C/5C" )
	PORT_DIPSETTING(    0x88, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, "1 Coin/3 Credits 2C/7C" )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )

	PORT_START("IN0")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT ( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT ( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("INTSOURCE")
/*
    The schematics claim these exist, but there's nothing in
    the ROMs to support that claim (as far as I can see):

    PORT_DIPNAME( 0x03, 0x00, DEF_STR( Language ) )
    PORT_DIPSETTING(    0x00, DEF_STR( English ) )
    PORT_DIPSETTING(    0x01, DEF_STR( French ) )
    PORT_DIPSETTING(    0x02, DEF_STR( German ) )
    PORT_DIPSETTING(    0x03, DEF_STR( Spanish ) )
    PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
    PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
    PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
*/
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_CUSTOM )
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(exidy_state, intsource_coins_r)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( fax )
	PORT_START("DSW")
	PORT_BIT ( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 )
	// note: set switches 2 to 8 to ON for freeplay
	PORT_DIPNAME( 0x06, 0x04, "Bonus Time" ) PORT_DIPLOCATION("SW1:2,3")
	PORT_DIPSETTING(    0x06, "8000" )
	PORT_DIPSETTING(    0x04, "13000" )
	PORT_DIPSETTING(    0x02, "18000" )
	PORT_DIPSETTING(    0x00, "25000" )
	PORT_DIPNAME( 0x60, 0x40, "Game/Bonus Times" ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x60, ":32/:24" )
	PORT_DIPSETTING(    0x40, ":48/:36" )
	PORT_DIPSETTING(    0x20, "1:04/:48" )
	PORT_DIPSETTING(    0x00, "1:12/1:04" )
	PORT_DIPNAME( 0x98, 0x98, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:4,5,8")
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, "Coin A 2C/1C Coin B 1C/3C" )
	PORT_DIPSETTING(    0x98, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, "Coin A 1C/1C Coin B 1C/4C" )
	PORT_DIPSETTING(    0x18, "Coin A 1C/1C Coin B 1C/5C" )
	PORT_DIPSETTING(    0x88, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, "1 Coin/3 Credits 2C/7C" )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_4C ) )

	PORT_START("IN0")
	PORT_BIT ( 0x7f, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("INTSOURCE")
/*
    The schematics claim these exist, but there's nothing in
    the ROMs to support that claim (as far as I can see):

    PORT_DIPNAME( 0x03, 0x00, DEF_STR( Language ) )
    PORT_DIPSETTING(    0x00, DEF_STR( English ) )
    PORT_DIPSETTING(    0x01, DEF_STR( French ) )
    PORT_DIPSETTING(    0x02, DEF_STR( German ) )
    PORT_DIPSETTING(    0x03, DEF_STR( Spanish ) )
    PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )
    PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
    PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
*/
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_CUSTOM )
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(exidy_state, intsource_coins_r)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("IN4")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1), STEP8(16*8,1), },
	{ STEP16(0,8) },
	8*32
};


static GFXDECODE_START( gfx_exidy )
	GFXDECODE_ENTRY( "gfx1", 0x0000, spritelayout, 0, 2 )
GFXDECODE_END



/*************************************
 *
 *  Video configuration
 *
 *************************************/

void exidy_state::exidy_video_config(uint8_t _collision_mask, uint8_t _collision_invert, int _is_2bpp)
{
	m_collision_mask   = _collision_mask;
	m_collision_invert = _collision_invert;
	m_is_2bpp          = _is_2bpp;
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

void exidy_state::video_start()
{
	m_screen->register_screen_bitmap(m_background_bitmap);
	m_motion_object_1_vid.allocate(16, 16);
	m_motion_object_2_vid.allocate(16, 16);
	m_motion_object_2_clip.allocate(16, 16);

	save_item(NAME(m_int_condition));
	save_item(NAME(m_background_bitmap));
	save_item(NAME(m_motion_object_1_vid));
	save_item(NAME(m_motion_object_2_vid));
	save_item(NAME(m_motion_object_2_clip));
}



/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

inline void exidy_state::latch_condition(int collision)
{
	collision ^= m_collision_invert;
	m_int_condition = (ioport("INTSOURCE")->read() & ~0x1c) | (collision & m_collision_mask);
}


INTERRUPT_GEN_MEMBER(exidy_state::exidy_vblank_interrupt)
{
	/* latch the current condition */
	latch_condition(0);
	m_int_condition &= ~0x80;

	/* set the IRQ line */
	device.execute().set_input_line(0, ASSERT_LINE);
}


uint8_t exidy_state::exidy_interrupt_r()
{
	/* clear any interrupts */
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(0, CLEAR_LINE);

	/* return the latched condition */
	return m_int_condition;
}



/*************************************
 *
 *  Palette handling
 *
 *************************************/

inline void exidy_state::set_1_color(int index, int which)
{
	m_palette->set_pen_color(index,
			pal1bit(m_color_latch[2] >> which),
			pal1bit(m_color_latch[1] >> which),
			pal1bit(m_color_latch[0] >> which));
}

void exidy_state::set_colors()
{
	/* motion object 1 */
	set_1_color(0, 0);
	set_1_color(1, 7);

	/* motion object 2 */
	set_1_color(2, 0);
	set_1_color(3, 6);

	/* characters */
	set_1_color(4, 4);
	set_1_color(5, 3);
	set_1_color(6, 2);
	set_1_color(7, 1);
}



/*************************************
 *
 *  Background update
 *
 *************************************/

void exidy_state::draw_background()
{
	const uint8_t *const cram = m_characterram ? &m_characterram[0] : memregion("maincpu")->base() + 0x4800;

	pen_t off_pen = 0;

	for (offs_t offs = 0; offs < 0x400; offs++)
	{
		uint8_t y = offs >> 5 << 3;
		uint8_t const code = m_videoram[offs];

		pen_t on_pen_1, on_pen_2;
		if (m_is_2bpp)
		{
			on_pen_1 = 4 + ((code >> 6) & 0x02);
			on_pen_2 = 5 + ((code >> 6) & 0x02);
		}
		else
		{
			on_pen_1 = 4 + ((code >> 6) & 0x03);
			on_pen_2 = off_pen;  /* unused */
		}

		for (uint8_t cy = 0; cy < 8; cy++)
		{
			uint8_t x = offs << 3;

			if (m_is_2bpp)
			{
				uint8_t data1 = cram[0x000 | (code << 3) | cy];
				uint8_t data2 = cram[0x800 | (code << 3) | cy];

				for (int i = 0; i < 8; i++)
				{
					if (data1 & 0x80)
						m_background_bitmap.pix(y, x) = (data2 & 0x80) ? on_pen_2 : on_pen_1;
					else
						m_background_bitmap.pix(y, x) = off_pen;

					x++;
					data1 <<= 1;
					data2 <<= 1;
				}
			}
			else // 1bpp
			{
				uint8_t data = cram[(code << 3) | cy];

				for (int i = 0; i < 8; i++)
				{
					m_background_bitmap.pix(y, x) = (data & 0x80) ? on_pen_1 : off_pen;

					x++;
					data <<= 1;
				}
			}

			y++;
		}
	}
}



/*************************************
 *
 *  Sprite hardware
 *
 *************************************/

inline int exidy_state::sprite_1_enabled()
{
	/* if the collision_mask is 0x00, then we are on old hardware that always has */
	/* sprite 1 enabled regardless */
	return (!(*m_sprite_enable & 0x80) || (*m_sprite_enable & 0x10) || (m_collision_mask == 0x00));
}


void exidy_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* draw sprite 2 first */
	int sprite_set_2 = ((*m_sprite_enable & 0x40) != 0);

	int sx = 236 - *m_sprite2_xpos - 4;
	int sy = 244 - *m_sprite2_ypos - 4;

	m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
			((*m_spriteno >> 4) & 0x0f) + 32 + 16 * sprite_set_2, 1,
			0, 0, sx, sy, 0);

	/* draw sprite 1 next */
	if (sprite_1_enabled())
	{
		int sprite_set_1 = ((*m_sprite_enable & 0x20) != 0);

		sx = 236 - *m_sprite1_xpos - 4;
		sy = 244 - *m_sprite1_ypos - 4;

		if (sy < 0) sy = 0;

		m_gfxdecode->gfx(0)->transpen(bitmap,cliprect,
				(*m_spriteno & 0x0f) + 16 * sprite_set_1, 0,
				0, 0, sx, sy, 0);
	}
}



/*************************************
 *
 *  Collision detection
 *
 *************************************/

/***************************************************************************

    Exidy hardware checks for two types of collisions based on the video
    signals.  If the Motion Object 1 and Motion Object 2 signals are on at
    the same time, an M1M2 collision bit gets set.  If the Motion Object 1
    and Background Character signals are on at the same time, an M1CHAR
    collision bit gets set.  So effectively, there's a pixel-by-pixel
    collision check comparing Motion Object 1 (the player) to the
    background and to the other Motion Object (typically a bad guy).

***************************************************************************/

TIMER_CALLBACK_MEMBER(exidy_state::latch_collision)
{
	/* latch the collision bits */
	latch_condition(param);

	/* set the IRQ line */
	m_maincpu->set_input_line(0, ASSERT_LINE);
}


void exidy_state::check_collision()
{
	uint8_t sprite_set_1 = ((*m_sprite_enable & 0x20) != 0);
	uint8_t sprite_set_2 = ((*m_sprite_enable & 0x40) != 0);
	const rectangle clip(0, 15, 0, 15);
	int org_1_x = 0, org_1_y = 0;
	int org_2_x = 0, org_2_y = 0;
	int count = 0;

	/* if there is nothing to detect, bail */
	if (m_collision_mask == 0)
		return;

	/* draw sprite 1 */
	m_motion_object_1_vid.fill(0xff, clip);
	if (sprite_1_enabled())
	{
		org_1_x = 236 - *m_sprite1_xpos - 4;
		org_1_y = 244 - *m_sprite1_ypos - 4;
		m_gfxdecode->gfx(0)->transpen(m_motion_object_1_vid,clip,
				(*m_spriteno & 0x0f) + 16 * sprite_set_1, 0,
				0, 0, 0, 0, 0);
	}

	/* draw sprite 2 */
	m_motion_object_2_vid.fill(0xff, clip);
	org_2_x = 236 - *m_sprite2_xpos - 4;
	org_2_y = 244 - *m_sprite2_ypos - 4;
	m_gfxdecode->gfx(0)->transpen(m_motion_object_2_vid,clip,
			((*m_spriteno >> 4) & 0x0f) + 32 + 16 * sprite_set_2, 0,
			0, 0, 0, 0, 0);

	/* draw sprite 2 clipped to sprite 1's location */
	m_motion_object_2_clip.fill(0xff, clip);
	if (sprite_1_enabled())
	{
		int sx = org_2_x - org_1_x;
		int sy = org_2_y - org_1_y;
		m_gfxdecode->gfx(0)->transpen(m_motion_object_2_clip,clip,
				((*m_spriteno >> 4) & 0x0f) + 32 + 16 * sprite_set_2, 0,
				0, 0, sx, sy, 0);
	}

	/* scan for collisions */
	for (int sy = 0; sy < 16; sy++)
		for (int sx = 0; sx < 16; sx++)
		{
			if (m_motion_object_1_vid.pix(sy, sx) != 0xff)
			{
				uint8_t current_collision_mask = 0;

				/* check for background collision (M1CHAR) */
				if (m_background_bitmap.pix(org_1_y + sy, org_1_x + sx) != 0)
					current_collision_mask |= 0x04;

				/* check for motion object collision (M1M2) */
				if (m_motion_object_2_clip.pix(sy, sx) != 0xff)
					current_collision_mask |= 0x10;

				/* if we got one, trigger an interrupt */
				if ((current_collision_mask & m_collision_mask) && count < 128)
				{
					m_collision_timer[count]->adjust(m_screen->time_until_pos(org_1_x + sx, org_1_y + sy), current_collision_mask);
					count++;
				}
			}

			if (m_motion_object_2_vid.pix(sy, sx) != 0xff)
			{
				/* check for background collision (M2CHAR) */
				if (m_background_bitmap.pix(org_2_y + sy, org_2_x + sx) != 0)
					if ((m_collision_mask & 0x08) && count < 128)
					{
						m_collision_timer[count]->adjust(m_screen->time_until_pos(org_2_x + sx, org_2_y + sy), 0x08);
						count++;
					}
			}
		}
}



/*************************************
 *
 *  Standard screen refresh callback
 *
 *************************************/

uint32_t exidy_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* refresh the colors from the palette (static or dynamic) */
	set_colors();

	/* update the background and draw it */
	draw_background();
	copybitmap(bitmap, m_background_bitmap, 0, 0, 0, 0, cliprect);

	/* draw the sprites */
	draw_sprites(bitmap, cliprect);

	/* check for collision, this will set the appropriate bits in collision_mask */
	check_collision();

	return 0;
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

void exidy_state::machine_start()
{
	m_led.resolve();

	for (int i = 0; i < 128; i++)
		m_collision_timer[i] = timer_alloc(FUNC(exidy_state::latch_collision), this);
}

void spectar_state::machine_start()
{
	exidy_state::machine_start();

	/* start_raw can't be called here: chan.source will be set by
	samples_device::device_start and then nulled out by samples_device::device_reset
	at the soft_reset stage of init_machine() and will never be set again.
	Thus, I've moved it to exidy_state::adjust_sample() were it will be set after
	machine initialization. */
	//m_samples->set_volume(3, 0);
	//m_samples->start_raw(3, sine_wave, 32, 1000, true);

	save_item(NAME(m_port_1_last));
	save_item(NAME(m_tone_freq));
	save_item(NAME(m_tone_active));
}

void targ_state::machine_start()
{
	spectar_state::machine_start();

	save_item(NAME(m_port_2_last));
	save_item(NAME(m_tone_pointer));
}

void teetert_state::machine_start()
{
	exidy_state::machine_start();

	save_item(NAME(m_last_dial));
}

void fax_state::machine_start()
{
	exidy_state::machine_start();

	m_rom_bank->configure_entries(0, 32, memregion("maincpu")->base() + 0x10000, 0x2000);
}


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void exidy_state::base(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, EXIDY_CPU_CLOCK);
	m_maincpu->set_vblank_int("screen", FUNC(exidy_state::exidy_vblank_interrupt));

	// video hardware
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_exidy);
	PALETTE(config, m_palette).set_entries(8);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);
	m_screen->set_raw(EXIDY_PIXEL_CLOCK, EXIDY_HTOTAL, EXIDY_HBEND, EXIDY_HBSTART, EXIDY_VTOTAL, EXIDY_VBEND, EXIDY_VBSTART);
	m_screen->set_screen_update(FUNC(exidy_state::screen_update));
	m_screen->set_palette(m_palette);
}


void spectar_state::sidetrac(machine_config &config)
{
	static const char *const sample_names[] =
	{
		"*targ",
		"expl",
		"shot",
		"sexpl",
		"spslow",
		"spfast",
		nullptr
	};

	base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &spectar_state::sidetrac_map);

	// video hardware
	exidy_video_config(0x00, 0x00, false);

	// audio hardware
	set_max_freq(525'000);

	SPEAKER(config, "speaker").front_center();

	SAMPLES(config, m_samples);
	m_samples->set_channels(4);
	m_samples->set_samples_names(sample_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.25);

	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.99);
}


void targ_state::targ(machine_config &config)
{
	sidetrac(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &targ_state::targ_map);

	// audio hardware
	set_max_freq(125'000);
}


void spectar_state::spectar(machine_config &config)
{
	sidetrac(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &spectar_state::spectar_map);
}


void spectar_state::rallys(machine_config &config)
{
	sidetrac(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &spectar_state::rallys_map);
}


void spectar_state::phantoma(machine_config &config)
{
	sidetrac(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &spectar_state::phantoma_map);
}


void exidy_state::venture(machine_config &config)
{
	base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &exidy_state::venture_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	// video hardware
	exidy_video_config(0x04, 0x04, false);

	// audio hardware
	pia6821_device &pia(PIA6821(config, "pia"));
	pia.writepa_handler().set("soundbd", FUNC(venture_sound_device::pb_w));
	pia.writepb_handler().set("soundbd", FUNC(venture_sound_device::pa_w));
	pia.ca2_handler().set("soundbd", FUNC(venture_sound_device::cb_w));
	pia.cb2_handler().set("soundbd", FUNC(venture_sound_device::ca_w));

	venture_sound_device &soundbd(EXIDY_VENTURE(config, "soundbd", 0));
	soundbd.pa_callback().set("pia", FUNC(pia6821_device::portb_w));
	soundbd.pb_callback().set("pia", FUNC(pia6821_device::porta_w));
	soundbd.ca2_callback().set("pia", FUNC(pia6821_device::cb1_w));
	soundbd.cb2_callback().set("pia", FUNC(pia6821_device::ca1_w));
}


void teetert_state::teetert(machine_config &config)
{
	venture(config);

	// basic machine hardware
	m_maincpu->set_periodic_int(FUNC(teetert_state::nmi_line_pulse), attotime::from_hz(10*60));

	// video hardware
	exidy_video_config(0x0c, 0x0c, false);
}


void exidy_state::mtrap(machine_config &config)
{
	base(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &exidy_state::mtrap_map);

	config.set_maximum_quantum(attotime::from_hz(1920));

	// video hardware
	exidy_video_config(0x14, 0x00, false);

	// audio hardware
	pia6821_device &pia(PIA6821(config, "pia"));
	pia.writepa_handler().set("soundbd", FUNC(venture_sound_device::pb_w));
	pia.writepb_handler().set("soundbd", FUNC(venture_sound_device::pa_w));
	pia.ca2_handler().set("soundbd", FUNC(venture_sound_device::cb_w));
	pia.cb2_handler().set("soundbd", FUNC(venture_sound_device::ca_w));

	mtrap_sound_device &soundbd(EXIDY_MTRAP(config, "soundbd", 0));
	soundbd.pa_callback().set("pia", FUNC(pia6821_device::portb_w));
	soundbd.pb_callback().set("pia", FUNC(pia6821_device::porta_w));
	soundbd.ca2_callback().set("pia", FUNC(pia6821_device::cb1_w));
	soundbd.cb2_callback().set("pia", FUNC(pia6821_device::ca1_w));
}


void exidy_state::pepper2(machine_config &config)
{
	venture(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &exidy_state::pepper2_map);

	// video hardware
	exidy_video_config(0x14, 0x04, true);
}


void fax_state::fax(machine_config &config)
{
	pepper2(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &fax_state::fax_map);

	// video hardware
	exidy_video_config(0x04, 0x04, true);
}


/*************************************************************************

    Targ hardware

*************************************************************************/

/* Sound channel usage
   0 = CPU music,  Shoot
   1 = Crash
   2 = Spectar sound
   3 = Tone generator
*/

static const int16_t sine_wave[32] =
{
	 0x0f0f,  0x0f0f,  0x0f0f,  0x0606,  0x0606,  0x0909,  0x0909,  0x0606,  0x0606,  0x0909,  0x0606,  0x0d0d,  0x0f0f,  0x0f0f,  0x0d0d,  0x0000,
	-0x191a, -0x2122, -0x1e1f, -0x191a, -0x1314, -0x191a, -0x1819, -0x1819, -0x1819, -0x1314, -0x1314, -0x1314, -0x1819, -0x1e1f, -0x1e1f, -0x1819
};


void spectar_state::adjust_sample(uint8_t freq)
{
	m_tone_freq = freq;

	if (!m_samples->playing(3))
	{
		m_samples->set_volume(3, 0);
		m_samples->start_raw(3, sine_wave, 32, 1000, true);
	}

	if ((m_tone_freq == 0xff) || (m_tone_freq == 0x00))
		m_samples->set_volume(3, 0);
	else
	{
		m_samples->set_frequency(3, 1.0 * m_max_freq / (0xff - m_tone_freq));
		m_samples->set_volume(3, m_tone_active);
	}
}


void spectar_state::spectar_audio_1_w(uint8_t data)
{
	// CPU music
	if (BIT(m_port_1_last ^ data, 0))
		m_dac->write(BIT(data, 0));

	// shot
	if (FALLING_EDGE(data, 0x02) && !m_samples->playing(0))
		m_samples->start(0,1);
	if (RISING_EDGE(data, 0x02))
		m_samples->start(0,1);

	// crash
	if (RISING_EDGE(data, 0x20))
	{
		if (data & 0x40)
			m_samples->start(1,0);
		else
			m_samples->start(1,2);
	}

	// Sspec
	if (data & 0x10)
		m_samples->stop(2);
	else
	{
		if ((data & 0x08) != (m_port_1_last & 0x08))
		{
			if (data & 0x08)
				m_samples->start(2,3,true);
			else
				m_samples->start(2,4,true);
		}
	}

	// Game (tone generator enable)
	if (FALLING_EDGE(data, 0x80))
	{
		m_tone_active = 0;

		adjust_sample(m_tone_freq);
	}

	if (RISING_EDGE(data, 0x80))
		m_tone_active = 1;

	m_port_1_last = data;
}


void spectar_state::spectar_audio_2_w(uint8_t data)
{
	adjust_sample(data);
}


void targ_state::targ_audio_1_w(uint8_t data)
{
	if (FALLING_EDGE(data, 0x80))
		m_tone_pointer = 0;

	spectar_audio_1_w(data);
}


void targ_state::targ_audio_2_w(uint8_t data)
{
	if ((data & 0x01) && !(m_port_2_last & 0x01))
	{
		m_tone_pointer = (m_tone_pointer + 1) & 0x0f;

		adjust_sample(m_sound_prom[((data & 0x02) << 3) | m_tone_pointer]);
	}

	m_port_2_last = data;
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( sidetrac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "stl8a-1",  0x2800, 0x0800, CRC(e41750ff) SHA1(3868a0d7e34a5118b39b31cff9e4fc839df541ff) )
	ROM_LOAD( "stl7a-2",  0x3000, 0x0800, CRC(57fb28dc) SHA1(6addd633d655d6a56b3e509d18e5f7c0ab2d0fbb) )
	ROM_LOAD( "stl6a-2",  0x3800, 0x0800, CRC(4226d469) SHA1(fd18b732b66082988b01e04adc2b1e5dae410c98) )
	ROM_LOAD( "stl9c-1",  0x4800, 0x0400, CRC(08710a84) SHA1(4bff254a14af7c968656ccc85277d31ab5a8f0c4) ) /* PROM instead of RAM char generator */

	ROM_REGION( 0x0200, "gfx1", 0 )
	ROM_LOAD( "stl11d",   0x0000, 0x0200, CRC(3bd1acc1) SHA1(06f900cb8f56cd4215c5fbf58a852426d390e0c1) )
ROM_END


ROM_START( targ )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hrl_10a1.10a", 0x1800, 0x0800, CRC(969744e1) SHA1(e123bdb02b3b5f6a59c1e7c9ef557fe6bb19c62c) )
	ROM_LOAD( "hrl_9a1.9a",   0x2000, 0x0800, CRC(a177a72d) SHA1(0e705e3e32021e55af4414fa0e2ccbc4980ee848) )
	ROM_LOAD( "hrl_8a1.8a",   0x2800, 0x0800, CRC(6e6928a5) SHA1(10c725b27225ac5aad8639b081df68dd61522cf2) )
	ROM_LOAD( "hrl_7a4.7a",   0x3000, 0x0800, CRC(e2f37f93) SHA1(b66743c296d3d4caba3bcbe6aa68cd6edd414816) )
	ROM_LOAD( "hrl_6a3.6a",   0x3800, 0x0800, CRC(a60a1bfc) SHA1(17c0e67e1a0b263b57d70a148cc5d5099fecbb40) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "hrl_11d-1.11d", 0x0000, 0x0400, CRC(9f03513e) SHA1(aa4763e49df65e5686a96431543580b8d8285893) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "hrl_5c-1.5c",   0x0000, 0x0100, CRC(a24290d0) SHA1(5f2888d168de874021b51c5d19a62fb8165e4454) ) // IM5623CJE (N82S129 compatible) BPROM - address decoder
	ROM_LOAD( "stl_6d-1.6d",   0x0100, 0x0020, CRC(e26f9053) SHA1(eec35b6aa2c2d305418306bf4a1754a0583f109f) ) // IM5610CPE (N82S123 compatible) BPROM - video RAM control
	ROM_LOAD( "hrl_14h-1.14h", 0x0120, 0x0020, CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) ) // IM5610CPE (N82S123 compatible) BPROM - sprite control

	ROM_REGION( 0x0020, "targ", 0 )
	ROM_LOAD( "hra2b-1",  0x0000, 0x0020, CRC(38e8024b) SHA1(adf1c1770695f7614c95eceb803f662c5b096a76) )    // sound "program" (tone frequencies)
ROM_END

ROM_START( targc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "358_ctl_10a-1_tsv_11-10-80.10a", 0x1800, 0x0800, CRC(058b3983) SHA1(8079667613c9273e95131c3c68cd92ce34c18148) ) // hand written label:  358   CTL   10A-1   TSV   11/10/80
	ROM_LOAD( "358_ctl_9a-1_tsv_11-10-80.9a",   0x2000, 0x0800, CRC(3ac44b6b) SHA1(8261ee7ee1c3cb05b2549464086bf6df09685743) ) // hand written label:  358   CTL   9A-1   TSV   11/10/80
	ROM_LOAD( "358_ctl_8a-1_tsv_11-10-80.8a",   0x2800, 0x0800, CRC(5c470021) SHA1(3638fc6827640857848cd649f10c1493025014de) ) // hand written label:  358   CTL   8A-1   TSV   11/10/80
	ROM_LOAD( "358_ctl_7a-1_tsv_11-10-80.7a",   0x3000, 0x0800, CRC(c774fd9b) SHA1(46272a64ad5cda0ff5ef3e9eeedefc555100a71a) ) // hand written label:  358   CTL   7A-1   TSV   11/10/80
	ROM_LOAD( "358_ctl_6a-1_tsv_11-10-80.6a",   0x3800, 0x0800, CRC(3d020439) SHA1(ebde4c851c9ecc310f110c7643a80275d97dc02c) ) // hand written label:  358   CTL   6A-1   TSV   11/10/80

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "hrl_11d-1.11d", 0x0000, 0x0400, CRC(9f03513e) SHA1(aa4763e49df65e5686a96431543580b8d8285893) )

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "hrl_5c-1.5c",   0x0000, 0x0100, CRC(a24290d0) SHA1(5f2888d168de874021b51c5d19a62fb8165e4454) ) // IM5623CJE (N82S129 compatible) BPROM - address decoder
	ROM_LOAD( "stl_6d-1.6d",   0x0100, 0x0020, CRC(e26f9053) SHA1(eec35b6aa2c2d305418306bf4a1754a0583f109f) ) // IM5610CPE (N82S123 compatible) BPROM - video RAM control
	ROM_LOAD( "hrl_14h-1.14h", 0x0120, 0x0020, CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) ) // IM5610CPE (N82S123 compatible) BPROM - sprite control

	ROM_REGION( 0x0020, "targ", 0 )
	ROM_LOAD( "hra2b-1",  0x0000, 0x0020, CRC(38e8024b) SHA1(adf1c1770695f7614c95eceb803f662c5b096a76) )    // sound "program" (tone frequencies)
ROM_END


ROM_START( spectar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spl11a-3.11a", 0x1000, 0x0800, CRC(08880aff) SHA1(3becef348245ff4c8b0aae4a14751ab740b7d160) )
	ROM_LOAD( "spl10a-2.10a", 0x1800, 0x0800, CRC(fca667c1) SHA1(168426f9e87c002d2673c0230fceac4d0831d594) )
	ROM_LOAD( "spl9a-3.9a",   0x2000, 0x0800, CRC(9d4ce8ba) SHA1(2ef45c225fe704e49d10247c3eba1ef14141b3b7) )
	ROM_LOAD( "spl8a-2.8a",   0x2800, 0x0800, CRC(cfacbadf) SHA1(77b27cf6f35e8e8dd2fd4f31bba2a96f3076163e) )
	ROM_LOAD( "spl7a-2.7a",   0x3000, 0x0800, CRC(4c4741ff) SHA1(8de72613a385095253bb9e6da76493caec3115e4) )
	ROM_LOAD( "spl6a-2.6a",   0x3800, 0x0800, CRC(0cb46b25) SHA1(65c5d2cc8df67225339dc8781dd29d4b57ded70c) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "hrl11d-2.11d", 0x0000, 0x0400, CRC(c55b645d) SHA1(0c18277939d74e3e1281a7f114a34781d30c2baf) )  /* this is actually not used (all FF) */
	ROM_CONTINUE(             0x0000, 0x0400 )  /* overwrite with the real one */

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "spl5c-2.5c",   0x0000, 0x0100, CRC(9ca2e061) SHA1(4111325b00a1017042d55c59308d41e8333ba627) ) // 6301 according to the Spectar manual, also seen as IM 5623CPE on PCB
	ROM_LOAD( "hrl6d-1.6d",   0x0100, 0x0020, CRC(e26f9053) SHA1(eec35b6aa2c2d305418306bf4a1754a0583f109f) ) // screen controller PROM, 6331 according to the Spectar manual, also seen as IM 5610CPE on PCB
	ROM_LOAD( "hrl14h-1.14h", 0x0120, 0x0020, CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) ) // 6331 according to the Spectar manual
ROM_END

ROM_START( spectar1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spl12a-1.12a", 0x0800, 0x0800, CRC(7002efb4) SHA1(fbb19ccd2aee49b78606eadcbef94e842e1be905) )
	ROM_LOAD( "spl11a-1.11a", 0x1000, 0x0800, CRC(8eb8526a) SHA1(0c42ee073fc73c89731dec4e3ecfc82c9b8301e9) )
	ROM_LOAD( "spl10a-1.10a", 0x1800, 0x0800, CRC(9d169b3d) SHA1(bee9d029df6e2fba24a5ba41a76f1658e9038838) )
	ROM_LOAD( "spl9a-1.9a",   0x2000, 0x0800, CRC(40e3eba1) SHA1(197aaed9a6159b6f3e347c0446be9e44733c1341) )
	ROM_LOAD( "spl8a-1.8a",   0x2800, 0x0800, CRC(64d8eb84) SHA1(a249c832ea951fddc6699f7ac0b4486e8a5be98e) )
	ROM_LOAD( "spl7a-1.7a",   0x3000, 0x0800, CRC(e08b0d8d) SHA1(6ffd6f8fb50c9fc09c38f56da7d6d005b66e78cc) )
	ROM_LOAD( "spl6a-1.6a",   0x3800, 0x0800, CRC(f0e4e71a) SHA1(5487a94650c964a7ab07f30aacab0b470dcb3b40) )

	ROM_REGION( 0x0400, "gfx1", 0 ) // some PCBs were seen with hrl11d-1 (CRC(9f03513e) SHA1(aa4763e49df65e5686a96431543580b8d8285893))
	ROM_LOAD( "hrl11d-2.11d", 0x0000, 0x0400, CRC(c55b645d) SHA1(0c18277939d74e3e1281a7f114a34781d30c2baf) )  /* this is actually not used (all FF) */
	ROM_CONTINUE(             0x0000, 0x0400 )  /* overwrite with the real one */

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "spl5c-2.5c",   0x0000, 0x0100, CRC(9ca2e061) SHA1(4111325b00a1017042d55c59308d41e8333ba627) ) // 6301 according to the Spectar manual, also seen as IM 5623CPE on PCB
	ROM_LOAD( "hrl6d-1.6d",   0x0100, 0x0020, CRC(e26f9053) SHA1(eec35b6aa2c2d305418306bf4a1754a0583f109f) ) // screen controller PROM, 6331 according to the Spectar manual, also seen as IM 5610CPE on PCB
	ROM_LOAD( "hrl14h-1.14h", 0x0120, 0x0020, CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) ) // 6331 according to the Spectar manual
ROM_END

ROM_START( spectarrf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "spl11a-2", 0x1000, 0x0800, CRC(0a0ea985) SHA1(e080eddb9094ce1a487022c670d283e71e744f51) )
	ROM_LOAD( "spl10a-2", 0x1800, 0x0800, CRC(fca667c1) SHA1(168426f9e87c002d2673c0230fceac4d0831d594) )
	ROM_LOAD( "spl9a-3",  0x2000, 0x0800, CRC(9d4ce8ba) SHA1(2ef45c225fe704e49d10247c3eba1ef14141b3b7) )
	ROM_LOAD( "spl8a-2",  0x2800, 0x0800, CRC(cfacbadf) SHA1(77b27cf6f35e8e8dd2fd4f31bba2a96f3076163e) )
	ROM_LOAD( "spl7a-2",  0x3000, 0x0800, CRC(4c4741ff) SHA1(8de72613a385095253bb9e6da76493caec3115e4) )
	ROM_LOAD( "sp26a-2",  0x3800, 0x0800, CRC(559ab427) SHA1(4c38417042be6032377dfb1e70aa814bf2395e55) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "hrl11d-2", 0x0000, 0x0400, CRC(c55b645d) SHA1(0c18277939d74e3e1281a7f114a34781d30c2baf) )  /* this is actually not used (all FF) */
	ROM_CONTINUE(         0x0000, 0x0400 )  /* overwrite with the real one */

	ROM_REGION( 0x0140, "proms", 0 )
	ROM_LOAD( "prom.5c",  0x0000, 0x0100, CRC(9ca2e061) SHA1(4111325b00a1017042d55c59308d41e8333ba627) )
	ROM_LOAD( "prom.6d",  0x0100, 0x0020, CRC(e26f9053) SHA1(eec35b6aa2c2d305418306bf4a1754a0583f109f) )
	ROM_LOAD( "hrl14h-1", 0x0120, 0x0020, CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) )
ROM_END

ROM_START( rallys )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rallys.01",   0x1000, 0x0400, CRC(a192b22b) SHA1(aaae0b1822f934df30b354f787ffa8848c71b52f) )
	ROM_LOAD( "rallys.02",   0x1400, 0x0400, CRC(19e730aa) SHA1(4f4e87d26c14a9ff2be5b4173c4e5804db551e33) )
	ROM_LOAD( "rallys.03",   0x1800, 0x0400, CRC(2a3e7b69) SHA1(d31a3e6acca87881741e88e70d46a4a0ee59fcf8) )
	ROM_LOAD( "rallys.04",   0x1c00, 0x0400, CRC(6d224696) SHA1(586bc8efdc8ac0a73e4a4300459efaf89021f6f5) )
	ROM_LOAD( "rallys.05",   0x2000, 0x0400, CRC(af943b5e) SHA1(819fa8a6ee78a39cdade49789cd42b4a215f82f0) )
	ROM_LOAD( "rallys.06",   0x2400, 0x0400, CRC(9b3d9e61) SHA1(b183e0844706713eb0a241a6e45c09c53e4077a3) )
	ROM_LOAD( "rallys.07",   0x2800, 0x0400, CRC(8ef8bc67) SHA1(c8d80cc8e89a9bc5d957d648d704e4c66b17932d) )
	ROM_LOAD( "rallys.08",   0x2c00, 0x0400, CRC(243c54f2) SHA1(813b3ecbd5642034b5de0bae96698ed2b036fc7b) )
	ROM_LOAD( "rallys.10",   0x3400, 0x0400, CRC(46f473d2) SHA1(e6a180fdcf2ac13ffab624554ef8aab128e80321) )
	ROM_LOAD( "rallys.09",   0x3c00, 0x0400, CRC(56ce8a94) SHA1(becd31cda58e59267517a39c82ccfa70abdd31c6) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "hrl11d-1",    0x0000, 0x0400, CRC(9f03513e) SHA1(aa4763e49df65e5686a96431543580b8d8285893) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "targ82s.123", 0x0000, 0x0020, CRC(9eb9125c) SHA1(660ad9b2c7c28c3fda4b10c1401c03165d131c61) ) /* unknown */
ROM_END

ROM_START( rallysa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rallys.01",   0x1000, 0x0400, CRC(a192b22b) SHA1(aaae0b1822f934df30b354f787ffa8848c71b52f) ) // 1a
	ROM_LOAD( "rallys.02",   0x1400, 0x0400, CRC(19e730aa) SHA1(4f4e87d26c14a9ff2be5b4173c4e5804db551e33) ) // 2a
	ROM_LOAD( "rallys.03",   0x1800, 0x0400, CRC(2a3e7b69) SHA1(d31a3e6acca87881741e88e70d46a4a0ee59fcf8) ) // 3a
	ROM_LOAD( "rallys.04",   0x1c00, 0x0400, CRC(6d224696) SHA1(586bc8efdc8ac0a73e4a4300459efaf89021f6f5) ) // 4a
	ROM_LOAD( "rallys.05",   0x2000, 0x0400, CRC(af943b5e) SHA1(819fa8a6ee78a39cdade49789cd42b4a215f82f0) ) // 5a
	ROM_LOAD( "rallys.06",   0x2400, 0x0400, CRC(9b3d9e61) SHA1(b183e0844706713eb0a241a6e45c09c53e4077a3) ) // 6a
	ROM_LOAD( "rallys.07",   0x2800, 0x0400, CRC(8ef8bc67) SHA1(c8d80cc8e89a9bc5d957d648d704e4c66b17932d) ) // p7
	ROM_LOAD( "rallys.08",   0x2c00, 0x0400, CRC(243c54f2) SHA1(813b3ecbd5642034b5de0bae96698ed2b036fc7b) ) // p8
	ROM_LOAD( "rallys.10",   0x3400, 0x0400, CRC(46f473d2) SHA1(e6a180fdcf2ac13ffab624554ef8aab128e80321) ) // 10b
	ROM_LOAD( "unk.c13",     0x3c00, 0x0400, CRC(57527332) SHA1(1c6ff898a06cc4fde01f39d1222e36cf762c4345) )

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "hrl11d-1",    0x0000, 0x0400, CRC(9f03513e) SHA1(aa4763e49df65e5686a96431543580b8d8285893) ) // 5c

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "6331.f6",     0x0000, 0x0020, CRC(9fb1daee) SHA1(2ec1189a57c95d7ad820eb12343fcf2c3fb08431) ) /* unknown */
ROM_END

ROM_START( panzer )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p1.1a",   0x1000, 0x0400, CRC(a192b22b) SHA1(aaae0b1822f934df30b354f787ffa8848c71b52f) )
	ROM_LOAD( "p2.2a",   0x1400, 0x0400, CRC(19e730aa) SHA1(4f4e87d26c14a9ff2be5b4173c4e5804db551e33) )
	ROM_LOAD( "p3.3a",   0x1800, 0x0400, CRC(2a3e7b69) SHA1(d31a3e6acca87881741e88e70d46a4a0ee59fcf8) )
	ROM_LOAD( "p4.4a",   0x1c00, 0x0400, CRC(6d224696) SHA1(586bc8efdc8ac0a73e4a4300459efaf89021f6f5) )
	ROM_LOAD( "p5.5a",   0x2000, 0x0400, CRC(af943b5e) SHA1(819fa8a6ee78a39cdade49789cd42b4a215f82f0) )
	ROM_LOAD( "p6.6a",   0x2400, 0x0400, CRC(9b3d9e61) SHA1(b183e0844706713eb0a241a6e45c09c53e4077a3) )
	ROM_LOAD( "p7.7a",   0x2800, 0x0400, CRC(8ef8bc67) SHA1(c8d80cc8e89a9bc5d957d648d704e4c66b17932d) )
	ROM_LOAD( "p8.8a",   0x2c00, 0x0400, CRC(243c54f2) SHA1(813b3ecbd5642034b5de0bae96698ed2b036fc7b) )
	ROM_LOAD( "p10.15b", 0x3400, 0x0400, CRC(46f473d2) SHA1(e6a180fdcf2ac13ffab624554ef8aab128e80321) )
	ROM_LOAD( "p9.13b",  0x3c00, 0x0400, CRC(f01e474e) SHA1(454d9f32f95b87819d490aefe26cc3db6de29700) ) // only rom different to rallys

	ROM_REGION( 0x0400, "gfx1", 0 )
	ROM_LOAD( "sc.4d",    0x0000, 0x0400, CRC(9f03513e) SHA1(aa4763e49df65e5686a96431543580b8d8285893) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "targ82s.123", 0x0000, 0x0020, CRC(9eb9125c) SHA1(660ad9b2c7c28c3fda4b10c1401c03165d131c61) ) /* unknown */
ROM_END

ROM_START( phantoma )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "156_a2",   0x1000, 0x0800, CRC(c5af9d34) SHA1(4c9f9a06cc7f6caf13a79fa8491db17b01b24774) )
	ROM_LOAD( "156_a3",   0x1800, 0x0800, CRC(30121e69) SHA1(1588cfb61eb9aa9598b3ff600cc02b0f1ac622bf) )
	ROM_LOAD( "156_a4",   0x2000, 0x0800, CRC(02d7fb94) SHA1(634e952a6a0d4c1a42692100e1913ecd5ab9faed) )
	ROM_LOAD( "156_a5",   0x2800, 0x0800, CRC(0127bc8d) SHA1(c555507f2662d1b45caf0b696147f70749292930) )
	ROM_LOAD( "156_a1",   0xf800, 0x0800, CRC(26292c0a) SHA1(d4157e261f6247cfafb948d1a9dbf0b02b2b84de) )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "156_d1",   0x0000, 0x0800, CRC(d18e5f14) SHA1(5cd327500e74eca378ad5d0924949f96dd955cf8) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "156_pal",  0x0000, 0x0020, CRC(9fb1daee) SHA1(2ec1189a57c95d7ad820eb12343fcf2c3fb08431) )
ROM_END

ROM_START( phantom )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "156_a2",   0x1000, 0x0800, CRC(c5af9d34) SHA1(4c9f9a06cc7f6caf13a79fa8491db17b01b24774) )
	ROM_LOAD( "156_a3",   0x1800, 0x0800, CRC(30121e69) SHA1(1588cfb61eb9aa9598b3ff600cc02b0f1ac622bf) )
	ROM_LOAD( "156_a4",   0x2000, 0x0800, CRC(02d7fb94) SHA1(634e952a6a0d4c1a42692100e1913ecd5ab9faed) )
	ROM_LOAD( "156_a5",   0x2800, 0x0800, CRC(0127bc8d) SHA1(c555507f2662d1b45caf0b696147f70749292930) )
	ROM_LOAD( "1a.bin",   0xf800, 0x0800, CRC(a4e40b67) SHA1(809d89393f80c1094fc4b1fc95e480aaa253c556) )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "156_d1",   0x0000, 0x0800, CRC(d18e5f14) SHA1(5cd327500e74eca378ad5d0924949f96dd955cf8) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "156_pal",  0x0000, 0x0020, CRC(9fb1daee) SHA1(2ec1189a57c95d7ad820eb12343fcf2c3fb08431) )
ROM_END


ROM_START( mtrap )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mtl-5_11a.11a", 0xa000, 0x1000, CRC(bd6c3eb5) SHA1(248956374222a09caa5b8c8fa842e9286d8e1c5d) )
	ROM_LOAD( "mtl-5_10a.10a", 0xb000, 0x1000, CRC(75b0593e) SHA1(48ce5382905f7c52929a95267d65fd0d3f0dcc92) )
	ROM_LOAD( "mtl-5_9a.9a",   0xc000, 0x1000, CRC(28dd20ff) SHA1(8ac44ec27ac25209c8b49da4c6b423917ed8907e) )
	ROM_LOAD( "mtl-5_8a.8a",   0xd000, 0x1000, CRC(cc09f7a4) SHA1(e806dc0e10b909b61e347f3e28eb024f3b3a9702) )
	ROM_LOAD( "mtl-5_7a.7a",   0xe000, 0x1000, CRC(caafbb6d) SHA1(96823ac4e49f192121c53f70382a20f7c52e290b) )
	ROM_LOAD( "mtl-5_6a.6a",   0xf000, 0x1000, CRC(d85e52ca) SHA1(51296247e365a468fe9458b722bbdbbeeed59fa0) )

	ROM_REGION( 0x8000, "soundbd:audiocpu", 0 )
	ROM_LOAD( "mta_5a.5a", 0x6800, 0x0800, CRC(dbe4ec02) SHA1(34e965428dbb4b9c558927bb80d19cb550b53228) )
	ROM_LOAD( "mta_6a.6a", 0x7000, 0x0800, CRC(c00f0c05) SHA1(398b0bc2a7e54b1e2326ed067bf6bb15cc52ed39) )
	ROM_LOAD( "mta_7a.7a", 0x7800, 0x0800, CRC(f3f16ca7) SHA1(3928c5da246c43036a7b4cbb140a1734d5f1fb03) )

	ROM_REGION( 0x4000, "soundbd:cvsdcpu", 0 ) /* 16k for digital sound processor */
	ROM_LOAD( "mta_2a.2a", 0x0000, 0x1000, CRC(13db8ed3) SHA1(939352323bdcd7df25db5eb2e30f269bcaebe6af) )
	ROM_LOAD( "mta_3a.3a", 0x1000, 0x1000, CRC(31bdfe5c) SHA1(b10bfe9e56dd617c5b4cd8b5bfec9c7f537b1086) )
	ROM_LOAD( "mta_4a.4a", 0x2000, 0x1000, CRC(1502d0e8) SHA1(8ef51ad4601299016f1821a5c65bec0199dd5474) )
	ROM_LOAD( "mta_1a.1a", 0x3000, 0x1000, CRC(658482a6) SHA1(c0d770fbeaa7cb3e0eef47d8caa0f8a78841692e) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "mtl_11d.11d", 0x0000, 0x0800, CRC(c6e4d339) SHA1(b091923e4d52e93d7c567afba217a10b2a3735fc) )

	ROM_REGION( 0x140, "proms", 0 )
	ROM_LOAD( "hrl14h.h14",  0x0000, 0x0020, CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) ) // 74S288 or 7603 - 6331 according to the Mouse Trap manual
	ROM_LOAD( "vel5c-11.c5", 0x0020, 0x0100, CRC(43b35bb7) SHA1(0a0cecea8faff9f3ff4c2ceda0b5b25e8e1cd667) ) // 742S97 - 6301 according to the Mouse Trap manual
	ROM_LOAD( "hrl6d.d6",    0x0120, 0x0020, CRC(e26f9053) SHA1(eec35b6aa2c2d305418306bf4a1754a0583f109f) ) // 74S288 or 7603 - 6331 according to the Mouse Trap manual
ROM_END

ROM_START( mtrap4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mtl-4_11a.11a", 0xa000, 0x1000, CRC(2879cb8d) SHA1(738bd3cd968fd733adcfe0fb5efdb2e2fcfb344e) )
	ROM_LOAD( "mtl-4_10a.10a", 0xb000, 0x1000, CRC(d7378af9) SHA1(44c8ba4c84f51306e5bdd64e6c255d1c1018db72) )
	ROM_LOAD( "mtl-4_9a.9a",   0xc000, 0x1000, CRC(be667e64) SHA1(c5f686e3c403691f14992354af690dc89e1722f7) )
	ROM_LOAD( "mtl-4_8a.8a",   0xd000, 0x1000, CRC(de0442f8) SHA1(61774921adf016b3a2ae18baa79af60dca2d9e45) )
	ROM_LOAD( "mtl-4_7a.7a",   0xe000, 0x1000, CRC(cdf8c6a8) SHA1(932ae9c0ea5700bd79862efa94742136d8e15641) )
	ROM_LOAD( "mtl-4_6a.6a",   0xf000, 0x1000, CRC(77d3f2e6) SHA1(2c21dd7ee326ccb41d3c64eec90a19198382edea) )

	ROM_REGION( 0x8000, "soundbd:audiocpu", 0 )
	ROM_LOAD( "mta_5a.5a", 0x6800, 0x0800, CRC(dbe4ec02) SHA1(34e965428dbb4b9c558927bb80d19cb550b53228) )
	ROM_LOAD( "mta_6a.6a", 0x7000, 0x0800, CRC(c00f0c05) SHA1(398b0bc2a7e54b1e2326ed067bf6bb15cc52ed39) )
	ROM_LOAD( "mta_7a.7a", 0x7800, 0x0800, CRC(f3f16ca7) SHA1(3928c5da246c43036a7b4cbb140a1734d5f1fb03) )

	ROM_REGION( 0x4000, "soundbd:cvsdcpu", 0 ) /* 16k for digital sound processor */
	ROM_LOAD( "mta_2a.2a", 0x0000, 0x1000, CRC(13db8ed3) SHA1(939352323bdcd7df25db5eb2e30f269bcaebe6af) )
	ROM_LOAD( "mta_3a.3a", 0x1000, 0x1000, CRC(31bdfe5c) SHA1(b10bfe9e56dd617c5b4cd8b5bfec9c7f537b1086) )
	ROM_LOAD( "mta_4a.4a", 0x2000, 0x1000, CRC(1502d0e8) SHA1(8ef51ad4601299016f1821a5c65bec0199dd5474) )
	ROM_LOAD( "mta_1a.1a", 0x3000, 0x1000, CRC(658482a6) SHA1(c0d770fbeaa7cb3e0eef47d8caa0f8a78841692e) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "mtl_11d.11d", 0x0000, 0x0800, CRC(c6e4d339) SHA1(b091923e4d52e93d7c567afba217a10b2a3735fc) )

	ROM_REGION( 0x140, "proms", 0 )
	ROM_LOAD( "hrl14h.h14",  0x0000, 0x0020, CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) ) // 74S288 or 7603 - 6331 according to the Mouse Trap manual
	ROM_LOAD( "vel5c-11.c5", 0x0020, 0x0100, CRC(43b35bb7) SHA1(0a0cecea8faff9f3ff4c2ceda0b5b25e8e1cd667) ) // 742S97 - 6301 according to the Mouse Trap manual
	ROM_LOAD( "hrl6d.d6",    0x0120, 0x0020, CRC(e26f9053) SHA1(eec35b6aa2c2d305418306bf4a1754a0583f109f) ) // 74S288 or 7603 - 6331 according to the Mouse Trap manual
ROM_END

ROM_START( mtrap4g )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gmtl-4_11a.11a", 0xa000, 0x1000, CRC(d84aa55e) SHA1(e13fe6cd027f1afa751d4914e50fac8a02d90bd0) )
	ROM_LOAD( "gmtl-4_10a.10a", 0xb000, 0x1000, CRC(c4a83a69) SHA1(8b904fb7b6416e668b400475f6491af8ac66c49d) )
	ROM_LOAD( "gmtl-4_9a.9a",   0xc000, 0x1000, CRC(9b7e5e7a) SHA1(6c63f118c24cdce9bb1343e778eeb524296dd3e0) )
	ROM_LOAD( "gmtl-4_8a.8a",   0xd000, 0x1000, CRC(fbcf1572) SHA1(c5e9a7427544a61389b489542e8e12a5bc7b6741) )
	ROM_LOAD( "gmtl-4_7a.7a",   0xe000, 0x1000, CRC(7786e51b) SHA1(08535122b84898cd91bc39d65996f6030807e0bf) )
	ROM_LOAD( "gmtl-4_6a.6a",   0xf000, 0x1000, CRC(bbb535cd) SHA1(db24015170951c1c8dba68261d948214c7ec805d) )

	ROM_REGION( 0x8000, "soundbd:audiocpu", 0 )
	ROM_LOAD( "mta-1_5a.5a", 0x6800, 0x0800, CRC(dbe4ec02) SHA1(34e965428dbb4b9c558927bb80d19cb550b53228) ) // Same data as other sets, but explicitly labeled  MTA-1
	ROM_LOAD( "mta-1_6a.6a", 0x7000, 0x0800, CRC(c00f0c05) SHA1(398b0bc2a7e54b1e2326ed067bf6bb15cc52ed39) )
	ROM_LOAD( "mta-1_7a.7a", 0x7800, 0x0800, CRC(f3f16ca7) SHA1(3928c5da246c43036a7b4cbb140a1734d5f1fb03) )

	ROM_REGION( 0x4000, "soundbd:cvsdcpu", 0 ) /* 16k for digital sound processor */
	ROM_LOAD( "mta-1_2a.2a", 0x0000, 0x1000, CRC(13db8ed3) SHA1(939352323bdcd7df25db5eb2e30f269bcaebe6af) ) // Same data as other sets, but explicitly labeled  MTA-1
	ROM_LOAD( "mta-1_3a.3a", 0x1000, 0x1000, CRC(31bdfe5c) SHA1(b10bfe9e56dd617c5b4cd8b5bfec9c7f537b1086) )
	ROM_LOAD( "mta-1_4a.4a", 0x2000, 0x1000, CRC(1502d0e8) SHA1(8ef51ad4601299016f1821a5c65bec0199dd5474) )
	ROM_LOAD( "mta-1_1a.1a", 0x3000, 0x1000, CRC(658482a6) SHA1(c0d770fbeaa7cb3e0eef47d8caa0f8a78841692e) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "mtl-1_11d.11d", 0x0000, 0x0800, CRC(c6e4d339) SHA1(b091923e4d52e93d7c567afba217a10b2a3735fc) )

	ROM_REGION( 0x140, "proms", 0 )
	ROM_LOAD( "hrl14h.h14",  0x0000, 0x0020, CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) ) // 74S288 or 7603 - 6331 according to the Mouse Trap manual
	ROM_LOAD( "vel5c-11.c5", 0x0020, 0x0100, CRC(43b35bb7) SHA1(0a0cecea8faff9f3ff4c2ceda0b5b25e8e1cd667) ) // 742S97 - 6301 according to the Mouse Trap manual
	ROM_LOAD( "hrl6d.d6",    0x0120, 0x0020, CRC(e26f9053) SHA1(eec35b6aa2c2d305418306bf4a1754a0583f109f) ) // 74S288 or 7603 - 6331 according to the Mouse Trap manual
ROM_END

ROM_START( mtrap3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mtl-3_11a.11a", 0xa000, 0x1000, CRC(4091be6e) SHA1(a4432f4588915276583f4b2e8db527fd24eb4291) )
	ROM_LOAD( "mtl-3_10a.10a", 0xb000, 0x1000, CRC(38250c2f) SHA1(b70a2a1d423ba90ca873cc43db40422abee07718) )
	ROM_LOAD( "mtl-3_9a.9a",   0xc000, 0x1000, CRC(2eec988e) SHA1(52167dabd672d16d454df746fb2c83c9e4253624) )
	ROM_LOAD( "mtl-3_8a.8a",   0xd000, 0x1000, CRC(744b4b1c) SHA1(94955d0703559d668988cb7045f835f955e5dd8a) )
	ROM_LOAD( "mtl-3_7a.7a",   0xe000, 0x1000, CRC(ea8ec479) SHA1(785557a242d9343c83cdc403b1f726cbea9d230f) )
	ROM_LOAD( "mtl-3_6a.6a",   0xf000, 0x1000, CRC(d72ba72d) SHA1(4c5b311bc7ecfc6133bc09e586635844e2f1d6a9) )

	ROM_REGION( 0x8000, "soundbd:audiocpu", 0 )
	ROM_LOAD( "mta_5a.5a", 0x6800, 0x0800, CRC(dbe4ec02) SHA1(34e965428dbb4b9c558927bb80d19cb550b53228) )
	ROM_LOAD( "mta_6a.6a", 0x7000, 0x0800, CRC(c00f0c05) SHA1(398b0bc2a7e54b1e2326ed067bf6bb15cc52ed39) )
	ROM_LOAD( "mta_7a.7a", 0x7800, 0x0800, CRC(f3f16ca7) SHA1(3928c5da246c43036a7b4cbb140a1734d5f1fb03) )

	ROM_REGION( 0x4000, "soundbd:cvsdcpu", 0 ) /* 16k for digital sound processor */
	ROM_LOAD( "mta_2a.2a", 0x0000, 0x1000, CRC(13db8ed3) SHA1(939352323bdcd7df25db5eb2e30f269bcaebe6af) )
	ROM_LOAD( "mta_3a.3a", 0x1000, 0x1000, CRC(31bdfe5c) SHA1(b10bfe9e56dd617c5b4cd8b5bfec9c7f537b1086) )
	ROM_LOAD( "mta_4a.4a", 0x2000, 0x1000, CRC(1502d0e8) SHA1(8ef51ad4601299016f1821a5c65bec0199dd5474) )
	ROM_LOAD( "mta_1a.1a", 0x3000, 0x1000, CRC(658482a6) SHA1(c0d770fbeaa7cb3e0eef47d8caa0f8a78841692e) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "mtl_11d.11d", 0x0000, 0x0800, CRC(c6e4d339) SHA1(b091923e4d52e93d7c567afba217a10b2a3735fc) )

	ROM_REGION( 0x140, "proms", 0 )
	ROM_LOAD( "hrl14h.h14",  0x0000, 0x0020, CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) ) // 74S288 or 7603 - 6331 according to the Mouse Trap manual
	ROM_LOAD( "vel5c-11.c5", 0x0020, 0x0100, CRC(43b35bb7) SHA1(0a0cecea8faff9f3ff4c2ceda0b5b25e8e1cd667) ) // 742S97 - 6301 according to the Mouse Trap manual
	ROM_LOAD( "hrl6d.d6",    0x0120, 0x0020, CRC(e26f9053) SHA1(eec35b6aa2c2d305418306bf4a1754a0583f109f) ) // 74S288 or 7603 - 6331 according to the Mouse Trap manual
ROM_END

ROM_START( mtrap2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mtl-2_11a.11a", 0xa000, 0x1000, CRC(a8cc3a18) SHA1(09fa9eff5ae5cb923d6e9137ba9a4b6944acffb2) )
	ROM_LOAD( "mtl-2_10a.10a", 0xb000, 0x1000, CRC(e26b074c) SHA1(e8882edc818860d887c08e8083a40c4717412b2f) )
	ROM_LOAD( "mtl-2_9a.9a",   0xc000, 0x1000, CRC(845394f6) SHA1(a150a6f00465abb787d8d46a2e32d8985364554e) )
	ROM_LOAD( "mtl-2_8a.8a",   0xd000, 0x1000, CRC(854d2d50) SHA1(4213999acbc03ee8300b2fcf86349cd7450adae5) )
	ROM_LOAD( "mtl-2_7a.7a",   0xe000, 0x1000, CRC(3d235f95) SHA1(57031d9784c55853dcf6e396ee685d8dd0d3ef87) )
	ROM_LOAD( "mtl-2_6a.6a",   0xf000, 0x1000, CRC(7ed7632a) SHA1(6800d3c1b901808373d3edd2a3fcf699f93d7daf) )

	ROM_REGION( 0x8000, "soundbd:audiocpu", 0 )
	ROM_LOAD( "mta_5a.5a", 0x6800, 0x0800, CRC(dbe4ec02) SHA1(34e965428dbb4b9c558927bb80d19cb550b53228) )
	ROM_LOAD( "mta_6a.6a", 0x7000, 0x0800, CRC(c00f0c05) SHA1(398b0bc2a7e54b1e2326ed067bf6bb15cc52ed39) )
	ROM_LOAD( "mta_7a.7a", 0x7800, 0x0800, CRC(f3f16ca7) SHA1(3928c5da246c43036a7b4cbb140a1734d5f1fb03) )

	ROM_REGION( 0x4000, "soundbd:cvsdcpu", 0 ) /* 16k for digital sound processor */
	ROM_LOAD( "mta_2a.2a", 0x0000, 0x1000, CRC(13db8ed3) SHA1(939352323bdcd7df25db5eb2e30f269bcaebe6af) )
	ROM_LOAD( "mta_3a.3a", 0x1000, 0x1000, CRC(31bdfe5c) SHA1(b10bfe9e56dd617c5b4cd8b5bfec9c7f537b1086) )
	ROM_LOAD( "mta_4a.4a", 0x2000, 0x1000, CRC(1502d0e8) SHA1(8ef51ad4601299016f1821a5c65bec0199dd5474) )
	ROM_LOAD( "mta_1a.1a", 0x3000, 0x1000, CRC(658482a6) SHA1(c0d770fbeaa7cb3e0eef47d8caa0f8a78841692e) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "mtl_11d.11d", 0x0000, 0x0800, CRC(c6e4d339) SHA1(b091923e4d52e93d7c567afba217a10b2a3735fc) )

	ROM_REGION( 0x140, "proms", 0 )
	ROM_LOAD( "hrl14h.h14",  0x0000, 0x0020, CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) ) // 74S288 or 7603 - 6331 according to the Mouse Trap manual
	ROM_LOAD( "vel5c-11.c5", 0x0020, 0x0100, CRC(43b35bb7) SHA1(0a0cecea8faff9f3ff4c2ceda0b5b25e8e1cd667) ) // 742S97 - 6301 according to the Mouse Trap manual
	ROM_LOAD( "hrl6d.d6",    0x0120, 0x0020, CRC(e26f9053) SHA1(eec35b6aa2c2d305418306bf4a1754a0583f109f) ) // 74S288 or 7603 - 6331 according to the Mouse Trap manual
ROM_END

ROM_START( mtrapb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cpu.p2",  0xa000, 0x1000, CRC(a0faa3e5) SHA1(2a9259d945619a5188f8903f46ddadc685516c43) )
	ROM_LOAD( "cpu.m2",  0xb000, 0x1000, CRC(d7378af9) SHA1(44c8ba4c84f51306e5bdd64e6c255d1c1018db72) )
	ROM_LOAD( "cpu.l2",  0xc000, 0x1000, CRC(be667e64) SHA1(c5f686e3c403691f14992354af690dc89e1722f7) )
	ROM_LOAD( "cpu.k2",  0xd000, 0x1000, CRC(69471f27) SHA1(17fe085cc4ebb527a0f85cf5a0a66778e0df443e) )
	ROM_LOAD( "cpu.j2",  0xe000, 0x1000, CRC(1eb0c4c9) SHA1(b7049b86385798f1098945d07fe41b71ddbbc980) )
	ROM_LOAD( "cpu.h2",  0xf000, 0x1000, CRC(16ea9a51) SHA1(59714f50c82b54f490c69fb5b91ac0aa16cb9abb) )

	ROM_REGION( 0x8000, "soundbd:audiocpu", 0 )
	ROM_LOAD( "2564.j10",     0x6000, 0x2000, CRC(d4160aa8) SHA1(d3bae8fa54e71c397ec60f998a012e088588a2e4) )

	ROM_REGION( 0x4000, "soundbd:cvsdcpu", 0 ) /* 16k for digital sound processor */
	ROM_LOAD( "mta_2a.2a", 0x0000, 0x1000, CRC(13db8ed3) SHA1(939352323bdcd7df25db5eb2e30f269bcaebe6af) )
	ROM_LOAD( "mta_3a.3a", 0x1000, 0x1000, CRC(31bdfe5c) SHA1(b10bfe9e56dd617c5b4cd8b5bfec9c7f537b1086) )
	ROM_LOAD( "mta_4a.4a", 0x2000, 0x1000, CRC(1502d0e8) SHA1(8ef51ad4601299016f1821a5c65bec0199dd5474) )
	ROM_LOAD( "mta_1a.1a", 0x3000, 0x1000, CRC(658482a6) SHA1(c0d770fbeaa7cb3e0eef47d8caa0f8a78841692e) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "2516.j6",   0x0000, 0x0800, CRC(c6e4d339) SHA1(b091923e4d52e93d7c567afba217a10b2a3735fc) )

	ROM_REGION( 0x140, "proms", 0 )
	ROM_LOAD( "74s288.6c", 0x0000, 0x0020, BAD_DUMP CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) ) // 6331 according to the Mouse Trap manual
	ROM_LOAD( "24s10n.1c", 0x0020, 0x0100, CRC(43b35bb7) SHA1(0a0cecea8faff9f3ff4c2ceda0b5b25e8e1cd667) ) // 6301 according to the Mouse Trap manual
	ROM_LOAD( "74s288.3d", 0x0120, 0x0020, CRC(e26f9053) SHA1(eec35b6aa2c2d305418306bf4a1754a0583f109f) ) // 6331 according to the Mouse Trap manual
ROM_END

/*
This bootleg was manufactured in Italy. 2 different PCBs were dumped and dumps match.
main PCB is marked: "900T.C"
piggyback PCB is marked: "LC" (it's Italian for "Lato Componenti" = Component Side)
piggyback PCB fits into main PCB @ 8l
*/
ROM_START( mtrapb2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ms11.2p",   0xa000, 0x1000, CRC(2879cb8d) SHA1(738bd3cd968fd733adcfe0fb5efdb2e2fcfb344e) )
	ROM_LOAD( "ms10.2m",   0xb000, 0x1000, CRC(d7378af9) SHA1(44c8ba4c84f51306e5bdd64e6c255d1c1018db72) )
	ROM_LOAD( "ms9.2l",    0xc000, 0x1000, CRC(be667e64) SHA1(c5f686e3c403691f14992354af690dc89e1722f7) )
	ROM_LOAD( "ms8.2k",    0xd000, 0x1000, CRC(de0442f8) SHA1(61774921adf016b3a2ae18baa79af60dca2d9e45) )
	ROM_LOAD( "ms7.2h",    0xe000, 0x1000, CRC(cdf8c6a8) SHA1(932ae9c0ea5700bd79862efa94742136d8e15641) )
	ROM_LOAD( "ms6.2f",    0xf000, 0x1000, CRC(77d3f2e6) SHA1(2c21dd7ee326ccb41d3c64eec90a19198382edea) )

	ROM_REGION( 0x8000, "soundbd:audiocpu", 0 )
	ROM_LOAD( "ms1a.10j", 0x6000, 0x2000, CRC(d4160aa8) SHA1(d3bae8fa54e71c397ec60f998a012e088588a2e4) )

	ROM_REGION( 0x4000, "soundbd:cvsdcpu", 0 ) /* 16k for digital sound processor */
	ROM_LOAD( "ms2a.8d", 0x0000, 0x1000, CRC(13db8ed3) SHA1(939352323bdcd7df25db5eb2e30f269bcaebe6af) )
	ROM_LOAD( "ms3a.8e", 0x1000, 0x1000, CRC(31bdfe5c) SHA1(b10bfe9e56dd617c5b4cd8b5bfec9c7f537b1086) )
	ROM_LOAD( "ms4a.8f", 0x2000, 0x1000, CRC(1502d0e8) SHA1(8ef51ad4601299016f1821a5c65bec0199dd5474) )
	ROM_LOAD( "ms5a.8h", 0x3000, 0x1000, CRC(658482a6) SHA1(c0d770fbeaa7cb3e0eef47d8caa0f8a78841692e) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "ms11d.6j", 0x0000, 0x0800, CRC(c6e4d339) SHA1(b091923e4d52e93d7c567afba217a10b2a3735fc) )

	ROM_REGION( 0x140, "proms", 0 )
	ROM_LOAD( "74s288.6c", 0x0000, 0x0020, CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) )
	ROM_LOAD( "24s10n.1c", 0x0020, 0x0100, CRC(43b35bb7) SHA1(0a0cecea8faff9f3ff4c2ceda0b5b25e8e1cd667) )
	ROM_LOAD( "74s288.3d", 0x0120, 0x0020, CRC(e26f9053) SHA1(eec35b6aa2c2d305418306bf4a1754a0583f109f) )
ROM_END


ROM_START( venture )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "13a-cpu", 0x8000, 0x1000, CRC(f4e4d991) SHA1(6683c1552b56b20f2296e461aff697af73563792) )
	ROM_LOAD( "12a-cpu", 0x9000, 0x1000, CRC(c6d8cb04) SHA1(3b9ae8fdc35117c73c91daed66e93e5344bdcd7e) )
	ROM_LOAD( "11a-cpu", 0xa000, 0x1000, CRC(3bdb01f4) SHA1(3c1f43a3c37a21524b64d69e4dae58af8c2e0d90) )
	ROM_LOAD( "10a-cpu", 0xb000, 0x1000, CRC(0da769e9) SHA1(3604dc08c63461b2ea957a396887fb32e4a1a970) )
	ROM_LOAD( "9a-cpu",  0xc000, 0x1000, CRC(0ae05855) SHA1(29b3c2ca9740aa753e90131e6edcc61f414277e1) )
	ROM_LOAD( "8a-cpu",  0xd000, 0x1000, CRC(4ae59676) SHA1(36fc9dce9dd0c764a861634859ca0d7f98e20382) )
	ROM_LOAD( "7a-cpu",  0xe000, 0x1000, CRC(48d66220) SHA1(97b1605170c67b3a945b4d5f088df79328e163ce) )
	ROM_LOAD( "6a-cpu",  0xf000, 0x1000, CRC(7b78cf49) SHA1(1d484172465d3db6c4fc3733aa2b409e3a2e228f) )

	ROM_REGION( 0x8000, "soundbd:audiocpu", 0 )
	ROM_LOAD( "vea_3a-3.3a", 0x5800, 0x0800, CRC(4ea1c3d9) SHA1(d0c99c9d5b887d717c68e8745906ae4e65aec6ad) )
	ROM_LOAD( "vea_4a-3.4a", 0x6000, 0x0800, CRC(5154c39e) SHA1(e6f011630eb1aa4116a0e5824ad6b65c1be2455f) )
	ROM_LOAD( "vea_5a-3.5a", 0x6800, 0x0800, CRC(1e1e3916) SHA1(867e586583e07cd01e0e852f6ea52a040995725d) )
	ROM_LOAD( "vea_6a-3.6a", 0x7000, 0x0800, CRC(80f3357a) SHA1(f1ee638251e8676a526e6367c11866b1d52f5910) )
	ROM_LOAD( "vea_7a-3.7a", 0x7800, 0x0800, CRC(466addc7) SHA1(0230b5365d6aeee3ca47666a9eadee4141de125b) )

	ROM_REGION( 0x0800, "gfx1", 0 )
//  ROM_LOAD( "11d-cpu", 0x0000, 0x0800, BAD_DUMP CRC(b4bb2503) SHA1(67303603b7c5e6301e976ef19f81c7519648b179) ) // bytes 0x536 & 0x537 have the high bit set (IE:0x80 instead of 0x00 in vel_11d-2.11d)
	ROM_LOAD( "vel_11d-2.11d", 0x0000, 0x0800, CRC(ea6fd981) SHA1(46b1658e1607423d5a073f14097c2a48d59057c0) )

	ROM_REGION( 0x140, "proms", 0 )
	ROM_LOAD( "hrl14h-1.h14", 0x0000, 0x0020, CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) )
	ROM_LOAD( "vel5c-1.c5",   0x0020, 0x0100, CRC(43b35bb7) SHA1(0a0cecea8faff9f3ff4c2ceda0b5b25e8e1cd667) )
	ROM_LOAD( "hrl6d-1.d6",   0x0120, 0x0020, CRC(e26f9053) SHA1(eec35b6aa2c2d305418306bf4a1754a0583f109f) )
ROM_END

ROM_START( venture5a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vel_13a-57e.a13", 0x8000, 0x1000, CRC(4c833f99) SHA1(1ff4eafe48b9f0ab8a123659d78c3dfa0bf56d7d) )
	ROM_LOAD( "vel_12a-5.a12",   0x9000, 0x1000, CRC(8163cefc) SHA1(7061819dd1105e8368c045dad2effae62d124539) )
	ROM_LOAD( "vel_11a-5.a11",   0xa000, 0x1000, CRC(324a5054) SHA1(f845ff2f717ea627891e0dc9d6e66f690c0843d8) )
	ROM_LOAD( "vel_10a-5.a10",   0xb000, 0x1000, CRC(24358203) SHA1(10c3ea83a892d6fd2751e590afe45bffa65bd6e0) )
	ROM_LOAD( "vel_9a-5.a9",     0xc000, 0x1000, CRC(04428165) SHA1(6d8d860ce1f805ba2eb315f47c8660799256e921) )
	ROM_LOAD( "vel_8a-5.a8",     0xd000, 0x1000, CRC(4c1a702a) SHA1(7f6a68d3cfdd885108eebb7ea76b3c2ce6070b18) )
	ROM_LOAD( "vel_7a-5.a7",     0xe000, 0x1000, CRC(1aab27c2) SHA1(66c7274dbb8bda3c78cc61d96a6cb1a9b29939b5) )
	ROM_LOAD( "vel_6a-5.a6",     0xf000, 0x1000, CRC(767bdd71) SHA1(334a903e05fc86186f90aa2d9ce3b0d367d7e516) )

	ROM_REGION( 0x8000, "soundbd:audiocpu", 0 )
	ROM_LOAD( "vea_3a-3.3a", 0x5800, 0x0800, CRC(4ea1c3d9) SHA1(d0c99c9d5b887d717c68e8745906ae4e65aec6ad) )
	ROM_LOAD( "vea_4a-3.4a", 0x6000, 0x0800, CRC(5154c39e) SHA1(e6f011630eb1aa4116a0e5824ad6b65c1be2455f) )
	ROM_LOAD( "vea_5a-3.5a", 0x6800, 0x0800, CRC(1e1e3916) SHA1(867e586583e07cd01e0e852f6ea52a040995725d) )
	ROM_LOAD( "vea_6a-3.6a", 0x7000, 0x0800, CRC(80f3357a) SHA1(f1ee638251e8676a526e6367c11866b1d52f5910) )
	ROM_LOAD( "vea_7a-3.7a", 0x7800, 0x0800, CRC(466addc7) SHA1(0230b5365d6aeee3ca47666a9eadee4141de125b) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "vel_11d-2.11d", 0x0000, 0x0800, CRC(ea6fd981) SHA1(46b1658e1607423d5a073f14097c2a48d59057c0) )

	ROM_REGION( 0x140, "proms", 0 )
	ROM_LOAD( "hrl14h-1.h14", 0x0000, 0x0020, CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) )
	ROM_LOAD( "vel5c-1.c5",   0x0020, 0x0100, CRC(43b35bb7) SHA1(0a0cecea8faff9f3ff4c2ceda0b5b25e8e1cd667) )
	ROM_LOAD( "hrl6d-1.d6",   0x0120, 0x0020, CRC(e26f9053) SHA1(eec35b6aa2c2d305418306bf4a1754a0583f109f) )
ROM_END

ROM_START( venture4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "vel_13a-4.13a", 0x8000, 0x1000, CRC(1c5448f9) SHA1(59d3ca2a2d7048f5f7bd23fa5d9c9a2cc0734cb8) )
	ROM_LOAD( "vel_12a-4.12a", 0x9000, 0x1000, CRC(e62491cc) SHA1(a98b6c6e60d83fd4591d0de145a99c5e4576121a) )
	ROM_LOAD( "vel_11a-4.11a", 0xa000, 0x1000, CRC(e91faeaf) SHA1(ce50a9f1016671282d16f2d0ad3553598e0c7e89) )
	ROM_LOAD( "vel_10a-4.10a", 0xb000, 0x1000, CRC(da3a2991) SHA1(2b5175b0f3642e735b6d87fbd5b75118cf6b7faa) )
	ROM_LOAD( "vel_9a-4.a9",   0xc000, 0x1000, CRC(d1887b11) SHA1(40ed1e1bdcb95d6e317cb5e4fb8572a314b3fbf8) )
	ROM_LOAD( "vel_8a-4.a8",   0xd000, 0x1000, CRC(8e8153fc) SHA1(409cf0ed39ef04c1e9359f0499d7cba3aed8f36e) )
	ROM_LOAD( "vel_7a-4.a7",   0xe000, 0x1000, CRC(0a091701) SHA1(ffdea1d60371779d0c28fb3c6111639cace79dad) )
	ROM_LOAD( "vel_6a-4.a6",   0xf000, 0x1000, CRC(7b165f67) SHA1(4109797bcfd33c870234930790e3cecaaf90b706) )

	ROM_REGION( 0x8000, "soundbd:audiocpu", 0 )
	ROM_LOAD( "vea_3a-2.3a", 0x5800, 0x0800, CRC(83b8836f) SHA1(ec0e2de62caea61ceff56e924449213997bff8cd) )
	ROM_LOAD( "vea_3a-2.4a", 0x6000, 0x0800, CRC(5154c39e) SHA1(e6f011630eb1aa4116a0e5824ad6b65c1be2455f) ) // here down, same data as the "-3" revision listed above
	ROM_LOAD( "vea_3a-2.5a", 0x6800, 0x0800, CRC(1e1e3916) SHA1(867e586583e07cd01e0e852f6ea52a040995725d) )
	ROM_LOAD( "vea_3a-2.6a", 0x7000, 0x0800, CRC(80f3357a) SHA1(f1ee638251e8676a526e6367c11866b1d52f5910) )
	ROM_LOAD( "vea_3a-2.7a", 0x7800, 0x0800, CRC(466addc7) SHA1(0230b5365d6aeee3ca47666a9eadee4141de125b) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "vel_11d-2.11d", 0x0000, 0x0800, CRC(ea6fd981) SHA1(46b1658e1607423d5a073f14097c2a48d59057c0) )

	ROM_REGION( 0x140, "proms", 0 )
	ROM_LOAD( "hrl14h-1.h14", 0x0000, 0x0020, CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) )
	ROM_LOAD( "vel5c-1.c5",   0x0020, 0x0100, CRC(43b35bb7) SHA1(0a0cecea8faff9f3ff4c2ceda0b5b25e8e1cd667) )
	ROM_LOAD( "hrl6d-1.d6",   0x0120, 0x0020, CRC(e26f9053) SHA1(eec35b6aa2c2d305418306bf4a1754a0583f109f) )
ROM_END

ROM_START( venture5b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d2732.2s", 0x8000, 0x1000, CRC(87d69fe9) SHA1(d5ccf71af478873f32e23530a62515327f39f672) ) // only unique program ROM
	ROM_LOAD( "d2732.2r", 0x9000, 0x1000, CRC(8163cefc) SHA1(7061819dd1105e8368c045dad2effae62d124539) )
	ROM_LOAD( "d2732.2n", 0xa000, 0x1000, CRC(324a5054) SHA1(f845ff2f717ea627891e0dc9d6e66f690c0843d8) )
	ROM_LOAD( "d2732.2m", 0xb000, 0x1000, CRC(24358203) SHA1(10c3ea83a892d6fd2751e590afe45bffa65bd6e0) )
	ROM_LOAD( "d2732.2l", 0xc000, 0x1000, CRC(04428165) SHA1(6d8d860ce1f805ba2eb315f47c8660799256e921) )
	ROM_LOAD( "d2732.2k", 0xd000, 0x1000, CRC(4c1a702a) SHA1(7f6a68d3cfdd885108eebb7ea76b3c2ce6070b18) )
	ROM_LOAD( "d2732.2h", 0xe000, 0x1000, CRC(1aab27c2) SHA1(66c7274dbb8bda3c78cc61d96a6cb1a9b29939b5) )
	ROM_LOAD( "d2732.2f", 0xf000, 0x1000, CRC(767bdd71) SHA1(334a903e05fc86186f90aa2d9ce3b0d367d7e516) )

	ROM_REGION( 0x8000, "soundbd:audiocpu", 0 )
	ROM_LOAD( "tms2516.10f", 0x5800, 0x0800, CRC(4ea1c3d9) SHA1(d0c99c9d5b887d717c68e8745906ae4e65aec6ad) ) // == vea_3a-3.3a
	ROM_LOAD( "tms2564.10j", 0x6000, 0x2000, CRC(da9d8588) SHA1(b2e6509748059fc317af56d66396427c5ca78748) ) // bigger ROM, but contents identical to venture5a

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "tms2516.6j", 0x0000, 0x0800, CRC(ea6fd981) SHA1(46b1658e1607423d5a073f14097c2a48d59057c0) ) // == vel_11d-2.11d

	ROM_REGION( 0x140, "proms", 0 ) // only 2 PROMs
	ROM_LOAD( "sn74s288n.6c", 0x0000, 0x0020, CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) )
	ROM_LOAD( "sn74s288n.3d", 0x0120, 0x0020, CRC(e26f9053) SHA1(eec35b6aa2c2d305418306bf4a1754a0583f109f) )
ROM_END


ROM_START( pepper2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p2l-8_12a.12a", 0x9000, 0x1000, CRC(33db4737) SHA1(d8f7a5d340ddbc4d06d403c3bff0102ce637d24e) )
	ROM_LOAD( "p2l-8_11a.11a", 0xa000, 0x1000, CRC(a1f43b1f) SHA1(a669f2ef55d9a0617110f65863822fdcaf153511) )
	ROM_LOAD( "p2l-8_10a.10a", 0xb000, 0x1000, CRC(4d7d7786) SHA1(ea1390b887404a67ea556720219e81007b954a7d) )
	ROM_LOAD( "p2l-8_9a.9a",   0xc000, 0x1000, CRC(b3362298) SHA1(7adad138ec5f94caa39f9c0fabece538d5db4913) )
	ROM_LOAD( "p2l-8_8a.8a",   0xd000, 0x1000, CRC(64d106ed) SHA1(49646a97def9e1793cac6ee0044f68232b294e4f) )
	ROM_LOAD( "p2l-8_7a.7a",   0xe000, 0x1000, CRC(b1c6f07c) SHA1(53d07211d014336bb43671c51f4190c6515e9cde) )
	ROM_LOAD( "p2l-8_6a.6a",   0xf000, 0x1000, CRC(515b1046) SHA1(bdcccd4e415c00ee8e5ec185597df75ecafe7d3d) )

	ROM_REGION( 0x8000, "soundbd:audiocpu", 0 )
	ROM_LOAD( "p2a-2_5a.5a", 0x6800, 0x0800, CRC(90e3c781) SHA1(d51a9e011167a132e8af9f4b1201600a58e86b62) )
	ROM_LOAD( "p2a-2_6a.6a", 0x7000, 0x0800, CRC(dd343e34) SHA1(4ec55bb73d6afbd167fa91d2606d1d55a15b5c39) )
	ROM_LOAD( "p2a-2_7a.7a", 0x7800, 0x0800, CRC(e02b4356) SHA1(9891e14d84221c1d6f2d15a29813eb41024290ca) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "p2l-1_11d.11d", 0x0000, 0x0800, CRC(b25160cd) SHA1(3d768552960a3a660891dcb85da6a5c382b33991) )

	// loaded, but not hooked up
	ROM_REGION( 0x140, "proms", 0 )
	ROM_LOAD( "hrl14h-1.h14", 0x0000, 0x0020, CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) ) // == HRL14H-1 from other Exidy games
	ROM_LOAD( "p2l5c-1.c5",   0x0020, 0x0100, CRC(e1e867ae) SHA1(fe4cb560860579102aedad2c81fd7bed5825f484) ) // == fxl-6b from FAX set
	ROM_LOAD( "p2l6d-1.d6",   0x0120, 0x0020, CRC(0da1bdf9) SHA1(0c2d85da59cf86f2d9cf5f33bdc63902ca5507d3) ) // == fxl-8b from FAX set
ROM_END

ROM_START( pepper27 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p2l-7_12a.12a", 0x9000, 0x1000, CRC(b3bc51cd) SHA1(12475ac5784bb2ab6887476ee8166c3585864cd6) )
	ROM_LOAD( "p2l-7_11a.11a", 0xa000, 0x1000, CRC(c8b834cd) SHA1(28b4de322de845effaa1d2fc6c9f129145965b8a) )
	ROM_LOAD( "p2l-7_10a.10a", 0xb000, 0x1000, CRC(c3e864a2) SHA1(cfc769b34d181724a5826d3a1bb3313ef5fbbd62) )
	ROM_LOAD( "p2l-7_9a.9a",   0xc000, 0x1000, CRC(451003b2) SHA1(87b9aecfcf861b3d812f0e3c23b40c98c198e933) )
	ROM_LOAD( "p2l-7_8a.8a",   0xd000, 0x1000, CRC(c666cafb) SHA1(5783fcfeeb651c850a1d9676e97a6beaafb06c6e) )
	ROM_LOAD( "p2l-7_7a.7a",   0xe000, 0x1000, CRC(ac1282ef) SHA1(34023d8a01c1f26ec8268d7387660d6f7e875014) )
	ROM_LOAD( "p2l-7_6a.6a",   0xf000, 0x1000, CRC(db8dd4fc) SHA1(9ae00f8d1a19280670dc65a20cf9cc4e7f1cc973) )

	ROM_REGION( 0x8000, "soundbd:audiocpu", 0 )
	ROM_LOAD( "p2a-2_5a.5a", 0x6800, 0x0800, CRC(90e3c781) SHA1(d51a9e011167a132e8af9f4b1201600a58e86b62) )
	ROM_LOAD( "p2a-2_6a.6a", 0x7000, 0x0800, CRC(dd343e34) SHA1(4ec55bb73d6afbd167fa91d2606d1d55a15b5c39) )
	ROM_LOAD( "p2a-2_7a.7a", 0x7800, 0x0800, CRC(e02b4356) SHA1(9891e14d84221c1d6f2d15a29813eb41024290ca) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "p2l-1_11d.11d", 0x0000, 0x0800, CRC(b25160cd) SHA1(3d768552960a3a660891dcb85da6a5c382b33991) )

	// loaded, but not hooked up
	ROM_REGION( 0x140, "proms", 0 )
	ROM_LOAD( "hrl14h-1.h14", 0x0000, 0x0020, CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) ) // == HRL14H-1 from other Exidy games
	ROM_LOAD( "p2l5c-1.c5",   0x0020, 0x0100, CRC(e1e867ae) SHA1(fe4cb560860579102aedad2c81fd7bed5825f484) ) // == fxl-6b from FAX set
	ROM_LOAD( "p2l6d-1.d6",   0x0120, 0x0020, CRC(0da1bdf9) SHA1(0c2d85da59cf86f2d9cf5f33bdc63902ca5507d3) ) // == fxl-8b from FAX set
ROM_END


ROM_START( hardhat )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "hhl-2.11a", 0xa000, 0x1000, CRC(7623deea) SHA1(3c47c0439c80e66536af42c5ee4e522fea5f8374) )
	ROM_LOAD( "hhl-2.10a", 0xb000, 0x1000, CRC(e6bf2fb1) SHA1(ad41859129774fc51462726a825c0ae16ed81a6e) )
	ROM_LOAD( "hhl-2.9a",  0xc000, 0x1000, CRC(acc2bce5) SHA1(0f7b8cfbd2628b8587c423fbc2c8310d71d8ad2a) )
	ROM_LOAD( "hhl-2.8a",  0xd000, 0x1000, CRC(23c7a2f8) SHA1(5eb1d512d73ba6bd1c23501664b582e9d3cf777f) )
	ROM_LOAD( "hhl-2.7a",  0xe000, 0x1000, CRC(6f7ce1c2) SHA1(356dcea22e50c95a8552566a0fb5f9b4e3e5de2a) )
	ROM_LOAD( "hhl-2.6a",  0xf000, 0x1000, CRC(2a20cf10) SHA1(31eb4556647e78e3d9be1c30d970eac8aaa5cf18) )

	ROM_REGION( 0x8000, "soundbd:audiocpu", 0 )
	ROM_LOAD( "hha-1.5a",  0x6800, 0x0800, CRC(16a5a183) SHA1(cf3fed55db9c61fd33c222275d472fa109bed081) )
	ROM_LOAD( "hha-1.6a",  0x7000, 0x0800, CRC(bde64021) SHA1(a403590d5a27b859eaa299e47df4ebd6ce4a5772) )
	ROM_LOAD( "hha-1.7a",  0x7800, 0x0800, CRC(505ee5d3) SHA1(efa228465688f2bb30f00dc1511cc5f3a287356c) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "hhl-1.11d", 0x0000, 0x0800, CRC(dbcdf353) SHA1(76ea287326a5c9e75e407cc010414212d8fdd52a) )
ROM_END


ROM_START( teetert )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "13a-cpu", 0x8000, 0x1000, CRC(f4e4d991) SHA1(6683c1552b56b20f2296e461aff697af73563792) )
	ROM_LOAD( "12a-cpu", 0x9000, 0x1000, CRC(c6d8cb04) SHA1(3b9ae8fdc35117c73c91daed66e93e5344bdcd7e) )
	ROM_LOAD( "11a-cpu", 0xa000, 0x1000, CRC(bac9b259) SHA1(0265cbd683fadf42f8a6b71958cbe782a732c257) )
	ROM_LOAD( "10a-cpu", 0xb000, 0x1000, CRC(3ae7e445) SHA1(e511ce4c553ac58e87b6ee623f8c42d7653de972) )
	ROM_LOAD( "9a-cpu",  0xc000, 0x1000, CRC(0cba424d) SHA1(54377163a8b8082639baf56b960eb26268462d46) )
	ROM_LOAD( "8a-cpu",  0xd000, 0x1000, CRC(68de66e7) SHA1(70a0cc950f16f2c408fae717e6fdb75eb0fd8039) )
	ROM_LOAD( "7a-cpu",  0xe000, 0x1000, CRC(84491333) SHA1(db9f8e4c49057a4574a3784d71e627da7f7a4b44) )
	ROM_LOAD( "6a-cpu",  0xf000, 0x1000, CRC(3600d465) SHA1(84d633e042f73bfd6bf4a4d0ffee1cd2027c65d2) )

	ROM_REGION( 0x8000, "soundbd:audiocpu", 0 )
	ROM_LOAD( "3a-ac",   0x5800, 0x0800, CRC(83b8836f) SHA1(ec0e2de62caea61ceff56e924449213997bff8cd) )
	ROM_LOAD( "4a-ac",   0x6000, 0x0800, CRC(5154c39e) SHA1(e6f011630eb1aa4116a0e5824ad6b65c1be2455f) )
	ROM_LOAD( "5a-ac",   0x6800, 0x0800, CRC(1e1e3916) SHA1(867e586583e07cd01e0e852f6ea52a040995725d) )
	ROM_LOAD( "6a-ac",   0x7000, 0x0800, CRC(80f3357a) SHA1(f1ee638251e8676a526e6367c11866b1d52f5910) )
	ROM_LOAD( "7a-ac",   0x7800, 0x0800, CRC(466addc7) SHA1(0230b5365d6aeee3ca47666a9eadee4141de125b) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "11d-cpu", 0x0000, 0x0800, CRC(0fe70b00) SHA1(6068be263d7a8e6b71af6f4dceec40bb8d246376) )

	ROM_REGION( 0x140, "proms", 0 )
	ROM_LOAD( "tt14h.123",  0x0000, 0x0020, CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) )
	ROM_LOAD( "tt5c.129",   0x0020, 0x0100, CRC(43b35bb7) SHA1(0a0cecea8faff9f3ff4c2ceda0b5b25e8e1cd667) )
	ROM_LOAD( "tt6d.123",   0x0120, 0x0020, CRC(e26f9053) SHA1(eec35b6aa2c2d305418306bf4a1754a0583f109f) )
ROM_END


ROM_START( fax )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "fxl8-13a.32", 0x08000, 0x1000, CRC(8e30bf6b) SHA1(1fdf010da0258bc038554cf33c26e539a1f6b648) )
	ROM_LOAD( "fxl8-12a.32", 0x09000, 0x1000, CRC(60a41ff1) SHA1(1703dbedd09354d89c6014644d0ffe13ec657b8b) )
	ROM_LOAD( "fxl8-11a.32", 0x0a000, 0x1000, CRC(2c9cee8a) SHA1(169045b4d840730cfbaa0b9a8a8d82907ea09d0c) )
	ROM_LOAD( "fxl8-10a.32", 0x0b000, 0x1000, CRC(9b03938f) SHA1(af4c27b06a1f1be917316910b88d026b67cc60c0) )
	ROM_LOAD( "fxl8-9a.32",  0x0c000, 0x1000, CRC(fb869f62) SHA1(cea6ff423c60662a1b36e9565940432707d5299b) )
	ROM_LOAD( "fxl8-8a.32",  0x0d000, 0x1000, CRC(db3470bc) SHA1(7786f84ab41765ea91ab241d14a207044eda0e93) )
	ROM_LOAD( "fxl8-7a.32",  0x0e000, 0x1000, CRC(1471fef5) SHA1(89308f3c2a0d7ea699e99622d37c5c95e3eaaf95) )
	ROM_LOAD( "fxl8-6a.32",  0x0f000, 0x1000, CRC(812e39f3) SHA1(41c99f8483c69617f9c8dd82f979630ea9190454) )
	/* Banks of question ROMs */
	ROM_LOAD( "fxd-1c.64",   0x10000, 0x2000, CRC(fd7e3137) SHA1(6fda53737cd7c886c66c60436ae3ed5c62e6b178) )
	ROM_LOAD( "fxd-2c.64",   0x12000, 0x2000, CRC(e78cb16f) SHA1(d58dfa2385368ccf00ecfbaeccaf5ba82ef7da9b) )
	ROM_LOAD( "fxd-3c.64",   0x14000, 0x2000, CRC(57a94c6f) SHA1(fc27fe805c4cc29f797bfc0e4cd13a570ac5c1ec) )
	ROM_LOAD( "fxd-4c.64",   0x16000, 0x2000, CRC(9036c5a2) SHA1(b7a01e4002f615702cb691764cfae93707bf3c0f) )
	ROM_LOAD( "fxd-5c.64",   0x18000, 0x2000, CRC(38c03405) SHA1(c490252825dc3c4bf91255c7cb70a5ead92de85b) )
	ROM_LOAD( "fxd-6c.64",   0x1a000, 0x2000, CRC(f48fc308) SHA1(bfaf43e57a4d92b593d51d8cd61fe4d5c06e836c) )
	ROM_LOAD( "fxd-7c.64",   0x1c000, 0x2000, CRC(cf93b924) SHA1(892e6e6aa33bbcd271f5e0a63c1e8393df62f360) )
	ROM_LOAD( "fxd-8c.64",   0x1e000, 0x2000, CRC(607b48da) SHA1(6c8f2f207f3dd936c529b86cef917a0f0699a21c) )
	ROM_LOAD( "fxd-1b.64",   0x20000, 0x2000, CRC(62872d4f) SHA1(c020fdeae6c2e7d04c16048fdaa99ecf3e40af31) )
	ROM_LOAD( "fxd-2b.64",   0x22000, 0x2000, CRC(625778d0) SHA1(6c8d6b50653bff3774f5ccef0e000a2ef3f7030c) )
	ROM_LOAD( "fxd-3b.64",   0x24000, 0x2000, CRC(c3473dee) SHA1(8675f9b93bbbae4f5a5682c5b1623afeeacc0a4b) )
	ROM_LOAD( "fxd-4b.64",   0x26000, 0x2000, CRC(e39a15f5) SHA1(43b04cc2e4750b649116ade5b1004c2580293134) )
	ROM_LOAD( "fxd-5b.64",   0x28000, 0x2000, CRC(101a9d70) SHA1(2b839cd707e03b0e50037e1ffabcb8fe375dc4c0) )
	ROM_LOAD( "fxd-6b.64",   0x2a000, 0x2000, CRC(374a8f05) SHA1(ec41470932823242fff36ab6e6f158fa5c07d0a8) )
	ROM_LOAD( "fxd-7b.64",   0x2c000, 0x2000, CRC(f7e7f824) SHA1(1bed1ee07032b25675ace612a883cba4ab4b2f77) )
	ROM_LOAD( "fxd-8b.64",   0x2e000, 0x2000, CRC(8f1a5287) SHA1(a1102d49bacb25887eaa67ae64bcf64c8cad94fe) )
	ROM_LOAD( "fxd-1a.64",   0x30000, 0x2000, CRC(fc5e6344) SHA1(c61aad5100819f2fe98c3a159b64739fa6322d09) )
	ROM_LOAD( "fxd-2a.64",   0x32000, 0x2000, CRC(43cf60b3) SHA1(5169196d0a95450801b3a57703cb9f2861a25948) )
	ROM_LOAD( "fxd-3a.64",   0x34000, 0x2000, CRC(6b7d29cb) SHA1(fd4006efd24b33f8e2baf7f97d4b776d5ef90959) )
	ROM_LOAD( "fxd-4a.64",   0x36000, 0x2000, CRC(b9de3c2d) SHA1(229f9f0762d4d659acf516c2c1a42e70d2f98652) )
	ROM_LOAD( "fxd-5a.64",   0x38000, 0x2000, CRC(67285bc6) SHA1(f929c916fb19dbc91fc3a75dfed6375b63cb2043) )
	ROM_LOAD( "fxd-6a.64",   0x3a000, 0x2000, CRC(ba67b7b2) SHA1(12265f678b1e4dfc3b36a964f78b0103112753ee) )
	/* The last two ROM sockets were apparently never populated */
//  ROM_LOAD( "fxd-7a.64",   0x3c000, 0x2000, NO_DUMP )
//  ROM_LOAD( "fxd-8a.64",   0x3e000, 0x2000, NO_DUMP )

	ROM_REGION( 0x8000, "soundbd:audiocpu", 0 )
	ROM_LOAD( "fxa2-5a.16",   0x6800, 0x0800, CRC(7c525aec) SHA1(f3afd3bfc0ba4265106e6ca217d113d23ad66016) )
	ROM_LOAD( "fxa2-6a.16",   0x7000, 0x0800, CRC(2b3bfc44) SHA1(7e3b9133916c8121b2145942155601b3ade420da) )
	ROM_LOAD( "fxa2-7a.16",   0x7800, 0x0800, CRC(578c62b7) SHA1(1bcb987e8730c001b7339c3dfab2467bf76421c7) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "fxl1-11d.32",  0x0000, 0x0800, CRC(62083db2) SHA1(0c6e90b73419bff53f991e66d4faa9495c7d8e09) )

	// loaded, but not hooked up
	ROM_REGION( 0x0240, "proms", 0 )
	ROM_LOAD( "fxl-6b",   0x0000, 0x0100, CRC(e1e867ae) SHA1(fe4cb560860579102aedad2c81fd7bed5825f484) )
	ROM_LOAD( "fxl-8b",   0x0100, 0x0020, CRC(0da1bdf9) SHA1(0c2d85da59cf86f2d9cf5f33bdc63902ca5507d3) )
	ROM_LOAD( "fxl-11b",  0x0120, 0x0020, CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) )
	ROM_LOAD( "fxl-12b",  0x0140, 0x0100, CRC(6b5aa3d7) SHA1(bfc4a6d01b977d55ad4dadc0123339343f1aa975) )
ROM_END


ROM_START( fax2 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "fxl8-13a.32",  0x08000, 0x1000, CRC(8e30bf6b) SHA1(1fdf010da0258bc038554cf33c26e539a1f6b648) )
	ROM_LOAD( "fxl8-12a.32",  0x09000, 0x1000, CRC(60a41ff1) SHA1(1703dbedd09354d89c6014644d0ffe13ec657b8b) )
	ROM_LOAD( "fxl8-11a.32",  0x0a000, 0x1000, CRC(2c9cee8a) SHA1(169045b4d840730cfbaa0b9a8a8d82907ea09d0c) )
	ROM_LOAD( "fxl8-10a.32",  0x0b000, 0x1000, CRC(9b03938f) SHA1(af4c27b06a1f1be917316910b88d026b67cc60c0) )
	ROM_LOAD( "fxl8-9a.32",   0x0c000, 0x1000, CRC(fb869f62) SHA1(cea6ff423c60662a1b36e9565940432707d5299b) )
	ROM_LOAD( "fxl8-8a.32",   0x0d000, 0x1000, CRC(db3470bc) SHA1(7786f84ab41765ea91ab241d14a207044eda0e93) )
	ROM_LOAD( "fxl8-7a.32",   0x0e000, 0x1000, CRC(1471fef5) SHA1(89308f3c2a0d7ea699e99622d37c5c95e3eaaf95) )
	ROM_LOAD( "fxl8-6a.32",   0x0f000, 0x1000, CRC(812e39f3) SHA1(41c99f8483c69617f9c8dd82f979630ea9190454) )
	/* Banks of question ROMs */
	ROM_LOAD( "fxdb1-1c.bin", 0x10000, 0x2000, CRC(0e42a2a4) SHA1(f7021aad36b49338cdaae8e13638dbdd12327afc) )
	ROM_LOAD( "fxdb1-2c.bin", 0x12000, 0x2000, CRC(cef8d49a) SHA1(a3005ab21add22ffb4c6f4cb9843db8964fef49c) )
	ROM_LOAD( "fxdb1-3c.bin", 0x14000, 0x2000, CRC(96217b39) SHA1(fe80255c8763e714fbbf8b2a7ec481712fd463e8) )
	ROM_LOAD( "fxdb1-4c.bin", 0x16000, 0x2000, CRC(9f1522d8) SHA1(e79ef8f4642245f4d1c4d0fe14eb432a432e6f8a) )
	ROM_LOAD( "fxdb1-5c.bin", 0x18000, 0x2000, CRC(4770eb04) SHA1(c8123ba4fd66da471099cd364615a196c4d1ea94) )
	ROM_LOAD( "fxdb1-6c.bin", 0x1a000, 0x2000, CRC(07c742ab) SHA1(c709be4a21cc946f2de9061948fe095ec511447e) )
	ROM_LOAD( "fxdb1-7c.bin", 0x1c000, 0x2000, CRC(f2f39ebb) SHA1(879f0954331d219d44e82d4cf13f84c1d03dd6d5) )
	ROM_LOAD( "fxdb1-8c.bin", 0x1e000, 0x2000, CRC(00f73e30) SHA1(586c28d1ea14626ccd0e9195f26f18168bd0cf72) )
	ROM_LOAD( "fxdb1-1b.bin", 0x20000, 0x2000, CRC(e13341cf) SHA1(5cc2c9f060436026dbcbc6f893f8ef17f10b6c75) )
	ROM_LOAD( "fxdb1-2b.bin", 0x22000, 0x2000, CRC(300d7a6f) SHA1(4e28a522e14e0cf1b63be35e2a16b816d51f328c) )
	ROM_LOAD( "fxdb1-3b.bin", 0x24000, 0x2000, CRC(db9a6a3a) SHA1(d192f8ece73dfa12cd390c6e0218d5bf58c46074) )
	ROM_LOAD( "fxdb1-4b.bin", 0x26000, 0x2000, CRC(47faeb43) SHA1(7991341d78b1d0c9e35c2472dbb05d21c001ba7f) )
	ROM_LOAD( "fxdb1-5b.bin", 0x28000, 0x2000, CRC(3ecf974f) SHA1(d18b7bdbb5e490b0d4b70baae06cb44cf0f2b643) )
	ROM_LOAD( "fxdb1-6b.bin", 0x2a000, 0x2000, CRC(526c4c0d) SHA1(68480fb61d8e9e5386daeb94f5854913ab36b48d) )
	ROM_LOAD( "fxdb1-7b.bin", 0x2c000, 0x2000, CRC(4cf8217c) SHA1(fd7b9c34b0ae532209453d68f30f6f9fbcec1964) )
	ROM_LOAD( "fxdb1-8b.bin", 0x2e000, 0x2000, CRC(99485e27) SHA1(1f10e71507f111724a8b496f3cc57a6790116ac0) )
	ROM_LOAD( "fxdb1-1a.bin", 0x30000, 0x2000, CRC(97c153b5) SHA1(0d5e66f0ded4453e1fe81cdcc56697664f76f28d) )
	ROM_LOAD( "fxdb1-2a.bin", 0x32000, 0x2000, CRC(63258140) SHA1(cdb03f5130c72d286d1a2f227dcf646da6e8f40a) )
	ROM_LOAD( "fxdb1-3a.bin", 0x34000, 0x2000, CRC(1c698727) SHA1(6886bbf04f837fbda9a192e55e995cac9c9e1cad) )
	ROM_LOAD( "fxdb1-4a.bin", 0x36000, 0x2000, CRC(8283d6fc) SHA1(dba1f0f53a6b16f2ee7ce1e8e6e081a6be7586f6) )
	ROM_LOAD( "fxdb1-5a.bin", 0x38000, 0x2000, CRC(b69542d2) SHA1(67a102790b24a5638c8938579ace0b3dd9b0c953) )
	ROM_LOAD( "fxdb1-6a.bin", 0x3a000, 0x2000, CRC(ff949367) SHA1(fb3d9c0abe3c915eaea983d9b429eb5227688532) )
	ROM_LOAD( "fxdb1-7a.bin", 0x3c000, 0x2000, CRC(0f97b874) SHA1(5790d3ed9eed2ce05947bc28cc252f720a7f3aeb) )
	ROM_LOAD( "fxdb1-8a.bin", 0x3e000, 0x2000, CRC(1d055bea) SHA1(96531db0a3a36319bc0a28096e601302eb2eb115) )

	ROM_REGION( 0x8000, "soundbd:audiocpu", 0 )
	ROM_LOAD( "fxa2-5a.16",    0x6800, 0x0800, CRC(7c525aec) SHA1(f3afd3bfc0ba4265106e6ca217d113d23ad66016) )
	ROM_LOAD( "fxa2-6a.16",    0x7000, 0x0800, CRC(2b3bfc44) SHA1(7e3b9133916c8121b2145942155601b3ade420da) )
	ROM_LOAD( "fxa2-7a.16",    0x7800, 0x0800, CRC(578c62b7) SHA1(1bcb987e8730c001b7339c3dfab2467bf76421c7) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "fxl1-11d.32",   0x0000, 0x0800, CRC(62083db2) SHA1(0c6e90b73419bff53f991e66d4faa9495c7d8e09) )

	// loaded, but not hooked up
	ROM_REGION( 0x0240, "proms", 0 )
	ROM_LOAD( "fxl-6b",   0x0000, 0x0100, CRC(e1e867ae) SHA1(fe4cb560860579102aedad2c81fd7bed5825f484) )
	ROM_LOAD( "fxl-8b",   0x0100, 0x0020, CRC(0da1bdf9) SHA1(0c2d85da59cf86f2d9cf5f33bdc63902ca5507d3) )
	ROM_LOAD( "fxl-11b",  0x0120, 0x0020, CRC(f76b4fcf) SHA1(197e0cc508ffeb5cefa4046bdfb158939d598225) )
	ROM_LOAD( "fxl-12b",  0x0140, 0x0100, CRC(6b5aa3d7) SHA1(bfc4a6d01b977d55ad4dadc0123339343f1aa975) )
ROM_END



/*************************************
 *
 *  Driver init
 *
 *************************************/

void spectar_state::init_sidetrac()
{
	// hard-coded palette controlled via 8x3 DIP switches on the board
	m_color_latch[2] = 0xf8;
	m_color_latch[1] = 0xdc;
	m_color_latch[0] = 0xb8;
}


void targ_state::init_targ()
{
	// hard-coded palette controlled via 8x3 jumpers on the color adapter board
	m_color_latch[2] = 0x54; // Red
	m_color_latch[1] = 0xee; // Green
	m_color_latch[0] = 0x6b; // Blue
}


void spectar_state::init_spectar()
{
	// hard-coded palette controlled via 8x3 jumpers on the color adapter board
	m_color_latch[2] = 0xda; // Red
	m_color_latch[1] = 0xee; // Green
	m_color_latch[0] = 0x61; // Blue
}

} // anonymous namespace



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1979, sidetrac,  0,       sidetrac, sidetrac,  spectar_state, init_sidetrac, ROT0, "Exidy",   "Side Trak", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // "Side Track" on title screen, but cabinet/flyers/documentation clearly indicates otherwise, "Side Trak" it is

GAME( 1980, targ,      0,       targ,     targ,      targ_state,    init_targ,     ROT0, "Exidy",   "Targ", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, targc,     targ,    targ,     targ,      targ_state,    init_targ,     ROT0, "Exidy",   "Targ (cocktail?)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1980, spectar,   0,       spectar,  spectar,   spectar_state, init_spectar,  ROT0, "Exidy",   "Spectar (revision 3)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, spectar1,  spectar, spectar,  spectar,   spectar_state, init_spectar,  ROT0, "Exidy",   "Spectar (revision 1?)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1984, spectarrf, spectar, spectar,  spectarrf, spectar_state, init_spectar,  ROT0, "bootleg (Recreativos Franco)", "Spectar (revision 2, bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, rallys,    spectar, rallys,   rallys,    spectar_state, init_spectar,  ROT0, "bootleg (Novar)", "Rallys (bootleg of Spectar, set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, rallysa,   spectar, rallys,   rallys,    spectar_state, init_spectar,  ROT0, "bootleg (Musik Box Brescia)", "Rallys (bootleg of Spectar, set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, panzer,    spectar, rallys,   rallys,    spectar_state, init_spectar,  ROT0, "bootleg (Proel)", "Panzer (bootleg of Spectar)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, phantoma,  spectar, phantoma, phantoma,  spectar_state, init_spectar,  ROT0, "bootleg (Jeutel)", "Phantomas (bootleg of Spectar)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1980, phantom,   spectar, phantoma, phantoma,  spectar_state, init_spectar,  ROT0, "bootleg (Proel)", "Phantom (bootleg of Spectar)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

GAME( 1981, mtrap,     0,       mtrap,    mtrap,     exidy_state,   empty_init,    ROT0, "Exidy",   "Mouse Trap (version 5)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, mtrap4,    mtrap,   mtrap,    mtrap,     exidy_state,   empty_init,    ROT0, "Exidy",   "Mouse Trap (version 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, mtrap4g,   mtrap,   mtrap,    mtrap,     exidy_state,   empty_init,    ROT0, "Exidy",   "Mouse Trap (German, version 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, mtrap3,    mtrap,   mtrap,    mtrap,     exidy_state,   empty_init,    ROT0, "Exidy",   "Mouse Trap (version 3)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, mtrap2,    mtrap,   mtrap,    mtrap,     exidy_state,   empty_init,    ROT0, "Exidy",   "Mouse Trap (version 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, mtrapb,    mtrap,   mtrap,    mtrap,     exidy_state,   empty_init,    ROT0, "bootleg", "Mouse Trap (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, mtrapb2,   mtrap,   mtrap,    mtrap,     exidy_state,   empty_init,    ROT0, "bootleg", "Mouse Trap (version 4, bootleg)", MACHINE_SUPPORTS_SAVE )

GAME( 1981, venture,   0,       venture,  venture,   exidy_state,   empty_init,    ROT0, "Exidy",   "Venture (version 5 set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, venture5a, venture, venture,  venture,   exidy_state,   empty_init,    ROT0, "Exidy",   "Venture (version 5 set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, venture4,  venture, venture,  venture,   exidy_state,   empty_init,    ROT0, "Exidy",   "Venture (version 4)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, venture5b, venture, venture,  venture,   exidy_state,   empty_init,    ROT0, "bootleg", "Venture (version 5 set 2, bootleg)", MACHINE_SUPPORTS_SAVE )

GAME( 1982, teetert,   0,       teetert,  teetert,   teetert_state, empty_init,    ROT0, "Exidy",   "Teeter Torture (prototype)", MACHINE_SUPPORTS_SAVE )

GAME( 1982, pepper2,   0,       pepper2,  pepper2,   exidy_state,   empty_init,    ROT0, "Exidy",   "Pepper II (version 8)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, pepper27,  pepper2, pepper2,  pepper2,   exidy_state,   empty_init,    ROT0, "Exidy",   "Pepper II (version 7)", MACHINE_SUPPORTS_SAVE )

GAME( 1982, hardhat,   0,       pepper2,  pepper2,   exidy_state,   empty_init,    ROT0, "Exidy",   "Hard Hat", MACHINE_SUPPORTS_SAVE )

GAME( 1983, fax,       0,       fax,      fax,       fax_state,     empty_init,    ROT0, "Exidy",   "FAX", MACHINE_SUPPORTS_SAVE )
GAME( 1983, fax2,      fax,     fax,      fax,       fax_state,     empty_init,    ROT0, "Exidy",   "FAX 2", MACHINE_SUPPORTS_SAVE )
