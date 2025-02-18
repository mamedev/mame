// license:BSD-3-Clause
// copyright-holders:rfka01
/***************************************************************************

Mikrocomputer fuer Ausbildung
Berufsfoerdungszentrum Essen
Information found on Wikipedia:
- System is a backbone upon which all functions are available on plug-in cards
- 32k RAM, 32k ROM - either multiple 8k or 16k modules or a single 64k module 
  are used for the desired RAM/ROM complement
- Serial is via CPU SID/SOD pins, but can be replaced by 8251 on RS232 card
- Protocol is COM1, 4800, 8N1
- Timer card uses 8253
- PIO card uses 8255 and has a printer interface
- Cassette interface uses RS232 card
- Optional floppy (both sizes); and a EPROM burner
- OS: MAT85

Manuals have no schematics and no mention of the 8253 or 8255. The bios
doesn't try communicating with them either.

Row    a         c
|-------------------------|
| +5V       1    +5V      |
| CLKOUT/   2    S1/      |
| ALE/      3    S0/      | 
| D0        4    D1       | 
| D2        5    D3       | 
| D4        6    D5       | 
| D6        7    D7       |
| HLDA/     8    INTA/    |  
| IOW/      9    MEMW/    |
| IOR/     10    MEMR/    | 
| ROM/     11    IN/      | 
| RAM/     12    OUT/     |
| TRAP     13    RESOUT/  |  
|          14             | 
|          15    A16      | 
| A17      16    A00      | 
| A01      17    A02      | 
| A03      18    A04      |
| A05      19    A06      | 
| A07      20    A08      |
| A09      21    A10      | 
| A11      22    A12      |
| A13      23    A14      |
| A15      24             |
| RST7.5   25    INTR     |
| READY    26    RST5.5   |
| RESIN/   27    RST6.5   | 
| HOLD     28             | 
| PULSOUT  29    RX       | 
| PLS f. I.30    TX       | signal 30a: PULS F.INT.
| +12V     31    -12V     |
| 0V       32    0V       | 
|-------------------------

Commands:
A     Assembler
B     Set Breakpoint
D     Disassembler
G     Go
H     Help
I     Inport
L     Load memory from tape
M     Print/Modify memory (A=ascii, B=bit, H=hex)
N     Turn on tracer & step to next instruction
O     Outport
P     Display memory contents in various formats
R     Set initial register contents
S     Save memory to tape
T     Trace interval

Pressing enter will change the prompt from KMD > to KMD+> for the commands present in
the extended MAT ROM, and pressing space will change it back.

mfabfz85 -bios 0, 3 and 4 work; others produce rubbish.

Cassette:
- Like many early designs, the interface is grossly over-complicated, using 12 chips.
- Similar to Kansas City, except that 1 = 3600Hz, 0 = 2400Hz
- The higher frequencies, only 50% apart, cause the interface to be less reliable
- Baud rates of 150, 300, 600, 1200 selected by a jumper. We emulate 1200 only,
  as the current code would be too unreliable with the lower rates.
  
CP/M Ausbaustufe (CP/M expansion level):
The modularity of this teaching system meant that most of the components of a complete
system capable of CP/M were available, you could etch your own PCBs or buy kits.
You could buy the manual for the CP/M expansion that describes the modifications to turn
your training kit into a full blown CP/M machine, but it was closed source.
In 2019, in a community effort for the German vintage computer forum https://forum.classic-computing.de/forum/ ,
Mike Douglas at https://deramp.com/downloads/mfa_computer/ released an open source CP/M version and
monitor ROM. This effort revealed that the MFA's floppy module contained a 3.5"/80 track DD floppy drive
and could use any DD capable drive. In its native MFA and the original MFA CP/M mode, only
the first 40 tracks were used.
In the CP/M capable version, the MFA uses an 8085 processor, 64K RAM card with a bankable 2K boot
EPROM, one or more serial cards at I/O addresses 0xA0, 0x90 and 0xF0, a printer card at E0 and a WD1793
based floppy controller at 0xC0. Bank switching is performed by a read at address 0xf200.
The console is at 0xA0 and can be handled via a terminal emulator on a PC or routed over the computer's
bus to a video/keyboard module. The terminal is set to 9600 Baud, no parity, 1 stop bit.

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "bus/rs232/rs232.h"
#include "speaker.h"
#include "softlist_dev.h"


namespace {

class mfabfz_state : public driver_device
{
public:
	mfabfz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cass(*this, "cassette")
		, m_uart(*this, "uart2")
	{ }

	void mfabfz85(machine_config &config);
	void mfabfz(machine_config &config);

private:
	void mfabfz85_io(address_map &map) ATTR_COLD;
	void mfabfz_io(address_map &map) ATTR_COLD;
	void mfabfz_mem(address_map &map) ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	void machine_start() override ATTR_COLD;
	void kansas_r(int state);
	void kansas_w(int state);
	u8 m_cass_data[5]{};
	bool m_cassoutbit = false, m_cassbit = false, m_cassold = false;
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cass;
	required_device<i8251_device> m_uart;
};

class mfacpm_state : public driver_device
{
public:
	mfacpm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_shared_ram(*this, "shared_ram")
		, m_bootview(*this, "bootview") // see memory map
		, m_uart(*this, "uart2")
	{ }

	void mfacpm(machine_config &config);

private:
	void mfacpm_io(address_map &map) ATTR_COLD;
	void mfacpm_mem(address_map &map) ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	void machine_start() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_shared_ram;
	memory_view m_bootview;
	required_device<i8251_device> m_uart;
};

void mfabfz_state::mfabfz_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rom().region("roms", 0);
	map(0x8000, 0xffff).ram();
}

// According to the manual for the 64k RAM card, in the original MFA mode, 
// reading an address equal or higher to the boundary set by solder bridges on the card
// (which offers the banking of 2/4/8/16/32k ROMs and is needed for CP/M mode), 
// memory operations below the boundary reset the card to its ROM active state.
// For CP/M mode, this behaviour can be changed, so that ROM is banked out by a 
// read from 0xf200 (CP/M boot ROMs are 2k), and only a reset will restore 
// ROM from 0x0000.

// Memory card manual at https://oldcomputers.dyndns.org/public/pub/rechner/mfa_mikrocomputer_fuer_ausbildung/mfa_64k_ram_rom_3.3b/mfa_-_64k_ram_rom.pdf
// CP/M manual at https://oldcomputers.dyndns.org/public/pub/rechner/mfa_mikrocomputer_fuer_ausbildung/mfa_cpm_handbuch/mfa_-_cpm_handbuch.pdf

void mfacpm_state::mfacpm_mem(address_map &map)
{
	map(0x0000, 0x07ff).ram().share("shared_ram");
	map(0x0800, 0xffff).ram();
	map(0x0000, 0xffff).view(m_bootview);
	m_bootview[0](0x0000, 0x07ff).rom().region("roms", 0); // the actual banking mechanism is missing
}

void mfabfz_state::mfabfz_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xbe, 0xbf).rw("uart1", FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0xfe, 0xff).rw("uart2", FUNC(i8251_device::read), FUNC(i8251_device::write));
	// map(0xc0, 0xcf).rw // Floppy port, c0 ... c3 and c8 are used with a WD1793 floppy controller, 40 tracks, 8 sectors, on 3.5" DD media
}

void mfacpm_state::mfacpm_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x90, 0x9f).rw("uart2", FUNC(i8251_device::read), FUNC(i8251_device::write)); // secondary serial port
	map(0xa0, 0xaf).rw("uart1", FUNC(i8251_device::read), FUNC(i8251_device::write)); // Terminal and data transfer
	// map(0xc0, 0xc3).rw // Floppy port, 0xc0 ... 0xc3 and 0xc8 are used with a WD1793 floppy controller, 80 tracks, 9 sectors on 3.5" DD media 
	// for the open CP/M, 40 tracks, 8 sectors on 3.5" DD media for the original CP/M
	// map(0xe0, 0xef).rw // Centronics printer card
	// map(0xf0, 0xff).rw // tertiary serial port
}
/* Floppy interface, needed for CP/M
https://oldcomputers.dyndns.org/public/pub/rechner/mfa_mikrocomputer_fuer_ausbildung/mfa_floppy_controller_4.7b_and_drive/mfa_-_floppy_disk(seiten_einzeln).pdf
The floppy interface at 0xc0 is based on the WD1793 floppy controller. IO lines 0xc0 ... 0xc3 are used for communication,
0xc8 halts the processor via the CPU's READY line. DRQ and INTRQ are used to synchronize the data transfer.
The floppy controller responds to RST5.5. */

