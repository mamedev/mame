// license:BSD-3-Clause
// copyright-holders: Jonathan Gevaryahu, Carl
/***************************************************************************

        DECtalk DTC-03

****************************************************************************/

// LED error codes (repeating pattern of a 4 bit number and 0xF blinking on the 4 LEDs):
// 0x2 - bad ROM at f0000-fffff (E21/E44) c3b0a
// 0x4 - bad ROM at d0000-dffff (E33/E56) c3adf
// 0x6 - bad RAM at a0000-a3fff (E59?)    c3be8
// 0x8 - bad ROM at c0000-cffff (E41/E60) c3af6
// 0xa - bad RAM at 00000-03fff (E32/E55) c3b7e
// 0xc - bad ROM at e0000-effff (E29/E50) c3b21
// in test order:
// 0x4 - bad ROM at d0000-dffff (E33/E56) c3adf
// 0x8 - bad ROM at c0000-cffff (E41/E60) c3af6
// 0x2 - bad ROM at f0000-fffff (E21/E44) c3b0a
// 0xc - bad ROM at e0000-effff (E29/E50) c3b21
// 0xa - bad RAM at 00000-03fff (E32/E55) c3b7e
// 0x6 - bad RAM at a0000-a3fff (E59?)    c3be8
// 0x3 - DSP Fault




// 6264 SRAMs are at E32, E55 and E59, unclear which two are a pair (likely E32/E55 or E55/E59)

#include "emu.h"
#include "cpu/i86/i186.h"
#include "cpu/tms32010/tms32010.h"
#include "bus/rs232/rs232.h"
#include "machine/input_merger.h"
#include "machine/scn_pci.h"
#include "sound/dac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class dtc03_state : public driver_device
{
public:
	dtc03_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mainram(*this, "mainram")
		, m_dac(*this, "dac")
		, m_dsp(*this, "dsp")
		, m_epci(*this, "epci")
		, m_epci_irq(*this, "epci_irq")
		, m_rs232(*this, "rs232")
		, m_dsp_dma(0)
		, m_bio(ASSERT_LINE)
		, m_ctl(0)
		//, m_dbgclk(attotime::never)
	{
	}

	void dtc03(machine_config &config);

private:
	required_device<i80186_cpu_device> m_maincpu;
	required_shared_ptr<uint16_t> m_mainram;
	required_device<dac_12bit_r2r_device> m_dac;
	required_device<tms32010_device> m_dsp;
	required_device<scn2661c_device> m_epci;
	required_device<input_merger_device> m_epci_irq;
	required_device<rs232_port_device> m_rs232;
	void dtc03_io(address_map &map) ATTR_COLD;
	void dtc03_mem(address_map &map) ATTR_COLD;
	void dsp_io(address_map &map) ATTR_COLD;
	void dsp_mem(address_map &map) ATTR_COLD;

	u16 m_dsp_dma;
	u8 m_bio;
	u16 m_ctl;
	attotime m_dbgclk;

	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

	void dac_w(uint16_t data);
	uint16_t dsp_dma_r();
	void dsp_dma_w(uint16_t data);
	int bio_line_r();
	void dsp_clock_w(int state);
	void epci_txrx_clock_w(int state);
	void ctl_w(uint16_t data);
};


void dtc03_state::machine_reset()
{
	m_bio = ASSERT_LINE; // ?
	//m_bio = CLEAR_LINE;
}

void dtc03_state::machine_start()
{
	save_item(NAME(m_dsp_dma));
	save_item(NAME(m_bio));
	save_item(NAME(m_ctl));
}

void dtc03_state::dac_w(uint16_t data)
{
	m_dac->write(data >> 4);
}

uint16_t dtc03_state::dsp_dma_r()
{
	//m_bio = ASSERT_LINE;
	m_maincpu->drq1_w(0);
	logerror("dsp read dma\n");
	return m_dsp_dma;
}

void dtc03_state::dsp_dma_w(uint16_t data)
{
	m_bio = data&1; // CLEAR_LINE; ???
	//m_dsp_dma = data;
}

int dtc03_state::bio_line_r()
{
	// TODO: reading the bio line doesn't cause any direct external effects so this is wrong
	//if(m_bio == ASSERT_LINE)
	//  m_maincpu->drq0_w(1);
	return m_bio;
}

