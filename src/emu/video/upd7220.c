// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Miodrag Milanovic, Carl
/**********************************************************************

    Intel 82720 Graphics Display Controller emulation

**********************************************************************/

/*

    TODO:

    - implement FIFO as ring buffer
    - commands
        - DMAR
        - DMAW
    - incomplete / unimplemented FIGD / GCHRD draw modes
        - FIGD character
        - slanted character
    - read-modify-write cycle
        - read data
        - modify data
        - write data
    - QX-10 diagnostic test has positioning bugs with the bitmap display test;
    - QX-10 diagnostic test misses the zooming factor (external pin);
    - compis2 SAD address for bitmap is 0x20000 for whatever reason (presumably missing banking);
    - A5105 has a FIFO bug with the RDAT, should be a lot larger when it scrolls up.
      The problem is that DMA-ing with RDAT/WDAT shouldn't be instant;

    - honor visible area
    - wide mode (32-bit access)
    - light pen
    - dad and mask are the same, in figd dad is shifted every step and when msb or lsb are 1 ead is advanced in x dir

*/

#include "emu.h"
#include "upd7220.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define VERBOSE         0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


// todo typedef
enum
{
	COMMAND_INVALID = -1,
	COMMAND_RESET,
	COMMAND_SYNC,
	COMMAND_VSYNC,
	COMMAND_CCHAR,
	COMMAND_START,
	COMMAND_BCTRL,
	COMMAND_ZOOM,
	COMMAND_CURS,
	COMMAND_PRAM,
	COMMAND_PITCH,
	COMMAND_WDAT,
	COMMAND_MASK,
	COMMAND_FIGS,
	COMMAND_FIGD,
	COMMAND_GCHRD,
	COMMAND_RDAT,
	COMMAND_CURD,
	COMMAND_LPRD,
	COMMAND_DMAR,
	COMMAND_DMAW,
	COMMAND_5A
};

enum
{
	FIFO_READ = 0,
	FIFO_WRITE
};

enum
{
	FIFO_EMPTY = -1,
	FIFO_PARAMETER,
	FIFO_COMMAND
};

#define UPD7220_COMMAND_RESET           0x00
#define UPD7220_COMMAND_SYNC            0x0e // & 0xfe
#define UPD7220_COMMAND_VSYNC           0x6e // & 0xfe
#define UPD7220_COMMAND_CCHAR           0x4b
#define UPD7220_COMMAND_START           0x6b
#define UPD7220_COMMAND_BCTRL           0x0c // & 0xfe
#define UPD7220_COMMAND_ZOOM            0x46
#define UPD7220_COMMAND_CURS            0x49
#define UPD7220_COMMAND_PRAM            0x70 // & 0xf0
#define UPD7220_COMMAND_PITCH           0x47
#define UPD7220_COMMAND_WDAT            0x20 // & 0xe4
#define UPD7220_COMMAND_MASK            0x4a
#define UPD7220_COMMAND_FIGS            0x4c
#define UPD7220_COMMAND_FIGD            0x6c
#define UPD7220_COMMAND_GCHRD           0x68
#define UPD7220_COMMAND_RDAT            0xa0 // & 0xe4
#define UPD7220_COMMAND_CURD            0xe0
#define UPD7220_COMMAND_LPRD            0xc0
#define UPD7220_COMMAND_DMAR            0xa4 // & 0xe4
#define UPD7220_COMMAND_DMAW            0x24 // & 0xe4
#define UPD7220_COMMAND_5A              0x5a

#define UPD7220_SR_DATA_READY           0x01
#define UPD7220_SR_FIFO_FULL            0x02
#define UPD7220_SR_FIFO_EMPTY           0x04
#define UPD7220_SR_DRAWING_IN_PROGRESS  0x08
#define UPD7220_SR_DMA_EXECUTE          0x10
#define UPD7220_SR_VSYNC_ACTIVE         0x20
#define UPD7220_SR_HBLANK_ACTIVE        0x40
#define UPD7220_SR_LIGHT_PEN_DETECT     0x80

#define UPD7220_MODE_REFRESH_RAM        0x04
#define UPD7220_MODE_DRAW_ON_RETRACE    0x10
#define UPD7220_MODE_DISPLAY_MASK       0x22
#define UPD7220_MODE_DISPLAY_MIXED      0x00
#define UPD7220_MODE_DISPLAY_GRAPHICS   0x02
#define UPD7220_MODE_DISPLAY_CHARACTER  0x20
#define UPD7220_MODE_DISPLAY_INVALID    0x22
#define UPD7220_MODE_INTERLACE_MASK     0x09
#define UPD7220_MODE_INTERLACE_NONE     0x00
#define UPD7220_MODE_INTERLACE_INVALID  0x01
#define UPD7220_MODE_INTERLACE_REPEAT   0x08
#define UPD7220_MODE_INTERLACE_ON       0x09


static const int x_dir[8] = { 0, 1, 1, 1, 0,-1,-1,-1};
static const int y_dir[8] = { 1, 1, 0,-1,-1,-1, 0, 1};


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
const device_type UPD7220 = &device_creator<upd7220_device>;


// default address map
static ADDRESS_MAP_START( upd7220_vram, AS_0, 16, upd7220_device )
	AM_RANGE(0x00000, 0x3ffff) AM_RAM
ADDRESS_MAP_END


// internal 128x14 control ROM
// hand-dumped as little-endian from a die shot
ROM_START( upd7220 )
	ROM_REGION( 0x100, "upd7220", 0 )
	ROM_LOAD( "upd7220.bin", 0x000, 0x100, CRC(3c92b218) SHA1(e154b3106a80c9c98d9f2ee18efcd7f4b4aa7d49) )
