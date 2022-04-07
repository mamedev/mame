// license:BSD-3-Clause
// copyright-holders:Vas Crabb

/*
 * Motorola M6805 Family HMOS devices.
 *
 * TODO
 *   - INT2 and miscellaneous register
 *   - A/D peripheral
 *   - SPI peripheral
 *   - multiple timer variants
 */

#include "emu.h"
#include "m68705.h"
#include "m6805defs.h"
#include "6805dasm.h"

/****************************************************************************
 * Configurable logging
 ****************************************************************************/

#define LOG_GENERAL (1U <<  0)
#define LOG_INT     (1U <<  1)
#define LOG_IOPORT  (1U <<  2)
#define LOG_TIMER   (1U <<  3)
#define LOG_EPROM   (1U <<  4)

//#define VERBOSE (LOG_GENERAL | LOG_IOPORT | LOG_TIMER | LOG_EPROM)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#define LOGINT(...)     LOGMASKED(LOG_INT,    __VA_ARGS__)
#define LOGIOPORT(...)  LOGMASKED(LOG_IOPORT, __VA_ARGS__)
#define LOGTIMER(...)   LOGMASKED(LOG_TIMER,  __VA_ARGS__)
#define LOGEPROM(...)   LOGMASKED(LOG_EPROM,  __VA_ARGS__)


namespace {

std::pair<u16, char const *> const m6805_hmos_syms[] = {
	{ 0x0000, "PORTA" }, { 0x0001, "PORTB" }, { 0x0002, "PORTC" }, { 0x0003, "PORTD" },
	{ 0x0004, "DDRA"  }, { 0x0005, "DDRB"  }, { 0x0006, "DDRC"  },
	{ 0x0008, "TDR"   }, { 0x0009, "TCR"   },
	{ 0x000a, "MISC"  },
	{ 0x000e, "ACR"   }, { 0x000f, "ARR"   } };

std::pair<u16, char const *> const m68705p_syms[] = {
	{ 0x0000, "PORTA" }, { 0x0001, "PORTB" }, { 0x0002, "PORTC" },
	{ 0x0004, "DDRA"  }, { 0x0005, "DDRB"  }, { 0x0006, "DDRC"  },
	{ 0x0008, "TDR"   }, { 0x0009, "TCR"   },
	{ 0x000b, "PCR"   },
	{ 0x0784, "MOR"   } };

std::pair<u16, char const *> const m68705r_syms[] = {
	{ 0x0000, "PORTA" }, { 0x0001, "PORTB" }, { 0x0002, "PORTC" }, { 0x0003, "PORTD" },
	{ 0x0004, "DDRA"  }, { 0x0005, "DDRB"  }, { 0x0006, "DDRC"  },
	{ 0x0008, "TDR"   }, { 0x0009, "TCR"   },
	{ 0x000a, "MISC"  },
	{ 0x000b, "PCR"   },
	{ 0x000e, "ACR"   }, { 0x000f, "ARR"   },
	{ 0x0f38, "MOR"   } };

std::pair<u16, char const *> const m68705u_syms[] = {
	{ 0x0000, "PORTA" }, { 0x0001, "PORTB" }, { 0x0002, "PORTC" }, { 0x0003, "PORTD" },
	{ 0x0004, "DDRA"  }, { 0x0005, "DDRB"  }, { 0x0006, "DDRC"  },
	{ 0x0008, "TDR"   }, { 0x0009, "TCR"   },
	{ 0x000a, "MISC"  },
	{ 0x000b, "PCR"   },
	{ 0x0f38, "MOR"   } };


ROM_START( m68705p3 )
	ROM_REGION(0x0073, "bootstrap", 0)
	ROM_LOAD("bootstrap.bin", 0x0000, 0x0073, CRC(696e1383) SHA1(45104fe1dbd683d251ed2b9411b1f4befbb5aff4))
ROM_END

ROM_START( m68705p5 )
	ROM_REGION(0x0073, "bootstrap", 0)
	ROM_LOAD("bootstrap.bin", 0x0000, 0x0073, CRC(f70a8620) SHA1(c154f78c23f10bb903a531cb19e99121d5f7c19c))
ROM_END

ROM_START( m68705r3 )
	ROM_REGION(0x0078, "bootstrap", 0)
	ROM_LOAD("bootstrap.bin", 0x0000, 0x0078, CRC(5946479b) SHA1(834ea00aef5de12dbcd6421a6e21d5ea96cfbf37))
ROM_END

ROM_START( m68705u3 )
	ROM_REGION(0x0078, "bootstrap", 0)
	ROM_LOAD("bootstrap.bin", 0x0000, 0x0078, CRC(5946479b) SHA1(834ea00aef5de12dbcd6421a6e21d5ea96cfbf37))
ROM_END

enum : u16 {
	M68705_VECTOR_BOOTSTRAP  = 0xfff6,
	M6805_VECTOR_TIMER       = 0xfff8,
	M6805_VECTOR_INT2        = 0xfff8,
	M6805_VECTOR_INT         = 0xfffa,
	M6805_VECTOR_SWI         = 0xfffc,
	M6805_VECTOR_RESET       = 0xfffe
};

constexpr u16 M6805_INT_MASK           = 0x03;

} // anonymous namespace


