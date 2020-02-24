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

class xavix2_state : public driver_device
{
public:
	xavix2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_i2cmem(*this, "i2cmem")
	{ }

	void xavix2(machine_config &config);

private:
	enum {
		IRQ_TIMER =  7,
		IRQ_DMA   = 12
	};

	required_device<xavix2_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<i2c_24c08_device> m_i2cmem;

	u32 m_dma_src;
	u16 m_dma_dst;
	u16 m_dma_count;
	emu_timer *m_dma_timer;

	u16 m_port0_ddr;
	u8  m_port0_dataw, m_port0_data;
	u8  m_port0_maskw, m_port0_maskr;

  u16 m_gpu0_adr,  m_gpu0_count,  m_gpu1_adr,  m_gpu1_count;
  u16 m_gpu0b_adr, m_gpu0b_count, m_gpu1b_adr, m_gpu1b_count;

	u16 m_gpu_adr, m_gpu_descsize_adr, m_gpu_descdata_adr;
	u32 m_int_active;
  u32 m_int_enabled;
  u32 m_int_nmi;

	u32 m_sd[0x400][0x800];

	std::string m_debug_string;

	void irq_raise(u32 level);
	void irq_clear(u32 level);
	bool irq_state(u32 level) const;
	void irq_clear_w(u16 data);
	u16 irq_nmi_r();
	void irq_nmi_w(u16 data);
	u16 irq_enable_r();
	void irq_enable_w(u16 data);
	u8 irq_level_r();

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

	void port0_ddr_w(u16 data);
	u16 port0_ddr_r();
	void port0_update();
	void port0_w(u8 data);
	u8 port0_r();

	void crtc_w(offs_t reg, u16 data);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem(address_map &map);
};

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
	for(u32 i=0; i != count; i++) {
		u64 command = m_maincpu->space(AS_PROGRAM).read_qword(adr + 8*i);
		logerror("gpu %02d: %016x x=%03x y=%03x ?=%02x ?=%x ?=%02x w=%02x h=%02x c=%04x %s\n",
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

		u32 sadr = (descdata << 14) | ((command >> 43) & 0x3fe0);
		u32 x = (command >>  0) &  0x7ff;
		u32 y = (command >> 11) &  0x3ff;
		u32 sx = 1+(descsize & 0xff);
		u32 sy = 1 + ((descsize >> 8) & 0xff);
		u32 bpp = 1 + ((descsize >> 24) & 7);
		logerror("gpu    - data %06x size %08x w=%x h=%x ?=%x bpp=%x ?=%x\n", sadr, descsize, sx, sy, (descsize >> 16) & 0xff, bpp, descsize >> 27);

		if(x+sx > 0x800)
			sx = 0x800 - x;
		if(y+sy > 0x400)
			sy = 0x400 - y;

    u32 avail = 0;
    u32 mask  = (1 << bpp) - 1;
    u32 shift = (8 - bpp);
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
          u32 c = (v & mask) << shift;
          m_sd[y+yy][x+xx] = 0xff000000 | (0x010101 * c);
          v >>= bpp;
          avail -= bpp;
				}
			}
/*
		switch(bpp) {
		case 2: {
			stride = 8*((sx+31)/32);
			for(u32 yy=0; yy<sy; yy++) {
				u32 cadr = sadr;
				for(u32 xx=0; xx<sx; xx += 32) {
					u64 v = m_maincpu->space(AS_PROGRAM).read_qword(cadr);
					u32 xxl = sx - xx;
					if(xxl > 32)
						xxl = 32;
					for(u32 xxx=0; xxx<xxl; xxx++) {
						u32 c = (v >> (2*xxx)) & 3;
						m_sd[y+yy][x+xx+xxx] = 0xff000000 | (0x555555 * c);
					}
					cadr += 8;
				}
				sadr += stride;
			}
			break;
		}

		case 3: {
			stride = 8*((sx+20)/21);
			for(u32 yy=0; yy<sy; yy++) {
				u32 cadr = sadr;
				for(u32 xx=0; xx<sx; xx += 21) {
					u64 v = m_maincpu->space(AS_PROGRAM).read_qword(cadr);
					u32 xxl = sx - xx;
					if(xxl > 21)
						xxl = 21;
					for(u32 xxx=0; xxx<xxl; xxx++) {
						u32 c = (v >> (3*xxx)) & 7;
						m_sd[y+yy][x+xx+xxx] = 0xff000000 | (0x242424 * c);
					}
					cadr += 8;
				}
				sadr += stride;
			}
			break;
		}

		default:
			for(u32 yy=0; yy<sy; yy++)
				for(u32 xx=0; xx<sx; xx++)
					m_sd[yy+y][xx+x] = 0xffff0000;
			break;
		}
    */
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
	memset(m_sd, 0, sizeof(m_sd));
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

