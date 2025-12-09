// license:BSD-3-Clause
// copyright-holders: Karl Stenerud, NaokiS

/*
    Notes:
        Chip Select Module:
            * Not currently implemented, quite possible it doesn't need to be added for any functional reason aside from completeness/debugging
        System Integration Module:
            * MBAR: should be set up in m68000_musashi_device::x4e7a_movec_l_c() to move the ColdFire IO register map according to the contents of MBAR
                        could store m_mbar in m68000_musashi_device and move the address_space_map to the given offset?
            * SIMR: Logged and stored, but not used as there is no BDM interface present
            * MARB: Logged and stored, but not used, not required?
        DRAM Controller Module:
            * All functions just log and store the registers, again, possibly not required for any functional reasons.
        UART Modules:
            * Uses the MC68681 device driver as a base as effectively it is an integrated one. The difference however is both UART modules are entirely
                independant of the other, so there is two command registers e.t.c. They also only have a single channel each.
            * In this implementation, the driver uses two MC68681s at the appropriate offsets to emulate this, with the MC68681 driver having an
                entry for the MCF5206E which changes the mapping a little bit. A more appropriate solution would be to have a single UART module and load
                two of them.
        Timer Modules:
            * Slow, could be improved.

        * Some of these modules can probably be split back into the /devices/machine device, namely the MBUS and timer modules, possibly SIM too.
*/

#include "emu.h"
#include "mcf5206e.h"
#include "m68kdasm.h"

#define LOG_DEBUG       (1U << 1)
#define LOG_INVALID     (1U << 2)
#define LOG_TIMER       (1U << 3)
#define LOG_UART        (1U << 4)
#define LOG_SWDT        (1U << 5)
#define LOG_MBUS        (1U << 6)
#define LOG_DRAM        (1U << 7)
#define LOG_SIM         (1U << 8)
#define LOG_DMA         (1U << 9)
#define VERBOSE         ( LOG_DEBUG | LOG_UART | LOG_SWDT | LOG_MBUS | LOG_DRAM | LOG_SIM | LOG_DMA | LOG_TIMER )
#include "logmacro.h"

#define INT_LEVEL(N)    ((N&0x1c) >> 2)
#define INT_PRIORITY(N)     (N&0x03)
#define ICR_USE_AUTOVEC(N)  ((N & 0x80) != 0)

static constexpr int EXCEPTION_BUS_ERROR                = 2;
static constexpr int EXCEPTION_UNINITIALIZED_INTERRUPT = 15;
static constexpr int EXCEPTION_SPURIOUS_INTERRUPT      = 24;

template <typename T, typename U>
inline void BITWRITE(T &var, U bit_number, bool state)
{
	var = (var & ~(static_cast<T>(1) << bit_number)) | (static_cast<T>(state) << bit_number);
}

DEFINE_DEVICE_TYPE(MCF5206E,    mcf5206e_device,    "mcf5206e",     "Freescale MCF5206E")
DEFINE_DEVICE_TYPE(COLDFIRE_SIM,    coldfire_sim_device,    "coldfire_sim",     "ColdFire SIM Module")
DEFINE_DEVICE_TYPE(COLDFIRE_DMA,    coldfire_dma_device,    "coldfire_dma",     "ColdFire DMA Module")
DEFINE_DEVICE_TYPE(COLDFIRE_MBUS,    coldfire_mbus_device,    "coldfire_mbus",     "ColdFire MBUS Module")
DEFINE_DEVICE_TYPE(COLDFIRE_TIMER,    coldfire_timer_device,    "coldfire_timer",     "ColdFire Timer Module")

std::unique_ptr<util::disasm_interface> mcf5206e_device::create_disassembler()
{
	return std::make_unique<m68k_disassembler>(m68k_disassembler::TYPE_COLDFIRE);
}

mcf5206e_device::mcf5206e_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68000_musashi_device(mconfig, tag, owner, clock, MCF5206E, 32, 32)
	, m_mbar_config("cpu_mbar", ENDIANNESS_BIG, 32, 10, 0, address_map_constructor(FUNC(mcf5206e_device::mbar_map), this))
	, m_coldfire_vector_map("cpu_space", ENDIANNESS_BIG, 32, 28, 0, address_map_constructor(FUNC(mcf5206e_device::coldfire_vector_map), this))
	, m_sim(*this, "sim")
	, m_timer(*this, "timer%u", 1U)
	, write_chip_select(*this)
	, m_uart(*this, "uart%u", 1U)
	, write_tx1(*this)
	, write_tx2(*this)
	, m_gpio_r_cb(*this, 0xff)
	, m_gpio_w_cb(*this)
	, m_mbus(*this, "mbus")
	, write_sda(*this)
	, write_scl(*this)
	, m_dma(*this, "dma%u", 0U)
{
}

// TODO: make it derive from m68000_musashi_device properly
device_memory_interface::space_config_vector mcf5206e_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO, &m_mbar_config),
		std::make_pair(AS_CPU_SPACE, &m_coldfire_vector_map)
	};
}

void mcf5206e_device::device_add_mconfig(machine_config &config)
{
	COLDFIRE_SIM(config, m_sim, this->clock(), this->tag());

	COLDFIRE_TIMER(config, m_timer[0], this->clock());
	m_timer[0]->irq_cb().set(FUNC(mcf5206e_device::timer_1_irq));
	COLDFIRE_TIMER(config, m_timer[1], this->clock());
	m_timer[1]->irq_cb().set(FUNC(mcf5206e_device::timer_2_irq));

	MCF5206E_UART(config, m_uart[0], this->clock());
	MCF5206E_UART(config, m_uart[1], this->clock());
	m_uart[0]->irq_cb().set(FUNC(mcf5206e_device::uart_1_irq));
	m_uart[0]->set_clocks(m_tin[0], 0, 0, 0);
	m_uart[1]->irq_cb().set(FUNC(mcf5206e_device::uart_2_irq));
	m_uart[1]->set_clocks(m_tin[1], 0, 0, 0);

	COLDFIRE_MBUS(config, m_mbus, this->clock());
	m_mbus->sda_cb().set(FUNC(mcf5206e_device::mbus_sda_w));
	m_mbus->scl_cb().set(FUNC(mcf5206e_device::mbus_scl_w));
	m_mbus->irq_cb().set(FUNC(mcf5206e_device::mbus_irq_w));

	COLDFIRE_DMA(config, m_dma[0], this->clock());
	COLDFIRE_DMA(config, m_dma[1], this->clock());
	m_dma[0]->irq_cb().set(FUNC(mcf5206e_device::dma0_irq_w));
	m_dma[1]->irq_cb().set(FUNC(mcf5206e_device::dma1_irq_w));

}

static inline u32 dword_from_byte(u8 data) { return data * 0x01010101U; }
static inline u32 dword_from_word(u16 data) { return data * 0x00010001U; }
static inline u32 dword_from_unaligned_word(u16 data) { return u32(data) << 8 | ((data >> 8) * 0x01000001U); }

bool mcf5206e_device::is_mbar_access(offs_t offset)
{
	const u32 mbar_start = m_mbar & ~1;
	const u32 mbar_end = m_mbar | 0x3ff;
	return BIT(m_mbar, 0) && offset >= mbar_start && offset <= mbar_end;
}


