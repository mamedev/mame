// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay,Brice Onken

/*
 * Sony NEWS NWS-38xx workstations
 *
 * The NWS-3800 series was the last model of NEWS workstation to use the original NWS-8xx/9xx architecture of
 * having a CPU running NEWS-OS (handling the processing of user code) and a CPU running a separate OS (iopboot/rtx/mrx)
 * to handle system I/O. Unlike the 8xx/9xx and 18xx/19xx, the NWS-3800 is a mixed architecture system. It uses a MIPS
 * R3000 CPU for NEWS-OS and user programs, and a 68030 running rtx (NEWS-OS 3) or mrx (NEWS-OS 4) as the IOP.
 *
 * The motherboard designator for the 3800 series is MPU-10.
 *
 * Supported:
 *  - NWS-3860
 *
 * Known NWS-3800 configurations
 *  - NWS-3840: Apr 1990, 20MHz CPU/IOP, 16MB RAM (expandable to 80MB), 286MB HDD, without tape drive
 *  - NWS-3860: Dec 1989, 20MHz CPU/IOP, 16MB RAM (expandable to 80MB), 640MB HDD, with tape drive
 *  - NWS-3865: Aug 1991, 25MHz CPU/IOP, 16MB RAM (expandable to 80MB), 640MB HDD (Type E) or 1.25GB HDD (Type G), with tape drive
 *  - NWS-3870: Jan 1991, 25MHz CPU/IOP, 64MB RAM (expandable to 128MB), 1.25GB HDD, with tape drive
 *  - NWS-3880: Feb 1992, 25MHz CPU/IOP, 64MB RAM (expandable to ?), 2.0GB HDD, tape drive unknown
 *
 * Sources:
 *  - NWS-3840/3860 Service Guide
 *  - http://bitsavers.org/pdf/sony/news/Sony_NEWS_Technical_Manual_3ed_199103.pdf
 *  - https://katsu.watanabe.name/doc/sonynews/model.html
 *
 * TODO:
 *   - slots (I/O and UBUS expansion slots)
 *   - graphics
 *   - sound
 *   - AST
 *   - Something in the format/install flow is iffy: you have to manually write the disklabel before the installer works
 *     I do not have the NEWS-OS 4.1R install manual, so I don't know if I am doing something wrong or if the emulation
 *     is missing something. The format program does seem to spin for a bit on reading the defect information after
 *     running FORMAT UNIT.
 */

#include "emu.h"

#include "cpu/mips/mips1.h"
#include "cpu/m68000/m68030.h"

// memory
#include "machine/ram.h"

// various hardware
#include "machine/msm6242.h"
#include "machine/z80scc.h"
#include "machine/am79c90.h"
#include "machine/upd765.h"
#include "dmac_0266.h"
#include "news_hid.h"
#include "machine/ncr5380.h"

// video
#include "screen.h"

// busses and connectors
#include "machine/nscsi_bus.h"
#include "bus/centronics/ctronics.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/nscsi/tape.h"
#include "bus/rs232/rs232.h"

#include "imagedev/floppy.h"

#define LOG_INTERRUPT (1U << 1)
#define LOG_TIMER (1U << 2)
#define LOG_TAS (1U << 3)
#define LOG_IOP (1U << 4)
#define LOG_CPU (1U << 5)
#define LOG_PARALLEL (1U << 6)

#include "logmacro.h"


namespace {

using namespace std::literals::string_view_literals;

// Logging constants
constexpr auto ENABLED = "enabled";
constexpr auto DISABLED = "disabled";

class news_38xx_state : public driver_device
{
public:
	static constexpr feature_type unemulated_features() { return feature::GRAPHICS; }

	news_38xx_state(machine_config const &mconfig, device_type type, char const *tag) :
		driver_device(mconfig, type, tag),
		m_cpu(*this, "cpu"),
		m_iop(*this, "iop"),
		m_ram(*this, "ram"),
		m_dma(*this, "dma%u", 0U),
		m_rtc(*this, "rtc"),
		m_scc(*this, "scc"),
		m_net(*this, "net"),
		m_fdc(*this, "fdc"),
		m_hid(*this, "hid"),
		m_scsi(*this, "scsi%u:7:cxd1180", 0U),
		m_serial(*this, "serial%u", 0U),
		m_scsibus(*this, "scsi%u", 0U),
		m_parallel(*this, "parallel"),
		m_parallel_data(*this, "parallel_data"),
		m_eprom(*this, "eprom"),
		m_led(*this, "led%u", 0U)
	{
	}

	void nws3860(machine_config &config) ATTR_COLD;

	void init_common() ATTR_COLD;

protected:
	// Not all IRQs feed into the status register directly
	// Because we have to handle IRQ data being stored everywhere and/or apply filtering to the status register,
	// apply the same strategy used in news_68k_iop, which also had to deal with this problem.
	enum class iop_irq : unsigned
	{
		AST,
		SOFTINTR,
		FDCIRQ,
		PARALLEL,
		SLOT,
		LANCE,
		SCSI0,
		SCSI1,
		CPU,
		UBUS,
		SCC,
		KEYBOARD,
		MOUSE,
		TIMER,
		FDCDRQ,
		PERR
	};

	static constexpr std::array IOP_IRQ_NAMES = {
		"AST"sv, "SOFTINTR"sv, "FDCIRQ"sv, "PARALLEL"sv, "SLOT"sv, "LANCE"sv, "SCSI0"sv, "SCSI1"sv, "CPU"sv, "UBUS"sv,
		"SCC"sv, "KEYBOARD"sv, "MOUSE"sv, "TIMER"sv, "FDCDRQ"sv, "PERR"sv
	};
	static constexpr u32 IOP_NMI_MASK = 1 << u32(iop_irq::FDCDRQ) |
										1 << u32(iop_irq::SCC) |
										1 << u32(iop_irq::SCSI0) |
										1 << u32(iop_irq::SCSI1) |
										1 << u32(iop_irq::LANCE) |
										1 << u32(iop_irq::FDCIRQ) |
										1 << u32(iop_irq::SLOT) |
										1 << u32(iop_irq::SOFTINTR) |
										1 << u32(iop_irq::AST);
	static constexpr std::array IOP_LINE_MASK = {
		std::make_pair(INPUT_LINE_IRQ1, 1 << u32(iop_irq::AST)),
		std::make_pair(INPUT_LINE_IRQ2, 1 << u32(iop_irq::SOFTINTR)),
		std::make_pair(INPUT_LINE_IRQ3, 1 << u32(iop_irq::FDCIRQ) | 1 << u32(iop_irq::PARALLEL) | 1 << u32(iop_irq::SLOT)),
		std::make_pair(INPUT_LINE_IRQ4, 1 << u32(iop_irq::LANCE) | 1 << u32(iop_irq::SCSI0) | 1 << u32(iop_irq::SCSI1) | 1 << u32(iop_irq::CPU) | 1 << u32(iop_irq::UBUS)),
		std::make_pair(INPUT_LINE_IRQ5, 1 << u32(iop_irq::SCC) | 1 << u32(iop_irq::KEYBOARD) | 1 << u32(iop_irq::MOUSE)),
		std::make_pair(INPUT_LINE_IRQ6, 1 << u32(iop_irq::TIMER)),
		std::make_pair(INPUT_LINE_IRQ7, 1 << u32(iop_irq::FDCDRQ) | 1 << u32(iop_irq::PERR))};

	enum class cpu_irq : unsigned
	{
		UBUS = 0, // Interprocessor communication interrupt from UPU
		IOP = 1, // Interprocessor communication interrupt from IOP
		TIMER = 2, // 100Hz timer
		FPA [[maybe_unused]] = 3, // FPU interrupt (handled by set_fpu)
		WRBERR = 4, // async bus write error
		PERR = 5 // Main memory parity error
	};

	static constexpr std::array CPU_IRQ_NAMES = {"UBUS"sv, "IOP"sv, "TIMER"sv, "FPA"sv, "WRBERR"sv, "PERR"sv};
	static constexpr u32 CPU_NMI_MASK = 1 << u32(cpu_irq::WRBERR) | 1 << u32(cpu_irq::IOP);
	static constexpr std::array CPU_IRQ_LINES = {
		std::make_pair(INPUT_LINE_IRQ0, cpu_irq::UBUS),
		std::make_pair(INPUT_LINE_IRQ1, cpu_irq::IOP),
		std::make_pair(INPUT_LINE_IRQ2, cpu_irq::TIMER),
		std::make_pair(INPUT_LINE_IRQ4, cpu_irq::WRBERR),
		std::make_pair(INPUT_LINE_IRQ5, cpu_irq::PERR)};

	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// address maps
	void cpu_map(address_map &map) ATTR_COLD;
	void iop_map(address_map &map) ATTR_COLD;
	void iop_vector_map(address_map &map) ATTR_COLD;

	// machine config
	void common(machine_config &config) ATTR_COLD;

	template<iop_irq Number>
	void irq_w(u8 state);
	template<iop_irq Number>
	void inten_w(u8 state);
	template<iop_irq Number>
	bool is_irq_set();
	void int_check_iop();

	template<cpu_irq Number>
	void irq_w(u8 state);
	template<cpu_irq Number>
	void inten_w(u8 state);
	template<cpu_irq Number>
	bool is_irq_set();
	void int_check_cpu();

	// Hardware-enforced test-and-set access to RAM
	u32 ram_tas_r(offs_t offset, u32 mem_mask);
	void ram_tas_w(offs_t offset, u32 data, u32 mem_mask);

	// IOP platform hardware
	u8 iop_intst_r();
	u8 iop_ipc_intst_r();
	u8 park_status_r();
	u32 iop_bus_error_r();
	void poweron_w(u8 data);
	void romdis_w(u8 data);
	void iop_parity_check_enable_w(u8 data);
	void iop_timer_w(u8 data);
	void ast_w(u8 data);
	void iopled_w(offs_t offset, u8 data);
	void xpustart_w(offs_t offset, u8 data);
	void index_divide_w(offs_t offset, u8 data);

	// CPU platform hardware
	u32 cpu_status_r();
	u32 cpu_bus_error_address_r();
	u8 boot_vector_r(offs_t offset);
	void cpu_parity_check_enable_w(u32 data);
	void cpu_timer_w(u32 data);
	void cpu_boot_vector_map_w(u32 data);
	void cpuled_w(offs_t offset, u32 data);
	void reset_cpu_registers();

	TIMER_CALLBACK_MEMBER(timer);

	// devices
	required_device<r3000a_device> m_cpu;
	required_device<m68030_device> m_iop;
	required_device<ram_device> m_ram;
	required_device_array<dmac_0266_device, 2> m_dma;
	required_device<rtc62421_device> m_rtc;
	required_device<z80scc_device> m_scc;
	required_device<am7990_device> m_net;
	required_device<n82077aa_device> m_fdc;

	required_device<news_hid_hle_device> m_hid;
	required_device_array<cxd1180_device, 2> m_scsi;

	required_device_array<rs232_port_device, 2> m_serial;
	required_device_array<nscsi_bus_device, 2> m_scsibus;
	required_device<centronics_device> m_parallel;
	optional_device<output_latch_device> m_parallel_data;

	required_region_ptr<u32> m_eprom;
	output_finder<4> m_led;
	std::unique_ptr<u16[]> m_net_ram;

	// For RAM test-and-set implementation
	memory_access<32, 2, 0, ENDIANNESS_BIG>::specific m_memory_access;