/* The parallel printer card uses a non-standard cable to connect to printers with a Centronics interface
https://oldcomputers.dyndns.org/public/pub/rechner/mfa_mikrocomputer_fuer_ausbildung/mfa_programmable_parallel_&_eprommer_4.3b/mfa_-_programmable_parallel_&_eprommer_4.3.pdf
DB25 	Centronics
1		 2
2		 3
3		 4
4		 5
5		 6
6		 7
7		 8
8		 9
9 - 21 not connected 	
22		32
23		10
24		 1
25		16 */

void mfabfz_state::mfabfz85_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0xfe, 0xff).rw("uart2", FUNC(i8251_device::read), FUNC(i8251_device::write));
}

/* Input ports */
static INPUT_PORTS_START( mfabfz )
INPUT_PORTS_END

/* Input ports */
static INPUT_PORTS_START( mfacpm )
INPUT_PORTS_END

// Note: if the other baud rates are to be supported, then this function
//       will need to be redesigned.
void mfabfz_state::kansas_w(int state)
{
	if ((m_cass->get_state() & CASSETTE_MASK_UISTATE) == CASSETTE_RECORD)
	{
		if (state)
		{
			// incoming @76923Hz (1200), 38461.5 (600), 19231.77 (300), 9615.38 (150)
			u8 twobit = m_cass_data[3] & 63;
			static u8 cycles[3] = { 11, 10, 11 }; // 1200 baud

			if (twobit == 0)
			{
				m_cassold = m_cassoutbit;
				m_cass_data[2] = 0;
				m_cass_data[4] = 0;
			}

			if (m_cass_data[2] == 0)
			{
				m_cassbit ^= 1;
				m_cass->output(m_cassbit ? -1.0 : +1.0);

				if (m_cassold)
				{
					m_cass_data[4]++;
					if (m_cass_data[4] > 2)
						m_cass_data[4] = 0;
					m_cass_data[2] = cycles[m_cass_data[4]]; // 3600 Hz
				}
				else
					m_cass_data[2] = 16; // 2400 Hz
			}

			m_cass_data[2]--;
			m_cass_data[3]++;
		}
	}

	m_uart->write_txc(state);
}

