// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Silicon Graphics RE2 device.
 *
 * TODO:
 *   - z buffer r/w and function
 *   - stipple, dither, raster operations
 *   - other pixel modes
 *   - line drawing
 */

#include "emu.h"
#include "debugger.h"
#include "sgi_re2.h"

#define LOG_GENERAL   (1U << 0)
#define LOG_REG       (1U << 1)

//#define VERBOSE       (LOG_GENERAL|LOG_REG)
#include "logmacro.h"

static char const *const regname[] =
{
	nullptr,     nullptr,     nullptr,     nullptr,     "ENABRGB",   "BIGENDIAN", "FUNC",      "HADDR",
	"NOPUP",     "XYFRAC",    "RGB",       "YX",        "PUPDATA",   "PATL",      "PATH",      "DZI",
	"DZF",       "DR",        "DG",        "DB",        "Z",         "R",         "G",         "B",
	"STIP",      "STIPCOUNT", "DX",        "DY",        "NUMPIX",    "X",         "Y",         "IR",

	"RWDATA",    "PIXMASK",   "AUXMASK",   "WIDDATA",   "UAUXDATA",  "RWMODE",    "READBUF",   "PIXTYPE",
	"ASELECT",   "ALIGNPAT",  "ENABPAT",   "ENABSTIP",  "ENABDITH",  "ENABWID",   "CURWID",    "DEPTHFN",
	"REPSTIP",   "ENABLWID",  "FBOPTION",  "TOPSCAN",   "TESTMODE",  "TESTDATA",  "ZBOPTION",  "XZOOM",
	"UPACMODE",  "YMIN",      "YMAX",      "XMIN",      "XMAX",      "COLORCMP",  "MEGOPTION", nullptr,
};

static u32 const regmask[] =
{
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0x00000001, 0x0000000f, 0x00000003,
	0x00000001, 0x0000000f, 0x07ffffff, 0x003fffff, 0x00000003, 0x0000ffff, 0x0000ffff, 0x00ffffff,
	0x00003fff, 0x00ffffff, 0x000fffff, 0x000fffff, 0x00ffffff, 0x007fffff, 0x0007ffff, 0x0007ffff,
	0x0000ffff, 0x000000ff, 0x0000ffff, 0x0000ffff, 0x000007ff, 0x0000ffff, 0x000007ff, 0x00000007,

	0xffffffff, 0x00ffffff, 0x000001ff, 0x0000000f, 0x0000000f, 0x00000007, 0x00000001, 0x00000003,
	0x0000003f, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x0000000f, 0x0000000f,
	0x000000ff, 0x00000001, 0x00000003, 0x0003ffff, 0x00000001, 0x00003fff, 0x00000001, 0x000000ff,
	0x00000003, 0x000007ff, 0x000007ff, 0x00000fff, 0x00000fff, 0x00000001, 0x00000001, 0x00000000,
};

DEFINE_DEVICE_TYPE(SGI_RE2, sgi_re2_device, "sgi_re2", "SGI Raster Engine 2")

sgi_re2_device::sgi_re2_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, SGI_RE2, tag, owner, clock)
	, m_xmap(*this, "^xmap%u", 0U)
	, m_cursor(*this, "^cursor%u", 0U)
	, m_ramdac(*this, "^ramdac%u", 0U)
	, m_options_port(*this, "^options")
	, m_rdy_cb(*this)
	, m_drq_cb(*this)
	, m_rdy(false)
	, m_drq(false)
{
}

void sgi_re2_device::device_start()
{
	m_rdy_cb.resolve();
	m_drq_cb.resolve();

	m_vram = std::make_unique<u32[]>(1280 * 1024);
	m_dram = std::make_unique<u32[]>(1280 * 1024);

	// save state
	for (unsigned i = 0; i < ARRAY_LENGTH(m_reg); i++)
		if (regmask[i])
			save_item(m_reg[i], regname[i]);

	set_rdy(true);
	set_drq(false);

	m_step = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sgi_re2_device::step), this));
}

