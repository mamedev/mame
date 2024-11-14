// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    drivers/ncdmips.cpp
    NCD MIPS-based color X terminals

    Hardware:
        - R4600 CPU
        - 2681 DUART (Logitech serial mouse)
        - AM79C950 Ethernet
        - AT&T ATT21C505 "PrecisionDAC" for audio out
        - PS/2 keyboard port

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/mips/mips3.h"
#include "machine/mc68681.h"
#include "screen.h"


namespace {

class ncd_mips_state : public driver_device
{
public:
	ncd_mips_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_mainram(*this, "mainram"),
		m_duart(*this, "duart")
	{
	}

	void hmxpro(machine_config &config);
	void hmxpro_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void duart_irq_handler(int state);
	INTERRUPT_GEN_MEMBER(vblank);

private:
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_shared_ptr<uint32_t> m_mainram;
	required_device<scn2681_device> m_duart;

	u32 unk_r();
	void tty_w(u32 data);

//  u32 m_palette[256];
//  u8 m_r, m_g, m_b, m_entry, m_stage;
};


void ncd_mips_state::machine_reset()
{
//  m_entry = 0;
//  m_stage = 0;
//  m_r = m_g = m_b = 0;
}

INTERRUPT_GEN_MEMBER(ncd_mips_state::vblank)
{
}


uint32_t ncd_mips_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

u32 ncd_mips_state::unk_r()
{
	return 0xffffffff;
}

void ncd_mips_state::tty_w(u32 data)
{
	printf("%c", (data>>16) & 0x7f);
}

void ncd_mips_state::hmxpro_map(address_map &map)
{
	map(0x00000000, 0x003fffff).ram();  // VRAM
	map(0x10000000, 0x103fffff).ram();
	map(0x18000028, 0x1800002b).r(FUNC(ncd_mips_state::unk_r));
	map(0x18000058, 0x1800005b).w(FUNC(ncd_mips_state::tty_w));
	map(0x19000010, 0x19000013).r(FUNC(ncd_mips_state::unk_r));
	map(0x1b000000, 0x1b00007f).rw(m_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write)).umask32(0xff000000);

	map(0x1fc00000, 0x1fc3ffff).rom().region("maincpu", 0);
	map(0x20000000, 0x207fffff).ram();
}

void ncd_mips_state::duart_irq_handler(int state)
{
	//m_maincpu->set_input_line(M68K_IRQ_6, state);
}

void ncd_mips_state::hmxpro(machine_config &config)
{
	/* basic machine hardware */
	R4600BE(config, m_maincpu, 50000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ncd_mips_state::hmxpro_map);
	m_maincpu->set_periodic_int(FUNC(ncd_mips_state::vblank), attotime::from_hz(70.06));

	SCN2681(config, m_duart, 3.6864_MHz_XTAL);
	m_duart->irq_cb().set(FUNC(ncd_mips_state::duart_irq_handler));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(77.4144_MHz_XTAL, 1376, 0, 1024, 803, 0, 768);
	m_screen->set_screen_update(FUNC(ncd_mips_state::screen_update));
}

static INPUT_PORTS_START( hmxpro )
INPUT_PORTS_END

/***************************************************************************

  ROM definition(s)

***************************************************************************/

ROM_START( hmxpro )
	ROM_REGION32_BE(0x40000, "maincpu", 0)
	ROM_LOAD16_BYTE( "ncdhmx_bm_v2.7.2_b0e.bin", 0x000000, 0x020000, CRC(66072e5c) SHA1(a12dbd3befda55f755e684ba6e5c3b067f2ded93) )
	ROM_LOAD16_BYTE( "ncdhmx_bm_v2.7.2_b0o.bin", 0x000001, 0x020000, CRC(7f7af795) SHA1(5b31bda8cb42dfb52869d29637fe415e43aa53f4) )
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT               COMPANY                 FULLNAME           FLAGS
COMP( 1994, hmxpro, 0,      0,        hmxpro, hmxpro, ncd_mips_state,  empty_init,   "Network Computing Devices", "NCD HMX PRO", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
