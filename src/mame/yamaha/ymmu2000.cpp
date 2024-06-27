// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*************************************************************************************

    Yamaha MU-500/1000/2000:
        32/64-part, 64/128-note polyphonic/multitimbral General MIDI/XG tone module

    Driver by O. Galibert

    MU2000: SH7043, dual SWP30, sample ram, M37640 for usb, smartcard port
    MU1000: sample ram and smartcard port removed
    MU500:  one SWP30 removed

**************************************************************************************/

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "bus/plg1x0/plg1x0.h"
#include "cpu/sh/sh7042.h"
#include "machine/input_merger.h"
#include "machine/nvram.h"
#include "machine/sci4.h"
#include "sound/swp30.h"

#include "mulcd.h"
#include "speaker.h"

#include "mu128.lh"
#include "mu2000.lh"


namespace {

static INPUT_PORTS_START( mu500 )
	PORT_START("SWS0")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Strings")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Bass")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Guitar")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Organ")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Chrom. Perc.")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Piano")

	PORT_START("SWS1")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Synth pad")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Synth lead")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Pipe")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Reed")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Brass")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Ensemble")

	PORT_START("SWS2")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Drum")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Model excl.")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("SFX")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Percussive")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Ethnic")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Synth effects")

	PORT_START("SWS3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part +")    PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part -")    PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mute/Solo") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Effect")    PORT_CODE(KEYCODE_F)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Util")      PORT_CODE(KEYCODE_U)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Edit")      PORT_CODE(KEYCODE_E)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Play")      PORT_CODE(KEYCODE_A)

	PORT_START("SWS4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value +")   PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value -")   PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Exit")      PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select >")  PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select <")  PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Enter")     PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Seq")       PORT_CODE(KEYCODE_Q)

	PORT_START("SWS5")
	PORT_BIT(0x1f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Audition")  PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select")    PORT_CODE(KEYCODE_X)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Sampling/Mode") PORT_CODE(KEYCODE_M)
INPUT_PORTS_END

class mu500_state : public driver_device
{
public:
	mu500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "ram")
		, m_swp30m(*this, "swp30m")
		, m_sci(*this, "sci")
		, m_lcd(*this, "lcd")
		, m_ram(*this, "ram")
		, m_ext(*this, "plg%u", 1U)
		, m_ioports(*this, "SWS%u", 0U)

	{ }

	void mu500(machine_config &config);

protected:
	required_device<sh7043a_device> m_maincpu;
	required_device<nvram_device> m_nvram;
	required_device<swp30_device> m_swp30m;
	required_device<sci4_device> m_sci;
	required_device<mulcd_device> m_lcd;
	required_shared_ptr<u32> m_ram;
	required_device_array<plg1x0_connector, 3> m_ext;
	required_ioport_array<6> m_ioports;

	u16 m_pe;
	u8 m_ledsw1, m_ledsw2;

	void map_500(address_map &map);
	void swp30_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	u16 adc_ar_r();
	u16 adc_al_r();
	u16 adc_midisw_r();
	u16 adc_battery_r();

	u16 pe_r();
	void pe_w(u16 data);
	u16 pa_r();

	void update_leds();
	u8 ledsw_r();
	void ledsw1_w(u8 data);
	void ledsw2_w(u8 data);
};

class mu1000_state : public mu500_state
{
public:
	mu1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: mu500_state(mconfig, type, tag)
		, m_swp30s(*this, "swp30s")

	{ }

	void mu1000(machine_config &config);

protected:
	required_device<swp30_device> m_swp30s;
	void map_1000(address_map &map);
};

class mu2000_state : public mu1000_state
{
public:
	mu2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: mu1000_state(mconfig, type, tag)
	{ }

	void mu2000(machine_config &config);

protected:
	void map_2000(address_map &map);
};



void mu500_state::machine_start()
{
	save_item(NAME(m_pe));
	save_item(NAME(m_ledsw1));
	save_item(NAME(m_ledsw2));
	m_pe = 0;
	m_ledsw1 = 0;
	m_ledsw2 = 0;
}

void mu500_state::machine_reset()
{
}

u16 mu500_state::pe_r()
{
	if(BIT(m_pe, 4)) {
		if(BIT(m_pe, 0)) {
			if(BIT(m_pe, 2))
				return m_lcd->data_read() << 8;
			else
				return m_lcd->control_read() << 8;
		} else
			return 0x0000;
	}

	return 0;
}

