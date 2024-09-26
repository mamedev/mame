// license:BSD-3-Clause
// copyright-holders:MetalliC
/*************************************************************************

    Konami GSAN1 hardware
    (c) 2000

    CPU: Hitachi HD6417709 SH-3
    GPU: Hitachi HD64413AF 'Q2SD' Quick 2D Graphics Renderer with Synchronous DRAM Interface
    SPU: Yamaha YMZ280B-F
    Misc:
         Altera Max EPM3256ATC144-10
         Altera Max EPM3064ATC100-10
         Epson RTC-4553

    Known games (preliminary, some of listed below might not belong to this hardware):
    *Dance Dance Revolution Kids
     Muscle Ranking Football Masters
     Muscle Ranking Kick Target
    *Muscle Ranking Spray Hitter
     Muscle Ranking Struck Out
     Neratte Don Don
    *Run Run Puppy / らんらんぱぴぃ
     Soreike! Hanapuu

    * denotes these games are archived

    TODO:
     - proper ROZ, runpuppy uses rotation at juming dog animation
     - currently implemented very basic set of Q2SD GPU features, required/used by dumped games, should be improved if more games will be found.
     - hook IRQs from GPU and SPU (not used by dumped games), possible controlled by one MMIO registers in 140010xx area.
     - fix/improve timings, currently DDR Kids have notable desync with music.

    Notes:
     - hold Test + Service while booting to initialise RTC NVRAM
     - games do not enable SH-3 CPU cache, so it's actual rate is way lower than may/should be.

**************************************************************************/

#include "emu.h"

#include "bus/ata/ataintf.h"
#include "bus/ata/hdd.h"
#include "cpu/sh/sh3comn.h"
#include "cpu/sh/sh4.h"
#include "machine/s3520cf.h"
#include "machine/ticket.h"
#include "sound/ymz280b.h"

#include "speaker.h"
#include "screen.h"


namespace {

class gsan_state : public driver_device
{
public:
	gsan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ymz(*this, "ymz")
		, m_ata(*this, "ata")
		, m_rtc_r(*this, "RTCR")
		, m_rtc_w(*this, "RTCW")
		, m_dipsw_r(*this, "DSW")
		, m_vram(*this, "vram", 0x800000, ENDIANNESS_LITTLE)
		, m_gpuregs(*this, "gpu_regs", 0x800, ENDIANNESS_LITTLE)
		, m_ymzram(*this, "ymz_ram")
		, m_screen(*this, "screen")
		, m_hopper(*this, "hopper")
	{ }

	void gsan(machine_config &config);
	void gs_medal(machine_config &config);
	void init_gsan();

protected:
	required_device<sh34_base_device> m_maincpu;
	required_device<ymz280b_device> m_ymz;
	required_device<ata_interface_device> m_ata;
	required_ioport m_rtc_r;
	required_ioport m_rtc_w;
	required_ioport m_dipsw_r;
	memory_share_creator<u16> m_vram;
	memory_share_creator<u16> m_gpuregs;
	required_shared_ptr<u8> m_ymzram;
	required_device<screen_device> m_screen;
	optional_device<hopper_device> m_hopper;

	void main_map_common(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void main_map_medal(address_map &map) ATTR_COLD;
	void main_port(address_map &map) ATTR_COLD;
	void main_port_medal(address_map &map) ATTR_COLD;
	void ymz280b_map(address_map &map) ATTR_COLD;
	void ymz280b_map_medal(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	u8 ymzram_r(offs_t offset);
	void ymzram_w(offs_t offset, u8 data);
	u16 cf_regs_r(offs_t offset, u16 mem_mask = ~0);
	void cf_regs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 cf_data_r();
	void cf_data_w(u16 data);
	u8 rtc_r();
	void rtc_w(u8 data);
	u64 portc_r();
	void portc_w(u64 data);
	void portc_medal_w(u64 data);
	u64 porte_r();
	void porte_w(u64 data);
	void porte_medal_w(u64 data);
	u16 dipsw_r();
	u8 m_portc_data = 0xff;
	u8 m_porte_data = 0xff;

	// Q2SD GPU
	u16 gpu_r(offs_t offset);
	void gpu_w(offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	u16 vram_r(offs_t offset);
	void vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void vblank(int state);
	void do_render(bool vbkem);
	void draw_quad_tex(u16 cmd, u16 *data);
	void draw_quad_bin(u16 cmd, u16 *data);
	void fill_quad(u16 cmd, u16 *data);
	void draw_line(u16 cmd, u16 *data);

	int m_dbmode = 0;
	bool m_fg16bit = false;
	bool m_bg16bit = false;
	bool m_rend16bit = false;
	bool m_width1024 = false;
	bool m_rsae = false;
	bool m_vbkem = false;
	s16 m_xo = 0;
	s16 m_yo = 0;
	s16 m_uxmin = 0;
	s16 m_uxmax = 0;
	s16 m_uymin = 0;
	s16 m_uymax = 0;
	s16 m_sxmax = 0;
	s16 m_symax = 0;

	u32 get_rend_offset()
	{
		u16 val = m_rsae ? m_gpuregs[0x098 / 2] : m_gpuregs[0x014 / 2 + (BIT(m_gpuregs[0x002 / 2], 8) ? 0 : 1)];
		return (val & 0x7f) << 16;
	};
	u16 get_color(u8 col)
	{
		return ((m_gpuregs[0x100 + col * 2] & 0x00f8) << 8) | ((m_gpuregs[0x101 + col * 2] & 0xfc00) >> 5) | ((m_gpuregs[0x101 + col * 2] & 0x00f8) >> 3);
	};
	u16 get_pixel(u32 offset, u16 x, u16 y, bool bits16)
	{
		if (bits16)
		{
			if (!m_width1024)
				offset += ((x & 0xf) << 1) + ((y & 0xf) << 5) + ((x & 0x01f0) << 5) + ((y & 0x1ff0) << 10);
			else
				offset += ((x & 0xf) << 1) + ((y & 0xf) << 5) + ((x & 0x03f0) << 5) + ((y & 0x0ff0) << 11);
			offset &= 0x7fffff;
			return m_vram[offset / 2];
		}
		else
		{
			if (!m_width1024)
				offset += ((x & 0x1f) << 0) + ((y & 0xf) << 5) + ((x & 0x01e0) << 4) + ((y & 0x3ff0) << 9);
			else
				offset += ((x & 0x1f) << 0) + ((y & 0xf) << 5) + ((x & 0x03e0) << 4) + ((y & 0x1ff0) << 10);
			offset &= 0x7fffff;
			return (m_vram[offset / 2] >> ((offset & 1) * 8)) & 0xff;
		}
	};
	void put_pixel(u32 offset, u16 x, u16 y, u16 color, bool bits16)
	{
		if (bits16)
		{
			if (!m_width1024)
				offset += ((x & 0xf) << 1) + ((y & 0xf) << 5) + ((x & 0x01f0) << 5) + ((y & 0x1ff0) << 10);
			else
				offset += ((x & 0xf) << 1) + ((y & 0xf) << 5) + ((x & 0x03f0) << 5) + ((y & 0x0ff0) << 11);
			offset &= 0x7fffff;
			m_vram[offset / 2] = color;
		}
		else
		{
			if (!m_width1024)
				offset += ((x & 0x1f) << 0) + ((y & 0xf) << 5) + ((x & 0x01e0) << 4) + ((y & 0x3ff0) << 9);
			else
				offset += ((x & 0x1f) << 0) + ((y & 0xf) << 5) + ((x & 0x03e0) << 4) + ((y & 0x1ff0) << 10);
			offset &= 0x7fffff;
			u8 shift = (offset & 1) * 8;
			m_vram[offset / 2] = (m_vram[offset / 2] & (0xff00 >> shift)) | (color << shift);
		}
	};
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};



// CF interface, looks like standard memory-mapped ATA layout, probably should be devicified
u16 gsan_state::cf_regs_r(offs_t offset, u16 mem_mask)
{
	offset *= 2;
	u16 data = 0;
	if (ACCESSING_BITS_0_7)
		data |= m_ata->cs0_r(offset, 0xff) & 0xff;
	if (ACCESSING_BITS_8_15)
		data |= (m_ata->cs0_r(offset + 1, 0xff) << 8);
	return data;
}

void gsan_state::cf_regs_w(offs_t offset, u16 data, u16 mem_mask)
{
	offset *= 2;
	if (ACCESSING_BITS_0_7)
		m_ata->cs0_w(offset, data & 0xff, 0xff);
	if (ACCESSING_BITS_8_15)
		m_ata->cs0_w(offset + 1, data >> 8, 0xff);
}

u16 gsan_state::cf_data_r()
{
	u16 data = m_ata->cs0_r(0, 0xffff);
	return data;
}

void gsan_state::cf_data_w(u16 data)
{
	m_ata->cs0_w(0, data, 0xffff);
}

// misc I/O
u16 gsan_state::dipsw_r()
{
	return m_dipsw_r->read();
}
u8 gsan_state::rtc_r()
{
	return m_rtc_r->read();
}
void gsan_state::rtc_w(u8 data)
{
	m_rtc_w->write(data);
}

u8 gsan_state::ymzram_r(offs_t offset)
{
	return m_ymzram[offset];
}
void gsan_state::ymzram_w(offs_t offset, u8 data)
{
	m_ymzram[offset] = data;
}

// SH-3 GPIO output ports
u64 gsan_state::portc_r()
{
	return m_portc_data;
}
void gsan_state::portc_w(u64 data)
{
/* DDR
    ---- x--- /Coin counter
    --x- ---- Start button lamp
    -x-- ---- Right button lamp
    x--- ---- Left button lamp
*/
	m_portc_data = data;

	machine().bookkeeping().coin_counter_w(0, BIT(~data, 3));
}
void gsan_state::portc_medal_w(u64 data)
{
/* Medal
    ---- ---x Medal in counter
    ---- --x- 100Y in counter
    ---- -x-- 10Y in counter
    x--- ---- Hopper
*/
	m_portc_data = data;

	m_hopper->motor_w(BIT(~data, 7));
	machine().bookkeeping().coin_counter_w(0, BIT(data, 2));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
	machine().bookkeeping().coin_counter_w(2, BIT(data, 0));
}
u64 gsan_state::porte_r()
{
	return m_porte_data;
}
void gsan_state::porte_w(u64 data)
{
/* DDR
    ---- -x-- Lamp R3
    ---- x--- Lamp R2
    ---x ---- Lamp R1
    --x- ---- Lamp L3
    -x-- ---- Lamp L2
    x--- ---- Lamp L1
*/
	m_porte_data = data;
}
void gsan_state::porte_medal_w(u64 data)
{
/* Medal
    ---- ---x Medal in lock
    ---- --x- 100Y in lock
    ---- -x-- 10Y in lock
    -x-- ---- Button lamp
*/
	m_porte_data = data;

	machine().bookkeeping().coin_lockout_w(0, BIT(data, 2));
	machine().bookkeeping().coin_lockout_w(1, BIT(data, 1));
	machine().bookkeeping().coin_lockout_w(2, BIT(data, 0));
}


// Q2SD GPU
u16 gsan_state::vram_r(offs_t offset)
{
	return m_vram[offset];
}

void gsan_state::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_vram[offset]);
}

u16 gsan_state::gpu_r(offs_t offset)
{
	return m_gpuregs[offset];
}

void gsan_state::gpu_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	u16 prevval = m_gpuregs[offset];
	COMBINE_DATA(&m_gpuregs[offset]);
	data = m_gpuregs[offset];
#if 0
	if (prevval != data)
		logerror("Q2SD reg %02X %04X\n", offset*2, data);
#endif

