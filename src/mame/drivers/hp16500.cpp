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
    SCN2661 DUART/timer

  IRQ1 = VBL, IRQ2 = 20b007, IRQ3 = ?, IRQ4 = 20d000, IRQ5 = 20d007,
  IRQ6 = ?, IRQ7 = ?

  16500a rev 00.02:
    MC68000 @ 10 MHz
    MC68A45 CRTC
    Z0765A08PSC floppy controller (NEC765 type)
    TMS9914A GPIB bus interface
    SCN2661 DUART/timer

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
#include "video/mc6845.h"
#include "machine/mc68681.h"

#define MAINCPU_TAG "maincpu"
#define CRTC_TAG    "crtc"
#define SCREEN_TAG  "screen"
#define DUART_TAG   "duart"

class hp16500_state : public driver_device
{
public:
	hp16500_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, MAINCPU_TAG)
		{ }

	virtual void video_start() override;
	UINT32 screen_update_hp16500(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_hp16500a(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	std::vector<UINT8> m_vram;

	UINT8 m_mask, m_val;

	DECLARE_WRITE32_MEMBER(palette_w);

	DECLARE_WRITE16_MEMBER(vram_w);
	DECLARE_READ8_MEMBER  (vram_r);
	DECLARE_WRITE8_MEMBER (mask_w);
	DECLARE_WRITE8_MEMBER (val_w);
	DECLARE_READ32_MEMBER(vbl_state_r);
	DECLARE_WRITE32_MEMBER(vbl_ack_w);
	DECLARE_READ16_MEMBER(vbl_ack16_r);
	DECLARE_WRITE16_MEMBER(vbl_ack16_w);

	DECLARE_WRITE8_MEMBER(pal_ctrl_w);
	DECLARE_WRITE8_MEMBER(pal_r_w);
	DECLARE_WRITE8_MEMBER(pal_g_w);
	DECLARE_WRITE8_MEMBER(pal_b_w);

	DECLARE_WRITE16_MEMBER(maskval_w);

	DECLARE_WRITE_LINE_MEMBER(vsync_changed);
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_UPDATE_ROW(crtc_update_row_1650);

	INTERRUPT_GEN_MEMBER(vblank);

private:
	UINT32 m_palette[256], m_colors[3], m_count, m_clutoffs;
};

READ32_MEMBER(hp16500_state::vbl_state_r)
{
	return 0x03000000;  // bit 0 set means the interrupt handler advances the pSOS tick counter.
}

WRITE32_MEMBER(hp16500_state::vbl_ack_w)
{
	m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
}

READ16_MEMBER(hp16500_state::vbl_ack16_r)
{
	m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
	return 0;
}

WRITE16_MEMBER(hp16500_state::vbl_ack16_w)
{
	m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
}

WRITE_LINE_MEMBER( hp16500_state::vsync_changed )
{
	if (state)
	{
		m_maincpu->set_input_line(M68K_IRQ_1, ASSERT_LINE);
	}
}

MC6845_UPDATE_ROW( hp16500_state::crtc_update_row )
{
	UINT32 *p = &bitmap.pix32(y);
	int i, pos;

	pos =  y * 144;

	for (i = 0; i < x_count; i++)
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
	UINT32 *p = &bitmap.pix32(y);
	int i, pos;

	pos =  y * 148;

	for (i = 0; i < x_count; i++)
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

WRITE8_MEMBER(hp16500_state::pal_ctrl_w)
{
	m_clutoffs = data & 0xf;
}


WRITE8_MEMBER(hp16500_state::pal_r_w)
{
	m_colors[0] = (data<<4);
	m_palette[m_clutoffs] = rgb_t(m_colors[0], m_colors[1], m_colors[2]);
}

WRITE8_MEMBER(hp16500_state::pal_g_w)
{
	m_colors[1] = (data<<4);
	m_palette[m_clutoffs] = rgb_t(m_colors[0], m_colors[1], m_colors[2]);
}

WRITE8_MEMBER(hp16500_state::pal_b_w)
{
	m_colors[2] = (data<<4);
	m_palette[m_clutoffs] = rgb_t(m_colors[0], m_colors[1], m_colors[2]);
}

WRITE16_MEMBER(hp16500_state::maskval_w)
{
	// by analogy with the string printer code from the 16500b, which
	// appears to be a direct port...
	m_val =  ((data>>8) & 0xff) ^ 0xff;
	m_mask = (data & 0xff) ^ 0xff;
}

static ADDRESS_MAP_START(hp1650_map, AS_PROGRAM, 16, hp16500_state)
	AM_RANGE(0x000000, 0x00ffff) AM_ROM AM_REGION("bios", 0)

	AM_RANGE(0x201000, 0x201001) AM_WRITE(maskval_w)

	AM_RANGE(0x204000, 0x204001) AM_WRITE8(pal_ctrl_w, 0x00ff)
	AM_RANGE(0x205000, 0x205001) AM_WRITE8(pal_r_w, 0x00ff)
	AM_RANGE(0x206000, 0x206001) AM_WRITE8(pal_g_w, 0x00ff)
	AM_RANGE(0x207000, 0x207001) AM_WRITE8(pal_b_w, 0x00ff)

	AM_RANGE(0x20c000, 0x20c001) AM_DEVREADWRITE8(CRTC_TAG, mc6845_device, status_r, address_w, 0x00ff)
	AM_RANGE(0x20c002, 0x20c003) AM_DEVREADWRITE8(CRTC_TAG, mc6845_device, register_r, register_w, 0x00ff)

	AM_RANGE(0x20e000, 0x20e001) AM_READWRITE(vbl_ack16_r, vbl_ack16_w)

	AM_RANGE(0x2050fe, 0x2050ff) AM_NOP
	AM_RANGE(0x206000, 0x206001) AM_NOP
	AM_RANGE(0x20f000, 0x20f001) AM_NOP

	AM_RANGE(0x600000, 0x61ffff) AM_WRITE(vram_w)
	AM_RANGE(0x600000, 0x67ffff) AM_READ8(vram_r, 0x00ff)

	AM_RANGE(0x900000, 0x9fffff) AM_RAM
ADDRESS_MAP_END

// like 1650 but moves main RAM to match 16500a
static ADDRESS_MAP_START(hp1651_map, AS_PROGRAM, 16, hp16500_state)
	AM_RANGE(0x000000, 0x00ffff) AM_ROM AM_REGION("bios", 0)

	AM_RANGE(0x201000, 0x201001) AM_WRITE(maskval_w)

	AM_RANGE(0x204000, 0x204001) AM_WRITE8(pal_ctrl_w, 0x00ff)
	AM_RANGE(0x205000, 0x205001) AM_WRITE8(pal_r_w, 0x00ff)
	AM_RANGE(0x206000, 0x206001) AM_WRITE8(pal_g_w, 0x00ff)
	AM_RANGE(0x207000, 0x207001) AM_WRITE8(pal_b_w, 0x00ff)

	AM_RANGE(0x20c000, 0x20c001) AM_DEVREADWRITE8(CRTC_TAG, mc6845_device, status_r, address_w, 0x00ff)
	AM_RANGE(0x20c002, 0x20c003) AM_DEVREADWRITE8(CRTC_TAG, mc6845_device, register_r, register_w, 0x00ff)

	AM_RANGE(0x20e000, 0x20e001) AM_READWRITE(vbl_ack16_r, vbl_ack16_w)

	AM_RANGE(0x2050fe, 0x2050ff) AM_NOP
	AM_RANGE(0x206000, 0x206001) AM_NOP
	AM_RANGE(0x20f000, 0x20f001) AM_NOP

	AM_RANGE(0x600000, 0x61ffff) AM_WRITE(vram_w)
	AM_RANGE(0x600000, 0x67ffff) AM_READ8(vram_r, 0x00ff)

	AM_RANGE(0x980000, 0xa7ffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(hp16500a_map, AS_PROGRAM, 16, hp16500_state)
	AM_RANGE(0x000000, 0x00ffff) AM_ROM AM_REGION("bios", 0)

	AM_RANGE(0x201000, 0x201001) AM_WRITE(maskval_w)

	AM_RANGE(0x204000, 0x204001) AM_WRITE8(pal_ctrl_w, 0x00ff)
	AM_RANGE(0x205000, 0x205001) AM_WRITE8(pal_r_w, 0x00ff)
	AM_RANGE(0x206000, 0x206001) AM_WRITE8(pal_g_w, 0x00ff)
	AM_RANGE(0x207000, 0x207001) AM_WRITE8(pal_b_w, 0x00ff)

	AM_RANGE(0x20c000, 0x20c001) AM_DEVREADWRITE8(CRTC_TAG, mc6845_device, status_r, address_w, 0x00ff)
	AM_RANGE(0x20c002, 0x20c003) AM_DEVREADWRITE8(CRTC_TAG, mc6845_device, register_r, register_w, 0x00ff)

	AM_RANGE(0x20e000, 0x20e001) AM_READWRITE(vbl_ack16_r, vbl_ack16_w)

	AM_RANGE(0x600000, 0x61ffff) AM_WRITE(vram_w)
	AM_RANGE(0x600000, 0x67ffff) AM_READ8(vram_r, 0x00ff)

	AM_RANGE(0x980000, 0xa7ffff) AM_RAM
ADDRESS_MAP_END

UINT32 hp16500_state::screen_update_hp16500a(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

static ADDRESS_MAP_START(hp16500_map, AS_PROGRAM, 32, hp16500_state)
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM AM_REGION("bios", 0)
	AM_RANGE(0x0020f000, 0x0020f003) AM_WRITE(palette_w)

	AM_RANGE(0x00202800, 0x00202803) AM_WRITE(vbl_ack_w)
	AM_RANGE(0x00203000, 0x00203003) AM_WRITE(vbl_ack_w)
	AM_RANGE(0x00209800, 0x00209803) AM_READ(vbl_state_r)

	AM_RANGE(0x0020b800, 0x0020b8ff) AM_RAM // system ram test is really strange.

	AM_RANGE(0x00600000, 0x0061ffff) AM_WRITE16(vram_w, 0xffffffff)
	AM_RANGE(0x00600000, 0x0067ffff) AM_READ8  (vram_r, 0x00ff00ff)
	AM_RANGE(0x00700000, 0x00700003) AM_WRITE8 (mask_w, 0xff000000)
	AM_RANGE(0x00740000, 0x00740003) AM_WRITE8 (val_w,  0xff000000)
	AM_RANGE(0x00800000, 0x009fffff) AM_RAM
ADDRESS_MAP_END

INTERRUPT_GEN_MEMBER(hp16500_state::vblank)
{
	m_maincpu->set_input_line(M68K_IRQ_1, ASSERT_LINE);
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

WRITE16_MEMBER(hp16500_state::vram_w)
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

READ8_MEMBER (hp16500_state::vram_r)
{
	return m_vram[offset];
}

WRITE8_MEMBER(hp16500_state::mask_w)
{
	m_mask = data;
}

WRITE8_MEMBER(hp16500_state::val_w)
{
	m_val = data;
}

WRITE32_MEMBER(hp16500_state::palette_w)
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
UINT32 hp16500_state::screen_update_hp16500(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int pos = 0;
	for (int y = 0; y < 384; y++)
	{
		UINT32 *scanline = &bitmap.pix32(y);

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

static MACHINE_CONFIG_START( hp1650, hp16500_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(MAINCPU_TAG, M68000, 10000000)
	MCFG_CPU_PROGRAM_MAP(hp1650_map)

	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_RAW_PARAMS(25000000, 0x330, 0, 0x250, 0x198, 0, 0x180 )
	MCFG_SCREEN_UPDATE_DEVICE( CRTC_TAG, mc6845_device, screen_update )

	MCFG_MC6845_ADD(CRTC_TAG, MC6845, SCREEN_TAG, 25000000/9)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(hp16500_state, crtc_update_row_1650)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(hp16500_state, vsync_changed))

	MCFG_DEVICE_ADD(DUART_TAG, MC68681, 20000000)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( hp1651, hp16500_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(MAINCPU_TAG, M68000, 10000000)
	MCFG_CPU_PROGRAM_MAP(hp1651_map)

	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_RAW_PARAMS(25000000, 0x330, 0, 0x250, 0x198, 0, 0x180 )
	MCFG_SCREEN_UPDATE_DEVICE( CRTC_TAG, mc6845_device, screen_update )

	MCFG_MC6845_ADD(CRTC_TAG, MC6845, SCREEN_TAG, 25000000/9)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(hp16500_state, crtc_update_row_1650)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(hp16500_state, vsync_changed))

	MCFG_DEVICE_ADD(DUART_TAG, MC68681, 20000000)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( hp16500a, hp16500_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(MAINCPU_TAG, M68000, 10000000)
	MCFG_CPU_PROGRAM_MAP(hp16500a_map)

	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_RAW_PARAMS(25000000, 0x320, 0, 0x240, 0x19c, 0, 0x170 )
	MCFG_SCREEN_UPDATE_DEVICE( CRTC_TAG, mc6845_device, screen_update )

	MCFG_MC6845_ADD(CRTC_TAG, MC6845, SCREEN_TAG, 25000000/9)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(hp16500_state, crtc_update_row)
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(hp16500_state, vsync_changed))

	MCFG_DEVICE_ADD(DUART_TAG, MC68681, 20000000)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( hp16500, hp16500_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(MAINCPU_TAG, M68EC030, 25000000)
	MCFG_CPU_PROGRAM_MAP(hp16500_map)
	MCFG_CPU_VBLANK_INT_DRIVER(SCREEN_TAG, hp16500_state, vblank)

	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(hp16500_state, screen_update_hp16500)
	MCFG_SCREEN_SIZE(576,384)
	MCFG_SCREEN_VISIBLE_AREA(0, 576-1, 0, 384-1)
	MCFG_SCREEN_REFRESH_RATE(60)

	MCFG_DEVICE_ADD(DUART_TAG, MC68681, 20000000)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
MACHINE_CONFIG_END

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

COMP( 1989, hp1650b,  0, 0, hp1650,  hp16500, driver_device, 0,  "Hewlett Packard", "HP 1650b", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
COMP( 1989, hp1651b,  0, 0, hp1651,  hp16500, driver_device, 0,  "Hewlett Packard", "HP 1651b", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
COMP( 1991, hp165ka0, 0, 0, hp16500a, hp16500, driver_device, 0, "Hewlett Packard", "HP 16500a", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
COMP( 1991, hp16500b, 0, 0, hp16500, hp16500, driver_device, 0,  "Hewlett Packard", "HP 16500b", MACHINE_NOT_WORKING|MACHINE_NO_SOUND)
