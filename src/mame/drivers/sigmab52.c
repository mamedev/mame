/******************************************************************************

  SIGMA B52 SYSTEM.
  -----------------

  Driver by Roberto Fresca.


  Games running on this hardware:

  * Joker's Wild (B52 system, set 1),        199?, Sigma.
  * Joker's Wild (B52 system, set 2),        199?, Sigma.
  * Joker's Wild (B52 system, Harrah's GFX), 199?, Sigma.


  The HD63484 ACRTC support is a bit hacky and incomplete,
  due to its preliminary emulation state.


*******************************************************************************

  Hardware Notes:
  ---------------

  CPU:

  - 2x MC68B09CP         ; 6809 CPU @ 2 MHz, from Motorola.
  - 1x HD63484P8 @ 8MHz  ; Advanced CRT controller (ACRTC), from Hitachi Semiconductor.

  RAM devices:

  - 8x TC51832ASPL-10    ; 32K x 8-bit CMOS Pseudo Static RAM.
  - 1x HM62256ALP-10     ; 32K x 8-bit High Speed CMOS Static RAM.
  - 1x HM6264ALP-10      ; 8K x 8-bit High Speed CMOS Static RAM.

  ROM devices:

  - 1x 64K main program ROM.
  - 4x 64K graphics ROM.
  - 1x 32K sound program ROM.
  - 1x 256 bytes bipolar PROM.

  Sound device:

  - 1x YM3812            ; Sound IC, from Yamaha.

  Other:

  - 2x EF68B40P          ; Frequency clock 2 MHz Programmable Timer, from SGS-Thomson Microelectronics.
  - 1x EF68B50P          ; Asynchronous Communications Interface Adapter (ACIA, 2 MHz), from SGS-Thomson Microelectronics.

  - 1x Xtal @ 18 MHz.
  - 1x Xtal @ 8 MHz.
  - 1x Xtal @ 3.579545 MHz.


  Silkscreened on main PCB:

  "SIGMA GAME INC."
  "VIDEO PCB 340016"
  "REV. A"

  Silkscreened on daughterboard:

  "SIGMA GAMES   340003"
  "BILL VALID BOARD"


  - Seems that you can set the node (01-32) for a network.
  - Cards graphics from set 2 have the Harrah's Casino logo.


*******************************************************************************


  *** Game Notes ***

  Nothing yet...


*******************************************************************************

  ---------------------------------
  ***  Memory Map (preliminary) ***
  ---------------------------------

  0x0000 - 0x3FFF    ; RAM.
  0xF730 - 0xF731    ; ACRTC.
  0xF740 - 0xF746    ; I/O.

  Sound:

  0x8000 - 0xFFFF    ; ROM space.


*******************************************************************************


  DRIVER UPDATES:


  [2010-02-04]

  - Initial release.
  - Pre-defined Xtals.
  - Started a preliminary memory map.
  - Hooked both CPUs.
  - Preliminary ACRTC support.
  - Added technical notes.


  TODO:

  - GFX decode.
  - Improve memory map.
  - Color decode.
  - Sound support.
  - Inputs.


*******************************************************************************/


#define MAIN_CLOCK	XTAL_18MHz
#define SEC_CLOCK	XTAL_8MHz
#define AUX_CLOCK	XTAL_3_579545MHz

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6850acia.h"
#include "sound/3812intf.h"
#include "video/hd63484.h"


class sigmab52_state : public driver_device
{
public:
	sigmab52_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	int m_latch;
	unsigned int m_acrtc_data;
	DECLARE_WRITE8_MEMBER(acrtc_w);
	DECLARE_READ8_MEMBER(acrtc_r);
	DECLARE_READ8_MEMBER(unk_f700_r);
	DECLARE_WRITE8_MEMBER(unk_f710_w);
	DECLARE_READ8_MEMBER(unk_f721_r);
};




/*************************
*     Video Hardware     *
*************************/


static VIDEO_START( jwildb52 )
{

}


static SCREEN_UPDATE_IND16( jwildb52 )
{
	device_t *hd63484 = screen.machine().device("hd63484");

	int x, y, b, src;

	b = ((hd63484_regs_r(hd63484, 0xcc/2, 0xffff) & 0x000f) << 16) + hd63484_regs_r(hd63484, 0xce/2, 0xffff);

//save vram to file
#if 0
	if (screen.machine().input().code_pressed_once(KEYCODE_Q))
	{
		FILE *p = fopen("vram.bin", "wb");
		fwrite(&HD63484_ram[0], 1, 0x40000 * 4, p);
		fclose(p);
	}
#endif
//copied form other acrtc based games

	for (y = 0; y < 480; y++)
	{
		for (x = 0; x < (hd63484_regs_r(hd63484, 0xca/2, 0xffff) & 0x0fff) * 4; x += 4)
		{

			src = hd63484_ram_r(hd63484, b & (HD63484_RAM_SIZE - 1), 0xffff);

			bitmap.pix16(y, x    ) = ((src & 0x000f) >>  0) << 0;
			bitmap.pix16(y, x + 1) = ((src & 0x00f0) >>  4) << 0;
			bitmap.pix16(y, x + 2) = ((src & 0x0f00) >>  8) << 0;
			bitmap.pix16(y, x + 3) = ((src & 0xf000) >> 12) << 0;
			b++;
		}
	}

if (!screen.machine().input().code_pressed(KEYCODE_O))
	if ((hd63484_regs_r(hd63484, 0x06/2, 0xffff) & 0x0300) == 0x0300)
	{
		int sy = (hd63484_regs_r(hd63484, 0x94/2, 0xffff) & 0x0fff) - (hd63484_regs_r(hd63484, 0x88/2, 0xffff) >> 8);
		int h = hd63484_regs_r(hd63484, 0x96/2, 0xffff) & 0x0fff;
		int sx = ((hd63484_regs_r(hd63484, 0x92/2, 0xffff) >> 8) - (hd63484_regs_r(hd63484, 0x84/2, 0xffff) >> 8)) * 4;
		int w = (hd63484_regs_r(hd63484, 0x92/2, 0xffff) & 0xff) * 2;
		if (sx < 0) sx = 0;	// not sure about this (shangha2 title screen)

		b = (((hd63484_regs_r(hd63484, 0xdc/2, 0xffff) & 0x000f) << 16) + hd63484_regs_r(hd63484, 0xde/2, 0xffff));


		for (y = sy; y <= sy + h && y < 480; y++)
		{
			for (x = 0; x < (hd63484_regs_r(hd63484, 0xca/2, 0xffff) & 0x0fff)* 4; x += 4)
			{
					src = hd63484_ram_r(hd63484, b & (HD63484_RAM_SIZE - 1), 0xffff);

				if (x <= w && x + sx >= 0 && x + sx < (hd63484_regs_r(hd63484, 0xca/2, 0xffff) & 0x0fff) * 4)
					{
						bitmap.pix16(y, x + sx    ) = ((src & 0x000f) >>  0) << 0;
						bitmap.pix16(y, x + sx + 1) = ((src & 0x00f0) >>  4) << 0;
						bitmap.pix16(y, x + sx + 2) = ((src & 0x0f00) >>  8) << 0;
						bitmap.pix16(y, x + sx + 3) = ((src & 0xf000) >> 12) << 0;
					}
				b++;
			}
		}
	}

	return 0;
}


static PALETTE_INIT( jwildb52 )
{

}


/*************************
*      ACRTC Access      *
*************************/

WRITE8_MEMBER(sigmab52_state::acrtc_w)
{
	device_t *hd63484 = machine().device("hd63484");
	if(!offset)
	{
		//address select
		hd63484_address_w(hd63484, 0, data, 0x00ff);
		m_latch = 0;
	}
	else
	{
		if(!m_latch)
		{
			m_acrtc_data = data;

		}

		else
		{
			m_acrtc_data <<= 8;
			m_acrtc_data |= data;

			hd63484_data_w(hd63484, 0, m_acrtc_data, 0xffff);
		}

		m_latch ^= 1;
	}
}

