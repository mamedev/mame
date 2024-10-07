// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

    XaviX 2

    unknown architecture, does not appear to be 6502 derived like XaviX / SuperXaviX

    die is marked  "SSD 2002-2004 NEC 800208-51"

*******************************************************************************/

#include "emu.h"

#include "screen.h"
#include "emupal.h"
#include "softlist.h"
#include "speaker.h"
#include "cpu/xavix2/xavix2.h"
#include "machine/i2cmem.h"
#include <algorithm>


namespace {

class xavix2_state : public driver_device
{
public:
	xavix2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_i2cmem(*this, "i2cmem")
		, m_pio(*this, "pio")
	{ }

	virtual void config(machine_config &config);

protected:
	enum {
		IRQ_TIMER =  7,
		IRQ_DMA   = 12
	};

	required_device<xavix2_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_device<i2cmem_device> m_i2cmem;
	required_ioport m_pio;

	u32 m_dma_src;
	u16 m_dma_dst;
	u16 m_dma_count;
	emu_timer *m_dma_timer;

	u32 m_pio_mode[2];
	u32 m_pio_dataw;
	u32 m_pio_mask_out;

	u16 m_gpu0_adr,  m_gpu0_count,  m_gpu1_adr,  m_gpu1_count;
	u16 m_gpu0b_adr, m_gpu0b_count, m_gpu1b_adr, m_gpu1b_count;

	u16 m_gpu_adr, m_gpu_descsize_adr, m_gpu_descdata_adr;
	u32 m_int_active;
	u32 m_int_enabled;
	u32 m_int_nmi;

	u16 m_bg_color;
	u32 m_palette[0x200];
	u32 m_sd[0x400][0x800];

	std::string m_debug_string;

	static u32 rgb555_888(u16 color);

	void irq_raise(u32 level);
	void irq_clear(u32 level);
	bool irq_state(u32 level) const;
	void irq_clear_w(u16 data);
	u16 irq_nmi_r();
	void irq_nmi_w(u16 data);
	u16 irq_enable_r();
	void irq_enable_w(u16 data);
	u8 irq_level_r();

	void bg_color_w(u16 data);
	u32 palette_r(offs_t reg);
	void palette_w(offs_t reg, u32 data);

	void gpu0_adr_w(u16 data);
	u16 gpu0b_adr_r();
	void gpu0b_adr_w(u16 data);
	void gpu0_count_w(u16 data);
	u16 gpu0b_count_r();
	void gpu0b_count_w(u16 data);
	void gpu0_trigger_w(u8 data);

	void gpu1_adr_w(u16 data);
	u16 gpu1b_adr_r();
	void gpu1b_adr_w(u16 data);
	void gpu1_count_w(u16 data);
	u16 gpu1b_count_r();
	void gpu1b_count_w(u16 data);
	void gpu1_trigger_w(u8 data);

	void gpu_update(u16 count, u16 adr);

	void gpu_descsize_w(u16 data);
	void gpu_descdata_w(u16 data);
	void gpu_adr_w(u16 data);
	void gpu_count_w(u16 data);

	void dma_src_w(offs_t, u32 data, u32 mem_mask);
	void dma_dst_w(offs_t, u16 data, u16 mem_mask);
	void dma_count_w(offs_t, u16 data, u16 mem_mask);
	void dma_control_w(u8 data);
	void dma_status_w(u8 data);
	u8 dma_status_r();

	TIMER_CALLBACK_MEMBER(dma_end);
	INTERRUPT_GEN_MEMBER(vblank_irq);

	void debug_port_w(u8 data);
	u8 debug_port_r();
	u8 debug_port_status_r();

	void pio_mode_w(offs_t offset, u32 data, u32 mem_mask);
	u32 pio_mode_r(offs_t offset);
	virtual void pio_update() = 0;
	void pio_w(offs_t offset, u32 data, u32 mem_mask);
	u32 pio_r();