	emu_timer *m_timer = nullptr;

	// IOP hardware
	bool m_parallel_busy = false;
	bool m_parallel_fault = false;

	// CPU hardware
	bool m_mapvec = false;
	u32 m_wrbeadr = 0;

	// IOP IRQ state
	bool m_scc_irq_state = false;
	u32 m_iop_intst = 0;
	u32 m_iop_inten = 0;

	// CPU IRQ state
	u32 m_cpu_intst = 0;
	u32 m_cpu_inten = 0;
};

void news_38xx_state::machine_start()
{
	m_led.resolve();
	m_cpu->space(AS_PROGRAM).specific(m_memory_access);
	m_timer = timer_alloc(FUNC(news_38xx_state::timer), this);
	m_net_ram = std::make_unique<u16[]>(8192);

	save_pointer(NAME(m_net_ram), 8192);
	save_item(NAME(m_parallel_busy));
	save_item(NAME(m_parallel_fault));
	save_item(NAME(m_mapvec));
	save_item(NAME(m_wrbeadr));
	save_item(NAME(m_scc_irq_state));
	save_item(NAME(m_iop_intst));
	save_item(NAME(m_iop_inten));
	save_item(NAME(m_cpu_intst));
	save_item(NAME(m_cpu_inten));
}

void news_38xx_state::machine_reset()
{
	// For the IOP, EPROM is mapped at 0 after reset
	m_iop->space(0).install_rom(0x00000000, 0x0000ffff, m_eprom);

	// External timer is 100Hz
	m_timer->adjust(attotime::from_hz(100), 0, attotime::from_hz(100));

	// CPU does not run until the IOP tells it to
	m_cpu->set_input_line(INPUT_LINE_HALT, 1);

	// Clear platform hardware state
	m_scc_irq_state = false;
	m_iop_intst = 0;
	m_iop_inten = 0;
	reset_cpu_registers();
}

void news_38xx_state::init_common()
{
	// HACK: hardwire the rate
	m_fdc->set_rate(500'000);

	// RAM is always mapped for the CPU, since the MIPS boot vector is well above the max RAM value
	m_cpu->space(0).install_ram(0x00000000, m_ram->mask(), m_ram->pointer());
}

void news_38xx_state::cpu_map(address_map &map)
{
	map.global_mask(0x1fffffff); // A28-A0 are connected

	map(0x08000000, 0x0fffffff).rw(FUNC(news_38xx_state::ram_tas_r), FUNC(news_38xx_state::ram_tas_w));

	// 0x10000000 - 0x17ffffff UBUS

	// All registers below this line are 32 bit registers, LSB 1 = Set, 0 = Reset
	map(0x18000000, 0x18000003).r(FUNC(news_38xx_state::cpu_status_r));
	map(0x18000000, 0x18000003).w(FUNC(news_38xx_state::cpu_parity_check_enable_w)).mirror(0xffff00);
	map(0x18000004, 0x18000007).w(FUNC(news_38xx_state::cpu_timer_w)).mirror(0xffff00);
	map(0x18000020, 0x18000023).lw32(NAME([this] (u32) { irq_w<iop_irq::CPU>(1); })).mirror(0xffff00);
	map(0x18000024, 0x18000027).lw32(
		NAME([] (u32) { fatalerror("CPU tried to interrupt UBUS without UPU installed!"); })).mirror(0xffff00);
	map(0x18000040, 0x18000043).w(FUNC(news_38xx_state::cpu_boot_vector_map_w)).mirror(0xffff00);
	map(0x18000044, 0x18000047).w(FUNC(news_38xx_state::inten_w<cpu_irq::UBUS>)).mirror(0xffff00);
	map(0x18000060, 0x18000063).lw32(NAME([this] (u32) { irq_w<cpu_irq::IOP>(0); })).mirror(0xffff00);
	map(0x18000064, 0x18000067).lw32(NAME([this] (u32) { irq_w<cpu_irq::UBUS>(0); })).mirror(0xffff00);
	map(0x18000080, 0x18000087).w(FUNC(news_38xx_state::cpuled_w)).mirror(0xffff00);
	map(0x1c000000, 0x1c000003).r(FUNC(news_38xx_state::cpu_bus_error_address_r));
	map(0x1f000000, 0x1fffffff).r(FUNC(news_38xx_state::boot_vector_r)); // MAPVEC forces A28~24 to 0
}

void news_38xx_state::iop_map(address_map &map)
{
	map.global_mask(0x3fffffff); // A29-A0 are connected

	map(0x08000000, 0x0fffffff).rw(FUNC(news_38xx_state::ram_tas_r), FUNC(news_38xx_state::ram_tas_w));

	map(0x18000000, 0x1803ffff).ram(); // IOP ram
	map(0x20000000, 0x2000ffff).rom().region("eprom", 0).mirror(0x1fff0000);

	// Platform control hardware
	map(0x22000000, 0x22000000).w(FUNC(news_38xx_state::poweron_w));
	map(0x22000001, 0x22000001).w(FUNC(news_38xx_state::romdis_w));
	map(0x22000002, 0x22000002).w(FUNC(news_38xx_state::iop_parity_check_enable_w));
	map(0x22000003, 0x22000003).w(FUNC(news_38xx_state::iop_timer_w));
	map(0x22000004, 0x22000004).w(FUNC(news_38xx_state::irq_w<iop_irq::SOFTINTR>));
	map(0x22000005, 0x22000005).w(FUNC(news_38xx_state::ast_w));
	map(0x22000006, 0x22000007).w(FUNC(news_38xx_state::iopled_w));

	map(0x22800000, 0x22800001).w(FUNC(news_38xx_state::xpustart_w));
	map(0x22800008, 0x22800008).lw8(NAME([this] (u8) { irq_w<cpu_irq::IOP>(1); }));
	map(0x22800009, 0x22800009).
			lw8(NAME([] (u8) { fatalerror("IOP tried to interrupt UBUS without UPU installed!"); }));
	map(0x22800010, 0x22800010).w(FUNC(news_38xx_state::inten_w<iop_irq::CPU>));
	map(0x22800011, 0x22800011).w(FUNC(news_38xx_state::inten_w<iop_irq::UBUS>));
	map(0x22800018, 0x22800018).lw8(NAME([this] (u8) { irq_w<iop_irq::CPU>(0); }));
	map(0x22800019, 0x22800019).lw8(NAME([this] (u8) { irq_w<iop_irq::UBUS>(0); }));

	map(0x23000000, 0x23000000).r(FUNC(news_38xx_state::iop_intst_r));
	map(0x23800000, 0x23800000).r(FUNC(news_38xx_state::iop_ipc_intst_r));

	map(0x24000000, 0x24000007).m(m_fdc, FUNC(n82077aa_device::map));
	map(0x24000105, 0x24000105).rw(m_fdc, FUNC(n82077aa_device::dma_r), FUNC(n82077aa_device::dma_w));

	map(0x26040000, 0x26040000).lw8(NAME([this] (u8 data) {
		LOGMASKED(LOG_PARALLEL, "Parallel data w 0x%x\n", data);
		m_parallel_data->write(data);
		}));
	map(0x26040001, 0x26040001).lw8(NAME([this] (u8 data) {
		LOGMASKED(LOG_PARALLEL, "Parallel strobe w 0x%x\n", data);
		m_parallel->write_strobe(!data);
		}));
	map(0x26040002, 0x26040002).lw8(NAME([this] (u8) { irq_w<iop_irq::PARALLEL>(0);}));
	map(0x26040003, 0x26040003).w(FUNC(news_38xx_state::inten_w<iop_irq::PARALLEL>));

	map(0x26080000, 0x26080007).m(m_scsi[0], FUNC(cxd1180_device::map));
	map(0x260c0000, 0x260c0007).m(m_scsi[1], FUNC(cxd1180_device::map));

	map(0x26100000, 0x26100007).m(m_hid, FUNC(news_hid_hle_device::map_68k));
	map(0x26140000, 0x26140003).rw(m_scc, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w));
	map(0x26180000, 0x2618000f).rw(m_rtc, FUNC(rtc62421_device::read), FUNC(rtc62421_device::write));

	// 0x261c0000 // kb beep frequency

	map(0x26200000, 0x26203fff).lrw16(
		[this](offs_t offset) { return m_net_ram[offset]; }, "net_ram_r",
		[this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_net_ram[offset]); }, "net_ram_w");

