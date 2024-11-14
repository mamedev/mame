// license:GPL-2.0+
// copyright-holders:Jarek Burczynski, Angelo Salese
/****************************************************************************

Mazer Blazer by Stern Electronics (c) 1983
Great Guns by Stern Electronics (c) 1983


Driver by Jarek Burczynski
Added notes by Lord Nightmare
2003.03.19

Notes:
======
Mazer blazer consists of four boards in a cage:
ZPU-2000 - main cpu board (Zentral (sic) Processor Unit)
 - this board has the main cpu on it and four roms (seven sockets for roms, three empty)
   - roms in sockets "ROM0"@H1 "ROM1"@H2 "ROM2"@H3 "ROM3"@H4
   - "ROM4"@H5 "ROM5"@H6 "ROM6"@H7 are empty
   - roms are marked "MAZER BLAZER // ZPU <rom#> RA<revision#> // 1983 STERN"
 - 32 dipswitches in 4 banks of 8
 - four 'test button' style switches
 - one 4Mhz xtal @A1
 - one PAL16R8? @7D (UNDUMPED, if present)
 - this same board is shared with cliff hanger (cliffhgr.c)

CFB-1000 - video/subcpu board (Color Frame Board)
 - this board has a sub-cpu on it and four roms (six sockets for roms, two empty)
   - the roms go in sockets "ROM0"@G8, "ROM2"@K8, "ROM3"@K10, "ROM4"@K11
   - "ROM1"@G6 and "ROM5"@K12 are empty
   - roms are marked "MAZER BLAZER // CFB <rom#> RA<revision#> // 1983 STERN"
 - "shared ram" ?6116? SRAM @G3
 - DIP64 custom framebuffer controller "Video Controller"@E11
 - "Parameter ram" ?6116? SRAM @K13
 - "Frame buffer" 16x ?4116? DRAM @ right edge of pcb
 - "Erase PROM" 82S137 or MMI6353-1 @A16 (UNDUMPED)
 - 22.1164Mhz xtal @K14
 - 8 dipswitches in 2 banks of 4, @B5 and @B7
 - LED @B7

VSB-2000 - sound/speech/subcpu board (Voice and Sound Board)
 - this board has a sub-cpu on it, a digitalker speech chip, 4 roms (2 dumped, 2 undumped) and 4 PROMs (all dumped)
 - Z80 CPU @E6
   - Roms at "ROM0"@D2, "ROM1"@D4
 - MM54104 Digitalker "SPU" @A6
   - Roms at "ROM2"@A2, "ROM3"@A4 (UNDUMPED)
 - roms are marked "MAZER BLAZER // VSB <rom#> RA<revision#> // 1983 STERN"
 - Sound section
   - PROMS: 82s123: @B8 @B9; 82s129: @G8 @G9 (all dumped)

CRF-1001 - RF Filter board for video/audio output
 - this same board is shared with cliff hanger (cliffhgr.c)

UIB-1000 - coin inputs, start/fire buttons and gun ADCs (Vin(-) and Vref/2 are calibrated by potentiometers)

Versions:
======
Mazer blazer's zpu-2000 roms are known to exist in at least the following versions:
RA3
and probably exist in versions RA1 and RA2 as well.
It is currently unknown what versions the two sets in MAME correspond to.
The other roms are likely always version RA1, as the RA3-zpu-2000 board has RA1
roms for all roms except the zpu-2000 board.

TO DO:
=====
- fix remaining issues in mb_vcu device.

- figure out what really should happen during VCU test in Great Guns (patched
  out at the moment) (btw. Mazer Blazer doesn't test VCU)

- add sound interface to Mazer Blazer - Speech processor is Digitalker chip, sample ROMs are currently
  undumped;

============================================================================

Mazer Blazer DASM notes (reference only):
master z80
[0x0000]: clear 0x4c-0x4f i/o ports (ls670)
[0x0792]: z80 regs check
[0x0924]: z80 SP reg check
(following two happens quite often, basically after every POST test)
    [0x08e1]: writes 0xaa to led i/o port
    [0x08d2]: writes 0x02 to A, then 0 to led i/o port
[0x07d7]: checks ROM 0x0000-0x1fff
[0x07ee]: checks ROM 0x2000-0x3fff
[0x0805]: checks ROM 0x4000-0x5fff
[0x081c]: checks ROM 0x6000-0x7fff
[0x0833]: checks RAM 0xe800-0xefff, with a bit-wise pattern (1-2-4-8-0x10-0x20-0x40-0x80)
[0x0844]: transfers RAM 0xe000-0xe7ff to RAM 0xe800-0xefff
[0x0850]: checks RAM 0xe000-0xe7ff, with a bit-wise pattern (1-2-4-8-0x10-0x20-0x40-0x80)
[0x085a]: transfers RAM 0xe800-0xefff to RAM 0xe000-0xe7ff
[0x086b]: Puts some values to ls670 ports, reads back afterwards, does this twice (with different values)
[0x088b]: shared RAM check, values at 0xc000-0xc7ff must be 0x55 (otherwise wait until they are)
[0x089c]: writes 0xaa to shared RAM 0xc000-0xc7ff
[0x08ab]: shared RAM check, values at 0xc000-0xc7ff must be 0x00 (otherwise wait until they are)
[0x08c2]: writes 0 to shared RAM
[0x000d]: clears RAM 0xe0fb, 0xe0fd, 0xe0fe (word)
    [0x2138]: puts a 1 to 0xe0fb
[0x0021]: clears i/o at 0x6a (lamps), clears 0xe563, 0xe004, 0xe000, 0xe001, 0xe007, 0xe002, 0xe003, 0xe005,
          0xc04f, 0xe572, enables IM 2, clears 0xe581 / 0xe583 (word), puts default initials (0x2274->0xe058)

video z80
[0x535]:
[0x03c]: start of proper code

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/adc0804.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/rescap.h"
#include "sound/ay8910.h"
#include "video/resnet.h"
#include "video/mb_vcu.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"



namespace {

#define MASTER_CLOCK XTAL(4'000'000)
#define SOUND_CLOCK XTAL(14'318'181)


class mazerbla_state : public driver_device
{
public:
	mazerbla_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "sub")
		, m_vcu(*this,"vcu")
		, m_screen(*this, "screen")
		, m_soundlatch(*this, "soundlatch")
		, m_uib_adc(*this, "adc%u", 0U)
		, m_digital_inputs(*this, { "ZPU", "DSW0", "DSW1", "DSW2", "DSW3", "BUTTONS" })
		, m_leds(*this, "led%u", 0U)
		, m_lamps(*this, "lamp%u", 0U)
	{ }

	void greatgun(machine_config &config);
	void mazerbla(machine_config &config);

	void init_mazerbla();
	void init_greatgun();

private:
	void cfb_rom_bank_sel_w(uint8_t data);
	void cfb_zpu_int_req_set_w(uint8_t data);
	uint8_t cfb_zpu_int_req_clr();
	uint8_t ls670_0_r(offs_t offset);
	void ls670_0_w(offs_t offset, uint8_t data);
	uint8_t ls670_1_r(offs_t offset);
	void ls670_1_w(offs_t offset, uint8_t data);
	void zpu_bcd_decoder_w(uint8_t data);
	uint8_t zpu_inputs_r();
	void zpu_led_w(offs_t offset, uint8_t data);
	void zpu_lamps_w(uint8_t data);
	void zpu_coin_counter_w(offs_t offset, uint8_t data);
	void cfb_led_w(uint8_t data);
	void vsb_ls273_audio_control_w(uint8_t data);
	void sound_int_clear_w(uint8_t data);
	void gg_led_ctrl_w(uint8_t data);
	void mazerbla_palette(palette_device &palette);
	uint32_t screen_update_mazerbla(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	INTERRUPT_GEN_MEMBER(sound_interrupt);
	TIMER_CALLBACK_MEMBER(deferred_ls670_0_w);
	TIMER_CALLBACK_MEMBER(deferred_ls670_1_w);
	IRQ_CALLBACK_MEMBER(irq_callback);
	void greatgun_cpu3_io_map(address_map &map) ATTR_COLD;
	void greatgun_io_map(address_map &map) ATTR_COLD;
	void greatgun_sound_map(address_map &map) ATTR_COLD;
	void mazerbla_cpu2_io_map(address_map &map) ATTR_COLD;
	void mazerbla_cpu2_map(address_map &map) ATTR_COLD;
	void mazerbla_cpu3_io_map(address_map &map) ATTR_COLD;
	void mazerbla_cpu3_map(address_map &map) ATTR_COLD;
	void mazerbla_io_map(address_map &map) ATTR_COLD;
	void mazerbla_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<mb_vcu_device> m_vcu;
	required_device<screen_device> m_screen;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_device_array<adc0804_device, 4> m_uib_adc;
	required_ioport_array<6> m_digital_inputs;
	output_finder<3> m_leds;
	output_finder<2> m_lamps;

	uint8_t m_port02_status = 0;
	uint32_t m_gfx_rom_bank = 0;  /* graphics ROMs are banked */

	double m_weights_r[2];
	double m_weights_g[3];
	double m_weights_b[3];

	/* misc */
	uint8_t m_ls670_0[4];
	uint8_t m_ls670_1[4];

	uint8_t m_zpu_int_vector = 0;

	uint8_t m_bcd_7445 = 0;

	uint8_t m_vsb_ls273 = 0;
};