void mcf5206e_device::device_start()
{
	m68000_musashi_device::device_start();
	init_cpu_coldfire();

	m_readimm16 = [this](offs_t address) -> u16 {
		if (is_mbar_access(address))
			return space(AS_IO).read_word(address & 0x3ff);
		return m_oprogram32.read_word(address);
	};
	m_read8   = [this](offs_t address) -> u8 {
		if (is_mbar_access(address))
			return space(AS_IO).read_byte(address & 0x3ff);
		return m_program32.read_byte(address);
	};
	m_read16  = [this](offs_t address) -> u16 {
		if (is_mbar_access(address))
			return space(AS_IO).read_word_unaligned(address & 0x3ff);

		return m_program32.read_word_unaligned(address);
	};
	m_read32  = [this](offs_t address) -> u32 {
		if (is_mbar_access(address))
			return space(AS_IO).read_dword_unaligned(address & 0x3ff);
		return m_program32.read_dword_unaligned(address);
	};
	m_write8  = [this](offs_t address, u8 data)  {
		if (is_mbar_access(address))
		{
			address &= 0x3ff;
			space(AS_IO).write_dword(address & 0xfffffffcU, dword_from_byte(data), 0xff000000U >> 8 * (address & 3));
			return;
		}
		m_program32.write_dword(address & 0xfffffffcU, dword_from_byte(data), 0xff000000U >> 8 * (address & 3));
	};
	m_write16 = [this](offs_t address, u16 data)  {
		if (is_mbar_access(address))
		{
			address &= 0x3ff;
			switch (address & 3)
			{
			case 0:
				space(AS_IO).write_dword(address, dword_from_word(data), 0xffff0000U);
				break;

			case 1:
				space(AS_IO).write_dword(address - 1, dword_from_unaligned_word(data), 0x00ffff00);
				break;

			case 2:
				space(AS_IO).write_dword(address - 2, dword_from_word(data), 0x0000ffff);
				break;

			case 3:
				space(AS_IO).write_dword(address - 3, dword_from_unaligned_word(data), 0x000000ff);
				space(AS_IO).write_dword(address + 1, dword_from_byte(data & 0x00ff), 0xff000000U);
				break;
			}
			return;
		}

		switch (address & 3) {
		case 0:
			m_program32.write_dword(address, dword_from_word(data), 0xffff0000U);
			break;

		case 1:
			m_program32.write_dword(address - 1, dword_from_unaligned_word(data), 0x00ffff00);
			break;

		case 2:
			m_program32.write_dword(address - 2, dword_from_word(data), 0x0000ffff);
			break;

		case 3:
			m_program32.write_dword(address - 3, dword_from_unaligned_word(data), 0x000000ff);
			m_program32.write_dword(address + 1, dword_from_byte(data & 0x00ff), 0xff000000U);
			break;
		}
	};
	m_write32 = [this](offs_t address, u32 data) {
		if (is_mbar_access(address))
		{
			address &= 0x3ff;

			switch (address & 3)
			{
				case 0:
					space(AS_IO).write_dword(address, data, 0xffffffffU);
					break;

				case 1:
					space(AS_IO).write_dword(address - 1, (data & 0xff000000U) | (data & 0xffffff00U) >> 8, 0x00ffffff);
					space(AS_IO).write_dword(address + 3, dword_from_byte(data & 0x000000ff), 0xff000000U);
					break;

				case 2:
					space(AS_IO).write_dword(address - 2, dword_from_word((data & 0xffff0000U) >> 16), 0x0000ffff);
					space(AS_IO).write_dword(address + 2, dword_from_word(data & 0x0000ffff), 0xffff0000U);
					break;

				case 3:
					space(AS_IO).write_dword(address - 3, dword_from_unaligned_word((data & 0xffff0000U) >> 16), 0x000000ff);
					space(AS_IO).write_dword(address + 1, rotl_32(data, 8), 0xffffff00U);
					break;
			}
			return;
		}

		switch (address & 3) {
		case 0:
			m_program32.write_dword(address, data, 0xffffffffU);
			break;

		case 1:
			m_program32.write_dword(address - 1, (data & 0xff000000U) | (data & 0xffffff00U) >> 8, 0x00ffffff);
			m_program32.write_dword(address + 3, dword_from_byte(data & 0x000000ff), 0xff000000U);
			break;

		case 2:
			m_program32.write_dword(address - 2, dword_from_word((data & 0xffff0000U) >> 16), 0x0000ffff);
			m_program32.write_dword(address + 2, dword_from_word(data & 0x0000ffff), 0xffff0000U);
			break;

		case 3:
			m_program32.write_dword(address - 3, dword_from_unaligned_word((data & 0xffff0000U) >> 16), 0x000000ff);
			m_program32.write_dword(address + 1, rotl_32(data, 8), 0xffffff00U);
			break;
		}
	};


	init_regs(true);

	save_item(NAME(m_dcrr));
	save_item(NAME(m_dctr));
	save_item(NAME(m_dcar0));
	save_item(NAME(m_dcmr0));
	save_item(NAME(m_dccr0));
	save_item(NAME(m_dcar1));
	save_item(NAME(m_dcmr1));
	save_item(NAME(m_dccr1));

	save_item(NAME(m_csar));
	save_item(NAME(m_csmr));
	save_item(NAME(m_cscr));
	save_item(NAME(m_dmcr));

	save_item(NAME(m_ppddr));
	save_item(NAME(m_ppdat_out));

}

void mcf5206e_device::device_reset()
{
	m68000_musashi_device::device_reset();
	m_timer[0]->reset();
	m_timer[1]->reset();
	m_uart[0]->reset();
	m_uart[1]->reset();
	m_mbus->reset();

	init_regs(false);
}

// should be set up in m68000_musashi_device::x4e7a_movec_l_c() to move this map according to the contents of MBAR
void mcf5206e_device::mbar_map(address_map &map)
{
	/* SIM Module */
	map(0x000, 0x0cf).m(m_sim, FUNC(coldfire_sim_device::sim_map));

	/* dram controller */
	map(0x046, 0x047).rw(FUNC(mcf5206e_device::dcrr_r), FUNC(mcf5206e_device::dcrr_w));
	map(0x04a, 0x04b).rw(FUNC(mcf5206e_device::dctr_r), FUNC(mcf5206e_device::dctr_w));
	map(0x04c, 0x04d).rw(FUNC(mcf5206e_device::dcar0_r), FUNC(mcf5206e_device::dcar0_w));
	map(0x050, 0x053).rw(FUNC(mcf5206e_device::dcmr0_r), FUNC(mcf5206e_device::dcmr0_w));
	map(0x057, 0x057).rw(FUNC(mcf5206e_device::dccr0_r), FUNC(mcf5206e_device::dccr0_w));
	map(0x058, 0x059).rw(FUNC(mcf5206e_device::dcar1_r), FUNC(mcf5206e_device::dcar1_w));
	map(0x05c, 0x05f).rw(FUNC(mcf5206e_device::dcmr1_r), FUNC(mcf5206e_device::dcmr1_w));
	map(0x063, 0x063).rw(FUNC(mcf5206e_device::dccr1_r), FUNC(mcf5206e_device::dccr1_w));

	/* chip select registers */
	map(0x064, 0x065).rw(FUNC(mcf5206e_device::csar0_r), FUNC(mcf5206e_device::csar0_w));
	map(0x068, 0x06b).rw(FUNC(mcf5206e_device::csmr0_r), FUNC(mcf5206e_device::csmr0_w));
	map(0x06e, 0x06e).rw(FUNC(mcf5206e_device::cscr0_r), FUNC(mcf5206e_device::cscr0_w));
	map(0x070, 0x071).rw(FUNC(mcf5206e_device::csar1_r), FUNC(mcf5206e_device::csar1_w));
	map(0x074, 0x077).rw(FUNC(mcf5206e_device::csmr1_r), FUNC(mcf5206e_device::csmr1_w));
	map(0x07a, 0x07a).rw(FUNC(mcf5206e_device::cscr1_r), FUNC(mcf5206e_device::cscr1_w));
	map(0x07c, 0x07d).rw(FUNC(mcf5206e_device::csar2_r), FUNC(mcf5206e_device::csar2_w));
	map(0x080, 0x083).rw(FUNC(mcf5206e_device::csmr2_r), FUNC(mcf5206e_device::csmr2_w));
	map(0x086, 0x086).rw(FUNC(mcf5206e_device::cscr2_r), FUNC(mcf5206e_device::cscr2_w));
	map(0x088, 0x089).rw(FUNC(mcf5206e_device::csar3_r), FUNC(mcf5206e_device::csar3_w));
	map(0x08c, 0x08f).rw(FUNC(mcf5206e_device::csmr3_r), FUNC(mcf5206e_device::csmr3_w));
	map(0x092, 0x092).rw(FUNC(mcf5206e_device::cscr3_r), FUNC(mcf5206e_device::cscr3_w));
	map(0x094, 0x095).rw(FUNC(mcf5206e_device::csar4_r), FUNC(mcf5206e_device::csar4_w));
	map(0x098, 0x09b).rw(FUNC(mcf5206e_device::csmr4_r), FUNC(mcf5206e_device::csmr4_w));
	map(0x09e, 0x09e).rw(FUNC(mcf5206e_device::cscr4_r), FUNC(mcf5206e_device::cscr4_w));
	map(0x0a0, 0x0a1).rw(FUNC(mcf5206e_device::csar5_r), FUNC(mcf5206e_device::csar5_w));
	map(0x0a4, 0x0a7).rw(FUNC(mcf5206e_device::csmr5_r), FUNC(mcf5206e_device::csmr5_w));
	map(0x0aa, 0x0aa).rw(FUNC(mcf5206e_device::cscr5_r), FUNC(mcf5206e_device::cscr5_w));
	map(0x0ac, 0x0ad).rw(FUNC(mcf5206e_device::csar6_r), FUNC(mcf5206e_device::csar6_w));
	map(0x0b0, 0x0b3).rw(FUNC(mcf5206e_device::csmr6_r), FUNC(mcf5206e_device::csmr6_w));
	map(0x0b6, 0x0b6).rw(FUNC(mcf5206e_device::cscr6_r), FUNC(mcf5206e_device::cscr6_w));
	map(0x0b8, 0x0b9).rw(FUNC(mcf5206e_device::csar7_r), FUNC(mcf5206e_device::csar7_w));
	map(0x0bc, 0x0bf).rw(FUNC(mcf5206e_device::csmr7_r), FUNC(mcf5206e_device::csmr7_w));
	map(0x0c2, 0x0c2).rw(FUNC(mcf5206e_device::cscr7_r), FUNC(mcf5206e_device::cscr7_w));
	map(0x0c4, 0x0c7).rw(FUNC(mcf5206e_device::dmcr_r), FUNC(mcf5206e_device::dmcr_w));

	// timer 1
	map(0x100, 0x11f).m(m_timer[0], FUNC(coldfire_timer_device::timer_map));
	map(0x120, 0x13f).m(m_timer[1], FUNC(coldfire_timer_device::timer_map));

	// uart (mc68681 derived)
	map(0x140, 0x17f).rw(m_uart[0], FUNC(mcf5206e_uart_device::read), FUNC(mcf5206e_uart_device::write)).umask32(0xff000000);
	map(0x180, 0x1bf).rw(m_uart[1], FUNC(mcf5206e_uart_device::read), FUNC(mcf5206e_uart_device::write)).umask32(0xff000000);

	// parallel port
	map(0x1c5, 0x1c5).rw(FUNC(mcf5206e_device::ppddr_r), FUNC(mcf5206e_device::ppddr_w));
	map(0x1c9, 0x1c9).rw(FUNC(mcf5206e_device::ppdat_r), FUNC(mcf5206e_device::ppdat_w));

	// mbus (i2c)
	map(0x1e0, 0x1ff).m(m_mbus, FUNC(coldfire_mbus_device::mbus_map));

	// dma
	map(0x200, 0x21f).m(m_dma[0], FUNC(coldfire_dma_device::dma_map));
	map(0x240, 0x25f).m(m_dma[1], FUNC(coldfire_dma_device::dma_map));
}

