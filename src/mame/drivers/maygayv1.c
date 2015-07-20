// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    MAYGAY MV1 hardware

    preliminary driver by Phil Bennett

    Games supported:
        * Screen Play

    Other games on this hardware (incomplete dumps)
        * Believe it or not?
        * Caesar's Palace (reel to video)
        * Crossword Quiz
        * Give us a Clue
        * Special Effects (reel to video)
        * World Cup (reel to video)


Main MV1 board:

U1 ST 8901 TS68000CP12
U2 ST M74HC04B1 99135R
U3 ST 16S25HB1 9235 (think this is the gal chip)
U4 ST M74HC138B1 99131R
U5 P8948G MM74HC05N
U6 ST M74HC74B1 99144R
U7 ST M74HC08B1 99135R
U8 ST NE556N 99135
U9 ST M74HC259B1 99148R
U10 ST M74HC237B1 99148R
U11 ULN2803A 9135
U12 ST M74HC74B1 99144R
U13 ST M74HC32B1 99041R
U14 ST M74HCO4B1 99135R
U15 27C010 (Game Rom)
U16 27C010 (Game Rom)
U17 27C010 (Game Rom)
U18 27C010 (Game Rom)
U19 HYUNDAI HY6264ALP- 10 9147T
U20 HYUNDAI HY6264ALP- 10 9147T
U21 NEC IRELAND D8279C- 2 9135X8006]
U22 ULN2803A 9135
U23 ULN2803A 9135
U24 TEXAS INSTRUMENT F 9140 AN SN74HC148N
U25 ST 2 9148 EF68B21P
U26 Can't see one on the board!
U27 MOTOROLA MC68681P 14PT18715
U28 ST MC1488P 99136
U29 ST MC1489AP 99148
U30 ST HCF 4514 BE 2 9049
U31 MOTOROLA MC74F139N XXAA9145
U32 TEXAS INSTRUMENT TMS4464- 12NL IHE 9145
U33 TEXAS INSTRUMENT TMS4464- 12NL IHE 9114
U34 TEXAS INSTRUMENT TMS4464- 12NL IHE 9145
U35 TEXAS INSTRUMENT TMS4464- 12NL IHE 9114
U36 MHS S-82716-4 9210
U37 TEXAS INSTRUMENT TMS4464- 12NL IHE 9114
U38 TEXAS INSTRUMENT TMS4464- 12NL IHE 9145
U39 TEXAS INSTRUMENT TMS4464- 12NL IHE 9145
U40 TEXAS INSTRUMENT TMS4464- 12NL IHE 9114
U41 ST M74HC244B1 99131R
U42 ST M74HC244B1 99131R
U43 ST M74HC245B1 99136R
U44 ST M74HC245B1 99136R
U45 ST M74HC00B1 99135R
U46 ULN2803A 9135
U47 ULN2803A 9135
U48 YM2413 9127 HADG
U49 78L05 .194

The memory card that plugs in has 4 M27C010 game roms on.

The extra digital and reel board has the following:

U1 GS G06 KOREA GD74HC245
U2 SIEMENS SAB 8032B-P SINGAPORE BB INTEL 80 9148
U3 M27C512 (reels rom)
U4 ST M74HC373B1 99205R
U5 TEXAS INSTRUMENT 14530QT SN75155P
U6 ULN 2803A 9203
U7 ST M74HC374B1 99205R
U8 ST M74HC374B1 99205R
U9 ULN 2803A 9203
U10 ULN 2803A 9203
U11 ULN 2803A 9203
U12 M27C010 (Sound Rom)
U13 NEC JAPAN D7759C 9015KP009
U14 ST M74HC373B1 99205R
U15 Can't see one on the board!
U16 MOTOROLA MC74HC04AN FFA09202




upd7759 change:

* Only accept FIFO bytes when the chip is playing!!!

Port 1 is connected directly to the upd bus.
upd /CE is grounded
/WR bit is

Toggle reset pin
Write data on port 1
Toggle WR bit...

Then, it goes off to write.

INT1  Power failure?
INT2  -
INT3  V Sync
INT4  -
INT5  68681 DUART
INT6  -
INT7  -


DUART: 0 = Power failure

M68681 Output port drives slides

Todo:

Find lamps/reels after UPD changes.
***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "video/awpvid.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/i8279.h"
#include "machine/6821pia.h"
#include "machine/mc68681.h"
#include "sound/2413intf.h"
#include "sound/upd7759.h"
#include "machine/nvram.h"


/*************************************
 *
 *  Defines
 *
 *************************************/

#define DUART_CLOCK     XTAL_3_6864MHz
#define PIXEL_CLOCK     0
#define MASTER_CLOCK    XTAL_16MHz
#define SOUND_CLOCK     XTAL_11_0592MHz

/*************************************
 *
 *  Video hardware
 *
 *************************************/


#define VREG(a)     i82716.r[a]

enum
{
	VCR0 = 0,
	VCR1,
	RWBA,
	DWBA,
	DWSLM,
	DSBA,
	PAQ,
	ODTBA,
	ATBA,
	CTBA,
	CBASE,
	ATBAC,
	HVCONST0,
	HVCONST1,
	HVCONST2,
	HVCONST3
};

#define VCR0_UCF            0x0001
#define VCR0_DEI            0x0002
#define VCR0_SAB            0x0004
#define VCR0_DEN            0x0008
#define VCR0_HRS            0x0010
#define VCR0_DOF            0x0020

#define VCR0_DS_MASK        0x00c0
#define VCR0_DS_SHIFT       6
#define VCR0_BLINK_MASK     0x1f00
#define VCR0_BLINK_SHIFT    8
#define VCR0_DUTY_MASK      0xe000
#define VCR0_DUTY_SHIFT     13

static const UINT32 banks[4] = { 0, 0x40000/2, 0x20000/2, 0x60000/2 };

#define DRAM_BANK_SEL       (banks[(VREG(DSBA) >> 7) & 3])

struct i82716_t
{
	UINT16  r[16];
	UINT16  *dram;

	UINT8   *line_buf;  // there's actually two
};


class maygayv1_state : public driver_device
{
public:
	maygayv1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_upd7759(*this, "upd"),
		m_duart68681(*this, "duart68681"),
		m_palette(*this, "palette") { }

	required_device<cpu_device> m_maincpu;
	required_device<i8052_device> m_soundcpu;
	required_device<upd7759_device> m_upd7759;
	required_device<mc68681_device> m_duart68681;
	required_device<palette_device> m_palette;