	switch (offset)
	{
	case 0x000: // System control
		if (BIT(data, 15)) // reset
		{
			m_gpuregs[0x000 / 2] &= ~(1 << 10);
			m_gpuregs[0x002 / 2] &= ~0x1680;
		}
		if (BIT(data, 14)) // display reset
		{
			m_gpuregs[0x002 / 2] &= ~(1 << 15); // TVR
			m_gpuregs[0x002 / 2] &= ~(1 << 14); // FRM
			m_gpuregs[0x002 / 2] &= ~(1 << 11); // VBK
		}
		if (BIT(data, 10)) // render break
		{
			if (m_vbkem)
			{
				m_gpuregs[0x002 / 2] |= 1 << 7;
				m_vbkem = false;
			}
		}
		if (BIT(data, 8)) // start render
		{
			m_gpuregs[0x000 / 2] &= ~(1 << 8);
			do_render(false);
		}
		m_dbmode = (data >> 6) & 3;
		break;
	case 0x004 / 2: // Status register clear
		m_gpuregs[0x002 / 2] &= ~(data & 0xff80);
		break;
	case 0x006 / 2: // Interrupt enable
		if (data)
			logerror("Q2SD interrupts not implemented! enabled %04\n", data);
		break;
	case 0x00c / 2: // Rendering mode
		m_fg16bit = BIT(data, 0);
		m_bg16bit = BIT(data, 1) ^ BIT(data, 0);
		m_rend16bit = BIT(data, 2) ^ BIT(data, 0);
		m_width1024 = BIT(data, 6);
		m_rsae = BIT(data, 15);
		break;
	// R/O registers
	case 0x002 / 2: // Status
	case 0x03e / 2: case 0x040 / 2:
	case 0x080 / 2: case 0x082 / 2:
	case 0x084 / 2: case 0x086 / 2:
	case 0x088 / 2: case 0x08a / 2: case 0x08c / 2: case 0x08e / 2:
	case 0x090 / 2: case 0x092 / 2:
		m_gpuregs[offset] = prevval;
		break;
	}
}

void gsan_state::vblank(int state)
{
	if (state)
	{
		switch (m_dbmode)
		{
		case 0: // Auto display
			m_gpuregs[0x002 / 2] ^= 1 << 8; // DBF (display buffer)
			m_vbkem = false; // kind of abort drawing
			break;
		case 1: // Auto render mode
			if (m_vbkem)
			{
				m_vbkem = false;
				do_render(true);
			}
			else
				m_gpuregs[0x002 / 2] ^= 1 << 8; // DBF (display buffer)
			break;
		case 2: // Manual mode
		case 3:
			if (BIT(m_gpuregs[0x000 / 2], 9)) // DC (manual display area change)
			{
				m_gpuregs[0x000 / 2] &= ~(1 << 9);
				m_gpuregs[0x002 / 2] ^= 1 << 8; // DBF (display buffer)
			}
			if (m_vbkem)
			{
				m_vbkem = false;
				do_render(true);
			}
			break;
		}

		m_gpuregs[0x002 / 2] |= 1 << 11; // VBK (vblank)
	}
}

void gsan_state::draw_quad_tex(u16 cmd, u16 *data)
{
	//  logerror("Q2SD draw %04X src %d:%d sz %d:%d dst %d:%d %d:%d %d:%d %d:%d\n", cmd, data[0], data[1], data[2], data[3], (s16)data[4], (s16)data[5], (s16)data[6], (s16)data[7], (s16)data[8], (s16)data[9], (s16)data[10], (s16)data[11]);
	if (cmd & 0x57f)
		logerror("Q2SD unhandled draw tex mode %04X\n", cmd);
	u32 ssx = data[0] & 0x3ff;
	u32 ssy = data[1] & 0x3ff;
	s16 sdx = data[4];
	s16 sdy = data[5];
	s16 edx = data[8];
	s16 edy = data[9];

	sdx += m_xo;
	edx += m_xo;
	sdy += m_yo;
	edy += m_yo;

	s16 sclipx, sclipy, eclipx, eclipy;
	if (BIT(cmd, 7))
	{
		sclipx = m_uxmin;
		sclipy = m_uymin;
		eclipx = std::min(m_uxmax, m_sxmax);
		eclipy = std::min(m_uymax, m_symax);
	}
	else
	{
		sclipx = 0; sclipy = 0;
		eclipx = m_sxmax; eclipy = m_symax;
	}
	u32 fg_offs = get_rend_offset();

	s16 ddx = (edx >= sdx) ? 1 : -1;
	s16 ddy = (edy >= sdy) ? 1 : -1;
	edx += ddx;
	edy += ddy;

	u32 src_offs = ((m_gpuregs[0x01c / 2] & 0x7f) << 16) | (m_gpuregs[0x01c / 2] & 0xe000);

	bool opaque = !BIT(cmd, 9);

	for (int y = sdy, sy = 0; y != edy; y += ddy, sy++)
		for (int x = sdx, sx = 0; x != edx; x += ddx, sx++)
			if (x >= sclipx && x <= eclipx && y >= sclipy && y <= eclipy)
			{
				u16 pix = get_pixel(src_offs, ssx + sx, ssy + sy, m_rend16bit);
				if (pix || opaque)
					put_pixel(fg_offs, x, y, pix, m_rend16bit);
			}
}

void gsan_state::draw_quad_bin(u16 cmd, u16 *data)
{
	//  logerror("Q2SD draw %04X src %d:%d sz %d:%d dst %d:%d %d:%d %d:%d %d:%d\n", cmd, data[0], data[1], data[2], data[3], (s16)data[4], (s16)data[5], (s16)data[6], (s16)data[7], (s16)data[8], (s16)data[9], (s16)data[10], (s16)data[11]);
	if (cmd & 0x57f)
		logerror("Q2SD unhandled draw bin mode %04X\n", cmd);

	u32 src_offs = ((data[0] & 0x03ff) << 13) | (data[1] & 0x1fff);
	u32 tdx = data[2];
	s16 sdx = data[4];
	s16 sdy = data[5];
	s16 edx = data[8];
	s16 edy = data[9];
	u16 color0 = data[0xc];
	u16 color1 = data[0xd];

	sdx += m_xo;
	edx += m_xo;
	sdy += m_yo;
	edy += m_yo;

	s16 sclipx, sclipy, eclipx, eclipy;
	if (BIT(cmd, 7))
	{
		sclipx = m_uxmin;
		sclipy = m_uymin;
		eclipx = std::min(m_uxmax, m_sxmax);
		eclipy = std::min(m_uymax, m_symax);
	}
	else
	{
		sclipx = 0; sclipy = 0;
		eclipx = m_sxmax; eclipy = m_symax;
	}
	u32 fg_offs = get_rend_offset();

	s16 ddx = (edx >= sdx) ? 1 : -1;
	s16 ddy = (edy >= sdy) ? 1 : -1;
	edx += ddx;
	edy += ddy;

	bool opaque = !BIT(cmd, 9);
	if (!m_rend16bit)
	{
		color0 &= 0xff;
		color1 &= 0xff;
	}

	for (int y = sdy, sy = 0; y != edy; y += ddy, sy++)
		for (int x = sdx, sx = 0; x != edx; x += ddx, sx++)
			if (x >= sclipx && x <= eclipx && y >= sclipy && y <= eclipy)
			{
				u32 pixidx = sx + sy * tdx + src_offs * 8;
				bool pix = (m_vram[pixidx / 16] >> (pixidx % 16)) & 1;
				if (opaque || pix)
					put_pixel(fg_offs, x, y, pix ? color1 : color0, m_rend16bit);
			}
}

void gsan_state::fill_quad(u16 cmd, u16 *data)
{
	//  logerror("Q2SD fill dst %d:%d %d:%d %d:%d %d:%d col %04X\n", (s16)data[0], (s16)data[1], (s16)data[2], (s16)data[3], (s16)data[4], (s16)data[5], (s16)data[6], (s16)data[7], data[8]);
	if (cmd & 0x77f)
		logerror("Q2SD unhandled draw mode %04X\n", cmd);
	s16 sdx = data[0];
	s16 sdy = data[1];
	s16 edx = data[4];
	s16 edy = data[5];
	u16 color = data[8];

	sdx += m_xo;
	edx += m_xo;
	sdy += m_yo;
	edy += m_yo;

	s16 sclipx, sclipy, eclipx, eclipy;
	if (BIT(cmd, 7))
	{
		sclipx = m_uxmin;
		sclipy = m_uymin;
		eclipx = std::min(m_uxmax, m_sxmax);
		eclipy = std::min(m_uymax, m_symax);
	}
	else
	{
		sclipx = 0; sclipy = 0;
		eclipx = m_sxmax; eclipy = m_symax;
	}
	u32 fg_offs = get_rend_offset();
	if (!m_rend16bit)
		color &= 0xff;

	s16 ddx = (edx >= sdx) ? 1 : -1;
	s16 ddy = (edy >= sdy) ? 1 : -1;
	edx += ddx;
	edy += ddy;

	for (int y = sdy; y != edy; y += ddy)
		for (int x = sdx; x != edx; x += ddx)
		{
			if (x >= sclipx && x <= eclipx && y >= sclipy && y <= eclipy)
				put_pixel(fg_offs, x, y, color, m_rend16bit);
		}
}

void gsan_state::draw_line(u16 cmd, u16 *data)
{
	if (cmd & 0x77f)
		logerror("Q2SD unhandled line mode %04X\n", cmd);

	s16 sclipx, sclipy, eclipx, eclipy;
	if (BIT(cmd, 7))
	{
		sclipx = m_uxmin;
		sclipy = m_uymin;
		eclipx = std::min(m_uxmax, m_sxmax);
		eclipy = std::min(m_uymax, m_symax);
	}
	else
	{
		sclipx = 0; sclipy = 0;
		eclipx = m_sxmax; eclipy = m_symax;
	}
	u32 fg_offs = get_rend_offset();

	u16 color = *data++;
	if (!m_rend16bit)
		color &= 0xff;

	u16 count = *data++;
	while (count > 1)
	{
		s16 x0 = *data++ + m_xo;
		s16 y0 = *data++ + m_yo;
		s16 x1 = data[0] + m_xo;
		s16 y1 = data[1] + m_yo;
		--count;

		int dx = abs(x1 - x0);
		int dy = -abs(y1 - y0);
		int sx = x0 < x1 ? 1 : -1;
		int sy = y0 < y1 ? 1 : -1;
		int err = dx + dy;

		while (true)
		{
			if (x0 >= sclipx && x0 <= eclipx && y0 >= sclipy && y0 <= eclipy)
				put_pixel(fg_offs, x0, y0, color, m_rend16bit);

			if (x0 == x1 && y0 == y1)
				break;

			int e2 = 2 * err;
			if (e2 >= dy)
			{
				err += dy;
				x0 += sx;
			}
			if (e2 <= dx)
			{
				err += dx;
				y0 += sy;
			}
		}
	}
}

void gsan_state::do_render(bool vbkem)
{
	u32 listoffs = (vbkem ? ((m_gpuregs[0x03e / 2] << 16) | m_gpuregs[0x040 / 2]) : ((m_gpuregs[0x018 / 2] << 16) | m_gpuregs[0x01a / 2])) / 2;
	bool end_of_list = false;
	do
	{
		u16 cmd = m_vram[listoffs++];
		switch (cmd >> 11)
		{
		case 0x00: // POLYGON4A
			draw_quad_tex(cmd, &m_vram[listoffs]);
			listoffs += 12;
			break;
		case 0x01: // POLYGON4B
			draw_quad_bin(cmd, &m_vram[listoffs]);
			listoffs += 14;
			break;
		case 0x02: // POLYGON4C
			fill_quad(cmd, &m_vram[listoffs]);
			listoffs += 9;
			break;
		case 0x0c: // LINE
			draw_line(cmd, &m_vram[listoffs]);
			listoffs += m_vram[listoffs + 1] * 2 + 2;
			break;
		case 0x12: // LCOFS
			m_xo = m_gpuregs[0x84 / 2] = m_vram[listoffs++];
			m_yo = m_gpuregs[0x86 / 2] = m_vram[listoffs++];
			break;
		case 0x15: // UCLIP
			m_uxmin = m_gpuregs[0x88 / 2] = m_vram[listoffs++];
			m_uymin = m_gpuregs[0x8a / 2] = m_vram[listoffs++];
			m_uxmax = m_gpuregs[0x8c / 2] = m_vram[listoffs++];
			m_uymax = m_gpuregs[0x8e / 2] = m_vram[listoffs++];
			break;
		case 0x16: // WPR
			gpu_w(m_vram[listoffs] & 0x3ff, m_vram[listoffs + 1]);
			listoffs += 2;
			break;
		case 0x17: // SCLIP
			m_sxmax = m_gpuregs[0x90 / 2] = m_vram[listoffs++];
			m_symax = m_gpuregs[0x92 / 2] = m_vram[listoffs++];
			break;
		case 0x1a: // VBKEM
			listoffs += 2;
			// remember current address and wait until vblank
			m_gpuregs[0x03e / 2] = (listoffs * 2) >> 16;
			m_gpuregs[0x040 / 2] = (listoffs * 2) & 0xffff;
			m_vbkem = true;
			end_of_list = true;
			break;
		case 0x1e: // NOP3
			listoffs += 2;
			break;
		case 0x1f: // TRAP
			m_gpuregs[0x000 / 2] &= ~(1 << 10);
			m_gpuregs[0x002 / 2] |= 1 << 10;
			end_of_list = true;
			break;
		default:
			logerror("Q2SD not implemented command %04X addr %08X\n", cmd, (listoffs - 1) * 2);
			end_of_list = true;
			break;
		}
	} while (!end_of_list);
}

u32 gsan_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (BIT(m_gpuregs[0x000 / 2], 13))
	{
		u32 fg_offs = (m_gpuregs[0x014/2 + BIT(m_gpuregs[0x002/2], 8)] & 0x7f) << 16;
		bool fg_en = !BIT(m_gpuregs[0x056 / 2], 3);
		bool bg_en = BIT(m_gpuregs[0x00a / 2], 10);

		int bgsx = m_gpuregs[0x04c / 2];
		int bgsy = m_gpuregs[0x04e / 2];

		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
			for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				u16 col = 0;
				if (fg_en)
				{
					col = get_pixel(fg_offs, x, y, m_fg16bit);
					if (!m_fg16bit)
						col = get_color(col);
				}
				if (bg_en && col == 0)
				{
					col = get_pixel(0, x + bgsx, y + bgsy, m_bg16bit);
					if (!m_bg16bit)
						col = get_color(col);
				}
				bitmap.pix(y, x) = pal565(col, 11, 5, 0);
			}
	}
	else
		bitmap.fill(rgb_t::black(), cliprect);
	return 0;
}