void mu500_state::pe_w(u16 data)
{
	if(BIT(m_pe, 4) && !BIT(data, 4)) {
		if(!BIT(data, 0)) {
			if(BIT(data, 2))
				m_lcd->data_write(data >> 8);
			else
				m_lcd->control_write(data >> 8);
		}
	}

	m_pe = data;
}

u16 mu500_state::pa_r()
{
	// 21: out, selects between front and back midi a to go on rxd0
	// 20: smvprt
	// 19: smvins
	// 18: smbusy
	// 17: rea (rotary encoder)
	// 16: reb
	return 0xffff;
}

void mu500_state::update_leds()
{
	m_lcd->set_leds(util::bitswap((m_ledsw2 << 8) | m_ledsw1, 9, 8, 7, 6, 10, 11, 12, 13, 14, 15));
}

u8 mu500_state::ledsw_r()
{
	u8 res = 0xff;
	for(u32 i=0; i != 6; i++)
		if(BIT(m_ledsw1, i))
			res &= m_ioports[i]->read();
	return res;
}

void mu500_state::ledsw1_w(u8 data)
{
	m_ledsw1 = data;
	update_leds();
}

void mu500_state::ledsw2_w(u8 data)
{
	m_ledsw2 = data;
	update_leds();
}

// Analog input right (also sent to the swp30m dac0)
u16 mu500_state::adc_ar_r()
{
	return 0;
}

// Analog input left (also sent to the swp30m dac0)
u16 mu500_state::adc_al_r()
{
	return 0;
}

// Put the host switch to pure midi
u16 mu500_state::adc_midisw_r()
{
	return 0;
}

// Battery level
u16 mu500_state::adc_battery_r()
{
	return 0x3ff;
}

void mu500_state::map_500(address_map &map)
{
	// 000000-3fffff: cs0 space, 32 bits, 2 wait states
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);

	// 400000-7fffff: cs1 space, 16 bits, 2 wait states
	map(0x400000, 0x43ffff).ram().share(m_ram);

	// 800000-bfffff: cs2 space, 16 bits, cs assert extension, 3 wait states
	map(0x800000, 0x801fff).m(m_swp30m, FUNC(swp30_device::map));
	// 802000 : slave swp30

	// c00000-ffffff: cs3 space, 8 bits, cs assert extension, 2 wait states, 3 idle states

	// c00000 : smartcard
	map(0xc80000, 0xc80000).rw(FUNC(mu500_state::ledsw_r), FUNC(mu500_state::ledsw1_w));
	// d00000 : smartcard
	// d80000 : contrast, levels
	map(0xe00000, 0xe00000).w(FUNC(mu500_state::ledsw2_w));
	// e80000 : dit2 (digital output)
	map(0xf00000, 0xf0003f).m(m_sci, FUNC(sci4_device::map));

	// f80000 : usb

	// Dedicated dram space, ras precharge = 2.5, ras-cas delay 2, cas-before-ras 2.5, dram write 3, read 3, idle 0, burst, ras down, 16bits, 9-bit address
	// Automatic refresh every 420 cycles, cas-before-ras
	map(0x01000000, 0x0107ffff).ram(); // dram
}

void mu1000_state::map_1000(address_map &map)
{
	map_500(map);
	map(0x802000, 0x803fff).m(m_swp30s, FUNC(swp30_device::map));
}

void mu2000_state::map_2000(address_map &map)
{
	map_1000(map);
	map(0x000000, 0x3fffff).rom().region("maincpu", 0);
}

void mu500_state::swp30_map(address_map &map)
{
	map(0x000000, 0x7fffff).rom().region("swp30", 0);
}

