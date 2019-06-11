// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

#include "emu.h"
#include "cpu/i86/i186.h"
#include "cpu/mcs48/mcs48.h"
#include "imagedev/floppy.h"
#include "machine/upd765.h"

#include "screen.h"
#include "speaker.h"

class mindset_state: public driver_device
{
public:
	mindset_state(const machine_config &mconfig, device_type type, const char *tag);
	virtual ~mindset_state() = default;

	void mindset(machine_config &config);

protected:
	required_device<i80186_cpu_device> m_maincpu;
	required_device<i8042_device> m_syscpu, m_soundcpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<u16> m_vram;

	memory_access_cache<1, 0, ENDIANNESS_LITTLE> *m_gcos;

	u16 m_dispctrl;

	static u16 gco_blend_0(u16, u16);
	static u16 gco_blend_1(u16, u16);
	static u16 gco_blend_2(u16, u16);
	static u16 gco_blend_3(u16, u16);
	static u16 gco_blend_4(u16, u16);
	static u16 gco_blend_5(u16, u16);
	static u16 gco_blend_6(u16, u16);
	static u16 gco_blend_7(u16, u16);

	static u16 (*const gco_blend[8])(u16, u16);

	static inline u16 msk(int bit) { return (1U << bit) - 1; }
	static inline u16 sw(u16 data) { return (data >> 8) | (data << 8); }

	void maincpu_mem(address_map &map);
	void maincpu_io(address_map &map);

	void display_mode();
	void blit(u16 packet_seg, u16 packet_adr);

	void gco_w(u16 data);
	u16 dispctrl_r();
	void dispctrl_w(u16 data);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void machine_reset() override;
};


mindset_state::mindset_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_syscpu(*this, "syscpu"),
	m_soundcpu(*this, "soundcpu"),
	m_screen(*this, "screen"),
	m_vram(*this, "vram")
{
}

void mindset_state::machine_start()
{
	m_gcos = m_maincpu->space(AS_PROGRAM).cache<1, 0, ENDIANNESS_LITTLE>();
}

void mindset_state::machine_reset()
{
}

u16 mindset_state::dispctrl_r()
{
	return m_dispctrl;
}

void mindset_state::dispctrl_w(u16 data)
{
	m_dispctrl = data;
	//	logerror("display control %04x\n", data);
}

u32 mindset_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	static const u32 pal[4] = { 0x000000, 0x555555, 0xaaaaaa, 0xffffff };

	if(false) {
		for(u32 y=0; y<200; y++) {
			// Interleaved
			const u16 *src = m_vram + 40*(y >> 1) + 4096*(y & 1);
			u32 *dest = &bitmap.pix32(y);
			for(u32 x=0; x<320; x+=8) {
				u16 sv = *src++;
				*dest++ = pal[(sv >>  6) & 3];
				*dest++ = pal[(sv >>  4) & 3];
				*dest++ = pal[(sv >>  2) & 3];
				*dest++ = pal[(sv >>  0) & 3];
				*dest++ = pal[(sv >> 14) & 3];
				*dest++ = pal[(sv >> 12) & 3];
				*dest++ = pal[(sv >> 10) & 3];
				*dest++ = pal[(sv >>  8) & 3];
			}
		}
	} else if(true) {
		for(u32 y=0; y<200; y++) {
			const u16 *src = m_vram + 40*y;
			u32 *dest = &bitmap.pix32(y);
			for(u32 x=0; x<320; x+=8) {
				u16 sv = *src++;
				*dest++ = pal[(sv >>  6) & 3];
				*dest++ = pal[(sv >>  4) & 3];
				*dest++ = pal[(sv >>  2) & 3];
				*dest++ = pal[(sv >>  0) & 3];
				*dest++ = pal[(sv >> 14) & 3];
				*dest++ = pal[(sv >> 12) & 3];
				*dest++ = pal[(sv >> 10) & 3];
				*dest++ = pal[(sv >>  8) & 3];
			}
		}
	} else {
		for(u32 y=0; y<25; y++) {
			for(u32 x=0; x<40; x++) {
				u16 val = m_vram[y*40+x];
				const u16 *src = m_vram + 0x1000 + ((val >> 1) & 0x7f);
				for(u32 yy=0; yy<8; yy++) {
					u8 pix = val & 1 ? *src >> 8 : *src;
					src += 128;
					u32 *dest = &bitmap.pix32(8*y+yy, 8*x);
					for(u32 xx=0; xx<8; xx++)
						*dest++ = pix & (0x80 >> xx) ? 0xffffff : 0x000000;					
				}
			}
		}
	}

	return 0;
}

u16 mindset_state::gco_blend_0(u16 src, u16)
{
	return src;
}

u16 mindset_state::gco_blend_1(u16 src, u16 dst)
{
	return src & dst;
}

u16 mindset_state::gco_blend_2(u16 src, u16 dst)
{
	return src | dst;
}