	// 0x262c0000 // audio

	map(0x26300000, 0x26300003).rw(m_net, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w));

	map(0x26340000, 0x26340000).r(FUNC(news_38xx_state::park_status_r));
	map(0x26380000, 0x26380000).w(FUNC(news_38xx_state::index_divide_w));

	map(0x28000000, 0x28000017).m(m_dma[0], FUNC(dmac_0266_device::map));
	map(0x2a000000, 0x2a000017).m(m_dma[1], FUNC(dmac_0266_device::map));

	map(0x2c000000, 0x2c0000ff).rom().region("idrom", 0);
	map(0x2c000100, 0x2c000103).portr("SW1");

	map(0x2e000000, 0x2effffff).r(FUNC(news_38xx_state::iop_bus_error_r)).mirror(0x10000000); // Expansion I/O
}

void news_38xx_state::iop_vector_map(address_map &map)
{
	map(0xfffffff3, 0xfffffff3).lr8(NAME([] { return m68000_base_device::autovector(1); }));
	map(0xfffffff5, 0xfffffff5).lr8(NAME([] { return m68000_base_device::autovector(2); }));
	map(0xfffffff7, 0xfffffff7).lr8(NAME([] { return m68000_base_device::autovector(3); }));
	map(0xfffffff9, 0xfffffff9).lr8(NAME([] { return m68000_base_device::autovector(4); }));
	map(0xfffffffb, 0xfffffffb).lr8(
		NAME([this] { return m_scc_irq_state ? m_scc->m1_r() : m68000_base_device::autovector(5); }));
	map(0xfffffffd, 0xfffffffd).lr8(NAME([] { return m68000_base_device::autovector(6); }));
	map(0xffffffff, 0xffffffff).lr8(NAME([] { return m68000_base_device::autovector(7); }));
}

u32 news_38xx_state::ram_tas_r(offs_t offset, u32 mem_mask)
{
	if (mem_mask != 0xffffffff)
	{
		fatalerror("%s tried to acquire lock with unaligned memory access!", machine().describe_context());
	}

	constexpr u32 LOCK_VALUE = 0x80000000;
	const u32 current_value = m_memory_access.read_dword(offset << 2);
	if (machine().side_effects_disabled())
	{
		return current_value;
	}

	if (current_value < LOCK_VALUE)
	{
		LOGMASKED(LOG_TAS, "%s acquired lock at offset 0x%x\n", machine().describe_context(), offset << 2);
	}

	// TODO: what is the actual value written by the hardware?
	//       Both vmunix and mrx appear to use bltz (r3k)/bmi (030) instructions for lock spin
	m_memory_access.write_dword(offset << 2, LOCK_VALUE);
	return current_value;
}

void news_38xx_state::ram_tas_w(offs_t offset, u32 data, u32 mem_mask)
{
	if (data && !machine().side_effects_disabled())
	{
		fatalerror("lock released by %s with non-zero data!", machine().describe_context());
	}

	if (mem_mask != 0xffffffff)
	{
		fatalerror("%s tried to release lock with unaligned memory access!", machine().describe_context());
	}

	offset <<= 2;
	if (m_memory_access.read_dword(offset) != 0x80000000)
	{
		fatalerror("%s tried to release unheld lock!");
	}

	LOGMASKED(LOG_TAS, "%s releasing lock at offset 0x%x\n", machine().describe_context(), offset);
	m_memory_access.write_dword(offset, data);
}

