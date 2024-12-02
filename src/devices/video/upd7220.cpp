// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Miodrag Milanovic, Carl, Brian Johnson
/**************************************************************************************************

Intel 82720 Graphics Display Controller emulation
a.k.a. NEC uPD7220

TODO:
- implement FIFO as ring buffer;
- incomplete / unimplemented FIGD / GCHRD draw modes;
\- FIGD character
\- slanted character
- read-modify-write cycle (fixed by now?);
- a5105: has a FIFO bug with the RDAT, should be a lot larger when it scrolls up.
  Can be fixed with a DRDY mechanism for RDAT/WDAT (fixed by now?);
- later pc9801/pc9821 machines: throws "Invalid command byte 05" (zettmj on Epson logo),
  actual undocumented command to reset something?
- wide mode (32-bit access);
- microbx2: sets mode=0x1f (interlace), oddly cuts off area scroll on prompt (double height chars?)
  while DEMO.1 don't.
- repeat fields interlace mode;
- support non-draw on retrace mode (pc98 bitmap layer -> potential raster effects), better
  honoring of visible area (bail out thru m_al, cutting off unnecessary comparisons);
- light pen;

**************************************************************************************************/


#include "emu.h"
#include "upd7220.h"

#include "screen.h"

#define LOG_CRTC      (1U << 1)
#define LOG_DRAW      (1U << 2)
#define LOG_AREA      (1U << 3) // printout works best with -oslog
#define LOG_CMD       (1U << 4)
#define LOG_CMD2      (1U << 5) // DE related commands
#define LOG_CMD3      (1U << 6) // memory interface (RDAT/WDAT)
#define LOG_CMD4      (1U << 7) // DMA

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGCRTC(...)     LOGMASKED(LOG_CRTC, __VA_ARGS__)
#define LOGDRAW(...)     LOGMASKED(LOG_DRAW, __VA_ARGS__)
#define LOGAREA(...)     LOGMASKED(LOG_AREA, __VA_ARGS__)
#define LOGCMD(...)      LOGMASKED(LOG_CMD, __VA_ARGS__)
#define LOGCMD2(...)     LOGMASKED(LOG_CMD2, __VA_ARGS__)
#define LOGCMD3(...)     LOGMASKED(LOG_CMD3, __VA_ARGS__)
#define LOGCMD4(...)     LOGMASKED(LOG_CMD4, __VA_ARGS__)


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

// TODO: typedef
enum
{
	COMMAND_INVALID = -1,
	COMMAND_RESET,
	COMMAND_RESET2,
	COMMAND_RESET3,
	COMMAND_SYNC,
	COMMAND_VSYNC,
	COMMAND_CCHAR,
	COMMAND_START,
	COMMAND_BLANK,
	COMMAND_BLANK2,
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
#define UPD7220_COMMAND_RESET2          0x01
#define UPD7220_COMMAND_RESET3          0x09
#define UPD7220_COMMAND_SYNC            0x0e // & 0xfe
#define UPD7220_COMMAND_VSYNC           0x6e // & 0xfe
#define UPD7220_COMMAND_CCHAR           0x4b
#define UPD7220_COMMAND_START           0x6b
#define UPD7220_COMMAND_BLANK           0x0c // & 0xfe
#define UPD7220_COMMAND_BLANK2          0x05
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


static constexpr int x_dir[8] = { 0, 1, 1, 1, 0,-1,-1,-1};
static constexpr int y_dir[8] = { 1, 1, 0,-1,-1,-1, 0, 1};


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// devices
DEFINE_DEVICE_TYPE(UPD7220, upd7220_device, "upd7220", "NEC uPD7220")
DEFINE_DEVICE_TYPE(UPD7220A, upd7220a_device, "upd7220a", "NEC uPD7220A")


// default address map
void upd7220_device::upd7220_vram(address_map &map)
{
	if (!has_configured_map(0))
		map(0x00000, 0x3ffff).ram();
}


// internal 128x14 control ROM
// hand-dumped as little-endian from a die shot
ROM_START( upd7220 )
	ROM_REGION( 0x100, "upd7220", 0 )
	ROM_LOAD( "upd7220.bin", 0x000, 0x100, CRC(3c92b218) SHA1(e154b3106a80c9c98d9f2ee18efcd7f4b4aa7d49) )
ROM_END

ROM_START( upd7220a )
	ROM_REGION( 0x100, "upd7220", 0 )
	ROM_LOAD( "upd7220a.bin", 0x000, 0x100, NO_DUMP )
ROM_END


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector upd7220_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *upd7220_device::device_rom_region() const
{
	return ROM_NAME( upd7220 );
}

const tiny_rom_entry *upd7220a_device::device_rom_region() const
{
	return ROM_NAME( upd7220a );
}


//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

inline uint16_t upd7220_device::readword(offs_t address)
{
	return space().read_word(address);
}


inline void upd7220_device::writeword(offs_t address, uint16_t data)
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

inline void upd7220_device::queue(uint8_t data, int flag)
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

inline void upd7220_device::dequeue(uint8_t *data, int *flag)
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

		if (m_fifo_ptr <= 0)
			m_sr |= UPD7220_SR_FIFO_EMPTY;
		if (m_fifo_ptr == -1)
			m_sr &= ~UPD7220_SR_DATA_READY;
	}
	else
	{
		// TODO: underflow details
		// pc9821:skinpan does SR checks over the wrong port during intro ...
		*data = 0xff;
	}
}


//-------------------------------------------------
//  update_vsync_timer -
//-------------------------------------------------

inline void upd7220_device::update_vsync_timer(int state)
{
	const int vert_mult = (m_mode & UPD7220_MODE_INTERLACE_MASK) == UPD7220_MODE_INTERLACE_ON ? 2 : 1;

	// page 6-68, route vsync so that top is start of back porch, end is at sync time
	// (at bottom of MAME vpos() mechanism)
	// - pc9801:lemmings and pc9801:spindiz2 cares
	int next_y = state ? 0 : (m_vbp + m_al + m_vfp) * vert_mult;

	attotime duration = screen().time_until_pos(next_y, 0);

	m_vsync_timer->adjust(duration, !state);
}


//-------------------------------------------------
//  update_hsync_timer -
//-------------------------------------------------

inline void upd7220_device::update_hsync_timer(int state)
{
	const int horiz_mult = (m_mode & UPD7220_MODE_DISPLAY_MASK) == UPD7220_MODE_DISPLAY_GRAPHICS ? 16 : 8;
	int y = screen().vpos();

	int next_x = state ? m_hs * horiz_mult : 0;
	int next_y = state ? y : ((y + 1) % (m_vs + m_vbp + m_al + m_vfp - 1));

	attotime duration = screen().time_until_pos(next_y, next_x);

	m_hsync_timer->adjust(duration, !state);
}


//-------------------------------------------------
//  update_blank_timer -
//-------------------------------------------------

inline void upd7220_device::update_blank_timer(int state)
{
	int y = screen().vpos();

	int next_x = state ? (m_hs + m_hbp) : (m_hs + m_hbp + (m_aw << 3));
	int next_y = state ? ((y + 1) % (m_vs + m_vbp + m_al + m_vfp - 1)) : y;

	attotime duration = screen().time_until_pos(next_y, next_x);

	m_hsync_timer->adjust(duration, !state);
}


//-------------------------------------------------
//  recompute_parameters -
//-------------------------------------------------

void upd7220_device::device_clock_changed()
{
	recompute_parameters();
}

inline void upd7220_device::recompute_parameters()
{
	// microbx2 wants horizontal multiplier of x16
	// pc9801:diremono sets up m_mode to be specifically in character mode, wanting x8 here
	// TODO: verify compis uhrg video & high reso Hyper 98
	const int horiz_mult = (m_mode & UPD7220_MODE_DISPLAY_MASK) == UPD7220_MODE_DISPLAY_GRAPHICS ? 16 : 8;
	const int vert_mult = (m_mode & UPD7220_MODE_INTERLACE_MASK) == UPD7220_MODE_INTERLACE_ON ? 2 : 1;

	int horiz_pix_total = (m_hs + m_hbp + m_hfp + m_aw) * horiz_mult;
	int vert_pix_total = (m_vs + m_vbp + m_al + m_vfp) * vert_mult;

	//printf("%d %d %d %d\n",m_hs,m_hbp,m_aw,m_hfp);
	//printf("%d %d\n",m_aw * 8,m_pitch * 8);

	if (horiz_pix_total == 0 || vert_pix_total == 0) //bail out if screen params aren't valid
		return;

	attoseconds_t refresh = HZ_TO_ATTOSECONDS(clock() * 8) * horiz_pix_total * vert_pix_total;

	rectangle visarea(
			0, //(m_hs + m_hbp) * 8;
			m_aw * horiz_mult - 1,//horiz_pix_total - (m_hfp * 8) - 1;
			m_vbp, //m_vs + m_vbp;
			m_al * vert_mult + m_vbp - 1);//vert_pix_total - m_vfp - 1;

	LOGCRTC("New Screen setup: %u x %u @ %f Hz\n", horiz_pix_total, vert_pix_total, 1 / ATTOSECONDS_TO_DOUBLE(refresh));
	LOGCRTC("Visible Area: (%u, %u) - (%u, %u)\n", visarea.left(), visarea.top(), visarea.right(), visarea.bottom());
	//LOGCRTC("%d %d %d %d %d\n",m_hs,m_hbp,m_aw,m_hfp,m_pitch);
	//LOGCRTC("%d %d %d %d\n",m_vs,m_vbp,m_al,m_vfp);

	if (m_m)
	{
		screen().configure(horiz_pix_total, vert_pix_total, visarea, refresh);
		screen().reset_origin();

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
	m_figs.m_d1 = 0xffff;
	m_figs.m_d2 = 0x0008;
	m_figs.m_dm = 0xffff;
	m_figs.m_gd = 0;
	m_figs.m_figure_type = 0;
	m_pattern = (m_ra[8]) | (m_ra[9] << 8);
}


//-------------------------------------------------
//  read_vram -
//-------------------------------------------------
inline uint16_t upd7220_device::read_vram()
{
	uint16_t const data = readword(m_ead);
	m_ead += x_dir[m_figs.m_dir] + (y_dir[m_figs.m_dir] * get_pitch());
	m_ead &= 0x3ffff;

	return data;
}

inline void upd7220_device::rdat(uint8_t type, uint8_t mod)
{
	if (type == 1)
	{
		LOG("invalid type 1 RDAT parameter\n");
		return;
	}

	if (mod)
		LOG("RDAT used with mod = %02x?\n",mod);

	while (m_figs.m_dc && m_fifo_ptr < (type ? 15 : 14))
	{
		uint16_t const data = read_vram();
		switch(type)
		{
		case 0:
			queue(data & 0xff, 0);
			queue((data >> 8) & 0xff, 0);
			break;
		case 2:
			queue(data & 0xff, 0);
			break;
		case 3:
			queue((data >> 8) & 0xff, 0);
			break;
		}

		m_figs.m_dc--;
	}

	if (m_figs.m_dc == 0)
		reset_figs_param();
}

//-------------------------------------------------
//  write_vram -
//-------------------------------------------------
inline void upd7220_device::write_vram(uint8_t type, uint8_t mod, uint16_t data, uint16_t mask)
{
	uint16_t current = readword(m_ead);

	switch(mod & 3)
	{
	case 0x00: //replace
		switch (type)
		{
		case 0: current = (current & (~mask)) | (data & mask); break;
		case 2: current = (current & ~(mask & 0xff)) | (data & (mask & 0xff)); break;
		case 3: current = (current & ~(mask & 0xff00)) | (data & (mask & 0xff00)); break;
		}
		break;
	case 0x01: //complement
		switch (type)
		{
		case 0: current = current ^ (data & mask); break;
		case 2: current = current ^ (data & (mask & 0xff)); break;
		case 3: current = current ^ (data & (mask & 0xff00)); break;
		}
		break;
	case 0x02: //reset to zero
		switch (type)
		{
		case 0: current = current & ~(data & mask); break;
		case 2: current = current & ~(data & (mask & 0xff)); break;
		case 3: current = current & ~(data & (mask & 0xff00)); break;
		}
		break;
	case 0x03: //set to one
		switch (type)
		{
		case 0: current = current | (data & mask); break;
		case 2: current = current | (data & (mask & 0xff)); break;
		case 3: current = current | (data & (mask & 0xff00)); break;
		}
		break;
	}
	writeword(m_ead, current);
}

inline void upd7220_device::wdat(uint8_t type, uint8_t mod)
{
	if(type == 1)
	{
		logerror("uPD7220 invalid type 1 WDAT parameter\n");
		return;
	}

	uint16_t result = m_pr[1] | (m_pr[2] << 8);

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
		write_vram(type, mod, result);
		m_ead += x_dir[m_figs.m_dir] + (y_dir[m_figs.m_dir] * get_pitch());
		m_ead &= 0x3ffff;
	}
}