/* "ctl" bits known:
 fedcba9876543210
 ||||||||\\\\\\\\- unknown, unclear if used
 ||||\\\\--------- 4 bit LED value for the front panel, unclear which LED is which
 |||\------------- unknown
 ||\-------------- unknown, used/set after ROM/RAM tests, almost certainly 'DSP interrupt gate'
 |\--------------- unknown, probably 'DSP RESET'
 \---------------- unknown
 */
void dtc03_state::ctl_w(u16 data)
{
	logerror("CTL Write: %04X\n", data);
	popmessage("dsp out clock: %s; LED status: %01X\n", (data&0x2000)?"On":"Off",(data&0x0f00)>>8);
	//m_dsp->set_input_line(INPUT_LINE_RESET, (data & 0x4000) ? CLEAR_LINE : ASSERT_LINE); // this should be right but doesn't work?
	m_ctl = data;
}

/* port 500 read bits meaning
 fedcba9876543210
 ||||||||\\\\\\\\- DSW1 dip switches
 ||||||\\--------- open bus(reads as 1?)
 |||||\----------- X2 button?, read/checked at c3aac/c3aad (before rom test) and c3c32/c3c33
 ||||\------------ X1 button?, read/checked at c3aac/c3ab2 (before rom test)
 |||\------------- ST button
 ||\-------------- CL button
 |\--------------- DL button?, read/checked at c3b98/c3b99 (after rom test)
 \---------------- DM button
*/

void dtc03_state::dsp_clock_w(int state)
{
#if 0
	if (((m_ctl&0x2000) && state))
	{
		attotime dbgtime = (machine().time() - m_dbgclk);
		m_dbgclk = machine().time();
		logerror("dsp clock asserted, timing = %f hz\n", ATTOSECONDS_TO_HZ(dbgtime.attoseconds()));
	}
	else
	{
		logerror("dsp clock cleared\n");
	}
#endif
	//m_dsp->set_input_line(INPUT_LINE_IRQ0, (!(m_ctl & 0x2000) || state) ? CLEAR_LINE : ASSERT_LINE);
	m_dsp->set_input_line(INPUT_LINE_IRQ0, ((m_ctl&0x2000) && state) ? ASSERT_LINE : CLEAR_LINE);
	//m_dsp->set_input_line(0, ((m_ctl&0x2000) && state) ? ASSERT_LINE : CLEAR_LINE)); // TMS32010 INT
}

void dtc03_state::epci_txrx_clock_w(int state)
{
	m_epci->txc_w(state);
	m_epci->rxc_w(state);
}

void dtc03_state::dtc03_mem(address_map &map)
{
/* 80186 peripheral regs:
UMCS: C03C - 1100 0000 0011 1100 - address: c0000-fffff - purpose: rom area, no waitstates, no RDY
LMCS: 3FFC - 0011 1111 1111 1100 - address: 00000-3ffff - purpose: ram area 1, no waitstates, no RDY
PACS: 007C - 0000 0000 0111 1100 - peripheral base is 00000, no waitstates, no RDY
MMCS: 81FC - 1000 0001 1111 1100 - base address is 1000 000x xxxx xxxx xxxx i.e. 0x80000, no waitstates, no RDY
MPCS: A0BF - 1010 0000 1011 1111 - 256k block size, 64k select size (i.e. on a 0x10000 boundary), EX=1, MS=0, 3 waitstates, no RDY
This implies a memory mapped /cs for 80000, 90000, a0000, b0000 for the four lines
*/
/*
                   |           |           |           |
        19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0           a=0/a=1
/LCS     0  0  x  x  x  x  *  *  *  *  *  *  *  *  *  *  *  *  *  a  RW  SRAM E55/E32(loc?)
         0  1  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x  x      OPEN BUS (maybe used by the unreleased(?) expansion module)
/MCS1    1  0  0  0                                                      open footprint at E43(loc?)
/MCS2    1  0  0  1                                                      open footprint at E49(loc?)
/MCS3    1  0  1  0  x  x  *  *  *  *  *  *  *  *  *  *  *  *  *  0? RW  SRAM E59/xxx(loc?) (low(?) byte only)
/MCS4    1  0  1  1  x  x  *  *  *  *  *  *  *  *  *  *  *  *  *  a  RW  SRAM E55/E32(loc?), again
/UCS     1  1  0  0  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  a  R  EPROM E60/E41
/UCS     1  1  0  1  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  a  R  EPROM E56/E33
/UCS     1  1  1  0  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  a  R  EPROM E50/E29
/UCS     1  1  1  1  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *  a  R  EPROM E44/E21
*/
	map(0x00000, 0x03fff).mirror(0x3c000).ram().share("mainram");
	map(0xa0000, 0xa3fff).mirror(0x0c000).ram(); // should be only low or high order byte?
	map(0xb0000, 0xb3fff).mirror(0x0c000).ram().share("mainram");
	map(0xc0000, 0xfffff).rom().region("maincpu", 0xc0000);
}

