// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Juergen Buchmueller, Alex Pasadyn, Aaron Giles, Nicola Salmoria
/***************************************************************************

Pole Position    (c) 1982 Namco
Pole Position II (c) 1983 Namco

driver by Ernesto Corvi, Juergen Buchmueller, Alex Pasadyn, Aaron Giles, Nicola Salmoria


Custom ICs:
----------
CPU board:
06XX     interface to custom 5xXX
07XX     clock divider
08XX(x2) bus controller
10XX(x4) Z8002 bus controller
51XX     I/O
52XX     sample player
53XX     I/O
54XX     explosion sound generator

Video board:
02XX(x3) gfx data shifter and mixer (16-bit in, 4-bit out)
03XX(x2) ?
04XX     sprite address generator
07XX     clock divider
09XX     address bus interface


Memory maps:
-----------
Part of the address decoding is done by PALs so it is inferred by program behaviour

Z80:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
000xxxxxxxxxxxxx R   xxxxxxxx ROM 7H    program ROM
0010xxxxxxxxxxxx R   xxxxxxxx ROM 7F    program ROM
0011-xxxxxxxxxxx R/W xxxxxxxx CMOSRAMCS battery back-up RAM
010000xxxxxxxxxx R/W xxxxxxxx CSMB      work RAM                                      [1]
010000111xxxxxxx R/W xxxxxxxx           portion holding sprite registers (POSI)       [1]
010001xxxxxxxxxx R/W xxxxxxxx CSMD      work RAM                                      [1]
010001111xxxxxxx R/W xxxxxxxx           portion holding sprite registers (SIZE, DATA) [1]
01001xxxxxxxxxxx R/W xxxxxxxx RAM 3F    1st half is road, 2nd half alpha tilemap      [1]
01010xxxxxxxxxxx R/W xxxxxxxx RAM 3E    background tilemap (1st half only)            [1]
1000--xxxxxxxxxx R/W xxxxxxxx RAMCS     work RAM
1000--1111xxxxxx R/W xxxxxxxx           portion holding the sound registers
1001---0-------- R/W xxxxxxxx IODBENBL  custom 06XX data
1001---1-------- R/W xxxxxxxx IODBENBL  custom 06XX control
1010--00-------- R   -------x READY     +5V
1010--00-------- R   ------x- READY     128V
1010--00-------- R   -----x-- READY     PWRUP (power line sense)
1010--00-------- R   ----x--- READY     ADC0804 INTR (end flag)
1010--01-------- R            n.c.
1010--10-------- R            n.c.
1010--11-------- R            n.c.
1010--00-----000   W -------x IRQON     Z80 IRQ enable/acknowledge
1010--00-----001   W -------x IOSEL     reset 5xXX chips
1010--00-----010   W -------x CLSON     sound enable [2]
1010--00-----011   W -------x GASEL     accelerator/brake select
1010--00-----100   W -------x RESB      reset Z8002 #1
1010--00-----101   W -------x RESA      reset Z8002 #2
1010--00-----110   W -------x SB0       start (goes to 51XX start button input)
1010--00-----111   W -------x CHACL     alpha layer enable color and msb
1010--01--------   W -------- WDR       watchdog reset
1010--10--------   W -------x XSOUND    engine enable
1010--10--------   W --xxxxx- XSOUND    engine pitch lsb
1010--11--------   W --xxxxxx XSON      engine pitch msb

[1] shared with the two Z8002, but the Z80 can only access the low 8 bits of these
    16-bit areas
[2] affects wave and engine, but not 54XX and 52XX. Note that for the engine, this
    clears the XSOUND and XSON latches.


Z80 I/O:

Address          Dir Data     Name      Description
---------------- --- -------- --------- -----------------------
---------------- R   xxxxxxxx           ADC0804 (accelerator/brake pedals)


Z8002 #1:

Address          Dir Data             Name      Description
---------------- --- ---------------- --------- -----------------------
00xxxxxxxxxxxxx- R   xxxxxxxxxxxxxxxx ROM 4L/3L program ROM
01xxxxxxxxxxxxx- R   xxxxxxxxxxxxxxxx ROM 4K/3K program ROM
011-------------   W ---------------x NMIACKB   Z8002 #2 NVI enable/acknowledge [1]
the rest of the memory map is common to the other Z8002


Z8002 #2:

Address          Dir Data             Name      Description
---------------- --- ---------------- --------- -----------------------
00xxxxxxxxxxxxx- R   xxxxxxxxxxxxxxxx ROM 4E/3E program ROM
01xxxxxxxxxxxxx- R   xxxxxxxxxxxxxxxx ROM 4D/3D program ROM
011-------------   W ---------------x NMIACKA   Z8002 #1 NVI enable/acknowledge [1]
the rest of the memory map is common to the other Z8002

[1] One Z8002 writes at $6000 and the other at $6002, but they did it only for clarity
    because the low address bits are ignored and the location is not shared.

    NMIACK is not used to handle the NMI or NVI acknowledge cycles (whose codes are
    ignored).


Z8002 (common):

Address          Dir Data             Name      Description
---------------- --- ---------------- --------- -----------------------
10000xxxxxxxxxx- R/W xxxxxxxxxxxxxxxx CSMA/CSMB work RAM
10000111xxxxxxx- R/W xxxxxxxxxxxxxxxx           portion holding sprite registers (POSI)
10001xxxxxxxxxx- R/W xxxxxxxxxxxxxxxx CSMC/CSMD work RAM
10001111xxxxxxx- R/W xxxxxxxxxxxxxxxx           portion holding sprite registers (SIZE, DATA)
1001xxxxxxxxxxx- R/W xxxxxxxxxxxxxxxx RAM 4F/3F 1st half is road, 2nd half alpha tilemap
1010xxxxxxxxxxx- R/W xxxxxxxxxxxxxxxx RAM 4E/3E background tilemap (1st half only)
11---000--------   W ------xxxxxxxxxx VHP       background horizontal position
11---001--------   W ----xxxxxxxxxxxx RVP       road vertical position
11---010--------   W                  n.c.
11---011--------   W                  n.c.
11---100--------   W                  n.c.
11---101--------   W                  n.c.
11---110--------   W                  n.c.
11---111--------   W                  n.c.


Namco vs Atari ROM names and locations
--------------------------------------
* = not present

Location  ID (PP1)    ID (PP2)    Location  ID (PP1)                  ID (PP2)
--------  ----------  ----------  --------  ------------------------  ----------
CPU 8M    PP1-1       PP4-1       CPU 3L    136014-101                136014-176
CPU 8L    PP1-2       PP4-2       CPU 4L    136014-102                136014-177
   ?      PP1-3*      PP4-3*      CPU 3K    136014-112*               *
   ?      PP1-4*      PP4-4*      CPU 4K    136014-113*               *
CPU 4M    PP1-5       PP4-5       CPU 3E    136014-103 or 136014-203  136014-178
CPU 4L    PP1-6       PP4-6       CPU 4E    136014-104 or 136014-204  136014-179
CPU 3M    PP1-7*      PP4-7       CPU 3D    136014-114*               136014-184
CPU 3L    PP1-8*      PP4-8       CPU 4D    136014-115*               136014-185
CPU 6H    PP1-9       PP4-9       CPU 7H    136014-105 or 136014-160  136014-180
CPU 5H    PP1-10      PP4-10      CPU 7F    136014-116                136014-183
CPU 2E    PP1-11      <--         CPU 9C    136014-106 or 136014-147  <--
CPU 2F    PP1-12      <--         CPU 9A    136014-108*               *
CPU 1E    PP1-13      <--         CPU 8C    136014-107*               *
CPU 1F    PP1-14      <--         CPU 8A    136014-109*               *
CPU 6A    PP1-15      PP4-15      CPU 12F   136014-110 or 136014-148  136014-181
CPU 5A    PP1-16      PP4-16      CPU 12E   136014-111 or 136014-149  136014-182
   ?      PP1-1[pal]  <--         CPU 5C    PAL-1                     <--
   ?      PP1-2[pal]  <--         CPU 2N    PAL-1                     <--
   ?      PP1-3[pal]  <--         CPU 7C    PAL-3                     <--
CPU 9H    PP1-4[bpr]  <--         CPU 7L    136014-117                <--
CPU 3B    PP1-5[bpr]  <--         CPU 11D   136014-118                <--

VID 5N    PP1-17      <--         VID 13J   136014-119 == 136014-150  <--
VID 5M    PP1-18      <--         VID 12J   136014-120 == 136014-151  <--
VID 4N    PP1-19      <--         VID 13K   136014-121 or 136014-152  136014-166
VID 4M    PP1-20      <--         VID 12K   136014-122 or 136014-153  136014-167
VID 3N    PP1-21      <--         VID 13L   136014-123 or 136014-154  136014-168
VID 3M    PP1-22      <--         VID 12L   136014-124 or 136014-155  136014-169
VID 2N    PP1-23*     PP4-23      VID 13M   136014-129*               136014-175
VID 2M    PP1-24*     PP4-24      VID 12M   136014-130*               136014-174
VID 1N    PP1-25      PP4-25      VID 13N   136014-125 or 136014-156  136014-170
VID 1M    PP1-26      PP4-26      VID 12N   136014-126 or 136014-157  136014-171
VID 1L    PP1-27      <--         VID 11N   136014-131 or 136014-231  <--
VID 1F    PP1-28      PP4-28      VID 7N    136014-132 or 137205-001? 136014-172
VID 1E    PP1-29      PP4-29      VID 6N    136014-133 or 137205-001? 136014-173
VID 3A    PP1-30      <--         VID 2L    136014-127 == 136014-158  <--
VID 2A    PP1-31      <--         VID 2M    136014-128 == 136014-159  <--
VID 1A    PP1-32      <--         VID 2N    136014-134 or 137205-001? <--
VID 6M    PP1-6[bpr]  PP4-6[bpr]  VID 12H   136014-146                136014-192
VID 8L    PP1-7[bpr]  PP4-7[bpr]  VID 11E   136014-137                136014-186
VID 9L    PP1-8[bpr]  PP4-8[bpr]  VID 11D   136014-138                136014-187
VID 10L   PP1-9[bpr]  PP4-9[bpr]  VID 11C   136014-139                136014-188
VID 2H    PP1-10[bpr] PP4-10[bpr] VID 8M    136014-140                136014-189
VID 4D    PP1-11[bpr] PP4-11[bpr] VID 5K    136014-141                136014-190
VID 3C    PP1-12[bpr] PP4-12[bpr] VID 4L    136014-145                136014-191
VID 8E    PP1-13[bpr] <--         VID 6E    136014-135                <--
VID 9E    PP1-14[bpr] <--         VID 6D    136014-136                <--
VID 9A    PP1-15[bpr] <--         VID 2D    136014-142                <--
VID 10A   PP1-16[bpr] <--         VID 2C    136014-143                <--
VID 11A   PP1-17[bpr] <--         VID 2B    136014-144                <--


Notes:
-----
- Easter egg (both Pole Position and Pole Position II):
  - enter service mode
  - turn wheel to 04; change the shifter from LO to HI
  - turn wheel to 45; change the shifter from LO to HI
  - turn wheel to 55; change the shifter from LO to HI
  - turn wheel to 56; change the shifter from LO to HI
  - turn wheel to 91; change the shifter from LO to HI
  (c) 1982 NAMCO LTD. will appear on the screen.

- To reset the high score table, enter service mode, press the accelerator and
  change the shifter from LO to HI

- Pole Position II reports 'Manual Start' on the Test Mode. This is ok,
  because they had to accommodate the hardware from Pole Position I to allow
  track selection.

- Change POLEPOS_TOGGLE to 0 if you are using the original gearshift.

- The old version of the vertical scaling ROM, 136014-131, has (apart from
  some irrelevant differences) one bad bit.
  This seems to be a genuine error on Atari's part, since they replaced it with
  136014-231 which matches Namco's PP1-27. The bad bit should cause a tiny gfx
  glitch, though it's difficult to notice.


Todo:

- the bootlegs without Namco devices (e.g. topracern, polepos2bi) still require
  our 06xx and 51xx emulation to boot, this is incorrect.

***************************************************************************/

#include "emu.h"
#include "polepos.h"
#include "namco52.h"
#include "namco54.h"
#include "polepos_a.h"

#include "cpu/z80/z80.h"
#include "cpu/z8000/z8000.h"
#include "cpu/mb88xx/mb88xx.h"
#include "namco06.h"
#include "namco51.h"
#include "namco53.h"
#include "machine/nvram.h"
#include "machine/watchdog.h"
#include "sound/dac.h"
#include "sound/tms5220.h"
#include "speaker.h"

#include "polepos.lh"
#include "topracer.lh"


#define MASTER_CLOCK    XTAL(24'576'000)
#define POLEPOS_TOGGLE  PORT_TOGGLE


/*************************************************************************************/
/* Pole Position II protection                                                       */
/*************************************************************************************/

uint16_t polepos_state::polepos2_ic25_r(offs_t offset)
{
	int result;
	/* protection states */

	offset = offset & 0x1ff;
	if (offset < 0x100)
	{
		m_last_signed = offset & 0xff;
		result = m_last_result & 0xff;
	}
	else
	{
		m_last_unsigned = offset & 0xff;
		result = (m_last_result >> 8) & 0xff;
		m_last_result = (int8_t)m_last_signed * (uint8_t)m_last_unsigned;
	}

//  logerror("%s: read IC25 @ %04X = %02X\n", machine().describe_context(), offset, result);

	return result | (result << 8);
}


uint8_t polepos_state::analog_r()
{
	return ioport(m_adc_input ? "ACCEL" : "BRAKE")->read();
}

uint8_t polepos_state::ready_r()
{
	int ret = 0xff;

	if (m_screen->vpos() >= 128)
		ret ^= 0x02;

	if (!m_adc->intr_r())
		ret ^= 0x08; /* ADC End Flag */

	return ret;
}

void polepos_state::gasel_w(int state)
{
	m_adc_input = state;
}

void polepos_state::sb0_w(int state)
{
	m_auto_start_mask = !state;
}

template<bool sub1> void polepos_state::z8002_nvi_enable_w(uint16_t data)
{
	data &= 1;

	m_sub_irq_mask = data;
	if (!data)
		(sub1 ? m_subcpu : m_subcpu2)->set_input_line(z8002_device::NVI_LINE, CLEAR_LINE);
}

int polepos_state::auto_start_r()
{
	return m_auto_start_mask;
}

void polepos_state::out(uint8_t data)
{
// no start lamps in pole position
//  output().set_led_value(1,data & 1);
//  output().set_led_value(0,data & 2);
	machine().bookkeeping().coin_counter_w(1,~data & 4);
	machine().bookkeeping().coin_counter_w(0,~data & 8);
}

void polepos_state::lockout(int state)
{
	machine().bookkeeping().coin_lockout_global_w(state);
}

uint8_t polepos_state::namco_52xx_rom_r(offs_t offset)
{
	uint32_t length = memregion("52xx")->bytes();
logerror("ROM @ %04X\n", offset);
	return (offset < length) ? memregion("52xx")->base()[offset] : 0xff;
}

uint8_t polepos_state::namco_52xx_si_r()
{
	/* pulled to +5V */
	return 1;
}

uint8_t polepos_state::namco_53xx_k_r()
{
	/* hardwired to 0 */
	return 0;
}

uint8_t polepos_state::steering_changed_r()
{
	/* read the current steering value and update our delta */
	uint8_t steer_new = ioport("STEER")->read();
	m_steer_accum += (int8_t)(steer_new - m_steer_last) * 2;
	m_steer_last = steer_new;

	/* if we have delta, clock things */
	if (m_steer_accum < 0)
	{
		m_steer_delta = 0;
		m_steer_accum++;
	}
	else if (m_steer_accum > 0)
	{
		m_steer_delta = 1;
		m_steer_accum--;
	}

	return m_steer_accum & 1;
}

uint8_t polepos_state::steering_delta_r()
{
	return m_steer_delta;
}

TIMER_DEVICE_CALLBACK_MEMBER(polepos_state::scanline)
{
	int scanline = param;

	if (((scanline == 64) || (scanline == 192)) && m_latch->q0_r()) // 64V
		m_maincpu->set_input_line(0, ASSERT_LINE);

	if (scanline == 240 && m_sub_irq_mask)  // VBLANK
	{
		m_subcpu->set_input_line(z8002_device::NVI_LINE, ASSERT_LINE);
		m_subcpu2->set_input_line(z8002_device::NVI_LINE, ASSERT_LINE);
	}
}

void polepos_state::machine_start()
{
	save_item(NAME(m_steer_last));
	save_item(NAME(m_steer_delta));
	save_item(NAME(m_steer_accum));
	save_item(NAME(m_last_result));
	save_item(NAME(m_last_signed));
	save_item(NAME(m_last_unsigned));
	save_item(NAME(m_adc_input));
	save_item(NAME(m_auto_start_mask));
	save_item(NAME(m_sub_irq_mask));
}

void polepos_state::machine_reset()
{
}



/*********************************************************************
 * CPU memory structures
 *********************************************************************/

void polepos_state::z80_map(address_map &map)
{
	map(0x0000, 0x2fff).rom().region("maincpu", 0);
	map(0x3000, 0x37ff).mirror(0x0800).ram().share("nvram");                 /* Battery Backup */
	map(0x4000, 0x47ff).rw(FUNC(polepos_state::sprite_r), FUNC(polepos_state::sprite_w));           /* Motion Object */
	map(0x4800, 0x4bff).rw(FUNC(polepos_state::road_r), FUNC(polepos_state::road_w));               /* Road Memory */
	map(0x4c00, 0x4fff).rw(FUNC(polepos_state::alpha_r), FUNC(polepos_state::alpha_w));             /* Alphanumeric (char ram) */
	map(0x5000, 0x57ff).rw(FUNC(polepos_state::view_r), FUNC(polepos_state::view_w));               /* Background Memory */

	map(0x8000, 0x83bf).mirror(0x0c00).ram();                                   /* Sound Memory */
	map(0x83c0, 0x83ff).mirror(0x0c00).rw(m_namco_sound, FUNC(namco_device::polepos_sound_r), FUNC(namco_device::polepos_sound_w));    /* Sound data */

	map(0x9000, 0x9000).mirror(0x0eff).rw("06xx", FUNC(namco_06xx_device::data_r), FUNC(namco_06xx_device::data_w));
	map(0x9100, 0x9100).mirror(0x0eff).rw("06xx", FUNC(namco_06xx_device::ctrl_r), FUNC(namco_06xx_device::ctrl_w));
	map(0xa000, 0xa000).mirror(0x0cff).r(FUNC(polepos_state::ready_r));                 /* READY */
	map(0xa000, 0xa007).mirror(0x0cf8).w(m_latch, FUNC(ls259_device::write_d0));
	map(0xa100, 0xa100).mirror(0x0cff).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xa200, 0xa200).mirror(0x0cff).w("polepos", FUNC(polepos_sound_device::polepos_engine_sound_lsb_w));    /* Car Sound ( Lower Nibble ) */
	map(0xa300, 0xa300).mirror(0x0cff).w("polepos", FUNC(polepos_sound_device::polepos_engine_sound_msb_w));    /* Car Sound ( Upper Nibble ) */
}

void polepos_state::z80_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(m_adc, FUNC(adc0804_device::read), FUNC(adc0804_device::write));
}


/* the same memory map is used by both Z8002 CPUs; all RAM areas are shared */
void polepos_state::z8002_map(address_map &map)
{
	map(0x8000, 0x8fff).ram().share(m_sprite16_memory);   /* Motion Object */
	map(0x9000, 0x97ff).ram().share(m_road16_memory);     /* Road Memory */
	map(0x9800, 0x9fff).ram().w(FUNC(polepos_state::alpha16_w)).share(m_alpha16_memory);  /* Alphanumeric (char ram) */
	map(0xa000, 0xafff).ram().w(FUNC(polepos_state::view16_w)).share(m_view16_memory);     /* Background memory */
	map(0xc000, 0xc001).mirror(0x38fe).w(FUNC(polepos_state::view16_hscroll_w));                       /* Background horz scroll position */
	map(0xc100, 0xc101).mirror(0x38fe).w(FUNC(polepos_state::road16_vscroll_w));                       /* Road vertical position */
}

void polepos_state::z8002_map_1(address_map &map)
{
	z8002_map(map);
	map(0x0000, 0x7fff).rom().region("sub", 0);
	map(0x6000, 0x6001).mirror(0x1ffe).w(FUNC(polepos_state::z8002_nvi_enable_w<true>)); /* NVI enable - *NOT* shared by the two CPUs */
}

void polepos_state::z8002_map_2(address_map &map)
{
	z8002_map(map);
	map(0x0000, 0x7fff).rom().region("sub2", 0);
	map(0x6000, 0x6001).mirror(0x1ffe).w(FUNC(polepos_state::z8002_nvi_enable_w<false>)); /* NVI enable - *NOT* shared by the two CPUs */
}


/*********************************************************************
 * Input port definitions
 *********************************************************************/

static INPUT_PORTS_START( polepos )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Gear Change") POLEPOS_TOGGLE /* Gear */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(polepos_state::auto_start_r))  // start 1, program controlled
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x60, "90 secs." )
	PORT_DIPSETTING(    0x20, "100 secs." )
	PORT_DIPSETTING(    0x40, "110 secs." )
	PORT_DIPSETTING(    0x00, "120 secs." )
	PORT_DIPNAME( 0x80, 0x80, "Racing Laps" )       PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "3" ) /* Manufacturer's recommended settings for Upright cabinet */
	PORT_DIPSETTING(    0x00, "4" ) /* Manufacturer's recommended settings for Sit-Down cabinet */

	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x03, "Extended Rank" )     PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x07, "A" )
	PORT_DIPSETTING(    0x03, "B" )
	PORT_DIPSETTING(    0x05, "C" )
	PORT_DIPSETTING(    0x01, "D" )
	PORT_DIPSETTING(    0x06, "E" )
	PORT_DIPSETTING(    0x02, "F" )
	PORT_DIPSETTING(    0x04, "G" )
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPNAME( 0x38, 0x28, "Practice Rank" )     PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x38, "A" )
	PORT_DIPSETTING(    0x18, "B" )
	PORT_DIPSETTING(    0x28, "C" )
	PORT_DIPSETTING(    0x08, "D" )
	PORT_DIPSETTING(    0x30, "E" )
	PORT_DIPSETTING(    0x10, "F" )
	PORT_DIPSETTING(    0x20, "G" )
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW2:7") /* Is MPH or Km/H for "English" regions, but only Km/H for Japan ;-) */
	PORT_DIPSETTING(    0x40, DEF_STR( Off) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BRAKE")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0,0x90) PORT_SENSITIVITY(100) PORT_KEYDELTA(16)

	PORT_START("ACCEL")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0,0x90) PORT_SENSITIVITY(100) PORT_KEYDELTA(16)

	PORT_START("STEER")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(4)
INPUT_PORTS_END


