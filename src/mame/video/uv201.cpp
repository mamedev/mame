// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VideoBrain UV201/UV202 video chip emulation

**********************************************************************/

#include "uv201.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG         1


// screen parameters
#define SCREEN_WIDTH                232
#define SCREEN_HEIGHT               262
#define VISAREA_WIDTH               193
#define VBLANK_WIDTH                21
#define HBLANK_WIDTH                39
#define HSYNC_WIDTH                 18
#define HFP_WIDTH                   16
#define HBP_WIDTH                   5
#define HBLANK_END                  HSYNC_WIDTH + HFP_WIDTH
#define HBLANK_START                HBLANK_END + VISAREA_WIDTH


// write-only registers
#define REGISTER_COMMAND            0xf7
#define REGISTER_BACKGROUND         0xf5
#define REGISTER_FINAL_MODIFIER     0xf2
#define REGISTER_Y_INTERRUPT        0xf0


// read-only registers
#define REGISTER_X_FREEZE           0xf8
#define REGISTER_Y_FREEZE_LOW       0xf9
#define REGISTER_Y_FREEZE_HIGH      0xfa
#define REGISTER_CURRENT_Y_LOW      0xfb


// read/write registers - RAM memory
#define RAM_RP_LO                   0x00    // cartridge pointer low order
#define RAM_RP_HI_COLOR             0x10    // cartridge pointer high order and color
#define RAM_DX_INT_XCOPY            0x20    // dX, intensity, X-copy
#define RAM_DY                      0x30    // dY
#define RAM_X                       0x40    // X value
#define RAM_Y_LO_A                  0x50    // Y value low order list A
#define RAM_Y_LO_B                  0x60    // Y value low order list B
#define RAM_XY_HI_A                 0x70    // Y value high order and X order list A
#define RAM_XY_HI_B                 0x80    // Y value high order and X order list B


// command register bits
#define COMMAND_YINT_H_O            0x80
#define COMMAND_A_B                 0x40
#define COMMAND_Y_ZM                0x20
#define COMMAND_KBD                 0x10
#define COMMAND_INT                 0x08
#define COMMAND_ENB                 0x04
#define COMMAND_FRZ                 0x02
#define COMMAND_X_ZM                0x01


#define IS_CHANGED(_bit) \
	((m_cmd & _bit) != (data & _bit))

#define RAM(_offset) \
	m_ram[_offset + i]

#define RAM_XORD(_offset) \
	m_ram[_offset + xord]

#define IS_VISIBLE(_y) \
	((_y >= cliprect.min_y) && (_y <= cliprect.max_y))

#define DRAW_PIXEL(_scanline, _dot) \
	if (IS_VISIBLE(_scanline)) bitmap.pix32((_scanline), HSYNC_WIDTH + HFP_WIDTH + _dot) = m_palette_val[pixel];



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type UV201 = &device_creator<uv201_device>;


//-------------------------------------------------
//  uv201_device - constructor
//-------------------------------------------------

