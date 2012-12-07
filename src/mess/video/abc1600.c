#include "includes/abc1600.h"
#include "abc1600.lh"



//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define LOG 0


// video RAM
#define VIDEORAM_SIZE		0x40000
#define VIDEORAM16_MASK		0x3ffff
#define VIDEORAM8_MASK		0x7fffe


// flag register
#define L_P			BIT(m_flag, 0)
#define BLANK		BIT(m_flag, 1)
#define PIX_POL		BIT(m_flag, 2)
#define FRAME_POL	BIT(m_flag, 3)
#define HOLD_FY		BIT(m_flag, 4)
#define HOLD_FX		BIT(m_flag, 5)
#define COMP_MOVE	BIT(m_flag, 6)
#define REPLACE		BIT(m_flag, 7)


// image position
#define HFP			96
#define VFP			23


// IOWR0 registers
enum
{
	LDSX_HB = 0,
	LDSX_LB,
	LDSY_HB,
	LDSY_LB,
	LDTX_HB,
	LDTX_LB,
	LDTY_HB,
	LDTY_LB
};


// IOWR1 registers
enum
{
	LDFX_HB = 0,
	LDFX_LB,
	LDFY_HB,
	LDFY_LB,
	WRML = 5,
	WRDL = 7
};


// IOWR2 registers
enum
{
	WRMASK_STROBE_HB = 0,
	WRMASK_STROBE_LB,
	ENABLE_CLOCKS,
	FLAG_STROBE,
	ENDISP
};



//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  read_videoram -
//-------------------------------------------------

inline UINT16 abc1600_state::read_videoram(UINT32 offset)
{
	return m_video_ram[offset & VIDEORAM16_MASK];
}


//-------------------------------------------------
//  write_videoram -
//-------------------------------------------------

inline void abc1600_state::write_videoram(UINT32 offset, UINT16 data, UINT16 mask)
{
	UINT16 old_data = m_video_ram[offset & VIDEORAM16_MASK];

	m_video_ram[offset & VIDEORAM16_MASK] = (data & mask) | (old_data & (mask ^ 0xffff));
}


//-------------------------------------------------
//  video_ram_r -
//-------------------------------------------------

READ8_MEMBER( abc1600_state::video_ram_r )
{
	UINT32 addr = (offset & VIDEORAM8_MASK) >> 1;
	UINT8 data = 0;

	if (offset & 0x01)
	{
		data = m_video_ram[addr] & 0xff;
	}
	else
	{
		data = m_video_ram[addr] >> 8;
	}

	return data;
}


//-------------------------------------------------
//  video_ram_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_state::video_ram_w )
{
	UINT32 addr = (offset & VIDEORAM8_MASK) >> 1;

	if (offset & 0x01)
	{
		if (REPLACE)
		{
			// WRPORT_LB
			m_wrm = (m_wrm & 0xff00) | data;
			if (LOG) logerror("WRM LB %02x -> %04x\n", data, m_wrm);
		}
		else
		{
			// DATAPORT_LB
			m_gmdi = (m_gmdi & 0xff00) | data;
			if (LOG) logerror("GMDI LB %02x -> %04x\n", data, m_gmdi);
		}

		write_videoram(addr, m_gmdi, m_wrm & 0x00ff);

		if (LOG) logerror("Video RAM write LB to %05x : %04x\n", addr, m_video_ram[addr]);
	}
	else
	{
		if (REPLACE)
		{
			// WRPORT_HB
			m_wrm = (data << 8) | (m_wrm & 0xff);
			if (LOG) logerror("WRM HB %02x -> %04x\n", data, m_wrm);
		}
		else
		{
			// DATAPORT_HB
			m_gmdi = (data << 8) | (m_gmdi & 0xff);
			if (LOG) logerror("GMDI HB %02x -> %04x\n", data, m_gmdi);
		}

		write_videoram(addr, m_gmdi, m_wrm & 0xff00);

		if (LOG) logerror("Video RAM write HB to %05x : %04x\n", addr, m_video_ram[addr]);
	}
}


//-------------------------------------------------
//  iord0_r -
//-------------------------------------------------

READ8_MEMBER( abc1600_state::iord0_r )
{
	/*

        bit     description

        0       0
        1       SCREENPOS
        2
        3
        4
        5
        6       VSYNC
        7       BUSY

    */

	UINT8 data = 0;

	// monitor orientation (portrait/landscape)
	data |= machine().render().first_target()->view() << 1;

	// vertical sync
	data |= m_crtc->vsync_r() << 6;

	return data;
}