void mcf5206e_device::coldfire_vector_map(address_map &map)
{
	map(0xfffffe0, 0xfffffff).r(m_sim, FUNC(coldfire_sim_device::interrupt_callback));
}

/*
 * DRAM Controller
 * Handles the DRAM refresh and access control circuits
 */
void mcf5206e_device::dcrr_w(u16 data)
{
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Refresh Register) DCRR_w: %04x\n", this->machine().describe_context(), data);
	m_dcrr = data;
}

void mcf5206e_device::dctr_w(u16 data)
{
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Timing Register) DCTR_w: %04x\n", this->machine().describe_context(), data);
	m_dctr = data;
}

void mcf5206e_device::dcar0_w(u16 data)
{
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Access Register 0) DCAR0_w: %04x\n", this->machine().describe_context(), data);
	m_dcar0 = data;
}

void mcf5206e_device::dcmr0_w(u32 data)
{
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Mask Register 0) DCMR0_w: %08x\n", this->machine().describe_context(), data);
	m_dcmr0 = data;
}

void mcf5206e_device::dccr0_w(u8 data)
{
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Control Register 0) DCCR0_w: %04x\n", this->machine().describe_context(), data);
	m_dccr0 = data;
}

void mcf5206e_device::dcar1_w(u16 data)
{
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Access Register 1) DCAR1_w: %04x\n", this->machine().describe_context(), data);
	m_dcar1 = data;
}

void mcf5206e_device::dcmr1_w(u32 data)
{
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Mask Register 1) DCMR1_w: %04x\n", this->machine().describe_context(), data);
	m_dcmr1 = data;
}

void mcf5206e_device::dccr1_w(u8 data)
{
	LOGMASKED(LOG_DRAM, "%s: (DRAM Controller Control Register 1) DCCR1_w: %04x\n", this->machine().describe_context(), data);
	m_dccr1 = data;
}

/*
 * Chip Select Module
 * Controls what address spaces that the configurable chip select pins will be assigned to.
 */

u16 mcf5206e_device::csar_x_r(offs_t offset)
{
	LOGMASKED(LOG_DEBUG, "%s: (Chip Select Address Register) CSAR%d_r\n", this->machine().describe_context(), offset);
	return m_csar[offset];
}

void mcf5206e_device::csar_x_w(offs_t offset, u16 data)
{
	m_csar[offset] = data;
	LOGMASKED(LOG_DEBUG, "%s: (Chip Select Address Register) CSAR%d_w %04x\n", this->machine().describe_context(), offset, data);
}

u32 mcf5206e_device::csmr_x_r(offs_t offset)
{
	LOGMASKED(LOG_DEBUG, "%s: (Chip Select Mask Register) CSMR%d_r\n", this->machine().describe_context(), offset);
	return m_csmr[offset];
}

void mcf5206e_device::csmr_x_w(offs_t offset, u32 data)
{
	m_csmr[offset] = data;
	LOGMASKED(LOG_DEBUG, "%s: (Chip Select Mask Register) CSMR%d_w %08x\n", this->machine().describe_context(), offset, data);
}

u8 mcf5206e_device::cscr_x_r(offs_t offset)
{
	LOGMASKED(LOG_DEBUG, "%s: (Chip Select Control Register) CSCR%d_r\n", this->machine().describe_context(), offset);
	return m_cscr[offset];
}

void mcf5206e_device::cscr_x_w(offs_t offset, u8 data)
{
	m_cscr[offset] = data;
	LOGMASKED(LOG_DEBUG, "%s: (Chip Select Control Register) CSCR%d_w %04x\n", this->machine().describe_context(), offset, data);
}

void mcf5206e_device::dmcr_w(u16 data)
{
	m_dmcr = data;
	LOGMASKED(LOG_DEBUG, "%s: (Default Memory Control Register) DMCR_w %04x\n", this->machine().describe_context(), data);
}


/*
 * Parallel port
 * Just a 8 bit GPIO.
 */


void mcf5206e_device::ppddr_w(u8 data)
{
	LOGMASKED(LOG_DEBUG, "%s: (Port A Data Direction Register) PPDDR_w %02x\n", this->machine().describe_context(), data);

	if(m_ppddr != data)
	{
		// Updating the register updates the pins immediately
		u8 mask = 0;
		if(!BIT(m_sim->get_par(), 4)) mask |= 0x0f; // PP 0-3 / DDATA 0-3
		if(!BIT(m_sim->get_par(), 5)) mask |= 0xf0; // PP 4-7 / PST 0-3

		u8 ppdat_in = m_gpio_r_cb();

		// GPIO pins will physically be set to the current input and output state, and masked according to PAR
		m_gpio_w_cb(((m_ppdat_out & m_ppddr) | (ppdat_in & ~m_ppddr)) & mask);
	}

	m_ppddr = data;
}

u8 mcf5206e_device::ppddr_r()
{
	return m_ppddr;
}

u8 mcf5206e_device::ppdat_r()
{
	u8 ppdat_in = m_gpio_r_cb();

	return (ppdat_in & ~m_ppddr) | (m_ppdat_out & m_ppddr);
}

void mcf5206e_device::ppdat_w(u8 data)
{
	LOGMASKED(LOG_DEBUG, "%s: (Port A Data Register) PPDAT_w %02x\n", this->machine().describe_context(), data);
	m_ppdat_out = data;

	u8 mask = 0;
	if(!BIT(m_sim->get_par(), 4)) mask |= 0x0f; // PP 0-3 / DDATA 0-3
	if(!BIT(m_sim->get_par(), 5)) mask |= 0xf0; // PP 4-7 / PST 0-3
	u8 ppdat_in = m_gpio_r_cb();

	m_gpio_w_cb(((m_ppdat_out & m_ppddr) | (ppdat_in & ~m_ppddr)) & mask);
}


/*
 * System Integration Module
 * Handles the interrupt system and bus
*/

