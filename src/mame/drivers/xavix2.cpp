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

class xavix2_state : public driver_device
{
public:
	xavix2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
	{ }

	void xavix2(machine_config &config);

private:
	enum {
		IRQ_TIMER =  7,
		IRQ_DMA   = 12
	};

	required_device<xavix2_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	u32 m_dma_src;
	u16 m_dma_dst;
	u16 m_dma_count;
	emu_timer *m_dma_timer;

	u32 m_int_active;

	std::string m_debug_string;

	void irq_raise(u32 level);
	void irq_clear(u32 level);
	bool irq_state(u32 level) const;
	u8 irq_level_r();

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

	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mem(address_map &map);
};

void xavix2_state::irq_raise(u32 level)
{
	if(!m_int_active)
		m_maincpu->set_input_line(0, ASSERT_LINE);
	m_int_active |= 1 << level;
}

void xavix2_state::irq_clear(u32 level)
{
	m_int_active &= ~(1 << level);
	if(!m_int_active)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

u8 xavix2_state::irq_level_r()
{
	for(u32 i=31; i>=0; i--)
		if(m_int_active & (1 << i))
			return i;
	return 0xff;
}

bool xavix2_state::irq_state(u32 level) const
{
	return m_int_active & (1 << level);
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
	return irq_state(IRQ_DMA) ? 7 : 0;
}

TIMER_CALLBACK_MEMBER(xavix2_state::dma_end)
{
	irq_raise(IRQ_DMA);
}

INTERRUPT_GEN_MEMBER(xavix2_state::vblank_irq)
{
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



void xavix2_state::mem(address_map &map)
{
	map(0x00000000, 0x0000ffff).ram().share("mainram");
	map(0x00010000, 0x00ffffff).rom().region("maincpu", 0x010000);

	map(0x40000000, 0x40ffffff).rom().region("maincpu", 0);

	map(0xc0000000, 0xc000ffff).ram().share("mainram");
	map(0xc0010000, 0xc001ffff).ram();

	map(0xffffe000, 0xffffe003).w(FUNC(xavix2_state::dma_src_w));
	map(0xffffe004, 0xffffe005).w(FUNC(xavix2_state::dma_dst_w));
	map(0xffffe008, 0xffffe009).w(FUNC(xavix2_state::dma_count_w));
	map(0xffffe00c, 0xffffe00c).w(FUNC(xavix2_state::dma_control_w));
	map(0xffffe010, 0xffffe010).rw(FUNC(xavix2_state::dma_status_r), FUNC(xavix2_state::dma_status_w));

	map(0xffffe238, 0xffffe238).rw(FUNC(xavix2_state::debug_port_r), FUNC(xavix2_state::debug_port_w));
	map(0xffffe239, 0xffffe239).r(FUNC(xavix2_state::debug_port_status_r));

	map(0xffffe630, 0xffffe631).lr16(NAME([]() { return 0x210; }));
	map(0xffffe632, 0xffffe633).lr16(NAME([]() { return 0x210; }));
	map(0xfffffc00, 0xfffffc00).r(FUNC(xavix2_state::irq_level_r));
}

uint32_t xavix2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
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
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 256);

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




