// license:BSD-3-Clause
// copyright-holders:David Haywood

/************************************************************************

 Prin-C use a Fujitsu MB90611A MCU (F2MC-16L)

[:maincpu] ':maincpu' (FFC431): unmapped program memory write to 0000A1 = 00 & FF  CKSCR
[:maincpu] ':maincpu' (FFC437): unmapped program memory write to 000048 = 04 & FF  CS control 0  (Enable out, region is 1 MByte @ F00000)
[:maincpu] ':maincpu' (FFC43D): unmapped program memory write to 000049 = 04 & FF  CS control 1  (Enable out, region is 1 MByte @ E00000)
[:maincpu] ':maincpu' (FFC443): unmapped program memory write to 00004A = 07 & FF  CS control 2  (Enable out, region is 128 byte @ 68FF80)
[:maincpu] ':maincpu' (FFC449): unmapped program memory write to 00004B = 00 & FF  CS control 3  (No out, region is reserved)
[:maincpu] ':maincpu' (FFC44F): unmapped program memory write to 0000A5 = D3 & FF  ARSR (3 cycle wait state from addrs 002000 to 7FFFFF, 3 waits from C0 to FF, 1 cycle wait on addresses > 800000)
[:maincpu] ':maincpu' (FFC455): unmapped program memory write to 0000A6 = 00 & FF  HACR ()
[:maincpu] ':maincpu' (FFC45B): unmapped program memory write to 0000A7 = 7F & FF  ECSR
[:maincpu] ':maincpu' (FFC461): unmapped program memory write to 000011 = 00 & FF  Port 1 DDR
[:maincpu] ':maincpu' (FFC467): unmapped program memory write to 000012 = FF & FF       2
[:maincpu] ':maincpu' (FFC46D): unmapped program memory write to 000013 = FF & FF       3
[:maincpu] ':maincpu' (FFC473): unmapped program memory write to 000014 = FF & FF       4
[:maincpu] ':maincpu' (FFC479): unmapped program memory write to 000015 = 01 & FF       5
[:maincpu] ':maincpu' (FFC47F): unmapped program memory write to 000016 = 1F & FF  Analog input enable
[:maincpu] ':maincpu' (FFC485): unmapped program memory write to 000016 = E0 & FF       7
[:maincpu] ':maincpu' (FFC48B): unmapped program memory write to 000017 = 30 & FF       8
[:maincpu] ':maincpu' (FFC491): unmapped program memory write to 000018 = 0C & FF       9
[:maincpu] ':maincpu' (FFC497): unmapped program memory write to 00001A = FF & FF       A
[:maincpu] ':maincpu' (FFC189): unmapped program memory write to 00000A = 00 & FF  port A
[:maincpu] ':maincpu' (FFC257): unmapped program memory write to 00000A = 80 & FF  port A
[:maincpu] ':maincpu' (FE2C08): unmapped program memory write to 0000A9 = 96 & FF  TBTC - IRQ enabled, 16.384 ms timebase
[:maincpu] ':maincpu' (FE2C11): unmapped program memory write to 0000BB = 06 & FF  ICR11 - level 6 interrupt, no intelligent I/O
[:maincpu] ':maincpu' (FE2959): unmapped program memory write to 000017 = 30 & FF  port 7 DDR
[:maincpu] ':maincpu' (FE2963): unmapped program memory write to 0000A9 = 96 & FF  TBTC
[:maincpu] ':maincpu' (FE296C): unmapped program memory write to 0000BB = 06 & FF  ICR11
[:maincpu] ':maincpu' (FE29CC): unmapped program memory write to 000007 = 00 & FF  port 7 out
[:maincpu] ':maincpu' (FE2A69): unmapped program memory write to 0000A9 = 96 & FF  TBTC
[:maincpu] ':maincpu' (FE2A72): unmapped program memory write to 0000BB = 06 & FF  ICR11
[:maincpu] ':maincpu' (FC2AD5): unmapped program memory write to 000018 = 0C & FF  port 8 DDR
[:maincpu] ':maincpu' (FC2ADE): unmapped program memory write to 000039 = 0C & FF  TMCSR0 (clock = phase 16 MHz / 2^1, trigger input, rising edge)
[:maincpu] ':maincpu' (FC2AE8): unmapped program memory write to 000038 = F0 & FF  TMCSR0 (toggle output, H Level at start, no count or interrupt enable)
[:maincpu] ':maincpu' (FC2AF1): unmapped program memory write to 00003D = 0C & FF  TMCSR1
[:maincpu] ':maincpu' (FC2AFB): unmapped program memory write to 00003C = F0 & FF  TMCSR1
[:maincpu] ':maincpu' (FE2B89): unmapped program memory write to 000007 = 00 & FF  port 7 out
[:maincpu] ':maincpu' (FCE68D): unmapped program memory write to 000007 = 10 & FF  port 7 out
[:maincpu] ':maincpu' (FCE6BA): unmapped program memory write to 000034 = 73 & FF  PRL0 (PPG0 reload)
[:maincpu] ':maincpu' (FCE6D3): unmapped program memory write to 000036 = 0D & FF  PRL1 (PPG1 reload)
[:maincpu] ':maincpu' (FCE6DE): unmapped program memory write to 000030 = 85 & FF  PPG0C0 (PPG0 control)  (PPG Enabled, 16 MHz / 16, no interrupts)

************************************************************************/

