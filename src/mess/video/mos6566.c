/***************************************************************************

    MOS 6566/6567/6569 Video Interface Chip (VIC-II) emulation

    A part of the code (cycle routine and drawing routines) is a modified version of the vic ii emulation used in
    commodore 64 emulator "frodo" by Christian Bauer

    http://frodo.cebix.net/
    The rights on the source code remain at the author.
    It may not - not even in parts - used for commercial purposes without explicit written permission by the author.
    Permission to use it for non-commercial purposes is hereby granted als long as my copyright notice remains in the program.
    You are not allowed to use the source to create and distribute a modified version of Frodo.

    Copyright the MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

/*

    TODO:

    - cleanup
    - light pen
    - remove RDY hack
    - VIC IIe
    - http://hitmen.c02.at/temp/palstuff/

*/

#include "mos6566.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


enum
{
	REGISTER_M0X = 0,
	REGISTER_M0Y,
	REGISTER_M1X,
	REGISTER_M1Y,
	REGISTER_M2X,
	REGISTER_M2Y,
	REGISTER_M3X,
	REGISTER_M3Y,
	REGISTER_M4X,
	REGISTER_M4Y,
	REGISTER_M5X,
	REGISTER_M5Y,
	REGISTER_M6X,
	REGISTER_M6Y,
	REGISTER_M7X,
	REGISTER_M7Y,
	REGISTER_MX_MSB,
	REGISTER_CR1,
	REGISTER_RASTER,
	REGISTER_LPX,
	REGISTER_LPY,
	REGISTER_ME,
	REGISTER_CR2,
	REGISTER_MYE,
	REGISTER_MP,
	REGISTER_IRQ,
	REGISTER_IE,
	REGISTER_MDP,
	REGISTER_MMC,
	REGISTER_MXE,
	REGISTER_MM,
	REGISTER_MD,
	REGISTER_EC,
	REGISTER_B0C,
	REGISTER_B1C,
	REGISTER_B2C,
	REGISTER_B3C,
	REGISTER_MM0,
	REGISTER_MM1,
	REGISTER_M0C,
	REGISTER_M1C,
	REGISTER_M2C,
	REGISTER_M3C,
	REGISTER_M4C,
	REGISTER_M5C,
	REGISTER_M6C,
	REGISTER_M7C
};


static const UINT8 PALETTE[] =
{
// black, white, red, cyan
// purple, green, blue, yellow
// orange, brown, light red, dark gray,
// medium gray, light green, light blue, light gray
// taken from the vice emulator
	0x00, 0x00, 0x00,  0xfd, 0xfe, 0xfc,  0xbe, 0x1a, 0x24,  0x30, 0xe6, 0xc6,
	0xb4, 0x1a, 0xe2,  0x1f, 0xd2, 0x1e,  0x21, 0x1b, 0xae,  0xdf, 0xf6, 0x0a,
	0xb8, 0x41, 0x04,  0x6a, 0x33, 0x04,  0xfe, 0x4a, 0x57,  0x42, 0x45, 0x40,
	0x70, 0x74, 0x6f,  0x59, 0xfe, 0x59,  0x5f, 0x53, 0xfe,  0xa4, 0xa7, 0xa2
};


#define VERBOSE_LEVEL 0
#define DBG_LOG(N,M,A) \
	do { \
		if(VERBOSE_LEVEL >= N) \
		{ \
			if( M ) \
				logerror("%11.6f: %-24s", machine().time().as_double(), (char*) M ); \
			logerror A; \
		} \
	} while (0)

#define IS_PAL					((m_variant == TYPE_6569) || (m_variant == TYPE_6572) || (m_variant == TYPE_6573) || (m_variant == TYPE_8565) || (m_variant == TYPE_8565) || (m_variant == TYPE_8569))
#define IS_VICIIE				((m_variant == TYPE_8564) || (m_variant == TYPE_8566) || (m_variant == TYPE_8569))

#define ROW25_YSTART      0x33
#define ROW25_YSTOP       0xfb
#define ROW24_YSTART      0x37
#define ROW24_YSTOP       0xf7

#define RASTERLINE_2_C64(a)		(a)
#define C64_2_RASTERLINE(a)		(a)
#define XPOS				(VIC2_STARTVISIBLECOLUMNS + (VIC2_VISIBLECOLUMNS - VIC2_HSIZE) / 2)
#define YPOS				(VIC2_STARTVISIBLELINES /* + (VIC2_VISIBLELINES - VIC2_VSIZE) / 2 */)
#define FIRSTCOLUMN			50

/* 2008-05 FP: lightpen code needs to read input port from c64.c and cbmb.c */

#define LIGHTPEN_BUTTON		(m_in_lightpen_button_func(0))
#define LIGHTPEN_X_VALUE	(m_in_lightpen_x_func(0))
#define LIGHTPEN_Y_VALUE	(m_in_lightpen_y_func(0))

/* lightpen delivers values from internal counters; they do not start with the visual area or frame area */
#define VIC2_MAME_XPOS			0
#define VIC2_MAME_YPOS			0
#define VIC6567_X_BEGIN			38
#define VIC6567_Y_BEGIN			-6			   /* first 6 lines after retrace not for lightpen! */
#define VIC6569_X_BEGIN			38
#define VIC6569_Y_BEGIN			-6
#define VIC2_X_BEGIN			(IS_PAL ? VIC6569_X_BEGIN : VIC6567_X_BEGIN)
#define VIC2_Y_BEGIN			(IS_PAL ? VIC6569_Y_BEGIN : VIC6567_Y_BEGIN)
#define VIC2_X_VALUE			((LIGHTPEN_X_VALUE / 1.3) + 12)
#define VIC2_Y_VALUE			((LIGHTPEN_Y_VALUE      ) + 10)

#define VIC2E_K0_LEVEL			(m_reg[0x2f] & 0x01)
#define VIC2E_K1_LEVEL			(m_reg[0x2f] & 0x02)
#define VIC2E_K2_LEVEL			(m_reg[0x2f] & 0x04)


/* sprites 0 .. 7 */
#define SPRITEON(nr)			(m_reg[0x15] & (1 << nr))
#define SPRITE_Y_EXPAND(nr)		(m_reg[0x17] & (1 << nr))
#define SPRITE_Y_SIZE(nr)		(SPRITE_Y_EXPAND(nr) ? 2 * 21 : 21)
#define SPRITE_X_EXPAND(nr)		(m_reg[0x1d] & (1 << nr))
#define SPRITE_X_SIZE(nr)		(SPRITE_X_EXPAND(nr) ? 2 * 24 : 24)
#define SPRITE_X_POS(nr)		(m_reg[(nr) * 2] | (m_reg[0x10] & (1 << (nr)) ? 0x100 : 0))
#define SPRITE_Y_POS(nr)		(m_reg[1 + 2 * (nr)])
#define SPRITE_MULTICOLOR(nr)	(m_reg[0x1c] & (1 << nr))
#define SPRITE_PRIORITY(nr)		(m_reg[0x1b] & (1 << nr))
#define SPRITE_MULTICOLOR1		(m_reg[0x25] & 0x0f)
#define SPRITE_MULTICOLOR2		(m_reg[0x26] & 0x0f)
#define SPRITE_COLOR(nr)		(m_reg[0x27+nr] & 0x0f)
#define SPRITE_ADDR(nr)			(m_videoaddr | 0x3f8 | nr)
#define SPRITE_COLL				(m_reg[0x1e])
#define SPRITE_BG_COLL			(m_reg[0x1f])

#define GFXMODE					((m_reg[0x11] & 0x60) | (m_reg[0x16] & 0x10)) >> 4
#define SCREENON				(m_reg[0x11] & 0x10)
#define VERTICALPOS				(m_reg[0x11] & 0x07)
#define HORIZONTALPOS			(m_reg[0x16] & 0x07)
#define ECMON					(m_reg[0x11] & 0x40)
#define HIRESON					(m_reg[0x11] & 0x20)
#define COLUMNS40				(m_reg[0x16] & 0x08)		   /* else 38 Columns */

#define VIDEOADDR				((m_reg[0x18] & 0xf0) << (10 - 4))
#define CHARGENADDR				((m_reg[0x18] & 0x0e) << 10)
#define BITMAPADDR				((data & 0x08) << 10)

#define RASTERLINE				(((m_reg[0x11] & 0x80) << 1) | m_reg[0x12])

#define FRAMECOLOR				(m_reg[0x20] & 0x0f)
#define BACKGROUNDCOLOR			(m_reg[0x21] & 0x0f)
#define MULTICOLOR1				(m_reg[0x22] & 0x0f)
#define MULTICOLOR2				(m_reg[0x23] & 0x0f)
#define FOREGROUNDCOLOR			(m_reg[0x24] & 0x0f)

#define VIC2_LINES				(IS_PAL ? VIC6569_LINES : VIC6567_LINES)
#define VIC2_FIRST_DMA_LINE		(IS_PAL ? VIC6569_FIRST_DMA_LINE : VIC6567_FIRST_DMA_LINE)
#define VIC2_LAST_DMA_LINE		(IS_PAL ? VIC6569_LAST_DMA_LINE : VIC6567_LAST_DMA_LINE)
#define VIC2_FIRST_DISP_LINE	(IS_PAL ? VIC6569_FIRST_DISP_LINE : VIC6567_FIRST_DISP_LINE)
#define VIC2_LAST_DISP_LINE		(IS_PAL ? VIC6569_LAST_DISP_LINE : VIC6567_LAST_DISP_LINE)
#define VIC2_RASTER_2_EMU(a)	(IS_PAL ? VIC6569_RASTER_2_EMU(a) : VIC6567_RASTER_2_EMU(a))
#define VIC2_FIRSTCOLUMN		(IS_PAL ? VIC6569_FIRSTCOLUMN : VIC6567_FIRSTCOLUMN)
#define VIC2_X_2_EMU(a)			(IS_PAL ? VIC6569_X_2_EMU(a) : VIC6567_X_2_EMU(a))




//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type MOS6566 = &device_creator<mos6566_device>;
const device_type MOS6567 = &device_creator<mos6567_device>;
const device_type MOS8562 = &device_creator<mos8562_device>;
const device_type MOS6569 = &device_creator<mos6569_device>;
const device_type MOS8565 = &device_creator<mos8565_device>;


// default address maps
static ADDRESS_MAP_START( mos6566_videoram_map, AS_0, 8, mos6566_device )
	AM_RANGE(0x0000, 0x3fff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mos6566_colorram_map, AS_1, 8, mos6566_device )
	AM_RANGE(0x000, 0x3ff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *mos6566_device::memory_space_config(address_spacenum spacenum) const
{
	switch (spacenum)
	{
		case AS_0: return &m_videoram_space_config;
		case AS_1: return &m_colorram_space_config;
		default: return NULL;
	}
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

inline void mos6566_device::vic2_set_interrupt( int mask )
{
	if (((m_reg[0x19] ^ mask) & m_reg[0x1a] & 0xf))
	{
		if (!(m_reg[0x19] & 0x80))
		{
			DBG_LOG(2, "vic2", ("irq start %.2x\n", mask));
			m_reg[0x19] |= 0x80;
			m_out_irq_func(1);
		}
	}
	m_reg[0x19] |= mask;
}

inline void mos6566_device::vic2_clear_interrupt( int mask )
{
	m_reg[0x19] &= ~mask;
	if ((m_reg[0x19] & 0x80) && !(m_reg[0x19] & m_reg[0x1a] & 0xf))
	{
		DBG_LOG(2, "vic2", ("irq end %.2x\n", mask));
		m_reg[0x19] &= ~0x80;
		m_out_irq_func(0);
	}
}

inline UINT8 mos6566_device::read_videoram(offs_t offset)
{
	m_last_data = space(AS_0)->read_byte(offset & 0x3fff);
	
	return m_last_data;
}

inline UINT8 mos6566_device::read_colorram(offs_t offset)
{
	return space(AS_1)->read_byte(offset & 0x3ff);
}

// Idle access
inline void mos6566_device::vic2_idle_access()
{
	read_videoram(0x3fff);
}

// Fetch sprite data pointer
inline void mos6566_device::vic2_spr_ptr_access( int num )
{
	m_spr_ptr[num] = read_videoram(SPRITE_ADDR(num)) << 6;
}

// Fetch sprite data, increment data counter
inline void mos6566_device::vic2_spr_data_access( int num, int bytenum )
{
	if (m_spr_dma_on & (1 << num))
	{
		m_spr_data[num][bytenum] = read_videoram((m_mc[num] & 0x3f) | m_spr_ptr[num]);
		m_mc[num]++;
	}
	else
		if (bytenum == 1)
			vic2_idle_access();
}

// Turn on display if Bad Line
inline void mos6566_device::vic2_display_if_bad_line()
{
	if (m_is_bad_line)
		m_display_state = 1;
}

// Suspend CPU
inline void mos6566_device::vic2_suspend_cpu()
{
	if (m_device_suspended == 0)
	{
		m_first_ba_cycle = m_cycles_counter;
		if (m_in_rdy_workaround_func(0) != 7 )
		{
//          device_suspend(machine.firstcpu, SUSPEND_REASON_SPIN, 0);
		}
		m_device_suspended = 1;
	}
}

// Resume CPU
inline void mos6566_device::vic2_resume_cpu()
{
	if (m_device_suspended == 1)
	{
	//  device_resume(machine.firstcpu, SUSPEND_REASON_SPIN);
		m_device_suspended = 0;
	}
}

// Refresh access
inline void mos6566_device::vic2_refresh_access()
{
	read_videoram(0x3f00 | m_ref_cnt--);
}


inline void mos6566_device::vic2_fetch_if_bad_line()
{
	if (m_is_bad_line)
		m_display_state = 1;
}


// Turn on display and matrix access and reset RC if Bad Line
inline void mos6566_device::vic2_rc_if_bad_line()
{
	if (m_is_bad_line)
	{
		m_display_state = 1;
		m_rc = 0;
	}
}

// Sample border color and increment m_graphic_x
inline void mos6566_device::vic2_sample_border()
{
	if (m_draw_this_line)
	{
		if (m_border_on)
			m_border_color_sample[m_cycle - 13] = FRAMECOLOR;
		m_graphic_x += 8;
	}
}


// Turn on sprite DMA if necessary
inline void mos6566_device::vic2_check_sprite_dma()
{
	int i;
	UINT8 mask = 1;

	for (i = 0; i < 8; i++, mask <<= 1)
		if (SPRITEON(i) && ((m_rasterline & 0xff) == SPRITE_Y_POS(i)))
		{
			m_spr_dma_on |= mask;
			m_mc_base[i] = 0;
			if (SPRITE_Y_EXPAND(i))
				m_spr_exp_y &= ~mask;
		}
}

// Video matrix access
inline void mos6566_device::vic2_matrix_access()
{
//  if (m_device_suspended == 1)
	{
		if (m_cycles_counter < m_first_ba_cycle)
			m_matrix_line[m_ml_index] = m_color_line[m_ml_index] = 0xff;
		else
		{
			UINT16 adr = (m_vc & 0x03ff) | VIDEOADDR;
			m_matrix_line[m_ml_index] = read_videoram(adr);
			m_color_line[m_ml_index] = read_colorram((adr & 0x03ff));
		}
	}
}

// Graphics data access
inline void mos6566_device::vic2_graphics_access()
{
	if (m_display_state == 1)
	{
		UINT16 adr;
		if (HIRESON)
			adr = ((m_vc & 0x03ff) << 3) | m_bitmapaddr | m_rc;
		else
			adr = (m_matrix_line[m_ml_index] << 3) | m_chargenaddr | m_rc;
		if (ECMON)
			adr &= 0xf9ff;
		m_gfx_data = read_videoram(adr);
		m_char_data = m_matrix_line[m_ml_index];
		m_color_data = m_color_line[m_ml_index];
		m_ml_index++;
		m_vc++;
	}
	else
	{
		m_gfx_data = read_videoram((ECMON ? 0x39ff : 0x3fff));
		m_char_data = 0;
	}
}

inline void mos6566_device::vic2_draw_background()
{
	if (m_draw_this_line)
	{
		UINT8 c;

		switch (GFXMODE)
		{
			case 0:
			case 1:
			case 3:
				c = m_colors[0];
				break;
			case 2:
				c = m_last_char_data & 0x0f;
				break;
			case 4:
				if (m_last_char_data & 0x80)
					if (m_last_char_data & 0x40)
						c = m_colors[3];
					else
						c = m_colors[2];
				else
					if (m_last_char_data & 0x40)
						c = m_colors[1];
					else
						c = m_colors[0];
				break;
			default:
				c = 0;
				break;
		}
		m_bitmap->plot_box(m_graphic_x, VIC2_RASTER_2_EMU(m_rasterline), 8, 1, c);
	}
}

inline void mos6566_device::vic2_draw_mono( UINT16 p, UINT8 c0, UINT8 c1 )
{
	UINT8 c[2];
	UINT8 data = m_gfx_data;

	c[0] = c0;
	c[1] = c1;

	m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 7) = c[data & 1];
	m_fore_coll_buf[p + 7] = data & 1; data >>= 1;
	m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 6) = c[data & 1];
	m_fore_coll_buf[p + 6] = data & 1; data >>= 1;
	m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 5) = c[data & 1];
	m_fore_coll_buf[p + 5] = data & 1; data >>= 1;
	m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 4) = c[data & 1];
	m_fore_coll_buf[p + 4] = data & 1; data >>= 1;
	m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 3) = c[data & 1];
	m_fore_coll_buf[p + 3] = data & 1; data >>= 1;
	m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 2) = c[data & 1];
	m_fore_coll_buf[p + 2] = data & 1; data >>= 1;
	m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 1) = c[data & 1];
	m_fore_coll_buf[p + 1] = data & 1; data >>= 1;
	m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 0) = c[data];
	m_fore_coll_buf[p + 0] = data & 1;
}

inline void mos6566_device::vic2_draw_multi( UINT16 p, UINT8 c0, UINT8 c1, UINT8 c2, UINT8 c3 )
{
	UINT8 c[4];
	UINT8 data = m_gfx_data;

	c[0] = c0;
	c[1] = c1;
	c[2] = c2;
	c[3] = c3;

	m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 7) = c[data & 3];
	m_fore_coll_buf[p + 7] = data & 2;
	m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 6) = c[data & 3];
	m_fore_coll_buf[p + 6] = data & 2; data >>= 2;
	m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 5) = c[data & 3];
	m_fore_coll_buf[p + 5] = data & 2;
	m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 4) = c[data & 3];
	m_fore_coll_buf[p + 4] = data & 2; data >>= 2;
	m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 3) = c[data & 3];
	m_fore_coll_buf[p + 3] = data & 2;
	m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 2) = c[data & 3];
	m_fore_coll_buf[p + 2] = data & 2; data >>= 2;
	m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 1) = c[data];
	m_fore_coll_buf[p + 1] = data & 2;
	m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 0) = c[data];
	m_fore_coll_buf[p + 0] = data & 2;
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  mos6566_device - constructor
//-------------------------------------------------

mos6566_device::mos6566_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, MOS6566, "MOS6566", tag, owner, clock),
	  device_memory_interface(mconfig, *this),
	  device_execute_interface(mconfig, *this),
	  m_icount(0),
	  m_variant(TYPE_6566),
	  m_videoram_space_config("videoram", ENDIANNESS_LITTLE, 8, 14, 0, NULL, *ADDRESS_MAP_NAME(mos6566_videoram_map)),
	  m_colorram_space_config("colorram", ENDIANNESS_LITTLE, 8, 10, 0, NULL, *ADDRESS_MAP_NAME(mos6566_colorram_map))
{
}

