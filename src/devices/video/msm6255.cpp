// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    OKI MSM6255 Dot Matrix LCD Controller implementation

**********************************************************************/

#include "msm6255.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


#define MOR_GRAPHICS        0x01
#define MOR_4_BIT_PARALLEL  0x02
#define MOR_2_BIT_PARALLEL  0x04
#define MOR_DISPLAY_ON      0x08
#define MOR_CURSOR_BLINK    0x10
#define MOR_CURSOR_ON       0x20
#define MOR_BLINK_TIME_16   0x40


#define PR_HP_4             0x03
#define PR_HP_5             0x04
#define PR_HP_6             0x05
#define PR_HP_7             0x06
#define PR_HP_8             0x07
#define PR_HP_MASK          0x07
#define PR_VP_MASK          0xf0


#define HNR_HN_MASK         0x7f


#define DVR_DN_MASK         0x7f


#define CPR_CPD_MASK        0x0f
#define CPR_CPU_MASK        0xf0



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
const device_type MSM6255 = &device_creator<msm6255_device>;

// I/O map
DEVICE_ADDRESS_MAP_START( map, 8, msm6255_device )
	AM_RANGE(0x00, 0x00) AM_READWRITE(dr_r, dr_w)
	AM_RANGE(0x01, 0x01) AM_READWRITE(ir_r, ir_w)
ADDRESS_MAP_END

// default address map
static ADDRESS_MAP_START( msm6255, AS_0, 8, msm6255_device )
	AM_RANGE(0x00000, 0xfffff) AM_RAM
ADDRESS_MAP_END



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  msm6255_device - constructor
//-------------------------------------------------