static INPUT_PORTS_START( poleposa )
	PORT_INCLUDE( polepos )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0xc0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x06, "90 secs." )
	PORT_DIPSETTING(    0x02, "100 secs." )
	PORT_DIPSETTING(    0x04, "110 secs." )
	PORT_DIPSETTING(    0x00, "120 secs." )
	PORT_DIPNAME( 0x01, 0x01, "Racing Laps" )       PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, "3" ) /* Manufacturer's recommended settings for Upright cabinet */
	PORT_DIPSETTING(    0x00, "4" ) /* Manufacturer's recommended settings for Sit-Down cabinet */

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0xe0, 0x60, "Practice Rank" )     PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0xe0, "A" )
	PORT_DIPSETTING(    0x60, "B" )
	PORT_DIPSETTING(    0xa0, "C" )
	PORT_DIPSETTING(    0x20, "D" )
	PORT_DIPSETTING(    0xc0, "E" )
	PORT_DIPSETTING(    0x40, "F" )
	PORT_DIPSETTING(    0x80, "G" )
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPNAME( 0x1c, 0x14, "Extended Rank" )     PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(    0x1c, "A" )
	PORT_DIPSETTING(    0x0c, "B" )
	PORT_DIPSETTING(    0x14, "C" )
	PORT_DIPSETTING(    0x04, "D" )
	PORT_DIPSETTING(    0x18, "E" )
	PORT_DIPSETTING(    0x08, "F" )
	PORT_DIPSETTING(    0x10, "G" )
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPNAME( 0x02, 0x00, "Speed Unit" )        PORT_DIPLOCATION("SW2:7") /* MPH as per Atari manuals for the US regions */
	PORT_DIPSETTING(    0x00, "mph" )
	PORT_DIPSETTING(    0x02, "km/h" )
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( topracern )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Gear Change") POLEPOS_TOGGLE /* Gear */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Game_Time ) )
	PORT_DIPSETTING(    0x60, "90 secs." )
	PORT_DIPSETTING(    0x20, "100 secs." )
	PORT_DIPSETTING(    0x40, "110 secs." )
	PORT_DIPSETTING(    0x00, "120 secs." )
	PORT_DIPNAME( 0x80, 0x80, "Racing Laps" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	/* FIXME: these dips don't work and may not even exist on this bootleg */
	PORT_START("DSWB")
	PORT_DIPNAME( 0x07, 0x07, "Extended Rank" )
	PORT_DIPSETTING(    0x07, "A" )
	PORT_DIPSETTING(    0x03, "B" )
	PORT_DIPSETTING(    0x05, "C" )
	PORT_DIPSETTING(    0x01, "D" )
	PORT_DIPSETTING(    0x06, "E" )
	PORT_DIPSETTING(    0x02, "F" )
	PORT_DIPSETTING(    0x04, "G" )
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPNAME( 0x38, 0x38, "Practice Rank" )
	PORT_DIPSETTING(    0x38, "A" )
	PORT_DIPSETTING(    0x18, "B" )
	PORT_DIPSETTING(    0x28, "C" )
	PORT_DIPSETTING(    0x08, "D" )
	PORT_DIPSETTING(    0x30, "E" )
	PORT_DIPSETTING(    0x10, "F" )
	PORT_DIPSETTING(    0x20, "G" )
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BRAKE")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_MINMAX(0,0x90) PORT_SENSITIVITY(100) PORT_KEYDELTA(16)

	PORT_START("ACCEL")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_MINMAX(0,0x90) PORT_SENSITIVITY(100) PORT_KEYDELTA(16)

	PORT_START("STEER")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(30) PORT_KEYDELTA(4)
INPUT_PORTS_END


static INPUT_PORTS_START( polepos2 )
	PORT_INCLUDE( polepos )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0xc0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x04, 0x00, "Speed Unit" )        PORT_DIPLOCATION("SW1:6") /* Set default to MPH for "English" regions */
	PORT_DIPSETTING(    0x00, "mph" )
	PORT_DIPSETTING(    0x04, "km/h" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x01, 0x01, "Freeze" )                PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x80, "90 secs." )
	PORT_DIPSETTING(    0x00, "120 secs." )
	PORT_DIPNAME( 0x60, 0x60, "Practice Rank" )     PORT_DIPLOCATION("SW2:2,3")
	PORT_DIPSETTING(    0x20, "A" )
	PORT_DIPSETTING(    0x60, "B" )
	PORT_DIPSETTING(    0x40, "C" )
	PORT_DIPSETTING(    0x00, "D" )
	PORT_DIPNAME( 0x18, 0x18, "Extended Rank" )     PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x08, "A" )
	PORT_DIPSETTING(    0x18, "B" )
	PORT_DIPSETTING(    0x10, "C" )
	PORT_DIPSETTING(    0x00, "D" )
	PORT_DIPNAME( 0x06, 0x06, "Goal" )          PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x06, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x01, 0x01, "Speed" )         PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x01, "Average" )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
INPUT_PORTS_END


static INPUT_PORTS_START( polepos2j )
	PORT_INCLUDE( polepos2 )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x04, 0x04, "Speed Unit" )        PORT_DIPLOCATION("SW1:6") /* Set default to km/h for Japan */
	PORT_DIPSETTING(    0x00, "mph" )
	PORT_DIPSETTING(    0x04, "km/h" )
INPUT_PORTS_END


static INPUT_PORTS_START( polepos2bi )
	PORT_INCLUDE( topracern )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:1,3,5")
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWA:7,2")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x00, "Speed Unit" )        PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x20, "mph" )
	PORT_DIPSETTING(    0x00, "km/h" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Speed" )         PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x80, "Average" )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x03, 0x01, "Extended Rank" )     PORT_DIPLOCATION("SWB:1,3")
	PORT_DIPSETTING(    0x00, "A" )
	PORT_DIPSETTING(    0x01, "B" )
	PORT_DIPSETTING(    0x02, "C" )
	PORT_DIPSETTING(    0x03, "D" )
	PORT_DIPNAME( 0x0c, 0x04, "Practice Rank" )     PORT_DIPLOCATION("SWB:5,7")
	PORT_DIPSETTING(    0x00, "A" )
	PORT_DIPSETTING(    0x04, "B" )
	PORT_DIPSETTING(    0x08, "C" )
	PORT_DIPSETTING(    0x0c, "D" )
	PORT_DIPNAME( 0x30, 0x10, "Goal" )          PORT_DIPLOCATION("SWB:2,4")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Game_Time ) )    PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x00, "90 secs." )
	PORT_DIPSETTING(    0x40, "120 secs." )
	PORT_DIPNAME( 0x80, 0x00, "Freeze" )                PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ))
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END



/*********************************************************************
 * Graphics layouts
 *********************************************************************/

static const gfx_layout charlayout_2bpp =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8*2
};

static const gfx_layout bigspritelayout =
{
	32,32,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4 },
	{  0,  1,  2,  3,  8,  9, 10, 11,
		16, 17, 18, 19, 24, 25, 26, 27,
		32, 33, 34, 35, 40, 41, 42, 43,
		48, 49, 50, 51, 56, 57, 58, 59},
	{  0*64,  1*64,  2*64,  3*64,  4*64,  5*64,  6*64,  7*64,
		8*64,  9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64,
		16*64, 17*64, 18*64, 19*64, 20*64, 21*64, 22*64, 23*64,
		24*64, 25*64, 26*64, 27*64, 28*64, 29*64, 30*64, 31*64 },
	32*64
};

static const gfx_layout smallspritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2), RGN_FRAC(1,2)+4
	},
	{  0,  1,  2,  3,  8,  9, 10, 11,
		16, 17, 18, 19, 24, 25, 26, 27 },
	{ 0*32,  1*32,  2*32,  3*32,  4*32,  5*32,  6*32,  7*32,
		8*32,    9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	16*32
};

static GFXDECODE_START( gfx_polepos )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_2bpp,   0x0000, 128 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout_2bpp,   0x0200,  64 )
	GFXDECODE_ENTRY( "gfx3", 0, smallspritelayout, 0x0300, 128 )
	GFXDECODE_ENTRY( "gfx4", 0, bigspritelayout,   0x0300, 128 )
GFXDECODE_END


/*********************************************************************
 * Machine driver
 *********************************************************************/

void polepos_state::polepos(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK/8);   /* 3.072 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &polepos_state::z80_map);
	m_maincpu->set_addrmap(AS_IO, &polepos_state::z80_io);

	Z8002(config, m_subcpu, MASTER_CLOCK/8);  /* 3.072 MHz */
	m_subcpu->set_addrmap(AS_PROGRAM, &polepos_state::z8002_map_1);

	Z8002(config, m_subcpu2, MASTER_CLOCK/8); /* 3.072 MHz */
	m_subcpu2->set_addrmap(AS_PROGRAM, &polepos_state::z8002_map_2);

	namco_51xx_device &n51xx(NAMCO_51XX(config, "51xx", MASTER_CLOCK/8/2));      /* 1.536 MHz */
	n51xx.input_callback<0>().set_ioport("DSWB").mask(0x0f);
	n51xx.input_callback<1>().set_ioport("DSWB").rshift(4);
	n51xx.input_callback<2>().set_ioport("IN0").mask(0x0f);
	n51xx.input_callback<3>().set_ioport("IN0").rshift(4);
	n51xx.output_callback().set(FUNC(polepos_state::out));
	n51xx.lockout_callback().set(FUNC(polepos_state::lockout));

	namco_52xx_device &n52xx(NAMCO_52XX(config, "52xx", MASTER_CLOCK/8/2));      /* 1.536 MHz */
	n52xx.set_discrete("discrete");
	n52xx.set_basenote(NODE_04);
	n52xx.romread_callback().set(FUNC(polepos_state::namco_52xx_rom_r));
	n52xx.si_callback().set(FUNC(polepos_state::namco_52xx_si_r));

	namco_53xx_device &n53xx(NAMCO_53XX(config, "53xx", MASTER_CLOCK/8/2));      /* 1.536 MHz */
	n53xx.k_port_callback().set(FUNC(polepos_state::namco_53xx_k_r));
	n53xx.input_callback<0>().set(FUNC(polepos_state::steering_changed_r));
	n53xx.input_callback<1>().set(FUNC(polepos_state::steering_delta_r));
	n53xx.input_callback<2>().set_ioport("DSWA").mask(0x0f);
	n53xx.input_callback<3>().set_ioport("DSWA").rshift(4);

	namco_54xx_device &n54xx(NAMCO_54XX(config, "54xx", MASTER_CLOCK/8/2));      /* 1.536 MHz */
	n54xx.set_discrete("discrete");
	n54xx.set_basenote(NODE_01);

	namco_06xx_device &n06xx(NAMCO_06XX(config, "06xx", MASTER_CLOCK/8/64));
	n06xx.set_maincpu(m_maincpu);
	n06xx.chip_select_callback<0>().set("51xx", FUNC(namco_51xx_device::chip_select));
	n06xx.rw_callback<0>().set("51xx", FUNC(namco_51xx_device::rw));
	n06xx.read_callback<0>().set("51xx", FUNC(namco_51xx_device::read));
	n06xx.write_callback<0>().set("51xx", FUNC(namco_51xx_device::write));
	n06xx.read_callback<1>().set("53xx", FUNC(namco_53xx_device::read));
	n06xx.chip_select_callback<1>().set("53xx", FUNC(namco_53xx_device::chip_select));
	n06xx.write_callback<2>().set("52xx", FUNC(namco_52xx_device::write));
	n06xx.chip_select_callback<2>().set("52xx", FUNC(namco_52xx_device::chip_select));
	n06xx.write_callback<3>().set("54xx", FUNC(namco_54xx_device::write));
	n06xx.chip_select_callback<3>().set("54xx", FUNC(namco_54xx_device::chip_select));

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count(m_screen, 16);   // 128V clocks the same as VBLANK

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	TIMER(config, "scantimer").configure_scanline(FUNC(polepos_state::scanline), "screen", 0, 1);

	LS259(config, m_latch); // at 8E on polepos
	m_latch->q_out_cb<0>().set_inputline(m_maincpu, 0, CLEAR_LINE).invert();
	m_latch->q_out_cb<1>().set("51xx", FUNC(namco_51xx_device::reset));
	m_latch->q_out_cb<1>().append("52xx", FUNC(namco_52xx_device::reset));
	m_latch->q_out_cb<1>().append("53xx", FUNC(namco_53xx_device::reset));
	m_latch->q_out_cb<1>().append("54xx", FUNC(namco_54xx_device::reset));
	m_latch->q_out_cb<2>().set(m_namco_sound, FUNC(namco_device::sound_enable_w));
	m_latch->q_out_cb<2>().append("polepos", FUNC(polepos_sound_device::clson_w));
	m_latch->q_out_cb<3>().set(FUNC(polepos_state::gasel_w));
	m_latch->q_out_cb<4>().set_inputline(m_subcpu, INPUT_LINE_RESET).invert();
	m_latch->q_out_cb<5>().set_inputline(m_subcpu2, INPUT_LINE_RESET).invert();
	m_latch->q_out_cb<6>().set(FUNC(polepos_state::sb0_w));
	m_latch->q_out_cb<7>().set(FUNC(polepos_state::chacl_w));

	ADC0804(config, m_adc, MASTER_CLOCK/8/8);
	m_adc->vin_callback().set(FUNC(polepos_state::analog_r));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK/4, 384, 0, 256, 264, 16, 224+16);
	m_screen->set_screen_update(FUNC(polepos_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set("51xx", FUNC(namco_51xx_device::vblank));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_polepos);
	PALETTE(config, m_palette, FUNC(polepos_state::polepos_palette), 0x0f00, 128);

	config.set_default_layout(layout_polepos);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	NAMCO(config, m_namco_sound, MASTER_CLOCK/512);
	m_namco_sound->set_voices(8);
	m_namco_sound->set_stereo(true);
	m_namco_sound->add_route(0, "lspeaker", 0.80);
	m_namco_sound->add_route(1, "rspeaker", 0.80);

	/* discrete circuit on the 54XX outputs */
	discrete_sound_device &discrete(DISCRETE(config, "discrete", polepos_discrete));
	discrete.add_route(ALL_OUTPUTS, "lspeaker", 0.90);
	discrete.add_route(ALL_OUTPUTS, "rspeaker", 0.90);

	/* engine sound */
	polepos_sound_device &polepos(POLEPOS_SOUND(config, "polepos", MASTER_CLOCK/8));
	polepos.add_route(ALL_OUTPUTS, "lspeaker", 0.90 * 0.77);
	polepos.add_route(ALL_OUTPUTS, "rspeaker", 0.90 * 0.77);
}

void polepos_state::bootleg_soundlatch_w(uint8_t data)
{
	if (m_soundlatch.found()) // topracern also uses this; no idea what it should do there
		m_soundlatch->write(data | 0xfc);
}

void polepos_state::topracern_io(address_map &map)
{
	map.global_mask(0xff);
	z80_io(map);
	// extra direct mapped inputs read
	map(0x02, 0x02).portr("STEER").nopw();
	map(0x03, 0x03).portr("IN0").w("dac", FUNC(dac_byte_interface::data_w));
	map(0x04, 0x04).portr("DSWA").nopw(); // explosion sound trigger
	map(0x05, 0x05).portr("DSWB").w(FUNC(polepos_state::bootleg_soundlatch_w));
}

void polepos_state::sound_z80_bootleg_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("soundz80bl", 0);
	map(0x2700, 0x27ff).ram();
	map(0x4000, 0x4000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x6000, 0x6000).r(m_soundlatch, FUNC(generic_latch_8_device::acknowledge_r));
}

void polepos_state::sound_z80_bootleg_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw("tms", FUNC(tms5220_device::status_r), FUNC(tms5220_device::data_w));
}

void polepos_state::topracern(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK/8);   /* 3.072 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &polepos_state::z80_map);
	m_maincpu->set_addrmap(AS_IO, &polepos_state::topracern_io);

	Z8002(config, m_subcpu, MASTER_CLOCK/8);  /* 3.072 MHz */
	m_subcpu->set_addrmap(AS_PROGRAM, &polepos_state::z8002_map_1);

	Z8002(config, m_subcpu2, MASTER_CLOCK/8); /* 3.072 MHz */
	m_subcpu2->set_addrmap(AS_PROGRAM, &polepos_state::z8002_map_2);

	// TODO, remove these devices too, this bootleg doesn't have them, but the emulation doesn't boot without them.
	// doesn't exist on the bootleg, but required for now or the game only boots in test mode!  they probably simulate some of the logic
	namco_51xx_device &n51xx(NAMCO_51XX(config, "51xx", MASTER_CLOCK/8/2));      /* 1.536 MHz */
	n51xx.input_callback<3>().set_ioport("IN0").rshift(4);

	namco_06xx_device &n06xx(NAMCO_06XX(config, "06xx", MASTER_CLOCK/8/64));
	n06xx.set_maincpu(m_maincpu);
	n06xx.chip_select_callback<0>().set("51xx", FUNC(namco_51xx_device::chip_select));
	n06xx.rw_callback<0>().set("51xx", FUNC(namco_51xx_device::rw));
	n06xx.read_callback<0>().set("51xx", FUNC(namco_51xx_device::read));
	n06xx.write_callback<0>().set("51xx", FUNC(namco_51xx_device::write));

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count(m_screen, 16);   // 128V clocks the same as VBLANK

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	TIMER(config, "scantimer").configure_scanline(FUNC(polepos_state::scanline), "screen", 0, 1);

	LS259(config, m_latch);
	m_latch->q_out_cb<0>().set_inputline(m_maincpu, 0, CLEAR_LINE).invert();
	m_latch->q_out_cb<2>().set(m_namco_sound, FUNC(namco_device::sound_enable_w));
	m_latch->q_out_cb<2>().append("polepos", FUNC(polepos_sound_device::clson_w));
	m_latch->q_out_cb<3>().set(FUNC(polepos_state::gasel_w));
	m_latch->q_out_cb<4>().set_inputline(m_subcpu, INPUT_LINE_RESET).invert();
	m_latch->q_out_cb<5>().set_inputline(m_subcpu2, INPUT_LINE_RESET).invert();
	m_latch->q_out_cb<6>().set(FUNC(polepos_state::sb0_w));
	m_latch->q_out_cb<7>().set(FUNC(polepos_state::chacl_w));

	ADC0804(config, m_adc, MASTER_CLOCK/8/8);
	m_adc->vin_callback().set(FUNC(polepos_state::analog_r));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MASTER_CLOCK/4, 384, 0, 256, 264, 16, 224+16);
	m_screen->set_screen_update(FUNC(polepos_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set("51xx", FUNC(namco_51xx_device::vblank));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_polepos);
	PALETTE(config, m_palette, FUNC(polepos_state::polepos_palette), 0x0f00, 128);

	config.set_default_layout(layout_topracer);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	NAMCO(config, m_namco_sound, MASTER_CLOCK/512);
	m_namco_sound->set_voices(8);
	m_namco_sound->set_stereo(true);
	m_namco_sound->add_route(0, "lspeaker", 0.80);
	m_namco_sound->add_route(1, "rspeaker", 0.80);

	/* engine sound */
	polepos_sound_device &polepos(POLEPOS_SOUND(config, "polepos", 0));
	polepos.add_route(ALL_OUTPUTS, "lspeaker", 0.90 * 0.77);
	polepos.add_route(ALL_OUTPUTS, "rspeaker", 0.90 * 0.77);

	dac_4bit_r2r_device &dac(DAC_4BIT_R2R(config, "dac", 0)); // unknown resistor configuration
	dac.add_route(ALL_OUTPUTS, "lspeaker", 0.12);
	dac.add_route(ALL_OUTPUTS, "rspeaker", 0.12);
}

void polepos_state::polepos2bi(machine_config &config)
{
	topracern(config);

	Z80(config, m_sound_z80, MASTER_CLOCK/8); /*? MHz */
	m_sound_z80->set_addrmap(AS_PROGRAM, &polepos_state::sound_z80_bootleg_map);
	m_sound_z80->set_addrmap(AS_IO, &polepos_state::sound_z80_bootleg_iomap);

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_sound_z80, INPUT_LINE_NMI);
	m_soundlatch->set_separate_acknowledge(true);

	TMS5220(config, "tms", 600000) /* ? Mhz */
			.add_route(ALL_OUTPUTS, "lspeaker", 0.80)
			.add_route(ALL_OUTPUTS, "rspeaker", 0.80);
}



/*********************************************************************
 * ROM definitions
 *********************************************************************/

/*
    Pole Position - Namco Version
*/

ROM_START( polepos )
	/* Z80 memory/ROM data */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp3_9.6h",    0x0000, 0x2000, CRC(c0511173) SHA1(88a1d4eefacbcf7d0e59edc0110edf225cad15c4) )
	ROM_LOAD( "pp1_10b.5h",  0x2000, 0x1000, CRC(7174bcb7) SHA1(460326a6cea201db2df813013c95562a222ea95d) )

	/* Z8002 #1 memory/ROM data */
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD16_BYTE( "pp3_1.8m",    0x0001, 0x2000, CRC(65c1c2c2) SHA1(69f3e2e871f1cdc1efee91688acad4417683474d) )
	ROM_LOAD16_BYTE( "pp3_2.8l",    0x0000, 0x2000, CRC(fafb9049) SHA1(92424c1042f520af115fb271fc11f4914a346ae2) )

	/* Z8002 #2 memory/ROM data */
	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD16_BYTE( "pp3_5.4m",    0x0001, 0x2000, CRC(46e5c99a) SHA1(d5fd657a9197f1751f6fca430d3ef18d37ed774e) )
	ROM_LOAD16_BYTE( "pp3_6.4l",    0x0000, 0x2000, CRC(acc1ebc3) SHA1(41745f5b6b0af2cb1ee80843194c070eac9e74e7) )

	/* graphics data */
	ROM_REGION( 0x01000, "gfx1", 0 )    /* 2bpp alpha layer */
	ROM_LOAD( "pp3_28.1f",    0x0000, 0x1000, CRC(2e77187e) SHA1(869a7389a684ccedd14868fb03400b1f8088acca) )

	ROM_REGION( 0x01000, "gfx2", 0 )    /* 2bpp view layer */
	ROM_LOAD( "pp1_29.1e",    0x0000, 0x1000, CRC(706e888a) SHA1(af1aa2199fcf73a3afbe760857ff117865350954) )

	ROM_REGION( 0x04000, "gfx3", 0 )    /* 4bpp 16x16 sprites */
	ROM_LOAD( "pp3_25.1n",    0x0000, 0x2000, CRC(b52c086b) SHA1(ea4a58fcc1d829ad0efa13a02f90fadc61e6e0bc) )    /* 4bpp sm sprites, planes 0+1 */
	ROM_LOAD( "pp3_26.1m",    0x2000, 0x2000, CRC(d24a5707) SHA1(468319469bde6b7dc0cf8244299d8dc927059b2d) )    /* 4bpp sm sprites, planes 2+3 */

	ROM_REGION( 0x10000, "gfx4", 0 )    /* 4bpp 32x32 sprites */
	ROM_LOAD( "pp1_17.5n",    0x0000, 0x2000, CRC(2e134b46) SHA1(0938f5f9f5cc6d7c1096c569449db78dbc42da01) )    /* 4bpp lg sprites, planes 0+1 */
	ROM_LOAD( "pp1_19.4n",    0x2000, 0x2000, CRC(43ff83e1) SHA1(8f830549a629b019125e59801e5027e4e4b3c0f2) )
	ROM_LOAD( "pp1_21.3n",    0x4000, 0x2000, CRC(5f958eb4) SHA1(b56d84e5e5e0ddeb0e71851ba66e5fa1b1409551) )
	ROM_LOAD( "pp1_18.5m",    0x8000, 0x2000, CRC(6f9997d2) SHA1(b26d505266ccf23bfd867f881756c3251c80f57b) )    /* 4bpp lg sprites, planes 2+3 */
	ROM_LOAD( "pp1_20.4m",    0xa000, 0x2000, CRC(ec18075b) SHA1(af7be549c5fa47551a8dca4c0a531552147fa50f) )
	ROM_LOAD( "pp1_22.3m",    0xc000, 0x2000, CRC(1d2f30b1) SHA1(1d88a3069e9b15febd2835dd63e5511b3b2a6b45) )

	ROM_REGION( 0x5000, "gfx5", 0 )     /* road generation ROMs needed at runtime */
	ROM_LOAD( "pp1_30.3a",    0x0000, 0x2000, CRC(ee6b3315) SHA1(9cc26c6d3604c0f60d716f86e67e9d9c0487f87d) )    /* road control */
	ROM_LOAD( "pp1_31.2a",    0x2000, 0x2000, CRC(6d1e7042) SHA1(90113ff0c93ed86d95067290088705bb5e6608d1) )    /* road bits 1 */
	ROM_LOAD( "pp1_32.1a",    0x4000, 0x1000, CRC(4e97f101) SHA1(f377d053821c74aee93ebcd30a4d43e6156f3cfe) )    /* road bits 2 */

	ROM_REGION( 0x1000, "gfx6", 0 )     /* sprite scaling */
	ROM_LOAD( "pp1_27.1l",    0x0000, 0x1000, CRC(a61bff15) SHA1(f7a59970831cdaaa7bf59c2221a38e4746c54244) )    /* vertical scaling */

	/* graphics (P)ROM data */
	ROM_REGION( 0x1040, "proms", 0 )
	ROM_LOAD( "pp1-7.8l",    0x0000, 0x0100, CRC(f07ff2ad) SHA1(e1f3cb10a03d23f8c1d422acf271dba4e7b98cb1) )    /* red palette */
	ROM_LOAD( "pp1-8.9l",    0x0100, 0x0100, CRC(adbde7d7) SHA1(956ac5117c1e310f554ac705aa2dc24a796c36a5) )    /* green palette */
	ROM_LOAD( "pp1-9.10l",   0x0200, 0x0100, CRC(ddac786a) SHA1(d1860105bf91297533ccc4aa6775987df198d0fa) )    /* blue palette */
	ROM_LOAD( "pp2-10.2h",   0x0300, 0x0100, CRC(1e8d0491) SHA1(e8bf1db5c1fb04a35763099965cf5c588240bde5) )    /* alpha color - Same as pp1-10.2h - Verified */
	ROM_LOAD( "pp1-11.4d",   0x0400, 0x0100, CRC(0e4fe8a0) SHA1(d330b1e5ebccf5bbefcf71486fd80d816de38196) )    /* background color */
	ROM_LOAD( "pp1-15.9a",   0x0500, 0x0100, CRC(2d502464) SHA1(682b7dd22e51d5db52c0804b7e27e47641dfa6bd) )    /* vertical position low */
	ROM_LOAD( "pp1-16.10a",  0x0600, 0x0100, CRC(027aa62c) SHA1(c7030d8b64b80e107c446f6fbdd63f560c0a91c0) )    /* vertical position med */
	ROM_LOAD( "pp1-17.11a",  0x0700, 0x0100, CRC(1f8d0df3) SHA1(b8f17758f114f5e247b65b3f2922ca2660757e66) )    /* vertical position hi */
	ROM_LOAD( "pp1-12.3c",   0x0800, 0x0400, CRC(7afc7cfc) SHA1(ba2407f6eff124e881b354f13205a4c058b7cf60) )    /* road color */
	ROM_LOAD( "pp3-6.6m",    0x0c00, 0x0400, CRC(63fb6057) SHA1(453fbdfd053c2a026cd41b57d0a71754b69a15da) )    /* sprite color */
	ROM_LOAD( "pp1-13.8e",   0x1000, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */
	ROM_LOAD( "pp1-14.9e",   0x1020, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */

	/* sound (P)ROM data */
	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "pp1-5.3b",    0x0000, 0x0100, CRC(8568decc) SHA1(0aac1fa082858d4d201e21511c609a989f9a1535) )    /* Namco sound */

	ROM_REGION( 0x4000, "engine", 0 )
	ROM_LOAD( "pp1_15.6a",    0x0000, 0x2000, CRC(b5ad4d5f) SHA1(c07e77a050200d6fe9952031f971ca35f4d15ff8) )    /* engine sound */
	ROM_LOAD( "pp1_16.5a",    0x2000, 0x2000, CRC(8fdd2f6f) SHA1(3818dc94c60cd78c4212ab7a4367cf3d98166ee6) )    /* engine sound */

	ROM_REGION( 0x8000, "52xx", 0 )
	ROM_LOAD( "pp2_11.2e",    0x0000, 0x2000, CRC(5b4cf05e) SHA1(52342572940489175607bbf5b6cfd05ee9b0f004) )    /* voice */
	ROM_LOAD( "pp2_12.2f",    0x2000, 0x2000, CRC(32b694c2) SHA1(101d9da28333ca290b0235eefb5ec9b094e1736e) )    /* voice */
	ROM_LOAD( "pp2_13.1e",    0x4000, 0x2000, CRC(8842138a) SHA1(7e94f5b6ee32f6af37df54cfb72d96f9b543f9e2) )    /* voice */
	/* No ROM PPx 14 is present. Empty socket on the PCB */

	/* unknown or unused (P)ROM data */
	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "pp1-4.9h",    0x0000, 0x0100, CRC(2401c817) SHA1(8991b7994513a469e64392fa8f233af5e5f06d54) )    /* sync chain */
ROM_END

ROM_START( poleposj )
	/* Z80 memory/ROM data */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp1_9b.6h",    0x0000, 0x2000, CRC(94436b70) SHA1(7495c2a8c3928c59146760d19e672afee01c5b17) )
	ROM_LOAD( "pp1_10b.5h",   0x2000, 0x1000, CRC(7174bcb7) SHA1(460326a6cea201db2df813013c95562a222ea95d) )

	/* Z8002 #1 memory/ROM data */
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD16_BYTE( "pp1_1b.8m",    0x0001, 0x2000, CRC(361c56dd) SHA1(6e4abf98b10077c6980e8aa3861f0233135ea68f) )
	ROM_LOAD16_BYTE( "pp1_2b.8l",    0x0000, 0x2000, CRC(582b530a) SHA1(4fc38aa8b70816e14b321ec778090f6c7e7f1640) )

	/* Z8002 #2 memory/ROM data */
	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD16_BYTE( "pp1_5b.4m",    0x0001, 0x2000, CRC(5cdf5294) SHA1(dbdf327a541fd71aadafda9c925fa4cf7f7c4a24) )
	ROM_LOAD16_BYTE( "pp1_6b.4l",    0x0000, 0x2000, CRC(81696272) SHA1(27041a7c24297a6f317537c44922b51d2b2278a6) )

	/* graphics data */
	ROM_REGION( 0x01000, "gfx1", 0 )    /* 2bpp alpha layer */
	ROM_LOAD( "pp1_28.1f",    0x0000, 0x1000, CRC(5b277daf) SHA1(0b1feeb2c0c63a5db5ba9b0115aa1b2388636a70) )

	ROM_REGION( 0x01000, "gfx2", 0 )    /* 2bpp view layer */
	ROM_LOAD( "pp1_29.1e",    0x0000, 0x1000, CRC(706e888a) SHA1(af1aa2199fcf73a3afbe760857ff117865350954) )

	ROM_REGION( 0x04000, "gfx3", 0 )    /* 4bpp 16x16 sprites */
	ROM_LOAD( "pp1_25.1n",    0x0000, 0x2000, CRC(ac8e28c1) SHA1(13bc2bf4be28d9ae987f79034f9532272b3a2543) )    /* 4bpp sm sprites, planes 0+1 */
	ROM_LOAD( "pp1_26.1m",    0x2000, 0x2000, CRC(94443079) SHA1(413d7b762c8dff541675e96874be6ee0251d3581) )    /* 4bpp sm sprites, planes 2+3 */

	ROM_REGION( 0x10000, "gfx4", 0 )    /* 4bpp 32x32 sprites */
	ROM_LOAD( "pp1_17.5n",    0x0000, 0x2000, CRC(2e134b46) SHA1(0938f5f9f5cc6d7c1096c569449db78dbc42da01) )    /* 4bpp lg sprites, planes 0+1 */
	ROM_LOAD( "pp1_19.4n",    0x2000, 0x2000, CRC(43ff83e1) SHA1(8f830549a629b019125e59801e5027e4e4b3c0f2) )
	ROM_LOAD( "pp1_21.3n",    0x4000, 0x2000, CRC(5f958eb4) SHA1(b56d84e5e5e0ddeb0e71851ba66e5fa1b1409551) )
	ROM_LOAD( "pp1_18.5m",    0x8000, 0x2000, CRC(6f9997d2) SHA1(b26d505266ccf23bfd867f881756c3251c80f57b) )    /* 4bpp lg sprites, planes 2+3 */
	ROM_LOAD( "pp1_20.4m",    0xa000, 0x2000, CRC(ec18075b) SHA1(af7be549c5fa47551a8dca4c0a531552147fa50f) )
	ROM_LOAD( "pp1_22.3m",    0xc000, 0x2000, CRC(1d2f30b1) SHA1(1d88a3069e9b15febd2835dd63e5511b3b2a6b45) )

	ROM_REGION( 0x5000, "gfx5", 0 )     /* road generation ROMs needed at runtime */
	ROM_LOAD( "pp1_30.3a",    0x0000, 0x2000, CRC(ee6b3315) SHA1(9cc26c6d3604c0f60d716f86e67e9d9c0487f87d) )    /* road control */
	ROM_LOAD( "pp1_31.2a",    0x2000, 0x2000, CRC(6d1e7042) SHA1(90113ff0c93ed86d95067290088705bb5e6608d1) )    /* road bits 1 */
	ROM_LOAD( "pp1_32.1a",    0x4000, 0x1000, CRC(4e97f101) SHA1(f377d053821c74aee93ebcd30a4d43e6156f3cfe) )    /* road bits 2 */

	ROM_REGION( 0x1000, "gfx6", 0 )     /* sprite scaling */
	ROM_LOAD( "pp1_27.1l",    0x0000, 0x1000, CRC(a61bff15) SHA1(f7a59970831cdaaa7bf59c2221a38e4746c54244) )    /* vertical scaling */

	/* graphics (P)ROM data */
	ROM_REGION( 0x1040, "proms", 0 )
	ROM_LOAD( "pp1-7.8l",    0x0000, 0x0100, CRC(f07ff2ad) SHA1(e1f3cb10a03d23f8c1d422acf271dba4e7b98cb1) )    /* red palette */
	ROM_LOAD( "pp1-8.9l",    0x0100, 0x0100, CRC(adbde7d7) SHA1(956ac5117c1e310f554ac705aa2dc24a796c36a5) )    /* green palette */
	ROM_LOAD( "pp1-9.10l",   0x0200, 0x0100, CRC(ddac786a) SHA1(d1860105bf91297533ccc4aa6775987df198d0fa) )    /* blue palette */
	ROM_LOAD( "pp1-10.2h",   0x0300, 0x0100, CRC(1e8d0491) SHA1(e8bf1db5c1fb04a35763099965cf5c588240bde5) )    /* alpha color */
	ROM_LOAD( "pp1-11.4d",   0x0400, 0x0100, CRC(0e4fe8a0) SHA1(d330b1e5ebccf5bbefcf71486fd80d816de38196) )    /* background color */
	ROM_LOAD( "pp1-15.9a",   0x0500, 0x0100, CRC(2d502464) SHA1(682b7dd22e51d5db52c0804b7e27e47641dfa6bd) )    /* vertical position low */
	ROM_LOAD( "pp1-16.10a",  0x0600, 0x0100, CRC(027aa62c) SHA1(c7030d8b64b80e107c446f6fbdd63f560c0a91c0) )    /* vertical position med */
	ROM_LOAD( "pp1-17.11a",  0x0700, 0x0100, CRC(1f8d0df3) SHA1(b8f17758f114f5e247b65b3f2922ca2660757e66) )    /* vertical position hi */
	ROM_LOAD( "pp1-12.3c",   0x0800, 0x0400, CRC(7afc7cfc) SHA1(ba2407f6eff124e881b354f13205a4c058b7cf60) )    /* road color */
	ROM_LOAD( "pp1-6.6m",    0x0c00, 0x0400, CRC(2f1079ee) SHA1(18a27998a78deff13dd198f3668a7e92f084f467) )    /* sprite color */
	ROM_LOAD( "pp1-13.8e",   0x1000, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */
	ROM_LOAD( "pp1-14.9e",   0x1020, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */

	/* sound (P)ROM data */
	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "pp1-5.3b",    0x0000, 0x0100, CRC(8568decc) SHA1(0aac1fa082858d4d201e21511c609a989f9a1535) )    /* Namco sound */

	ROM_REGION( 0x4000, "engine", 0 )
	ROM_LOAD( "pp1_15.6a",    0x0000, 0x2000, CRC(b5ad4d5f) SHA1(c07e77a050200d6fe9952031f971ca35f4d15ff8) )    /* engine sound */
	ROM_LOAD( "pp1_16.5a",    0x2000, 0x2000, CRC(8fdd2f6f) SHA1(3818dc94c60cd78c4212ab7a4367cf3d98166ee6) )    /* engine sound */

	ROM_REGION( 0x8000, "52xx", 0 )
	ROM_LOAD( "pp1_11.2e",    0x0000, 0x2000, CRC(45b9bfeb) SHA1(ff8c690471944d414931fb88666594ef608997f8) )    /* voice */
	ROM_LOAD( "pp1_12.2f",    0x2000, 0x2000, CRC(a31b4be5) SHA1(38298093bb97ea8647fe187359cae05b65e1c616) )    /* voice */
	ROM_LOAD( "pp1_13.1e",    0x4000, 0x2000, CRC(a4237466) SHA1(88a397276038cc2fc05f2c18472e6b7cef167f2e) )    /* voice */
	ROM_LOAD( "pp1_14.1f",    0x6000, 0x2000, CRC(944580f9) SHA1(c76f529cae718674ce97a1a599a3c6eaf6bf561a) )    /* voice */

	/* unknown or unused (P)ROM data */
	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "pp1-4.9h",    0x0000, 0x0100, CRC(2401c817) SHA1(8991b7994513a469e64392fa8f233af5e5f06d54) )    /* sync chain */
ROM_END

/*
    Pole Position - Atari Version

    CPU/Sound Board: A039185
    Video Board:     A039187
*/

ROM_START( poleposa1 )
	// Z80 memory/ROM data
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "136014-105.7h", 0x0000, 0x2000, CRC(c918c043) SHA1(abc1aa3d7b670b5a65b4565dc646cd3c4edf4e6f) )
	ROM_LOAD( "136014-116.7f", 0x2000, 0x1000, CRC(7174bcb7) SHA1(460326a6cea201db2df813013c95562a222ea95d) )

	// Z8002 #1 memory/ROM data
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD16_BYTE( "136014-101.3l", 0x0001, 0x2000, CRC(8c2cf172) SHA1(57c774afab79599ac3f434113c3170fbb3d42620) )
	ROM_LOAD16_BYTE( "136014-102.4l", 0x0000, 0x2000, CRC(51018857) SHA1(ed28d44d172a01f76461f556229d1fe3a1b779a7) )

	// Z8002 #2 memory/ROM data
	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD16_BYTE( "136014-103.3e", 0x0001, 0x2000, CRC(af4fc019) SHA1(1bb6c0f3ffada2e1df72e1767581f8e8bb2b18f9) )
	ROM_LOAD16_BYTE( "136014-104.4e", 0x0000, 0x2000, CRC(ba0045f3) SHA1(aedb8d8c56407963aa4ffb66243288c8fd6d845a) )

	// graphics data
	ROM_REGION( 0x01000, "gfx1", 0 ) // 2bpp alpha layer
	ROM_LOAD( "136014-132.7n", 0x0000, 0x1000, CRC(a949aa85) SHA1(2d6414196b6071101001128418233e585279ffb9) )

	ROM_REGION( 0x01000, "gfx2", 0 ) // 2bpp view layer
	ROM_LOAD( "136014-133.6n", 0x0000, 0x1000, CRC(706e888a) SHA1(af1aa2199fcf73a3afbe760857ff117865350954) )

	ROM_REGION( 0x04000, "gfx3", 0 ) // 4bpp 16x16 sprites
	ROM_LOAD( "136014-125.13n", 0x0000, 0x2000, CRC(e7a09c93) SHA1(47cc5c6776333bba8454a3df9e2f6e7de4a465e1) ) // 4bpp sm sprites, planes 0+1
	ROM_LOAD( "136014-126.12n", 0x2000, 0x2000, CRC(dee7d687) SHA1(ea34b51c91f6915b74a4a7b53ddb4ff36b72bf66) ) // 4bpp sm sprites, planes 2+3

	ROM_REGION( 0x10000, "gfx4", 0 ) // 4bpp 32x32 sprites
	ROM_LOAD( "136014-119.13j", 0x0000, 0x2000, CRC(2e134b46) SHA1(0938f5f9f5cc6d7c1096c569449db78dbc42da01) ) // 4bpp lg sprites, planes 0+1
	ROM_LOAD( "136014-121.13k", 0x2000, 0x2000, CRC(a7e3a1c6) SHA1(b7340318afaa4b5f416fe4444899579242cd36c2) )
	ROM_LOAD( "136014-123.13l", 0x4000, 0x2000, CRC(8992d381) SHA1(3bf2544dbe88132137acec2c064a104a74139ec7) )
	ROM_LOAD( "136014-120.12j", 0x8000, 0x2000, CRC(6f9997d2) SHA1(b26d505266ccf23bfd867f881756c3251c80f57b) ) // bpp lg sprites, planes 2+3
	ROM_LOAD( "136014-122.12k", 0xa000, 0x2000, CRC(6c5c6e68) SHA1(dce74ee0e69e0fc0a1942a489c2065381239f0f1) )
	ROM_LOAD( "136014-124.12l", 0xc000, 0x2000, CRC(111896ad) SHA1(15032b4c859231373bebfa640421fdcc8ba9d211) )

	ROM_REGION( 0x5000, "gfx5", 0 )  // road generation ROMs needed at runtime
	ROM_LOAD( "136014-127.2l", 0x0000, 0x2000, CRC(ee6b3315) SHA1(9cc26c6d3604c0f60d716f86e67e9d9c0487f87d) ) // road control
	ROM_LOAD( "136014-128.2m", 0x2000, 0x2000, CRC(6d1e7042) SHA1(90113ff0c93ed86d95067290088705bb5e6608d1) ) // road bits 1
	ROM_LOAD( "136014-134.2n", 0x4000, 0x1000, CRC(4e97f101) SHA1(f377d053821c74aee93ebcd30a4d43e6156f3cfe) ) // road bits 2

	ROM_REGION( 0x1000, "gfx6", 0 )  // sprite scaling
	ROM_LOAD( "136014-131.11n", 0x0000, 0x1000, CRC(5921777f) SHA1(4d9c91a26e0d84fbbe08f748d6e0364311ed6f73) ) // vertical scaling

	// graphics PROM data
	ROM_REGION( 0x1040, "proms", 0 )
	ROM_LOAD( "136014-137.11e", 0x0000, 0x0100, CRC(f07ff2ad) SHA1(e1f3cb10a03d23f8c1d422acf271dba4e7b98cb1) ) // red palette
	ROM_LOAD( "136014-138.11d", 0x0100, 0x0100, CRC(adbde7d7) SHA1(956ac5117c1e310f554ac705aa2dc24a796c36a5) ) // green palette
	ROM_LOAD( "136014-139.11c", 0x0200, 0x0100, CRC(ddac786a) SHA1(d1860105bf91297533ccc4aa6775987df198d0fa) ) // blue palette
	ROM_LOAD( "136014-140.8m",  0x0300, 0x0100, CRC(1e8d0491) SHA1(e8bf1db5c1fb04a35763099965cf5c588240bde5) ) // alpha color
	ROM_LOAD( "136014-141.5k",  0x0400, 0x0100, CRC(0e4fe8a0) SHA1(d330b1e5ebccf5bbefcf71486fd80d816de38196) ) // background color
	ROM_LOAD( "136014-142.2d",  0x0500, 0x0100, CRC(2d502464) SHA1(682b7dd22e51d5db52c0804b7e27e47641dfa6bd) ) // vertical position low
	ROM_LOAD( "136014-143.2c",  0x0600, 0x0100, CRC(027aa62c) SHA1(c7030d8b64b80e107c446f6fbdd63f560c0a91c0) ) // vertical position med
	ROM_LOAD( "136014-144.2b",  0x0700, 0x0100, CRC(1f8d0df3) SHA1(b8f17758f114f5e247b65b3f2922ca2660757e66) ) // vertical position hi
	ROM_LOAD( "136014-145.4l",  0x0800, 0x0400, CRC(7afc7cfc) SHA1(ba2407f6eff124e881b354f13205a4c058b7cf60) ) // road color
	ROM_LOAD( "136014-146.12h", 0x0c00, 0x0400, CRC(ca4ba741) SHA1(de93d738bd27e24dbc4a8378d2c120ef8388c261) ) // sprite color
	ROM_LOAD( "136014-135.6e",  0x1000, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) ) // video RAM address decoder (not used)
	ROM_LOAD( "136014-136.6d",  0x1020, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) ) // video RAM address decoder (not used)

	// sound PROM data
	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "136014-118.11d", 0x0000, 0x0100, CRC(8568decc) SHA1(0aac1fa082858d4d201e21511c609a989f9a1535) ) // Namco sound

	ROM_REGION( 0x4000, "engine", 0 )
	ROM_LOAD( "136014-110.12f", 0x0000, 0x2000, CRC(b5ad4d5f) SHA1(c07e77a050200d6fe9952031f971ca35f4d15ff8) ) // engine sound
	ROM_LOAD( "136014-111.12e", 0x2000, 0x2000, CRC(8fdd2f6f) SHA1(3818dc94c60cd78c4212ab7a4367cf3d98166ee6) ) // engine sound

	ROM_REGION( 0x6000, "52xx", 0 )
	ROM_LOAD( "136014-106.9c", 0x0000, 0x2000, CRC(5b4cf05e) SHA1(52342572940489175607bbf5b6cfd05ee9b0f004) ) // voice

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "136014-117.7l", 0x0000, 0x0100, CRC(2401c817) SHA1(8991b7994513a469e64392fa8f233af5e5f06d54) ) // sync chain (unused)

	ROM_REGION( 0x0300, "cpu_pals", 0 ) // PAL's located on the cpu board
	ROM_LOAD( "137316-001.2n", 0x0000, 0x0034, CRC(dd37bd15) SHA1(3820203c8d7a64826ed6172be38d51fc70792ccd) ) // MMI PAL12L6CN - manual states 137280-001
	ROM_LOAD( "137316-001.5c", 0x0100, 0x0034, CRC(dd37bd15) SHA1(3820203c8d7a64826ed6172be38d51fc70792ccd) ) // MMI PAL12L6CN - manual states 137280-001
	ROM_LOAD( "137279-001.7c", 0x0200, 0x00eb, CRC(4ddc51ba) SHA1(78e64ef9074bd103662bc78a312e3a2b2b0957cc) ) // N82S153N
ROM_END