/***************************************************************************

  Convert the color PROMs into a more useable format.


  bit 0 -- 10.0Kohm resistor--\
  bit 1 -- 4.7 Kohm resistor --+-- 3.6 Kohm pulldown resistor -- BLUE
  bit 2 -- 2.2 Kohm resistor --/

  bit 3 -- 10.0Kohm resistor--\
  bit 4 -- 4.7 Kohm resistor --+-- 3.6 Kohm pulldown resistor -- GREEN
  bit 5 -- 2.2 Kohm resistor --/

  bit 6 -- 4.7 Kohm resistor --+-- 3.6 Kohm pulldown resistor -- RED
  bit 7 -- 2.2 Kohm resistor --/

***************************************************************************/

void mazerbla_state::mazerbla_palette(palette_device &palette)
{
	static constexpr int resistances_r[2]  = { 4700, 2200 };
	static constexpr int resistances_gb[3] = { 10000, 4700, 2200 };

	// just to calculate coefficients for later use
	compute_resistor_weights(0, 255,    -1.0,
			3,  resistances_gb, m_weights_g,    3600,   0,
			3,  resistances_gb, m_weights_b,    3600,   0,
			2,  resistances_r,  m_weights_r,    3600,   0);
}

void mazerbla_state::video_start()
{
	// ...
}


uint32_t mazerbla_state::screen_update_mazerbla(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_vcu->screen_update(screen,bitmap,cliprect);
	return 0;
}

void mazerbla_state::screen_vblank(int state)
{
	if (state)
	{
		m_vcu->screen_eof();
	}
}

void mazerbla_state::cfb_rom_bank_sel_w(uint8_t data)
{
	m_gfx_rom_bank = data;

	membank("bank1")->set_entry(m_gfx_rom_bank);
}

/*************************************
 *
 *  IRQ & Timer handlers
 *
 *************************************/

void mazerbla_state::cfb_zpu_int_req_set_w(uint8_t data)
{
	m_zpu_int_vector &= ~2; /* clear D1 on INTA (interrupt acknowledge) */

	m_maincpu->set_input_line(0, ASSERT_LINE);  /* main cpu interrupt (comes from CFB (generated at the start of INT routine on CFB) - vblank?) */
}

uint8_t mazerbla_state::cfb_zpu_int_req_clr()
{
	// this clears all interrupts
	m_zpu_int_vector = 0xff;
	m_maincpu->set_input_line(0, CLEAR_LINE);

	return 0;
}

uint8_t mazerbla_state::ls670_0_r(offs_t offset)
{
	/* set a timer to force synchronization after the read */
	machine().scheduler().synchronize();

	return m_ls670_0[offset];
}

TIMER_CALLBACK_MEMBER(mazerbla_state::deferred_ls670_0_w)
{
	int offset = (param >> 8) & 255;
	int data = param & 255;

	m_ls670_0[offset] = data;
}

void mazerbla_state::ls670_0_w(offs_t offset, uint8_t data)
{
	/* do this on a timer to let the CPUs synchronize */
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(mazerbla_state::deferred_ls670_0_w),this), (offset << 8) | data);
}

uint8_t mazerbla_state::ls670_1_r(offs_t offset)
{
	/* set a timer to force synchronization after the read */
	machine().scheduler().synchronize();

	return m_ls670_1[offset];
}

