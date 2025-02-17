// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*
    Motorola M68HC05PGE semi-custom "PG&E" 68HC05 Power Management Unit for Apple
    Emulation by R. Belmont

    Named for the electric utility in Cupertino, Pacific Gas & Electric.
    Early versions of this chip had screened artwork of a power station with
    lightning bolts, see the PCB photo at:
    https://www.dobreprogramy.pl/@macminik/powerbook-duo-best-of-both-worlds,blog,39291

    This is a 160-pin part with a lot of feature blocks.  Most of the components appear
    in other 68HC05 variants so I have included references to HC05 parts with similar
    or identical versions of that functionality.

    - 512 bytes of internal ROM, which can be banked out
    - Eleven 8-bit GPIO ports A, B, C, D, E, F, G, H, J, K, and L
    - Four PWM (pulse width modulation) analog outputs (68HC05F32)
    - One PLM (pulse length modulation) analog output (68HC05B4)
    - Four ADC inputs (68HC05B4)
    - An SPI interface (68HC05F32)
    - A hardware 10x8 keyboard matrix scanner
    - A 2-axis hardware quadrature decoder for a mouse or trackball
    - Fixed-interval one second and 5.86 millisecond timers
    - A custom RTC that counts a uint32 number of seconds in the classic Mac/IIgs format
    - A custom Apple Desktop Bus modem
*/

#include "emu.h"
#include "m68hc05pge.h"
#include "m6805defs.h"
#include "6805dasm.h"

#define LOG_ADB         (1U << 1)
#define LOG_IRQ         (1U << 2)
#define LOG_PWM         (1U << 3)
#define LOG_PLM         (1U << 4)
#define LOG_SPI         (1U << 5)
#define LOG_SPI_VERBOSE (1U << 6)
#define LOG_ADC         (1U << 7)

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(M68HC05PGE, m68hc05pge_device, "m68hc05pge", "Motorola M68HC05PGE")

static constexpr int M68HC05PGE_INT_IRQ = M6805_IRQ_LINE;               // external IRQ line
static constexpr int M68HC05PGE_INT_ADB = M68HC05PGE_INT_IRQ + 1;       // ADB interrupt
static constexpr int M68HC05PGE_INT_RTI = M68HC05PGE_INT_IRQ + 2;       // real-time (5.86 ms) interrupt
static constexpr int M68HC05PGE_INT_CPI = M68HC05PGE_INT_IRQ + 3;       // one second interrupt
static constexpr int M68HC05PGE_INT_SPI = M68HC05PGE_INT_IRQ + 4;       // SPI interrupt
static constexpr int M68HC05PGE_INT_KEY = M68HC05PGE_INT_IRQ + 5;       // keyboard scanner interrupt

static constexpr u8  CSCR_SRAM_CS       = 5;                            // chip select for $8000-$FFFF SRAM
static constexpr u8  CSCR_RESET         = 0;

static constexpr u8  OPTION_INTROM      = 7;                            // internal ROM at fe00, else external address bus
static constexpr u8  OPTION_EXTBUS      = 6;                            // tri-state external address bus, else ext. bus enabled
static constexpr u8  OPTION_IRQSENSE    = 1;                            // IRQ is edge-triggered, else level triggered
static constexpr u8  OPTION_RESET       = (1<<OPTION_INTROM);

static constexpr u8  SPCR_IRQ_ENABLE    = 7;                            // IRQ enabled
static constexpr u8  SPCR_ENABLE        = 6;                            // SPI mode enabled
static constexpr u8  SPCR_MASTER        = 4;                            // SPI master mode if set, slave otherwise
static constexpr u8  SPCR_POLARITY      = 3;                            // SPI clock polarity: 0 = clock starts low, 1 = clock starts high
static constexpr u8  SPCR_PHASE         = 2;                            // SPI phase: 0 = sampled on the rising edge, 1 = sampled on the falling edge

static constexpr u8  SPSR_IRQ_FLAG      = 7;                            // SPI interrupt flag: 1 = interrupt would occur if IRQ_ENABLE in SPCR is set