mos6566_device::mos6566_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, type, name, tag, owner, clock),
	  device_memory_interface(mconfig, *this),
	  device_execute_interface(mconfig, *this),
	  m_icount(0),
	  m_videoram_space_config("videoram", ENDIANNESS_LITTLE, 8, 14, 0, NULL, *ADDRESS_MAP_NAME(mos6566_videoram_map)),
	  m_colorram_space_config("colorram", ENDIANNESS_LITTLE, 8, 10, 0, NULL, *ADDRESS_MAP_NAME(mos6566_colorram_map))
{
}

mos6567_device::mos6567_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:mos6566_device(mconfig, MOS6567, "MOS6567", tag, owner, clock) { m_variant = TYPE_6567; }

mos6567_device::mos6567_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	:mos6566_device(mconfig, type, name, tag, owner, clock) { }

mos8562_device::mos8562_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:mos6567_device(mconfig, MOS8562, "MOS8562", tag, owner, clock) { m_variant = TYPE_8562; }

mos6569_device::mos6569_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:mos6566_device(mconfig, MOS6566, "MOS6569", tag, owner, clock) { m_variant = TYPE_6569; }

mos6569_device::mos6569_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	:mos6566_device(mconfig, type, name, tag, owner, clock) { }

mos8565_device::mos8565_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	:mos6569_device(mconfig, MOS8565, "MOS8565", tag, owner, clock) { m_variant = TYPE_8565; }


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void mos6566_device::device_config_complete()
{
	// inherit a copy of the static data
	const mos6566_interface *intf = reinterpret_cast<const mos6566_interface *>(static_config());
	if (intf != NULL)
		*static_cast<mos6566_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		// TODO
    }
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mos6566_device::device_start()
{
	// set our instruction counter
	m_icountptr = &m_icount;

	// resolve callbacks
	m_out_irq_func.resolve(m_out_irq_cb, *this);
	m_out_rdy_func.resolve(m_out_rdy_cb, *this);
	m_in_lightpen_x_func.resolve(m_in_x_cb, *this);
	m_in_lightpen_y_func.resolve(m_in_y_cb, *this);
	m_in_lightpen_button_func.resolve(m_in_button_cb, *this);
	m_in_rdy_workaround_func.resolve(m_in_rdy_cb, *this);

	m_cpu = machine().device<cpu_device>(m_cpu_tag);

	m_screen = machine().device<screen_device>(m_screen_tag);
	int width = m_screen->width();
	int height = m_screen->height();

	m_bitmap = auto_bitmap_ind16_alloc(machine(), width, height);

	// initialize palette
	int i;

	for (i = 0; i < 16; i++)
	{
		palette_set_color_rgb(machine(), i, PALETTE[i * 3], PALETTE[i * 3 + 1], PALETTE[i * 3 + 2]);
	}

	for (i = 0; i < 256; i++)
	{
		m_expandx[i] = 0;
		if (i & 1)
			m_expandx[i] |= 3;
		if (i & 2)
			m_expandx[i] |= 0xc;
		if (i & 4)
			m_expandx[i] |= 0x30;
		if (i & 8)
			m_expandx[i] |= 0xc0;
		if (i & 0x10)
			m_expandx[i] |= 0x300;
		if (i & 0x20)
			m_expandx[i] |= 0xc00;
		if (i & 0x40)
			m_expandx[i] |= 0x3000;
		if (i & 0x80)
			m_expandx[i] |= 0xc000;
	}

	for (i = 0; i < 256; i++)
	{
		m_expandx_multi[i] = 0;
		if (i & 1)
			m_expandx_multi[i] |= 5;
		if (i & 2)
			m_expandx_multi[i] |= 0xa;
		if (i & 4)
			m_expandx_multi[i] |= 0x50;
		if (i & 8)
			m_expandx_multi[i] |= 0xa0;
		if (i & 0x10)
			m_expandx_multi[i] |= 0x500;
		if (i & 0x20)
			m_expandx_multi[i] |= 0xa00;
		if (i & 0x40)
			m_expandx_multi[i] |= 0x5000;
		if (i & 0x80)
			m_expandx_multi[i] |= 0xa000;
	}

	// state saving
	save_item(NAME(m_reg));

	save_item(NAME(m_on));

	//save_item(NAME(m_bitmap));

	save_item(NAME(m_chargenaddr));
	save_item(NAME(m_videoaddr));
	save_item(NAME(m_bitmapaddr));

	save_item(NAME(m_colors));
	save_item(NAME(m_spritemulti));

	save_item(NAME(m_rasterline));
	save_item(NAME(m_cycles_counter));
	save_item(NAME(m_cycle));
	save_item(NAME(m_raster_x));
	save_item(NAME(m_graphic_x));
	save_item(NAME(m_last_data));

	save_item(NAME(m_dy_start));
	save_item(NAME(m_dy_stop));

	save_item(NAME(m_draw_this_line));
	save_item(NAME(m_is_bad_line));
	save_item(NAME(m_bad_lines_enabled));
	save_item(NAME(m_display_state));
	save_item(NAME(m_char_data));
	save_item(NAME(m_gfx_data));
	save_item(NAME(m_color_data));
	save_item(NAME(m_last_char_data));
	save_item(NAME(m_matrix_line));
	save_item(NAME(m_color_line));
	save_item(NAME(m_vblanking));
	save_item(NAME(m_ml_index));
	save_item(NAME(m_rc));
	save_item(NAME(m_vc));
	save_item(NAME(m_vc_base));
	save_item(NAME(m_ref_cnt));

	save_item(NAME(m_spr_coll_buf));
	save_item(NAME(m_fore_coll_buf));
	save_item(NAME(m_spr_exp_y));
	save_item(NAME(m_spr_dma_on));
	save_item(NAME(m_spr_draw));
	save_item(NAME(m_spr_disp_on));
	save_item(NAME(m_spr_ptr));
	save_item(NAME(m_mc_base));
	save_item(NAME(m_mc));

	for (i = 0; i < 8; i++)
	{
		save_item(NAME(m_spr_data[i]), i);
		save_item(NAME(m_spr_draw_data[i]), i);
	}

	save_item(NAME(m_border_on));
	save_item(NAME(m_ud_border_on));
	save_item(NAME(m_border_on_sample));
	save_item(NAME(m_border_color_sample));

	save_item(NAME(m_first_ba_cycle));
	save_item(NAME(m_device_suspended));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mos6566_device::device_reset()
{
	int i, j;

	memset(m_reg, 0, ARRAY_LENGTH(m_reg));

	for (i = 0; i < ARRAY_LENGTH(m_mc); i++)
		m_mc[i] = 63;

	// from 0 to 311 (0 first, PAL) or from 0 to 261 (? first, NTSC 6567R56A) or from 0 to 262 (? first, NTSC 6567R8)
	m_rasterline = 0; // VIC2_LINES - 1;

	m_cycles_counter = -1;
	m_cycle = 63;

	m_on = 1;

	m_dy_start = ROW24_YSTART;
	m_dy_stop = ROW24_YSTOP;

	m_draw_this_line = 0;
	m_is_bad_line = 0;
	m_bad_lines_enabled = 0;
	m_display_state = 0;
	m_char_data = 0;
	m_gfx_data = 0;
	m_color_data = 0;
	m_last_char_data = 0;
	m_vblanking = 0;
	m_ml_index = 0;
	m_rc = 0;
	m_vc = 0;
	m_vc_base = 0;
	m_ref_cnt = 0;

	m_spr_exp_y = 0;
	m_spr_dma_on = 0;
	m_spr_draw = 0;
	m_spr_disp_on = 0;


	m_border_on = 0;
	m_ud_border_on = 0;

	m_first_ba_cycle = 0;
	m_device_suspended = 0;

	memset(m_matrix_line, 0, ARRAY_LENGTH(m_matrix_line));
	memset(m_color_line, 0, ARRAY_LENGTH(m_color_line));

	memset(m_spr_coll_buf, 0, ARRAY_LENGTH(m_spr_coll_buf));
	memset(m_fore_coll_buf, 0, ARRAY_LENGTH(m_fore_coll_buf));
	memset(m_border_on_sample, 0, ARRAY_LENGTH(m_border_on_sample));
	memset(m_border_color_sample, 0, ARRAY_LENGTH(m_border_color_sample));

	for (i = 0; i < 8; i++)
	{
		m_spr_ptr[i] = 0;
		m_mc_base[i] = 0;
		m_mc[i] = 0;
		for (j = 0; j < 4; j++)
		{
			m_spr_draw_data[i][j] = 0;
			m_spr_data[i][j] = 0;
		}
	}
}


//-------------------------------------------------
//  execute_run -
//-------------------------------------------------

void mos6566_device::execute_run()
{
	do
	{
		int i;
		UINT8 mask;
		m_cycles_counter++;

		switch (m_cycle)
		{

		// Sprite 3, raster counter, raster IRQ, bad line
		case 1:
			if (m_rasterline == (VIC2_LINES - 1))
			{
				m_vblanking = 1;

	//          if (LIGHTPEN_BUTTON)
				{
					m_reg[0x13] = VIC2_X_VALUE;
					m_reg[0x14] = VIC2_Y_VALUE;
				}
				vic2_set_interrupt(8);
			}
			else
			{
				m_rasterline++;

				if (m_rasterline == VIC2_FIRST_DMA_LINE)
					m_bad_lines_enabled = SCREENON;

				m_is_bad_line = ((m_rasterline >= VIC2_FIRST_DMA_LINE) && (m_rasterline <= VIC2_LAST_DMA_LINE) &&
							((m_rasterline & 0x07) == VERTICALPOS) && m_bad_lines_enabled);

				m_draw_this_line = ((VIC2_RASTER_2_EMU(m_rasterline) >= VIC2_RASTER_2_EMU(VIC2_FIRST_DISP_LINE)) &&
							(VIC2_RASTER_2_EMU(m_rasterline ) <= VIC2_RASTER_2_EMU(VIC2_LAST_DISP_LINE)));
			}

			m_border_on_sample[0] = m_border_on;
			vic2_spr_ptr_access(3);
			vic2_spr_data_access(3, 0);
			vic2_display_if_bad_line();

			if (m_spr_dma_on & 0x08)
				vic2_suspend_cpu();
			else
				vic2_resume_cpu();

			m_cycle++;
			break;

		// Sprite 3
		case 2:
			if (m_vblanking)
			{
				// Vertical blank, reset counters
				m_rasterline = m_vc_base = 0;
				m_ref_cnt = 0xff;
				m_vblanking = 0;

				// Trigger raster IRQ if IRQ in line 0
				if (RASTERLINE == 0)
				{
					vic2_set_interrupt(1);
				}
			}

			if (m_rasterline == RASTERLINE)
			{
				vic2_set_interrupt(1);
			}

			m_graphic_x = VIC2_X_2_EMU(0);

			vic2_spr_data_access(3, 1);
			vic2_spr_data_access(3, 2);
			vic2_display_if_bad_line();

			m_cycle++;
			break;

		// Sprite 4
		case 3:
			vic2_spr_ptr_access(4);
			vic2_spr_data_access(4, 0);
			vic2_display_if_bad_line();

			if (m_spr_dma_on & 0x10)
				vic2_suspend_cpu();
			else
				vic2_resume_cpu();

			m_cycle++;
			break;

		// Sprite 4
		case 4:
			vic2_spr_data_access(4, 1);
			vic2_spr_data_access(4, 2);
			vic2_display_if_bad_line();

			m_cycle++;
			break;

		// Sprite 5
		case 5:
			vic2_spr_ptr_access(5);
			vic2_spr_data_access(5, 0);
			vic2_display_if_bad_line();

			if (m_spr_dma_on & 0x20)
				vic2_suspend_cpu();
			else
				vic2_resume_cpu();

			m_cycle++;
			break;

		// Sprite 5
		case 6:
			vic2_spr_data_access(5, 1);
			vic2_spr_data_access(5, 2);
			vic2_display_if_bad_line();

			m_cycle++;
			break;

		// Sprite 6
		case 7:
			vic2_spr_ptr_access(6);
			vic2_spr_data_access(6, 0);
			vic2_display_if_bad_line();

			if (m_spr_dma_on & 0x40)
				vic2_suspend_cpu();
			else
				vic2_resume_cpu();

			m_cycle++;
			break;

		// Sprite 6
		case 8:
			vic2_spr_data_access(6, 1);
			vic2_spr_data_access(6, 2);
			vic2_display_if_bad_line();

			m_cycle++;
			break;

		// Sprite 7
		case 9:
			vic2_spr_ptr_access(7);
			vic2_spr_data_access(7, 0);
			vic2_display_if_bad_line();

			if (m_spr_dma_on & 0x80)
				vic2_suspend_cpu();
			else
				vic2_resume_cpu();

			m_cycle++;
			break;

		// Sprite 7
		case 10:
			vic2_spr_data_access(7, 1);
			vic2_spr_data_access(7, 2);
			vic2_display_if_bad_line();

			m_cycle++;
			break;

		// Refresh
		case 11:
			vic2_refresh_access();
			vic2_display_if_bad_line();

			vic2_resume_cpu();

			m_cycle++;
			break;

		// Refresh, fetch if bad line
		case 12:
			vic2_refresh_access();
			vic2_fetch_if_bad_line();

			m_cycle++;
			break;

		// Refresh, fetch if bad line, raster_x
		case 13:
			vic2_draw_background();
			vic2_sample_border();
			vic2_refresh_access();
			vic2_fetch_if_bad_line();

			m_raster_x = 0xfffc;

			m_cycle++;
			break;

		// Refresh, fetch if bad line, RC, VC
		case 14:
			vic2_draw_background();
			vic2_sample_border();
			vic2_refresh_access();
			vic2_rc_if_bad_line();

			m_vc = m_vc_base;

			m_cycle++;
			break;

		// Refresh, fetch if bad line, sprite y expansion
		case 15:
			vic2_draw_background();
			vic2_sample_border();
			vic2_refresh_access();
			vic2_fetch_if_bad_line();

			for (i = 0; i < 8; i++)
				if (m_spr_exp_y & (1 << i))
					m_mc_base[i] += 2;

			if (m_is_bad_line)
				vic2_suspend_cpu();

			m_ml_index = 0;
			vic2_matrix_access();

			m_cycle++;
			break;

		// Graphics, sprite y expansion, sprite DMA
		case 16:
			vic2_draw_background();
			vic2_sample_border();
			vic2_graphics_access();
			vic2_fetch_if_bad_line();

			mask = 1;
			for (i = 0; i < 8; i++, mask <<= 1)
			{
				if (m_spr_exp_y & mask)
					m_mc_base[i]++;
				if ((m_mc_base[i] & 0x3f) == 0x3f)
					m_spr_dma_on &= ~mask;
			}

			vic2_matrix_access();

			m_cycle++;
			break;

		// Graphics, check border
		case 17:
			if (COLUMNS40)
			{
				if (m_rasterline == m_dy_stop)
					m_ud_border_on = 1;
				else
				{
					if (SCREENON)
					{
						if (m_rasterline == m_dy_start)
							m_border_on = m_ud_border_on = 0;
						else
							if (m_ud_border_on == 0)
								m_border_on = 0;
					}
					else
						if (m_ud_border_on == 0)
							m_border_on = 0;
				}
			}

			// Second sample of border state
			m_border_on_sample[1] = m_border_on;

			vic2_draw_background();
			vic2_draw_graphics();
			vic2_sample_border();
			vic2_graphics_access();
			vic2_fetch_if_bad_line();
			vic2_matrix_access();

			m_cycle++;
			break;

		// Check border
		case 18:
			if (!COLUMNS40)
			{
				if (m_rasterline == m_dy_stop)
					m_ud_border_on = 1;
				else
				{
					if (SCREENON)
					{
						if (m_rasterline == m_dy_start)
							m_border_on = m_ud_border_on = 0;
						else
							if (m_ud_border_on == 0)
								m_border_on = 0;
					}
					else
						if (m_ud_border_on == 0)
							m_border_on = 0;
				}
			}

			// Third sample of border state
			m_border_on_sample[2] = m_border_on;

		// Graphics

		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 40:
		case 41:
		case 42:
		case 43:
		case 44:
		case 45:
		case 46:
		case 47:
		case 48:
		case 49:
		case 50:
		case 51:
		case 52:
		case 53:
		case 54:
			vic2_draw_graphics();
			vic2_sample_border();
			vic2_graphics_access();
			vic2_fetch_if_bad_line();
			vic2_matrix_access();
			m_last_char_data = m_char_data;

			m_cycle++;
			break;

		// Graphics, sprite y expansion, sprite DMA
		case 55:
			vic2_draw_graphics();
			vic2_sample_border();
			vic2_graphics_access();
			vic2_display_if_bad_line();

			// sprite y expansion
			mask = 1;
			for (i = 0; i < 8; i++, mask <<= 1)
				if (SPRITE_Y_EXPAND (i))
					m_spr_exp_y ^= mask;

			vic2_check_sprite_dma();

			vic2_resume_cpu();

			m_cycle++;
			break;

		// Check border, sprite DMA
		case 56:
			if (!COLUMNS40)
				m_border_on = 1;

			// Fourth sample of border state
			m_border_on_sample[3] = m_border_on;

			vic2_draw_graphics();
			vic2_sample_border();
			vic2_idle_access();
			vic2_display_if_bad_line();
			vic2_check_sprite_dma();

			m_cycle++;
			break;

		// Check border, sprites
		case 57:
			if (COLUMNS40)
				m_border_on = 1;

			// Fifth sample of border state
			m_border_on_sample[4] = m_border_on;

			// Sample spr_disp_on and spr_data for sprite drawing
			m_spr_draw = m_spr_disp_on;
			if (m_spr_draw)
				memcpy(m_spr_draw_data, m_spr_data, 8 * 4);

			mask = 1;
			for (i = 0; i < 8; i++, mask <<= 1)
				if ((m_spr_disp_on & mask) && !(m_spr_dma_on & mask))
					m_spr_disp_on &= ~mask;

			vic2_draw_background();
			vic2_sample_border();
			vic2_idle_access();
			vic2_display_if_bad_line();

			m_cycle++;
			break;

		// for NTSC 6567R8
		case 58:
			vic2_draw_background();
			vic2_sample_border();
			vic2_idle_access();
			vic2_display_if_bad_line();

			m_cycle++;
			break;

		// for NTSC 6567R8
		case 59:
			vic2_draw_background();
			vic2_sample_border();
			vic2_idle_access();
			vic2_display_if_bad_line();

			m_cycle++;
			break;

		// Sprite 0, sprite DMA, MC, RC
		case 60:
			vic2_draw_background();
			vic2_sample_border();

			mask = 1;
			for (i = 0; i < 8; i++, mask <<= 1)
			{
				m_mc[i] = m_mc_base[i];
				if ((m_spr_dma_on & mask) && ((m_rasterline & 0xff) == SPRITE_Y_POS(i)))
					m_spr_disp_on |= mask;
			}

			vic2_spr_ptr_access(0);
			vic2_spr_data_access(0, 0);

			if (m_rc == 7)
			{
				m_vc_base = m_vc;
				m_display_state = 0;
			}

			if (m_is_bad_line || m_display_state)
			{
				m_display_state = 1;
				m_rc = (m_rc + 1) & 7;
			}

			if (m_spr_dma_on & 0x01)
				vic2_suspend_cpu();
			else
				vic2_resume_cpu();

			m_cycle++;
			break;

		// Sprite 0
		case 61:
			vic2_draw_background();
			vic2_sample_border();
			vic2_spr_data_access(0, 1);
			vic2_spr_data_access(0, 2);
			vic2_display_if_bad_line();

			m_cycle++;
			break;

		// Sprite 1, draw
		case 62:
			vic2_draw_background();
			vic2_sample_border();

			if (m_draw_this_line)
			{
				vic2_draw_sprites();

				if (m_border_on_sample[0])
					for (i = 0; i < 4; i++)
						m_bitmap->plot_box(VIC2_X_2_EMU(i * 8), VIC2_RASTER_2_EMU(m_rasterline), 8, 1, m_border_color_sample[i]);

				if (m_border_on_sample[1])
					m_bitmap->plot_box(VIC2_X_2_EMU(4 * 8), VIC2_RASTER_2_EMU(m_rasterline), 8, 1, m_border_color_sample[4]);

				if (m_border_on_sample[2])
					for (i = 5; i < 43; i++)
						m_bitmap->plot_box(VIC2_X_2_EMU(i * 8), VIC2_RASTER_2_EMU(m_rasterline), 8, 1, m_border_color_sample[i]);

				if (m_border_on_sample[3])
					m_bitmap->plot_box(VIC2_X_2_EMU(43 * 8), VIC2_RASTER_2_EMU(m_rasterline), 8, 1, m_border_color_sample[43]);

				if (m_border_on_sample[4])
				{
					for (i = 44; i < 48; i++)
						m_bitmap->plot_box(VIC2_X_2_EMU(i * 8), VIC2_RASTER_2_EMU(m_rasterline), 8, 1, m_border_color_sample[i]);
					for (i = 48; i < 53; i++)
						m_bitmap->plot_box(VIC2_X_2_EMU(i * 8), VIC2_RASTER_2_EMU(m_rasterline), 8, 1, m_border_color_sample[47]);
				}
			}

			vic2_spr_ptr_access(1);
			vic2_spr_data_access(1, 0);
			vic2_display_if_bad_line();

			if (m_spr_dma_on & 0x02)
				vic2_suspend_cpu();
			else
				vic2_resume_cpu();

			m_cycle++;
			break;

		// Sprite 1
		case 63:
			vic2_spr_data_access(1, 1);
			vic2_spr_data_access(1, 2);
			vic2_display_if_bad_line();

			m_cycle++;
			break;

		// Sprite 2
		case 64:
			vic2_spr_ptr_access(2);
			vic2_spr_data_access(2, 0);
			vic2_display_if_bad_line();

			if (m_spr_dma_on & 0x04)
				vic2_suspend_cpu();
			else
				vic2_resume_cpu();

			m_cycle++;
			break;

		// Sprite 2
		case 65:
			vic2_spr_data_access(2, 1);
			vic2_spr_data_access(2, 2);
			vic2_display_if_bad_line();

			if (m_rasterline == m_dy_stop)
				m_ud_border_on = 1;
			else
				if (SCREENON && (m_rasterline == m_dy_start))
					m_ud_border_on = 0;

			// Last cycle
			m_cycle = 1;
		}

		m_raster_x += 8;
		m_icount--;
	} while (m_icount > 0);
}


//-------------------------------------------------
//  execute_run -
//-------------------------------------------------

void mos6569_device::execute_run()
{
	do
	{
		int i;
		UINT8 mask;
		//static int adjust[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

		UINT8 cpu_cycles = m_cpu->total_cycles() & 0xff;
		UINT8 vic_cycles = (m_cycles_counter + 1) & 0xff;
		m_cycles_counter++;

	//  printf("%02x %02x %02x\n",cpu_cycles,vic_cycles,m_rdy_cycles);
	#if 0
	if (machine.input().code_pressed(KEYCODE_X))
	{
	if (machine.input().code_pressed_once(KEYCODE_Q)) adjust[1]++;
	if (machine.input().code_pressed_once(KEYCODE_W)) adjust[2]++;
	if (machine.input().code_pressed_once(KEYCODE_E)) adjust[3]++;
	if (machine.input().code_pressed_once(KEYCODE_R)) adjust[4]++;
	if (machine.input().code_pressed_once(KEYCODE_T)) adjust[5]++;
	if (machine.input().code_pressed_once(KEYCODE_Y)) adjust[6]++;
	if (machine.input().code_pressed_once(KEYCODE_U)) adjust[7]++;
	if (machine.input().code_pressed_once(KEYCODE_I)) adjust[8]++;
	if (machine.input().code_pressed_once(KEYCODE_A)) adjust[1]--;
	if (machine.input().code_pressed_once(KEYCODE_S)) adjust[2]--;
	if (machine.input().code_pressed_once(KEYCODE_D)) adjust[3]--;
	if (machine.input().code_pressed_once(KEYCODE_F)) adjust[4]--;
	if (machine.input().code_pressed_once(KEYCODE_G)) adjust[5]--;
	if (machine.input().code_pressed_once(KEYCODE_H)) adjust[6]--;
	if (machine.input().code_pressed_once(KEYCODE_J)) adjust[7]--;
	if (machine.input().code_pressed_once(KEYCODE_K)) adjust[8]--;
	if (machine.input().code_pressed_once(KEYCODE_C)) adjust[0]++;
	if (machine.input().code_pressed_once(KEYCODE_V)) adjust[0]--;
	if (machine.input().code_pressed_once(KEYCODE_Z)) printf("b:%02x 1:%02x 2:%02x 3:%02x 4:%02x 5:%02x 6:%02x 7:%02x 8:%02x\n",
	                                adjust[0],adjust[1],adjust[2],adjust[3],adjust[4],adjust[5],adjust[6],adjust[7],adjust[8]);
	}
	#define adjust(x) adjust[x]
	#else
	#define adjust(x) 0
	#endif

		switch(m_cycle)
		{

		// Sprite 3, raster counter, raster IRQ, bad line
		case 1:
			if (m_rasterline == (VIC2_LINES - 1))
			{
				m_vblanking = 1;

	//          if (LIGHTPEN_BUTTON)
				{
					m_reg[0x13] = VIC2_X_VALUE;
					m_reg[0x14] = VIC2_Y_VALUE;
				}
				vic2_set_interrupt(8);
			}
			else
			{
				m_rasterline++;

				if (m_rasterline == VIC2_FIRST_DMA_LINE)
					m_bad_lines_enabled = SCREENON;

				m_is_bad_line = ((m_rasterline >= VIC2_FIRST_DMA_LINE) && (m_rasterline <= VIC2_LAST_DMA_LINE) &&
							((m_rasterline & 0x07) == VERTICALPOS) && m_bad_lines_enabled);

				m_draw_this_line =	((VIC2_RASTER_2_EMU(m_rasterline) >= VIC2_RASTER_2_EMU(VIC2_FIRST_DISP_LINE)) &&
							(VIC2_RASTER_2_EMU(m_rasterline ) <= VIC2_RASTER_2_EMU(VIC2_LAST_DISP_LINE)));
			}

			m_border_on_sample[0] = m_border_on;
			vic2_spr_ptr_access(3);
			vic2_spr_data_access(3, 0);
			vic2_display_if_bad_line();

			if (m_spr_dma_on & 0x08)
				vic2_suspend_cpu();
			else
				vic2_resume_cpu();

			if (m_spr_dma_on & 0x08) m_rdy_cycles += (2 + adjust(1));

			m_cycle++;
			break;

		// Sprite 3
		case 2:
			if (m_vblanking)
			{
				// Vertical blank, reset counters
				m_rasterline = m_vc_base = 0;
				m_ref_cnt = 0xff;
				m_vblanking = 0;

				// Trigger raster IRQ if IRQ in line 0
				if (RASTERLINE == 0)
				{
					vic2_set_interrupt(1);
				}
			}

			if (m_rasterline == RASTERLINE)
			{
				vic2_set_interrupt(1);
			}

			m_graphic_x = VIC2_X_2_EMU(0);

			vic2_spr_data_access(3, 1);
			vic2_spr_data_access(3, 2);
			vic2_display_if_bad_line();

			m_cycle++;
			break;

		// Sprite 4
		case 3:
			vic2_spr_ptr_access(4);
			vic2_spr_data_access(4, 0);
			vic2_display_if_bad_line();

			if (m_spr_dma_on & 0x10)
				vic2_suspend_cpu();
			else
				vic2_resume_cpu();

			if (m_spr_dma_on & 0x10) m_rdy_cycles += (2 + adjust(2));

			m_cycle++;
			break;

		// Sprite 4
		case 4:
			vic2_spr_data_access(4, 1);
			vic2_spr_data_access(4, 2);
			vic2_display_if_bad_line();

			m_cycle++;
			break;

		// Sprite 5
		case 5:
			vic2_spr_ptr_access(5);
			vic2_spr_data_access(5, 0);
			vic2_display_if_bad_line();

			if (m_spr_dma_on & 0x20)
				vic2_suspend_cpu();
			else
				vic2_resume_cpu();

			if (m_spr_dma_on & 0x20) m_rdy_cycles += (2 + adjust(3));

			m_cycle++;
			break;

		// Sprite 5
		case 6:
			vic2_spr_data_access(5, 1);
			vic2_spr_data_access(5, 2);
			vic2_display_if_bad_line();

			m_cycle++;
			break;

		// Sprite 6
		case 7:
			vic2_spr_ptr_access(6);
			vic2_spr_data_access(6, 0);
			vic2_display_if_bad_line();

			if (m_spr_dma_on & 0x40)
				vic2_suspend_cpu();
			else
				vic2_resume_cpu();

			if (m_spr_dma_on & 0x40) m_rdy_cycles += (2 + adjust(4));

			m_cycle++;
			break;

		// Sprite 6
		case 8:
			vic2_spr_data_access(6, 1);
			vic2_spr_data_access(6, 2);
			vic2_display_if_bad_line();

			m_cycle++;
			break;

		// Sprite 7
		case 9:
			vic2_spr_ptr_access(7);
			vic2_spr_data_access(7, 0);
			vic2_display_if_bad_line();

			if (m_spr_dma_on & 0x80)
				vic2_suspend_cpu();
			else
				vic2_resume_cpu();

			if (m_spr_dma_on & 0x80) m_rdy_cycles += (2 + adjust(5));

			m_cycle++;
			break;

		// Sprite 7
		case 10:
			vic2_spr_data_access(7, 1);
			vic2_spr_data_access(7, 2);
			vic2_display_if_bad_line();

			m_cycle++;
			break;

		// Refresh
		case 11:
			vic2_refresh_access();
			vic2_display_if_bad_line();

			vic2_resume_cpu();

			m_cycle++;
			break;

		// Refresh, fetch if bad line
		case 12:
			vic2_refresh_access();
			vic2_fetch_if_bad_line();

			m_cycle++;
			break;

		// Refresh, fetch if bad line, raster_x
		case 13:
			vic2_draw_background();
			vic2_sample_border();
			vic2_refresh_access();
			vic2_fetch_if_bad_line();

			m_raster_x = 0xfffc;

			if ((m_in_rdy_workaround_func(0) == 0 ) && (m_is_bad_line))
				m_rdy_cycles += (43+adjust(0));

			m_cycle++;
			break;

		// Refresh, fetch if bad line, RC, VC
		case 14:
			vic2_draw_background();
			vic2_sample_border();
			vic2_refresh_access();
			vic2_rc_if_bad_line();

			m_vc = m_vc_base;

			if ((m_in_rdy_workaround_func(0) == 1 ) && (m_is_bad_line))
				m_rdy_cycles += (42+adjust(0));

			m_cycle++;
			break;

		// Refresh, fetch if bad line, sprite y expansion
		case 15:
			vic2_draw_background();
			vic2_sample_border();
			vic2_refresh_access();
			vic2_fetch_if_bad_line();

			for (i = 0; i < 8; i++)
				if (m_spr_exp_y & (1 << i))
					m_mc_base[i] += 2;

			m_ml_index = 0;
			vic2_matrix_access();

			if ((m_in_rdy_workaround_func(0) == 2 ) && (m_is_bad_line))
				m_rdy_cycles += (41+adjust(0));

			m_cycle++;
			break;

		// Graphics, sprite y expansion, sprite DMA
		case 16:
			vic2_draw_background();
			vic2_sample_border();
			vic2_graphics_access();
			vic2_fetch_if_bad_line();

			mask = 1;
			for (i = 0; i < 8; i++, mask <<= 1)
			{
				if (m_spr_exp_y & mask)
					m_mc_base[i]++;
				if ((m_mc_base[i] & 0x3f) == 0x3f)
					m_spr_dma_on &= ~mask;
			}

			vic2_matrix_access();

			if ((m_in_rdy_workaround_func(0) == 3 ) && (m_is_bad_line))
				m_rdy_cycles += (40+adjust(0));

			m_cycle++;
			break;

		// Graphics, check border
		case 17:
			if (COLUMNS40)
			{
				if (m_rasterline == m_dy_stop)
					m_ud_border_on = 1;
				else
				{
					if (SCREENON)
					{
						if (m_rasterline == m_dy_start)
							m_border_on = m_ud_border_on = 0;
						else
							if (m_ud_border_on == 0)
								m_border_on = 0;
					} else
						if (m_ud_border_on == 0)
							m_border_on = 0;
				}
			}

			// Second sample of border state
			m_border_on_sample[1] = m_border_on;

			vic2_draw_background();
			vic2_draw_graphics();
			vic2_sample_border();
			vic2_graphics_access();
			vic2_fetch_if_bad_line();
			vic2_matrix_access();

			if ((m_in_rdy_workaround_func(0) == 4 ) && (m_is_bad_line))
				m_rdy_cycles += (40+adjust(0));

			m_cycle++;
			break;

		// Check border
		case 18:
			if (!COLUMNS40)
			{
				if (m_rasterline == m_dy_stop)
					m_ud_border_on = 1;
				else
				{
					if (SCREENON)
					{
						if (m_rasterline == m_dy_start)
							m_border_on = m_ud_border_on = 0;
						else
							if (m_ud_border_on == 0)
								m_border_on = 0;
					} else
						if (m_ud_border_on == 0)
							m_border_on = 0;
				}
			}

			// Third sample of border state
			m_border_on_sample[2] = m_border_on;

		// Graphics

		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 40:
		case 41:
		case 42:
		case 43:
		case 44:
		case 45:
		case 46:
		case 47:
		case 48:
		case 49:
		case 50:
		case 51:
		case 52:
		case 53:
		case 54:
			vic2_draw_graphics();
			vic2_sample_border();
			vic2_graphics_access();
			vic2_fetch_if_bad_line();
			vic2_matrix_access();
			m_last_char_data = m_char_data;

			m_cycle++;
			break;

		// Graphics, sprite y expansion, sprite DMA
		case 55:
			vic2_draw_graphics();
			vic2_sample_border();
			vic2_graphics_access();
			vic2_display_if_bad_line();

			// sprite y expansion
			mask = 1;
			for (i = 0; i < 8; i++, mask <<= 1)
				if (SPRITE_Y_EXPAND (i))
					m_spr_exp_y ^= mask;

			vic2_check_sprite_dma();

			vic2_resume_cpu();

			m_cycle++;
			break;

		// Check border, sprite DMA
		case 56:
			if (!COLUMNS40)
				m_border_on = 1;

			// Fourth sample of border state
			m_border_on_sample[3] = m_border_on;

			vic2_draw_graphics();
			vic2_sample_border();
			vic2_idle_access();
			vic2_display_if_bad_line();
			vic2_check_sprite_dma();

			m_cycle++;
			break;

		// Check border, sprites
		case 57:
			if (COLUMNS40)
				m_border_on = 1;

			// Fifth sample of border state
			m_border_on_sample[4] = m_border_on;

			// Sample spr_disp_on and spr_data for sprite drawing
			m_spr_draw = m_spr_disp_on;
			if (m_spr_draw)
				memcpy(m_spr_draw_data, m_spr_data, 8 * 4);

			mask = 1;
			for (i = 0; i < 8; i++, mask <<= 1)
				if ((m_spr_disp_on & mask) && !(m_spr_dma_on & mask))
					m_spr_disp_on &= ~mask;

			vic2_draw_background();
			vic2_sample_border();
			vic2_idle_access();
			vic2_display_if_bad_line();

			m_cycle++;
			break;

		// Sprite 0, sprite DMA, MC, RC
		case 58:
			vic2_draw_background();
			vic2_sample_border();

			mask = 1;
			for (i = 0; i < 8; i++, mask <<= 1)
			{
				m_mc[i] = m_mc_base[i];
				if ((m_spr_dma_on & mask) && ((m_rasterline & 0xff) == SPRITE_Y_POS(i)))
					m_spr_disp_on |= mask;
			}

			vic2_spr_ptr_access(0);
			vic2_spr_data_access(0, 0);

			if (m_rc == 7)
			{
				m_vc_base = m_vc;
				m_display_state = 0;
			}

			if (m_is_bad_line || m_display_state)
			{
				m_display_state = 1;
				m_rc = (m_rc + 1) & 7;
			}

			if (m_spr_dma_on & 0x01)
				vic2_suspend_cpu();
			else
				vic2_resume_cpu();

			if (m_spr_dma_on & 0x01) m_rdy_cycles += (2 + adjust(6));

			m_cycle++;
			break;

		// Sprite 0
		case 59:
			vic2_draw_background();
			vic2_sample_border();
			vic2_spr_data_access(0, 1);
			vic2_spr_data_access(0, 2);
			vic2_display_if_bad_line();

			m_cycle++;
			break;

		// Sprite 1, draw
		case 60:
			vic2_draw_background();
			vic2_sample_border();

			if (m_draw_this_line)
			{
				vic2_draw_sprites();

				if (m_border_on_sample[0])
					for (i = 0; i < 4; i++)
						m_bitmap->plot_box(VIC2_X_2_EMU(i * 8), VIC2_RASTER_2_EMU(m_rasterline), 8, 1, m_border_color_sample[i]);

				if (m_border_on_sample[1])
					m_bitmap->plot_box(VIC2_X_2_EMU(4 * 8), VIC2_RASTER_2_EMU(m_rasterline), 8, 1, m_border_color_sample[4]);

				if (m_border_on_sample[2])
					for (i = 5; i < 43; i++)
						m_bitmap->plot_box(VIC2_X_2_EMU(i * 8), VIC2_RASTER_2_EMU(m_rasterline), 8, 1, m_border_color_sample[i]);

				if (m_border_on_sample[3])
					m_bitmap->plot_box(VIC2_X_2_EMU(43 * 8), VIC2_RASTER_2_EMU(m_rasterline), 8, 1, m_border_color_sample[43]);

				if (m_border_on_sample[4])
				{
					for (i = 44; i < 48; i++)
						m_bitmap->plot_box(VIC2_X_2_EMU(i * 8), VIC2_RASTER_2_EMU(m_rasterline), 8, 1, m_border_color_sample[i]);
					for (i = 48; i < 51; i++)
						m_bitmap->plot_box(VIC2_X_2_EMU(i * 8), VIC2_RASTER_2_EMU(m_rasterline), 8, 1, m_border_color_sample[47]);
				}
			}

			vic2_spr_ptr_access(1);
			vic2_spr_data_access(1, 0);
			vic2_display_if_bad_line();

			if (m_spr_dma_on & 0x02)
				vic2_suspend_cpu();
			else
				vic2_resume_cpu();

			if (m_spr_dma_on & 0x02) m_rdy_cycles += (2 + adjust(7));

			m_cycle++;
			break;

		// Sprite 1
		case 61:
			vic2_spr_data_access(1, 1);
			vic2_spr_data_access(1, 2);
			vic2_display_if_bad_line();

			m_cycle++;
			break;

		// Sprite 2
		case 62:
			vic2_spr_ptr_access(2);
			vic2_spr_data_access(2, 0);
			vic2_display_if_bad_line();

			if (m_spr_dma_on & 0x04)
				vic2_suspend_cpu();
			else
				vic2_resume_cpu();

			if (m_spr_dma_on & 0x04) m_rdy_cycles += (2 + adjust(8));

			m_cycle++;
			break;

		// Sprite 2
		case 63:
			vic2_spr_data_access(2, 1);
			vic2_spr_data_access(2, 2);
			vic2_display_if_bad_line();

			if (m_rasterline == m_dy_stop)
				m_ud_border_on = 1;
			else
				if (SCREENON && (m_rasterline == m_dy_start))
					m_ud_border_on = 0;

			// Last cycle
			m_cycle = 1;
		}

		if ((cpu_cycles == vic_cycles) && (m_rdy_cycles > 0))
		{
			device_spin_until_time (m_cpu, m_cpu->cycles_to_attotime(m_rdy_cycles));
			m_rdy_cycles = 0;
		}

		m_raster_x += 8;
		m_icount--;
	} while (m_icount > 0);
}

// Graphics display (8 pixels)
void mos6566_device::vic2_draw_graphics()
{
	if (m_draw_this_line == 0)
	{
		UINT16 p = m_graphic_x + HORIZONTALPOS;
		m_fore_coll_buf[p + 7] = 0;
		m_fore_coll_buf[p + 6] = 0;
		m_fore_coll_buf[p + 5] = 0;
		m_fore_coll_buf[p + 4] = 0;
		m_fore_coll_buf[p + 3] = 0;
		m_fore_coll_buf[p + 2] = 0;
		m_fore_coll_buf[p + 1] = 0;
		m_fore_coll_buf[p + 0] = 0;
	}
	else if (m_ud_border_on)
	{
		UINT16 p = m_graphic_x + HORIZONTALPOS;
		m_fore_coll_buf[p + 7] = 0;
		m_fore_coll_buf[p + 6] = 0;
		m_fore_coll_buf[p + 5] = 0;
		m_fore_coll_buf[p + 4] = 0;
		m_fore_coll_buf[p + 3] = 0;
		m_fore_coll_buf[p + 2] = 0;
		m_fore_coll_buf[p + 1] = 0;
		m_fore_coll_buf[p + 0] = 0;
		vic2_draw_background();
	}
	else
	{
		UINT8 tmp_col;
		UINT16 p = m_graphic_x + HORIZONTALPOS;
		switch (GFXMODE)
		{
			case 0:
				vic2_draw_mono(p, m_colors[0], m_color_data & 0x0f);
				break;
			case 1:
				if (m_color_data & 0x08)
					vic2_draw_multi(p, m_colors[0], m_colors[1], m_colors[2], m_color_data & 0x07);
				else
					vic2_draw_mono(p, m_colors[0], m_color_data & 0x0f);
				break;
			case 2:
				vic2_draw_mono(p, m_char_data & 0x0f, m_char_data >> 4);
				break;
			case 3:
				vic2_draw_multi(p, m_colors[0], m_char_data >> 4, m_char_data & 0x0f, m_color_data & 0x0f);
				break;
			case 4:
				if (m_char_data & 0x80)
					if (m_char_data & 0x40)
						tmp_col = m_colors[3];
					else
						tmp_col = m_colors[2];
				else
					if (m_char_data & 0x40)
						tmp_col = m_colors[1];
					else
						tmp_col = m_colors[0];
				vic2_draw_mono(p, tmp_col, m_color_data & 0x0f);
				break;
			case 5:
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 7) = 0;
				m_fore_coll_buf[p + 7] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 6) = 0;
				m_fore_coll_buf[p + 6] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 5) = 0;
				m_fore_coll_buf[p + 5] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 4) = 0;
				m_fore_coll_buf[p + 4] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 3) = 0;
				m_fore_coll_buf[p + 3] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 2) = 0;
				m_fore_coll_buf[p + 2] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 1) = 0;
				m_fore_coll_buf[p + 1] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 0) = 0;
				m_fore_coll_buf[p + 0] = 0;
				break;
			case 6:
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 7) = 0;
				m_fore_coll_buf[p + 7] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 6) = 0;
				m_fore_coll_buf[p + 6] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 5) = 0;
				m_fore_coll_buf[p + 5] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 4) = 0;
				m_fore_coll_buf[p + 4] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 3) = 0;
				m_fore_coll_buf[p + 3] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 2) = 0;
				m_fore_coll_buf[p + 2] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 1) = 0;
				m_fore_coll_buf[p + 1] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 0) = 0;
				m_fore_coll_buf[p + 0] = 0;
				break;
			case 7:
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 7) = 0;
				m_fore_coll_buf[p + 7] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 6) = 0;
				m_fore_coll_buf[p + 6] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 5) = 0;
				m_fore_coll_buf[p + 5] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 4) = 0;
				m_fore_coll_buf[p + 4] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 3) = 0;
				m_fore_coll_buf[p + 3] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 2) = 0;
				m_fore_coll_buf[p + 2] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 1) = 0;
				m_fore_coll_buf[p + 1] = 0;
				m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + 0) = 0;
				m_fore_coll_buf[p + 0] = 0;
				break;
		}
	}
}