void gsan_state::main_map_common(address_map &map)
{
	map(0x00000000, 0x0000ffff).rom().region("maincpu", 0);
	map(0x0c000000, 0x0c3fffff).ram().share("main_ram");
	map(0x10000000, 0x100007ff).rw(FUNC(gsan_state::gpu_r), FUNC(gsan_state::gpu_w));
	// misc I/O
	map(0x14000800, 0x14000807).rw(FUNC(gsan_state::cf_regs_r), FUNC(gsan_state::cf_regs_w));
	map(0x14000c00, 0x14000c03).rw(FUNC(gsan_state::cf_data_r), FUNC(gsan_state::cf_data_w));
	map(0x14001000, 0x14001001).r(FUNC(gsan_state::dipsw_r)); // write: pixel clock divider control, 1: /2, 2: /3, 4: /5
	map(0x14001019, 0x14001019).w(FUNC(gsan_state::rtc_w));
	map(0x14001039, 0x14001039).r(FUNC(gsan_state::rtc_r));

	map(0x18000000, 0x18000001).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));

	map(0x1f000000, 0x1f000fff).ram(); // cache RAM-mode (SH3 internal), actually should be 7Fxxxxxx, but current SH3 core doesn't like 7Fxxxxxx
	map(0xa0000000, 0xa000ffff).rom().region("maincpu", 0); // uncached mirror, otherwise no disassembly can bee seen in debugger (bug?)
}
void gsan_state::main_map(address_map &map)
{
	main_map_common(map);
	map(0x08000000, 0x087fffff).rw(FUNC(gsan_state::vram_r), FUNC(gsan_state::vram_w));
	map(0x18800000, 0x18ffffff).rw(FUNC(gsan_state::ymzram_r), FUNC(gsan_state::ymzram_w));
}

void gsan_state::main_port(address_map &map)
{
	map(SH3_PORT_C, SH3_PORT_C + 7).rw(FUNC(gsan_state::portc_r), FUNC(gsan_state::portc_w));
	map(SH3_PORT_E, SH3_PORT_E + 7).rw(FUNC(gsan_state::porte_r), FUNC(gsan_state::porte_w));
	map(SH3_PORT_F, SH3_PORT_F + 7).portr("PORT_F");
	map(SH3_PORT_L, SH3_PORT_L + 7).portr("PORT_L");
}

void gsan_state::ymz280b_map(address_map &map)
{
	map.global_mask(0x7fffff);
	map(0x000000, 0x7fffff).ram().share("ymz_ram");
}

void gsan_state::main_map_medal(address_map &map)
{
	main_map_common(map);
	map(0x08000000, 0x083fffff).rw(FUNC(gsan_state::vram_r), FUNC(gsan_state::vram_w));
	map(0x18800000, 0x18bfffff).rw(FUNC(gsan_state::ymzram_r), FUNC(gsan_state::ymzram_w));
}

void gsan_state::main_port_medal(address_map &map)
{
	main_port(map);
	map(SH3_PORT_C, SH3_PORT_C + 7).rw(FUNC(gsan_state::portc_r), FUNC(gsan_state::portc_medal_w));
	map(SH3_PORT_E, SH3_PORT_E + 7).rw(FUNC(gsan_state::porte_r), FUNC(gsan_state::porte_medal_w));
}

