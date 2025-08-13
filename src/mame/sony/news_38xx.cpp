// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Sony NEWS NWS-38xx workstations
 *
 * The NWS-3800 series was the last model of NEWS workstation to use the original NWS-8xx/9xx architecture of
 * having a CPU running NEWS-OS (handling the processing of user code) and a CPU running a separate OS (iopboot/rtx/mrx)
 * to handle system I/O. Unlike the 8xx/9xx and 18xx/19xx, the NWS-3800 is a mixed architecture system. It uses a MIPS
 * R3000 CPU for NEWS-OS and user programs, and a 68030 running rtx (NEWS-OS 3) or mrx (NEWS-OS 4) as the IOP.
 *
 * Known NWS-3800 configurations
 *  - TODO
 *
 * Sources:
 *  - NWS-3840/3860 Service Guide
 *  - http://bitsavers.org/pdf/sony/news/Sony_NEWS_Technical_Manual_3ed_199103.pdf
 *  - https://katsu.watanabe.name/doc/sonynews/model.html
 *
 * TODO:
 *   - slots (I/O expansion and UBUS expansion)
 *   - graphics
 *   - sound
 *   - centronics
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
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/rs232/rs232.h"

#include "imagedev/floppy.h"

#define LOG_INTERRUPT (1U << 1)
#define LOG_TIMER (1U << 2)
#define LOG_LED (1U << 3)
#define LOG_TAS (1U << 4)

#define VERBOSE (LOG_GENERAL|LOG_INTERRUPT|LOG_TAS)
#include "logmacro.h"


namespace {

using namespace std::literals::string_view_literals;

class news_38xx_state : public driver_device
{
public:
	news_38xx_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_iop(*this, "iop")
		, m_ram(*this, "ram")
		, m_dma(*this, "dma%u", 0U)
		, m_rtc(*this, "rtc")
		, m_scc(*this, "scc")
		, m_net(*this, "net")
		, m_fdc(*this, "fdc")
		, m_hid(*this, "hid")
		, m_scsi(*this, "scsi%u:7:cxd1180", 0U)
		, m_serial(*this, "serial%u", 0U)
		, m_scsibus(*this, "scsi%u", 0U)
		, m_eprom(*this, "eprom")
		, iop_irq_line_map({
			{INPUT_LINE_IRQ1, {iop_irq::AST}},
			{INPUT_LINE_IRQ2, {iop_irq::SOFTINTR}},
			{INPUT_LINE_IRQ3, {iop_irq::FDCIRQ, iop_irq::PARALLEL, iop_irq::SLOT}},
			{INPUT_LINE_IRQ4, {iop_irq::LANCE, iop_irq::SCSI0, iop_irq::SCSI1, iop_irq::CPU, iop_irq::UBUS}},
			{INPUT_LINE_IRQ5, {iop_irq::SCC, iop_irq::KEYBOARD, iop_irq::MOUSE}},
			{INPUT_LINE_IRQ6, {iop_irq::TIMER}},
			{INPUT_LINE_IRQ7, {iop_irq::FDCDRQ, iop_irq::PERR}}
		})
		, cpu_irq_line_map({
			{INPUT_LINE_IRQ0, cpu_irq::UBUS},
			{INPUT_LINE_IRQ1, cpu_irq::IOP},
			{INPUT_LINE_IRQ2, cpu_irq::TIMER},
			{INPUT_LINE_IRQ3, cpu_irq::FPA},
			{INPUT_LINE_IRQ4, cpu_irq::WRBERR},
			{INPUT_LINE_IRQ5, cpu_irq::PERR}
		})
//      , m_led(*this, "led%u", 0U)
	{
	}

protected:
	// driver_device overrides
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

	// address maps
	void cpu_map(address_map &map) ATTR_COLD;
	void iop_map(address_map &map) ATTR_COLD;
	void iop_vector_map(address_map &map) ATTR_COLD;

	// machine config
	void common(machine_config &config);

public:
	void nws3860(machine_config &config);

	void init_common();

protected:
	// Hardware-enforced test-and-set access to RAM
	u32 ram_tas_r(offs_t offset, u32 mem_mask);
	void ram_tas_w(offs_t offset, u32 data, u32 mem_mask);

	u8 intst_r();
	u8 iop_ipc_intst_r();

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