coldfire_sim_device::coldfire_sim_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, COLDFIRE_SIM, tag, owner, clock)
	, irq_vector_cb(*this, (u8)EXCEPTION_BUS_ERROR)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_swdt(*this, "watchdog")
{
}

void coldfire_sim_device::device_add_mconfig(machine_config &config)
{
	WATCHDOG_TIMER(config, m_swdt);
}

void coldfire_sim_device::device_start()
{
	m_swdt->watchdog_enable(0);
	m_timer_swdt = timer_alloc( FUNC( coldfire_sim_device::swdt_callback ), this ); // For interrupt version

	save_item(NAME(m_simr));
	save_item(NAME(m_marb));

	save_item(NAME(m_par));
	save_item(NAME(m_icr));
	save_item(NAME(m_imr));
	save_item(NAME(m_ipr));
	save_item(NAME(m_sypcr));
	save_item(NAME(m_swivr));
	save_item(NAME(m_rsr));
	save_item(NAME(m_swdt_w_count));
	save_item(NAME(m_sypcr_locked));
	save_item(NAME(m_external_ipl));
}

void coldfire_sim_device::device_reset()
{
	if(!(m_rsr & 0x20)) m_rsr = 0x00;   // don't clear if watchdog triggered
	else m_rsr = 0x70;

	m_simr = 0xc0;
	m_marb = 0x00;
	m_sypcr = 0x00;
	m_swivr = 0x0f;
	m_imr = 0x3ffe;
	m_ipr = 0x0000;
	m_par = 0x0000;

	m_swdt_w_count = 0;
	m_sypcr_locked = false;

	m_icr[ICR1] =   0x04;
	m_icr[ICR2] =   0x08;
	m_icr[ICR3] =   0x0c;
	m_icr[ICR4] =   0x10;
	m_icr[ICR5] =   0x14;
	m_icr[ICR6] =   0x18;
	m_icr[ICR7] =   0x1c;
	m_icr[ICR_SWDT] =   0x1c;
	m_icr[ICR_TMR1] =   0x80;
	m_icr[ICR_TMR2] =  0x80;
	m_icr[ICR_MBUS] =  0x80;
	m_icr[ICR_UART1] =  0x00;
	m_icr[ICR_UART2] =  0x00;
	m_icr[ICR_DMA0] =  0x00;
	m_icr[ICR_DMA1] =  0x00;

	m_external_ipl = 0;
}

void coldfire_sim_device::sim_map(address_map &map)
{
	map(0x03, 0x03).rw(FUNC(coldfire_sim_device::simr_r), FUNC(coldfire_sim_device::simr_w));
	map(0x07, 0x07).rw(FUNC(coldfire_sim_device::marb_r), FUNC(coldfire_sim_device::marb_w));

	/* interrupt control registers */
	map(0x14, 0x22).rw(FUNC(coldfire_sim_device::icr_r), FUNC(coldfire_sim_device::icr_w));
	map(0x36, 0x37).rw(FUNC(coldfire_sim_device::imr_r), FUNC(coldfire_sim_device::imr_w));
	map(0x3a, 0x3b).r(FUNC(coldfire_sim_device::ipr_r));

	map(0x40, 0x40).rw(FUNC(coldfire_sim_device::rsr_r), FUNC(coldfire_sim_device::rsr_w));
	map(0x41, 0x41).rw(FUNC(coldfire_sim_device::sypcr_r), FUNC(coldfire_sim_device::sypcr_w));
	map(0x42, 0x42).rw(FUNC(coldfire_sim_device::swivr_r), FUNC(coldfire_sim_device::swivr_w));
	map(0x43, 0x43).w(FUNC(coldfire_sim_device::swsr_w));

	map(0xca, 0xcb).rw(FUNC(coldfire_sim_device::par_r), FUNC(coldfire_sim_device::par_w));
}

// MBAR + 0x003: SIM Configuration Register - Not really applicable to MAME as there's no BDM port currently but hey.
void  coldfire_sim_device::simr_w(u8 data)
{
	LOGMASKED(LOG_DEBUG, "%s: SIMR_w %02x\n", this->machine().describe_context(), data);
	m_simr = data;
}

// MBAR + 0x003: Bus Master Arbitration Control
void coldfire_sim_device::marb_w(u8 data)
{
	LOGMASKED(LOG_DEBUG, "%s: (Bus Master Arbitration Control) MARB_w %02x\n", this->machine().describe_context(), data);
	m_marb = data;
}

// MBAR + 0x014 -> 0x022: Interupt Control Registers
u8 coldfire_sim_device::icr_r(offs_t offset)
{
	if(offset > 15)
	{
		logerror("%s: Request to read invalid ICR offset received: %d\n", this->machine().describe_context(), offset);
		return 0;
	}
	LOGMASKED(LOG_DEBUG, "%s: (Interrupt Control Register %d) read: %02x\n", this->machine().describe_context(), offset, m_icr[offset]);
	return m_icr[offset];
}

void coldfire_sim_device::icr_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0: m_icr[offset] = (data & 0x83) + (1 << 2); break;
		case 1: m_icr[offset] = (data & 0x83) + (2 << 2); break;
		case 2: m_icr[offset] = (data & 0x83) + (3 << 2); break;
		case 3: m_icr[offset] = (data & 0x83) + (4 << 2); break;
		case 4: m_icr[offset] = (data & 0x83) + (5 << 2); break;
		case 5: m_icr[offset] = (data & 0x83) + (6 << 2); break;
		case 6: m_icr[offset] = (data & 0x83) + (7 << 2); break;
		case 7: m_icr[offset] = (data & 0x03) + (7 << 2); break;    // IPL7 and SWDT share same level, also you cannot use autovector on SWT.
		case 8: m_icr[offset] = (data & 0x1f) + 0x80; break;        // Timer 1 *must* use autovector
		case 9: m_icr[offset] = (data & 0x1f) + 0x80; break;        // Timer 2 *must* use autovector
		case 10: m_icr[offset] = (data & 0x1f) + 0x80; break;       // MBUS *must* use autovector
		case 11: m_icr[offset] = (data & 0x9f); break;
		case 12: m_icr[offset] = (data & 0x9f); break;
		case 13: m_icr[offset] = (data & 0x9f); break;
		case 14: m_icr[offset] = (data & 0x9f); break;
		default: logerror("%s: Implausible ICR offset received: %d", this->machine().describe_context(), offset);
	}
	//printf("%d %02x -> %02x\n", offset, data, m_icr[offset]);
	//icr_info(m_icr[offset]);
}

// MBAR + 0x036: Interrupt Mask Register
void coldfire_sim_device::imr_w(u16 data)
{
	m_imr = (data & 0xfffe);
	LOGMASKED(LOG_DEBUG, "%s: (Interrupt Mask Register) IMR_w %04x\n", this->machine().describe_context(), data);
}

void coldfire_sim_device::icr_info(u8 icr)
{
	LOGMASKED(LOG_DEBUG, "  (AutoVector) AVEC : %01x | ", (icr&0x80)>>7);
	LOGMASKED(LOG_DEBUG, "(Interrupt Level) IL : %01x | ", INT_LEVEL(icr)); // if autovector (AVEC) is used then the vectors referenced are at +24 (+0x18) + IL, ie the standard 68k autovectors, otherwise vector must be provided by device
	LOGMASKED(LOG_DEBUG, "(Interrupt Priority) IP : %01x |", (icr&0x03)>>0);
	LOGMASKED(LOG_DEBUG, "(Unused bits) : %01x\n", (icr&0x60)>>5);
}

void coldfire_sim_device::par_w(u16 data)
{
	m_par = data;
	LOGMASKED(LOG_DEBUG, "%s: (Pin Assignment Register) PAR_w %04x\n", this->machine().describe_context(), data);
}

void coldfire_sim_device::set_external_interrupt(int level, int state)
{
	// State here is inverted, inputs are active low
	if(BIT(m_par, 6))
	{
		// External IPL pins are encoded (IPL 1-7 levels)
		m_external_ipl = level;
	}
	else
	{
		// External IPL pins are discrete (IRQ1, IRQ4, IRQ7)
		switch(level)
		{
			case 1: BITWRITE(m_external_ipl, 0, state); break;
			case 4: BITWRITE(m_external_ipl, 1, state); break;
			case 7: BITWRITE(m_external_ipl, 2, state); break;
			default: break;
		}
	}
}