/****************************************************************************
 * Global variables
 ****************************************************************************/

DEFINE_DEVICE_TYPE(M68705P3, m68705p3_device, "m68705p3", "Motorola MC68705P3")
DEFINE_DEVICE_TYPE(M68705P5, m68705p5_device, "m68705p5", "Motorola MC68705P5")
DEFINE_DEVICE_TYPE(M68705R3, m68705r3_device, "m68705r3", "Motorola MC68705R3")
DEFINE_DEVICE_TYPE(M68705U3, m68705u3_device, "m68705u3", "Motorola MC68705U3")

DEFINE_DEVICE_TYPE(M6805P2, m6805p2_device, "m6805p2", "Motorola MC6805P2")
DEFINE_DEVICE_TYPE(M6805P6, m6805p6_device, "m6805p6", "Motorola MC6805P6")
DEFINE_DEVICE_TYPE(M6805R2, m6805r2_device, "m6805r2", "Motorola MC6805R2")
DEFINE_DEVICE_TYPE(M6805R3, m6805r3_device, "m6805r3", "Motorola MC6805R3")
DEFINE_DEVICE_TYPE(M6805U2, m6805u2_device, "m6805u2", "Motorola MC6805U2")
DEFINE_DEVICE_TYPE(M6805U3, m6805u3_device, "m6805u3", "Motorola MC6805U3")
DEFINE_DEVICE_TYPE(HD6805S1, hd6805s1_device, "hd6805s1", "Hitachi HD6805S1")
DEFINE_DEVICE_TYPE(HD6805U1, hd6805u1_device, "hd6805u1", "Hitachi HD6805U1")

/****************************************************************************
 * M68705 base device
 ****************************************************************************/

/*
The 68(7)05 peripheral memory map:
Common for Px, Rx, Ux parts:
0x00: Port A data (RW)
0x01: Port B data (RW)
0x02: Port C data (RW) [top 4 bits do nothing (read as 1s) on Px parts, work as expected on Rx, Ux parts]
0x03: [Port D data (Read only), only on Rx, Ux parts]
0x04: Port A DDR (Write only, reads as 0xFF)
0x05: Port B DDR (Write only, reads as 0xFF)
0x06: Port C DDR (Write only, reads as 0xFF) [top 4 bits do nothing on Px parts, work as expected on Rx, Ux parts]
0x07: Unused (reads as 0xFF?)
0x08: Timer Data Register (RW; acts as ram when timer isn't counting, otherwise decrements once per prescaler expiry)
0x09: Timer Control Register (RW; on certain mask part and when MOR bit 6 is not set, all bits are RW except bit 3 which
always reads as zero. when MOR bit 6 is set and on all mask parts except one listed in errata in the 6805 daatsheet,
the top two bits are RW, bottom 6 always read as 1 and writes do nothing; on the errata chip, bit 3 is writable and
clears the prescaler, reads as zero)
0x0A: [Miscellaneous Register, only on Rx, Sx, Ux parts]
0x0B: [Eprom parts: Programming Control Register (write only?, low 3 bits; reads as 0xFF?); Unused (reads as 0xFF?) on
Mask parts]
0x0C: Unused (reads as 0xFF?)
0x0D: Unused (reads as 0xFF?)
0x0E: [A/D control register, only on Rx, Sx parts]
0x0F: [A/D result register, only on Rx, Sx parts]
0x10-0x7f: internal ram; SP can only point to 0x60-0x7F. U2/R2 parts have an unused hole from 0x10-0x3F (reads as 0xFF?)
0x80-0xFF: Page 0 user rom
The remainder of the memory map differs here between parts, see appropriate datasheet for each part.
The four vectors are always stored in big endian form as the last 8 bytes of the address space.

Sx specific differences:
0x02: Port C data (RW) [top 6 bits do nothing (read as 1s) on Sx parts]
0x06: Port C DDR (Write only, reads as 0xFF) [top 6 bits do nothing on Sx parts]
0x0B: Timer 2 Data Register MSB
0x0C: Timer 2 Data Register LSB
0x0D: Timer 2 Control Register
0x10: SPI Data Register
0x11: SPI Control Register
0x12-0x3F: Unused (reads as 0xFF?)

Port A has internal pull-ups (about 10kΩ), and can sink 1.6mA at 0.4V
making it capable of driving CMOS or one TTL load directly.

Port B has a true high-Z state (maximum 20µA for Px parts or 10µA for
Rx/Ux parts), and can sink 10mA at 1V (LED drive) or 3.2mA at 0.4V (two
TTL loads).  It requires external pull-ups to drive CMOS.

Port C has a true high-Z state (maximum 20µA for Px parts or 10µA for
Rx/Ux parts), and can sink 1.6mA at 0.4V (one TTL load).  It requires
external pull-ups to drive CMOS.

MOR ADDRESS: Mask Option Register; does not exist on R2 and several other but not all mask parts, located at 0x784 on Px parts

Px Parts:
* 28 pins
* address space is 0x000-0x7ff
* has Ports A-C
* port C is just 4 bits
* EPROM parts have MOR at 0x784 and bootstrap ROM at 0x785-0x7f7
* mask parts have a selftest rom at similar area

Rx Parts:
* 40 pins
* address space is 0x000-0xfff with an unused hole at 0xf39-0xf7f
* R2 parts have an unused hole at 0x10-0x3f and at 0x100-0x7bF
* has A/D converter, Ports A-D
* mask parts lack programmable prescaler
* EPROM parts have MOR at 0xf38 and bootstrap ROM at 0xf80-0xff7
* mask parts have selftest ROM at 0xf38-0xff7
* selftest ROMs differ between the U2 and U3 versions

Sx Parts:
* 40 pins
* address space is 0x000-0xfff with an unused hole at 0x12-0x3f and at 0x100-0x9BF
* has A/D converter, SPI serial
* port C is just two bits
* has an extra 16-bit timer compared to Ux/Rx
* selftest rom at 0xF00-0xFF7

Ux Parts:
* 40 pins
* address space is 0x000-0xfff with an unused hole at 0xf39-0xf7f
* U2 parts have an unused hole at 0x10-0x3f and at 0x100-0x7bF
* has Ports A-D
* EPROM parts have MOR at 0xf38 and bootstrap ROM at 0xf80-0xff7
* mask parts have selftest ROM at 0xf38-0xff7
* selftest ROMs differ between the U2 and U3 versions

*/