void mu500_state::mu500(machine_config &config)
{
	SH7043A(config, m_maincpu, 7_MHz_XTAL * 4); // md=9, no on-chip rom, 32-bit space, pll 4x
	m_maincpu->set_addrmap(AS_PROGRAM, &mu500_state::map_500);
	m_maincpu->read_porta().set(FUNC(mu500_state::pa_r));
	m_maincpu->read_porte().set(FUNC(mu500_state::pe_r));
	m_maincpu->write_porte().set(FUNC(mu500_state::pe_w));
	m_maincpu->read_adc<0>().set(FUNC(mu500_state::adc_ar_r));
	m_maincpu->read_adc<1>().set_constant(0);
	m_maincpu->read_adc<2>().set(FUNC(mu500_state::adc_al_r));
	m_maincpu->read_adc<3>().set_constant(0);
	m_maincpu->read_adc<4>().set(FUNC(mu500_state::adc_midisw_r));
	m_maincpu->read_adc<5>().set_constant(0);
	m_maincpu->read_adc<6>().set(FUNC(mu500_state::adc_battery_r));
	m_maincpu->read_adc<7>().set_constant(0);

	INPUT_MERGER_ANY_HIGH(config, "sciirq").output_handler().set_inputline(m_maincpu, 0);

	SCI4(config, m_sci);
	m_sci->write_irq<0>().set("sciirq", FUNC(input_merger_device::in_w<0>));
	m_sci->write_irq<1>().set("sciirq", FUNC(input_merger_device::in_w<1>));
	m_sci->write_irq<3>().set_inputline(m_maincpu, 1);

	NVRAM(config, m_nvram, nvram_device::DEFAULT_NONE);

	MULCD(config, m_lcd);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SWP30(config, m_swp30m);
	m_swp30m->set_addrmap(AS_DATA, &mu500_state::swp30_map);
	m_swp30m->add_route(0, "lspeaker", 1.0);
	m_swp30m->add_route(1, "rspeaker", 1.0);

	auto &mdin_a(MIDI_PORT(config, "mdin_a"));
	midiin_slot(mdin_a);
	mdin_a.rxd_handler().set(m_maincpu, FUNC(sh7043_device::sci_rx_w<0>));

	auto &mdin_b(MIDI_PORT(config, "mdin_b"));
	midiin_slot(mdin_b);
	mdin_b.rxd_handler().set(m_maincpu, FUNC(sh7043_device::sci_rx_w<1>));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_maincpu->write_sci_tx<0>().set(mdout, FUNC(midi_port_device::write_txd));

	PLG1X0_CONNECTOR(config, m_ext[0], plg1x0_intf, nullptr);
	m_ext[0]->midi_tx().set(m_sci, FUNC(sci4_device::rx_w<30>));
	m_sci->write_tx<30>().set(m_ext[0], FUNC(plg1x0_connector::midi_rx));

	PLG1X0_CONNECTOR(config, m_ext[1], plg1x0_intf, nullptr);
	m_ext[1]->midi_tx().set(m_sci, FUNC(sci4_device::rx_w<31>));
	m_sci->write_tx<31>().set(m_ext[1], FUNC(plg1x0_connector::midi_rx));

	PLG1X0_CONNECTOR(config, m_ext[2], plg1x0_intf, nullptr);
	m_ext[2]->midi_tx().set(m_sci, FUNC(sci4_device::rx_w<32>));
	m_sci->write_tx<32>().set(m_ext[2], FUNC(plg1x0_connector::midi_rx));

	config.set_default_layout(layout_mu128);
}

void mu1000_state::mu1000(machine_config &config)
{
	mu500(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mu1000_state::map_1000);

	SWP30(config, m_swp30s);
	m_swp30s->set_addrmap(AS_DATA, &mu1000_state::swp30_map);
	m_swp30s->add_route(0, "lspeaker", 1.0);
	m_swp30s->add_route(1, "rspeaker", 1.0);
}

void mu2000_state::mu2000(machine_config &config)
{
	mu1000(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &mu2000_state::map_2000);

	config.set_default_layout(layout_mu2000);
}


#define ROM_LOAD32_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
	ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2) | ROM_BIOS(bios))