void mfabfz_state::kansas_r(int state)
{
	// incoming @76923Hz
	if (state)
	{
		// no tape - set to idle
		m_cass_data[1]++;
		if (m_cass_data[1] > 32)
		{
			m_cass_data[1] = 32;
			m_uart->write_rxd(1);
		}

		if ((m_cass->get_state() & CASSETTE_MASK_UISTATE) != CASSETTE_PLAY)
			return;

		/* cassette - turn 2400/3600Hz to a bit */
		uint8_t cass_ws = (m_cass->input() > +0.04) ? 1 : 0;

		if (cass_ws != m_cass_data[0])
		{
			m_cass_data[0] = cass_ws;
			m_uart->write_rxd((m_cass_data[1] < 14) ? 1 : 0);
			m_cass_data[1] = 0;
		}
	}

	m_uart->write_rxc(state);
}

void mfabfz_state::machine_start()
{
	save_item(NAME(m_cass_data));
	save_item(NAME(m_cassoutbit));
	save_item(NAME(m_cassbit));
	save_item(NAME(m_cassold));
}

void mfabfz_state::machine_reset()
{
	m_cass_data[0] = m_cass_data[1] = m_cass_data[2] = m_cass_data[3] = m_cass_data[4] = 0;
	m_cassoutbit = m_cassold = m_cassbit = 1;
	m_uart->write_rxd(1);
	m_uart->write_cts(0);
}

void mfacpm_state::machine_start()
{
}