	static constexpr std::array iop_irq_names = {
		"AST"sv, "SOFTINTR"sv, "FDCIRQ"sv, "PARALLEL"sv, "SLOT"sv, "LANCE"sv, "SCSI0"sv, "SCSI1"sv, "CPU"sv, "UBUS"sv,
		"SCC"sv, "KEYBOARD"sv, "MOUSE"sv, "TIMER"sv, "FDCDRQ"sv, "PERR"sv
	};
	static constexpr uint32_t iop_nmi_mask = 1 << static_cast<uint32_t>(iop_irq::FDCDRQ) |
	                                         1 << static_cast<uint32_t>(iop_irq::SCC) |
	                                         1 << static_cast<uint32_t>(iop_irq::SCSI0) |
	                                         1 << static_cast<uint32_t>(iop_irq::SCSI1) |
	                                         1 << static_cast<uint32_t>(iop_irq::LANCE) |
	                                         1 << static_cast<uint32_t>(iop_irq::FDCIRQ) |
	                                         1 << static_cast<uint32_t>(iop_irq::SLOT) |
	                                         1 << static_cast<uint32_t>(iop_irq::SOFTINTR) |
	                                         1 << static_cast<uint32_t>(iop_irq::AST);

	template<iop_irq Number>
	void iop_irq_w(u8 state); // TODO: see if template deduction can work for having irq_w overloaded
	template<iop_irq Number>
	void iop_inten_w(uint8_t state);
	template<iop_irq Number>
	bool is_iop_irq_set();
	void int_check_iop();

	enum class cpu_irq : unsigned
	{
		UBUS = 0,   // Interprocessor communication interrupt from UPU
		IOP = 1,    // Interprocessor communication interrupt from IOP
		TIMER = 2,  // 100Hz timer
		FPA = 3,    // FPU interrupt
		WRBERR = 4, // async bus write error
		PERR = 5    // Main memory parity error
	};

	static constexpr std::array cpu_irq_names = {"UBUS"sv, "IOP"sv, "TIMER"sv, "FPA"sv, "WRBERR"sv, "PERR"sv};
	static constexpr uint32_t cpu_nmi_mask = 1 << static_cast<uint32_t>(cpu_irq::WRBERR) |
	                                         1 << static_cast<uint32_t>(cpu_irq::FPA) |
	                                         1 << static_cast<uint32_t>(cpu_irq::IOP);

	template <cpu_irq Number>
	void cpu_irq_w(u8 state);
	template <cpu_irq Number>
	void cpu_inten_w(uint8_t state);
	template<cpu_irq Number>
	bool is_cpu_irq_set();
	void int_check_cpu();

	// IOP platform hardware
	u32 iop_bus_error_r();
	void poweron_w(u8 data);
	void romdis_w(u8 data);
	void ptycken_w(u8 data);
	void timeren_w(u8 data);
	void astintr_w(u8 data);
	void iopled_w(offs_t offset, u8 data); // IOPLED0/IOPLED1
	void xpustart_w(offs_t offset, u8 data); // CPUSTART/HPUSTART
	void ipintxp_w(offs_t offset, u8 data); // IPINTCP/IPINTHP
	void ipenixp_w(offs_t offset, u8 data); // IPENICP/IPENIHP
	void ipclixp_w(offs_t offset, u8 data);	// IPCLICP/IPCLIHP

	TIMER_CALLBACK_MEMBER(timer);

	// CPU platform hardware
	u32 cpstat_r();
	u32 wrbeadr_r();
	u8 boot_vector_r(offs_t offset);
	void cpenipty_w(u32 data);
	void cpenitmr_w(u32 data);
	void cpintxp_w(offs_t offset, u32 data);
	void mapvec_w(u32 data);
	void cpclixp_w(offs_t offset, u32 data);
	void cpuled_w(offs_t offset, u32 data);

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

	required_region_ptr<u32> m_eprom;
	//output_finder<4> m_led;

	std::unique_ptr<u16[]> m_net_ram;

	emu_timer *m_timer = nullptr;

	bool m_mapvec = false;

	// IOP IRQ state
	const std::map<int, std::vector<iop_irq>> iop_irq_line_map;
	bool m_scc_irq_state = false;
	uint32_t m_iop_intst = 0;
	uint32_t m_iop_inten = 0;