u8 news_38xx_state::iop_intst_r()
{
	const u8 iop_status = is_irq_set<iop_irq::PERR>() << 7 |
						  is_irq_set<iop_irq::SCSI0>() << 6 |
						  is_irq_set<iop_irq::SCSI1>() << 5 |
						  (is_irq_set<iop_irq::CPU>() || is_irq_set<iop_irq::UBUS>()) << 4 |
						  is_irq_set<iop_irq::LANCE>() << 3 |
						  is_irq_set<iop_irq::FDCIRQ>() << 2 |
						  is_irq_set<iop_irq::PARALLEL>() << 1 |
						  is_irq_set<iop_irq::SLOT>();
	LOGMASKED(LOG_INTERRUPT, "%s iop_intst_r: 0x%x\n", machine().describe_context(), iop_status);
	return iop_status;
}

u8 news_38xx_state::iop_ipc_intst_r()
{
	const u8 iop_status = is_irq_set<iop_irq::UBUS>() << 1 | is_irq_set<iop_irq::CPU>();
	LOGMASKED(LOG_INTERRUPT, "%s iop_ipc_intst_r: 0x%x\n", machine().describe_context(), iop_status);
	return iop_status;
}

u8 news_38xx_state::park_status_r()
{
	const u8 park_status = (m_parallel_fault ? 0x40 : 0x0) |
						   (m_serial[0]->dsr_r() ? 0x20 : 0x0) |
						   (m_parallel_busy ? 0x10 : 0x0) |
						   (!is_irq_set<iop_irq::PARALLEL>() ? 0x8 : 0x0) |
						   (m_serial[0]->ri_r() ? 0x4 : 0x0) |
						   (m_serial[1]->dsr_r() ? 0x2 : 0x0) |
						   (m_serial[1]->ri_r() ? 0x1 : 0x0);
	LOGMASKED(LOG_INTERRUPT, "%s park_status_r: 0x%x\n", machine().describe_context(), park_status);
	return park_status;
}

template<news_38xx_state::iop_irq Number>
void news_38xx_state::irq_w(const u8 state)
{
	if (Number != iop_irq::TIMER)
	{
		LOGMASKED(LOG_INTERRUPT, "(%s) IOP IRQ %s %s\n", machine().describe_context(),
				  IOP_IRQ_NAMES[(unsigned)Number], state ? "set" : "cleared");
	}
	else
	{
		LOGMASKED(LOG_TIMER, "(%s) IOP IRQ %s %s\n", machine().describe_context(),
				  IOP_IRQ_NAMES[(unsigned)Number], state ? "set" : "cleared");
	}

	if (state)
	{
		m_iop_intst |= 1U << u32(Number);
	}
	else
	{
		m_iop_intst &= ~(1U << u32(Number));
	}

	int_check_iop();
}

template<news_38xx_state::iop_irq Number>
void news_38xx_state::inten_w(const u8 state)
{
	if (Number != iop_irq::TIMER)
	{
		LOGMASKED(LOG_INTERRUPT, "(%s) IOP IRQ %s %s\n", machine().describe_context(),
				  IOP_IRQ_NAMES[(unsigned)Number], state ? ENABLED : DISABLED);
	}
	else
	{
		LOGMASKED(LOG_TIMER, "(%s) IOP IRQ %s %s\n", machine().describe_context(),
				  IOP_IRQ_NAMES[(unsigned)Number], state ? ENABLED : DISABLED);
	}

	if (state)
	{
		m_iop_inten |= 1 << u32(Number);
	}
	else
	{
		m_iop_inten &= ~(1 << u32(Number));
	}

	int_check_iop();
}

template<news_38xx_state::iop_irq Number>
bool news_38xx_state::is_irq_set()
{
	return BIT(m_iop_intst, u32(Number));
}

void news_38xx_state::int_check_iop()
{
	const u32 active_irq = m_iop_intst & (m_iop_inten | IOP_NMI_MASK);
	for (const auto &[input_line, line_mask]: IOP_LINE_MASK)
	{
		// Calculate state of input pin (logical OR of all attached inputs)
		const bool state = active_irq & line_mask;

		// Update input pin status if it has changed
		if (m_iop->input_line_state(input_line) != state)
		{
			if (input_line != INPUT_LINE_IRQ6)
			{
				LOGMASKED(LOG_INTERRUPT, "Setting IOP input line %d to %d\n", input_line, state ? 1 : 0);
			}

			m_iop->set_input_line(input_line, state ? 1 : 0);
		}
	}
}

template<news_38xx_state::cpu_irq Number>
void news_38xx_state::irq_w(const u8 state)
{
	if (Number != cpu_irq::TIMER)
	{
		LOGMASKED(LOG_INTERRUPT, "(%s) CPU IRQ %s %s\n", machine().describe_context(),
				  CPU_IRQ_NAMES[(unsigned)Number], state ? "set" : "cleared");
	}
	else
	{
		LOGMASKED(LOG_TIMER, "(%s) CPU IRQ %s %s\n", machine().describe_context(),
				  CPU_IRQ_NAMES[(unsigned)Number], state ? "set" : "cleared");
	}

	if (state)
	{
		m_cpu_intst |= 1U << u32(Number);
	}
	else
	{
		m_cpu_intst &= ~(1U << u32(Number));
	}

	int_check_cpu();
}