TIMER_CALLBACK_MEMBER(mazerbla_state::deferred_ls670_1_w)
{
	int offset = (param >> 8) & 255;
	int data = param & 255;

	m_ls670_1[offset] = data;
}

void mazerbla_state::ls670_1_w(offs_t offset, uint8_t data)
{
	/* do this on a timer to let the CPUs synchronize */
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(mazerbla_state::deferred_ls670_1_w),this), (offset << 8) | data);
}


/*************************************
 *
 *  I/O
 *
 *************************************/

/*
name:           Strobe(bcd_value)   BIT
---------------------------------------
ZPU switch 1    0                   6
ZPU switch 2    0                   7

dipsw 35        1                   7
dipsw 34        1                   6
dipsw 33        1                   5
dipsw 32        1                   4
dipsw 31        1                   3
dipsw 30        1                   2
dipsw 29        1                   1
dipsw 28        1                   0
dipsw 27        2                   7
dipsw 26        2                   6
...
dipsw 8         4                   4
dipsw 7         4                   3
dipsw 6         4                   2
dipsw 5         4                   1
dipsw 4         4                   0

Right Coin Sw.  5                   0
Left Coin Sw.   5                   1
Player One      5                   2
Player Two      5                   3
Fire Button     5                   4

Horizontal movement of gun is Strobe 6, Bits 0-7.
    Movement is from 0000 0000 to 1111 1111

Vertical movement of gun is Strobe 7, Bits 0-7.
    Movement is from 0000 0000 to 1111 1111


Great Guns has two guns and here is necessary support for second gun:

Horizontal movement of gun is Strobe 8, Bits 0-7.
    Movement is from 0000 0000 to 1111 1111

Vertical movement of gun is Strobe 9, Bits 0-7.
    Movement is from 0000 0000 to 1111 1111

*/

void mazerbla_state::zpu_bcd_decoder_w(uint8_t data)
{
	/* bcd decoder used a input select (a mux) for reads from port 0x62 */
	if (m_bcd_7445 != (data & 0xf))
	{
		if (m_bcd_7445 >= 6 && m_bcd_7445 < 10 && m_uib_adc[m_bcd_7445 - 6].found())
			m_uib_adc[m_bcd_7445 - 6]->wr_w(1);
		m_bcd_7445 = data & 0xf;
		if (m_bcd_7445 >= 6 && m_bcd_7445 < 10 && m_uib_adc[m_bcd_7445 - 6].found())
			m_uib_adc[m_bcd_7445 - 6]->wr_w(0);
	}
}

uint8_t mazerbla_state::zpu_inputs_r()
{
	uint8_t ret = 0xff;

	if (m_bcd_7445 < 6)
		ret = m_digital_inputs[m_bcd_7445]->read();
	else if (m_bcd_7445 < 10 && m_uib_adc[m_bcd_7445 - 6].found())
		ret = m_uib_adc[m_bcd_7445 - 6]->read();

	return ret;
}

void mazerbla_state::zpu_led_w(offs_t offset, uint8_t data)
{
	/* 0x6e - reset (offset = 0)*/
	/* 0x6f - set */
	m_leds[0] = BIT(offset, 0);
}

void mazerbla_state::zpu_lamps_w(uint8_t data)
{
	/* bit 4 = /LAMP0 */
	/* bit 5 = /LAMP1 */

	/*m_lamps[0] = BIT(data, 4);*/
	/*m_lamps[1] = BIT(data, 5);*/
}

void mazerbla_state::zpu_coin_counter_w(offs_t offset, uint8_t data)
{
	/* bit 6 = coin counter */
	machine().bookkeeping().coin_counter_w(offset, BIT(data, 6));
}

void mazerbla_state::cfb_led_w(uint8_t data)
{
	/* bit 7 - led on */
	m_leds[2] = BIT(data, 7);
}

void mazerbla_state::gg_led_ctrl_w(uint8_t data)
{
	/* bit 0, bit 1 - led on */
	m_leds[1] = BIT(data, 0);
}


/*************************************
 *
 *  Sound comms
 *
 *************************************/

void mazerbla_state::vsb_ls273_audio_control_w(uint8_t data)
{
	m_vsb_ls273 = data;

	/* bit 5 - led on */
	m_leds[1] = BIT(data, 5);
}

void mazerbla_state::sound_int_clear_w(uint8_t data)
{
	m_subcpu->set_input_line(0, CLEAR_LINE);
}


/*************************************
 *
 *  Memory Maps (Mazer Blazer)
 *
 *************************************/

void mazerbla_state::mazerbla_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc7ff).ram().share("share1");
	map(0xd800, 0xd800).r(FUNC(mazerbla_state::cfb_zpu_int_req_clr));
	map(0xe000, 0xefff).ram().share("nvram");
}

void mazerbla_state::mazerbla_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x4c, 0x4f).rw(FUNC(mazerbla_state::ls670_1_r), FUNC(mazerbla_state::ls670_0_w));
	map(0x60, 0x60).w(FUNC(mazerbla_state::zpu_bcd_decoder_w)); // r(FUNC(mazerbla_state::) from protection pal, if populated
	map(0x62, 0x62).r(FUNC(mazerbla_state::zpu_inputs_r));
	// 64 is some sort of output latch, unpopulated?
	// 66 is some sort of output latch, unpopulated?
	map(0x68, 0x68).w(FUNC(mazerbla_state::zpu_coin_counter_w));
	map(0x6a, 0x6a).w(FUNC(mazerbla_state::zpu_lamps_w));
	// 6c RW is a 6850 acia for communication with another cabinet or debug console? unpopulated?
	map(0x6e, 0x6f).w(FUNC(mazerbla_state::zpu_led_w));
}

void mazerbla_state::mazerbla_cpu2_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x43ff).ram(); /* main RAM (stack) */
	map(0x8000, 0x83ff).ram(); /* waveform ???*/
	map(0xc000, 0xc003).nopw();
}

void mazerbla_state::mazerbla_cpu2_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(mazerbla_state::vsb_ls273_audio_control_w));
	map(0x40, 0x41).nopw();
	map(0x80, 0x83).rw(FUNC(mazerbla_state::ls670_0_r), FUNC(mazerbla_state::ls670_1_w));
}