void mcf5206e_device::execute_set_input(int inputnum, int state)
{
	m68000_musashi_device::execute_set_input(inputnum, state);
	// HACK: make it accept external interrupts
	if (inputnum == 1 || inputnum == 4 || inputnum == 7)
		m_sim->set_ipr(inputnum, state);

	if (BIT(m_sim->get_par(), 6))
		popmessage("mcf5206e: encoded interrupt priority!");
}

// Return the vector for the highest priority and level interrupt
u8 coldfire_sim_device::interrupt_callback(offs_t level)
{
	u8 ipl = (level >> 1) & 7;  // Should be 2 for coldFire, really

	u8 highest_priority_icr = 0;
	u8 highest_priority_device = 0;

	if(!this->machine().side_effects_disabled())
	{
		//logerror("%s: interrupt_callback(%u), ipl: %x, m_ipr: %x, m_imr: %x\n", this->machine().describe_context(), level, ipl, m_ipr, m_imr);
		m_maincpu->set_input_line(ipl, CLEAR_LINE);
	}

	for (int i = 0; i < 15; i++)
	{
		//if(!this->machine().side_effects_disabled()) logerror("i: %x, m_icr: %x, ipl: %x, ipr_bit: %x\n", i, INT_LEVEL(m_icr[i]), ipl, BIT(m_ipr, 1 + i));
		if (BIT(m_ipr, 1 + i) && (INT_LEVEL(m_icr[i]) == ipl))
		{
			if (highest_priority_device == 0 || INT_PRIORITY(m_icr[i]) > INT_PRIORITY(highest_priority_icr))
			{
				highest_priority_icr = m_icr[i];
				highest_priority_device = i + 1;
			}
		}
	}

	if (highest_priority_device == 0)
	{
		if(!this->machine().side_effects_disabled()) logerror("%s: Spurious interrupt detected: %u\n", this->machine().describe_context(), ipl);
		return EXCEPTION_SPURIOUS_INTERRUPT;
	}

	BITWRITE(m_ipr, ipl, 0);

	u8 vector = 0xff;

	// Check if ICR specifies to use autovectoring
	if (BIT(highest_priority_icr, 7))
	{
		vector = m68000_base_device::autovector(ipl);
	}
	else
	{
		// Determine the correct vector to return
		switch (highest_priority_device)
		{
			case EXTERNAL_IPL_1:
			case EXTERNAL_IPL_2:
			case EXTERNAL_IPL_3:
			case EXTERNAL_IPL_4:
			case EXTERNAL_IPL_5:
			case EXTERNAL_IPL_6:
			case EXTERNAL_IPL_7:
				if (!BIT(highest_priority_icr, 7))
					vector = irq_vector_cb();
				break;
			case WATCHDOG_IRQ:
				if (!BIT(highest_priority_icr, 7))
					vector = m_swivr;
				break;
			case UART_1_IRQ:
				if (!BIT(highest_priority_icr, 7))
					vector = m_maincpu->m_uart[0]->get_irq_vector();
				break;
			case UART_2_IRQ:
				if (!BIT(highest_priority_icr, 7))
					vector = m_maincpu->m_uart[1]->get_irq_vector();
				break;
			case DMA_0_IRQ:
				if (!BIT(highest_priority_icr, 7))
					vector = m_maincpu->m_dma[0]->get_irq_vector();
				//m_maincpu->m_dma[0]->dma_int_callback();
				break;
			case DMA_1_IRQ:
				if (!BIT(highest_priority_icr, 7))
					vector = m_maincpu->m_dma[1]->get_irq_vector();
				//m_maincpu->m_dma[1]->dma_int_callback();
				break;
			default:
				if(!this->machine().side_effects_disabled())
					logerror("%s: Vector required for device that only supports autovectoring: %u, %u\n",
						this->machine().describe_context(), ipl, highest_priority_device);
				vector = EXCEPTION_UNINITIALIZED_INTERRUPT;
				break;
		}
	}

	return vector;
}

void coldfire_sim_device::set_interrupt(int interrupt, int state)
{
	BITWRITE(m_ipr, interrupt, ((state == CLEAR_LINE) ? 0 : 1));

	//LOGMASKED(LOG_DEBUG, "%s: set_interrupt(%u, %u): %x, %d, %d\n", this->machine().describe_context(), interrupt, state, m_ipr, BIT(m_imr, interrupt), m_imr & interrupt);

	//printf("%d %d -> %04x %04x\n", interrupt, state, m_imr, m_ipr);

	// IMR enables interrupts when bit is 0
	if(state != CLEAR_LINE)
	{
		if(!BIT(m_imr, interrupt))
		{
			u8 icr = 0;
			switch (interrupt)
			{
				case EXTERNAL_IPL_1: icr = m_icr[ICR1]; break;
				case EXTERNAL_IPL_2: icr = m_icr[ICR2]; break;
				case EXTERNAL_IPL_3: icr = m_icr[ICR3]; break;
				case EXTERNAL_IPL_4: icr = m_icr[ICR4]; break;
				case EXTERNAL_IPL_5: icr = m_icr[ICR5]; break;
				case EXTERNAL_IPL_6: icr = m_icr[ICR6]; break;
				case EXTERNAL_IPL_7: icr = m_icr[ICR7]; break;
				case WATCHDOG_IRQ:  icr = m_icr[ICR_SWDT]; break;
				case TIMER_1_IRQ:   icr = m_icr[ICR_TMR1]; break;   // Always autovector
				case TIMER_2_IRQ:   icr = m_icr[ICR_TMR2]; break;   // Always autovector
				case MBUS_IRQ:      icr = m_icr[ICR_MBUS]; break;   // Always autovector
				case UART_1_IRQ:    icr = m_icr[ICR_UART1]; break;
				case UART_2_IRQ:    icr = m_icr[ICR_UART2]; break;
				case DMA_0_IRQ:     icr = m_icr[ICR_DMA0];  break;
				case DMA_1_IRQ:     icr = m_icr[ICR_DMA1];  break;
				default:
					logerror("%s: Unknown device trying to set interrupt level: %u\n", this->machine().describe_context(), interrupt);
					return;
			}

			m_maincpu->set_input_line(INT_LEVEL(icr), ASSERT_LINE);
		}
	}
}

/* Reset status */
// MBAR + 0x040
void coldfire_sim_device::rsr_w(u8 data)
{
	m_rsr &= ~data;
	LOGMASKED(LOG_DEBUG, "%s: (Reset Status Register) RSR_w %02x\n", this->machine().describe_context(), data);
}

/* Watchdog */
// MBAR + 0x041
void coldfire_sim_device::sypcr_w(u8 data)
{
	if(!m_sypcr_locked)
	{
		// SYPCR is a write-once register, whatever is written first remains until system reset.
		LOGMASKED(LOG_SWDT, "%s: (System Protection Control) SYPCR_w %02x\n", this->machine().describe_context(), data);
		m_sypcr_locked = true;
		m_sypcr = data;

		// Bus monitoring is not supported (nor will it be?)

		if(BIT(m_sypcr, 6))
		{
			m_swdt->watchdog_enable(BIT(data, 7));
		}
		else
		{
			// TODO: Set timer for interrupt
		}
	}
	else
	{
		LOG("%s: Write to SYPCR_w (%02x) when PCR is locked\n", this->machine().describe_context(), data);
	}
}

// MBAR + 0x042
void coldfire_sim_device::swivr_w(u8 data)
{
	LOGMASKED(LOG_SWDT, "%s: (Software Watchdog Interrupt Vector) SWIVR_w %02x\n", this->machine().describe_context(), data);
	m_swivr = data;
}

// MBAR + 0x043
void coldfire_sim_device::swsr_w(u8 data)
{
	LOGMASKED(LOG_SWDT, "%s: (Software Watchdog Service Routine) SWIVR_r %02x\n", this->machine().describe_context(), data);
	if(data == swdt_reset_sequence[m_swdt_w_count])
		m_swdt_w_count++;
	if(m_swdt_w_count == 2)
	{
		m_swdt->watchdog_reset();
		m_swdt_w_count = 0;
	}
}

TIMER_CALLBACK_MEMBER(coldfire_sim_device::swdt_callback)
{
	// Todo
	//set_interrupt(WDINT);
}

/*
 * Timer Module/s
 * Creates an MCF5206e compatible 16-bit timer
 */
coldfire_timer_device::coldfire_timer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, COLDFIRE_TIMER, tag, owner, clock)
	, write_irq(*this)
{
}