	// CPU IRQ state
	const std::map<int, cpu_irq> cpu_irq_line_map;
	uint32_t m_cpu_intst = 0;
	uint32_t m_cpu_inten = 0;
};

void news_38xx_state::machine_start()
{
	//m_led.resolve();

	m_net_ram = std::make_unique<u16[]>(8192);
	save_pointer(NAME(m_net_ram), 8192);

	save_item(NAME(m_mapvec));
	save_item(NAME(m_scc_irq_state));
	save_item(NAME(m_iop_intst));
	save_item(NAME(m_iop_inten));
	save_item(NAME(m_cpu_intst));
	save_item(NAME(m_cpu_inten));

	m_timer = timer_alloc(FUNC(news_38xx_state::timer), this);
}

void news_38xx_state::machine_reset()
{
	// For the IOP, EPROM is mapped at 0 after reset
	m_iop->space(0).install_rom(0x00000000, 0x0000ffff, m_eprom);

	m_timer->adjust(attotime::from_hz(100), 0, attotime::from_hz(100));

	// CPU does not run until the IOP tells it to
	m_cpu->set_input_line(INPUT_LINE_HALT, 1);

	// Clear platform hardware state
	m_mapvec = false;
	m_scc_irq_state = false;
	m_cpu_intst = 0;
	m_cpu_inten = 0;
	m_iop_intst = 0;
	m_iop_inten = 0;
}

void news_38xx_state::init_common()
{
	// HACK: hardwire the rate TODO: does the index pulse divide control this?
	m_fdc->set_rate(500000);

	// RAM is always mapped for the CPU, since the MIPS boot vector is well above the max RAM value
	m_cpu->space(0).install_ram(0x00000000, m_ram->mask(), m_ram->pointer());
}

void news_38xx_state::cpu_map(address_map &map)
{
	map.global_mask(0x1fffffff); // A28-A0 are connected

	map(0x08000000, 0x0fffffff).rw(FUNC(news_38xx_state::ram_tas_r), FUNC(news_38xx_state::ram_tas_w));

	// 0x10000000 - 0x17ffffff: UBUS I/O

	// All registers below this line are 32 bit registers, LSB 1 = Set, 0 = Reset
	map(0x18000000, 0x18000003).r(FUNC(news_38xx_state::cpstat_r));
	map(0x18000000, 0x18000003).w(FUNC(news_38xx_state::cpenipty_w)).mirror(0xffff00);
	map(0x18000004, 0x18000007).w(FUNC(news_38xx_state::cpenitmr_w)).mirror(0xffff00); // todo: check schematic to see if same timer as IOP or not
	map(0x18000020, 0x18000027).w(FUNC(news_38xx_state::cpintxp_w)).mirror(0xffff00);
	map(0x18000040, 0x18000043).w(FUNC(news_38xx_state::mapvec_w)).mirror(0xffff00);
	map(0x18000044, 0x18000047).w(FUNC(news_38xx_state::cpu_inten_w<cpu_irq::UBUS>)).mirror(0xffff00);
	map(0x18000060, 0x18000067).w(FUNC(news_38xx_state::cpclixp_w)).mirror(0xffff00);
	map(0x18000080, 0x18000087).w(FUNC(news_38xx_state::cpuled_w)).mirror(0xffff00);
	map(0x1c000000, 0x1c000003).r(FUNC(news_38xx_state::wrbeadr_r));

	// TODO: what is the proper end address here?
	map(0x1fc00000, 0x1fc1ffff).r(FUNC(news_38xx_state::boot_vector_r));
}

void news_38xx_state::iop_map(address_map &map)
{
	map.global_mask(0x3fffffff); // A29-A0 are connected

	map(0x08000000, 0x0fffffff).rw(FUNC(news_38xx_state::ram_tas_r), FUNC(news_38xx_state::ram_tas_w));

	map(0x18000000, 0x1803ffff).ram(); // IOP ram
	map(0x20000000, 0x2000ffff).rom().region("eprom", 0).mirror(0x1fff0000);

	// IOP Control Registers
	map(0x22000000, 0x22000000).w(FUNC(news_38xx_state::poweron_w));
	map(0x22000001, 0x22000001).w(FUNC(news_38xx_state::romdis_w));
	map(0x22000002, 0x22000002).w(FUNC(news_38xx_state::ptycken_w));
	map(0x22000003, 0x22000003).w(FUNC(news_38xx_state::timeren_w));
	map(0x22000004, 0x22000004).w(FUNC(news_38xx_state::iop_irq_w<iop_irq::SOFTINTR>));
	map(0x22000005, 0x22000005).w(FUNC(news_38xx_state::astintr_w));
	map(0x22000006, 0x22000007).w(FUNC(news_38xx_state::iopled_w));

	// Inter-Processor Control Registers
	map(0x22800000, 0x22800001).w(FUNC(news_38xx_state::xpustart_w));
	map(0x22800008, 0x22800009).w(FUNC(news_38xx_state::ipintxp_w));
	map(0x22800010, 0x22800011).w(FUNC(news_38xx_state::ipenixp_w));
	map(0x22800018, 0x22800019).w(FUNC(news_38xx_state::ipclixp_w));

	// IOP and Inter-Processor Interrupt Status Registers
	map(0x23000000, 0x23000000).r(FUNC(news_38xx_state::intst_r));
	map(0x23800000, 0x23800000).r(FUNC(news_38xx_state::iop_ipc_intst_r));

	map(0x24000000, 0x24000007).m(m_fdc, FUNC(n82077aa_device::map));
	map(0x24000105, 0x24000105).rw(m_fdc, FUNC(n82077aa_device::dma_r), FUNC(n82077aa_device::dma_w));

	// 0x26040000 // Centronics data
	// 0x26040001 Centronics strobe
	// 0x26040002 Centronics IRQ clear
	map(0x26040003, 0x26040003).w(FUNC(news_38xx_state::iop_inten_w<iop_irq::PARALLEL>));

	map(0x26080000, 0x26080007).m(m_scsi[0], FUNC(cxd1180_device::map));
	map(0x260c0000, 0x260c0007).m(m_scsi[1], FUNC(cxd1180_device::map));

	map(0x26100000, 0x26100007).m(m_hid, FUNC(news_hid_hle_device::map_68k));
	map(0x26140000, 0x26140003).rw(m_scc, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w));
	map(0x26180000, 0x2618000f).rw(m_rtc, FUNC(rtc62421_device::read), FUNC(rtc62421_device::write));
	// 0x261c0000 // kb beep frequency LOWTONE 0x1 = low, 0x0 = high