READ8_MEMBER(sigmab52_state::acrtc_r)
{
	if(offset&1)
	{
		device_t *hd63484 = machine().device("hd63484");
		return hd63484_data_r(hd63484, 0, 0xff);
	}

	else
	{
		return 0x7b; //fake status read (instead HD63484_status_r(&space, 0, 0xff); )
	}
}


/*************************
*      Misc Handlers     *
*************************/

READ8_MEMBER(sigmab52_state::unk_f700_r)
{
	return 0x7f;
}

WRITE8_MEMBER(sigmab52_state::unk_f710_w)
{
	memory_set_bankptr(machine(), "bank1" ,&machine().region("maincpu")->base()[0x10000 + ((data&0x80)?0x4000:0x0000)]);
}

READ8_MEMBER(sigmab52_state::unk_f721_r)
{
	return 0x04;	// test is stuck. feeding bit3 the error message appear...
}


/*************************
*      Memory Maps       *
*************************/

static ADDRESS_MAP_START( jwildb52_map, AS_PROGRAM, 8, sigmab52_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAM
	AM_RANGE(0x4000, 0x7fff) AM_RAMBANK("bank1")

	AM_RANGE(0x8000, 0xf6ff) AM_RAMBANK("bank3")

	AM_RANGE(0xf700, 0xf700) AM_READ(unk_f700_r)
	AM_RANGE(0xf710, 0xf710) AM_WRITE(unk_f710_w)
	AM_RANGE(0xf721, 0xf721) AM_READ(unk_f721_r)

	//AM_RANGE(0x00, 0x01) AM_DEVREADWRITE_LEGACY("hd63484", hd63484_status_r, hd63484_address_w)
	//AM_RANGE(0x02, 0x03) AM_DEVREADWRITE_LEGACY("hd63484", hd63484_data_r, hd63484_data_w)

	AM_RANGE(0xf730, 0xf731) AM_READWRITE(acrtc_r, acrtc_w)
	AM_RANGE(0xf740, 0xf740) AM_READ_PORT("IN0")
	AM_RANGE(0xf741, 0xf741) AM_READ_PORT("IN1")	// random checks to active high to go further with the test.
	AM_RANGE(0xf742, 0xf742) AM_READ_PORT("IN2")
	AM_RANGE(0xf743, 0xf743) AM_READ_PORT("DSW1")
	AM_RANGE(0xf744, 0xf744) AM_READ_PORT("DSW2")
	AM_RANGE(0xf745, 0xf745) AM_READ_PORT("DSW3")
	AM_RANGE(0xf746, 0xf746) AM_READ_PORT("DSW4")
	AM_RANGE(0xf800, 0xffff) AM_RAMBANK("bank2")
ADDRESS_MAP_END

/* Unknown R/W:

  F700  W
  F701 R

  F726 RW
  F727 RW

  F747 R

  F750  W

  F7D4  W
  F7D5  W

  F7E6 RW
  F7E7 RW

*/

#ifdef UNUSED_CODE
static ADDRESS_MAP_START( sound_prog_map, AS_PROGRAM, 8, sigmab52_state )
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END
#endif

/* Unknown R/W:


*/


/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( jwildb52 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("IN0-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN0-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN0-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("IN0-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN0-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN0-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("IN0-8")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_9) PORT_NAME("IN1-1")	// keys Q & O are used for debugging purposes.
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN1-2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN1-3")	// pressing this one the message "PLEASE SET #10 IN D/S-2 ON" appear. Only SW4-8 seems to has effect.
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN1-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN1-8")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("IN2-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("IN2-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN2-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN2-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN2-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN2-8")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("IN3-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("IN3-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("IN3-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("IN3-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("IN3-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("IN3-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("IN3-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("IN3-8")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("IN4-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("IN4-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("IN4-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("IN4-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("IN4-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("IN4-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("IN4-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("IN4-8")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1 - #01" )	PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW1 - #02" )	PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW1 - #04" )	PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW1 - #08" )	PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW1 - #10" )	PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW1 - #20" )	PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW1 - #40" )	PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW1 - #80" )	PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "DSW2 - #01" )	PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW2 - #02" )	PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW2 - #04" )	PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW2 - #08" )	PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW2 - #10" )	PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW2 - #20" )	PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW2 - #40" )	PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW2 - #80" )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "DSW3 - #01" )	PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW3 - #02" )	PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW3 - #04" )	PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW3 - #08" )	PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW3 - #10" )	PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW3 - #20" )	PORT_DIPLOCATION("SW3:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW3 - #40" )	PORT_DIPLOCATION("SW3:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW3 - #80" )	PORT_DIPLOCATION("SW3:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )


	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, "DSW4 - #01" )	PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW4 - #02" )	PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW4 - #04" )	PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW4 - #08" )	PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW4 - #10" )	PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW4 - #20" )	PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW4 - #40" )	PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW4 - #80" )	PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


/******************************
*   Interrupts Gen (to fix)   *
******************************/

static INTERRUPT_GEN( timer_irq )
{
	generic_pulse_irq_line(device, M6809_IRQ_LINE, 1);
}


/*************************
*     Machine Start      *
*************************/

static MACHINE_START(jwildb52)
{
	memory_set_bankptr(machine, "bank1", &machine.region("maincpu")->base()[0x10000 + 0x0000]);

	memory_set_bankptr(machine, "bank2", &machine.region("maincpu")->base()[0x10000 + 0xf800]);

	memory_set_bankptr(machine, "bank3", &machine.region("maincpu")->base()[0x10000 + 0x8000]);

/*

  ACRTC memory:

  00000-3ffff = RAM
  40000-7ffff = ROM
  80000-bffff = unused
  c0000-fffff = unused

*/

	{
		UINT16 *rom = (UINT16*)machine.region("gfx1")->base();
		int i;

		device_t *hd63484 = machine.device("hd63484");

		for(i = 0; i < 0x40000/2; ++i)
		{
			hd63484_ram_w(hd63484, i + 0x40000/2, rom[i], 0xffff);
		}
	}
}

static const hd63484_interface jwildb52_hd63484_intf = { 1 };


/*************************
*    Machine Drivers     *
*************************/

static MACHINE_CONFIG_START( jwildb52, sigmab52_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, MAIN_CLOCK/9)	/* 2 MHz */
	MCFG_CPU_PROGRAM_MAP(jwildb52_map)
	MCFG_CPU_VBLANK_INT("screen", timer_irq)	/* Fix me */

#if 0
	MCFG_CPU_ADD("audiocpu", M6809, MAIN_CLOCK/9)	/* 2 MHz */
	MCFG_CPU_PROGRAM_MAP(sound_prog_map)
	//temporary teh same int as for main cpu
	MCFG_CPU_PERIODIC_INT(timer_irq, 1000)			/* Fix me */
#endif

	MCFG_MACHINE_START(jwildb52)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(30)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(1024, 1024)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 384-1)
	MCFG_SCREEN_UPDATE_STATIC(jwildb52)

	MCFG_HD63484_ADD("hd63484", jwildb52_hd63484_intf)

	MCFG_PALETTE_INIT(jwildb52)
	MCFG_PALETTE_LENGTH(256)

	MCFG_VIDEO_START(jwildb52)

MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

ROM_START( jwildb52 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "poker.ic95", 0x10000, 0x10000, CRC(07eb9007) SHA1(ee814c40c6d8c9ea9e5246cae0cfa2c30f2976ed) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "cards_2001-1.ic45", 0x00002, 0x10000, CRC(7664455e) SHA1(c9f129060e63b9ac9058ab94208846e4dc578ead) )
	ROM_LOAD32_BYTE( "cards_2001-2.ic46", 0x00000, 0x10000, CRC(c1455d64) SHA1(ddb576ba471b5d2faa415ec425615cf5f9d87911) )
	ROM_LOAD32_BYTE( "cards_2001-3.ic47", 0x00001, 0x10000, CRC(cb2ece6e) SHA1(f2b6949085fe395d0fdd16322a880ec87e2efd50) )
	ROM_LOAD32_BYTE( "cards_2001-4.ic48", 0x00003, 0x10000, CRC(8131d236) SHA1(8984aa1f2af70df41973b61df17f184796a2ffe9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound-01-00.43",	0x8000, 0x8000, CRC(2712d44c) SHA1(295526b27676cd97cbf111d47305d63c2b3ea50d) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7118.41", 0x0000, 0x0100, CRC(b362f9e2) SHA1(3963b40389ed6584e4cd96ab48849552857d99af) )
ROM_END


ROM_START( jwildb52a )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "sigm_wrk.bin", 0x10000, 0x10000, CRC(15c83c6c) SHA1(7a05bd94ea8b1ad051fbe6580a6550d4bb47dd15) )

	/* No gfx & sound dumps. Using the ones from parent set for now... */

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "cards_2001-1.ic45", 0x00002, 0x10000, BAD_DUMP CRC(7664455e) SHA1(c9f129060e63b9ac9058ab94208846e4dc578ead) )
	ROM_LOAD32_BYTE( "cards_2001-2.ic46", 0x00000, 0x10000, BAD_DUMP CRC(c1455d64) SHA1(ddb576ba471b5d2faa415ec425615cf5f9d87911) )
	ROM_LOAD32_BYTE( "cards_2001-3.ic47", 0x00001, 0x10000, BAD_DUMP CRC(cb2ece6e) SHA1(f2b6949085fe395d0fdd16322a880ec87e2efd50) )
	ROM_LOAD32_BYTE( "cards_2001-4.ic48", 0x00003, 0x10000, BAD_DUMP CRC(8131d236) SHA1(8984aa1f2af70df41973b61df17f184796a2ffe9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sound-01-00.43",	0x8000, 0x8000, BAD_DUMP CRC(2712d44c) SHA1(295526b27676cd97cbf111d47305d63c2b3ea50d) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7118.41", 0x0000, 0x0100, BAD_DUMP CRC(b362f9e2) SHA1(3963b40389ed6584e4cd96ab48849552857d99af) )
ROM_END


ROM_START( jwildb52h )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "jokers_wild_ver_xxx.ic95", 0x10000, 0x10000, CRC(07eb9007) SHA1(ee814c40c6d8c9ea9e5246cae0cfa2c30f2976ed) )

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "2006-1_harrahs.ic45", 0x00002, 0x10000, CRC(6e6871dc) SHA1(5dfc99c808c06ec34838324181988d4550c1ed1a) )
	ROM_LOAD32_BYTE( "2006-2_harrahs.ic46", 0x00000, 0x10000, CRC(1039c62d) SHA1(11f0dbcbbff5f6e9028a0305f7e16a0654be40d4) )
	ROM_LOAD32_BYTE( "2006-3_harrahs.ic47", 0x00001, 0x10000, CRC(d66af95a) SHA1(70bba1aeea9221541b82642045ce8ecf26e1d08c) )
	ROM_LOAD32_BYTE( "2006-4_harrahs.ic48", 0x00003, 0x10000, CRC(2bf196cb) SHA1(686ca0dd84c48f51efee5349ea3db65531dd4a52) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "poker-01-00.43", 0x8000, 0x8000, CRC(2712d44c) SHA1(295526b27676cd97cbf111d47305d63c2b3ea50d) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7118.41", 0x0000, 0x0100, CRC(b362f9e2) SHA1(3963b40389ed6584e4cd96ab48849552857d99af) )
ROM_END


/*************************
*      Driver Init       *
*************************/

static DRIVER_INIT(jwildb52)
{
	//HD63484_start(machine);
}


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME       PARENT    MACHINE   INPUT     INIT      ROT    COMPANY  FULLNAME                                  FLAGS */
GAME( 199?, jwildb52,  0,        jwildb52, jwildb52, jwildb52, ROT0, "Sigma", "Joker's Wild (B52 system, set 1)",        GAME_NO_SOUND | GAME_NOT_WORKING )
GAME( 199?, jwildb52a, jwildb52, jwildb52, jwildb52, jwildb52, ROT0, "Sigma", "Joker's Wild (B52 system, set 2)",        GAME_NO_SOUND | GAME_NOT_WORKING )
GAME( 199?, jwildb52h, jwildb52, jwildb52, jwildb52, jwildb52, ROT0, "Sigma", "Joker's Wild (B52 system, Harrah's GFX)", GAME_NO_SOUND | GAME_NOT_WORKING )