void mazerbla_state::mazerbla_cpu3_map(address_map &map)
{
	map(0x0000, 0x37ff).rom();
	map(0x3800, 0x3fff).ram().share("share1");
	map(0x4000, 0x5fff).bankr("bank1");                    /* GFX roms */
	map(0x4000, 0x4003).w(m_vcu, FUNC(mb_vcu_device::write_vregs));
	map(0x6000, 0x67ff).rw(m_vcu, FUNC(mb_vcu_device::read_ram), FUNC(mb_vcu_device::write_ram));
	map(0xa000, 0xa7ff).r(m_vcu, FUNC(mb_vcu_device::load_params));
	map(0xc000, 0xdfff).r(m_vcu, FUNC(mb_vcu_device::load_gfx));
	map(0xe000, 0xffff).r(m_vcu, FUNC(mb_vcu_device::load_set_clr));
}

void mazerbla_state::mazerbla_cpu3_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).w(m_vcu, FUNC(mb_vcu_device::background_color_w));
	map(0x02, 0x02).r(m_vcu, FUNC(mb_vcu_device::status_r)).w(FUNC(mazerbla_state::cfb_led_w));
	map(0x03, 0x03).w(FUNC(mazerbla_state::cfb_zpu_int_req_set_w));
	map(0x04, 0x04).w(FUNC(mazerbla_state::cfb_rom_bank_sel_w));
	map(0x05, 0x05).w(m_vcu, FUNC(mb_vcu_device::vbank_w));
}


/*************************************
 *
 *  Memory Maps (Great Guns)
 *
 *************************************/

void mazerbla_state::greatgun_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x4c, 0x4c).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x60, 0x60).w(FUNC(mazerbla_state::zpu_bcd_decoder_w));
	map(0x62, 0x62).r(FUNC(mazerbla_state::zpu_inputs_r));
	map(0x66, 0x66).nopw();
	map(0x68, 0x68).nopw();
	map(0x6e, 0x6f).w(FUNC(mazerbla_state::zpu_led_w));
}

void mazerbla_state::greatgun_sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x27ff).ram();
	map(0x4000, 0x4000).r("ay1", FUNC(ay8910_device::data_r));
	map(0x4000, 0x4001).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x6000, 0x6001).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0x8000, 0x8000).w(FUNC(mazerbla_state::sound_int_clear_w));
	map(0xa000, 0xa000).w(m_soundlatch, FUNC(generic_latch_8_device::acknowledge_w));
}

void mazerbla_state::greatgun_cpu3_io_map(address_map &map)
{
	map.global_mask(0xff);
	mazerbla_cpu3_io_map(map);
	map(0x05, 0x05).w(m_vcu, FUNC(mb_vcu_device::vbank_clear_w));
}


/*************************************
 *
 *  Input Port Definitions
 *
 *************************************/