	int m_lamp_strobe;
	int m_old_lamp_strobe;
	int m_vsync_latch_preset;
	UINT8 m_p1;
	UINT8 m_p3;
	int m_d68681_val;
	i82716_t m_i82716;
	DECLARE_WRITE16_MEMBER(i82716_w);
	DECLARE_READ16_MEMBER(i82716_r);
	DECLARE_WRITE16_MEMBER(write_odd);
	DECLARE_READ16_MEMBER(read_odd);
	DECLARE_WRITE16_MEMBER(vsync_int_ctrl);
	DECLARE_READ8_MEMBER(mcu_r);
	DECLARE_WRITE8_MEMBER(mcu_w);
	DECLARE_READ8_MEMBER(b_read);
	DECLARE_WRITE8_MEMBER(b_writ);
	DECLARE_WRITE8_MEMBER(strobe_w);
	DECLARE_WRITE8_MEMBER(lamp_data_w);
	DECLARE_READ8_MEMBER(kbd_r);
	DECLARE_DRIVER_INIT(screenpl);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_maygayv1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_maygayv1(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(vsync_interrupt);
	DECLARE_WRITE8_MEMBER(data_from_i8031);
	DECLARE_READ8_MEMBER(data_to_i8031);
	DECLARE_WRITE_LINE_MEMBER(duart_irq_handler);
	DECLARE_WRITE_LINE_MEMBER(duart_txa);
};




WRITE16_MEMBER(maygayv1_state::i82716_w)
{
	i82716_t &i82716 = m_i82716;
	// Accessing register window?
	if ((VREG(RWBA) & 0xfff0) == (offset & 0xfff0))
	{
		// Register segment is fixed at start of DRAM
		COMBINE_DATA(&i82716.dram[offset & 0xf]);
	}

	// Accessing data window?
	// TODO: mask
	if (offset >= (VREG(DWBA) & 0xf800))
	{
		offset -= (VREG(DWBA) & 0xf800);
		COMBINE_DATA(&i82716.dram[DRAM_BANK_SEL + (VREG(DSBA) & 0xf800) + offset]);
	}
}

READ16_MEMBER(maygayv1_state::i82716_r)
{
	i82716_t &i82716 = m_i82716;
	// Accessing register window?
	if ((VREG(RWBA) & ~0xf) == (offset & ~0xf))
	{
		return(i82716.r[offset & 0xf]);
	}

	// Accessing data window? TODO: mask?
	if (VREG(VCR1) & 4)
	{
		if (offset >= (VREG(DWBA) & 0xf800))
		{
			offset -= (VREG(DWBA) & 0xf800);
			return i82716.dram[DRAM_BANK_SEL +(VREG(DSBA) & 0xf800) + (offset)];
		}
	}

	return 0;
}

void maygayv1_state::video_start()
{
}


UINT32 maygayv1_state::screen_update_maygayv1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	i82716_t &i82716 = m_i82716;
	UINT16 *atable = &i82716.dram[VREG(ATBA)];
	UINT16 *otable = &i82716.dram[VREG(ODTBA) & 0xfc00];  // both must be bank 0

	int sl, sx;
	int slmask = 0xffff;     // TODO: Save if using scanline callbacks
	int xbound = (VREG(DWBA) & 0x3f8) | 7;

	/* Sign extend to 10 bits */
	xbound = (xbound & 0x3ff) - (xbound & 0x400);

	/* If screen output is disabled, fill with black */
	if (!(VREG(VCR0) & VCR0_DEN))
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	/* For every scanline... */
	for (sl = cliprect.min_x; sl <= cliprect.max_y; ++sl)
	{
		int obj;
		UINT16 aflags = atable[sl];
		UINT16 slmask_old = slmask;

		UINT16 *bmp_ptr = &bitmap.pix16(sl);

		slmask = 0xffff ^ (slmask ^ aflags);

		/* Clear the frame buffer on each line to BG colour (palette entry 2) */
		/* 4bpp only ! */
		memset(i82716.line_buf, 0x22, 512);

		/* Parse the list of 16 objects */
		for (obj = 0; obj < 16; ++obj)
		{
			int offs = obj * 4;

			// Draw on this line?
			if ( !BIT(slmask, obj) )
			{
				UINT32  objbase, trans, width;
				INT32   x, xpos;
				UINT16  w0, w1, w2;
				UINT16  *objptr;
				UINT8 *bmpptr; // ?

				/* Get object table entry words */
				w0 = otable[offs];
				w1 = otable[offs + 1];
				w2 = otable[offs + 2];

				/* Blanked */
				if ( BIT(w0, 4) )
					break;

//              if ( BIT(w0, 5) )
//                  printf("Blinking\n");

				/* Resolution: either 4bpp or 2bpp */
//              res = (w0 >> 9) & 3;


				/* Expand 2bpp to 3bpp */
				//cspec = (w0 & 3) << 2;

				if ( BIT(w0, 11) )
				{
					logerror("i82716: Characters not supported\n");
					break;
				}

				/* 10-bit signed - in terms of bytes */
				xpos = (w1 & 0x3ff);
				xpos = (xpos & 0x3ff) - (xpos & 0x400);

				/* Transparency detect enable */
				trans = BIT(w0, 2);

				/* Width is expressed in units of 64-bit words */
				width = (w1 >> 10) & 0x3f;

				/* First scanline? Clear current object entry address */
				if ( BIT(slmask_old, obj) )
					otable[offs + 3] = 0;

				/* Bitmap data pointer */
				objbase = ((w0 & 0x00c0) << 10) | w2;
				objptr = &i82716.dram[objbase + ((4 * width) * otable[offs + 3])];

				// endian alert
				bmpptr = (UINT8*)objptr;

				// 4bpp
				for (x = xpos; x < MIN(xbound, xpos + width * 8); ++x)
				{
					if (x >= 0)
					{
						UINT8 p1 = *bmpptr & 0xf;
						UINT8 p2 = *bmpptr >> 4;

						if (!trans || p1)
							i82716.line_buf[x] = p1;

						if (!trans || p2)
							i82716.line_buf[x] |= p2 << 4;
					}
					bmpptr++;
				}

				/* Update scanline pointer - WRONG */
				otable[offs + 3]++;
			}
		}

		// Write it out
		for (sx = cliprect.min_x; sx < cliprect.max_x; sx += 2)
		{
			UINT8 pix = i82716.line_buf[sx / 2];

			bmp_ptr[sx + 0] = pix & 0xf;
			bmp_ptr[sx + 1] = pix >> 4;
		}
	}

	return 0;
}

void maygayv1_state::screen_eof_maygayv1(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		i82716_t &i82716 = m_i82716;
		// UCF
		if (VREG(VCR0) & VCR0_UCF)
		{
			int i;

			for (i = 0; i < 16; ++i)
				VREG(i) = i82716.dram[i];
		}
		else
		{
			VREG(VCR0) = i82716.dram[VCR0];
			VREG(ATBA) = i82716.dram[ATBA];
		}

		if (!(VREG(VCR0) & VCR0_DEI))
		{
			int i;
			UINT16 *palbase = &i82716.dram[VREG(CTBA)];

			for (i = 0; i < 16; ++i)
			{
				UINT16 entry = *palbase++;
				m_palette->set_pen_color(entry & 0xf, pal4bit(entry >> 12), pal4bit(entry >> 8), pal4bit(entry >> 4));
			}
		}
	}
}



/*************************************
 *
 *  68000 CPU memory handlers
 *
 *************************************/

/*
    68681
    YM2413
    68B21
    8279C

    8a0008 0xe0
    8a000c 0x7
    8a000e 0x33
    8a000a 0x8
    8a001c 0xff R/W
*/



WRITE16_MEMBER(maygayv1_state::write_odd)
{
}

//;860008 is a latch of some sort
READ16_MEMBER(maygayv1_state::read_odd)
{
	return 0;
}

/*************************************
 *
 *  8279 display/keyboard driver
 *
 *************************************/

WRITE8_MEMBER( maygayv1_state::strobe_w )
{
	m_lamp_strobe = data;
}

WRITE8_MEMBER( maygayv1_state::lamp_data_w )
{
	//The two A/B ports are merged back into one, to make one row of 8 lamps.

	if (m_old_lamp_strobe != m_lamp_strobe)
	{
		// Because of the nature of the lamping circuit, there is an element of persistance
		// As a consequence, the lamp column data can change before the input strobe without
		// causing the relevant lamps to black out.

		for (int i = 0; i < 8; i++)
		{
			output_set_lamp_value((8*m_lamp_strobe)+i, ((data  & (1 << i)) !=0));
		}
		m_old_lamp_strobe = m_lamp_strobe;
	}

}

READ8_MEMBER( maygayv1_state::kbd_r )
{
	static const char *const portnames[] = { "STROBE1","STROBE2","STROBE3","STROBE4","STROBE5","STROBE6","STROBE7","STROBE8" };

	return ioport(portnames[m_lamp_strobe&0x07])->read();
}

