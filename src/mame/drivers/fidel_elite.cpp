// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

* fidel_elite.cpp, subdriver of machine/fidelbase.cpp, machine/chessbase.cpp

Fidelity Elite A/S series hardware (EAS, EAG, PC)
see fidel_eag68k.cpp for 68000-based EAG hardware

*******************************************************************************

Elite A/S Challenger (EAS)
---------------------------------
This came out in 1982. 2 program updates were released in 1983 and 1984,
named Budapest and Glasgow, places where Fidelity won chess computer matches.
A/S stands for auto sensory, it's the 1st Fidelity board with magnet sensors.
The magnetic chessboard was licensed from AVE Micro Systems, in fact it's the
exact same one as in AVE's ARB (ave_arb.cpp driver).

hardware overview:
- 8*8 magnet sensors, 11 buttons, 8*(8+1) LEDs + 4*7seg LEDs
- R65C02P4 or R6502BP CPU, default frequency 3MHz*
- 4KB RAM (2*HM6116), 24KB ROM
- TSI S14001A + speech ROM
- I/O with 8255 PPI and bunch of TTL
- module slot and printer port
- PCB label 510-1071A01

*In West Germany, some distributors released it with overclocked CPUs,
advertised as 3.2, 3.6, or 4MHz. Unmodified EAS PCB photos show only a 3MHz XTAL.

A condensator keeps RAM contents alive for a few hours when powered off.

Prestige Challenger (PC) hardware is very similar. They stripped the 8255 PPI,
and added more RAM(7*TMM2016P). Some were released at 3.6MHz instead of 4MHz,
perhaps due to hardware instability? Opening module PC16 was included by default,
this module is the same as CB16 but at different form factor.

Elite Avant Garde (models 6081,6088,6089) is on the same hardware as EAS.

Fidelity Private Line is a modified EAS Glasgow. They took out the motherboard
and leds and placed them a little box separate from a (ledless) magnetic chessboard.
It was probably only released in Germany.

******************************************************************************/

#include "emu.h"
#include "includes/fidelbase.h"

#include "cpu/m6502/m65c02.h"
#include "cpu/m6502/r65c02.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "sound/volt_reg.h"
#include "speaker.h"

// internal artwork
#include "fidel_eag.lh" // clickable
#include "fidel_eas.lh" // clickable
#include "fidel_eas_priv.lh" // clickable
#include "fidel_pc.lh" // clickable


namespace {

class elite_state : public fidelbase_state
{
public:
	elite_state(const machine_config &mconfig, device_type type, const char *tag) :
		fidelbase_state(mconfig, type, tag),
		m_ppi8255(*this, "ppi8255")
	{ }

	// machine drivers
	void pc(machine_config &config);
	void eas(machine_config &config);
	void eas_priv(machine_config &config);
	void eag(machine_config &config);
	void eag2100(machine_config &config);

	void init_eag2100();

private:
	// devices/pointers
	optional_device<i8255_device> m_ppi8255;

	// address maps
	void eas_map(address_map &map);
	void eag_map(address_map &map);
	void eag2100_map(address_map &map);
	void pc_map(address_map &map);