static INPUT_PORTS_START( mazerbla )
	PORT_START("ZPU")   /* Strobe 0: ZPU Switches */
	PORT_DIPNAME( 0x40, 0x40, "ZPU Switch 1" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "ZPU Switch 2" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW0")  /* Strobe 1: Dip Switches 28-35*/
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x0c, 0x0c, "Freeze Time" )
	PORT_DIPSETTING(    0x0c, "2.0 seconds" )
	PORT_DIPSETTING(    0x08, "2.5 seconds" )
	PORT_DIPSETTING(    0x04, "3.0 seconds" )
	PORT_DIPSETTING(    0x00, "3.5 seconds" )
	PORT_DIPNAME( 0x70, 0x40, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x70, "10000" )
	PORT_DIPSETTING(    0x60, "15000" )
	PORT_DIPSETTING(    0x50, "20000" )
	PORT_DIPSETTING(    0x40, "25000" )
	PORT_DIPSETTING(    0x30, "30000" )
	PORT_DIPSETTING(    0x20, "35000" )
	PORT_DIPSETTING(    0x10, "40000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  //probably unused
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")  /* Strobe 2: Dip Switches 20-27*/
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_7C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x08, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin/14 Credits" )

	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_7C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x80, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x70, "1 Coin/14 Credits" )

	PORT_START("DSW2")  /* Strobe 3: Dip Switches 12-19*/
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW ) PORT_NAME("Service Index")
	PORT_DIPNAME( 0x02, 0x02, "Switch Test" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Player Immortality" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Super Shot" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  //probably unused
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  //probably unused
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  /* Strobe 4: Dip Switches 4-11 */
	PORT_DIPNAME( 0x03, 0x02, "Number of Freezes" )
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, "Gun Knocker" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	//dips 7-11 - not listed in manual
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  //probably unused
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  //probably unused
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  //probably unused
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  //probably unused
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  //probably unused
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BUTTONS")   /* Strobe 5: coin1&2, start1&2, fire */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("STICK0_X")  /* Strobe 6: horizontal movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("STICK0_Y")  /* Strobe 7: vertical movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( mazerblaa )
	PORT_INCLUDE( mazerbla )

	PORT_MODIFY("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x0c, 0x0c, "Freeze Time" )
	PORT_DIPSETTING(    0x0c, "1.5 seconds" )
	PORT_DIPSETTING(    0x08, "2.0 seconds" )
	PORT_DIPSETTING(    0x04, "2.5 seconds" )
	PORT_DIPSETTING(    0x00, "3.0 seconds" )
	PORT_DIPNAME( 0x30, 0x00, "Number of points for extra frezze & first life" )
	PORT_DIPSETTING(    0x30, "20000" )
	PORT_DIPSETTING(    0x20, "25000" )
	PORT_DIPSETTING(    0x10, "30000" )
	PORT_DIPSETTING(    0x00, "35000" )
	PORT_DIPNAME( 0xc0, 0x00, "Number of points for extra life other than first" )
	PORT_DIPSETTING(    0xc0, "40000" )
	PORT_DIPSETTING(    0x80, "50000" )
	PORT_DIPSETTING(    0x40, "60000" )
	PORT_DIPSETTING(    0x00, "70000" )
INPUT_PORTS_END

// TODO: defaults, not listed in manual
static INPUT_PORTS_START( greatgun )
	PORT_START("ZPU")   /* Strobe 0: ZPU Switches */
	PORT_DIPNAME( 0x40, 0x40, "ZPU Switch 1" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "ZPU Switch 2" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW0")  /* Strobe 1: Dip Switches 28-35*/
	PORT_DIPNAME( 0x03, 0x03, "Starting Number of Bullets/Credit" )
	PORT_DIPSETTING(    0x03, "60" )
	PORT_DIPSETTING(    0x02, "70" )
	PORT_DIPSETTING(    0x01, "80" )
	PORT_DIPSETTING(    0x00, "90" )
	PORT_DIPNAME( 0x0c, 0x0c, "Target Size" )
	PORT_DIPSETTING(    0x0c, "7 x 7" )
	PORT_DIPSETTING(    0x08, "9 x 9" )
	PORT_DIPSETTING(    0x04, "11x11" )
	PORT_DIPSETTING(    0x00, "7 x 7" )
	PORT_DIPNAME( 0x70, 0x70, "Number of points for extra bullet" )
	PORT_DIPSETTING(    0x70, "1000" )
	PORT_DIPSETTING(    0x60, "2000" )
	PORT_DIPSETTING(    0x50, "3000" )
	PORT_DIPSETTING(    0x40, "4000" )
	PORT_DIPSETTING(    0x30, "5000" )
	PORT_DIPSETTING(    0x20, "6000" )
	PORT_DIPSETTING(    0x10, "7000" )
	PORT_DIPSETTING(    0x00, "8000" )
	/* from manual:
	    "This switch is used when an optional coin return or ticket dispenser is used"
	*/
	PORT_DIPNAME( 0x80, 0x80, "Number of coins or tickets returned" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "2" )

	PORT_START("DSW1")  /* Strobe 2: Dip Switches 20-27*/
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_7C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x08, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x07, "1 Coin/14 Credits" )

	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_7C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x80, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x70, "1 Coin/14 Credits" )

	PORT_START("DSW2")  /* Strobe 3: Dip Switches 12-19*/
	PORT_DIPNAME( 0x01, 0x01, "Service Index" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Switch Test" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Player Immortality" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Rack Advance" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  //probably unused
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  //probably unused
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")  /* Strobe 4: Dip Switches 4-11 */
	PORT_DIPNAME( 0x01, 0x01, "Free game/coin return" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	//dips 5-11 - not listed in manual
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

	PORT_START("BUTTONS")   /* Strobe 5: coin1&2, start1&2, fire */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Fire")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Fire")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("STICK0_X")  /* Strobe 6: horizontal movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_MINMAX(0x00, 0xff) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_PLAYER(1)

	PORT_START("STICK0_Y")  /* Strobe 7: vertical movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_MINMAX(0x00, 0xff) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_PLAYER(1)

	PORT_START("STICK1_X")  /* Strobe 8: horizontal movement of gun */
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_MINMAX(0x00, 0xff) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_KEYDELTA(7) PORT_PLAYER(2)

	PORT_START("STICK1_Y")  /* Strobe 9: vertical movement of gun */
	// for whatever reason this should be inverted?
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_MINMAX(0x00, 0xff) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(25) PORT_INVERT PORT_KEYDELTA(7) PORT_PLAYER(2)
INPUT_PORTS_END

/*************************************
 *
 *  Sound HW Configs
 *
 *************************************/

IRQ_CALLBACK_MEMBER(mazerbla_state::irq_callback)
{
	/* all data lines are tied to +5V via 10K resistors */
	/* D1 is set to GND when INT comes from CFB */
	/* D2 is set to GND when INT comes from ZPU board - from 6850 on schematics (RS232 controller) */

	/* resulting vectors:    - effect according to disasm:
	--------------------------------------------------------
	1111 11000 (0xf8)        - results in same as 0xfc
	1111 11010 (0xfa)        - does nothing, assume it was used for debugging
	1111 11100 (0xfc)        - calls several routines

	note:
	1111 11110 (0xfe) - cannot happen and is not handled by game */

	return (m_zpu_int_vector & ~1);  /* D0->GND is performed on CFB board */
}

/* frequency is 14.318 MHz/16/16/16/16 */
INTERRUPT_GEN_MEMBER(mazerbla_state::sound_interrupt)
{
	device.execute().set_input_line(0, ASSERT_LINE);
}


/*************************************
 *
 *  Machine driver definitions
 *
 *************************************/

void mazerbla_state::machine_start()
{
	m_leds.resolve();
	m_lamps.resolve();

	membank("bank1")->configure_entries(0, 256, memregion("sub2")->base() + 0x10000, 0x2000);

	save_item(NAME(m_port02_status));
	save_item(NAME(m_gfx_rom_bank));

	save_item(NAME(m_ls670_0));
	save_item(NAME(m_ls670_1));

	save_item(NAME(m_zpu_int_vector));

	save_item(NAME(m_bcd_7445));

	save_item(NAME(m_vsb_ls273));
}

void mazerbla_state::machine_reset()
{
	m_zpu_int_vector = 0xff;

	m_gfx_rom_bank = 0xff;

	m_port02_status = 0;
	m_bcd_7445 = 0;
	m_vsb_ls273 = 0;

	if (m_soundlatch.found())
	{
		m_soundlatch->clear_w();
		m_soundlatch->acknowledge_w();
	}

	for (int i = 0; i < 4; i++)
	{
		m_ls670_0[i] = 0;
		m_ls670_1[i] = 0;
	}

	zpu_bcd_decoder_w(0);
}

void mazerbla_state::mazerbla(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK);  /* 4 MHz, no NMI, IM2 - vectors at 0xf8, 0xfa, 0xfc */
	m_maincpu->set_addrmap(AS_PROGRAM, &mazerbla_state::mazerbla_map);
	m_maincpu->set_addrmap(AS_IO, &mazerbla_state::mazerbla_io_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(mazerbla_state::irq_callback));

	Z80(config, m_subcpu, MASTER_CLOCK);  /* 4 MHz, NMI, IM1 INT */
	m_subcpu->set_addrmap(AS_PROGRAM, &mazerbla_state::mazerbla_cpu2_map);
	m_subcpu->set_addrmap(AS_IO, &mazerbla_state::mazerbla_cpu2_io_map);
	m_subcpu->set_periodic_int(FUNC(mazerbla_state::irq0_line_hold), attotime::from_hz(400)); /* frequency in Hz */

	z80_device &sub2(Z80(config, "sub2", MASTER_CLOCK)); /* 4 MHz, no  NMI, IM1 INT */
	sub2.set_addrmap(AS_PROGRAM, &mazerbla_state::mazerbla_cpu3_map);
	sub2.set_addrmap(AS_IO, &mazerbla_state::mazerbla_cpu3_io_map);
/* (vblank related ??) int generated by a custom video processor
    and cleared on ANY port access.
    but handled differently for now
    */
	sub2.set_vblank_int("screen", FUNC(mazerbla_state::irq0_line_hold));

	for (int i = 0; i < 2; i++)
		ADC0804(config, m_uib_adc[i], RES_R(10), CAP_P(150)).set_rd_mode(adc0804_device::RD_GROUNDED);
	m_uib_adc[0]->vin_callback().set_ioport("STICK0_X");
	m_uib_adc[1]->vin_callback().set_ioport("STICK0_Y");

	/* synchronization forced on the fly */
	MB_VCU(config, m_vcu, SOUND_CLOCK/4);
	m_vcu->set_cpu_tag("sub2");
	m_vcu->set_palette_tag("palette");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(40*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 28*8-1);
	m_screen->set_screen_update(FUNC(mazerbla_state::screen_update_mazerbla));
	m_screen->screen_vblank().set(FUNC(mazerbla_state::screen_vblank));

	PALETTE(config, "palette", FUNC(mazerbla_state::mazerbla_palette), 256+1);

	/* sound hardware */
}


void mazerbla_state::greatgun(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK);  /* 4 MHz, no NMI, IM2 - vectors at 0xf8, 0xfa, 0xfc */
	m_maincpu->set_addrmap(AS_PROGRAM, &mazerbla_state::mazerbla_map);
	m_maincpu->set_addrmap(AS_IO, &mazerbla_state::greatgun_io_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(mazerbla_state::irq_callback));

	Z80(config, m_subcpu, SOUND_CLOCK / 4);   /* 3.579500 MHz, NMI - caused by sound command write, periodic INT */
	m_subcpu->set_addrmap(AS_PROGRAM, &mazerbla_state::greatgun_sound_map);
	m_subcpu->set_periodic_int(FUNC(mazerbla_state::sound_interrupt), attotime::from_hz((double)14318180/16/16/16/16 ));

	z80_device &sub2(Z80(config, "sub2", MASTER_CLOCK)); /* 4 MHz, no  NMI, IM1 INT */
	sub2.set_addrmap(AS_PROGRAM, &mazerbla_state::mazerbla_cpu3_map);
	sub2.set_addrmap(AS_IO, &mazerbla_state::greatgun_cpu3_io_map);
/* (vblank related ??) int generated by a custom video processor
    and cleared on ANY port access.
    but handled differently for now
    */
	sub2.set_vblank_int("screen", FUNC(mazerbla_state::irq0_line_hold));

	for (int i = 0; i < 4; i++)
		ADC0804(config, m_uib_adc[i], RES_R(10), CAP_P(150)).set_rd_mode(adc0804_device::RD_GROUNDED);
	m_uib_adc[0]->vin_callback().set_ioport("STICK0_X");
	m_uib_adc[1]->vin_callback().set_ioport("STICK0_Y");
	m_uib_adc[2]->vin_callback().set_ioport("STICK1_X");
	m_uib_adc[3]->vin_callback().set_ioport("STICK1_Y");

	MB_VCU(config, m_vcu, SOUND_CLOCK/4);
	m_vcu->set_cpu_tag("sub2");
	m_vcu->set_palette_tag("palette");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(40*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 28*8-1);
	m_screen->set_screen_update(FUNC(mazerbla_state::screen_update_mazerbla));
	m_screen->screen_vblank().set(FUNC(mazerbla_state::screen_vblank));

	PALETTE(config, "palette", FUNC(mazerbla_state::mazerbla_palette), 256+1);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", SOUND_CLOCK / 8));
	ay1.port_b_read_callback().set(m_soundlatch, FUNC(generic_latch_8_device::read));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.30);

	ay8910_device &ay2(AY8910(config, "ay2", SOUND_CLOCK / 8));
	ay2.port_b_write_callback().set(FUNC(mazerbla_state::gg_led_ctrl_w));
	ay2.add_route(ALL_OUTPUTS, "mono", 1.0);

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_subcpu, INPUT_LINE_NMI);
	m_soundlatch->set_separate_acknowledge(true);
}

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