ROM_START( poleposa2 )
	// Z80 memory/ROM data
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "136014-105.7h", 0x0000, 0x2000, CRC(c918c043) SHA1(abc1aa3d7b670b5a65b4565dc646cd3c4edf4e6f) )
	ROM_LOAD( "136014-116.7f", 0x2000, 0x1000, CRC(7174bcb7) SHA1(460326a6cea201db2df813013c95562a222ea95d) )

	// Z8002 #1 memory/ROM data
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD16_BYTE( "136014-101.3l", 0x0001, 0x2000, CRC(8c2cf172) SHA1(57c774afab79599ac3f434113c3170fbb3d42620) )
	ROM_LOAD16_BYTE( "136014-102.4l", 0x0000, 0x2000, CRC(51018857) SHA1(ed28d44d172a01f76461f556229d1fe3a1b779a7) )


	// Z8002 #2 memory/ROM data
	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD16_BYTE( "136014-203.3e", 0x0001, 0x2000, CRC(eedea6e7) SHA1(e1459c5e3f824e589e624c3acb18a183fd160df6) )
	ROM_LOAD16_BYTE( "136014-204.4e", 0x0000, 0x2000, CRC(c52c98ed) SHA1(2e33c487deaf8afb941e07e511a9828d2d8f6b31) )

	// graphics data
	ROM_REGION( 0x01000, "gfx1", 0 ) // 2bpp alpha layer
	ROM_LOAD( "136014-132.7n", 0x0000, 0x1000, CRC(a949aa85) SHA1(2d6414196b6071101001128418233e585279ffb9) )

	ROM_REGION( 0x01000, "gfx2", 0 ) // 2bpp view layer
	ROM_LOAD( "136014-133.6n", 0x0000, 0x1000, CRC(706e888a) SHA1(af1aa2199fcf73a3afbe760857ff117865350954) )

	ROM_REGION( 0x04000, "gfx3", 0 ) // 4bpp 16x16 sprites
	ROM_LOAD( "136014-125.13n", 0x0000, 0x2000, CRC(e7a09c93) SHA1(47cc5c6776333bba8454a3df9e2f6e7de4a465e1) ) // 4bpp sm sprites, planes 0+1
	ROM_LOAD( "136014-126.12n", 0x2000, 0x2000, CRC(dee7d687) SHA1(ea34b51c91f6915b74a4a7b53ddb4ff36b72bf66) ) // 4bpp sm sprites, planes 2+3

	ROM_REGION( 0x10000, "gfx4", 0 ) // 4bpp 32x32 sprites
	ROM_LOAD( "136014-119.13j", 0x0000, 0x2000, CRC(2e134b46) SHA1(0938f5f9f5cc6d7c1096c569449db78dbc42da01) ) // bpp lg sprites, planes 0+1
	ROM_LOAD( "136014-121.13k", 0x2000, 0x2000, CRC(a7e3a1c6) SHA1(b7340318afaa4b5f416fe4444899579242cd36c2) )
	ROM_LOAD( "136014-123.13l", 0x4000, 0x2000, CRC(8992d381) SHA1(3bf2544dbe88132137acec2c064a104a74139ec7) )
	ROM_LOAD( "136014-120.12j", 0x8000, 0x2000, CRC(6f9997d2) SHA1(b26d505266ccf23bfd867f881756c3251c80f57b) ) // 4bpp lg sprites, planes 2+3
	ROM_LOAD( "136014-122.12k", 0xa000, 0x2000, CRC(6c5c6e68) SHA1(dce74ee0e69e0fc0a1942a489c2065381239f0f1) )
	ROM_LOAD( "136014-124.12l", 0xc000, 0x2000, CRC(111896ad) SHA1(15032b4c859231373bebfa640421fdcc8ba9d211) )

	ROM_REGION( 0x5000, "gfx5", 0 )  // road generation ROMs needed at runtime
	ROM_LOAD( "136014-127.2l", 0x0000, 0x2000, CRC(ee6b3315) SHA1(9cc26c6d3604c0f60d716f86e67e9d9c0487f87d) ) // road control
	ROM_LOAD( "136014-128.2m", 0x2000, 0x2000, CRC(6d1e7042) SHA1(90113ff0c93ed86d95067290088705bb5e6608d1) ) // road bits 1
	ROM_LOAD( "136014-134.2n", 0x4000, 0x1000, CRC(4e97f101) SHA1(f377d053821c74aee93ebcd30a4d43e6156f3cfe) ) // road bits 2

	ROM_REGION( 0x1000, "gfx6", 0 ) // sprite scaling
	ROM_LOAD( "136014-231.11n", 0x0000, 0x1000, CRC(a61bff15) SHA1(f7a59970831cdaaa7bf59c2221a38e4746c54244) ) // vertical scaling

	// graphics PROM data
	ROM_REGION( 0x1040, "proms", 0 )
	ROM_LOAD( "136014-137.11e", 0x0000, 0x0100, CRC(f07ff2ad) SHA1(e1f3cb10a03d23f8c1d422acf271dba4e7b98cb1) ) // red palette
	ROM_LOAD( "136014-138.11d", 0x0100, 0x0100, CRC(adbde7d7) SHA1(956ac5117c1e310f554ac705aa2dc24a796c36a5) ) // green palette
	ROM_LOAD( "136014-139.11c", 0x0200, 0x0100, CRC(ddac786a) SHA1(d1860105bf91297533ccc4aa6775987df198d0fa) ) // blue palette
	ROM_LOAD( "136014-140.8m",  0x0300, 0x0100, CRC(1e8d0491) SHA1(e8bf1db5c1fb04a35763099965cf5c588240bde5) ) // alpha color
	ROM_LOAD( "136014-141.5k",  0x0400, 0x0100, CRC(0e4fe8a0) SHA1(d330b1e5ebccf5bbefcf71486fd80d816de38196) ) // background color
	ROM_LOAD( "136014-142.2d",  0x0500, 0x0100, CRC(2d502464) SHA1(682b7dd22e51d5db52c0804b7e27e47641dfa6bd) ) // vertical position low
	ROM_LOAD( "136014-143.2c",  0x0600, 0x0100, CRC(027aa62c) SHA1(c7030d8b64b80e107c446f6fbdd63f560c0a91c0) ) // vertical position med
	ROM_LOAD( "136014-144.2b",  0x0700, 0x0100, CRC(1f8d0df3) SHA1(b8f17758f114f5e247b65b3f2922ca2660757e66) ) // vertical position hi
	ROM_LOAD( "136014-145.4l",  0x0800, 0x0400, CRC(7afc7cfc) SHA1(ba2407f6eff124e881b354f13205a4c058b7cf60) ) // road color
	ROM_LOAD( "136014-146.12h", 0x0c00, 0x0400, CRC(ca4ba741) SHA1(de93d738bd27e24dbc4a8378d2c120ef8388c261) ) // sprite color
	ROM_LOAD( "136014-135.6e",  0x1000, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) ) // video RAM address decoder (not used)
	ROM_LOAD( "136014-136.6d",  0x1020, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) ) // video RAM address decoder (not used)

	// sound PROM data
	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "136014-118.11d", 0x0000, 0x0100, CRC(8568decc) SHA1(0aac1fa082858d4d201e21511c609a989f9a1535) ) // Namco sound

	ROM_REGION( 0x4000, "engine", 0 )
	ROM_LOAD( "136014-110.12f", 0x0000, 0x2000, CRC(b5ad4d5f) SHA1(c07e77a050200d6fe9952031f971ca35f4d15ff8) ) // engine sound
	ROM_LOAD( "136014-111.12e", 0x2000, 0x2000, CRC(8fdd2f6f) SHA1(3818dc94c60cd78c4212ab7a4367cf3d98166ee6) ) // engine sound

	ROM_REGION( 0x6000, "52xx", 0 )
	ROM_LOAD( "136014-106.9c", 0x0000, 0x2000, CRC(5b4cf05e) SHA1(52342572940489175607bbf5b6cfd05ee9b0f004) ) // voice

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "136014-117.7l", 0x0000, 0x0100, CRC(2401c817) SHA1(8991b7994513a469e64392fa8f233af5e5f06d54) ) // sync chain (unused)

	ROM_REGION( 0x0300, "cpu_pals", 0 ) // PAL's located on the cpu board
	ROM_LOAD( "137316-001.2n", 0x0000, 0x0034, CRC(dd37bd15) SHA1(3820203c8d7a64826ed6172be38d51fc70792ccd) ) // MMI PAL12L6CN - manual states 137280-001
	ROM_LOAD( "137316-001.5c", 0x0100, 0x0034, CRC(dd37bd15) SHA1(3820203c8d7a64826ed6172be38d51fc70792ccd) ) // MMI PAL12L6CN - manual states 137280-001
	ROM_LOAD( "137279-001.7c", 0x0200, 0x00eb, CRC(4ddc51ba) SHA1(78e64ef9074bd103662bc78a312e3a2b2b0957cc) ) // N82S153N
ROM_END

ROM_START( poleposa1n ) // dumped from an original NAMCO 22089612 (22089632) PCB
	// Z80 memory/ROM data
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp2_9.6h",  0x0000, 0x2000, CRC(c918c043) SHA1(abc1aa3d7b670b5a65b4565dc646cd3c4edf4e6f) )
	ROM_LOAD( "pp2_10.5h", 0x2000, 0x1000, CRC(7174bcb7) SHA1(460326a6cea201db2df813013c95562a222ea95d) )

	// Z8002 #1 memory/ROM data
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD16_BYTE( "pp2_1.8m", 0x0001, 0x2000, CRC(8c2cf172) SHA1(57c774afab79599ac3f434113c3170fbb3d42620) )
	ROM_LOAD16_BYTE( "pp2_2.8l", 0x0000, 0x2000, CRC(51018857) SHA1(ed28d44d172a01f76461f556229d1fe3a1b779a7) )

	// Z8002 #2 memory/ROM data
	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD16_BYTE( "pp2_5.4m", 0x0001, 0x2000, CRC(af4fc019) SHA1(1bb6c0f3ffada2e1df72e1767581f8e8bb2b18f9) )
	ROM_LOAD16_BYTE( "pp2_6.4l", 0x0000, 0x2000, CRC(ba0045f3) SHA1(aedb8d8c56407963aa4ffb66243288c8fd6d845a) )

	// graphics data
	ROM_REGION( 0x01000, "gfx1", 0 ) // 2bpp alpha layer
	ROM_LOAD( "pp2_28.1f", 0x0000, 0x1000, CRC(a949aa85) SHA1(2d6414196b6071101001128418233e585279ffb9) )

	ROM_REGION( 0x01000, "gfx2", 0 ) // 2bpp view layer
	ROM_LOAD( "pp1_29.1e", 0x0000, 0x1000, CRC(706e888a) SHA1(af1aa2199fcf73a3afbe760857ff117865350954) )

	ROM_REGION( 0x04000, "gfx3", 0 ) // 4bpp 16x16 sprites
	ROM_LOAD( "pp2_25.1n", 0x0000, 0x2000, CRC(34c2d310) SHA1(151513da03c49436e16856bfe9666526f81eda71) ) // 4bpp sm sprites, planes 0+1
	ROM_LOAD( "pp2_26.1m", 0x2000, 0x2000, CRC(ba3005f3) SHA1(faef996b1282c97644a0cb503cb6a98a16084d30) ) // 4bpp sm sprites, planes 2+3

	ROM_REGION( 0x10000, "gfx4", 0 ) // 4bpp 32x32 sprites
	ROM_LOAD( "pp2_17.5n", 0x0000, 0x2000, CRC(2e134b46) SHA1(0938f5f9f5cc6d7c1096c569449db78dbc42da01) ) // 4bpp lg sprites, planes 0+1
	ROM_LOAD( "pp2_19.4n", 0x2000, 0x2000, CRC(1c5c05b7) SHA1(2c2e912b7fd030d8c120508195aa6b1f3a6b34fc) )
	ROM_LOAD( "pp2_21.3n", 0x4000, 0x2000, CRC(020f4de5) SHA1(50cb1633f50703fdc941b3183b0d2fac4977f687) )
	ROM_LOAD( "pp2_18.5m", 0x8000, 0x2000, CRC(6f9997d2) SHA1(b26d505266ccf23bfd867f881756c3251c80f57b) ) // bpp lg sprites, planes 2+3
	ROM_LOAD( "pp2_20.4m", 0xa000, 0x2000, CRC(b8a0411c) SHA1(fe6ac41c950004ef729733c6561d6d76ffeecf98) )
	ROM_LOAD( "pp2_22.3m", 0xc000, 0x2000, CRC(4fbf3e94) SHA1(ec7ab91c7db768425e47c625098008f4d7da7bb0) )

	ROM_REGION( 0x5000, "gfx5", 0 )  // road generation ROMs needed at runtime
	ROM_LOAD( "pp1_30.3a", 0x0000, 0x2000, CRC(ee6b3315) SHA1(9cc26c6d3604c0f60d716f86e67e9d9c0487f87d) )    /* road control */
	ROM_LOAD( "pp1_31.2a", 0x2000, 0x2000, CRC(6d1e7042) SHA1(90113ff0c93ed86d95067290088705bb5e6608d1) )    /* road bits 1 */
	ROM_LOAD( "pp1_32.1a", 0x4000, 0x1000, CRC(4e97f101) SHA1(f377d053821c74aee93ebcd30a4d43e6156f3cfe) )    /* road bits 2 */

	ROM_REGION( 0x1000, "gfx6", 0 )  // sprite scaling
	ROM_LOAD( "pp1_27.1l", 0x0000, 0x1000, CRC(a61bff15) SHA1(f7a59970831cdaaa7bf59c2221a38e4746c54244) ) // vertical scaling

	// graphics PROM data
	ROM_REGION( 0x1040, "proms", 0 )
	ROM_LOAD( "pp1-7.8l",   0x0000, 0x0100, CRC(f07ff2ad) SHA1(e1f3cb10a03d23f8c1d422acf271dba4e7b98cb1) ) // red palette
	ROM_LOAD( "pp1-8.9l",   0x0100, 0x0100, CRC(adbde7d7) SHA1(956ac5117c1e310f554ac705aa2dc24a796c36a5) ) // green palette
	ROM_LOAD( "pp1-9.10l",  0x0200, 0x0100, CRC(ddac786a) SHA1(d1860105bf91297533ccc4aa6775987df198d0fa) ) // blue palette
	ROM_LOAD( "pp2-10.2h",  0x0300, 0x0100, CRC(1e8d0491) SHA1(e8bf1db5c1fb04a35763099965cf5c588240bde5) ) // alpha color
	ROM_LOAD( "pp1-11.4d",  0x0400, 0x0100, CRC(0e4fe8a0) SHA1(d330b1e5ebccf5bbefcf71486fd80d816de38196) ) // background color
	ROM_LOAD( "pp1-15.9a",  0x0500, 0x0100, CRC(2d502464) SHA1(682b7dd22e51d5db52c0804b7e27e47641dfa6bd) ) // vertical position low
	ROM_LOAD( "pp1-16.10a", 0x0600, 0x0100, CRC(027aa62c) SHA1(c7030d8b64b80e107c446f6fbdd63f560c0a91c0) ) // vertical position med
	ROM_LOAD( "pp1-17.11a", 0x0700, 0x0100, CRC(1f8d0df3) SHA1(b8f17758f114f5e247b65b3f2922ca2660757e66) ) // vertical position hi
	ROM_LOAD( "pp1-12.3c",  0x0800, 0x0400, CRC(7afc7cfc) SHA1(ba2407f6eff124e881b354f13205a4c058b7cf60) ) // road color
	ROM_LOAD( "pp2-6.6m",   0x0c00, 0x0400, CRC(b448c934) SHA1(8f3f440a922c5c4a1cad17c87662860610c4dd9d) ) // sprite color
	ROM_LOAD( "pp1-13.8e",  0x1000, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) ) // video RAM address decoder (not used)
	ROM_LOAD( "pp1-14.9e",  0x1020, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) ) // video RAM address decoder (not used)

	// sound PROM data
	ROM_REGION( 0x0100, "namco", 0 ) // not dumped for this set
	ROM_LOAD( "136014-118.11d", 0x0000, 0x0100, CRC(8568decc) SHA1(0aac1fa082858d4d201e21511c609a989f9a1535) ) // Namco sound

	ROM_REGION( 0x4000, "engine", 0 ) // not dumped for this set
	ROM_LOAD( "136014-110.12f", 0x0000, 0x2000, CRC(b5ad4d5f) SHA1(c07e77a050200d6fe9952031f971ca35f4d15ff8) ) // engine sound
	ROM_LOAD( "136014-111.12e", 0x2000, 0x2000, CRC(8fdd2f6f) SHA1(3818dc94c60cd78c4212ab7a4367cf3d98166ee6) ) // engine sound

	ROM_REGION( 0x6000, "52xx", 0 ) // not dumped for this set
	ROM_LOAD( "136014-106.9c", 0x0000, 0x2000, CRC(5b4cf05e) SHA1(52342572940489175607bbf5b6cfd05ee9b0f004) ) // voice

	ROM_REGION( 0x0100, "user1", 0 ) // not dumped for this set
	ROM_LOAD( "136014-117.7l", 0x0000, 0x0100, CRC(2401c817) SHA1(8991b7994513a469e64392fa8f233af5e5f06d54) ) // sync chain (unused)

	ROM_REGION( 0x0300, "cpu_pals", 0 ) // PALs located on the CPU board, not dumped for this set
	ROM_LOAD( "137316-001.2n", 0x0000, 0x0034, CRC(dd37bd15) SHA1(3820203c8d7a64826ed6172be38d51fc70792ccd) ) // MMI PAL12L6CN - manual states 137280-001
	ROM_LOAD( "137316-001.5c", 0x0100, 0x0034, CRC(dd37bd15) SHA1(3820203c8d7a64826ed6172be38d51fc70792ccd) ) // MMI PAL12L6CN - manual states 137280-001
	ROM_LOAD( "137279-001.7c", 0x0200, 0x00eb, CRC(4ddc51ba) SHA1(78e64ef9074bd103662bc78a312e3a2b2b0957cc) ) // N82S153N
ROM_END

/*
Top Racer / Pole Position I/II (?)

PCB Layouts
===========

Upper Board
-----------
PP-1126
|----------------------------------------------------------------------------------|
|                LM324   8A   9A                                                   |
|1B DIP28  DIP28  DIP28                                                            |
|                4066                                                              |
|                                                                                  |
|                LM324                                                             |
|        DSW2            4066  4066                                                |
|  MB8841  MB8841  MB8841                82S129.14C                                |
|                                                                                  |
|                                                                                  |
|        DSW1                                                                      |
|                  4066  4066                      82S153.16D                      |
|                                                                                  |
|                                                                                  |
|  MB8842                                                                          |
|           LM324           82S129.9E  2148 2148           Z80   Z8002    Z8002    |
|                                                  |-----daughterboard-------|     |
|                                                  |                         |     |
|                                                  |  82S153.18E   82S153.21E|     |
|                                                  |              20E        |     |
|                                              6116|                         |     |
|                                          ADC0804 |16F  17F      20F   21F  | 23F |
|                                                  |-------------------------|     |
|                                                   16F  17F                       |
|                                          4066                                    |
|                                                                                  |
|                 4093                                                             |
|                                               3.6V_BATT                          |
|---------|----18-way-----|-----------------J2-------|----50-pin cable---|---------|
          |---------------|                          |-------------------|
Notes:
      82S153  - Field Programmable Logic Array (DIP20)
      2148    - 1K x4bit SRAM (DIP18)
      6116    - 2K x8bit SRAM (DIP24)
      ADC0804 - 8bit Microprocessor Compatible A/D Convertor (DIP20)
      J2      - 3 Pin Power Connector
      DIP28   - Unpopulated Sockets
      MB8841  - Fujitsu 4bit Microcontroller (DIP40)
      MB8842  - Fujitsu 4bit Microcontroller (DIP28)
      LM324   - Low Power Quad Operational Amplifier (DIP14)

      Note - All ROMs labelled PP2_U.* are located on the upper PCB.
             All ROMs labelled PP2_D.* are located on the plug-in daughterboard.


Lower Board
-----------

|----------------------------------------------------------------------------------|
| 1A  2A  3A  4A  5A  6A  7A  8A  9A  10A                                          |
|                                                                                  |
|                                 9D                  82S129.13D                   |
|                                                                                  |
|                                                     82S129.13E                   |
|     2148  2148                                                                   |
|                                                     82S129.13F                   |
|            6116  6116  82S137.7H                                                 |
|                                                                                  |
|                                                                                  |
|                                                                                  |
|     2J     6116  6116                                                            |
|                                                                                  |
|                                                                                  |
|                                                                                  |
|     2M                       8M  9M                                              |
|                                                                                  |
|                                                                                  |
|    82S129.2N                                                                     |
|                                                           2114  2114             |
|    82S129.2P   82S137.5P     8P                                                  |
|                                                           2114  2114  82S123.15R |
|    82S129.2S                                                                     |
|                                                           2114  2114  82S123.15S |
|    82S129.2T                                                                     |
|             MB3730 MB3730 MB3730 MB3730      24.576MHz    2114  2114             |
|  82S129.2U   VOL    VOL    VOL    VOL                                            |
|                                                                                  |
|---------|----18-way-----|-----------------J2-------|----50-pin cable---|---------|
          |---------------|                          |-------------------|
Notes:
      2114   - 1K x4bit SRAM (DIP18)
      2148   - 1K x4bit SRAM (DIP18)
      6116   - 2K x8bit SRAM (DIP24)
      MB3730 - 12W Power Amp IC (SIP7)
      J2     - 3 Pin Power Connector
      VOL    - Volume Potentiometer

      Note - All ROMs labelled PP2_L.* are located on the lower PCB.
*/
ROM_START( topracer )
	/* Z80 memory/ROM data */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp1_9b.6h",    0x0000, 0x2000, CRC(94436b70) SHA1(7495c2a8c3928c59146760d19e672afee01c5b17) )
	ROM_LOAD( "136014.116",   0x2000, 0x1000, CRC(7174bcb7) SHA1(460326a6cea201db2df813013c95562a222ea95d) )

	/* Z8002 #1 memory/ROM data */
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD16_BYTE( "tr1b.bin",     0x0001, 0x2000, CRC(127f0750) SHA1(97ae6c6f8086187c7cdb8bff5fec94914791890b) )
	ROM_LOAD16_BYTE( "tr2b.bin",     0x0000, 0x2000, CRC(6bd4ff6b) SHA1(cf992de39a8cf7804961a8e6773fc4f7feb1878b) )

	/* Z8002 #2 memory/ROM data */
	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD16_BYTE( "tr5b.bin",     0x0001, 0x2000, CRC(4e5f7b9c) SHA1(d26b1f24dd9ef00388987890bc5b95d4db403815) )
	ROM_LOAD16_BYTE( "tr6b.bin",     0x0000, 0x2000, CRC(9d038ada) SHA1(7a9496c3fb93fd1945393656f8510a0c6421a9ab) )

	/* graphics data */
	ROM_REGION( 0x01000, "gfx1", 0 )    /* 2bpp alpha layer */
	ROM_LOAD( "tr28.bin",     0x0000, 0x1000, CRC(b8217c96) SHA1(aba311bc3c4b118ba322a00e33e2d5cbe7bc6e4a) )

	ROM_REGION( 0x01000, "gfx2", 0 )    /* 2bpp view layer */
	ROM_LOAD( "tr29.bin",     0x0000, 0x1000, CRC(c6e15c21) SHA1(e2a70b3f7ce51a003068eb75d9fe82548f0206d7) )

	ROM_REGION( 0x04000, "gfx3", 0 )    /* 4bpp 16x16 sprites */
	ROM_LOAD( "trus25.bin",   0x0000, 0x2000, CRC(9e1a9c3b) SHA1(deca026c39093119985d1486ed61abc3e6e5705c) )    /* 4bpp sm sprites, planes 0+1 */
	ROM_LOAD( "trus26.bin",   0x2000, 0x2000, CRC(3b39a176) SHA1(d04c9c2c9129c8dd7d7eab24c43502b67162407c) )    /* 4bpp sm sprites, planes 2+3 */

	ROM_REGION( 0x10000, "gfx4", 0 )    /* 4bpp 32x32 sprites */
	ROM_LOAD( "pp17.bin",     0x0000, 0x2000, CRC(613ab0df) SHA1(88aa4500275aae010fc9783c1d8d843feab89afa) )    /* 4bpp lg sprites, planes 0+1 */
	ROM_LOAD( "tr19.bin",     0x2000, 0x2000, CRC(f8e7f551) SHA1(faa23c55bc43325e6f71936be970f2ca144697d8) )
	ROM_LOAD( "tr21.bin",     0x4000, 0x2000, CRC(17c798b0) SHA1(ae2047bc0e4e8c85e1de09c39c200ea8f7c6a72e) )
	ROM_LOAD( "pp18.bin",     0x8000, 0x2000, CRC(5fd933e3) SHA1(5b27a8519234c935308f943cd58abc1efc463726) )    /* 4bpp lg sprites, planes 2+3 */
	ROM_LOAD( "tr20.bin",     0xa000, 0x2000, CRC(7053e219) SHA1(97700fbe887e2d11c9f9a0937147725f6787f081) )
