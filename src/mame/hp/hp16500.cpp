// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  Hewlett-Packard HP16500a/b/c Logic Analyzer

  These are weird, the "a" has more in common with the older 1650/1651
  than the 16500b.

  16500a rev 00.00:
    MC68000 @ 10 MHz
    MC68A45 CRTC
    FDC9793 floppy controller (WD1793 type)
    TMS9914A GPIB bus interface
    SCN2661 EPCI

  IRQ1 = VBL, IRQ2 = 20b007, IRQ3 = ?, IRQ4 = 20d000, IRQ5 = 20d007,
  IRQ6 = ?, IRQ7 = ?

  16500a rev 00.02:
    MC68000 @ 10 MHz
    MC68A45 CRTC
    Z0765A08PSC floppy controller (NEC765 type)
    TMS9914A GPIB bus interface
    SCN2661 EPCI

  16500b:
    MC68EC030 @ 25 MHz

    WD37C65C floppy controller (NEC765 type)
    Bt475 video RAMDAC
    TMS9914A GPIB bus interface
    Dallas DS1286 RTC/CMOS RAM

    IRQ 1 = VBL
    IRQ 2 = 35b8
    IRQ 3 = 35ce (jump 840120)
    IRQ 4 = 17768
    IRQ 5 = 814a
    IRQ 6 = 35c8 (jump 840120)
    IRQ 7 = 35d4 (jump 840120)

  16500c:
    Same as 16500b, but add:
    53C720 SCSI controller
    DP83934 SONIC ethernet

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m68000/m68030.h"
#include "machine/ds1386.h"
#include "machine/scn_pci.h"
#include "bus/hp_hil/hp_hil.h"
#include "bus/hp_hil/hil_devices.h"
#include "video/mc6845.h"
#include "screen.h"
#include "speaker.h"


namespace {

class hp16500_state : public driver_device
{
public:
	hp16500_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mlc(*this, "mlc")
	{ }