static constexpr u8  CPICSR_586_IRQ_FLAG        = 7;                    // 5.86ms interrupt flag
static constexpr u8  CPICSR_ONESEC_IRQ_FLAG     = 6;                    // 1 second interrupt flag
static constexpr u8  CPICSR_586_IRQ_ENABLE      = 5;                    // 5.86ms interrupt enable
static constexpr u8  CPICSR_ONESEC_IRQ_ENABLE   = 4;                    // 1 second interrupt enable

static constexpr u8  ADCSR_CONV_COMPLETE        = 7;                    // ADC conversion complete
static constexpr u8  ADCSR_START_CONV           = 5;                    // ADC start conversion
static constexpr u8  ADCSR_CHANNEL_MASK         = 0x0f;                 // ADC channel mask for ADCSR

static constexpr u8  ADBXR_TDRE                 = 7;                    // ADB transmitter empty
static constexpr u8  ADBXR_TC                   = 6;                    // ADB transmit complete
static constexpr u8  ADBXR_SRQ                  = 5;                    // ADB got a Service ReQuest
static constexpr u8  ADBXR_RDRF                 = 3;                    // ADB receiver full
static constexpr u8  ADBXR_BRST                 = 0;                    // ADB send reset
static constexpr u8  ADBXR_IRQS ((1 << ADBXR_TDRE) | (1 << ADBXR_TC) | (1 << ADBXR_SRQ) | (1 << ADBXR_RDRF));

static constexpr int s_spi_divisors[4] = {2, 4, 16, 32};

ROM_START( m68hc05pge )
	ROM_REGION(0x200, "pge", 0)
	ROM_LOAD( "pge_boot.bin", 0x000000, 0x000200, CRC(62d4dfed) SHA1(79dc721651bf47aec53f57885779c84c4781761d) )
ROM_END

m68hc05pge_device::m68hc05pge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, int addrbits, address_map_constructor internal_map) :
	m6805_base_device(mconfig, tag, owner, clock, type, {s_hc_b_ops, s_hc_cycles, 16, 0x00ff, 0x0040, 0xfffc}),
	device_nvram_interface(mconfig, *this),
	m_program_config("program", ENDIANNESS_BIG, 8, addrbits, 0, internal_map),
	m_internal_ram(*this, "internal_ram"),
	m_introm(*this, "bankfe00"),
	m_read_tbX(*this, 0),
	m_read_tbY(*this, 0),
	m_read_tbB(*this, 0),
	m_read_p(*this, 0),
	m_write_p(*this),
	m_ad_in(*this, 0),
	m_pwm_out(*this),
	write_spi_mosi(*this),
	write_spi_clock(*this),
	m_pll_ctrl(0), m_timer_ctrl(0), m_onesec(0),
	m_option(OPTION_RESET),
	m_cscr(CSCR_RESET),
	m_spi_in(0), m_spi_out(0),
	m_spi_bit(0), m_spi_clock(0), m_spi_miso(0),
	m_spcr(0), m_spsr(0),
	m_cpicsr(0),
	m_adcsr(0),
	m_adbcr(0), m_adbsr(1 << ADBXR_TDRE), m_adbdr(0),
	m_tbcs(0),
	m_pwmacr(0), m_pwma0(0), m_pwma1(0),
	m_pwmbcr(0), m_pwmb0(0), m_pwmb1(0),
	m_plmcr(0), m_plmt1(0), m_plmt2(0)
{
	std::fill(std::begin(m_pullups), std::end(m_pullups), 0);
}

m68hc05pge_device::m68hc05pge_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	m68hc05pge_device(mconfig, M68HC05PGE, tag, owner, clock, 16, address_map_constructor(FUNC(m68hc05pge_device::m68hc05pge_map), this))
{
}

void m68hc05pge_device::device_start()
{
	m6805_base_device::device_start();

	save_item(NAME(m_ports));
	save_item(NAME(m_ddrs));
	save_item(NAME(m_pll_ctrl));
	save_item(NAME(m_timer_ctrl));
	save_item(NAME(m_onesec));
	save_item(NAME(m_option));
	save_item(NAME(m_cscr));
	save_item(NAME(m_spi_in));
	save_item(NAME(m_spi_out));
	save_item(NAME(m_spi_bit));
	save_item(NAME(m_spi_clock));
	save_item(NAME(m_spi_miso));
	save_item(NAME(m_spcr));
	save_item(NAME(m_spsr));
	save_item(NAME(m_cpicsr));
	save_item(NAME(m_adcsr));
	save_item(NAME(m_adbcr));
	save_item(NAME(m_adbsr));
	save_item(NAME(m_adbdr));
	save_item(NAME(m_tbcs));
	save_item(NAME(m_pwmacr));
	save_item(NAME(m_pwma0));
	save_item(NAME(m_pwma1));
	save_item(NAME(m_pwmbcr));
	save_item(NAME(m_pwmb0));
	save_item(NAME(m_pwmb1));
	save_item(NAME(m_plmcr));
	save_item(NAME(m_plmt1));
	save_item(NAME(m_plmt2));

	m_seconds_timer = timer_alloc(FUNC(m68hc05pge_device::seconds_tick), this);
	m_cpi_timer = timer_alloc(FUNC(m68hc05pge_device::cpi_tick), this);
	m_spi_timer = timer_alloc(FUNC(m68hc05pge_device::spi_tick), this);
	m_adb_timer = timer_alloc(FUNC(m68hc05pge_device::adb_tick), this);

	system_time systime;
	struct tm cur_time, macref;
	machine().current_datetime(systime);

	cur_time.tm_sec = systime.local_time.second;
	cur_time.tm_min = systime.local_time.minute;
	cur_time.tm_hour = systime.local_time.hour;
	cur_time.tm_mday = systime.local_time.mday;
	cur_time.tm_mon = systime.local_time.month;
	cur_time.tm_year = systime.local_time.year - 1900;
	cur_time.tm_isdst = 0;

	macref.tm_sec = 0;
	macref.tm_min = 0;
	macref.tm_hour = 0;
	macref.tm_mday = 1;
	macref.tm_mon = 0;
	macref.tm_year = 4;
	macref.tm_isdst = 0;
	const u32 ref = (u32)mktime(&macref);
	m_rtc = (u32)((u32)mktime(&cur_time) - ref);
}

void m68hc05pge_device::device_reset()
{
	option_w(OPTION_RESET);
	cscr_w(CSCR_RESET);

	m6805_base_device::device_reset();

	// all ports reset to input on startup
	memset(m_ports, 0, sizeof(m_ports));
	memset(m_ddrs, 0, sizeof(m_ddrs));

	// on reset the transmitter is empty
	m_adbsr = (1 << ADBXR_TDRE);

	// start the 1 second timer
	m_seconds_timer->adjust(attotime::from_hz(1), 0, attotime::from_hz(1));
	// and the 5.86ms timer (5.86ms = 5860 uSec)
	m_cpi_timer->adjust(attotime::from_usec(5860), 0, attotime::from_usec(5860));
}

device_memory_interface::space_config_vector m68hc05pge_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

void m68hc05pge_device::interrupt_vector()
{
	for (int irq = M68HC05PGE_INT_IRQ; irq <= M68HC05PGE_INT_KEY; irq++)
	{
		if (BIT(m_pending_interrupts, irq))
		{
			LOGMASKED(LOG_IRQ, "Taking IRQ %d vector %04x\n", irq, 0xfffa - (irq << 1));
			m_pending_interrupts &= ~(1 << irq);
			rm16<true>(0xfffa - (irq << 1), m_pc);
			return;
		}
	}
}

u64 m68hc05pge_device::execute_clocks_to_cycles(u64 clocks) const noexcept
{
	return (clocks + 1) / 2;
}

u64 m68hc05pge_device::execute_cycles_to_clocks(u64 cycles) const noexcept
{
	return cycles * 2;
}

std::unique_ptr<util::disasm_interface> m68hc05pge_device::create_disassembler()
{
	return std::make_unique<m68hc05_disassembler>();
}

