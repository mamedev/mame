// license:GPL-2.0+
// copyright-holders:Juergen Buchmueller
/***************************************************************************

    Atari 400/800

    GTIA  graphics television interface adapter

    Juergen Buchmueller, June 1998

***************************************************************************/

#include "emu.h"
#include "video/gtia.h"

#define GTIA_P0 0x01
#define GTIA_P1 0x02
#define GTIA_P2 0x04
#define GTIA_P3 0x08
#define GTIA_M0 0x10
#define GTIA_M1 0x20
#define GTIA_M2 0x40
#define GTIA_M3 0x80

#define GTIA_HWIDTH    48      /* total characters per line */
#define GTIA_TRIGGER    0x04

#define CHECK_GRACTL    0
#define VERBOSE         0

#define PFD     0x00    /* 00000000b playfield default color */

#define PBK     0x00    /* 00000000b playfield background */
#define PF0     0x01    /* 00000001b playfield color #0   */
#define PF1     0x02    /* 00000010b playfield color #1   */
#define PF2     0x04    /* 00000100b playfield color #2   */
#define PF3     0x08    /* 00001000b playfield color #3   */
#define PL0     0x11    /* 00010001b player #0            */
#define PL1     0x12    /* 00010010b player #1            */
#define PL2     0x14    /* 00010100b player #2            */
#define PL3     0x18    /* 00011000b player #3            */
#define MI0     0x21    /* 00100001b missile #0           */
#define MI1     0x22    /* 00100010b missile #1           */
#define MI2     0x24    /* 00100100b missile #2           */
#define MI3     0x28    /* 00101000b missile #3           */
#define T00     0x40    /* 01000000b text mode pixels 00  */
#define P000    0x48    /* 01001000b player #0 pixels 00  */
#define P100    0x4a    /* 01001010b player #1 pixels 00  */
#define P200    0x4c    /* 01001100b player #2 pixels 00  */
#define P300    0x4e    /* 01001110b player #3 pixels 00  */
#define P400    0x4f    /* 01001111b missiles  pixels 00  */
#define T01     0x50    /* 01010000b text mode pixels 01  */
#define P001    0x58    /* 01011000b player #0 pixels 01  */
#define P101    0x5a    /* 01011010b player #1 pixels 01  */
#define P201    0x5c    /* 01011100b player #2 pixels 01  */
#define P301    0x5e    /* 01011110b player #3 pixels 01  */
#define P401    0x5f    /* 01011111b missiles  pixels 01  */
#define T10     0x60    /* 01100000b text mode pixels 10  */
#define P010    0x68    /* 01101000b player #0 pixels 10  */
#define P110    0x6a    /* 01101010b player #1 pixels 10  */
#define P210    0x6c    /* 01101100b player #2 pixels 10  */
#define P310    0x6e    /* 01101110b player #3 pixels 10  */
#define P410    0x6f    /* 01101111b missiles  pixels 10  */
#define T11     0x70    /* 01110000b text mode pixels 11  */
#define P011    0x78    /* 01111000b player #0 pixels 11  */
#define P111    0x7a    /* 01111010b player #1 pixels 11  */
#define P211    0x7c    /* 01111100b player #2 pixels 11  */
#define P311    0x7e    /* 01111110b player #3 pixels 11  */
#define P411    0x7f    /* 01111111b missiles  pixels 11  */
#define G00     0x80    /* 10000000b hires gfx pixels 00  */
#define G01     0x90    /* 10010000b hires gfx pixels 01  */
#define G10     0xa0    /* 10100000b hires gfx pixels 10  */
#define G11     0xb0    /* 10110000b hires gfx pixels 11  */
#define GT1     0xc0    /* 11000000b gtia mode 1          */
#define GT2     0xd0    /* 11010000b gtia mode 2          */
#define GT3     0xe0    /* 11100000b gtia mode 3          */
#define ILL     0xfe    /* 11111110b illegal priority     */
#define EOR     0xff    /* 11111111b EOR mode color       */

#define LUM     0x0f    /* 00001111b luminance bits       */
#define HUE     0xf0    /* 11110000b hue bits             */


/**********************************************
 * split a color into hue and luminance values
 **********************************************/
#define SPLIT_HUE(data, hue) \
	hue = (data & HUE)

#define SPLIT_LUM(data, lum) \
	lum = (data & LUM)

/**********************************************
 * set both color clocks equal for one color
 **********************************************/
#define SETCOL_B(o, d) \
	m_color_lookup[o] = ((d) << 8) | (d)

/**********************************************
 * set left color clock for one color
 **********************************************/
#define SETCOL_L(o, d) \
	*((UINT8*)&m_color_lookup[o] + 0) = d

/**********************************************
 * set right color clock for one color
 **********************************************/
#define SETCOL_R(o, d) \
	*((UINT8*)&m_color_lookup[o] + 1) = d



// devices
const device_type ATARI_GTIA = &device_creator<gtia_device>;

//-------------------------------------------------
//  upd7220_device - constructor
//-------------------------------------------------