ROM_END


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *upd7220_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : NULL;
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *upd7220_device::device_rom_region() const
{
	return ROM_NAME( upd7220 );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  readbyte - read a byte at the given address
//-------------------------------------------------

inline UINT8 upd7220_device::readbyte(offs_t address)
{
	return space().read_byte(address);
}


//-------------------------------------------------
//  writebyte - write a byte at the given address
//-------------------------------------------------

inline void upd7220_device::writebyte(offs_t address, UINT8 data)
{
	space().write_byte(address, data);
}

inline UINT16 upd7220_device::readword(offs_t address)
{
	return space().read_word(address);
}


inline void upd7220_device::writeword(offs_t address, UINT16 data)
{
	space().write_word(address, data);
}

//-------------------------------------------------
//  fifo_clear -
//-------------------------------------------------

inline void upd7220_device::fifo_clear()
{
	for (int i = 0; i < 16; i++)
	{
		m_fifo[i] = 0;
		m_fifo_flag[i] = FIFO_EMPTY;
	}

	m_fifo_ptr = -1;

	m_sr &= ~UPD7220_SR_DATA_READY;
	m_sr |= UPD7220_SR_FIFO_EMPTY;
	m_sr &= ~UPD7220_SR_FIFO_FULL;
}


//-------------------------------------------------
//  fifo_param_count -
//-------------------------------------------------

inline int upd7220_device::fifo_param_count()
{
	int i;

	for (i = 0; i < 16; i++)
	{
		if (m_fifo_flag[i] != FIFO_PARAMETER) break;
	}

	return i;
}


//-------------------------------------------------
//  fifo_set_direction -
//-------------------------------------------------

inline void upd7220_device::fifo_set_direction(int dir)
{
	if (m_fifo_dir != dir)
	{
		fifo_clear();
	}

	m_fifo_dir = dir;
}


//-------------------------------------------------
//  queue -
//-------------------------------------------------

inline void upd7220_device::queue(UINT8 data, int flag)
{
	if (m_fifo_ptr < 15)
	{
		m_fifo_ptr++;

		m_fifo[m_fifo_ptr] = data;
		m_fifo_flag[m_fifo_ptr] = flag;

		if (m_fifo_ptr == 16)
		{
			m_sr |= UPD7220_SR_FIFO_FULL;
		}

		m_sr &= ~UPD7220_SR_FIFO_EMPTY;
	}
	else
	{
		// TODO what happen? somebody set us up the bomb
		logerror("FIFO?\n");
	}
}


//-------------------------------------------------
//  dequeue -
//-------------------------------------------------

inline void upd7220_device::dequeue(UINT8 *data, int *flag)
{
	*data = m_fifo[0];
	*flag = m_fifo_flag[0];

	if (m_fifo_ptr > -1)
	{
		for (int i = 0; i < 15; i++)
		{
			m_fifo[i] = m_fifo[i + 1];
			m_fifo_flag[i] = m_fifo_flag[i + 1];
		}

		m_fifo[15] = 0;
		m_fifo_flag[15] = 0;

		m_fifo_ptr--;

		if (m_fifo_ptr == -1)
		{
			m_sr &= ~UPD7220_SR_DATA_READY;
			m_sr |= UPD7220_SR_FIFO_EMPTY;
		}
	}
}


//-------------------------------------------------
//  update_vsync_timer -
//-------------------------------------------------

inline void upd7220_device::update_vsync_timer(int state)
{
	int next_y = state ? m_vs : 0;

	attotime duration = m_screen->time_until_pos(next_y, 0);

	m_vsync_timer->adjust(duration, !state);
}


//-------------------------------------------------
//  update_hsync_timer -
//-------------------------------------------------

inline void upd7220_device::update_hsync_timer(int state)
{
	int y = m_screen->vpos();

	int next_x = state ? m_hs : 0;
	int next_y = state ? y : ((y + 1) % m_al);

	attotime duration = m_screen->time_until_pos(next_y, next_x);

	m_hsync_timer->adjust(duration, !state);
}


//-------------------------------------------------
//  update_blank_timer -
//-------------------------------------------------

inline void upd7220_device::update_blank_timer(int state)
{
	int y = m_screen->vpos();

	int next_x = state ? (m_hs + m_hbp) : (m_hs + m_hbp + (m_aw << 3));
	int next_y = state ? ((y + 1) % (m_vs + m_vbp + m_al + m_vfp - 1)) : y;

	attotime duration = m_screen->time_until_pos(next_y, next_x);

	m_hsync_timer->adjust(duration, !state);
}


//-------------------------------------------------
//  recompute_parameters -
//-------------------------------------------------

inline void upd7220_device::recompute_parameters()
{
	int horiz_mult = 16, vert_mult = 1;
	/* TODO: assume that the pitch also controls number of horizontal pixels in a single cell */
	// horiz_mult = 4 if both mixed and interlace?
	if((m_mode & UPD7220_MODE_DISPLAY_MASK) == UPD7220_MODE_DISPLAY_MIXED)
		horiz_mult = 8;
	else if((m_mode & UPD7220_MODE_INTERLACE_MASK) == UPD7220_MODE_INTERLACE_ON)
	{
		// in interlaced mode every line contains both fields
		horiz_mult = 8;
		vert_mult = 2;
	}

	int horiz_pix_total = (m_hs + m_hbp + m_hfp + m_aw) * horiz_mult;
	int vert_pix_total = (m_vs + m_vbp + m_al + m_vfp) * vert_mult;

	//printf("%d %d %d %d\n",m_hs,m_hbp,m_aw,m_hfp);
	//printf("%d %d\n",m_aw * 8,m_pitch * 8);

	if (horiz_pix_total == 0 || vert_pix_total == 0) //bail out if screen params aren't valid
		return;

	attoseconds_t refresh = HZ_TO_ATTOSECONDS(clock() * 8) * horiz_pix_total * vert_pix_total;

	rectangle visarea;

	visarea.min_x = 0; //(m_hs + m_hbp) * 8;
	visarea.min_y = m_vbp; //m_vs + m_vbp;
	visarea.max_x = m_aw * horiz_mult - 1;//horiz_pix_total - (m_hfp * 8) - 1;
	visarea.max_y = m_al * vert_mult + m_vbp - 1;//vert_pix_total - m_vfp - 1;

	LOG(("uPD7220 '%s' Screen: %u x %u @ %f Hz\n", tag(), horiz_pix_total, vert_pix_total, 1 / ATTOSECONDS_TO_DOUBLE(refresh)));
	LOG(("Visible Area: (%u, %u) - (%u, %u)\n", visarea.min_x, visarea.min_y, visarea.max_x, visarea.max_y));
	LOG(("%d %d %d %d %d\n",m_hs,m_hbp,m_aw,m_hfp,m_pitch));
	LOG(("%d %d %d %d\n",m_vs,m_vbp,m_al,m_vfp));

	if (m_m)
	{
		m_screen->configure(horiz_pix_total, vert_pix_total, visarea, refresh);

		update_hsync_timer(0);
		update_vsync_timer(0);
	}
	else
	{
		m_hsync_timer->enable(0);
		m_vsync_timer->enable(0);
	}

	update_blank_timer(0);
}


//-------------------------------------------------
//  reset_figs_param -
//-------------------------------------------------

inline void upd7220_device::reset_figs_param()
{
	m_figs.m_dc = 0x0000;
	m_figs.m_d = 0x0008;
	m_figs.m_d1 = 0x0008;
	m_figs.m_d2 = 0x0000;
	m_figs.m_dm = 0x0000;
	m_figs.m_gd = 0;
}


//-------------------------------------------------
//  read_vram -
//-------------------------------------------------

inline void upd7220_device::read_vram(UINT8 type, UINT8 mod)
{
	if (type == 1)
	{
		LOG (("uPD7220 invalid type 1 RDAT parameter\n"));
		return;
	}

	if (mod)
		LOG (("uPD7220 RDAT used with mod = %02x?\n",mod));

	while (m_figs.m_dc && m_fifo_ptr < (type ? 15 : 14))
	{
		switch(type)
		{
			case 0:
				queue(readbyte(m_ead*2), 0);
				queue(readbyte(m_ead*2+1), 0);
				break;
			case 2:
				queue(readbyte(m_ead*2), 0);
				break;
			case 3:
				queue(readbyte(m_ead*2+1), 0);
				break;
		}

		m_figs.m_dc--;
		m_ead += x_dir[m_figs.m_dir] + (y_dir[m_figs.m_dir] * m_pitch);
		m_ead &= 0x3ffff;
	}

	if (m_figs.m_dc == 0)
		reset_figs_param();
}


//-------------------------------------------------
//  write_vram -
//-------------------------------------------------

inline void upd7220_device::write_vram(UINT8 type, UINT8 mod)
{
	UINT16 result;

	if (type == 1)
	{
		logerror("uPD7220 invalid type 1 WDAT parameter\n");
		return;
	}

	result = 0;

	result = m_pr[1] | (m_pr[2] << 8);

	switch(type)
	{
		case 0:
			result &= m_mask;
			break;
		case 2:
			result &= (m_mask & 0xff);
			break;
		case 3:
			result <<= 8;
			result &= (m_mask & 0xff00);
			break;
	}

	//if(result)
	{
		//printf("%04x %02x %02x %04x %02x %02x\n",readbyte(m_ead),m_pr[1],m_pr[2],m_mask,type,mod);
		//printf("%04x %02x %02x\n",m_ead,m_figs.m_dir,m_pitch);
		//printf("%04x %04x %02x %04x\n",m_ead,result,mod,m_figs.m_dc);
	}

	for(int i = 0; i < m_figs.m_dc + 1; i++)
	{
		switch(mod & 3)
		{
			case 0x00: //replace
				if(type == 0)
					writeword(m_ead*2+0, result);
				if(type == 2)
					writebyte(m_ead*2+0, result & 0xff);
				if(type == 3)
					writebyte(m_ead*2+1, result >> 8);
				break;
			case 0x01: //complement
				if(type == 0)
					writeword(m_ead*2+0, readword(m_ead*2+0) ^ result);
				if(type == 2)
					writebyte(m_ead*2+0, readbyte(m_ead*2+0) ^ (result & 0xff));
				if(type == 3)
					writebyte(m_ead*2+1, readbyte(m_ead*2+1) ^ (result >> 8));
				break;
			case 0x02: //reset to zero
				if(type == 0)
					writeword(m_ead*2+0, readword(m_ead*2+0) & ~result);
				if(type == 2)
					writebyte(m_ead*2+0, readbyte(m_ead*2+0) & ~(result & 0xff));
				if(type == 3)
					writebyte(m_ead*2+1, readbyte(m_ead*2+1) & ~(result >> 8));
				break;
			case 0x03: //set to one
				if(type == 0)
					writeword(m_ead*2+0, readword(m_ead*2+0) | result);
				if(type == 2)
					writebyte(m_ead*2+0, readbyte(m_ead*2+0) | (result & 0xff));
				if(type == 3)
					writebyte(m_ead*2+1, readbyte(m_ead*2+1) | (result >> 8));
				break;
		}

		m_ead += x_dir[m_figs.m_dir] + (y_dir[m_figs.m_dir] * m_pitch);
		m_ead &= 0x3ffff;
	}
}


//-------------------------------------------------
//  get_text_partition -
//-------------------------------------------------

inline void upd7220_device::get_text_partition(int index, UINT32 *sad, UINT16 *len, int *im, int *wd)
{
	*sad = ((m_ra[(index * 4) + 1] & 0x1f) << 8) | m_ra[(index * 4) + 0];
	*len = ((m_ra[(index * 4) + 3] & 0x3f) << 4) | (m_ra[(index * 4) + 2] >> 4);
	*im = BIT(m_ra[(index * 4) + 3], 6);
	*wd = BIT(m_ra[(index * 4) + 3], 7);
}


//-------------------------------------------------
//  get_graphics_partition -
//-------------------------------------------------

inline void upd7220_device::get_graphics_partition(int index, UINT32 *sad, UINT16 *len, int *im, int *wd)
{
	*sad = ((m_ra[(index * 4) + 2] & 0x03) << 16) | (m_ra[(index * 4) + 1] << 8) | m_ra[(index * 4) + 0];
	*len = ((m_ra[(index * 4) + 3] & 0x3f) << 4) | (m_ra[(index * 4) + 2] >> 4);
	*im = BIT(m_ra[(index * 4) + 3], 6);
	*wd = BIT(m_ra[(index * 4) + 3], 7);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  upd7220_device - constructor
//-------------------------------------------------

upd7220_device::upd7220_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, UPD7220, "uPD7220", tag, owner, clock, "upd7220", __FILE__),
	device_memory_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_write_drq(*this),
	m_write_hsync(*this),
	m_write_vsync(*this),
	m_write_blank(*this),
	m_mask(0),
	m_pitch(0),
	m_ead(0),
	m_dad(0),
	m_lad(0),
	m_ra_addr(0),
	m_sr(UPD7220_SR_FIFO_EMPTY),
	m_cr(0),
	m_param_ptr(0),
	m_fifo_ptr(-1),
	m_fifo_dir(0),
	m_mode(0),
	m_de(0),
	m_m(0),
	m_aw(0),
	m_al(0),
	m_vs(0),
	m_vfp(0),
	m_vbp(0),
	m_hs(0),
	m_hfp(0),
	m_hbp(0),
	m_dc(0),
	m_sc(0),
	m_br(0),
	m_ctop(0),
	m_cbot(0),
	m_lr(0),
	m_disp(0),
	m_gchr(0),
	m_bitmap_mod(0),
	m_space_config("videoram", ENDIANNESS_LITTLE, 16, 18, 0, NULL, *ADDRESS_MAP_NAME(upd7220_vram))
{
	for (int i = 0; i < 16; i++)
	{
		m_fifo[i] = 0;
		m_fifo_flag[i] = FIFO_EMPTY;

		m_ra[i] = 0;
	}

	for (int i = 0; i < 17; i++)
	{
		m_pr[i] = 0;
	}

	memset(&m_figs, 0x00, sizeof(m_figs));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd7220_device::device_start()
{
	// resolve callbacks
	m_display_cb.bind_relative_to(*owner());
	m_draw_text_cb.bind_relative_to(*owner());

	m_write_drq.resolve_safe();
	m_write_hsync.resolve_safe();
	m_write_vsync.resolve_safe();
	m_write_blank.resolve_safe();

	// allocate timers
	m_vsync_timer = timer_alloc(TIMER_VSYNC);
	m_hsync_timer = timer_alloc(TIMER_HSYNC);
	m_blank_timer = timer_alloc(TIMER_BLANK);

	// register for state saving
	save_item(NAME(m_ra));
	save_item(NAME(m_sr));
	save_item(NAME(m_mode));
	save_item(NAME(m_de));
	save_item(NAME(m_aw));
	save_item(NAME(m_al));
	save_item(NAME(m_vs));
	save_item(NAME(m_vfp));
	save_item(NAME(m_vbp));
	save_item(NAME(m_hs));
	save_item(NAME(m_hfp));
	save_item(NAME(m_hbp));
	save_item(NAME(m_m));
	save_item(NAME(m_dc));
	save_item(NAME(m_sc));
	save_item(NAME(m_br));
	save_item(NAME(m_lr));
	save_item(NAME(m_ctop));
	save_item(NAME(m_cbot));
	save_item(NAME(m_ead));
	save_item(NAME(m_dad));
	save_item(NAME(m_lad));
	save_item(NAME(m_disp));
	save_item(NAME(m_gchr));
	save_item(NAME(m_mask));
	save_item(NAME(m_pitch));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd7220_device::device_reset()
{
	m_write_drq(CLEAR_LINE);
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void upd7220_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_HSYNC:
		if (param)
		{
			m_sr |= UPD7220_SR_HBLANK_ACTIVE;
		}
		else
		{
			m_sr &= ~UPD7220_SR_HBLANK_ACTIVE;
		}

		m_write_hsync(param);

		update_hsync_timer(param);
		break;

	case TIMER_VSYNC:
		if (param)
		{
			m_sr |= UPD7220_SR_VSYNC_ACTIVE;
		}
		else
		{
			m_sr &= ~UPD7220_SR_VSYNC_ACTIVE;
		}

		m_write_vsync(param);

		update_vsync_timer(param);
		break;

	case TIMER_BLANK:
		if (param)
		{
			m_sr |= UPD7220_SR_HBLANK_ACTIVE;
		}
		else
		{
			m_sr &= ~UPD7220_SR_HBLANK_ACTIVE;
		}

		m_write_blank(param);

		update_blank_timer(param);
		break;
	}
}


//-------------------------------------------------
//  draw_pixel -
//-------------------------------------------------

void upd7220_device::draw_pixel(int x, int y, int xi, UINT16 tile_data)
{
	UINT32 addr = ((y * (m_pitch << (m_figs.m_gd ? 0 : 1))) + (x >> 3)) & 0x3ffff;
	UINT16 data = readword(addr);
	UINT16 new_pixel = (tile_data & (1 << (xi & 0xf))) ? (1 << (x & 0xf)) : 0;

	switch(m_bitmap_mod)
	{
		case 0: //replace
			writeword(addr, (data & ~(1 << (x & 0xf))) | new_pixel);
			break;
		case 1: //complement
			writeword(addr, data ^ new_pixel);
			break;
		case 2: //reset
			writeword(addr, data & ~new_pixel);
			break;
		case 3: //set
			writeword(addr, data | new_pixel);
			break;
	}
}


//-------------------------------------------------
//  draw_line -
//-------------------------------------------------

void upd7220_device::draw_line(int x, int y)
{
	int xi, yi;
	int d = (m_figs.m_d & 0x2000) ? (INT16)(m_figs.m_d | 0xe000) : m_figs.m_d;
	int d2 = (m_figs.m_d2 & 0x2000) ? (INT16)(m_figs.m_d2 | 0xe000) : m_figs.m_d2;
	UINT16 pattern = (m_ra[8]) | (m_ra[9]<<8);
	const int dot_dir[4] = {1, -1, -1, 1};

	LOG(("uPD7220 line check: %d %d %02x %08x %d %d %d\n",x,y,m_figs.m_dir,m_ead,m_figs.m_d1,m_figs.m_dc,m_bitmap_mod));

	for(yi = xi = 0; yi <= m_figs.m_dc; yi++)
	{
		switch(m_figs.m_dir & 3)
		{
			case 1:
			case 2:
				draw_pixel(yi * dot_dir[((m_figs.m_dir >> 1) + 3) & 3] + x, xi * dot_dir[m_figs.m_dir >> 1] + y, yi, pattern);
				break;
			default:
				draw_pixel(xi * dot_dir[((m_figs.m_dir >> 1) + 3) & 3] + x, yi * dot_dir[m_figs.m_dir >> 1] + y, yi, pattern);
				break;
		}
		if(d > 0)
		{
			xi++;
			d += d2;
		}
		else
			d += m_figs.m_d1;
	}

	switch(m_figs.m_dir & 3)
	{
		case 1:
		case 2:
			x += yi * dot_dir[((m_figs.m_dir >> 1) + 3) & 3];
			y += xi * dot_dir[m_figs.m_dir >> 1];
			break;
		default:
			x += xi * dot_dir[((m_figs.m_dir >> 1) + 3) & 3];
			y += yi * dot_dir[m_figs.m_dir >> 1];
			break;
	}

	m_ead = (x >> 4) + (y * (m_pitch >> m_figs.m_gd));
	m_dad = x & 0x0f;
}

//-------------------------------------------------
//  draw_arc -
//-------------------------------------------------

void upd7220_device::draw_arc(int x, int y)
{
	int xi = m_figs.m_d + 1, yi = 0, err = -m_figs.m_d;
	int x0, y0;
	UINT16 pattern = (m_ra[8]) | (m_ra[9]<<8);
	const int dot_dir[4] = {1, -1, -1, 1};

	switch(m_figs.m_dir & 3)
	{
		case 1:
		case 2:
			x0 = x;
			y0 = y + xi * dot_dir[m_figs.m_dir >> 1];
			break;
		default:
			x0 = x + xi * dot_dir[((m_figs.m_dir >> 1) + 3) & 3];
			y0 = y;
			break;
	}

	LOG(("uPD7220 arc check: %d %d %02x %08x %d %d %d\n",x,y,m_figs.m_dir,m_ead,m_figs.m_dm,m_figs.m_dc,m_figs.m_d));

	for(int i = 0; i <= m_figs.m_dc; i++)
	{
		if(i >= m_figs.m_dm)
		{
			switch(m_figs.m_dir & 3)
			{
				case 1:
				case 2:
					draw_pixel(yi * dot_dir[((m_figs.m_dir >> 1) + 3) & 3] + x0, xi * dot_dir[m_figs.m_dir >> 1] + y0, i, pattern);
					break;
				default:
					draw_pixel(xi * dot_dir[m_figs.m_dir >> 1] + x0, yi * dot_dir[((m_figs.m_dir >> 1) + 3) & 3] + y0, i, pattern);
					break;
			}
		}
		yi++;
		if(err < 0)
			err += (yi + 1) << 1;
		else
		{
			xi--;
			err += (yi - xi + 1) << 1;
		}
	}
	switch(m_figs.m_dir & 3)
	{
		case 1:
		case 2:
			x += (m_figs.m_dc + 1) * dot_dir[((m_figs.m_dir >> 1) + 3) & 3];
			break;
		default:
			y += (m_figs.m_dc + 1) * dot_dir[m_figs.m_dir >> 1];
			break;
	}

	m_ead = (x >> 4) + (y * (m_pitch >> m_figs.m_gd));
	m_dad = x & 0x0f;
}

//-------------------------------------------------
//  draw_rectangle -
//-------------------------------------------------

void upd7220_device::draw_rectangle(int x, int y)
{
	int i;
	const int rect_x_dir[8] = { 0, 1, 0,-1, 1, 1,-1,-1 };
	const int rect_y_dir[8] = { 1, 0,-1, 0, 1,-1,-1, 1 };
	UINT8 rect_type,rect_dir;
	UINT16 pattern = (m_ra[8]) | (m_ra[9]<<8);

	LOG(("uPD7220 rectangle check: %d %d %02x %08x\n",x,y,m_figs.m_dir,m_ead));

	rect_type = (m_figs.m_dir & 1) << 2;
	rect_dir = rect_type | (((m_figs.m_dir >> 1) + 0) & 3);

	for(i = 0;i < m_figs.m_d;i++)
	{
		draw_pixel(x,y,i,pattern);
		x+=rect_x_dir[rect_dir];
		y+=rect_y_dir[rect_dir];
	}

	rect_dir = rect_type | (((m_figs.m_dir >> 1) + 1) & 3);

	for(i = 0;i < m_figs.m_d2;i++)
	{
		draw_pixel(x,y,i,pattern);
		x+=rect_x_dir[rect_dir];
		y+=rect_y_dir[rect_dir];
	}

	rect_dir = rect_type | (((m_figs.m_dir >> 1) + 2) & 3);

	for(i = 0;i < m_figs.m_d;i++)
	{
		draw_pixel(x,y,i,pattern);
		x+=rect_x_dir[rect_dir];
		y+=rect_y_dir[rect_dir];
	}

	rect_dir = rect_type | (((m_figs.m_dir >> 1) + 3) & 3);

	for(i = 0;i < m_figs.m_d2;i++)
	{
		draw_pixel(x,y,i,pattern);
		x+=rect_x_dir[rect_dir];
		y+=rect_y_dir[rect_dir];
	}

	m_ead = (x >> 4) + (y * (m_pitch >> m_figs.m_gd));
	m_dad = x & 0x0f;

}


//-------------------------------------------------
//  draw_char -
//-------------------------------------------------

void upd7220_device::draw_char(int x, int y)
{
	int isize,psize;
	UINT16 tile_data = 0;

	LOG(("uPD7220 char check: %d %d %02x %08x %d %d %02x\n",x,y,m_figs.m_dir,m_ead,m_figs.m_d,m_figs.m_dc,m_figs.m_figure_type));

	isize = m_figs.m_d & 0x3ff;
	/* Guess: D has presumably upper bits for ysize, QX-10 relies on this (TODO: check this on any real HW) */
	psize = ((m_figs.m_d & 0x400) + m_figs.m_dc) + 1;

	for(int pi = 0; pi < psize; pi++)
	{
		tile_data = (m_ra[((psize-1-pi) & 7) | 8] << 8) | m_ra[((psize-1-pi) & 7) | 8];
		for(int pz = 0; pz <= m_gchr; pz++)
		{
			for(int ii = 0, curpixel = 0; ii < isize; ii++)
			{
				for(int iz = 0; iz <= m_gchr; iz++)
				{
					draw_pixel(x + (curpixel * x_dir[m_figs.m_dir]), y + (curpixel * y_dir[m_figs.m_dir]), ii, tile_data);
					curpixel++;
				}
			}
			if(m_figs.m_figure_type == 2)
			{
				x += x_dir[(m_figs.m_dir + 2) & 7];
				y += y_dir[(m_figs.m_dir + 2) & 7];
			}
			else
			{
				x += x_dir[(m_figs.m_dir + 1) & 7];
				y += y_dir[(m_figs.m_dir + 1) & 7];
			}
		}
	}

	m_ead = (x >> 4) + (y * (m_pitch >> m_figs.m_gd));
	m_dad = (x & 0xf);
}


//-------------------------------------------------
//  translate_command -
//-------------------------------------------------

int upd7220_device::translate_command(UINT8 data)
{
	int command = COMMAND_INVALID;

	switch (data)
	{
	case UPD7220_COMMAND_RESET: command = COMMAND_RESET; break;
	case UPD7220_COMMAND_CCHAR: command = COMMAND_CCHAR; break;
	case UPD7220_COMMAND_START: command = COMMAND_START; break;
	case UPD7220_COMMAND_ZOOM:  command = COMMAND_ZOOM;  break;
	case UPD7220_COMMAND_CURS:  command = COMMAND_CURS;  break;
	case UPD7220_COMMAND_PITCH: command = COMMAND_PITCH; break;
	case UPD7220_COMMAND_MASK:  command = COMMAND_MASK;  break;
	case UPD7220_COMMAND_FIGS:  command = COMMAND_FIGS;  break;
	case UPD7220_COMMAND_FIGD:  command = COMMAND_FIGD;  break;
	case UPD7220_COMMAND_GCHRD: command = COMMAND_GCHRD; break;
	case UPD7220_COMMAND_CURD:  command = COMMAND_CURD;  break;
	case UPD7220_COMMAND_LPRD:  command = COMMAND_LPRD;  break;
	case UPD7220_COMMAND_5A:    command = COMMAND_5A;    break;
	default:
		switch (data & 0xfe)
		{
		case UPD7220_COMMAND_SYNC:  command = COMMAND_SYNC;  break;
		case UPD7220_COMMAND_VSYNC: command = COMMAND_VSYNC; break;
		case UPD7220_COMMAND_BCTRL: command = COMMAND_BCTRL; break;
		default:
			switch (data & 0xf0)
			{
			case UPD7220_COMMAND_PRAM: command = COMMAND_PRAM; break;
			default:
				switch (data & 0xe4)
				{
				case UPD7220_COMMAND_WDAT: command = COMMAND_WDAT; break;
				case UPD7220_COMMAND_RDAT: command = COMMAND_RDAT; break;
				case UPD7220_COMMAND_DMAR: command = COMMAND_DMAR; break;
				case UPD7220_COMMAND_DMAW: command = COMMAND_DMAW; break;
				}
			}
		}
	}

	return command;
}


//-------------------------------------------------
//  process_fifo -
//-------------------------------------------------

void upd7220_device::process_fifo()
{
	UINT8 data;
	int flag;
	UINT16 eff_pitch = m_pitch >> m_figs.m_gd;

	dequeue(&data, &flag);

	if (flag == FIFO_COMMAND)
	{
		m_cr = data;
		m_param_ptr = 1;
	}
	else
	{
		m_pr[m_param_ptr] = data;
		m_param_ptr++;
	}

	switch (translate_command(m_cr))
	{
	case COMMAND_INVALID:
		logerror("uPD7220 '%s' Invalid Command Byte %02x\n", tag(), m_cr);
		break;

	case COMMAND_5A:
		if (m_param_ptr == 4)
			logerror("uPD7220 '%s' Undocumented Command 0x5A Executed %02x %02x %02x\n", tag(),m_pr[1],m_pr[2],m_pr[3] );
		break;

	case COMMAND_RESET: /* reset */
		switch (m_param_ptr)
		{
		case 0:
			LOG(("uPD7220 '%s' RESET\n", tag()));

			m_de = 0;
			m_ra[0] = m_ra[1] = m_ra[2] = 0;
			m_ra[3] = 0x19;
			m_ead = 0;
			m_dad = 0;
			m_mask = 0;
			break;

		case 9:
			m_mode = m_pr[1];
			m_aw = m_pr[2] + 2;
			m_hs = (m_pr[3] & 0x1f) + 1;
			m_vs = ((m_pr[4] & 0x03) << 3) | (m_pr[3] >> 5);
			m_hfp = (m_pr[4] >> 2) + 1;
			m_hbp = (m_pr[5] & 0x3f) + 1;
			m_vfp = m_pr[6] & 0x3f;
			m_al = ((m_pr[8] & 0x03) << 8) | m_pr[7];
			m_vbp = m_pr[8] >> 2;

			m_pitch = m_aw;

			LOG(("uPD7220 '%s' Mode: %02x\n", tag(), m_mode));
			LOG(("uPD7220 '%s' AW: %u\n", tag(), m_aw));
			LOG(("uPD7220 '%s' HS: %u\n", tag(), m_hs));
			LOG(("uPD7220 '%s' VS: %u\n", tag(), m_vs));
			LOG(("uPD7220 '%s' HFP: %u\n", tag(), m_hfp));
			LOG(("uPD7220 '%s' HBP: %u\n", tag(), m_hbp));
			LOG(("uPD7220 '%s' VFP: %u\n", tag(), m_vfp));
			LOG(("uPD7220 '%s' AL: %u\n", tag(), m_al));
			LOG(("uPD7220 '%s' VBP: %u\n", tag(), m_vbp));
			LOG(("uPD7220 '%s' PITCH: %u\n", tag(), m_pitch));

			recompute_parameters();
			break;
		}
		break;

	case COMMAND_SYNC: /* sync format specify */
		if (m_param_ptr == 9)
		{
			m_mode = m_pr[1];
			m_aw = m_pr[2] + 2;
			m_hs = (m_pr[3] & 0x1f) + 1;
			m_vs = ((m_pr[4] & 0x03) << 3) | (m_pr[3] >> 5);
			m_hfp = (m_pr[4] >> 2) + 1;
			m_hbp = (m_pr[5] & 0x3f) + 1;
			m_vfp = m_pr[6] & 0x3f;
			m_al = ((m_pr[8] & 0x03) << 8) | m_pr[7];
			m_vbp = m_pr[8] >> 2;

			m_pitch = m_aw;

			LOG(("uPD7220 '%s' Mode: %02x\n", tag(), m_mode));
			LOG(("uPD7220 '%s' AW: %u\n", tag(), m_aw));
			LOG(("uPD7220 '%s' HS: %u\n", tag(), m_hs));
			LOG(("uPD7220 '%s' VS: %u\n", tag(), m_vs));
			LOG(("uPD7220 '%s' HFP: %u\n", tag(), m_hfp));
			LOG(("uPD7220 '%s' HBP: %u\n", tag(), m_hbp));
			LOG(("uPD7220 '%s' VFP: %u\n", tag(), m_vfp));
			LOG(("uPD7220 '%s' AL: %u\n", tag(), m_al));
			LOG(("uPD7220 '%s' VBP: %u\n", tag(), m_vbp));
			LOG(("uPD7220 '%s' PITCH: %u\n", tag(), m_pitch));

			recompute_parameters();
		}
		break;

	case COMMAND_VSYNC: /* vertical sync mode */
		m_m = m_cr & 0x01;

		LOG(("uPD7220 '%s' M: %u\n", tag(), m_m));

		recompute_parameters();
		break;

	case COMMAND_CCHAR: /* cursor & character characteristics */
		if(m_param_ptr == 2)
		{
			m_lr = (m_pr[1] & 0x1f) + 1;
			m_dc = BIT(m_pr[1], 7);

			LOG(("uPD7220 '%s' LR: %u\n", tag(), m_lr));
			LOG(("uPD7220 '%s' DC: %u\n", tag(), m_dc));
		}

		if(m_param_ptr == 3)
		{
			m_ctop = m_pr[2] & 0x1f;
			m_sc = BIT(m_pr[2], 5);
			m_br = (m_pr[2] >> 6); /* guess, assume that blink rate clears upper bits (if any) */

			LOG(("uPD7220 '%s' CTOP: %u\n", tag(), m_ctop));
			LOG(("uPD7220 '%s' SC: %u\n", tag(), m_sc));
		}

		if(m_param_ptr == 4)
		{
			m_br = ((m_pr[3] & 0x07) << 2) | (m_pr[2] >> 6);
			m_cbot = m_pr[3] >> 3;

			LOG(("uPD7220 '%s' BR: %u\n", tag(), m_br));
			LOG(("uPD7220 '%s' CBOT: %u\n", tag(), m_cbot));
		}
		break;

	case COMMAND_START: /* start display & end idle mode */
		m_de = 1;

		//LOG(("uPD7220 '%s' DE: 1\n", tag()));
		break;

	case COMMAND_BCTRL: /* display blanking control */
		m_de = m_cr & 0x01;

		//LOG(("uPD7220 '%s' DE: %u\n", tag(), m_de));
		break;

	case COMMAND_ZOOM: /* zoom factors specify */
		if (flag == FIFO_PARAMETER)
		{
			m_gchr = m_pr[1] & 0x0f;
			m_disp = m_pr[1] >> 4;

			LOG(("uPD7220 '%s' GCHR: %01x\n", tag(), m_gchr));
			LOG(("uPD7220 '%s' DISP: %01x\n", tag(), m_disp));
		}
		break;

	case COMMAND_CURS: /* cursor position specify */
		if (m_param_ptr >= 3)
		{
			UINT8 upper_addr = (m_param_ptr == 3) ? 0 : (m_pr[3] & 0x03);

			m_ead = (upper_addr << 16) | (m_pr[2] << 8) | m_pr[1];

			LOG(("uPD7220 '%s' EAD: %06x\n", tag(), m_ead));

			if(m_param_ptr == 4)
			{
				m_dad = m_pr[3] >> 4;
				LOG(("uPD7220 '%s' DAD: %01x\n", tag(), m_dad));
			}
		}
		break;

	case COMMAND_PRAM: /* parameter RAM load */
		if (flag == FIFO_COMMAND)
		{
			m_ra_addr = m_cr & 0x0f;
		}
		else
		{
			if (m_ra_addr < 16)
			{
				LOG(("uPD7220 '%s' RA%u: %02x\n", tag(), m_ra_addr, data));

				m_ra[m_ra_addr] = data;
				m_ra_addr++;
			}

			m_param_ptr = 0;
		}
		break;

	case COMMAND_PITCH: /* pitch specification */
		if (flag == FIFO_PARAMETER)
		{
			m_pitch = data;

			LOG(("uPD7220 '%s' PITCH: %u\n", tag(), m_pitch));
		}
		break;

	case COMMAND_WDAT: /* write data into display memory */
		m_bitmap_mod = m_cr & 3;

		if (m_param_ptr == 3 || (m_param_ptr == 2 && m_cr & 0x10))
		{
			LOG(("%02x = %02x %02x (%c) %06x %04x\n",m_cr,m_pr[2],m_pr[1],m_pr[1]?m_pr[1]:' ',m_ead,m_figs.m_dc));
			fifo_set_direction(FIFO_WRITE);

			write_vram((m_cr & 0x18) >> 3,m_cr & 3);
			reset_figs_param();
			m_param_ptr = 1;
		}
		break;

	case COMMAND_MASK: /* mask register load */
		if (m_param_ptr == 3)
		{
			m_mask = (m_pr[2] << 8) | m_pr[1];

			LOG(("uPD7220 '%s' MASK: %04x\n", tag(), m_mask));
		}
		break;

	case COMMAND_FIGS: /* figure drawing parameters specify */
		if (m_param_ptr == 2)
		{
			m_figs.m_dir = m_pr[1] & 0x7;
			m_figs.m_figure_type = (m_pr[1] & 0xf8) >> 3;

			//if(m_figs.m_dir != 2)
			//  printf("DIR %02x\n",m_pr[1]);
		}

		// the Decision Mate V during start-up test upload only 2 params before execute the
		// RDAT command, so I assume this is the expected behaviour, but this needs to be verified.
		if (m_param_ptr == 3)
			m_figs.m_dc = (m_pr[2]) | (m_figs.m_dc & 0x3f00);

		if (m_param_ptr == 4)
		{
			m_figs.m_dc = (m_pr[2]) | ((m_pr[3] & 0x3f) << 8);
			m_figs.m_gd = (m_pr[3] & 0x40) && ((m_mode & UPD7220_MODE_DISPLAY_MASK) == UPD7220_MODE_DISPLAY_MIXED);
		}

		if (m_param_ptr == 6)
			m_figs.m_d = (m_pr[4]) | ((m_pr[5] & 0x3f) << 8);

		if (m_param_ptr == 8)
			m_figs.m_d2 = (m_pr[6]) | ((m_pr[7] & 0x3f) << 8);

		if (m_param_ptr == 10)
			m_figs.m_d1 = (m_pr[8]) | ((m_pr[9] & 0x3f) << 8);

		if (m_param_ptr == 12)
			m_figs.m_dm = (m_pr[10]) | ((m_pr[11] & 0x3f) << 8);

		break;

	case COMMAND_FIGD: /* figure draw start */
		if(m_figs.m_figure_type == 0)
			draw_pixel(((m_ead % eff_pitch) << 4) | (m_dad & 0xf),(m_ead / eff_pitch),m_dad,(m_ra[8]) | (m_ra[9]<<8));
		else if(m_figs.m_figure_type == 1)
			draw_line(((m_ead % eff_pitch) << 4) | (m_dad & 0xf),(m_ead / eff_pitch));
		else if(m_figs.m_figure_type == 4)
			draw_arc(((m_ead % eff_pitch) << 4) | (m_dad & 0xf),(m_ead / eff_pitch));
		else if(m_figs.m_figure_type == 8)
			draw_rectangle(((m_ead % eff_pitch) << 4) | (m_dad & 0xf),(m_ead / eff_pitch));
		else
			logerror("uPD7220 '%s' Unimplemented command FIGD %02x\n", tag(),m_figs.m_figure_type);

		reset_figs_param();
		m_sr |= UPD7220_SR_DRAWING_IN_PROGRESS;
		break;

	case COMMAND_GCHRD: /* graphics character draw and area filling start */
		if((m_figs.m_figure_type & 0xf) == 2)
			draw_char(((m_ead % eff_pitch) << 4) | (m_dad & 0xf),(m_ead / eff_pitch));
		else
			logerror("uPD7220 '%s' Unimplemented command GCHRD %02x\n", tag(),m_figs.m_figure_type);

		reset_figs_param();
		m_sr |= UPD7220_SR_DRAWING_IN_PROGRESS;
		break;

	case COMMAND_RDAT: /* read data from display memory */
		fifo_set_direction(FIFO_READ);

		read_vram((m_cr & 0x18) >> 3,m_cr & 3);

		m_sr |= UPD7220_SR_DATA_READY;
		break;

	case COMMAND_CURD: /* cursor address read */
	{
		UINT16 dad = 1 << m_dad;
		fifo_set_direction(FIFO_READ);

		queue(m_ead & 0xff, 0);
		queue((m_ead >> 8) & 0xff, 0);
		queue(m_ead >> 16, 0);
		queue(dad & 0xff, 0);
		queue(dad >> 8, 0);

		m_sr |= UPD7220_SR_DATA_READY;
		break;
	}

	case COMMAND_LPRD: /* light pen address read */
		fifo_set_direction(FIFO_READ);

		queue(m_lad & 0xff, 0);
		queue((m_lad >> 8) & 0xff, 0);
		queue(m_lad >> 16, 0);

		m_sr |= UPD7220_SR_DATA_READY;
		m_sr &= ~UPD7220_SR_LIGHT_PEN_DETECT;
		break;

	case COMMAND_DMAR: /* DMA read request */
		logerror("uPD7220 '%s' Unimplemented command DMAR\n", tag());
		break;

	case COMMAND_DMAW: /* DMA write request */
		logerror("uPD7220 '%s' Unimplemented command DMAW\n", tag());
		break;
	}
}


//-------------------------------------------------
//  continue command
//-------------------------------------------------

void upd7220_device::continue_command()
{
	// continue RDAT command when data to read are larger than the FIFO (a5105 and dmv text scrolling)
	if (m_figs.m_dc && translate_command(m_cr) == COMMAND_RDAT)
	{
		read_vram((m_cr & 0x18) >> 3, m_cr & 3);
		m_sr |= UPD7220_SR_DATA_READY;
	}
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

READ8_MEMBER( upd7220_device::read )
{
	UINT8 data;

	if (offset & 1)
	{
		/* FIFO read */
		int flag;
		fifo_set_direction(FIFO_READ);
		dequeue(&data, &flag);

		continue_command();
	}
	else
	{
		/* status register */
		data = m_sr;

		/* TODO: timing of these */
		m_sr &= ~UPD7220_SR_DRAWING_IN_PROGRESS;
		m_sr &= ~UPD7220_SR_DMA_EXECUTE;
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

WRITE8_MEMBER( upd7220_device::write )
{
	if (offset & 1)
	{
		/* command into FIFO */
		fifo_set_direction(FIFO_WRITE);
		queue(data, 1);
	}
	else
	{
		/* parameter into FIFO */
//      fifo_set_direction(FIFO_WRITE);
		queue(data, 0);
	}

	process_fifo();
}


//-------------------------------------------------
//  dack_r -
//-------------------------------------------------

READ8_MEMBER( upd7220_device::dack_r )
{
	return 0;
}


//-------------------------------------------------
//  dack_w -
//-------------------------------------------------

WRITE8_MEMBER( upd7220_device::dack_w )
{
}


//-------------------------------------------------
//  ext_sync_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( upd7220_device::ext_sync_w )
{
	//LOG(("uPD7220 '%s' External Synchronization: %u\n", tag(), state));

	if (state)
	{
		m_sr |= UPD7220_SR_VSYNC_ACTIVE;
	}
	else
	{
		m_sr &= ~UPD7220_SR_VSYNC_ACTIVE;
	}
}


//-------------------------------------------------
//  ext_sync_w -
//-------------------------------------------------

WRITE_LINE_MEMBER( upd7220_device::lpen_w )
{
	/* only if 2 rising edges on the lpen input occur at the same
	   point during successive video fields are the pulses accepted */

	/*

	    1. compute the address of the location on the CRT
	    2. compare with LAD
	    3. if not equal move address to LAD
	    4. if equal set LPEN DETECT flag to 1

	*/
}


//-------------------------------------------------
//  update_text -
//-------------------------------------------------

void upd7220_device::update_text(bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 addr, sad;
	UINT16 len;
	int im, wd;
	int y, sy = 0;

	for (int area = 0; area < 4; area++)
	{
		get_text_partition(area, &sad, &len, &im, &wd);

		for (y = sy; y < sy + len; y++)
		{
			addr = sad + (y * m_pitch);

			if (!m_draw_text_cb.isnull())
				m_draw_text_cb(bitmap, addr, (y * m_lr) + m_vbp, wd, m_pitch, m_lr, m_dc, m_ead);
		}

		sy = y + 1;
	}
}


//-------------------------------------------------
//  draw_graphics_line -
//-------------------------------------------------

void upd7220_device::draw_graphics_line(bitmap_rgb32 &bitmap, UINT32 addr, int y, int wd, int pitch)
{
	int sx, al = bitmap.cliprect().height();

	for (sx = 0; sx < pitch; sx++)
	{
		if((sx << 4) < m_aw * 16 && y < al)
			m_display_cb(bitmap, y, sx << 4, addr);

		addr+= (wd + 1) * 2;
	}
}


//-------------------------------------------------
//  update_graphics -
//-------------------------------------------------

void upd7220_device::update_graphics(bitmap_rgb32 &bitmap, const rectangle &cliprect, int force_bitmap)
{
	UINT32 addr, sad;
	UINT16 len;
	int im, wd, area;
	int y = 0, tsy = 0, bsy = 0;
	bool mixed = ((m_mode & UPD7220_MODE_DISPLAY_MASK) == UPD7220_MODE_DISPLAY_MIXED);
	UINT8 interlace = ((m_mode & UPD7220_MODE_INTERLACE_MASK) == UPD7220_MODE_INTERLACE_ON) ? 0 : 1;

	for (area = 0; area < 4; area++)
	{
		get_graphics_partition(area, &sad, &len, &im, &wd);

		if (im || force_bitmap)
		{
			//get_graphics_partition(area, &sad, &len, &im, &wd);

			if(area >= 3) // TODO: most likely to be correct, Quarth (PC-98xx) definitely draws with area 2. We might see an area 3 someday ...
				break;

			if(!interlace)
				len <<= 1;

			for (y = 0; y < len; y++)
			{
				/* TODO: again correct?
				         Quarth (PC-98xx) doesn't seem to use pitch here and it definitely wants bsy to be /2 to make scrolling to work.
				         Xevious (PC-98xx) wants the pitch to be fixed at 80, and wants bsy to be /1
				         Dragon Buster (PC-98xx) contradicts with Xevious with regards of the pitch tho ... */
				addr = ((sad << 1) & 0x3ffff) + (y * (m_pitch << (im ? 0 : 1)));

				if (!m_display_cb.isnull())
					draw_graphics_line(bitmap, addr, y + ((bsy + m_vbp) / (mixed ? 1 : m_lr)), wd, (m_pitch << interlace));
			}
		}
		else
		{
			get_text_partition(area, &sad, &len, &im, &wd);

			if(m_lr)
			{
				for (y = 0; y < len; y+=m_lr)
				{
					addr = (sad & 0x3ffff) + ((y / m_lr) * m_pitch);

					if (!m_draw_text_cb.isnull())
						m_draw_text_cb(bitmap, addr, y + tsy + m_vbp, wd, m_pitch, m_lr, m_dc, m_ead);
				}
			}
		}

		if (m_lr)
			tsy += y;
		bsy += y;
	}
}


//-------------------------------------------------
//  update_screen -
//-------------------------------------------------

UINT32 upd7220_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_de)
	{
		switch (m_mode & UPD7220_MODE_DISPLAY_MASK)
		{
		case UPD7220_MODE_DISPLAY_MIXED:
			update_graphics(bitmap, cliprect, 0);
			break;

		case UPD7220_MODE_DISPLAY_GRAPHICS:
			update_graphics(bitmap, cliprect, 1);
			break;

		case UPD7220_MODE_DISPLAY_CHARACTER:
			update_text(bitmap, cliprect);
			break;

		case UPD7220_MODE_DISPLAY_INVALID:
			LOG(("uPD7220 '%s' Invalid Display Mode!\n", tag()));
		}
	}
	return 0;
}