void sgi_re2_device::device_reset()
{
	u8 const options = m_options_port->read();
	m_vram_mask = 0xffffffffU;
	m_dram_mask = (options & 0x10) ? 0xffffffff : 0;

	m_state = IDLE;
	m_ir_pending = false;

	set_rdy(true);
	set_drq(false);

	for (u32 &reg : m_reg)
		reg = 0;

	// reset register values indicate presence of RE2
	m_reg[REG_DZF] = 0;
	m_reg[REG_DZF] = ~u32(0);

	// FIXME: how is clipping disabled on reset?
	m_clip.set(0, 1279, 0, 1023);
}

u32 sgi_re2_device::reg_r(offs_t offset)
{
	u32 data = 0xffffffff & regmask[offset];

	// only some registers can be read
	switch (offset)
	{
	case REG_RWDATA:
		if (m_state == DMA_R)
		{
			data = m_reg[REG_RWDATA];
			read_buffer();
			step();
		}
		else
			logerror("rwdata read when empty\n");
		break;

	case REG_DZI:
	case REG_DZF:
	case REG_STIP:
	case REG_STIPCOUNT:
		data = m_reg[offset];
		LOGMASKED(LOG_REG, "reg_r register 0x%02x (%s) data 0x%x\n", offset, regname[offset], data);
		break;

	default:
		logerror("reg_r unhandled register 0x%02x\n", offset);
		break;
	}

	return data;
}

void sgi_re2_device::reg_w(offs_t offset, u32 data)
{
	if (regmask[offset])
	{
		m_reg[offset] = data & regmask[offset];

		if (offset != REG_RWDATA)
			LOGMASKED(LOG_REG, "reg_w register 0x%02x (%s) data 0x%x\n", offset, regname[offset], m_reg[offset]);

		// special case register handling
		switch (offset)
		{
		case REG_RGB:
			m_reg[REG_B] = (data & 0x000000ff) << 11;
			m_reg[REG_G] = (data & 0x0000ff00) << 3;
			m_reg[REG_R] = (data & 0x0fff0000) >> 5;
			break;

		case REG_YX:
			m_reg[REG_X] = (data & 0x00000fff) >> 0;
			m_reg[REG_Y] = (data & 0x007ff000) >> 12;
			break;

		case REG_IR:
			m_ir_pending = true;
			if (m_state == IDLE)
				step();
			break;

		case REG_RWDATA:
			if (m_state == DMA_W)
			{
				write_buffer();
				step();
			}
			break;

		case REG_YMIN:
		case REG_YMAX:
		case REG_XMIN:
		case REG_XMAX:
			m_clip.set(
				(m_reg[REG_XMIN] >> 3) * 5 + (m_reg[REG_XMIN] & 0x7),
				(m_reg[REG_XMAX] >> 3) * 5 + (m_reg[REG_XMAX] & 0x7),
				m_reg[REG_YMIN],
				m_reg[REG_YMAX]);
			break;
		}
	}
	else
		logerror("reg_w unhandled register 0x%02x data 0x%x\n", offset, data);
}

void sgi_re2_device::step(void *ptr, int param)
{
	switch (m_state)
	{
	case IDLE:
		set_rdy(!m_ir_pending);
		if (m_ir_pending)
		{
			m_ir_pending = false;
			m_state = EXECUTE;
			m_step->adjust(attotime::zero);
		}
		break;

	case EXECUTE:
		execute();
		if (m_state == IDLE)
			step();
		break;

	default:
		break;
	}
}

void sgi_re2_device::execute()
{
	// load buffered registers
	m_enabrgb = bool(m_reg[REG_ENABRGB]);
	m_bigendian = bool(m_reg[REG_BIGENDIAN]);
	m_func[0] = BIT(m_reg[REG_FUNC], 0) ? ~u32(0) : 0;
	m_func[1] = BIT(m_reg[REG_FUNC], 1) ? ~u32(0) : 0;
	m_func[2] = BIT(m_reg[REG_FUNC], 2) ? ~u32(0) : 0;
	m_func[3] = BIT(m_reg[REG_FUNC], 3) ? ~u32(0) : 0;
	// TODO: haddr
	m_nopup = bool(m_reg[REG_NOPUP]);
	// TODO: xyfrac
	m_pupdata = m_reg[REG_PUPDATA];
	m_pat = (m_reg[REG_PATH] << 16) | m_reg[REG_PATL];
	m_dz = (s64(u64(m_reg[REG_DZI]) << 40) >> 26) | m_reg[REG_DZF];
	m_dr = s32(m_reg[REG_DR] << 8) >> 8;
	m_dg = s32(m_reg[REG_DG] << 12) >> 12;
	m_db = s32(m_reg[REG_DB] << 12) >> 12;
	m_z = s64(u64(m_reg[REG_Z]) << 40) >> 26;
	m_r = m_reg[REG_R];
	m_g = m_reg[REG_G];
	m_b = m_reg[REG_B];
	m_stip = m_reg[REG_STIP];
	m_stipcount = m_reg[REG_STIPCOUNT];
	m_dx = s32(s16(m_reg[REG_DX]));
	m_dy = s32(s16(m_reg[REG_DY]));
	m_numpix = m_reg[REG_NUMPIX];
	m_x = ((m_reg[REG_X] >> 3) * 5 + (m_reg[REG_X] & 0x7)) << 14;
	m_y = m_reg[REG_Y] << 14;
	m_ir = m_reg[REG_IR];

	switch (m_ir)
	{
	case IR_SHADED:
		LOG("ri draw shaded span\n");
		draw_shaded_span();
		break;

	case IR_FLAT:
		LOG("ri draw 1x5 flat span\n");
		draw_flat_span(5);
		break;

	case IR_FLAT4:
		LOG("ri draw 1x20 flat span\n");
		draw_flat_span(20);
		break;

	case IR_TOPLINE:
		LOG("ri draw top of anti-aliased line\n");
		m_state = IDLE;
		break;

	case IR_BOTLINE:
		LOG("ri draw bottom of anti-aliased line\n");
		m_state = IDLE;
		break;

	case IR_READBUF:
		LOG("ri read buffer rwmode %d\n", m_reg[REG_RWMODE]);
		read_buffer();
		break;

	case IR_WRITEBUF:
		LOG("write buffer rwmode %d\n", m_reg[REG_RWMODE]);
		write_buffer();
		break;
	}
}

void sgi_re2_device::draw_shaded_span()
{
/*
 * raster operation logic:
 *
    u32 const result =
        (m_func[0] & src & dst) +
        (m_func[1] & src & ~dst) +
        (m_func[2] & ~src & dst) +
        (m_func[3] & ~src & ~dst);
*/

	u32 const mask = (m_reg[REG_AUXMASK] << 24) | m_reg[REG_PIXMASK];
	u32 const aux = m_nopup ?
		(m_reg[REG_WIDDATA] << 28) | (m_reg[REG_UAUXDATA] << 24) :
		(m_reg[REG_WIDDATA] << 28) | ((m_reg[REG_UAUXDATA] & 0x3) << 26) | (m_pupdata << 24);

	for (unsigned n = 0; m_numpix--; n++)
	{
		// TODO: z buffer check

		if (m_clip.contains(m_x >> 14, m_y >> 14))
		{
			if (pattern(m_x >> 14, n))
			{
				offs_t const offset = (m_y >> 14) * 0x500 + (m_x >> 14);
				if (wid(IR_SHADED, offset))
				{
					u32 const color = (m_r >> 11) << 0 | (m_g >> 11) << 8 | (m_b >> 11) << 16;

					vram_w(offset, aux | color, mask);
				}
			}
		}

		increment();
	}

	m_state = IDLE;
}