m6805_hmos_device::m6805_hmos_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, device_type type, u32 addr_width, unsigned ram_size)
	: m6805_base_device(mconfig, tag, owner, clock, type, { addr_width > 13 ? s_hmos_b_ops : s_hmos_s_ops, s_hmos_cycles, addr_width, 0x007f, 0x0060, M6805_VECTOR_SWI }, address_map_constructor(FUNC(m6805_hmos_device::map), this))
	, m_timer(*this)
	, m_port_open_drain{ false, false, false, false }
	, m_port_mask{ 0x00, 0x00, 0x00, 0x00 }
	, m_port_input{ 0xff, 0xff, 0xff, 0xff }
	, m_port_latch{ 0xff, 0xff, 0xff, 0xff }
	, m_port_ddr{ 0x00, 0x00, 0x00, 0x00 }
	, m_port_cb_r(*this)
	, m_port_cb_w(*this)
	, m_ram_size(ram_size)
{
}

m68705_device::m68705_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, device_type type, u32 addr_width, unsigned ram_size)
	: m6805_hmos_device(mconfig, tag, owner, clock, type, addr_width, ram_size)
	, device_nvram_interface(mconfig, *this)
	, m_user_rom(*this, DEVICE_SELF)
	, m_vihtp(CLEAR_LINE)
	, m_pcr(0xff)
	, m_pl_data(0xff)
	, m_pl_addr(0xffff)
{
}

template <offs_t B> u8 m68705_device::eprom_r(offs_t offset)
{
	if (pcr_vpon() && !pcr_ple())
		LOGEPROM("read EPROM %04X prevented when Vpp high and /PLE = 0\n", B + offset);

	// read locked out when /VPON and /PLE are asserted
	return (!pcr_vpon() || !pcr_ple()) ? m_user_rom[B + offset] : 0xff;
}

template <offs_t B> void m68705_device::eprom_w(offs_t offset, u8 data)
{
	LOGEPROM("EPROM programming latch write%s%s: %04X = %02X\n",
			!pcr_vpon() ? " [Vpp low]" : "", !pcr_ple() ? " [disabled]" : "", B + offset, data);

	// programming latch enabled when /VPON and /PLE are asserted
	if (pcr_vpon() && pcr_ple())
	{
		if (!pcr_pge())
		{
			m_pl_data = data;
			m_pl_addr = B + offset;
		}
		else
		{
			// this causes undefined behaviour, which is bad when EPROM programming is involved
			logerror("warning: write to EPROM when /PGE = 0 (%04X = %02X)\n", B + offset, data);
		}
	}
}

template <std::size_t N> void m6805_hmos_device::set_port_open_drain(bool value)
{
	m_port_open_drain[N] = value;
}

template <std::size_t N> void m6805_hmos_device::set_port_mask(u8 mask)
{
	if (configured() || started())
		throw emu_fatalerror("Attempt to set physical port mask after configuration");
	m_port_mask[N] = mask;
}

template <std::size_t N> u8 m6805_hmos_device::port_r()
{
	if (!m_port_cb_r[N].isnull())
	{
		u8 const newval(m_port_cb_r[N](0, ~m_port_ddr[N] & ~m_port_mask[N]) & ~m_port_mask[N]);
		if (newval != m_port_input[N])
		{
			LOGIOPORT("read PORT%c: new input = %02X & %02X (was %02X)\n",
					char('A' + N), newval, ~m_port_ddr[N] & ~m_port_mask[N], m_port_input[N]);
		}
		m_port_input[N] = newval;
	}
	return m_port_mask[N] | (m_port_latch[N] & m_port_ddr[N]) | (m_port_input[N] & ~m_port_ddr[N]);
}