//  ROM_LOAD( "tr22.bin",     0xc000, 0x2000, CRC(f48917b2) SHA1(2823cfc33ae97ef979d92e2eeeb94c95f1f3d9f3) )    /* differs by one bit, almost certainly bitrot */
	ROM_LOAD( "tr22.bin",     0xc000, 0x2000, CRC(5fe9b365) SHA1(1a3ac099a6bb506a5f71c12c6fb14d014172371c) )

	ROM_REGION( 0x5000, "gfx5", 0 )     /* road generation ROMs needed at runtime */
	ROM_LOAD( "136014.158",   0x0000, 0x2000, CRC(ee6b3315) SHA1(9cc26c6d3604c0f60d716f86e67e9d9c0487f87d) )    /* road control */
	ROM_LOAD( "136014.159",   0x2000, 0x2000, CRC(6d1e7042) SHA1(90113ff0c93ed86d95067290088705bb5e6608d1) )    /* road bits 1 */
	ROM_LOAD( "136014.134",   0x4000, 0x1000, CRC(4e97f101) SHA1(f377d053821c74aee93ebcd30a4d43e6156f3cfe) )    /* road bits 2 */

	ROM_REGION( 0x1000, "gfx6", 0 )     /* sprite scaling */
	ROM_LOAD( "136014.231",   0x0000, 0x1000, CRC(a61bff15) SHA1(f7a59970831cdaaa7bf59c2221a38e4746c54244) )    /* vertical scaling */

	/* graphics (P)ROM data */
	ROM_REGION( 0x1040, "proms", 0 )
	ROM_LOAD( "136014.137",   0x0000, 0x0100, CRC(f07ff2ad) SHA1(e1f3cb10a03d23f8c1d422acf271dba4e7b98cb1) )    /* red palette */
	ROM_LOAD( "136014.138",   0x0100, 0x0100, CRC(adbde7d7) SHA1(956ac5117c1e310f554ac705aa2dc24a796c36a5) )    /* green palette */
	ROM_LOAD( "136014.139",   0x0200, 0x0100, CRC(ddac786a) SHA1(d1860105bf91297533ccc4aa6775987df198d0fa) )    /* blue palette */
	ROM_LOAD( "7052-10.h15",  0x0300, 0x0100, CRC(5af3f710) SHA1(da13d17acf8abd0f6ebb4b51b23c3324c6197b7d) )    /* alpha color */
	ROM_LOAD( "7052-11.j15",  0x0400, 0x0100, CRC(8c90e36e) SHA1(2646288d9e0f86300da7f06e1dc0595673205bb4) )    /* background color */
	ROM_LOAD( "136014.142",   0x0500, 0x0100, CRC(2d502464) SHA1(682b7dd22e51d5db52c0804b7e27e47641dfa6bd) )    /* vertical position low */
	ROM_LOAD( "136014.143",   0x0600, 0x0100, CRC(027aa62c) SHA1(c7030d8b64b80e107c446f6fbdd63f560c0a91c0) )    /* vertical position med */
	ROM_LOAD( "136014.144",   0x0700, 0x0100, CRC(1f8d0df3) SHA1(b8f17758f114f5e247b65b3f2922ca2660757e66) )    /* vertical position hi */
	ROM_LOAD( "136014.145",   0x0800, 0x0400, CRC(7afc7cfc) SHA1(ba2407f6eff124e881b354f13205a4c058b7cf60) )    /* road color */
	ROM_LOAD( "7122.e7",      0x0c00, 0x0400, CRC(2f1079ee) SHA1(18a27998a78deff13dd198f3668a7e92f084f467) )    /* sprite color */
	ROM_LOAD( "136014.135",   0x1000, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */
	ROM_LOAD( "136014.136",   0x1020, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */

	/* sound (P)ROM data */
	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "136014.118",   0x0000, 0x0100, CRC(8568decc) SHA1(0aac1fa082858d4d201e21511c609a989f9a1535) )    /* Namco sound */

	ROM_REGION( 0x4000, "engine", 0 )
	ROM_LOAD( "136014.110",   0x0000, 0x2000, CRC(b5ad4d5f) SHA1(c07e77a050200d6fe9952031f971ca35f4d15ff8) )    /* engine sound */
	ROM_LOAD( "136014.111",   0x2000, 0x2000, CRC(8fdd2f6f) SHA1(3818dc94c60cd78c4212ab7a4367cf3d98166ee6) )    /* engine sound */

	ROM_REGION( 0x6000, "52xx", 0 )
	ROM_LOAD( "136014.106",   0x0000, 0x2000, CRC(5b4cf05e) SHA1(52342572940489175607bbf5b6cfd05ee9b0f004) )    /* voice */

	/* unknown or unused (P)ROM data */
	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "7052-4.c14",   0x0000, 0x0100, CRC(0e742cb1) SHA1(3ae43270aab4848fdeece1648e7e040ab216b08e) )    /* sync chain */
ROM_END

ROM_START( topracera )
	/* Z80 memory/ROM data */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tr9.f17",    0x0000, 0x2000, CRC(94436b70) SHA1(7495c2a8c3928c59146760d19e672afee01c5b17) )
	ROM_LOAD( "tr10.f16",   0x2000, 0x1000, CRC(7174bcb7) SHA1(460326a6cea201db2df813013c95562a222ea95d) )

	/* Z8002 #1 memory/ROM data */
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD16_BYTE( "tr1b.f11",    0x0001, 0x2000, CRC(127f0750) SHA1(97ae6c6f8086187c7cdb8bff5fec94914791890b) )
	ROM_LOAD16_BYTE( "tr2b.f8",     0x0000, 0x2000, CRC(6bd4ff6b) SHA1(cf992de39a8cf7804961a8e6773fc4f7feb1878b) )

	/* Z8002 #2 memory/ROM data */
	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD16_BYTE( "tr5b.f7",     0x0001, 0x2000, CRC(4e5f7b9c) SHA1(d26b1f24dd9ef00388987890bc5b95d4db403815) )
	ROM_LOAD16_BYTE( "tr6b.f5",     0x0000, 0x2000, CRC(b3641d0c) SHA1(38ce172b2e38895749cbd3cc1c0e2c0fe8be744a) )

	/* graphics data */
	ROM_REGION( 0x01000, "gfx1", 0 )    /* 2bpp alpha layer */
	ROM_LOAD( "tr28.j9",      0x0000, 0x1000, CRC(b8217c96) SHA1(aba311bc3c4b118ba322a00e33e2d5cbe7bc6e4a) )

	ROM_REGION( 0x01000, "gfx2", 0 )    /* 2bpp view layer */
	ROM_LOAD( "tr29.k9",      0x0000, 0x1000, CRC(c6e15c21) SHA1(e2a70b3f7ce51a003068eb75d9fe82548f0206d7) )

	ROM_REGION( 0x04000, "gfx3", 0 )    /* 4bpp 16x16 sprites */
	ROM_LOAD( "tr25.d5",      0x0000, 0x2000, CRC(9e1a9c3b) SHA1(deca026c39093119985d1486ed61abc3e6e5705c) )    /* 4bpp sm sprites, planes 0+1 */
	ROM_LOAD( "tr26.d8",      0x2000, 0x2000, CRC(3b39a176) SHA1(d04c9c2c9129c8dd7d7eab24c43502b67162407c) )    /* 4bpp sm sprites, planes 2+3 */

	ROM_REGION( 0x10000, "gfx4", 0 )    /* 4bpp 32x32 sprites */
	ROM_LOAD( "tr17.a5",      0x0000, 0x2000, CRC(613ab0df) SHA1(88aa4500275aae010fc9783c1d8d843feab89afa) )    /* 4bpp lg sprites, planes 0+1 */
	ROM_LOAD( "tr19.b5",      0x2000, 0x2000, CRC(f8e7f551) SHA1(faa23c55bc43325e6f71936be970f2ca144697d8) )
	ROM_LOAD( "tr21.c5",      0x4000, 0x2000, CRC(17c798b0) SHA1(ae2047bc0e4e8c85e1de09c39c200ea8f7c6a72e) )
	ROM_LOAD( "tr18.a8",      0x8000, 0x2000, CRC(5fd933e3) SHA1(5b27a8519234c935308f943cd58abc1efc463726) )    /* 4bpp lg sprites, planes 2+3 */
	ROM_LOAD( "tr20.b8",      0xa000, 0x2000, CRC(7053e219) SHA1(97700fbe887e2d11c9f9a0937147725f6787f081) )
	ROM_LOAD( "tr22.c8",      0xc000, 0x2000, CRC(5fe9b365) SHA1(1a3ac099a6bb506a5f71c12c6fb14d014172371c) )

	ROM_REGION( 0x5000, "gfx5", 0 )     /* road generation ROMs needed at runtime */
	ROM_LOAD( "tr30.b15",     0x0000, 0x2000, CRC(ee6b3315) SHA1(9cc26c6d3604c0f60d716f86e67e9d9c0487f87d) )    /* road control */
	ROM_LOAD( "tr31.a15",     0x2000, 0x2000, CRC(6d1e7042) SHA1(90113ff0c93ed86d95067290088705bb5e6608d1) )    /* road bits 1 */
	ROM_LOAD( "tr32.c15",     0x4000, 0x1000, CRC(4e97f101) SHA1(f377d053821c74aee93ebcd30a4d43e6156f3cfe) )    /* road bits 2 */

	ROM_REGION( 0x1000, "gfx6", 0 )     /* sprite scaling */
	ROM_LOAD( "tr27.d3",      0x0000, 0x1000, CRC(a61bff15) SHA1(f7a59970831cdaaa7bf59c2221a38e4746c54244) )    /* vertical scaling */

	/* graphics (P)ROM data */
	ROM_REGION( 0x1040, "proms", 0 )
	ROM_LOAD( "7052-7.k21",   0x0000, 0x0100, CRC(f07ff2ad) SHA1(e1f3cb10a03d23f8c1d422acf271dba4e7b98cb1) )    /* red palette */
	ROM_LOAD( "7052-8.k20",   0x0100, 0x0100, CRC(adbde7d7) SHA1(956ac5117c1e310f554ac705aa2dc24a796c36a5) )    /* green palette */
	ROM_LOAD( "7052-9.k19",   0x0200, 0x0100, CRC(ddac786a) SHA1(d1860105bf91297533ccc4aa6775987df198d0fa) )    /* blue palette */
	ROM_LOAD( "7052-10.h15",  0x0300, 0x0100, CRC(5af3f710) SHA1(da13d17acf8abd0f6ebb4b51b23c3324c6197b7d) )    /* alpha color */
	ROM_LOAD( "7052-11.j15",  0x0400, 0x0100, CRC(8c90e36e) SHA1(2646288d9e0f86300da7f06e1dc0595673205bb4) )    /* background color */
	ROM_LOAD( "7052-15.d1",   0x0500, 0x0100, CRC(2d502464) SHA1(682b7dd22e51d5db52c0804b7e27e47641dfa6bd) )    /* vertical position low */
	ROM_LOAD( "7052-16.d2",   0x0600, 0x0100, CRC(027aa62c) SHA1(c7030d8b64b80e107c446f6fbdd63f560c0a91c0) )    /* vertical position med */
	ROM_LOAD( "7052-17.d3",   0x0700, 0x0100, CRC(1f8d0df3) SHA1(b8f17758f114f5e247b65b3f2922ca2660757e66) )    /* vertical position hi */
	ROM_LOAD( "7122.a19",     0x0800, 0x0400, CRC(7afc7cfc) SHA1(ba2407f6eff124e881b354f13205a4c058b7cf60) )    /* road color */
	ROM_LOAD( "7122.e7",      0x0c00, 0x0400, CRC(2f1079ee) SHA1(18a27998a78deff13dd198f3668a7e92f084f467) )    /* sprite color */
	ROM_LOAD( "7051-13.l7",   0x1000, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */
	ROM_LOAD( "7051-14.l8",   0x1020, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */

	/* sound (P)ROM data */
	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "7052-5.e9",    0x0000, 0x0100, CRC(8568decc) SHA1(0aac1fa082858d4d201e21511c609a989f9a1535) )    /* Namco sound */

	ROM_REGION( 0x4000, "engine", 0 )
	ROM_LOAD( "tr15.a8",      0x0000, 0x2000, CRC(b5ad4d5f) SHA1(c07e77a050200d6fe9952031f971ca35f4d15ff8) )    /* engine sound */
	ROM_LOAD( "tr16.b9",      0x2000, 0x2000, CRC(8fdd2f6f) SHA1(3818dc94c60cd78c4212ab7a4367cf3d98166ee6) )    /* engine sound */

	ROM_REGION( 0x6000, "52xx", 0 )
	ROM_LOAD( "tr11.b1",      0x0000, 0x2000, CRC(5b4cf05e) SHA1(52342572940489175607bbf5b6cfd05ee9b0f004) )    /* voice */

	/* unknown or unused (P)ROM data */
	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "7052-4.c14",   0x0000, 0x0100, CRC(0e742cb1) SHA1(3ae43270aab4848fdeece1648e7e040ab216b08e) )    /* sync chain */
ROM_END



ROM_START( ppspeed )
	/* Z80 memory/ROM data */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tr9b.bin",   0x0000, 0x2000, CRC(538bd0cb) SHA1(36a0628ce735c76e5db83d195af3a9bed5155c60) )
	ROM_LOAD( "tr10.bin",   0x2000, 0x1000, CRC(7174bcb7) SHA1(460326a6cea201db2df813013c95562a222ea95d) )

	/* Z8002 #1 memory/ROM data */
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD16_BYTE( "tr1b.bin",  0x0001, 0x2000, CRC(127f0750) SHA1(97ae6c6f8086187c7cdb8bff5fec94914791890b) )
	ROM_LOAD16_BYTE( "tr2b.bin",  0x0000, 0x2000, CRC(6bd4ff6b) SHA1(cf992de39a8cf7804961a8e6773fc4f7feb1878b) )

	/* Z8002 #2 memory/ROM data */
	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD16_BYTE( "tr5b.bin",  0x0001, 0x2000, CRC(4e5f7b9c) SHA1(d26b1f24dd9ef00388987890bc5b95d4db403815) )
	ROM_LOAD16_BYTE( "tr6b.bin",  0x0000, 0x2000, CRC(b3641d0c) SHA1(38ce172b2e38895749cbd3cc1c0e2c0fe8be744a) ) // sldh

	/* graphics data */
	ROM_REGION( 0x01000, "gfx1", 0 )    /* 2bpp alpha layer */
	ROM_LOAD( "tr28.bin",     0x0000, 0x1000, CRC(cd80b4c3) SHA1(5f237c1e7eb94ecb2680270afdf31c8e111164c5) ) // sldh

	ROM_REGION( 0x01000, "gfx2", 0 )    /* 2bpp view layer */
	ROM_LOAD( "tr29.bin",     0x0000, 0x1000, CRC(c6e15c21) SHA1(e2a70b3f7ce51a003068eb75d9fe82548f0206d7) )

	ROM_REGION( 0x04000, "gfx3", 0 )    /* 4bpp 16x16 sprites */
	ROM_LOAD( "tr25.bin",     0x0000, 0x2000, CRC(f44d33c1) SHA1(e09bcc127e61b351e99c54bf0e3cbab8583949ec) ) /* 4bpp sm sprites, planes 0+1 */
	ROM_LOAD( "tr26.bin",     0x2000, 0x2000, CRC(87e8482d) SHA1(3f1c7f0f9b27e8b61e62db55dd4332c75dc31558) ) /* 4bpp sm sprites, planes 2+3 */

	ROM_REGION( 0x10000, "gfx4", 0 )    /* 4bpp 32x32 sprites */
	ROM_LOAD( "tr17.bin",     0x0000, 0x2000, CRC(613ab0df) SHA1(88aa4500275aae010fc9783c1d8d843feab89afa) ) /* 4bpp lg sprites, planes 0+1 */
	ROM_LOAD( "tr19.bin",     0x2000, 0x2000, CRC(1ea04ccd) SHA1(1cec1e4d0f47365245d92489f78d3edd4f23481f) ) // sldh
	ROM_LOAD( "tr21.bin",     0x4000, 0x2000, CRC(bd7b4a62) SHA1(66175a9382f627053097f0bc9a3fd49a26f8ac8f) ) // sldh
	ROM_LOAD( "tr18.bin",     0x8000, 0x2000, CRC(5fd933e3) SHA1(5b27a8519234c935308f943cd58abc1efc463726) ) /* 4bpp lg sprites, planes 2+3 */
	ROM_LOAD( "tr20.bin",     0xa000, 0x2000, CRC(c572c6ed) SHA1(c2398a82a57be92a0bdc58330504e821878492ea) ) // sldh
	ROM_LOAD( "tr22.bin",     0xc000, 0x2000, CRC(db1bcdd8) SHA1(ffd4edd8c02914a0a85cd7e39153c27d79526457) ) // sldh

	ROM_REGION( 0x5000, "gfx5", 0 )     /* road generation ROMs needed at runtime */
	ROM_LOAD( "tr30.bin",     0x0000, 0x2000, CRC(ee6b3315) SHA1(9cc26c6d3604c0f60d716f86e67e9d9c0487f87d) ) /* road control */
	ROM_LOAD( "tr31.bin",     0x2000, 0x2000, CRC(6d1e7042) SHA1(90113ff0c93ed86d95067290088705bb5e6608d1) ) /* road bits 1 */
	ROM_LOAD( "tr32.bin",     0x4000, 0x1000, CRC(4e97f101) SHA1(f377d053821c74aee93ebcd30a4d43e6156f3cfe) ) /* road bits 2 */

	ROM_REGION( 0x1000, "gfx6", 0 )     /* sprite scaling */
	ROM_LOAD( "tr27.bin",     0x0000, 0x1000, CRC(a61bff15) SHA1(f7a59970831cdaaa7bf59c2221a38e4746c54244) ) /* vertical scaling */

	ROM_REGION( 0x4000, "engine", 0 )
	ROM_LOAD( "tr15.bin",     0x0000, 0x2000, CRC(b5ad4d5f) SHA1(c07e77a050200d6fe9952031f971ca35f4d15ff8) ) /* engine sound */
	ROM_LOAD( "tr16.bin",     0x2000, 0x2000, CRC(8fdd2f6f) SHA1(3818dc94c60cd78c4212ab7a4367cf3d98166ee6) ) /* engine sound */


	// nothing below was verified on this boardset, assumed to be the same

	/* graphics (P)ROM data */
	ROM_REGION( 0x1040, "proms", 0 )
	ROM_LOAD( "7052-7.k21",   0x0000, 0x0100, CRC(f07ff2ad) SHA1(e1f3cb10a03d23f8c1d422acf271dba4e7b98cb1) )    /* red palette */
	ROM_LOAD( "7052-8.k20",   0x0100, 0x0100, CRC(adbde7d7) SHA1(956ac5117c1e310f554ac705aa2dc24a796c36a5) )    /* green palette */
	ROM_LOAD( "7052-9.k19",   0x0200, 0x0100, CRC(ddac786a) SHA1(d1860105bf91297533ccc4aa6775987df198d0fa) )    /* blue palette */
	ROM_LOAD( "7052-10.h15",  0x0300, 0x0100, CRC(5af3f710) SHA1(da13d17acf8abd0f6ebb4b51b23c3324c6197b7d) )    /* alpha color */
	ROM_LOAD( "7052-11.j15",  0x0400, 0x0100, CRC(8c90e36e) SHA1(2646288d9e0f86300da7f06e1dc0595673205bb4) )    /* background color */
	ROM_LOAD( "7052-15.d1",   0x0500, 0x0100, CRC(2d502464) SHA1(682b7dd22e51d5db52c0804b7e27e47641dfa6bd) )    /* vertical position low */
	ROM_LOAD( "7052-16.d2",   0x0600, 0x0100, CRC(027aa62c) SHA1(c7030d8b64b80e107c446f6fbdd63f560c0a91c0) )    /* vertical position med */
	ROM_LOAD( "7052-17.d3",   0x0700, 0x0100, CRC(1f8d0df3) SHA1(b8f17758f114f5e247b65b3f2922ca2660757e66) )    /* vertical position hi */
	ROM_LOAD( "7122.a19",     0x0800, 0x0400, CRC(7afc7cfc) SHA1(ba2407f6eff124e881b354f13205a4c058b7cf60) )    /* road color */
	ROM_LOAD( "7122.e7",      0x0c00, 0x0400, CRC(2f1079ee) SHA1(18a27998a78deff13dd198f3668a7e92f084f467) )    /* sprite color */
	ROM_LOAD( "7051-13.l7",   0x1000, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */
	ROM_LOAD( "7051-14.l8",   0x1020, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */

	/* sound (P)ROM data */
	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "7052-5.e9",    0x0000, 0x0100, CRC(8568decc) SHA1(0aac1fa082858d4d201e21511c609a989f9a1535) )    /* Namco sound */

	ROM_REGION( 0x6000, "52xx", 0 )
	ROM_LOAD( "tr11.b1",      0x0000, 0x2000, CRC(5b4cf05e) SHA1(52342572940489175607bbf5b6cfd05ee9b0f004) )    /* voice */

	/* unknown or unused (P)ROM data */
	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "7052-4.c14",   0x0000, 0x0100, CRC(0e742cb1) SHA1(3ae43270aab4848fdeece1648e7e040ab216b08e) )    /* sync chain */
ROM_END



/* Top Racer (bootleg without MB8841 / MB8842)

CPU Lower Board (label "1081-C")
2x ZILOG Z8002PS (DIP40)
1x NEC D708C-1 (DIP40)

Upper Board (label "1080-C")
1x oscillator 24.576 MHz
ROMs    Lower Board (label "1081-C")
7x HN482764G (1a,a2,a3,a4,a5,a,b)
1x HN482732G (a6)
1x PROM 82S129
2x PAL 12L6
1x PAL 16L8

Upper Board (label "1080-C")
8x HN482764G (1,2,3,6,7,8,9,10)
2x AM2764 (5,11)
4x TMS2732JL (12,13,14,15)
8x PROM 24S10
2x PROM 7603
2x PROM 7643
Note    Lower Board (label "1081-C")
1x flat cable connector to upper
1x 18x2 edge connector

Upper Board (label "1080-C")
1x flat cable connector to lower
1x 18x2 edge connector

*/