gtia_device::gtia_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
				device_t(mconfig, ATARI_GTIA, "Atari GTIA", tag, owner, clock, "gtia", __FILE__),
				m_read_cb(*this),
				m_write_cb(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gtia_device::device_start()
{
	m_read_cb.resolve();
	m_write_cb.resolve();

	save_item(NAME(m_r.m0pf));
	save_item(NAME(m_r.m1pf));
	save_item(NAME(m_r.m2pf));
	save_item(NAME(m_r.m3pf));
	save_item(NAME(m_r.p0pf));
	save_item(NAME(m_r.p1pf));
	save_item(NAME(m_r.p2pf));
	save_item(NAME(m_r.p3pf));
	save_item(NAME(m_r.m0pl));
	save_item(NAME(m_r.m1pl));
	save_item(NAME(m_r.m2pl));
	save_item(NAME(m_r.m3pl));
	save_item(NAME(m_r.p0pl));
	save_item(NAME(m_r.p1pl));
	save_item(NAME(m_r.p2pl));
	save_item(NAME(m_r.p3pl));
	save_item(NAME(m_r.but));
	save_item(NAME(m_r.pal));
	save_item(NAME(m_r.gtia15));
	save_item(NAME(m_r.gtia16));
	save_item(NAME(m_r.gtia17));
	save_item(NAME(m_r.gtia18));
	save_item(NAME(m_r.gtia19));
	save_item(NAME(m_r.gtia1a));
	save_item(NAME(m_r.gtia1b));
	save_item(NAME(m_r.gtia1c));
	save_item(NAME(m_r.gtia1d));
	save_item(NAME(m_r.gtia1e));
	save_item(NAME(m_r.cons));

	save_item(NAME(m_w.hposp0));
	save_item(NAME(m_w.hposp1));
	save_item(NAME(m_w.hposp2));
	save_item(NAME(m_w.hposp3));
	save_item(NAME(m_w.hposm0));
	save_item(NAME(m_w.hposm1));
	save_item(NAME(m_w.hposm2));
	save_item(NAME(m_w.hposm3));
	save_item(NAME(m_w.sizep0));
	save_item(NAME(m_w.sizep1));
	save_item(NAME(m_w.sizep2));
	save_item(NAME(m_w.sizep3));
	save_item(NAME(m_w.sizem));
	save_item(NAME(m_w.grafp0));
	save_item(NAME(m_w.grafp1));
	save_item(NAME(m_w.grafp2));
	save_item(NAME(m_w.grafp3));
	save_item(NAME(m_w.grafm));
	save_item(NAME(m_w.colpm0));
	save_item(NAME(m_w.colpm1));
	save_item(NAME(m_w.colpm2));
	save_item(NAME(m_w.colpm3));
	save_item(NAME(m_w.colpf0));
	save_item(NAME(m_w.colpf1));
	save_item(NAME(m_w.colpf2));
	save_item(NAME(m_w.colpf3));
	save_item(NAME(m_w.colbk));
	save_item(NAME(m_w.prior));
	save_item(NAME(m_w.vdelay));
	save_item(NAME(m_w.gractl));
	save_item(NAME(m_w.hitclr));
	save_item(NAME(m_w.cons));

	save_item(NAME(m_h.grafp0));
	save_item(NAME(m_h.grafp1));
	save_item(NAME(m_h.grafp2));
	save_item(NAME(m_h.grafp3));
	save_item(NAME(m_h.grafm0));
	save_item(NAME(m_h.grafm1));
	save_item(NAME(m_h.grafm2));
	save_item(NAME(m_h.grafm3));
	save_item(NAME(m_h.hitclr_frames));
	save_item(NAME(m_h.sizem));
	save_item(NAME(m_h.usedp));
	save_item(NAME(m_h.usedm0));
	save_item(NAME(m_h.usedm1));
	save_item(NAME(m_h.usedm2));
	save_item(NAME(m_h.usedm3));
	save_item(NAME(m_h.vdelay_m0));
	save_item(NAME(m_h.vdelay_m1));
	save_item(NAME(m_h.vdelay_m2));
	save_item(NAME(m_h.vdelay_m3));
	save_item(NAME(m_h.vdelay_p0));
	save_item(NAME(m_h.vdelay_p1));
	save_item(NAME(m_h.vdelay_p2));
	save_item(NAME(m_h.vdelay_p3));

	save_item(NAME(m_lumpf1));
	save_item(NAME(m_huepm0));
	save_item(NAME(m_huepm1));
	save_item(NAME(m_huepm2));
	save_item(NAME(m_huepm3));
	save_item(NAME(m_huepm4));
	save_item(NAME(m_huepf2));
	save_item(NAME(m_huebk));

	save_item(NAME(m_color_lookup));

	machine().save().register_postload(save_prepost_delegate(FUNC(gtia_device::gtia_postload), this));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void gtia_device::device_reset()
{
	memset(&m_r, 0, sizeof(m_r));
	memset(&m_h, 0, sizeof(m_h));
	memset(m_color_lookup, 0, sizeof(m_color_lookup));

	m_lumpf1 = 0;

	/* reset the GTIA read/write/helper registers */
	for (int i = 0; i < 32; i++)
		write(machine().driver_data()->generic_space(), i, 0);

	if (is_ntsc())
		m_r.pal = 0xff;
	else
		m_r.pal = 0xf1;
	m_r.gtia15 = 0xff;
	m_r.gtia16 = 0xff;
	m_r.gtia17 = 0xff;
	m_r.gtia18 = 0xff;
	m_r.gtia19 = 0xff;
	m_r.gtia1a = 0xff;
	m_r.gtia1b = 0xff;
	m_r.gtia1c = 0xff;
	m_r.gtia1d = 0xff;
	m_r.gtia1e = 0xff;
	m_r.cons = 0x07;     /* console keys */
	SETCOL_B(ILL, 0x3e);     /* bright red */
	SETCOL_B(EOR, 0xff);     /* yellow */

	m_huepm0 = 0;
	m_huepm1 = 0;
	m_huepm2 = 0;
	m_huepm3 = 0;
	m_huepm4 = 0;
	m_huepf2 = 0;
	m_huebk = 0;
}



int gtia_device::is_ntsc()
{
	return ATTOSECONDS_TO_HZ(machine().first_screen()->frame_period().attoseconds()) > 55;
}

void gtia_device::button_interrupt(int button_count, UINT8 button_port)
{
	/* specify buttons relevant to this Atari variant */
	for (int i = 0; i < button_count; i++)
	{
		if ((m_w.gractl & GTIA_TRIGGER) == 0)
			m_r.but[i] = 1;
		m_r.but[i] &= BIT(button_port, i);
	}

	/* button registers for xl/xe */
	if (button_count == 2)
	{
		m_r.but[2] = 1;  /* not used on xl/xe */
		m_r.but[3] = 0;  /* 1 if external cartridge is inserted */
	}

}



/**************************************************************
 *
 * Read GTIA hardware registers
 *
 **************************************************************/

READ8_MEMBER( gtia_device::read )
{
	switch (offset & 31)
	{
		case  0: return m_r.m0pf;
		case  1: return m_r.m1pf;
		case  2: return m_r.m2pf;
		case  3: return m_r.m3pf;
		case  4: return m_r.p0pf;
		case  5: return m_r.p1pf;
		case  6: return m_r.p2pf;
		case  7: return m_r.p3pf;
		case  8: return m_r.m0pl;
		case  9: return m_r.m1pl;
		case 10: return m_r.m2pl;
		case 11: return m_r.m3pl;
		case 12: return m_r.p0pl;
		case 13: return m_r.p1pl;
		case 14: return m_r.p2pl;
		case 15: return m_r.p3pl;

		case 16: return m_r.but[0];
		case 17: return m_r.but[1];
		case 18: return m_r.but[2];
		case 19: return m_r.but[3];

		case 20: return m_r.pal;

		case 21: return m_r.gtia15;
		case 22: return m_r.gtia16;
		case 23: return m_r.gtia17;
		case 24: return m_r.gtia18;
		case 25: return m_r.gtia19;
		case 26: return m_r.gtia1a;
		case 27: return m_r.gtia1b;
		case 28: return m_r.gtia1c;
		case 29: return m_r.gtia1d;
		case 30: return m_r.gtia1e;

		case 31:
			m_r.cons = !m_read_cb.isnull() ? (m_read_cb(0) & 0x0f) : 0x00;
			return m_r.cons;
	}
	return 0xff;
}



void gtia_device::recalc_p0()
{
	if (
#if CHECK_GRACTL
		(m_w.gractl & GTIA_PLAYER) == 0 ||
#endif
		m_w.grafp0[m_h.vdelay_p0] == 0 || m_w.hposp0 >= 224 )
	{
		m_h.grafp0 = 0;
		m_h.usedp &= ~0x10;
	}
	else
	{
		m_h.grafp0 = m_w.grafp0[m_h.vdelay_p0];
		m_h.usedp |= 0x10;
	}
}

void gtia_device::recalc_p1()
{
	if (
#if CHECK_GRACTL
		(m_w.gractl & GTIA_PLAYER) == 0 ||
#endif
		m_w.grafp1[m_h.vdelay_p1] == 0 || m_w.hposp1 >= 224 )
	{
		m_h.grafp1 = 0;
		m_h.usedp &= ~0x20;
	}
	else
	{
		m_h.grafp1 = m_w.grafp1[m_h.vdelay_p1];
		m_h.usedp |= 0x20;
	}
}

void gtia_device::recalc_p2()
{
	if (
#if CHECK_GRACTL
		(m_w.gractl & GTIA_PLAYER) == 0 ||
#endif
		m_w.grafp2[m_h.vdelay_p2] == 0 || m_w.hposp2 >= 224 )
	{
		m_h.grafp2 = 0;
		m_h.usedp &= ~0x40;
	}
	else
	{
		m_h.grafp2 = m_w.grafp2[m_h.vdelay_p2];
		m_h.usedp |= 0x40;
	}
}

void gtia_device::recalc_p3()
{
	if (
#if CHECK_GRACTL
		(m_w.gractl & GTIA_PLAYER) == 0 ||
#endif
		m_w.grafp3[m_h.vdelay_p3] == 0 || m_w.hposp3 >= 224 )
	{
		m_h.grafp3 = 0;
		m_h.usedp &= ~0x80;
	}
	else
	{
		m_h.grafp3 = m_w.grafp3[m_h.vdelay_p3];
		m_h.usedp |= 0x80;
	}
}

void gtia_device::recalc_m0()
{
	if (
#if CHECK_GRACTL
		(m_w.gractl & GTIA_MISSILE) == 0 ||
#endif
		(m_w.grafm[m_h.vdelay_m0] & 0x03) == 0 || m_w.hposm0 >= 224 )
	{
		m_h.grafm0 = 0;
		m_h.usedm0 = 0;
	}
	else
	{
		m_h.grafm0 = (m_w.grafm[m_h.vdelay_m0] << 6) & 0xc0;
		m_h.usedm0 = (m_w.prior & 0x10) ? 0x08 : 0x10;
	}
}

void gtia_device::recalc_m1()
{
	if (
#if CHECK_GRACTL
		(m_w.gractl & GTIA_MISSILE) == 0 ||
#endif
		(m_w.grafm[m_h.vdelay_m1] & 0x0c) == 0 || m_w.hposm1 >= 224 )
	{
		m_h.grafm1 = 0;
		m_h.usedm1 = 0;
	}
	else
	{
		m_h.grafm1 = (m_w.grafm[m_h.vdelay_m1] << 4) & 0xc0;
		m_h.usedm1 = (m_w.prior & 0x10) ? 0x08 : 0x20;
	}
}

void gtia_device::recalc_m2()
{
	if (
#if CHECK_GRACTL
		(m_w.gractl & GTIA_MISSILE) == 0 ||
#endif
		(m_w.grafm[m_h.vdelay_m2] & 0x30) == 0 || m_w.hposm2 >= 224 )
	{
		m_h.grafm2 = 0;
		m_h.usedm2 = 0;
	}
	else
	{
		m_h.grafm2 = (m_w.grafm[m_h.vdelay_m2] << 2) & 0xc0;
		m_h.usedm2 = (m_w.prior & 0x10) ? 0x08 : 0x40;
	}
}

void gtia_device::recalc_m3()
{
	if (
#if CHECK_GRACTL
		(m_w.gractl & GTIA_MISSILE) == 0 ||
#endif
		(m_w.grafm[m_h.vdelay_m3] & 0xc0) == 0 || m_w.hposm3 >= 224)
	{
		m_h.grafm3 = 0;
		m_h.usedm3 = 0;
	}
	else
	{
		m_h.grafm3 = (m_w.grafm[m_h.vdelay_m3] << 0) & 0xc0;
		m_h.usedm3 = (m_w.prior & 0x10) ? 0x08 : 0x80;
	}
}



WRITE8_MEMBER( gtia_device::write )
{
	/* used for mixing hue/lum of different colors */
//  static UINT8 lumpm0=0,lumpm1=0,lumpm2=0,lumpm3=0,lumpm4=0;
//  static UINT8 lumpf2=0;
//  static UINT8 lumbk= 0;
//  static UINT8 huepf1=0;

	switch (offset & 31)
	{
	case  0:
		m_w.hposp0 = data;
		recalc_p0();
		break;
	case  1:
		m_w.hposp1 = data;
		recalc_p1();
		break;
	case  2:
		m_w.hposp2 = data;
		recalc_p2();
		break;
	case  3:
		m_w.hposp3 = data;
		recalc_p3();
		break;

	case  4:
		m_w.hposm0 = data;
		recalc_m0();
		break;
	case  5:
		m_w.hposm1 = data;
		recalc_m1();
		break;
	case  6:
		m_w.hposm2 = data;
		recalc_m2();
		break;
	case  7:
		m_w.hposm3 = data;
		recalc_m3();
		break;

	case  8:
		data &= 3;
		m_w.sizep0 = data;
		recalc_p0();
		break;
	case  9:
		data &= 3;
		m_w.sizep1 = data;
		recalc_p1();
		break;
	case 10:
		data &= 3;
		m_w.sizep2 = data;
		recalc_p2();
		break;
	case 11:
		data &= 3;
		m_w.sizep3 = data;
		recalc_p3();
		break;

	case 12:
		data &= 3;
		m_w.sizem = data;
		recalc_m0();
		recalc_m1();
		recalc_m2();
		recalc_m3();
		break;

	case 13:
		m_w.grafp0[0] = data;
		recalc_p0();
		break;
	case 14:
		m_w.grafp1[0] = data;
		recalc_p1();
		break;
	case 15:
		m_w.grafp2[0] = data;
		recalc_p2();
		break;
	case 16:
		m_w.grafp3[0] = data;
		recalc_p3();
		break;

	case 17:
		m_w.grafm[0] = data;
		recalc_m0();
		recalc_m1();
		recalc_m2();
		recalc_m3();
		break;

	case 18:    /* color for player/missile #0 */
		if (data == m_w.colpm0)
			break;
		m_w.colpm0 = data;
		if (VERBOSE)
			logerror("atari colpm0 $%02x\n", data);

		SETCOL_B(PL0, data);     /* set player 0 color */
		SETCOL_B(MI0, data);     /* set missile 0 color */
		SETCOL_B(GT2, data);     /* set GTIA mode 2 color 0 */
		SETCOL_B(P000, data);    /* set player 0 both pixels 0 */
		SETCOL_L(P001, data);    /* set player 0 left pixel 0 */
		SETCOL_R(P010, data);    /* set player 0 right pixel 0 */
		SPLIT_HUE(data, m_huepm0);
		data = m_huepm0 | m_lumpf1;
		SETCOL_R(P001, data);    /* set player 0 right pixel 1 */
		SETCOL_L(P010, data);    /* set player 0 left pixel 1 */
		SETCOL_B(P011, data);    /* set player 0 both pixels 1 */
		break;

	case 19:    /* color for player/missile #1 */
		if (data == m_w.colpm1)
			break;
		m_w.colpm1 = data;
		if (VERBOSE)
			logerror("atari colpm1 $%02x\n", data);

		SETCOL_B(PL1, data);     /* set player color 1 */
		SETCOL_B(MI1, data);     /* set missile color 1 */
		SETCOL_B(GT2+1, data);   /* set GTIA mode 2 color 1 */
		SETCOL_B(P100, data);    /* set player 1 both pixels 0 */
		SETCOL_L(P101, data);    /* set player 1 left pixel 0 */
		SETCOL_R(P110, data);    /* set player 1 right pixel 0 */
		SPLIT_HUE(data, m_huepm1);
		data = m_huepm1 | m_lumpf1;
		SETCOL_R(P101, data);    /* set player 1 right pixel 1 */
		SETCOL_L(P110, data);    /* set player 1 left pixel 1 */
		SETCOL_B(P111, data);    /* set player 1 both pixels 1 */
		break;

	case 20:    /* color for player/missile #2 */
		if (data == m_w.colpm2)
			break;
		m_w.colpm2 = data;
		if (VERBOSE)
			logerror("atari colpm2 $%02x\n", data);

		SETCOL_B(PL2, data);     /* set player 2 color */
		SETCOL_B(MI2, data);     /* set missile 2 color */
		SETCOL_B(GT2+2, data);   /* set GTIA mode 2 color 2 */
		SETCOL_B(P200, data);    /* set player 2 both pixels 0 */
		SETCOL_L(P201, data);    /* set player 2 left pixel 0 */
		SETCOL_R(P210, data);    /* set player 2 right pixel 0 */
		SPLIT_HUE(data, m_huepm2);
		data = m_huepm2 | m_lumpf1;
		SETCOL_R(P201, data);    /* set player 2 right pixel 1 */
		SETCOL_L(P210, data);    /* set player 2 left pixel 1 */
		SETCOL_B(P211, data);    /* set player 2 both pixels 1 */
		break;

	case 21:    /* color for player/missile #3 */
		if (data == m_w.colpm3)
			break;
		m_w.colpm3 = data;
		if (VERBOSE)
			logerror("atari colpm3 $%02x\n", data);

		SETCOL_B(PL3, data);     /* set player 3 color */
		SETCOL_B(MI3, data);     /* set missile 3 color */
		SETCOL_B(GT2+3, data);   /* set GTIA mode 2 color 3 */
		SETCOL_B(P300, data);    /* set player 3 both pixels 0 */
		SETCOL_L(P301, data);    /* set player 3 left pixel 0 */
		SETCOL_R(P310, data);    /* set player 3 right pixel 0 */
		SPLIT_HUE(data, m_huepm3);
		data = m_huepm3 | m_lumpf1;
		SETCOL_R(P301, data);    /* set player 3 right pixel 1 */
		SETCOL_L(P310, data);    /* set player 3 left pixel 1 */
		SETCOL_B(P311, data);    /* set player 3 both pixels 1 */
		break;

	case 22:    /* playfield color #0 */
		if (data == m_w.colpf0)
			break;
		m_w.colpf0 = data;
		if (VERBOSE)
			logerror("atari colpf0 $%02x\n", data);

		SETCOL_B(PF0, data);     /* set playfield 0 color */
		SETCOL_B(GT2+4, data);   /* set GTIA mode 2 color 4 */
		break;

	case 23:    /* playfield color #1 */
		if (data == m_w.colpf1)
			break;
		m_w.colpf1 = data;
		if (VERBOSE)
			logerror("atari colpf1 $%02x\n", data);

		SETCOL_B(PF1, data);     /* set playfield 1 color */
		SETCOL_B(GT2+5, data);   /* set GTIA mode 2 color 5 */
		SPLIT_LUM(data, m_lumpf1);
		data = m_huepf2 | m_lumpf1;
		SETCOL_R(T01, data);     /* set text mode right pixel 1 */
		SETCOL_L(T10, data);     /* set text mode left pixel 1 */
		SETCOL_B(T11, data);     /* set text mode both pixels 1 */
		data = m_huebk | m_lumpf1;
		SETCOL_R(G01, data);     /* set graphics mode right pixel 1 */
		SETCOL_L(G10, data);     /* set graphics mode left pixel 1 */
		SETCOL_B(G11, data);     /* set graphics mode both pixels 1 */
		data = m_huepm0 | m_lumpf1;
		SETCOL_R(P001, data);    /* set player 0 right pixel 1 */
		SETCOL_L(P010, data);    /* set player 0 left pixel 1 */
		SETCOL_B(P011, data);    /* set player 0 both pixels 1 */
		data = m_huepm1 | m_lumpf1;
		SETCOL_R(P101, data);    /* set player 1 right pixel 1 */
		SETCOL_L(P110, data);    /* set player 1 left pixel 1 */
		SETCOL_B(P111, data);    /* set player 1 both pixels 1 */
		data = m_huepm2 | m_lumpf1;
		SETCOL_R(P201, data);    /* set player 2 right pixel 1 */
		SETCOL_L(P210, data);    /* set player 2 left pixel 1 */
		SETCOL_B(P211, data);    /* set player 2 both pixels 1 */
		data = m_huepm3 | m_lumpf1;
		SETCOL_R(P301, data);    /* set player 3 right pixel 1 */
		SETCOL_L(P310, data);    /* set player 3 left pixel 1 */
		SETCOL_B(P311, data);    /* set player 3 both pixels 1 */
		data = m_huepm4 | m_lumpf1;
		SETCOL_R(P401, data);    /* set missiles right pixel 1 */
		SETCOL_L(P410, data);    /* set missiles left pixel 1 */
		SETCOL_B(P411, data);    /* set missiles both pixels 1 */
		break;

	case 24:    /* playfield color #2 */
		if (data == m_w.colpf2)
			break;
		m_w.colpf2 = data;
		if (VERBOSE)
			logerror("atari colpf2 $%02x\n", data);

		SETCOL_B(PF2, data);     /* set playfield color 2 */
		SETCOL_B(GT2+6, data);   /* set GTIA mode 2 color 6 */
		SETCOL_B(T00, data);     /* set text mode both pixels 0 */
		SETCOL_L(T01, data);     /* set text mode left pixel 0 */
		SETCOL_R(T10, data);     /* set text mode right pixel 0 */
		SPLIT_HUE(data, m_huepf2);
		data = m_huepf2 | m_lumpf1;
		SETCOL_R(T01, data);     /* set text mode right pixel 1 */
		SETCOL_L(T10, data);     /* set text mode left pixel 1 */
		SETCOL_B(T11, data);     /* set text mode both pixels 1 */
		break;

	case 25:    /* playfield color #3 */
		if (data == m_w.colpf3)
			break;
		m_w.colpf3 = data;
		if (VERBOSE)
			logerror("atari colpf3 $%02x\n", data);

		SETCOL_B(PF3, data);     /* set playfield color 3 */
		SETCOL_B(GT2+7, data);   /* set GTIA mode 2 color 7 */
		SETCOL_B(P400, data);    /* set p/m xor mode both pixels 0 */
		SETCOL_L(P401, data);    /* set p/m xor mode left pixel 0 */
		SETCOL_R(P410, data);    /* set p/m xor mode right pixel 0 */
		SPLIT_HUE(data, m_huepm4);
		data = m_huepm4 | m_lumpf1;
		SETCOL_R(P401, data);    /* set p/m xor mode right pixel 1 */
		SETCOL_L(P410, data);    /* set p/m xor mode left pixel 1 */
		SETCOL_B(P411, data);    /* set p/m xor mode both pixels 1 */
		break;

	case 26:    /* playfield background */
		if (data == m_w.colbk)
			break;
		m_w.colbk = data;
		if (VERBOSE)
			logerror("atari colbk  $%02x\n", data);

		SETCOL_B(PBK, data);     /* set background color */
		SETCOL_B(GT2+8, data);   /* set GTIA mode 2 color 8 */
		SETCOL_B(GT2+9, data);   /* set GTIA mode 2 color 9 */
		SETCOL_B(GT2+10, data);  /* set GTIA mode 2 color 10 */
		SETCOL_B(GT2+11, data);  /* set GTIA mode 2 color 11 */
		SETCOL_B(GT2+12, data);  /* set GTIA mode 2 color 12 */
		SETCOL_B(GT2+13, data);  /* set GTIA mode 2 color 13 */
		SETCOL_B(GT2+14, data);  /* set GTIA mode 2 color 14 */
		SETCOL_B(GT2+15, data);  /* set GTIA mode 2 color 15 */
		SETCOL_B(G00, data);     /* set 2 color graphics both pixels 0 */
		SETCOL_L(G01, data);     /* set 2 color graphics left pixel 0 */
		SETCOL_R(G10, data);     /* set 2 color graphics right pixel 0 */
		SPLIT_HUE(data, m_huebk);
		data = m_huebk | m_lumpf1;
		SETCOL_R(G01, data);     /* set 2 color graphics right pixel 1 */
		SETCOL_L(G10, data);     /* set 2 color graphics left pixel 1 */
		SETCOL_B(G11, data);     /* set 2 color graphics both pixels 1 */
		SETCOL_B(GT1+ 0, (data & HUE) + (0x00 & LUM));   /* set GTIA mode 1 HUE + LUM 0..15 */
		SETCOL_B(GT1+ 1, (data & HUE) + (0x11 & LUM));
		SETCOL_B(GT1+ 2, (data & HUE) + (0x22 & LUM));
		SETCOL_B(GT1+ 3, (data & HUE) + (0x33 & LUM));
		SETCOL_B(GT1+ 4, (data & HUE) + (0x44 & LUM));
		SETCOL_B(GT1+ 5, (data & HUE) + (0x55 & LUM));
		SETCOL_B(GT1+ 6, (data & HUE) + (0x66 & LUM));
		SETCOL_B(GT1+ 7, (data & HUE) + (0x77 & LUM));
		SETCOL_B(GT1+ 8, (data & HUE) + (0x88 & LUM));
		SETCOL_B(GT1+ 9, (data & HUE) + (0x99 & LUM));
		SETCOL_B(GT1+10, (data & HUE) + (0xaa & LUM));
		SETCOL_B(GT1+11, (data & HUE) + (0xbb & LUM));
		SETCOL_B(GT1+12, (data & HUE) + (0xcc & LUM));
		SETCOL_B(GT1+13, (data & HUE) + (0xdd & LUM));
		SETCOL_B(GT1+14, (data & HUE) + (0xee & LUM));
		SETCOL_B(GT1+15, (data & HUE) + (0xff & LUM));
		SETCOL_B(GT3+ 0, (data & LUM) + (0x00 & HUE));   /* set GTIA mode 3 LUM + HUE 0..15 */
		SETCOL_B(GT3+ 1, (data & LUM) + (0x11 & HUE));
		SETCOL_B(GT3+ 2, (data & LUM) + (0x22 & HUE));
		SETCOL_B(GT3+ 3, (data & LUM) + (0x33 & HUE));
		SETCOL_B(GT3+ 4, (data & LUM) + (0x44 & HUE));
		SETCOL_B(GT3+ 5, (data & LUM) + (0x55 & HUE));
		SETCOL_B(GT3+ 6, (data & LUM) + (0x66 & HUE));
		SETCOL_B(GT3+ 7, (data & LUM) + (0x77 & HUE));
		SETCOL_B(GT3+ 8, (data & LUM) + (0x88 & HUE));
		SETCOL_B(GT3+ 9, (data & LUM) + (0x99 & HUE));
		SETCOL_B(GT3+10, (data & LUM) + (0xaa & HUE));
		SETCOL_B(GT3+11, (data & LUM) + (0xbb & HUE));
		SETCOL_B(GT3+12, (data & LUM) + (0xcc & HUE));
		SETCOL_B(GT3+13, (data & LUM) + (0xdd & HUE));
		SETCOL_B(GT3+14, (data & LUM) + (0xee & HUE));
		SETCOL_B(GT3+15, (data & LUM) + (0xff & HUE));
		break;

	case 27:
		m_w.prior = data;
		recalc_m0();
		recalc_m1();
		recalc_m2();
		recalc_m3();
		break;

	case 28:    /* delay until vertical retrace */
		m_w.vdelay = data;
		m_h.vdelay_m0 = (data >> 0) & 1;
		m_h.vdelay_m1 = (data >> 1) & 1;
		m_h.vdelay_m2 = (data >> 2) & 1;
		m_h.vdelay_m3 = (data >> 3) & 1;
		m_h.vdelay_p0 = (data >> 4) & 1;
		m_h.vdelay_p1 = (data >> 5) & 1;
		m_h.vdelay_p2 = (data >> 6) & 1;
		m_h.vdelay_p3 = (data >> 7) & 1;
		break;

	case 29:
		m_w.gractl = data;
		recalc_p0();
		recalc_p1();
		recalc_p2();
		recalc_p3();
		recalc_m0();
		recalc_m1();
		recalc_m2();
		recalc_m3();
		break;

		case 30:    /* clear collisions */
		m_r.m0pf = m_r.m1pf = m_r.m2pf = m_r.m3pf =
		m_r.p0pf = m_r.p1pf = m_r.p2pf = m_r.p3pf =
		m_r.m0pl = m_r.m1pl = m_r.m2pl = m_r.m3pl =
		m_r.p0pl = m_r.p1pl = m_r.p2pl = m_r.p3pl = 0;
		m_w.hitclr = data;
		break;

	case 31:    /* write console (speaker) */
		if (data == m_w.cons)
			break;
		m_w.cons  = data;
		if (!m_write_cb.isnull())
			m_write_cb((offs_t)0, m_w.cons);
		break;
	}
}



void gtia_device::gtia_postload()
{
	recalc_p0();
	recalc_p1();
	recalc_p2();
	recalc_p3();
	recalc_m0();
	recalc_m1();
	recalc_m2();
	recalc_m3();
}



static const UINT8 pf_collision[256] = {
	0,1,2,0,4,0,0,0,8,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static const int    pf_prioindex[256] = {
/*          PBK   PF0   PF1         PF2                     PF3                                            */
/*     */   0x000,0x100,0x100,0x000,0x200,0x000,0x000,0x000,0x200,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
/*     */   0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
/*     */   0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
/*     */   0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
/* T00 */   0x400,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
/* T01 */   0x500,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
/* T10 */   0x600,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
/* T11 */   0x700,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
/* G00 */   0x400,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
/* G01 */   0x500,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
/* G10 */   0x600,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
/* G11 */   0x700,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
/* GT1 */   0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
/* GT2 */   0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
/* GT3 */   0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
/*     */   0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000
};

inline void gtia_device::player_render(UINT8 gfx, int size, UINT8 color, UINT8 *dst)
{
	// size is the number of bits in *dst to be filled: 1, 2 or 4
	if (size == 3)
		size = 2;
	for (int i = 0; i < 8; i++)
		if (BIT(gfx, 7 - i))
			for (int s = 0; s < size; s++)
				dst[i * size + s] |= color;
}

inline void gtia_device::missile_render(UINT8 gfx, int size, UINT8 color, UINT8 *dst)
{
	// size is the number of bits in *dst to be filled: 1, 2 or 4
	if (size == 3)
		size = 2;
	for (int i = 0; i < 2; i++)
		if (BIT(gfx, 7 - i))
			for (int s = 0; s < size; s++)
				dst[i * size + s] |= color;
}


void gtia_device::render(UINT8 *src, UINT8 *dst, UINT8 *prio, UINT8 *pmbits)
{
	if (m_h.grafp0)
		player_render(m_h.grafp0, m_w.sizep0 + 1, GTIA_P0, &pmbits[m_w.hposp0]);
	if (m_h.grafp1)
		player_render(m_h.grafp1, m_w.sizep1 + 1, GTIA_P1, &pmbits[m_w.hposp1]);
	if (m_h.grafp2)
		player_render(m_h.grafp2, m_w.sizep2 + 1, GTIA_P2, &pmbits[m_w.hposp2]);
	if (m_h.grafp3)
		player_render(m_h.grafp3, m_w.sizep3 + 1, GTIA_P3, &pmbits[m_w.hposp3]);

	if (m_h.grafm0)
		missile_render(m_h.grafm0, m_w.sizem + 1, GTIA_M0, &pmbits[m_w.hposm0]);
	if (m_h.grafm1)
		missile_render(m_h.grafm1, m_w.sizem + 1, GTIA_M1, &pmbits[m_w.hposm1]);
	if (m_h.grafm2)
		missile_render(m_h.grafm2, m_w.sizem + 1, GTIA_M2, &pmbits[m_w.hposm2]);
	if (m_h.grafm3)
		missile_render(m_h.grafm3, m_w.sizem + 1, GTIA_M3, &pmbits[m_w.hposm3]);

	for (int x = 0; x < GTIA_HWIDTH * 4; x++, src++, dst++)
	{
		UINT8 pm, pc, pf;
		if (!*src)
			continue;
		/* get the player/missile combination bits and reset the buffer */
		pm = *src;
		*src = 0;
		/* get the current playfield color */
		pc = *dst;
		pf = pf_collision[pc];
		if (pm & GTIA_P0) { m_r.p0pf |= pf; m_r.p0pl |= pm & (          GTIA_P1 | GTIA_P2 | GTIA_P3); }
		if (pm & GTIA_P1) { m_r.p1pf |= pf; m_r.p1pl |= pm & (GTIA_P0 |           GTIA_P2 | GTIA_P3); }
		if (pm & GTIA_P2) { m_r.p2pf |= pf; m_r.p2pl |= pm & (GTIA_P0 | GTIA_P1 |           GTIA_P3); }
		if (pm & GTIA_P3) { m_r.p3pf |= pf; m_r.p3pl |= pm & (GTIA_P0 | GTIA_P1 | GTIA_P2          ); }
		if (pm & GTIA_M0) { m_r.m0pf |= pf; m_r.m0pl |= pm & (GTIA_P0 | GTIA_P1 | GTIA_P2 | GTIA_P3); }
		if (pm & GTIA_M1) { m_r.m1pf |= pf; m_r.m1pl |= pm & (GTIA_P0 | GTIA_P1 | GTIA_P2 | GTIA_P3); }
		if (pm & GTIA_M2) { m_r.m2pf |= pf; m_r.m2pl |= pm & (GTIA_P0 | GTIA_P1 | GTIA_P2 | GTIA_P3); }
		if (pm & GTIA_M3) { m_r.m3pf |= pf; m_r.m3pl |= pm & (GTIA_P0 | GTIA_P1 | GTIA_P2 | GTIA_P3); }
		/* color with higher priority? change playfield */
		pc = prio[pf_prioindex[pc] | pm];
		if (pc) *dst = pc;
	}
	/* copy player/missile graphics in case of vdelay */
	m_w.grafp0[1] = m_w.grafp0[0];
	m_w.grafp1[1] = m_w.grafp1[0];
	m_w.grafp2[1] = m_w.grafp2[0];
	m_w.grafp3[1] = m_w.grafp3[0];
	m_w.grafm[1] = m_w.grafm[0];
}