template <std::size_t N> void m6805_hmos_device::port_latch_w(u8 data)
{
	data &= ~m_port_mask[N];
	u8 const diff = m_port_latch[N] ^ data;
	if (diff)
		LOGIOPORT("write PORT%c latch: %02X & %02X (was %02X)\n", char('A' + N), data, m_port_ddr[N], m_port_latch[N]);
	m_port_latch[N] = data;
	if (diff & m_port_ddr[N])
		port_cb_w<N>();
}

template <std::size_t N> void m6805_hmos_device::port_ddr_w(u8 data)
{
	data &= ~m_port_mask[N];
	if (data != m_port_ddr[N])
	{
		LOGIOPORT("write DDR%c: %02X (was %02X)\n", char('A' + N), data, m_port_ddr[N]);
		m_port_ddr[N] = data;
		port_cb_w<N>();
	}
}

template <std::size_t N> void m6805_hmos_device::port_cb_w()
{
	u8 const data(m_port_open_drain[N] ? m_port_latch[N] | ~m_port_ddr[N] : m_port_latch[N]);
	u8 const mask(m_port_open_drain[N] ? (~m_port_latch[N] & m_port_ddr[N]) : m_port_ddr[N]);
	m_port_cb_w[N](0, data, mask);
}

u8 m68705_device::pcr_r()
{
	return m_pcr;
}

void m68705_device::pcr_w(u8 data)
{
	// 7  1
	// 6  1
	// 5  1
	// 4  1
	// 3  1
	// 2  /VPON  R   Vpp On
	// 1  /PGE   RW  Program Enable
	// 0  /PLE   RW  Programming Latch Enable

	LOGEPROM("write PCR: /PGE=%u%s /PLE=%u\n", BIT(data, 1), BIT(data, 0) ? " [inhibited]" : "", BIT(data, 0));

	// lock out /PGE if /PLE is not asserted
	data |= ((data & 0x01) << 1);

	// write EPROM if /PGE is asserted (erase requires UV so don't clear bits)
	if (!pcr_pge() && !BIT(data, 1))
	{
		LOGEPROM("write EPROM%s: %04X = %02X | %02X\n",
				pcr_vpon() ? "" : " prevented when Vpp low", m_pl_addr, m_pl_data, m_user_rom[m_pl_addr]);
		if (pcr_vpon())
			m_user_rom[m_pl_addr] |= m_pl_data;
	}

	m_pcr = (m_pcr & 0xfc) | (data & 0x03);
}

u8 m6805_hmos_device::acr_r()
{
	logerror("unsupported read ACR\n");
	return 0xff;
}

void m6805_hmos_device::acr_w(u8 data)
{
	// 7  conversion complete
	// 6
	// 5
	// 4
	// 3
	// 2  MUX
	// 1  MUX
	// 0  MUX

	// MUX=0  AN0 (PD0)
	// MUX=1  AN1 (PD1)
	// MUX=2  AN2 (PD2)
	// MUX=3  AN3 (PD3)
	// MUX=4  VRH (PD5)  $FF+0/-1
	// MUX=5  VRL (PD4)  $00+1/-0
	// MUX=6  VRH/4      $40+/-1
	// MUX=7  VRH/2      $80+/-1

	// on-board ratiometric successive approximation ADC
	// 30 machine cycle conversion time (input sampled during first 5 cycles)
	// on completion, ACR7 is set, result is placed in ARR, and new conversion starts
	// writing to ACR aborts current conversion, clears ACR7, and starts new conversion

	logerror("unsupported write ACR = %02X\n", data);
}

u8 m6805_hmos_device::arr_r()
{
	logerror("unsupported read ARR\n");
	return 0xff;
}

void m6805_hmos_device::arr_w(u8 data)
{
	logerror("unsupported write ARR = %02X\n", data);
}

void m6805_hmos_device::device_start()
{
	m6805_base_device::device_start();

	save_item(NAME(m_port_input));
	save_item(NAME(m_port_latch));
	save_item(NAME(m_port_ddr));

	// initialise digital I/O
	for (u8 &input : m_port_input) input = 0xff;
	m_port_cb_r.resolve_all();
	m_port_cb_w.resolve_all_safe();

	add_port_latch_state<0>();
	add_port_latch_state<1>();
	add_port_latch_state<2>();
	// PORTD has no latch registered since it is either input-only or nonexistent
	add_port_ddr_state<0>();
	add_port_ddr_state<1>();
	add_port_ddr_state<2>();

	m_timer.start(M6805_PS);
}

void m68705_device::device_start()
{
	m6805_hmos_device::device_start();

	save_item(NAME(m_vihtp));
	save_item(NAME(m_pcr));
	save_item(NAME(m_pl_data));
	save_item(NAME(m_pl_addr));

	// initialise timer/counter
	u8 const options(get_mask_options());
	if (options & MOR_TOPT)
	{
		m_timer.set_options(m6805_timer::TIMER_MOR);
		m_timer.set_divisor(options & MOR_PS);
		m_timer.set_source(m6805_timer::timer_source((options & (MOR_CLS | MOR_TIE)) >> 4));
	}
	else
		m_timer.set_options(m6805_timer::TIMER_PGM);

	// initialise EPROM control
	m_vihtp = CLEAR_LINE;
	m_pcr = 0xff;
	m_pl_data = 0xff;
	m_pl_addr = 0xffff;

	state_add(M68705_PCR, "PCR", m_pcr).mask(0xff);
	state_add(M68705_PLA, "PLA", m_pl_addr).mask(0xffff);
	state_add(M68705_PLD, "PLD", m_pl_data).mask(0xff);
}