void coldfire_timer_device::timer_map(address_map &map)
{
	map(0x00, 0x01).rw(FUNC(coldfire_timer_device::tmr_r), FUNC(coldfire_timer_device::tmr_w));
	map(0x04, 0x05).rw(FUNC(coldfire_timer_device::trr_r), FUNC(coldfire_timer_device::trr_w));
	map(0x08, 0x09).r(FUNC(coldfire_timer_device::tcr_r));  // TCR is r/only
	map(0x0c, 0x0d).rw(FUNC(coldfire_timer_device::tcn_r), FUNC(coldfire_timer_device::tcn_w));
	map(0x11, 0x11).rw(FUNC(coldfire_timer_device::ter_r), FUNC(coldfire_timer_device::ter_w));
}

void coldfire_timer_device::device_start()
{
	m_timer = timer_alloc( FUNC( coldfire_timer_device::timer_callback ), this );

	save_item(NAME(m_tmr));
	save_item(NAME(m_trr));
	save_item(NAME(m_tcr));
	save_item(NAME(m_tcn));
	save_item(NAME(m_ter));
	save_item(NAME(m_timer_start_time));
}

void coldfire_timer_device::device_reset()
{
	m_tmr = 0x0000;
	m_trr = 0xffff;
	m_tcn = 0x0000;
	m_tcr = 0x0000;
	m_ter = 0x00;
	m_timer_start_time = attotime::zero;
	m_timer->adjust(attotime::never);
	write_irq(CLEAR_LINE);
}

TIMER_CALLBACK_MEMBER(coldfire_timer_device::timer_callback)
{
	if(m_tmr & T_FRR)
	{
		// FRR resets counter to 0
		m_tcn = 0;
	}
	m_ter |= T_EREF;

	// TODO: capture edge irq modes
	if(BIT(m_tmr, 4))
		write_irq(ASSERT_LINE);
}

void coldfire_timer_device::tmr_w(u16 data)
{
	u16 cmd = data;

	if((m_tmr & T_RST) && !(cmd & T_RST))
	{
		// T_RST high to low resets the entire timer
		device_reset();
		return;
	}

	m_tmr = cmd;

	if(m_tmr & T_RST)
	{
		// TODO: add tin pin support
		int div, start, interval;
		start = (m_trr - m_tcn);
		div = ((m_tmr & 0xff00) >> 8) + 1;                      // 1 -> 256 division scale
		if ((m_tmr & T_CL1) && !(m_tmr & T_CL0)) div = div * 16;    // input clock / 16
		if (!(m_tmr & T_FRR)) interval = (0xffff * div);            // don't reset tcn to 0
		else interval = start;                                      // else tcn is reset to 0
		m_timer_start_time = machine().time();
		m_timer->adjust(clocks_to_attotime(start * div), 0, clocks_to_attotime(interval * div));
	}

	LOGMASKED(LOG_TIMER, "%s: (Timer Mode Register) TMR_w: %04x\n", this->machine().describe_context(), data);
	//LOGMASKED(LOG_TIMER, " (Prescale) PS : %02x  (Capture Edge/Interrupt) CE : %01x (Output Mode) OM : %01x  (Output Reference Interrupt En) ORI : %01x   Free Run (FRR) : %01x  Input Clock Source (ICLK) : %01x  (Reset Timer) RST : %01x  \n", (m_TMR1 & 0xff00)>>8, (m_TMR1 & 0x00c0)>>6,  (m_TMR1 & 0x0020)>>5, (m_TMR1 & 0x0010)>>4, (m_TMR1 & 0x0008)>>3, (m_TMR1 & 0x0006)>>1, (m_TMR1 & 0x0001)>>0);
}

void coldfire_timer_device::trr_w(u16 data)
{
	m_trr = data;
	LOGMASKED(LOG_TIMER, "%s: (Timer Reference Register) TRR_w: %04x\n", this->machine().describe_context(), data);
}

u16 coldfire_timer_device::tcn_r()
{
	m_tcn = (attotime_to_clocks(machine().time() - m_timer_start_time) & 0xffff);
	return m_tcn;
}

void coldfire_timer_device::tcn_w(u16 data)
{
	// Writing any value resets the counter
	m_tcn = 0;
	LOGMASKED(LOG_TIMER, "%s: (Timer Counter Reset) TCN_w: %04x\n", this->machine().describe_context(), data);
}

void coldfire_timer_device::ter_w(u8 data)
{
	m_ter &= ~data; // Programmer must write bit to clear it. IE write 0x80 to clear bit 7.
	LOGMASKED(LOG_TIMER, "%s: (Timer Event) TER_w: %02x\n", this->machine().describe_context(), data);
	write_irq(CLEAR_LINE);
}

#define UNINIT 0
#define UNINIT_NOTE 0

/*
 * init_regs
 * Resets the internal registers to their POR states.
 * first_init is used during boot, set to false during reset
 */
void mcf5206e_device::init_regs(bool first_init)
{
	m_dcrr = 0x0000;
	m_dctr = 0x0000;
	m_dcar0 = UNINIT;
	m_dcmr0 = UNINIT;
	m_dccr0 = 0x00;
	m_dcar1 = UNINIT;
	m_dcmr1 = UNINIT;
	m_dccr1 = 0x00;

	m_csar[0] = 0x0000;
	m_csmr[0] = 0x00000000;
	m_cscr[0] = 0x3c1f; /* 3C1F, 3C5F, 3C9F, 3CDF, 3D1F, 3D5F, 3D9F, 3DDF |  AA set by IRQ 7 at reset, PS1 set by IRQ 4 at reset, PS0 set by IRQ 1 at reset*/

	if (first_init)
	{
		for (int x=1;x<8;x++)
		{
			m_csar[1] = UNINIT;
			m_csmr[1] = UNINIT;
			m_cscr[1] = UNINIT_NOTE; // except BRST=ASET=WRAH=RDAH=WR=RD=0
		}
	}

	m_dmcr = 0x0000;

	m_ppddr = 0x00;
	m_ppdat_out = 0x00;
}


/*
 * MBUS Module
 * Hosts I2C and Motorola extensions to the format. Can act as a device or host.
*/

coldfire_mbus_device::coldfire_mbus_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, COLDFIRE_MBUS, tag, owner, clock)
	, write_sda(*this)
	, write_scl(*this)
	, write_irq(*this)
{
}


void coldfire_mbus_device::device_start()
{
	m_timer_mbus = timer_alloc( FUNC( coldfire_mbus_device::mbus_callback ), this );

	save_item(NAME(m_madr));
	save_item(NAME(m_mbcr));
	save_item(NAME(m_mbsr));
	save_item(NAME(m_mfdr));
	save_item(NAME(m_mbdr));

	save_item(NAME(m_tx_in_progress));
	save_item(NAME(m_clk_state));
	save_item(NAME(m_tx_bit));
	save_item(NAME(m_tx_out));
	save_item(NAME(m_tx_in));
}

void coldfire_mbus_device::device_reset()
{
	m_madr = 0x00;
	m_mfdr = 0x00;
	m_mbcr = 0x00;
	m_mbsr = 0x81;
	m_mbdr = 0x00;

	m_tx_in_progress = false;
	m_clk_state = 1;
	m_tx_bit = 0;
	m_tx_out = 0;
	m_tx_in = 0;

	m_timer_mbus->adjust(attotime::never);
}


void coldfire_mbus_device::mbus_map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(coldfire_mbus_device::madr_r), FUNC(coldfire_mbus_device::madr_w));
	map(0x04, 0x04).rw(FUNC(coldfire_mbus_device::mfdr_r), FUNC(coldfire_mbus_device::mfdr_w));
	map(0x08, 0x08).rw(FUNC(coldfire_mbus_device::mbcr_r), FUNC(coldfire_mbus_device::mbcr_w));
	map(0x0c, 0x0c).rw(FUNC(coldfire_mbus_device::mbsr_r), FUNC(coldfire_mbus_device::mbsr_w));
	map(0x10, 0x10).rw(FUNC(coldfire_mbus_device::mbdr_r), FUNC(coldfire_mbus_device::mbdr_w));
}

TIMER_CALLBACK_MEMBER(coldfire_mbus_device::mbus_callback)
{
	// TODO: Do bit transfers etc
	// gamtor wants i2c irqs, as it runs the task based EEPROM checks there.
}

void coldfire_mbus_device::madr_w(u8 data)
{
	m_madr = (data & 0xfe);
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Control Register) madr_w: %02x\n", this->machine().describe_context(), data);
}

