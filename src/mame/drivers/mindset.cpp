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
	required_device<i8749_device> m_kbdcpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<u16> m_vram;
	required_ioport_array<11> m_kbd_row;

	memory_access_cache<1, 0, ENDIANNESS_LITTLE> *m_gcos;

	u32 m_palette[16];
	bool m_genlock[16];
	u16 m_dispctrl, m_screenpos, m_intpos, m_intaddr;
	u8 m_kbd_p1, m_kbd_p2, m_borderidx;

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

	void gco_w(u16);
	u16 dispctrl_r();
	void dispctrl_w(u16 data);
	u16 dispreg_r();
	void dispreg_w(u16 data);

	int sys_t0_r();
	int sys_t1_r();
	u8 sys_p1_r();
	u8 sys_p2_r();
	void sys_p1_w(u8 data);
	void sys_p2_w(u8 data);

	void kbd_p1_w(u8 data);
	void kbd_p2_w(u8 data);
	int kbd_t1_r();
	u8 kbd_d_r();

	u16 keyscan();

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void machine_reset() override;
};


mindset_state::mindset_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_syscpu(*this, "syscpu"),
	m_soundcpu(*this, "soundcpu"),
	m_kbdcpu(*this, "kbdcpu"),
	m_screen(*this, "screen"),
	m_vram(*this, "vram"),
	m_kbd_row(*this, "K%02u", 0U)
{
}

void mindset_state::machine_start()
{
	m_gcos = m_maincpu->space(AS_PROGRAM).cache<1, 0, ENDIANNESS_LITTLE>();
}

void mindset_state::machine_reset()
{
}

int mindset_state::sys_t0_r()
{
	//	logerror("SYS: %d read t0 %d (%03x)\n", m_kbdcpu->total_cycles(), (m_kbd_p2 & 0x40) != 0, m_syscpu->pc());
	return (m_kbd_p2 & 0x40) != 0;
}

int mindset_state::sys_t1_r()
{
	logerror("SYS: read t1\n");
	return true;
}

u8 mindset_state::sys_p1_r()
{
	//	logerror("SYS: read p1\n");
	return 0xff;
}

u8 mindset_state::sys_p2_r()
{
	//	logerror("SYS: read p2 (%03x)\n", m_syscpu->pc());
	return 0xff;
}

void mindset_state::sys_p1_w(u8 data)
{
	//	logerror("SYS: write p1 %02x\n", data);
}

void mindset_state::sys_p2_w(u8 data)
{
	m_maincpu->int3_w(!(data & 0x80));
	//	logerror("SYS: write p2 %02x\n", data);
}

void mindset_state::kbd_p1_w(u8 data)
{
	m_kbd_p1 = data;
}

void mindset_state::kbd_p2_w(u8 data)
{
	//	if((m_kbd_p2 ^ data) & 0x40)
	//		logerror("KBD: %d output bit %d\n", m_kbdcpu->total_cycles(), (m_kbd_p2 & 0x40) != 0);
	m_kbd_p2 = data;
}

u8 mindset_state::kbd_d_r()
{
	return keyscan();
}

int mindset_state::kbd_t1_r()
{
	return keyscan() & 0x100;
}

u16 mindset_state::keyscan()
{
	u16 src = (m_kbd_p2 << 8) | m_kbd_p1;
	u16 res = 0x1ff;
	for(unsigned int i=0; i<11; i++)
		if(!(src & (1 << i)))
			res &= m_kbd_row[i]->read();
	return res;
}



u16 mindset_state::dispctrl_r()
{
	return m_dispctrl;
}

void mindset_state::dispctrl_w(u16 data)
{
	// 4000 = buffer choice (switch on int2, frame int instead of field int?)
	u16 chg = m_dispctrl ^ data;
	m_dispctrl = data;
	if(chg & 0xff00)
		logerror("display control %04x\n", m_dispctrl);
}

u16 mindset_state::dispreg_r()
{
	// a..5 needed to pass the display test
	// 0080 needed to allow uploading the palette
	return 0xa085;
}