void m6805_hmos_device::device_reset()
{
	m6805_base_device::device_reset();

	// reset digital I/O
	port_ddr_w<0>(0x00);
	port_ddr_w<1>(0x00);
	port_ddr_w<2>(0x00);
	port_ddr_w<3>(0x00);

	// reset timer/counter
	m_timer.reset();

	if (m_params.m_addr_width > 13)
		rm16<true>(M6805_VECTOR_RESET, m_pc);
	else
		rm16<false>(M6805_VECTOR_RESET, m_pc);
}

void m68705_device::device_reset()
{
	m6805_hmos_device::device_reset();

	// reset EPROM control
	m_pcr |= 0xfb; // b2 (/VPON) is driven by external input and hence unaffected by reset

	if (CLEAR_LINE != m_vihtp)
	{
		LOG("loading bootstrap vector\n");
		if (m_params.m_addr_width > 13)
			rm16<true>(M68705_VECTOR_BOOTSTRAP, m_pc);
		else
			rm16<false>(M68705_VECTOR_BOOTSTRAP, m_pc);
	}
	else
	{
		LOG("loading reset vector\n");
		if (m_params.m_addr_width > 13)
			rm16<true>(M6805_VECTOR_RESET, m_pc);
		else
			rm16<false>(M6805_VECTOR_RESET, m_pc);
	}
}

void m6805_hmos_device::execute_set_input(int inputnum, int state)
{
	if (m_irq_state[inputnum] != state)
	{
		m_irq_state[inputnum] = (state == ASSERT_LINE) ? ASSERT_LINE : CLEAR_LINE;

		if (state != CLEAR_LINE)
			m_pending_interrupts |= 1 << inputnum;
		else if (M6805_INT_TIMER == inputnum)
			m_pending_interrupts &= ~(1 << inputnum); // this one is level-sensitive
	}
}

void m68705_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case M68705_VPP_LINE:
		m_pcr = (m_pcr & 0xfb) | ((ASSERT_LINE == state) ? 0x00 : 0x04);
		break;
	case M68705_VIHTP_LINE:
		// TODO: this is actually the same physical pin as the timer input, so they should be tied up
		m_vihtp = (ASSERT_LINE == state) ? ASSERT_LINE : CLEAR_LINE;
		break;
	default:
		m6805_hmos_device::execute_set_input(inputnum, state);
		break;
	}
}

void m68705_device::nvram_default()
{
}

bool m68705_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(&m_user_rom[0], m_user_rom.bytes(), actual) && actual == m_user_rom.bytes();
}

bool m68705_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(&m_user_rom[0], m_user_rom.bytes(), actual) && actual == m_user_rom.bytes();
}

void m6805_hmos_device::interrupt()
{
	if (m_pending_interrupts & M6805_INT_MASK)
	{
		if ((CC & IFLAG) == 0)
		{
			if (m_params.m_addr_width > 13) {
				pushword<true>(m_pc);
				pushbyte<true>(m_x);
				pushbyte<true>(m_a);
				pushbyte<true>(m_cc);
			}
			else
			{
				pushword<false>(m_pc);
				pushbyte<false>(m_x);
				pushbyte<false>(m_a);
				pushbyte<false>(m_cc);
			}
			SEI;
			standard_irq_callback(0);

			if (BIT(m_pending_interrupts, M6805_IRQ_LINE))
			{
				LOGINT("servicing /INT interrupt\n");
				m_pending_interrupts &= ~(1 << M6805_IRQ_LINE);
				if (m_params.m_addr_width > 13)
					rm16<true>(M6805_VECTOR_INT, m_pc);
				else
					rm16<false>(M6805_VECTOR_INT, m_pc);
			}
			else if (BIT(m_pending_interrupts, M6805_INT_TIMER))
			{
				LOGINT("servicing timer/counter interrupt\n");
				if (m_params.m_addr_width > 13)
					rm16<true>(M6805_VECTOR_TIMER, m_pc);
				else
					rm16<false>(M6805_VECTOR_TIMER, m_pc);
			}
			else
			{
				throw emu_fatalerror("Unknown pending interrupt");
			}
			m_icount -= 11;
			burn_cycles(11);
		}
	}
}

void m6805_hmos_device::burn_cycles(unsigned count)
{
	m_timer.update(count);
}

template <std::size_t N> void m6805_hmos_device::add_port_latch_state()
{
	if (m_port_mask[N] != 0xff)
		state_add(M68705_LATCHA + N, util::string_format("LATCH%c", 'A' + N).c_str(), m_port_latch[N]).mask(~m_port_mask[N] & 0xff).formatstr("%02X");
}