uv201_device::uv201_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, UV201, "UV201", tag, owner, clock, "uv201", __FILE__),
	device_video_interface(mconfig, *this),
	m_write_ext_int(*this),
	m_write_hblank(*this),
	m_read_db(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void uv201_device::device_start()
{
	// resolve callbacks
	m_write_ext_int.resolve_safe();
	m_write_hblank.resolve_safe();
	m_read_db.resolve_safe(0);

	// allocate timers
	m_timer_y_odd = timer_alloc(TIMER_Y_ODD);
	m_timer_y_even = timer_alloc(TIMER_Y_EVEN);
	m_timer_hblank_on = timer_alloc(TIMER_HBLANK_ON);
	m_timer_hblank_off = timer_alloc(TIMER_HBLANK_OFF);

	initialize_palette();

	memset(m_ram, 0x00, sizeof(m_ram));
	m_y_int = 0;
	m_fmod = 0;
	m_bg = 0;
	m_cmd = 0;
	m_freeze_x = 0;
	m_freeze_y = 0;
	m_field = 0;

	// state saving
	save_item(NAME(m_ram));
	save_item(NAME(m_y_int));
	save_item(NAME(m_fmod));
	save_item(NAME(m_bg));
	save_item(NAME(m_cmd));
	save_item(NAME(m_freeze_x));
	save_item(NAME(m_freeze_y));
	save_item(NAME(m_field));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void uv201_device::device_reset()
{
	m_write_ext_int(CLEAR_LINE);

	m_write_hblank(1);
	m_timer_hblank_off->adjust(attotime::from_ticks( HBLANK_END, m_clock ));
}


//-------------------------------------------------
//  device_timer - handle timer events
//-------------------------------------------------

void uv201_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int scanline = m_screen->vpos();

	switch (id)
	{
	case TIMER_Y_ODD:
	case TIMER_Y_EVEN:
		if ((m_cmd & COMMAND_INT) && !(m_cmd & COMMAND_FRZ))
		{
			if (LOG) logerror("Y-Interrupt at scanline %u\n", scanline);

			m_freeze_y = scanline;

			m_write_ext_int(ASSERT_LINE);
			m_write_ext_int(CLEAR_LINE);
		}
		break;

	case TIMER_HBLANK_ON:
		m_write_hblank(1);

		m_timer_hblank_off->adjust(attotime::from_ticks( HBLANK_WIDTH, m_clock ) );
		break;

	case TIMER_HBLANK_OFF:
		m_write_hblank(0);

		m_timer_hblank_on->adjust(attotime::from_ticks( VISAREA_WIDTH, m_clock ) );
		break;
	}
}


//-------------------------------------------------
//  initialize_palette -
//-------------------------------------------------

void uv201_device::initialize_palette()
{
	UINT8 offlointensity = 0x00;
	UINT8 offhiintensity = 0xc0;

	UINT8 onlointensity = 0xa0;
	UINT8 onhiintensity = 0xff;

	for (int i = 0; i < 4; i++)
	{
		int offset = i * 8;
		UINT8 onvalue, offvalue;

		if (offset < 16)
		{
			offvalue = offlointensity;
			onvalue = onlointensity;
		}
		else
		{
			offvalue = offhiintensity;
			onvalue = onhiintensity;
		}

		m_palette_val[offset + 0] = rgb_t(offvalue, offvalue, offvalue); // black
		m_palette_val[offset + 1] = rgb_t(onvalue, offvalue, offvalue); // red
		m_palette_val[offset + 2] = rgb_t(offvalue, onvalue, offvalue); // green
		m_palette_val[offset + 3] = rgb_t(onvalue, onvalue, offvalue); // red-green
		m_palette_val[offset + 4] = rgb_t(offvalue, offvalue, onvalue); // blue
		m_palette_val[offset + 5] = rgb_t(onvalue, offvalue, onvalue); // red-blue
		m_palette_val[offset + 6] = rgb_t(offvalue, onvalue, onvalue); // green-blue
		m_palette_val[offset + 7] = rgb_t(onvalue, onvalue, onvalue); // white
	}
}


//-------------------------------------------------
//  get_field_vpos - get scanline within field
//-------------------------------------------------

int uv201_device::get_field_vpos()
{
	int vpos = m_screen->vpos();

	if (vpos >= SCREEN_HEIGHT)
	{
		// even field
		vpos -= SCREEN_HEIGHT;
	}

	return vpos;
}


//-------------------------------------------------
//  get_field - get video field
//-------------------------------------------------

int uv201_device::get_field()
{
	return m_screen->vpos() < SCREEN_HEIGHT;
}


//-------------------------------------------------
//  set_y_interrupt - set Y interrupt timer
//-------------------------------------------------

void uv201_device::set_y_interrupt()
{
	int scanline = ((m_cmd & COMMAND_YINT_H_O) << 1) | m_y_int;

	m_timer_y_odd->adjust(m_screen->time_until_pos(scanline), 0, m_screen->frame_period());
	//m_timer_y_even->adjust(m_screen->time_until_pos(scanline + SCREEN_HEIGHT), 0, m_screen->frame_period());
}


//-------------------------------------------------
//  do_partial_update - update screen
//-------------------------------------------------

void uv201_device::do_partial_update()
{
	int vpos = m_screen->vpos();

	if (LOG) logerror("Partial screen update at scanline %u\n", vpos);

	m_screen->update_partial(vpos);
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( uv201_device::read )
{
	UINT8 data = 0xff;

	switch (offset)
	{
	case REGISTER_X_FREEZE:
		data = m_freeze_x;

		if (LOG) logerror("X-Freeze %02x\n", data);
		break;

	case REGISTER_Y_FREEZE_LOW:
		data = m_freeze_y & 0xff;

		if (LOG) logerror("Y-Freeze Low %02x\n", data);
		break;

	case REGISTER_Y_FREEZE_HIGH:
		/*

		    bit     signal      description

		    0       Y-F8        Y freeze high order (MSB) bit
		    1       Y-C8        current Y counter high order (MSB) bit
		    2
		    3
		    4
		    5
		    6
		    7       O/_E        odd/even field

		*/

		data = (get_field() << 7) | (BIT(get_field_vpos(), 8) << 1) | BIT(m_freeze_y, 8);

		if (LOG) logerror("Y-Freeze High %02x\n", data);
		break;

	case REGISTER_CURRENT_Y_LOW:
		data = get_field_vpos() & 0xff;

		if (LOG) logerror("Current-Y Low %02x\n", data);
		break;

	default:
		if (offset < 0x90)
			data = m_ram[offset];
		else
			if (LOG) logerror("Unknown VLSI read from %02x!\n", offset);
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( uv201_device::write )
{
	switch (offset)
	{
	case REGISTER_Y_INTERRUPT:
		if (LOG) logerror("Y-Interrupt %02x\n", data);

		if (m_y_int != data)
		{
			m_y_int = data;
			set_y_interrupt();
		}
		break;

	case REGISTER_FINAL_MODIFIER:
		/*

		    bit     signal      description

		    0       RED         red
		    1       GREEN       green
		    2       BLUE        blue
		    3       INT 0       intensity 0
		    4       INT 1       intensity 1
		    5       not used
		    6       not used
		    7       not used

		*/

		if (LOG) logerror("Final Modifier %02x\n", data);

		do_partial_update();
		m_fmod = data & 0x1f;
		break;

	case REGISTER_BACKGROUND:
		/*

		    bit     signal      description

		    0       RED         red
		    1       GREEN       green
		    2       BLUE        blue
		    3       INT 0       intensity 0
		    4       INT 1       intensity 1
		    5       not used
		    6       not used
		    7       not used

		*/

		if (LOG) logerror("Background %02x\n", data);

		do_partial_update();
		m_bg = data & 0x1f;
		break;

	case REGISTER_COMMAND:
		/*

		    bit     signal      description

		    0       X-ZM        X zoom
		    1       FRZ         freeze
		    2       ENB         video enable
		    3       INT         interrupt enable
		    4       KBD         general purpose output
		    5       Y-ZM        Y zoom
		    6       A/_B        list selection
		    7       YINT H.O.   Y COMMAND_INT register high order bit

		*/

		if (LOG) logerror("Command %02x\n", data);

		if (IS_CHANGED(COMMAND_YINT_H_O))
		{
			set_y_interrupt();
		}

		if (IS_CHANGED(COMMAND_A_B) || IS_CHANGED(COMMAND_Y_ZM) || IS_CHANGED(COMMAND_X_ZM))
		{
			do_partial_update();
		}

		m_cmd = data;
		break;

	default:
		if (offset < 0x90)
			m_ram[offset] = data;
		else
			logerror("Unknown VLSI write %02x to %02x!\n", data, offset);
	}
}


//-------------------------------------------------
//  ext_int_w - external interrupt write
//-------------------------------------------------

WRITE_LINE_MEMBER( uv201_device::ext_int_w )
{
	if (!state && (m_cmd & COMMAND_FRZ))
	{
		m_freeze_y = get_field_vpos();
		m_freeze_x = m_screen->hpos();
	}
}


//-------------------------------------------------
//  kbd_r - keyboard select read
//-------------------------------------------------

READ_LINE_MEMBER( uv201_device::kbd_r )
{
	return (m_cmd & COMMAND_KBD) ? 1 : 0;
}


//-------------------------------------------------
//  screen_update -
//-------------------------------------------------

UINT32 uv201_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(rgb_t(0x00,0x00,0x00), cliprect);

	if (!(m_cmd & COMMAND_ENB))
	{
		return 0;
	}

	for (int y = 0; y < SCREEN_HEIGHT; y++)
	{
		for (int x = 0; x < VISAREA_WIDTH; x++)
		{
			int pixel = m_bg;
			DRAW_PIXEL(y, x);
		}
	}

	for (int i = 0; i < 16; i++)
	{
		UINT8 xy_hi = (m_cmd & COMMAND_A_B) ? RAM(RAM_XY_HI_A) : RAM(RAM_XY_HI_B);
		UINT8 y_lo = (m_cmd & COMMAND_A_B) ? RAM(RAM_Y_LO_A) : RAM(RAM_Y_LO_B);
		UINT16 y = (BIT(xy_hi, 7) << 8) | y_lo;
		int xord = xy_hi & 0x0f;

		UINT8 rp_hi_color = RAM_XORD(RAM_RP_HI_COLOR);
		UINT8 rp_lo = RAM_XORD(RAM_RP_LO);
		UINT16 rp = ((rp_hi_color << 8) | rp_lo) & 0x1fff;

		if (rp < 0x800) rp |= 0x2000;

		UINT8 dx_int_xcopy = RAM_XORD(RAM_DX_INT_XCOPY);
		int color = ((dx_int_xcopy & 0x60) >> 2) | (BIT(rp_hi_color, 5) << 2) | (BIT(rp_hi_color, 6) << 1) | (BIT(rp_hi_color, 7));
		UINT8 dx = dx_int_xcopy & 0x1f;
		UINT8 dy = RAM_XORD(RAM_DY);
		int xcopy = BIT(dx_int_xcopy, 7);
		UINT8 x = RAM_XORD(RAM_X);

		if (LOG) logerror("Object %u xord %u y %u x %u dy %u dx %u xcopy %u color %u rp %04x\n", i, xord, y, x, dy, dx, xcopy, color, rp);

		if (rp == 0) continue;
		if (y > SCREEN_HEIGHT) continue;

		for (int sy = 0; sy < dy; sy++)
		{
			for (int sx = 0; sx < dx; sx++)
			{
				UINT8 data = m_read_db(rp);

				for (int bit = 0; bit < 8; bit++)
				{
					int pixel = ((BIT(data, 7) ? color : m_bg) ^ m_fmod) & 0x1f;

					if (m_cmd & COMMAND_Y_ZM)
					{
						int scanline = y + (sy * 2);

						if (m_cmd & COMMAND_X_ZM)
						{
							int dot = (x * 2) + (sx * 16) + (bit * 2);

							DRAW_PIXEL(scanline, dot);
							DRAW_PIXEL(scanline, dot + 1);
							DRAW_PIXEL(scanline + 1, dot);
							DRAW_PIXEL(scanline + 1, dot + 1);
						}
						else
						{
							int dot = x + (sx * 8) + bit;

							DRAW_PIXEL(scanline, dot);
							DRAW_PIXEL(scanline + 1, dot);
						}
					}
					else
					{
						int scanline = y + sy;

						if (m_cmd & COMMAND_X_ZM)
						{
							int dot = (x * 2) + (sx * 16) + (bit * 2);

							DRAW_PIXEL(scanline, dot);
							DRAW_PIXEL(scanline, dot + 1);
						}
						else
						{
							int dot = x + (sx * 8) + bit;

							DRAW_PIXEL(scanline, dot);
						}
					}

					data <<= 1;
				}

				if (!xcopy) rp++;
			}

			if (xcopy) rp++;
		}
	}

	return 0;
}
