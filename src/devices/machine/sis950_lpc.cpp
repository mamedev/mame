// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

SiS950 LPC implementation (Super I/O & southbridge)

TODO:
- Convert most stuff declared here to generic interfaces;
  \- Despite what the datasheet claims it really looks like that a separate Super I/O provides
     the usual x86 resources;
- Flash ROM handling
  \- Doesn't survive a soft reset;
- Fix EISA;
- INIT register (reset & A20 control + fast gates + fast reset timing control);
- Override PS/2 ports if USB legacy mode is enabled;
- NMI & SMI handling;
- SMBus handling;
- RTC extended bank enable;
  \- Doesn't survive a CMOS write after fast reset;
- Shadow registers for PIC and PIT;
- IRQ remaps for PCI_SLOT
  \- INTA GUI
  \- INTB AUDIO and MODEM
  \- INTC ethernet
  \- INTD USB
- IRQ software traps ($6e-$6f);
  \- Documentation mentions that those can be read-back too, huh?
- Understand what's the catch of "changing device ID number" via BIOS control $40 bit 6;

**************************************************************************************************/

#include "emu.h"
#include "sis950_lpc.h"

#include "bus/pc_kbd/keyboards.h"
#include "machine/pckeybrd.h"
#include "speaker.h"

#define LOG_IO     (1U << 1) // log PCI register accesses
#define LOG_TODO   (1U << 2) // log unimplemented registers
#define LOG_MAP    (1U << 3) // log full remaps
#define LOG_LPC    (1U << 4) // log LPC legacy regs
#define LOG_IRQ    (1U << 5) // log IRQ remaps

#define VERBOSE (LOG_GENERAL | LOG_IO | LOG_TODO | LOG_IRQ)
//#define LOG_OUTPUT_FUNC osd_printf_warning

#include "logmacro.h"

#define LOGIO(...)     LOGMASKED(LOG_IO,   __VA_ARGS__)
#define LOGMAP(...)    LOGMASKED(LOG_MAP,  __VA_ARGS__)
#define LOGTODO(...)   LOGMASKED(LOG_TODO, __VA_ARGS__)
#define LOGLPC(...)    LOGMASKED(LOG_LPC, __VA_ARGS__)
#define LOGIRQ(...)    LOGMASKED(LOG_IRQ, __VA_ARGS__)

DEFINE_DEVICE_TYPE(SIS950_LPC, sis950_lpc_device, "sis950_lpc", "SiS 950 LPC Super-South Bridge")

sis950_lpc_device::sis950_lpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, SIS950_LPC, tag, owner, clock)
	, m_host_cpu(*this, finder_base::DUMMY_TAG)
	, m_flash_rom(*this, finder_base::DUMMY_TAG)
	, m_pic_master(*this, "pic_master")
	, m_pic_slave(*this, "pic_slave")
	, m_dmac_master(*this, "dmac_master")
	, m_dmac_slave(*this, "dmac_slave")
	, m_pit(*this, "pit")
	, m_keybc(*this, "keybc")
	, m_isabus(*this, "isabus")
	, m_speaker(*this, "speaker")
	, m_rtc(*this, "rtc")
	, m_ps2_con(*this, "ps2_con")
	, m_aux_con(*this, "aux_con")
	, m_acpi(*this, "acpi")
	, m_smbus(*this, "smbus")
	, m_fast_reset_cb(*this)
{
}

void sis950_lpc_device::device_reset()
{
	pci_device::device_reset();

	command = 0x000c;
	status = 0x0200;

	m_bios_control = 1;
	m_acpi_base = 0;
	m_flash_control = 0x40;
	m_keybc_reg = 0x51;
	m_dma_channel = -1;
//  m_cur_eop = false;
	m_dma_high_byte = 0;
	m_init_reg = 0;
	m_rtc_reg = 0x10;
	// remapping is disabled for all as default
	std::fill(std::begin(m_irq_remap), std::end(m_irq_remap), 0x80);

	m_lpc_legacy.fast_init = 0;
	remap_cb();
}

void sis950_lpc_device::cpu_a20_w(int state)
{
	// TODO: confirm "A20M# being always high"
//  if (BIT(m_init_reg, 1))
//      state = ASSERT_LINE;
	m_host_cpu->set_input_line(INPUT_LINE_A20, state);
}

void sis950_lpc_device::cpu_reset_w(int state)
{
	// TODO: masked via INIT $46 bit 0
	m_host_cpu->set_input_line(INPUT_LINE_RESET, state);
}

