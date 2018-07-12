// license:GPL3.0+
// copyright-holders:Melissa Goad
/***************************************************************************

  iphone2g.cpp

  Driver file to handle emulation of the original iPhone
  
***************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "screen.h"

class iphone2g_state : public driver_device
{
public:
	iphone2g_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, "ram"),
		m_screen(*this, "screen"),
		m_bank1(*this, "bank1") { }

	void iphone2g(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

    DECLARE_READ32_MEMBER(clock1_r);

private:
    required_device<cpu_device> m_maincpu;
    required_shared_ptr<uint32_t> m_ram;
	required_device<screen_device> m_screen;
    required_memory_bank m_bank1;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map);
};

uint32_t iphone2g_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

READ32_MEMBER(iphone2g_state::clock1_r)
{
    uint32_t ret = 0;
    switch(offset)
    {
        case 0x40/4: ret = 1; break; //boot rom needs this to not infinite loop at startup.
    }

    logerror("%s: Clock1 read: offset %08x data %08x\n", machine().describe_context(), offset << 2, ret);

    return ret;
}

void iphone2g_state::mem_map(address_map &map)
{
	map(0x00000000, 0x07FFFFFF).mirror(0x18000000).bankrw(m_bank1).share(m_ram);       /* DRAM */
	map(0x20000000, 0x2000FFFF).rom().region("bios", 0);                               /* BIOS */
    map(0x22000000, 0x224FFFFF).ram();                                                 /* SRAM */
    map(0x3C500000, 0x3C500FFF).r(FUNC(iphone2g_state::clock1_r)).nopw();
}

void iphone2g_state::machine_start()
{
	/* configure overlay */
	m_bank1->configure_entry(0, m_ram);
	m_bank1->configure_entry(1, memregion("overlay")->base());
}

void iphone2g_state::machine_reset()
{
	/* start with overlay enabled */
	m_bank1->set_entry(1);
}

MACHINE_CONFIG_START(iphone2g_state::iphone2g)

	/* Basic machine hardware */
	MCFG_DEVICE_ADD(m_maincpu, ARM1176JZF_S, XTAL(12'000'000) * 103 / 3) //412 MHz, downclocked from 600 MHz
	MCFG_DEVICE_PROGRAM_MAP(mem_map)

	MCFG_SCREEN_ADD(m_screen, RASTER)
    MCFG_SCREEN_RAW_PARAMS(XTAL(12'000'000), 320, 0, 320, 480, 0, 480) //Complete guess
	MCFG_SCREEN_UPDATE_DRIVER(iphone2g_state, screen_update)
MACHINE_CONFIG_END

ROM_START(iphone2g)
    ROM_REGION(0x10000, "bios", 0)
    ROM_LOAD("s5l8900-bootrom.bin", 0x00000, 0x10000, CRC(beb15cd1) SHA1(079a3acab577eb52cc349ea811af3cbd5d01b8f5))
    ROM_REGION(0x10000, "overlay", 0)
	ROM_COPY("bios", 0, 0, 0x10000)
ROM_END

/*    YEAR  NAME     PARENT  COMPAT  MACHINE     INPUT   STATE       INIT        COMPANY            FULLNAME      FLAGS */
// console section
CONS( 2007, iphone2g, 0, 0, iphone2g, 0, iphone2g_state, empty_init, "Apple", "iPhone (A1203)", MACHINE_IS_SKELETON )