u16 mindset_state::gco_blend_3(u16 src, u16 dst)
{
	return src ^ dst;
}

u16 mindset_state::gco_blend_4(u16 src, u16)
{
	return ~src;
}

u16 mindset_state::gco_blend_5(u16 src, u16 dst)
{
	return (~src) & dst;
}

u16 mindset_state::gco_blend_6(u16 src, u16 dst)
{
	return (~src) | dst;
}

u16 mindset_state::gco_blend_7(u16 src, u16 dst)
{
	return (~src) ^ dst;
}

u16 (*const mindset_state::gco_blend[8])(u16, u16) = {
	gco_blend_0,
	gco_blend_1,
	gco_blend_2,
	gco_blend_3,
	gco_blend_4,
	gco_blend_5,
	gco_blend_6,
	gco_blend_7
};


void mindset_state::blit(u16 packet_seg, u16 packet_adr)
{
	u16 mode    = sw(m_gcos->read_word((packet_seg << 4) + ((packet_adr +  0) & 0xffff)));
	u16 src_adr = sw(m_gcos->read_word((packet_seg << 4) + ((packet_adr +  2) & 0xffff)));
	u16 src_sft = sw(m_gcos->read_word((packet_seg << 4) + ((packet_adr +  4) & 0xffff)));
	u16 dst_adr = sw(m_gcos->read_word((packet_seg << 4) + ((packet_adr +  6) & 0xffff)));
	u16 dst_sft = sw(m_gcos->read_word((packet_seg << 4) + ((packet_adr +  8) & 0xffff)));
	u16 width   = sw(m_gcos->read_word((packet_seg << 4) + ((packet_adr + 10) & 0xffff)));
	u16 height  = sw(m_gcos->read_word((packet_seg << 4) + ((packet_adr + 12) & 0xffff)));
	u16 sy      = sw(m_gcos->read_word((packet_seg << 4) + ((packet_adr + 14) & 0xffff)));
	u16 dy      = sw(m_gcos->read_word((packet_seg << 4) + ((packet_adr + 16) & 0xffff)));
	u16 rmask   = sw(m_gcos->read_word((packet_seg << 4) + ((packet_adr + 18) & 0xffff)));
	u16 src_seg = sw(m_gcos->read_word((packet_seg << 4) + ((packet_adr + 20) & 0xffff)));
	u16 dst_seg = sw(m_gcos->read_word((packet_seg << 4) + ((packet_adr + 22) & 0xffff)));
	u16 wmask   = sw(m_gcos->read_word((packet_seg << 4) + ((packet_adr + 24) & 0xffff)));
	u16 kmask   = sw(m_gcos->read_word((packet_seg << 4) + ((packet_adr + 26) & 0xffff)));
	logerror("GCO: p src %04x:%04x.%x dst %04x:%04x.%x sz %xx%x step %x:%x mask %04x:%04x k %x:%x mode %c%c%c%d%c%d%c%c%c\n", src_seg, src_adr, src_sft, dst_seg, dst_adr, dst_sft, width, height, sy, dy, rmask, wmask, (kmask >> 8) & 15, kmask & 15,
			 mode & 0x80 ? 'k' : '-',
			 mode & 0x40 ? 't' : 'o',
			 mode & 0x20 ? 'x' : '-',
			 (mode >> 2) & 7,
			 mode & 0x4000 ? 'f' : '-',
			 (mode >> 11) & 3,
			 mode & 0x400 ? 'n' : '-',
			 mode & 0x200 ? 'p' : '-',
			 mode & 0x100 ? 'i' : 'f');

	// k = detect collision (unimplemented)
	// t/o = transparent/opaque
	// x = go right to left (unimplemented)
	// f = fast (no idea what it means)
	// n = invert collision flag (unimplemented)
	// p = pattern fill (unimplemented, use by fill_dest_buffer)
	// i/f = increment source / don't (unimplemented, compare with p?, used by blt_copy_word)

	auto blend = gco_blend[(mode >> 2) & 7];

	u16 awmask = ((wmask << 16) | wmask) >> (15 - dst_sft);
	u16 swmask, mwmask, ewmask;
	if(dst_sft >= width) {
		swmask = msk(dst_sft+1) & ~msk(dst_sft - width + 1);
		mwmask = 0xffff;
		ewmask = swmask;
	} else {
		swmask = msk(dst_sft+1);
		mwmask = 0xffff;
		ewmask = ~msk((dst_sft - width + 1) & 15);
	}

	swmask &= awmask;
	mwmask &= awmask;
	ewmask &= awmask;

	u16 nw = ((width + (15 - dst_sft)) + 15) >> 4;

	for(u32 y=0; y<height; y++) {
		u16 src_cadr = src_adr;
		u16 dst_cadr = dst_adr;
			
		u16 cmask = swmask;
		u16 nw1 = nw;
		u32 srcs = sw(m_gcos->read_word((src_seg << 4) + src_cadr));
		src_cadr += 2;
		do {
			srcs = (srcs << 16) | sw(m_gcos->read_word((src_seg << 4) + src_cadr));
			u16 src = (srcs >> (src_sft + 1)) & rmask;
			u16 dst = sw(m_gcos->read_word((dst_seg << 4) + dst_cadr));
			u16 res = blend(src, dst);
			if(mode & 0x40) {
				u16 tmask;
				switch((mode >> 10) & 3) {
				case 0:
					tmask = dst;
					break;
				case 1:
					tmask = ((dst & 0xaaaa) >> 1) | (dst & 0x5555);
					tmask = tmask * 0x3;
					break;
				case 2:
					tmask = ((dst & 0xcccc) >> 2) | (dst & 0x3333);
					tmask = ((dst & 0x2222) >> 1) | (dst & 0x1111);
					tmask = tmask * 0xf;
					break;
				case 3:
					tmask = ((dst & 0xf0f0) >> 4) | (dst & 0x0f0f);
					tmask = ((dst & 0x0c0c) >> 2) | (dst & 0x0303);
					tmask = ((dst & 0x0202) >> 1) | (dst & 0x0101);
					tmask = tmask * 0xff;
					break;
				}
				cmask &= ~tmask;
			}

			res = (dst & ~cmask) | (res & cmask);

			logerror("GCO: %04x * %04x = %04x @ %04x\n", src, dst, res, cmask);
			
			m_gcos->write_word((dst_seg << 4) + dst_cadr, sw(res));
			src_cadr += 2;
			dst_cadr += 2;

			nw1 --;

			cmask = nw1 == 1 ? ewmask : mwmask;
		} while(nw1);

		src_adr += sy;
		dst_adr += dy;
	}
}