template <std::size_t N> void m6805_hmos_device::add_port_ddr_state()
{
	if (m_port_mask[N] != 0xff)
		state_add(M68705_DDRA + N, util::string_format("DDR%c", 'A' + N).c_str(), m_port_ddr[N]).mask(~m_port_mask[N] & 0xff).formatstr("%02X");
}

void m68705_device::internal_map(address_map &map)
{
	m6805_hmos_device::internal_map(map);

	map(0x000b, 0x000b).rw(FUNC(m68705_device::pcr_r), FUNC(m68705_device::pcr_w));
}

/****************************************************************************
 * M68705Px family
 ****************************************************************************/

void m68705p_device::internal_map(address_map &map)
{
	m68705_device::internal_map(map);

	map(0x0080, 0x0784).rw(FUNC(m68705p_device::eprom_r<0x0080>), FUNC(m68705p_device::eprom_w<0x0080>)); // User EPROM
	map(0x0785, 0x07f7).rom().region("bootstrap", 0);
	map(0x07f8, 0x07ff).rw(FUNC(m68705p_device::eprom_r<0x07f8>), FUNC(m68705p_device::eprom_w<0x07f8>)); // Interrupt vectors
}

m68705p_device::m68705p_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock,  device_type type)
	: m68705_device(mconfig, tag, owner, clock, type, 11, 112)
{
	set_port_open_drain<0>(true);   // Port A is open drain with internal pull-ups
	set_port_mask<2>(0xf0);         // Port C is four bits wide
	set_port_mask<3>(0xff);         // Port D isn't present
}

void m68705p_device::device_start()
{
	m68705_device::device_start();

	state_add(M68705_MOR, "MOR", get_user_rom()[0x0784]).mask(0xff);
}

std::unique_ptr<util::disasm_interface> m68705p_device::create_disassembler()
{
	return std::make_unique<m6805_disassembler>(m68705p_syms);
}


/****************************************************************************
 * M68705Ux family
 ****************************************************************************/

void m68705u_device::internal_map(address_map &map)
{
	m68705_device::internal_map(map);

	map(0x0080, 0x0f38).rw(FUNC(m68705u_device::eprom_r<0x0080>), FUNC(m68705u_device::eprom_w<0x0080>)); // User EPROM
	// 0x0f39-0x0f7f not used
	map(0x0f80, 0x0ff7).rom().region("bootstrap", 0);
	map(0x0ff8, 0x0fff).rw(FUNC(m68705u_device::eprom_r<0x0ff8>), FUNC(m68705u_device::eprom_w<0x0ff8>)); // Interrupt vectors
}

m68705u_device::m68705u_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, device_type type)
	: m68705_device(mconfig, tag, owner, clock, type, 12, 112)
{
	set_port_open_drain<0>(true);   // Port A is open drain with internal pull-ups
}

void m68705u_device::device_start()
{
	m68705_device::device_start();

	state_add(M68705_MOR, "MOR", get_user_rom()[0x0f38]).mask(0xff);
}

std::unique_ptr<util::disasm_interface> m68705u_device::create_disassembler()
{
	return std::make_unique<m6805_disassembler>(m68705u_syms);
}


/****************************************************************************
 * M68705Rx family
 ****************************************************************************/

void m68705r_device::internal_map(address_map &map)
{
	m68705_device::internal_map(map);

	map(0x000e, 0x000e).rw(FUNC(m68705r_device::acr_r), FUNC(m68705r_device::acr_w));
	map(0x000f, 0x000f).rw(FUNC(m68705r_device::arr_r), FUNC(m68705r_device::arr_w));

	map(0x0080, 0x0f38).rw(FUNC(m68705r_device::eprom_r<0x0080>), FUNC(m68705r_device::eprom_w<0x0080>)); // User EPROM
	// 0x0f39-0x0f7f not used
	map(0x0f80, 0x0ff7).rom().region("bootstrap", 0);
	map(0x0ff8, 0x0fff).rw(FUNC(m68705r_device::eprom_r<0x0ff8>), FUNC(m68705r_device::eprom_w<0x0ff8>)); // Interrupt vectors
}

m68705r_device::m68705r_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, device_type type)
	: m68705u_device(mconfig, tag, owner, clock, type)
{
}

void m68705r_device::device_start()
{
	m68705u_device::device_start();

	// TODO: ADC
}

std::unique_ptr<util::disasm_interface> m68705r_device::create_disassembler()
{
	return std::make_unique<m6805_disassembler>(m68705r_syms);
}

/****************************************************************************
 * M68705P3 device
 ****************************************************************************/

m68705p3_device::m68705p3_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: m68705p_device(mconfig, tag, owner, clock, M68705P3)
{
}

tiny_rom_entry const *m68705p3_device::device_rom_region() const
{
	return ROM_NAME(m68705p3);
}

u8 m68705p3_device::get_mask_options() const
{
	return get_user_rom()[0x0784] & 0xf7; // no SNM bit
}


/****************************************************************************
 * M68705P5 device
 ****************************************************************************/

m68705p5_device::m68705p5_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: m68705p_device(mconfig, tag, owner, clock, M68705P5)
{
}

tiny_rom_entry const *m68705p5_device::device_rom_region() const
{
	return ROM_NAME(m68705p5);
}

u8 m68705p5_device::get_mask_options() const
{
	return get_user_rom()[0x0784];
}


/****************************************************************************
 * M68705R3 device
 ****************************************************************************/

m68705r3_device::m68705r3_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: m68705r_device(mconfig, tag, owner, clock, M68705R3)
{
}

tiny_rom_entry const *m68705r3_device::device_rom_region() const
{
	return ROM_NAME(m68705r3);
}

u8 m68705r3_device::get_mask_options() const
{
	return get_user_rom()[0x0f38] & 0xf7; // no SNM bit
}


/****************************************************************************
 * M68705U3 device
 ****************************************************************************/

m68705u3_device::m68705u3_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: m68705u_device(mconfig, tag, owner, clock, M68705U3)
{
}

tiny_rom_entry const *m68705u3_device::device_rom_region() const
{
	return ROM_NAME(m68705u3);
}

u8 m68705u3_device::get_mask_options() const
{
	return get_user_rom()[0x0f38] & 0xf7; // no SNM bit
}

m6805p2_device::m6805p2_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: m6805_mrom_device(mconfig, tag, owner, clock, M6805P2, 11, 64)
{
	/*
	 * 1983 User's Manual states that M6805P2, M6805P4 and M6805T2 don't
	 * support prescalar clear, however the 1988 databook indicates the
	 * M6805P2 does?
	 */
	//m_timer.set_options(m6805_timer::TIMER_NPC);

	set_port_mask<2>(0xf0); // Port C is four bits wide
	set_port_mask<3>(0xff); // Port D isn't present
}

m6805p6_device::m6805p6_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: m6805_mrom_device(mconfig, tag, owner, clock, M6805P6, 11, 64)
{
	set_port_mask<2>(0xf0); // Port C is four bits wide
	set_port_mask<3>(0xff); // Port D isn't present
}

m6805r2_device::m6805r2_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: m6805_mrom_device(mconfig, tag, owner, clock, M6805R2, 12, 64)
{
}

m6805r3_device::m6805r3_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: m6805_mrom_device(mconfig, tag, owner, clock, M6805R3, 12, 112)
{
	m_timer.set_options(m6805_timer::TIMER_PGM);
}

m6805u2_device::m6805u2_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: m6805_mrom_device(mconfig, tag, owner, clock, M6805U2, 12, 64)
{
}

m6805u3_device::m6805u3_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: m6805_mrom_device(mconfig, tag, owner, clock, M6805U3, 12, 112)
{
	m_timer.set_options(m6805_timer::TIMER_PGM);
}

hd6805s1_device::hd6805s1_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: m6805_mrom_device(mconfig, tag, owner, clock, HD6805S1, 11, 64)
{
	m_timer.set_options(m6805_timer::TIMER_NPC);

	set_port_mask<2>(0xf0); // Port C is four bits wide
	set_port_mask<3>(0xff); // Port D isn't present
}

hd6805u1_device::hd6805u1_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: m6805_mrom_device(mconfig, tag, owner, clock, HD6805U1, 12, 96)
{
	// Port D has optional analog comparator but no INT2 (no MISC register, either)
	m_timer.set_options(m6805_timer::TIMER_PGM | m6805_timer::TIMER_NPC);
}

void m6805_hmos_device::internal_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0x0000).rw(FUNC(m6805_hmos_device::port_r<0>), FUNC(m6805_hmos_device::port_latch_w<0>));
	map(0x0001, 0x0001).rw(FUNC(m6805_hmos_device::port_r<1>), FUNC(m6805_hmos_device::port_latch_w<1>));
	map(0x0002, 0x0002).rw(FUNC(m6805_hmos_device::port_r<2>), FUNC(m6805_hmos_device::port_latch_w<2>));

	map(0x0004, 0x0004).w(FUNC(m6805_hmos_device::port_ddr_w<0>));
	map(0x0005, 0x0005).w(FUNC(m6805_hmos_device::port_ddr_w<1>));
	map(0x0006, 0x0006).w(FUNC(m6805_hmos_device::port_ddr_w<2>));

	map(0x0008, 0x0008).lrw8(NAME([this]() { return m_timer.tdr_r(); }), NAME([this](u8 data) { m_timer.tdr_w(data); }));
	map(0x0009, 0x0009).lrw8(NAME([this]() { return m_timer.tcr_r(); }), NAME([this](u8 data) { m_timer.tcr_w(data); }));

	// M68?05Px devices don't have Port D or the Miscellaneous register
	if (m_port_mask[3] != 0xff)
	{
		map(0x0003, 0x0003).rw(FUNC(m6805_hmos_device::port_r<3>), FUNC(m6805_hmos_device::port_latch_w<3>));
		map(0x000a, 0x000a).rw(FUNC(m6805_hmos_device::misc_r), FUNC(m6805_hmos_device::misc_w));
	}

	map(0x80 - m_ram_size, 0x007f).ram();
}

void m6805_mrom_device::internal_map(address_map &map)
{
	m6805_hmos_device::internal_map(map);

	/*
	 * Mask ROM devices have address ranges defined as follows:
	 *
	 *   Device  Reg/RAM    Page Zero  Main User  Self Check  Vectors
	 *   6805P2  0000-007f  0080-00ff  03c0-07b3  07b4-07f7   07f8-07ff
	 *   6805P4  0000-007f  0080-00ff  03c0-0783  0784-07f7   07f8-07ff
	 *   6805P6  0000-007f  0080-00ff  0100-07b3  07b4-07f7   07f8-07ff
	 *   6805R2  0000-007f  0080-00ff  07c0-0f37  0f38-0ff7   0ff8-0fff
	 *   6805R3  0000-007f             0080-0f37  0f38-0ff7   0ff8-0fff
	 *   6805S2  0000-007f  0080-00ff  09c0-0eff  0f00-0ff7   0ff8-0fff
	 *   6805S3  0000-007f  0080-00ff  0100-0eff  0f00-0ff7   0ff8-0fff
	 *   6805T2  0000-007f  0080-07ff  0d40-0f83  0f84-0ff7   0ff8-0fff
	 *   6805U2  0000-007f  0080-00ff  07c0-0f37  0f38-0ff7   0ff8-0fff
	 *   6805U3  0000-007f             0080-0f37  0f38-0ff7   0ff8-0fff
	 *
	 * Hitachi NMOS variants:
	 *
	 *   Device  Reg/RAM    Page Zero  Main User  Self Check  Vectors
	 *   6805S1  0000-007f  0080-00ff  03c0-0783  0784-07f7   07f8-07ff
	 *   6805S6  0000-007f  0080-00ff  0100-0783  0784-07f7   07f8-07ff
	 *   6805U1  0000-007f  0080-00ff  0800-0f7f  0f80-0ff7   0ff8-0fff
	 *   6805V1  0000-007f  0080-00ff  0100-0f7f  0f80-0ff7   0ff8-0fff
	 *   6805W1  0000-007f  0080-00ff  0100-0f79  0f7a-0ff1   0ff2-0fff
	 *
	 * This code assumes that dumps are captured contiguously from address 0 to
	 * the end of the address range, and are not split by range. Register, RAM
	 * and unused areas of the dump may contain invalid data.
	 */
	offs_t const end = (1U << space_config(map.m_spacenum)->addr_width()) - 1;

	map(0x0080, end).rom().region(tag(), 0x80);
}

void m6805r2_device::internal_map(address_map &map)
{
	m6805_hmos_device::internal_map(map);

	map(0x000e, 0x000e).rw(FUNC(m6805r2_device::acr_r), FUNC(m6805r2_device::acr_w));
	map(0x000f, 0x000f).rw(FUNC(m6805r2_device::arr_r), FUNC(m6805r2_device::arr_w));
}

void m6805r3_device::internal_map(address_map &map)
{
	m6805_hmos_device::internal_map(map);

	map(0x000e, 0x000e).rw(FUNC(m6805r3_device::acr_r), FUNC(m6805r3_device::acr_w));
	map(0x000f, 0x000f).rw(FUNC(m6805r3_device::arr_r), FUNC(m6805r3_device::arr_w));
}

std::unique_ptr<util::disasm_interface> m6805_hmos_device::create_disassembler()
{
	return std::make_unique<m6805_disassembler>(m6805_hmos_syms);
}

void m6805_timer::tcr_w(u8 data)
{
	if (VERBOSE & LOG_TIMER)
		m_parent.logerror("tcr_w 0x%02x\n", data);

	if (m_options & TIMER_MOR)
		data |= TCR_TIE;

	if (m_options & TIMER_PGM)
	{
		set_divisor(data & TCR_PS);
		set_source(timer_source((data & (TCR_TIN | TCR_TIE)) >> 4));
	}

	if ((data & TCR_PSC) && !(m_options & TIMER_NPC))
		m_prescale = 0;

	m_tcr = (m_tcr & (data & TCR_TIR)) | (data & ~(TCR_TIR | TCR_PSC));

	// this is a level-sensitive interrupt
	m_parent.set_input_line(M6805_INT_TIMER, (m_tcr & TCR_TIR) && !(m_tcr & TCR_TIM));
}

void m6805_timer::update(unsigned count)
{
	if (m_source == DISABLED || (m_source == CLOCK_TIMER && !m_timer))
		return;

	// compute new prescaler value and counter decrements
	unsigned const prescale = (m_prescale & ((1 << m_divisor) - 1)) + ((m_source == TIMER) ? m_timer_edges : count);
	unsigned const decrements(prescale >> m_divisor);

	// check for zero crossing
	bool const interrupt = (m_tdr ? unsigned(m_tdr) : 256U) <= decrements;

	// update state
	m_prescale = prescale & 0x7f;
	m_tdr -= decrements;
	m_timer_edges = 0;

	if (interrupt)
	{
		if (VERBOSE & LOG_TIMER)
			m_parent.logerror("timer interrupt\n");

		m_tcr |= TCR_TIR;

		if (!(m_tcr & TCR_TIM))
			m_parent.set_input_line(M6805_INT_TIMER, ASSERT_LINE);
	}
}

void m6805_timer::timer_w(int state)
{
	// count falling edges
	if (m_timer && !state)
		m_timer_edges++;

	m_timer = bool(state);
}