	void crtc_w(offs_t reg, u16 data);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem(address_map &map) ATTR_COLD;
};

class naruto_state : public xavix2_state
{
public:
	using xavix2_state::xavix2_state;
	virtual void config(machine_config& config) override;

protected:
	virtual void pio_update() override;
};

class domyos_state : public xavix2_state
{
public:
	using xavix2_state::xavix2_state;
	virtual void config(machine_config& config) override;

protected:
	virtual void pio_update() override;
};

u32 xavix2_state::rgb555_888(u16 color)
{
	u8 r = (color >>  0) & 31;
	u8 g = (color >>  5) & 31;
	u8 b = (color >> 10) & 31;
	r = (r << 3) | (r >> 2);
	g = (g << 3) | (g >> 2);
	b = (b << 3) | (b >> 2);
	return (r << 16) | (g << 8) | b;
}

void xavix2_state::bg_color_w(u16 data)
{
	m_bg_color = data;
}

u32 xavix2_state::palette_r(offs_t reg)
{
	return m_palette[reg];
}

void xavix2_state::palette_w(offs_t reg, u32 data)
{
	m_palette[reg] = data & 0x8000 ? 0: rgb555_888(data) | 0xff000000;
}

void xavix2_state::irq_raise(u32 level)
{
	u32 line = 1 << level;
	if ((m_int_enabled | m_int_nmi) & line) {
		if(!m_int_active)
			m_maincpu->set_input_line(0, ASSERT_LINE);
		m_int_active |= line;
	}
}

void xavix2_state::irq_clear(u32 level)
{
	m_int_active &= ~(1 << level);
	if(!m_int_active)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

void xavix2_state::irq_clear_w(u16 data)
{
	m_int_active &= ~data;
	if(!m_int_active)
		m_maincpu->set_input_line(0, CLEAR_LINE);

}

u16 xavix2_state::irq_nmi_r()
{
	return m_int_nmi;
}

void xavix2_state::irq_nmi_w(u16 data)
{
	m_int_nmi = data;
}

u16 xavix2_state::irq_enable_r()
{
	return m_int_enabled;
}

void xavix2_state::irq_enable_w(u16 data)
{
	irq_clear(~data);
	m_int_enabled = data;
}

u8 xavix2_state::irq_level_r()
{
	for(u32 i=0; i<=12; i++)
		if(m_int_active & (1 << i))
			return i;
	return 0xff;
}

bool xavix2_state::irq_state(u32 level) const
{
	return m_int_active & (1 << level);
}

void xavix2_state::gpu0_adr_w(u16 data)
{
	m_gpu0_adr = data;
}

u16 xavix2_state::gpu0b_adr_r()
{
	return m_gpu0b_adr;
}

void xavix2_state::gpu0b_adr_w(u16 data)
{
	m_gpu0b_adr = data;
}

void xavix2_state::gpu0_count_w(u16 data)
{
	m_gpu0_count = data;
}

u16 xavix2_state::gpu0b_count_r()
{
	return m_gpu0b_count;
}

void xavix2_state::gpu0b_count_w(u16 data)
{
	m_gpu0b_count = data;
}

void xavix2_state::gpu0_trigger_w(u8 data)
{
	gpu_update(m_gpu0_count, m_gpu0_adr);
}

void xavix2_state::gpu1_adr_w(u16 data)
{
	m_gpu1_adr = data;
}

u16 xavix2_state::gpu1b_adr_r()
{
	return m_gpu1b_adr;
}

void xavix2_state::gpu1b_adr_w(u16 data)
{
	m_gpu1b_adr = data;
}

void xavix2_state::gpu1_count_w(u16 data)
{
	m_gpu1_count = data;
}

u16 xavix2_state::gpu1b_count_r()
{
	return m_gpu1b_count;
}

void xavix2_state::gpu1b_count_w(u16 data)
{
	m_gpu1b_count = data;
}

void xavix2_state::gpu1_trigger_w(u8 data)
{
	gpu_update(m_gpu1_count, m_gpu1_adr);
}

void xavix2_state::gpu_update(u16 count, u16 adr)
{
	std::unique_ptr<int []> list(new int[count]);
	for(u32 i=0; i != count; i++) {
		u64 command = m_maincpu->space(AS_PROGRAM).read_qword(adr + 8*i);
		list[i] = (command & 0x1fe00000) | i;
	}
	std::sort(list.get(), list.get() + count, std::greater<int>());
	for(u32 i=0; i != count; i++) {
		u64 command = m_maincpu->space(AS_PROGRAM).read_qword(adr + 8*(list[i] & 0xffff));
		logerror("gpu %02d: %016x x=%03x y=%03x ?=%02x ?=%x s=%02x w=%02x h=%02x c=%04x %s\n",
				 i, command,
				 (command >>  0) &  0x7ff,
				 (command >> 11) &  0x3ff,
				 (command >> 21) &   0xff,
				 (command >> 29) &    0x1,
				 (command >> 30) &   0x3f,
				 (command >> 36) &   0x3f,
				 (command >> 42) &   0x3f,
				 (command >> 48) & 0xffff,
				 machine().describe_context());
		u32 idx  = (command >> 30) & 0x3f;
		u32 idx2 = (command >> 58) & 0x3f;
		u32 descsize = m_maincpu->space(AS_PROGRAM).read_dword(m_gpu_descsize_adr + 4*idx);
		u16 descdata = m_maincpu->space(AS_PROGRAM).read_word(m_gpu_descdata_adr + 2*idx2);

		u32 sadr = (descdata << 14) + ((command >> 43) & 0x7fe0);
		u32 x = (command >>  0) &  0x7ff;
		u32 y = (command >> 11) &  0x3ff;
		u32 sx = 1+(descsize & 0xff);
		u32 sy = 1 + ((descsize >> 8) & 0xff);
		u32 scalex = (command >> 40) & 0x3; //only 1 and 2 seen as values for these two so far
		u32 scaley = (command >> 46) & 0x3;
		u32 bpp = 1 + ((descsize >> 24) & 7);
		logerror("gpu    - data %06x size %08x w=%x h=%x ?=%x bpp=%x pal=%x\n", sadr, descsize, sx, sy, (descsize >> 16) & 0xff, bpp, descsize >> 27);

		if(x+sx > 0x800)
			sx = 0x800 - x;
		if(y+sy > 0x400)
			sy = 0x400 - y;

		u32 avail = 0;
		u32 mask  = (1 << bpp) - 1;
		u32 palette = ((descsize >> 27) & 0x1f) << bpp;
		for(u32 yy=0; yy<sy; yy++) {
			u64 v = m_maincpu->space(AS_PROGRAM).read_qword(sadr);
			sadr += 8;
			avail = 64;
			for(u32 xx=0; xx<sx; xx++) {
				if (avail < bpp) {
					v = m_maincpu->space(AS_PROGRAM).read_qword(sadr);
					sadr += 8;
					avail = 64;
				}
				u32 color = m_palette[palette | (v & mask)];
				if(color) {
					for(u32 yyy = 0; yyy<scaley; yyy++) {
						for(u32 xxx = 0; xxx < scalex; xxx++) {
							m_sd[y+(yy*scaley)+yyy][x+(xx*scalex)+xxx] = color;
						}
					}
				}
				v >>= bpp;
				avail -= bpp;
			}
		}
	}
}

void xavix2_state::gpu_descsize_w(u16 data)
{
	m_gpu_descsize_adr = data;
	logerror("gpu descsize_w %04x\n", data);
}

void xavix2_state::gpu_descdata_w(u16 data)
{
	m_gpu_descdata_adr = data;
	logerror("gpu descdata_w %04x\n", data);
}

void xavix2_state::dma_src_w(offs_t, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dma_src);
}

void xavix2_state::dma_dst_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_dma_dst);
}