void xavix2_state::port0_ddr_w(u16 data)
{
//	logerror("%s: port0 ddr %04x -> %04x\n", machine().describe_context(), m_port0_ddr, data);
	m_port0_ddr = data;
	m_port0_maskr = m_port0_maskw = 0;
	for (u32 i=0; i<8; i++) {
		m_port0_maskr |= ((data & 3) == 0) ? 1 << i : 0;
		m_port0_maskw |= ((data & 3) == 3) ? 1 << i : 0;
		data >>= 2;
	}
//	logerror("%s: port0 maskr %04x, maskw %04x\n", machine().describe_context(), m_port0_maskr, m_port0_maskw);
	port0_update();
}

u16 xavix2_state::port0_ddr_r()
{
	return m_port0_ddr;
}

void xavix2_state::port0_update()
{
	u8 old = m_port0_data;
	m_port0_data &= ~m_port0_maskw;
	m_port0_data |= (m_port0_dataw & m_port0_maskw);
//	logerror("%s: port0 %04x -> %04x (%04x)\n", machine().describe_context(), old, m_port0_data, m_port0_dataw);
	if (m_port0_maskw & 0x20)
		m_i2cmem->write_sda((m_port0_dataw >> 5) & 0x1);
	if (m_port0_maskw & 0x10)
		m_i2cmem->write_scl((m_port0_dataw >> 4) & 0x1);
	if ((old ^ m_port0_dataw) & 0x30) {
		m_port0_data &= ~m_port0_maskr;
		m_port0_data |= ((m_i2cmem->read_sda() << 5) & m_port0_maskr);
	}
}

void xavix2_state::port0_w(u8 data)
{
	m_port0_dataw = data;
	port0_update();
}

u8 xavix2_state::port0_r()
{
	u8 data = (m_port0_dataw & m_port0_maskw) | (m_port0_data & m_port0_maskr);
	return data;
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

	map(0xc0000000, 0xc001ffff).ram();

	map(0xffffe000, 0xffffe003).w(FUNC(xavix2_state::dma_src_w));
	map(0xffffe004, 0xffffe005).w(FUNC(xavix2_state::dma_dst_w));
	map(0xffffe008, 0xffffe009).w(FUNC(xavix2_state::dma_count_w));
	map(0xffffe00c, 0xffffe00c).w(FUNC(xavix2_state::dma_control_w));
	map(0xffffe010, 0xffffe010).rw(FUNC(xavix2_state::dma_status_r), FUNC(xavix2_state::dma_status_w));

	map(0xffffe204, 0xffffe205).rw(FUNC(xavix2_state::port0_ddr_r), FUNC(xavix2_state::port0_ddr_w));
	map(0xffffe20a, 0xffffe20a).rw(FUNC(xavix2_state::port0_r), FUNC(xavix2_state::port0_w));
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
	m_dma_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(xavix2_state::dma_end), this));
}

void xavix2_state::machine_reset()
{
	m_dma_src = 0;
	m_dma_dst = 0;
	m_dma_count = 0;
	m_int_active = 0;
}

static INPUT_PORTS_START( xavix2 )
INPUT_PORTS_END

void xavix2_state::xavix2(machine_config &config)
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

	I2C_24C08(config, m_i2cmem);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	// unknown sound hardware
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


CONS( 2006, ltv_naru, 0, 0, xavix2, xavix2, xavix2_state, empty_init, "Bandai / SSD Company LTD", "Let's TV Play Naruto", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// These are for the 'Domyos Interactive System' other Domyos Interactive System games can be found in xavix.cpp (the SoC is inside the cartridge, base acts as a 'TV adapter' only)

// Has SEEPROM and an RTC.  Adventure has the string DOMYSSDCOLTD a couple of times.
CONS( 2008, domfitad, 0, 0, xavix2, xavix2, xavix2_state, empty_init, "Decathlon / SSD Company LTD", "Domyos Fitness Adventure (Domyos Interactive System)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2008, dombikec, 0, 0, xavix2, xavix2, xavix2_state, empty_init, "Decathlon / SSD Company LTD", "Domyos Bike Concept (Domyos Interactive System)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )




