// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Sean Riddle, David Viens
/*******************************************************************************

Bingo Bear (model 70501) / Monkgomery Monkey (model 70502), aka the Yakity Yaks,
they're talking hand puppets.

Initially released by Hasbro, although the 1986 Bingo Bear trademark was
registered by Tiger, and eventually in 1989 Tiger published it themselves.
Programming was apparently done by Micom Tech.

Hardware notes:
- PCB label: 7-130M-9R
- TMS1100 E7CL04N2L (die label: 1100G E7CL04)
- TMS5110ANL, 2*16KB VSM
- module slot

Sensor locations (just electronic switches):
- mouth: close/open it to make him talk
- body (left and right, connected to eachother): hug
- neck: scratch here when he asks for it

Monkgomery Monkey (trademarked by Hasbro) has the same MCU. The voice box label
is 7~150 instead of 7~130.

The external modules also came with a new outfit for the plushie to wear.
It looks like the Bingo Bear modules mostly work fine with Monkgomery Monkey,
though obviously his voice will change.

*******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/tms1000/tms1100.h"
#include "machine/tms6100.h"
#include "sound/tms5110.h"

#include "softlist_dev.h"
#include "speaker.h"


namespace {

class bingobear_state : public driver_device
{
public:
	bingobear_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tms5100(*this, "tms5100"),
		m_tms6100(*this, "tms6100"),
		m_cart(*this, "cartslot"),
		m_inputs(*this, "IN.0")
	{ }

	void bingobear(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_on);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices/pointers
	required_device<tms1100_cpu_device> m_maincpu;
	required_device<tms5110_device> m_tms5100;
	required_device<tms6100_device> m_tms6100;
	optional_device<generic_slot_device> m_cart;
	required_ioport m_inputs;

	bool m_power_on = false;
	u16 m_inp_mux = 0;
	u32 m_r = 0;

	void power_off();
	u8 read_k();
	void write_o(u16 data);
	void write_r(u32 data);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
};

void bingobear_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_power_on));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_r));
}



/*******************************************************************************
    Power
*******************************************************************************/

void bingobear_state::machine_reset()
{
	m_power_on = true;
	m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}

INPUT_CHANGED_MEMBER(bingobear_state::power_on)
{
	if (newval && !m_power_on)
		machine_reset();
}

void bingobear_state::power_off()
{
	m_power_on = false;
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}



/*******************************************************************************
    Cartridge
*******************************************************************************/