ROM_START( topracern )
	/* Z80 memory/ROM data */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a5.bin",      0x0000, 0x2000, CRC(caab829a) SHA1(826f25f5c792ab8b24e73ebb735aebcad552454f) )
	ROM_LOAD( "a6.bin",      0x2000, 0x1000, CRC(148f5000) SHA1(071f75518f06a317f53db78f11da3ee878569f86) )

	/* Z8002 #1 memory/ROM data */
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD16_BYTE( "a1.bin",     0x0001, 0x2000, CRC(127f0750) SHA1(97ae6c6f8086187c7cdb8bff5fec94914791890b) )
	ROM_LOAD16_BYTE( "a2.bin",     0x0000, 0x2000, CRC(6bd4ff6b) SHA1(cf992de39a8cf7804961a8e6773fc4f7feb1878b) )

	/* Z8002 #2 memory/ROM data */
	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD16_BYTE( "a3.bin",     0x0001, 0x2000, CRC(4e5f7b9c) SHA1(d26b1f24dd9ef00388987890bc5b95d4db403815) )
	//ROM_LOAD16_BYTE( "a4.bin",     0x0000, 0x2000, CRC(b3641d0c) SHA1(38ce172b2e38895749cbd3cc1c0e2c0fe8be744a) ) /* one set had this rom, which matches topracera */
	ROM_LOAD16_BYTE( "pole-d",       0x0000, 0x2000, CRC(932bb5a7) SHA1(8045fe1f9b4b1973ec0d6705adf3ba3891bddaa1) ) /* the other set had this one, what's the difference? */

	/* graphics data */
	ROM_REGION( 0x01000, "gfx1", 0 )    /* 2bpp alpha layer */
	ROM_LOAD( "13.bin",     0x0000, 0x1000, CRC(b8217c96) SHA1(aba311bc3c4b118ba322a00e33e2d5cbe7bc6e4a) )

	ROM_REGION( 0x01000, "gfx2", 0 )    /* 2bpp view layer */
	ROM_LOAD( "12.bin",     0x0000, 0x1000, CRC(c6e15c21) SHA1(e2a70b3f7ce51a003068eb75d9fe82548f0206d7) )

	ROM_REGION( 0x04000, "gfx3", 0 )    /* 4bpp 16x16 sprites */
	ROM_LOAD( "5.bin",       0x0000, 0x2000, CRC(301117d2) SHA1(0d8be9e50da4601963a8392aa3e0f3414e721fa1) )    /* 4bpp sm sprites, planes 0+1 */
	ROM_LOAD( "6.bin",       0x2000, 0x2000, CRC(3c9db014) SHA1(c26098dd78803e699845fefa92bf034c38259cea) )    /* 4bpp sm sprites, planes 2+3 */

	ROM_REGION( 0x10000, "gfx4", 0 )    /* 4bpp 32x32 sprites */
	ROM_LOAD( "1.bin",     0x0000, 0x2000, CRC(613ab0df) SHA1(88aa4500275aae010fc9783c1d8d843feab89afa) )    /* 4bpp lg sprites, planes 0+1 */
	ROM_LOAD( "2.bin",     0x2000, 0x2000, CRC(f8e7f551) SHA1(faa23c55bc43325e6f71936be970f2ca144697d8) )
	//ROM_LOAD( "3.bin",   0x4000, 0x2000, CRC(94d0b00c) SHA1(24e4004d1091292afda76bdfb007f08c13778675) ) // this one has more corrupt lines in one of the flags
	ROM_LOAD( "3.bin",     0x4000, 0x2000, CRC(17c798b0) SHA1(ae2047bc0e4e8c85e1de09c39c200ea8f7c6a72e) )
	ROM_LOAD( "7.bin",     0x8000, 0x2000, CRC(5fd933e3) SHA1(5b27a8519234c935308f943cd58abc1efc463726) )    /* 4bpp lg sprites, planes 2+3 */
	ROM_LOAD( "8.bin",     0xa000, 0x2000, CRC(7053e219) SHA1(97700fbe887e2d11c9f9a0937147725f6787f081) )
	ROM_LOAD( "9.bin",     0xc000, 0x2000, CRC(5fe9b365) SHA1(1a3ac099a6bb506a5f71c12c6fb14d014172371c) )

	ROM_REGION( 0x5000, "gfx5", 0 )     /* road generation ROMs needed at runtime */
	ROM_LOAD( "11.bin",   0x0000, 0x2000, CRC(ee6b3315) SHA1(9cc26c6d3604c0f60d716f86e67e9d9c0487f87d) )    /* road control */
	ROM_LOAD( "10.bin",   0x2000, 0x2000, CRC(6d1e7042) SHA1(90113ff0c93ed86d95067290088705bb5e6608d1) )    /* road bits 1 */
	ROM_LOAD( "14.bin",   0x4000, 0x1000, CRC(4e97f101) SHA1(f377d053821c74aee93ebcd30a4d43e6156f3cfe) )    /* road bits 2 */

	ROM_REGION( 0x1000, "gfx6", 0 )     /* sprite scaling */
	ROM_LOAD( "15.bin",   0x0000, 0x1000, CRC(a61bff15) SHA1(f7a59970831cdaaa7bf59c2221a38e4746c54244) )    /* vertical scaling */

	/* graphics (P)ROM data */
	ROM_REGION( 0x1040, "proms", 0 )
	ROM_LOAD( "24s10.5",  0x0000, 0x0100, CRC(f07ff2ad) SHA1(e1f3cb10a03d23f8c1d422acf271dba4e7b98cb1) )    /* red palette */
	ROM_LOAD( "24s10.4",  0x0100, 0x0100, CRC(adbde7d7) SHA1(956ac5117c1e310f554ac705aa2dc24a796c36a5) )    /* green palette */
	ROM_LOAD( "24s10.3",  0x0200, 0x0100, CRC(ddac786a) SHA1(d1860105bf91297533ccc4aa6775987df198d0fa) )    /* blue palette */
	ROM_LOAD( "24s10.2",  0x0300, 0x0100, CRC(5af3f710) SHA1(da13d17acf8abd0f6ebb4b51b23c3324c6197b7d) )    /* alpha color */
	ROM_LOAD( "24s10.1",  0x0400, 0x0100, CRC(8c90e36e) SHA1(2646288d9e0f86300da7f06e1dc0595673205bb4) )    /* background color */
	ROM_LOAD( "24s10.8",  0x0500, 0x0100, CRC(2d502464) SHA1(682b7dd22e51d5db52c0804b7e27e47641dfa6bd) )    /* vertical position low */
	ROM_LOAD( "24s10.7",  0x0600, 0x0100, CRC(027aa62c) SHA1(c7030d8b64b80e107c446f6fbdd63f560c0a91c0) )    /* vertical position med */
	ROM_LOAD( "24s10.6",  0x0700, 0x0100, CRC(1f8d0df3) SHA1(b8f17758f114f5e247b65b3f2922ca2660757e66) )    /* vertical position hi */
	ROM_LOAD( "7643.1",   0x0800, 0x0400, CRC(7afc7cfc) SHA1(ba2407f6eff124e881b354f13205a4c058b7cf60) )    /* road color */
	ROM_LOAD( "7643.2",   0x0c00, 0x0400, CRC(2f1079ee) SHA1(18a27998a78deff13dd198f3668a7e92f084f467) )    /* sprite color */
	ROM_LOAD( "7603.1",   0x1000, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */
	ROM_LOAD( "7603.2",   0x1020, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */

	/* sound (P)ROM data */
	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "136014.118",   0x0000, 0x0100, CRC(8568decc) SHA1(0aac1fa082858d4d201e21511c609a989f9a1535) )    /* Namco sound */

	ROM_REGION( 0x4000, "engine", 0 )
	ROM_LOAD( "a.bin",   0x0000, 0x2000, CRC(b5ad4d5f) SHA1(c07e77a050200d6fe9952031f971ca35f4d15ff8) )    /* engine sound */
	ROM_LOAD( "b.bin",   0x2000, 0x2000, CRC(8fdd2f6f) SHA1(3818dc94c60cd78c4212ab7a4367cf3d98166ee6) )    /* engine sound */

	/* unknown or unused (P)ROM data */
	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "7052-4.c14",   0x0000, 0x0100, CRC(0e742cb1) SHA1(3ae43270aab4848fdeece1648e7e040ab216b08e) )    /* sync chain */
ROM_END


/*
    Pole Position 2 - Namco Version
*/

ROM_START( polepos2 )
	/* Z80 memory/ROM data */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pp4_9.6h",     0x0000, 0x2000, CRC(bcf87004) SHA1(0c60cbb777fe72dfd11c6f3e9da806a515cd0f8a) )
	ROM_LOAD( "pp4_10.5h",    0x2000, 0x1000, CRC(a9d4c380) SHA1(6048a8e858824936901e8e3e6b65d7505ccd82b4) )

	/* Z8002 #1 memory/ROM data */
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD16_BYTE( "pp4_1.8m",     0x0001, 0x2000, CRC(3f6ac294) SHA1(414ea7e43e62a573ad8971a7045f61eb997cf94e) )
	ROM_LOAD16_BYTE( "pp4_2.8l",     0x0000, 0x2000, CRC(51b9a669) SHA1(563ba42098d330801a992cd9c008c4cbbb993530) )

	/* Z8002 #2 memory/ROM data */
	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD16_BYTE( "pp4_5.4m",     0x0001, 0x2000, CRC(c3053cae) SHA1(f42cf61fe696dd7e282b29e2234ea7f487ec2372) )
	ROM_LOAD16_BYTE( "pp4_6.4l",     0x0000, 0x2000, CRC(38d04e0f) SHA1(5527cb1864248208b10d219a50ad742f286a119f) )
	ROM_LOAD16_BYTE( "pp4_7.3m",     0x4001, 0x1000, CRC(ad1c8994) SHA1(2877de9641516767170c0109900955cc7d1ff402) )
	ROM_LOAD16_BYTE( "pp4_8.3l",     0x4000, 0x1000, CRC(ef25a2ee) SHA1(45959355cad1a48f19ae14193374e03d4f9965c7) )

	/* graphics data */
	ROM_REGION( 0x02000, "gfx1", 0 )    /* 2bpp alpha layer */
	ROM_LOAD( "pp4_28.1f",    0x0000, 0x2000, CRC(280dde7d) SHA1(b7c7fb3a5076aa4d0e0cf3256ece9a6194315626) )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* 2bpp view layer */
	ROM_LOAD( "pp4_29.1e",    0x0000, 0x2000, CRC(ec3ec6e6) SHA1(ae905d0ae802d1010b2c1f1a13e88a1f0dbe57da) )

	ROM_REGION( 0x04000, "gfx3", 0 )    /* 4bpp 16x16 sprites */
	ROM_LOAD( "pp4_25.1n",    0x0000, 0x2000, CRC(fd098e65) SHA1(2c497f1d278ba6730752706a0d1b5a5a0fec3d5b) )    /* 4bpp sm sprites, planes 0+1 */
	ROM_LOAD( "pp4_26.1m",    0x2000, 0x2000, CRC(35ac62b3) SHA1(21038a78eb73d520e3e1ae8e1c0047d06b94cdab) )    /* 4bpp sm sprites, planes 2+3 */

	ROM_REGION( 0x10000, "gfx4", 0 )    /* 4bpp 32x32 sprites */
	ROM_LOAD( "pp1_17.5n",    0x0000, 0x2000, CRC(2e134b46) SHA1(0938f5f9f5cc6d7c1096c569449db78dbc42da01) )    /* 4bpp lg sprites, planes 0+1 */
	ROM_LOAD( "pp1_19.4n",    0x2000, 0x2000, CRC(43ff83e1) SHA1(8f830549a629b019125e59801e5027e4e4b3c0f2) )
	ROM_LOAD( "pp1_21.3n",    0x4000, 0x2000, CRC(5f958eb4) SHA1(b56d84e5e5e0ddeb0e71851ba66e5fa1b1409551) )
	ROM_LOAD( "pp4_23.2n",    0x6000, 0x2000, CRC(9e056fcd) SHA1(8545e0a9b6ebf8c2903321ceb9c4d693db10d750) )
	ROM_LOAD( "pp1_18.5m",    0x8000, 0x2000, CRC(6f9997d2) SHA1(b26d505266ccf23bfd867f881756c3251c80f57b) )    /* 4bpp lg sprites, planes 2+3 */
	ROM_LOAD( "pp1_20.4m",    0xa000, 0x2000, CRC(ec18075b) SHA1(af7be549c5fa47551a8dca4c0a531552147fa50f) )
	ROM_LOAD( "pp1_22.3m",    0xc000, 0x2000, CRC(1d2f30b1) SHA1(1d88a3069e9b15febd2835dd63e5511b3b2a6b45) )
	ROM_LOAD( "pp4_24.2m",    0xe000, 0x2000, CRC(795268cf) SHA1(84136142ef4bdcd97ede2209ecb16745960ac393) )

	ROM_REGION( 0x5000, "gfx5", 0 )     /* road generation ROMs needed at runtime */
	ROM_LOAD( "pp1_30.3a",    0x0000, 0x2000, CRC(ee6b3315) SHA1(9cc26c6d3604c0f60d716f86e67e9d9c0487f87d) )    /* road control */
	ROM_LOAD( "pp1_31.2a",    0x2000, 0x2000, CRC(6d1e7042) SHA1(90113ff0c93ed86d95067290088705bb5e6608d1) )    /* road bits 1 */
	ROM_LOAD( "pp1_32.1a",    0x4000, 0x1000, CRC(4e97f101) SHA1(f377d053821c74aee93ebcd30a4d43e6156f3cfe) )    /* road bits 2 */

	ROM_REGION( 0x1000, "gfx6", 0 )     /* sprite scaling */
	ROM_LOAD( "pp1_27.1l",    0x0000, 0x1000, CRC(a61bff15) SHA1(f7a59970831cdaaa7bf59c2221a38e4746c54244) )    /* vertical scaling */

	/* graphics (P)ROM data */
	ROM_REGION( 0x1040, "proms", 0 )
	ROM_LOAD( "pp4-7.8l",    0x0000, 0x0100, CRC(16d69c31) SHA1(f24b345448e4f4ef4e2f3b057b81d399cf427f88) )    /* red palette */
	ROM_LOAD( "pp4-8.9l",    0x0100, 0x0100, CRC(07340311) SHA1(3820d1fa99013ed18de5d9400ad376cc446d1217) )    /* green palette */
	ROM_LOAD( "pp4-9.10l",   0x0200, 0x0100, CRC(1efc84d7) SHA1(6946e1c209eec0a4b75778ae88111e6cb63c63fb) )    /* blue palette */
	ROM_LOAD( "pp4-10.2h",   0x0300, 0x0100, CRC(064d51a0) SHA1(d5baa29930530a8930b44a374e285de849c2a6ce) )    /* alpha color */
	ROM_LOAD( "pp4-11.4d",   0x0400, 0x0100, CRC(7880c5af) SHA1(e4388e354420be3f99594a10c091e3d2f745cc04) )    /* background color */
	ROM_LOAD( "pp1-15.9a",   0x0500, 0x0100, CRC(2d502464) SHA1(682b7dd22e51d5db52c0804b7e27e47641dfa6bd) )    /* vertical position low */
	ROM_LOAD( "pp1-16.10a",  0x0600, 0x0100, CRC(027aa62c) SHA1(c7030d8b64b80e107c446f6fbdd63f560c0a91c0) )    /* vertical position med */
	ROM_LOAD( "pp1-17.11a",  0x0700, 0x0100, CRC(1f8d0df3) SHA1(b8f17758f114f5e247b65b3f2922ca2660757e66) )    /* vertical position hi */
	ROM_LOAD( "pp4-12.3c",   0x0800, 0x0400, CRC(8b270902) SHA1(27b3ebc92d3a2a5c0432bde018a0e43669041d50) )    /* road color */
	ROM_LOAD( "pp4-6.6m",    0x0c00, 0x0400, CRC(647212b5) SHA1(ad58dfebd0ce8226285c2671c3b7797852c26d07) )    /* sprite color */
	ROM_LOAD( "pp1-13.8e",   0x1000, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */
	ROM_LOAD( "pp1-14.9e",   0x1020, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */

	/* sound (P)ROM data */
	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "pp1-5.3b",    0x0000, 0x0100, CRC(8568decc) SHA1(0aac1fa082858d4d201e21511c609a989f9a1535) )    /* Namco sound */

	ROM_REGION( 0x4000, "engine", 0 )
	ROM_LOAD( "pp4_15.6a",    0x0000, 0x2000, CRC(7d93bc1c) SHA1(dad7c0aa24aef593c84e21f7f8858ca7ada86364) )    /* engine sound */
	ROM_LOAD( "pp4_16.5a",    0x2000, 0x2000, CRC(7d93bc1c) SHA1(dad7c0aa24aef593c84e21f7f8858ca7ada86364) )    /* engine sound */

	ROM_REGION( 0x8000, "52xx", 0 )
	ROM_LOAD( "pp1_11.2e",    0x0000, 0x2000, CRC(45b9bfeb) SHA1(ff8c690471944d414931fb88666594ef608997f8) )    /* voice */
	ROM_LOAD( "pp1_13.1e",    0x2000, 0x2000, CRC(a4237466) SHA1(88a397276038cc2fc05f2c18472e6b7cef167f2e) )    /* voice */
	ROM_LOAD( "pp1_12.2f",    0x4000, 0x2000, CRC(a31b4be5) SHA1(38298093bb97ea8647fe187359cae05b65e1c616) )    /* voice */
	ROM_LOAD( "pp1_14.1f",    0x6000, 0x2000, CRC(944580f9) SHA1(c76f529cae718674ce97a1a599a3c6eaf6bf561a) )    /* voice */

	/* unknown or unused (P)ROM data */
	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "pp1-4.9h",    0x0000, 0x0100, CRC(2401c817) SHA1(8991b7994513a469e64392fa8f233af5e5f06d54) )    /* sync chain */
ROM_END


/*
    Pole Position 2 - Atari Version

    CPU/Sound Board: A039185
    Video Board:     A039187

    Pole Position 2 uses the same hardware as Pole Position except there are a couple of extra roms.
*/

ROM_START( polepos2a )
	// Z80 memory/ROM data
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "136014-180.7h", 0x0000, 0x2000, CRC(f85212c4) SHA1(666e55a7662247e72393b105b3e719be4233f1ff) )
	ROM_LOAD( "136014-183.7f", 0x2000, 0x1000, CRC(a9d4c380) SHA1(6048a8e858824936901e8e3e6b65d7505ccd82b4) )

	// Z8002 #1 memory/ROM data
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD16_BYTE( "136014-176.3l", 0x0001, 0x2000, CRC(8aeaec98) SHA1(76b3bbb64a17090bf28858f1e91d2206a3beaf5b) )
	ROM_LOAD16_BYTE( "136014-177.4l", 0x0000, 0x2000, CRC(7051df35) SHA1(cf23118ab05f5af273d756f97e6453496a276c9a) )

	// Z8002 #2 memory/ROM data
	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD16_BYTE( "136014-178.3e", 0x0001, 0x2000, CRC(eac35cfa) SHA1(f96005b3b63d85fc30695ab746af79c60f2f1341) )
	ROM_LOAD16_BYTE( "136014-179.4e", 0x0000, 0x2000, CRC(613e917d) SHA1(97c139f8aa7bd871a907e72980757b83f99fd8a0) )
	ROM_LOAD16_BYTE( "136014-184.3d", 0x4001, 0x2000, CRC(d893c4ed) SHA1(60d39abefbb0c8df68864a30b1f5fcbf4780c86c) )
	ROM_LOAD16_BYTE( "136014-185.4d", 0x4000, 0x2000, CRC(899de75e) SHA1(4a16535115e37a3d342b2cb53f610a87c0d0abe1) )

	// graphics data
	ROM_REGION( 0x02000, "gfx1", 0 ) // 2bpp alpha layer
	ROM_LOAD( "136014-172.7n", 0x0000, 0x2000, CRC(fbe5e72f) SHA1(07965d6e98ac1332ac6192b5e9cc927dd9eb706f) )

	ROM_REGION( 0x02000, "gfx2", 0 ) // 2bpp view layer
	ROM_LOAD( "136014-173.6n", 0x0000, 0x2000, CRC(ec3ec6e6) SHA1(ae905d0ae802d1010b2c1f1a13e88a1f0dbe57da) )

	ROM_REGION( 0x04000, "gfx3", 0 ) // 4bpp 16x16 sprites
	ROM_LOAD( "136014-170.13n", 0x0000, 0x2000, CRC(455d79a0) SHA1(03ef7c58f3145d9a6a461ef1aea3b5a49e653f80) ) // 4bpp sm sprites, planes 0+1
	ROM_LOAD( "136014-171.12n", 0x2000, 0x2000, CRC(78372b81) SHA1(5defaf2074c1ab4d13dc36a190c658ddf7f7931b) ) // 4bpp sm sprites, planes 2+3

	ROM_REGION( 0x10000, "gfx4", 0 ) // 4bpp 32x32 sprites
	ROM_LOAD( "136014-119.13j", 0x0000, 0x2000, CRC(2e134b46) SHA1(0938f5f9f5cc6d7c1096c569449db78dbc42da01) ) // 4bpp lg sprites, planes 0+1
	ROM_LOAD( "136014-166.13k", 0x2000, 0x2000, CRC(2b0517bd) SHA1(ebe447ba3dcd8a3b56f47d707483074f61953fec) )
	ROM_LOAD( "136014-168.13l", 0x4000, 0x2000, CRC(4d7916d9) SHA1(052745f252f51bfdd456e54cf7b8d22ab3aace27) )
	ROM_LOAD( "136014-175.13m", 0x6000, 0x2000, CRC(bd6df480) SHA1(58f39fa3ae43d94fe42dc51da341384a9c3879ae) )
	ROM_LOAD( "136014-120.12j", 0x8000, 0x2000, CRC(6f9997d2) SHA1(b26d505266ccf23bfd867f881756c3251c80f57b) ) // 4bpp lg sprites, planes 2+3
	ROM_LOAD( "136014-167.12k", 0xa000, 0x2000, CRC(411e21b5) SHA1(9659ee429d819926b5e5b12c41b968ae6e7f186e) )
	ROM_LOAD( "136014-169.12l", 0xc000, 0x2000, CRC(662ff24b) SHA1(4cf8509034742c2bec8a96c7a786dafdf5875e4f) )
	ROM_LOAD( "136014-174.12m", 0xe000, 0x2000, CRC(f0c571dc) SHA1(9e6839e9e203fc120a0389f4e11c9d46a817dbdf) )

	ROM_REGION( 0x5000, "gfx5", 0 )  //road generation ROMs needed at runtime
	ROM_LOAD( "136014-127.2l", 0x0000, 0x2000, CRC(ee6b3315) SHA1(9cc26c6d3604c0f60d716f86e67e9d9c0487f87d) ) // road control
	ROM_LOAD( "136014-128.2m", 0x2000, 0x2000, CRC(6d1e7042) SHA1(90113ff0c93ed86d95067290088705bb5e6608d1) ) // road bits 1
	ROM_LOAD( "136014-134.2n", 0x4000, 0x1000, CRC(4e97f101) SHA1(f377d053821c74aee93ebcd30a4d43e6156f3cfe) ) // road bits 2

	ROM_REGION( 0x1000, "gfx6", 0 ) // sprite scaling
	ROM_LOAD( "136014-231.11n", 0x0000, 0x1000, CRC(a61bff15) SHA1(f7a59970831cdaaa7bf59c2221a38e4746c54244) ) // vertical scaling

	// graphics PROM data
	ROM_REGION( 0x1040, "proms", 0 )
	ROM_LOAD( "136014-186.11e", 0x0000, 0x0100, CRC(16d69c31) SHA1(f24b345448e4f4ef4e2f3b057b81d399cf427f88) ) // red palette
	ROM_LOAD( "136014-187.11d", 0x0100, 0x0100, CRC(07340311) SHA1(3820d1fa99013ed18de5d9400ad376cc446d1217) ) // green palette
	ROM_LOAD( "136014-188.11c", 0x0200, 0x0100, CRC(1efc84d7) SHA1(6946e1c209eec0a4b75778ae88111e6cb63c63fb) ) // blue palette
	ROM_LOAD( "136014-189.8m",  0x0300, 0x0100, CRC(064d51a0) SHA1(d5baa29930530a8930b44a374e285de849c2a6ce) ) // alpha color
	ROM_LOAD( "136014-190.5k",  0x0400, 0x0100, CRC(7880c5af) SHA1(e4388e354420be3f99594a10c091e3d2f745cc04) ) // background color
	ROM_LOAD( "136014-142.2d",  0x0500, 0x0100, CRC(2d502464) SHA1(682b7dd22e51d5db52c0804b7e27e47641dfa6bd) ) // vertical position low
	ROM_LOAD( "136014-143.2c",  0x0600, 0x0100, CRC(027aa62c) SHA1(c7030d8b64b80e107c446f6fbdd63f560c0a91c0) ) // vertical position med
	ROM_LOAD( "136014-144.2b",  0x0700, 0x0100, CRC(1f8d0df3) SHA1(b8f17758f114f5e247b65b3f2922ca2660757e66) ) // vertical position hi
	ROM_LOAD( "136014-191.4l",  0x0800, 0x0400, CRC(8b270902) SHA1(27b3ebc92d3a2a5c0432bde018a0e43669041d50) ) // road color
	ROM_LOAD( "136014-192.12h", 0x0c00, 0x0400, CRC(caddb0b0) SHA1(e41b89f2b40bf8f93546012f373ae63dcae870da) ) // sprite color
	ROM_LOAD( "136014-135.6e",  0x1000, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) ) // video RAM address decoder (not used)
	ROM_LOAD( "136014-136.6d",  0x1020, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) ) // video RAM address decoder (not used)

	// sound PROM data
	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "136014-118.11d", 0x0000, 0x0100, CRC(8568decc) SHA1(0aac1fa082858d4d201e21511c609a989f9a1535) ) // Namco sound

	ROM_REGION( 0x4000, "engine", 0 )
	ROM_LOAD( "136014-181.12f", 0x0000, 0x2000, CRC(7d93bc1c) SHA1(dad7c0aa24aef593c84e21f7f8858ca7ada86364) ) // engine sound
	ROM_LOAD( "136014-182.12e", 0x2000, 0x2000, CRC(7d93bc1c) SHA1(dad7c0aa24aef593c84e21f7f8858ca7ada86364) ) // engine sound

	ROM_REGION( 0x6000, "52xx", 0 )
	ROM_LOAD( "136014-106.9c", 0x0000, 0x2000, CRC(5b4cf05e) SHA1(52342572940489175607bbf5b6cfd05ee9b0f004) ) // voice

	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "136014-117.7l", 0x0000, 0x0100, CRC(2401c817) SHA1(8991b7994513a469e64392fa8f233af5e5f06d54) ) // sync chain (unused)

	ROM_REGION( 0x0300, "cpu_pals", 0 ) // PAL's located on the cpu board
	ROM_LOAD( "137316-001.2n", 0x0000, 0x0034, CRC(dd37bd15) SHA1(3820203c8d7a64826ed6172be38d51fc70792ccd) ) // MMI PAL12L6CN - manual states 137280-001
	ROM_LOAD( "137316-001.5c", 0x0100, 0x0034, CRC(dd37bd15) SHA1(3820203c8d7a64826ed6172be38d51fc70792ccd) ) // MMI PAL12L6CN - manual states 137280-001
	ROM_LOAD( "137279-001.7c", 0x0200, 0x00eb, CRC(4ddc51ba) SHA1(78e64ef9074bd103662bc78a312e3a2b2b0957cc) ) // N82S153N
ROM_END


ROM_START( polepos2b )
	/* Z80 memory/ROM data */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "136014.180",   0x0000, 0x2000, CRC(f85212c4) SHA1(666e55a7662247e72393b105b3e719be4233f1ff) )
	ROM_LOAD( "136014.183",   0x2000, 0x1000, CRC(a9d4c380) SHA1(6048a8e858824936901e8e3e6b65d7505ccd82b4) )

	/* Z8002 #1 memory/ROM data */
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD16_BYTE( "3lcpu.rom",    0x0001, 0x2000, CRC(cf95a6b7) SHA1(6a8419af8a52d3a8c88663b67845e4cb18e35723) )
	ROM_LOAD16_BYTE( "4lcpu.rom",    0x0000, 0x2000, CRC(643483f7) SHA1(020822f623b8e65c6016492266b6e328f7637b68) )
	ROM_LOAD16_BYTE( "cpu-4k.rom",   0x4000, 0x1000, CRC(97a496b3) SHA1(fe79d2376c5fa9fe242905a841a1c894a5ccfba4) )

	/* Z8002 #2 memory/ROM data */
	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD16_BYTE( "136014.178",   0x0001, 0x2000, CRC(eac35cfa) SHA1(f96005b3b63d85fc30695ab746af79c60f2f1341) )
	ROM_LOAD16_BYTE( "136014.179",   0x0000, 0x2000, CRC(613e917d) SHA1(97c139f8aa7bd871a907e72980757b83f99fd8a0) )
	ROM_LOAD16_BYTE( "136014.184",   0x4001, 0x2000, CRC(d893c4ed) SHA1(60d39abefbb0c8df68864a30b1f5fcbf4780c86c) )
	ROM_LOAD16_BYTE( "136014.185",   0x4000, 0x2000, CRC(899de75e) SHA1(4a16535115e37a3d342b2cb53f610a87c0d0abe1) )

	/* graphics data */
	ROM_REGION( 0x02000, "gfx1", 0 )    /* 2bpp alpha layer */
	ROM_LOAD( "136014.172",   0x0000, 0x2000, CRC(fbe5e72f) SHA1(07965d6e98ac1332ac6192b5e9cc927dd9eb706f) )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* 2bpp view layer */
	ROM_LOAD( "136014.173",   0x0000, 0x2000, CRC(ec3ec6e6) SHA1(ae905d0ae802d1010b2c1f1a13e88a1f0dbe57da) )

	ROM_REGION( 0x04000, "gfx3", 0 )    /* 4bpp 16x16 sprites */
	ROM_LOAD( "136014.170",   0x0000, 0x2000, CRC(455d79a0) SHA1(03ef7c58f3145d9a6a461ef1aea3b5a49e653f80) )    /* 4bpp sm sprites, planes 0+1 */
	ROM_LOAD( "136014.171",   0x2000, 0x2000, CRC(78372b81) SHA1(5defaf2074c1ab4d13dc36a190c658ddf7f7931b) )    /* 4bpp sm sprites, planes 2+3 */

	ROM_REGION( 0x10000, "gfx4", 0 )    /* 4bpp 32x32 sprites */
	ROM_LOAD( "136014.119",   0x0000, 0x2000, CRC(2e134b46) SHA1(0938f5f9f5cc6d7c1096c569449db78dbc42da01) )    /* 4bpp lg sprites, planes 0+1 */
	ROM_LOAD( "136014.166",   0x2000, 0x2000, CRC(2b0517bd) SHA1(ebe447ba3dcd8a3b56f47d707483074f61953fec) )
	ROM_LOAD( "136014.168",   0x4000, 0x2000, CRC(4d7916d9) SHA1(052745f252f51bfdd456e54cf7b8d22ab3aace27) )
	ROM_LOAD( "136014.175",   0x6000, 0x2000, CRC(bd6df480) SHA1(58f39fa3ae43d94fe42dc51da341384a9c3879ae) )
	ROM_LOAD( "136014.120",   0x8000, 0x2000, CRC(6f9997d2) SHA1(b26d505266ccf23bfd867f881756c3251c80f57b) )    /* 4bpp lg sprites, planes 2+3 */
	ROM_LOAD( "136014.167",   0xa000, 0x2000, CRC(411e21b5) SHA1(9659ee429d819926b5e5b12c41b968ae6e7f186e) )
	ROM_LOAD( "136014.169",   0xc000, 0x2000, CRC(662ff24b) SHA1(4cf8509034742c2bec8a96c7a786dafdf5875e4f) )
	ROM_LOAD( "136014.174",   0xe000, 0x2000, CRC(f0c571dc) SHA1(9e6839e9e203fc120a0389f4e11c9d46a817dbdf) )

	ROM_REGION( 0x5000, "gfx5", 0 )     /* road generation ROMs needed at runtime */
	ROM_LOAD( "136014.127",   0x0000, 0x2000, CRC(ee6b3315) SHA1(9cc26c6d3604c0f60d716f86e67e9d9c0487f87d) )    /* road control */
	ROM_LOAD( "136014.128",   0x2000, 0x2000, CRC(6d1e7042) SHA1(90113ff0c93ed86d95067290088705bb5e6608d1) )    /* road bits 1 */
	ROM_LOAD( "136014.134",   0x4000, 0x1000, CRC(4e97f101) SHA1(f377d053821c74aee93ebcd30a4d43e6156f3cfe) )    /* road bits 2 */

	ROM_REGION( 0x1000, "gfx6", 0 )     /* sprite scaling */
	ROM_LOAD( "136014.231",   0x0000, 0x1000, CRC(a61bff15) SHA1(f7a59970831cdaaa7bf59c2221a38e4746c54244) )    /* vertical scaling */

	/* graphics (P)ROM data */
	ROM_REGION( 0x1040, "proms", 0 )
	ROM_LOAD( "136014.186",   0x0000, 0x0100, CRC(16d69c31) SHA1(f24b345448e4f4ef4e2f3b057b81d399cf427f88) )    /* red palette */
	ROM_LOAD( "136014.187",   0x0100, 0x0100, CRC(07340311) SHA1(3820d1fa99013ed18de5d9400ad376cc446d1217) )    /* green palette */
	ROM_LOAD( "136014.188",   0x0200, 0x0100, CRC(1efc84d7) SHA1(6946e1c209eec0a4b75778ae88111e6cb63c63fb) )    /* blue palette */
	ROM_LOAD( "136014.189",   0x0300, 0x0100, CRC(064d51a0) SHA1(d5baa29930530a8930b44a374e285de849c2a6ce) )    /* alpha color */
	ROM_LOAD( "136014.190",   0x0400, 0x0100, CRC(7880c5af) SHA1(e4388e354420be3f99594a10c091e3d2f745cc04) )    /* background color */
	ROM_LOAD( "136014.142",   0x0500, 0x0100, CRC(2d502464) SHA1(682b7dd22e51d5db52c0804b7e27e47641dfa6bd) )    /* vertical position low */
	ROM_LOAD( "136014.143",   0x0600, 0x0100, CRC(027aa62c) SHA1(c7030d8b64b80e107c446f6fbdd63f560c0a91c0) )    /* vertical position med */
	ROM_LOAD( "136014.144",   0x0700, 0x0100, CRC(1f8d0df3) SHA1(b8f17758f114f5e247b65b3f2922ca2660757e66) )    /* vertical position hi */
	ROM_LOAD( "136014.191",   0x0800, 0x0400, CRC(8b270902) SHA1(27b3ebc92d3a2a5c0432bde018a0e43669041d50) )    /* road color */
	ROM_LOAD( "136014.192",   0x0c00, 0x0400, CRC(caddb0b0) SHA1(e41b89f2b40bf8f93546012f373ae63dcae870da) )    /* sprite color */
	ROM_LOAD( "136014.135",   0x1000, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */
	ROM_LOAD( "136014.136",   0x1020, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */

	/* sound (P)ROM data */
	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "136014.118",   0x0000, 0x0100, CRC(8568decc) SHA1(0aac1fa082858d4d201e21511c609a989f9a1535) )    /* Namco sound */

	ROM_REGION( 0x4000, "engine", 0 )
	ROM_LOAD( "136014.181",   0x0000, 0x2000, CRC(7d93bc1c) SHA1(dad7c0aa24aef593c84e21f7f8858ca7ada86364) )    /* engine sound */
	ROM_LOAD( "136014.182",   0x2000, 0x2000, CRC(7d93bc1c) SHA1(dad7c0aa24aef593c84e21f7f8858ca7ada86364) )    /* engine sound */

	ROM_REGION( 0x6000, "52xx", 0 )
	ROM_LOAD( "136014.106",   0x0000, 0x2000, CRC(5b4cf05e) SHA1(52342572940489175607bbf5b6cfd05ee9b0f004) )    /* voice */

	/* unknown or unused (P)ROM data */
	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "136014.117",   0x0000, 0x0100, CRC(2401c817) SHA1(8991b7994513a469e64392fa8f233af5e5f06d54) )    /* sync chain */
ROM_END


ROM_START( polepos2bi )
	/* Z80 memory/ROM data */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "5ita.bin",   0x0000, 0x2000, CRC(1a6412a1) SHA1(ccc41e60aad6ed332f8f2582860e11f10937dffa) )
	ROM_LOAD( "6ita.bin",   0x2000, 0x1000, CRC(e7362148) SHA1(5a4ab037fa6a773b90c10ac4c4e9417183e0cfd8) )

	/* Z8002 #1 memory/ROM data */
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD16_BYTE( "19.bin",   0x0001, 0x2000, CRC(41da3c28) SHA1(c9294d686282adfc72796511c3c9e186ad057374) )
	ROM_LOAD16_BYTE( "18.bin",   0x0000, 0x2000, CRC(2856d5b1) SHA1(96f5c3d67901a1abceca12b3448f381cc4852a33) )

	/* Z8002 #2 memory/ROM data */
	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD16_BYTE( "17.bin",   0x0001, 0x4000, CRC(6c823932) SHA1(68ef9f70c4305c3a3bacf83a64b727fd3711f34f) )
	ROM_LOAD16_BYTE( "4.bin",    0x0000, 0x4000, CRC(fe9baeb6) SHA1(9a8ad2d8a69b4005f7abed278093fd57b9242bca) )

	/* graphics data */
	ROM_REGION( 0x02000, "gfx1", 0 )    /* 2bpp alpha layer */
	ROM_LOAD( "05.bin",   0x0000, 0x2000, CRC(55bec6f3) SHA1(8b405c74473abb7debaa9114991e7b134d06fe42) )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* 2bpp view layer */
	ROM_LOAD( "04.bin",   0x0000, 0x2000, CRC(ec3ec6e6) SHA1(ae905d0ae802d1010b2c1f1a13e88a1f0dbe57da) )

	ROM_REGION( 0x04000, "gfx3", 0 )    /* 4bpp 16x16 sprites */
	ROM_LOAD( "12.bin",   0x0000, 0x2000, CRC(1c72041a) SHA1(b65b09c4251ee61d247f359615e7adc7c80bc8d5) )    /* 4bpp sm sprites, planes 0+1 */
	ROM_LOAD( "11.bin",   0x2000, 0x2000, CRC(1b38b257) SHA1(c7eec0692a31e1c8285bd1cba3ebd17ab253d2c9) )    /* 4bpp sm sprites, planes 2+3 */

	ROM_REGION( 0x10000, "gfx4", 0 )    /* 4bpp 32x32 sprites */
	ROM_LOAD( "16.bin",      0x0000, 0x2000, CRC(613ab0df) SHA1(88aa4500275aae010fc9783c1d8d843feab89afa) )    /* 4bpp lg sprites, planes 0+1 */
	ROM_LOAD( "15.bin",      0x2000, 0x2000, CRC(f8e7f551) SHA1(faa23c55bc43325e6f71936be970f2ca144697d8) )
	ROM_LOAD( "14.bin",      0x4000, 0x2000, CRC(17c798b0) SHA1(ae2047bc0e4e8c85e1de09c39c200ea8f7c6a72e) )
	ROM_LOAD( "13.bin",      0x6000, 0x2000, CRC(ed6a8052) SHA1(dedd6d63f9a06a1edd57cb134e86c048cff7a3c1) )
	ROM_LOAD( "10.bin",      0x8000, 0x2000, CRC(5fd933e3) SHA1(5b27a8519234c935308f943cd58abc1efc463726) )    /* 4bpp lg sprites, planes 2+3 */
	ROM_LOAD( "09.bin",      0xa000, 0x2000, CRC(7053e219) SHA1(97700fbe887e2d11c9f9a0937147725f6787f081) )
	ROM_LOAD( "08.bin",      0xc000, 0x2000, CRC(5fe9b365) SHA1(1a3ac099a6bb506a5f71c12c6fb14d014172371c) )
	ROM_LOAD( "07.bin",      0xe000, 0x2000, CRC(ca14ca7b) SHA1(e58e40fdf1385ae9b080225d9ffe3ec5b122bf69) )

	ROM_REGION( 0x5000, "gfx5", 0 )     /* road generation ROMs needed at runtime */
	ROM_LOAD( "03.bin",   0x0000, 0x2000, CRC(ee6b3315) SHA1(9cc26c6d3604c0f60d716f86e67e9d9c0487f87d) )    /* road control */
	ROM_LOAD( "02.bin",   0x2000, 0x2000, CRC(6d1e7042) SHA1(90113ff0c93ed86d95067290088705bb5e6608d1) )    /* road bits 1 */
	ROM_LOAD( "01.bin",   0x4000, 0x1000, CRC(4e97f101) SHA1(f377d053821c74aee93ebcd30a4d43e6156f3cfe) )    /* road bits 2 */

	ROM_REGION( 0x1000, "gfx6", 0 )     /* sprite scaling */
	ROM_LOAD( "06.bin",   0x0000, 0x1000, CRC(a61bff15) SHA1(f7a59970831cdaaa7bf59c2221a38e4746c54244) )    /* vertical scaling */

	/* graphics (P)ROM data */
	ROM_REGION( 0x1040, "proms", 0 )
	ROM_LOAD( "7611-5-e.bin",   0x0000, 0x0100, CRC(16d69c31) SHA1(f24b345448e4f4ef4e2f3b057b81d399cf427f88) )    /* red palette */
	ROM_LOAD( "7611-5-d.bin",   0x0100, 0x0100, CRC(07340311) SHA1(3820d1fa99013ed18de5d9400ad376cc446d1217) )    /* green palette */
	ROM_LOAD( "7611-5-c.bin",   0x0200, 0x0100, CRC(1efc84d7) SHA1(6946e1c209eec0a4b75778ae88111e6cb63c63fb) )    /* blue palette */
	ROM_LOAD( "7611-5-b.bin",   0x0300, 0x0100, CRC(064d51a0) SHA1(d5baa29930530a8930b44a374e285de849c2a6ce) )    /* alpha color */
	ROM_LOAD( "7611-5-a.bin",   0x0400, 0x0100, CRC(7880c5af) SHA1(e4388e354420be3f99594a10c091e3d2f745cc04) )    /* background color */
	ROM_LOAD( "7611-5-h.bin",   0x0500, 0x0100, CRC(2d502464) SHA1(682b7dd22e51d5db52c0804b7e27e47641dfa6bd) )    /* vertical position low */
	ROM_LOAD( "7611-5-g.bin",   0x0600, 0x0100, CRC(027aa62c) SHA1(c7030d8b64b80e107c446f6fbdd63f560c0a91c0) )    /* vertical position med */
	ROM_LOAD( "7611-5-f.bin",   0x0700, 0x0100, CRC(1f8d0df3) SHA1(b8f17758f114f5e247b65b3f2922ca2660757e66) )    /* vertical position hi */
	ROM_LOAD( "7643-5-b.bin",   0x0800, 0x0400, CRC(8b270902) SHA1(27b3ebc92d3a2a5c0432bde018a0e43669041d50) )    /* road color */
	ROM_LOAD( "7643-5-a.bin",   0x0c00, 0x0400, CRC(a079ed19) SHA1(134b3d156a1ed0fa21cc5dc3cc84ea16ef7f84f7) )    /* sprite color - bad?*/
	ROM_LOAD( "6331-1-b.bin",   0x1000, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */
	ROM_LOAD( "6331-1-a.bin",   0x1020, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */

	/* sound (P)ROM data */
	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "74s287-b.bin",   0x0000, 0x0100, CRC(8568decc) SHA1(0aac1fa082858d4d201e21511c609a989f9a1535) )    /* Namco sound */

	ROM_REGION( 0x4000, "engine", 0 )
//  ROM_LOAD( "pp4-15.bin",   0x0000, 0x2000, CRC(5d79e1ad) SHA1(0323bdf3b9aca298b788bb07020653a43114c952) )    /* differs by one bit, almost certainly bitrot */
	ROM_LOAD( "pp4-15.bin",   0x0000, 0x2000, CRC(7d93bc1c) SHA1(dad7c0aa24aef593c84e21f7f8858ca7ada86364) )    /* engine sound */
	ROM_LOAD( "pp4-16.bin",   0x2000, 0x2000, CRC(7d93bc1c) SHA1(dad7c0aa24aef593c84e21f7f8858ca7ada86364) )    /* engine sound */

	ROM_REGION( 0x6000, "52xx", ROMREGION_ERASEFF )
	/* the bootleg has a TMS5220, NOT the Namco 52xx */

	/* unknown or unused (P)ROM data */
	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "74s287-a.bin",   0x0000, 0x0100, CRC(0e742cb1) SHA1(3ae43270aab4848fdeece1648e7e040ab216b08e) )    /* sync chain */

	/* this is used for the Italian speech with a TMS5220 */
	ROM_REGION( 0x2000, "soundz80bl", 0 )
	ROM_LOAD( "20.bin",       0x0000, 0x2000, CRC(1771fe1b) SHA1(da74ca85dfd4f5ad5a9dbfe6f7668d93105e3575) )

	ROM_REGION( 0x2000, "pals", 0 )
	ROM_LOAD( "pal12l6-a.bin", 0x0000, 0x34, NO_DUMP )
	ROM_LOAD( "pal12l6-b.bin", 0x0000, 0x34, NO_DUMP )
	ROM_LOAD( "pal16l8.bin",   0x0000,0x104, NO_DUMP )
ROM_END