void mindset_state::gco_w(u16 data)
{
	u16 packet_seg  = sw(m_gcos->read_word(0xbfd7a));
	u16 packet_adr  = sw(m_gcos->read_word(0xbfd78));
	u16 global_mode = sw(m_gcos->read_word(0xbfd76));

	logerror("GCO: start %04x:%04x mode %04x (%05x)\n", packet_seg, packet_adr, global_mode, m_maincpu->pc());

	switch(global_mode) {
	case 0x0101:
		blit(packet_seg, packet_adr);
		break;
	}

	// 100 = done, 200 = done too???, 400 = collision?
	m_gcos->write_word(0xbfd74, m_gcos->read_word(0xbfd74) | 0x0700);

	// Can trigger an irq, on mode & 2 (or is it 200?) (0x40 on 8282, ack on 0x41)
}

void mindset_state::maincpu_mem(address_map &map)
{
	map(0x00000, 0x3ffff).ram();
	map(0xb8000, 0xbffff).ram().share("vram");
	map(0xf8000, 0xfffff).rom().region("maincpu", 0);
}

void mindset_state::maincpu_io(address_map &map)
{
	map(0x8300, 0x8301).w(FUNC(mindset_state::gco_w));
	map(0x8320, 0x8321).lr16("8320", []() -> u16 { return 0xa005; }); // To pass the display test
	map(0x8322, 0x8323).rw(FUNC(mindset_state::dispctrl_r), FUNC(mindset_state::dispctrl_w));
}

void mindset_state::mindset(machine_config &config)
{
	I80186(config, m_maincpu, 12_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &mindset_state::maincpu_mem);
	m_maincpu->set_addrmap(AS_IO,      &mindset_state::maincpu_io);

	I8042(config, m_syscpu, 12_MHz_XTAL/2);

	I8042(config, m_soundcpu, 12_MHz_XTAL/2);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(100));
	m_screen->set_size(320, 200);
	m_screen->set_visarea(0, 319, 0, 199);
	m_screen->set_screen_update(FUNC(mindset_state::screen_update));
	m_screen->screen_vblank().set(m_maincpu, FUNC(i80186_cpu_device::int1_w));
}

static INPUT_PORTS_START(mindset)
INPUT_PORTS_END

ROM_START(mindset)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD16_BYTE("1.7_lo.u60", 0, 0x4000, CRC(00474dc1) SHA1(676f30f170c14174dbff3b5cbf98d0f23472b7c4))
	ROM_LOAD16_BYTE("1.7_hi.u59", 1, 0x4000, CRC(1434af10) SHA1(39105eacdd7ddc13e449e2c32743e828bef33595))

	ROM_REGION(0x0800, "syscpu", 0)
	ROM_LOAD("253002-001.u17", 0, 0x800, CRC(69da82c9) SHA1(2f0bf5b134dc703cbc72e0c6df5b7beda1b39e70))

	ROM_REGION(0x0800, "soundcpu", 0)
	ROM_LOAD("253006-001.u16", 0, 0x800, CRC(7bea5edd) SHA1(30cdc0dedaa5246f4952df452a99ca22e3cd0636))
ROM_END

COMP( 1984, mindset, 0, 0, mindset, mindset, mindset_state, empty_init, "Mindset Corporation", "Mindset Video Production System", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)