DEVICE_IMAGE_LOAD_MEMBER(bingobear_state::cart_load)
{
	u32 const size = m_cart->common_get_size("rom");

	if (size > 0x4000)
		return std::make_pair(image_error::INVALIDLENGTH, "Invalid image file size (must be no more than 16K)");

	u8 *const base = memregion("tms6100")->base() + 0x8000;
	m_cart->common_load_rom(base, size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}



/*******************************************************************************
    I/O
*******************************************************************************/

void bingobear_state::write_r(u32 data)
{
	// R1234: TMS5100 CTL8421
	u32 r = bitswap<5>(data,0,1,2,3,4) | (data & ~0x1f);
	m_tms5100->ctl_w(r & 0xf);

	// R0: TMS5100 PDC pin
	m_tms5100->pdc_w(data & 1);

	// R6-R8: input mux high
	m_inp_mux = (m_inp_mux & 1) | (data >> 5 & 0xe);

	// R9: power-off request, on falling edge
	if (~data & m_r & 0x200)
		power_off();

	m_r = r;
}

void bingobear_state::write_o(u16 data)
{
	// O0: input mux low
	m_inp_mux = (m_inp_mux & ~1) | (data & 1);
}

u8 bingobear_state::read_k()
{
	// K: sensors
	u8 data = m_inp_mux & m_inputs->read();

	// and TMS5100 CTL (also tied to R1234)
	return data | m_tms5100->ctl_r() | (m_r & 0xf);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( bingobear )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("Neck Sensor")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // test pad
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_CHANGED_MEMBER(DEVICE_SELF, bingobear_state, power_on, 0) PORT_NAME("Mouth Sensor")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_CHANGED_MEMBER(DEVICE_SELF, bingobear_state, power_on, 0) PORT_NAME("Body Sensor")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void bingobear_state::bingobear(machine_config &config)
{
	constexpr u32 MASTER_CLOCK = 640'000; // approximation

	// basic machine hardware
	TMS1100(config, m_maincpu, MASTER_CLOCK/2);
	m_maincpu->read_k().set(FUNC(bingobear_state::read_k));
	m_maincpu->write_o().set(FUNC(bingobear_state::write_o));
	m_maincpu->write_r().set(FUNC(bingobear_state::write_r));

	// sound hardware
	TMS6100(config, m_tms6100, MASTER_CLOCK/4);

	SPEAKER(config, "mono").front_center();
	TMS5110A(config, m_tms5100, MASTER_CLOCK);
	m_tms5100->m0().set(m_tms6100, FUNC(tms6100_device::m0_w));
	m_tms5100->m1().set(m_tms6100, FUNC(tms6100_device::m1_w));
	m_tms5100->addr().set(m_tms6100, FUNC(tms6100_device::add_w));
	m_tms5100->data().set(m_tms6100, FUNC(tms6100_device::data_line_r));
	m_tms5100->romclk().set(m_tms6100, FUNC(tms6100_device::clk_w));
	m_tms5100->add_route(ALL_OUTPUTS, "mono", 0.5);

	// cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "bingobear", "vsm,bin");
	m_cart->set_device_load(FUNC(bingobear_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("bingobear");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( bbear )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "e7cl04n2l", 0x0000, 0x0800, CRC(e2e4ff07) SHA1(8a944ade69b71667b18ce024c1105c522128ea75) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_bbear_output.pla", 0, 365, CRC(5a0b8e88) SHA1(e96cffb2ac3bf81f335d6f95125637446ff0f1b7) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cm62056", 0x0000, 0x4000, CRC(550b40d1) SHA1(ead5ed19a875309ec0be07adcb2986fead3405e0) ) // CM62091 label also seen, same ROM contents
	ROM_LOAD( "cm62057", 0x4000, 0x4000, CRC(383a8a9a) SHA1(3a4b8837febbec9fe9603970cf520f69304c5a68) ) // CM62092 "
ROM_END

ROM_START( monkmonk )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "e7cl04n2l", 0x0000, 0x0800, CRC(e2e4ff07) SHA1(8a944ade69b71667b18ce024c1105c522128ea75) )

	ROM_REGION( 867, "maincpu:mpla", 0 )
	ROM_LOAD( "tms1100_common1_micro.pla", 0, 867, CRC(62445fc9) SHA1(d6297f2a4bc7a870b76cc498d19dbb0ce7d69fec) )
	ROM_REGION( 365, "maincpu:opla", 0 )
	ROM_LOAD( "tms1100_bbear_output.pla", 0, 365, CRC(5a0b8e88) SHA1(e96cffb2ac3bf81f335d6f95125637446ff0f1b7) )

	ROM_REGION( 0x10000, "tms6100", ROMREGION_ERASEFF ) // 8000-bfff = space reserved for cartridge
	ROM_LOAD( "cm62059", 0x0000, 0x4000, CRC(fa00a337) SHA1(b8625cee6605a9083de5da9e6ef3ea1c5d36e15c) )
	ROM_LOAD( "cm62060", 0x4000, 0x4000, CRC(494e06c2) SHA1(679517f1304617f7db1cd89ea2260e969170c6ac) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY, FULLNAME, FLAGS
SYST( 1986, bbear,    0,      0,      bingobear, bingobear, bingobear_state, empty_init, "Hasbro / Tiger Electronics", "Bingo Bear", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_CONTROLS | MACHINE_IMPERFECT_SOUND )
SYST( 1986, monkmonk, 0,      0,      bingobear, bingobear, bingobear_state, empty_init, "Hasbro / Tiger Electronics", "Monkgomery Monkey", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_CONTROLS | MACHINE_IMPERFECT_SOUND )
