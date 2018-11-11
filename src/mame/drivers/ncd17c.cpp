// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    drivers/ncd17c.cpp
    NCD 17" color X terminal

    Hardware:
        - MC68020 CPU, no FPU, no MMU
        - 2681 DUART (Logitech serial mouse)
        - Unknown AMD Ethernet chip (c) 1987.  LANCE or derivative?
        - Bt478 RAMDAC

    1c8000 mask ff000000 - DUART?
****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68681.h"
#include "screen.h"

class ncd_17c_state : public driver_device
{
public:
	ncd_17c_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_mainram(*this, "mainram"),
		m_duart(*this, "duart")
	{
	}

	void ncd_17c(machine_config &config);
	void ncd_17c_map(address_map &map);

	void init_ncd_17c();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(duart_irq_handler);
	DECLARE_WRITE32_MEMBER(bt478_palette_w);
	DECLARE_READ32_MEMBER(ramsize_r);
	DECLARE_WRITE32_MEMBER(ramsize_w);
	INTERRUPT_GEN_MEMBER(vblank);

private:
	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint32_t> m_mainram;
	required_device<scn2681_device> m_duart;

	inline void ATTR_PRINTF(3,4) verboselog( int n_level, const char *s_fmt, ... );

	u32 m_palette[256];
	u8 m_r, m_g, m_b, m_entry, m_stage;
	u8 m_ramsize_magic, m_ramsize_phase;
};


#define VERBOSE_LEVEL ( 0 )

#define ENABLE_VERBOSE_LOG (0)

inline void ATTR_PRINTF(3,4) ncd_17c_state::verboselog( int n_level, const char *s_fmt, ... )
{
#if ENABLE_VERBOSE_LOG
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror("%s: %s", machine().describe_context(), buf);
	}
#endif
}

void ncd_17c_state::machine_reset()
{
	m_entry = 0;
	m_stage = 0;
	m_r = m_g = m_b = 0;
	m_ramsize_phase = 0;
}

INTERRUPT_GEN_MEMBER(ncd_17c_state::vblank)
{
	m_maincpu->set_input_line(M68K_IRQ_5, HOLD_LINE);
}


uint32_t ncd_17c_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t *scanline;
	int x, y;
	uint8_t pixels;
	uint8_t *m_vram = (uint8_t *)m_mainram.target();

	for (y = 0; y < 768; y++)
	{
		scanline = &bitmap.pix32(y);
		for (x = 0; x < 1024; x++)
		{
			pixels = m_vram[(y * 1024) + (BYTE4_XOR_BE(x))];
			*scanline++ = m_palette[pixels];
		}
	}

	return 0;
}

void ncd_17c_state::ncd_17c_map(address_map &map)
{
	map(0x00000000, 0x000bffff).rom().region("maincpu", 0);
	map(0x001c8000, 0x001c803f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0xff000000);
	map(0x001d0000, 0x001d0003).w(FUNC(ncd_17c_state::bt478_palette_w));
	map(0x01000000, 0x02ffffff).ram(); //w(FUNC(ncd_17c_state::ramsize_r), FUNC(ncd_17c_state::ramsize_w));
	map(0x03000000, 0x03ffffff).ram().share("mainram");
}

READ32_MEMBER(ncd_17c_state::ramsize_r)
{
	return m_ramsize_magic << 24;
}

WRITE32_MEMBER(ncd_17c_state::ramsize_w)
{
	if (!m_ramsize_phase)
	{
		m_ramsize_magic = (data >> 24);
	}
	m_ramsize_phase ^= 1;
}

WRITE_LINE_MEMBER(ncd_17c_state::duart_irq_handler)
{
	//m_maincpu->set_input_line_and_vector(M68K_IRQ_6, state, M68K_INT_ACK_AUTOVECTOR);
}

WRITE32_MEMBER(ncd_17c_state::bt478_palette_w)
{
	if (mem_mask & 0xff000000)
	{
		m_entry = data >> 24;
		m_stage = 0;
		m_r = m_g = m_b = 0;
	}
	else if (mem_mask & 0x00ff0000)
	{
		switch (m_stage)
		{
			case 0:
				m_r = (data>>16) & 0xff;
				m_stage++;
				break;

			case 1:
				m_g = (data>>16) & 0xff;
				m_stage++;
				break;

			case 2:
				m_b = (data>>16) & 0xff;
				m_palette[m_entry] = rgb_t(m_r, m_g, m_b);
				//printf("palette[%d] = RGB(%02x, %02x, %02x)\n", m_entry, m_r, m_g, m_b);
				m_entry++;
				m_entry &= 0xff;
				m_stage = 0;
				m_r = m_g = m_b = 0;
				break;
		}
	}
	else if (mem_mask & 0x0000ff00)
	{
		// pixel mask register, unsure if this is used yet
	}
}

void ncd_17c_state::ncd_17c(machine_config &config)
{
	/* basic machine hardware */
	M68020(config, m_maincpu, 20000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ncd_17c_state::ncd_17c_map);
	m_maincpu->set_periodic_int(FUNC(ncd_17c_state::vblank), attotime::from_hz(72));

	SCN2681(config, m_duart, 3.6864_MHz_XTAL);
	m_duart->irq_cb().set(FUNC(ncd_17c_state::duart_irq_handler));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(72);
	m_screen->set_visarea(0, 1024-1, 0, 768-1);
	m_screen->set_size(1152, 800);
	m_screen->set_screen_update(FUNC(ncd_17c_state::screen_update));
}

static INPUT_PORTS_START( ncd_17c )
INPUT_PORTS_END

void ncd_17c_state::init_ncd_17c()
{
//  uint32_t *src = (uint32_t*)(memregion("maincpu")->base());
//  uint32_t *dst = m_mainram;
//  memcpy(dst, src, 8);

//  m_maincpu->reset();
}

/***************************************************************************

  ROM definition(s)

***************************************************************************/

ROM_START( ncd17c )
	ROM_REGION32_BE(0xc0000, "maincpu", 0)
	ROM_LOAD16_BYTE( "ncd17c_v2.2.1_b0e.bin", 0x000000, 0x020000, CRC(c47648af) SHA1(563e17ea8f5c9d418fd81f1e797a226937c0e187) )
	ROM_LOAD16_BYTE( "ncd17c_v2.2.1_b0o.bin", 0x000001, 0x020000, CRC(b6a8c3ca) SHA1(f02e33d88861ebcb402fb554719c1cb072a5fd14) )
	ROM_LOAD16_BYTE( "ncd17c_v2.2.1_b1e.bin", 0x040000, 0x020000, CRC(25500987) SHA1(cebdf07c69f1c783a67b92d6efdfdd7067dc910f) )
	ROM_LOAD16_BYTE( "ncd17c_v2.2.1_b1o.bin", 0x040001, 0x020000, CRC(a5d5ab8a) SHA1(51ddb7020abd5f83224bff48eab254375e9d27f9) )
	ROM_LOAD16_BYTE( "ncd17c_v2.2.1_b2e.bin", 0x080000, 0x020000, CRC(390dac65) SHA1(3f9c886433dff87847135b8f3d8e8ead75d3abf3) )
	ROM_LOAD16_BYTE( "ncd17c_v2.2.1_b2o.bin", 0x080001, 0x020000, CRC(2e5ebfaa) SHA1(d222c6cc743046a1c1dec1829c24fa918a54849d) )
ROM_END

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT          COMPANY                 FULLNAME           FLAGS
COMP( 1990, ncd17c, 0,      0,      ncd_17c, ncd_17c, ncd_17c_state, init_ncd_17c, "Network Computing Devices", "NCD-17C", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
