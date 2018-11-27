// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    drivers/ncd17c.cpp
    NCD 17" color X terminal
    NCD 19" monochrome X terminal

    Hardware:
        - MC68020 CPU, no FPU, no MMU
        - 2681 DUART (Logitech serial mouse)
        - AMD LANCE Ethernet controller
        - Bt478 RAMDAC
        - MC6805 keyboard and NVRAM handler

        68020 IRQs: 2 = keyboard, 3 = LANCE, 4 = DUART, 5 = vblank

        6805 port assignments:
        A0 - I/O - PS/2 connector DATA
        A1
        A2 - not sure
        A3 - OUT - chip select on the 93C46 EEPROM
        A4 - OUT - when clear port B reads as the mailslot from the 68020
        A5 - IN  - set if the 68020 has nothing new for us
        A6 - OUT - rising edge latches port B to the 68020 mailslot and raises the IRQ
        A7 - IN  - set if the 68020 hasn't yet read our last transmission

        B0 - OUT - SK line on 93C46 EEPROM
        B1 - OUT - Data In line on 93C46 EEPROM
        B2 - IN  - Data Out line on 93C46 EEPROM
        B3-B7 unused except '020 mailslot interface

        C0-C3 = speaker (4-bit DAC?)  The 6805 timer is used to control this.

        IRQ in = PS/2 clock line

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6805/m6805.h"
#include "machine/mc68681.h"
#include "machine/am79c90.h"
#include "screen.h"

class ncd_020_state : public driver_device
{
public:
	ncd_020_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_screen(*this, "screen"),
		m_mainram(*this, "mainram"),
		m_vram(*this, "vram"),
		m_duart(*this, "duart"),
		m_lance(*this, "am79c90")
	{
	}

	void ncd_17c(machine_config &config);
	void ncd_17c_map(address_map &map);
	void ncd_19(machine_config &config);
	void ncd_19_map(address_map &map);
	void ncd_mcu_map(address_map &map);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_19(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(duart_irq_handler);
	DECLARE_WRITE_LINE_MEMBER(lance_irq_w);
	DECLARE_READ16_MEMBER(lance_dma_r);
	DECLARE_WRITE16_MEMBER(lance_dma_w);
	DECLARE_READ16_MEMBER(lance19_dma_r);
	DECLARE_WRITE16_MEMBER(lance19_dma_w);
	DECLARE_WRITE32_MEMBER(bt478_palette_w);
	DECLARE_READ32_MEMBER(from_mcu_r);
	DECLARE_WRITE32_MEMBER(to_mcu_w);
	DECLARE_READ32_MEMBER(mcu_status_r);
	DECLARE_WRITE32_MEMBER(irq_w);
	INTERRUPT_GEN_MEMBER(vblank);
	DECLARE_READ8_MEMBER(mcu_ports_r);
	DECLARE_WRITE8_MEMBER(mcu_ports_w);

private:
	virtual void machine_reset() override;

	required_device<m68020_device> m_maincpu;
	required_device<m6805_device> m_mcu;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint32_t> m_mainram;
	optional_shared_ptr<uint32_t> m_vram;
	required_device<scn2681_device> m_duart;
	required_device<am7990_device> m_lance;

	inline void ATTR_PRINTF(3,4) verboselog( int n_level, const char *s_fmt, ... );

	u32 m_palette[256];
	u8 m_r, m_g, m_b, m_entry, m_stage;
	u8 m_porta, m_portb, m_portc, m_to020, m_from020;
	bool m_unread_to020, m_unread_from020;
};

#define VERBOSE_LEVEL ( 0 )

#define ENABLE_VERBOSE_LOG (0)

inline void ATTR_PRINTF(3,4) ncd_020_state::verboselog( int n_level, const char *s_fmt, ... )
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

void ncd_020_state::machine_reset()
{
	m_entry = 0;
	m_stage = 0;
	m_r = m_g = m_b = 0;
	m_porta = m_portb = m_portc = 0;
	m_to020 = m_from020 = 0;
	m_unread_to020 = m_unread_from020 = false;
}

INTERRUPT_GEN_MEMBER(ncd_020_state::vblank)
{
	m_maincpu->set_input_line(M68K_IRQ_5, HOLD_LINE);
}


uint32_t ncd_020_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t *scanline;
	int x, y;
	uint8_t pixels;
	uint8_t *vram = (uint8_t *)m_mainram.target();