//-------------------------------------------------
//  get_text_partition -
//-------------------------------------------------

inline void upd7220_device::get_text_partition(int index, uint32_t *sad, uint16_t *len, int *im, int *wd)
{
	*sad = ((m_ra[(index * 4) + 1] & 0x1f) << 8) | m_ra[(index * 4) + 0];
	*len = ((m_ra[(index * 4) + 3] & 0x3f) << 4) | (m_ra[(index * 4) + 2] >> 4);
	*im = BIT(m_ra[(index * 4) + 3], 6);
	*wd = BIT(m_ra[(index * 4) + 3], 7);
}


//-------------------------------------------------
//  get_graphics_partition -
//-------------------------------------------------

inline void upd7220_device::get_graphics_partition(int index, uint32_t *sad, uint16_t *len, int *im, int *wd)
{
	*sad = ((m_ra[(index * 4) + 2] & 0x03) << 16) | (m_ra[(index * 4) + 1] << 8) | m_ra[(index * 4) + 0];
	*len = ((m_ra[(index * 4) + 3] & 0x3f) << 4) | (m_ra[(index * 4) + 2] >> 4);
	*im = BIT(m_ra[(index * 4) + 3], 6);
	*wd = BIT(m_ra[(index * 4) + 3], 7);
}

/*
 * experimental, for non-canonical stuff such as PC9821 PEGC
 */