void mfacpm_state::machine_reset()
{
	m_bootview.select(0);
	m_uart->write_rxd(1);
	m_uart->write_cts(0);
}

void mfabfz_state::mfabfz(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, 4_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &mfabfz_state::mfabfz_mem);
	m_maincpu->set_addrmap(AS_IO, &mfabfz_state::mfabfz_io);

	// uart1 - terminal
	clock_device &uart1_clock(CLOCK(config, "uart1_clock", 4_MHz_XTAL / 26));
	uart1_clock.signal_handler().set("uart1", FUNC(i8251_device::write_txc));
	uart1_clock.signal_handler().append("uart1", FUNC(i8251_device::write_rxc));

	i8251_device &uart1(I8251(config, "uart1", 0));
	uart1.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	uart1.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	uart1.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("uart1", FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set("uart1", FUNC(i8251_device::write_dsr));
	rs232.cts_handler().set("uart1", FUNC(i8251_device::write_cts));

	// uart2 - cassette - clock comes from 2MHz through a divider consisting of 4 chips and some jumpers.
	I8251(config, m_uart, 4_MHz_XTAL / 2);
	m_uart->txd_handler().set([this] (bool state) { m_cassoutbit = state; });

	clock_device &uart_clock(CLOCK(config, "uart_clock", 4_MHz_XTAL / 52));
	uart_clock.signal_handler().set(FUNC(mfabfz_state::kansas_w));
	uart_clock.signal_handler().append(FUNC(mfabfz_state::kansas_r));

	// cassette is connected to the uart
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	SPEAKER(config, "mono").front_center();
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_4800 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_4800 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
DEVICE_INPUT_DEFAULTS_END

void mfabfz_state::mfabfz85(machine_config &config)
{
	/* basic machine hardware */
	i8085a_cpu_device &maincpu(I8085A(config, m_maincpu, 4_MHz_XTAL / 2));
	maincpu.set_addrmap(AS_PROGRAM, &mfabfz_state::mfabfz_mem);
	maincpu.set_addrmap(AS_IO, &mfabfz_state::mfabfz85_io);
	maincpu.in_sid_func().set("rs232", FUNC(rs232_port_device::rxd_r));
	maincpu.out_sod_func().set("rs232", FUNC(rs232_port_device::write_txd)).invert();

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	I8251(config, m_uart, 4_MHz_XTAL / 2);
	m_uart->txd_handler().set([this] (bool state) { m_cassoutbit = state; });

	clock_device &uart_clock(CLOCK(config, "uart_clock", 4_MHz_XTAL / 52));
	uart_clock.signal_handler().set(FUNC(mfabfz_state::kansas_w));
	uart_clock.signal_handler().append(FUNC(mfabfz_state::kansas_r));

	// cassette is connected to the uart
	CASSETTE(config, m_cass);
	m_cass->set_default_state(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED);
	SPEAKER(config, "mono").front_center();
	m_cass->add_route(ALL_OUTPUTS, "mono", 0.05);
}

void mfacpm_state::mfacpm(machine_config &config)
{
	/* basic machine hardware */
	I8085A(config, m_maincpu, 4_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &mfacpm_state::mfacpm_mem);
	m_maincpu->set_addrmap(AS_IO, &mfacpm_state::mfacpm_io);
	
	// uart1 - terminal
	clock_device &uart1_clock(CLOCK(config, "uart1_clock", 4_MHz_XTAL / 26));
	uart1_clock.signal_handler().set("uart1", FUNC(i8251_device::write_txc));
	uart1_clock.signal_handler().append("uart1", FUNC(i8251_device::write_rxc));

	i8251_device &uart1(I8251(config, "uart1", 0));
	uart1.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	uart1.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	uart1.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("uart1", FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set("uart1", FUNC(i8251_device::write_dsr));
	rs232.cts_handler().set("uart1", FUNC(i8251_device::write_cts));

	// uart2 - cassette - clock comes from 2MHz through a divider consisting of 4 chips and some jumpers.
	I8251(config, m_uart, 4_MHz_XTAL / 2);
	
	// Needs at least a floppy at 0xc0
	
	SOFTWARE_LIST(config, "floppy_list").set_original("mfacpm");

}

/* ROM definition */
ROM_START( mfabfz )
	ROM_REGION( 0x8000, "roms", 0 ) // MAT32K, 1986, works
	ROM_LOAD( "mfa_mat32k_vers.1.8-t_ic0.bin", 0x0000, 0x8000, CRC(6cba989e) SHA1(81611b6250a5319e5d28af5ce3a1e261af8315ae) )
ROM_END

ROM_START( mfabfz85 )
	ROM_REGION( 0x8000, "roms", 0 )
	ROM_SYSTEM_BIOS( 0, "32k", "MAT32K v1.8s" ) // 1982, 4800, 8N2, txd-invert
	ROMX_LOAD( "mfa_mat32k_vers.1.8-s_ic0.bin", 0x0000, 0x8000, CRC(021d7dff) SHA1(aa34b3a8bac52fc7746d35f5ffc6328734788cc2), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "8k", "MAT85 8k" ) // 1982, not working
	ROMX_LOAD( "mfa_mat_1_0000.bin", 0x0000, 0x0800, CRC(73b588ea) SHA1(2b9570fe44c3c19d6aa7c7c11ecf390fa5d48998), ROM_BIOS(1) )
	ROMX_LOAD( "mfa_mat_2_0800.bin", 0x0800, 0x0800, CRC(13f5be91) SHA1(2b9d64600679bab319a37381fc84e874c3b2a877), ROM_BIOS(1) )
	ROMX_LOAD( "mfa_mat_3_1000.bin", 0x1000, 0x0800, CRC(c9b91bb4) SHA1(ef829964f507b1f6bbcf3c557c274fe728636efe), ROM_BIOS(1) )
	ROMX_LOAD( "mfa_mat_4_1800.bin", 0x1800, 0x0800, CRC(649cd7f0) SHA1(e92f29c053234b36f22d525fe92e61bf24476f14), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "16k_set1", "MAT85+ 16k set1" ) // not working
	ROMX_LOAD( "mfa_mat85_0x0000-0x07ff.bin", 0x0000, 0x0800, CRC(73b588ea) SHA1(2b9570fe44c3c19d6aa7c7c11ecf390fa5d48998), ROM_BIOS(2) )
	ROMX_LOAD( "mfa_mat85_0x0800-0x0fff.bin", 0x0800, 0x0800, CRC(13f5be91) SHA1(2b9d64600679bab319a37381fc84e874c3b2a877), ROM_BIOS(2) )
	ROMX_LOAD( "mfa_mat85_0x1000-0x17ff.bin", 0x1000, 0x0800, CRC(c9b91bb4) SHA1(ef829964f507b1f6bbcf3c557c274fe728636efe), ROM_BIOS(2) )
	ROMX_LOAD( "mfa_mat85_0x1800-0x1fff.bin", 0x1800, 0x0800, CRC(649cd7f0) SHA1(e92f29c053234b36f22d525fe92e61bf24476f14), ROM_BIOS(2) )
	ROMX_LOAD( "mfa_mat85_0x2000-0x27ff.bin", 0x2000, 0x0800, CRC(d3592915) SHA1(68daec6c5c63692bc147b1710b9c45ca780f2c7b), ROM_BIOS(2) )
	ROMX_LOAD( "mfa_mat85_0x2800-0x2fff.bin", 0x2800, 0x0800, CRC(9a6aafa9) SHA1(af897e91cc2ce5d6e49fa88c920ad85e1f0209bf), ROM_BIOS(2) )
	ROMX_LOAD( "mfa_mat85_0x3000-0x37ff.bin", 0x3000, 0x0800, CRC(eae4e3d5) SHA1(f7112965874417bbfc4a32f31f84e1db83249ab7), ROM_BIOS(2) )
	ROMX_LOAD( "mfa_mat85_0x3800-0x3fff.bin", 0x3800, 0x0800, CRC(536db0e3) SHA1(328ccc18455f710390c29c0fd0f4b0713a4a69ae), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "16k_set2", "MAT85+ 16k set2" ) // 2400, 7N2, txd-invert
	ROMX_LOAD( "mat85_1_1of8.bin", 0x0000, 0x0800, CRC(73b588ea) SHA1(2b9570fe44c3c19d6aa7c7c11ecf390fa5d48998), ROM_BIOS(3) )
	ROMX_LOAD( "mat85_2_2of8.bin", 0x0800, 0x0800, CRC(c97acc82) SHA1(eedb27c19a2d21b5ec5bca6cafeb25584e21e500), ROM_BIOS(3) )
	ROMX_LOAD( "mat85_3_3of8.bin", 0x1000, 0x0800, CRC(c9b91bb4) SHA1(ef829964f507b1f6bbcf3c557c274fe728636efe), ROM_BIOS(3) )
	ROMX_LOAD( "mat85_4_4of8.bin", 0x1800, 0x0800, CRC(649cd7f0) SHA1(e92f29c053234b36f22d525fe92e61bf24476f14), ROM_BIOS(3) )
	ROMX_LOAD( "soft_1_5of8.bin",  0x2000, 0x0800, CRC(98d9e86e) SHA1(af78b370fe97a6017b192dadec4059256ee4f4c7), ROM_BIOS(3) )
	ROMX_LOAD( "soft_2_6of8.bin",  0x2800, 0x0800, CRC(81fc3b24) SHA1(186dbd389fd700c5af1ef7c37948e11701ec596e), ROM_BIOS(3) )
	ROMX_LOAD( "soft_3_7of8.bin",  0x3000, 0x0800, CRC(eae4e3d5) SHA1(f7112965874417bbfc4a32f31f84e1db83249ab7), ROM_BIOS(3) )
	ROMX_LOAD( "soft_4_8of8.bin",  0x3800, 0x0800, CRC(536db0e3) SHA1(328ccc18455f710390c29c0fd0f4b0713a4a69ae), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS (4, "32k_dtp", "MAT32K dtp" ) // 2400, 7N2, txd-invert
	ROMX_LOAD( "mfa_mat85_sp1_ed_kpl_dtp_terminal.bin", 0x0000, 0x8000, CRC(ed432c19) SHA1(31cbc06d276dbb201d50967f4ddba26a42560753), ROM_BIOS(4) )
ROM_END

ROM_START( mfacpm )
	ROM_REGION( 0x0800, "roms", 0 ) 
	// Original MFA CP/M boot ROM as outlined in the MFA CP/M manual
	ROM_SYSTEM_BIOS(0, "mfacpm", "Original MFA CP/M") 
	ROMX_LOAD( "boot-bios_2k_v03.20_mfa.bin", 0x0000, 0x0800, CRC(b543a61b) SHA1(36039c3351c6d039407027829abcc8b0a0d6f1a2), ROM_BIOS(0) )
	// MFA Open CP/M boot ROM by Mike Douglas https://deramp.com/downloads/mfa_computer/, integrated monitor program is displayed on start
	// The size of the ROM on Mike's site is just 0x750 as a result of a conversion from HEX and has been padded with 0x00 to the end for convenience
	ROM_SYSTEM_BIOS(1, "mfaopen", "Open MFA CP/M") 
	ROMX_LOAD( "monopencpm.bin", 0x0000, 0x0800, CRC(93d01a8b) SHA1(553473EDC15608A927007639AC4AA43338684D9B), ROM_BIOS(1) )
ROM_END

} // anonymous namespace


/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT   CLASS,        INIT        COMPANY                         FULLNAME                               FLAGS */
COMP( 1979, mfabfz,   0,      0,      mfabfz,   mfabfz, mfabfz_state, empty_init, "Berufsfoerdungszentrum Essen", "Mikrocomputer fuer Ausbildung",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1979, mfabfz85, mfabfz, 0,      mfabfz85, mfabfz, mfabfz_state, empty_init, "Berufsfoerdungszentrum Essen", "Mikrocomputer fuer Ausbildung MAT85", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1979, mfacpm,   0,      0,      mfacpm,   mfacpm, mfacpm_state, empty_init, "Berufsfoerdungszentrum Essen", "Mikrocomputer fuer Ausbildung CP/M Ausbaustufe", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
