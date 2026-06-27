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
#include "news_iop_scsi.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/nscsi/tape.h"
#include "bus/rs232/rs232.h"

#include "cpu/m68000/m68020.h"

#include "machine/am79c90.h"
#include "machine/msm58321.h"
#include "machine/ncr5380.h"
#include "machine/nscsi_bus.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "machine/z80scc.h"

#include "imagedev/floppy.h"

#include "screen.h"

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
#define LOG_FB (1U << 11)

#define VERBOSE (LOG_FB)

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
			m_krom(*this, "krom"),
			m_rtc(*this, "rtc"),
			m_interval_timer(*this, "interval_timer"),
			m_scc_external(*this, "scc_external"),
			m_scc_peripheral(*this, "scc_peripheral"),
			m_net(*this, "net"),
			m_fdc(*this, "fdc"),
			m_scsi(*this, "am5380"),
			m_scsi_dma(*this, "scsi_dma"),
			m_dip_switch(*this, "FRONT_PANEL"),
			m_serial(*this, "serial%u", 0U),
			m_screen(*this, "nwp512d"),
			m_vram(*this, "vram"),
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
		[[maybe_unused]] uint32_t extio_bus_error_r(offs_t offset, uint32_t mem_mask);
		[[maybe_unused]] void extio_bus_error_w(offs_t offset, uint32_t data, uint32_t mem_mask);
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

		// Framebuffer functions
		u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);
		[[maybe_unused]] void fb_reset();
		[[maybe_unused]] void destination_w(offs_t offset, uint16_t data, uint16_t mem_mask);
		[[maybe_unused]] void source_w(offs_t offset, uint16_t data, uint16_t mem_mask);
		[[maybe_unused]] void limit_w(offs_t offset, uint16_t data, uint16_t mem_mask);
		[[maybe_unused]] void shift_w(offs_t offset, uint16_t data, uint16_t mem_mask);
		[[maybe_unused]] void rop_config_w(offs_t offset, uint16_t data, uint16_t mem_mask);
		[[maybe_unused]] void pixel_w(offs_t offset, uint16_t data, uint16_t mem_mask);
		[[maybe_unused]] void check_operation_start(offs_t offset);
		[[maybe_unused]] void fb_fifo_w(offs_t offset, uint16_t data, uint16_t mem_mask);
		[[maybe_unused]] void fb_set_pixel(uint16_t x, uint16_t y, bool on);
		[[maybe_unused]] uint16_t execute_rop(uint8_t opcode, uint16_t source, uint16_t destination);
		[[maybe_unused]] void fb_rop_cycle(uint16_t data);

		// [[maybe_unused]] void fb_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	private:
		// CPUs (2x 68020, but only the main processor has an FPU)
		required_device<m68020_device> m_iop;    // I/O Processor (BSP, brings up system)
		required_device<m68020fpu_device> m_cpu; // Main Processor

		// Memory devices and main CPU's MMU
		required_device<news_020_mmu_device> m_mmu;
		required_device<ram_device> m_ram;
		required_region_ptr<u32> m_eprom;
		required_region_ptr<u32> m_idrom;
		required_region_ptr<u16> m_krom;

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

		// Framebuffer and CRT
		required_device<screen_device> m_screen; // NWP-512D 15" Monochrome Display
		required_device<ram_device> m_vram;

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
		emu_timer *m_cpu_bus_error_timer = nullptr;

		// Front panel
		bool m_panel_clear = false;
		bool m_panel_shift = false;
		int m_panel_shift_count = 0;
		uint8_t m_rtc_data = 0;

		// Other constants
		static constexpr int NET_RAM_SIZE = 8192; // 16K RAM, in 16-bit words

		// Framebuffer state
		// CXD1029: raster operation engine
		// ROCC - Raster Operation Cycle Controller, state machine for framebuffer
		// Where the NWS-1500 service manual uses specific names for signals, they are noted below.
		// Technically, they may not have used this terminology on this framebuffer.
		[[maybe_unused]] uint8_t m_fb_status = 0;
		[[maybe_unused]] uint8_t m_fb_ctrl = 0;
		[[maybe_unused]] uint16_t m_fb_destination_x = 0; // DRXA
		[[maybe_unused]] uint16_t m_fb_current_x = 0; // offset from dst/src, DXA
		[[maybe_unused]] uint16_t m_fb_destination_y = 0; // DYA
		[[maybe_unused]] uint16_t m_fb_source_x = 0; // SRXA
		[[maybe_unused]] uint16_t m_fb_source_y = 0; // SYA
		[[maybe_unused]] uint16_t m_fb_limit_x = 0; // LXA
		[[maybe_unused]] uint16_t m_fb_limit_y = 0; // YC
		[[maybe_unused]] uint16_t m_fb_shift = 0;
		[[maybe_unused]] uint16_t m_fb_carry = 0; // leftover bits after shifting
		[[maybe_unused]] uint8_t m_fb_rop_func = 0;
		[[maybe_unused]] bool m_fb_suppress_advance = false; // false = auto-increment after rop
		[[maybe_unused]] bool m_fb_x_direction = false; // true = top -> bottom
		[[maybe_unused]] bool m_fb_y_direction = false; // true = left -> right
		[[maybe_unused]] bool m_fb_row_fill = false; // true = write data fifo word across whole line
		[[maybe_unused]] bool m_fb_read = false; // Tri-state with write - read = fb -> host, write = host -> fb, both = fb -> fb copy
		[[maybe_unused]] bool m_fb_write = false;
		[[maybe_unused]] bool m_fb_busy = false; // operation in progress

		[[maybe_unused]] bool m_fb_mask1_armed = false; // at rectangle edge 1
		[[maybe_unused]] uint16_t m_fb_mask1 = 0;
		[[maybe_unused]] uint16_t m_fb_mask2 = 0;
		[[maybe_unused]] uint16_t m_fb_op_counter = 0;
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
		m_cpu->set_emmu_enable(false);
		m_mmu->set_mmu_enable(false);
		m_mmu->set_rom_enable(true);

		// Reset internal framebuffer state
		fb_reset();
	}

	void news_iop_state::init_common()
	{
	}

	u32 news_iop_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
	{
		// uint8_t const *pixel_pointer = m_vram->pointer();
		static int log = 0;

		// TODO: optimize this to read one byte at a time rather than bit
		for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
		{
			for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x++) // make this 1024 instead of max_x to view offscreen area
			{
				const uint32_t byte_offset = y * 128 + (x >> 3); // Y: 1024 / 8 = 128 bytes per row
				const uint16_t x_bit = 7 - (x & 7); // pixel 0 = bit 7

				// Set desired pixel
				uint8_t pixel = m_vram->read(byte_offset);
				pixel = pixel & (1u << x_bit);
				bitmap.pix(y, x) = pixel ? rgb_t::white() : rgb_t::black();

				if (bitmap.pix(y,x) != rgb_t::black())
				{
					log = 1;
				}

				if (log > 0 && log < 1024 * 1024)
				{
					++log;
					// LOGMASKED(LOG_FB, "y = %d x = %d pixel = 0x%x, bitmap = %d, byte offset = 0x%x\n", y, x, pixel, bitmap.pix(y, x), byte_offset);
				}
			}
		}

		return 0;
	}

	void news_iop_state::fb_reset()
	{
		m_fb_busy = false;
	}

	uint16_t x_address(uint16_t data)
	{
		return data & 0xfff; // When X is supplied, it is in D11-D0
	}

	uint16_t y_address(uint16_t data) // word offset
	{
		return data & 0xfff; // When Y is supplied, it is in A12-A1, but is already shifted down.
	}

	void check_mem_mask(uint16_t mem_mask)
	{
		if (mem_mask != 0xffff)
		{
			fatalerror("fb: non-word write!");
		}
	}

	void news_iop_state::check_operation_start(offs_t offset)
	{
		if (offset & (0x4000 >> 1)) // word offset
		{
			LOGMASKED(LOG_FB, "Operation started\n");
			m_fb_busy = true;
			m_fb_current_x = m_fb_destination_x;

			// TODO: if this is FB -> FB op, start it here, because the host won't write anything
			if (m_fb_read && m_fb_write)
			{
				// fatalerror("Yikes, fb copy isn't done yet\n");
			}
		}
	}

	void news_iop_state::destination_w(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
		check_mem_mask(mem_mask);

		m_fb_destination_x = x_address(data);
		m_fb_destination_y = y_address(offset);

		if (offset & (0x2000 >> 1))
		{
			// how much do we need to shift source by to align it with destination?
			m_fb_shift = m_fb_destination_x % 16 - m_fb_source_x % 16;
		}

		// Compute first mask
		m_fb_mask1 = ~(0xffff >> (m_fb_destination_x % 16)); // determine first mask based on bit offset in first word TODO: direction?
		m_fb_mask1_armed = true;

		LOGMASKED(LOG_FB, "Destination set to (0x%x, 0x%x) x word offset = 0x%x\n", m_fb_destination_x, m_fb_destination_y, (m_fb_destination_x - 1) >> 4);
		check_operation_start(offset);
	}

	void news_iop_state::source_w(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
		check_mem_mask(mem_mask);

		m_fb_source_x = x_address(data);
		m_fb_source_y = y_address(offset);

		if (offset & (0x2000 >> 1))
		{
			// how much do we need to shift source by to align it with destination?
			m_fb_shift = m_fb_destination_x % 16 - m_fb_source_x % 16; // direction determined by x direction
		}

		LOGMASKED(LOG_FB, "Source set to (0x%x, 0x%x)\n", m_fb_source_x, m_fb_source_y);
		check_operation_start(offset);
	}

	void news_iop_state::limit_w(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
		check_mem_mask(mem_mask);
		LOGMASKED(LOG_FB, "limit_w(0x%x, 0x%x, 0x%x)\n", offset, data, mem_mask);

		m_fb_limit_x = x_address(data);
		m_fb_limit_y = y_address(offset);
		const uint16_t last_pixel = (m_fb_limit_x - 1) & 0xf;     // last pixel's position in the last word
		m_fb_mask2 = 0xffff >> (last_pixel + 1);        // preserve everything at/after the right edge

		LOGMASKED(LOG_FB, "Limit set to (0x%x, 0x%x) x word offset = 0x%x\n", m_fb_limit_x, m_fb_limit_y, (m_fb_limit_x - 1) >> 4);
		check_operation_start(offset);
	}

	void news_iop_state::shift_w(offs_t, uint16_t data, uint16_t mem_mask)
	{
		check_mem_mask(mem_mask);

		m_fb_shift = data;
		LOGMASKED(LOG_FB, "Shift set to 0x%x\n", m_fb_shift);
	}

	void news_iop_state::rop_config_w(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
		check_mem_mask(mem_mask);

		LOGMASKED(LOG_FB, "rop_config_w(0x%x, 0x%x) = ", offset, data);

		// These mappings are an educated guess based on NWS-1500 service manual and IOP behavior
		offset <<= 1; // use byte offset
		m_fb_suppress_advance = offset & 0x4;
		m_fb_x_direction = offset & 0x10;
		m_fb_y_direction = offset & 0x20;
		m_fb_row_fill = offset & 0x80;
		m_fb_read = offset & 0x100;
		m_fb_write = offset & 0x200;
		LOGMASKED(LOG_FB, "%s%s%s%s%s%s",
			m_fb_write ? "fbwrite " : "",
			m_fb_read ? "fbread " : "",
			m_fb_row_fill ? "rowfill " : "",
			m_fb_y_direction ? "yfwd " : "yrev ",
			m_fb_x_direction ? "xfwd " : "xrev ",
			m_fb_suppress_advance ? "supadv " : "");

		if (offset & 400) // If set, load ROP function
		{
			m_fb_rop_func = data & 0xf;
			LOGMASKED(LOG_FB, "func=0x%x ", m_fb_rop_func);
		}
		LOGMASKED(LOG_FB, "\n");
		//machine().debug_break();
	}

	void news_iop_state::pixel_w(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
		// Set pixel to 1 at X (data), Y (offset[12:1])
		check_mem_mask(mem_mask);
		LOGMASKED(LOG_FB, "pixel_w(0x%x, 0x%x)\n", offset, data);
		fb_set_pixel(x_address(data), y_address(offset), true);
	}

	uint16_t news_iop_state::execute_rop(const uint8_t opcode, const uint16_t source, const uint16_t destination)
	{
		uint16_t result;
		[[maybe_unused]] std::string op; // Unused when tracing is disabled

		switch (opcode)
		{
			case 0x0:
				op = "~S";
				result = ~source;
				break;
			case 0x1:
				op = "~(S | D)";
				result = ~(source | destination);
				break;
			case 0x2:
				op = "~S & D";
				result = ~source & destination;
				break;
			case 0x3:
				op = "0";
				result = 0;
				break;
			case 0x4:
				op = "~(S & D)";
				result = ~(source & destination);
				break;
			case 0x5:
				op = "~D";
				result = ~destination;
				break;
			case 0x6:
				op = "S ^ D";
				result = source ^ destination;
				break;
			case 0x7:
				op = "S & ~D";
				result = source & ~destination;
				break;
			case 0x8:
				op = "~S | D";
				result = ~source | destination;
				break;
			case 0x9:
				op = "~(S ^ D)";
				result = ~(source ^ destination);
				break;
			case 0xa:
				op = "D";
				result = destination;
				break;
			case 0xb:
				op = "S & D";
				result = source & destination;
				break;
			case 0xc:
				op = "1";
				result = 1;
				break;
			case 0xd:
				op = "S | ~D";
				result = source | ~destination;
				break;
			case 0xe:
				op = "S | D";
				result = source | destination;
				break;
			case 0xf:
				op = "S";
				result = source;
				break;
			default:
				fatalerror("news_68k_iop: invalid ROP function code 0x%x", opcode);
		}

		LOGMASKED(LOG_FB, "execute_rop(0x%x/%s, 0x%x, 0x%x) = 0x%x\n", opcode, op, source, destination, result);
		return result;
	}

 	void news_iop_state::fb_rop_cycle(uint16_t data)
	{
		// Calculate source by shifting bits and update carryover
		// TODO: clean this up
		uint16_t source;
		if (m_fb_x_direction)
		{
			source = (uint16_t)((data >> m_fb_shift) | m_fb_carry);
			m_fb_carry = (uint16_t)(((uint32_t)data << (16 - m_fb_shift)) & 0xffff);
		}
		else
		{
			source = (uint16_t)((data << m_fb_shift) | m_fb_carry);
			m_fb_carry = (uint16_t)(((uint32_t)data >> (16 - m_fb_shift)) & 0xffff);
		}

		// Get destination data
		// Graphics ops are always word-aligned. However, mask1 and mask2 make non-aligned access work as expected.
		uint32_t byte_offset = m_fb_destination_y * 128 + ((m_fb_current_x >> 4) << 1); // Y: 1024 / 8 = 128 bytes per row. Guarantee X is word-aligned first by chopping LSB
		uint16_t destination = m_vram->read(byte_offset) << 8 | m_vram->read(byte_offset + 1);
		LOGMASKED(LOG_FB, "fb_rop_cycle: destination (0x%x, 0x%x) => 0x%x = 0x%x\n", m_fb_destination_x, m_fb_destination_y, byte_offset, destination);

		uint16_t result = execute_rop(m_fb_rop_func, source, destination);

// dest_x, limit_x, width = limit_x - dest_x, mask2
		LOGMASKED(LOG_FB, "dest_x: 0x%x limit_x: 0x%x width: 0x%x mask2: 0x%x\n", m_fb_destination_x, m_fb_limit_x, m_fb_limit_x - m_fb_destination_x, m_fb_mask2);

		if (m_fb_mask1_armed) // TODO: replace with first word check?
		{
			result &= ~m_fb_mask1; // Clear bits that need to be preserved
			result |= destination & m_fb_mask1; // Apply masked bits from destination
			m_fb_mask1_armed = false;
		}

		// If this is the last FIFO push of the row (we have leftover bits from the shift in the shifter), we need to apply them.
		const uint16_t source_words = ((m_fb_source_x & 0xf) // Get the pixel offset within word
											+ m_fb_limit_x - m_fb_destination_x + 0xf) // Add the width in pixels, rounded up
											>> 4; // Convert to word address
		const uint16_t destination_words = ((m_fb_destination_x & 0xf) + m_fb_limit_x - m_fb_destination_x + 0xf) >> 4;
		LOGMASKED(LOG_FB, "source words = 0x%x, destination words = 0x%x\n", source_words, destination_words);

		const uint16_t rel_word = (m_fb_current_x - m_fb_destination_x) >> 4;
		if (rel_word >= source_words - 1 && source_words < destination_words) {
			if (m_fb_carry != 0) {
			LOGMASKED(LOG_FB, "Last source word detected when extra write is needed! destination (0x%x, 0x%x) => 0x%x = 0x%x 0x%x\n", m_fb_destination_x, m_fb_destination_y, byte_offset, result >> 8 & 0xff, result & 0xff);
			}

			// write last FIFO word of row as assembled above and prepare destination word with carry
			m_vram->write(byte_offset, result >> 8 & 0xff);
			m_vram->write(byte_offset + 1, result & 0xff);
			m_fb_current_x += m_fb_x_direction ? 16 : -16;
			result = m_fb_carry;
			// TODO: writing a destination word should be in a helper, but that is far from the only thing wrong with this.
			byte_offset += 2;
			destination = m_vram->read(byte_offset) << 8 | m_vram->read(byte_offset + 1);
		}

		// If we are setting the last word of the row, we need to apply mask2.
		const uint16_t last_destination_word = (m_fb_limit_x - 1) >> 4;
		const bool is_last_destination_word = m_fb_current_x >> 4 == last_destination_word;
		LOGMASKED(LOG_FB, "fb_rop_cycle: current_word = 0x%x last_word = 0x%x\n", m_fb_current_x >> 4, last_destination_word);
		if (is_last_destination_word)
		{
			LOGMASKED(LOG_FB, "fb_rop_cycle: applying mask2: 0x%x vs shift of 0x%x, carry = 0x%x, mask 1 was 0x%x\n", m_fb_mask2, m_fb_shift, m_fb_carry, m_fb_mask1);
			result &= ~m_fb_mask2;
			result |= destination & m_fb_mask2;
		}

		// Write result back to VRAM
		LOGMASKED(LOG_FB, "fb_rop_cycle: destination (0x%x, 0x%x) => 0x%x = 0x%x 0x%x\n", m_fb_destination_x, m_fb_destination_y, byte_offset, result >> 8 & 0xff, result & 0xff);
		m_vram->write(byte_offset, result >> 8 & 0xff);
		m_vram->write(byte_offset + 1, result & 0xff);

		// Advance ROCC state
		m_fb_current_x += m_fb_x_direction ? 16 : -16;
		LOGMASKED(LOG_FB, "fb_rop_cycle: advanced current x to 0x%x\n", m_fb_current_x);

		// If we are now past the limit, that means we just finished a row and need to reset x and advance y
		if (is_last_destination_word)
		{
			LOGMASKED(LOG_FB, "fb_rop_cycle: done with row, advancing y\n");
			m_fb_current_x = m_fb_destination_x;
			m_fb_destination_y += m_fb_y_direction ? 1 : -1;
			m_fb_mask1_armed = true;
			m_fb_carry = 0;

			if (!m_fb_limit_y)
			{
				LOGMASKED(LOG_FB, "fb_rop_cycle: ROP done!\n");
				m_fb_busy = false;
			}
			else
			{
				m_fb_limit_y--;
				LOGMASKED(LOG_FB, "fb_rop_cycle: decremented limit to 0x%x\n", m_fb_limit_y);
			}
		}
	}

	void news_iop_state::fb_fifo_w(offs_t offset, uint16_t data, uint16_t mem_mask)
	{
		LOGMASKED(LOG_FB, "(%s) fb_fifo_w(0x%x, 0x%x, 0x%x)\n", machine().describe_context(), offset, data, mem_mask);
		check_mem_mask(mem_mask);

		if (!m_fb_busy)
		{
			fatalerror("IOP tried to push data without starting an op!");
		}

		// TODO: validate offset versus the setup in the ROP reg

		if (!m_fb_row_fill)
		{
			LOGMASKED(LOG_FB, "fb_fifo_w: pushing unpacked word 0x%x!\n", data);
			fb_rop_cycle(data);
		}
		else
		{
			// LOGMASKED(LOG_FB, "fb_fifo_w: pushing 0x%x to row 0x%x vs limit 0x%x!\n", data, m_fb_destination_y, m_fb_limit_y);

			// Graphics ops are always word-aligned. However, mask1 and mask2 make non-aligned access work as expected.
			const uint16_t word_count = (m_fb_limit_x >> 4) - (m_fb_current_x >> 4); // TODO: this doesn't work for single-transfers
			for (uint16_t i = 0; i < word_count; i++)
			{
				const uint32_t byte_offset = m_fb_destination_y * 128 + (m_fb_current_x >> 3); // Y: 1024 / 8 = 128 bytes per row

				// Set word
				const uint16_t destination = m_vram->read(byte_offset) << 8 | m_vram->read(byte_offset + 1);
				// LOGMASKED(LOG_FB, "fb_fifo_w: destination (0x%x, 0x%x) => 0x%x = 0x%x\n", m_fb_destination_x, m_fb_destination_y, byte_offset, destination);
				uint16_t result = execute_rop(m_fb_rop_func, data, destination); // TODO: apply shift here

				if (m_fb_mask1_armed)
				{
					result &= ~m_fb_mask1; // Clear bits that need to be preserved
					result |= destination & m_fb_mask1; // Unconditionally apply masked bits from destination
					m_fb_mask1_armed = false;
				}

				// If we are setting the last word of the row, we need to apply mask2
				// TODO: properly use direction: last_word needs to be m_fb_destination_x >> 4 when x_direction is false
				//       and the advancing code needs to account for it as well.
				const uint16_t last_word = (m_fb_limit_x - 1) >> 4;
				const bool is_last_word = m_fb_current_x >> 4 == last_word;
				if (is_last_word)
				{
					result &= ~m_fb_mask2;
					result |= destination & m_fb_mask2;
				}

				// Write result back to VRAM
				// LOGMASKED(LOG_FB, "fb_fifo_w: destination (0x%x, 0x%x) => 0x%x = 0x%x 0x%x\n", m_fb_destination_x, m_fb_destination_y, byte_offset, result >> 8 & 0xff, result & 0xff);
				m_vram->write(byte_offset, result >> 8 & 0xff);
				m_vram->write(byte_offset + 1, result & 0xff);

				// Advance ROCC state
				m_fb_current_x += 16;

				// If we are now past the limit, that means we just finished a row and need to reset x and advance y
				if (is_last_word)
				{
					// LOGMASKED(LOG_FB, "fb_fifo_w: done with row, advancing y\n");
					m_fb_current_x = m_fb_destination_x;
					m_fb_destination_y++;
					m_fb_mask1_armed = true;

					if (!m_fb_limit_y)
					{
						// LOGMASKED(LOG_FB, "fb_fifo_w: ROP done!\n");
						m_fb_busy = false;
					}
					else
					{
						m_fb_limit_y--;
					}
				}
			}
		}
	}

	void news_iop_state::fb_set_pixel(uint16_t x, uint16_t y, bool on)
	{
		// Wrap to framebuffer size
		x %= 1024;
		y %= 1024;

		// LOGMASKED(LOG_FB, "fb_set_pixel(0x%x, 0x%x) = %d\n", x, y, on ? 1 : 0);
		// Determine byte offset in VRAM
		const uint32_t byte_offset = y * 128 + (x >> 3); // Y: 1024 / 8 = 128 bytes per row
		const uint16_t x_bit = 7 - (x & 7); // pixel 0 = bit 7

		// Set desired pixel
		uint8_t pixel = m_vram->read(byte_offset);
		pixel = (pixel & ~(1u << x_bit)) | ((on ? 1u : 0u) << x_bit);
		m_vram->write(byte_offset, pixel);
	}

	void news_iop_state::iop_map(address_map &map)
	{
		map.unmap_value_low();
		// Silence unmapped read/write warnings during memory probe - whatever RAM is present will be installed over top of this region
		map(0x00000000, 0x00ffffff).noprw();

		map(0x03000000, 0x0300ffff).rom().region("eprom", 0).mirror(0x007f0000);

		// IOP bus expansion I/O
		map(0x20000000, 0x20efffff).rw(FUNC(news_iop_state::extio_bus_error_r),FUNC(news_iop_state::extio_bus_error_w)).mirror(0x1f000000);

		// NWB-512 Monochrome Framebuffer (816x1024)
		// TODO: convert ranges to use `select` where it makes sense
		map(0x20f02000, 0x20f02001).w(FUNC(news_iop_state::shift_w)); // shift count when only "load shift" is asserted
		map(0x20f08000, 0x20f0ffff).w(FUNC(news_iop_state::destination_w)); // A13 = load shift, A14 = start execution
		map(0x20f20000, 0x20f27fff).w(FUNC(news_iop_state::source_w)); // A13 = load shift, A14 = start execution
		map(0x20f40000, 0x20f55fff).w(FUNC(news_iop_state::limit_w)); // A16 = source-space select (FB vs host), A14 = start execution
		map(0x20f1a000, 0x20f1bfff).w(FUNC(news_iop_state::pixel_w));

		map(0x20f60000, 0x20f60fff).w(FUNC(news_iop_state::rop_config_w)); // rop configuration TODO: is the end addr correct here?
		map(0x20f70000, 0x20f70fff).w(FUNC(news_iop_state::fb_fifo_w)); // data ports

		// TODO: double-check word mapping of these
		map(0x20f80000, 0x20f80001).lr8(NAME([this] {
			return 0x20 | (m_fb_busy ? 0x3 : 0x0);
		}));
		map(0x20f80000, 0x20f80001).lw8(NAME([this] (uint8_t data) { // confirmed 8-bit write
			// NWS-1580 uses a newer version of the framebuffer that uses a similar ROP engine, these definitions
			// can be found in the NWS-1580 service manual.
			LOGMASKED(LOG_FB, "Write FB control = 0x%x (%s)\n", data, machine().describe_context());
			LOGMASKED(LOG_FB, "\t%s framebuffer\n", data & 0x1 ? "enable" : "reset"); // TODO: implement reset
			LOGMASKED(LOG_FB, "\t%s kanji ROM\n", data & 0x2 ? "enable" : "disable"); // TODO: not for mono?
			LOGMASKED(LOG_FB, "\t%s v-sync interrupt\n", data & 0x8 ? "enable" : "disable");
			LOGMASKED(LOG_FB, "\t%s command completion interrupt\n", data & 0x10 ? "enable" : "disable");
			LOGMASKED(LOG_FB, "\treserved %s\n", data & 0x20 ? "set" : "cleared");
			LOGMASKED(LOG_FB, "\tdimmer setting = 0x%x\n", (data & 0xC0) >> 6);

			m_fb_ctrl = data;
		}));

		map(0x20fa0000, 0x20fa0000).lw8([this] (uint8_t data) {
			LOGMASKED(LOG_FB, "Write plane mask = 0x%x\n", data);
		}, "plane_mask_w");

		// map(0x20fc0000, 0x20ffffff).rom().region("krom", 0);
		map(0x20fc0000, 0x20ffffff).lr16([this] (offs_t offset, uint16_t mem_mask) {
			LOGMASKED(LOG_FB, "krom read offset 0x%x = 0x%x\n", offset, m_krom[offset]);
			//machine().debug_break();
			return m_krom[offset];
			//return 0xffff;
		}, "krom_r");   //.rom().region("krom", 0);

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
			if (m_iop->input_line_state(irq.first) != state) {
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
			if (m_cpu->input_line_state(irq.first) != state)
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
		device.option_add("cdrom", NSCSI_CDROM_NEWS);
		device.option_add("tape", NSCSI_TAPE_NEWS);
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
		multiplexer_value |= m_panel_shift ? 0x1 : 0x0;

		bool dcd_state = false;
		if (multiplexer_value < 6) // DIPSW
		{
			dcd_state = BIT(m_dip_switch->read(), multiplexer_value);
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

		NEWS_020_MMU(config, m_mmu);
		m_mmu->set_addrmap(AS_PROGRAM, &news_iop_state::mmu_map);
		m_mmu->set_bus_error_callback(FUNC(news_iop_state::cpu_bus_error));

		RAM(config, m_ram);
		m_ram->set_default_size("8M");
		m_ram->set_extra_options("16M");
		m_ram->set_default_value(0);

		// NEC uPD8253 programmable interval timer
		PIT8253(config, m_interval_timer);
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
		RS232_PORT(config, m_serial[0], default_rs232_devices, nullptr);//"terminal");
		m_serial[0]->cts_handler().set(m_scc_peripheral, FUNC(z80scc_device::ctsa_w));
		m_serial[0]->dcd_handler().set(m_scc_peripheral, FUNC(z80scc_device::dcda_w));
		m_serial[0]->rxd_handler().set(m_scc_peripheral, FUNC(z80scc_device::rxa_w));
		m_scc_peripheral->out_rtsa_callback().set(m_serial[0], FUNC(rs232_port_device::write_rts));
		m_scc_peripheral->out_txda_callback().set(m_serial[0], FUNC(rs232_port_device::write_txd));

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
		auto &scsi(NSCSI_BUS(config, "scsi"));
		NSCSI_CONNECTOR(config, "scsi:0", news_scsi_devices, "harddisk");
		NSCSI_CONNECTOR(config, "scsi:1", news_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi:2", news_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi:3", news_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi:4", news_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi:5", news_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi:6", news_scsi_devices, nullptr);

		// AMD Am5380PC SCSI interface
		NCR5380(config, m_scsi);
		scsi.set_external_device(7, m_scsi);
		m_scsi->irq_handler().set([this] (int state){ m_scsi_dma->irq_w(state); });
		m_scsi->drq_handler().set(DEVICE_SELF, FUNC(news_iop_state::scsi_drq_handler));

		// Virtual device for handling SCSI DMA
		// (workaround for 68k bus access limitations; normally iopboot's BERR handler would deal with SCSI DMA stuff)
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

		SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
		m_screen->set_raw(816 * 1024 * 60, 816, 0, 816, 1024, 0, 1024);
		m_screen->set_screen_update(FUNC(news_iop_state::screen_update));

		RAM(config, m_vram);
		m_vram->set_default_value(0xff); // TODO: should be 0, but setting to 1 for debugging block write
		m_vram->set_default_size("128KB");
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
				PORT_DIPSETTING(0x06, "NWB-225A")
				PORT_DIPSETTING(0x03, "NWB-512")
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

		// NWB-512 Kanji ROM
		// TODO: this mapping should be set up by the graphics card instead when it is moved to a separate device
		ROM_REGION16_BE(0x40000, "krom", 0)
		ROM_LOAD16_BYTE("nwb512-ic128-hn62301ap-aa1.bin", 0x0000, 0x20000, CRC(3eaa0dbc) SHA1(9c4e66cc122fd60963f51649c945cdfc39f230eb))
		ROM_LOAD16_BYTE("nwb512-ic127-hn62301ap-aa2.bin", 0x0001, 0x20000, CRC(b8705af7) SHA1(915333f082ecbfee512f1166fc38d9dd29dcaf26))
	ROM_END

} // anonymous namespace

// Machine definitions
//   YEAR  NAME    P  C  MACHINE INPUT   CLASS           INIT         COMPANY FULLNAME   FLAGS
COMP(1987, nws831, 0, 0, nws831, nws8xx, news_iop_state, init_common, "Sony", "NWS-831", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