WRITE16_MEMBER(maygayv1_state::vsync_int_ctrl)
{
	m_vsync_latch_preset = data & 0x0100;

	// Active low
	if (!(m_vsync_latch_preset))
		m_maincpu->set_input_line(3, CLEAR_LINE);
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, maygayv1_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x080000, 0x083fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x100000, 0x17ffff) AM_ROM AM_REGION("maincpu", 0x80000)
	AM_RANGE(0x820000, 0x820001) AM_DEVREADWRITE8("i8279", i8279_device, data_r, data_w ,0xff)
	AM_RANGE(0x820002, 0x820003) AM_DEVREADWRITE8("i8279", i8279_device, status_r, cmd_w,0xff)
	AM_RANGE(0x800000, 0x800003) AM_DEVWRITE8("ymsnd", ym2413_device, write, 0xff00)
	AM_RANGE(0x860000, 0x86000d) AM_READWRITE(read_odd, write_odd)
	AM_RANGE(0x86000e, 0x86000f) AM_WRITE(vsync_int_ctrl)
	AM_RANGE(0x880000, 0x89ffff) AM_READWRITE(i82716_r, i82716_w)
	AM_RANGE(0x8a0000, 0x8a001f) AM_DEVREADWRITE8("duart68681", mc68681_device, read, write, 0xff)
	AM_RANGE(0x8c0000, 0x8c000f) AM_DEVREAD8("pia", pia6821_device, read, 0x00ff)
	AM_RANGE(0x8c0000, 0x8c000f) AM_DEVWRITE8("pia", pia6821_device, write, 0xff00)
ADDRESS_MAP_END


/*************************************
 *
 *  8032 CPU memory handlers
 *
 *************************************/

/*
 74HC245 @ U1 read port
 (P3.4 = /ENABLE)
 P1.0 = Reel 1 optic (I)
 P1.1 = Reel 2 optic (I)
 P1.2 = Reel 3 optic (I)
 P1.3
 P1.4 = DIPSWITCH (GND/ON by default)
 P1.5 = DIPSWITCH (GND/ON by default)
 P1.6 = DIPSWITCH (GND/ON by default)
 P1.7 = DIPSWITCH (GND/ON by default)

 WRITE
 P1.0 - 7 => D7759C
          => 74HC374 @ U8 write port (CLK = 3.7)

 P3.0 = RXD
 P3.1 = TXD
 P3.2 = /UPD_RESET
 P3.3 = !(/UPD_BUSY)
 P3.4 = U1 /ENABLE
 P3.5 = Status LED (inverted twice)
 P3.6 = (WR) /UPD_START
 P3.7 = (RD) U8 CLK - for writing! P1



*/

READ8_MEMBER(maygayv1_state::mcu_r)
{
	switch (offset)
	{
		case 1:
		{
			if ( !BIT(m_p3, 4) )
				return (ioport("REEL")->read());    // Reels???
			else
				return 0;
		}

		case 3: return m_upd7759->busy_r() ? 0 : 0x08;
	}
	return 0;
}

WRITE8_MEMBER(maygayv1_state::mcu_w)
{
			logerror("O %x D %x",offset,data);

	switch (offset)
	{
		// Bottom nibble = UPD
		case 1:
			m_p1 = data;
//          m_upd7759->msg_w(data);//?
			break;
		case 3:
			m_upd7759->reset_w (BIT(data, 2));
			m_upd7759->start_w(BIT(data, 6));

//          if ( !BIT(m_p3, 7) && BIT(data, 7) )
				// P1 propagates to outputs

			m_p3 = data;
			break;
	}
}


static ADDRESS_MAP_START( sound_prg, AS_PROGRAM, 8, maygayv1_state )
	AM_RANGE(0x0000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_data, AS_DATA, 8, maygayv1_state )
	AM_RANGE(0x0000, 0xffff) AM_RAM // nothing?
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io, AS_IO, 8, maygayv1_state )
	AM_RANGE(0x00, 0xff) AM_READWRITE(mcu_r, mcu_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( screenpl )
	PORT_START("STROBE1")
	PORT_DIPNAME( 0x01, 0x01, "DSW01")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW02")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW03")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW04")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Stake selection?")
	PORT_DIPSETTING(    0x10, "5p" )
	PORT_DIPSETTING(    0x00, "10p" )
	PORT_DIPNAME( 0x20, 0x20, "DSW06")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW07")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW08")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("STROBE2")
	PORT_DIPNAME( 0x01, 0x01, "Test")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Unknown 1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Nudge 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Nudge 2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Nudge 3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Unknown 2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Collect")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Unknown 3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("Spin")

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Red")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Yellow")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Green")
	PORT_DIPNAME( 0x08, 0x08, "DSW34")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW35")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Jackpot Selection")
	PORT_DIPSETTING(    0x20, "600p" )
	PORT_DIPSETTING(    0x00, "300p" )
	PORT_DIPNAME( 0x40, 0x40, "Reset?")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Cash door")
	PORT_DIPSETTING(    0x80, "Open"   )
	PORT_DIPSETTING(    0x00, "Closed" )

	PORT_START("STROBE5")
	PORT_DIPNAME( 0x01, 0x01, "DSW41")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Re-fill key")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW43")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW44")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW45")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW46")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW47")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW48")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("STROBE6")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE7")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("STROBE8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN5 ) PORT_NAME("Token")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("100p")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("50p")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("20p")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("10p")
	PORT_BIT( 0xe0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("REEL")
	PORT_DIPNAME( 0x01, 0x00, "REEL 1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "REEL 2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "REEL 3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "REEL 4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "MCU DIP1")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "MCU DIP2")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "MCU DIP3")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "MCU DIP4")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

/***************************************************************************

    68681 DUART

***************************************************************************/

WRITE_LINE_MEMBER(maygayv1_state::duart_irq_handler)
{
	m_maincpu->set_input_line_and_vector(5, state, m_duart68681->get_irq_vector());
//  m_maincpu->set_input_line(5, state ? ASSERT_LINE : CLEAR_LINE);
}


WRITE_LINE_MEMBER(maygayv1_state::duart_txa)
{
	m_d68681_val = state;
	m_soundcpu->set_input_line(MCS51_RX_LINE, ASSERT_LINE);  // ?
}

READ8_MEMBER(maygayv1_state::data_to_i8031)
{
	return m_d68681_val;
}

WRITE8_MEMBER(maygayv1_state::data_from_i8031)
{
	m_duart68681->rx_a_w(data);
}

READ8_MEMBER(maygayv1_state::b_read)
{
	// Meters - upper nibble?
	return 0xff;
}

WRITE8_MEMBER(maygayv1_state::b_writ)
{
	logerror("B WRITE %x\n",data);
}


void maygayv1_state::machine_start()
{
	i82716_t &i82716 = m_i82716;
	i82716.dram = auto_alloc_array(machine(), UINT16, 0x80000/2);   // ???
	i82716.line_buf = auto_alloc_array(machine(), UINT8, 512);

	save_pointer(NAME(i82716.dram), 0x40000);

	m_soundcpu->i8051_set_serial_tx_callback(write8_delegate(FUNC(maygayv1_state::data_from_i8031),this));
	m_soundcpu->i8051_set_serial_rx_callback(read8_delegate(FUNC(maygayv1_state::data_to_i8031),this));
}

void maygayv1_state::machine_reset()
{
	i82716_t &i82716 = m_i82716;
	// ?
	memset(i82716.dram, 0, 0x40000);
	i82716.r[RWBA] = 0x0200;
}