ROM_START( mazerbla )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for main CPU (ZPU board) */
	ROM_LOAD( "mblzpu0.1h",0x0000, 0x2000, CRC(82766187) SHA1(cfc425c87cccb84180f1091998eafeaede126d9d) )
	ROM_LOAD( "mblzpu1.2h",0x2000, 0x2000, CRC(8ba2b3f9) SHA1(1d203332e434d1d9821f98c6ac959ae65dcc51ef) )
	ROM_LOAD( "mblzpu2.3h",0x4000, 0x2000, CRC(48e5306c) SHA1(d27cc85d24c7b6c23c5c96be4dad5cae6e8069be) )
	ROM_LOAD( "mblzpu3.4h",0x6000, 0x2000, CRC(eba91546) SHA1(8c1da4e0d9b562dbbf7c7583dbf567c804eb670f) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for sound CPU (VSB board) */
	ROM_LOAD( "mblvsb0.2d",0x0000, 0x1000, CRC(0cf7a1c3) SHA1(af27e3a3b51d03d46c62c2797268744d0577d075) )
	ROM_LOAD( "mblvsb1.4d",0x1000, 0x1000, CRC(0b8d0e43) SHA1(b3ddb7561e715a58ca512fe76e53cda39402a8e4) )

	ROM_REGION( 0x10000, "digitalker", 0) /* 64k? for digitalker voice samples */
	ROM_LOAD( "mblvsb2.2a",0x0000, 0x1000, NO_DUMP ) /* size may be wrong */
	ROM_LOAD( "mblvsb3.4a",0x1000, 0x1000, NO_DUMP ) /* size may be wrong */

	ROM_REGION( 0x18000, "sub2", 0 )     /* 64k for video CPU (CFB board) */
	ROM_LOAD( "mblcfb0.8g",0x0000, 0x2000, CRC(948a2c5e) SHA1(d693f1b96caf31649f600c5038bb79b0d1d16133) )

	ROM_LOAD( "mblcfb2.8k",0x10000,0x2000, CRC(36237058) SHA1(9db8fced37a3d40c4ea5b87ea18ac8e75d71e586) )/*banked at 0x4000 (select=0)*/
	ROM_LOAD( "mblcfb3.10k",0x12000,0x2000, CRC(18d75d7f) SHA1(51c35ea4a2127439a1299863eb74e57be833e2e4) )/*banked at 0x4000 (select=1)*/
	/* empty socket??? (the *name* of next rom seems good ?) or wrong schematics ?*/
	ROM_LOAD( "mblcfb4.14k",0x16000,0x2000, CRC(1805acdc) SHA1(40b8e70e6ba69ac864af0b276e81218e63e48deb) )/*banked at 0x4000 (select=3) (assumed to be at 14k, may be at 12k)*/

	ROM_REGION( 0x00640, "proms", 0 )
	ROM_LOAD( "82s123.8b", 0x0000, 0x0020, CRC(d558af5a) SHA1(060556beeb1f6732c4520dcfb0086c428f7b9ce3) )
	ROM_LOAD( "82s123.9b", 0x0020, 0x0020, CRC(0390d748) SHA1(df0f750c1df45cc7bfb9dbabfa2b94563d19172a) )
	ROM_LOAD( "82s129.8g", 0x0040, 0x0100, CRC(19680615) SHA1(c309eb83e66b202bae9174dc2ffce231fca40644) )
	ROM_LOAD( "82s129.9g", 0x0140, 0x0100, CRC(f8c2c85b) SHA1(d9514af5682a2c5dec5366dcbdf5c7f6ef9f5380) )
	ROM_LOAD( "6353-1.16a", 0x240, 0x0400, NO_DUMP ) /* 82s137-equivalent video prom, next to VCU */

	ROM_REGION( 0x00240, "pals", 0 )
	ROM_LOAD( "pal16r8.7d", 0x0000, 0x098, NO_DUMP ) /* pal on zpu board, for ?protection? (similar to bagman?) */
ROM_END

ROM_START( mazerblaa )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for main CPU (ZPU board) */
	ROM_LOAD( "zpu0.1h",       0x0000, 0x2000, CRC(aa77705c) SHA1(ef93c3eaa66591bef495caa101ef2aff93f2de8c) )
	ROM_LOAD( "zpu1.2h",       0x2000, 0x2000, CRC(599e1b97) SHA1(ceeb3017d6130d4d54ff4436261f2d3f2a29f8ab) )
	ROM_LOAD( "zpu2.3h",       0x4000, 0x2000, CRC(e1504613) SHA1(815b56e067d60dda6c5ebed97ef8da3f6c2927ad) )
	ROM_LOAD( "zpu3.4h",       0x6000, 0x2000, CRC(fd27f409) SHA1(e3d49b931325c75cc0c1075944095bb48501501f) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for sound CPU (VSB board) */
	ROM_LOAD( "mblvsb0.2d",0x0000, 0x1000, CRC(0cf7a1c3) SHA1(af27e3a3b51d03d46c62c2797268744d0577d075) )
	ROM_LOAD( "mblvsb1.4d",0x1000, 0x1000, CRC(0b8d0e43) SHA1(b3ddb7561e715a58ca512fe76e53cda39402a8e4) )

	ROM_REGION( 0x10000, "digitalker", 0) /* 64k? for digitalker voice samples */
	ROM_LOAD( "mblvsb2.2a",0x0000, 0x1000, NO_DUMP ) /* size may be wrong */
	ROM_LOAD( "mblvsb3.4a",0x1000, 0x1000, NO_DUMP ) /* size may be wrong */

	ROM_REGION( 0x18000, "sub2", 0 )     /* 64k for video CPU (CFB board) */
	ROM_LOAD( "mblcfb0.8g",0x0000, 0x2000, CRC(948a2c5e) SHA1(d693f1b96caf31649f600c5038bb79b0d1d16133) )

	ROM_LOAD( "mblcfb2.8k",0x10000,0x2000, CRC(36237058) SHA1(9db8fced37a3d40c4ea5b87ea18ac8e75d71e586) )/*banked at 0x4000 (select=0)*/
	ROM_LOAD( "mblcfb3.10k",0x12000,0x2000, CRC(18d75d7f) SHA1(51c35ea4a2127439a1299863eb74e57be833e2e4) )/*banked at 0x4000 (select=1)*/
	/* empty socket??? (the *name* of next rom seems good ?) or wrong schematics ?*/
	ROM_LOAD( "mblcfb4.14k",0x16000,0x2000, CRC(1805acdc) SHA1(40b8e70e6ba69ac864af0b276e81218e63e48deb) )/*banked at 0x4000 (select=3) (assumed to be at 14k, may be at 12k)*/

	ROM_REGION( 0x00640, "proms", 0 )
	ROM_LOAD( "82s123.8b", 0x0000, 0x0020, CRC(d558af5a) SHA1(060556beeb1f6732c4520dcfb0086c428f7b9ce3) )
	ROM_LOAD( "82s123.9b", 0x0020, 0x0020, CRC(0390d748) SHA1(df0f750c1df45cc7bfb9dbabfa2b94563d19172a) )
	ROM_LOAD( "82s129.8g", 0x0040, 0x0100, CRC(19680615) SHA1(c309eb83e66b202bae9174dc2ffce231fca40644) )
	ROM_LOAD( "82s129.9g", 0x0140, 0x0100, CRC(f8c2c85b) SHA1(d9514af5682a2c5dec5366dcbdf5c7f6ef9f5380) )
	ROM_LOAD( "6353-1.16a", 0x240, 0x0400, NO_DUMP ) /* 82s137-equivalent video prom, next to VCU */

	ROM_REGION( 0x00240, "pals", 0 )
	ROM_LOAD( "pal16r8.7d", 0x0000, 0x098, NO_DUMP ) /* pal on zpu board, for ?protection? (similar to bagman?) */
ROM_END


ROM_START( greatgun )
	ROM_REGION( 0x10000, "maincpu", 0 )     /* 64k for main CPU (ZPU board) */
	ROM_LOAD( "zpu0.1h",0x0000, 0x2000, CRC(80cf2cbf) SHA1(ea24b844ea6d8fc54adb2e28be68e1f3e1184b8b) )
	ROM_LOAD( "zpu1.2h",0x2000, 0x2000, CRC(fc12af94) SHA1(65f5bca2853271c232bd02dfc3467e6a4f7f0a6f) )
	ROM_LOAD( "zpu2.3h",0x4000, 0x2000, CRC(b34cfa26) SHA1(903adc6de0d34e5bc8fb0f8d3e74ff53204d8c68) )
	ROM_LOAD( "zpu3.4h",0x6000, 0x2000, CRC(c142ebdf) SHA1(0b87740d26b19a05f65b811225ee0053ddb27d22) )

	ROM_REGION( 0x10000, "sub", 0 )     /* 64k for sound CPU (PSB board) */
	ROM_LOAD( "psb.prom0.4a",0x0000, 0x2000, CRC(172a793e) SHA1(3618a778af1f4a6267bf7e0786529be731ac9b76) )/*ROM number is silkscreened on the PCB & PCB location is also given */

	ROM_REGION( 0x60000, "sub2", 0 )     /* 64k for video CPU (CFB board) */
	ROM_LOAD( "cfb0.7f",0x00000, 0x2000, CRC(ee372b1f) SHA1(b630fd659d59eb8c2540f18d91ae0d72e859fc4f) )
	ROM_LOAD( "cfb1.8f",0x02000, 0x2000, CRC(b76d9527) SHA1(8f16b850bd67d553aaaf7e176754e36aba581445) )

	ROM_LOAD( "great_guns_psb0_ra1.rom0.5g",0x10000,0x2000,   CRC(b4956100) SHA1(98baf5c27c76dc5c4eafc44f42705239504637fe) )/*banked at 0x4000*/
	ROM_LOAD( "great_guns_psb1_ra1.rom1.5f",0x14000,0x2000,   CRC(acdce2ee) SHA1(96b8961afbd0006b10cfdc825aefe27ec18121ff) )/*ROM number is silkscreened on the PCB & PCB locations are also given */
	ROM_LOAD( "great_guns_psb2_ra1.rom2.5d",0x18000,0x2000,   CRC(cb840fc6) SHA1(c30c72d355e1957f3715e9fab701f65b9d7d632a) )
	ROM_LOAD( "great_guns_psb3_ra1.rom3.5c",0x1c000,0x2000,   CRC(86ea6f99) SHA1(ce5d42557d0a62eebe3d0cee28587d60707573e4) )
	ROM_LOAD( "great_guns_psb4_ra1.rom4.6g",0x20000,0x2000,   CRC(65379893) SHA1(84bb755e23d5ce13b1c82e59f24f3890c50697cc) )
	ROM_LOAD( "great_guns_psb5_ra1.rom5.6f",0x24000,0x2000,   CRC(f82245cb) SHA1(fa1cab94a03ce7b8e45ea6eec572b21f268f7547) )
	ROM_LOAD( "great_guns_psb6_ra1.rom6.6d",0x28000,0x2000,   CRC(6b86794f) SHA1(72cf67ecf5a9198ecb44dd846de968e6cdd6458d) )
	ROM_LOAD( "great_guns_psb7_ra1.rom7.6c",0x2c000,0x2000,   CRC(60a7abf3) SHA1(44b932d8af29ec706c29d6b71a8bac6318d92315) )
	ROM_LOAD( "great_guns_psb8_ra1.rom8.7g",0x30000,0x2000,   CRC(854be14e) SHA1(ae9b1fe2443c87bb4334bc776f7bc7e5fa874f38) )
	ROM_LOAD( "great_guns_psb9_ra1.rom9.7f",0x34000,0x2000,   CRC(b2e8afa3) SHA1(30a3d83bf1ec7885549b47f9569e9ae0d05b948d) )
	ROM_LOAD( "great_guns_psb10_ra1.rom10.7d",0x38000,0x2000, CRC(fbfb0aab) SHA1(2eb666a5eff31019b4ffdfc82e242ff47cd59527) )
	ROM_LOAD( "great_guns_psb11_ra1.rom11.7c",0x3c000,0x2000, CRC(ddcd3cec) SHA1(7d0c3b4160b11ebe9b097664190d8ae605413baa) )
	ROM_LOAD( "great_guns_psb12_ra1.rom12.8g",0x40000,0x2000, CRC(c6617377) SHA1(29a6fc52e06c41f06ee333aad707c3a1952dff4d) )
	ROM_LOAD( "great_guns_psb13_ra1.rom13.8f",0x44000,0x2000, CRC(aeab8555) SHA1(c398cac5210022e3c9e25a9f2ef1017b27c21e62) )
	ROM_LOAD( "great_guns_psb14_ra1.rom14.8d",0x48000,0x2000, CRC(ef35e314) SHA1(2e20517ff89b153fd888cf4eb0404a802e16b1b7) )
	ROM_LOAD( "great_guns_psb15_ra1.rom15.8c",0x4c000,0x2000, CRC(1fafe83d) SHA1(d1d406275f50d87547aabe1295795099f341433d) )
	ROM_LOAD( "great_guns_psb16_ra1.rom16.9g",0x50000,0x2000, CRC(ec49864f) SHA1(7a3b295972b52682406f75c4fe12c29632452491) )
	ROM_LOAD( "great_guns_psb17_ra1.rom17.9f",0x54000,0x2000, CRC(d9778e85) SHA1(2998f0a08cdba8a75e687a54cb9a03edeb4b22cd) )
	ROM_LOAD( "great_guns_psb18_ra1.rom18.9d",0x58000,0x2000, CRC(ef61b6c0) SHA1(7e8a82beefb9fd8e219fc4d7d25a3a43ab8aadf7) )
	ROM_LOAD( "great_guns_psb19_ra1.rom19.9c",0x5c000,0x2000, CRC(68752e0d) SHA1(58a4921e4f774af5e1ef7af67f06e9b43643ffab) )
//  ROM20.10g, ROM21.10f, ROM22.10d and ROM23.10c are unpopulated.
ROM_END

void mazerbla_state::init_mazerbla()
{
//  m_game_id = MAZERBLA;
}

void mazerbla_state::init_greatgun()
{
	uint8_t *rom = memregion("sub2")->base();

//  m_game_id = GREATGUN;

	//  patch VCU test
	//  VCU test starts at PC=0x56f
	rom[0x05b6] = 0;
	rom[0x05b7] = 0;
	//  so we also need to patch ROM checksum test
	rom[0x037f] = 0;
	rom[0x0380] = 0;
}

} // anonymous namespace


GAME( 1983, mazerbla,  0,        mazerbla,  mazerbla, mazerbla_state, init_mazerbla, ROT0, "Stern Electronics", "Mazer Blazer (set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE )
GAME( 1983, mazerblaa, mazerbla, mazerbla,  mazerblaa,mazerbla_state, init_mazerbla, ROT0, "Stern Electronics", "Mazer Blazer (set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_SOUND | MACHINE_UNEMULATED_PROTECTION | MACHINE_SUPPORTS_SAVE ) // newer?
GAME( 1983, greatgun,  0,        greatgun,  greatgun, mazerbla_state, init_greatgun, ROT0, "Stern Electronics", "Great Guns",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