	for (y = 0; y < 768; y++)
	{
		scanline = &bitmap.pix32(y);
		for (x = 0; x < 1024; x++)
		{
			pixels = vram[(y * 1024) + (BYTE4_XOR_BE(x))];
			*scanline++ = m_palette[pixels];
		}
	}

	return 0;
}

uint32_t ncd_020_state::screen_update_19(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t *scanline;
	int x, y;
	uint8_t pixels;
	static const uint32_t palette[2] = { 0, 0xffffff };
	uint8_t *vram = (uint8_t *)m_vram.target();

	for (y = 0; y < 1024; y++)
	{
		scanline = &bitmap.pix32(y);
		for (x = 0; x < 1024/8; x++)
		{
			pixels = vram[(y * (2048/8)) + (BYTE4_XOR_BE(x))];

			*scanline++ = palette[(pixels>>7)&1];
			*scanline++ = palette[(pixels>>6)&1];
			*scanline++ = palette[(pixels>>5)&1];
			*scanline++ = palette[(pixels>>4)&1];
			*scanline++ = palette[(pixels>>3)&1];
			*scanline++ = palette[(pixels>>2)&1];
			*scanline++ = palette[(pixels>>1)&1];
			*scanline++ = palette[(pixels&1)];
		}
	}

	return 0;
}

void ncd_020_state::ncd_17c_map(address_map &map)
{
	map(0x00000000, 0x000bffff).rom().region("maincpu", 0);
	map(0x001c0000, 0x001c0003).rw(FUNC(ncd_020_state::from_mcu_r), FUNC(ncd_020_state::to_mcu_w));
	map(0x001c8000, 0x001c803f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0xff000000);
	map(0x001d0000, 0x001d0003).w(FUNC(ncd_020_state::bt478_palette_w));
	map(0x001d8000, 0x001d8003).rw(FUNC(ncd_020_state::mcu_status_r), FUNC(ncd_020_state::irq_w));
	map(0x00200000, 0x00200003).rw(m_lance, FUNC(am79c90_device::regs_r), FUNC(am79c90_device::regs_w)).umask32(0xffffffff);
	map(0x01000000, 0x02ffffff).ram();
	map(0x03000000, 0x03ffffff).ram().share("mainram");
}

void ncd_020_state::ncd_19_map(address_map &map)
{
	map(0x00000000, 0x0000ffff).rom().region("maincpu", 0);
	map(0x001c0000, 0x001c0003).rw(FUNC(ncd_020_state::from_mcu_r), FUNC(ncd_020_state::to_mcu_w));
	map(0x001d8000, 0x001d8003).rw(FUNC(ncd_020_state::mcu_status_r), FUNC(ncd_020_state::irq_w));
	map(0x001e0000, 0x001e003f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0xff000000);
	map(0x00200000, 0x00200003).rw(m_lance, FUNC(am79c90_device::regs_r), FUNC(am79c90_device::regs_w)).umask32(0xffffffff);
	map(0x00400000, 0x0043ffff).ram().share("vram");
	map(0x00800000, 0x00ffffff).ram().share("mainram");
}

READ8_MEMBER(ncd_020_state::mcu_ports_r)
{
	u8 rv = 0;

	switch (offset)
	{
		case 0: // port A
			rv = m_porta & ~(0xa5);
			if (!m_unread_from020)
			{
				rv |= 0x20;
			}
			if (m_unread_to020)
			{
				rv |= 0x80;
			}
			break;

		case 1: // port B
			if (!(m_porta & 0x10))
			{
				rv = m_from020;
			}
			else
			{
				rv = m_portb;
			}
			break;

		case 2: // port C
			rv = m_portc & 0xf;
			rv |= 0xf0;
			break;
	}

	return rv;
}

