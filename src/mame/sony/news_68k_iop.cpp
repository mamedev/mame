// license:BSD-3-Clause
// copyright-holders:Brice Onken

/*
 * Sony NEWS first generation dual-processor 68k systems
 *
 * The NWS-800 series was the first model of NEWS workstation. The design was also turned into the NWS-900 series for
 * server use. The later NWS-1800 and NWS-3800 models use the same system design concept as the 800 series (dual-CPU).
 * The NWS-1800/1900 series is especially similar to the NWS-800/900 series in terms of overall design, even though some peripheral
 * chips are different and the custom MMU is not needed because the 1800/1900 design uses the '030.
 *
 * The NWS-800 series uses one '020 as the I/O Processor (IOP). This processor is responsible for booting the system
 * and also runs a small sub-OS (called iopboot) with processes for handling DMA and interrupts for data-intensive peripherals
 * like SCSI, Ethernet, etc. The Main Processor (often referred to, confusingly, as the CPU) runs NEWS-OS (BSD derivative),
 * and uses a block of main memory allocated for CPU<->IOP interprocessor communication to trigger I/O.
 * The CPU and IOP interrupt each other whenever they need to communicate, after populating main memory with whatever data is needed.
 * The CPU delegates almost all I/O to the IOP (as the DMA controller), only handling I/O for some VME bus peripherals.
 * The CPU bus (on the NWS-800, this is routed through the MMU) is often called the Hyperbus. The NWS-1800/1900/3800 series and the single-CPU
 * 68k and R3000 models explictly call this the Hyperbus, but it is unclear if the NWS-800's CPU bus was only referred to that retroactively or not.
 * There are a couple of mentions of the Hyperbus in NEWS-OS 4 related to even the NWS-800, but the NWS-1xxx/3xxx series were already released by the time NEWS-OS 4 was.
 * So, for clarity due to the number of busses on this system, the Hyperbus term is used throughout this code, but it may or may not be a 100% accurate description.
 * The Hyperbus is directly connected to main memory, the system ROM, and the VME bus interface. Everything else is connected to the I/O
 * bus. The IOP, CPU/MMU, and VME bus can all access the Hyperbus through a buffer. As far as I can tell, nothing on the Hyperbus
 * can talk to the I/O bus, only the IOP is able to do that.
 *
 *  Supported:
 *   - NWS-831
 *
 *  Not supported yet:
 *   - All other NWS-8xx workstations
 *   - NWS-9xx servers (2x '020)
 *
 * Known NWS-800 Series Base Configurations
 * - NWS-811: Jun 1987, 4MB RAM, no cache, diskless
 * - NWS-820: Jan 1987, 4MB RAM, ?? cache, 86MB HDD
 * - NWS-821: Aug 1987, 4MB RAM, no cache, 156MB HDD
 * - NWS-830: Jan 1987, 8MB RAM (expandable?), ?? cache, 156MB HDD
 * - NWS-831: Sep 1987, 8MB RAM (up to 16MB), 8KB cache, 156MB HDD
 * - NWS-841: Sep 1987, 8MB RAM (up to 16MB), 8KB cache, 256MB HDD
 * - NWS-891: Apr 1988, 4MB RAM, no cache, 86MB HDD, with CD-ROM drive
 *
 * Known NWS-900 Series Base Configurations:
 * - NWS-911: Feb 1988, 2x 286MB HDD
 * - NWS-921: Feb 1988, 4x 286MB HDD
 *
 * References:
 *  - NWS-1960 Service Guide
 *  - http://bitsavers.org/pdf/sony/news/Sony_NEWS_Technical_Manual_3ed_199103.pdf
 *  - https://katsu.watanabe.name/doc/sonynews/model.html
 *
 * TODO:
 *   - MMU emulation improvements (are all the right status bits set? any missing features? etc)
 *   - Debug general OS issues - random segfaults when running `ps`, system sometimes fails to shutdown when running `shutdown -x now` after using the networking stack, etc.
 *   - AST (Asynchronous System Trap) emulation
 *   - System cache emulation
 *   - Expansion slots (I/O Bus and VMEBus)
 *   - Networking is very flaky, especially on NEWS-OS 4.
 *   - Graphics, kbms, and parallel port emulation
 *   - Hyperbus handshake for IOP and CPU accesses. The bus has arbitration circuitry to prevent bus contention when both the CPU and IOP are trying to access the hyperbus (RAM and VME)
 */

#include "emu.h"

#include "news_020_mmu.h"
#include "news_hid.h"
#include "news_iop_scsi.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/nscsi/tape.h"
#include "bus/rs232/rs232.h"

#include "cpu/m68000/m68020.h"

#include "machine/am79c90.h"
#include "machine/bankdev.h"
#include "machine/msm58321.h"
#include "machine/ncr5380.h"
#include "machine/nscsi_bus.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/timekpr.h"
#include "machine/upd765.h"
#include "machine/z80scc.h"

#include "imagedev/floppy.h"

#include "formats/pc_dsk.h"

#define LOG_INTERRUPT (1U << 1)
#define LOG_ALL_INTERRUPT (1U << 2)
#define LOG_LED (1U << 3)
#define LOG_MEMORY (1U << 4)
#define LOG_PANEL (1U << 5)
#define LOG_MEMORY_ERROR (1U << 6)
#define LOG_TIMER (1U << 7)
#define LOG_SCSI (1U << 8)
#define LOG_AST (1U << 9)
#define LOG_RTC (1U << 10)

#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

namespace
{
	using namespace std::literals::string_view_literals;

	class news_iop_state : public driver_device
	{
	public:
		static constexpr feature_type unemulated_features() { return feature::GRAPHICS; }

		news_iop_state(machine_config const &mconfig, device_type type, char const *tag) :
			driver_device(mconfig, type, tag),
			m_iop(*this, "iop"),
			m_cpu(*this, "cpu"),
			m_mmu(*this, "mmu"),
			m_ram(*this, "ram"),
			m_eprom(*this, "eprom"),
			m_idrom(*this, "idrom"),
			m_rtc(*this, "rtc"),
			m_interval_timer(*this, "interval_timer"),
			m_scc_external(*this, "scc_external"),
			m_scc_peripheral(*this, "scc_peripheral"),
			m_net(*this, "net"),
			m_fdc(*this, "fdc"),
			m_scsi(*this, "scsi:7:am5380"),
			m_scsi_dma(*this, "scsi_dma"),
			m_dip_switch(*this, "FRONT_PANEL"),
			m_serial(*this, "serial%u", 0U),
			iop_irq_line_map({
				{INPUT_LINE_IRQ1, {CPIRQ_3_1}},
				{INPUT_LINE_IRQ2, {SCSI_IRQ, SCSI_DRQ}},
				{INPUT_LINE_IRQ3, {LANCE}},
				{INPUT_LINE_IRQ4, {CPU}},
				{INPUT_LINE_IRQ5, {SCC, SCC_PERIPHERAL}},
				{INPUT_LINE_IRQ6, {TIMEOUT, FDCIRQ}},
				{INPUT_LINE_IRQ7, {FDCDRQ}} }),
			cpu_irq_line_map({
				// AST is excluded from this check
				{INPUT_LINE_IRQ2, CPIRQ1},
				{INPUT_LINE_IRQ3, IOPIRQ3},
				{INPUT_LINE_IRQ4, CPIRQ3},
				{INPUT_LINE_IRQ5, IOPIRQ5},
				{INPUT_LINE_IRQ6, TIMER},
				{INPUT_LINE_IRQ7, PERR} })
		{
		}

		void nws831(machine_config &config) ATTR_COLD;
		void init_common() ATTR_COLD;

	protected:
		// driver_device overrides
		virtual void machine_start() override ATTR_COLD;
		virtual void machine_reset() override ATTR_COLD;

		// address maps
		void iop_map(address_map &map) ATTR_COLD;
		void iop_autovector_map(address_map &map) ATTR_COLD;
		void mmu_map(address_map &map) ATTR_COLD;
		void cpu_map(address_map &map) ATTR_COLD;

		// machine config
		void common(machine_config &config) ATTR_COLD;

		// IOP IRQ setup
		enum iop_irq_number : unsigned
		{
			CPIRQ_3_1,      // Expansion I/O bus and VME bus interrupts
			SCSI_IRQ,       // SCSI IRQ (unmaskable)
			SCSI_DRQ,       // SCSI DRQ (maskable)
			LANCE,          // Ethernet controller interrupts
			CPU,            // Interprocessor communication interrupt (from CPU)
			SCC,            // Serial communication interrupts
			SCC_PERIPHERAL, // IRQ from kb/ms SCC
			TIMEOUT,        // IOP timeout and IRQ from FDC
			FDCIRQ,         // IRQ from FDC (uses same IRQ input as TIMEOUT)
			FDCDRQ          // DRQ from FDC
		};
		static constexpr std::array iop_irq_names = {"CPIRQ"sv, "SCSI_IRQ"sv, "SCSI_DRQ"sv, "LANCE"sv, "CPU"sv, "SCC"sv, "SCC_PERIPHERAL"sv, "TIMEOUT"sv, "FDCIRQ"sv, "FDCDRQ"sv};
		static constexpr uint32_t iop_nmi_mask = (1 << SCSI_IRQ) | (1 << SCC) | (1 << LANCE) | (1 << FDCIRQ) | (1 << FDCDRQ);

		template <iop_irq_number Number>
		void iop_irq_w(int state);
		template <iop_irq_number Number>
		void iop_inten_w(uint8_t state);
		template <iop_irq_number Number>
		bool is_iop_irq_set();
		void int_check_iop();

		// CPU IRQ setup
		enum cpu_irq_number : unsigned
		{
			AST,     // Asynchronous System Trap interrupt
			CPIRQ1,  // VME bus interrupt
			IOPIRQ3, // Low-priority interprocessor communication interrupts (from IOP)
			CPIRQ3,  // VME bus interrupt
			IOPIRQ5, // High-priority interprocessor communication interrupts (from IOP)
			TIMER,   // 100Hz timer interrupt
			PERR     // Parity error interrupt
		};
		static constexpr std::array cpu_irq_names = {"AST"sv, "CPIRQ1"sv, "IOPIRQ3"sv, "CPIRQ3"sv, "IOPIRQ5"sv, "TIMER"sv, "PERR"sv};

		template <cpu_irq_number Number>
		void cpu_irq_w(int state);
		template <cpu_irq_number Number>
		void cpu_inten_w(uint8_t state);
		void int_check_cpu();

		// 68k bus error handlers
		void cpu_bus_error(offs_t offset, uint32_t mem_mask, bool write, uint8_t status);
		void scsi_bus_error(uint8_t data);
		uint32_t extio_bus_error_r(offs_t offset, uint32_t mem_mask);
		void extio_bus_error_w(offs_t offset, uint32_t data, uint32_t mem_mask);
		uint32_t vme_bus_error_r(offs_t offset, uint32_t mem_mask);
		void vme_bus_error_w(offs_t offset, uint32_t data, uint32_t mem_mask);

		// Platform hardware used by the IOP
		uint8_t iop_status_r();
		void iop_romdis_w(uint8_t data);
		void min_w(uint8_t data);
		void motoron_w(uint8_t data);
		void powoff_w(uint8_t data);
		void cpureset_w(uint8_t data);
		uint8_t rtcreg_r();
		void rtcreg_w(uint8_t data);
		void rtcsel_w(uint8_t data);
		void set_rtc_data_bit(uint8_t bit_number, int data);
		uint32_t scsi_dma_r(offs_t offset, uint32_t mem_mask);
		void scsi_dma_w(offs_t offset, uint32_t data, uint32_t mem_mask);
		void scsi_drq_handler(int status);

		// Front panel signal handlers
		void handle_rts(int data);
		void handle_dtr(int data);
		void update_dcd();

		// Platform hardware used by the main processor
		void cpu_romdis_w(uint8_t data);
		void mmuen_w(uint8_t data);
		uint8_t berr_status_r();
		void astreset_w(uint8_t data);
		void astset_w(uint8_t data);

		// CPU timer handlers
		void interval_timer_tick(uint8_t data);
		TIMER_CALLBACK_MEMBER(bus_error_off_cpu);

	private:
		// CPUs (2x 68020, but only the main processor has an FPU)
		required_device<m68020_device> m_iop;    // I/O Processor (BSP, brings up system)
		required_device<m68020fpu_device> m_cpu; // Main Processor

		// Memory devices and main CPU's MMU
		required_device<news_020_mmu_device> m_mmu;
		required_device<ram_device> m_ram;
		required_region_ptr<u32> m_eprom;
		required_region_ptr<u32> m_idrom;

		// Platform hardware
		required_device<msm58321_device> m_rtc;
		required_device<pit8253_device> m_interval_timer;
		required_device<scc8530_device> m_scc_external;
		required_device<scc8530_device> m_scc_peripheral;
		required_device<am7990_device> m_net;
		required_device<upd765a_device> m_fdc;
		required_device<ncr5380_device> m_scsi;
		required_device<news_iop_scsi_helper_device> m_scsi_dma;
		required_ioport m_dip_switch;
		required_device_array<rs232_port_device, 2> m_serial;
		std::unique_ptr<u16[]> m_net_ram;

		// IOP IRQ state
		const std::map<int, std::vector<iop_irq_number>> iop_irq_line_map;
		uint32_t m_iop_intst = 0;
		uint32_t m_iop_inten = 0;

		// CPU IRQ state
		const std::map<int, cpu_irq_number> cpu_irq_line_map;
		uint32_t m_cpu_intst = 0;
		uint32_t m_cpu_inten = 0;

		// Bus error details
		bool m_cpu_bus_error = false;
		offs_t m_last_berr_pc_cpu = 0;
		uint8_t m_cpu_bus_error_status = 0;
		emu_timer *m_cpu_bus_error_timer;

		// Front panel
		bool m_panel_clear = false;
		bool m_panel_shift = false;
		int m_sw1_first_read = 0;
		int m_panel_shift_count = 0;
		uint8_t m_rtc_data = 0;

		// Other constants
		static constexpr int NET_RAM_SIZE = 8192; // 16K RAM, in 16-bit words
	};

	void news_iop_state::machine_start()
	{
		m_cpu_bus_error_timer = timer_alloc(FUNC(news_iop_state::bus_error_off_cpu), this);

		m_net_ram = std::make_unique<u16[]>(NET_RAM_SIZE);
		save_pointer(NAME(m_net_ram), NET_RAM_SIZE);

		m_mmu->space(0).install_ram(0x0, m_ram->mask(), m_ram->pointer());

		// Save state support
		save_item(NAME(m_iop_intst));
		save_item(NAME(m_cpu_intst));
		save_item(NAME(m_iop_inten));
		save_item(NAME(m_cpu_inten));
		save_item(NAME(m_cpu_bus_error));
		save_item(NAME(m_last_berr_pc_cpu));
		save_item(NAME(m_cpu_bus_error_status));
		save_item(NAME(m_panel_clear));
		save_item(NAME(m_panel_shift));
		save_item(NAME(m_sw1_first_read));
		save_item(NAME(m_panel_shift_count));
		save_item(NAME(m_rtc_data));
	}

	void news_iop_state::interval_timer_tick(uint8_t data)
	{
		// TODO: Confirm that channel 0 drives both CPU and IOP
		// On the NWS-1960, the 8253 uses channel 0 for the 100Hz CPU timer, channel 1 for main memory refresh, and channel 2 for the IOP timeout
		// On the 831, using the same assignment breaks things because channel 2 is too fast for the IOP to operate correctly because it gets interrupted far too frequently.
		// The 1850 (same design as 1960) loads channels 0 and 2 with count 0x4e20, whereas the 831 loads channel 0 with 0x4e20 and channel 2 with 0x2.
		// Therefore, because the 831 seems to behave with 0x4e20 driving the IOP as well as the CPU (100Hz), they both hook off of channel 0 for now. However, one or both could be
		// generated elsewhere (or some other trickery is afoot) on the real system, so this needs more investigation in the future.
		// Math: 0x4e20 cycles * 500ns = 10ms period, 1 / 10ms = 100Hz

		iop_irq_w<TIMEOUT>(data > 0);
		cpu_irq_w<TIMER>(data > 0);
	}

	TIMER_CALLBACK_MEMBER(news_iop_state::bus_error_off_cpu)
	{
		m_cpu_bus_error = false;
	}

	uint8_t news_iop_state::iop_status_r()
	{
		// IOP status bits defined as below for the NWS-18xx/19xx series
		// 7: FDC IRQ
		// 6: ~CPIRQ3
		// 5: Main Memory Parity Error Flag
		// 4: ~CPIRQ1 (On the NWS-800, this seems to be SCSI interrupt status?)
		// 3: DSR CHB
		// 2: RI CHB
		// 1: DSR CHA
		// 0: RI CHA

		const uint8_t status = (is_iop_irq_set<SCSI_IRQ>() ? 0x10 : 0) | (is_iop_irq_set<FDCIRQ>() ? 0x80 : 0);
		LOGMASKED(LOG_ALL_INTERRUPT, "Read IOPSTATUS = 0x%x\n", status);
		return status;
	}

	void news_iop_state::iop_romdis_w(uint8_t data)
	{
		LOG("IOP ROMDIS 0x%x\n", data);
		if (data > 0)
		{
			m_iop->space(0).install_ram(0, m_ram->mask(), 0x0, m_ram->pointer());
		}
		else
		{
			m_iop->space(0).install_rom(0x00000000, 0x0000ffff, m_eprom);
		}
		m_panel_shift_count = 0; // hack, clear state from init
	}

	void news_iop_state::min_w(uint8_t data)
	{
		constexpr int HD_RATE = 500000;
		constexpr int DD_RATE = 250000; // TODO: Test DD rate when image is available
		const int rate = (data > 0) ? DD_RATE : HD_RATE; // 0 = HD, 1 = DD

		LOG("Write MIN = 0x%x, set rate to %s (%s)\n", data, rate == HD_RATE ? "HD" : "DD", machine().describe_context());
		m_fdc->set_rate(rate);
	}

	void news_iop_state::motoron_w(uint8_t data)
	{
		LOG("Write MOTORON = 0x%x (%s)\n", data, machine().describe_context());
		m_fdc->subdevice<floppy_connector>("0")->get_device()->mon_w(data & 8 ? 1 : 0);
	}

	void news_iop_state::powoff_w(uint8_t data)
	{
		LOG("Write POWOFF = 0x%x (%s)\n", data, machine().describe_context());

		if (!data && !machine().side_effects_disabled())
		{
			machine().schedule_exit();
		}
	}

	void news_iop_state::cpureset_w(uint8_t data)
	{
		LOG("Write CPURESET = 0x%x\n", data);
		m_cpu->set_input_line(INPUT_LINE_HALT, data ? 0 : 1);
	}

	uint8_t news_iop_state::rtcreg_r()
	{
		m_rtc->cs1_w(1);
		m_rtc->cs2_w(1);

		m_rtc->read_w(1);
		uint8_t result = m_rtc_data;
		m_rtc->read_w(0);
		m_rtc->cs2_w(0);

		LOGMASKED(LOG_RTC, "rtc r 0x%x\n", result);
		return result;
	}

	void news_iop_state::rtcreg_w(uint8_t data)
	{
		LOGMASKED(LOG_RTC, "rtc w 0x%x\n", data);
		m_rtc->cs1_w(1);
		m_rtc->cs2_w(1);

		m_rtc->d0_w((data & 0x1) > 0);
		m_rtc->d1_w((data & 0x2) > 0);
		m_rtc->d2_w((data & 0x4) > 0);
		m_rtc->d3_w((data & 0x8) > 0);

		m_rtc->write_w(1);
		m_rtc->write_w(0);
		m_rtc->cs2_w(0);
	}

	void news_iop_state::rtcsel_w(uint8_t data)
	{
		LOGMASKED(LOG_RTC, "rtc sel w 0x%x\n", data);
		m_rtc->cs1_w(1);
		m_rtc->cs2_w(1);

		m_rtc->d0_w(BIT(data, 0));
		m_rtc->d1_w(BIT(data, 1));
		m_rtc->d2_w(BIT(data, 2));
		m_rtc->d3_w(BIT(data, 3));

		m_rtc->address_write_w(1);
		m_rtc->address_write_w(0);
		m_rtc->cs2_w(0);
	}

	void news_iop_state::set_rtc_data_bit(uint8_t bit_number, int data)
	{
		const uint8_t bit = 1 << bit_number;
		m_rtc_data = (m_rtc_data & ~bit) | (data ? bit : 0x0);
	}

	uint32_t news_iop_state::scsi_dma_r(offs_t offset, uint32_t mem_mask)
	{
		uint32_t result = 0;
		switch (mem_mask)
		{
		case 0xff000000:
			result = m_scsi_dma->read_wrapper(true, 6) << 24;
			break;

		case 0xffff0000:
			result = (m_scsi_dma->read_wrapper(true, 6) << 24) |
					 (m_scsi_dma->read_wrapper(true, 6) << 16);
			break;

		case 0xffffffff:
			result = (m_scsi_dma->read_wrapper(true, 6) << 24) |
					 (m_scsi_dma->read_wrapper(true, 6) << 16) |
					 (m_scsi_dma->read_wrapper(true, 6) << 8) |
					 m_scsi_dma->read_wrapper(true, 6);
			break;

		default:
			fatalerror("scsi_dma_r: unknown mem_mask %08x\n", mem_mask);
		}
		return result;
	}

	void news_iop_state::scsi_dma_w(offs_t offset, uint32_t data, uint32_t mem_mask)
	{
	   switch (mem_mask)
		{
		case 0xff000000:
			m_scsi_dma->write_wrapper(true, 0, data >> 24);
			break;

		case 0xffff0000:
			m_scsi_dma->write_wrapper(true, 0, data >> 24);
			m_scsi_dma->write_wrapper(true, 0, data >> 16);
			break;

		case 0xffffffff:
			m_scsi_dma->write_wrapper(true, 0, data >> 24);
			m_scsi_dma->write_wrapper(true, 0, data >> 16);
			m_scsi_dma->write_wrapper(true, 0, data >> 8);
			m_scsi_dma->write_wrapper(true, 0, data & 0xff);
			break;

		default:
			fatalerror("scsi_dma_w: unknown mem_mask %08x\n", mem_mask);
			break;
		}
	}

	void news_iop_state::scsi_drq_handler(int status)
	{
		LOGMASKED(LOG_SCSI, "SCSI DRQ changed to 0x%x\n", status);
		m_scsi_dma->drq_w(status);
		iop_irq_w<SCSI_DRQ>(status);
	}

	void news_iop_state::machine_reset()
	{
		// Reset memory configuration
		m_iop->space(0).install_rom(0x00000000, 0x0000ffff, m_eprom);

		// CPU doesn't run until the IOP tells it to
		m_cpu->set_input_line(INPUT_LINE_HALT, 1);
	}

	void news_iop_state::init_common()
	{
	}

	void news_iop_state::iop_map(address_map &map)
	{
		map.unmap_value_low();
		// Silence unmapped read/write warnings during memory probe - whatever RAM is present will be installed over top of this region
		map(0x00000000, 0x00ffffff).noprw();

		map(0x03000000, 0x0300ffff).rom().region("eprom", 0).mirror(0x007f0000);

		// IOP bus expansion I/O
		map(0x20000000, 0x20ffffff).rw(FUNC(news_iop_state::extio_bus_error_r),FUNC(news_iop_state::extio_bus_error_w)).mirror(0x1f000000);

		// Expansion slot SCC (bus errors here kill iopboot, so the probe process may not use bus errors, at least not with the same setup as extio)
		// TODO: Does the fact that NEWS-OS 4 poke at these addresses mean that something is not configured properly elsewhere?
		map(0x4c000100, 0x4c0001ff).noprw();

		map(0x60000000, 0x60000000).r(FUNC(news_iop_state::iop_status_r));
		map(0x40000000, 0x40000000).w(FUNC(news_iop_state::iop_inten_w<TIMEOUT>));
		map(0x40000001, 0x40000001).w(FUNC(news_iop_state::min_w));
		map(0x40000002, 0x40000002).w(FUNC(news_iop_state::motoron_w));
		map(0x40000003, 0x40000003).w(FUNC(news_iop_state::iop_inten_w<SCSI_DRQ>));
		map(0x40000004, 0x40000004).w(FUNC(news_iop_state::iop_romdis_w));
		map(0x40000005, 0x40000005).w(FUNC(news_iop_state::powoff_w));
		map(0x40000006, 0x40000006).w(FUNC(news_iop_state::iop_inten_w<CPU>));
		map(0x40000007, 0x40000007).w(FUNC(news_iop_state::cpureset_w));

		map(0x42000000, 0x42000003).rw(m_interval_timer, FUNC(pit8253_device::read), FUNC(pit8253_device::write));

		map(0x44000000, 0x44000003).m(m_fdc, FUNC(upd765a_device::map));
		map(0x44000007, 0x44000007).rw(m_fdc, FUNC(upd765a_device::dma_r), FUNC(upd765a_device::dma_w));

		map(0x46000001, 0x46000001).rw(FUNC(news_iop_state::rtcreg_r), FUNC(news_iop_state::rtcreg_w));
		map(0x46000002, 0x46000002).w(FUNC(news_iop_state::rtcsel_w));

		map(0x4a000000, 0x4a000003).rw(m_scc_peripheral, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)); // kbms
		map(0x4c000000, 0x4c000003).rw(m_scc_external, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w));   // rs232

		map(0x4e000000, 0x4e000007).r(m_scsi, FUNC(ncr5380_device::read));
		map(0x4e000000, 0x4e000007).lrw8(NAME([this] (offs_t offset) {
			return m_scsi_dma->read_wrapper(false, offset);
		}), NAME([this] (offs_t offset, uint8_t data) {
			m_scsi_dma->write_wrapper(false, offset, data);
		}));

		map(0x6a000001, 0x6a000001).lw8(NAME([this] (uint8_t data) { cpu_irq_w<IOPIRQ5>(data > 0); }));
		map(0x6a000002, 0x6a000002).lw8(NAME([this] (uint8_t data) { cpu_irq_w<IOPIRQ3>(data > 0); }));

		map(0x64000000, 0x64000003).rw(FUNC(news_iop_state::scsi_dma_r), FUNC(news_iop_state::scsi_dma_w));

		map(0x66000000, 0x66003fff).lrw16([this] (offs_t offset) { return m_net_ram[offset]; }, "net_ram_r",
										  [this] (offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_net_ram[offset]); }, "net_ram_w");

		map(0x68000000, 0x68000003).rw(m_net, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w));

		map(0x80000000, 0x8001ffff).ram(); // IOP work RAM
	}

	void news_iop_state::cpu_romdis_w(uint8_t data)
	{
		LOG("(%s) CPU ROMDIS 0x%x\n", machine().describe_context(), data);
		m_mmu->set_rom_enable(data == 0);
	}

	void news_iop_state::mmuen_w(uint8_t data)
	{
		LOGMASKED(LOG_MEMORY, "(%s) CPU MMUEN 0x%x\n", machine().describe_context(), data);
		const bool mmu_enable = data > 0;
		m_cpu->set_emmu_enable(mmu_enable);
		m_mmu->set_mmu_enable(mmu_enable);
	}

	uint8_t news_iop_state::berr_status_r()
	{
		LOGMASKED(LOG_MEMORY, "(%s) BERR_STATUS read = 0x%x\n", machine().describe_context(), m_cpu_bus_error_status);
		uint8_t status = m_cpu_bus_error_status;
		m_cpu_bus_error_status = 0;
		m_last_berr_pc_cpu = 0;
		return status;
	}

	// TODO: implement AST (Asynchronous System Trap)
	void news_iop_state::astreset_w(uint8_t data)
	{
		LOGMASKED(LOG_AST, "(%s) AST_RESET 0x%x\n", machine().describe_context(), data);
	}

	void news_iop_state::astset_w(uint8_t data)
	{
		LOGMASKED(LOG_AST, "(%s) AST_SET 0x%x\n", machine().describe_context(), data);
	}

	void news_iop_state::mmu_map(address_map &map)
	{
		map(0x03000000, 0x0300ffff).rom().region("eprom", 0).mirror(0x007f0000);

		// VME bus
		map(0x03900000, 0x039fffff).rw(FUNC(news_iop_state::vme_bus_error_r), FUNC(news_iop_state::vme_bus_error_w)); // TODO: full region start/end

		// Various platform control registers (MMU and otherwise)
		map(0x04400000, 0x04400000).w(FUNC(news_iop_state::cpu_romdis_w));
		map(0x04400001, 0x04400001).w(FUNC(news_iop_state::mmuen_w));
		map(0x04400002, 0x04400002).w(FUNC(news_iop_state::cpu_inten_w<IOPIRQ3>));
		map(0x04400003, 0x04400003).w(FUNC(news_iop_state::cpu_inten_w<IOPIRQ5>));
		map(0x04400004, 0x04400004).w(FUNC(news_iop_state::cpu_inten_w<TIMER>));
		map(0x04400005, 0x04400005).lw8([this] (uint8_t data)
										{
											LOGMASKED(LOG_MEMORY, "(%s) Write CACHEEN = 0x%x\n", machine().describe_context(), data);
										}, "CACHEEN");
		map(0x04400006, 0x04400006).w(FUNC(news_iop_state::cpu_inten_w<PERR>));
		map(0x04800000, 0x04800000).lw8([this] (uint8_t data) { iop_irq_w<CPU>(1); }, "INT_IOP");
		map(0x04c00000, 0x04c00000).select(0x10000000).w(m_mmu, FUNC(news_020_mmu_device::clear_entries));

		map(0x05000000, 0x05000000).select(0x400000).lw8([this] (offs_t offset, uint8_t data)
														 {
															 LOGMASKED(LOG_MEMORY, "%s cache clear = 0x%x (%s)\n", (offset & 0x400000) ? "system" : "user", data, machine().describe_context());
														 }, "CACHE_CLR");
		map(0x05800000, 0x05800000).w(FUNC(news_iop_state::astreset_w));
		map(0x05800001, 0x05800001).w(FUNC(news_iop_state::astset_w));
		map(0x05c00000, 0x05c00000).r(FUNC(news_iop_state::berr_status_r));
		map(0x06000000, 0x061fffff).rw(m_mmu, FUNC(news_020_mmu_device::mmu_entry_r), FUNC(news_020_mmu_device::mmu_entry_w)).select(0x10000000);
	}

	void news_iop_state::cpu_map(address_map &map)
	{
		map(0x0, 0xffffffff).lrw32(
				[this] (offs_t offset, uint32_t mem_mask) {
					return m_mmu->hyperbus_r(offset, mem_mask, m_cpu->supervisor_mode());
				}, "hyperbus_r",
				[this] (offs_t offset, uint32_t data, uint32_t mem_mask) {
					m_mmu->hyperbus_w(offset, data, mem_mask, m_cpu->supervisor_mode());
				}, "hyperbus_w");
	}

	template <news_iop_state::iop_irq_number Number>
	bool news_iop_state::is_iop_irq_set()
	{
		return BIT(m_iop_intst, Number);
	}

	template <news_iop_state::iop_irq_number Number>
	void news_iop_state::iop_irq_w(int state)
	{
		if (Number != TIMEOUT)
		{
			LOGMASKED(LOG_INTERRUPT, "(%s) IOP IRQ %s %s\n", machine().describe_context(), iop_irq_names[Number], state ? "set" : "cleared");
		}

		if (state)
		{
			m_iop_intst |= 1U << Number;
		}
		else
		{
			m_iop_intst &= ~(1U << Number);
		}

		int_check_iop();
	}

	template <news_iop_state::iop_irq_number Number>
	void news_iop_state::iop_inten_w(uint8_t state)
	{
		if (Number != TIMEOUT)
		{
			LOGMASKED(LOG_INTERRUPT, "(%s) IOP IRQ %s %s\n", machine().describe_context(), iop_irq_names[Number], state ? "enabled" : "disabled");
		}

		if (state)
		{
			m_iop_inten |= (1 << Number);
		}
		else
		{
			m_iop_inten &= ~(1 << Number);
			m_iop_intst &= ~(1 << Number); // TODO: does this actually clear in all cases?
		}

		int_check_iop();
	}

	void news_iop_state::int_check_iop()
	{
	   const int active_irq = m_iop_intst & (m_iop_inten | iop_nmi_mask);
	   for (auto irq : iop_irq_line_map)
	   {
			// Calculate state of input pin (logical OR of all attached inputs)
			bool state = false;
			for (auto irq_input : irq.second)
			{
				state |= active_irq & (1 << irq_input);
			}

			// Update input pin status if it has changed
			if (m_iop->input_state(irq.first) != state) {
				if (irq.first != INPUT_LINE_IRQ6)
				{
					LOGMASKED(LOG_INTERRUPT, "Setting IOP input line %d to %d\n", irq.first, state ? 1 : 0);
				}

				m_iop->set_input_line(irq.first, state ? 1 : 0);
			}
	   }
	}

	template <news_iop_state::cpu_irq_number Number>
	void news_iop_state::cpu_irq_w(int state)
	{
		if (Number != TIMER)
		{
			LOGMASKED(LOG_INTERRUPT, "(%s) CPU IRQ %s %s\n", machine().describe_context(), cpu_irq_names[Number], state ? "set" : "cleared");
		}

		if (state)
		{
			m_cpu_intst |= 1U << Number;
		}
		else
		{
			m_cpu_intst &= ~(1U << Number);
		}

		int_check_cpu();
	}

	template <news_iop_state::cpu_irq_number Number>
	void news_iop_state::cpu_inten_w(uint8_t state)
	{
		if (Number != TIMER)
		{
			LOGMASKED(LOG_INTERRUPT, "(%s) CPU IRQ %s %s\n", machine().describe_context(), cpu_irq_names[Number], state ? "enabled" : "disabled");
		}

		if (state)
		{
			m_cpu_inten |= (1 << Number);
		}
		else
		{
			m_cpu_inten &= ~(1 << Number);
			m_cpu_intst &= ~(1 << Number); // TODO: does this actually clear in all cases?
		}

		int_check_cpu();
	}

	void news_iop_state::int_check_cpu()
	{
		const int active_irq = m_cpu_intst & m_cpu_inten;
		for (auto irq : cpu_irq_line_map)
		{
			// Update input pin status if it has changed
			const bool state = BIT(active_irq, irq.second);
			if (m_cpu->input_state(irq.first) != state)
			{
				if (irq.first != INPUT_LINE_IRQ6)
				{
					LOGMASKED(LOG_INTERRUPT, "Setting CPU input line %d to %d\n", irq.first, state ? 1 : 0);
				}

				m_cpu->set_input_line(irq.first, state ? 1 : 0);
			}
		}
	}

	static void news_scsi_devices(device_slot_interface &device)
	{
		device.option_add("harddisk", NSCSI_HARDDISK);
		device.option_add("cdrom", NSCSI_CDROM);
		device.option_add("tape", NSCSI_TAPE);
	}

	void news_iop_state::handle_rts(int data)
	{
		if (m_panel_shift != !data)
		{
			m_panel_shift = !data;

			if (!m_panel_clear && data) // falling edge?
			{
				// Count not forced to 0 by the clear pin
				++m_panel_shift_count;
			}
		}
		LOGMASKED(LOG_PANEL, "panel I/O rts = %d new count = 0x%x (%s)\n", !data, m_panel_shift_count, machine().describe_context());
		update_dcd();
	}

	void news_iop_state::handle_dtr(int data)
	{
		if (m_panel_clear != data)
		{
			LOGMASKED(LOG_PANEL, "panel I/O dtr = %d (%s)\n", data, machine().describe_context());
			m_panel_clear = data;
			if (data)
			{
				m_panel_shift_count = 0;
			}
		}

		update_dcd();
	}

	void news_iop_state::update_dcd()
	{
		// the shift pin and the counter drive a multiplexer (the shift pin also drives the counter)
		uint8_t multiplexer_value = (m_panel_shift_count & 0x3) << 1;
		multiplexer_value |= (m_panel_shift ? 0x1 : 0x0);

		bool dcd_state = false;
		if (multiplexer_value < 6) // DIPSW
		{
			// The reset condition (somehow) seems to start the multiplexer count at 5. This is a workaround to achieve the same results.
			// This seems to be correct because it causes diag mode to start when SW6 is set, and also allows the NWB-512 init code to start
			// when SW1 is set. When framebuffer emulation is added, this can be confirmed (maybe also with an oscilloscope readout?)
			// The 2 is there because the reset condition of the emulated pins results in an extra reset.
			if ((m_sw1_first_read < 2) && multiplexer_value == 0)
			{
				m_sw1_first_read++;
				dcd_state = BIT(m_dip_switch->read(), 5);
			}
			else
			{
				dcd_state = BIT(m_dip_switch->read(), multiplexer_value);
			}
		}
		else // IDROM
		{
			// Step 1: Get IDROM word address and byte offset
			const uint8_t idrom_idx = m_panel_shift_count >> 4;
			const uint8_t offset = (m_panel_shift_count >> 2) % 4;

			// Step 2: Get target data byte
			const uint8_t idrom_data = m_idrom[idrom_idx] >> ((3 - offset) * 8);

			// Step 3: Apply bit mask to get one of two data bits from the byte-encoded nibble and set DCD
			dcd_state = idrom_data & (multiplexer_value == 6 ? 0x1 : 0x2);
			LOGMASKED(LOG_PANEL, "idrom d0 [0x%02x] -> 0x%02x\n", idrom_idx, dcd_state);
		}
		LOGMASKED(LOG_PANEL, "mplex 0x%04x cnt 0x%04x set dcd = %d\n", multiplexer_value, m_panel_shift_count, dcd_state);
		m_scc_peripheral->dcdb_w(dcd_state);
	}

	void news_iop_state::cpu_bus_error(offs_t offset, uint32_t mem_mask, bool write, uint8_t status)
	{
		if (machine().side_effects_disabled() || (m_cpu_bus_error && m_cpu->pc() == m_last_berr_pc_cpu))
		{
			LOGMASKED(LOG_MEMORY, "Suppressing cpu bus error at offset 0x%x mask 0x%x r/w %s status 0x%x pc 0x%x", offset, mem_mask, write ? "w" : "r", status, m_cpu->pc());
			return;
		}

		LOGMASKED(LOG_MEMORY, "Applying cpu bus error at offset 0x%x mask 0x%x r/w %s status 0x%x pc 0x%x\n", offset, mem_mask, write ? "w" : "r", status, m_cpu->pc());

		m_last_berr_pc_cpu = m_cpu->pc();
		m_cpu_bus_error = true;
		m_cpu_bus_error_status = status;
		m_cpu->set_buserror_details(offset, !write, m_cpu->get_fc(), true);
		m_cpu_bus_error_timer->adjust(m_cpu->cycles_to_attotime(16)); // TODO: 16 is what hcpu30 uses for a similar implementation - is that OK for NEWS?
	}

	void news_iop_state::scsi_bus_error(uint8_t data)
	{
		if (!machine().side_effects_disabled())
		{
			m_iop->set_buserror_details(0x64000000, data == news_iop_scsi_helper_device::READ_ERROR, m_iop->get_fc());
			m_iop->pulse_input_line(M68K_LINE_BUSERROR, attotime::zero);
		}
	}

	uint32_t news_iop_state::extio_bus_error_r(offs_t offset, uint32_t mem_mask)
	{
		if (!machine().side_effects_disabled())
		{
			LOG("extio_bus_error_r(0x%x, 0x%x) -> 0x%x = 0x%x (%s)\n", offset, mem_mask, 0x20000000 + offset, 0xff, machine().describe_context());
			m_iop->set_buserror_details(0x20000000 + offset, 1, m_iop->get_fc());
			m_iop->pulse_input_line(M68K_LINE_BUSERROR, attotime::zero);
		}
		return 0xff;
	}

	void news_iop_state::extio_bus_error_w(offs_t offset, uint32_t data, uint32_t mem_mask)
	{
		if (!machine().side_effects_disabled())
		{
			LOG("extio_bus_error_w(0x%x, 0x%x, 0x%x) -> 0x%x = (%s)\n", offset, data, mem_mask, 0x20000000 + offset, machine().describe_context());
			m_iop->set_buserror_details(0x20000000 + offset, 0, m_iop->get_fc());
			m_iop->pulse_input_line(M68K_LINE_BUSERROR, attotime::zero);
		}
	}

	uint32_t news_iop_state::vme_bus_error_r(offs_t offset, uint32_t mem_mask)
	{
		if (!machine().side_effects_disabled())
		{
			// TODO: this might be more accurate with the full cpu bus error func and setting the status to HB_BERR
			LOG("vme_r(0x%x, 0x%x) -> 0x%x = 0x%x (%s)\n", offset, mem_mask, 0x03900000 + offset, 0xff, machine().describe_context());
			m_cpu->set_buserror_details(0x03900000 + offset, 1, m_cpu->get_fc());
			m_cpu->pulse_input_line(M68K_LINE_BUSERROR, attotime::zero);
		}
		return 0xff;
	}

	void news_iop_state::vme_bus_error_w(offs_t offset, uint32_t data, uint32_t mem_mask)
	{
		if (!machine().side_effects_disabled())
		{
			LOG("vme_w(0x%x, 0x%x, 0x%x) -> 0x%x = (%s)\n", offset, data, mem_mask, 0x03900000 + offset, machine().describe_context());
			m_cpu->set_buserror_details(0x03900000 + offset, 0, m_cpu->get_fc());
			m_cpu->pulse_input_line(M68K_LINE_BUSERROR, attotime::zero);
		}
	}

	void news_iop_state::common(machine_config &config)
	{
		M68020(config, m_iop, 16.67_MHz_XTAL); // TODO: this might come from a 33.3333MHz crystal divided by two
		m_iop->set_addrmap(AS_PROGRAM, &news_iop_state::iop_map);
		m_iop->set_addrmap(m68000_base_device::AS_CPU_SPACE, &news_iop_state::iop_autovector_map);

		M68020FPU(config, m_cpu, 16.67_MHz_XTAL);
		m_cpu->set_addrmap(AS_PROGRAM, &news_iop_state::cpu_map);

		NEWS_020_MMU(config, m_mmu, 0);
		m_mmu->set_addrmap(AS_PROGRAM, &news_iop_state::mmu_map);
		m_mmu->set_bus_error_callback(FUNC(news_iop_state::cpu_bus_error));

		RAM(config, m_ram);
		m_ram->set_default_size("8M");
		m_ram->set_extra_options("16M");
		m_ram->set_default_value(0);

		// NEC uPD8253 programmable interval timer
		PIT8253(config, m_interval_timer, 0);
		constexpr XTAL PIT_INPUT_FREQUENCY = XTAL(2'000'000); // Assume same as 1960 for now
		m_interval_timer->set_clk<0>(PIT_INPUT_FREQUENCY);
		m_interval_timer->set_clk<1>(PIT_INPUT_FREQUENCY);
		m_interval_timer->set_clk<2>(PIT_INPUT_FREQUENCY);
		m_interval_timer->out_handler<0>().set(FUNC(news_iop_state::interval_timer_tick));

		// 2x Sharp LH8530A Z8530A-SCC
		SCC8530(config, m_scc_external, (16_MHz_XTAL / 4));
		SCC8530(config, m_scc_peripheral, (16_MHz_XTAL / 4));
		m_scc_external->out_int_callback().set(FUNC(news_iop_state::iop_irq_w<SCC>));
		m_scc_peripheral->out_int_callback().set(FUNC(news_iop_state::iop_irq_w<SCC_PERIPHERAL>));

		// The monitor ROM repurposes the status pins as a serial interface to read the front panel switches and IDROM data
		m_scc_peripheral->out_dtrb_callback().set(FUNC(news_iop_state::handle_dtr));
		m_scc_peripheral->out_rtsb_callback().set(FUNC(news_iop_state::handle_rts));

		// scc channel A
		RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
		m_serial[0]->cts_handler().set(m_scc_external, FUNC(z80scc_device::ctsa_w));
		m_serial[0]->dcd_handler().set(m_scc_external, FUNC(z80scc_device::dcda_w));
		m_serial[0]->rxd_handler().set(m_scc_external, FUNC(z80scc_device::rxa_w));
		m_scc_external->out_rtsa_callback().set(m_serial[0], FUNC(rs232_port_device::write_rts));
		m_scc_external->out_txda_callback().set(m_serial[0], FUNC(rs232_port_device::write_txd));

		// scc channel B
		RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
		m_serial[1]->cts_handler().set(m_scc_external, FUNC(z80scc_device::ctsb_w));
		m_serial[1]->dcd_handler().set(m_scc_external, FUNC(z80scc_device::dcdb_w));
		m_serial[1]->rxd_handler().set(m_scc_external, FUNC(z80scc_device::rxb_w));
		m_scc_external->out_rtsb_callback().set(m_serial[1], FUNC(rs232_port_device::write_rts));
		m_scc_external->out_txdb_callback().set(m_serial[1], FUNC(rs232_port_device::write_txd));

		// LANCE ethernet controller
		AM7990(config, m_net, 20_MHz_XTAL);
		m_net->intr_out().set(FUNC(news_iop_state::iop_irq_w<LANCE>)).invert();
		m_net->dma_in().set([this] (offs_t offset)
							{ return m_net_ram[(offset >> 1) & 0x1fff]; });
		m_net->dma_out().set([this] (offs_t offset, u16 data, u16 mem_mask)
							 { COMBINE_DATA(&m_net_ram[(offset >> 1) & 0x1fff]); });

		// uPD7265 FDC (Compatible with 765A except it should use Sony/ECMA format by default?)
		UPD765A(config, m_fdc, 4'000'000); // TODO: confirm clock rate
		m_fdc->intrq_wr_callback().set(FUNC(news_iop_state::iop_irq_w<FDCIRQ>));
		m_fdc->drq_wr_callback().set(FUNC(news_iop_state::iop_irq_w<FDCDRQ>));
		FLOPPY_CONNECTOR(config, "fdc:0", "35hd", FLOPPY_35_HD, true, floppy_image_device::default_pc_floppy_formats).enable_sound(false);

		// SCSI bus and devices
		// Most NEWS workstations generally follow this convention (when each given item is present):
		// ID 0-2: Internal hard drive(s)
		// ID 3: Free
		// ID 4: Internal MO drive
		// ID 5: Internal tape drive
		// ID 6: Internal CD-ROM drive
		// ID 7: NEWS (the workstation itself, SCSI initiator)
		// Most Sony tools can be configured to use any ID for anything, but the defaults will generally follow the above
		// So, even for expansion MO, tape drives, etc. it is usually easiest to use the above IDs for assignment
		// Early (pre-OS-3) bootloaders are extremely picky about IDNT data and reported capacity; ensure you are using IDNT data and the matching size reported
		// from an actual CDC drive if dealing with early versions.
		// Note: Only the NWS-891 came with a CD-ROM as a default option, others required an external CD-ROM drive
		NSCSI_BUS(config, "scsi");
		NSCSI_CONNECTOR(config, "scsi:0", news_scsi_devices, "harddisk");
		NSCSI_CONNECTOR(config, "scsi:1", news_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi:2", news_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi:3", news_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi:4", news_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi:5", news_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi:6", news_scsi_devices, nullptr);

		// AMD Am5380PC SCSI interface
		NSCSI_CONNECTOR(config, "scsi:7").option_set("am5380", NCR5380).machine_config([this] (device_t *device)
		{
			ncr5380_device &adapter = downcast<ncr5380_device &>(*device);
			adapter.irq_handler().set([this] (int state){ m_scsi_dma->irq_w(state); });
			adapter.drq_handler().set(*this, FUNC(news_iop_state::scsi_drq_handler));
		});

		// Virtual device for handling SCSI DMA
		// (workaround for 68k bus access limitiations; normally iopboot's BERR handler would deal with SCSI DMA stuff)
		NEWS_IOP_SCSI_HELPER(config, m_scsi_dma);
		m_scsi_dma->scsi_read_callback().set(m_scsi, FUNC(ncr53c80_device::read));
		m_scsi_dma->scsi_write_callback().set(m_scsi, FUNC(ncr53c80_device::write));
		m_scsi_dma->scsi_dma_read_callback().set(m_scsi, FUNC(ncr53c80_device::dma_r));
		m_scsi_dma->scsi_dma_write_callback().set(m_scsi, FUNC(ncr53c80_device::dma_w));
		m_scsi_dma->iop_halt_callback().set_inputline(m_iop, INPUT_LINE_HALT);
		m_scsi_dma->bus_error_callback().set(FUNC(news_iop_state::scsi_bus_error));
		m_scsi_dma->irq_out_callback().set(FUNC(news_iop_state::iop_irq_w<SCSI_IRQ>));

		// Epson RTC-58321B
		MSM58321(config, m_rtc, 32.768_kHz_XTAL);
		m_rtc->d0_handler().set([this] (int data) { set_rtc_data_bit(0, data); });
		m_rtc->d1_handler().set([this] (int data) { set_rtc_data_bit(1, data); });
		m_rtc->d2_handler().set([this] (int data) { set_rtc_data_bit(2, data); });
		m_rtc->d3_handler().set([this] (int data) { set_rtc_data_bit(3, data); });
	}

	void news_iop_state::iop_autovector_map(address_map &map)
	{
		map(0xfffffff3, 0xfffffff3).lr8(NAME([] ()
											 { return m68000_base_device::autovector(1); }));
		map(0xfffffff5, 0xfffffff5).lr8(NAME([] ()
											 { return m68000_base_device::autovector(2); }));
		map(0xfffffff7, 0xfffffff7).lr8(NAME([] ()
											 { return m68000_base_device::autovector(3); }));
		map(0xfffffff9, 0xfffffff9).lr8(NAME([] ()
											 { return m68000_base_device::autovector(4); }));
		map(0xfffffffb, 0xfffffffb).lr8(NAME([this] ()
											 {
												uint8_t vector = m68000_base_device::autovector(5);
												// TODO: serial port vs peripheral SCC vector priority?
												if (is_iop_irq_set<SCC>())
												{
													vector = m_scc_external->m1_r();
												}
												else if (is_iop_irq_set<SCC_PERIPHERAL>())
												{
													vector = m_scc_peripheral->m1_r();
												}
												return vector;
											 }));
		map(0xfffffffd, 0xfffffffd).lr8(NAME([] ()
											 { return m68000_base_device::autovector(6); }));
		map(0xffffffff, 0xffffffff).lr8(NAME([] ()
											 { return m68000_base_device::autovector(7); }));
	}

	void news_iop_state::nws831(machine_config &config)
	{
		common(config);
	}

	static INPUT_PORTS_START(nws8xx)
		PORT_START("FRONT_PANEL")
			PORT_DIPNAME(0x07, 0x07, "Display")
				PORT_DIPLOCATION("FRONT_PANEL:1,2,3")
				PORT_DIPSETTING(0x07, "Console")
				PORT_DIPSETTING(0x06, "NWB-512")
				PORT_DIPSETTING(0x03, "NWB-225A")
				PORT_DIPSETTING(0x00, "Autoselect")
			PORT_DIPNAME(0x08, 0x08, "Boot Device")
				PORT_DIPLOCATION("FRONT_PANEL:4")
				PORT_DIPSETTING(0x08, "SCSI")
				PORT_DIPSETTING(0x00, "Floppy")
			PORT_DIPNAME(0x10, 0x10, "Automatic Boot")
				PORT_DIPLOCATION("FRONT_PANEL:5")
				PORT_DIPSETTING(0x10, DEF_STR(Off))
				PORT_DIPSETTING(0x00, DEF_STR(On))
			PORT_DIPNAME(0x20, 0x20, "Diagnostic Mode")
				PORT_DIPLOCATION("FRONT_PANEL:6")
				PORT_DIPSETTING(0x20, DEF_STR(Off))
				PORT_DIPSETTING(0x00, DEF_STR(On))
			PORT_DIPUNUSED_DIPLOC(0xc0, 0xc0, "FRONT_PANEL:7,8")
	INPUT_PORTS_END

	ROM_START(nws831)
		// 2x Fujitsu MBM27C256-25 EPROMs
		ROM_REGION32_BE(0x10000, "eprom", 0)
		ROM_SYSTEM_BIOS(0, "nws831", "SONY NET WORK STATION monitor Release 2.1")
		ROM_LOAD16_BYTE("nws831-mbm27c256-ver2p1-h.bin", 0x0000, 0x8000, CRC(11e23140) SHA1(8c87a6198f918a69f611ff7be6497552ead67b31))
		ROM_LOAD16_BYTE("nws831-mbm27c256-ver2p1-l.bin", 0x0001, 0x8000, CRC(1ea72294) SHA1(e7edfad08e46b7dbcca5279457901ba83fbf481f))

		// Fujitsu MB7114L 256 x 4 bit (1K) TTL PROM
		// There is no golden dump of this PROM because it is holds per-system values
		// The front panel addresses the idrom and only uses the bottom two bits of each nibble.
		// The upper two bits per nibble are programmed to ones on my NWS-831
		ROM_REGION32_BE(0x100, "idrom", 0)
		ROM_LOAD("nws831-idrom.bin", 0x0, 0x100, CRC(167de13d) SHA1(3e46392671324e92f1e34ddd8ac3c6d94144b3d2) BAD_DUMP)
	ROM_END

} // anonymous namespace

// Machine definitions
//   YEAR  NAME    P  C  MACHINE INPUT   CLASS           INIT         COMPANY FULLNAME   FLAGS
COMP(1987, nws831, 0, 0, nws831, nws8xx, news_iop_state, init_common, "Sony", "NWS-831", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
