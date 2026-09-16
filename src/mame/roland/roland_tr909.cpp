// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Roland TR-909 drum machine.

***************************************************************************/

#include "emu.h"
#include "bus/generic/slot.h"
#include "cpu/upd7810/upd7810.h"
#include "machine/nvram.h"


namespace {

class roland_tr909_state : public driver_device
{
public:
	roland_tr909_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cartslot(*this, "cartslot")
		, m_inputs(*this, "IN%u", 1U)
		, m_porta(0)
		, m_portb(0)
	{
	}

	void tr909(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void dac1_w(u8 data);
	void dac2_w(u8 data);
	void dac3_w(u8 data);
	void dac4_w(u8 data);
	void dac5_w(u8 data);
	void dac6_w(u8 data);
	void dac7_w(u8 data);
	void dac8_w(u8 data);
	void trig1_w(u8 data);
	void trig2_w(u8 data);

	void pa_w(u8 data);
	void pb_w(u8 data);
	u8 switches_r();
	u8 cart_sense_r();

	void tr909_mem(address_map &map) ATTR_COLD;

	required_device<generic_cartslot_device> m_cartslot;
	required_ioport_array<5> m_inputs;

	u8 m_porta;
	u8 m_portb;
};


void roland_tr909_state::machine_start()
{
	save_item(NAME(m_porta));
	save_item(NAME(m_portb));
}

void roland_tr909_state::machine_reset()
{
	dac1_w(0);
	dac2_w(0);
	dac3_w(0);
	dac4_w(0);
	dac5_w(0);
	dac6_w(0);
	dac7_w(0);
	dac8_w(0);
	trig1_w(0);
	trig2_w(0);
}


void roland_tr909_state::dac1_w(u8 data)
{
	logerror("%s: DAC 1 = %02X\n", machine().describe_context(), data & 0x7e);
}

void roland_tr909_state::dac2_w(u8 data)
{
	logerror("%s: DAC 2 = %02X\n", machine().describe_context(), data & 0x7e);
}

void roland_tr909_state::dac3_w(u8 data)
{
	logerror("%s: DAC 3 = %02X\n", machine().describe_context(), data & 0x7e);
}

void roland_tr909_state::dac4_w(u8 data)
{
	logerror("%s: DAC 4 = %02X\n", machine().describe_context(), data & 0x7e);
}

void roland_tr909_state::dac5_w(u8 data)
{
	logerror("%s: DAC 5 = %02X\n", machine().describe_context(), data & 0x7e);
}

void roland_tr909_state::dac6_w(u8 data)
{
	logerror("%s: DAC 6 = %02X\n", machine().describe_context(), data & 0x7e);
}

void roland_tr909_state::dac7_w(u8 data)
{
	logerror("%s: DAC 7 = %02X\n", machine().describe_context(), data & 0x7e);
}

void roland_tr909_state::dac8_w(u8 data)
{
	logerror("%s: DAC 8 = %02X\n", machine().describe_context(), data & 0x7e);
}

void roland_tr909_state::trig1_w(u8 data)
{
	// D6 is unused
	logerror("%s: Trigger 1 = %02X\n", machine().describe_context(), data & 0x3e);
}

void roland_tr909_state::trig2_w(u8 data)
{
	logerror("%s: Trigger 2 = %02X\n", machine().describe_context(), data & 0x7e);
}


void roland_tr909_state::pa_w(u8 data)
{
	// TODO: LED matrix and 4511 digits
	m_porta = data;
}

void roland_tr909_state::pb_w(u8 data)
{
	// TODO: LED matrix, 4511 LE and tape outputs
	m_portb = data;
}

u8 roland_tr909_state::switches_r()
{
	u8 data = 0xff;
	for (int i = 0; i < 5; i++)
		if (!BIT(m_porta, i))
			data &= m_inputs[i]->read();

	return data;
}

u8 roland_tr909_state::cart_sense_r()
{
	// TODO
	return 0;
}


void roland_tr909_state::tr909_mem(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("firmware", 0);
	map(0x2000, 0x3fff).ram().share("nvram");
	map(0x4000, 0x4000).mirror(0x1fff).r(FUNC(roland_tr909_state::switches_r));
	map(0x4001, 0x4001).mirror(0x1ff0).w(FUNC(roland_tr909_state::dac1_w));
	map(0x4002, 0x4002).mirror(0x1ff0).w(FUNC(roland_tr909_state::dac2_w));
	map(0x4003, 0x4003).mirror(0x1ff0).w(FUNC(roland_tr909_state::dac3_w));
	map(0x4004, 0x4004).mirror(0x1ff0).w(FUNC(roland_tr909_state::dac4_w));
	map(0x4005, 0x4005).mirror(0x1ff0).w(FUNC(roland_tr909_state::dac5_w));
	map(0x4006, 0x4006).mirror(0x1ff0).w(FUNC(roland_tr909_state::dac6_w));
	map(0x4007, 0x4007).mirror(0x1ff0).w(FUNC(roland_tr909_state::dac7_w));
	map(0x4008, 0x4008).mirror(0x1ff0).w(FUNC(roland_tr909_state::dac8_w));
	map(0x4009, 0x4009).mirror(0x1ff0).w(FUNC(roland_tr909_state::trig1_w));
	map(0x400a, 0x400a).mirror(0x1ff0).w(FUNC(roland_tr909_state::trig2_w));
	map(0x6000, 0x7fff).unmaprw(); // cartridge
}


static INPUT_PORTS_START(tr909)
	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SW1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SW2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SW3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SW4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SW5")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SW6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SW7")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SW8")

	PORT_START("IN2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SW9")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SW10")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SW11")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SW12")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SW13")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SW14")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SW15")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SW16")

	PORT_START("IN3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Shift")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Scale")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Inst Select")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Shuffle/Flam")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Last Step")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Stop/Cont")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Start")

	PORT_START("IN4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Clear")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Track 1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Track 2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Track 3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Track 4")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Pattern 1")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Pattern 2")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Pattern 3")

	PORT_START("IN5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Enter/Accent")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Tape Sync")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Cycle")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Available Meas")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Fwd")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Back")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Tempo")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Ext Inst")

	PORT_START("TEMPO")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN) // TODO: analog

	PORT_START("TOTAL")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN) // TODO: analog
INPUT_PORTS_END


static void ram_carts(device_slot_interface &device)
{
}

void roland_tr909_state::tr909(machine_config &config)
{
	upd7810_device &maincpu(UPD7810(config, "maincpu", 12_MHz_XTAL));
	maincpu.set_addrmap(AS_PROGRAM, &roland_tr909_state::tr909_mem);
	maincpu.pa_out_cb().set(FUNC(roland_tr909_state::pa_w));
	maincpu.pb_out_cb().set(FUNC(roland_tr909_state::pb_w));
	//maincpu.pc_in_cb().set(FUNC(roland_tr909_state::tr909_portc_r));
	//maincpu.pc_out_cb().set(FUNC(roland_tr909_state::tr909_portc_w));
	maincpu.an0_func().set_ioport("TEMPO");
	maincpu.an1_func().set(FUNC(roland_tr909_state::cart_sense_r));
	maincpu.an2_func().set_constant(0);
	maincpu.an3_func().set_ioport("TOTAL");
	maincpu.an4_func().set_constant(0);
	maincpu.an5_func().set_constant(0);
	maincpu.an6_func().set_constant(0);
	maincpu.an7_func().set_constant(0);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // TC5565PL-15 + battery

	// no screen

	GENERIC_CARTSLOT(config, m_cartslot, ram_carts, "tr909_cart", "bin");

	//SOFTWARE_LIST(config, "cart_list", "tr909");
}


ROM_START(tr909)
	ROM_REGION(0x2000, "firmware", 0)
	//cpu is a D7811G 037 with 4096w of ROM
	//mode pins are both pulled to 5v

	//version 0 and 1 firmware also exist
	ROM_SYSTEM_BIOS(0, "v4", "v4.0")
	ROMX_LOAD("tr909_v4.ic609", 0, 0x2000, CRC(a9b45f30) SHA1(b94374e2cd6b5a6c405da4fa7793c98c0a6015c4), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v1", "v1.0")
	ROMX_LOAD("tr909_v1.ic609", 0, 0x2000, CRC(bfb9682b) SHA1(bcd754900574d1861034af8056574ffa77db3067), ROM_BIOS(1))

	//samples
	ROM_REGION(0x8000, "hihat", 0)
	ROM_LOAD("hn61256p__c43.ic69", 0, 0x8000, CRC(2aaae11b) SHA1(22a34d603e78673bb096f5e56a8971afe14d8fee))

	ROM_REGION(0x8000, "crash", 0)
	ROM_LOAD("hn61256p__c42.ic62", 0, 0x8000, CRC(dc61cf1a) SHA1(bc730107b0c391f4879c9777a3e85f3e57f3aad6))

	ROM_REGION(0x8000, "ride", 0)
	ROM_LOAD("hn61256p__c44.ic54", 0, 0x8000, CRC(01a9b435) SHA1(daa54c58c7e3ae3398f125568537ec82d5bd1dfd))
ROM_END

} // anonymous namespace


SYST(1984, tr909, 0, 0, tr909, tr909, roland_tr909_state, empty_init, "Roland", "TR-909 Rhythm Composer", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