	// I/O handlers
	void update_display();
	DECLARE_READ8_MEMBER(speech_r);
	DECLARE_WRITE8_MEMBER(segment_w);
	DECLARE_WRITE8_MEMBER(led_w);
	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE8_MEMBER(ppi_porta_w);
	DECLARE_READ8_MEMBER(ppi_portb_r);
	DECLARE_WRITE8_MEMBER(ppi_portc_w);
};

void elite_state::init_eag2100()
{
	m_rombank->configure_entries(0, 4, memregion("rombank")->base(), 0x2000);
}


/******************************************************************************
    Devices, I/O
******************************************************************************/

// TTL/generic

void elite_state::update_display()
{
	// 4/8 7seg leds+H, 8*8(+1) chessboard leds
	set_display_segmask(0x1ef, 0x7f);
	display_matrix(16, 9, m_led_data << 8 | m_7seg_data, m_led_select);
}

READ8_MEMBER(elite_state::speech_r)
{
	return m_speech_rom[m_speech_bank << 12 | offset];
}

WRITE8_MEMBER(elite_state::segment_w)
{
	// a0-a2,d7: digit segment
	m_7seg_data = (data & 0x80) >> offset;
	m_7seg_data = bitswap<8>(m_7seg_data,7,6,4,5,0,2,1,3);
	update_display();
}

WRITE8_MEMBER(elite_state::led_w)
{
	// a0-a2,d0: led data
	m_led_data = (data & 1) << offset;
	update_display();
}

READ8_MEMBER(elite_state::input_r)
{
	// multiplexed inputs (active low)
	return read_inputs(9) ^ 0xff;
}


// 8255 PPI (PC: done with TTL instead)

WRITE8_MEMBER(elite_state::ppi_porta_w)
{
	// d0-d5: TSI C0-C5
	// d6: TSI START line
	m_speech->data_w(space, 0, data & 0x3f);
	m_speech->start_w(data >> 6 & 1);

	// d7: printer? (black wire to LED pcb)
}

WRITE8_MEMBER(elite_state::ppi_portc_w)
{
	// d0-d3: 7442 a0-a3
	// 7442 0-8: led select, input mux
	m_led_select = 1 << (data & 0xf) & 0x3ff;
	m_inp_mux = m_led_select & 0x1ff;
	update_display();

	// 7442 9: speaker out
	m_dac->write(BIT(m_led_select, 9));

	// d4: speech ROM A12
	m_speech->force_update(); // update stream to now
	m_speech_bank = data >> 4 & 1;

	// d5: lower TSI volume
	m_speech->set_output_gain(0, (data & 0x20) ? 0.25 : 1.0);

	// d6,d7: bookrom bankswitch (model EAG)
	if (m_rombank != nullptr)
		m_rombank->set_entry(data >> 6 & 3);
}

READ8_MEMBER(elite_state::ppi_portb_r)
{
	// d0: printer? white wire from LED pcb
	u8 data = 1;

	// d1: TSI BUSY line
	data |= (m_speech->busy_r()) ? 2 : 0;

	// d2,d3: language switches(hardwired)
	data |= *m_language << 2 & 0x0c;

	// d5: multiplexed inputs highest bit
	data |= (read_inputs(9) & 0x100) ? 0 : 0x20;

	// other: ?
	return data | 0xd0;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void elite_state::eas_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).ram().share("nvram");
	map(0x2000, 0x5fff).r(FUNC(elite_state::cartridge_r));
	map(0x7000, 0x7003).rw(m_ppi8255, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x7020, 0x7027).w(FUNC(elite_state::segment_w)).nopr();
	map(0x7030, 0x7037).w(FUNC(elite_state::led_w)).nopr();
	map(0x7050, 0x7050).r(FUNC(elite_state::input_r));
	map(0x8000, 0x9fff).rom();
	map(0xc000, 0xffff).rom();
}

void elite_state::eag_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).ram().share("nvram.ic8");
	map(0x2000, 0x5fff).r(FUNC(elite_state::cartridge_r));
	map(0x7000, 0x7003).rw(m_ppi8255, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x7020, 0x7027).w(FUNC(elite_state::segment_w)).nopr();
	map(0x7030, 0x7037).w(FUNC(elite_state::led_w)).nopr();
	map(0x7050, 0x7050).r(FUNC(elite_state::input_r));
	map(0x8000, 0x9fff).ram().share("nvram.ic6");
	map(0xa000, 0xffff).rom();
}

void elite_state::eag2100_map(address_map &map)
{
	eag_map(map);
	map(0xa000, 0xbfff).bankr("rombank");
}