	map(0x26200000, 0x26203fff).lrw16(
		[this](offs_t offset) { return m_net_ram[offset]; }, "net_ram_r",
		[this](offs_t offset, u16 data, u16 mem_mask) { COMBINE_DATA(&m_net_ram[offset]); }, "net_ram_w");
	// 0x262c0000 // audio

	map(0x26300000, 0x26300003).rw(m_net, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w));
	// 0x26340000 // park status (SCC and parallel info, TODO: double-check that park has this)
	// 0x26380000 // divide fdd index pulse for ECMA format (1 = 1/2x 0 = 1x)

	map(0x28000000, 0x28000017).m(m_dma[0], FUNC(dmac_0266_device::map));
	map(0x2a000000, 0x2a000017).m(m_dma[1], FUNC(dmac_0266_device::map));

	map(0x2c000000, 0x2c0000ff).rom().region("idrom", 0);
	map(0x2c000100, 0x2c000103).portr("SW1");

	map(0x2e000000, 0x2effffff).r(FUNC(news_38xx_state::iop_bus_error_r)).mirror(0x10000000); // Expansion I/O and mirror
}

void news_38xx_state::iop_vector_map(address_map &map)
{
	map(0xfffffff3, 0xfffffff3).lr8(NAME([] { return m68000_base_device::autovector(1); }));
	map(0xfffffff5, 0xfffffff5).lr8(NAME([] { return m68000_base_device::autovector(2); }));
	map(0xfffffff7, 0xfffffff7).lr8(NAME([] { return m68000_base_device::autovector(3); }));
	map(0xfffffff9, 0xfffffff9).lr8(NAME([] { return m68000_base_device::autovector(4); }));
	map(0xfffffffb, 0xfffffffb).lr8(NAME([this] { return m_scc_irq_state ? m_scc->m1_r() : m68000_base_device::autovector(5); }));
	map(0xfffffffd, 0xfffffffd).lr8(NAME([] { return m68000_base_device::autovector(6); }));
	map(0xffffffff, 0xffffffff).lr8(NAME([] { return m68000_base_device::autovector(7); }));
}

u32 news_38xx_state::ram_tas_r(offs_t offset, u32 mem_mask)
{
	const u32 current_value = m_iop->space(0).read_dword(offset << 2);
	if (machine().side_effects_disabled())
	{
		return current_value;
	}

	if (mem_mask != 0xffffffff)
	{
		fatalerror("%s tried to acquire lock with unaligned memory access!", machine().describe_context());
	}

	if (current_value == 0)
	{
		LOGMASKED(LOG_TAS, "%s acquired lock at offset 0x%x\n", machine().describe_context(), offset << 2);
	}

	// TODO: what is the actual value written by the hardware?
	//		 Both vmunix and mrx appear to use bltz (r3k)/bmi (030) instructions, at least during boot
	m_cpu->space(0).write_dword(offset << 2, 0x80000000); // TODO: fix usage of ->space
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

	LOGMASKED(LOG_TAS, "%s releasing lock at offset 0x%x\n", machine().describe_context(), offset << 2);
	m_cpu->space(0).write_dword(offset << 2, data); // TODO: fix usage of ->space
}

u8 news_38xx_state::intst_r()
{
	const u8 iop_status = is_iop_irq_set<iop_irq::PERR>() << 7 |
	                      is_iop_irq_set<iop_irq::SCSI0>() << 6 |
	                      is_iop_irq_set<iop_irq::SCSI1>() << 5 |
	                      (is_iop_irq_set<iop_irq::CPU>() || is_iop_irq_set<iop_irq::UBUS>()) << 4 |
	                      is_iop_irq_set<iop_irq::LANCE>() << 3 |
	                      is_iop_irq_set<iop_irq::FDCIRQ>() << 2 |
	                      is_iop_irq_set<iop_irq::PARALLEL>() << 1 |
	                      is_iop_irq_set<iop_irq::SLOT>();
	LOGMASKED(LOG_INTERRUPT, "%s intst_r: 0x%x\n", machine().describe_context(), iop_status);
	return iop_status;
}