//-------------------------------------------------
//  iowr0_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_state::iowr0_w )
{
	switch (offset & 0x07)
	{
	case LDSX_HB:
		/*

            bit     description

            0       XSIZE8
            1       XSIZE9
            2       U/D* Y
            3       U/D* X
            4
            5
            6
            7

        */

		if (LOG) logerror("%s LDSX HB: %02x\n", machine().describe_context(), data);

		m_xsize = ((data & 0x03) << 8) | (m_xsize & 0xff);
		m_udy = BIT(data, 2);
		m_udx = BIT(data, 3);
		break;

	case LDSX_LB:
		/*

            bit     description

            0       XSIZE0
            1       XSIZE1
            2       XSIZE2
            3       XSIZE3
            4       XSIZE4
            5       XSIZE5
            6       XSIZE6
            7       XSIZE7

        */

		if (LOG) logerror("%s LDSX LB: %02x\n", machine().describe_context(), data);

		m_xsize = (m_xsize & 0x300) | data;
		break;

	case LDSY_HB:
		/*

            bit     description

            0       YSIZE8
            1       YSIZE9
            2       YSIZE10
            3       YSIZE11
            4
            5
            6
            7

        */

		if (LOG) logerror("%s LDSY HB: %02x\n", machine().describe_context(), data);

		m_ysize = ((data & 0x0f) << 8) | (m_ysize & 0xff);
		break;

	case LDSY_LB:
		/*

            bit     description

            0       YSIZE0
            1       YSIZE1
            2       YSIZE2
            3       YSIZE3
            4       YSIZE4
            5       YSIZE5
            6       YSIZE6
            7       YSIZE7

        */

		if (LOG) logerror("%s LDSX LB: %02x\n", machine().describe_context(), data);

		m_ysize = (m_ysize & 0xf00) | data;
		break;

	case LDTX_HB:
		/*

            bit     description

            0       XTO8, MTA4
            1       XTO9, MTA5
            2
            3
            4
            5
            6
            7

        */

		if (LOG) logerror("%s LDTX HB: %02x\n", machine().describe_context(), data);

		m_xto = ((data & 0x03) << 8) | (m_xto & 0xff);
		m_mta = (m_mta & 0x3ffcf) | ((data & 0x03) << 4);
		break;

	case LDTX_LB:
		/*

            bit     description

            0       XTO0
            1       XTO1
            2       XTO2
            3       XTO3
            4       XTO4, MTA0
            5       XTO5, MTA1
            6       XTO6, MTA2
            7       XTO7, MTA3

        */

		if (LOG) logerror("%s LDTX LB: %02x\n", machine().describe_context(), data);

		m_xto = (m_xto & 0x300) | data;
		m_mta = (m_mta & 0x3fff0) | (data >> 4);
		break;

	case LDTY_HB:
		/*

            bit     description

            0       YTO8, MTA14
            1       YTO9, MTA15
            2       YTO10, MTA16
            3       YTO11, MTA17
            4
            5
            6
            7

        */

		if (LOG) logerror("%s LDTY HB: %02x\n", machine().describe_context(), data);

		m_ty = ((data & 0x0f) << 8) | (m_yto & 0xff);
		m_yto = ((data & 0x0f) << 8) | (m_yto & 0xff);
		m_mta = ((data & 0x0f) << 14) | (m_mta & 0x3fff);
		break;

	case LDTY_LB:
		/*

            bit     description

            0       YTO0, MTA6
            1       YTO1, MTA7
            2       YTO2, MTA8
            3       YTO3, MTA9
            4       YTO4, MTA10
            5       YTO5, MTA11
            6       YTO6, MTA12
            7       YTO7, MTA13

        */

		if (LOG) logerror("%s LDTY LB: %02x\n", machine().describe_context(), data);

		m_ty = (m_ty & 0xf00) | data;
		m_yto = (m_yto & 0xf00) | data;
		m_mta = (m_mta & 0x3c03f) | (data << 6);
		break;
	}
}


//-------------------------------------------------
//  iowr1_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_state::iowr1_w )
{
	switch (offset & 0x07)
	{
	case LDFX_HB:
		/*

            bit     description

            0       XFROM8, MFA4
            1       XFROM9, MFA5
            2
            3
            4
            5
            6
            7

        */

		if (LOG) logerror("%s LDFX HB: %02x\n", machine().describe_context(), data);

		m_xfrom = ((data & 0x03) << 8) | (m_xfrom & 0xff);
		m_mfa = (m_mfa & 0x3ffcf) | ((data & 0x03) << 4);
		break;

	case LDFX_LB:
		/*

            bit     description

            0       XFROM0
            1       XFROM1
            2       XFROM2
            3       XFROM3
            4       XFROM4, MFA0
            5       XFROM5, MFA1
            6       XFROM6, MFA2
            7       XFROM7, MFA3

        */

		if (LOG) logerror("%s LDFX LB: %02x\n", machine().describe_context(), data);

		m_xfrom = (m_xfrom & 0x300) | data;
		m_mfa = (m_mfa & 0x3fff0) | (data >> 4);
		break;

	case LDFY_HB:
		/*

            bit     description

            0       MFA14
            1       MFA15
            2       MFA16
            3       MFA17
            4
            5
            6
            7

        */

		if (LOG) logerror("%s LDFY HB: %02x\n", machine().describe_context(), data);

		m_mfa = ((data & 0x0f) << 14) | (m_mfa & 0x3fff);
		break;

	case LDFY_LB:
		/*

            bit     description

            0       MFA6
            1       MFA7
            2       MFA8
            3       MFA9
            4       MFA10
            5       MFA11
            6       MFA12
            7       MFA13

        */

		if (LOG) logerror("%s LDFY LB: %02x\n", machine().describe_context(), data);

		m_mfa = (m_mfa & 0x3c03f) | (data << 6);

		mover();
		break;

	case WRML:
		/*

            bit     description

            0       MOVE CYK CLK
            1       DISP CYC SEL / DISP CYK PRE FETCH (+1 PIXCLK)
            2       DATA CLK
            3       _DISP MEM WE
            4       _CAS HB
            5       DTACK CLK / BLANK TEST (+2 PIXCLK)
            6       DISPREC CLK
            7       _RAS HB

        */

		if (LOG) logerror("MS %u : %02x\n", (offset >> 4) & 0x0f, data);

		if (m_clocks_disabled)
		{
			m_ms[(offset >> 4) & 0x0f] = data;
		}
		break;

	case WRDL:
		/*

            bit     description

            0       MOVE CYK CLK
            1       DISP CYC SEL / DISP CYK PRE FETCH (+1 PIXCLK)
            2       DATA CLK
            3       _DISP MEM WE
            4       _CAS HB
            5       DTACK CLK / BLANK TEST (+2 PIXCLK)
            6       DISPREC CLK
            7       _RAS HB

        */

		if (LOG) logerror("WS %u : %02x\n", (offset >> 4) & 0x0f, data);

		if (m_clocks_disabled)
		{
			m_ds[(offset >> 4) & 0x0f] = data;
		}
		break;
	}
}


//-------------------------------------------------
//  iowr2_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_state::iowr2_w )
{
	switch (offset & 0x07)
	{
	case WRMASK_STROBE_HB:
		if (REPLACE)
		{
			// DATAPORT_HB
			m_gmdi = (data << 8) | (m_gmdi & 0xff);
			if (LOG) logerror("GMDI HB %04x\n", m_gmdi);
		}
		else
		{
			// WRPORT_HB
			m_wrm = (data << 8) | (m_wrm & 0xff);
			if (LOG) logerror("WRM HB %04x\n", m_gmdi);
		}
		break;

	case WRMASK_STROBE_LB:
		if (REPLACE)
		{
			// DATAPORT_LB
			m_gmdi = (m_gmdi & 0xff00) | data;
			if (LOG) logerror("GMDI LB %04x\n", m_gmdi);
		}
		else
		{
			// WRPORT_LB
			m_wrm = (m_wrm & 0xff00) | data;
			if (LOG) logerror("WRM LB %04x\n", m_gmdi);
		}
		break;

	case ENABLE_CLOCKS:
		if (LOG) logerror("ENABLE CLOCKS\n");
		m_clocks_disabled = 0;
		break;

	case FLAG_STROBE:
		/*

            bit     description

            0       L/_P FLAG
            1       BLANK FLAG
            2       PIX POL
            3       FRAME POL
            4       HOLD FY
            5       HOLD FX
            6       COMP MOVE FLAG
            7       REPLACE/SET & RESET

        */

		m_flag = data;
		if (LOG) logerror("FLAG %02x\n", m_flag);
		break;

	case ENDISP:
		if (LOG) logerror("ENDISP\n");
		m_endisp = 1;
		break;
	}
}