void gsan_state::ymz280b_map_medal(address_map &map)
{
	map.global_mask(0x3fffff);
	map(0x000000, 0x3fffff).ram().share("ymz_ram");
}


static INPUT_PORTS_START( ddrkids )
	PORT_START("PORT_F")
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Select R")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Select L")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused

	PORT_START("PORT_L")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("S2") // right-up hidden switch
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("S1") // left-down hidden switch
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_16WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_16WAY
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0003, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Easiest ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Medium_Hard ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( Very_Hard ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0078, 0x0000, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:4,5,6,7")
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0048, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0058, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0068, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0078, DEF_STR( 4C_5C ) )
	PORT_DIPNAME( 0x0080, 0x0000, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Demo Volume" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, DEF_STR( Low ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( High ) )
	PORT_DIPNAME( 0x1c00, 0x0800, "Max Stage" ) PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0400, "2" )
	PORT_DIPSETTING(      0x0800, "3" )
	PORT_DIPSETTING(      0x0c00, "4" )
	PORT_DIPSETTING(      0x1000, "5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x2000, 0x0000, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x4000, 0x0000, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x8000, 0x0000, "SW2:8" )

	PORT_START("RTCW")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("rtc", rtc4553_device, set_dir_line)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("rtc", rtc4553_device, set_cs_line)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("rtc", rtc4553_device, write_bit)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("rtc", rtc4553_device, set_clock_line)

	PORT_START("RTCR")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("rtc", rtc4553_device, read_bit)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( muscl )
	PORT_START("PORT_L")
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x8f, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused

	PORT_START("PORT_F")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Medal")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) // looks like not regular coin in to play, but coins for medals exchange
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", hopper_device, line_r)
	PORT_BIT( 0x78, IP_ACTIVE_LOW, IPT_UNKNOWN ) // unused

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, "Coin Slot 1" ) PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0003, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0000, "5 Coins/2 Credits" )
	PORT_DIPNAME( 0x0078, 0x0078, "Coin Slot 2" ) PORT_DIPLOCATION("SW1:4,5,6,7")
	PORT_DIPSETTING(      0x0078, "2 Medals" )