void elite_state::pc_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x17ff).ram();
	map(0x2000, 0x5fff).r(FUNC(elite_state::cartridge_r));
	map(0x7000, 0x7000).w(FUNC(elite_state::ppi_porta_w));
	map(0x7010, 0x7010).r(FUNC(elite_state::ppi_portb_r));
	map(0x7020, 0x7027).w(FUNC(elite_state::segment_w)).nopr();
	map(0x7030, 0x7037).w(FUNC(elite_state::led_w)).nopr();
	map(0x7040, 0x7040).w(FUNC(elite_state::ppi_portc_w));
	map(0x7050, 0x7050).r(FUNC(elite_state::input_r));
	map(0x8000, 0x9fff).ram();
	map(0xb000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( eas )
	PORT_INCLUDE( fidel_cpu_div_4 )
	PORT_INCLUDE( generic_cb_magnets )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("DM")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("CL")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("RV")

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Game Control") // labeled RESET on the Prestige, but led display still says - G C -
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("Speaker")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("PB / King")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("PV / Queen")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("TM / Rook")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("ST / Bishop")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("TB / Knight")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("LV / Pawn")
INPUT_PORTS_END

static INPUT_PORTS_START( eag )
	PORT_INCLUDE( fidel_cpu_div_4 )
	PORT_INCLUDE( generic_cb_magnets )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("CL")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("DM")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("RV")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Option")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("LV / Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("TB / Knight")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("ST / Bishop")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("TM / Rook")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("PV / Queen")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("PB / King")
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void elite_state::pc(machine_config &config)
{
	/* basic machine hardware */
	R65C02(config, m_maincpu, 4_MHz_XTAL); // R65C02P4
	m_maincpu->set_addrmap(AS_PROGRAM, &elite_state::div_trampoline);
	ADDRESS_MAP_BANK(config, m_mainmap).set_map(&elite_state::pc_map).set_options(ENDIANNESS_LITTLE, 8, 16);

	const attotime irq_period = attotime::from_hz(38.4_kHz_XTAL/64); // through 4060 IC, 600Hz
	TIMER(config, m_irq_on).configure_periodic(FUNC(elite_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_hz(38.4_kHz_XTAL*2)); // edge!
	TIMER(config, "irq_off").configure_periodic(FUNC(elite_state::irq_off<M6502_IRQ_LINE>), irq_period);

	TIMER(config, "display_decay").configure_periodic(FUNC(elite_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_fidel_pc);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	S14001A(config, m_speech, 25000); // R/C circuit, around 25khz
	m_speech->ext_read().set(FUNC(elite_state::speech_r));
	m_speech->add_route(ALL_OUTPUTS, "speaker", 0.75);

	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	/* cartridge */
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "fidel_scc", "bin,dat");
	m_cart->set_device_load(device_image_load_delegate(&elite_state::device_image_load_scc_cartridge, this));

	SOFTWARE_LIST(config, "cart_list").set_original("fidel_scc");
}

void elite_state::eas(machine_config &config)
{
	pc(config);

	/* basic machine hardware */
	m_maincpu->set_clock(3_MHz_XTAL);
	m_mainmap->set_addrmap(AS_PROGRAM, &elite_state::eas_map);

	I8255(config, m_ppi8255); // port B: input, port A & C: output
	m_ppi8255->out_pa_callback().set(FUNC(elite_state::ppi_porta_w));
	m_ppi8255->tri_pa_callback().set_constant(0);
	m_ppi8255->in_pb_callback().set(FUNC(elite_state::ppi_portb_r));
	m_ppi8255->out_pc_callback().set(FUNC(elite_state::ppi_portc_w));
	m_ppi8255->tri_pc_callback().set_constant(0);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	config.set_default_layout(layout_fidel_eas);
}

void elite_state::eas_priv(machine_config &config)
{
	eas(config);

	/* basic machine hardware */
	M65C02(config.replace(), m_maincpu, 3.579545_MHz_XTAL); // UM6502C
	m_maincpu->set_addrmap(AS_PROGRAM, &elite_state::div_trampoline);

	config.set_default_layout(layout_fidel_eas_priv);
}

void elite_state::eag(machine_config &config)
{
	eas(config);

	/* basic machine hardware */
	m_maincpu->set_clock(5_MHz_XTAL); // R65C02P4
	m_mainmap->set_addrmap(AS_PROGRAM, &elite_state::eag_map);

	config.device_remove("nvram");
	NVRAM(config, "nvram.ic8", nvram_device::DEFAULT_ALL_0);
	NVRAM(config, "nvram.ic6", nvram_device::DEFAULT_ALL_0);

	config.set_default_layout(layout_fidel_eag);
}

void elite_state::eag2100(machine_config &config)
{
	eag(config);

	/* basic machine hardware */
	m_mainmap->set_addrmap(AS_PROGRAM, &elite_state::eag2100_map);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( feasbu )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("eli_bu.6", 0x8000, 0x0800, CRC(93dcc23b) SHA1(2eb8c5a85e566948bc256d6b1804694e6b0ffa6f) ) // ST M27C64A, unknown label
	ROM_CONTINUE( 0x9000, 0x0800 )
	ROM_CONTINUE( 0x8800, 0x0800 )
	ROM_CONTINUE( 0x9800, 0x0800 )
	ROM_LOAD("101-1052a02.5", 0xc000, 0x2000, CRC(859d69f1) SHA1(a8b057683369e2387f22fc7e916b6f3c75d44b21) ) // Mostek MK36C63N-5
	ROM_LOAD("101-1052a01.4", 0xe000, 0x2000, CRC(571a33a7) SHA1(43b110cf0918caf16643178f401e58b2dc73894f) ) // Mostek MK36C63N-5

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) ) // NEC D2332C
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( feasgla )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("eli_gla.6", 0x8000, 0x0800, CRC(2fdddb4f) SHA1(6da0a328a45462f285ae6a0756f97c5a43148f97) )
	ROM_CONTINUE( 0x9000, 0x0800 )
	ROM_CONTINUE( 0x8800, 0x0800 )
	ROM_CONTINUE( 0x9800, 0x0800 )
	ROM_LOAD("eli_gla.5", 0xc000, 0x0800, CRC(f094e625) SHA1(fef84c6a3da504aac15988ec9af94417e5fedfbd) )
	ROM_CONTINUE( 0xd000, 0x0800 )
	ROM_CONTINUE( 0xc800, 0x0800 )
	ROM_CONTINUE( 0xd800, 0x0800 )
	ROM_LOAD("eli_gla.4", 0xe000, 0x0800, CRC(5f6845d1) SHA1(684eb16faf36a49560e5a73b55fd0022dc090e35) )
	ROM_CONTINUE( 0xf000, 0x0800 )
	ROM_CONTINUE( 0xe800, 0x0800 )
	ROM_CONTINUE( 0xf800, 0x0800 )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) ) // NEC D2332C
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( fepriv )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("4,0_8.6", 0x8000, 0x0800, CRC(32784e2d) SHA1(dae060a5c49cc1993a78db293cd80464adfd892d) )
	ROM_CONTINUE( 0x9000, 0x0800 )
	ROM_CONTINUE( 0x8800, 0x0800 )
	ROM_CONTINUE( 0x9800, 0x0800 )
	ROM_LOAD("c.5", 0xc000, 0x0800, CRC(ddb80412) SHA1(b1d9435d9a71b8eb241a2169bfbaa0499f510769) )
	ROM_CONTINUE( 0xd000, 0x0800 )
	ROM_CONTINUE( 0xc800, 0x0800 )
	ROM_CONTINUE( 0xd800, 0x0800 )
	ROM_LOAD("4,0_e.4", 0xe000, 0x0800, CRC(62a5305a) SHA1(a361bd9a54b903d7b0fbacabe55ea5ccbbc1dc51) )
	ROM_CONTINUE( 0xf000, 0x0800 )
	ROM_CONTINUE( 0xe800, 0x0800 )
	ROM_CONTINUE( 0xf800, 0x0800 )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END