void xavix2_state::dma_count_w(offs_t, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_dma_count);
}

void xavix2_state::dma_control_w(u8 data)
{
	if(data == 3 || data == 7) {
		logerror("DMA %s:%08x -> %04x (%04x) %s\n",
				 data == 3 ? "main" : "rom",
				 m_dma_src, m_dma_dst, m_dma_count,
				 machine().describe_context());
		u32 sadr = m_dma_src | (data == 3 ? 0x00000000 : 0x40000000);
		u32 dadr = m_dma_dst;
		auto &prg = m_maincpu->space(AS_PROGRAM);
		for(u32 i=0; i != m_dma_count; i++)
			prg.write_byte(dadr + i, prg.read_byte(sadr + i));
		m_dma_timer->adjust(attotime::from_ticks(m_dma_count, m_maincpu->clock()));
	}
}

void xavix2_state::dma_status_w(u8 data)
{
	if(data == 2)
		irq_clear(IRQ_DMA);
}

u8 xavix2_state::dma_status_r()
{
	return irq_state(IRQ_DMA) ? 6 : 0;
}

TIMER_CALLBACK_MEMBER(xavix2_state::dma_end)
{
	irq_raise(IRQ_DMA);
}

INTERRUPT_GEN_MEMBER(xavix2_state::vblank_irq)
{
	u32 color = rgb555_888(m_bg_color);
	for(u32 y=0; y != 0x400; y++)
		for(u32 x=0; x != 0x800; x++)
			m_sd[y][x] = color;
	irq_raise(IRQ_TIMER);
}