//  PORT_DIPSETTING(      0x0070, "2 Medals" )
	PORT_DIPSETTING(      0x0068, "3 Medals" )
	PORT_DIPSETTING(      0x0060, "4 Medals" )
	PORT_DIPSETTING(      0x0058, "5 Medals" )
	PORT_DIPSETTING(      0x0050, "6 Medals" )
	PORT_DIPSETTING(      0x0048, "7 Medals" )
	PORT_DIPSETTING(      0x0040, "8 Medals" )
	PORT_DIPSETTING(      0x0038, "9 Medals" )
	PORT_DIPSETTING(      0x0030, "10 Medals" )
	PORT_DIPSETTING(      0x0028, "11 Medals" )
	PORT_DIPSETTING(      0x0020, "12 Medals" )
	PORT_DIPSETTING(      0x0018, "13 Medals" )
	PORT_DIPSETTING(      0x0010, "14 Medals" )
	PORT_DIPSETTING(      0x0008, "15 Medals" )
	PORT_DIPSETTING(      0x0000, "16 Medals" )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0f00, 0x0000, "Standard of Payout" ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(      0x0f00, "15%" )
	PORT_DIPSETTING(      0x0e00, "20%" )
	PORT_DIPSETTING(      0x0d00, "25%" )
	PORT_DIPSETTING(      0x0c00, "30%" )
	PORT_DIPSETTING(      0x0b00, "35%" )
	PORT_DIPSETTING(      0x0a00, "40%" )
	PORT_DIPSETTING(      0x0900, "45%" )
	PORT_DIPSETTING(      0x0800, "50%" )
	PORT_DIPSETTING(      0x0700, "55%" )
	PORT_DIPSETTING(      0x0600, "60%" )
	PORT_DIPSETTING(      0x0500, "65%" )
	PORT_DIPSETTING(      0x0400, "70%" )
	PORT_DIPSETTING(      0x0300, "75%" )
	PORT_DIPSETTING(      0x0200, "80%" )
	PORT_DIPSETTING(      0x0100, "85%" )
	PORT_DIPSETTING(      0x0000, "90%" )
	PORT_DIPNAME( 0x3000, 0x0000, "Play Timer" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x3000, "1" )
	PORT_DIPSETTING(      0x2000, "2" )
	PORT_DIPSETTING(      0x1000, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x4000, 0x4000, "Backup Memory" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x4000, "Keep" )
	PORT_DIPSETTING(      0x0000, "Clear" )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("RTCW")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("rtc", rtc4553_device, set_dir_line)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("rtc", rtc4553_device, set_cs_line)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("rtc", rtc4553_device, write_bit)
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("rtc", rtc4553_device, set_clock_line)

	PORT_START("RTCR")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("rtc", rtc4553_device, read_bit)
	PORT_BIT( 0xfe, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( runpuppy )
	PORT_INCLUDE( muscl )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x3000, 0x0000, "Play Timer" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x3000, "8" )
	PORT_DIPSETTING(      0x2000, "12" )
	PORT_DIPSETTING(      0x1000, "16" )
	PORT_DIPSETTING(      0x0000, "20" )
INPUT_PORTS_END

//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

void gsan_state::machine_start()
{
	save_item(NAME(m_portc_data));
	save_item(NAME(m_porte_data));

	save_item(NAME(m_dbmode));
	save_item(NAME(m_fg16bit));
	save_item(NAME(m_bg16bit));
	save_item(NAME(m_rend16bit));
	save_item(NAME(m_width1024));
	save_item(NAME(m_rsae));
	save_item(NAME(m_vbkem));
	save_item(NAME(m_xo));
	save_item(NAME(m_yo));
	save_item(NAME(m_uxmin));
	save_item(NAME(m_uxmax));
	save_item(NAME(m_uymin));
	save_item(NAME(m_uymax));
	save_item(NAME(m_sxmax));
	save_item(NAME(m_symax));
}

void gsan_state::machine_reset()
{
	memset(&m_gpuregs[0], 0, 0x800);
	m_gpuregs[0x000 / 2] = 0xc000;
	m_gpuregs[0x002 / 2] = 0x0044;
	m_gpuregs[0x00a / 2] = 0x0088;
	m_gpuregs[0x072 / 2] = 0xc000;

	m_dbmode = 0;
	m_fg16bit = m_bg16bit = m_rend16bit = m_width1024 = m_rsae = m_vbkem = false;
	m_xo = m_yo = m_uxmin = m_uxmax = m_uymin = m_uymax = m_sxmax = m_symax = 0;
}