void mos6566_device::vic2_draw_sprites()
{
	int i;
	UINT8 snum, sbit;
	UINT8 spr_coll = 0, gfx_coll = 0;
	UINT32 plane0_l, plane0_r, plane1_l, plane1_r;
	UINT32 sdata_l = 0, sdata_r = 0;

	for (i = 0; i < 0x400; i++)
		m_spr_coll_buf[i] = 0;

	for (snum = 0, sbit = 1; snum < 8; snum++, sbit <<= 1)
	{
		if ((m_spr_draw & sbit) && (SPRITE_X_POS(snum) <= (403 - (VIC2_FIRSTCOLUMN + 1))))
		{
			UINT16 p = SPRITE_X_POS(snum) + VIC2_X_2_EMU(0) + 8;
			UINT8 color = SPRITE_COLOR(snum);
			UINT32 sdata = (m_spr_draw_data[snum][0] << 24) | (m_spr_draw_data[snum][1] << 16) | (m_spr_draw_data[snum][2] << 8);

			if (SPRITE_X_EXPAND(snum))
			{
				if (SPRITE_X_POS(snum) > (403 - 24 - (VIC2_FIRSTCOLUMN + 1)))
					continue;

				if (SPRITE_MULTICOLOR(snum))
				{
					sdata_l = (m_expandx_multi[(sdata >> 24) & 0xff] << 16) | m_expandx_multi[(sdata >> 16) & 0xff];
					sdata_r = m_expandx_multi[(sdata >> 8) & 0xff] << 16;
					plane0_l = (sdata_l & 0x55555555) | (sdata_l & 0x55555555) << 1;
					plane1_l = (sdata_l & 0xaaaaaaaa) | (sdata_l & 0xaaaaaaaa) >> 1;
					plane0_r = (sdata_r & 0x55555555) | (sdata_r & 0x55555555) << 1;
					plane1_r = (sdata_r & 0xaaaaaaaa) | (sdata_r & 0xaaaaaaaa) >> 1;
					for (i = 0; i < 32; i++, plane0_l <<= 1, plane1_l <<= 1)
					{
						UINT8 col;

						if (plane1_l & 0x80000000)
						{
							if (m_fore_coll_buf[p + i])
							{
								gfx_coll |= sbit;
							}
							if (plane0_l & 0x80000000)
								col = m_spritemulti[3];
							else
								col = color;
						}
						else
						{
							if (plane0_l & 0x80000000)
							{
								if (m_fore_coll_buf[p + i])
								{
									gfx_coll |= sbit;
								}
								col = m_spritemulti[1];
							}
							else
								continue;
						}

						if (m_spr_coll_buf[p + i])
							spr_coll |= m_spr_coll_buf[p + i] | sbit;
						else
						{
							if (SPRITE_PRIORITY(snum))
							{
								if (m_fore_coll_buf[p + i] == 0)
									m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + i) = col;
								m_spr_coll_buf[p + i] = sbit;
							}
							else
							{
								m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + i) = col;
								m_spr_coll_buf[p + i] = sbit;
							}
						}
					}

					for (; i < 48; i++, plane0_r <<= 1, plane1_r <<= 1)
					{
						UINT8 col;

						if(plane1_r & 0x80000000)
						{
							if (m_fore_coll_buf[p + i])
							{
								gfx_coll |= sbit;
							}

							if (plane0_r & 0x80000000)
								col = m_spritemulti[3];
							else
								col = color;
						}
						else
						{
							if (plane0_r & 0x80000000)
							{
								if (m_fore_coll_buf[p + i])
								{
									gfx_coll |= sbit;
								}
								col =  m_spritemulti[1];
							}
							else
								continue;
						}

						if (m_spr_coll_buf[p + i])
							spr_coll |= m_spr_coll_buf[p + i] | sbit;
						else
						{
							if (SPRITE_PRIORITY(snum))
							{
								if (m_fore_coll_buf[p + i] == 0)
									m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + i) = col;
								m_spr_coll_buf[p + i] = sbit;
							}
							else
							{
								m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + i) = col;
								m_spr_coll_buf[p + i] = sbit;
							}
						}
					}
				}
				else
				{
					sdata_l = (m_expandx[(sdata >> 24) & 0xff] << 16) | m_expandx[(sdata >> 16) & 0xff];
					sdata_r = m_expandx[(sdata >> 8) & 0xff] << 16;

					for (i = 0; i < 32; i++, sdata_l <<= 1)
						if (sdata_l & 0x80000000)
						{
							if (m_fore_coll_buf[p + i])
							{
								gfx_coll |= sbit;
							}

							if (m_spr_coll_buf[p + i])
								spr_coll |= m_spr_coll_buf[p + i] | sbit;
							else
							{
								if (SPRITE_PRIORITY(snum))
								{
									if (m_fore_coll_buf[p + i] == 0)
										m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + i) = color;
									m_spr_coll_buf[p + i] = sbit;
								}
								else
								{
									m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + i) = color;
									m_spr_coll_buf[p + i] = sbit;
								}
							}
						}

					for (; i < 48; i++, sdata_r <<= 1)
						if (sdata_r & 0x80000000)
						{
							if (m_fore_coll_buf[p + i])
							{
								gfx_coll |= sbit;
							}

							if (m_spr_coll_buf[p + i])
								spr_coll |= m_spr_coll_buf[p + i] | sbit;
							else
							{
								if (SPRITE_PRIORITY(snum))
								{
									if (m_fore_coll_buf[p + i] == 0)
										m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + i) = color;
									m_spr_coll_buf[p + i] = sbit;
								}
								else
								{
									m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + i) = color;
									m_spr_coll_buf[p + i] = sbit;
								}
							}
						}
				}
			}
			else
			{
				if (SPRITE_MULTICOLOR(snum))
				{
					UINT32 plane0 = (sdata & 0x55555555) | (sdata & 0x55555555) << 1;
					UINT32 plane1 = (sdata & 0xaaaaaaaa) | (sdata & 0xaaaaaaaa) >> 1;

					for (i = 0; i < 24; i++, plane0 <<= 1, plane1 <<= 1)
					{
						UINT8 col;

						if (plane1 & 0x80000000)
						{
							if (m_fore_coll_buf[p + i])
							{
								gfx_coll |= sbit;
							}

							if (plane0 & 0x80000000)
								col = m_spritemulti[3];
							else
								col = color;
						}
						else
						{
							if (m_fore_coll_buf[p + i])
							{
								gfx_coll |= sbit;
							}

							if (plane0 & 0x80000000)
								col = m_spritemulti[1];
							else
								continue;
						}

						if (m_spr_coll_buf[p + i])
							spr_coll |= m_spr_coll_buf[p + i] | sbit;
						else
						{
							if (SPRITE_PRIORITY(snum))
							{
								if (m_fore_coll_buf[p + i] == 0)
									m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + i) = col;
								m_spr_coll_buf[p + i] = sbit;
							}
							else
							{
								m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + i) = col;
								m_spr_coll_buf[p + i] = sbit;
							}
						}
					}
				}
				else
				{
					for (i = 0; i < 24; i++, sdata <<= 1)
					{
						if (sdata & 0x80000000)
						{
							if (m_fore_coll_buf[p + i])
							{
								gfx_coll |= sbit;
							}
							if (m_spr_coll_buf[p + i])
							{
								spr_coll |= m_spr_coll_buf[p + i] | sbit;
							}
							else
							{
								if (SPRITE_PRIORITY(snum))
								{
									if (m_fore_coll_buf[p + i] == 0)
										m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + i) = color;
									m_spr_coll_buf[p + i] = sbit;
								}
								else
								{
									m_bitmap->pix16(VIC2_RASTER_2_EMU(m_rasterline), p + i) = color;
									m_spr_coll_buf[p + i] = sbit;
								}
							}
						}
					}
				}
			}
		}
	}

	if (SPRITE_COLL)
		SPRITE_COLL |= spr_coll;
	else
	{
		SPRITE_COLL = spr_coll;
		if (SPRITE_COLL)
			vic2_set_interrupt(4);
	}

	if (SPRITE_BG_COLL)
		SPRITE_BG_COLL |= gfx_coll;
	else
	{
		SPRITE_BG_COLL = gfx_coll;
		if (SPRITE_BG_COLL)
			vic2_set_interrupt(2);
	}
}


//-------------------------------------------------
//  screen_update -
//-------------------------------------------------

UINT32 mos6566_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_on)
		copybitmap(bitmap, *m_bitmap, 0, 0, 0, 0, cliprect);

	return 0;
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( mos6566_device::read )
{
	UINT8 val = 0;

	offset &= 0x3f;

	switch (offset)
	{
	case 0x11:
		val = (m_reg[offset] & ~0x80) | ((m_rasterline & 0x100) >> 1);
		break;

	case 0x12:
		val = m_rasterline & 0xff;
		break;

	case 0x16:
		val = m_reg[offset] | 0xc0;
		break;

	case 0x18:
		val = m_reg[offset] | 0x01;
		break;

	case 0x19:							/* interrupt flag register */
		/* vic2_clear_interrupt(0xf); */
		val = m_reg[offset] | 0x70;
		break;

	case 0x1a:
		val = m_reg[offset] | 0xf0;
		break;

	case 0x1e:							/* sprite to sprite collision detect */
		val = m_reg[offset];
		m_reg[offset] = 0;
		vic2_clear_interrupt(4);
		break;

	case 0x1f:							/* sprite to background collision detect */
		val = m_reg[offset];
		m_reg[offset] = 0;
		vic2_clear_interrupt(2);
		break;

	case 0x20:
	case 0x21:
	case 0x22:
	case 0x23:
	case 0x24:
		val = m_reg[offset];
		break;

	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	case 0x0e:
	case 0x0f:
	case 0x10:
	case 0x17:
	case 0x1b:
	case 0x1c:
	case 0x1d:
	case 0x25:
	case 0x26:
	case 0x27:
	case 0x28:
	case 0x29:
	case 0x2a:
	case 0x2b:
	case 0x2c:
	case 0x2d:
	case 0x2e:
		val = m_reg[offset];
		break;

	case 0x2f:
	case 0x30:
		if (IS_VICIIE)
		{
			val = m_reg[offset];
			DBG_LOG(2, "vic read", ("%.2x:%.2x\n", offset, val));
		}
		else
			val = 0xff;
		break;

	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x37:
	case 0x38:
	case 0x39:
	case 0x3a:
	case 0x3b:
	case 0x3c:
	case 0x3d:
	case 0x3e:
	case 0x3f:							/* not used */
		// val = m_reg[offset]; //
		val = 0xff;
		DBG_LOG(2, "vic read", ("%.2x:%.2x\n", offset, val));
		break;

	default:
		val = m_reg[offset];
	}

	if ((offset != 0x11) && (offset != 0x12))
		DBG_LOG(2, "vic read", ("%.2x:%.2x\n", offset, val));

	return val;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( mos6566_device::write )
{
	DBG_LOG(2, "vic write", ("%.2x:%.2x\n", offset, data));
	offset &= 0x3f;

	switch (offset)
	{
	case 0x01:
	case 0x03:
	case 0x05:
	case 0x07:
	case 0x09:
	case 0x0b:
	case 0x0d:
	case 0x0f:
		m_reg[offset] = data;		/* sprite y positions */
		break;

	case 0x00:
	case 0x02:
	case 0x04:
	case 0x06:
	case 0x08:
	case 0x0a:
	case 0x0c:
	case 0x0e:
		m_reg[offset] = data;		/* sprite x positions */
		break;

	case 0x10:
		m_reg[offset] = data;		/* sprite x positions */
		break;

	case 0x17:							/* sprite y size */
		m_spr_exp_y |= ~data;
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
		}
		break;

	case 0x1d:							/* sprite x size */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
		}
		break;

	case 0x1b:							/* sprite background priority */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
		}
		break;

	case 0x1c:							/* sprite multicolor mode select */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
		}
		break;

	case 0x27:
	case 0x28:
	case 0x29:
	case 0x2a:
	case 0x2b:
	case 0x2c:
	case 0x2d:
	case 0x2e:
									/* sprite colors */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
		}
		break;

	case 0x25:							/* sprite multicolor */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
			m_spritemulti[1] = SPRITE_MULTICOLOR1;
		}
		break;

	case 0x26:							/* sprite multicolor */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
			m_spritemulti[3] = SPRITE_MULTICOLOR2;
		}
		break;

	case 0x19:
		vic2_clear_interrupt(data & 0x0f);
		break;

	case 0x1a:							/* irq mask */
		m_reg[offset] = data;
		vic2_set_interrupt(0);	// beamrider needs this
		break;

	case 0x11:
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
			if (data & 8)
			{
				m_dy_start = ROW25_YSTART;
				m_dy_stop = ROW25_YSTOP;
			}
			else
			{
				m_dy_start = ROW24_YSTART;
				m_dy_stop = ROW24_YSTOP;
			}
		}
		break;

	case 0x12:
		if (data != m_reg[offset])
		{
			m_reg[offset] = data;
		}
		break;

	case 0x16:
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
		}
		break;

	case 0x18:
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
			m_videoaddr = VIDEOADDR;
			m_chargenaddr = CHARGENADDR;
			m_bitmapaddr = BITMAPADDR;
		}
		break;

	case 0x21:							/* background color */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
			m_colors[0] = BACKGROUNDCOLOR;
		}
		break;

	case 0x22:							/* background color 1 */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
			m_colors[1] = MULTICOLOR1;
		}
		break;

	case 0x23:							/* background color 2 */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
			m_colors[2] = MULTICOLOR2;
		}
		break;

	case 0x24:							/* background color 3 */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
			m_colors[3] = FOREGROUNDCOLOR;
		}
		break;

	case 0x20:							/* framecolor */
		if (m_reg[offset] != data)
		{
			m_reg[offset] = data;
		}
		break;

	case 0x2f:
		if (IS_VICIIE)
		{
			DBG_LOG(2, "vic write", ("%.2x:%.2x\n", offset, data));
			m_reg[offset] = data;
		}
		break;

	case 0x30:
		if (IS_VICIIE)
		{
			m_reg[offset] = data;
		}
		break;

	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x37:
	case 0x38:
	case 0x39:
	case 0x3a:
	case 0x3b:
	case 0x3c:
	case 0x3d:
	case 0x3e:
	case 0x3f:
		m_reg[offset] = data;
		DBG_LOG(2, "vic write", ("%.2x:%.2x\n", offset, data));
		break;

	default:
		m_reg[offset] = data;
		break;
	}
}


//-------------------------------------------------
//  lp_w - light pen strobe
//-------------------------------------------------

WRITE_LINE_MEMBER( mos6566_device::lp_w )
{
}


//-------------------------------------------------
//  bus_r - data bus read
//-------------------------------------------------

UINT8 mos6566_device::bus_r()
{
	return m_last_data;
}