void coldfire_mbus_device::mfdr_w(u8 data)
{
	m_mfdr = (data & 0x3F);
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Frequency Divider Register) mfdr_w: %02x\n", this->machine().describe_context(), data);
}

void coldfire_mbus_device::mbcr_w(u8 data)
{
	m_mbcr = (data & 0xfc);
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Control Register) mbcr_w: %02x\n", this->machine().describe_context(), data);
}

u8 coldfire_mbus_device::mbsr_r()
{
	int hack = 0x00;

	hack ^= (machine().rand()&0xff);
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Status Register) mbsr_r: %02x\n", this->machine().describe_context(), m_mbsr);
	return m_mbsr ^ hack; // will loop on this after a while
}

void coldfire_mbus_device::mbsr_w(u8 data)
{
	m_mbsr = (data & 0x14); // MAL & MIF
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Status Register) mbsr_w: %02x\n", this->machine().describe_context(), data);
}

u8 coldfire_mbus_device::mbdr_r()
{
	int hack = 0x00;
	hack ^= (machine().rand()&0xff);
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Data I/O Register) mbdr_r: %02x\n", this->machine().describe_context(), m_mbdr);
	return m_mbdr ^ hack;
}

void coldfire_mbus_device::mbdr_w(u8 data)
{
	m_mbdr = data;
	LOGMASKED(LOG_MBUS, "%s: (M-Bus Data I/O Register) mbdr_w: %02x\n", this->machine().describe_context(), data);
}


/*
 * DMA Module
 *
*/

coldfire_dma_device::coldfire_dma_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, COLDFIRE_DMA, tag, owner, clock)
	, write_irq(*this)
{
}


void coldfire_dma_device::device_start()
{
	//m_timer_dma = timer_alloc( FUNC( coldfire_dma_device::dma_callback ), this );

	save_item(NAME(m_sar));
	save_item(NAME(m_dar));
	save_item(NAME(m_dcr));
	save_item(NAME(m_bcr));
	save_item(NAME(m_dsr));
	save_item(NAME(m_divr));
}

void coldfire_dma_device::device_reset()
{
	m_sar = 0x00000000;
	m_dar = 0x00000000;
	m_dcr    = 0x0000;
	m_bcr    = 0x0000;
	m_dsr    = 0x00;
	m_divr   = 0x0F;

	//m_timer_dma->adjust(attotime::never);
}


void coldfire_dma_device::dma_map(address_map &map)
{
	map(0x00, 0x03).rw(FUNC(coldfire_dma_device::sar_r), FUNC(coldfire_dma_device::sar_w));
	map(0x04, 0x07).rw(FUNC(coldfire_dma_device::dar_r), FUNC(coldfire_dma_device::dar_w));
	map(0x08, 0x09).rw(FUNC(coldfire_dma_device::dcr_r), FUNC(coldfire_dma_device::dcr_w));
	map(0x0c, 0x0d).rw(FUNC(coldfire_dma_device::bcr_r), FUNC(coldfire_dma_device::bcr_w));
	map(0x10, 0x10).rw(FUNC(coldfire_dma_device::dsr_r), FUNC(coldfire_dma_device::dsr_w));
	map(0x14, 0x14).rw(FUNC(coldfire_dma_device::divr_r), FUNC(coldfire_dma_device::divr_w));
}

void coldfire_dma_device::sar_w(u32 data)
{
	m_sar = data;
	LOGMASKED(LOG_DMA, "%s: (DMA Source Address) sar_w: %08x\n", this->machine().describe_context(), data);
}

void coldfire_dma_device::dar_w(u32 data)
{
	m_dar = data;
	LOGMASKED(LOG_DMA, "%s: (DMA Destination Address) dar_w: %08x\n", this->machine().describe_context(), data);
}

void coldfire_dma_device::dcr_w(u16 data)
{
	m_dcr = data;
	LOGMASKED(LOG_DMA, "%s: (DMA Control Register) dcr_w: %04x\n", this->machine().describe_context(), data);
	if (BIT(data, 0))
		popmessage("%s unemulated DMA trigger", this->tag());
}

void coldfire_dma_device::bcr_w(u16 data)
{
	m_bcr = data;
	LOGMASKED(LOG_DMA, "%s: (DMA Byte Count) bcr_w: %04x\n", this->machine().describe_context(), data);
}

void coldfire_dma_device::dsr_w(u8 data)
{
	BITWRITE(m_dsr, 0, BIT(data, 0));   // Manual states only writes to bit 0 has any effect on the register
	LOGMASKED(LOG_DMA, "%s: (DMA Status Register) dsr_w: %02x\n", this->machine().describe_context(), data);
}

void coldfire_dma_device::divr_w(u8 data)
{
	m_divr = data;
	LOGMASKED(LOG_DMA, "%s: (DMA Interrupt Vector) divr_w: %02x\n", this->machine().describe_context(), data);
}