void dtc03_state::dtc03_io(address_map &map)
{
	/* DTC-07
	map(0x0400, 0x0401).rw(FUNC(dectalk_isa_device::cmd_r), FUNC(dectalk_isa_device::status_w)); //PCS0
	map(0x0480, 0x0481).rw(FUNC(dectalk_isa_device::data_r), FUNC(dectalk_isa_device::data_w)); //PCS1
	map(0x0500, 0x0501).w(FUNC(dectalk_isa_device::dsp_dma_w)); //PCS2
	map(0x0580, 0x0581).r(FUNC(dectalk_isa_device::host_irq_r)); //PCS3
	map(0x0600, 0x0601).w(FUNC(dectalk_isa_device::output_ctl_w)); //PCS4
	map(0x0680, 0x0680).rw(FUNC(dectalk_isa_device::dma_r), FUNC(dectalk_isa_device::dma_w)); //PCS5
	map(0x0700, 0x0701).w(FUNC(dectalk_isa_device::irq_line_w)); //PCS6
	*/
	// DTC-03
	// dsp dma w is mapped somewhere here
	// the 6 front panel buttons are mapped somewhere here
	// the MT8870 DTMF decoder is mapped somewhere here
	// the control for the on-hook relay is mapped somewhere here
	// some pins on the expansion port might be mapped somewhere here as well.
	//?? // PCS0
	map(0x480, 0x481).w(FUNC(dtc03_state::ctl_w)).portr("IN1"); // PCS1
	map(0x500, 0x501).portr("IN1"); // PCS2
	//?? // PCS3
	//?? // PCS4
	map(0x680, 0x681).w(FUNC(dtc03_state::dsp_dma_w)); // PCS5
	map(0x700, 0x708).rw(m_epci, FUNC(scn2661c_device::read), FUNC(scn2661c_device::write)).umask16(0x00ff); // PCS6
}

void dtc03_state::dsp_mem(address_map &map) // guessing this hookup is the same as DTC-07
{
	map(0x0000, 0x0fff).rom().region("dsp", 0);
}

void dtc03_state::dsp_io(address_map &map) // guessing this hookup is the same as DTC-07
{
	map(0x0, 0x0).r(FUNC(dtc03_state::dsp_dma_r));
	map(0x1, 0x1).rw(FUNC(dtc03_state::dsp_dma_r), FUNC(dtc03_state::dac_w));
}