#include "emu.h"
#include "screen.h"
#include "speaker.h"
#include "machine/timer.h"
#include "cpu/f2mc16/mb9061x.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

class tomy_princ_state : public driver_device
{
public:
	tomy_princ_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_cart(*this, "cartslot")
		, m_screen(*this, "screen")
		, m_maincpu(*this, "maincpu")
		, m_scantimer(*this, "scantimer")
	{ }

	void tomy_princ(machine_config &config);

protected:

private:
	required_device<generic_slot_device> m_cart;
	required_device<screen_device> m_screen;
	required_device<mb90611_device> m_maincpu;
	required_device<timer_device> m_scantimer;

	virtual void machine_reset() override;

	TIMER_DEVICE_CALLBACK_MEMBER(scan_interrupt);

	void princ_map(address_map &map);

	u8 read_gpu_status();

	uint32_t screen_update_tomy_princ(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	bool bFirstPort8Read;
};

TIMER_DEVICE_CALLBACK_MEMBER(tomy_princ_state::scan_interrupt)
{
	for (int i = 0; i < 128; i++)
	{
		m_maincpu->tin0_w(ASSERT_LINE);
		m_maincpu->tin0_w(CLEAR_LINE);
	}

	m_maincpu->tin1_w(ASSERT_LINE);
	m_maincpu->tin1_w(CLEAR_LINE);
}

void tomy_princ_state::machine_reset()
{
	bFirstPort8Read = true;
}

u8 tomy_princ_state::read_gpu_status()
{
	if (bFirstPort8Read)
	{
		bFirstPort8Read = false;
		return 0x20;
	}

	return 0x00;
}

uint32_t tomy_princ_state::screen_update_tomy_princ(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

// fe2d25
void tomy_princ_state::princ_map(address_map &map)
{
	map(0x000001, 0x000001).lw8([] (u8 data) { }, "free2");
	map(0x000008, 0x000008).r(FUNC(tomy_princ_state::read_gpu_status));
	map(0x68ff00, 0x68ff00).lw8([this] (u8 data) { bFirstPort8Read = true; }, "free1");
	map(0x68ff44, 0x68ff44).lr8([this] () -> u8 { return m_screen->vblank() ? 0x11 : 0x10; }, "free0");
	map(0xe00000, 0xe07fff).ram();  // stacks are placed here
	map(0xf00000, 0xffffff).rom().region("maincpu", 0x00000);
}

static INPUT_PORTS_START( tomy_princ )
INPUT_PORTS_END

void tomy_princ_state::tomy_princ(machine_config &config)
{
	// MB90611A microcontroller, F2MC-16L architecture
	MB90611A(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tomy_princ_state::princ_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_screen_update(FUNC(tomy_princ_state::screen_update_tomy_princ));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-1);

	TIMER(config, m_scantimer, 0);
	m_scantimer->configure_scanline(FUNC(tomy_princ_state::scan_interrupt), "screen", 0, 1);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "princ_cart");
	SOFTWARE_LIST(config, "cart_list").set_original("princ");
}

ROM_START( princ )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD("29f800t.u4", 0x00000, 0x100000, CRC(30b6b864) SHA1(7ada3af85dd8dd3f95ca8965ad8e642c26445293))
ROM_END

COMP( 1996?, princ,    0,       0,      tomy_princ,    tomy_princ, tomy_princ_state, empty_init, "Tomy", "Prin-C", MACHINE_IS_SKELETON )
