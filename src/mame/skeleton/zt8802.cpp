// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

Ziatech ZT-8802 single-board computer. Advert says: V40 CPU @ 8MHz, 1MB DRAM, 512K EPROM, 3 RS-232 PORTS.

CPU is NEC D70208L-10 (V40) which is a V20 with 8251,8253,8259 included.

Chips: Maxim MAX249CQH, Exar XT16C452CJPS, Ziatech ICPSMCI-16C49A, 2x GAL22V10D, 2x ACT15245, 2x ACT11245,
       ACT11240, PALCE 16V??-15JC, 2x CYM1465LPD-120C (RAM).

Other: A 3.6volt battery, a tiny crystal, a red LED, and about 2 dozen jumpers.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/nec/v5x.h"
#include "machine/ds1302.h"


namespace {

class zt8802_state : public driver_device
{
public:
	zt8802_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pia(*this, "pia")
		, m_rtc(*this, "rtc")
	{ }

	void zt8802(machine_config &config);

private:
	void pia_w(offs_t offset, u8 data);
	u8 rtc_r();
	void rtc_w(u8 data);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_pia;
	required_device<ds1202_device> m_rtc;
};

void zt8802_state::pia_w(offs_t offset, u8 data)
{
	if (offset == 5)
		m_rtc->ce_w(BIT(data, 4));

	m_pia[offset] = data;
}

u8 zt8802_state::rtc_r()
{
	if (!machine().side_effects_disabled())
		m_rtc->sclk_w(0);
	u8 data = m_rtc->io_r();
	if (!machine().side_effects_disabled())
		m_rtc->sclk_w(1);
	return data;
}

void zt8802_state::rtc_w(u8 data)
{
	m_rtc->sclk_w(0);
	m_rtc->io_w(BIT(data, 0));
	m_rtc->sclk_w(1);
}

void zt8802_state::mem_map(address_map &map)
{
	map(0x00000, 0x7ffff).ram();
	map(0x80000, 0xfffff).rom().region("roms", 0);
}

void zt8802_state::io_map(address_map &map)
{
	map(0x0080, 0x0080).nopr(); // watchdog?
	map(0xfa00, 0xfa07).ram().share("pia").w(FUNC(zt8802_state::pia_w)); //.rw("pia", FUNC(zt16c49_device::read), FUNC(zt16c49_device::write));
	map(0xfa80, 0xfa80).rw(FUNC(zt8802_state::rtc_r), FUNC(zt8802_state::rtc_w));
}

static INPUT_PORTS_START( zt8802 )
INPUT_PORTS_END

void zt8802_state::zt8802(machine_config &config)
{
	V40(config, m_maincpu, 16'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &zt8802_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &zt8802_state::io_map);

	DS1202(config, m_rtc, 32'768);
}

ROM_START( zt8802 )
	ROM_REGION( 0x80000, "roms", 0 )
	ROM_LOAD( "c103207-218 a.rom", 0x00000, 0x80000, CRC(fc1c6e99) SHA1(cfbb2f0c9927bac5abc85c12d2b82f7da46cab03) )
ROM_END

} // anonymous namespace


COMP( 1994, zt8802, 0, 0, zt8802, zt8802, zt8802_state, empty_init, "Ziatech", "ZT-8802 SBC", MACHINE_IS_SKELETON )
