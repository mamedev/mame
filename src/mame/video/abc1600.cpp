// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 1600 Mover emulation

**********************************************************************/

#include "includes/abc1600.h"
#include "abc1600.lh"
#include "render.h"


//**************************************************************************
//  CONSTANTS / MACROS
//**************************************************************************

#define LOG 0


#define SY6845E_TAG         "sy6845e"


// video RAM
#define VIDEORAM_SIZE       0x40000
#define VIDEORAM16_MASK     0x3ffff
#define VIDEORAM8_MASK      0x7fffe


// flag register
#define L_P         BIT(m_flag, 0)
#define BLANK       BIT(m_flag, 1)
#define PIX_POL     BIT(m_flag, 2)
#define FRAME_POL   BIT(m_flag, 3)
#define HOLD_FY     BIT(m_flag, 4)
#define HOLD_FX     BIT(m_flag, 5)
#define COMP_MOVE   BIT(m_flag, 6)
#define REPLACE     BIT(m_flag, 7)



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ABC1600_MOVER = &device_creator<abc1600_mover_device>;


DEVICE_ADDRESS_MAP_START( vram_map, 8, abc1600_mover_device )
	AM_RANGE(0x00000, 0x7ffff) AM_READWRITE(video_ram_r, video_ram_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START( crtc_map, 8, abc1600_mover_device )
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xfe) AM_DEVREADWRITE(SY6845E_TAG, mc6845_device, status_r, address_w)
	AM_RANGE(0x01, 0x01) AM_MIRROR(0xfe) AM_DEVREADWRITE(SY6845E_TAG, mc6845_device, register_r, register_w)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START( io_map, 8, abc1600_mover_device )
	AM_RANGE(0x000, 0x000) AM_MIRROR(0xff) AM_READ(iord0_r)
	AM_RANGE(0x000, 0x000) AM_MIRROR(0xf8) AM_WRITE(ldsx_hb_w)
	AM_RANGE(0x001, 0x001) AM_MIRROR(0xf8) AM_WRITE(ldsx_lb_w)
	AM_RANGE(0x002, 0x002) AM_MIRROR(0xf8) AM_WRITE(ldsy_hb_w)
	AM_RANGE(0x003, 0x003) AM_MIRROR(0xf8) AM_WRITE(ldsy_lb_w)
	AM_RANGE(0x004, 0x004) AM_MIRROR(0xf8) AM_WRITE(ldtx_hb_w)
	AM_RANGE(0x005, 0x005) AM_MIRROR(0xf8) AM_WRITE(ldtx_lb_w)
	AM_RANGE(0x006, 0x006) AM_MIRROR(0xf8) AM_WRITE(ldty_hb_w)
	AM_RANGE(0x007, 0x007) AM_MIRROR(0xf8) AM_WRITE(ldty_lb_w)
	AM_RANGE(0x100, 0x100) AM_MIRROR(0xf8) AM_WRITE(ldfx_hb_w)
	AM_RANGE(0x101, 0x101) AM_MIRROR(0xf8) AM_WRITE(ldfx_lb_w)
	AM_RANGE(0x102, 0x102) AM_MIRROR(0xf8) AM_WRITE(ldfy_hb_w)
	AM_RANGE(0x103, 0x103) AM_MIRROR(0xf8) AM_WRITE(ldfy_lb_w)
	AM_RANGE(0x105, 0x105) AM_MIRROR(0xf8) AM_WRITE(wrml_w)
	AM_RANGE(0x107, 0x107) AM_MIRROR(0xf8) AM_WRITE(wrdl_w)
	AM_RANGE(0x200, 0x200) AM_MIRROR(0xf8) AM_WRITE(wrmask_strobe_hb_w)
	AM_RANGE(0x201, 0x201) AM_MIRROR(0xf8) AM_WRITE(wrmask_strobe_lb_w)
	AM_RANGE(0x202, 0x202) AM_MIRROR(0xf8) AM_WRITE(enable_clocks_w)
	AM_RANGE(0x203, 0x203) AM_MIRROR(0xf8) AM_WRITE(flag_strobe_w)
	AM_RANGE(0x204, 0x204) AM_MIRROR(0xf8) AM_WRITE(endisp_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( mover_map, AS_0, 16, abc1600_mover_device )
	AM_RANGE(0x00000, 0x7ffff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ROM( abc1600_mover )
//-------------------------------------------------

ROM_START( abc1600_mover )
	ROM_REGION16_BE( 0x2000, "wrmsk", 0 )
	ROM_LOAD16_BYTE( "wrmskl 6490362-01.1g", 0x0001, 0x1000, CRC(bc737538) SHA1(80e2c3757eb7f713018808d6e41ebef612425028) )
	ROM_LOAD16_BYTE( "wrmskh 6490363-01.1j", 0x0000, 0x1000, CRC(6b7c9f0b) SHA1(7155a993adcf08a5a8a2f22becf9fd66fda698be) )

	ROM_REGION( 0x200, "shinf", 0 )
	ROM_LOAD( "shinf 6490361-01.1f", 0x000, 0x200, CRC(20260f8f) SHA1(29bf49c64e7cc7592e88cde2768ac57c7ce5e085) )

	ROM_REGION16_BE( 0x40, "drmsk", 0 )
	ROM_LOAD16_BYTE( "drmskl 6490359-01.1k", 0x01, 0x20, CRC(6e71087c) SHA1(0acf67700d6227f4b315cf8fb0fb31c0e7fb9496) )
	ROM_LOAD16_BYTE( "drmskh 6490358-01.1l", 0x00, 0x20, CRC(a4a9a9dc) SHA1(d8575c0335d6021cbb5f7bcd298b41c35294a80a) )

	ROM_REGION( 0x104, "plds", 0 )
	ROM_LOAD( "drmsk 6490360-01.1m", 0x000, 0x104, CRC(5f7143c1) SHA1(1129917845f8e505998b15288f02bf907487e4ac) ) // mover word mixer @ 1m,1n,1t,2t
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *abc1600_mover_device::device_rom_region() const
{
	return ROM_NAME( abc1600_mover );
}


//-------------------------------------------------
//  mc6845
//-------------------------------------------------

inline UINT16 abc1600_mover_device::get_crtca(UINT16 ma, UINT8 ra, UINT8 column)
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

MC6845_UPDATE_ROW(abc1600_mover_device::crtc_update_row)
{
	int x = 0;
	const pen_t *pen = m_palette->pens();

	for (int column = 0; column < x_count; column += 2)
	{
		UINT16 dma = get_crtca(ma, ra, column);

		// data is read out of video RAM in nibble mode by strobing CAS 4 times
		for (int cas = 0; cas < 4; cas++)
		{
			UINT16 data = read_videoram(dma + cas);

			for (int bit = 0; bit < 16; bit++)
			{
				int color = ((BIT(data, 15) ^ PIX_POL) && !BLANK) && de;

				bitmap.pix32(vbp + y, hbp + x++) = pen[color];

				data <<= 1;
			}
		}
	}
}

MC6845_ON_UPDATE_ADDR_CHANGED( abc1600_mover_device::crtc_update )
{
}

//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( abc1600_mover )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( abc1600_mover )
	MCFG_DEFAULT_LAYOUT(layout_abc1600)

	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
	MCFG_SCREEN_UPDATE_DRIVER(abc1600_mover_device, screen_update)
	MCFG_SCREEN_SIZE(958, 1067)
	MCFG_SCREEN_VISIBLE_AREA(0, 958-1, 0, 1067-1)

	MCFG_PALETTE_ADD_MONOCHROME_GREEN("palette")

	MCFG_MC6845_ADD(SY6845E_TAG, SY6845E, SCREEN_TAG, XTAL_64MHz/32)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(32)
	MCFG_MC6845_UPDATE_ROW_CB(abc1600_mover_device, crtc_update_row)
	MCFG_MC6845_ADDR_CHANGED_CB(abc1600_mover_device, crtc_update)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor abc1600_mover_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( abc1600_mover );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc1600_mover_device - constructor
//-------------------------------------------------

abc1600_mover_device::abc1600_mover_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, ABC1600_MOVER, "ABC 1600 Mover", tag, owner, clock, "abc1600mover", __FILE__),
	device_memory_interface(mconfig, *this),
	m_space_config("vram", ENDIANNESS_BIG, 16, 18, -1, *ADDRESS_MAP_NAME(mover_map)),
	m_crtc(*this, SY6845E_TAG),
	m_palette(*this, "palette"),
	m_wrmsk_rom(*this, "wrmsk"),
	m_shinf_rom(*this, "shinf"),
	m_drmsk_rom(*this, "drmsk")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc1600_mover_device::device_start()
{
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
//  device_reset - device-specific reset
//-------------------------------------------------

void abc1600_mover_device::device_reset()
{
	// disable display
	m_clocks_disabled = 1;
	m_endisp = 0;
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *abc1600_mover_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : nullptr;
}


//-------------------------------------------------
//  read_videoram -
//-------------------------------------------------

inline UINT16 abc1600_mover_device::read_videoram(offs_t offset)
{
	return space().read_word((offset & VIDEORAM16_MASK) << 1);
}


//-------------------------------------------------
//  write_videoram -
//-------------------------------------------------

inline void abc1600_mover_device::write_videoram(offs_t offset, UINT16 data, UINT16 mask)
{
	UINT16 old_data = read_videoram(offset);

	space().write_word((offset & VIDEORAM16_MASK) << 1, (data & mask) | (old_data & (mask ^ 0xffff)));
}


//-------------------------------------------------
//  video_ram_r -
//-------------------------------------------------

READ8_MEMBER( abc1600_mover_device::video_ram_r )
{
	offs_t addr = (offset & VIDEORAM8_MASK) >> 1;
	UINT8 data = 0;

	if (offset & 0x01)
	{
		data = read_videoram(addr) & 0xff;
	}
	else
	{
		data = read_videoram(addr) >> 8;
	}

	return data;
}


//-------------------------------------------------
//  video_ram_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mover_device::video_ram_w )
{
	offs_t addr = (offset & VIDEORAM8_MASK) >> 1;

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

		if (LOG) logerror("Video RAM write LB to %05x : %04x\n", addr, read_videoram(addr));
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

		if (LOG) logerror("Video RAM write HB to %05x : %04x\n", addr, read_videoram(addr));
	}
}


//-------------------------------------------------
//  iord0_r -
//-------------------------------------------------

READ8_MEMBER( abc1600_mover_device::iord0_r )
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
//  ldsx_hb_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mover_device::ldsx_hb_w )
{
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
}


//-------------------------------------------------
//  ldsx_lb_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mover_device::ldsx_lb_w )
{
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
}


//-------------------------------------------------
//  ldsy_hb_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mover_device::ldsy_hb_w )
{
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
}


//-------------------------------------------------
//  ldsy_lb_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mover_device::ldsy_lb_w )
{
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

	if (LOG) logerror("%s LDSY LB: %02x\n", machine().describe_context(), data);

	m_ysize = (m_ysize & 0xf00) | data;
}


//-------------------------------------------------
//  ldtx_hb_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mover_device::ldtx_hb_w )
{
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
}


//-------------------------------------------------
//  ldtx_lb_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mover_device::ldtx_lb_w )
{
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
}


//-------------------------------------------------
//  ldty_hb_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mover_device::ldty_hb_w )
{
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

	if (L_P) return;

	m_ty = ((data & 0x0f) << 8) | (m_yto & 0xff);
	m_yto = ((data & 0x0f) << 8) | (m_yto & 0xff);
	m_mta = ((data & 0x0f) << 14) | (m_mta & 0x3fff);
}


//-------------------------------------------------
//  ldty_lb_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mover_device::ldty_lb_w )
{
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

	if (L_P) return;

	m_ty = (m_ty & 0xf00) | data;
	m_yto = (m_yto & 0xf00) | data;
	m_mta = (m_mta & 0x3c03f) | (data << 6);
}


//-------------------------------------------------
//  ldfx_hb_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mover_device::ldfx_hb_w )
{
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
}


//-------------------------------------------------
//  ldfx_lb_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mover_device::ldfx_lb_w )
{
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
}


//-------------------------------------------------
//  ldfy_hb_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mover_device::ldfy_hb_w )
{
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
}


//-------------------------------------------------
//  ldfy_lb_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mover_device::ldfy_lb_w )
{
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
}


//-------------------------------------------------
//  wrml_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mover_device::wrml_w )
{
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
}


//-------------------------------------------------
//  wrdl_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mover_device::wrdl_w )
{
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
}


//-------------------------------------------------
//  wrmask_strobe_hb_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mover_device::wrmask_strobe_hb_w )
{
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
}


//-------------------------------------------------
//  wrmask_strobe_lb_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mover_device::wrmask_strobe_lb_w )
{
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
}


//-------------------------------------------------
//  enable_clocks_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mover_device::enable_clocks_w )
{
	if (LOG) logerror("ENABLE CLOCKS\n");
	m_clocks_disabled = 0;
}


//-------------------------------------------------
//  flag_strobe_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mover_device::flag_strobe_w )
{
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
}


//-------------------------------------------------
//  endisp_w -
//-------------------------------------------------

WRITE8_MEMBER( abc1600_mover_device::endisp_w )
{
	m_endisp = 1;
	if (LOG) logerror("ENDISP\n");
}



//**************************************************************************
//  MOVER
//**************************************************************************

//-------------------------------------------------
//  clock_mfa_x -
//-------------------------------------------------

inline void abc1600_mover_device::clock_mfa_x()
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

inline void abc1600_mover_device::clock_mfa_y()
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

inline void abc1600_mover_device::clock_mta_x()
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

inline void abc1600_mover_device::clock_mta_y()
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

inline void abc1600_mover_device::load_mfa_x()
{
	UINT16 mfa_y = m_mfa >> 6;
	UINT8 mfa_x = m_xfrom >> 4;

	m_mfa = (mfa_y << 6) | mfa_x;
}


//-------------------------------------------------
//  load_mta_x -
//-------------------------------------------------

inline void abc1600_mover_device::load_mta_x()
{
	UINT16 mta_y = m_mta >> 6;
	UINT8 mta_x = m_xto >> 4;

	m_mta = (mta_y << 6) | mta_x;
}


//-------------------------------------------------
//  load_xy_reg -
//-------------------------------------------------

inline void abc1600_mover_device::load_xy_reg()
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

inline void abc1600_mover_device::compare_mta_x()
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

inline void abc1600_mover_device::compare_mta_y()
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

inline void abc1600_mover_device::get_shinf()
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

inline UINT16 abc1600_mover_device::get_drmsk()
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
	return m_drmsk_rom[drmsk_addr];
}


//-------------------------------------------------
//  get_wrmsk - get mover write mask
//-------------------------------------------------

inline UINT16 abc1600_mover_device::get_wrmsk()
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
	return m_wrmsk_rom[wrmsk_addr] ^ 0xffff;
}


//-------------------------------------------------
//  barrel_shift -
//-------------------------------------------------

inline UINT16 abc1600_mover_device::barrel_shift(UINT16 gmdr)
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

inline UINT16 abc1600_mover_device::word_mixer(UINT16 rot)
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

void abc1600_mover_device::mover()
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

UINT32 abc1600_mover_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_endisp)
	{
		bitmap.fill(m_palette->pen(FRAME_POL), cliprect);
		m_crtc->screen_update(screen, bitmap, cliprect);
	}
	else
	{
		bitmap.fill(rgb_t::black, cliprect);
	}

	return 0;
}