template<news_38xx_state::cpu_irq Number>
void news_38xx_state::inten_w(const u8 state)
{
	if (Number != cpu_irq::TIMER)
	{
		LOGMASKED(LOG_INTERRUPT, "(%s) CPU IRQ %s %s\n", machine().describe_context(),
				  CPU_IRQ_NAMES[(unsigned)Number], state ? ENABLED : DISABLED);
	}
	else
	{
		LOGMASKED(LOG_TIMER, "(%s) CPU IRQ %s %s\n", machine().describe_context(),
				  CPU_IRQ_NAMES[(unsigned)Number], state ? ENABLED : DISABLED);
	}

	if (state)
	{
		m_cpu_inten |= 1 << u32(Number);
	}
	else
	{
		m_cpu_inten &= ~(1 << u32(Number));
	}

	int_check_cpu();
}

template<news_38xx_state::cpu_irq Number>
bool news_38xx_state::is_irq_set()
{
	return BIT(m_cpu_intst, u32(Number));
}

void news_38xx_state::int_check_cpu()
{
	const u32 active_irq = m_cpu_intst & (m_cpu_inten | CPU_NMI_MASK);
	for (const auto &[input_line, irq_input] : CPU_IRQ_LINES)
	{
		// Update input pin status if it has changed
		const bool state = BIT(active_irq, u32(irq_input));
		if (m_cpu->input_line_state(input_line) != state)
		{
			if (input_line != INPUT_LINE_IRQ2)
			{
				LOGMASKED(LOG_INTERRUPT, "Setting CPU input line %d to %d\n", input_line, state ? 1 : 0);
			}

			m_cpu->set_input_line(input_line, state ? 1 : 0);
		}
	}
}

u32 news_38xx_state::iop_bus_error_r()
{
	if (!machine().side_effects_disabled())
		m_iop->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);

	return 0;
}

TIMER_CALLBACK_MEMBER(news_38xx_state::timer)
{
	// 100Hz clock from PARK is used to generate both IOP and CPU interrupts
	if (BIT(m_iop_inten, u32(iop_irq::TIMER)))
	{
		irq_w<iop_irq::TIMER>(1);
	}

	if (BIT(m_cpu_inten, u32(cpu_irq::TIMER)))
	{
		irq_w<cpu_irq::TIMER>(1);
	}
}

void news_38xx_state::poweron_w(u8 data)
{
	LOGMASKED(LOG_IOP, "(%s) Write POWERON = 0x%x\n", machine().describe_context(), data);

	if (!machine().side_effects_disabled() && !data)
	{
		machine().schedule_exit();
	}
}

void news_38xx_state::romdis_w(u8 data)
{
	LOGMASKED(LOG_IOP, "(%s) IOP ROMDIS = 0x%x\n", machine().describe_context(), data);

	if (data)
	{
		m_iop->space(0).install_ram(0x00000000, m_ram->mask(), m_ram->pointer());
	}
	else
	{
		m_iop->space(0).install_rom(0x00000000, 0x0000ffff, m_eprom);
	}
}

void news_38xx_state::iop_parity_check_enable_w(u8 data)
{
	LOGMASKED(LOG_IOP, "(%s) IOP parity checking %s\n", machine().describe_context(), data ? ENABLED : DISABLED);

	inten_w<iop_irq::PERR>(data > 0);
	if (!data)
	{
		irq_w<iop_irq::PERR>(0);
	}
}

void news_38xx_state::iop_timer_w(u8 data)
{
	LOGMASKED(LOG_TIMER, "(%s) IOP timer %s\n", machine().describe_context(), data ? ENABLED : DISABLED);

	inten_w<iop_irq::TIMER>(data > 0);
	if (!data)
	{
		irq_w<iop_irq::TIMER>(0);
	}
}

void news_38xx_state::ast_w(u8 data)
{
	LOGMASKED(LOG_IOP, "(%s) AST interrupt %s\n", machine().describe_context(), data ? ENABLED : DISABLED);
	// TODO: how to trigger AST IRQ when IOP enters user mode? No easy injection point like my experiments with NWS-831.
	//       mrx doesn't seem to use it, but that could be due to other issues.
}

void news_38xx_state::iopled_w(offs_t offset, u8 data)
{
	constexpr int IOP_LED_BASE = 2;
	m_led[IOP_LED_BASE + offset % 2] = data > 0;
}

void news_38xx_state::xpustart_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_IOP, "(%s) %s %cPU\n", machine().describe_context(), data ? "Starting" : "Stopping",
			  !offset ? 'C' : 'U');

	if (!offset)
	{
		m_cpu->set_input_line(INPUT_LINE_HALT, data ? 0 : 1);
		if (!data)
		{
			reset_cpu_registers();
		}
	}
	else
	{
		if (data) fatalerror("Tried to start UPU without UPU installed!");
	}
}

void news_38xx_state::index_divide_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_IOP, "(%s) Set divide index to %s\n", machine().describe_context(), data ? "divide" : "passthrough");
	// TODO: what to do with this?
	//       I think it divides INDX pulse by half (2x INDX pulses from the drive to make 1 go to the FDC controller)
}

u32 news_38xx_state::cpu_status_r()
{
	const u8 cpu_status = is_irq_set<cpu_irq::UBUS>() << 1 | is_irq_set<cpu_irq::IOP>();
	LOGMASKED(LOG_INTERRUPT, "%s read CPU status 0x%x\n", machine().describe_context(), cpu_status);
	return cpu_status;
}

u32 news_38xx_state::cpu_bus_error_address_r()
{
	LOGMASKED(LOG_CPU, "(%s) Read last bus error address = 0x%x\n", machine().describe_context(), m_wrbeadr);
	// This will need to be properly implemented if anything is added that can cause a CPU bus error
	return m_wrbeadr;
}

u8 news_38xx_state::boot_vector_r(offs_t offset)
{
	if (!m_mapvec)
	{
		return m_memory_access.read_byte(offset);
	}

	// In reality, this probably just causes a bus error, returns 0x0/0xff, or something like that
	fatalerror("CPU tried to read from boot vector space without boot vector mapping!");
}