u8 news_38xx_state::iop_ipc_intst_r()
{
	const u8 iop_status = is_iop_irq_set<iop_irq::UBUS>() << 1 | is_iop_irq_set<iop_irq::CPU>();
	LOGMASKED(LOG_INTERRUPT, "%s iop_ipc_intst_r: 0x%x\n", machine().describe_context(), iop_status);
	return iop_status;
}

template<news_38xx_state::iop_irq Number>
void news_38xx_state::iop_irq_w(u8 state)
{
	if (Number != iop_irq::TIMER)
	{
		LOGMASKED(LOG_INTERRUPT, "(%s) IOP IRQ %s %s\n", machine().describe_context(),
		          iop_irq_names[static_cast<unsigned>(Number)], state ? "set" : "cleared");
	}
	else
	{
		LOGMASKED(LOG_TIMER, "(%s) IOP IRQ %s %s\n", machine().describe_context(),
				  iop_irq_names[static_cast<unsigned>(Number)], state ? "set" : "cleared");
	}

	if (state)
	{
		m_iop_intst |= 1U << static_cast<u32>(Number);
	}
	else
	{
		m_iop_intst &= ~(1U << static_cast<u32>(Number));
	}

	int_check_iop();
}

template<news_38xx_state::iop_irq Number>
void news_38xx_state::iop_inten_w(uint8_t state)
{
	if (Number != iop_irq::TIMER)
	{
		LOGMASKED(LOG_INTERRUPT, "(%s) IOP IRQ %s %s\n", machine().describe_context(),
		          iop_irq_names[static_cast<unsigned>(Number)], state ? "enabled" : "disabled");
	}
	else
	{
		LOGMASKED(LOG_TIMER, "(%s) IOP IRQ %s %s\n", machine().describe_context(),
			iop_irq_names[static_cast<unsigned>(Number)], state ? "enabled" : "disabled");
	}

	if (state)
	{
		m_iop_inten |= 1 << static_cast<u32>(Number);
	}
	else
	{
		m_iop_inten &= ~(1 << static_cast<u32>(Number));
	}

	int_check_iop();
}

template<news_38xx_state::iop_irq Number>
bool news_38xx_state::is_iop_irq_set()
{
	return BIT(m_iop_intst, static_cast<u32>(Number));
}

void news_38xx_state::int_check_iop()
{
	const uint32_t active_irq = m_iop_intst & (m_iop_inten | iop_nmi_mask);
	for (const auto& [input_line, irq_inputs] : iop_irq_line_map)
	{
		// Calculate state of input pin (logical OR of all attached inputs)
		bool state = false;
		for (auto irq_input : irq_inputs)
		{
			state |= active_irq & 1 << static_cast<u32>(irq_input);
		}

		// Update input pin status if it has changed
		if (m_iop->input_line_state(input_line) != state) {
			if (input_line != INPUT_LINE_IRQ6)
			{
				LOGMASKED(LOG_INTERRUPT, "Setting IOP input line %d to %d\n", input_line, state ? 1 : 0);
			}

			m_iop->set_input_line(input_line, state ? 1 : 0);
		}
	}
}

template <news_38xx_state::cpu_irq Number>
void news_38xx_state::cpu_irq_w(u8 state)
{
	if (Number != cpu_irq::TIMER)
	{
		LOGMASKED(LOG_INTERRUPT, "(%s) CPU IRQ %s %s\n", machine().describe_context(),
		          cpu_irq_names[static_cast<unsigned>(Number)], state ? "set" : "cleared");
	}
	else
	{
		LOGMASKED(LOG_TIMER, "(%s) CPU IRQ %s %s\n", machine().describe_context(),
		  cpu_irq_names[static_cast<unsigned>(Number)], state ? "set" : "cleared");
	}

	if (state)
	{
		m_cpu_intst |= 1U << static_cast<u32>(Number);
	}
	else
	{
		m_cpu_intst &= ~(1U << static_cast<u32>(Number));
	}

	int_check_cpu();
}

template<news_38xx_state::cpu_irq Number>
void news_38xx_state::cpu_inten_w(uint8_t state)
{
	if (Number != cpu_irq::TIMER)
	{
		LOGMASKED(LOG_INTERRUPT, "(%s) CPU IRQ %s %s\n", machine().describe_context(),
		          cpu_irq_names[static_cast<unsigned>(Number)], state ? "enabled" : "disabled");
	}
	else
	{
		LOGMASKED(LOG_TIMER, "(%s) CPU IRQ %s %s\n", machine().describe_context(),
		  cpu_irq_names[static_cast<unsigned>(Number)], state ? "enabled" : "disabled");
	}

	if (state)
	{
		m_cpu_inten |= 1 << static_cast<u32>(Number);
	}
	else
	{
		m_cpu_inten &= ~(1 << static_cast<u32>(Number));
	}

	int_check_cpu();
}

template<news_38xx_state::cpu_irq Number>
bool news_38xx_state::is_cpu_irq_set()
{
	return BIT(m_cpu_intst, static_cast<u32>(Number));
}