void sis950_lpc_device::device_add_mconfig(machine_config &config)
{
	constexpr XTAL lpc_pit_clock = XTAL(14'318'181);

	// confirmed 82C54
	PIT8254(config, m_pit, 0);
	// heartbeat IRQ
	m_pit->set_clk<0>(lpc_pit_clock / 12);
	m_pit->out_handler<0>().set(FUNC(sis950_lpc_device::pit_out0));
	// DRAM refresh
	m_pit->set_clk<1>(lpc_pit_clock / 12);
	m_pit->out_handler<1>().set(FUNC(sis950_lpc_device::pit_out1));
	// PIO port C pin 4, and speaker polling enough
	m_pit->set_clk<2>(lpc_pit_clock / 12);
	m_pit->out_handler<2>().set(FUNC(sis950_lpc_device::pit_out2));

	// TODO: unknown part #
	AM9517A(config, m_dmac_master, lpc_pit_clock / 3);
	m_dmac_master->out_hreq_callback().set(m_dmac_slave, FUNC(am9517a_device::dreq0_w));
//  m_dmac_master->out_eop_callback().set(FUNC(sis950_lpc_device::at_dma8237_out_eop));
	m_dmac_master->in_memr_callback().set(FUNC(sis950_lpc_device::pc_dma_read_byte));
	m_dmac_master->out_memw_callback().set(FUNC(sis950_lpc_device::pc_dma_write_byte));
	// TODO: ior/iow/dack/eop callbacks

	AM9517A(config, m_dmac_slave, lpc_pit_clock / 3);
	m_dmac_slave->out_hreq_callback().set(FUNC(sis950_lpc_device::pc_dma_hrq_changed));
	m_dmac_slave->in_memr_callback().set(FUNC(sis950_lpc_device::pc_dma_read_word));
	m_dmac_slave->out_memw_callback().set(FUNC(sis950_lpc_device::pc_dma_write_word));
	// TODO: ior/iow/dack callbacks

	// Confirmed 82C59s
	PIC8259(config, m_pic_master, 0);
	m_pic_master->out_int_callback().set_inputline(m_host_cpu, 0);
	m_pic_master->in_sp_callback().set_constant(1);
	m_pic_master->read_slave_ack_callback().set(
		[this](offs_t offset) -> u8
		{
			if (offset == 2)
				return m_pic_slave->acknowledge();

			return 0;
		});

	PIC8259(config, m_pic_slave, 0);
	m_pic_slave->out_int_callback().set(m_pic_master, FUNC(pic8259_device::ir2_w));
	m_pic_slave->in_sp_callback().set_constant(0);

	// TODO: EISA, from virtual bridge

	// TODO: selectable between PCI clock / 4 or 7.159 MHz, via reg $47 bit 5
	PS2_KEYBOARD_CONTROLLER(config, m_keybc, DERIVED_CLOCK(1, 4));
	m_keybc->set_default_bios_tag("compaq");
	m_keybc->hot_res().set(FUNC(sis950_lpc_device::cpu_reset_w));
	m_keybc->gate_a20().set(FUNC(sis950_lpc_device::cpu_a20_w));
	m_keybc->kbd_irq().set(m_pic_master, FUNC(pic8259_device::ir1_w));
	m_keybc->kbd_clk().set(m_ps2_con, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_keybc->kbd_data().set(m_ps2_con, FUNC(pc_kbdc_device::data_write_from_mb));
	m_keybc->aux_irq().set(m_pic_slave, FUNC(pic8259_device::ir4_w));
	m_keybc->aux_clk().set(m_aux_con, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_keybc->aux_data().set(m_aux_con, FUNC(pc_kbdc_device::data_write_from_mb));

	PC_KBDC(config, m_ps2_con, pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL);
	m_ps2_con->out_clock_cb().set(m_keybc, FUNC(ps2_keyboard_controller_device::kbd_clk_w));
	m_ps2_con->out_data_cb().set(m_keybc, FUNC(ps2_keyboard_controller_device::kbd_data_w));

	// TODO: doesn't work (wrong PS/2 BIOS?), worked around by disabling for now
	PC_KBDC(config, m_aux_con, ps2_mice, nullptr);
	m_aux_con->out_clock_cb().set(m_keybc, FUNC(ps2_keyboard_controller_device::aux_clk_w));
	m_aux_con->out_data_cb().set(m_keybc, FUNC(ps2_keyboard_controller_device::aux_data_w));

	// TODO: unknown RTC type
	// Has external RTC bank select at $48, using this one as convenience
	DS12885EXT(config, m_rtc, XTAL(32'768));
	m_rtc->irq().set(m_pic_slave, FUNC(pic8259_device::ir0_w));
	m_rtc->set_century_index(0x32);

	// TODO: left/right speaker connection
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	ISA16(config, m_isabus, 0);
	m_isabus->irq3_callback().set(FUNC(sis950_lpc_device::pc_irq3_w));
	m_isabus->irq4_callback().set(FUNC(sis950_lpc_device::pc_irq4_w));
	m_isabus->irq6_callback().set(FUNC(sis950_lpc_device::pc_irq6_w));
	m_isabus->irq5_callback().set(FUNC(sis950_lpc_device::pc_irq5_w));
	m_isabus->irq7_callback().set(FUNC(sis950_lpc_device::pc_irq7_w));
	m_isabus->irq2_callback().set(FUNC(sis950_lpc_device::pc_irq9_w));
	m_isabus->irq10_callback().set(FUNC(sis950_lpc_device::pc_irq10_w));
	m_isabus->irq11_callback().set(FUNC(sis950_lpc_device::pc_irq11_w));
	m_isabus->irq12_callback().set(FUNC(sis950_lpc_device::pc_irq12m_w));
	m_isabus->irq14_callback().set(FUNC(sis950_lpc_device::pc_irq14_w));
	m_isabus->irq15_callback().set(FUNC(sis950_lpc_device::pc_irq15_w));
	m_isabus->iochck_callback().set(FUNC(sis950_lpc_device::iochck_w));
}

void sis950_lpc_device::device_config_complete()
{
	auto isabus = m_isabus.finder_target();
	isabus.first.subdevice<isa16_device>(isabus.second)->set_memspace(m_host_cpu, AS_PROGRAM);
	isabus.first.subdevice<isa16_device>(isabus.second)->set_iospace(m_host_cpu, AS_IO);

	pci_device::device_config_complete();
}

void sis950_lpc_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x10, 0x4f).unmaprw();
	map(0x10, 0x75).rw(FUNC(sis950_lpc_device::unmap_log_r), FUNC(sis950_lpc_device::unmap_log_w));

	// LPC control regs
	map(0x40, 0x40).rw(FUNC(sis950_lpc_device::bios_control_r), FUNC(sis950_lpc_device::bios_control_w));
	map(0x41, 0x41).rw(FUNC(sis950_lpc_device::irq_remap_r<IRQ_INTA>), FUNC(sis950_lpc_device::irq_remap_w<IRQ_INTA>));
	map(0x42, 0x42).rw(FUNC(sis950_lpc_device::irq_remap_r<IRQ_INTB>), FUNC(sis950_lpc_device::irq_remap_w<IRQ_INTB>));
	map(0x43, 0x43).rw(FUNC(sis950_lpc_device::irq_remap_r<IRQ_INTC>), FUNC(sis950_lpc_device::irq_remap_w<IRQ_INTC>));
	map(0x44, 0x44).rw(FUNC(sis950_lpc_device::irq_remap_r<IRQ_INTD>), FUNC(sis950_lpc_device::irq_remap_w<IRQ_INTD>));

	map(0x45, 0x45).rw(FUNC(sis950_lpc_device::flash_ctrl_r), FUNC(sis950_lpc_device::flash_ctrl_w));
	map(0x46, 0x46).rw(FUNC(sis950_lpc_device::init_enable_r), FUNC(sis950_lpc_device::init_enable_w));
	map(0x47, 0x47).rw(FUNC(sis950_lpc_device::keybc_reg_r), FUNC(sis950_lpc_device::keybc_reg_w));
	map(0x48, 0x48).rw(FUNC(sis950_lpc_device::rtc_reg_r), FUNC(sis950_lpc_device::rtc_reg_w));

	// DMA control regs
//  map(0x49, 0x49) Distributed DMA channel enable
//  map(0x4a, 0x4b) Distributed DMA Master config

	// Shadow regs (r/o)
//  map(0x4c, 0x4f) PIC master ICW*
//  map(0x50, 0x53) PIC slave ICW*
//  map(0x54, 0x55) PIC master OCW2-3
//  map(0x56, 0x57) PIC slave OCW2-3 (NB: assume documentation 0x54-0x55 to be a typo)
//  map(0x58, 0x5f) PIT counters 0-1-2 low/high, $5e -> control 43h, $5f -> read count pointer statuses
//  map(0x60, 0x60) EISA port $70

	map(0x61, 0x61).rw(FUNC(sis950_lpc_device::irq_remap_r<IRQ_IDE>), FUNC(sis950_lpc_device::irq_remap_w<IRQ_IDE>));
//  map(0x62, 0x62) <reserved>, hardwired to 0x80 (PIT irq remap?)
	map(0x62, 0x62).lr8(NAME([] () { return 0x80; }));
	map(0x63, 0x63).rw(FUNC(sis950_lpc_device::irq_remap_r<IRQ_GPE>), FUNC(sis950_lpc_device::irq_remap_w<IRQ_GPE>));

//  map(0x64, 0x64) PCI bus priority timer
//  map(0x65, 0x65) PHOLD# timer

//  map(0x66, 0x66) <reserved>
//  map(0x67, 0x67) Serial IRQ 1 & 12 latch control (FDC super I/O)
//  map(0x68, 0x69) <reserved>

	map(0x63, 0x63).rw(FUNC(sis950_lpc_device::irq_remap_r<IRQ_ACPI>), FUNC(sis950_lpc_device::irq_remap_w<IRQ_ACPI>));
//  map(0x6b, 0x6b) <reserved>
	map(0x6c, 0x6c).rw(FUNC(sis950_lpc_device::irq_remap_r<IRQ_SMBUS>), FUNC(sis950_lpc_device::irq_remap_w<IRQ_SMBUS>));
	map(0x6d, 0x6d).rw(FUNC(sis950_lpc_device::irq_remap_r<IRQ_SWDOG>), FUNC(sis950_lpc_device::irq_remap_w<IRQ_SWDOG>));

//  map(0x6e, 0x6f) SW irq triggers
//  map(0x70, 0x70) Serial irq control
//  map(0x71, 0x73) Serial irq enable

	// $74 should be ACPI lower base bank, but marked as <reserved> regardless
	// (by logic that should go to NOP too)
	map(0x74, 0x74).lr8(NAME([] () { return 0; }));
	map(0x75, 0x75).rw(FUNC(sis950_lpc_device::acpi_base_r), FUNC(sis950_lpc_device::acpi_base_w));
}

u8 sis950_lpc_device::bios_control_r()
{
	LOGIO("Read BIOS control [$40] %02x\n", m_bios_control);
	return m_bios_control;
}

void sis950_lpc_device::bios_control_w(u8 data)
{
	// Mostly 0x9a or 0x9b
	LOGIO("Write BIOS control [$40] %02x\n", data);
	m_bios_control = data;
	remap_cb();
}

template <unsigned N> u8 sis950_lpc_device::irq_remap_r()
{
	return m_irq_remap[N];
}

template <unsigned N> void sis950_lpc_device::irq_remap_w(u8 data)
{
	m_irq_remap[N] = data;
	LOGIRQ("%s IRQ remap write %02x (%s)\n", std::array<char const *, 9> {{
		"INTA",
		"INTB",
		"INTC",
		"INTD",
		"IDEIRQ",
		"GPEIRQ",
		"ACPI/SCI",
		"SMBus",
		"Software Watchdog"
	 }}[N], data, BIT(data, 7) ? "disable" : "enable");
}

u8 sis950_lpc_device::flash_ctrl_r()
{
	LOGIO("Read Flash control [$45] %02x\n", m_flash_control);
	return m_flash_control;
}

void sis950_lpc_device::flash_ctrl_w(u8 data)
{
	LOGIO("Write Flash control [$45] %02x\n", data);
	m_flash_control = data;
	remap_cb();
}

u8 sis950_lpc_device::init_enable_r()
{
	LOGIO("Read INIT enable [$46] %02x\n", m_init_reg);
	return m_init_reg;
}

/*
 * 11-- ---- HW (fast?) reset
 * --x- ---- INIT enable
 * ---x ---- Fast Gate A20 emulation
 * ---- x--- Fast Reset latency control
 * ---- -x-- Fast Reset emulation
 * ---- --x- (0) enable A20M# (1) A20M# always high (?)
 * ---- ---x Keyboard HW reset
 */
void sis950_lpc_device::init_enable_w(u8 data)
{
	LOGIO("Write INIT enable [$46] %02x\n", data);
	// HW fast reset
	// TODO: is 0->1 implementation correct?
	// it will otherwise keep resetting itself, which may be a side effect of something else
	// (perhaps PS/2 controller can intercept this? Or it's a full on LPC reset like using an actual MAME soft reset implies?)
	if ((data & 0xc0) == 0xc0)// && (m_init_reg & 0xc0) == 0)
	{
		//const int fast_reset_time = BIT(data, 3) ? 6 : 2;
		LOGIO("Fast reset issued\n");
		//m_host_cpu->pulse_input_line(INPUT_LINE_RESET, attotime::from_usec(fast_reset_time));
		m_fast_reset_cb(1);
	}

	m_init_reg = data;
}

u8 sis950_lpc_device::keybc_reg_r()
{
	LOGIO("Read keyboard register [$47] %02x\n", m_keybc_reg);
	return m_keybc_reg;
}

/*
 * x--- ---- legacy USB (ports $60-$64 overrides)
 * -x-- ---- PS/2 mouse lock enable
 * --x- ---- Keyboard controller clock select (0) PCI Clock / 4 (1) 7.159 MHz
 * ---x ---- Keyboard lock enable
 * ---- x--- Integrated keyboard controller enable
 * ---- -x-- Integrated PS/2 mouse enable (needs bit 3 enabled)
 * ---- --x- Keyboard hot key status (needs bit 3 enabled, ctrl+alt+backspace)
 * ---- ---x Keyboard hot key control
 */
void sis950_lpc_device::keybc_reg_w(u8 data)
{
	LOGIO("Write keyboard register [$47] %02x\n", data);
	m_keybc_reg = data;
}

u8 sis950_lpc_device::rtc_reg_r()
{
	LOGIO("Read RTC register [$48] %02x\n", m_keybc_reg);
	return m_rtc_reg | 0x10;
}

/*
 * x--- ---- RTC extended bank enable
 * -x-- ---- APC enable
 * --x- ---- Instant Power-Off enable (thru APC)
 * ---x ---- Internal RTC status (r/o, always on?)
 * ---- xxxx <reserved>
 */
void sis950_lpc_device::rtc_reg_w(u8 data)
{
	LOGIO("Write RTC register [$48] %02x\n", data);
	m_rtc_reg = data;
}

u8 sis950_lpc_device::acpi_base_r()
{
	LOGIO("Read ACPI base [$75] %04x\n", m_acpi_base);
	return m_acpi_base >> 8;
}

void sis950_lpc_device::acpi_base_w(u8 data)
{
	u16 new_value = data << 8;
	LOGIO("Write ACPI base [$75] %04x\n", new_value);
	m_acpi_base = new_value;
	remap_cb();
}

template <unsigned N> void sis950_lpc_device::memory_map(address_map &map)
{
	map(0x00000000, 0x0001ffff).lrw8(
		NAME([this] (offs_t offset) { return m_flash_rom->read(offset | N * 0x20000); }),
		NAME([this] (offs_t offset, u8 data) { m_flash_rom->write(offset | N * 0x20000, data); })
	);
}

void sis950_lpc_device::io_map(address_map &map)
{
	// Legacy ISA (page 177)
	// map(0x0000, 0x000f) DMA1
	map(0x0000, 0x001f).rw(m_dmac_master, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	// map(0x0020, 0x0021) INT1
	map(0x0020, 0x0021).rw(m_pic_master, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	// map(0x0040, 0x0043) PIT
	map(0x0040, 0x0043).rw(m_pit, FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x0060, 0x0060).rw(m_keybc, FUNC(ps2_keyboard_controller_device::data_r), FUNC(ps2_keyboard_controller_device::data_w));
	// map(0x0061, 0x0061) NMI Status Register
	map(0x0061, 0x0061).rw(FUNC(sis950_lpc_device::nmi_status_r), FUNC(sis950_lpc_device::nmi_control_w));
	// undocumented but read, assume LPC compliant
	map(0x0064, 0x0064).rw(m_keybc, FUNC(ps2_keyboard_controller_device::status_r), FUNC(ps2_keyboard_controller_device::command_w));
	// map(0x0070, 0x0070) CMOS and NMI Mask
	map(0x0070, 0x0070).w(FUNC(sis950_lpc_device::rtc_index_w));
	map(0x0071, 0x0071).rw(FUNC(sis950_lpc_device::rtc_data_r), FUNC(sis950_lpc_device::rtc_data_w));
	// map(0x0080, 0x008f) DMA low page registers
	map(0x0080, 0x008f).rw(FUNC(sis950_lpc_device::at_page8_r), FUNC(sis950_lpc_device::at_page8_w));
	// map(0x0092, 0x0092) INIT and A20
	map(0x0092, 0x0092).rw(FUNC(sis950_lpc_device::lpc_fast_init_r), FUNC(sis950_lpc_device::lpc_fast_init_w));
	// map(0x00a0, 0x00a1) INT2
	map(0x00a0, 0x00a1).rw(m_pic_slave, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	// map(0x00c0, 0x00df) DMA2
	map(0x00c0, 0x00df).lrw8(
		NAME([this] (offs_t offset) { return m_dmac_slave->read( offset / 2 ); }),
		NAME([this] (offs_t offset, u8 data) { m_dmac_slave->write( offset / 2, data ); })
	);

	map(0x00e0, 0x00ef).noprw();
	// map(0x00f0, 0x00f0) COPRO error
	// map(0x0480, 0x048f) DMA high page registers
	// map(0x04d0, 0x04d1) IRQ edge/level control registers

	// http://bxr.su/DragonFly/share/man/man4/it.4
	// 0x0290, 0xc00, 0xd00, 0x228: Motherboard Super I/O HW monitoring

	// map(0x0295, 0x0295) - index register, $d-$e lower/upper RPM readback for fans (alternating on read)
	// map(0x0296, 0x0296) - data ^

	// Intel LPC interface specs (legacy host decode ranges, not necessarily present on '950)

	// map(0x002e, 0x002f) Super I/O config (config-index & data ports)
	// map(0x004e, 0x004f) alt Super I/O config (ISA converter index-data ports)
	// map(0x0062, 0x0062) - ACPI embedded controller
	// map(0x0066, 0x0066) /

	// map(0x0200, 0x020f) game ports ($201 as shutms11 default)

	// map(0x0220, 0x0227) serial 2
	// map(0x0228, 0x022f) serial 2
	// map(0x0220, 0x0233) sb compatible i/f 1
	// map(0x0240, 0x0253) sb compatible i/f 2
	// map(0x0260, 0x0273) sb compatible i/f 3
	// map(0x0280, 0x0293) sb compatible i/f 4

	// map(0x0238, 0x023f) serial 3

	// map(0x0278, 0x027f) parallel port 2 & PnP
	// map(0x02e8, 0x02ef) serial 3
	// unmap high: beos5 keeps PnP here for a COM3 that doesn't exist in shutms11
	map(0x02e8, 0x02ef).lr8(NAME([] () { return 0xff; }));
	// map(0x02f8, 0x02ff) serial 1

	// map(0x0300, 0x0301) - MIDI ($330 as shutms11 default)
	// map(0x0310, 0x0311) /
	// map(0x0320, 0x0321) /
	// map(0x0330, 0x0331) /

	// map(0x0338, 0x033f) serial 4
	// map(0x0370, 0x0377) FDC 2

	// map(0x0378, 0x037f) parallel port 1
	// map(0x0388, 0x0389) ADLIB
	// map(0x03bc, 0x03bf) parallel port 3
	// map(0x03e8, 0x03ef) serial 4
	// map(0x03f0, 0x03f7) FDC 1
	// map(0x03f8, 0x03ff).rw(m_uart, FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w)); // COM1

	// map(0x0530, 0x0537) - MSS (TCP Maximum Segment Size?)
	// map(0x0604, 0x060b) /
	// map(0x0e80, 0x0e87) /
	// map(0x0f40, 0x0f47) /

	// map(0x0678, 0x067f) ECP parallel port 1
	// map(0x0778, 0x0779) ECP parallel port 2 & PnP
	// map(0x07bc, 0x07bf) ECP parallel port 3
}

void sis950_lpc_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	m_isabus->remap(AS_PROGRAM, 0, 1 << 24);
	m_isabus->remap(AS_IO, 0, 0xffff);
	io_space->install_device(0, 0x07ff, *this, &sis950_lpc_device::io_map);

	LOGMAP("LPC Remapping table (BIOS: %02x, flash: %02x)\n", m_bios_control, m_flash_control);

	// ACPI enable
	if (BIT(m_bios_control, 7))
	{
		LOGMAP("- ACPI enable (%02x) %04x-%04x\n", m_bios_control, m_acpi_base, m_acpi_base + 0xff);
		// shutms11 BIOS POST maps this at $5000
		m_acpi->map_device(memory_window_start, memory_window_end, 0, memory_space, io_window_start, io_window_end, m_acpi_base, io_space);
		io_space->install_device(m_acpi_base | 0x80, m_acpi_base | 0xff, *m_smbus, &sis950_smbus_device::map);
	}

	// TODO: disable flash access write thru reg $45
	// TODO: BIOS positive decode
	// (bios_control bit 1), which should disable BIOS mapping to E and F segment

	memory_space->install_device(0x000e0000, 0x000fffff, *this, &sis950_lpc_device::memory_map<3>);
	// extended BIOS enable
	if (m_bios_control & 1)
	{
		LOGMAP("- Extend BIOS on\n");
		memory_space->install_device(0xfff80000, 0xfff9ffff, *this, &sis950_lpc_device::memory_map<0>);
		memory_space->install_device(0xfffa0000, 0xfffbffff, *this, &sis950_lpc_device::memory_map<1>);
		memory_space->install_device(0xfffc0000, 0xfffdffff, *this, &sis950_lpc_device::memory_map<2>);
	}
	memory_space->install_device(0xfffe0000, 0xffffffff, *this, &sis950_lpc_device::memory_map<3>);
}

u8 sis950_lpc_device::lpc_fast_init_r()
{
	LOGLPC("LPC fast init read [$92]\n");
	return m_lpc_legacy.fast_init;
}

void sis950_lpc_device::lpc_fast_init_w(offs_t offset, u8 data)
{
	LOGLPC("LPC fast init write [$92] %02x\n", data);
	if (data & 0xfd)
		LOG("Warning: unemulated LPC fast init type %02x", data);

	// TODO: pinpoint exact disable INIT condition and if that will be reflected on reading reg too
	m_host_cpu->set_input_line(INPUT_LINE_A20, BIT(data, 1));

	m_lpc_legacy.fast_init = data;
}

/*
 * Debugging
 */
u8 sis950_lpc_device::unmap_log_r(offs_t offset)
{
	LOGTODO("LPC Unemulated [%02x] R\n", offset + 0x10);
	return 0;
}

void sis950_lpc_device::unmap_log_w(offs_t offset, u8 data)
{
	LOGTODO("LPC Unemulated [%02x] %02x W\n", offset + 0x10, data);
}

/*
 * IRQ sharing
 */

void sis950_lpc_device::pc_irq1_w(int state)   { m_pic_master->ir1_w(state); }
void sis950_lpc_device::pc_irq3_w(int state)   { m_pic_master->ir3_w(state); }
void sis950_lpc_device::pc_irq4_w(int state)   { m_pic_master->ir4_w(state); }
void sis950_lpc_device::pc_irq5_w(int state)   { m_pic_master->ir5_w(state); }
void sis950_lpc_device::pc_irq6_w(int state)   { m_pic_master->ir6_w(state); }
void sis950_lpc_device::pc_irq7_w(int state)   { m_pic_master->ir7_w(state); }
void sis950_lpc_device::pc_irq8n_w(int state)  { m_pic_slave->ir0_w(state); }
void sis950_lpc_device::pc_irq9_w(int state)   { m_pic_slave->ir1_w(state); }
void sis950_lpc_device::pc_irq10_w(int state)  { m_pic_slave->ir2_w(state); }
void sis950_lpc_device::pc_irq11_w(int state)  { m_pic_slave->ir3_w(state); }
void sis950_lpc_device::pc_irq12m_w(int state) { m_pic_slave->ir4_w(state); }
void sis950_lpc_device::pc_irq14_w(int state)  { m_pic_slave->ir6_w(state); }
void sis950_lpc_device::pc_irq15_w(int state)  { m_pic_slave->ir7_w(state); }

/*
 * Start of legacy handling, to be moved out
 */

void sis950_lpc_device::pit_out0(int state)
{
	m_pic_master->ir0_w(state);
}

void sis950_lpc_device::pit_out1(int state)
{
	if(state)
		m_refresh = !m_refresh;
}

void sis950_lpc_device::pit_out2(int state)
{
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(m_at_spkrdata & m_pit_out2);
}

/*
 * x--- ---- Set if a PCI device or main memory module asserts PERR#/SERR# line
 * -x-- ---- Set when IOCHK# is asserted on ISA bus
 * --x- ---- Counter 2 out signal state (PIT ch. 2)
 * ---x ---- Refresh bit toggle (PIT ch. 1)
 * ---- xxxx <reads back NMI control enable>
 */
uint8_t sis950_lpc_device::nmi_status_r()
{
	uint8_t data = m_at_speaker;
	data &= ~0xd0; // AT BIOS don't likes this being set

	data |= m_refresh ? 0x10 : 0;

	if (m_pit_out2)
		data |= 0x20;
	else
		data &= ~0x20; // ps2m30 wants this

	return data;
}

void sis950_lpc_device::at_speaker_set_spkrdata(uint8_t data)
{
	m_at_spkrdata = data ? 1 : 0;
	m_speaker->level_w(m_at_spkrdata & m_pit_out2);
}

/*
 * xxxx ---- <must be zero when writing to this port>
 * ---- x--- (0) enable IOCHK# NMI (1) clear and disable IOCHK# NMI
 * ---- -x-- (0) enable PCI SERR# (1) clear and disable PCI SERR#
 * ---- --x- Speaker output enable
 * ---- ---x Timer counter 2 enable
 */
void sis950_lpc_device::nmi_control_w(uint8_t data)
{
	m_at_speaker = data;
	m_pit->write_gate2(BIT(data, 0));
	at_speaker_set_spkrdata(BIT(data, 1));
	m_channel_check = BIT(data, 3);
	if (m_channel_check)
		m_host_cpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void sis950_lpc_device::rtc_index_w(uint8_t data)
{
	m_rtc_index = data & 0x7f;
	// bit 7: NMI enable
}

u8 sis950_lpc_device::rtc_data_r()
{
	const u8 rtc_address = m_rtc_index | (m_rtc_reg & 0x80);
	return m_rtc->read_direct(rtc_address);
}

void sis950_lpc_device::rtc_data_w(u8 data)
{
	const u8 rtc_address = m_rtc_index | (m_rtc_reg & 0x80);
	m_rtc->write_direct(rtc_address, data);
}

uint8_t sis950_lpc_device::at_page8_r(offs_t offset)
{
	uint8_t data = m_at_pages[offset % 0x10];

	switch(offset % 8)
	{
	case 1:
		data = m_dma_offset[BIT(offset, 3)][2];
		break;
	case 2:
		data = m_dma_offset[BIT(offset, 3)][3];
		break;
	case 3:
		data = m_dma_offset[BIT(offset, 3)][1];
		break;
	case 7:
		data = m_dma_offset[BIT(offset, 3)][0];
		break;
	}
	return data;
}


void sis950_lpc_device::at_page8_w(offs_t offset, uint8_t data)
{
	m_at_pages[offset % 0x10] = data;

	switch(offset % 8)
	{
	case 0:
		// matches boot_state_infos_phoenix
		//m_boot_state_hook((offs_t)0, data);
		break;
	case 1:
		m_dma_offset[BIT(offset, 3)][2] = data;
		break;
	case 2:
		m_dma_offset[BIT(offset, 3)][3] = data;
		break;
	case 3:
		m_dma_offset[BIT(offset, 3)][1] = data;
		break;
	case 7:
		m_dma_offset[BIT(offset, 3)][0] = data;
		break;
	}
}

void sis950_lpc_device::pc_dma_hrq_changed(int state)
{
	m_host_cpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	// Assert HLDA
	m_dmac_slave->hack_w( state );
}

void sis950_lpc_device::iochck_w(int state)
{
//  if (!state && !m_channel_check && m_nmi_enabled)
	if (!state && !m_channel_check)
		m_host_cpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

uint8_t sis950_lpc_device::pc_dma_read_byte(offs_t offset)
{
	address_space& prog_space = m_host_cpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return 0xff;
	uint8_t result;
	offs_t page_offset = ((offs_t) m_dma_offset[0][m_dma_channel]) << 16;

	result = prog_space.read_byte(page_offset + offset);
	return result;
}


void sis950_lpc_device::pc_dma_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_host_cpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t) m_dma_offset[0][m_dma_channel]) << 16;

	prog_space.write_byte(page_offset + offset, data);
}


uint8_t sis950_lpc_device::pc_dma_read_word(offs_t offset)
{
	address_space& prog_space = m_host_cpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return 0xff;
	uint16_t result;
	offs_t page_offset = ((offs_t) m_dma_offset[1][m_dma_channel & 3]) << 16;

	result = prog_space.read_word((page_offset & 0xfe0000) | (offset << 1));
	m_dma_high_byte = result & 0xFF00;

	return result & 0xFF;
}


void sis950_lpc_device::pc_dma_write_word(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_host_cpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t) m_dma_offset[1][m_dma_channel & 3]) << 16;

	prog_space.write_word((page_offset & 0xfe0000) | (offset << 1), m_dma_high_byte | data);
}
