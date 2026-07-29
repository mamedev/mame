// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 Speech Plus Prose 4001 / Calltext 5010 speech synthesizer
  The same PCB is used for both, but with a clear 'Prose // 4001'
  or 'Calltext // 5010' sticker just below the write-in spot for
  the part number.
  Prototype earlier cards have "CallText 5000" silkscreened above
  the ISA card connector instead.

  Prose 4001 is PCB ASSY S36001-F
  Calltext 5010 is PCB ASSY S96006-N

 Full-length ISA card.  Supports operation in IBM PC mode (as an ISA bus
 card) and standalone mode (controlled over a serial connection).  The
 example the ROMs were dumped from had many components unpopulated.

 Major components include:
 * Siemens 80188-N CPU
 * NEC D77P20 DSP with EPROM memory
 * Three TI 27C512 EPROMS
 * 256 kbit SRAM
 * Intel P8251A UART
 * Three banks of eight DIP switches

 The Speech Plus Calltext 5010 is the same PCB, but with the DTMF generator,
  DTMF receiver, the external serial port, level shifters and passives
  populated.

LED error codes:

0x40 - rom checksum failure
0x28 - rom checksum passed, resetting dsp?
0x20 - rom checksum passed, dsp running?

 BTANB/original program "bugs": there is an opcode at DSP word offset 0x0db:
  10000f: "op mov @MEM,NONE" or "nop  m,a | mov none,mem" depending on the assembler
  this moves the a nonexistent/undocumented src register in the 7720 to the memory value
  and likely moves a value of all zeroes.
  this opcode is valid for the upd7725:
  20000f: "op mov @MEM,TRB" or "nop m,a | mov trb,mem"
  where it moves the TRB register, which was added in the upd7725, instead of zeroes.
  This opcode is only hit if the CPU->DSP packet buffer underruns mid-packet?
  See tracing work by pawel wozniak

 */
#include "emu.h"
#include "prose4k1.h"

#include "cpu/i86/i186.h"
#include "cpu/upd7725/upd7720.h"
#include "machine/i8251.h"
#include "machine/input_merger.h"
#include "sound/dac.h"
#include "speaker.h"

// defines

#define LOG_PARAM     (1U << 1)
#define LOG_DSP       (1U << 2)

#define VERBOSE (LOG_GENERAL)
//define VERBOSE (LOG_GENERAL | LOG_PARAM | LOG_DSP)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

namespace {

#define LOGPRM(...) LOGMASKED(LOG_PARAM, __VA_ARGS__)
#define LOGDSP(...) LOGMASKED(LOG_DSP, __VA_ARGS__)

class prose4k1_device : public device_t, public device_isa8_card_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::SOUND; }

	prose4k1_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock) :
		device_t(mconfig, ISA8_PROSE4001, tag, owner, clock),
		device_isa8_card_interface(mconfig, *this),
		m_maincpu(*this, "maincpu"),
		m_dsp(*this, "dsp"),
		m_uart(*this, "i8251"),
		m_txrdy_empty_int(*this, "txrdy_empty_int"),
		m_dac(*this, "dac")
	{
	}

protected:
	// device_t implementation
	virtual tiny_rom_entry const *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<i80188_cpu_device> m_maincpu;
	required_device<upd7720_device> m_dsp;
	required_device<i8251_device> m_uart;
	required_device<input_merger_device> m_txrdy_empty_int;
	required_device<dac_word_interface> m_dac;
	void main_map(address_map &map) ATTR_COLD;
	void dsp_prg_map(address_map &map) ATTR_COLD;
	void dsp_data_map(address_map &map) ATTR_COLD;

	void i8251_txrx_clock_w(int state);
	void peripheral_w(uint8_t data);
	uint8_t dsp_data_r();
	void dsp_data_w(uint8_t data);
	uint8_t dsp_status_r();
	void dsp_so16_cb(uint16_t data);
	void dsp_int_w(uint8_t state);

	//uint8_t m_dac_shifter = 0;
	uint16_t m_dac_latch = 0;
	uint8_t m_paramReg = 0;
};