void xavix2_state::debug_port_w(u8 data)
{
	if(data) {
		if(data == 0xa) {
			logerror("debug [%s]\n", m_debug_string);
			m_debug_string = "";
		} else if(data != 0xd)
			m_debug_string += char(data);
	}
}

u8 xavix2_state::debug_port_r()
{
	return 0;
}

u8 xavix2_state::debug_port_status_r()
{
	// 0: ok to recieve
	// 1: ok to send
	return 1<<1;
}

void xavix2_state::pio_mode_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pio_mode[offset]);
	//  logerror("%s: pio mode%d %08x %08x -> %08x\n", machine().describe_context(), offset, data, mem_mask, m_pio_mode[offset]);
	m_pio_mask_out = 0;
	for (u32 i=0; i<32; i++) {
		m_pio_mask_out |= (((m_pio_mode[i / 16] >> ((i % 16) * 2)) & 3) == 3) ? 1 << i : 0;
	}
	//  logerror("%s: pio mode in0 %08x, out %08x\n", machine().describe_context(), m_pio_mask_out);
	pio_update();
}

u32 xavix2_state::pio_mode_r(offs_t offset)
{
	return m_pio_mode[offset];
}

void naruto_state::pio_update()
{
	if (BIT(m_pio_mask_out, 21))
		m_i2cmem->write_sda(BIT(m_pio_dataw, 21));
	if (BIT(m_pio_mask_out, 20))
		m_i2cmem->write_scl(BIT(m_pio_dataw, 20));
}

void domyos_state::pio_update()
{
	if (BIT(m_pio_mask_out, 16))
		m_i2cmem->write_sda(BIT(m_pio_dataw, 16));
	if (BIT(m_pio_mask_out, 17))
		m_i2cmem->write_scl(BIT(m_pio_dataw, 17));
}

void xavix2_state::pio_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_pio_dataw);
	pio_update();
}

u32 xavix2_state::pio_r()
{
	return (m_pio->read() & ~m_pio_mask_out) | (m_pio_dataw & m_pio_mask_out);
}

/*
  NTSC? (63.55us)
[:] crtc[0] = 50034 (c372)
[:] crtc[1] = 463 (1cf)
[:] crtc[2] = 6760 (1a68)
[:] crtc[3] = 522 (20a)
[:] crtc[4] = 770 (302)
[:] crtc[5] = 525 (20d)
[:] crtc[6] = 9 (9)
[:] crtc[7] = 15 (f)
[:] crtc[8] = 3 (3)
[:] crtc[9] = 3 (3)
[:] crtc[10] = 3 (3)
[:] crtc[11] = 43 (2b)

  PAL? (64us)
[:] crtc[0] = 49924 (c304)
[:] crtc[1] = 457 (1c9)
[:] crtc[2] = 6758 (1a66)
[:] crtc[3] = 545 (221)
[:] crtc[4] = 765 (2fd)
[:] crtc[5] = 627 (273)
[:] crtc[6] = 8 (8)
[:] crtc[7] = 13 (d)
[:] crtc[8] = 3 (3)
[:] crtc[9] = 3 (3)
[:] crtc[10] = 3 (3)
[:] crtc[11] = 23 (17)
*/

