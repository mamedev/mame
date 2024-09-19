// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert
/*************************************************************************************

    Yamaha MU-100 : 32-part, 64-voice polyphonic/multitimbral General MIDI/GS/XG
                    tone module
    Preliminary driver by R. Belmont and O. Galibert

    The successor to the mu90, uses the same one-chip SWP30 with a better sample rom.
    Exists in rackable (mu100r) and screenless (mu100b), and single board (xt446)
    variants.

    MU100 CPU: Hitachi H8S/2655 (HD6432655F), strapped for mode 4 (24-bit address, 16-bit data, no internal ROM)
    Sound ASIC: Yamaha XS725A0/SWP30
    RAM: 1 MSM51008 (1 meg * 1 bit = 128KBytes)

    I/O ports from service manual:

    Port 1
        0 - LCD data, SW data, LED 1
        1 - LCD data, SW data, LED 2
        2 - LCD data, SW data, LED 3
        3 - LCD data, SW data, LED 4
        4 - LCD data, SW data, LED 5
        5 - LCD data, SW strobe data
        6 - LCD data, SW strobe data
        7 - LCD data, SW data, LED 6

    Port 2:
        0 - (out) LCD control RS
        1 - (out) LCD control R/W
        2 - (out) LCD control E
        3 - (out) LCD contrast A
        4 - (out) LCD contrast B
        5 - (out) LCD contrast C
        6 - (out) 1 MHz clock for serial
        7 - NC

    Port 3:
        4 - (out) A/D gain control 1
        5 - (out) A/D gain control 2

    Port 5:
        3 - (out) Reset signal for rotary encoder

    Port 6:
        1 - NC
        2 - (out) PB select (SW1)
        3 - (out) PB select (SW2)
        4 - (out) reset PB
        5 - (out) reset SWP30 (sound chip)
        6 - NC
        7 - (in) Plug detection for A/D input

    Port A:
        5 - (in) Off Line Detection
        6 - (out) Signal for rotary encoder (REB)
        7 - (out) Signal for rotary encoder (REA)


    Port F:
        0 - (out) (sws) LED,SW Strobe data latch
        1 - (out) (swd) SW data read control
        2 - (out) PB select (SW4)

    Port G:
        0 - (out) PB select (SW3)

    Analog input channels:
        0 - level input R
        2 - level output L
        4 - host SW type switch position
        6 - battery voltage
        7 - model check (0 for MU100, 0.5 for OEM, 1 for MU100R)

    Switch map at the connector (17=ground)
        09 8 play
        10 8 edit
        11 8 mute/solo
        12 8 part -
        13 8 part +
        14 8 util
        15 8 effect
        16 8 enter
        12 7 select <
        13 7 select >
        16 7 mode
        15 7 eq
        14 7 exit
        10 7 value -
        11 7 value +
           2 led play
           3 led edit
           4 led util
           5 led effect
           6 led mode
           1 led eq

     IC32:
        1 p10 c.2
        2 p11 c.3
        3 p12 c.4
        4 p13 c.5
        5 p14 c.6
        6 p15 c.7
        7 p16 c.8
        8 p17 c.1
        g sws

     IC33
        1 p17 c.09
        2 p16 c.10
        3 p15 c.11
        4 p14 c.12
        5 p13 c.13
        6 p12 c.14
        7 p11 c.15
        8 p10 c.16
        g swd

**************************************************************************************/

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/h8/h8s2655.h"
#include "mulcd.h"
#include "sound/swp30.h"
#include "bus/plg1x0/plg1x0.h"

#include "debugger.h"
#include "speaker.h"


namespace {

static INPUT_PORTS_START( mu100 )
	PORT_START("P7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Enter")     PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Effect")    PORT_CODE(KEYCODE_F)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Util")      PORT_CODE(KEYCODE_U)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part +")    PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Part -")    PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mute/Solo") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Edit")      PORT_CODE(KEYCODE_E)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Play")      PORT_CODE(KEYCODE_A)

	PORT_START("P8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Mode")      PORT_CODE(KEYCODE_M)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Eq")        PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Exit")      PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select >")  PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Select <")  PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value +")   PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Value -")   PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

class mu100_state : public driver_device
{
public:
	mu100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_swp30(*this, "swp30")
		, m_lcd(*this, "lcd")
		, m_ext1(*this, "ext1")
		, m_ext2(*this, "ext2")
		, m_ioport_p7(*this, "P7")
		, m_ioport_p8(*this, "P8")
	{ }

	void mu100(machine_config &config);
	void mu100b(machine_config &config);

protected:
	virtual u16 adc_type_r();

	enum {
		P2_LCD_RS     = 0x01,
		P2_LCD_RW     = 0x02,
		P2_LCD_ENABLE = 0x04
	};

	required_device<h8s2655_device> m_maincpu;
	required_device<swp30_device> m_swp30;
	optional_device<mulcd_device> m_lcd;
	required_device<plg1x0_connector> m_ext1;
	optional_device<plg1x0_connector> m_ext2;
	required_ioport m_ioport_p7;
	required_ioport m_ioport_p8;

	u8 m_cur_p1, m_cur_p2, m_cur_p3, m_cur_p5, m_cur_p6, m_cur_pa, m_cur_pb, m_cur_pc, m_cur_pf, m_cur_pg;
	u8 m_cur_ic32, m_cur_sw;
	int m_h8_tx, m_e1_tx, m_e2_tx;

	u16 adc_ar_r();
	u16 adc_al_r();
	u16 adc_midisw_r();
	u16 adc_battery_r();

	void p1_w(u8 data);
	u8 p1_r();
	void p2_w(u8 data);
	void p3_w(u8 data);
	void p5_w(u8 data);
	void p6_w(u8 data);
	u8 p6_r();
	void pa_w(u8 data);
	u8 pa_r();
	void pb_w(u8 data);
	u8 pb_r();
	void pf_w(u8 data);
	void pg_w(u8 data);

	void ext_serial_update();
	void h8_tx(int state);
	void e1_tx(int state);
	void e2_tx(int state);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void mu100_map(address_map &map) ATTR_COLD;
	void swp30_map(address_map &map) ATTR_COLD;
};

class mu100r_state : public mu100_state {
public:
	mu100r_state(const machine_config &mconfig, device_type type, const char *tag)
		: mu100_state(mconfig, type, tag)
	{ }

	void mu100r(machine_config &config);

private:
	virtual u16 adc_type_r() override;
};

void mu100_state::machine_start()
{
	save_item(NAME(m_cur_p1));
	save_item(NAME(m_cur_p2));
	save_item(NAME(m_cur_p3));
	save_item(NAME(m_cur_p5));
	save_item(NAME(m_cur_p6));
	save_item(NAME(m_cur_pa));
	save_item(NAME(m_cur_pc));
	save_item(NAME(m_cur_pf));
	save_item(NAME(m_cur_pg));
	save_item(NAME(m_cur_ic32));
	save_item(NAME(m_cur_sw));
	save_item(NAME(m_h8_tx));
	save_item(NAME(m_e1_tx));
	save_item(NAME(m_e2_tx));
}

void mu100_state::machine_reset()
{
	m_cur_p1 = m_cur_p2 = m_cur_p3 = m_cur_p5 = m_cur_p6 = m_cur_pa = m_cur_pc = m_cur_pf = m_cur_pg = m_cur_ic32 = 0xff;
	m_cur_sw = 0;
	m_h8_tx = m_e1_tx = m_e2_tx = 1;
	ext_serial_update();
}

void mu100_state::mu100_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("maincpu", 0);
	map(0x200000, 0x21ffff).ram(); // 128K work RAM
	map(0x400000, 0x401fff).m(m_swp30, FUNC(swp30_device::map));
}

// Analog input right (also sent to the swp)
u16 mu100_state::adc_ar_r()
{
	return 0;
}

// Analog input left (also sent to the swp)
u16 mu100_state::adc_al_r()
{
	return 0;
}

// Put the host switch to pure midi
u16 mu100_state::adc_midisw_r()
{
	return 0;
}

// Battery level
u16 mu100_state::adc_battery_r()
{
	return 0x200;
}

// model detect.  pulled to GND (0) on MU100, to 0.5Vcc on the card version, to Vcc on MU100R
u16 mu100_state::adc_type_r()
{
	return 0;
}

u16 mu100r_state::adc_type_r()
{
	return 0x3ff;
}

void mu100_state::p1_w(u8 data)
{
	m_cur_p1 = data;
}

u8 mu100_state::p1_r()
{
	if(m_lcd)
		if((m_cur_p2 & P2_LCD_ENABLE)) {
			if(m_cur_p2 & P2_LCD_RW) {
				if(m_cur_p2 & P2_LCD_RS)
					return m_lcd->data_read();
				else
					return m_lcd->control_read();
			} else
				return 0x00;
		}

	if(!(m_cur_pf & 0x02)) {
		u8 val = 0xff;
		if(!(m_cur_ic32 & 0x20))
			val &= m_ioport_p7->read();
		if(!(m_cur_ic32 & 0x40))
			val &= m_ioport_p8->read();
		return val;
	}

	return 0xff;
}

void mu100_state::p2_w(u8 data)
{
	if(m_lcd) {
	// LCD enable edge
		if((m_cur_p2 & P2_LCD_ENABLE) && !(data & P2_LCD_ENABLE)) {
			if(!(m_cur_p2 & P2_LCD_RW)) {
				if(m_cur_p2 & P2_LCD_RS)
					m_lcd->data_write(m_cur_p1);
				else
					m_lcd->control_write(m_cur_p1);
			}
		}
		m_lcd->set_contrast((data >> 3) & 7);
	}
	m_cur_p2 = data;
}

void mu100_state::p3_w(u8 data)
{
	m_cur_p3 = data;
	logerror("A/D gain control %d\n", (data >> 4) & 3);
}

void mu100_state::p5_w(u8 data)
{
	m_cur_p5 = data;
	logerror("Rotary reset %d\n", (data >> 3) & 1);
}

void mu100_state::p6_w(u8 data)
{
	m_cur_p6 = data;
	m_cur_sw = (m_cur_sw & 0xc) | BIT(m_cur_pf, 2, 2);
	ext_serial_update();
}

u8 mu100_state::p6_r()
{
	//  logerror("plug in detect read\n");
	return 0x00;
}

void mu100_state::pa_w(u8 data)
{
	m_cur_pa = data;
	logerror("rotary encoder %d\n", (data >> 6) & 3);
}

u8 mu100_state::pa_r()
{
	logerror("offline detect read\n");
	return 0x00;
}

void mu100_state::pf_w(u8 data)
{
	if(!(m_cur_pf & 0x01) && (data & 0x01)) {
		m_cur_ic32 = m_cur_p1;
		if(m_lcd)
			m_lcd->set_leds((m_cur_p1 & 0x1f) | ((m_cur_p1 & 0x80) >> 2));
	}
	m_cur_pf = data;
	m_cur_sw = (m_cur_sw & 0x7) | (BIT(m_cur_pf, 2) << 3);
	ext_serial_update();
}

void mu100_state::pg_w(u8 data)
{
	m_cur_pg = data;
	m_cur_sw = (m_cur_sw & 0xb) | (BIT(m_cur_pg, 0) << 2);
	ext_serial_update();
}

void mu100_state::ext_serial_update()
{
	m_ext1->midi_rx(BIT(m_cur_sw, 3) ? m_h8_tx : 1);
	if(m_ext2)
		m_ext2->midi_rx(BIT(m_cur_sw, 1) ? m_h8_tx : 1);
	if(BIT(m_cur_sw, 2))
		if(BIT(m_cur_sw, 0))
			m_maincpu->sci_rx_w<2>(m_e1_tx && m_e2_tx);
		else
			m_maincpu->sci_rx_w<2>(m_e1_tx);
	else
		if(BIT(m_cur_sw, 0))
			m_maincpu->sci_rx_w<2>(m_e2_tx);
		else
			m_maincpu->sci_rx_w<2>(1);
}

void mu100_state::h8_tx(int state)
{
	m_h8_tx = state;
	ext_serial_update();
}

void mu100_state::e1_tx(int state)
{
	m_e1_tx = state;
	ext_serial_update();
}

void mu100_state::e2_tx(int state)
{
	m_e2_tx = state;
	ext_serial_update();
}

void mu100_state::swp30_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom().region("swp30",         0).mirror(0x200000);
	map(0x400000, 0x4fffff).rom().region("swp30",  0x800000).mirror(0x300000);
	map(0x800000, 0x9fffff).rom().region("swp30", 0x1000000).mirror(0x200000);
}

void mu100_state::mu100b(machine_config &config)
{
	H8S2655(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mu100_state::mu100_map);
	m_maincpu->read_adc<0>().set(FUNC(mu100_state::adc_ar_r));
	m_maincpu->read_adc<1>().set_constant(0);
	m_maincpu->read_adc<2>().set(FUNC(mu100_state::adc_al_r));
	m_maincpu->read_adc<3>().set_constant(0);
	m_maincpu->read_adc<4>().set(FUNC(mu100_state::adc_midisw_r));
	m_maincpu->read_adc<5>().set_constant(0);
	m_maincpu->read_adc<6>().set(FUNC(mu100_state::adc_battery_r));
	m_maincpu->read_adc<7>().set(FUNC(mu100_state::adc_type_r));
	m_maincpu->read_port1().set(FUNC(mu100_state::p1_r));
	m_maincpu->write_port1().set(FUNC(mu100_state::p1_w));
	m_maincpu->write_port2().set(FUNC(mu100_state::p2_w));
	m_maincpu->write_port3().set(FUNC(mu100_state::p3_w));
	m_maincpu->write_port5().set(FUNC(mu100_state::p5_w));
	m_maincpu->read_port6().set(FUNC(mu100_state::p6_r));
	m_maincpu->write_port6().set(FUNC(mu100_state::p6_w));
	m_maincpu->read_porta().set(FUNC(mu100_state::pa_r));
	m_maincpu->write_porta().set(FUNC(mu100_state::pa_w));
	m_maincpu->write_portf().set(FUNC(mu100_state::pf_w));
	m_maincpu->write_portg().set(FUNC(mu100_state::pg_w));
	m_maincpu->write_sci_tx<2>().set(FUNC(mu100_state::h8_tx));

	PLG1X0_CONNECTOR(config, m_ext1, plg1x0_intf, nullptr);
	m_ext1->midi_tx().set(FUNC(mu100_state::e1_tx));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SWP30(config, m_swp30);
	m_swp30->set_addrmap(AS_DATA, &mu100_state::swp30_map);
	m_swp30->add_route(0, "lspeaker", 1.0);
	m_swp30->add_route(1, "rspeaker", 1.0);

	auto &mdin_a(MIDI_PORT(config, "mdin_a"));
	midiin_slot(mdin_a);
	mdin_a.rxd_handler().set(m_maincpu, FUNC(h8s2655_device::sci_rx_w<1>));

	auto &mdin_b(MIDI_PORT(config, "mdin_b"));
	midiin_slot(mdin_b);
	mdin_b.rxd_handler().set(m_maincpu, FUNC(h8s2655_device::sci_rx_w<0>));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_maincpu->write_sci_tx<0>().set(mdout, FUNC(midi_port_device::write_txd));
}

void mu100_state::mu100(machine_config &config)
{
	mu100b(config);

	MULCD(config, m_lcd);
}

void mu100r_state::mu100r(machine_config &config)
{
	mu100(config);

	PLG1X0_CONNECTOR(config, m_ext2, plg1x0_intf, nullptr);
	m_ext2->midi_tx().set(FUNC(mu100r_state::e2_tx));
}

#define ROM_LOAD16_WORD_SWAP_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_REVERSE | ROM_BIOS(bios))

ROM_START( mu100 )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("v111")
	ROM_SYSTEM_BIOS( 0, "v111", "xu50720 (v1.11, Aug. 3, 1999)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 0, "xu50720.ic11", 0x000000, 0x200000, CRC(1126a8a4) SHA1(e90b8bd9d14297da26ba12f4d9a4f2d22cd7d34a) )
	ROM_SYSTEM_BIOS( 1, "v106", "xt714h0 (v1.06, Oct. 14, 1997)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 1, "xt714h0.ic11", 0x000000, 0x200000, CRC(aa96ab38) SHA1(ec39eeab55d7d55b4f6d2b4b4cac2a01f98db8a0) )
	ROM_SYSTEM_BIOS( 2, "v105", "xt71420 (v1.05, Sep. 19, 1997)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 2, "xt71420.ic11", 0x000000, 0x200000, CRC(0e5b3bae) SHA1(3148c5bd59a3d00809d3ab1921216215fe2582c5) )
	ROM_SYSTEM_BIOS( 3, "v103", "xt714e0 (v1.03, Jul. 25, 1997)" )
	ROM_LOAD16_WORD_SWAP_BIOS( 3, "xt714e0.ic11", 0x000000, 0x200000, CRC(2d8cf9fc) SHA1(a81f988a315efe92106f1e7d407cd3626c4f843f) )

	ROM_REGION32_LE( 0x1800000, "swp30", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD( "xs518b0.ic34", 0x0000000, 0x400000, CRC(2550d44f) SHA1(fd3cce228c7d389a2fde25c808a5b26080588cba) )
	ROM_LOAD32_WORD( "xs743b0.ic35", 0x0000002, 0x400000, CRC(a9109a6c) SHA1(a67bb49378a38a2d809bd717d286e18bc6496db0) )
	ROM_LOAD32_WORD( "xt445a0-828.ic36", 0x0800000, 0x200000, CRC(225c2280) SHA1(23b5e046fd2e2ac01af3e6dc6357c5c6547b286b) )
	ROM_LOAD32_WORD( "xt461a0-829.ic37", 0x0800002, 0x200000, CRC(a1d138a3) SHA1(46a7a7225cd7e1818ba551325d2af5ac1bf5b2bf) )
	ROM_LOAD32_WORD( "xt462a0.ic39", 0x1000000, 0x400000, CRC(2e82cbd4) SHA1(d1f0e2713bf2cca9156c562e23fcce4fa5d7cfb3) )
	ROM_LOAD32_WORD( "xt463a0.ic38", 0x1000002, 0x400000, CRC(cce5f8d3) SHA1(bdca8c5158f452f2b5535c7d658c9b22c6d66048) )
ROM_END

// mu100r roms are identical to the mu100
#define rom_mu100r rom_mu100

ROM_START( mu100b )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// MU-100B v1.08 (Nov. 28, 1997)
	ROM_LOAD16_WORD_SWAP( "xu50710.bin", 0x000000, 0x200000, CRC(4b10bd27) SHA1(12d7c6e1bce7974b34916e1bfa5057ab55867476) )

	ROM_REGION32_LE( 0x1800000, "swp30", ROMREGION_ERASE00 )
	ROM_LOAD32_WORD( "sx518b0.ic34", 0x0000000, 0x400000, CRC(2550d44f) SHA1(fd3cce228c7d389a2fde25c808a5b26080588cba) )
	ROM_LOAD32_WORD( "sx743b0.ic35", 0x0000002, 0x400000, CRC(a9109a6c) SHA1(a67bb49378a38a2d809bd717d286e18bc6496db0) )
	ROM_LOAD32_WORD( "xt445a0-828.ic36", 0x0800000, 0x200000, CRC(225c2280) SHA1(23b5e046fd2e2ac01af3e6dc6357c5c6547b286b) )
	ROM_LOAD32_WORD( "xt461a0-829.ic37", 0x0800002, 0x200000, CRC(a1d138a3) SHA1(46a7a7225cd7e1818ba551325d2af5ac1bf5b2bf) )
	ROM_LOAD32_WORD( "xt462a0.ic39", 0x1000000, 0x400000, CRC(2e82cbd4) SHA1(d1f0e2713bf2cca9156c562e23fcce4fa5d7cfb3) )
	ROM_LOAD32_WORD( "xt463a0.ic38", 0x1000002, 0x400000, CRC(cce5f8d3) SHA1(bdca8c5158f452f2b5535c7d658c9b22c6d66048) )
ROM_END

} // anonymous namespace


SYST( 1997, mu100,  0,     0, mu100,  mu100, mu100_state,  empty_init, "Yamaha", "MU100",                    MACHINE_NOT_WORKING )
SYST( 1997, mu100r, mu100, 0, mu100r, mu100, mu100r_state, empty_init, "Yamaha", "MU100 Rackable version",   MACHINE_NOT_WORKING )
SYST( 1998, mu100b, mu100, 0, mu100b, mu100, mu100_state,  empty_init, "Yamaha", "MU100 Screenless version", MACHINE_NOT_WORKING )