void sgi_re2_device::draw_flat_span(unsigned const n)
{
	u32 const mask = (m_reg[REG_AUXMASK] << 24) | m_reg[REG_PIXMASK];
	u32 const aux = m_nopup ?
		(m_reg[REG_WIDDATA] << 28) | (m_reg[REG_UAUXDATA] << 24) :
		(m_reg[REG_WIDDATA] << 28) | ((m_reg[REG_UAUXDATA] & 0x3) << 26) | (m_pupdata << 24);
	offs_t const offset = (m_y >> 14) * 0x500 + (m_x >> 14);

	for (unsigned i = 0; i < m_numpix; i++)
	{
		if (m_clip.contains((m_x >> 14) + i, m_y >> 14))
		{
			u32 const color = (m_r >> 11) << 0 | (m_g >> 11) << 8 | (m_b >> 11) << 16;

			vram_w(offset + i, aux | color, mask);
		}

		if ((i % n) == 0)
			increment();
	}

	m_state = IDLE;
}

void sgi_re2_device::increment()
{
	m_x += m_dx;
	m_y += m_dy;
	m_z += m_dz;

	m_r += m_dr;
	m_g += m_dg;
	m_b += m_db;
}

void sgi_re2_device::read_buffer()
{
	if (m_numpix > 0)
	{
		switch (m_reg[REG_RWMODE])
		{
		case RWMODE_FB_P:
			m_reg[REG_RWDATA] = m_vram[(m_y >> 14) * 0x500 + (m_x >> 14)];
			break;
		}

		increment();
		m_numpix--;

		m_state = DMA_R;
	}
	else
		m_state = IDLE;

	set_drq(m_state == DMA_R);
}

void sgi_re2_device::write_buffer()
{
	if (m_state == DMA_W)
	{
		for (unsigned i = 0; i <= m_reg[REG_UPACMODE]; i++)
		{
			if (m_clip.contains(m_x >> 14, m_y >> 14))
			{
				offs_t const offset = (m_y >> 14) * 0x500 + (m_x >> 14);

				// FIXME: wid only for rwmode 0, 1, 2, 6
				if (wid(IR_WRITEBUF, offset))
				{
					// unpack pixel (TODO: HADDR != 0)
					u32 const data = unpack(m_reg[REG_RWDATA], i, m_reg[REG_UPACMODE]);

					// TODO: xzoom
					// TODO: pattern

					// format and write
					// TODO: raster op

					// write the pixel
					switch (m_reg[REG_RWMODE])
					{
					case RWMODE_UAUX:
						vram_w(offset, data << 24, (m_reg[REG_AUXMASK] & (m_nopup ? 0xf : 0xc)) << 24);
						break;
					}
				}
			}

			increment();
			m_numpix--;
		}
	}

	if (m_numpix > 0)
		m_state = DMA_W;
	else
		m_state = IDLE;

	set_drq(m_state == DMA_W);
}

u32 sgi_re2_device::unpack(u32 data, unsigned const n, u32 const mode) const
{
	switch (mode)
	{
	case 1: data = u16(data >> (16 * (1 - n))); break;
	case 3: data = u8(data >> (8 * (3 - n))); break;
	}

	return data;
}

bool sgi_re2_device::wid(unsigned const ir, offs_t const offset)
{
	if (!m_reg[REG_ENABWID])
		return true;

	if ((ir == IR_TOPLINE || ir == IR_BOTLINE) && !m_reg[REG_ENABLWID])
		return true;

	unsigned const wid = m_vram[offset] >> 28;

	// 2 or 4 wid bitplanes?
	if (m_reg[REG_FBOPTION] & 1)
	{
		if (BIT(m_reg[REG_DEPTHFN], 3))
			return (wid & 0xe) == (m_reg[REG_CURWID] & 0xe);
		else
			return (wid & 0xf) == (m_reg[REG_CURWID] & 0xf);
	}
	else
		return (wid & 0x3) == (m_reg[REG_CURWID] & 0x3);
}

bool sgi_re2_device::pattern(unsigned const x, unsigned const n) const
{
	if (!m_reg[REG_ENABPAT])
		return true;

	unsigned const index = (m_reg[REG_ALIGNPAT] ? x : n) % 32;

	return BIT(m_pat, 31 - index);
}

u32 sgi_re2_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	// TODO: variable topscan row and column
	for (unsigned screen_y = screen.visible_area().min_y, mem_y = 1023; screen_y <= screen.visible_area().max_y; screen_y++, mem_y--)
		for (unsigned screen_x = screen.visible_area().min_x, mem_x = 0; screen_x <= screen.visible_area().max_x; screen_x++, mem_x++)
		{
			unsigned const channel = mem_x % 5;
			u32 const data = m_vram[(mem_y * 0x500) + mem_x];
			u16 const mode = m_xmap[channel]->mode_r(data >> 28);

			// default is 24 bit rgb single buffered
			rgb_t color = rgb_t(data >> 0, data >> 8, data >> 16);

			// check overlay or underlay
			if (((data >> 20) & mode & sgi_xmap2_device::MODE_OE) || ((mode & sgi_xmap2_device::MODE_UE) && !(data & 0x00ffffffU)))
				color = m_xmap[channel]->overlay_r(data >> 24);
			else
				switch (mode & sgi_xmap2_device::MODE_DM)
				{
				case 0: // 8 bit indexed single buffered
					{
						u16 const index = BIT(mode, sgi_xmap2_device::BIT_ME) ? ((mode & sgi_xmap2_device::MODE_MC) >> 2) | u8(data) : u8(data);

						color = m_xmap[channel]->pen_color(index);
					}
					break;

				case 1: // 4 bit indexed double buffered
					{
						u8 const buffer = BIT(mode, sgi_xmap2_device::BIT_BS) ? u8(data) >> 4 : data & 0x0f;
						u16 const index = BIT(mode, sgi_xmap2_device::BIT_ME) ? ((mode & sgi_xmap2_device::MODE_MC) >> 2) | buffer : buffer;

						color = m_xmap[channel]->pen_color(index);
					}
					break;

				case 2: // 12 bit indexed double buffered
					{
						u16 const buffer = u16(BIT(mode, sgi_xmap2_device::BIT_BS) ? data >> 12 : data) & 0x0fff;
						u16 const index = BIT(mode, sgi_xmap2_device::BIT_ME) ? ((mode & sgi_xmap2_device::MODE_MC) >> 2) | (buffer & 0xff) : buffer;

						color = m_xmap[channel]->pen_color(index);
					}
					break;

				case 5: // 12 bit rgb double buffered
					color = BIT(mode, sgi_xmap2_device::BIT_BS) ?
						rgb_t(
							((data >> 0x00) & 0xf0) | ((data >> 0x04) & 0x0f),
							((data >> 0x08) & 0xf0) | ((data >> 0x0c) & 0x0f),
							((data >> 0x10) & 0xf0) | ((data >> 0x14) & 0x0f)) :
						rgb_t(
							((data << 0x04) & 0xf0) | ((data >> 0x00) & 0x0f),
							((data >> 0x04) & 0xf0) | ((data >> 0x08) & 0x0f),
							((data >> 0x0c) & 0xf0) | ((data >> 0x10) & 0x0f));
					break;
				}

			// read the cursor devices
			u8 const cursor =
				(m_cursor[0]->cur_r(screen_x, screen_y) ? 1 : 0) |
				(m_cursor[1]->cur_r(screen_x, screen_y) ? 2 : 0);

			// apply the gamma ramp and output the pixel
			bitmap.pix(screen_y, screen_x) = rgb_t(
				m_ramdac[0]->lookup(color.r(), cursor),
				m_ramdac[1]->lookup(color.g(), cursor),
				m_ramdac[2]->lookup(color.b(), cursor));
		}

	return 0;
}