/* Input ports */
static INPUT_PORTS_START( dtc03 )
PORT_START("IN1")
	PORT_CONFNAME( 0x8000, 0x0000, "DM (Busy Out/Disable Module)")
	PORT_CONFSETTING(      0x0000, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x8000, DEF_STR( On ) )
	PORT_CONFNAME( 0x4000, 0x0000, "DL (Data Loopback)")
	PORT_CONFSETTING(      0x0000, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x4000, DEF_STR( On ) )
	PORT_CONFNAME( 0x2000, 0x0000, "CL (Modem Control Loopback)")
	PORT_CONFSETTING(      0x0000, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x2000, DEF_STR( On ) )
	PORT_CONFNAME( 0x1000, 0x0000, "ST (Self Tests)")
	PORT_CONFSETTING(      0x0000, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x1000, DEF_STR( On ) )
	PORT_CONFNAME( 0x0800, 0x0000, "X1 (Expansion Module Option 1)")
	PORT_CONFSETTING(      0x0000, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x0800, DEF_STR( On ) )
	PORT_CONFNAME( 0x0400, 0x0000, "X2 (Expansion Module Option 2)")
	PORT_CONFSETTING(      0x0000, DEF_STR( Off ) )
	PORT_CONFSETTING(      0x0400, DEF_STR( On ) )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_DIPNAME(  0x0080, 0x0000, "Enable DM button" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(       0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x0040, 0x0000, "SW1:2" )
	PORT_DIPNAME(  0x0020, 0x0000, "Keypad Mask on Power-Up" ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(       0x0020, "All Zeroes" )
	PORT_DIPSETTING(       0x0000, "All Ones" )
	PORT_DIPNAME(  0x0018, 0x0000, "EIA Serial Format" ) PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(       0x0000, "7 bits, odd parity" )
	PORT_DIPSETTING(       0x0008, "7 bits, even parity" )
	PORT_DIPSETTING(       0x0010, "8 bits, no parity" )
	PORT_DIPSETTING(       0x0018, "7 bits, ignore parity" )
	PORT_DIPNAME(  0x0007, 0x0007, "EIA Serial Baud Rate" ) PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(       0x0007, "9600" )
	PORT_DIPSETTING(       0x0006, "4800" )
	PORT_DIPSETTING(       0x0005, "2400" )
	PORT_DIPSETTING(       0x0004, "1200" )
	PORT_DIPSETTING(       0x0003, "600" )
	PORT_DIPSETTING(       0x0002, "300" )
	PORT_DIPSETTING(       0x0001, "150" )
	PORT_DIPSETTING(       0x0000, "110" ) /* technically this is the default, but for ease of use set it to 9600 */
INPUT_PORTS_END

void dtc03_state::dtc03(machine_config &config)
{
	/* basic machine hardware */
	I80186(config, m_maincpu, XTAL(16'000'000)); // AMD 80186, 16MHz xtal
	m_maincpu->set_addrmap(AS_PROGRAM, &dtc03_state::dtc03_mem);
	m_maincpu->set_addrmap(AS_IO, &dtc03_state::dtc03_io);
	m_maincpu->tmrout0_handler().set(FUNC(dtc03_state::dsp_clock_w));
	m_maincpu->tmrout1_handler().set(FUNC(dtc03_state::epci_txrx_clock_w));

	INPUT_MERGER_ANY_HIGH(config, m_epci_irq).output_handler().set(m_maincpu, FUNC(i80186_cpu_device::int0_w));

	TMS32010(config, m_dsp, XTAL(20'000'000)); // 20MHz xtal
	m_dsp->set_addrmap(AS_PROGRAM, &dtc03_state::dsp_mem);
	m_dsp->set_addrmap(AS_IO, &dtc03_state::dsp_io);
	m_dsp->bio().set(FUNC(dtc03_state::bio_line_r)); // guessing this hookup is the same as DTC-07

	SCN2661C(config, m_epci, 5068800); // this is wrong as there's no dedicated 5.0688MHz xtal, probably actually just XTAL(20'000'000)/4
	m_epci->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_epci->rts_handler().set(m_rs232, FUNC(rs232_port_device::write_rts));
	m_epci->dtr_handler().set(m_rs232, FUNC(rs232_port_device::write_dtr));
	m_epci->rxrdy_handler().set(m_epci_irq, FUNC(input_merger_device::in_w<0>)); // these are a guess
	m_epci->txrdy_handler().set(m_epci_irq, FUNC(input_merger_device::in_w<1>));

	RS232_PORT(config, m_rs232, default_rs232_devices, "terminal");
	m_rs232->rxd_handler().set(m_epci, FUNC(scn2661c_device::rxd_w));
	m_rs232->dcd_handler().set(m_epci, FUNC(scn2661c_device::dcd_w));
	m_rs232->dsr_handler().set(m_epci, FUNC(scn2661c_device::dsr_w));
	m_rs232->cts_handler().set(m_epci, FUNC(scn2661c_device::cts_w));

	SPEAKER(config, "speaker").front_center();
	DAC_12BIT_R2R(config, m_dac, 0).add_route(0, "speaker", 1.0); // AD7541 DAC
}

/* ROM definition */
ROM_START( dtc03 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	// dictionary and boot vector, common for all versions
	ROMX_LOAD( "23-028e6.e50",       0x0e0000, 0x008000, CRC(2788190e) SHA1(13af6d1cf5a69a0fc160ebd75ce891f7938a9cad), ROM_SKIP(1) )
	ROMX_LOAD( "23-032e6.e29",       0x0e0001, 0x008000, CRC(d6f4edd0) SHA1(1268a938c44f2332173edab4a3b41a9fb58f31ae), ROM_SKIP(1) )
	ROMX_LOAD( "23-029e6.e44",       0x0f0000, 0x008000, CRC(49d0ff90) SHA1(7398c50cf10bc918ca8de55aec3032bf0d00e5c4), ROM_SKIP(1) )
	ROMX_LOAD( "23-033e6.e21",       0x0f0001, 0x008000, CRC(38516d0c) SHA1(228c6d644b8e3d9b9e649b3de2e24780a189e661), ROM_SKIP(1) )

	ROM_SYSTEM_BIOS( 0, "v43", "DTC-03 Version 4.3")
	ROMX_LOAD( "23-425e6.e60",       0x0c0000, 0x008000, CRC(a7c11541) SHA1(b0b5b8633849369d9a50a04498ebdf3597b74051), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "23-427e6.e41",       0x0c0001, 0x008000, CRC(8e45da49) SHA1(64e0b840365e1094e574841f22e9107a754dd70c), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "23-426e6.e56",       0x0d0000, 0x008000, CRC(7191c670) SHA1(32e18f00b459e868189a084abe528868f07705f5), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "23-428e6.e33",       0x0d0001, 0x008000, CRC(23a12574) SHA1(44c87987f7b6c8d7e5bec4bf39259ff7edc986ed), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v42", "DTC-03 Version 4.2")
	ROMX_LOAD( "23-305e6.e60",       0x0c0000, 0x008000, CRC(1c7fd2a0) SHA1(d64cb27a2583b33857ccfb0babc3e528d7bd5c1d), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "23-307e6.e41",       0x0c0001, 0x008000, CRC(aea1d679) SHA1(dae9a599c411c7c757c74f3ececa55209b0bf066), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "23-306e6.e56",       0x0d0000, 0x008000, CRC(83411f4d) SHA1(c0844fc2777b79f3997f97948f1cf1bc2a568fe9), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "23-308e6.e33",       0x0d0001, 0x008000, CRC(8e12680a) SHA1(d7b5870ded2568d5d9bad0cafa3996995e3bc3c5), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 2, "v41", "DTC-03 Version 4.1")
	ROMX_LOAD( "23-114e6.e60",       0x0c0000, 0x008000, CRC(122c69c0) SHA1(d6a2b6c57c966d07c3cfee346d3d0b4baca2426e), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "23-116e6.e41",       0x0c0001, 0x008000, CRC(3641f5e2) SHA1(6c93a697caf59f027d8a23ac34e5e356a1cba524), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "23-115e6.e56",       0x0d0000, 0x008000, CRC(21c4c6f8) SHA1(6796adfec81fa0bfa83f5f61c81e57dae97085c7), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "23-117e6.e33",       0x0d0001, 0x008000, CRC(3c65dfab) SHA1(d430fe04172f30aa196ef9480545575c7ec8fbb1), ROM_SKIP(1) | ROM_BIOS(2))

	ROM_REGION( 0x2000, "dsp", 0)
	ROMX_LOAD( "23-230f4.82s191.e11",0x000000, 0x000800, CRC(e32aff61) SHA1(7c014a988c9e583096ef1cfc142c7c0a4444168e), ROM_SKIP(1) )
	ROMX_LOAD( "23-229f4.82s191.e18",0x000001, 0x000800, CRC(0beddc21) SHA1(5eb4948c49ee40613dce93f2017ba55ba2b59767), ROM_SKIP(1) )
ROM_END

} // anonymous namespace

/* Driver */

/*    YEAR   NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY                          FULLNAME   FLAGS */
COMP( 1985,  dtc03,  0,      0,      dtc03,   dtc03, dtc03_state,  empty_init, "Digital Equipment Corporation", "DECtalk DTC-03",   MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