void mindset_state::dispreg_w(u16 data)
{
	switch(m_dispctrl & 0xf) {
	case 0:
		m_screenpos = data;
		logerror("screen position (%d, %d)\n", (data >> 8) & 15, (data >> 12) & 15);
		break;
	case 1:
		m_borderidx = data & 0xf;
		break;
	case 2: {
		m_intpos = data;
		int intx = (159 - ((m_intpos >> 8) & 255)) * ((m_dispctrl & 0x100) ? 2 : 4);
		int inty = 199 - (m_intpos & 255);
		m_intaddr = 0;
		int mode_type = (m_dispctrl & 0x6000) >> 13;
		int pixels_per_byte_order = (m_dispctrl & 0x0600) >> 9;
		bool large_pixels = m_dispctrl & 0x0100;
		switch(mode_type) {
		case 0:
			m_intaddr = inty * (160 >> (pixels_per_byte_order - large_pixels + 1)) + (intx >> (pixels_per_byte_order - large_pixels + 2));
			break;
		case 1:
			m_intaddr = (inty & 1) * 0x2000 + (inty >> 1) * 80 + (intx >> (3 - large_pixels)) ;
			break;
		}

		logerror("interrupt position (%3d, %3d) %04x.%04x = %04x (%d,%d)\n", intx, inty, m_dispctrl, m_intpos, m_intaddr, pixels_per_byte_order, large_pixels);
		break;
	}
	case 4: {
		data = sw(data);
		u8 r = 0x11*(((data & 0x4000) >> 11) | (data & 7));
		u8 g = 0x11*(((data & 0x2000) >> 10) | ((data & 0x38) >> 3));
		u8 b = 0x11*(((data & 0x1000) >>  9) | ((data & 0x1c0) >> 6));

		if(!(data & 0x8000)) {
			r = r * 0.75;
			g = g * 0.75;
			b = b * 0.75;
		}
		m_palette[m_borderidx] = (r << 16) | (g << 8) | b;
		m_genlock[m_borderidx] = data & 0x0200;
		logerror("palette[%x] = %04x -> %06x.%d\n", m_borderidx, data, m_palette[m_borderidx], m_genlock[m_borderidx]);
		m_borderidx = (m_borderidx + 1) & 0xf;
		break;
	}

	default:
		logerror("display reg[%x] = %04x\n", m_dispctrl & 0xf, data);
	}
}

u32 mindset_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int mode_type = (m_dispctrl & 0x6000) >> 13;
	bool interleave = m_dispctrl & 0x0800;
	int pixels_per_byte_order = (m_dispctrl & 0x0600) >> 9;
	bool large_pixels = m_dispctrl & 0x0100;

	switch(mode_type) {
	case 0: // Native mode
		if(large_pixels) {
			if(!interleave) {
				switch(pixels_per_byte_order) {
				case 1: {
					static int palind[4] = { 0, 1, 4, 5 };
					const u16 *src = m_vram;
					for(u32 y=0; y<200; y++) {
						u32 *dest = &bitmap.pix32(y);
						for(u32 x=0; x<320; x+=8) {
							u16 sv = *src++;
							*dest++ = m_palette[palind[(sv >>  6) & 3]];
							*dest++ = m_palette[palind[(sv >>  4) & 3]];
							*dest++ = m_palette[palind[(sv >>  2) & 3]];
							*dest++ = m_palette[palind[(sv >>  0) & 3]];
							*dest++ = m_palette[palind[(sv >> 14) & 3]];
							*dest++ = m_palette[palind[(sv >> 12) & 3]];
							*dest++ = m_palette[palind[(sv >> 10) & 3]];
							*dest++ = m_palette[palind[(sv >>  8) & 3]];
						}
					}
					return 0;
				}
				}
			}
		}

		logerror("Unimplemented native mode (%dx%d, ppb=%d)\n", large_pixels ? 320 : 640, interleave ? 400 : 200, 2 << pixels_per_byte_order);
		break;

	case 1: // IBM-compatible graphics mode
		if(large_pixels) {
			if(!interleave) {
				switch(pixels_per_byte_order) {
				case 1: {
					static int palind[4] = { 0, 1, 4, 5 };
					for(u32 yy=0; yy<2; yy++) {
						const u16 *src = m_vram + 4096*yy;
						for(u32 y=yy; y<200; y+=2) {
							u32 *dest = &bitmap.pix32(y);
							for(u32 x=0; x<320; x+=8) {
								u16 sv = *src++;
								*dest++ = m_palette[palind[(sv >>  6) & 3]];
								*dest++ = m_palette[palind[(sv >>  4) & 3]];
								*dest++ = m_palette[palind[(sv >>  2) & 3]];
								*dest++ = m_palette[palind[(sv >>  0) & 3]];
								*dest++ = m_palette[palind[(sv >> 14) & 3]];
								*dest++ = m_palette[palind[(sv >> 12) & 3]];
								*dest++ = m_palette[palind[(sv >> 10) & 3]];
								*dest++ = m_palette[palind[(sv >>  8) & 3]];
							}
						}
					}
					return 0;
				}
				}
			}
		}

		logerror("Unimplemented ibm-compatible graphics mode (%dx%d, ppb=%d)\n", large_pixels ? 320 : 640, interleave ? 400 : 200, 2 << pixels_per_byte_order);
		break;

	case 2: // IBM-compatible character mode
		if(large_pixels) {
			if(!interleave) {
				for(u32 y=0; y<25; y++) {
					for(u32 x=0; x<40; x++) {
						u16 val = m_vram[y*40+x];
						const u16 *src = m_vram + 0x1000 + ((val >> 1) & 0x7f);
						for(u32 yy=0; yy<8; yy++) {
							u8 pix = val & 1 ? *src >> 8 : *src;
							src += 128;
							u32 *dest = &bitmap.pix32(8*y+yy, 8*x);
							for(u32 xx=0; xx<8; xx++)
								*dest++ = pix & (0x80 >> xx) ? m_palette[1] : m_palette[0];					
						}
					}
				}
				return 0;
			}
		}
		logerror("Unimplemented ibm-compatible character mode (%dx%d)\n", large_pixels ? 320/8 : 640/8, interleave ? 400/8 : 200/8);
		break;

	case 3: // Unknown
		logerror("Unknown graphics mode type 3\n");
		break;
	}

	bitmap.fill(0);

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
	// p = pattern fill (unimplemented, used by fill_dest_buffer)
	// i/f = increment source / don't (unimplemented, compare with p?, used by blt_copy_word)

	if(mode & 0x200) {
		// Weird, does one with target bbe8:0000 which blows everything up
		if(dst_seg != 0xbbe8) {
			u16 src = m_gcos->read_word((src_seg << 4) + src_adr);
			for(u16 w=0; w != width; w++) {
				m_gcos->write_word((dst_seg << 4) + dst_adr, src);
				dst_adr += 2;
			}
		}

	} else {
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
			if(mode & 0x100)
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

				//				logerror("GCO: %04x * %04x = %04x @ %04x\n", src, dst, res, cmask);
			
				m_gcos->write_word((dst_seg << 4) + dst_cadr, sw(res));
				if(mode & 0x100)
					src_cadr += 2;
				dst_cadr += 2;

				nw1 --;

				cmask = nw1 == 1 ? ewmask : mwmask;
			} while(nw1);

			if(mode & 0x100)
				src_adr += sy;
			dst_adr += dy;
		}
	}
}