ROM_START( fpres )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("u09_yellow", 0xb000, 0x1000, CRC(03fac294) SHA1(5a9d72978318c61185efd4bc9e4a868c226465b8) )
	ROM_LOAD("u10_green",  0xc000, 0x1000, CRC(5d049d5e) SHA1(c7359bead92729e8a92d6cf1789d87ae43d23cbf) )
	ROM_LOAD("u11_black",  0xd000, 0x1000, CRC(98bd01b7) SHA1(48cc560c4ca736f54e30d757990ff403c05c39ae) )
	ROM_LOAD("u12_blue",   0xe000, 0x1000, CRC(6f18115f) SHA1(a08b3a66bfdc23f3400e03fe253a8b9a4967d14f) )
	ROM_LOAD("u13_red",    0xf000, 0x1000, CRC(dea8091d) SHA1(1d94a90ae076215c2c009e78ec4919dbd8467ef8) )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( fpresbu )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("u09_yellow", 0xb000, 0x1000, CRC(bb1cb486) SHA1(b83f50a3ef361d254b88eefaa5aac657aaa72375) )
	ROM_LOAD("u10_green",  0xc000, 0x1000, CRC(af0aec0e) SHA1(8293d00a12efa1c142b9e37bc7786012250536d9) )
	ROM_LOAD("u11_black",  0xd000, 0x1000, CRC(214a91cc) SHA1(aab07ecdd66ac208874f4053fc4b0b0659b017aa) )
	ROM_LOAD("u12_blue",   0xe000, 0x1000, CRC(dae4d8e4) SHA1(f06dbb643f0324c0bddaaae9537d5829768bda22) )
	ROM_LOAD("u13_red",    0xf000, 0x1000, CRC(5fb67708) SHA1(1e9ee724c2be38daf39d5cf37b0ae587e408777c) )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END