//**************************************************************************
//  MOVER
//**************************************************************************

//-------------------------------------------------
//  clock_mfa_x -
//-------------------------------------------------

inline void abc1600_state::clock_mfa_x()
{
	UINT16 mfa_y = m_mfa >> 6;
	UINT8 mfa_x = m_mfa & 0x3f;

	if (!HOLD_FX)
	{
		mfa_x += m_udx ? 1 : -1;
		mfa_x &= 0x3f;
	}

	m_mfa = (mfa_y << 6) | mfa_x;
}


//-------------------------------------------------
//  clock_mfa_y -
//-------------------------------------------------

inline void abc1600_state::clock_mfa_y()
{
	UINT16 mfa_y = m_mfa >> 6;
	UINT8 mfa_x = m_mfa & 0x3f;

	if (!HOLD_FY)
	{
		mfa_y += m_udy ? 1 : -1;
		mfa_y &= 0xfff;
	}

	m_mfa = (mfa_y << 6) | mfa_x;
}


//-------------------------------------------------
//  clock_mta_x -
//-------------------------------------------------

inline void abc1600_state::clock_mta_x()
{
	UINT16 mta_y = m_mta >> 6;
	UINT8 mta_x = m_mta & 0x3f;

	mta_x += m_udx ? 1 : -1;
	mta_x &= 0x3f;

	m_mta = (mta_y << 6) | mta_x;
}


//-------------------------------------------------
//  clock_mta_y -
//-------------------------------------------------

inline void abc1600_state::clock_mta_y()
{
	UINT16 mta_y = m_mta >> 6;
	UINT8 mta_x = m_mta & 0x3f;

	mta_y += m_udy ? 1 : -1;
	mta_y &= 0xfff;

	m_mta = (mta_y << 6) | mta_x;
}


//-------------------------------------------------
//  load_mfa_x -
//-------------------------------------------------

inline void abc1600_state::load_mfa_x()
{
	UINT16 mfa_y = m_mfa >> 6;
	UINT8 mfa_x = m_xfrom >> 4;

	m_mfa = (mfa_y << 6) | mfa_x;
}


//-------------------------------------------------
//  load_mta_x -
//-------------------------------------------------

inline void abc1600_state::load_mta_x()
{
	UINT16 mta_y = m_mta >> 6;
	UINT8 mta_x = m_xto >> 4;

	m_mta = (mta_y << 6) | mta_x;
}


//-------------------------------------------------
//  load_xy_reg -
//-------------------------------------------------

inline void abc1600_state::load_xy_reg()
{
	if (L_P) return;

	UINT16 sum = m_xto + m_xsize;

	m_xto = sum & 0x3ff;
	m_yto = m_ty & 0xfff;
	m_mta = (m_ty << 6) | (sum >> 4);
}


//-------------------------------------------------
//  compare_mta_x -
//-------------------------------------------------

inline void abc1600_state::compare_mta_x()
{
	UINT8 mta_x_end = ((m_xto + m_xsize) >> 4) & 0x3f;
	UINT8 mta_x = m_mta & 0x3f;

	if (mta_x == mta_x_end)
	{
		m_cmc = 0;
	}

	m_wrms1 = m_cmc & m_amm;
}


//-------------------------------------------------
//  compare_mta_y -
//-------------------------------------------------

inline void abc1600_state::compare_mta_y()
{
	int mta_y_end = (m_yto + m_ysize) & 0xfff;
	UINT16 mta_y = m_mta >> 6;

	if (mta_y == mta_y_end)
	{
		m_rmc = 0;
	}
}


//-------------------------------------------------
//  get_shinf -
//-------------------------------------------------

inline void abc1600_state::get_shinf()
{
	/*

        bit     description

        A0      XFROM0
        A1      XFROM1
        A2      XFROM2
        A3      XFROM3
        A4      XTO0
        A5      XTO1
        A6      XTO2
        A7      XTO3
        A8      U/D* X

    */

	UINT16 shinf_addr = (m_udx << 8) | ((m_xto & 0x0f) << 4) | (m_xfrom & 0x0f);
	UINT8 shinf = m_shinf_rom[shinf_addr];

	m_sh = shinf & 0x0f;
	m_hold_1w_cyk = BIT(shinf, 5);
}


//-------------------------------------------------
//  get_drmsk -
//-------------------------------------------------

