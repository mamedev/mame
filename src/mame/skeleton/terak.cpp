// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

Terak 8510A

2009-02-23 Skeleton driver.

Known chips: i8257 DMA, i8272a FDC
Floppies were 8 inch IBM format.

****************************************************************************/

#include "emu.h"

#include "bus/qbus/qbus.h"
#include "cpu/t11/t11.h"

#include "emupal.h"
#include "screen.h"


namespace {

class terak_state : public driver_device
{
public:
	terak_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_qbus(*this, "qbus")
		, m_ram(*this, "maincpu")
	{ }

	void terak(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	virtual void machine_reset() override ATTR_COLD;

	void reset_w(int state);

	required_device<lsi11_device> m_maincpu;
	required_device<qbus_device> m_qbus;
	required_region_ptr<uint16_t> m_ram;
};

void terak_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0000000, 0177777).lrw16(
		NAME([this](offs_t offset) { if (!machine().side_effects_disabled()) m_maincpu->pulse_input_line(t11_device::BUS_ERROR, attotime::zero); return 0; }),
		NAME([this](offs_t offset, u16 data) { m_maincpu->pulse_input_line(t11_device::BUS_ERROR, attotime::zero); }));
	map(0000000, 0017777).ram();
	map(0173000, 0173177).rom();
}

/* Input ports */
static INPUT_PORTS_START( terak )
INPUT_PORTS_END


static const z80_daisy_config daisy_chain[] =
{
	{ "qbus" },
	{ nullptr }
};

void terak_state::machine_reset()
{
	reset_w(ASSERT_LINE);
}

void terak_state::reset_w(int state)
{
	if (state == ASSERT_LINE)
	{
		m_qbus->init_w();
	}
}

void terak_state::terak(machine_config &config)
{
	/* basic machine hardware */
	LSI11(config, m_maincpu, 4'000'000);
	m_maincpu->set_initial_mode(6 << 13);
	m_maincpu->set_addrmap(AS_PROGRAM, &terak_state::mem_map);
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->out_reset().set(FUNC(terak_state::reset_w));

	QBUS(config, m_qbus, 0);
	m_qbus->set_space(m_maincpu, AS_PROGRAM);
	m_qbus->bus_error_callback().set([this] (int state) { m_maincpu->pulse_input_line(t11_device::BUS_ERROR, attotime::zero); });
	m_qbus->bevnt().set_inputline(m_maincpu, t11_device::CP2_LINE);
	m_qbus->birq4().set_inputline(m_maincpu, t11_device::VEC_LINE);
	QBUS_SLOT(config, "qbus" ":1", qbus_cards, nullptr);
	QBUS_SLOT(config, "qbus" ":2", qbus_cards, "terak_v");
	QBUS_SLOT(config, "qbus" ":3", qbus_cards, nullptr);
	QBUS_SLOT(config, "qbus" ":4", qbus_cards, nullptr);
}

/* ROM definition */
ROM_START( terak )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	// QX disk bootstrap
	ROM_LOAD( "terak.rom", 0173000, 0x0080, CRC(fd654b8e) SHA1(273a9933b68a290c5aedcd6d69faa7b1d22c0344))

	ROM_REGION( 0x2000, "kbd", 0)
	// keytronic keyboard, roms are unlabelled, type 6301-1J. CPU is 30293E-003. No crystal.
	ROM_LOAD( "82s129.z2", 0x0000, 0x0100, CRC(a5dce419) SHA1(819197a03eb9b6ea3318f5afc37c0b436dd747a7) )
	ROM_LOAD( "82s129.z1", 0x0100, 0x0100, CRC(f34e061f) SHA1(3cb354b2680056d4b3234c680958d4591279ac8a) )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME       FLAGS
COMP( 1977, terak, 0,      0,      terak,   terak, terak_state, empty_init, "Terak", "Terak 8510A", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