void news_38xx_state::cpu_parity_check_enable_w(u32 data)
{
	LOGMASKED(LOG_CPU, "(%s) Write CPU parity check enable = 0x%x\n", machine().describe_context(), data);

	inten_w<cpu_irq::PERR>(data > 0);
	if (!data)
	{
		irq_w<cpu_irq::PERR>(0);
	}
}

void news_38xx_state::cpu_timer_w(u32 data)
{
	LOGMASKED(LOG_TIMER, "(%s) CPU timer %s\n", machine().describe_context(), data ? ENABLED : DISABLED);

	inten_w<cpu_irq::TIMER>(data > 0);
	if (!data)
	{
		irq_w<cpu_irq::TIMER>(0);
	}
}

void news_38xx_state::cpu_boot_vector_map_w(u32 data)
{
	LOGMASKED(LOG_CPU, "(%s) CPU boot vector mapping %s\n", machine().describe_context(), data ? DISABLED : ENABLED);
	m_mapvec = data > 0;
}

void news_38xx_state::cpuled_w(offs_t offset, u32 data)
{
	m_led[offset % 2] = data > 0;
}

void news_38xx_state::reset_cpu_registers()
{
	m_mapvec = false;
	m_wrbeadr = 0;
	m_cpu_intst = 0;
	m_cpu_inten = 0;
}

void news_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("cdrom", NSCSI_CDROM);
	device.option_add("tape", NSCSI_TAPE);
}