	void hp16500b(machine_config &config);
	void hp16500a(machine_config &config);
	void hp1651(machine_config &config);
	void hp1650(machine_config &config);

private:
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update_hp16500(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	[[maybe_unused]] uint32_t screen_update_hp16500a(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	optional_device<hp_hil_mlc_device> m_mlc;
	std::vector<uint8_t> m_vram;

	uint8_t m_mask = 0, m_val = 0;

	void palette_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t vram_r(offs_t offset);
	void mask_w(uint8_t data);
	void val_w(uint8_t data);
	uint32_t vbl_state_r();
	void vbl_ack_w(uint32_t data);
	uint16_t vbl_ack16_r();
	void vbl_ack16_w(uint16_t data);

	void pal_ctrl_w(uint8_t data);
	void pal_r_w(uint8_t data);
	void pal_g_w(uint8_t data);
	void pal_b_w(uint8_t data);

	void maskval_w(uint16_t data);
	void irq_2(int state);
	void vsync_changed(int state);
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_UPDATE_ROW(crtc_update_row_1650);

	void hp16500_map(address_map &map) ATTR_COLD;
	void hp16500a_map(address_map &map) ATTR_COLD;
	void hp1650_map(address_map &map) ATTR_COLD;
	void hp1651_map(address_map &map) ATTR_COLD;

	uint32_t m_palette[256]{}, m_colors[3]{}, m_count = 0, m_clutoffs = 0;
};

uint32_t hp16500_state::vbl_state_r()
{
	return 0x03000000;  // bit 0 set means the interrupt handler advances the pSOS tick counter.
}

void hp16500_state::vbl_ack_w(uint32_t data)
{
	m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
}

uint16_t hp16500_state::vbl_ack16_r()
{
	m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
	return 0;
}

void hp16500_state::vbl_ack16_w(uint16_t data)
{
	m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
}

void hp16500_state::vsync_changed(int state)
{
	if (state)
	{
		m_maincpu->set_input_line(M68K_IRQ_1, ASSERT_LINE);
	}
}

MC6845_UPDATE_ROW( hp16500_state::crtc_update_row )
{
	uint32_t *p = &bitmap.pix(y);

	int pos =  y * 144;

	for (int i = 0; i < x_count; i++)
	{
		*p++  = m_palette[m_vram[pos+0x00000]];
		*p++  = m_palette[m_vram[pos+0x10000]];
		*p++  = m_palette[m_vram[pos+0x20000]];
		*p++  = m_palette[m_vram[pos+0x30000]];
		pos++;
		*p++  = m_palette[m_vram[pos+0x00000]];
		*p++  = m_palette[m_vram[pos+0x10000]];
		*p++  = m_palette[m_vram[pos+0x20000]];
		*p++  = m_palette[m_vram[pos+0x30000]];
		pos++;
	}
}

MC6845_UPDATE_ROW( hp16500_state::crtc_update_row_1650 )
{
	uint32_t *p = &bitmap.pix(y);

	int pos =  y * 148;

	for (int i = 0; i < x_count; i++)
	{
		*p++  = m_palette[m_vram[pos+0x00000]];
		*p++  = m_palette[m_vram[pos+0x10000]];
		*p++  = m_palette[m_vram[pos+0x20000]];
		*p++  = m_palette[m_vram[pos+0x30000]];
		pos++;
		*p++  = m_palette[m_vram[pos+0x00000]];
		*p++  = m_palette[m_vram[pos+0x10000]];
		*p++  = m_palette[m_vram[pos+0x20000]];
		*p++  = m_palette[m_vram[pos+0x30000]];
		pos++;
	}
}

void hp16500_state::pal_ctrl_w(uint8_t data)
{
	m_clutoffs = data & 0xf;
}


void hp16500_state::pal_r_w(uint8_t data)
{
	m_colors[0] = (data<<4);
	m_palette[m_clutoffs] = rgb_t(m_colors[0], m_colors[1], m_colors[2]);
}

void hp16500_state::pal_g_w(uint8_t data)
{
	m_colors[1] = (data<<4);
	m_palette[m_clutoffs] = rgb_t(m_colors[0], m_colors[1], m_colors[2]);
}

void hp16500_state::pal_b_w(uint8_t data)
{
	m_colors[2] = (data<<4);
	m_palette[m_clutoffs] = rgb_t(m_colors[0], m_colors[1], m_colors[2]);
}

void hp16500_state::maskval_w(uint16_t data)
{
	// by analogy with the string printer code from the 16500b, which
	// appears to be a direct port...
	m_val =  ((data>>8) & 0xff) ^ 0xff;
	m_mask = (data & 0xff) ^ 0xff;
}

void hp16500_state::irq_2(int state)
{
	m_maincpu->set_input_line(M68K_IRQ_2, state);
}

void hp16500_state::hp1650_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom().region("bios", 0);

	map(0x201000, 0x201001).w(FUNC(hp16500_state::maskval_w));

	map(0x204001, 0x204001).w(FUNC(hp16500_state::pal_ctrl_w));
	map(0x205001, 0x205001).w(FUNC(hp16500_state::pal_r_w));
	map(0x2050fe, 0x2050ff).noprw();
	map(0x206000, 0x206001).nopr();
	map(0x206001, 0x206001).w(FUNC(hp16500_state::pal_g_w));
	map(0x207001, 0x207001).w(FUNC(hp16500_state::pal_b_w));

	map(0x20a000, 0x20a007).rw("epci", FUNC(scn_pci_device::read), FUNC(scn_pci_device::write)).umask16(0x00ff);

	map(0x20c001, 0x20c001).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x20c003, 0x20c003).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));

	map(0x20e000, 0x20e001).rw(FUNC(hp16500_state::vbl_ack16_r), FUNC(hp16500_state::vbl_ack16_w));

	map(0x20f000, 0x20f001).noprw();

	map(0x600000, 0x61ffff).w(FUNC(hp16500_state::vram_w));
	map(0x600000, 0x67ffff).r(FUNC(hp16500_state::vram_r)).umask16(0x00ff);

	map(0x900000, 0x9fffff).ram();
}

// like 1650 but moves main RAM to match 16500a
void hp16500_state::hp1651_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom().region("bios", 0);

	map(0x201000, 0x201001).w(FUNC(hp16500_state::maskval_w));

	map(0x204001, 0x204001).w(FUNC(hp16500_state::pal_ctrl_w));
	map(0x205001, 0x205001).w(FUNC(hp16500_state::pal_r_w));
	map(0x2050fe, 0x2050ff).noprw();
	map(0x206000, 0x206001).nopr();
	map(0x206001, 0x206001).w(FUNC(hp16500_state::pal_g_w));
	map(0x207001, 0x207001).w(FUNC(hp16500_state::pal_b_w));

	map(0x20a000, 0x20a007).rw("epci", FUNC(scn_pci_device::read), FUNC(scn_pci_device::write)).umask16(0x00ff);

	map(0x20c001, 0x20c001).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x20c003, 0x20c003).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));

	map(0x20e000, 0x20e001).rw(FUNC(hp16500_state::vbl_ack16_r), FUNC(hp16500_state::vbl_ack16_w));

	map(0x20f000, 0x20f001).noprw();

	map(0x600000, 0x61ffff).w(FUNC(hp16500_state::vram_w));
	map(0x600000, 0x67ffff).r(FUNC(hp16500_state::vram_r)).umask16(0x00ff);

	map(0x980000, 0xa7ffff).ram();
}

void hp16500_state::hp16500a_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom().region("bios", 0);

	map(0x201000, 0x201001).w(FUNC(hp16500_state::maskval_w));

	map(0x204001, 0x204001).w(FUNC(hp16500_state::pal_ctrl_w));
	map(0x205001, 0x205001).w(FUNC(hp16500_state::pal_r_w));
	map(0x206001, 0x206001).w(FUNC(hp16500_state::pal_g_w));
	map(0x207001, 0x207001).w(FUNC(hp16500_state::pal_b_w));

	map(0x20c001, 0x20c001).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x20c003, 0x20c003).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));

	map(0x20e000, 0x20e001).rw(FUNC(hp16500_state::vbl_ack16_r), FUNC(hp16500_state::vbl_ack16_w));

	map(0x600000, 0x61ffff).w(FUNC(hp16500_state::vram_w));
	map(0x600000, 0x67ffff).r(FUNC(hp16500_state::vram_r)).umask16(0x00ff);

	map(0x980000, 0xa7ffff).ram();
}

uint32_t hp16500_state::screen_update_hp16500a(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void hp16500_state::hp16500_map(address_map &map)
{
	map(0x00000000, 0x0001ffff).rom().region("bios", 0);
	map(0x0020f000, 0x0020f003).w(FUNC(hp16500_state::palette_w));

	map(0x00202800, 0x00202803).w(FUNC(hp16500_state::vbl_ack_w));
	map(0x00203000, 0x00203003).w(FUNC(hp16500_state::vbl_ack_w));
	map(0x00209800, 0x00209803).r(FUNC(hp16500_state::vbl_state_r));

	map(0x0020b800, 0x0020b83f).rw("rtc", FUNC(ds1286_device::data_r), FUNC(ds1286_device::data_w));
	map(0x0020b840, 0x0020b843).noprw(); // system ram test is really strange.

	map(0x0020f800, 0x0020f80f).rw(m_mlc, FUNC(hp_hil_mlc_device::read), FUNC(hp_hil_mlc_device::write));
	map(0x00600000, 0x0061ffff).w(FUNC(hp16500_state::vram_w));
	map(0x00600000, 0x0067ffff).r(FUNC(hp16500_state::vram_r)).umask32(0x00ff00ff);
	map(0x00700000, 0x00700000).w(FUNC(hp16500_state::mask_w));
	map(0x00740000, 0x00740000).w(FUNC(hp16500_state::val_w));
	map(0x00800000, 0x009fffff).ram();
}

void hp16500_state::video_start()
{
	m_count = 0;
	m_clutoffs = 0;
	memset(m_palette, 0, sizeof(m_palette));
	m_vram.resize(0x40000);
	m_mask = 0;
	m_val = 0;
}

// The test code is buggy, it writes a byte in the wrong position
// (even instead of odd).  It still works because the 68k replicates
// the byte on the word and the hardware doesn't decode UDS/LDS.  But
// that is why the handler needs to be 16 bits, or it won't be called
// in the first place.

void hp16500_state::vram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(!ACCESSING_BITS_0_7)
		data = data | (data >> 8);
	for(int i=0; i<4; i++) {
		int off = offset + i * 0x10000;
		if(data & (8 >> i))
			m_vram[off] = (m_vram[off] & ~m_mask) | (m_val & m_mask);
		else
			m_vram[off] = (m_vram[off] & ~m_mask);
	}
}

uint8_t hp16500_state::vram_r(offs_t offset)
{
	return m_vram[offset];
}

void hp16500_state::mask_w(uint8_t data)
{
	m_mask = data;
}

void hp16500_state::val_w(uint8_t data)
{
	m_val = data;
}

void hp16500_state::palette_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (mem_mask == 0xff000000)
	{
		m_clutoffs = (data>>24) & 0xff;
	}
	else if (mem_mask == 0x00ff0000)
	{
		m_colors[m_count++] = (data>>14) & 0xfc;

		if (m_count == 3)
		{
			m_palette[m_clutoffs] = rgb_t(m_colors[0], m_colors[1], m_colors[2]);
			m_clutoffs++;
			if (m_clutoffs > 255)
			{
				m_clutoffs = 0;
			}
			m_count = 0;
		}
	}
}

// 4 bpp
// addr = ((Y * 0xfc0) + 0x360) + (X * 4)
uint32_t hp16500_state::screen_update_hp16500(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int pos = 0;
	for (int y = 0; y < 384; y++)
	{
		uint32_t *scanline = &bitmap.pix(y);

		for (int x = 0; x < 576; x+=4)
		{
			*scanline++ = m_palette[m_vram[pos+0x00000]];
			*scanline++ = m_palette[m_vram[pos+0x10000]];
			*scanline++ = m_palette[m_vram[pos+0x20000]];
			*scanline++ = m_palette[m_vram[pos+0x30000]];
			pos++;
		}
	}

	return 0;
}

void hp16500_state::hp1650(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 10000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &hp16500_state::hp1650_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25000000, 0x330, 0, 0x250, 0x198, 0, 0x180);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	mc6845_device &crtc(MC6845(config, "crtc", 25000000/9));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(hp16500_state::crtc_update_row_1650));
	crtc.out_vsync_callback().set(FUNC(hp16500_state::vsync_changed));

	SCN2661A(config, "epci", 5000000);

	SPEAKER(config, "speaker", 2).front();
}

void hp16500_state::hp1651(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 10000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &hp16500_state::hp1651_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25000000, 0x330, 0, 0x250, 0x198, 0, 0x180);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	mc6845_device &crtc(MC6845(config, "crtc", 25000000/9));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(hp16500_state::crtc_update_row_1650));
	crtc.out_vsync_callback().set(FUNC(hp16500_state::vsync_changed));

	SCN2661A(config, "epci", 5000000);

	SPEAKER(config, "speaker", 2).front();
}

void hp16500_state::hp16500a(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 10000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &hp16500_state::hp16500a_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25000000, 0x320, 0, 0x240, 0x19c, 0, 0x170);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	mc6845_device &crtc(MC6845(config, "crtc", 25000000/9));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(hp16500_state::crtc_update_row));
	crtc.out_vsync_callback().set(FUNC(hp16500_state::vsync_changed));

	SPEAKER(config, "speaker", 2).front();
}

void hp16500_state::hp16500b(machine_config &config)
{
	/* basic machine hardware */
	M68EC030(config, m_maincpu, 50_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &hp16500_state::hp16500_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(hp16500_state::screen_update_hp16500));
	screen.set_size(576,384);
	screen.set_visarea(0, 576-1, 0, 384-1);
	screen.set_refresh_hz(60);
	screen.screen_vblank().set(FUNC(hp16500_state::vsync_changed));
	// FIXME: Where is the AP line connected to? The MLC documentation recommends connecting it to VBLANK
	screen.screen_vblank().append(m_mlc, FUNC(hp_hil_mlc_device::ap_w));

	HP_HIL_MLC(config, m_mlc, XTAL(15'920'000)/2);
	m_mlc->int_callback().set(FUNC(hp16500_state::irq_2));

	// TODO: for now hook up the ipc hil keyboard - this might be replaced
	// later with a 16500b specific keyboard implementation
	HP_HIL_SLOT(config, "hil1", "mlc", hp_hil_devices, "hp_ipc_kbd");

	DS1286(config, "rtc", 32768);
	//WD37C65C(config, "fdc", 16_MHz_XTAL);

	SPEAKER(config, "speaker", 2).front();
}

static INPUT_PORTS_START( hp16500 )
INPUT_PORTS_END

ROM_START( hp1650b )
	ROM_REGION16_BE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "la1.bin",      0x000000, 0x008000, CRC(7c60e334) SHA1(c3661e4bb58e0951e9c13208b4991f5d9dda633b) )
	ROM_LOAD16_BYTE( "la2.bin",      0x000001, 0x008000, CRC(b0c6fe60) SHA1(a77b2e7098f26af93b0946abe6e89711b3332210) )
ROM_END

ROM_START( hp1651b )
	ROM_REGION16_BE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "u102.bin",     0x000000, 0x008000, CRC(d7e1b091) SHA1(af813cc7ae748aedf1b1d9d522ee2154961315c2) )
	ROM_LOAD16_BYTE( "u103.bin",     0x000001, 0x008000, CRC(f32a37c7) SHA1(8addda59923f64fd9b7e909eb0e3a17cc02bb70e) )
ROM_END

ROM_START( hp165ka0 )
	ROM_REGION16_BE(0x20000, "bios", 0)
	ROM_LOAD16_BYTE( "16500-80002.bin", 0x000000, 0x008000, CRC(0324b75a) SHA1(837855fce9288139226c914cc0c43061e25b57d2) )
	ROM_LOAD16_BYTE( "16500-80001.bin", 0x000001, 0x008000, CRC(362c8cbf) SHA1(812b79d1a31d09ec632a6842b11548168d82b5e7) )
ROM_END

ROM_START( hp16500b )
	ROM_REGION32_BE(0x20000, "bios", 0)
	ROM_LOAD32_BYTE( "16500-80014.bin", 0x000000, 0x008000, CRC(35187716) SHA1(82067737892ecd356a5454a43d9ce9b38ac2397f) )
	ROM_LOAD32_BYTE( "16500-80015.bin", 0x000001, 0x008000, CRC(d8d26c1c) SHA1(b5b956c17c9d6e54519a686b5e4aa733b266bf6f) )
	ROM_LOAD32_BYTE( "16500-80016.bin", 0x000002, 0x008000, CRC(61457b39) SHA1(f209315ec22a8ee9d44a0ec009b1afb47794bece) )
	ROM_LOAD32_BYTE( "16500-80017.bin", 0x000003, 0x008000, CRC(e0b1096b) SHA1(426bb9a4756d8087bded4f6b61365d733ffbb09a) )
ROM_END

} // anonymous namespace


COMP( 1989, hp1650b,  0, 0, hp1650,   hp16500, hp16500_state, empty_init, "Hewlett Packard", "HP 1650b",  MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
COMP( 1989, hp1651b,  0, 0, hp1651,   hp16500, hp16500_state, empty_init, "Hewlett Packard", "HP 1651b",  MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
COMP( 1991, hp165ka0, 0, 0, hp16500a, hp16500, hp16500_state, empty_init, "Hewlett Packard", "HP 16500a", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
COMP( 1991, hp16500b, 0, 0, hp16500b, hp16500, hp16500_state, empty_init, "Hewlett Packard", "HP 16500b", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