ROM_START( feag ) // model 6081, aka "Mobile Master"
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("eg_orange.ic9", 0xa000, 0x2000, CRC(df9e7e74) SHA1(db76750eba5515213ecce07402c4d974c14e1a23) ) // M5L2764K, orange sticker
	ROM_LOAD("eg_black.ic5",  0xc000, 0x2000, CRC(a5f6f295) SHA1(319f00d4b7a1704a3ca722c40f4096004b4b89d2) ) // M5L2764K, black sticker
	ROM_LOAD("eg_green.ic4",  0xe000, 0x2000, CRC(1dc6508a) SHA1(6f2e730b216bfb900074d1d786124fc3cb038a8d) ) // M5L2764K, green sticker

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107.ic16", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(                 0x1000, 0x1000)
	ROMX_LOAD("101-64101.ic16", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105.ic16", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106.ic16", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( feag2100 )
	ROM_REGION( 0x10000, "mainmap", 0 )
	ROM_LOAD("el2100.2",  0xc000, 0x2000, CRC(76fec42f) SHA1(34660edb8458919fd179e93fdab3fe428a6625d0) )
	ROM_LOAD("el2100.3",  0xe000, 0x2000, CRC(2079a506) SHA1(a7bb83138c7b6eff6ea96702d453a214697f4890) )

	ROM_REGION( 0x8000, "rombank", 0 )
	ROM_LOAD("el2100.1",  0x0000, 0x8000, CRC(9b62b7d5) SHA1(cfcaea2e36c2d52fe4a85c77dbc7fa135893860c) )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 3, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107.ic16", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(                 0x1000, 0x1000)
	ROMX_LOAD("101-64101.ic16", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105.ic16", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106.ic16", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT CMP MACHINE   INPUT  STATE        INIT          COMPANY, FULLNAME, FLAGS
CONS( 1983, feasbu,   0,      0, eas,      eas,   elite_state, empty_init,   "Fidelity Electronics", "Elite A/S Challenger (Budapest program)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS | MACHINE_IMPERFECT_TIMING )
CONS( 1984, feasgla,  feasbu, 0, eas,      eas,   elite_state, empty_init,   "Fidelity Electronics", "Elite A/S Challenger (Glasgow program)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS | MACHINE_IMPERFECT_TIMING )
CONS( 1984, fepriv,   feasbu, 0, eas_priv, eas,   elite_state, empty_init,   "Fidelity Deutschland", "Elite Private Line (red version)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS | MACHINE_IMPERFECT_TIMING )

CONS( 1982, fpres,    0,      0, pc,       eas,   elite_state, empty_init,   "Fidelity Electronics", "Prestige Challenger (original program)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS | MACHINE_IMPERFECT_TIMING )
CONS( 1983, fpresbu,  fpres,  0, pc,       eas,   elite_state, empty_init,   "Fidelity Electronics", "Prestige Challenger (Budapest program)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS | MACHINE_IMPERFECT_TIMING )

CONS( 1986, feag,     0,      0, eag,      eag,   elite_state, empty_init,   "Fidelity Electronics", "Elite Avant Garde (model 6081)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS | MACHINE_IMPERFECT_TIMING )
CONS( 1986, feag2100, feag,   0, eag2100,  eag,   elite_state, init_eag2100, "Fidelity Electronics", "Elite Avant Garde 2100", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS | MACHINE_IMPERFECT_TIMING )