ROM_START( mu500 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASE00 )
	// Soon (tm)

	ROM_REGION32_LE( 0x2000000, "swp30", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD( "xv364a0.ic49", 0x0000000, 0x800000, CRC(cda1afd6) SHA1(e7098246b33c3cf22ed8cc15ed6383f8a06d17e9) )
	ROM_LOAD32_WORD( "xv365a0.ic50", 0x0000002, 0x800000, CRC(10985ed0) SHA1(d45a2e85859e05046f3ede8317a9bb0b88898116) )
	ROM_LOAD32_WORD( "xw848a0.ic53", 0x1000000, 0x800000, CRC(34913e42) SHA1(9e8b55c2cbac3f69cc0b17aeaf02053145bfaeda) )
	ROM_LOAD32_WORD( "xw849a0.ic54", 0x1000002, 0x800000, CRC(3728f1f2) SHA1(7670d672e24d6388fa92799175f35869a140c451) )
ROM_END

ROM_START( mu1000 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("v201")
	ROM_SYSTEM_BIOS( 0, "v201", "Upgrade package (Ver2.01 02-MAY-29)" )
	ROM_LOAD32_WORD_SWAP_BIOS( 0, "mu1000-v2.01-h.bin", 0x000000, 0x100000, CRC(d0809297) SHA1(cee47062966b01ce72e8ebaf6f7fa9778b32f6ab) )
	ROM_LOAD32_WORD_SWAP_BIOS( 0, "mu1000-v2.01-l.bin", 0x000002, 0x100000, CRC(048a2750) SHA1(19f51c6304e3550d0bb8b3cca647f1fc609b0994) )
	// Version < 2.0 is very hard to find, most mu1000 have been upgraded to 2.x (aka EX)

	ROM_REGION32_LE( 0x2000000, "swp30", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD( "xv364a0.ic49", 0x0000000, 0x800000, CRC(cda1afd6) SHA1(e7098246b33c3cf22ed8cc15ed6383f8a06d17e9) )
	ROM_LOAD32_WORD( "xv365a0.ic50", 0x0000002, 0x800000, CRC(10985ed0) SHA1(d45a2e85859e05046f3ede8317a9bb0b88898116) )
	ROM_LOAD32_WORD( "xw848a0.ic53", 0x1000000, 0x800000, CRC(34913e42) SHA1(9e8b55c2cbac3f69cc0b17aeaf02053145bfaeda) )
	ROM_LOAD32_WORD( "xw849a0.ic54", 0x1000002, 0x800000, CRC(3728f1f2) SHA1(7670d672e24d6388fa92799175f35869a140c451) )
ROM_END

ROM_START( mu2000 )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("v200")
	ROM_SYSTEM_BIOS( 0, "v200", "Upgrade package (Ver2.01 02-MAY-29)" )
	ROM_LOAD32_WORD_SWAP_BIOS( 0, "mu2000-v2.01-h.bin", 0x000000, 0x200000, CRC(be3668f6) SHA1(00d008b6a2536a71681ce2f4fd1a5853406f82f2) )
	ROM_LOAD32_WORD_SWAP_BIOS( 0, "mu2000-v2.01-l.bin", 0x000002, 0x200000, CRC(55921a50) SHA1(fd8fe6a5cbba028d847453c004cb2dcf9ba02013) )
	ROM_SYSTEM_BIOS( 1, "v101", "20 (v1.01, 99-NOV-16)" )
	ROM_LOAD32_WORD_SWAP_BIOS( 1, "xw87020.ic25", 0x000000, 0x200000, CRC(79f6c158) SHA1(2213b9c661c6b1a79963321c37aff40be7cc1fff) )
	ROM_LOAD32_WORD_SWAP_BIOS( 1, "xw86920.ic24", 0x000002, 0x200000, CRC(afbac33c) SHA1(594b4c64aecf6a5204b058375e39e44b8fe373be) )

	ROM_REGION32_LE( 0x2000000, "swp30", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD( "xv364a0.ic49", 0x0000000, 0x800000, CRC(cda1afd6) SHA1(e7098246b33c3cf22ed8cc15ed6383f8a06d17e9) )
	ROM_LOAD32_WORD( "xv365a0.ic50", 0x0000002, 0x800000, CRC(10985ed0) SHA1(d45a2e85859e05046f3ede8317a9bb0b88898116) )
	ROM_LOAD32_WORD( "xw848a0.ic53", 0x1000000, 0x800000, CRC(34913e42) SHA1(9e8b55c2cbac3f69cc0b17aeaf02053145bfaeda) )
	ROM_LOAD32_WORD( "xw849a0.ic54", 0x1000002, 0x800000, CRC(3728f1f2) SHA1(7670d672e24d6388fa92799175f35869a140c451) )
ROM_END

} // anonymous namespace


CONS( 2000, mu500,  0,     0, mu500,  mu500, mu500_state,  empty_init, "Yamaha", "MU500",  MACHINE_NOT_WORKING )
CONS( 1999, mu1000, mu500, 0, mu1000, mu500, mu1000_state, empty_init, "Yamaha", "MU1000", MACHINE_NOT_WORKING )
CONS( 1999, mu2000, mu500, 0, mu2000, mu500, mu2000_state, empty_init, "Yamaha", "MU2000", MACHINE_NOT_WORKING )