static void gsan_devices(device_slot_interface &device)
{
	device.option_add("cfcard", ATA_CF);
}

void gsan_state::gsan(machine_config &config)
{
	// basic machine hardware
	// SH7709 is earlier version of SH7709S (cv1k), not exact same, have minor differences
	SH3BE(config, m_maincpu, 32_MHz_XTAL * 2);
	m_maincpu->set_md(0, 0);
	m_maincpu->set_md(1, 0);
	m_maincpu->set_md(2, 0);
	m_maincpu->set_md(3, 0);
	m_maincpu->set_md(4, 0);
	m_maincpu->set_md(5, 1);
	m_maincpu->set_md(6, 0);
	m_maincpu->set_md(7, 1);
	m_maincpu->set_md(8, 0);
	m_maincpu->set_sh4_clock(32_MHz_XTAL * 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &gsan_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &gsan_state::main_port);

	// misc
	ATA_INTERFACE(config, m_ata).options(gsan_devices, "cfcard", nullptr, true);
	RTC4553(config, "rtc");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update(FUNC(gsan_state::screen_update));
	m_screen->set_raw(XTAL(36'000'000) / 3, 500, 0, 400, 400, 0, 300);
	m_screen->screen_vblank().set(FUNC(gsan_state::vblank));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YMZ280B(config, m_ymz, 16.9344_MHz_XTAL);
	m_ymz->set_addrmap(0, &gsan_state::ymz280b_map);
	m_ymz->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void gsan_state::gs_medal(machine_config &config)
{
	gsan(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &gsan_state::main_map_medal);
	m_maincpu->set_addrmap(AS_IO, &gsan_state::main_port_medal);

	m_ymz->set_addrmap(0, &gsan_state::ymz280b_map_medal);

	m_screen->set_raw(XTAL(36'000'000) / 5, 457, 0, 320, 262, 0, 240);

	HOPPER(config, "hopper", attotime::from_msec(100));
}

void gsan_state::init_gsan()
{
	m_maincpu->sh2drc_set_options(SH2DRC_STRICT_VERIFY | SH2DRC_STRICT_PCREL);
	m_maincpu->sh2drc_add_fastram(0x00000000, 0x0000ffff, 0, memregion("maincpu")->base());
	m_maincpu->sh2drc_add_fastram(0x0c000000, 0x0c3fffff, 1, memshare("main_ram")->ptr());
}

//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( ddrkids )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gqan4-a.u17", 0x00000, 0x10000, CRC(f1346f33) SHA1(8e5d3fb64fb6e320bbde8e4cbde0689ad176a94e) )

	ROM_REGION( 0x0f, "rtc", ROMREGION_ERASE00 )
	ROM_LOAD( "nvram.u9", 0x00, 0x0f, CRC(96a2e20b) SHA1(e857d915b1ddcb34f4dfb63b1cd743a439776009) )

	DISK_REGION( "ata:0:cfcard" )
	DISK_IMAGE( "gqan4_b-005", 0, SHA1(6f9b190e06607766dea348f22f536aa1eb1336b5) )
ROM_END

ROM_START( musclhit )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gsan5-a.u17", 0x00000, 0x10000, CRC(6ae1d1e8) SHA1(3224e4b8198aa38c094088456281cbd62c085407) )

	ROM_REGION( 0x0f, "rtc", 0 )
	ROM_LOAD( "nvram.u9", 0x00, 0x0f, CRC(17614a6a) SHA1(f4714659937e7dd3eedc18bbedc4b3000134df16) )

	DISK_REGION( "ata:0:cfcard" )
	DISK_IMAGE( "gsan6_a-213", 0, SHA1(d9e7a350428d1621fc70e81561390c01837a94c0) )
ROM_END

ROM_START( runpuppy )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "gsan1-a.u17", 0x00000, 0x08000, CRC(515da4bf) SHA1(72062296077db26d6bd1bc47556c2af00d5952e6) )

	ROM_REGION( 0x0f, "rtc", 0 )
	ROM_LOAD( "nvram.u9", 0x00, 0x0f, CRC(907eb7d3) SHA1(bdbe3618a2c6dd3fb66f8e4c0226c5d827e38d67) )

	DISK_REGION( "ata:0:cfcard" )
	DISK_IMAGE( "an10311003", 0, SHA1(5f972e29c201cdd6697f25140b37a11f02b605f5) )
ROM_END

} // anonymous namespace


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

GAME( 2000, ddrkids,       0, gsan,     ddrkids, gsan_state, init_gsan, ROT0, "Konami",       "Dance Dance Revolution Kids (GQAN4 JAA)", MACHINE_IMPERFECT_TIMING|MACHINE_IMPERFECT_GRAPHICS|MACHINE_SUPPORTS_SAVE )
GAME( 2000, musclhit,      0, gs_medal, muscl,   gsan_state, init_gsan, ROT0, "Konami / TBS", "Muscle Ranking Kinniku Banzuke Spray Hitter", MACHINE_IMPERFECT_GRAPHICS|MACHINE_SUPPORTS_SAVE )
GAME( 2000, runpuppy,      0, gs_medal, runpuppy,gsan_state, init_gsan, ROT0, "Konami",       "Run Run Puppy", MACHINE_IMPERFECT_GRAPHICS|MACHINE_SUPPORTS_SAVE )