WRITE8_MEMBER(ncd_020_state::mcu_ports_w)
{
	switch (offset)
	{
		case 0: // port A
			//if (data != m_porta) printf("%02x to port A\n", data);
			if ((data & 0x40) && !(m_porta & 0x40))
			{
				//printf("Sending %02x to 020\n", m_portb);
				m_to020 = m_portb;
				m_unread_to020 = true;
				m_maincpu->set_input_line(M68K_IRQ_2, ASSERT_LINE);
			}
			m_porta = data;
			break;

		case 1: // port B
			//printf("%02x to port B\n", data);
			m_portb = data;
			break;

		case 2: // port C
			m_portc = data;
			break;
	}
}

READ32_MEMBER(ncd_020_state::from_mcu_r)
{
	//printf("020 read %02x\n", m_to020);
	m_unread_to020 = false;
	m_maincpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);
	return m_to020<<24;
}

WRITE32_MEMBER(ncd_020_state::to_mcu_w)
{
	//printf("Sending %02x to MCU\n", data);
	m_from020 = data>>24;
	m_unread_from020 = true;
	m_maincpu->yield();
}

READ32_MEMBER(ncd_020_state::mcu_status_r)
{
	u32 rv = 0;
	if (!m_unread_from020)
	{
		rv |= 0x01000000;
	}
	if (m_unread_to020)
	{
		rv |= 0x02000000;
	}

	return rv;
}

WRITE32_MEMBER(ncd_020_state::irq_w)
{
	if (data & 0x80000000)
	{
		m_maincpu->set_input_line(M68K_IRQ_1, ASSERT_LINE);
	}
	else
	{
		m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
	}
}

void ncd_020_state::ncd_mcu_map(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x0000, 0x0002).rw(FUNC(ncd_020_state::mcu_ports_r), FUNC(ncd_020_state::mcu_ports_w));
	map(0x0040, 0x007f).ram();
	map(0x0080, 0x00ff).rom().region("mcu", 0x80);
	map(0x03c0, 0x07ff).rom().region("mcu", 0x3c0);
}

WRITE_LINE_MEMBER(ncd_020_state::duart_irq_handler)
{
	m_maincpu->set_input_line(M68K_IRQ_4, state);
}

WRITE_LINE_MEMBER(ncd_020_state::lance_irq_w)
{
	m_maincpu->set_input_line(M68K_IRQ_3, state);
}

READ16_MEMBER(ncd_020_state::lance_dma_r)
{
	u32 const data = m_mainram.target()[offset >> 2];

	return (offset & 2) ? u16(data) : u16(data >> 16);
}

WRITE16_MEMBER(ncd_020_state::lance_dma_w)
{
	u32 const existing = m_mainram.target()[offset >> 2];

	if (offset & 2)
		m_mainram.target()[offset >> 2] = (existing & (0xffff0000U | ~mem_mask)) | (data & mem_mask);
	else
		m_mainram.target()[offset >> 2] = (existing & (0x0000ffffU | ~(mem_mask << 16))) | ((data & mem_mask) << 16);
}

READ16_MEMBER(ncd_020_state::lance19_dma_r)
{
	if (offset < 0x800000)
	{
		fatalerror("ncd17c.cpp: DMA target %08x not handled!", offset);
	}

	offset &= 0x7fffff;
	u32 const data = m_mainram.target()[offset >> 2];

	return (offset & 2) ? u16(data) : u16(data >> 16);
}

WRITE16_MEMBER(ncd_020_state::lance19_dma_w)
{
	if (offset < 0x800000)
	{
		fatalerror("ncd17c.cpp: DMA target %08x not handled!", offset);
	}

	offset &= 0x7fffff;
	u32 const existing = m_mainram.target()[offset >> 2];

	if (offset & 2)
		m_mainram.target()[offset >> 2] = (existing & (0xffff0000U | ~mem_mask)) | (data & mem_mask);
	else
		m_mainram.target()[offset >> 2] = (existing & (0x0000ffffU | ~(mem_mask << 16))) | ((data & mem_mask) << 16);
}

WRITE32_MEMBER(ncd_020_state::bt478_palette_w)
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

void ncd_020_state::ncd_17c(machine_config &config)
{
	/* basic machine hardware */
	M68020(config, m_maincpu, 20000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ncd_020_state::ncd_17c_map);
	m_maincpu->set_periodic_int(FUNC(ncd_020_state::vblank), attotime::from_hz(70.06));

	M6805(config, m_mcu, 3.6864_MHz_XTAL);  // MC6805P2
	m_mcu->set_addrmap(AS_PROGRAM, &ncd_020_state::ncd_mcu_map);

	SCN2681(config, m_duart, 3.6864_MHz_XTAL);
	m_duart->irq_cb().set(FUNC(ncd_020_state::duart_irq_handler));

	AM7990(config, m_lance);
	m_lance->intr_out().set(FUNC(ncd_020_state::lance_irq_w)).invert();
	m_lance->dma_in().set(FUNC(ncd_020_state::lance_dma_r));
	m_lance->dma_out().set(FUNC(ncd_020_state::lance_dma_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(77.4144_MHz_XTAL, 1376, 0, 1024, 803, 0, 768); // 56.260 kHz horizontal, 70.06 Hz vertical
	m_screen->set_screen_update(FUNC(ncd_020_state::screen_update));
}

void ncd_020_state::ncd_19(machine_config &config)
{
	/* basic machine hardware */
	M68020(config, m_maincpu, 15000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ncd_020_state::ncd_19_map);
	m_maincpu->set_periodic_int(FUNC(ncd_020_state::vblank), attotime::from_hz(72));

	M6805(config, m_mcu, 3.6864_MHz_XTAL);  // MC6805P2
	m_mcu->set_addrmap(AS_PROGRAM, &ncd_020_state::ncd_mcu_map);

	SCN2681(config, m_duart, 3.6864_MHz_XTAL);
	m_duart->irq_cb().set(FUNC(ncd_020_state::duart_irq_handler));

	AM7990(config, m_lance);
	m_lance->intr_out().set(FUNC(ncd_020_state::lance_irq_w)).invert();
	m_lance->dma_in().set(FUNC(ncd_020_state::lance19_dma_r));
	m_lance->dma_out().set(FUNC(ncd_020_state::lance19_dma_w));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(72);
	m_screen->set_visarea(0, 1280-1, 0, 1024-1);
	m_screen->set_size(1400, 1152);
	m_screen->set_screen_update(FUNC(ncd_020_state::screen_update_19));
}

static INPUT_PORTS_START( ncd_17c )
INPUT_PORTS_END

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

	ROM_REGION(0x800, "mcu", 0)
	ROM_LOAD( "ncd4200005.bin", 0x000000, 0x000800, CRC(075c3746) SHA1(6954cfab5141138df975f1b15d2c8e08d4d203c1) )
ROM_END

ROM_START( ncd19 )
	ROM_REGION32_BE(0x10000, "maincpu", 0)
	ROM_LOAD16_BYTE( "ncd19_v2.1.1_e.bin", 0x000000, 0x008000, CRC(28786528) SHA1(8f4ad6a593c55cce0477169132ecf38577086f4e) )
	ROM_LOAD16_BYTE( "ncd19_v2.1.1_o.bin", 0x000001, 0x008000, CRC(aeefbcf1) SHA1(0c28426d0ae7c18de02daee7d340c17dc461e7f4) )

	ROM_REGION(0x800, "mcu", 0)
	ROM_LOAD( "ncd4200005.bin", 0x000000, 0x000800, CRC(075c3746) SHA1(6954cfab5141138df975f1b15d2c8e08d4d203c1) )
ROM_END

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT               COMPANY                 FULLNAME           FLAGS
COMP( 1990, ncd17c, 0,      0,      ncd_17c, ncd_17c, ncd_020_state,  empty_init,   "Network Computing Devices", "NCD-17C", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
COMP( 1990, ncd19,  0,      0,      ncd_19,  ncd_17c, ncd_020_state,  empty_init,   "Network Computing Devices", "NCD-19", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