void xavix2_state::crtc_w(offs_t reg, u16 data)
{
	logerror("crtc[%d] = %d (%x)\n", reg, (u32)data, data);
}


void xavix2_state::mem(address_map &map)
{
	map(0x00000000, 0x0000ffff).ram();
	map(0x00010000, 0x00ffffff).rom().region("maincpu", 0x010000);

	map(0x40000000, 0x40ffffff).rom().region("maincpu", 0);

	map(0xc0000000, 0xc00007ff).rw(FUNC(xavix2_state::palette_r), FUNC(xavix2_state::palette_w));
	map(0xc0000800, 0xc001ffff).ram();

	map(0xffffe000, 0xffffe003).w(FUNC(xavix2_state::dma_src_w));
	map(0xffffe004, 0xffffe005).w(FUNC(xavix2_state::dma_dst_w));
	map(0xffffe008, 0xffffe009).w(FUNC(xavix2_state::dma_count_w));
	map(0xffffe00c, 0xffffe00c).w(FUNC(xavix2_state::dma_control_w));
	map(0xffffe010, 0xffffe010).rw(FUNC(xavix2_state::dma_status_r), FUNC(xavix2_state::dma_status_w));

	map(0xffffe200, 0xffffe207).rw(FUNC(xavix2_state::pio_mode_r), FUNC(xavix2_state::pio_mode_w));
	map(0xffffe208, 0xffffe20b).rw(FUNC(xavix2_state::pio_r), FUNC(xavix2_state::pio_w));
	map(0xffffe238, 0xffffe238).rw(FUNC(xavix2_state::debug_port_r), FUNC(xavix2_state::debug_port_w));
	map(0xffffe239, 0xffffe239).r(FUNC(xavix2_state::debug_port_status_r));

	map(0xffffe400, 0xffffe401).w(FUNC(xavix2_state::gpu0_adr_w));
	map(0xffffe404, 0xffffe405).w(FUNC(xavix2_state::gpu0_count_w));
	map(0xffffe408, 0xffffe408).w(FUNC(xavix2_state::gpu0_trigger_w));

	map(0xffffe40c, 0xffffe40d).w(FUNC(xavix2_state::gpu1_adr_w));
	map(0xffffe410, 0xffffe411).w(FUNC(xavix2_state::gpu1_count_w));
	map(0xffffe414, 0xffffe414).w(FUNC(xavix2_state::gpu1_trigger_w));

	map(0xffffe600, 0xffffe601).rw(FUNC(xavix2_state::gpu0b_adr_r), FUNC(xavix2_state::gpu0b_adr_w));
	map(0xffffe602, 0xffffe603).rw(FUNC(xavix2_state::gpu0b_count_r), FUNC(xavix2_state::gpu0b_count_w));
	map(0xffffe604, 0xffffe605).rw(FUNC(xavix2_state::gpu1b_adr_r), FUNC(xavix2_state::gpu1b_adr_w));
	map(0xffffe606, 0xffffe607).rw(FUNC(xavix2_state::gpu1b_count_r), FUNC(xavix2_state::gpu1b_count_w));
	map(0xffffe608, 0xffffe609).w(FUNC(xavix2_state::gpu_descsize_w));
	map(0xffffe60a, 0xffffe60b).lr16(NAME([]() { return 0x240; })); // pal/ntsc
	map(0xffffe60e, 0xffffe60f).w(FUNC(xavix2_state::bg_color_w));

	map(0xffffe622, 0xffffe623).w(FUNC(xavix2_state::gpu_descdata_w));
	map(0xffffe630, 0xffffe631).lr16(NAME([]() { return 0x210; }));
	map(0xffffe632, 0xffffe633).lr16(NAME([]() { return 0x210; }));
	map(0xffffe634, 0xffffe64b).w(FUNC(xavix2_state::crtc_w));

	map(0xfffffc00, 0xfffffc00).r(FUNC(xavix2_state::irq_level_r));
	map(0xfffffc04, 0xfffffc05).w(FUNC(xavix2_state::irq_clear_w));
	map(0xfffffc08, 0xfffffc09).rw(FUNC(xavix2_state::irq_nmi_r), FUNC(xavix2_state::irq_nmi_w));
	map(0xfffffc0a, 0xfffffc0b).rw(FUNC(xavix2_state::irq_enable_r), FUNC(xavix2_state::irq_enable_w));
}