void mindset_state::gco_w(u16)
{
	u16 packet_seg  = sw(m_gcos->read_word(0xbfd7a));
	u16 packet_adr  = sw(m_gcos->read_word(0xbfd78));
	u16 global_mode = sw(m_gcos->read_word(0xbfd76));

	logerror("GCO: start %04x:%04x mode %04x (%05x)\n", packet_seg, packet_adr, global_mode, m_maincpu->pc());

	switch(global_mode) {
	case 0x0005:
	case 0x0101:
		blit(packet_seg, packet_adr);
		break;
	}

	// 100 = done, 200 = done too???, 400 = collision?
	m_gcos->write_word(0xbfd74, m_gcos->read_word(0xbfd74) | 0x0700);

	// Can trigger an irq, on mode & 2 (or is it 200?) (0x40 on 8282, ack on 0x41, which means the system 8042...)
}

void mindset_state::maincpu_mem(address_map &map)
{
	map(0x00000, 0x3ffff).ram();
	map(0xb8000, 0xbffff).ram().share("vram");
	map(0xf8000, 0xfffff).rom().region("maincpu", 0);
}

void mindset_state::maincpu_io(address_map &map)
{
	map(0x8280, 0x8283).rw(m_syscpu, FUNC(i8042_device::upi41_master_r), FUNC(i8042_device::upi41_master_w)).umask16(0x00ff);
	map(0x82a0, 0x82a3).rw(m_soundcpu, FUNC(i8042_device::upi41_master_r), FUNC(i8042_device::upi41_master_w)).umask16(0x00ff);
	map(0x8300, 0x8301).w(FUNC(mindset_state::gco_w));
	map(0x8320, 0x8321).rw(FUNC(mindset_state::dispreg_r), FUNC(mindset_state::dispreg_w));
	map(0x8322, 0x8323).rw(FUNC(mindset_state::dispctrl_r), FUNC(mindset_state::dispctrl_w));
}