INTERRUPT_GEN_MEMBER(maygayv1_state::vsync_interrupt)
{
	if (m_vsync_latch_preset)
		m_maincpu->set_input_line(3, ASSERT_LINE);
}


static MACHINE_CONFIG_START( maygayv1, maygayv1_state )
	MCFG_CPU_ADD("maincpu", M68000, MASTER_CLOCK / 2)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", maygayv1_state,  vsync_interrupt)

	MCFG_CPU_ADD("soundcpu", I8052, SOUND_CLOCK)
	MCFG_CPU_PROGRAM_MAP(sound_prg)
	MCFG_CPU_DATA_MAP(sound_data)
	MCFG_CPU_IO_MAP(sound_io)

	/* U25 ST 2 9148 EF68B21P */
	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(maygayv1_state, b_read))
	MCFG_PIA_READPB_HANDLER(READ8(maygayv1_state, b_read))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(maygayv1_state, b_writ))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(maygayv1_state, b_writ))

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* TODO: Use real video timings */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(640, 300)
	MCFG_SCREEN_VISIBLE_AREA(0, 640 - 1, 0, 300 - 1)
	MCFG_SCREEN_UPDATE_DRIVER(maygayv1_state, screen_update_maygayv1)
	MCFG_SCREEN_VBLANK_DRIVER(maygayv1_state, screen_eof_maygayv1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)

	MCFG_MC68681_ADD("duart68681", DUART_CLOCK)
	MCFG_MC68681_IRQ_CALLBACK(WRITELINE(maygayv1_state, duart_irq_handler))
	MCFG_MC68681_A_TX_CALLBACK(WRITELINE(maygayv1_state, duart_txa))

	MCFG_DEVICE_ADD("i8279", I8279, MASTER_CLOCK/4)    // unknown clock
	MCFG_I8279_OUT_SL_CB(WRITE8(maygayv1_state, strobe_w))      // scan SL lines
	MCFG_I8279_OUT_DISP_CB(WRITE8(maygayv1_state, lamp_data_w)) // display A&B
	MCFG_I8279_IN_RL_CB(READ8(maygayv1_state, kbd_r))                   // kbd RL lines

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd",YM2413, MASTER_CLOCK / 4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.8)

	MCFG_SOUND_ADD("upd",UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( screenpl )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20p6pnd_u15.bin", 0x00000, 0x20000, CRC(4334453c) SHA1(5c18acf29c7b3f44589b92d74b79ac66730db810) )
	ROM_LOAD16_BYTE( "20p6pnd_u16.bin", 0x00001, 0x20000, CRC(90b3f67c) SHA1(a58a0bc4ccccf083fe3222f02eb06ee5fa6f386a) )
	ROM_LOAD16_BYTE( "20p6pnd_u17.bin", 0x40000, 0x20000, CRC(ba576b11) SHA1(3ba7bcaf4e3cc4eaeeece6e3f4957c3a8dfd5752) )
	ROM_LOAD16_BYTE( "20p6pnd_u18.bin", 0x40001, 0x20000, CRC(24dd1aff) SHA1(833c59e5b75130a8dc3a63027e09c0f5c7ed17f5) )

	ROM_LOAD16_BYTE( "20p6pnd_u2.bin",  0x80000, 0x20000, CRC(ee51ed98) SHA1(262e773cdb1465983a8f931698bc73de7c324088) )
	ROM_LOAD16_BYTE( "20p6pnd_u1.bin",  0x80001, 0x20000, CRC(d57bbe69) SHA1(b7cd93cef4828418328ca4ff16c496da7b2065e2) )
	ROM_LOAD16_BYTE( "20p6pnd_u4.bin",  0xc0000, 0x20000, CRC(01aafd7e) SHA1(d2161066655218468da8eae8aa9da8a80c07c489) )
	ROM_LOAD16_BYTE( "20p6pnd_u3.bin",  0xc0001, 0x20000, CRC(aa02dc54) SHA1(a05c8c26480f3bae671428380c7684b9c29b5a53) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "reels.bin", 0x0000, 0x10000, CRC(1319cf82) SHA1(7a233072890361bcf384de4f90170c2ca713b1de) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "sound.bin", 0x00000, 0x20000, CRC(498dd74f) SHA1(80bb204b3e9cadcecbfa75c78c52fb9908566c5e) )
ROM_END

ROM_START( screenp1 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sa4-379.u15", 0x00000, 0x20000, CRC(e21b120d) SHA1(bcdebf797381b0c585dbd3511b5e984f22de2206) )
	ROM_LOAD16_BYTE( "sa4-379.u16", 0x00001, 0x20000, CRC(b04588b7) SHA1(1f9b933e441969c95bbbabbfbe44349bd945c326) )
	ROM_LOAD16_BYTE( "sa4-379.u17", 0x40000, 0x20000, CRC(4b6cdc43) SHA1(1d6a4796ce67d0d00fe74a6bafd8b731450cdaab) )
	ROM_LOAD16_BYTE( "sa4-379.u18", 0x40001, 0x20000, CRC(d986355f) SHA1(86d3f1712cd1bcc90a54945a2baccae2596de691) )

	ROM_LOAD16_BYTE( "sq3-458.u2",  0x80000, 0x20000, CRC(7091dfcd) SHA1(d28abd70db5c49baa93f0488e443f29c27a7a559) )
	ROM_LOAD16_BYTE( "sq3-458.u1",  0x80001, 0x20000, CRC(1bb0efbf) SHA1(59d7e2e51928df149764502bc4bd5736463f40d7) )
	ROM_LOAD16_BYTE( "sq3-458.u4",  0xc0000, 0x20000, CRC(0fb0fc84) SHA1(e7ef68130f9627a842849f41f67accf8593a0819) )
	ROM_LOAD16_BYTE( "sq3-458.u3",  0xc0001, 0x20000, CRC(ef4617d8) SHA1(48231405a775585451bf970db5bb57ec2f238250) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "sr2-002", 0x0000, 0x10000, CRC(1319cf82) SHA1(7a233072890361bcf384de4f90170c2ca713b1de) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "dig2-001.u12", 0x00000, 0x20000, CRC(498dd74f) SHA1(80bb204b3e9cadcecbfa75c78c52fb9908566c5e) )
ROM_END

ROM_START( screenp1a ) // the same as screenp1 apart from the rom at u15
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sa4-378.u15", 0x00000, 0x20000, CRC(a5c0e623) SHA1(cd3215db924bbee80f5b3fbbd391c7246ad88c69) )
	ROM_LOAD16_BYTE( "sa4-378.u16", 0x00001, 0x20000, CRC(b04588b7) SHA1(1f9b933e441969c95bbbabbfbe44349bd945c326) )
	ROM_LOAD16_BYTE( "sa4-378.u17", 0x40000, 0x20000, CRC(4b6cdc43) SHA1(1d6a4796ce67d0d00fe74a6bafd8b731450cdaab) )
	ROM_LOAD16_BYTE( "sa4-378.u18", 0x40001, 0x20000, CRC(d986355f) SHA1(86d3f1712cd1bcc90a54945a2baccae2596de691) )

	ROM_LOAD16_BYTE( "sq3-458.u2",  0x80000, 0x20000, CRC(7091dfcd) SHA1(d28abd70db5c49baa93f0488e443f29c27a7a559) )
	ROM_LOAD16_BYTE( "sq3-458.u1",  0x80001, 0x20000, CRC(1bb0efbf) SHA1(59d7e2e51928df149764502bc4bd5736463f40d7) )
	ROM_LOAD16_BYTE( "sq3-458.u4",  0xc0000, 0x20000, CRC(0fb0fc84) SHA1(e7ef68130f9627a842849f41f67accf8593a0819) )
	ROM_LOAD16_BYTE( "sq3-458.u3",  0xc0001, 0x20000, CRC(ef4617d8) SHA1(48231405a775585451bf970db5bb57ec2f238250) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "sr2-002", 0x0000, 0x10000, CRC(1319cf82) SHA1(7a233072890361bcf384de4f90170c2ca713b1de) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "dig2-001.u12", 0x00000, 0x20000, CRC(498dd74f) SHA1(80bb204b3e9cadcecbfa75c78c52fb9908566c5e) )
ROM_END

ROM_START( screenp2 ) // exactly the same set has been seen with code sa5-196
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sa4-280.u15", 0x00000, 0x20000, CRC(d7206438) SHA1(70e7dc7d58bfa7dfe00250ab46fa71e951dbf901) )
	ROM_LOAD16_BYTE( "sa4-280.u16", 0x00001, 0x20000, CRC(f99e972f) SHA1(b01c8796967ff7f27269b31ef983b5fb26b03aab) )
	ROM_LOAD16_BYTE( "sa4-280.u17", 0x40000, 0x20000, CRC(cbde5343) SHA1(e341d642d8537bc221b3ca9803c221dc0cdf86c3) )
	ROM_LOAD16_BYTE( "sa4-280.u18", 0x40001, 0x20000, CRC(885b887b) SHA1(9cfb145c8cca49450fabbf4efab9c70f98ecd2af) )

	ROM_LOAD16_BYTE( "u2.bin", 0x80000, 0x20000, CRC(7091dfcd) SHA1(d28abd70db5c49baa93f0488e443f29c27a7a559) )
	ROM_LOAD16_BYTE( "u1.bin", 0x80001, 0x20000, CRC(1bb0efbf) SHA1(59d7e2e51928df149764502bc4bd5736463f40d7) )
	ROM_LOAD16_BYTE( "u4.bin", 0xc0000, 0x20000, CRC(0fb0fc84) SHA1(e7ef68130f9627a842849f41f67accf8593a0819) )
	ROM_LOAD16_BYTE( "u3.bin", 0xc0001, 0x20000, CRC(ef4617d8) SHA1(48231405a775585451bf970db5bb57ec2f238250) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "reels.bin", 0x00000, 0x10000, CRC(1319cf82) SHA1(7a233072890361bcf384de4f90170c2ca713b1de) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "dig2-001.u12", 0x00000, 0x20000, CRC(498dd74f) SHA1(80bb204b3e9cadcecbfa75c78c52fb9908566c5e) )
ROM_END

ROM_START( screenp2a ) // the same as screenp2 apart from the rom at u15
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sa5-197.u15", 0x00000, 0x20000, CRC(4c067b11) SHA1(2364421b2f92669c3c164a170017bdf255134004) )
	ROM_LOAD16_BYTE( "sa5-197.u16", 0x00001, 0x20000, CRC(f99e972f) SHA1(b01c8796967ff7f27269b31ef983b5fb26b03aab) )
	ROM_LOAD16_BYTE( "sa5-197.u17", 0x40000, 0x20000, CRC(cbde5343) SHA1(e341d642d8537bc221b3ca9803c221dc0cdf86c3) )
	ROM_LOAD16_BYTE( "sa5-197.u18", 0x40001, 0x20000, CRC(885b887b) SHA1(9cfb145c8cca49450fabbf4efab9c70f98ecd2af) )

	ROM_LOAD16_BYTE( "u2.bin", 0x80000, 0x20000, CRC(7091dfcd) SHA1(d28abd70db5c49baa93f0488e443f29c27a7a559) )
	ROM_LOAD16_BYTE( "u1.bin", 0x80001, 0x20000, CRC(1bb0efbf) SHA1(59d7e2e51928df149764502bc4bd5736463f40d7) )
	ROM_LOAD16_BYTE( "u4.bin", 0xc0000, 0x20000, CRC(0fb0fc84) SHA1(e7ef68130f9627a842849f41f67accf8593a0819) )
	ROM_LOAD16_BYTE( "u3.bin", 0xc0001, 0x20000, CRC(ef4617d8) SHA1(48231405a775585451bf970db5bb57ec2f238250) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "reels.bin", 0x00000, 0x10000, CRC(1319cf82) SHA1(7a233072890361bcf384de4f90170c2ca713b1de) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "dig2-001.u12", 0x00000, 0x20000, CRC(498dd74f) SHA1(80bb204b3e9cadcecbfa75c78c52fb9908566c5e) )
ROM_END

ROM_START( screenp3 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sa5-082.u15", 0x00000, 0x20000, CRC(ef98977d) SHA1(1cbbd4024b2076adc672b2a53fc8696615ce6eac) )
	ROM_LOAD16_BYTE( "sa5-082.u16", 0x00001, 0x20000, CRC(214823be) SHA1(db98ff915f1956e275425b48c4d829c407c81ed6) )
	ROM_LOAD16_BYTE( "sa5-082.u17", 0x40000, 0x20000, CRC(6e3b5259) SHA1(fb0e42a995768dfc08549a75021306978740ff85) )
	ROM_LOAD16_BYTE( "sa5-082.u18", 0x40001, 0x20000, CRC(fb1af777) SHA1(afc4123bc61900d41bd321aa7eb7fb0f982a9501) )

	ROM_LOAD16_BYTE( "u2.bin", 0x80000, 0x20000, CRC(7091dfcd) SHA1(d28abd70db5c49baa93f0488e443f29c27a7a559) )
	ROM_LOAD16_BYTE( "u1.bin", 0x80001, 0x20000, CRC(1bb0efbf) SHA1(59d7e2e51928df149764502bc4bd5736463f40d7) )
	ROM_LOAD16_BYTE( "u4.bin", 0xc0000, 0x20000, CRC(0fb0fc84) SHA1(e7ef68130f9627a842849f41f67accf8593a0819) )
	ROM_LOAD16_BYTE( "u3.bin", 0xc0001, 0x20000, CRC(ef4617d8) SHA1(48231405a775585451bf970db5bb57ec2f238250) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "reels.bin", 0x00000, 0x10000, CRC(1319cf82) SHA1(7a233072890361bcf384de4f90170c2ca713b1de) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "dig2-001.u12", 0x00000, 0x20000, CRC(498dd74f) SHA1(80bb204b3e9cadcecbfa75c78c52fb9908566c5e) )
ROM_END

ROM_START( screenp3a )
	ROM_REGION( 0x100000, "maincpu", 0 ) // only  u16 differs from above set
	ROM_LOAD16_BYTE( "sa5-083.u15", 0x00000, 0x20000, CRC(ef98977d) SHA1(1cbbd4024b2076adc672b2a53fc8696615ce6eac) )
	ROM_LOAD16_BYTE( "sa5-083.u16", 0x00001, 0x20000, CRC(84e17aa1) SHA1(cbc77315cb311a217d2b9092ffb3e3aee07a9633) )
	ROM_LOAD16_BYTE( "sa5-083.u17", 0x40000, 0x20000, CRC(6e3b5259) SHA1(fb0e42a995768dfc08549a75021306978740ff85) )
	ROM_LOAD16_BYTE( "sa5-083.u18", 0x40001, 0x20000, CRC(fb1af777) SHA1(afc4123bc61900d41bd321aa7eb7fb0f982a9501) )

	ROM_LOAD16_BYTE( "u2.bin", 0x80000, 0x20000, CRC(7091dfcd) SHA1(d28abd70db5c49baa93f0488e443f29c27a7a559) )
	ROM_LOAD16_BYTE( "u1.bin", 0x80001, 0x20000, CRC(1bb0efbf) SHA1(59d7e2e51928df149764502bc4bd5736463f40d7) )
	ROM_LOAD16_BYTE( "u4.bin", 0xc0000, 0x20000, CRC(0fb0fc84) SHA1(e7ef68130f9627a842849f41f67accf8593a0819) )
	ROM_LOAD16_BYTE( "u3.bin", 0xc0001, 0x20000, CRC(ef4617d8) SHA1(48231405a775585451bf970db5bb57ec2f238250) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "reels.bin", 0x00000, 0x10000, CRC(1319cf82) SHA1(7a233072890361bcf384de4f90170c2ca713b1de) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "dig2-001.u12", 0x00000, 0x20000, CRC(498dd74f) SHA1(80bb204b3e9cadcecbfa75c78c52fb9908566c5e) )
ROM_END

ROM_START( screenp4 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sply5p15", 0x00000, 0x20000, CRC(c1e956d8) SHA1(364bfa7d3d90bcbb50720e02b3330503002e6fe3) )
	ROM_LOAD16_BYTE( "sply5p16", 0x00001, 0x20000, CRC(e23e7e33) SHA1(0c73a5da1e52ae74427222acdbe81c16f0ba334c) )
	// not sure which other roms these pair with.. but the ones below work
	ROM_LOAD16_BYTE( "sa4-280.u17", 0x40000, 0x20000, CRC(cbde5343) SHA1(e341d642d8537bc221b3ca9803c221dc0cdf86c3) )
	ROM_LOAD16_BYTE( "sa4-280.u18", 0x40001, 0x20000, CRC(885b887b) SHA1(9cfb145c8cca49450fabbf4efab9c70f98ecd2af) )

	ROM_LOAD16_BYTE( "u2.bin", 0x80000, 0x20000, CRC(7091dfcd) SHA1(d28abd70db5c49baa93f0488e443f29c27a7a559) )
	ROM_LOAD16_BYTE( "u1.bin", 0x80001, 0x20000, CRC(1bb0efbf) SHA1(59d7e2e51928df149764502bc4bd5736463f40d7) )
	ROM_LOAD16_BYTE( "u4.bin", 0xc0000, 0x20000, CRC(0fb0fc84) SHA1(e7ef68130f9627a842849f41f67accf8593a0819) )
	ROM_LOAD16_BYTE( "u3.bin", 0xc0001, 0x20000, CRC(ef4617d8) SHA1(48231405a775585451bf970db5bb57ec2f238250) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "reels.bin", 0x00000, 0x10000, CRC(1319cf82) SHA1(7a233072890361bcf384de4f90170c2ca713b1de) )

	ROM_REGION( 0x20000, "upd", 0 )
	ROM_LOAD( "dig2-001.u12", 0x00000, 0x20000, CRC(498dd74f) SHA1(80bb204b3e9cadcecbfa75c78c52fb9908566c5e) )
ROM_END

// are these actually missing from all the other sets, or not used by those games?
#define MV1_MISSING_ROMS \
	ROM_REGION( 0x10000, "soundcpu", 0 ) \
	ROM_LOAD( "reels.bin", 0x00000, 0x10000, NO_DUMP ) \
	ROM_REGION( 0x20000, "upd", 0 ) \
	ROM_LOAD( "upd.bin", 0x00000, 0x20000, NO_DUMP )
#define MV1_MISSING_ROMSU1U4 \
	ROM_LOAD16_BYTE( "u2.bin", 0x80000, 0x20000, NO_DUMP ) \
	ROM_LOAD16_BYTE( "u1.bin", 0x80001, 0x20000, NO_DUMP ) \
	ROM_LOAD16_BYTE( "u4.bin", 0xc0000, 0x20000, NO_DUMP ) \
	ROM_LOAD16_BYTE( "u3.bin", 0xc0001, 0x20000, NO_DUMP )

ROM_START( mv1bon )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sw3-418.u15", 0x00000, 0x020000, CRC(0716a47b) SHA1(acbe903c270d2bb20c408d378007136803f96805) )
	ROM_LOAD16_BYTE( "sw3-418.u16", 0x00001, 0x020000, CRC(0d02369b) SHA1(4acbffb31bf6e98156e0b581e4e81459b33a845e) )
	ROM_LOAD16_BYTE( "sw3-418.u17", 0x40000, 0x020000, CRC(f1ddf287) SHA1(c1f7e92188995e9dbf47d50947bb7941b523b916) )
	ROM_LOAD16_BYTE( "sw3-418.u18", 0x40001, 0x020000, CRC(b81dbac9) SHA1(9549596169cff9b2bbbe12db551122e4d874b274) )
	MV1_MISSING_ROMSU1U4 // complains U1 is bad, so missing.

	MV1_MISSING_ROMS
ROM_END


ROM_START( mv1cpc )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sc4-066.u15", 0x00000, 0x020000, CRC(05599f6c) SHA1(3e2d5bc12c61b23ffbce40fcb1612611241f6db3) )
	ROM_LOAD16_BYTE( "sc4-066.u16", 0x00001, 0x020000, CRC(6e485115) SHA1(1288b83df9cbf8813ff07f53dcf03d3637d42f0e) )
	ROM_LOAD16_BYTE( "sc4-066.u17", 0x40000, 0x020000, CRC(ee7894e6) SHA1(070e1fed8b1b0f2876dbb5ef70b439baf9bbd2d7) )
	ROM_LOAD16_BYTE( "sc4-066.u18", 0x40001, 0x020000, CRC(a2320921) SHA1(424d26368b9452af9aa1760a98474c5b45edf6dd) )
	ROM_LOAD16_BYTE( "sq3-432.u2", 0x80000, 0x020000, CRC(4918a9e1) SHA1(6599c5f0b2ce5dc78758917195c04ae4bb078e94) )
	ROM_LOAD16_BYTE( "sq3-432.u1", 0x80001, 0x020000, CRC(2ee77952) SHA1(8f17d28220a25ad232aab029166a7535d5b5618b) )
	ROM_LOAD16_BYTE( "sq3-432.u4", 0xc0000, 0x020000, CRC(abd2df4d) SHA1(16b3df060094bef0ac41cb3cc71e910323d687f2) )
	ROM_LOAD16_BYTE( "sq3-432.u3", 0xc0001, 0x020000, CRC(7eb80747) SHA1(23158c400497f01ac7eddecf259ed988b6683eb9) )

	MV1_MISSING_ROMS
ROM_END



ROM_START( mv1cpca )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sc4-067.u15", 0x00000, 0x020000, CRC(05599f6c) SHA1(3e2d5bc12c61b23ffbce40fcb1612611241f6db3) )
	ROM_LOAD16_BYTE( "sc4-067.u16", 0x00001, 0x020000, CRC(4068eac8) SHA1(8c9a762c827118b96ec6d257ab828436224a2123) )
	ROM_LOAD16_BYTE( "sc4-067.u17", 0x40000, 0x020000, CRC(ee7894e6) SHA1(070e1fed8b1b0f2876dbb5ef70b439baf9bbd2d7) )
	ROM_LOAD16_BYTE( "sc4-067.u18", 0x40001, 0x020000, CRC(a2320921) SHA1(424d26368b9452af9aa1760a98474c5b45edf6dd) )
	ROM_LOAD16_BYTE( "sq3-432.u2", 0x80000, 0x020000, CRC(4918a9e1) SHA1(6599c5f0b2ce5dc78758917195c04ae4bb078e94) )
	ROM_LOAD16_BYTE( "sq3-432.u1", 0x80001, 0x020000, CRC(2ee77952) SHA1(8f17d28220a25ad232aab029166a7535d5b5618b) )
	ROM_LOAD16_BYTE( "sq3-432.u4", 0xc0000, 0x020000, CRC(abd2df4d) SHA1(16b3df060094bef0ac41cb3cc71e910323d687f2) )
	ROM_LOAD16_BYTE( "sq3-432.u3", 0xc0001, 0x020000, CRC(7eb80747) SHA1(23158c400497f01ac7eddecf259ed988b6683eb9) )

	MV1_MISSING_ROMS
ROM_END



ROM_START( mv1cpcb )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sc5-007.u15", 0x00000, 0x020000, CRC(d7e969ff) SHA1(2cf7bd0b7ae55f5570b699ff1a3a325658409c5c) )
	ROM_LOAD16_BYTE( "sc5-007.u16", 0x00001, 0x020000, CRC(bbc579c7) SHA1(ef1edd28f7015819f470cdf6736377268eee617c) )
	ROM_LOAD16_BYTE( "sc5-007.u17", 0x40000, 0x020000, CRC(f69bbac9) SHA1(27c6d06673a349bd2d0cd44692e0a0482b37c29f) )
	ROM_LOAD16_BYTE( "sc5-007.u18", 0x40001, 0x020000, CRC(6f3979b2) SHA1(09e74d05fb564293e0314e5dcdfb8edde4f1f9ec) )
	ROM_LOAD16_BYTE( "sq3-432.u2", 0x80000, 0x020000, CRC(4918a9e1) SHA1(6599c5f0b2ce5dc78758917195c04ae4bb078e94) )
	ROM_LOAD16_BYTE( "sq3-432.u1", 0x80001, 0x020000, CRC(2ee77952) SHA1(8f17d28220a25ad232aab029166a7535d5b5618b) )
	ROM_LOAD16_BYTE( "sq3-432.u4", 0xc0000, 0x020000, CRC(abd2df4d) SHA1(16b3df060094bef0ac41cb3cc71e910323d687f2) )
	ROM_LOAD16_BYTE( "sq3-432.u3", 0xc0001, 0x020000, CRC(7eb80747) SHA1(23158c400497f01ac7eddecf259ed988b6683eb9) )

	MV1_MISSING_ROMS
ROM_END



ROM_START( mv1cwq )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sw2-319.u15", 0x00000, 0x020000, CRC(0a591eb7) SHA1(dad833711a5779300757e3e0fbe8f55073470724) )
	ROM_LOAD16_BYTE( "sw2-319.u16", 0x00001, 0x020000, CRC(8460cb55) SHA1(793c7e31619e5a30ad8bf851253c8cfcbddefbd7) )
	ROM_LOAD16_BYTE( "sw2-319.u17", 0x40000, 0x020000, CRC(f065c219) SHA1(b83a835a107740e2e846d31b4988237f1e60db78) )
	ROM_LOAD16_BYTE( "sw2-319.u18", 0x40001, 0x020000, CRC(0c291cb3) SHA1(aa7740a7a34e653e9b39a0ce64b344ace9d74e19) )
	MV1_MISSING_ROMSU1U4 // complains U1 is bad, so missing.

	MV1_MISSING_ROMS
ROM_END


ROM_START( mv1cwqa )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sw2-320.u15", 0x00000, 0x020000, CRC(02679239) SHA1(9c3f2ca7048a6ef716dbdd20f9a921e1d505638f) )
	ROM_LOAD16_BYTE( "sw2-320.u16", 0x00001, 0x020000, CRC(8460cb55) SHA1(793c7e31619e5a30ad8bf851253c8cfcbddefbd7) )
	ROM_LOAD16_BYTE( "sw2-320.u17", 0x40000, 0x020000, CRC(f065c219) SHA1(b83a835a107740e2e846d31b4988237f1e60db78) )
	ROM_LOAD16_BYTE( "sw2-320.u18", 0x40001, 0x020000, CRC(0c291cb3) SHA1(aa7740a7a34e653e9b39a0ce64b344ace9d74e19) )
	MV1_MISSING_ROMSU1U4 // complains U1 is bad, so missing.

	MV1_MISSING_ROMS
ROM_END


ROM_START( mv1guac )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sw3-146.u15", 0x00000, 0x020000, CRC(faa7ffa9) SHA1(db7e555727d126c266b6e973b61d1df205256c38) )
	ROM_LOAD16_BYTE( "sw3-146.u16", 0x00001, 0x020000, CRC(1f7209d9) SHA1(b512e28cef973c9b6e0a8b630500a5de08206f32) )
	ROM_LOAD16_BYTE( "sw3-146.u17", 0x40000, 0x020000, CRC(0ecdd43b) SHA1(f666281d4c2a67675d75fc07f7dfd6b53558468a) )
	ROM_LOAD16_BYTE( "sw3-146.u18", 0x40001, 0x020000, CRC(a107f3a9) SHA1(35de6636f2ca07e5db0fc7527b6d94940242e2a3) )
	MV1_MISSING_ROMSU1U4 // complains U1 is bad, so missing.

	MV1_MISSING_ROMS

ROM_END


ROM_START( mv1guaca )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sw3-147.u15", 0x00000, 0x020000, CRC(c2d420ad) SHA1(3fe046fbb5eb51e85926707662e6009732f2d588) )
	ROM_LOAD16_BYTE( "sw3-147.u16", 0x00001, 0x020000, CRC(1f7209d9) SHA1(b512e28cef973c9b6e0a8b630500a5de08206f32) )
	ROM_LOAD16_BYTE( "sw3-147.u17", 0x40000, 0x020000, CRC(0ecdd43b) SHA1(f666281d4c2a67675d75fc07f7dfd6b53558468a) )
	ROM_LOAD16_BYTE( "sw3-147.u18", 0x40001, 0x020000, CRC(a107f3a9) SHA1(35de6636f2ca07e5db0fc7527b6d94940242e2a3) )
	MV1_MISSING_ROMSU1U4 // complains U1 is bad, so missing.

	MV1_MISSING_ROMS
ROM_END

ROM_START( mv1sfx )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sa4-005.u15", 0x00000, 0x020000, CRC(07d2cd6a) SHA1(97bf2384241cbd0df4a3a0878c6022bee392611b) )
	ROM_LOAD16_BYTE( "sa4-005.u16", 0x00001, 0x020000, CRC(11c8f456) SHA1(81c6aa0b60c256236416a65c1199afa30b05ff1b) )
	ROM_LOAD16_BYTE( "sa4-005.u17", 0x40000, 0x020000, CRC(f34a0d24) SHA1(377c067068f7a6a73d3c5c6cdb62409116246e71) )
	ROM_LOAD16_BYTE( "sa4-005.u18", 0x40001, 0x020000, CRC(e13c544a) SHA1(206151ad83af5cf939f917bcec6600ca6ffb4544) )

	MV1_MISSING_ROMS
ROM_END


ROM_START( mv1sfxa )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sa4-006.u15", 0x00000, 0x020000, CRC(3fa1126e) SHA1(1f94aa796b065f4b63aac31df5f8368623a587ef) )
	ROM_LOAD16_BYTE( "sa4-006.u16", 0x00001, 0x020000, CRC(11c8f456) SHA1(81c6aa0b60c256236416a65c1199afa30b05ff1b) )
	ROM_LOAD16_BYTE( "sa4-006.u17", 0x40000, 0x020000, CRC(f34a0d24) SHA1(377c067068f7a6a73d3c5c6cdb62409116246e71) )
	ROM_LOAD16_BYTE( "sa4-006.u18", 0x40001, 0x020000, CRC(e13c544a) SHA1(206151ad83af5cf939f917bcec6600ca6ffb4544) )

	MV1_MISSING_ROMS
ROM_END

ROM_START( mv1sfx2 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fx28.u15", 0x00000, 0x020000, CRC(70e52fa5) SHA1(353bcc610c73ca3d4c14bd9802cf1f394ed702a8) )
	ROM_LOAD16_BYTE( "fx28.u16", 0x00001, 0x020000, CRC(9095fdce) SHA1(2c9db9cc613c6660dad5054f2bef04fec8d6bb17) )
	ROM_LOAD16_BYTE( "fx28.u17", 0x40000, 0x020000, CRC(0015cd0a) SHA1(788dbdbda8e28427f994527a2564ee2fffa38533) )
	ROM_LOAD16_BYTE( "fx28.u18", 0x40001, 0x020000, CRC(d9234071) SHA1(af45a2acc3d10df46a4e096777813fc70a099aee) )
	ROM_LOAD16_BYTE( "sq2-407.u2", 0x80000, 0x020000, CRC(d9f072e0) SHA1(175bee58255dc4b0f840d1bf0a246539fe8f9ba0) )
	ROM_LOAD16_BYTE( "sq2-407.u1", 0x80001, 0x020000, CRC(6a96a535) SHA1(fb1e7986f078f52a1db2707b150727a21c7877fc) )
	ROM_LOAD16_BYTE( "sq2-407.u4", 0xc0000, 0x020000, CRC(4670e71b) SHA1(742d0f0881e0b2e0e66c454c7a2c31da9f65cf08) )
	ROM_LOAD16_BYTE( "sq2-407.u3", 0xc0001, 0x020000, CRC(3e719d35) SHA1(4b6fe1e6a037102588162923fce87e0a67e5a109) )

	MV1_MISSING_ROMS //Looks like dump was complete otherwise
ROM_END

ROM_START( mv1wc )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sa3-196.u15", 0x00000, 0x020000, CRC(ee78e8e6) SHA1(16cb3fb0ff23f054644d706583036dae44dac792) )
	ROM_LOAD16_BYTE( "sa3-196.u16", 0x00001, 0x020000, CRC(96beaa01) SHA1(b61487fbe470c776076cbdf7cd68063b56fde81d) )
	ROM_LOAD16_BYTE( "sa3-196.u17", 0x40000, 0x020000, CRC(d583ad06) SHA1(94d86d5481367624576bf489cd5347f1b4979646) )
	ROM_LOAD16_BYTE( "sa3-196.u18", 0x40001, 0x020000, CRC(84739f41) SHA1(a26d4df72fe723fceaf8471cdf89e4bd77585f0f) )
	MV1_MISSING_ROMSU1U4 // probably missing (no error, but hangs)

	MV1_MISSING_ROMS
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

DRIVER_INIT_MEMBER(maygayv1_state,screenpl)
{
	m_p1 = m_p3 = 0xff;
}

#define GAME_FLAGS GAME_NOT_WORKING | GAME_IMPERFECT_SOUND | GAME_REQUIRES_ARTWORK

GAME( 1991, screenpl, 0,        maygayv1, screenpl, maygayv1_state, screenpl, ROT0, "Maygay", "Screen Play (Maygay, MV1 Video, ver. 4.0)",               GAME_FLAGS )
GAME( 1991, screenp1, screenpl, maygayv1, screenpl, maygayv1_state, screenpl, ROT0, "Maygay", "Screen Play (Maygay, MV1 Video, ver. 1.9, set 1)",               GAME_FLAGS )
GAME( 1991, screenp1a,screenpl, maygayv1, screenpl, maygayv1_state, screenpl, ROT0, "Maygay", "Screen Play (Maygay, MV1 Video, ver. 1.9, set 2)",               GAME_FLAGS )
GAME( 1991, screenp2, screenpl, maygayv1, screenpl, maygayv1_state, screenpl, ROT0, "Maygay", "Screen Play (Maygay, MV1 Video, ver. 1.9, Isle of Man, set 1)",  GAME_FLAGS )
GAME( 1991, screenp2a,screenpl, maygayv1, screenpl, maygayv1_state, screenpl, ROT0, "Maygay", "Screen Play (Maygay, MV1 Video, ver. 1.9, Isle of Man, set 2)",  GAME_FLAGS )
GAME( 1991, screenp3, screenpl, maygayv1, screenpl, maygayv1_state, screenpl, ROT0, "Maygay", "Screen Play (Maygay, MV1 Video, SA5-082)",  GAME_FLAGS )
GAME( 1991, screenp3a,screenpl, maygayv1, screenpl, maygayv1_state, screenpl, ROT0, "Maygay", "Screen Play (Maygay, MV1 Video, SA5-083)",  GAME_FLAGS )
GAME( 1991, screenp4 ,screenpl, maygayv1, screenpl, maygayv1_state, screenpl, ROT0, "Maygay", "Screen Play (Maygay, MV1 Video, ver. ?.?)",  GAME_FLAGS )

// incomplete sets
GAME( 199?, mv1bon  , 0         ,maygayv1, screenpl, maygayv1_state, screenpl, ROT0, "Maygay", "Believe It Or Not (Maygay, MV1 Video)",  GAME_FLAGS )
GAME( 199?, mv1cpc  , 0         ,maygayv1, screenpl, maygayv1_state, screenpl, ROT0, "Maygay", "Caesar's Palace Club (Maygay, MV1 Video, set 1)",  GAME_FLAGS )
GAME( 199?, mv1cpca , mv1cpc    ,maygayv1, screenpl, maygayv1_state, screenpl, ROT0, "Maygay", "Caesar's Palace Club (Maygay, MV1 Video, set 2)",  GAME_FLAGS )
GAME( 199?, mv1cpcb , mv1cpc    ,maygayv1, screenpl, maygayv1_state, screenpl, ROT0, "Maygay", "Caesar's Palace Club (Maygay, MV1 Video, set 3)",  GAME_FLAGS )
GAME( 199?, mv1cwq  , 0         ,maygayv1, screenpl, maygayv1_state, screenpl, ROT0, "Maygay", "Crossword Quiz (Maygay, MV1 Video, set 1)",  GAME_FLAGS )
GAME( 199?, mv1cwqa , mv1cwq    ,maygayv1, screenpl, maygayv1_state, screenpl, ROT0, "Maygay", "Crossword Quiz (Maygay, MV1 Video, set 2)",  GAME_FLAGS )
GAME( 199?, mv1guac , 0         ,maygayv1, screenpl, maygayv1_state, screenpl, ROT0, "Maygay", "Give Us A Clue (Maygay, MV1 Video, set 1)",  GAME_FLAGS )
GAME( 199?, mv1guaca, mv1guac   ,maygayv1, screenpl, maygayv1_state, screenpl, ROT0, "Maygay", "Give Us A Clue (Maygay, MV1 Video, set 2)",  GAME_FLAGS )
GAME( 199?, mv1sfx  , 0         ,maygayv1, screenpl, maygayv1_state, screenpl, ROT90, "Maygay", "Special Effects (Maygay, MV1 Video, set 1)",  GAME_FLAGS )
GAME( 199?, mv1sfxa , mv1sfx    ,maygayv1, screenpl, maygayv1_state, screenpl, ROT90, "Maygay", "Special Effects (Maygay, MV1 Video, set 2)",  GAME_FLAGS )
GAME( 199?, mv1sfx2 , 0         ,maygayv1, screenpl, maygayv1_state, screenpl, ROT90, "Maygay", "Special Effects V2 (Maygay, MV1 Video)",  GAME_FLAGS )
GAME( 199?, mv1wc   , 0         ,maygayv1, screenpl, maygayv1_state, screenpl, ROT0, "Maygay", "World Cup (Maygay, MV1 Video)",  GAME_FLAGS )