void m68hc05pge_device::send_port(u8 offset, u8 data)
{
	m_write_p[offset](data);
}

u8 m68hc05pge_device::ports_r(offs_t offset)
{
	u8 incoming = m_read_p[offset]();

	// apply data direction registers
	incoming &= (m_ddrs[offset] ^ 0xff);
	// OR in ddr-masked version of port writes
	incoming |= (m_ports[offset] & m_ddrs[offset]);

	return incoming;
}

void m68hc05pge_device::ports_w(offs_t offset, u8 data)
{
	send_port(offset, (data & m_ddrs[offset]) | (m_pullups[offset] & ~m_ddrs[offset]));
	m_ports[offset] = data;
}

u8 m68hc05pge_device::ddrs_r(offs_t offset)
{
	return m_ddrs[offset];
}

void m68hc05pge_device::ddrs_w(offs_t offset, u8 data)
{
	send_port(offset, (m_ports[offset] & data) | (m_pullups[offset] & ~data));
	m_ddrs[offset] = data;
}

u8 m68hc05pge_device::pll_r()
{
	return m_pll_ctrl;
}

void m68hc05pge_device::pll_w(u8 data)
{
	if (m_pll_ctrl != data)
	{
		static const int clocks[4] = {524288, 1048576, 2097152, 4194304};
		LOG("PLL ctrl: clock %d TCS:%d BCS:%d AUTO:%d BWC:%d PLLON:%d (PC=%x)\n", clocks[data & 3],
			(data & 0x80) ? 1 : 0,
			(data & 0x40) ? 1 : 0,
			(data & 0x20) ? 1 : 0,
			(data & 0x10) ? 1 : 0,
			(data & 0x08) ? 1 : 0, pc());
	}

	m_pll_ctrl = data;
}

void m68hc05pge_device::execute_set_input(int inputnum, int state)
{
	if (state == CLEAR_LINE)
	{
		m_pending_interrupts &= ~(1 << inputnum);
	}
	else
	{
		m_pending_interrupts |= (1 << inputnum);
	}
}

// fires every 1 second
TIMER_CALLBACK_MEMBER(m68hc05pge_device::seconds_tick)
{
	m_rtc++;
	m_cpicsr |= (1 << CPICSR_ONESEC_IRQ_FLAG);
	if (BIT(m_cpicsr, CPICSR_ONESEC_IRQ_ENABLE))
	{
		set_input_line(M68HC05PGE_INT_CPI, ASSERT_LINE);
	}
}

// fires every 5.86ms
TIMER_CALLBACK_MEMBER(m68hc05pge_device::cpi_tick)
{
	m_cpicsr |= (1 << CPICSR_586_IRQ_FLAG);
	if (BIT(m_cpicsr, CPICSR_586_IRQ_ENABLE))
	{
		set_input_line(M68HC05PGE_INT_RTI, ASSERT_LINE);
	}
}

const tiny_rom_entry *m68hc05pge_device::device_rom_region() const
{
	return ROM_NAME(m68hc05pge);
}