void mindset_state::mindset(machine_config &config)
{
	config.m_perfect_cpu_quantum = ":syscpu";

	I80186(config, m_maincpu, 12_MHz_XTAL/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &mindset_state::maincpu_mem);
	m_maincpu->set_addrmap(AS_IO,      &mindset_state::maincpu_io);

	I8042(config, m_syscpu, 14.318181_MHz_XTAL/2);
	m_syscpu->p1_in_cb().set(FUNC(mindset_state::sys_p1_r));
	m_syscpu->p2_in_cb().set(FUNC(mindset_state::sys_p2_r));
	m_syscpu->p1_out_cb().set(FUNC(mindset_state::sys_p1_w));
	m_syscpu->p2_out_cb().set(FUNC(mindset_state::sys_p2_w));
	m_syscpu->t0_in_cb().set(FUNC(mindset_state::sys_t0_r));
	m_syscpu->t1_in_cb().set(FUNC(mindset_state::sys_t1_r));

	I8042(config, m_soundcpu, 12_MHz_XTAL/2);

	I8749(config, m_kbdcpu, 6_MHz_XTAL);
	m_kbdcpu->p1_out_cb().set(FUNC(mindset_state::kbd_p1_w));
	m_kbdcpu->p2_out_cb().set(FUNC(mindset_state::kbd_p2_w));
	m_kbdcpu->bus_in_cb().set(FUNC(mindset_state::kbd_d_r));
	m_kbdcpu->t1_in_cb().set(FUNC(mindset_state::kbd_t1_r));

	// Should be NTSC actually... we'll see
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(100));
	m_screen->set_size(320, 200);
	m_screen->set_visarea(0, 319, 0, 199);
	m_screen->set_screen_update(FUNC(mindset_state::screen_update));
	m_screen->scanline().set([this](int scanline) { m_maincpu->int2_w(scanline == 198); });
	// This is bad and wrong and I don't yet care
	//	m_screen->screen_vblank().set(m_maincpu, FUNC(i80186_cpu_device::int0_w));
	m_screen->screen_vblank().set(m_maincpu, FUNC(i80186_cpu_device::int1_w));
	//	m_screen->screen_vblank().set(m_maincpu, FUNC(i80186_cpu_device::int2_w));
}

static INPUT_PORTS_START(mindset)
	PORT_START("K00")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K01")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD)                               PORT_NAME("Start")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PAUSE)      PORT_NAME("Pause")
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)        PORT_NAME("Sys config")
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12)        PORT_NAME("Reset")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K02")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K03")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)       PORT_CHAR(UCHAR_MAMEKEY(PGUP))
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD)                               PORT_NAME("Break")
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K04")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_CHAR('\t')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K05")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)       PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_NAME("Caps lock")

	PORT_START("K06")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("Control")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K07")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13) PORT_NAME("Return")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K08")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(END))
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_MAMEKEY(LALT)) PORT_NAME("Alt")
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("Shift (Left)")
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("K09")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("Shift (Right)")
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PRTSCR)     PORT_CHAR(UCHAR_MAMEKEY(PRTSCR)) PORT_NAME("Prt Scn")
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SCRLOCK)    PORT_CHAR(UCHAR_MAMEKEY(SCRLOCK))

	PORT_START("K10")
	PORT_BIT(0x001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x008, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x020, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x100, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

ROM_START(mindset)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD16_BYTE("1.7_lo.u60", 0, 0x4000, CRC(00474dc1) SHA1(676f30f170c14174dbff3b5cbf98d0f23472b7c4))
	ROM_LOAD16_BYTE("1.7_hi.u59", 1, 0x4000, CRC(1434af10) SHA1(39105eacdd7ddc13e449e2c32743e828bef33595))

	ROM_REGION(0x0800, "syscpu", 0)
	ROM_LOAD("253002-001.u17", 0, 0x800, CRC(69da82c9) SHA1(2f0bf5b134dc703cbc72e0c6df5b7beda1b39e70))

	ROM_REGION(0x0800, "soundcpu", 0)
	ROM_LOAD("253006-001.u16", 0, 0x800, CRC(7bea5edd) SHA1(30cdc0dedaa5246f4952df452a99ca22e3cd0636))

	ROM_REGION(0x0800, "kbdcpu", 0)
	ROM_LOAD("kbd_v3.0.bin", 0, 0x800, CRC(1c6aa433) SHA1(1d01dbda4730f26125ba2564a608c2f8ddfc05b3))
ROM_END

COMP( 1984, mindset, 0, 0, mindset, mindset, mindset_state, empty_init, "Mindset Corporation", "Mindset Video Production System", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)

