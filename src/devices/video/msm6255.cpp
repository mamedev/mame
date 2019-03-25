// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    OKI MSM6255 Dot Matrix LCD Controller implementation

**********************************************************************/

#include "emu.h"
#include "msm6255.h"

//#define VERBOSE 1
#include "logmacro.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

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
DEFINE_DEVICE_TYPE(MSM6255, msm6255_device, "msm6255", "Oki MSM6255 LCD Controller")

// I/O map
void msm6255_device::map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(msm6255_device::dr_r), FUNC(msm6255_device::dr_w));
	map(0x01, 0x01).rw(FUNC(msm6255_device::ir_r), FUNC(msm6255_device::ir_w));
}

// default address map
void msm6255_device::msm6255(address_map &map)
{
	if (!has_configured_map(0))
		map(0x00000, 0xfffff).ram();
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  msm6255_device - constructor
//-------------------------------------------------

msm6255_device::msm6255_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MSM6255, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_space_config("videoram", ENDIANNESS_LITTLE, 8, 20, 0, address_map_constructor(FUNC(msm6255_device::msm6255), this)),
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

device_memory_interface::space_config_vector msm6255_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
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
	uint8_t data = 0;

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

uint8_t msm6255_device::read_byte(uint16_t ma, uint8_t ra)
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

void msm6255_device::draw_scanline(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, uint16_t ma, uint8_t ra)
{
	uint8_t hp = (m_pr & PR_HP_MASK) + 1;
	uint8_t hn = (m_hnr & HNR_HN_MASK) + 1;
	uint8_t cpu = m_cpr & CPR_CPU_MASK;
	uint8_t cpd = m_cpr & CPR_CPD_MASK;
	uint16_t car = (m_cur << 8) | m_clr;

	int sx, x;

	for (sx = 0; sx < hn; sx++)
	{
		uint8_t data = read_byte(ma, ra);

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
	uint8_t hn = (m_hnr & HNR_HN_MASK) + 1;
	uint8_t nx = (m_dvr & DVR_DN_MASK) + 1;
	uint16_t sar = (m_sur << 8) | m_slr;

	int y;

	m_cursor = 0;
	m_frame = 0;

	for (y = 0; y < nx; y++)
	{
		// draw upper half scanline
		uint16_t ma = sar + (y * hn);
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
	uint8_t hn = (m_hnr & HNR_HN_MASK) + 1;
	uint8_t vp = (m_pr & PR_VP_MASK) + 1;
	uint8_t nx = (m_dvr & DVR_DN_MASK) + 1;
	uint16_t sar = (m_sur << 8) | m_slr;

	int sy, y;

	update_cursor();

	for (sy = 0; sy < nx; sy++)
	{
		for (y = 0; y < vp; y++)
		{
			// draw upper half scanline
			uint16_t ma = sar + ((sy * vp) + y) * hn;
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

uint32_t msm6255_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
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