inline UINT16 abc1600_state::get_drmsk()
{
	/*

        bit     description

        A0      SH0
        A1      SH1
        A2      SH2
        A3      SH3
        A4      U/D* X

    */

	UINT16 drmsk_addr = (m_udx << 4) | (m_sh & 0x0f);
	UINT8 drmskl = m_drmsk_rom[drmsk_addr];
	UINT8 drmskh = m_drmsk_rom[drmsk_addr + 0x20];
	UINT16 drmsk = (drmskh << 8) | drmskl;

	return drmsk;
}


//-------------------------------------------------
//  get_wrmsk - get mover write mask
//-------------------------------------------------

inline UINT16 abc1600_state::get_wrmsk()
{
	/*

        bit     description

        A0      XTO0
        A1      XTO1
        A2      XTO2
        A3      XTO3
        A4      XSIZE0
        A5      XSIZE1
        A6      XSIZE2
        A7      XSIZE3
        A8      U/D* X
        A9      ANDED MASKS
        A10     WRMS0
        A11     WRMS1

    */

	UINT16 wrmsk_addr = (m_wrms1 << 11) | (m_wrms0 << 10) | ((!m_wrms1 && !m_wrms0) << 9) | (m_udx << 8) | ((m_xsize & 0x0f) << 4) | (m_xto & 0x0f);
	UINT8 wrmskl = m_wrmsk_rom[wrmsk_addr];
	UINT8 wrmskh = m_wrmsk_rom[wrmsk_addr + 0x1000];
	UINT16 wrmsk = (wrmskh << 8) | wrmskl;

	return wrmsk ^ 0xffff;
}


//-------------------------------------------------
//  barrel_shift -
//-------------------------------------------------

inline UINT16 abc1600_state::barrel_shift(UINT16 gmdr)
{
	UINT16 rot = gmdr;

	for (int sh = 0; sh < m_sh; sh++)
	{
		int msb = BIT(rot, 15);
		rot <<= 1;
		rot |= msb;
	}

	return rot;
}


//-------------------------------------------------
//  word_mixer -
//-------------------------------------------------

inline UINT16 abc1600_state::word_mixer(UINT16 rot)
{
	UINT16 drmsk = get_drmsk();
	UINT16 gmdi = (rot & drmsk) | (m_mdor & (drmsk ^ 0xffff));

	if (COMP_MOVE)
	{
		gmdi ^= 0xffff;
	}

	m_mdor = rot;

	return gmdi;
}


//-------------------------------------------------
//  mover -
//-------------------------------------------------

void abc1600_state::mover()
{
	if (LOG) logerror("XFROM %u XSIZE %u YSIZE %u XTO %u YTO %u MFA %05x MTA %05x U/D*X %u U/D*Y %u\n", m_xfrom, m_xsize, m_ysize, m_xto, m_yto, m_mfa, m_mta, m_udx, m_udy);

	m_amm = 1;

	m_rmc = 1;
	get_shinf();

	do
	{
		compare_mta_y();

		load_mfa_x();
		load_mta_x();
		m_cmc = 1;
		m_wrms0 = 0;

		if (m_hold_1w_cyk)
		{
			// read one word in advance
			UINT16 gmdr = read_videoram(m_mfa);
			UINT16 rot = barrel_shift(gmdr);
			word_mixer(rot);

			clock_mfa_x();
		}

		do
		{
			compare_mta_x();

			UINT16 gmdr = read_videoram(m_mfa);
			UINT16 rot = barrel_shift(gmdr);
			UINT16 gmdi = word_mixer(rot);
			UINT16 mask = get_wrmsk();

			write_videoram(m_mta, gmdi, mask);

			clock_mfa_x();
			clock_mta_x();

			m_wrms0 = 1;
		}
		while (m_cmc);

		clock_mfa_y();
		clock_mta_y();
	}
	while (m_rmc);

	load_xy_reg();

	m_amm = 0;
}



//**************************************************************************
//  CRT CONTROLLER
//**************************************************************************

//-------------------------------------------------
//  mc6845_interface crtc_intf
//-------------------------------------------------

inline UINT16 abc1600_state::get_crtca(UINT16 ma, UINT8 ra, UINT8 column)
{
	/*

        bit         description

        CRTCA0      0
        CRTCA1      0
        CRTCA2      CC1/MA1
        CRTCA3      CC2/MA2
        CRTCA4      CC3/MA3
        CRTCA5      CC4/MA4
        CRTCA6      RA0
        CRTCA7      RA1
        CRTCA8      RA2
        CRTCA9      RA3
        CRTCA10     CR0/MA8
        CRTCA11     CR1/MA9
        CRTCA12     CR2/MA10
        CRTCA13     CR3/MA11
        CRTCA14     CR4/MA12
        CRTCA15     CR5/MA13

    */

	UINT8 cc = (ma & 0xff) + column;
	UINT8 cr = ma >> 8;

	return (cr << 10) | ((ra & 0x0f) << 6) | ((cc << 1) & 0x3c);
}

void abc1600_state::crtc_update_row(device_t *device, bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT16 ma, UINT8 ra, UINT16 y, UINT8 x_count, INT8 cursor_x, void *param)
{
	if (y > 0x3ff) return;

	int x = HFP;

	for (int column = 0; column < x_count; column += 2)
	{
		UINT16 dma = get_crtca(ma, ra, column);

		// data is read out of video RAM in nibble mode by strobing CAS 4 times
		for (int cas = 0; cas < 4; cas++)
		{
			UINT16 data = m_video_ram[dma + cas];

			for (int bit = 0; bit < 16; bit++)
			{
				int color = (BIT(data, 15) ^ PIX_POL) & !BLANK;

				bitmap.pix32(y + VFP, x++) = RGB_MONOCHROME_GREEN[color];

				data <<= 1;
			}
		}
	}
}

static MC6845_UPDATE_ROW( abc1600_update_row )
{
	abc1600_state *state = device->machine().driver_data<abc1600_state>();
	state->crtc_update_row(device, bitmap, cliprect, ma, ra, y, x_count, cursor_x, param);
}

static MC6845_ON_UPDATE_ADDR_CHANGED( crtc_update )
{
}

static const mc6845_interface crtc_intf =
{
	SCREEN_TAG,
	32,
	NULL,
	abc1600_update_row,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	crtc_update
};



//**************************************************************************
//  VIDEO
//**************************************************************************

//-------------------------------------------------
//  VIDEO_START( abc1600 )
//-------------------------------------------------

void abc1600_state::video_start()
{
	// allocate video RAM
	m_video_ram.allocate(VIDEORAM_SIZE);

	// find memory regions
	m_wrmsk_rom = memregion("wrmsk")->base();
	m_shinf_rom = memregion("shinf")->base();
	m_drmsk_rom = memregion("drmsk")->base();

	// state saving
	save_item(NAME(m_endisp));
	save_item(NAME(m_clocks_disabled));
	save_item(NAME(m_gmdi));
	save_item(NAME(m_wrm));
	save_item(NAME(m_ms));
	save_item(NAME(m_ds));
	save_item(NAME(m_flag));
	save_item(NAME(m_xsize));
	save_item(NAME(m_ysize));
	save_item(NAME(m_udx));
	save_item(NAME(m_udy));
	save_item(NAME(m_xfrom));
	save_item(NAME(m_xto));
	save_item(NAME(m_yto));
	save_item(NAME(m_ty));
	save_item(NAME(m_mfa));
	save_item(NAME(m_mta));
	save_item(NAME(m_sh));
	save_item(NAME(m_mdor));
	save_item(NAME(m_hold_1w_cyk));
	save_item(NAME(m_wrms0));
	save_item(NAME(m_wrms1));
	save_item(NAME(m_rmc));
	save_item(NAME(m_cmc));
}


//-------------------------------------------------
//  SCREEN_UPDATE_IND16( abc1600 )
//-------------------------------------------------

UINT32 abc1600_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// HACK expand visible area to workaround MC6845
	screen.set_visible_area(0, 958-1, 0, 1067-1);

	if (m_endisp)
	{
		bitmap.fill(FRAME_POL, cliprect);
		m_crtc->screen_update(screen, bitmap, cliprect);
	}
	else
	{
		bitmap.fill(get_black_pen(machine()), cliprect);
	}

	return 0;
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( abc1600_video )
//-------------------------------------------------

MACHINE_CONFIG_FRAGMENT( abc1600_video )
	MCFG_DEFAULT_LAYOUT(layout_abc1600)

    MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
    MCFG_SCREEN_REFRESH_RATE(60)
    MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
	MCFG_SCREEN_UPDATE_DRIVER(abc1600_state, screen_update)
    MCFG_SCREEN_SIZE(958, 1067)
    MCFG_SCREEN_VISIBLE_AREA(0, 958-1, 0, 1067-1)

	MCFG_MC6845_ADD(SY6845E_TAG, SY6845E, XTAL_64MHz/32, crtc_intf)
MACHINE_CONFIG_END
