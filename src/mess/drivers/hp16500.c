/***************************************************************************
 
	Hewlett-Packard HP16500b Logic Analyzer

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
 
****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"

class hp16500_state : public driver_device
{
public:
	hp16500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	 { }                                      

	virtual void video_start();
	UINT32 screen_update_hp16500(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	dynamic_array<UINT8> m_vram;

	UINT8 m_mask, m_val;

	DECLARE_WRITE32_MEMBER(palette_w);

	DECLARE_WRITE16_MEMBER(vram_w);
	DECLARE_READ8_MEMBER  (vram_r);
	DECLARE_WRITE8_MEMBER (mask_w);
	DECLARE_WRITE8_MEMBER (val_w);
	DECLARE_READ32_MEMBER(vbl_state_r);
	DECLARE_WRITE32_MEMBER(vbl_ack_w);

	INTERRUPT_GEN_MEMBER(vblank);

private:
	UINT32 m_palette[256], m_colors[3], m_count, m_clutoffs;
};

READ32_MEMBER(hp16500_state::vbl_state_r)
{
	return 0x03000000;	// bit 0 set means the interrupt handler advances the pSOS tick counter.
}

WRITE32_MEMBER(hp16500_state::vbl_ack_w)
{
	m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
}

static ADDRESS_MAP_START(hp16500_map, AS_PROGRAM, 32, hp16500_state)
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM AM_REGION("bios", 0)
	AM_RANGE(0x0020f000, 0x0020f003) AM_WRITE(palette_w)

	AM_RANGE(0x00202800, 0x00202803) AM_WRITE(vbl_ack_w)
	AM_RANGE(0x00203000, 0x00203003) AM_WRITE(vbl_ack_w)
	AM_RANGE(0x00209800, 0x00209803) AM_READ(vbl_state_r)

	AM_RANGE(0x0020b800, 0x0020b8ff) AM_RAM	// system ram test is really strange.

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

static MACHINE_CONFIG_START( hp16500, hp16500_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68EC030, 25000000)
	MCFG_CPU_PROGRAM_MAP(hp16500_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", hp16500_state, vblank)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(hp16500_state, screen_update_hp16500)
	MCFG_SCREEN_SIZE(576,384)
	MCFG_SCREEN_VISIBLE_AREA(0, 576-1, 0, 384-1)
	MCFG_SCREEN_REFRESH_RATE(60)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
MACHINE_CONFIG_END

static INPUT_PORTS_START( hp16500 )
INPUT_PORTS_END

ROM_START( hp16500b )
	ROM_REGION32_BE(0x20000, "bios", 0)
		ROM_LOAD32_BYTE( "16500-80014.bin", 0x000000, 0x008000, CRC(35187716) SHA1(82067737892ecd356a5454a43d9ce9b38ac2397f) )
		ROM_LOAD32_BYTE( "16500-80015.bin", 0x000001, 0x008000, CRC(d8d26c1c) SHA1(b5b956c17c9d6e54519a686b5e4aa733b266bf6f) )
		ROM_LOAD32_BYTE( "16500-80016.bin", 0x000002, 0x008000, CRC(61457b39) SHA1(f209315ec22a8ee9d44a0ec009b1afb47794bece) )
		ROM_LOAD32_BYTE( "16500-80017.bin", 0x000003, 0x008000, CRC(e0b1096b) SHA1(426bb9a4756d8087bded4f6b61365d733ffbb09a) )
ROM_END

COMP( 1994, hp16500b, 0, 0, hp16500, hp16500, driver_device, 0,  "Hewlett Packard", "HP 16500b", GAME_NOT_WORKING|GAME_NO_SOUND)

