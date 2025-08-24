// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

HAL GSX-8800

TODO:
- Untested (failed attempt for rcmpc88v which seems to access $a0-$a1 for a "direct" MIDI);
- multi-card support, can move $a0-$a3 into $a4-$a7 thru a jumper for dual GSX;
- MIDI-IN/OUT connector, 8251 serial and 8253;
- RHYTHM-BOX connector (?)

**************************************************************************************************/


#include "emu.h"
#include "gsx8800.h"


DEFINE_DEVICE_TYPE(GSX8800, gsx8800_device, "gsx8800", "HAL GSX-8800")

gsx8800_device::gsx8800_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc8801_exp_device(mconfig, GSX8800, tag, owner, clock)
	, m_psg(*this, "psg.%u", 0U)
{
}

void gsx8800_device::io_map(address_map &map)
{
	map(0xa1, 0xa1).r(m_psg[0], FUNC(ym2149_device::data_r));
	map(0xa0, 0xa1).w(m_psg[0], FUNC(ym2149_device::address_data_w));
	map(0xa3, 0xa3).r(m_psg[1], FUNC(ym2149_device::data_r));
	map(0xa2, 0xa3).w(m_psg[1], FUNC(ym2149_device::address_data_w));
//  map(0xc2, 0xc3) 8251 to MIDI
	// TODO: PIT location is unknown
}

void gsx8800_device::device_add_mconfig(machine_config &config)
{
	// TODO: should be 3'993'600, divided by 32 (?)
	constexpr XTAL psg_x1_clock = XTAL(4'000'000);

	// TODO: mixing routing may be swapped
	// it's just known that one goes to the left and the other to the right
	// cfr. http://mydocuments.g2.xrea.com/html/p8/soundinfo.html
	YM2149(config, m_psg[0], psg_x1_clock);
	m_psg[0]->add_route(ALL_OUTPUTS, "^^speaker", 0.50, 0);

	YM2149(config, m_psg[1], psg_x1_clock);
	m_psg[1]->add_route(ALL_OUTPUTS, "^^speaker", 0.50, 1);


// ...->irq_handler().set(FUNC(gsx8800_device::int3_w));
}