std::tuple<u32, u16, u8> upd7220_device::get_area_partition_props(int line)
{
	uint32_t sad;
	uint16_t len;
	int im, wd;
	// TODO: multiareas (pc9821:skinpan title)
	get_graphics_partition(0, &sad, &len, &im, &wd);

	return std::make_tuple(sad, m_pitch, im);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  upd7220_device - constructor
//-------------------------------------------------

upd7220_device::upd7220_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	device_video_interface(mconfig, *this),
	m_display_cb(*this),
	m_draw_text_cb(*this),
	m_write_drq(*this),
	m_write_hsync(*this),
	m_write_vsync(*this),
	m_write_blank(*this),
	m_pattern(0),
	m_mask(0),
	m_pitch(0),
	m_ead(0),
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
	m_lr(1),
	m_disp(0),
	m_gchr(0),
	m_bitmap_mod(0),
	m_space_config("videoram", ENDIANNESS_LITTLE, 16, 18, -1, address_map_constructor(FUNC(upd7220_device::upd7220_vram), this))
{
	for (int i = 0; i < 16; i++)
	{
		m_fifo[i] = 0;
		m_fifo_flag[i] = FIFO_EMPTY;

		m_ra[i] = 0;
	}

	for (auto & elem : m_pr)
	{
		elem = 0;
	}

	memset(&m_figs, 0x00, sizeof(m_figs));
}

upd7220_device::upd7220_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: upd7220_device(mconfig, UPD7220, tag, owner, clock)
{
}

upd7220a_device::upd7220a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: upd7220_device(mconfig, UPD7220A, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd7220_device::device_start()
{
	// resolve callbacks
	m_display_cb.resolve_safe();
	m_draw_text_cb.resolve_safe();

	// allocate timers
	m_vsync_timer = timer_alloc(FUNC(upd7220_device::vsync_update), this);
	m_hsync_timer = timer_alloc(FUNC(upd7220_device::hsync_update), this);
	m_blank_timer = timer_alloc(FUNC(upd7220_device::blank_update), this);

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
	save_item(NAME(m_lad));
	save_item(NAME(m_disp));
	save_item(NAME(m_gchr));
	save_item(NAME(m_pattern));
	save_item(NAME(m_mask));
	save_item(NAME(m_pitch));
	save_item(NAME(m_ra_addr));
	save_item(NAME(m_cr));
	save_item(NAME(m_pr));
	save_item(NAME(m_param_ptr));
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_flag));
	save_item(NAME(m_fifo_ptr));
	save_item(NAME(m_fifo_dir));
	save_item(NAME(m_bitmap_mod));
	save_item(NAME(m_figs.m_dir));
	save_item(NAME(m_figs.m_figure_type));
	save_item(NAME(m_figs.m_dc));
	save_item(NAME(m_figs.m_gd));
	save_item(NAME(m_figs.m_d));
	save_item(NAME(m_figs.m_d1));
	save_item(NAME(m_figs.m_d2));
	save_item(NAME(m_figs.m_dm));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void upd7220_device::device_reset()
{
	m_write_drq(CLEAR_LINE);
}


//-------------------------------------------------
//  timer events
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(upd7220_device::hsync_update)
{
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
}

TIMER_CALLBACK_MEMBER(upd7220_device::vsync_update)
{
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
}

TIMER_CALLBACK_MEMBER(upd7220_device::blank_update)
{
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
}


//-------------------------------------------------
//  draw_pixel -
//-------------------------------------------------

void upd7220_device::draw_pixel()
{
	LOGDRAW("draw_pixel(): EAD=%08x MASK=%04x MOD=%02x - FIGS DIR=%02x DC=%d\n", m_ead, m_mask, m_bitmap_mod, m_figs.m_dir, m_figs.m_dc);

	for(int i = 0; i <= m_figs.m_dc; ++i)
	{
		const uint16_t pattern = get_pattern(i & 0xf);
		write_vram(0, m_bitmap_mod, pattern, m_mask);
		next_pixel(m_figs.m_dir);
	}
}


//-------------------------------------------------
//  draw_line -
//-------------------------------------------------

void upd7220_device::draw_line()
{
	int d = util::sext(m_figs.m_d, 14);
	int d1 = util::sext(m_figs.m_d1, 14);
	int d2 = util::sext(m_figs.m_d2, 14);
	const uint8_t octant = m_figs.m_dir;

	LOGDRAW("draw_line(): EAD=%08x MASK=%04x MOD=%02x - FIGS DIR=%02x DC=%d D=%d D1=%d D2=%d\n", m_ead, m_mask, m_bitmap_mod, m_figs.m_dir, m_figs.m_dc, d, d1, d2);

	for(int i = 0; i <= m_figs.m_dc; ++i)
	{
		const uint16_t pattern = get_pattern(i & 0xf);
		write_vram(0, m_bitmap_mod, pattern, m_mask);
		if (octant & 1)
		{
			m_figs.m_dir = (d < 0) ? (octant + 1) & 7 : octant;
		}
		else
		{
			m_figs.m_dir = (d < 0) ? octant : (octant + 1) & 7;
		}
		d += ((d < 0) ? d1 : d2);
		next_pixel(m_figs.m_dir);
	}
}


//-------------------------------------------------
//  draw_arc -
//-------------------------------------------------

void upd7220_device::draw_arc()
{
	int err = -m_figs.m_d, d = m_figs.m_d + 1;

	const uint16_t octant = m_figs.m_dir;

	LOGDRAW("draw_arc(): EAD=%08x MASK=%04x MOD=%02x - FIGS DIR=%02x DC=%d D=%d D2=%d DM=%d\n", m_ead, m_mask, m_bitmap_mod, m_figs.m_dir, m_figs.m_dc, m_figs.m_d, m_figs.m_d2, m_figs.m_dm);

	for (int i = 0; i <= m_figs.m_dc; ++i)
	{
		const uint16_t pattern = get_pattern(i % 0xf);
		if (i >= m_figs.m_dm)
		{
			write_vram(0, m_bitmap_mod, pattern, m_mask);
		}
		if (err < 0)
		{
			m_figs.m_dir = (octant & 1) ? (octant + 1) & 7 : octant;
		}
		else
		{
			m_figs.m_dir = (octant & 1) ? octant : (octant + 1) & 7;
		}
		err += (err < 0) ? ((i + 1) << 1) : ((i - --d + 1) << 1);
		next_pixel(m_figs.m_dir);
	}
}


//-------------------------------------------------
//  draw_rectangle -
//-------------------------------------------------

void upd7220_device::draw_rectangle()
{
	assert(m_figs.m_dc == 3);

	LOGDRAW("draw_rectangle(): EAD=%08x MASK=%04x MOD=%02x - FIGS DIR=%02x DC=%d D=%d D2=%d\n",m_ead, m_mask, m_bitmap_mod, m_figs.m_dir, m_figs.m_dc, m_figs.m_d, m_figs.m_d2);

	for (int i = 0; i <= m_figs.m_dc; ++i)
	{
		const uint16_t dist = (i & 1 ? m_figs.m_d2 : m_figs.m_d);
		for (int j = 0; j < dist; ++j)
		{
			const uint16_t pattern = get_pattern(j & 0xf);
			write_vram(0, m_bitmap_mod, pattern, m_mask);
			if (i > 0 && j == 0)
				m_figs.m_dir = (m_figs.m_dir + 2) & 7;
			next_pixel(m_figs.m_dir);
		}

	}
}


//-------------------------------------------------
//  draw_char -
//-------------------------------------------------

void upd7220_device::draw_char()
{
	const int8_t dir_change[2][4] = {
		{2, 2, -2, -2},
		{1, 3, -3, -1}
	};
	const uint8_t type = (m_figs.m_figure_type & 0x10) >> 4;

	LOGDRAW("draw_char(): EAD=%08x MASK=%04x MOD=%02x DIR=%02x D=%d DC=%d FIGURE=%02x\n",m_ead,m_mask,m_bitmap_mod,m_figs.m_dir,m_figs.m_d,m_figs.m_dc,m_figs.m_figure_type);

	for (int i = 0, di = 0; i < (m_figs.m_dc + 1); ++i)
	{
		m_pattern = (m_ra[15 - (i & 7)] << 8) | m_ra[15 - (i & 7)];
		for (int zdc = 0; zdc <= m_gchr; ++zdc, ++di)
		{
			for (int j = 0; j < m_figs.m_d; ++j)
			{
				const uint16_t pattern = (di % 2) ? get_pattern(15-(j & 0xf)) : get_pattern((j & 0xf));
				for (int zd = 0; zd <= m_gchr; ++zd)
				{
					write_vram(0, m_bitmap_mod, pattern, m_mask);

					if (j != (m_figs.m_d - 1) || zd != m_gchr)
						next_pixel(m_figs.m_dir);
				}
			}
			m_figs.m_dir = (((uint16_t)m_figs.m_dir + dir_change[type][(di % 2) << 1]) & 7);
			next_pixel(m_figs.m_dir);
			m_figs.m_dir = (((uint16_t)m_figs.m_dir + dir_change[type][((di % 2) << 1)+1]) & 7);
		}
	}
}


//-------------------------------------------------
//  helper functions used to rotate a uint16_t
//-------------------------------------------------

static constexpr uint16_t rotate_right(uint16_t value)
{
	return (value>>1) | (value<<( (-1) & 0x0f ));
}

static constexpr uint16_t rotate_left(uint16_t value)
{
	return (value<<1) | (value>>( (-1) & 0x0f ));
}


//-------------------------------------------------
//  get_pitch -
//-------------------------------------------------

uint16_t upd7220_device::get_pitch()
{
	bool mixed = ((m_mode & UPD7220_MODE_DISPLAY_MASK) == UPD7220_MODE_DISPLAY_MIXED);

	if (mixed)
		return m_pitch >> m_figs.m_gd;

	return m_pitch;
}


//-------------------------------------------------
//  get_pattern -
//-------------------------------------------------

uint16_t upd7220_device::get_pattern(uint8_t cycle)
{
	bool mixed = ((m_mode & UPD7220_MODE_DISPLAY_MASK) == UPD7220_MODE_DISPLAY_MIXED);
	bool graphics = ((m_mode & UPD7220_MODE_DISPLAY_MASK) == UPD7220_MODE_DISPLAY_GRAPHICS);

	if ((mixed && m_figs.m_gd) || graphics)
	{
		return ((m_pattern >> (cycle & 0xf)) & 1) * 0xffff;
	}
	return m_pattern;
}


//-------------------------------------------------
//  next_pixel -
//-------------------------------------------------

void upd7220_device::next_pixel(int direction)
{
	switch(direction & 7)
	{
	case 0:
		m_ead += get_pitch();
		break;
	case 1:
		m_ead += get_pitch();
		if (m_mask & 0x8000)
		{
			m_ead += 1;
		}
		m_mask = rotate_left(m_mask);
		break;
	case 2:
		if (m_mask & 0x8000)
		{
			m_ead += 1;
		}
		m_mask = rotate_left(m_mask);
		break;
	case 3:
		m_ead -= get_pitch();
		if (m_mask & 0x8000)
		{
			m_ead += 1;
		}
		m_mask = rotate_left(m_mask);
		break;
	case 4:
		m_ead -= get_pitch();
		break;
	case 5:
		m_ead -= get_pitch();
		if (m_mask & 0x1)
		{
			m_ead -= 1;
		}
		m_mask = rotate_right(m_mask);
		break;
	case 6:
		if (m_mask & 0x1)
		{
			m_ead -= 1;
		}
		m_mask = rotate_right(m_mask);
		break;
	case 7:
		m_ead += get_pitch();
		if (m_mask & 0x1)
		{
			m_ead -= 1;
		}
		m_mask = rotate_right(m_mask);
		break;
	}
	m_ead &= 0x3ffff;
}


//-------------------------------------------------
//  translate_command -
//-------------------------------------------------

int upd7220_device::translate_command(uint8_t data)
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
		case UPD7220_COMMAND_BLANK: command = COMMAND_BLANK; break;
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

int upd7220a_device::translate_command(uint8_t data)
{
	int command = upd7220_device::translate_command(data);

	switch (data)
	{
	case UPD7220_COMMAND_RESET2: command = COMMAND_RESET2; break;
	case UPD7220_COMMAND_RESET3: command = COMMAND_RESET3; break;
	case UPD7220_COMMAND_BLANK2: command = COMMAND_BLANK2; break;
	}

	return command;
}


//-------------------------------------------------
//  process_fifo -
//-------------------------------------------------

void upd7220_device::process_fifo()
{
	uint8_t data;
	int flag;
	int cr;

	dequeue(&data, &flag);

	if (flag == FIFO_COMMAND)
	{
		cr = translate_command(data);
		if (cr != COMMAND_BLANK) // workaround for Rainbow 100 Windows 1.03, needs verification
		{
			m_cr = data;
			m_param_ptr = 1;
		}
	}
	else
	{
		cr = translate_command(m_cr);
		m_pr[m_param_ptr] = data;
		m_param_ptr++;
	}

	switch (cr)
	{
	case COMMAND_INVALID:
		logerror("uPD7220 Invalid Command Byte %02x\n", m_cr);
		break;

	case COMMAND_5A:
		if (m_param_ptr == 4)
			logerror("uPD7220 Undocumented Command 0x5A Executed %02x %02x %02x\n", m_pr[1],m_pr[2],m_pr[3] );
		break;

	case COMMAND_RESET: /* reset */
	case COMMAND_RESET2:
	case COMMAND_RESET3:
		switch (m_param_ptr)
		{
		case 1:
			LOGCMD("RESET %02x\n", cr);

			if (cr != COMMAND_RESET3)
				m_de = 0;
			// NOTE: a reset doesn't touch anything belonging to parameters, as stated in docs.
			// pc9801:mightyhd won't program sad/len but just issue a reset after BIOS
			// (which originally sets len = 0x1ff)
			// with this on, it will len = 400 lines, repeating the bottommost two rows in a 432 setup.
			//m_ra[0] = m_ra[1] = m_ra[2] = 0;
			//m_ra[3] = 0x19;
			m_ead = 0;
			// TODO: very unlikely, should be 0xffff assuming it's even touched ...
			m_mask = 0;
			// FIFO, Command Processor and internal counters are cleared by this
			// - pc9801rs BIOS starts up a DMAW command that spindiz2 will dislike during its boot sequences
			m_sr &= ~UPD7220_SR_DRAWING_IN_PROGRESS;
			fifo_clear();
			stop_dma();
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

			if (type() == UPD7220A)
			{
				m_pitch = ((m_pr[5] & 0x40) << 2) | (m_pitch & 0xff);
				m_al += (m_pr[6] >> 6) & 1;
			}

			LOGCMD("- Mode: %02x\n", m_mode);
			LOGCMD("- HS=%u HBP=%u AW=%u HFP=%u PITCH=%u\n", m_hs, m_hbp, m_aw, m_hfp, m_pitch);
			LOGCMD("- VS=%u VBP=%u AL=%u VFP=%u\n", m_vs, m_vbp, m_al, m_vfp);

			recompute_parameters();
			break;
		}
		break;

	case COMMAND_SYNC: /* sync format specify */
		if (flag == FIFO_COMMAND)
		{
			m_de = m_cr & 1;
			LOGCMD2("SYNC %u\n", m_de);
		}

		// TODO: merge implementation with above
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

			if (type() == UPD7220A)
			{
				m_pitch = ((m_pr[5] & 0x40) << 2) | (m_pitch & 0xff);
				m_al += (m_pr[6] >> 6) & 1;
			}

			LOGCMD("- Mode: %02x\n", m_mode);
			LOGCMD("- HS=%u HBP=%u AW=%u HFP=%u PITCH=%u\n", m_hs, m_hbp, m_aw, m_hfp, m_pitch);
			LOGCMD("- VS=%u VBP=%u AL=%u VFP=%u\n", m_vs, m_vbp, m_al, m_vfp);

			recompute_parameters();
		}
		break;

	case COMMAND_VSYNC: /* vertical sync mode */
		m_m = m_cr & 0x01;

		LOGCMD("VSYNC M=%u\n", m_m);

		recompute_parameters();
		break;

	case COMMAND_CCHAR: /* cursor & character characteristics */
		switch (m_param_ptr)
		{
		case 2:
			m_lr = (m_pr[1] & 0x1f) + 1;
			m_dc = BIT(m_pr[1], 7);

			LOGCMD("CCHAR LR=%u DC=%u\n", m_lr, m_dc);
			break;

		case 3:
			m_ctop = m_pr[2] & 0x1f;
			m_sc = BIT(m_pr[2], 5);
			m_br = (m_pr[2] >> 6); /* guess, assume that blink rate clears upper bits (if any) */

			LOGCMD("- CTOP=%u SC=%u\n", m_ctop, m_sc);
			break;

		case 4:
			m_br = ((m_pr[3] & 0x07) << 2) | (m_pr[2] >> 6);
			m_cbot = m_pr[3] >> 3;

			LOGCMD("- BR=%u CBOT=%u\n", m_br, m_cbot);
			break;
		}
		break;

	case COMMAND_START: /* start display & end idle mode */
		m_de = 1;

		LOGCMD2("START\n");
		break;

	case COMMAND_BLANK2:
		m_de = 0;
		LOGCMD2("BLANK2\n");

		break;

	case COMMAND_BLANK: /* display blanking control */
		m_de = data & 0x01;

		LOGCMD2("BLANK %u\n", m_de);
		break;

	case COMMAND_ZOOM: /* zoom factors specify */
		if (flag == FIFO_PARAMETER)
		{
			m_gchr = m_pr[1] & 0x0f;
			m_disp = m_pr[1] >> 4;

			LOGCMD("ZOOM GCHR=%01x DISP=%01x\n", m_gchr, m_disp);
		}
		break;

	case COMMAND_CURS: /* cursor position specify */
		if (m_param_ptr >= 3)
		{
			uint8_t upper_addr = (m_param_ptr == 3) ? 0 : (m_pr[3] & 0x03);

			m_ead = (upper_addr << 16) | (m_pr[2] << 8) | m_pr[1];

			LOGCMD("CURS EAD: %06x\n", m_ead);

			if(m_param_ptr == 4)
			{
				m_mask = 1 << ((m_pr[3] >> 4) & 0x0f);
				LOGCMD("- MASK: %04x\n", m_mask);
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
				LOGCMD("PRAM RA%u: %02x\n", m_ra_addr, data);

				switch (m_ra_addr)
				{
				case 8: m_pattern = (m_pattern & 0xff00) | data; break;
				case 9: m_pattern = (m_pattern & 0x00ff) | (data << 8); break;
				}
				m_ra[m_ra_addr] = data;
				m_ra_addr++;
			}

			m_param_ptr = 0;
		}
		break;

	case COMMAND_PITCH: /* pitch specification */
		// pc9801:burai writes a spurious extra value during intro, effectively ignored
		// (only first value matters)
		if (flag == FIFO_PARAMETER && m_param_ptr == 2)
		{
			m_pitch = (m_pitch & 0x100) | data;

			if (m_pitch < 2)
			{
				// TODO: a pitch of zero will lead to a MAME crash in draw_graphics_line
				// Coerce a fail-safe minimum, what should really happen is to be verified ...
				popmessage("%s pitch == 0!", this->tag());
				m_pitch = 2;
			}

			LOGCMD("PITCH %u\n", m_pitch);
		}
		break;

	case COMMAND_WDAT: /* write data into display memory */
		m_bitmap_mod = m_cr & 3;

		if (m_param_ptr == 3 || (m_param_ptr == 2 && m_cr & 0x10))
		{
			m_pattern = (m_pattern & 0xff00) | m_pr[1];
			if (m_param_ptr == 3)
				m_pattern = (m_pattern & 0xff) | (m_pr[2] << 8);
			LOGCMD3("WDAT PATTERN=%04x\n", m_pattern);
			if (m_figs.m_figure_type)
				break;
			LOGCMD3("- CR=%02x (%02x %02x) (%c) EAD=%06x - FIGS DC=%04x\n"
				, m_cr
				, m_pr[2]
				, m_pr[1]
				, m_pr[1] ? m_pr[1]:' '
				, m_ead
				, m_figs.m_dc
			);
			fifo_set_direction(FIFO_WRITE);

			wdat((m_cr & 0x18) >> 3,m_cr & 3);
			reset_figs_param();
			m_param_ptr = 1;
		}
		break;

	case COMMAND_MASK: /* mask register load */
		if (m_param_ptr == 3)
		{
			m_mask = (m_pr[2] << 8) | m_pr[1];

			LOGCMD("MASK %04x\n", m_mask);
		}
		break;

	case COMMAND_FIGS: /* figure drawing parameters specify */
		switch (m_param_ptr)
		{
		case 2:
			m_figs.m_dir = m_pr[1] & 0x7;
			m_figs.m_figure_type = (m_pr[1] & 0xf8) >> 3;

			LOGCMD("FIGS DIR=%02x FIGURE=%02x\n", m_figs.m_dir, m_figs.m_figure_type);
			//if(m_figs.m_dir != 2)
			//  printf("DIR %02x\n",m_pr[1]);
			break;

			// the Decision Mate V during start-up test upload only 2 params before execute the
			// RDAT command, so I assume this is the expected behaviour, but this needs to be verified.
		case 3:
			m_figs.m_dc = (m_pr[2]) | (m_figs.m_dc & 0x3f00);
			LOGCMD("- DC=%04x (*)\n", m_figs.m_dc);
			break;

		case 4:
			m_figs.m_dc = (m_pr[2]) | ((m_pr[3] & 0x3f) << 8);
			m_figs.m_gd = (m_pr[3] & 0x40) && ((m_mode & UPD7220_MODE_DISPLAY_MASK) == UPD7220_MODE_DISPLAY_MIXED);
			LOGCMD("- DC=%04x\n", m_figs.m_dc);
			LOGCMD("- GD=%02x\n", m_figs.m_gd);
			break;

		case 6:
			m_figs.m_d = (m_pr[4]) | ((m_pr[5] & 0x3f) << 8);
			LOGCMD("- D=%04x\n", m_figs.m_d);
			break;

		case 8:
			m_figs.m_d2 = (m_pr[6]) | ((m_pr[7] & 0x3f) << 8);
			LOGCMD("- D2=%04x\n", m_figs.m_d2);
			break;

		case 10:
			m_figs.m_d1 = (m_pr[8]) | ((m_pr[9] & 0x3f) << 8);
			LOGCMD("- D1=%04x\n", m_figs.m_d1);
			break;

		case 12:
			m_figs.m_dm = (m_pr[10]) | ((m_pr[11] & 0x3f) << 8);
			LOGCMD("- DM=%04x\n", m_figs.m_dm);
			break;
		}
		break;

	case COMMAND_FIGD: /* figure draw start */
		LOGCMD("FIGD %u\n", m_figs.m_figure_type);
		switch (m_figs.m_figure_type)
		{
		case 0: draw_pixel(); break;
		case 1: draw_line(); break;
		case 4: draw_arc(); break;
		case 8: draw_rectangle(); break;
		default:
			popmessage("%s Unimplemented command FIGD %02x\n", this->tag(), m_figs.m_figure_type);
			break;
		}
		reset_figs_param();
		m_sr |= UPD7220_SR_DRAWING_IN_PROGRESS;
		break;

	case COMMAND_GCHRD: /* graphics character draw and area filling start */
		LOGCMD("GCHRD %u\n", m_figs.m_figure_type);

		if((m_figs.m_figure_type & 0xf) == 2)
			draw_char();
		else
			popmessage("%s Unimplemented command GCHRD %02x\n", this->tag(), m_figs.m_figure_type);

		reset_figs_param();
		m_sr |= UPD7220_SR_DRAWING_IN_PROGRESS;
		break;

	case COMMAND_RDAT: /* read data from display memory */
		LOGCMD3("RDAT %06x %02x\n", m_ead, m_cr);

		fifo_set_direction(FIFO_READ);

		rdat((m_cr & 0x18) >> 3,m_cr & 3);

		m_sr |= UPD7220_SR_DATA_READY;
		break;

	case COMMAND_CURD: /* cursor address read */
		LOGCMD3("CURD %06x\n", m_ead, m_mask);

		fifo_set_direction(FIFO_READ);

		queue(m_ead & 0xff, 0);
		queue((m_ead >> 8) & 0xff, 0);
		queue(m_ead >> 16, 0);
		queue(m_mask & 0xff, 0);
		queue(m_mask >> 8, 0);

		m_sr |= UPD7220_SR_DATA_READY;
		break;

	case COMMAND_LPRD: /* light pen address read */
		LOGCMD3("LPRD %06x\n", m_lad);

		fifo_set_direction(FIFO_READ);

		queue(m_lad & 0xff, 0);
		queue((m_lad >> 8) & 0xff, 0);
		queue(m_lad >> 16, 0);

		m_sr |= UPD7220_SR_DATA_READY;
		m_sr &= ~UPD7220_SR_LIGHT_PEN_DETECT;
		break;

	case COMMAND_DMAR: /* DMA read request */
		LOGCMD4("DMAR CR=%02x DC=%d D=%d\n", m_cr, m_figs.m_dc, m_figs.m_d);
		m_dma_type = (m_cr >> 3) & 3;
		m_dma_mod = m_cr & 3;
		m_dma_transfer_length = (m_figs.m_dc + 1) * (m_figs.m_d + 2);
		start_dma();
		break;

	case COMMAND_DMAW: /* DMA write request */
		LOGCMD4("DMAW CR=%02x DC=%d D=%d\n", m_cr, m_figs.m_dc, m_figs.m_d);
		m_dma_type = (m_cr >> 3) & 3;
		m_dma_mod = m_cr & 3;
		m_dma_transfer_length = (m_figs.m_dc + 1) * (m_figs.m_d + 1);
		start_dma();
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
		rdat((m_cr & 0x18) >> 3, m_cr & 3);
		m_sr |= UPD7220_SR_DATA_READY;
	}
}