ROM_START(prose4k1)
	ROM_REGION(0x3'0000, "u8", 0)
	// U4 socket is empty, would map at c0000-cffff
	ROM_LOAD("v3.4.1_pr4001.u3", 0x0'0000, 0x1'0000, CRC(12dac3ed) SHA1(cf8c0b9de1f00facbc5cb5dc8e2dcbb09d6ff479)) // TMS27C512, printed label, maps at d0000-dffff
	ROM_LOAD("v3.4.1_pr4001.u2", 0x1'0000, 0x1'0000, CRC(2ee241b7) SHA1(35b81f3b4deb552511f8d8f2d0aac9100fdee49d)) // TMS27C512, printed label, maps at e0000-effff
	ROM_LOAD("v3.4.1_pr4001.u1", 0x2'0000, 0x1'0000, CRC(559f4950) SHA1(5c8709c82dadaea7012859c20141ef8f59d5e473)) // TMS27C512, handwritten label, maps at f0000-fffff

	ROM_REGION32_LE( 0x800, "dsp:prg", 0) // packed 24 bit data
	//ROM_LOAD("v3.12__5-04-90.prg.u16.old", 0x0000, 0x0600, CRC(9e46425a) SHA1(80a915d731f5b6863aeeb448261149ff15e5b786)) // identical to the prose2k 3.12 8/9/88 dsp rom
	ROM_LOAD( "v3.12__5-04-90.prg.u16", 0x0000, 0x0800, CRC(6511df1e) SHA1(d898912bf6f630205340f0f5c17a8d88cf154787)) // identical to the prose2k 3.12 8/9/88 dsp rom

	ROM_REGION16_LE(0x0400, "dsp:dat", 0) // 512*13-bit words, left-justified
	ROM_LOAD("v3.12__5-04-90.dat.u16", 0x0000, 0x0400, CRC(95e4d57a) SHA1(f6f6d9073677515fcb5b7e47244f05c6a6c874d0)) // identical to the prose2k 3.12 8/9/88 dsp rom

	// An older version, v3.1.1 or "V3.11" is also known to exist, but is not dumped. This version only has two eproms, at u1 and u2.
	// (it may have a third eprom at u3 with leftover ibm pc rom-bios code in it accidentally in the socket but is unused)
	// this older version also uses the same dsp rom, but labeled v3.12 8/9/88 as on the prose2k
ROM_END


tiny_rom_entry const *prose4k1_device::device_rom_region() const
{
	return ROM_NAME(prose4k1);
}

void prose4k1_device::i8251_txrx_clock_w(int state)
{
	m_uart->write_txc(state);
	m_uart->write_rxc(state);
}

void prose4k1_device::peripheral_w(uint8_t data)
{
	// This controls the four LEDS, and the RESET line for the upd77p20.
	// This is similar to prose2k, but the bit order is mostly reversed.
	m_paramReg = data;
	m_dsp->set_input_line(INPUT_LINE_RESET, BIT(data,1)?CLEAR_LINE:ASSERT_LINE);
	LOGPRM("80188: Parameter Reg written: UNK7: %d, LED6: %d; LED5: %d; LED4: %d; LED3: %d; UNK2: %d, DSPRST1: %d, UNK0: %d\n", BIT(data,7), BIT(data,6), BIT(data,5), BIT(data,4), BIT(data,3), BIT(data,2), BIT(data,1), BIT(data,0));
}

uint8_t prose4k1_device::dsp_data_r()
{
	uint8_t r = m_dsp->data_r();
	LOGDSP("dsp data read: %02x\n", r);
	return r;
}

void prose4k1_device::dsp_data_w(uint8_t data)
{
	LOGDSP("dsp data write: %02x\n", data);
	m_dsp->data_w(data);
}

uint8_t prose4k1_device::dsp_status_r()
{
	uint8_t r = m_dsp->status_r();
	LOGDSP("dsp status read: %02x\n", r);
	return r;
}

void prose4k1_device::dsp_so16_cb(uint16_t data)
{
	m_dac_latch = data;
}

void prose4k1_device::dsp_int_w(uint8_t state)
{
	// writing the DSP INT pin at 10khz also latches the state of the external serial shifter to the DAC inputs on the rising edge
	if (state)
	{
		m_dac->write((m_dac_latch&0x1ffe)>>1); // DSP_SO taps bits 12 to 1; bits 15, 14, 13 and 0 are unconnected. bit 0 is used, just unconnected.
	}
	m_dsp->set_input_line(UPD7720_INPUT_LINE_INT, state);
}

void prose4k1_device::device_add_mconfig(machine_config &config)
{
	I80188(config, m_maincpu, 16_MHz_XTAL).set_addrmap(AS_PROGRAM, &prose4k1_device::main_map); // U8
	//m_maincpu->tmrout0_handler().set_inputline(m_dsp, UPD7720_INPUT_LINE_INT);
	m_maincpu->tmrout0_handler().set(FUNC(prose4k1_device::dsp_int_w));
	m_maincpu->tmrout1_handler().set(FUNC(prose4k1_device::i8251_txrx_clock_w));

	UPD7720(config, m_dsp, 16_MHz_XTAL / 2); // U16; UPD77P20
	m_dsp->set_addrmap(AS_PROGRAM, &prose4k1_device::dsp_prg_map);
	m_dsp->set_addrmap(AS_DATA, &prose4k1_device::dsp_data_map);
	m_dsp->p0().set(m_maincpu, FUNC(i80188_cpu_device::int0_w)).invert();
	// UPD7720 /SCK is UART_CLK which is as below 2MHz
	m_dsp->so16().set(FUNC(prose4k1_device::dsp_so16_cb));

	//config.set_maximum_quantum(attotime::from_hz(4194304));

	I8251(config, m_uart, (16_MHz_XTAL / 2) / 4); // U27 clocked by maincpu XTAL, divided by 2 (xin->CLKOUT, 80188 clkout is /2), to a 74HCT163 dividing it by 4, for 2MHz
	// when jumpered for standalone, RXINT is 80188 int1, TXINT || TXEMPTY is 80188 int2
	// when jumpered as a card, the interrupts connect to the host pc
	// note as shipped on the prose4k1 this UART is entirely useless and isn't
	// used by the 80188 software at all
	m_uart->rxrdy_handler().set(m_maincpu, FUNC(i80188_cpu_device::int1_w));
	m_uart->txrdy_handler().set(m_txrdy_empty_int, FUNC(input_merger_device::in_w<0>));
	m_uart->txempty_handler().set(m_txrdy_empty_int, FUNC(input_merger_device::in_w<1>));

	INPUT_MERGER_ANY_HIGH(config, m_txrdy_empty_int).output_handler().set(m_maincpu, FUNC(i80188_cpu_device::int2_w));

	SPEAKER(config, "speaker").front_center();
	AM6012(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 1.0); // AM6012 12-bit DAC
}


void prose4k1_device::device_start()
{
	save_item(NAME(m_dac_latch));
	save_item(NAME(m_paramReg));
	m_paramReg = 0x00; // on power up, all leds on, reset to upd7720 is high
}

/* 80186/8 peripheral regs:
a0 UMCS: F03C - 1111 0000 0011 1100 - address: f0000-fffff - purpose: rom area, 64k block, no waitstates, no RDY
a2 LMCS: 03F8 - 0000 0011 1111 1000 - address: 00000-03fff - purpose: ram area 1, no waitstates, external RDY
a4 PACS: 033A - 0000 0011 0011 1010 - peripheral base is 0000 0011 0000 0000 0000 = 03000, 2 waitstates, external RDY (for /PCS0-3)
a6 MMCS: C1FC - 1100 0001 1111 1100 - base address is 1100 000x xxxx xxxx xxxx i.e. 0xc0000, no waitstates, no RDY
a8 MPCS: A0FA - 1010 0000 1111 1010 - 256k block size, 64k select size (i.e. on a 0x10000 boundary), EX=1, MS=1, 2 waitstates, external RDY (for /PCS4-6)
This implies a memory mapped /cs for c0000, d0000, e0000, f0000 for the four /MCS pins, and the /PCS pins are memory-mapped at 03000, 03080, 03100, 03180, 03200, 03280, 03300
This also implies that for 3000-3fff the /LMCS and /PCS pin activations WILL OVERLAP, which means that there must be external circuitry to prevent bus contention!
*/
void prose4k1_device::main_map(address_map &map)
{
	map(0x0'0000, 0x0'2fff).ram(); // /LCS, M5M5256BP 32K SRAM maps at 0x0000-0x2fff only (3/8ths of the sram!)...
	// the next 0x1000 of sram is 'overlaid' by the peripheral selects:
	// 0x3000-0x307f /PCS0 DSP data and status reg, connected to D0-D7 and A1
	map(0x0'3000, 0x0'3000).mirror(0x007c).rw(FUNC(prose4k1_device::dsp_data_r), FUNC(prose4k1_device::dsp_data_w));
	map(0x0'3002, 0x0'3002).mirror(0x007c).r(FUNC(prose4k1_device::dsp_status_r)); // 0x3000-0x307f /PCS0 DSP status reg
	// 0x3080-0x30ff /PCS1 - this area is different depending on whether the device is in standalone or pc attached mode.
	map(0x0'3088, 0x0'3089)/*.mirror(0x0062)*/.rw(m_uart, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x0'3090, 0x0'3090)/*.mirror(0x006f)*/.portr("SW1"); //dipswitches when A4 high
	// 0x3100-0x317f /PCS2 N/C unused.
	// 0x3180-0x31ff /PCS3 DSP reset and status LEDs
	map(0x0'3180, 0x0'3180)/*.mirror(0x007f)*/.w(FUNC(prose4k1_device::peripheral_w));
	// 0x3200-0x327f /PCS4 -> 1B write to latch u13 (dsp->pc semaphore)
	// 0x3280-0x32ff /PCS5 -> C0 data written to registered pal using the high 4 bits as data bits and the U17-2 'latch bit'
	// 0x3300-0x337f /PCS6 -> 80 data written to registered pal using the high 4 bits as data bits and the U17-3 'latch bit'
	// 0x3380-0x33ff open bus
	map(0xd'0000, 0xf'ffff).rom().region("u8", 0x0'0000); // rom extends from c0000-fffff in four /MCS chunks as well as the /UCS chunk redundantly for the f0000-fffff area. however the f0000-fffff section can be theoretically overridden by somehow banking in a different rom or ram, but none is populated
}

void prose4k1_device::dsp_prg_map(address_map &map)
{
	map(0x0000, 0x01ff).rom().region("dsp:prg", 0);
}

void prose4k1_device::dsp_data_map(address_map &map)
{
	map(0x0000, 0x01ff).rom().region("dsp:dat", 0);
}

static INPUT_PORTS_START( prose4k1 )
PORT_START("SW1")
	PORT_DIPNAME( 0x07, 0x01, "Baud Rate/Test? mode") PORT_DIPLOCATION("SW1:1,2,3") // note this has no effect on the ISA 'latch interface' version romset except setting the baud clock to the (unused) UART.
	PORT_DIPSETTING( 0x07, "Test? Mode" )
	PORT_DIPSETTING( 0x03, "300" )
	PORT_DIPSETTING( 0x05, "600" )
	PORT_DIPSETTING( 0x01, "1200" )
	PORT_DIPSETTING( 0x06, "2400" )
	PORT_DIPSETTING( 0x02, "4800" )
	PORT_DIPSETTING( 0x04, "9600" )
	PORT_DIPSETTING( 0x00, "19200" )
	PORT_DIPNAME( 0x08, 0x00, "S1-4: Unknown") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPNAME( 0x10, 0x00, "S1-5: Unknown") PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x00, "S1-6: Unknown") PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPNAME( 0x40, 0x00, "S1-7: Self Test") PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPNAME( 0x80, 0x00, "S1-8: Unknown") PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
INPUT_PORTS_END

ioport_constructor prose4k1_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(prose4k1);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ISA8_PROSE4001, device_isa8_card_interface, prose4k1_device, "isa_prose4001", "Speech Plus Prose 4001 (IBM PC mode)")