void news_38xx_state::common(machine_config &config)
{
	// 40MHz goes into the R3000 pkg, but is divided internally to 20MHz
	R3000A(config, m_cpu, 40_MHz_XTAL / 2, 65536, 65536);
	m_cpu->set_addrmap(AS_PROGRAM, &news_38xx_state::cpu_map);
	m_cpu->set_fpu(mips1_device_base::MIPS_R3010Av4, INPUT_LINE_IRQ3);

	M68030(config, m_iop, 40_MHz_XTAL / 2);
	m_iop->set_addrmap(AS_PROGRAM, &news_38xx_state::iop_map);
	m_iop->set_addrmap(m68000_base_device::AS_CPU_SPACE, &news_38xx_state::iop_vector_map);

	// For NWS3840/3860: 16M onboard
	// With NWB-109 expansion kit: 16M onboard + 16M (1Mbit DRAM chips) expansion = 32M total
	// With NWB-112 expansion kit: 16M onboard + 64M (4Mbit DRAM chips) expansion = 80M total
	RAM(config, m_ram);
	m_ram->set_default_size("16M");
	m_ram->set_extra_options("32M,80M");

	RTC62421(config, m_rtc, 32.768_kHz_XTAL);

	DMAC_0266(config, m_dma[0], 0);
	m_dma[0]->set_bus(m_iop, 0);

	DMAC_0266(config, m_dma[1], 0);
	m_dma[1]->set_bus(m_iop, 0);

	SCC85C30(config, m_scc, 4.9152_MHz_XTAL);
	m_scc->out_int_callback().set(
		[this](const int state) {
			m_scc_irq_state = (bool)state;
			irq_w<iop_irq::SCC>(state != 0);
		});

	// scc channel A
	RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
	m_serial[0]->cts_handler().set(m_scc, FUNC(z80scc_device::ctsa_w));
	m_serial[0]->dcd_handler().set(m_scc, FUNC(z80scc_device::dcda_w));
	m_serial[0]->rxd_handler().set(m_scc, FUNC(z80scc_device::rxa_w));
	m_scc->out_rtsa_callback().set(m_serial[0], FUNC(rs232_port_device::write_rts));
	m_scc->out_txda_callback().set(m_serial[0], FUNC(rs232_port_device::write_txd));

	// scc channel B
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
	m_serial[1]->cts_handler().set(m_scc, FUNC(z80scc_device::ctsb_w));
	m_serial[1]->dcd_handler().set(m_scc, FUNC(z80scc_device::dcdb_w));
	m_serial[1]->rxd_handler().set(m_scc, FUNC(z80scc_device::rxb_w));
	m_scc->out_rtsb_callback().set(m_serial[1], FUNC(rs232_port_device::write_rts));
	m_scc->out_txdb_callback().set(m_serial[1], FUNC(rs232_port_device::write_txd));

	AM7990(config, m_net, 20_MHz_XTAL / 2);
	m_net->intr_out().set(FUNC(news_38xx_state::irq_w<iop_irq::LANCE>)).invert();
	m_net->dma_in().set([this](const offs_t offset) { return m_net_ram[offset >> 1]; });
	m_net->dma_out().set([this](const offs_t offset, const u16 data, const u16 mem_mask) {
		COMBINE_DATA(&m_net_ram[offset >> 1]);
	});

	N82077AA(config, m_fdc, 24_MHz_XTAL, n82077aa_device::mode_t::PS2);
	m_fdc->intrq_wr_callback().set(FUNC(news_38xx_state::irq_w<iop_irq::FDCIRQ>));
	m_fdc->drq_wr_callback().set(FUNC(news_38xx_state::irq_w<iop_irq::FDCDRQ>));
	FLOPPY_CONNECTOR(config, "fdc:0", "35hd", FLOPPY_35_HD, true, floppy_image_device::default_pc_floppy_formats).
			enable_sound(false);

	// scsi bus 0 and devices
	NSCSI_BUS(config, m_scsibus[0]);
	NSCSI_CONNECTOR(config, "scsi0:0", news_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi0:1", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:2", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:3", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:4", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:5", news_scsi_devices, "tape");
	NSCSI_CONNECTOR(config, "scsi0:6", news_scsi_devices, nullptr);

	// scsi bus 0 host adapter
	NSCSI_CONNECTOR(config, "scsi0:7").option_set("cxd1180", CXD1180).machine_config(
		[this](device_t *device) {
			auto &adapter = downcast<cxd1180_device &>(*device);
			adapter.set_clock(20_MHz_XTAL / 2);

			adapter.irq_handler().set(*this, FUNC(news_38xx_state::irq_w<iop_irq::SCSI0>));
			adapter.irq_handler().append(m_dma[0], FUNC(dmac_0266_device::eop_w));
			adapter.drq_handler().set(m_dma[0], FUNC(dmac_0266_device::req_w));

			subdevice<dmac_0266_device>(":dma0")->dma_r_cb().set(adapter, FUNC(cxd1180_device::dma_r));
			subdevice<dmac_0266_device>(":dma0")->dma_w_cb().set(adapter, FUNC(cxd1180_device::dma_w));
		});

	// scsi bus 1 and devices
	NSCSI_BUS(config, m_scsibus[1]);
	NSCSI_CONNECTOR(config, "scsi1:0", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:1", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:2", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:3", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:4", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:5", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:6", news_scsi_devices, nullptr);

	// scsi bus 1 host adapter
	NSCSI_CONNECTOR(config, "scsi1:7").option_set("cxd1180", CXD1180).machine_config(
		[this](device_t *device) {
			auto &adapter = downcast<cxd1180_device &>(*device);
			adapter.set_clock(20_MHz_XTAL / 2);

			adapter.irq_handler().set(*this, FUNC(news_38xx_state::irq_w<iop_irq::SCSI1>));
			adapter.irq_handler().append(m_dma[1], FUNC(dmac_0266_device::eop_w));
			adapter.drq_handler().set(m_dma[1], FUNC(dmac_0266_device::req_w));

			subdevice<dmac_0266_device>(":dma1")->dma_r_cb().set(adapter, FUNC(cxd1180_device::dma_r));
			subdevice<dmac_0266_device>(":dma1")->dma_w_cb().set(adapter, FUNC(cxd1180_device::dma_w));
		});

	CENTRONICS(config, m_parallel, centronics_devices, nullptr);
	// Note: printing works, but I'm not sure how accurate triggering the IRQ on each edge is.
	// mrx properly checks the PARK status before taking action, but this might not be what real hw does
	m_parallel->busy_handler().set([this](const int status) {
		const bool new_status = status;
		if (m_parallel_busy != new_status)
		{
			LOGMASKED(LOG_PARALLEL, "Parallel busy changed to %s\n", new_status ? "H" : "L");
			m_parallel_busy = new_status;
			irq_w<iop_irq::PARALLEL>(1);
		}
	});
	m_parallel->fault_handler().set([this](const int status) {
		const bool new_status = status;
		if (m_parallel_fault != new_status)
		{
			LOGMASKED(LOG_PARALLEL, "Parallel fault changed to %s\n", new_status ? "H" : "L");
			m_parallel_fault = !new_status;
			irq_w<iop_irq::PARALLEL>(1);
		}
	});

	OUTPUT_LATCH(config, m_parallel_data);
	m_parallel->set_output_latch(*m_parallel_data);

	NEWS_HID_HLE(config, m_hid);
	m_hid->irq_out<news_hid_hle_device::KEYBOARD>().set(*this, FUNC(news_38xx_state::irq_w<iop_irq::KEYBOARD>));
	m_hid->irq_out<news_hid_hle_device::MOUSE>().set(*this, FUNC(news_38xx_state::irq_w<iop_irq::MOUSE>));

	SOFTWARE_LIST(config, "software_list").set_original("sony_news").set_filter("RISC,NWS3000");
}

void news_38xx_state::nws3860(machine_config &config)
{
	common(config);
}

ROM_START(nws3860)
	ROM_REGION32_BE(0x10000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "nws3860", "SONY NET WORK STATION MC68030 Monitor Release 1.6")
	ROMX_LOAD("mpu_10__ver.1.6__1990_sony.ic159", 0x00000, 0x10000, CRC(542e21b8) SHA1(631dca1f3446761973073f5c32c1a0aeba538c2c), ROM_BIOS(0))

	ROM_REGION32_BE(0x100, "idrom", 0)
	ROM_LOAD("idrom.bin", 0x000, 0x100, CRC(a2b9e968) SHA1(09c7d253c7ed0b368c49f4b60bfe5ca76acd7cc3) BAD_DUMP)
ROM_END

INPUT_PORTS_START(nws3860)
	PORT_START("SW1")
	PORT_DIPNAME(0x07000000, 0x07000000, "Display") PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(0x07000000, "Console")
	PORT_DIPSETTING(0x06000000, "NWB-512")
	PORT_DIPSETTING(0x03000000, "NWB-225A")
	PORT_DIPSETTING(0x00000000, "Autoselect")

	PORT_DIPNAME(0x08000000, 0x08000000, "Boot Device") PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(0x08000000, "SCSI")
	PORT_DIPSETTING(0x00000000, "Floppy")

	PORT_DIPNAME(0x10000000, 0x10000000, "Automatic Boot") PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(0x10000000, DEF_STR(Off))
	PORT_DIPSETTING(0x00000000, DEF_STR(On))

	PORT_DIPNAME(0x20000000, 0x20000000, "Diagnostic Mode") PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(0x20000000, DEF_STR(Off))
	PORT_DIPSETTING(0x00000000, DEF_STR(On))

	PORT_DIPNAME(0xc0000000, 0xc0000000, "Unused") PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(0xc0000000, DEF_STR(Off))
INPUT_PORTS_END

} // anonymous namespace


/*   YEAR  NAME     P  C  MACHINE  INPUT    CLASS            INIT         COMPANY  FULLNAME   FLAGS */
COMP(1989, nws3860, 0, 0, nws3860, nws3860, news_38xx_state, init_common, "Sony", "NWS-3860", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