//-------------------------------------------------
//  read -
//-------------------------------------------------

uint8_t upd7220_device::read(offs_t offset)
{
	uint8_t data;

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
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void upd7220_device::write(offs_t offset, uint8_t data)
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

uint8_t upd7220_device::dack_r()
{
	uint8_t result = 0;
	switch(m_dma_type)
	{
	case 0:
		if (m_dma_transfer_length % 2 == 0)
		{
			m_dma_data = read_vram();
			result = m_dma_data & 0xff;
		}
		else
		{
			result = (m_dma_data >> 8) & 0xff;
		}
		break;
	case 2:
		m_dma_data = read_vram();
		result = m_dma_data & 0xff;
		break;
	case 3:
		m_dma_data = read_vram();
		result = (m_dma_data >> 8) & 0xff;
		break;
	default:
		logerror("uPD7220 Invalid DMA Transfer Type\n");
	}
	if (--m_dma_transfer_length == 0)
	{
		stop_dma();
	}
	return result;
}


//-------------------------------------------------
//  dack_w -
//-------------------------------------------------

void upd7220_device::dack_w(uint8_t data)
{
	switch(m_dma_type)
	{
	case 0:
		if (m_dma_transfer_length % 2)
		{
			m_dma_data = ((m_dma_data & 0xff) | data << 8) & m_mask;
			write_vram(m_dma_type, m_dma_mod, m_dma_data);
			m_ead += x_dir[m_figs.m_dir] + (y_dir[m_figs.m_dir] * get_pitch());
			m_ead &= 0x3ffff;
		}
		else
		{
			m_dma_data = (m_dma_data & 0xff00) | data;
		}
		break;
	case 2:
		m_dma_data = data & (m_mask & 0xff);
		write_vram(m_dma_type, m_dma_mod, m_dma_data);
		m_ead += x_dir[m_figs.m_dir] + (y_dir[m_figs.m_dir] * get_pitch());
		m_ead &= 0x3ffff;
		break;
	case 3:
		m_dma_data = (data << 8) & (m_mask & 0xff00);
		write_vram(m_dma_type, m_dma_mod, m_dma_data);
		m_ead += x_dir[m_figs.m_dir] + (y_dir[m_figs.m_dir] * get_pitch());
		m_ead &= 0x3ffff;
		break;
	default:
		logerror("uPD7220 Invalid DMA Transfer Type\n");
	}
	if (--m_dma_transfer_length == 0)
	{
		stop_dma();
	}
}

void upd7220_device::start_dma()
{
	if ((m_sr & UPD7220_SR_DMA_EXECUTE) == 0)
	{
		m_write_drq(ASSERT_LINE);
		m_sr |= UPD7220_SR_DMA_EXECUTE;
	}
}

void upd7220_device::stop_dma()
{
	if ((m_sr & UPD7220_SR_DMA_EXECUTE) == UPD7220_SR_DMA_EXECUTE)
	{
		m_write_drq(CLEAR_LINE);
		m_sr &= ~UPD7220_SR_DMA_EXECUTE;
		reset_figs_param();
	}
}


//-------------------------------------------------
//  ext_sync_w -
//-------------------------------------------------

void upd7220_device::ext_sync_w(int state)
{
	//LOG("uPD7220 External Synchronization: %u\n", state);

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

void upd7220_device::lpen_w(int state)
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
	uint32_t sad;
	uint16_t len;
	int im, wd;
	int sy = 0;

	for (int area = 0; area < 4; area++)
	{
		get_text_partition(area, &sad, &len, &im, &wd);

		int y;
		for (y = sy; y < sy + len; y++)
		{
			uint32_t const addr = sad + (y * m_pitch);
			m_draw_text_cb(bitmap, addr, (y * m_lr) + m_vbp, wd, m_pitch, m_lr, m_dc, m_ead, m_ctop, m_cbot);
		}

		sy = y + 1;
	}
}


//-------------------------------------------------
//  draw_graphics_line -
//-------------------------------------------------

// TODO: merge with update_graphics
void upd7220_device::draw_graphics_line(bitmap_rgb32 &bitmap, uint32_t addr, int y, int wd, int mixed)
{
	int al = bitmap.cliprect().height();
	int aw = m_aw >> mixed;
	int pitch = m_pitch >> mixed;

	for (int sx = 0; sx < aw; sx++)
	{
		if(sx < m_aw && y < al)
			m_display_cb(bitmap, y, sx << 4, addr + (wd + 1) * (sx % pitch));
	}
}


//-------------------------------------------------
//  update_graphics -
//-------------------------------------------------

void upd7220_device::update_graphics(bitmap_rgb32 &bitmap, const rectangle &cliprect, int force_bitmap)
{
	uint32_t sad;
	uint16_t len;
	int im, wd;
	int y = 0, tsy = 0, bsy = 0;
	int mixed = ((m_mode & UPD7220_MODE_DISPLAY_MASK) == UPD7220_MODE_DISPLAY_MIXED) ? 1 : 0;
	uint8_t interlace = ((m_mode & UPD7220_MODE_INTERLACE_MASK) == UPD7220_MODE_INTERLACE_ON) ? 1 : 0;
	uint8_t zoom = m_disp + 1;

	LOGAREA("FRAME=%d MODE=%02x FORCE BITMAP=%d ZOOM=%02x PITCH=%d\n", screen().frame_number(), m_mode, force_bitmap, zoom, m_pitch);

	for(int area = 0; area < 4; area++)
	{
		get_graphics_partition(area, &sad, &len, &im, &wd);

		LOGAREA("%s: AREA=%d BSY=%4d SAD=%06x len=%04x im=%d wd=%d\n", this->tag(), area, bsy, sad, len, im, wd);

		// pc9821:aitd (256 color mode) and pc9821:os2warp3 (16, installation screens) wants this shift
		const u8 pitch_shift = force_bitmap ? im : mixed;

		if(im || force_bitmap)
		{
			// according to documentation only areas 0-1-2 can be drawn in bitmap mode
			// - pc98:quarth definitely needs area 2 for player section.
			// - pc98:steamhea wants area 3 for scrolling and dialogue screens to work together,
			//   contradicting the doc. Fixed in 7220A or applies just for mixed mode?
			// TODO: what happens if combined area size is smaller than display height?
			// documentation suggests that it should repeat from area 0, needs real HW verification (no known SW do it).
			if (area >= 3 && !force_bitmap)
				break;

			// pc98:madoum1-3 sets up ALL areas to a length of 0 after initial intro screen.
			// madoum1: area 0 sad==0 on gameplay (PC=0x955e7), sad==0xaa0 on second intro screen (tower) then intentionally scrolls up and back to initial position.
			// Suggests that length should be treated as max size if this occurs, this is also proven to be correct via real HW verification.
			// TODO: check if text partition do the same.
			if (len == 0)
				len = 0x400;

			if (interlace)
				len <<= 1;

			for(y = 0; y < len; y++)
			{
				// TODO: not completely correct, all is drawn half size with real HW tests on pc98 msdos by just changing PRAM values.
				// pc98 quarth doesn't seem to use pitch here and it definitely wants bsy to be /2 to make scrolling to work.
				// pc98 xevious wants the pitch to be fixed at 80, and wants bsy to be /1
				// pc98 dbuster contradicts with Xevious with regards of the pitch tho ...
				uint32_t const addr = (sad & 0x3ffff) + ((y / (mixed ? 1 : m_lr)) * (m_pitch >> pitch_shift));
				for(int z = 0; z <= m_disp; ++z)
				{
					int yval = (y*zoom)+z + (bsy + m_vbp);
					// pc9801:duel sets up bitmap layer with height 384 vs. 400 of text layer
					// so we scissor here, interlace wants it bumped x2 (microbx2)
					if(yval >= cliprect.top() && yval <= cliprect.bottom() && (yval - m_vbp) < m_al << interlace)
						draw_graphics_line(bitmap, addr, yval, wd, pitch_shift);
				}
			}
		}
		else
		{
			if(m_lr)
			{
				for(y = 0; y < len; y += m_lr)
				{
					uint32_t const addr = (sad & 0x3ffff) + ((y / m_lr) * m_pitch);
					int yval = y * zoom + (tsy + m_vbp);
					m_draw_text_cb(bitmap, addr, yval, wd, m_pitch, m_lr, m_dc, m_ead, m_ctop, m_cbot);
				}
			}
		}

		if (m_lr)
			tsy += y * zoom;
		bsy += y * zoom;
	}
}


//-------------------------------------------------
//  update_screen -
//-------------------------------------------------

uint32_t upd7220_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
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
			popmessage("%s Invalid Display Mode!", this->tag());
		}
	}
	return 0;
}