/*
  Gran Premio F1 (Spanish bootleg of Pole Position II)

  This bootleg has a TMS5220 for Spanish voices instead of the Namco 52xx.

  DIP Switches reference (to be implemented):

  .-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----.
  | DIP Switch A (Derecha)        |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----------+
  | MONEDERO 25.  MONEDAS PART.   |     |     |     |     |     |                 |
  |                    1   1      | OFF |     | OFF |     | OFF |                 |
  |                    2   1      | OFF |     | OFF |     | ON  |                 |
  |                    3   1      | OFF |     | ON  |     | OFF |                 |
  |                    1   2      | ON  |     | OFF |     | OFF |                 |
  |                    1   3      | ON  |     | OFF |     | ON  |                 |
  |                    2   3      | ON  |     | ON  |     | OFF |                 |
  |                    1   6      | OFF |     | ON  |     | ON  |                 |
  |                    1   5      | ON  |     | ON  |     | ON  |                 |
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | MONEDERO 50.  MONEDAS PART.   |     |     |                       |     |     |
  |                    1   1      |     | OFF |                       | OFF |     |
  |                    2   1      |     | OFF |                       | ON  |     |
  |                    1   2      |     | ON  |                       | OFF |     |
  |                    2   3      |     | ON  |                       | ON  |     |
  +-------------------------------+-----+-----+-----+-----+-----------+-----+-----+
  | INDICACION         Km/h       |                 | ON  |                       |
  | VELOCIDAD          Mp/h       |                 | OFF |                       |
  +-------------------------------+-----------------+-----+-----+-----+-----------+
  | SONIDO EN          NO         |                             | ON  |           |
  | PRESENTACION       SI         |                             | OFF |           |
  +-------------------------------+-----------------------------+-----+-----+-----+
  | VELOCIDAD MAX.     Reducida   |                                         | ON  |
  |                    Elevada    |                                         | OFF |
  '-------------------------------+-----------------------------------------+-----'

  .-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----.
  | DIP Switch B (Izquierda)      |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | DIFICULTAD         D MAX.     | OFF |     | OFF |                       | ON  |
  | CARRERA            C          | ON  |     | OFF |                       | ON  |
  |                    B          | OFF |     | ON  |                       | ON  |
  |                    A MIN.     | ON  |     | ON  |                       | ON  |
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----+-----+
  | DIFICULTAD PARA    MAX.       |                       | OFF |     | OFF |     |
  | CLASIFICACION                 |                       | ON  |     | OFF |     |
  |                               |                       | OFF |     | ON  |     |
  |                    MIN.       |                       | ON  |     | ON  |     |
  +-------------------------------+-----------------------+-----+-----+-----+-----+
  | TIEMPO PARA        Tiempo     |                             |     |           |
  | CLASIFICACION      90 Seg.    |                             | ON  |           |
  |                    120 Seg.   |                             | OFF |           |
  +-------------------------------+-----+-----+-----+-----+-----+-----+-----------+
  | NUMERO DE          6          |     | OFF |     | OFF |                       |
  | VUELTAS            5          |     | ON  |     | OFF |                       |
  |                    4          |     | OFF |     | ON  |                       |
  |                    3          |     | ON  |     | ON  |                       |
  '-------------------------------+-----+-----+-----+-----+-----------------------'


  Pinouts / Edge Connectors....


            'A' board connector.                           'B' board connector.                           'C' board connector.
     ---------------+--+------------------          ---------------+--+------------------          ---------------+--+------------------
        Solder Side |PN| Components Side               Solder Side |PN| Components Side               Solder Side |PN| Components Side
     ---------------+--+------------------          ---------------+--+------------------          ---------------+--+------------------
                    |01| GND                                   GND |01| GND                                   +5V |01| +5V
       GEARS SWITCH |02| GND                                       |02|                                       +5V |02| +5V
     50 PTS COIN-IN |03| GND                                       |03|                                           |03|
                    |04| GND                                   +5V |04| +5V                                       |04|
     25 PTS COIN-IN |05| GND                                       |05| COMMON ACCEL + BREAK (HI)                 |05|
                +5V |06| +5V                             BRAKE POT |06| ACCEL POT                                 |06|
                +5V |07| +5V                                       |07| COMMON ACCEL + BREAK (LO)                 |07| GND
                +5V |08| +5V                                       |08| GND                                       |08| VIDEO GND
                    |09|                       AUDIO SIGNAL 4(***) |09| AUDIO SIGNAL 1(***)            VIDEO SYNC |09| VIDEO BLUE
                    |10|                       AUDIO SIGNAL 3(***) |10| AUDIO SIGNAL 2(***)           VIDEO GREEN |10| VIDEO RED
               +12V |11|                                       GND |11| GND                                       |11|
     WHEEL X SIGNAL |12|                                           |12|                                           |12|
     WHEEL Z SIGNAL |13|                                           |13|                                           |13|
                -5V |14|                                           |14|                                           |14|
  AUDIO SIGNAL 1(*) |15| AMP AUDIO SIGNAL (**)                     |15|                                       GND |15| GND
  AUDIO SIGNAL 2(*) |16| AMP AUDIO SIGNAL (**)                     |16|                                       GND |16| GND
  AUDIO SIGNAL 3(*) |17|                                           |17|                                       GND |17| GND
  AUDIO SIGNAL 4(*) |18|                                           |18|                                       GND |18| GND


  (*)   To 'B' connector.
  (**)  To 'D' connector.
  (***) To 'A' connector.

*/
ROM_START( polepos2bs )
	/* Z80 memory/ROM data */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bboard-5p.17f",   0x0000, 0x2000, CRC(1a6412a1) SHA1(ccc41e60aad6ed332f8f2582860e11f10937dffa) )
	ROM_LOAD( "bboard-6p.16f",   0x2000, 0x1000, CRC(e7362148) SHA1(5a4ab037fa6a773b90c10ac4c4e9417183e0cfd8) )

	/* Z8002 #1 memory/ROM data */
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD16_BYTE( "bboard-1p.23f",   0x0001, 0x2000, CRC(41da3c28) SHA1(c9294d686282adfc72796511c3c9e186ad057374) )
	ROM_LOAD16_BYTE( "bboard-2p.21f",   0x0000, 0x2000, CRC(2856d5b1) SHA1(96f5c3d67901a1abceca12b3448f381cc4852a33) )

	/* Z8002 #2 memory/ROM data */
	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD16_BYTE( "bboard-3p.20f",   0x0001, 0x4000, CRC(6c823932) SHA1(68ef9f70c4305c3a3bacf83a64b727fd3711f34f) )
	ROM_LOAD16_BYTE( "bboard-4p.18f",    0x0000, 0x4000, CRC(fe9baeb6) SHA1(9a8ad2d8a69b4005f7abed278093fd57b9242bca) )

	/* graphics data */
	ROM_REGION( 0x02000, "gfx1", 0 )    /* 2bpp alpha layer */
	ROM_LOAD( "cboard-12f.2j",   0x0000, 0x2000, CRC(e38956ea) SHA1(fde6cf83b3a06af9f74accade92e74dab5f914a9) )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* 2bpp view layer */
	ROM_LOAD( "cboard-15f.2l",   0x0000, 0x2000, CRC(ec3ec6e6) SHA1(ae905d0ae802d1010b2c1f1a13e88a1f0dbe57da) )

	ROM_REGION( 0x04000, "gfx3", 0 )    /* 4bpp 16x16 sprites */
	ROM_LOAD( "cboard-6f.5a",   0x0000, 0x2000, CRC(1c72041a) SHA1(b65b09c4251ee61d247f359615e7adc7c80bc8d5) )    /* 4bpp sm sprites, planes 0+1 */
	ROM_LOAD( "cboard-5f.6a",   0x2000, 0x2000, CRC(1b38b257) SHA1(c7eec0692a31e1c8285bd1cba3ebd17ab253d2c9) )    /* 4bpp sm sprites, planes 2+3 */

	ROM_REGION( 0x10000, "gfx4", 0 )    /* 4bpp 32x32 sprites */
	ROM_LOAD( "cboard-10f.1a",     0x0000, 0x2000, CRC(613ab0df) SHA1(88aa4500275aae010fc9783c1d8d843feab89afa) )    /* 4bpp lg sprites, planes 0+1 */
	ROM_LOAD( "cboard-9f.2a",      0x2000, 0x2000, CRC(2d11fc01) SHA1(0515df62073db993899dde3f9ad84334c5a12fc5) )
	ROM_LOAD( "cboard-8f.3a",      0x4000, 0x2000, CRC(17c798b0) SHA1(ae2047bc0e4e8c85e1de09c39c200ea8f7c6a72e) )
	ROM_LOAD( "cboard-7f.4a",      0x6000, 0x2000, CRC(ed6a8052) SHA1(dedd6d63f9a06a1edd57cb134e86c048cff7a3c1) )
	ROM_LOAD( "cboard-4f.7a",      0x8000, 0x2000, CRC(5fd933e3) SHA1(5b27a8519234c935308f943cd58abc1efc463726) )    /* 4bpp lg sprites, planes 2+3 */
	ROM_LOAD( "cboard-3f.8a",      0xa000, 0x2000, CRC(94a7155d) SHA1(c162e2b0e93745614475326905a30d2095101913) )
	ROM_LOAD( "cboard-2f.9a",      0xc000, 0x2000, CRC(5fe9b365) SHA1(1a3ac099a6bb506a5f71c12c6fb14d014172371c) )
	ROM_LOAD( "cboard-1f.10a",     0xe000, 0x2000, CRC(ca14ca7b) SHA1(e58e40fdf1385ae9b080225d9ffe3ec5b122bf69) )

	ROM_REGION( 0x5000, "gfx5", 0 )     /* road generation ROMs needed at runtime */
	ROM_LOAD( "cboard-14f.8m",   0x0000, 0x2000, CRC(ee6b3315) SHA1(9cc26c6d3604c0f60d716f86e67e9d9c0487f87d) )    /* road control */
	ROM_LOAD( "cboard-13f.9m",   0x2000, 0x2000, CRC(6d1e7042) SHA1(90113ff0c93ed86d95067290088705bb5e6608d1) )    /* road bits 1 */
	ROM_LOAD( "cboard-16f.8r",   0x4000, 0x1000, CRC(4e97f101) SHA1(f377d053821c74aee93ebcd30a4d43e6156f3cfe) )    /* road bits 2 */

	ROM_REGION( 0x1000, "gfx6", 0 )     /* sprite scaling */
	ROM_LOAD( "cboard-11f.9c",   0x0000, 0x1000, CRC(a61bff15) SHA1(f7a59970831cdaaa7bf59c2221a38e4746c54244) )    /* vertical scaling */

	/* graphics (P)ROM data */
	ROM_REGION( 0x1040, "proms", 0 )
	ROM_LOAD( "cboard-f1-82s129.2u",    0x0000, 0x0100, CRC(16d69c31) SHA1(f24b345448e4f4ef4e2f3b057b81d399cf427f88) )    /* red palette */
	ROM_LOAD( "cboard-f2-82s129.2t",    0x0100, 0x0100, CRC(07340311) SHA1(3820d1fa99013ed18de5d9400ad376cc446d1217) )    /* green palette */
	ROM_LOAD( "cboard-f3-82s129.2s",    0x0200, 0x0100, CRC(1efc84d7) SHA1(6946e1c209eec0a4b75778ae88111e6cb63c63fb) )    /* blue palette */
	ROM_LOAD( "cboard-f4-82s129.2p",    0x0300, 0x0100, CRC(064d51a0) SHA1(d5baa29930530a8930b44a374e285de849c2a6ce) )    /* alpha color */
	ROM_LOAD( "cboard-f5-82s129.2n",    0x0400, 0x0100, CRC(7880c5af) SHA1(e4388e354420be3f99594a10c091e3d2f745cc04) )    /* background color */
	ROM_LOAD( "cboard-f8-82s129.13f",   0x0500, 0x0100, CRC(2d502464) SHA1(682b7dd22e51d5db52c0804b7e27e47641dfa6bd) )    /* vertical position low */
	ROM_LOAD( "cboard-f9-82s129.13e",   0x0600, 0x0100, CRC(027aa62c) SHA1(c7030d8b64b80e107c446f6fbdd63f560c0a91c0) )    /* vertical position med */
	ROM_LOAD( "cboard-f10-82s129.13d",  0x0700, 0x0100, CRC(1f8d0df3) SHA1(b8f17758f114f5e247b65b3f2922ca2660757e66) )    /* vertical position hi */
	ROM_LOAD( "cboard-f6-82s137.5p",    0x0800, 0x0400, CRC(8b270902) SHA1(27b3ebc92d3a2a5c0432bde018a0e43669041d50) )    /* road color */
	ROM_LOAD( "cboard-f7-82s137.7h",    0x0c00, 0x0400, CRC(a079ed19) SHA1(134b3d156a1ed0fa21cc5dc3cc84ea16ef7f84f7) )    /* sprite color */
	ROM_LOAD( "cboard-xx-82s123.15s",   0x1000, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */
	ROM_LOAD( "cboard-xx-82s123.15t",   0x1020, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */

	/* sound (P)ROM data */
	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "bboard-2p.9e",    0x0000, 0x0100, CRC(8568decc) SHA1(0aac1fa082858d4d201e21511c609a989f9a1535) )    /* Namco sound */

	ROM_REGION( 0x4000, "engine", 0 )
	ROM_LOAD( "bboard-8p.8a",   0x0000, 0x2000, CRC(b5ad4d5f) SHA1(c07e77a050200d6fe9952031f971ca35f4d15ff8) )    /* engine sound */
	ROM_LOAD( "bboard-7p.9a",   0x2000, 0x2000, CRC(8fdd2f6f) SHA1(3818dc94c60cd78c4212ab7a4367cf3d98166ee6) )    /* engine sound */

	ROM_REGION( 0x6000, "52xx", ROMREGION_ERASEFF )
	/* the bootleg has a TMS5220, NOT the Namco 52xx */

	/* unknown or unused (P)ROM data */
	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "bboard-1p.14c",   0x0000, 0x0100, CRC(0e742cb1) SHA1(3ae43270aab4848fdeece1648e7e040ab216b08e) )    /* sync chain */

	/* this is used for the Spanish speech with a TMS5220 */
	ROM_REGION( 0x2000, "soundz80bl", 0 )
	ROM_LOAD( "aboard-spi.11",  0x0000, 0x2000, CRC(47226cda) SHA1(03115ead04b11e7ef3ef08d32d4d61a56dc35190) )    /* redumped. the former one has bit6 stuck */

//  ROM_REGION( 0x2000, "pals", 0 )
//  ...not dumped yet...
ROM_END

ROM_START( grally )
	/* Z80 memory/ROM data */
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gr_niemer_bboard-5p-2764.17f",   0x0000, 0x2000, CRC(1a6412a1) SHA1(ccc41e60aad6ed332f8f2582860e11f10937dffa) )
	ROM_LOAD( "gr_niemer_bboard-6p-2732.16f",   0x2000, 0x1000, CRC(e7362148) SHA1(5a4ab037fa6a773b90c10ac4c4e9417183e0cfd8) )

	/* Z8002 #1 memory/ROM data */
	ROM_REGION( 0x10000, "sub", 0 )
	ROM_LOAD16_BYTE( "gr_niemer_bboard-1p-2764.23f",   0x0001, 0x2000, CRC(41da3c28) SHA1(c9294d686282adfc72796511c3c9e186ad057374) )
	ROM_LOAD16_BYTE( "gr_niemer_bboard-2p-2764.21f",   0x0000, 0x2000, CRC(2856d5b1) SHA1(96f5c3d67901a1abceca12b3448f381cc4852a33) )

	/* Z8002 #2 memory/ROM data */
	ROM_REGION( 0x10000, "sub2", 0 )
	ROM_LOAD16_BYTE( "gr_niemer_bboard-3p-27128.20f",   0x0001, 0x4000, CRC(6c823932) SHA1(68ef9f70c4305c3a3bacf83a64b727fd3711f34f) )
	ROM_LOAD16_BYTE( "gr_niemer_bboard-4p-27128.18f",    0x0000, 0x4000, CRC(fe9baeb6) SHA1(9a8ad2d8a69b4005f7abed278093fd57b9242bca) )

	/* graphics data */
	ROM_REGION( 0x02000, "gfx1", 0 )    /* 2bpp alpha layer */
	ROM_LOAD( "gr_niemer_cboard-12f-2764.2j",   0x0000, 0x2000, CRC(4b0a3fe9) SHA1(b0c69df94ac41a2f0705db69500cd0a224b854ad) )

	ROM_REGION( 0x02000, "gfx2", 0 )    /* 2bpp view layer */
	ROM_LOAD( "gr_niemer_cboard-15f-2764.2l",   0x0000, 0x2000, CRC(ec3ec6e6) SHA1(ae905d0ae802d1010b2c1f1a13e88a1f0dbe57da) )

	ROM_REGION( 0x04000, "gfx3", 0 )    /* 4bpp 16x16 sprites */
	ROM_LOAD( "gr_niemer_cboard-6f-2764.5a",   0x0000, 0x2000, CRC(1c72041a) SHA1(b65b09c4251ee61d247f359615e7adc7c80bc8d5) )    /* 4bpp sm sprites, planes 0+1 */
	ROM_LOAD( "gr_niemer_cboard-5f-2764.6a",   0x2000, 0x2000, CRC(1b38b257) SHA1(c7eec0692a31e1c8285bd1cba3ebd17ab253d2c9) )    /* 4bpp sm sprites, planes 2+3 */

	ROM_REGION( 0x10000, "gfx4", 0 )    /* 4bpp 32x32 sprites */
	ROM_LOAD( "gr_niemer_cboard-10f-2764.1a",     0x0000, 0x2000, CRC(613ab0df) SHA1(88aa4500275aae010fc9783c1d8d843feab89afa) )    /* 4bpp lg sprites, planes 0+1 */
	ROM_LOAD( "gr_niemer_cboard-9f-2764.2a",      0x2000, 0x2000, CRC(2d11fc01) SHA1(0515df62073db993899dde3f9ad84334c5a12fc5) )
	ROM_LOAD( "gr_niemer_cboard-8f-2764.3a",      0x4000, 0x2000, CRC(17c798b0) SHA1(ae2047bc0e4e8c85e1de09c39c200ea8f7c6a72e) )
	ROM_LOAD( "gr_niemer_cboard-7f-2764.4a",      0x6000, 0x2000, CRC(ed6a8052) SHA1(dedd6d63f9a06a1edd57cb134e86c048cff7a3c1) )
	ROM_LOAD( "gr_niemer_cboard-4f-2764.7a",      0x8000, 0x2000, CRC(5fd933e3) SHA1(5b27a8519234c935308f943cd58abc1efc463726) )    /* 4bpp lg sprites, planes 2+3 */
	ROM_LOAD( "gr_niemer_cboard-3f-2764.8a",      0xa000, 0x2000, CRC(94a7155d) SHA1(c162e2b0e93745614475326905a30d2095101913) )
	ROM_LOAD( "gr_niemer_cboard-2f-2764.9a",      0xc000, 0x2000, CRC(5fe9b365) SHA1(1a3ac099a6bb506a5f71c12c6fb14d014172371c) )
	ROM_LOAD( "gr_niemer_cboard-1fr-2764.10a",    0xe000, 0x2000, CRC(1f553db8) SHA1(897e8103e8023918dae8b45b19ad1ecb34a92b3f) )

	ROM_REGION( 0x5000, "gfx5", 0 )     /* road generation ROMs needed at runtime */
	ROM_LOAD( "gr_niemer_cboard-14f-2764.8m",   0x0000, 0x2000, CRC(ee6b3315) SHA1(9cc26c6d3604c0f60d716f86e67e9d9c0487f87d) )    /* road control */
	ROM_LOAD( "gr_niemer_cboard-13f-2764.9m",   0x2000, 0x2000, CRC(6d1e7042) SHA1(90113ff0c93ed86d95067290088705bb5e6608d1) )    /* road bits 1 */
	ROM_LOAD( "gr_niemer_cboard-16f-2732.8r",   0x4000, 0x1000, CRC(4e97f101) SHA1(f377d053821c74aee93ebcd30a4d43e6156f3cfe) )    /* road bits 2 */

	ROM_REGION( 0x1000, "gfx6", 0 )     /* sprite scaling */
	ROM_LOAD( "gr_niemer_cboard-11f-2732.9c",   0x0000, 0x1000, CRC(a61bff15) SHA1(f7a59970831cdaaa7bf59c2221a38e4746c54244) )    /* vertical scaling */

	/* graphics (P)ROM data */
	ROM_REGION( 0x1040, "proms", 0 )
	ROM_LOAD( "gr-niemer-cboard-f1-82s129.2u",    0x0000, 0x0100, CRC(16d69c31) SHA1(f24b345448e4f4ef4e2f3b057b81d399cf427f88) )    /* red palette */
	ROM_LOAD( "gr-niemer-cboard-f2-82s129.2t",    0x0100, 0x0100, CRC(07340311) SHA1(3820d1fa99013ed18de5d9400ad376cc446d1217) )    /* green palette */
	ROM_LOAD( "gr-niemer-cboard-f3-82s129.2s",    0x0200, 0x0100, CRC(1efc84d7) SHA1(6946e1c209eec0a4b75778ae88111e6cb63c63fb) )    /* blue palette */
	ROM_LOAD( "gr-niemer-cboard-f4-82s129.2p",    0x0300, 0x0100, CRC(064d51a0) SHA1(d5baa29930530a8930b44a374e285de849c2a6ce) )    /* alpha color */
	ROM_LOAD( "gr-niemer-cboard-f5-82s129.2n",    0x0400, 0x0100, CRC(7880c5af) SHA1(e4388e354420be3f99594a10c091e3d2f745cc04) )    /* background color */
	ROM_LOAD( "gr-niemer-cboard-f8-82s129.13f",   0x0500, 0x0100, CRC(2d502464) SHA1(682b7dd22e51d5db52c0804b7e27e47641dfa6bd) )    /* vertical position low */
	ROM_LOAD( "gr-niemer-cboard-f9-82s129.13e",   0x0600, 0x0100, CRC(027aa62c) SHA1(c7030d8b64b80e107c446f6fbdd63f560c0a91c0) )    /* vertical position med */
	ROM_LOAD( "gr-niemer-cboard-f10-82s129.13d",  0x0700, 0x0100, CRC(1f8d0df3) SHA1(b8f17758f114f5e247b65b3f2922ca2660757e66) )    /* vertical position hi */
	ROM_LOAD( "gr-niemer-cboard-f6-82s137.5p",    0x0800, 0x0400, CRC(8b270902) SHA1(27b3ebc92d3a2a5c0432bde018a0e43669041d50) )    /* road color */
	ROM_LOAD( "gr-niemer-cboard-f7-82s137.7h",    0x0c00, 0x0400, CRC(a079ed19) SHA1(134b3d156a1ed0fa21cc5dc3cc84ea16ef7f84f7) )    /* sprite color */
	ROM_LOAD( "gr-niemer-cboard-82s123.15s",   0x1000, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */
	ROM_LOAD( "gr-niemer-cboard-82s123.15t",   0x1020, 0x0020, CRC(4330a51b) SHA1(9531d18ce2de4eda9913d47ef8c5cd8f05791716) )    /* video RAM address decoder (not used) */

	/* sound (P)ROM data */
	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "gr_niemer_bboard-fus2p-82s129.9e",    0x0000, 0x0100, CRC(8568decc) SHA1(0aac1fa082858d4d201e21511c609a989f9a1535) )    /* Namco sound */

	ROM_REGION( 0x4000, "engine", 0 )
	ROM_LOAD( "gr_niemer_bboard-8p-2764.8a",   0x0000, 0x2000, CRC(b5ad4d5f) SHA1(c07e77a050200d6fe9952031f971ca35f4d15ff8) )    /* engine sound */
	ROM_LOAD( "gr_niemer_bboard-7p-2764.9a",   0x2000, 0x2000, CRC(8fdd2f6f) SHA1(3818dc94c60cd78c4212ab7a4367cf3d98166ee6) )    /* engine sound */

	ROM_REGION( 0x6000, "52xx", ROMREGION_ERASEFF )
	/* the bootleg has a TMS5220, NOT the Namco 52xx */

	/* unknown or unused (P)ROM data */
	ROM_REGION( 0x0100, "user1", 0 )
	ROM_LOAD( "gr_niemer_bboard-fus1p-82s129.14c",   0x0000, 0x0100, CRC(0e742cb1) SHA1(3ae43270aab4848fdeece1648e7e040ab216b08e) )    /* sync chain */

	/* this is used for the Spanish speech with a TMS5220 */
	ROM_REGION( 0x2000, "soundz80bl", 0 )
	ROM_LOAD( "gr_niemer_aboard-sp1-2764.11",  0x0000, 0x2000, CRC(47226cda) SHA1(03115ead04b11e7ef3ef08d32d4d61a56dc35190) )
ROM_END

/*********************************************************************
 * Initialization routines
 *********************************************************************/

void polepos_state::init_polepos2()
{
	/* note that the bootleg version doesn't need this custom IC; it has a hacked ROM in its place */
	m_subcpu->space(AS_PROGRAM).install_read_handler(0x4000, 0x5fff, read16sm_delegate(*this, FUNC(polepos_state::polepos2_ic25_r)));
}


/*********************************************************************
 * Game drivers
 *********************************************************************/

/*    YEAR  NAME        PARENT    MACHINE     INPUT      STATE          INIT           ROT   COMPANY                    FULLNAME                                                FLAGS */
GAME( 1982, polepos,    0,        polepos,    poleposa,  polepos_state, empty_init,    ROT0, "Namco",                   "Pole Position (World)",                                MACHINE_SUPPORTS_SAVE )
GAME( 1982, poleposj,   polepos,  polepos,    polepos,   polepos_state, empty_init,    ROT0, "Namco",                   "Pole Position (Japan)",                                MACHINE_SUPPORTS_SAVE )
GAME( 1982, poleposa1,  polepos,  polepos,    poleposa,  polepos_state, empty_init,    ROT0, "Namco (Atari license)",   "Pole Position (Atari version 1)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1982, poleposa1n, polepos,  polepos,    poleposa,  polepos_state, empty_init,    ROT0, "Namco (Atari license)",   "Pole Position (Atari version 1 on Namco PCB)",         MACHINE_SUPPORTS_SAVE )
GAME( 1982, poleposa2,  polepos,  polepos,    poleposa,  polepos_state, empty_init,    ROT0, "Namco (Atari license)",   "Pole Position (Atari version 2)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1984, topracer,   polepos,  polepos,    polepos,   polepos_state, empty_init,    ROT0, "bootleg",                 "Top Racer (with MB8841 + MB8842, 1984)",               MACHINE_SUPPORTS_SAVE ) // the NAMCO customs have been cloned on these bootlegs
GAME( 1983, topracera,  polepos,  polepos,    polepos,   polepos_state, empty_init,    ROT0, "bootleg",                 "Top Racer (with MB8841 + MB8842, 1983)",               MACHINE_SUPPORTS_SAVE ) // the only difference between them is the year displayed on the title screen
GAME( 1983, ppspeed,    polepos,  polepos,    polepos,   polepos_state, empty_init,    ROT0, "bootleg",                 "Speed Up (Spanish bootleg of Pole Position)",          MACHINE_SUPPORTS_SAVE ) // very close to topracer / topracera
GAME( 1982, topracern,  polepos,  topracern,  topracern, polepos_state, empty_init,    ROT0, "bootleg",                 "Top Racer (no MB8841 + MB8842)",                       MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND ) // explosion sound generator missing

GAME( 1983, polepos2,   0,        polepos,    polepos2j, polepos_state, init_polepos2, ROT0, "Namco",                   "Pole Position II (Japan)",                             MACHINE_SUPPORTS_SAVE )
GAME( 1983, polepos2a,  polepos2, polepos,    polepos2,  polepos_state, init_polepos2, ROT0, "Namco (Atari license)",   "Pole Position II (Atari)",                             MACHINE_SUPPORTS_SAVE )
GAME( 1983, polepos2b,  polepos2, polepos,    polepos2,  polepos_state, empty_init,    ROT0, "bootleg",                 "Pole Position II (bootleg)",                           MACHINE_SUPPORTS_SAVE )
GAME( 1984, polepos2bi, polepos2, polepos2bi, polepos2bi,polepos_state, empty_init,    ROT0, "bootleg",                 "Gran Premio F1 (Italian bootleg of Pole Position II)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND )
GAME( 1984, polepos2bs, polepos2, polepos2bi, polepos2bi,polepos_state, empty_init,    ROT0, "bootleg (BCN Internacional S.A.)", "Gran Premio F1 (Spanish bootleg of Pole Position II)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND )
GAME( 1984, grally,     polepos2, polepos2bi, polepos2bi,polepos_state, empty_init,    ROT0, "bootleg (Niemer)",        "Gran Rally (Spanish bootleg of Pole Position II)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_COLORS | MACHINE_IMPERFECT_SOUND )
