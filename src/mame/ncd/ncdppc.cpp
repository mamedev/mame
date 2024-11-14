// license:BSD-3-Clause
// copyright-holders:R. Belmont
/****************************************************************************

    drivers/ncdppc.cpp
    NCD PowerPC-based color X terminals

    Hardware (Explora Pro):
        - PPC 403GA CPU
        - Custom PCI bridge
        - AM79C965 (? photos are not conclusive) "PCNet" PCI Ethernet
        - S3 86C868 Vision868 PCI graphics
        - National Semiconductor PC87303 PCI "SuperI/O"
        - PS/2 keyboard port

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/powerpc/ppc.h"
#include "screen.h"


namespace {

class ncd_ppc_state : public driver_device
{
public:
	ncd_ppc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_mainram(*this, "mainram")
	{
	}

	void explorapro(machine_config &config);
	void explorapro_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_shared_ptr<uint32_t> m_mainram;

	uint32_t unk_r();
	void tty_w(uint32_t data);
};


void ncd_ppc_state::machine_reset()
{
}

uint32_t ncd_ppc_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

uint32_t ncd_ppc_state::unk_r()
{
	return 0xffffffff;
}

void ncd_ppc_state::tty_w(uint32_t data)
{
	printf("%c", (data>>24) & 0x7f);
}

void ncd_ppc_state::explorapro_map(address_map &map)
{
	map(0x01000000, 0x3fffffff).ram();

	map(0x740002f8, 0x740002fb).w(FUNC(ncd_ppc_state::tty_w));
	map(0x740002fc, 0x740002ff).r(FUNC(ncd_ppc_state::unk_r));
	map(0x7ffc0000, 0x7fffffff).rom().region("maincpu", 0);
}

void ncd_ppc_state::explorapro(machine_config &config)
{
	/* basic machine hardware */
	PPC403GA(config, m_maincpu, 50000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ncd_ppc_state::explorapro_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(77.4144_MHz_XTAL, 1376, 0, 1024, 803, 0, 768);
	m_screen->set_screen_update(FUNC(ncd_ppc_state::screen_update));
}

static INPUT_PORTS_START( explorapro )
INPUT_PORTS_END

/***************************************************************************

  ROM definition(s)

***************************************************************************/

ROM_START( explorapro )
	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD( "explora__v2.7.6_bm.u37", 0x000000, 0x040000, CRC(038fb1dc) SHA1(036836359f59e70d5bebc50d69083bbe020ddf98) )
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT  COMPAT  MACHINE       INPUT    CLASS          INIT               COMPANY                 FULLNAME           FLAGS
COMP( 1995, explorapro, 0,      0,   explorapro, explorapro, ncd_ppc_state,  empty_init,   "Network Computing Devices", "NCD Explora Pro XQ", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )
