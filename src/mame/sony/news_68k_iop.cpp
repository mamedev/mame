// license:BSD-3-Clause
// copyright-holders:Brice Onken

/*
 * Sony NEWS first generation dual-processor 68k systems
 *
 * The NWS-800 series was the first model of NEWS workstation. The design was also turned into the NWS-900 series for
 * server use. The later NWS-1800 and NWS-3800 models use the same system design concept as the 800 series (dual-CPU).
 * The NWS-1800 series is especially similar to the NWS-800 series in terms of overall design, even though some peripheral
 * chips are different and the custom MMU is not needed because the 1800 uses the 030.
 *
 *  Supported:
 * 	 - NWS-831
 *
 *  Not supported yet (need system firmware dumps):
 *   - All other NWS-8xx systems
 *   - NWS-9xx server (2x '020)
 *
 * Known NWS-800 Series Base Configurations TODO: check that all of these appear in the model list in the headers
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
 *  - NWS-1960 Service Guide (much of the second generation dual-030 platform design is the same)
 *  - http://bitsavers.org/pdf/sony/news/Sony_NEWS_Technical_Manual_3ed_199103.pdf
 *  - https://katsu.watanabe.name/doc/sonynews/model.html
 *
 *  TODO: remove this unless I can test it more
 *  Note: The unsupported systems can be emulated by altering the ID-ROM data to tell the ROM which model it is, however,
 *        there is no guarantee that this ROM has full support for any other configuration. Therefore, the other
 *        models are marked as having no firmware dumps until/unless it can be verified that the other models
 *        actually shipped with the same ROMs but with different ID-ROM data.
 *
 * TODO:
 *   - MMU emulation improvements (are all the right status bits set? any missing features? etc)
 *   - System cache emulation
 *   - Networking is very flaky - XDMCP doesn't work, repeated telnet sessions eventually cause the network stack to hang, etc.
 *   - Graphics, kbms, and parallel port emulation
 *   - floppy drive testing
 *   - The '020 and '030 machines have a lot of similarities but the only service docs I have are for
 *     an '030 machine (NWS-1960) so there are some accuracy improvements needed for the '020 machines,
 *     especially since the '020 machines have a custom MMU. The '030 machines don't need it, so there
 *     is no information on it in the 1960 service manual.
 *   - hyperbus handshake for IOP and CPU accesses. The bus has arbitration circuitry to prevent bus contention when both the CPU and IOP are trying to access the hyperbus (RAM and VME)
 */

#include "emu.h"

#include "news_hid.h"
#include "news_020_mmu.h"

#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/nscsi/tape.h"
#include "bus/rs232/rs232.h"

#include "cpu/m68000/m68020.h"

#include "formats/pc_dsk.h"

#include "imagedev/floppy.h"

#include "machine/am79c90.h"
#include "machine/msm58321.h"
#include "machine/ncr5380.h"
#include "machine/nscsi_bus.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/timekpr.h"
#include "machine/upd765.h"
#include "machine/z80scc.h"
#include "machine/bankdev.h"

#include <bitset>
#include <queue>

#define LOG_INTERRUPT (1U << 1)
#define LOG_ALL_INTERRUPT (1U << 2)
#define LOG_LED (1U << 3)
#define LOG_MEMORY (1U << 4)
#define LOG_PANEL (1U << 5)
#define LOG_MEMORY_ERROR (1U << 6)
#define LOG_TIMER (1U << 7)
#define LOG_SCSI (1U << 8)

// #define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

namespace
{
	class news_iop_state : public driver_device
	{
	public:
		static constexpr feature_type unemulated_features() { return feature::GRAPHICS; }

		news_iop_state(machine_config const &mconfig, device_type type, char const *tag)
			: driver_device(mconfig, type, tag),
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
			  m_scsi(*this, "scsi:7:ncr5380"),
			  m_dip_switch(*this, "FRONT_PANEL"),
			  m_serial(*this, "serial%u", 0U)
		{
		}

		void nws831(machine_config &config);
		void init_common();

	protected:
		// driver_device overrides
		virtual void machine_start() override;
		virtual void machine_reset() override;

		// address maps
		void iop_map(address_map &map);
		void iop_autovector_map(address_map &map);
		void mmu_map(address_map &map);
		void cpu_map(address_map &map);

		// machine config
		void common(machine_config &config);

		enum iop_irq_number : unsigned
		{
			CPIRQ_3_1 = 1,
			SCSI = 2,
			LANCE = 3,
			CPU = 4,
			SCC = 5,
			TIMEOUT_FDCIRQ = 6,
			FDCDRQ = 7,
			SCC_PERIPHERAL = 99 // hack
		};
		template <iop_irq_number Number>
		void iop_irq_w(int state);
		void int_check_iop();

		enum cpu_irq_number : unsigned
		{
			AST = 1,
			CPIRQ1 = 2,
			IOPIRQ3 = 3,
			CPIRQ3 = 4,
			IOPIRQ5 = 5,
			TIMER = 6,
			PERR = 7
		};
		template <cpu_irq_number Number>
		void cpu_irq_w(int state);
		void int_check_cpu();

		[[maybe_unused]] u32 bus_error_r();
		[[maybe_unused]] void bus_error_w(offs_t offset, uint8_t data);

		void handle_rts(int data);
		void handle_dtr(int data);
		void update_dcd();

		void bus_error(offs_t offset, uint32_t mem_mask, bool write, uint8_t status);
		[[maybe_unused]] void iop_bus_error(offs_t offset, uint32_t mem_mask, bool write, uint8_t status);

		// Platform hardware used by the IOP
		uint8_t iop_status_r();
		void iop_romdis_w(uint8_t data);
		void timereni_w(uint8_t data);
		void min_w(uint8_t data);
		void motoron_w(uint8_t data);
		void scsiinte_w(uint8_t data);
		void powoff_w(uint8_t data);
		void iointen_w(uint8_t data);
		void cpureset_w(uint8_t data);
		uint8_t rtcreg_r();
		void rtcreg_w(uint8_t data);
		void rtcsel_w(uint8_t data);
		void scsi_reg_w(offs_t offset, uint8_t data);
		uint32_t scsi_dma_r(offs_t offset, uint32_t mem_mask);
		void scsi_dma_w(offs_t offset, uint32_t data, uint32_t mem_mask);
		void scsi_drq_handler(int status);

		// Platform hardware used by the main processor
		void cpu_romdis_w(uint8_t data);
		void mmuen_w(uint8_t data);
		void timerenc_w(uint8_t data);
		uint8_t berr_status_r();
		void astreset_w(uint8_t data);
		void astset_w(uint8_t data);

		TIMER_CALLBACK_MEMBER(bus_error_off_cpu);
		TIMER_CALLBACK_MEMBER(bus_error_off_iop);

	private:
		// CPUs (2x 68020, but only the main processor has an FPU)
		required_device<m68020_device> m_iop;	 // I/O Processor
		required_device<m68020fpu_device> m_cpu; // Main Processor

		// Main CPU MMU
		required_device<news_020_mmu_device> m_mmu;
		required_device<ram_device> m_ram;

		required_region_ptr<u32> m_eprom;
		required_region_ptr<u32> m_idrom;

		required_device<msm58321_device> m_rtc; // actually is an Epson RTC-58321B
		required_device<pit8253_device> m_interval_timer;
		required_device<z80scc_device> m_scc_external;
		required_device<z80scc_device> m_scc_peripheral;
		required_device<am7990_device> m_net;
		required_device<upd765a_device> m_fdc;
		required_device<ncr5380_device> m_scsi; // actually an AMD AM5380PC
		required_ioport m_dip_switch;

		required_device_array<rs232_port_device, 2> m_serial;

		std::unique_ptr<u16[]> m_net_ram;

		uint8_t m_iop_intst = 0;
		uint8_t m_iop_inten = 0;
		uint8_t m_cpu_intst = 0;
		uint8_t m_cpu_inten = 0;
		bool m_iop_int_state[8] = {false, false, false, false, false, false, false, false}; // 0 is unused
		bool m_cpu_int_state[8] = {false, false, false, false, false, false, false, false}; // 0 is unused
		bool m_iop_timereni = false;
		bool m_iop_timerout = false;
		bool m_cpu_timerenc = false;
		bool m_cpu_timerout = false;
		bool m_scsi_intst = false;
		bool m_scsi_inte = false;
		bool m_blame_peripheral = false;
		bool m_cpu_bus_error = false;
		offs_t m_last_berr_pc_cpu = 0;
		emu_timer  *m_cpu_bus_error_timer;

		bool m_iop_bus_error = false;
		offs_t m_last_berr_pc_iop = 0;
		emu_timer  *m_iop_bus_error_timer;

		int m_sw1_first_read = 0;

		bool m_panel_clear = false;
		bool m_panel_shift = false;
		int m_panel_shift_count = 0;
		uint8_t m_cpu_bus_error_status = 0;
		uint8_t m_iop_bus_error_status = 0;
		uint8_t m_rtc_data = 0;

		std::queue<uint8_t> scsi_dma_buffer;
		int scsi_trx_width = 0;
	};

	void news_iop_state::machine_start()
	{
		m_cpu_bus_error_timer = timer_alloc(FUNC(news_iop_state::bus_error_off_cpu), this);
		m_iop_bus_error_timer = timer_alloc(FUNC(news_iop_state::bus_error_off_iop), this);

		m_net_ram = std::make_unique<u16[]>(8192); // 16K RAM
		save_pointer(NAME(m_net_ram), 8192);

		m_mmu->space(0).install_ram(0x0, m_ram->mask(), m_ram->pointer());

		m_iop_intst = 0;
	}

	TIMER_CALLBACK_MEMBER(news_iop_state::bus_error_off_cpu)
	{
		m_cpu_bus_error = false;
		// m_cpu_bus_error_status = 0; TODO: might be better to remove this, in case the status is checked after this timer is fired
	}

	TIMER_CALLBACK_MEMBER(news_iop_state::bus_error_off_iop)
	{
		m_iop_bus_error = false;
	}

	uint8_t news_iop_state::iop_status_r()
	{
		// Assume no CPIRQ3/CPIRQ1 for now
		const uint8_t status = ((m_scsi_intst ? 0x10 : 0) | ((m_iop_intst & (1 << 6)) ? 0x80 : 0));
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
			// TODO: also unmap RAM?
			m_iop->space(0).install_rom(0x00000000, 0x0000ffff, m_eprom);
		}
		m_panel_shift_count = 0; // hack, clear state from init
	}

	void news_iop_state::timereni_w(uint8_t data)
	{
		LOGMASKED(LOG_TIMER, "Write TIMERENI = 0x%x (%s)\n", data, machine().describe_context());
		m_iop_timereni = data > 0;
		if (!m_iop_timereni)
		{
			m_iop_timerout = false;
		}
		int_check_iop();
	}

	void news_iop_state::min_w(uint8_t data)
	{
		// TODO: HD rate works, DD rate untested
		constexpr int HD_RATE = 500000;
		constexpr int DD_RATE = 250000;

		// 0 = HD, 1 = DD
		const int rate = (data > 0) ? DD_RATE : HD_RATE;

		LOG("Write MIN = 0x%x, set rate to %s (%s)\n", data, rate == HD_RATE ? "HD" : "DD", machine().describe_context());
		m_fdc->set_rate(rate);
	}

	void news_iop_state::motoron_w(uint8_t data)
	{
		LOG("Write MOTORON = 0x%x (%s)\n", data, machine().describe_context());
		m_fdc->subdevice<floppy_connector>("0")->get_device()->mon_w(data & 8 ? 1 : 0);
	}

	void news_iop_state::scsiinte_w(uint8_t data)
	{
		LOGMASKED(LOG_INTERRUPT, "Write SCSIINTE = 0x%x (%s)\n", data, machine().describe_context());
		m_scsi_inte = data > 0;
		int_check_iop();
	}

	void news_iop_state::powoff_w(uint8_t data)
	{
		LOG("Write POWOFF = 0x%x (%s)\n", data, machine().describe_context());

		if (!data && !machine().side_effects_disabled())
		{
			machine().schedule_exit();
		}
	}

	void news_iop_state::iointen_w(uint8_t data)
	{
		LOGMASKED(LOG_INTERRUPT, "%s Write IOINTEN = 0x%x\n", machine().describe_context(), data);
		if (data)
		{
			m_iop_inten |= (1 << CPU);
		}
		else
		{
			m_iop_inten &= ~(1 << CPU);
			m_iop_intst &= ~(1 << CPU);
		}
		int_check_iop();
	}

	void news_iop_state::cpureset_w(uint8_t data)
	{
		LOG("Write CPURESET = 0x%x\n", data);
		if (data > 0)
		{
			m_cpu->resume(SUSPEND_REASON_RESET);
			// m_cpu->spin_until_time(attotime::from_msec(500)); // Debug: delay CPU start. If you slow down the IOP timeout timer, this statement isn't needed. Why???
		}
		else
		{
			m_cpu->suspend(SUSPEND_REASON_RESET, true);
			int_check_cpu();
		}
	}

	uint8_t news_iop_state::rtcreg_r()
	{
		m_rtc->cs1_w(1);
		m_rtc->cs2_w(1);

		m_rtc->read_w(1);
		uint8_t result = m_rtc_data;
		m_rtc->read_w(0);
		m_rtc->cs2_w(0);

		LOG("rtc r 0x%x\n", result);
		return result;
	}

	void news_iop_state::rtcreg_w(uint8_t data)
	{
		LOG("rtc w 0x%x\n", data);
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
		LOG("rtc sel w 0x%x\n", data);
		m_rtc->cs1_w(1);
		m_rtc->cs2_w(1);

		m_rtc->d0_w((data & 0x1) > 0);
		m_rtc->d1_w((data & 0x2) > 0);
		m_rtc->d2_w((data & 0x4) > 0);
		m_rtc->d3_w((data & 0x8) > 0);

		m_rtc->address_write_w(1);
		m_rtc->address_write_w(0);
		m_rtc->cs2_w(0);
	}

	void news_iop_state::scsi_reg_w(offs_t offset, uint8_t data)
	{
		if (offset == 7)
		{
			// starting read transaction, pause IOP and prime the read buffer to avoid BERRs
			// Things go off the rails if a BERR happens, even though the iopboot bus error
			// handler has some SCSI code in it. The bus error mechanism relies on many details
			// from how the 68020 handles 32 bit read/writes on 8 bit devices, so this avoids the issue.
			LOGMASKED(LOG_SCSI, "Starting SCSI DMA trx! (%s)\n", machine().describe_context());
			scsi_trx_width = 4;
			m_iop->suspend(SUSPEND_REASON_HALT, true);
		}
		else if (offset == 2)
		{
			// Reset trx width on dma stop
			if (!BIT(data, 1))
			{
				scsi_trx_width = 0;
				if (scsi_dma_buffer.size() != 0)
				{
					LOGMASKED(LOG_SCSI, "Warning: Data left in buffer! (%s)\n", machine().describe_context());
					std::queue<unsigned char>().swap(scsi_dma_buffer);
				}
			}
		}
		m_scsi->write(offset, data);
	}

	uint32_t news_iop_state::scsi_dma_r(offs_t offset, uint32_t mem_mask)
	{
		// LOG("read offset 0x%x mem_mask 0x%x mem_mask_count = 0x%x\n", offset, mem_mask, std::bitset<32>(mem_mask).count());
		if (mem_mask == 0xff000000)
		{
			// byte access
			if (scsi_trx_width == 0)
			{
				fatalerror("scsi dma: wrong mode");
				return 0u;
			}
			else if (scsi_trx_width == 4 || scsi_dma_buffer.size() > 0)
			{
				LOGMASKED(LOG_SCSI, "Flushing buffer before switching to byte access...\n");
				scsi_trx_width = 1; // switch to byte access mode - one at a time for timing purposes
				uint32_t data = scsi_dma_buffer.front();
				scsi_dma_buffer.pop();
				return data << 24;
			}
			else
			{
				if (!(m_scsi->read(5) & 0x40)) // bas_r
				{
					fatalerror("scsi dma: no data!");
				}

				LOGMASKED(LOG_SCSI, "Getting DMA byte!\n");
				uint32_t data = m_scsi->dma_r();
				return data << 24;
			}
		}
		else if (mem_mask == 0xffffffff)
		{
			// This nasty-looking function is to deal with the way the 68020 and SCSI signals are abused in NEWS-OS 4's iopboot routines
			if (scsi_dma_buffer.size() == 3)
			{
				uint32_t data = scsi_dma_buffer.front() << 24;
				scsi_dma_buffer.pop();
				data |= (scsi_dma_buffer.front() << 16);
				scsi_dma_buffer.pop();
				data |= (scsi_dma_buffer.front() << 8);
				scsi_dma_buffer.pop();
				data |= m_scsi->dma_r(); // TODO: guard this with appropriate status check

				// scsi_trx_width = 0;
				m_scsi_intst = m_scsi->read(5) & 0x40; // bas_r
				int_check_iop();
				return data;
			}
			else
			{
				fatalerror("scsi_dma_r: invalid buffer count 0x%x", scsi_dma_buffer.size());
			}
		}
		else
		{
			fatalerror("scsi_dma_r: unexpected transaction width");
		}
		return 0u;
	}

	void news_iop_state::scsi_dma_w(offs_t offset, uint32_t data, uint32_t mem_mask)
	{
		// LOG("(%s) scsi_dma_w(0x%x, 0x%x, 0x%x) called\n", machine().describe_context(), offset, data, mem_mask);
		if (mem_mask == 0xff000000)
		{
			// byte access
			if (scsi_trx_width == 0)
			{
				m_scsi->dma_w((data >> 24) & 0xff);
			}
			else
			{
				fatalerror("scsi_dma_w: tried to switch from 32 to 8 in the middle of a transfer");
			}
		}
		else if (mem_mask == 0xffffffff)
		{
			if (scsi_trx_width > 0 || scsi_dma_buffer.size() != 0)
			{
				fatalerror("scsi_dma_w: tried to switch from read to write");
			}

			if (!(m_scsi->read(5) & 0x40)) // bas_r
			{
				fatalerror("scsi_dma_w: not ready for data");
			}

			// Halt I/O Processor to avoid having to deal with the bus error synchronization
			m_iop->suspend(SUSPEND_REASON_HALT, true);

			// Setup DMA buffer
			scsi_trx_width = -3;
			scsi_dma_buffer.push((data >> 16) & 0xff);
			scsi_dma_buffer.push((data >> 8) & 0xff);
			scsi_dma_buffer.push(data & 0xff);

			// Send byte
			m_scsi->dma_w((data >> 24) & 0xff);
		}
		else
		{
			fatalerror("scsi_dma_w: unexpected transaction width");
		}
	}

	void news_iop_state::scsi_drq_handler(int status)
	{
		LOGMASKED(LOG_SCSI, "SCSI DRQ changed to 0x%x\n", status);
		if (status && scsi_trx_width == 4 && scsi_dma_buffer.size() < 3)
		{
			LOGMASKED(LOG_SCSI, "Absorbing into buffer\n", status);
			scsi_dma_buffer.push(m_scsi->dma_r());
		}
		else if (status && scsi_trx_width == 4 && scsi_dma_buffer.size() == 3)
		{
			LOGMASKED(LOG_SCSI, "3 bytes of data are buffered and DRQ is asserted - resuming IOP to execute DMA trx\n");
			m_iop->resume(SUSPEND_REASON_HALT);
		}
		else if (status && scsi_trx_width < 0 && scsi_dma_buffer.size() > 0)
		{
			LOGMASKED(LOG_SCSI, "Writing next byte!\n");
			uint8_t next_byte = scsi_dma_buffer.front();
			scsi_dma_buffer.pop();
			scsi_trx_width++;
			m_scsi->dma_w(next_byte);
			if (scsi_trx_width == 0)
			{
				LOGMASKED(LOG_SCSI, "Write trx complete, resuming IOP\n");
				m_iop->resume(SUSPEND_REASON_HALT);
			}
		}

		m_scsi_intst = status; // can be masked, unlike SCSI IRQ
		int_check_iop();
	}

	void news_iop_state::machine_reset()
	{
		// Reset memory configuration
		m_iop->space(0).install_rom(0x00000000, 0x0000ffff, m_eprom);

		// CPU doesn't run until the IOP tells it to
		m_cpu->suspend(SUSPEND_REASON_RESET, true);
	}

	void news_iop_state::init_common()
	{
	}

	void news_iop_state::iop_map(address_map &map)
	{
		map.unmap_value_low();
		map(0x03000000, 0x0300ffff).rom().region("eprom", 0).mirror(0x007f0000); // TODO: Not 100% sure this mirror is correct
		map(0x00800000, 0x00ffffff).noprw(); // TODO: how to handle this, lower RAM

		// map(0x20000000, 0x3fffffff).lrw8(NAME([this](){ iop_bus_error(); return 0xff; }), NAME([this](uint8_t data){ /*iop_bus_error();*/ }));
		// map(0x4c000100, 0x4c0001ff).lrw8(NAME([this](){ iop_bus_error(); return 0; }), NAME([this](uint8_t data){ iop_bus_error(); }));

		map(0x60000000, 0x60000000).r(FUNC(news_iop_state::iop_status_r));
		map(0x40000000, 0x40000000).w(FUNC(news_iop_state::timereni_w));
		map(0x40000001, 0x40000001).w(FUNC(news_iop_state::min_w));
		map(0x40000002, 0x40000002).w(FUNC(news_iop_state::motoron_w));
		map(0x40000003, 0x40000003).w(FUNC(news_iop_state::scsiinte_w));
		map(0x40000004, 0x40000004).w(FUNC(news_iop_state::iop_romdis_w));
		map(0x40000005, 0x40000005).w(FUNC(news_iop_state::powoff_w));
		map(0x40000006, 0x40000006).w(FUNC(news_iop_state::iointen_w));
		map(0x40000007, 0x40000007).w(FUNC(news_iop_state::cpureset_w));

		map(0x42000000, 0x42000003).rw(m_interval_timer, FUNC(pit8253_device::read), FUNC(pit8253_device::write)); // 3-channel timer (uPD8253C)

		map(0x44000000, 0x44000003).m(m_fdc, FUNC(upd765a_device::map));
		map(0x44000007, 0x44000007).rw(m_fdc, FUNC(upd765a_device::dma_r), FUNC(upd765a_device::dma_w));

		map(0x46000001, 0x46000001).rw(FUNC(news_iop_state::rtcreg_r), FUNC(news_iop_state::rtcreg_w));
		map(0x46000002, 0x46000002).w(FUNC(news_iop_state::rtcsel_w)); // TODO: no reads to this addr, right?

		map(0x4a000000, 0x4a000003).rw(m_scc_peripheral, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)); // kbms
		map(0x4c000000, 0x4c000003).rw(m_scc_external, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w));	  // rs232

		map(0x4e000000, 0x4e000007).r(m_scsi, FUNC(ncr5380_device::read));
		map(0x4e000000, 0x4e000007).w(FUNC(news_iop_state::scsi_reg_w)); // Write is patched for SCSI pseudo-DMA implementation

		map(0x6a000001, 0x6a000001).lw8(NAME([this](uint8_t data) { cpu_irq_w<IOPIRQ5>(data > 0); }));
		map(0x6a000002, 0x6a000002).lw8(NAME([this](uint8_t data) { cpu_irq_w<IOPIRQ3>(data > 0); }));

		map(0x64000000, 0x64000003).rw(FUNC(news_iop_state::scsi_dma_r), FUNC(news_iop_state::scsi_dma_w));

		map(0x66000000, 0x66003fff).lrw16([this](offs_t offset) { return m_net_ram[offset]; }, "net_ram_r",
										  [this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_net_ram[offset]); }, "net_ram_w");

		map(0x68000000, 0x68000003).rw(m_net, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w));

		map(0x80000000, 0x8001ffff).ram(); // IOP work RAM
	}

	void news_iop_state::cpu_romdis_w(uint8_t data)
	{
		LOG("CPU ROMDIS 0x%x\n", data);
		m_mmu->set_rom_enable(data == 0);
	}

	void news_iop_state::mmuen_w(uint8_t data)
	{
		LOGMASKED(LOG_MEMORY, "(%s) CPU MMUEN 0x%x\n", machine().describe_context(), data);
		const bool mmu_enable = data > 0;
		m_cpu->set_emmu_enable(mmu_enable);
		m_mmu->set_mmu_enable(mmu_enable);
	}

	void news_iop_state::timerenc_w(uint8_t data)
	{
		LOGMASKED(LOG_TIMER, "TIMERENC 0x%x (%s)\n", data, machine().describe_context());

		m_cpu_timerenc = data > 0;
		if (!m_cpu_timerenc)
		{
			m_cpu_timerout = false;
		}
		int_check_cpu();
	}

	uint8_t news_iop_state::berr_status_r()
	{
		LOGMASKED(LOG_MEMORY, "(%s) r BERR_STATUS = 0x%x\n", machine().describe_context(), m_cpu_bus_error_status);
		uint8_t status = m_cpu_bus_error_status;
		m_cpu_bus_error_status = 0;
		return status;
	}

	void news_iop_state::astreset_w(uint8_t data)
	{
		LOG("(%s) AST_RESET 0x%x\n", machine().describe_context(), data);
		// TODO: Implement AST
		int_check_cpu();
	}

	void news_iop_state::astset_w(uint8_t data)
	{
		LOG("(%s) AST_SET 0x%x as %s\n", machine().describe_context(), data, m_cpu->supervisor_mode() ? "supervisor" : "user");
		// TODO: Implement AST
		int_check_cpu();
	}

	void news_iop_state::mmu_map(address_map &map)
	{
		map(0x03000000, 0x0300ffff).rom().region("eprom", 0).mirror(0x007f0000);

		// MMU mapping control
		map(0x06000000, 0x061fffff).rw(m_mmu, FUNC(news_020_mmu_device::mmu_entry_r), FUNC(news_020_mmu_device::mmu_entry_w)).select(0x10000000); // MMU map RAM
		map(0x04c00000, 0x04c00000).w(m_mmu, FUNC(news_020_mmu_device::clear_entries)).select(0x10000000);

		// Various platform control registers (MMU and otherwise)
		map(0x04400000, 0x04400000).w(FUNC(news_iop_state::cpu_romdis_w));
		map(0x04400001, 0x04400001).w(FUNC(news_iop_state::mmuen_w));

		map(0x04400002, 0x04400002).lw8([this](uint8_t data)
										{
											// TODO: resolve copy-paste between this, some of the IOP stuff, and CPINT4EN
											LOGMASKED(LOG_ALL_INTERRUPT, "CPU CPINT2EN 0x%x\n", data);
											if (data)
											{
												m_cpu_inten |= (1 << IOPIRQ3);
											}
											else
											{
												m_cpu_inten &= ~(1 << IOPIRQ3);
												m_cpu_intst &= ~(1 << IOPIRQ3);
											}
											int_check_cpu();
										}, "CPINT2EN");

		map(0x04400003, 0x04400003).lw8([this](uint8_t data)
										{
											LOGMASKED(LOG_ALL_INTERRUPT, "CPU CPINT4EN 0x%x\n", data);
											if (data)
											{
												m_cpu_inten |= (1 << IOPIRQ5);
											}
											else
											{
												m_cpu_inten &= ~(1 << IOPIRQ5);
												m_cpu_intst &= ~(1 << IOPIRQ5);
											}
											int_check_cpu();
										}, "CPINT4EN");

		map(0x04400004, 0x04400004).w(FUNC(news_iop_state::timerenc_w));
		map(0x04400005, 0x04400005).lw8(NAME([this](uint8_t data) { LOG("CACHEEN 0x%x\n", data); }));
		map(0x04400006, 0x04400006).lw8(NAME([this](uint8_t data) { LOG("PARITYEN 0x%x\n", data); }));
		map(0x04400007, 0x04400007).lw8(NAME([this](uint8_t data) { LOG("UNKNOWN 0x%x\n", data); }));

		map(0x04800000, 0x04800000).lw8([this](uint8_t data) { iop_irq_w<CPU>(1); }, "INT_IOP");

		map(0x05c00000, 0x05c00000).r(FUNC(news_iop_state::berr_status_r));

		map(0x05800000, 0x05800000).w(FUNC(news_iop_state::astreset_w));
		map(0x05800001, 0x05800001).w(FUNC(news_iop_state::astset_w));

		map(0x05000000, 0x05000000).lw8([this](uint8_t data)
										{ LOGMASKED(LOG_MEMORY, "(%s) user cache clear = 0x%x\n", machine().describe_context(), data); },
										"USER_CACHE_CLR");

		map(0x05400000, 0x05400000).lw8([this](uint8_t data)
										{ // TODO: use select and merge with previous one
											LOGMASKED(LOG_MEMORY, "(%s) system cache clear = 0x%x\n", machine().describe_context(), data);
										}, "SYS_CACHE_CLR");
	}

	void news_iop_state::cpu_map(address_map &map)
	{
		map(0x0, 0xffffffff).lrw32([this](offs_t offset, uint32_t mem_mask) {
			return m_mmu->hyperbus_r(offset, mem_mask, m_cpu->supervisor_mode());
		}, "HB_R",
		[this](offs_t offset, uint32_t data, uint32_t mem_mask) {
			m_mmu->hyperbus_w(offset, data, mem_mask, m_cpu->supervisor_mode());
		}, "HB_W");
	}

	template <news_iop_state::iop_irq_number Number>
	void news_iop_state::iop_irq_w(int state)
	{
		if (Number != TIMEOUT_FDCIRQ)
		{
			LOG("iop irq number %d state %d\n", Number, state);
		}

		int shift = Number == SCC_PERIPHERAL ? SCC : Number;
		if (Number == SCC_PERIPHERAL)
		{
			m_blame_peripheral = state > 0;
		}

		if (state)
		{
			m_iop_intst |= 1U << shift; // TODO: index from 0?
		}
		else
		{
			m_iop_intst &= ~(1U << shift);
		}

		int_check_iop();
	}

	void news_iop_state::int_check_iop()
	{
		// TODO: make this not jank
		static const int interrupt_map[] = {INPUT_LINE_IRQ0, INPUT_LINE_IRQ1, INPUT_LINE_IRQ2, INPUT_LINE_IRQ3, INPUT_LINE_IRQ4, INPUT_LINE_IRQ5, INPUT_LINE_IRQ6, INPUT_LINE_IRQ7};
		int active_irq = m_iop_intst & (m_iop_inten | (1 << SCSI) | (1 << SCC) | (1 << LANCE) | (1 << TIMEOUT_FDCIRQ) | (1 << FDCDRQ));

		if (m_scsi_intst && m_scsi_inte)
		{
			active_irq |= (1 << SCSI);
		}

		for (int i = 1; i <= 7; i++)
		{
			bool state = active_irq & (1 << i);

			if (m_iop_int_state[i] != state)
			{
				// if (i != 6)
				// {
					LOG("Setting IOP input line %d to %d (%d)\n", interrupt_map[i], state ? 1 : 0, i);
				// }
				m_iop_int_state[i] = state;
				m_iop->set_input_line(interrupt_map[i], state ? 1 : 0);
			}
		}

		if ((m_iop_timereni && m_iop_timerout) || (m_iop_intst & (1 << TIMEOUT_FDCIRQ))) // bad hack for now
		{
			m_iop->set_input_line(INPUT_LINE_IRQ6, 1);
		}
		else
		{
			m_iop->set_input_line(INPUT_LINE_IRQ6, 0);
			m_iop_timerout = false; // just in case
		}
	}

	template <news_iop_state::cpu_irq_number Number>
	void news_iop_state::cpu_irq_w(int state)
	{
		// if (Number != TIMER)
		// {
			LOG("cpu irq number %d state %d\n", Number, state);
		// }

		// int shift = Number == SCC_PERIPHERAL ? SCC : Number;
		// if (Number == SCC_PERIPHERAL)
		// {
		// 	m_blame_peripheral = state > 0;
		// }

		if (state)
		{
			m_cpu_intst |= 1U << Number; // TODO: index from 0?
		}
		else
		{
			m_cpu_intst &= ~(1U << Number);
		}

		int_check_cpu();
	}

	void news_iop_state::int_check_cpu()
	{
		// TODO: make this not jank
		static const int interrupt_map[] = {INPUT_LINE_IRQ0, INPUT_LINE_IRQ1, INPUT_LINE_IRQ2, INPUT_LINE_IRQ3, INPUT_LINE_IRQ4, INPUT_LINE_IRQ5, INPUT_LINE_IRQ6, INPUT_LINE_IRQ7};
		int active_irq = m_cpu_intst & m_cpu_inten;
		for (int i = 1; i <= 7; i++)
		{
			bool state = active_irq & (1 << i);

			if (m_cpu_int_state[i] != state)
			{
				if (i != 6)
				{
					LOGMASKED(LOG_GENERAL, "(%s) Setting CPU input line %d to %d (%d)\n", machine().describe_context(), interrupt_map[i], state ? 1 : 0, i);
				}
				m_cpu_int_state[i] = state;
				m_cpu->set_input_line(interrupt_map[i], state ? 1 : 0);
			}
		}

		if (m_cpu_timerenc && m_cpu_timerout)
		{
			LOG("Setting CPU timer interrupt!\n");
			m_cpu->set_input_line(INPUT_LINE_IRQ6, 1);
		}
		else
		{
			// LOG("Clearing CPU timer interrupt!\n");
			m_cpu->set_input_line(INPUT_LINE_IRQ6, 0);
			// m_cpu_timerout = false; // just in case
		}

		// TODO: Implement AST (Asynchronous System Trap, IRQ1)
	}

	static void news_scsi_devices(device_slot_interface &device)
	{
		device.option_add("harddisk", NSCSI_HARDDISK);
		device.option_add("cdrom", NSCSI_CDROM); // Only the NWS-891 came with a CD-ROM as a default option, others required an external CD-ROM drive
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
				dcd_state = m_dip_switch->read() & (1 << 5);
			}
			else
			{
				dcd_state = m_dip_switch->read() & (1 << multiplexer_value);
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

	void news_iop_state::bus_error(offs_t offset, uint32_t mem_mask, bool write, uint8_t status)
	{
		if (machine().side_effects_disabled() || (m_cpu_bus_error && m_cpu->pc() == m_last_berr_pc_cpu))
		{
			// LOGMASKED(LOG_MEMORY, "Suppressing cpu bus error at offset 0x%x mask 0x%x r/w %s status 0x%x pc 0x%x", offset, mem_mask, write ? "w" : "r", status, m_cpu->pc());
			return;
		}
		// TODO: is this needed?
		// LOG("bus error at %08x & %08x (%s)\n", offset, mem_mask, rw ? "read" : "write");
		if (!ACCESSING_BITS_16_31) // what to do about 8 bit access?
		{
			offset++;
		}
		// Based on hcpu30's handling of bus errors
		LOGMASKED(LOG_MEMORY, "Applying cpu bus error at offset 0x%x mask 0x%x r/w %s status 0x%x pc 0x%x\n", offset, mem_mask, write ? "w" : "r", status, m_cpu->pc());

		m_last_berr_pc_cpu = m_cpu->pc();
		m_cpu_bus_error = true;
		m_cpu_bus_error_status = status;
		m_cpu->set_buserror_details(offset, !write, m_cpu->get_fc(), true);
		m_cpu_bus_error_timer->adjust(m_cpu->cycles_to_attotime(16)); // 16 is shamelessly stolen from hcpu30 - is that right?
	}

	// TODO: for this and the CPU, can we count on the MROM and OS to always read BERR? If so, can we populate/clear that accordingly to ensure better timing? maybe.
	void news_iop_state::iop_bus_error(offs_t offset, uint32_t mem_mask, bool write, uint8_t status)
	{
		if (machine().side_effects_disabled() || (m_iop_bus_error && m_iop->pc() == m_last_berr_pc_iop))
		{
			LOGMASKED(LOG_GENERAL, "Suppressing iop bus error at offset 0x%x mask 0x%x r/w %s status 0x%x pc 0x%x\n", offset, mem_mask, write ? "w" : "r", status, m_iop->pc());
			return;
		}

		// LOGMASKED(LOG_GENERAL, "Applying iop bus error at offset 0x%x mask 0x%x r/w %s status 0x%x pc 0x%x\n", offset, mem_mask, write ? "w" : "r", status, m_iop->pc());
		// machine().debug_break();
		m_last_berr_pc_iop = m_iop->pc();
		m_iop_bus_error = true;
		m_iop_bus_error_status = status;
		m_iop->set_buserror_details(offset, !write, m_iop->get_fc(), true);
		// m_iop->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		// m_iop->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
		m_iop_bus_error_timer->adjust(m_cpu->cycles_to_attotime(16));
	}

	void news_iop_state::common(machine_config &config)
	{
		M68020(config, m_iop, 16.67_MHz_XTAL); // TODO: probably divided somehow
		m_iop->set_addrmap(AS_PROGRAM, &news_iop_state::iop_map);
		m_iop->set_addrmap(m68000_base_device::AS_CPU_SPACE, &news_iop_state::iop_autovector_map);

		M68020FPU(config, m_cpu, 16.67_MHz_XTAL);
		m_cpu->set_addrmap(AS_PROGRAM, &news_iop_state::cpu_map);

		NEWS_020_MMU(config, m_mmu, 0);
		m_mmu->set_addrmap(AS_PROGRAM, &news_iop_state::mmu_map);
		m_mmu->set_bus_error_callback(FUNC(news_iop_state::bus_error));

		ADDRESS_MAP_BANK(config, "mainbus").set_map(&news_iop_state::mmu_map).set_options(ENDIANNESS_BIG, 32, 32); // TODO: is the address length here actually 32? It might be, but it might not.

		RAM(config, m_ram);
		m_ram->set_default_size("8M");
		m_ram->set_extra_options("16M"); // TODO: any other configurations supported?
		m_ram->set_default_value(0);

		// NEC uPD8253 programmable interval timer
		PIT8253(config, m_interval_timer, 0);		   // TODO: which clock is divided to get the 2MHz input?
		m_interval_timer->set_clk<0>(XTAL(2'000'000)); // CPU time slice?
		m_interval_timer->set_clk<1>(XTAL(2'000'000)); // main memory refresh
		m_interval_timer->set_clk<2>(XTAL(200));	   // IOP timeout TODO: why does everything work better when slowing this down?? What is the real input freq?
													   // 					Using the input clock from the NWS-19xx schematic causes issues.

		m_interval_timer->out_handler<0>().set([this](uint8_t data)
												{
													// LOG("cpu timerout tick\n");
													if (data > 0)
													{
														m_cpu_timerout = true;
													}
													// m_cpu_timerout = (data > 0) && m_cpu_timerenc;
													int_check_cpu();
												});

		m_interval_timer->out_handler<2>().set([this](uint8_t data) { //
			// this signal drives the clock of a D Flip-Flop with the input tied to high and clear tied to TIMERENI
			// LOG("got iop timeout cb 0x%x\n", data);
			m_iop_timerout = (data > 0) && m_iop_timereni;
			int_check_iop();
		});

		// 2x Sharp LH8530A Z8530A-SCC
		SCC8530N(config, m_scc_external, (16_MHz_XTAL / 4));
		SCC8530N(config, m_scc_peripheral, (16_MHz_XTAL / 4));
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
		AM7990(config, m_net);
		m_net->intr_out().set(FUNC(news_iop_state::iop_irq_w<LANCE>)).invert();
		m_net->dma_in().set([this](offs_t offset)
							{ return m_net_ram[(offset >> 1) & 0x1fff]; });
		m_net->dma_out().set([this](offs_t offset, u16 data, u16 mem_mask)
							 { COMBINE_DATA(&m_net_ram[(offset >> 1) & 0x1fff]); });

		// uPD7265 FDC (Compatible with 765A except it should use Sony/ECMA format by default?)
		UPD765A(config, m_fdc, 4'000'000); // TODO: clock rate
		m_fdc->intrq_wr_callback().set(FUNC(news_iop_state::iop_irq_w<TIMEOUT_FDCIRQ>));
		m_fdc->drq_wr_callback().set(FUNC(news_iop_state::iop_irq_w<FDCDRQ>));
		FLOPPY_CONNECTOR(config, "fdc:0", "35hd", FLOPPY_35_HD, true, floppy_image_device::default_pc_floppy_formats).enable_sound(false);

		// scsi bus and devices
		NSCSI_BUS(config, "scsi");

		NSCSI_CONNECTOR(config, "scsi:0", news_scsi_devices, "harddisk");
		NSCSI_CONNECTOR(config, "scsi:1", news_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi:2", news_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi:3", news_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi:4", news_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi:5", news_scsi_devices, nullptr);
		NSCSI_CONNECTOR(config, "scsi:6", news_scsi_devices, nullptr);

		// scsi host adapter
		NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr5380", NCR5380).machine_config([this](device_t *device)
		{
			ncr5380_device &adapter = downcast<ncr5380_device &>(*device);
			adapter.irq_handler().set(*this, FUNC(news_iop_state::iop_irq_w<SCSI>));
			adapter.drq_handler().set(*this, FUNC(news_iop_state::scsi_drq_handler));
		});

		// Epson RTC-48321B
		// TODO: any other RTC connections or settings needed?
		MSM58321(config, m_rtc, 32.768_kHz_XTAL);
		m_rtc->d0_handler().set([this](int data)
								{ m_rtc_data = (m_rtc_data & ~0x1) | (data ? 0x1 : 0x0); }); // TODO: clean this up - move shared data into a func
		m_rtc->d1_handler().set([this](int data)
								{ m_rtc_data = (m_rtc_data & ~0x2) | (data ? 0x2 : 0x0); });
		m_rtc->d2_handler().set([this](int data)
								{ m_rtc_data = (m_rtc_data & ~0x4) | (data ? 0x4 : 0x0); });
		m_rtc->d3_handler().set([this](int data)
								{ m_rtc_data = (m_rtc_data & ~0x8) | (data ? 0x8 : 0x0); });
	}

	void news_iop_state::iop_autovector_map(address_map &map)
	{
		map(0xfffffff3, 0xfffffff3).lr8(NAME([]()
											 { return m68000_base_device::autovector(1); }));
		map(0xfffffff5, 0xfffffff5).lr8(NAME([]()
											 { return m68000_base_device::autovector(2); }));
		map(0xfffffff7, 0xfffffff7).lr8(NAME([]()
											 { return m68000_base_device::autovector(3); }));
		map(0xfffffff9, 0xfffffff9).lr8(NAME([]()
											 { return m68000_base_device::autovector(4); }));
		map(0xfffffffb, 0xfffffffb).lr8(NAME([this]()
											 {
												uint8_t vector = m68000_base_device::autovector(5);
												if (m_iop_intst & (1 << SCC))
												{
													// find the scc to blame
													if(m_blame_peripheral)
													{
														vector = m_scc_peripheral->m1_r();
													}
													else
													{
														vector = m_scc_external->m1_r();
													}
												}
												return vector;
											 }));
		map(0xfffffffd, 0xfffffffd).lr8(NAME([]()
											 { return m68000_base_device::autovector(6); }));
		map(0xffffffff, 0xffffffff).lr8(NAME([]()
											 { return m68000_base_device::autovector(7); }));
	}

	void news_iop_state::nws831(machine_config &config)
	{
		common(config);
	}

	u32 news_iop_state::bus_error_r()
	{
		if (!machine().side_effects_disabled())
			m_iop->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);

		return 0;
	}

	void news_iop_state::bus_error_w(offs_t offset, uint8_t data)
	{
		if (!machine().side_effects_disabled())
			m_iop->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
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

/*   YEAR  NAME    P  C  MACHINE INPUT   CLASS           INIT         COMPANY FULLNAME                    FLAGS */
COMP(1987, nws831, 0, 0, nws831, nws8xx, news_iop_state, init_common, "Sony", "NET WORK STATION NWS-831", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
