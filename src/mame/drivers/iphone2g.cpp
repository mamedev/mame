// license:BSD-3-Clause
// copyright-holders:Melissa Goad
/***************************************************************************

  iphone2g.cpp

  Driver file to handle emulation of the original iPhone

***************************************************************************/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "machine/bankdev.h"
#include "machine/vic_pl192.h"
#include "screen.h"

class iphone2g_state : public driver_device
{
public:
	iphone2g_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vic0(*this, "vic0"),
		m_vic1(*this, "vic1"),
		m_ram(*this, "ram"),
		m_bios(*this, "bios"),
		m_screen(*this, "screen")
	{ }

	void iphone2g(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_READ32_MEMBER(clock1_r);

private:
	required_device<cpu_device> m_maincpu;
	required_device<vic_pl192_device> m_vic0;
	required_device<vic_pl192_device> m_vic1;
	optional_shared_ptr<uint32_t> m_ram;
	required_region_ptr<uint32_t> m_bios;
	required_device<screen_device> m_screen;

	DECLARE_WRITE_LINE_MEMBER(irq_handler);
	DECLARE_WRITE_LINE_MEMBER(fiq_handler);

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
	map(0x00000000, 0x0000ffff).mirror(0x20000000).rom().region("bios", 0);            /* BIOS */
	map(0x22000000, 0x224fffff).ram();                                                 /* SRAM */
	map(0x38e00000, 0x38e00fff).m(m_vic0, FUNC(vic_pl192_device::map));
	map(0x38e01000, 0x38e01fff).m(m_vic1, FUNC(vic_pl192_device::map));
	map(0x3c500000, 0x3c500fff).r(FUNC(iphone2g_state::clock1_r)).nopw();
}

void iphone2g_state::machine_start()
{
	//std::copy_n(m_bios.target(), m_bios.length(), m_ram.target());
}

void iphone2g_state::machine_reset()
{
}

WRITE_LINE_MEMBER(iphone2g_state::irq_handler)
{
	m_maincpu->set_input_line(ARM7_IRQ_LINE, state);
}

WRITE_LINE_MEMBER(iphone2g_state::fiq_handler)
{
	m_maincpu->set_input_line(ARM7_FIRQ_LINE, state);
}

void iphone2g_state::iphone2g(machine_config &config)
{
	/* Basic machine hardware */
	ARM1176JZF_S(config, m_maincpu, XTAL(12'000'000) * 103 / 3); //412 MHz, downclocked from 600 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &iphone2g_state::mem_map);

	PL192_VIC(config, m_vic0);
	m_vic0->out_irq_cb().set(FUNC(iphone2g_state::irq_handler));
	m_vic0->out_fiq_cb().set(FUNC(iphone2g_state::fiq_handler));

	PL192_VIC(config, m_vic1);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(XTAL(12'000'000), 320, 0, 320, 480, 0, 480); //Complete guess
	m_screen->set_screen_update(FUNC(iphone2g_state::screen_update));
}

ROM_START(iphone2g)
	ROM_REGION32_LE(0x10000, "bios", 0)
	ROM_LOAD("s5l8900-bootrom.bin", 0x00000, 0x10000, CRC(beb15cd1) SHA1(079a3acab577eb52cc349ea811af3cbd5d01b8f5))
ROM_END

/*    YEAR  NAME     PARENT  COMPAT  MACHINE     INPUT   STATE       INIT        COMPANY            FULLNAME      FLAGS */
// console section
CONS( 2007, iphone2g, 0, 0, iphone2g, 0, iphone2g_state, empty_init, "Apple", "iPhone (A1203)", MACHINE_IS_SKELETON )