msm6255_device::msm6255_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, MSM6255, "MSM6255", tag, owner, clock, "msm6255", __FILE__),
	device_memory_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_space_config("videoram", ENDIANNESS_LITTLE, 8, 20, 0, nullptr, *ADDRESS_MAP_NAME(msm6255)),
	m_cursor(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void msm6255_device::device_start()
{
	// register for state saving
	save_item(NAME(m_ir));
	save_item(NAME(m_mor));
	save_item(NAME(m_pr));
	save_item(NAME(m_hnr));
	save_item(NAME(m_dvr));
	save_item(NAME(m_cpr));
	save_item(NAME(m_slr));
	save_item(NAME(m_sur));
	save_item(NAME(m_clr));
	save_item(NAME(m_cur));
	save_item(NAME(m_cursor));
	save_item(NAME(m_frame));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void msm6255_device::device_reset()
{
	m_frame = 0;
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *msm6255_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : nullptr;
}


//-------------------------------------------------
//  ir_r -
//-------------------------------------------------

READ8_MEMBER( msm6255_device::ir_r )
{
	return m_ir;
}


//-------------------------------------------------
//  ir_w -
//-------------------------------------------------

WRITE8_MEMBER( msm6255_device::ir_w )
{
	m_ir = data & 0x0f;
}


//-------------------------------------------------
//  dr_r -
//-------------------------------------------------

READ8_MEMBER( msm6255_device::dr_r )
{
	UINT8 data = 0;

	switch (m_ir)
	{
	case REGISTER_MOR:
		break; // write-only

	case REGISTER_PR:
		data = m_pr;
		break;

	case REGISTER_HNR:
		data = m_hnr;
		break;

	case REGISTER_DVR:
		break; // write-only

	case REGISTER_CPR:
		data = m_cpr;
		break;

	case REGISTER_SLR:
		data = m_slr;
		break;

	case REGISTER_SUR:
		data = m_sur;
		break;

	case REGISTER_CLR:
		data = m_clr;
		break;

	case REGISTER_CUR:
		data = m_cur;
		break;
	}

	return data;
}


//-------------------------------------------------
//  dr_w -
//-------------------------------------------------

WRITE8_MEMBER( msm6255_device::dr_w )
{
	switch (m_ir)
	{
	case REGISTER_MOR:
		m_mor = data & 0x7f;
		break;

	case REGISTER_PR:
		m_pr = data & 0xf7;
		break;

	case REGISTER_HNR:
		m_hnr = data & 0x7f;
		break;

	case REGISTER_DVR:
		m_dvr = data;
		break;

	case REGISTER_CPR:
		m_cpr = data;
		break;

	case REGISTER_SLR:
		m_slr = data;
		break;

	case REGISTER_SUR:
		m_sur = data;
		break;

	case REGISTER_CLR:
		m_clr = data;
		break;

	case REGISTER_CUR:
		m_cur = data;
		break;
	}
}


//-------------------------------------------------
//  read_byte -
//-------------------------------------------------

UINT8 msm6255_device::read_byte(UINT16 ma, UINT8 ra)
{
	offs_t offset;

	if (m_mor & MOR_GRAPHICS)
	{
		offset = ma;
	}
	else
	{
		offset = ((offs_t)ma << 4) | ra;
	}

	return space().read_byte(offset);
}


//-------------------------------------------------
//  update_cursor -
//-------------------------------------------------

void msm6255_device::update_cursor()
{
	if (m_mor & MOR_CURSOR_ON)
	{
		if (m_mor & MOR_CURSOR_BLINK)
		{
			if (m_mor & MOR_BLINK_TIME_16)
			{
				if (m_frame == 16)
				{
					m_cursor = !m_cursor;
					m_frame = 0;
				}
				else
				{
					m_frame++;
				}
			}
			else
			{
				if (m_frame == 32)
				{
					m_cursor = !m_cursor;
					m_frame = 0;
				}
				else
				{
					m_frame++;
				}
			}
		}
		else
		{
			m_cursor = 1;
		}
	}
	else
	{
		m_cursor = 0;
	}
}


//-------------------------------------------------
//  draw_scanline -
//-------------------------------------------------

void msm6255_device::draw_scanline(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, UINT16 ma, UINT8 ra)
{
	UINT8 hp = (m_pr & PR_HP_MASK) + 1;
	UINT8 hn = (m_hnr & HNR_HN_MASK) + 1;
	UINT8 cpu = m_cpr & CPR_CPU_MASK;
	UINT8 cpd = m_cpr & CPR_CPD_MASK;
	UINT16 car = (m_cur << 8) | m_clr;

	int sx, x;

	for (sx = 0; sx < hn; sx++)
	{
		UINT8 data = read_byte(ma, ra);

		if (m_cursor)
		{
			if (ma == car)
			{
				if (ra >= cpu && ra <= cpd)
				{
					data ^= 0xff;
				}
			}
		}

		for (x = 0; x < hp; x++)
		{
			bitmap.pix16(y, (sx * hp) + x) = BIT(data, 7);

			data <<= 1;
		}

		ma++;
	}
}


//-------------------------------------------------
//  update_graphics -
//-------------------------------------------------

void msm6255_device::update_graphics(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 hn = (m_hnr & HNR_HN_MASK) + 1;
	UINT8 nx = (m_dvr & DVR_DN_MASK) + 1;
	UINT16 sar = (m_sur << 8) | m_slr;

	int y;

	m_cursor = 0;
	m_frame = 0;

	for (y = 0; y < nx; y++)
	{
		// draw upper half scanline
		UINT16 ma = sar + (y * hn);
		draw_scanline(bitmap, cliprect, y, ma);

		// draw lower half scanline
		ma = sar + ((y + nx) * hn);
		draw_scanline(bitmap, cliprect, y + nx, ma);
	}
}


//-------------------------------------------------
//  update_text -
//-------------------------------------------------

void msm6255_device::update_text(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 hn = (m_hnr & HNR_HN_MASK) + 1;
	UINT8 vp = (m_pr & PR_VP_MASK) + 1;
	UINT8 nx = (m_dvr & DVR_DN_MASK) + 1;
	UINT16 sar = (m_sur << 8) | m_slr;

	int sy, y;

	update_cursor();

	for (sy = 0; sy < nx; sy++)
	{
		for (y = 0; y < vp; y++)
		{
			// draw upper half scanline
			UINT16 ma = sar + ((sy * vp) + y) * hn;
			draw_scanline(bitmap, cliprect, (sy * vp) + y, ma, y);

			// draw lower half scanline
			ma = sar + (((sy + nx) * vp) + y) * hn;
			draw_scanline(bitmap, cliprect, (sy * vp) + y, ma, y);
		}
	}
}


//-------------------------------------------------
//  update_screen - update screen
//-------------------------------------------------

UINT32 msm6255_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_mor & MOR_DISPLAY_ON)
	{
		if (m_mor & MOR_GRAPHICS)
		{
			update_graphics(bitmap, cliprect);
		}
		else
		{
			update_text(bitmap, cliprect);
		}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}

	return 0;
}