void news_38xx_state::int_check_cpu()
{
	const uint32_t active_irq = m_cpu_intst & (m_cpu_inten | cpu_nmi_mask);
	for (const auto& [input_line, irq_input] : cpu_irq_line_map)
	{
		// Update input pin status if it has changed
		const bool state = BIT(active_irq, static_cast<uint32_t>(irq_input));
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
	if (m_iop_inten & 1 << static_cast<u32>(iop_irq::TIMER))
	{
		iop_irq_w<iop_irq::TIMER>(1);
	}

	if (m_cpu_inten & 1 << static_cast<u32>(cpu_irq::TIMER))
	{
		cpu_irq_w<cpu_irq::TIMER>(1);
	}
}

// TODO: Add and unify logging for all of these, add a unique log level for IOP registers, and add machine context
void news_38xx_state::poweron_w(u8 data)
{
	LOG("Write POWERON = 0x%x (%s)\n", data, machine().describe_context());

	if (!machine().side_effects_disabled() && !data)
	{
		machine().schedule_exit();
	}
}

void news_38xx_state::romdis_w(u8 data)
{
	LOG("ROMDIS = 0x%x (%s)\n", data, machine().describe_context());
	if (data) {
		m_iop->space(0).install_ram(0x00000000, m_ram->mask(), m_ram->pointer());
	}
	else {
		// TODO: re-map EPROM
	}
}

void news_38xx_state::ptycken_w(u8 data)
{
	iop_inten_w<iop_irq::PERR>(data > 0);
	if (!data)
	{
		iop_irq_w<iop_irq::PERR>(0);
	}
}

void news_38xx_state::timeren_w(u8 data)
{
	iop_inten_w<iop_irq::TIMER>(data > 0);
	if (!data)
	{
		iop_irq_w<iop_irq::TIMER>(0);
	}
}

void news_38xx_state::astintr_w(u8 data)
{
	LOG("ASTINTR = 0x%x\n", data);
	// TODO: how to trigger AST IRQ when IOP enters user mode? No easy injection point like my experiments with NWS800
}

void news_38xx_state::iopled_w(offs_t offset, u8 data)
{
	LOGMASKED(LOG_LED, "IOPLED%01d = 0x%x\n", offset, data);
}

void news_38xx_state::xpustart_w(offs_t offset, u8 data)
{
	// offset 0 = start running main CPU
	// offset 1 = start running UPU (UBUS bus master, expansion slot A?)
	LOG("%cPUSTART = 0x%x\n", !offset ? 'C' : 'H', data);
	if (!offset) {
		// TODO: reset CPU platform registers here?
		m_cpu->set_input_line(INPUT_LINE_HALT, data ? 0 : 1);
	} else {
		if (data) fatalerror("Tried to start UPU without UPU installed!");
	}
}

void news_38xx_state::ipintxp_w(offs_t offset, u8 data)
{
	// Send interrupt to CPU or UPU
	LOGMASKED(LOG_INTERRUPT, "IPINT%cP = 0x%x\n", !offset ? 'C' : 'H', data);
	if (!offset)
	{
		cpu_irq_w<cpu_irq::IOP>(1);
	}
	else
	{
		fatalerror("IOP tried to interrupt UPU without UPU installed!");
	}
}

void news_38xx_state::ipenixp_w(offs_t offset, u8 data)
{
	// Enable or disable interrupts from CPU or UPU
	// TODO: this doesn't need to be it's own function anymore
	LOGMASKED(LOG_INTERRUPT, "IPENI%cP = 0x%x\n", !offset ? 'C' : 'H', data);
	if (!offset)
	{
		iop_inten_w<iop_irq::CPU>(data > 0);
	}
	else
	{
		iop_inten_w<iop_irq::UBUS>(data > 0);
	}
}

void news_38xx_state::ipclixp_w(offs_t offset, u8 data)
{
	// Clear interrupt from CPU or UPU
	// TODO: this doesn't need to be its own function anymore
	LOGMASKED(LOG_INTERRUPT, "IPCLI%cP = 0x%x\n", !offset ? 'C' : 'H', data);
	if (!offset)
	{
		iop_irq_w<iop_irq::CPU>(0);
	}
	else
	{
		iop_irq_w<iop_irq::UBUS>(0);
	}
}

u32 news_38xx_state::cpstat_r()
{
	const u8 cpu_status = is_cpu_irq_set<cpu_irq::UBUS>() << 1 | is_cpu_irq_set<cpu_irq::IOP>();
	LOGMASKED(LOG_INTERRUPT, "%s cpstat_r 0x%x\n", machine().describe_context(), cpu_status);
	return cpu_status;
}

u32 news_38xx_state::wrbeadr_r()
{
	LOG("WRBEADR read 0x%x\n"); // Write bus error address
	return 0;
}

u8 news_38xx_state::boot_vector_r(offs_t offset)
{
	// RAM endianness is an issue because RAM is installed with install_ram.
	// Therefore, go straight to the memory bus (technically its own bus, but we can go via IOP as a hack)
	// TODO: pick one of the following instead of doing this
	//       A) figure out the correct way to deal with the endianness issue
	//       B) Replace this with a memory_access::specific
	//       C) Use an accessor function for RAM along with a memory_access cache to avoid endianness mismatch

	if (!m_mapvec)
	{
		constexpr u32 BOOT_VECTOR_BASE = 0xc00000;
		return m_cpu->space(0).read_byte(BOOT_VECTOR_BASE + offset);
	}

	// In reality, this probably just causes a bus error, returns 0x0/0xff, or something like that
	fatalerror("CPU tried to read from boot vector space without MAPVEC!");
}

void news_38xx_state::cpenipty_w(u32 data)
{
	cpu_inten_w<cpu_irq::PERR>(data > 0);
	if (!data)
	{
		cpu_irq_w<cpu_irq::PERR>(0);
	}
}

void news_38xx_state::cpenitmr_w(u32 data)
{
	cpu_inten_w<cpu_irq::TIMER>(data > 0);
	if (!data)
	{
		cpu_irq_w<cpu_irq::TIMER>(0);
	}
}

void news_38xx_state::cpintxp_w(offs_t offset, u32 data)
{
	LOGMASKED(LOG_INTERRUPT, "CPINT%cP = 0x%x\n", !offset ? 'I' : 'H', data);
	if (!offset)
	{
		iop_irq_w<iop_irq::CPU>(1);
	}
	else
	{
		fatalerror("CPU tried to interrupt UPU without UPU installed!");
	}
}

void news_38xx_state::mapvec_w(u32 data)
{
	// 1 = normal operation, 0 = map CPU reset vector 0x1fc00000 to 0x00c00000 in RAM
	// Like other CPU control registers, this defaults to 0.
	LOG("(%s) MAPVEC = 0x%x\n", machine().describe_context(), data);
	m_mapvec = data > 0;
}

void news_38xx_state::cpclixp_w(offs_t offset, u32 data)
{
	// Clear interrupt from IOP or UPU
	LOGMASKED(LOG_INTERRUPT, "(%s) CPCLI%cP = 0x%x\n", machine().describe_context(), !offset ? 'I' : 'H', data);
	if (!offset)
	{
		cpu_irq_w<cpu_irq::IOP>(0);
	}
	else
	{
		cpu_irq_w<cpu_irq::UBUS>(0);
	}
}

void news_38xx_state::cpuled_w(offs_t offset, u32 data)
{
	LOGMASKED(LOG_LED, "CPULED%01d = 0x%x\n", offset, data);
}

void news_scsi_devices(device_slot_interface &device)
{
	// TODO: tape
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("cdrom", NSCSI_CDROM);
}

void news_38xx_state::common(machine_config &config)
{
	R3000A(config, m_cpu, 40_MHz_XTAL / 2, 65536, 65536); // 40MHz goes into the R3000 pkg, but is divided internally to 20MHz
	m_cpu->set_addrmap(AS_PROGRAM, &news_38xx_state::cpu_map);
	m_cpu->set_fpu(mips1_device_base::MIPS_R3010Av4); // TODO: FPA IRQ?

	M68030(config, m_iop, 40_MHz_XTAL / 2);
	m_iop->set_addrmap(AS_PROGRAM, &news_38xx_state::iop_map);
	m_iop->set_addrmap(m68000_base_device::AS_CPU_SPACE, &news_38xx_state::iop_vector_map);

	RAM(config, m_ram);
	m_ram->set_default_size("16M");
	// TODO: confirm each bank supports 4x1M or 4x4M
	//m_ram->set_extra_options("4M,8M,12M,20M,24M,32M,36M,48M");

	RTC62421(config, m_rtc, 32.768_kHz_XTAL);

	DMAC_0266(config, m_dma[0], 0);
	m_dma[0]->set_bus(m_iop, 0);

	DMAC_0266(config, m_dma[1], 0);
	m_dma[1]->set_bus(m_iop, 0);

	SCC85C30(config, m_scc, 4.9152_MHz_XTAL);
	m_scc->out_int_callback().set(
		[this](const int state)
		{
			m_scc_irq_state = static_cast<bool>(state);
			iop_irq_w<iop_irq::SCC>(state != 0);
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
	m_net->intr_out().set(FUNC(news_38xx_state::iop_irq_w<iop_irq::LANCE>)).invert();
	m_net->dma_in().set([this](const offs_t offset) { return m_net_ram[offset >> 1]; });
	m_net->dma_out().set([this](const offs_t offset, const u16 data, const u16 mem_mask) { COMBINE_DATA(&m_net_ram[offset >> 1]); });

	N82077AA(config, m_fdc, 24_MHz_XTAL, n82077aa_device::mode_t::PS2);
	m_fdc->intrq_wr_callback().set(FUNC(news_38xx_state::iop_irq_w<iop_irq::FDCIRQ>));
	m_fdc->drq_wr_callback().set(FUNC(news_38xx_state::iop_irq_w<iop_irq::FDCDRQ>));
	FLOPPY_CONNECTOR(config, "fdc:0", "35hd", FLOPPY_35_HD, true, floppy_image_device::default_pc_floppy_formats).enable_sound(false);

	// scsi bus 0 and devices
	// TODO: tape
	NSCSI_BUS(config, m_scsibus[0]);
	NSCSI_CONNECTOR(config, "scsi0:0", news_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi0:1", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:2", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:3", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:4", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:5", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:6", news_scsi_devices, nullptr);

	// scsi bus 0 host adapter
	NSCSI_CONNECTOR(config, "scsi0:7").option_set("cxd1180", CXD1180).machine_config(
		[this](device_t *device)
		{
			auto &adapter = downcast<cxd1180_device &>(*device);
			adapter.set_clock(20_MHz_XTAL / 2);

			adapter.irq_handler().set(*this, FUNC(news_38xx_state::iop_irq_w<iop_irq::SCSI0>));
			adapter.irq_handler().append(m_dma[0], FUNC(dmac_0266_device::eop_w));
			adapter.drq_handler().set(m_dma[0], FUNC(dmac_0266_device::req_w));

			//subdevice<dmac_0266_device>(":dma0")->out_eop_cb().set(adapter, FUNC(cxd1180_device::eop_w));
			subdevice<dmac_0266_device>(":dma0")->dma_r_cb().set(adapter, FUNC(cxd1180_device::dma_r));
			subdevice<dmac_0266_device>(":dma0")->dma_w_cb().set(adapter, FUNC(cxd1180_device::dma_w));
		});

	// scsi bus 1 and devices
	NSCSI_BUS(config, m_scsibus[1]);
	NSCSI_CONNECTOR(config, "scsi1:0", news_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi1:1", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:2", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:3", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:4", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:5", news_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:6", news_scsi_devices, nullptr);

	// scsi bus 1 host adapter
	NSCSI_CONNECTOR(config, "scsi1:7").option_set("cxd1180", CXD1180).machine_config(
		[this](device_t *device)
		{
			auto &adapter = downcast<cxd1180_device &>(*device);
			adapter.set_clock(20_MHz_XTAL / 2);

			adapter.irq_handler().set(*this, FUNC(news_38xx_state::iop_irq_w<iop_irq::SCSI1>));
			adapter.irq_handler().append(m_dma[1], FUNC(dmac_0266_device::eop_w));
			adapter.drq_handler().set(m_dma[1], FUNC(dmac_0266_device::req_w));

			//subdevice<dmac_0266_device>(":dma1")->out_eop_cb().set(adapter, FUNC(cxd1180_device::eop_w));
			subdevice<dmac_0266_device>(":dma1")->dma_r_cb().set(adapter, FUNC(cxd1180_device::dma_r));
			subdevice<dmac_0266_device>(":dma1")->dma_w_cb().set(adapter, FUNC(cxd1180_device::dma_w));
		});

	NEWS_HID_HLE(config, m_hid);
	m_hid->irq_out<news_hid_hle_device::KEYBOARD>().set(*this, FUNC(news_38xx_state::iop_irq_w<iop_irq::KEYBOARD>));
	m_hid->irq_out<news_hid_hle_device::MOUSE>().set(*this, FUNC(news_38xx_state::iop_irq_w<iop_irq::MOUSE>));

	SOFTWARE_LIST(config, "software_list").set_original("sony_news").set_filter("RISC");
}

void news_38xx_state::nws3860(machine_config &config)
{
	common(config);
}

ROM_START(nws3860)
	ROM_REGION32_BE(0x10000, "eprom", 0)
	ROM_SYSTEM_BIOS(0, "nws3860", "NWS-3860 v1.6")
	ROMX_LOAD("mpu_10__ver.1.6__1990_sony.ic159", 0x00000, 0x10000, CRC(542e21b8) SHA1(631dca1f3446761973073f5c32c1a0aeba538c2c), ROM_BIOS(0))

	ROM_REGION32_BE(0x100, "idrom", 0)
	ROM_LOAD("idrom.bin", 0x000, 0x100, CRC(a2b9e968) SHA1(09c7d253c7ed0b368c49f4b60bfe5ca76acd7cc3) BAD_DUMP)
ROM_END

static INPUT_PORTS_START(nws3860)
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


/*   YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS            INIT         COMPANY  FULLNAME    FLAGS */
COMP(1989, nws3860, 0,      0,      nws3860, nws3860, news_38xx_state, init_common, "Sony",  "NWS-3860", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