void m68hc05pge_device::m68hc05pge_map(address_map &map)
{
	map(0x0000, 0x0003).rw(FUNC(m68hc05pge_device::ports_r), FUNC(m68hc05pge_device::ports_w));
	map(0x0004, 0x0006).rw(FUNC(m68hc05pge_device::ddrs_r), FUNC(m68hc05pge_device::ddrs_w));
	map(0x0007, 0x0007).rw(FUNC(m68hc05pge_device::pll_r), FUNC(m68hc05pge_device::pll_w));
	map(0x000a, 0x000c).rw(FUNC(m68hc05pge_device::spi_r), FUNC(m68hc05pge_device::spi_w));
	map(0x000d, 0x000d).rw(FUNC(m68hc05pge_device::cpicsr_r), FUNC(m68hc05pge_device::cpicsr_w));
	map(0x000e, 0x000e).rw(FUNC(m68hc05pge_device::cscr_r), FUNC(m68hc05pge_device::cscr_w));
	map(0x000f, 0x000f).rw(FUNC(m68hc05pge_device::kcsr_r), FUNC(m68hc05pge_device::kcsr_w));
	map(0x0014, 0x0016).rw(FUNC(m68hc05pge_device::trackball_r), FUNC(m68hc05pge_device::trackball_w));
	map(0x0018, 0x001a).rw(FUNC(m68hc05pge_device::adb_r), FUNC(m68hc05pge_device::adb_w));
	map(0x001c, 0x001c).rw(FUNC(m68hc05pge_device::option_r), FUNC(m68hc05pge_device::option_w));
	map(0x001d, 0x001e).rw(FUNC(m68hc05pge_device::adc_r), FUNC(m68hc05pge_device::adc_w));
	map(0x0020, 0x002c).rw(FUNC(m68hc05pge_device::ports_high_r), FUNC(m68hc05pge_device::ports_high_w));
	map(0x002d, 0x0032).rw(FUNC(m68hc05pge_device::pwm_r), FUNC(m68hc05pge_device::pwm_w));
	map(0x0034, 0x0036).rw(FUNC(m68hc05pge_device::plm_r), FUNC(m68hc05pge_device::plm_w));
	map(0x0038, 0x003b).rw(FUNC(m68hc05pge_device::rtc_r), FUNC(m68hc05pge_device::rtc_w));
	map(0x0040, 0x03ff).ram().share(m_internal_ram);    // internal RAM
	map(0x0ff0, 0x0ff0).nopw();                         // watchdog reset (period not known)

	map(0x8000, 0xffff).view(m_introm);
	m_introm[0](0x8000, 0xffff).rw(FUNC(m68hc05pge_device::sram_r), FUNC(m68hc05pge_device::sram_w));
	m_introm[1](0x8000, 0xffff).rw(FUNC(m68hc05pge_device::sram_r), FUNC(m68hc05pge_device::sram_w));
	m_introm[1](0xfe00, 0xffff).rom().region("pge", 0);
}

u8 m68hc05pge_device::sram_r(offs_t offset)
{
	if (BIT(m_cscr, CSCR_SRAM_CS) && !BIT(m_option, OPTION_EXTBUS))
	{
		return m_sram[offset];
	}

	return 0xff;
}

void m68hc05pge_device::sram_w(offs_t offset, u8 data)
{
	if (BIT(m_cscr, CSCR_SRAM_CS) && !BIT(m_option, OPTION_EXTBUS))
	{
		m_sram[offset] = data;
	}
}

u8 m68hc05pge_device::spi_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
			return m_spcr;

		case 1:
			return m_spsr;

		case 2:
			if (!machine().side_effects_disabled())
			{
				LOGMASKED(LOG_SPI, "SPI got %02x\n", m_spi_in);
				if (BIT(m_spsr, SPSR_IRQ_FLAG))
				{
					set_input_line(M68HC05PGE_INT_SPI, CLEAR_LINE);
				}
			}
			return m_spi_in;
	}

	return 0;
}

void m68hc05pge_device::spi_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0:
			m_spcr = data;
			m_spi_clock = BIT(m_spcr, SPCR_POLARITY);
			write_spi_clock(m_spi_clock);
			break;

		case 2:
			if (!BIT(m_spcr, SPCR_MASTER))
			{
				logerror("68HC05PGE: SPI slave mode not implemented\n");
			}

			if (BIT(m_spsr, SPSR_IRQ_FLAG))
			{
				set_input_line(M68HC05PGE_INT_SPI, CLEAR_LINE);
			}
			m_spsr &= ~(1 << SPSR_IRQ_FLAG);

			m_spi_clock = BIT(m_spcr, SPCR_POLARITY);
			write_spi_clock(m_spi_clock);

			m_spi_out = data;
			m_spi_in = 0;
			LOGMASKED(LOG_SPI, "SPI: sending %02x, clock rate %d\n", data, clock() / s_spi_divisors[m_spcr & 3]);
			m_spi_bit = 16;
			m_spi_timer->adjust(attotime::from_hz(clock() / s_spi_divisors[m_spcr & 3]));
			break;
	}
}

TIMER_CALLBACK_MEMBER(m68hc05pge_device::spi_tick)
{
	LOGMASKED(LOG_SPI_VERBOSE, "spi_tick: bit %d\n", m_spi_bit);

	// first clock edge of a bit
	if (!(m_spi_bit & 1))
	{
		// phase = 0, set up the output data before sending the first clock edge of the bit
		if (!BIT(m_spcr, SPCR_PHASE))
		{
			write_spi_mosi(BIT(m_spi_out, 7));
			m_spi_out <<= 1;
		}

		write_spi_clock(m_spi_clock ^ 1);

		// phase = 0, input bit became valid on that first edge
		if (!BIT(m_spcr, SPCR_PHASE))
		{
			m_spi_in <<= 1;
			m_spi_in |= m_spi_miso;
			LOGMASKED(LOG_SPI_VERBOSE, "PGE: MISO %d, shift %02x (PH0 POL%d)\n", m_spi_miso, m_spi_in, BIT(m_spcr, SPCR_POLARITY));
		}
	}
	else    // second clock edge of the bit
	{
		// phase = 1, the output bit must be valid before this clock edge
		if (BIT(m_spcr, SPCR_PHASE))
		{
			write_spi_mosi(BIT(m_spi_out, 7));
			m_spi_out <<= 1;
		}

		write_spi_clock(m_spi_clock);

		// phase = 1, input bit became valid on this second edge
		if (BIT(m_spcr, SPCR_PHASE))
		{
			m_spi_in <<= 1;
			m_spi_in |= m_spi_miso;
			LOGMASKED(LOG_SPI_VERBOSE, "PGE: MISO %d, shift %02x (PH1 POL%d)\n", m_spi_miso, m_spi_in, BIT(m_spcr, SPCR_POLARITY));
		}
	}

	m_spi_bit--;
	if (m_spi_bit > 0)
	{
		m_spi_timer->adjust(attotime::from_hz(clock() / s_spi_divisors[m_spcr & 3]));
	}
	else
	{
		m_spsr |= (1 << SPSR_IRQ_FLAG);
		if (BIT(m_spcr, SPCR_IRQ_ENABLE))
		{
			set_input_line(M68HC05PGE_INT_SPI, ASSERT_LINE);
		}
	}
}

u8 m68hc05pge_device::cpicsr_r()
{
	return m_cpicsr;
}

void m68hc05pge_device::cpicsr_w(u8 data)
{
	m_cpicsr = data;
	if (!BIT(data, CPICSR_ONESEC_IRQ_FLAG))
	{
		set_input_line(M68HC05PGE_INT_CPI, CLEAR_LINE);
	}
	if (!BIT(data, CPICSR_586_IRQ_FLAG))
	{
		set_input_line(M68HC05PGE_INT_RTI, CLEAR_LINE);
	}
}

u8 m68hc05pge_device::cscr_r()
{
	return m_cscr;
}

void m68hc05pge_device::cscr_w(u8 data)
{
	m_cscr = data;
}

u8 m68hc05pge_device::kcsr_r()
{
	return 0;
}

void m68hc05pge_device::kcsr_w(u8 data)
{
//  printf("%02x to KCSR\n", data);
}

u8 m68hc05pge_device::trackball_r(offs_t offset)
{
	switch (offset)
	{
		case 0:     // TBCS
			return (m_read_tbB() << 7) | m_tbcs;    // button not pressed
			break;

		case 1:     // signed X delta
			return m_read_tbX();

		case 2:     // signed Y delta
			return m_read_tbY();
	}

	return 0;
}

void m68hc05pge_device::trackball_w(offs_t offset, u8 data)
{
	if (offset == 0)
	{
		m_tbcs = data & 0x7f;
	}
}

u8 m68hc05pge_device::adb_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
			return m_adbcr;

		case 1:
			return m_adbsr;

		case 2:
			return m_adbdr;
	}

	return 0;
}

void m68hc05pge_device::adb_w(offs_t offset, u8 data)
{
	//printf("%02x to ADB @ %d\n", data, offset);
	switch (offset)
	{
		case 0:
			//printf("%02x to ADBCR, previous %02x\n", data, m_adbcr);
			// if we're clearing transmit complete, set transmitter empty
			if (BIT(m_adbcr, ADBXR_TC) && !BIT(data, ADBXR_TC))
			{
				//printf("ADB enabling transmitter empty\n");
				m_adbsr |= (1 << ADBXR_TDRE);
			}

			// if we're clearing transmitter empty, kick the timer for transmitter complete
			if (BIT(m_adbcr, ADBXR_TDRE) && !BIT(data, ADBXR_TDRE))
			{
				//printf("ADB setting completion timer\n");
				m_adb_timer->adjust(attotime::from_usec(50), 1);
			}

			m_adbcr = data;
			if (m_adbsr & m_adbcr & ADBXR_IRQS)
			{
				set_input_line(M68HC05PGE_INT_ADB, ASSERT_LINE);
			}
			else
			{
				set_input_line(M68HC05PGE_INT_ADB, CLEAR_LINE);
			}
			break;

		case 1:
			m_adbsr = data;
			if (m_adbsr & m_adbcr & ADBXR_IRQS)
			{
				set_input_line(M68HC05PGE_INT_ADB, ASSERT_LINE);
			}
			else
			{
				set_input_line(M68HC05PGE_INT_ADB, CLEAR_LINE);
			}
			break;

		case 2:
			m_adbdr = data;
			LOGMASKED(LOG_ADB, "ADB sending %02x\n", data);
			m_adbsr &= ~((1 << ADBXR_TDRE) | (1 << ADBXR_TC));
			m_adb_timer->adjust(attotime::from_usec(1200), 0);
			break;
	}
}

TIMER_CALLBACK_MEMBER(m68hc05pge_device::adb_tick)
{
	switch (param)
	{
		case 0:         // byte transmitted, trigger transmitter empty
			m_adbsr |= (1 << ADBXR_TDRE);
			break;

		case 1:
			m_adbsr |= (1 << ADBXR_TC);
			break;
	}

	if (m_adbsr & m_adbcr & ADBXR_IRQS)
	{
		set_input_line(M68HC05PGE_INT_ADB, ASSERT_LINE);
	}
}

u8 m68hc05pge_device::option_r()
{
	return m_option;
}

void m68hc05pge_device::option_w(u8 data)
{
	LOGMASKED(LOG_GENERAL, "%02x to OPTION\n", data);
	m_option = data;

	m_introm.select(BIT(data, OPTION_INTROM));
}

u8 m68hc05pge_device::adc_r(offs_t offset)
{
	if (!offset)
	{
		LOGMASKED(LOG_ADC, "ADC read ch %d\n", m_adcsr & ADCSR_CHANNEL_MASK);
		return m_ad_in[m_adcsr & ADCSR_CHANNEL_MASK]();
	}

	return m_adcsr;
}

void m68hc05pge_device::adc_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_ADC, "%02x to ADC @ %d\n", data, offset);
	if (offset)
	{
		m_adcsr = data;

		if (BIT(m_adcsr, ADCSR_START_CONV))
		{
			m_adcsr |= (1 << ADCSR_CONV_COMPLETE);
		}
	}
}

u8 m68hc05pge_device::ports_high_r(offs_t offset)
{
	switch (offset)
	{
		case 0: // PORTE
		case 2: // PORTF
		case 4: // PORTG
		case 6: // PORTH
		case 8: // PORTJ
			return ports_r((offset >> 1) + PGE_PORTE);

		case 0xa: // PORTL
			return ports_r(PGE_PORTL);

		case 0xc: // PORTK
			return ports_r(PGE_PORTK);

		case 1: // DDRE
		case 3: // DDRF
		case 5: // DDRG
		case 7: // DDRH
		case 9: // DDRJ
			return m_ddrs[(offset >> 1) + PGE_PORTE];

		case 0xb: // DDRL
			return m_ddrs[PGE_PORTL];
	}

	return 0;
}

void m68hc05pge_device::ports_high_w(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0: // PORTE
	case 2: // PORTF
	case 4: // PORTG
	case 6: // PORTH
	case 8: // PORTJ
		ports_w((offset >> 1) + PGE_PORTE, data);
		break;

	case 0xa: // PORTL
		ports_w(PGE_PORTL, data);
		break;

	case 0xc: // PORTK
		ports_w(PGE_PORTK, data);
		break;

	case 1: // DDRE
	case 3: // DDRF
	case 5: // DDRG
	case 7: // DDRH
	case 9: // DDRJ
		ddrs_w((offset >> 1) + PGE_PORTE, data);
		break;

	case 0xb: // DDRL
		ddrs_w(PGE_PORTL, data);
		break;
	}
}

u8 m68hc05pge_device::pwm_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
			return m_pwmacr;

		case 1:
			return m_pwma0;

		case 2:
			return m_pwma1;

		case 3:
			return m_pwmbcr;

		case 4:
			return m_pwmb0;

		case 5:
			return m_pwmb1;
	}

	return 0;
}

void m68hc05pge_device::pwm_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_PWM, "%02x to PWM @ %d\n", data, offset);
	switch (offset)
	{
		case 0: // PWMACR
			m_pwmacr = data;
			break;

		case 1: // PWMA0
			LOGMASKED(LOG_PWM, "%02x to PWMA0\n", data);
			m_pwma0 = data;
			m_pwm_out[PGE_PWMA0](data);
			break;

		case 2: // PWMA1
			LOGMASKED(LOG_PWM, "%02x to PWMA1\n", data);
			m_pwma1 = data;
			m_pwm_out[PGE_PWMA1](data);
			break;

		case 3: // PWMBCR
			m_pwmbcr = data;
			break;

		case 4: // PWMB0
			LOGMASKED(LOG_PWM, "%02x to PWMB0\n", data);
			m_pwmb0 = data;
			m_pwm_out[PGE_PWMB0](data);
			break;

		case 5: // PWMB1
			LOGMASKED(LOG_PWM, "%02x to PWMB1\n", data);
			m_pwmb1 = data;
			m_pwm_out[PGE_PWMB1](data);
			break;
	}
}

u8 m68hc05pge_device::plm_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
			return m_plmcr;

		case 1:
			return m_plmt1;

		case 2:
			return m_plmt2;
	}

	return 0;
}

void m68hc05pge_device::plm_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_PLM, "%02x to PLM @ %d\n", data, offset);
	switch (offset)
	{
		case 0: // PLMCR
			m_plmcr = data;
			break;

		case 1: // PLM timer 1
			m_plmt1 = data;
			break;

		case 2: // PLM timer 2
			m_plmt2 = data;
			break;
	}
}

u8 m68hc05pge_device::rtc_r(offs_t offset)
{
	switch (offset)
	{
		case 0:
			return m_rtc >> 24;

		case 1:
			return (m_rtc >> 16) & 0xff;

		case 2:
			return (m_rtc >> 8) & 0xff;

		case 3:
			return m_rtc & 0xff;
	}

	return 0;
}

void m68hc05pge_device::rtc_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0:
			m_rtc &= ~0xff000000;
			m_rtc |= data << 24;
			break;

		case 1:
			m_rtc &= ~0x00ff0000;
			m_rtc |= data << 16;
			break;

		case 2:
			m_rtc &= ~0x0000ff00;
			m_rtc |= data << 8;
			break;

		case 3:
			m_rtc &= ~0x000000ff;
			m_rtc |= data;
			break;
	}
}

void m68hc05pge_device::nvram_default()
{
}

bool m68hc05pge_device::nvram_read(util::read_stream &file)
{
	auto const [err, actual] = read(file, m_internal_ram, 0x3c0);
	auto const [err2, actual2] = read(file, m_sram, 0x8000);

	m_internal_ram[0x91 - 0x40] = 0;    // clear power flag so the boot ROM does a cold boot

	return !err && !err2;
}

bool m68hc05pge_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = write(file, m_internal_ram, 0x3c0);
	auto const [err2, actual2] = write(file, m_sram, 0x8000);
	return !err && !err2;
}