/*
ADDRESS (LE)            REG         WIDTH   NAME/DESCRIPTION                                    INIT VALUE (MR=Master Reset, NR=Normal Reset)       Read or Write access
* = inited
- = skeleton handler

op MOVEC with $C0F      MBAR        32      Module Base Address Register                        uninit (except V=0)                                 W
$003 √                  SIMR        8       SIM Configuration Register                          C0                                                  R/W
$014*-                  ICR1        8       Interrupt Control Register 1 - External IRQ1/IPL1   04                                                  R/W
$015*-                  ICR2        8       Interrupt Control Register 2 - External IPL2        08                                                  R/W
$016*-                  ICR3        8       Interrupt Control Register 3 - External IPL3        0C                                                  R/W
$017*-                  ICR4        8       Interrupt Control Register 4 - External IRQ4/IPL4   10                                                  R/W
$018*                   ICR5        8       Interrupt Control Register 5 - External IPL5        14                                                  R/W
$019*                   ICR6        8       Interrupt Control Register 6 - External IPL6        18                                                  R/W
$01A*                   ICR7        8       Interrupt Control Register 7 - External IRQ7/IPL7   1C                                                  R/W
$01B*                   ICR8        8       Interrupt Control Register 8 - SWT                  1C                                                  R/W
$01C*-                  ICR9        8       Interrupt Control Register 9 - Timer 1 Interrupt    80                                                  R/W
$01D*-                  ICR10       8       Interrupt Control Register 10 - Timer 2 Interrupt   80                                                  R/W
$01E*-                  ICR11       8       Interrupt Control Register 11 - MBUS Interrupt      80                                                  R/W
$01F*-                  ICR12       8       Interrupt Control Register 12 - UART 1 Interrupt    00                                                  R/W
$020*-                  ICR13       8       Interrupt Control Register 13 - UART 2 Interrupt    00                                                  R/W
$020*-                  ICR14       8       Interrupt Control Register 14 - DMA 0  Interrupt    00                                                  R/W
$020*-                  ICR15       8       Interrupt Control Register 15 - DMA 1  Interrupt    00                                                  R/W
$036*-                  IMR         16      Interrupt Mask Register                             3FFE                                                R/W
$03A                    IPR         16      Interrupt Pending Register                          0000                                                R
$040                    RSR         8       Reset Status Register                               80 / 20                                             R/W
$041                    SYPCR       8       System Protection Control Register                  00                                                  R/W
$042                    SWIVR       8       Software Watchdog Interrupt Vector Register         0F                                                  R/W
$043                    SWSR        8       Software Watchdog Service Register                  uninit                                              W
$046                    DCRR        16      DRAM Controller Refresh                             MR 0000   - NR uninit                               R/W
$04A                    DCTR        16      DRAM Controller Timing Register                     MR 0000   - NR uninit                               R/W
$04C                    DCAR0       16      DRAM Controller 0 Address Register                  MR uninit - NR uninit                               R/W
$050                    DCMR0       32      DRAM Controller 0 Mask Register                     MR uninit - NR uninit                               R/W
$057                    DCCR0       8       DRAM Controller 0 Control Register                  MR 00     - NR 00                                   R/W
$058                    DCAR1       16      DRAM Controller 1 Address Register                  MR uninit - NR uninit                               R/W
$05C                    DCMR1       32      DRAM Controller 1 Mask Register                     MR uninit - NR uninit                               R/W
$063                    DCCR1       8       DRAM Controller 1 Control Register                  MR 00     - NR 00                                   R/W
--------- CHIP SELECTS -----------
$064*-                  CSAR0       16      Chip-Select 0 Address Register                      0000                                                R/W
$068*-                  CSMR0       32      Chip-Select 0 Mask Register                         00000000                                            R/W
$06E*-                  CSCR0       16      Chip-Select 0 Control Register                      3C1F, 3C5F, 3C9F, 3CDF, 3D1F, 3D5F, 3D9F, 3DDF      R/W
                                                                                                AA set by IRQ 7 at reset
                                                                                                PS1 set by IRQ 4 at reset
                                                                                                PS0 set by IRQ 1 at reset
$070*-                  CSAR1       16      Chip-Select 1 Address Register                      uninit                                              R/W
$074*-                  CSMR1       32      Chip-Select 1 Mask Register                         uninit                                              R/W
$07A*-                  CSCR1       16      Chip-Select 1 Control Register                      uninit *1                                           R/W
$07C*-                  CSAR2       16      Chip-Select 2 Address Register                      uninit                                              R/W
$080*-                  CSMR2       32      Chip-Select 2 Mask Register                         uninit                                              R/W
$086*-                  CSCR2       16      Chip-Select 2 Control Register                      uninit *1                                           R/W
$088*-                  CSAR3       16      Chip-Select 3 Address Register                      uninit                                              R/W
$08C*-                  CSMR3       32      Chip-Select 3 Mask Register                         uninit                                              R/W
$092*-                  CSCR3       16      Chip-Select 3 Control Register                      uninit *1                                           R/W
$094*-                  CSAR4       16      Chip-Select 4 Address Register                      uninit                                              R/W
$098*-                  CSMR4       32      Chip-Select 4 Mask Register                         uninit                                              R/W
$09E*-                  CSCR4       16      Chip-Select 4 Control Register                      uninit *1                                           R/W
$0A0*-                  CSAR5       16      Chip-Select 5 Address Register                      uninit                                              R/W
$0A4*-                  CSMR5       32      Chip-Select 5 Mask Register                         uninit                                              R/W
$0AA*-                  CSCR5       16      Chip-Select 5 Control Register                      uninit *1                                           R/W
$0AC*-                  CSAR6       16      Chip-Select 6 Address Register                      uninit                                              R/W
$0B0*-                  CSMR6       32      Chip-Select 6 Mask Register                         uninit                                              R/W
$0B6*-                  CSCR6       16      Chip-Select 6 Control Register                      uninit *1                                           R/W
$0B8*-                  CSAR7       16      Chip-Select 7 Address Register                      uninit                                              R/W
$0BC*-                  CSMR7       32      Chip-Select 7 Mask Register                         uninit                                              R/W
$0C2*-                  CSCR7       16      Chip-Select 7 Control Register                      uninit *1                                           R/W
$0C6*-                  DMCR        16      Default Memory Control Register                     0000                                                R/W
$0CA*-                  PAR         16      Pin Assignment Register                             00                                                  R/W
--------- TIMER MODULE -----------
$100 √                  TMR1        16      Timer 1 Mode Register                               0000                                                R/W
$104 √                  TRR1        16      Timer 1 Reference Register                          FFFF                                                R/W
$108 √*                 TCR1        16      Timer 1 Capture Register                            0000                                                R
$10C √                  TCN1        16      Timer 1 Counter                                     0000                                                R/W
$111 √                  TER1        8       Timer 1 Event Register                              00                                                  R/W
$120 √                  TMR2        16      Timer 2 Mode Register                               0000                                                R/W
$124 √                  TRR2        16      Timer 2 Reference Register                          FFFF                                                R/W
$128 √*                 TCR2        16      Timer 2 Capture Register                            0000                                                R
$12C √                  TCN2        16      Timer 2 Counter                                     0000                                                R/W
$131 √                  TER2        8       Timer 2 Event Register                              00                                                  R/W
------------ UART SERIAL PORTS  -----------
Using the mc68681 base device driver with the second port removed... not far removed from the actual implementation.
$140 √                  UMR1,2      8       UART 1 Mode Registers                               00                                                  R/W
$144 √                  USR         8       UART 1 Status Register                              00                                                  R
                        UCSR        8       UART 1 Clock-Select Register                        DD                                                  W
$148 √                  UCR         8       UART 1 Command Register                             00                                                  W
$14C √                  URB         8       UART 1 Receive Buffer                               FF                                                  R
                        UTB         8       UART 1 Transmit Buffer                              00                                                  W
$150 √                  UIPCR       8       UART Input Port Change Register                     0F                                                  R
                        UACR        8       UART 1 Auxilary Control Register                    00                                                  W
$154 √                  UISR        8       UART 1 Interrupt Status Register                    00                                                  R
                        UIMR        8       UART 1 Interrupt Mask Register                      00                                                  W
$158 √                  UBG1        8       UART 1 Baud Rate Generator Prescale MSB             uninit                                              W
$15C √                  UBG2        8       UART 1 Baud Rate Generator Prescale LSB             uninit                                              W
$170 √                  UIVR        8       UART 1 Interrupt Vector Register                    0F                                                  R/W
$174 √                  UIP         8       UART 1 Input Port Register                          FF                                                  R
$178 √                  UOP1        8       UART 1 Output Port Bit Set CMD                      UOP1[7-1]=undef; UOP1=0                             W
$17C √                  UOP0        8       UART 1 Output Port Bit Reset CMD                    uninit                                              W

$180 √                  UMR1,2      8       UART 2 Mode Registers                               00                                                  R/W
$184 √                  USR         8       UART 2 Status Register                              00                                                  R
                        UCSR        8       UART 2 Clock-Select Register                        DD                                                  W
$188 √                  UCR         8       UART 2 Command Register                             00                                                  W
$18C √                  URB         8       UART 2 Receive Buffer                               FF                                                  R
                        UTB         8       UART 2 Transmit Buffer                              00                                                  W
$190 √                  UIPCR       8       UART 2 Input Port Change Register                   0F                                                  R
                        UACR        8       UART 2 Auxilary Control Register                    00                                                  W
$194 √                  UISR        8       UART 2 Interrupt Status Register                    00                                                  R
                        UIMR        8       UART 2 Interrupt Mask Register                      00                                                  W
$198 √                  UBG1        8       UART 2 Baud Rate Generator Prescale MSB             uninit                                              R/W
$19C √                  UBG2        8       UART 2 Barud Rate Generator Prescale LSB            uninit                                              R/W
$1B0 √                  UIVR        8       UART 2 Interrupt Vector Register                    0F                                                  R/W
$1B4 √                  UIP         8       UART 2 Input Port Register                          FF                                                  R
$1B8 √                  UOP1        8       UART 2 Output Port Bit Set CMD                      UOP1[7-1]=undef; UOP1=0                             W
$1BC √                  UOP0        8       UART 2 Output Port Bit Reset CMD                    uninit                                              W
------------ GPIO -----------
$1C5 √                  PPDDR       8       Port A Data Direction Register                      00                                                  R/W
$1C9 √                  PPDAT       8       Port A Data Register                                00                                                  R/W
------------ MBUS  -----------
$1E0                    madr        8       M-Bus Address Register                              00                                                  R/W
$1E4*-                  mfdr        8       M-Bus Frequency Divider Register                    00                                                  R/W
$1E8*-                  mbcr        8       M-Bus Control Register                              00                                                  R/W
$1EC*-                  mbsr        8       M-Bus Status Register                               00                                                  R/W
$1F0*-                  mbdr        8       M-Bus Data I/O Register                             00                                                  R/W
------------ DMA Controller -----------
$200                    DMASAR0     32      Source Address Register 0                           00                                                  R/W
$204                    DMADAR0     32      Destination Address Register 0                      00                                                  R/W
$208                    DCR0        16      DMA Control Register 0                              00                                                  R/W
$20C                    BCR0        16      Byte Count Register 0                               00                                                  R/W
$210                    DSR0        8       Status Register 0                                   00                                                  R/W
$214                    DIVR0       8       Interrupt Vector Register 0                         0F                                                  R/W
$240                    DMASAR1     32      Source Address Register 1                           00                                                  R/W
$244                    DMADAR1     32      Destination Address Register 1                      00                                                  R/W
$248                    DCR1        16      DMA Control Register 1                              00                                                  R/W
$24C                    BCR1        16      Byte Count Register 1                               00                                                  R/W
$250                    DSR1        8       Status Register 1                                   00                                                  R/W
$254                    DIVR1       8       Interrupt Vector Register 1                         0F                                                  R/W

*1 - uninit except BRST=ASET=WRAH=RDAH=WR=RD=0

*/