uint32_t xavix2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if(machine().input().code_pressed_once(KEYCODE_8))
		irq_raise(8);
	if(machine().input().code_pressed_once(KEYCODE_0))
		irq_raise(10);

	constexpr int dx = 0x400 - 320;
	constexpr int dy = 0x200 - 200;

	for(int y=0; y < 400; y++)
		for(int x=0; x<640; x++)
			bitmap.pix(y, x) = m_sd[y+dy][x+dx];

	return 0;
}

void xavix2_state::machine_start()
{
	m_dma_timer = timer_alloc(FUNC(xavix2_state::dma_end), this);
}

void xavix2_state::machine_reset()
{
	m_dma_src = 0;
	m_dma_dst = 0;
	m_dma_count = 0;
	m_int_active = 0;
	m_bg_color = 0;
}

static INPUT_PORTS_START( naruto )
	PORT_START("pio")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON3)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON4)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_BUTTON5)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_BUTTON6)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_BUTTON7)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_BUTTON8)
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_BUTTON9)
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_BUTTON10)
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_BUTTON11)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_BUTTON12)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_BUTTON13)
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_BUTTON14)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_BUTTON15)
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_BUTTON16)
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(2)
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("B/Execute")
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("D/Cancel")
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_CUSTOM) // i2c clock
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", i2cmem_device, read_sda)
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_PLAYER(2)
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_PLAYER(2)
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_PLAYER(2)
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_PLAYER(2)
	PORT_BIT(0x04000000, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_PLAYER(2)
	PORT_BIT(0x08000000, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_PLAYER(2)
	PORT_BIT(0x10000000, IP_ACTIVE_HIGH, IPT_BUTTON9) PORT_PLAYER(2)
	PORT_BIT(0x20000000, IP_ACTIVE_HIGH, IPT_BUTTON10) PORT_PLAYER(2)
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_BUTTON11) PORT_PLAYER(2)
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_BUTTON12) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START(domyos)
	PORT_START("pio")
	PORT_BIT(0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON1)
	PORT_BIT(0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON2)
	PORT_BIT(0x00000004, IP_ACTIVE_HIGH, IPT_BUTTON3)
	PORT_BIT(0x00000008, IP_ACTIVE_HIGH, IPT_BUTTON4)
	PORT_BIT(0x00000010, IP_ACTIVE_HIGH, IPT_BUTTON5)
	PORT_BIT(0x00000020, IP_ACTIVE_HIGH, IPT_BUTTON6)
	PORT_BIT(0x00000040, IP_ACTIVE_HIGH, IPT_BUTTON7)
	PORT_BIT(0x00000080, IP_ACTIVE_HIGH, IPT_BUTTON8)
	PORT_BIT(0x00000100, IP_ACTIVE_HIGH, IPT_BUTTON9)
	PORT_BIT(0x00000200, IP_ACTIVE_HIGH, IPT_BUTTON10)
	PORT_BIT(0x00000400, IP_ACTIVE_HIGH, IPT_BUTTON11)
	PORT_BIT(0x00000800, IP_ACTIVE_HIGH, IPT_BUTTON12)
	PORT_BIT(0x00001000, IP_ACTIVE_HIGH, IPT_BUTTON13)
	PORT_BIT(0x00002000, IP_ACTIVE_HIGH, IPT_BUTTON14)
	PORT_BIT(0x00004000, IP_ACTIVE_HIGH, IPT_BUTTON15)
	PORT_BIT(0x00008000, IP_ACTIVE_HIGH, IPT_BUTTON16)
	PORT_BIT(0x00010000, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("i2cmem", i2cmem_device, read_sda)
	PORT_BIT(0x00020000, IP_ACTIVE_HIGH, IPT_CUSTOM) // i2c clock
	PORT_BIT(0x00040000, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_PLAYER(2)
	PORT_BIT(0x00080000, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_PLAYER(2)
	PORT_BIT(0x00100000, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_PLAYER(2)
	PORT_BIT(0x00200000, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_PLAYER(2)
	PORT_BIT(0x00400000, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_PLAYER(2)
	PORT_BIT(0x00800000, IP_ACTIVE_HIGH, IPT_BUTTON8) PORT_PLAYER(2)
	PORT_BIT(0x01000000, IP_ACTIVE_HIGH, IPT_BUTTON9) PORT_PLAYER(2)
	PORT_BIT(0x02000000, IP_ACTIVE_HIGH, IPT_BUTTON10) PORT_PLAYER(2)
	PORT_BIT(0x04000000, IP_ACTIVE_HIGH, IPT_BUTTON11) PORT_PLAYER(2)
	PORT_BIT(0x08000000, IP_ACTIVE_HIGH, IPT_BUTTON12) PORT_PLAYER(2)
	PORT_BIT(0x10000000, IP_ACTIVE_HIGH, IPT_BUTTON13) PORT_PLAYER(2)
	PORT_BIT(0x20000000, IP_ACTIVE_HIGH, IPT_BUTTON14) PORT_PLAYER(2)
	PORT_BIT(0x40000000, IP_ACTIVE_HIGH, IPT_BUTTON15) PORT_PLAYER(2)
	PORT_BIT(0x80000000, IP_ACTIVE_HIGH, IPT_BUTTON16) PORT_PLAYER(2)
INPUT_PORTS_END

void xavix2_state::config(machine_config &config)
{
	// unknown CPU 'SSD 2002-2004 NEC 800208-51'
	XAVIX2(config, m_maincpu, 98'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &xavix2_state::mem);
	m_maincpu->set_vblank_int("screen", FUNC(xavix2_state::vblank_irq));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(FUNC(xavix2_state::screen_update));
	m_screen->set_size(640, 400);
	m_screen->set_visarea(0, 639, 0, 399);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	// unknown sound hardware
}

void naruto_state::config(machine_config& config)
{
	xavix2_state::config(config);

	I2C_24C08(config, m_i2cmem);
}

void domyos_state::config(machine_config& config)
{
	xavix2_state::config(config);

	I2C_24C64(config, m_i2cmem);
}

ROM_START( ltv_naru )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "naruto.bin", 0x000000, 0x800000, CRC(e3465ad2) SHA1(13e3d2de5d5a084635cab158f3639a1ea73265dc) )
ROM_END

ROM_START( domfitad )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "xpfitnessadventure.bin", 0x000000, 0x1000000, CRC(a7917081) SHA1(95ae5dc6e64a78ae060cb0e61d8b0af34a93c4ce) )
ROM_END

ROM_START( dombikec )
	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "xpbikeconcept.bin", 0x000000, 0x1000000, CRC(3447fce5) SHA1(c7e9e9cd789a17ac886ecf253f67753213cf8d21) )
ROM_END

} // anonymous namespace


CONS( 2006, ltv_naru, 0, 0, config, naruto, naruto_state, empty_init, "Bandai / SSD Company LTD", "Let's TV Play Naruto", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// These are for the 'Domyos Interactive System' other Domyos Interactive System games can be found in xavix.cpp (the SoC is inside the cartridge, base acts as a 'TV adapter' only)

// Has SEEPROM and an RTC.  Adventure has the string DOMYSSDCOLTD a couple of times.
CONS( 2008, domfitad, 0, 0, config, domyos, domyos_state, empty_init, "Decathlon / SSD Company LTD", "Domyos Fitness Adventure (Domyos Interactive System)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2008, dombikec, 0, 0, config, domyos, domyos_state, empty_init, "Decathlon / SSD Company LTD", "Domyos Bike Concept (Domyos Interactive System)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )




