// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************************

MIT CADR emulation

TODO:
- Keyboard sometimes becomes unresponsive.
- Network not supported yet.
- Add support for a second disk controller board.
- System 100 errors not being able to communicatie to another host but it never
  tries to transmit anything.
- System 300 and 301 will not boot due to a timing/communication/bug issue with
  the keyboard.

**********************************************************************************/
#include "emu.h"
#include "cadr_disk.h"
#include "cadr_iob.h"
#include "cadr_tv_control.h"

#include "cpu/cadr/cadr.h"
#include "machine/input_merger.h"

#include "softlist_dev.h"


#define VERBOSE (0)
#include "logmacro.h"

namespace {

class cadr_state : public driver_device
{
public:
	cadr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_disk_controller(*this, "disk_controller")
		, m_maincpu(*this, "maincpu")
		, m_mainirq(*this, "mainirq")
		, m_xbusirq(*this, "xbusirq")
		, m_iob(*this, "iob")
		, m_tv_control(*this, "display")
	{ }

	void cadr(machine_config &config);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	static constexpr u8 IRQ_SOURCE_DISK = 0;
	static constexpr u8 IRQ_SOURCE_DISPLAY = 1;
	static constexpr u8 IRQ_SOURCE_IOB = 2;
	static constexpr u8 IRQ_SOURCE_UNIBUS = 3;
	static constexpr u8 IRQ_SOURCE_XBUS = 4;
	
	void mem_map(address_map &map);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) { return 0; }
	u32 disk_controller_r(offs_t offset);
	void disk_controller_w(offs_t offset, u32 data);
	TIMER_CALLBACK_MEMBER(tv_60hz_callback);
	void unibus_irq_and_vector(u16 vector);

	required_device<cadr_disk_device> m_disk_controller;
	required_device<cadr_cpu_device> m_maincpu;
	required_device<input_merger_any_high_device> m_mainirq;
	required_device<input_merger_any_high_device> m_xbusirq;
	required_device<cadr_iob_device> m_iob;
	required_device<cadr_tv_control_device> m_tv_control;

	// x------- -------- - Unibus interrupt request line (accepted)
	// -x------ -------- - Xbus interrupt request line
	// --xx---- -------- - Interrupt level
	// ----x--- -------- - Interrupt stop grants
	// -----x-- -------- - Enable unique interrupts
	// ------xx xxxxxx-- - Vector
	// -------- ------x- - Local Enable
	// -------- -------x - Disable interrupt grant
	u16 m_interrupt_status;

	// -------- --x----- - Unibus map error
	// -------- ---x---- - CADR parity error
	// -------- ----x--- - Unibus NXM error
	// -------- -----x-- - CADR address parity error
	// -------- ------x- - Xbus parity error
	// -------- -------x - Xbus NXM error
	u16 m_bus_error_status;
};


void cadr_state::mem_map(address_map &map)
{
	// Xbus memory 0 - 16777777 / 000000 - 3bffff

	// The system 100 boot program cannot handle more memory?
	map(0x000000, 0x1fffff).ram(); // 128KB - ~4MB

	// Xbus I/O 17000000 - 17377777 / 3c0000 - 3dffff
	//
	// Devices mentioned in documentation:
	// 17000000 - 17077777 / 3c0000 - 3c7fff - main tv screen
	// 17100000 - 17107777 / 3c8000 - 3c8fff - convolution box
	// 17110000 - 17110017 / 3c9000 - 3c900f - scheme chip interface
	// 17140000 - 17177777 / 3cc000 - 3cffff - audio i/o memory
	// 17200000 - 17277777 / 3d0000 - 3d7fff - color tv
	// 17377500 - 17377537 / 3dff40 - 3dff5f - grayscale tv (and frame grabber)
	// 17377600 - 17377677 / 3dff80 - 3dffbf - vision hardware control
	// 17377740 - 17377747 / 3dffe0 - 3dffe7 - audio i/o control
	// 17377750 - 17377757 / 3dffe8 - 3dffef - color tv control
	// 17377760 - 17377767 / 3dfff0 - 3dfff7 - main tv control
	// 17377770 - 17377773 / 3dfff8 - 3dfffb - second disk control
	// 17377774 - 17377777 / 3dfffc - 3dffff - first disk control

	// 3c0000 - main tv
	map(0x3c0000, 0x3c7fff).rw(m_tv_control, FUNC(cadr_tv_control_device::video_ram_read), FUNC(cadr_tv_control_device::video_ram_write));

	// 3dfff0-3dfff7 - TV control
	map(0x3dfff0, 0x3dfff7).rw(m_tv_control, FUNC(cadr_tv_control_device::tv_control_r), FUNC(cadr_tv_control_device::tv_control_w));

	map(0x3dfff8, 0x3dffff).rw(m_disk_controller, FUNC(cadr_disk_device::read), FUNC(cadr_disk_device::write));


	// Unibus - 16 bit bus - 17400000 - 17777777 / 3e0000 - 3fffff

	map(0x3e0000, 0x3fffff).lrw16(
		NAME([] (offs_t offset) {
			printf("Read unibus %08x\n", 0x3e0000 + offset);
			return 0xffff;
		}),
		NAME([] (offs_t offset, u16 data) {
			printf("Write unibus %08x %04x\n", 0x3e0000 + offset, data);
		})

	);
	map(0x3ff420, 0x3ff43f).rw(m_iob, FUNC(cadr_iob_device::read), FUNC(cadr_iob_device::write)).umask32(0x0000ffff);

	// 766000 - 766017 - diagnostic interface
	map(0x3ff600, 0x3ff60f).rw(m_maincpu, FUNC(cadr_cpu_device::diag_r), FUNC(cadr_cpu_device::diag_w)).umask32(0x0000ffff);

	// 3ff610/3ff611 - 766040/766042 - interrupt status
	map(0x3ff610, 0x3ff610).lrw16(
		NAME([this] () {
			return m_interrupt_status;
		}),
		NAME([this] (u16 data) {
			m_interrupt_status = (m_interrupt_status & ~0x3c01) | (data & 0x3c01);
		})
	).umask32(0x0000ffff);
	map(0x3ff611, 0x3ff611).lrw16(
		NAME([this] () {
			return m_interrupt_status;
		}),
		NAME([this] (u16 data) {
			m_interrupt_status = (m_interrupt_status & ~0x83fc) | (data & 0x83fc);
			if (!BIT(m_interrupt_status, 15))
				m_mainirq->in_w<IRQ_SOURCE_UNIBUS>(CLEAR_LINE);
		})
	).umask32(0x0000ffff);

	// 3ff612 - 766044 - (clear) bus error status
	map(0x3ff612, 0x3ff612).lrw16(
		NAME([this] () {
			return m_bus_error_status;
		}),
		NAME([this] (u16 data) {
			m_bus_error_status = 0;
		})
	).umask32(0x0000ffff);

	// This diagnostics interface is not described in AI Memo 528, but it
	// is used by the CC-TEST_MACHINE program to test another machine.
	//
	// This interface ends up writing to the debuggee's diagnostic/spy area
	// to single step through debug statements.
	//
	// 3ff620 - 3ff626 - Debug/diagnostic interface to another machine
	// 3ff620 / 766100 - read or write the debuggee unibus location
	// 3ff624 / 766110 - Bit 17 of the debuggee Unibus address
	// 3ff626 / 766114 - Debuggee Unibus address to access (bits 1-16)
	//
}


static INPUT_PORTS_START(cadr)
INPUT_PORTS_END


void cadr_state::machine_start()
{
	u8 *maincpu = memregion("maincpu")->base();
	u8 *prom = memregion("proms")->base();

	for (int i = 0; i < 0x200; i++)
	{
		for (int b = 0; b < 8; b++)
		{
			maincpu[((0x1ff - i) * 8) + b] = prom[(i * 8) + b];
		}
	}

	save_item(NAME(m_interrupt_status));
	save_item(NAME(m_bus_error_status));
}


void cadr_state::machine_reset()
{
	m_interrupt_status = 0;
	m_bus_error_status = 0;
}


void cadr_state::unibus_irq_and_vector(u16 vector)
{
	// around 025470 when bit 0 is set, bit 15 is checked
	if (!BIT(m_interrupt_status, 11) || !BIT(m_interrupt_status, 0))
	{
		m_interrupt_status = (m_interrupt_status & ~0x3fc) | 0x8000 | (vector & 0x3fc);
		if (BIT(m_interrupt_status, 11))
			m_interrupt_status |= 0x01;
		if (BIT(m_interrupt_status, 10))
		{
			m_interrupt_status |= 0x8000;
			m_mainirq->in_w<IRQ_SOURCE_UNIBUS>(ASSERT_LINE);
		}
	}
}


void cadr_state::cadr(machine_config &config)
{
	CADR(config, m_maincpu, 5'000'000); // Clock not correct
	m_maincpu->set_addrmap(AS_DATA, &cadr_state::mem_map);

	INPUT_MERGER_ANY_HIGH(config, m_mainirq).output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	INPUT_MERGER_ANY_HIGH(config, m_xbusirq).output_handler().set([this](int state) {
		m_interrupt_status = (m_interrupt_status & ~0x4000) | (state ? 0x4000 : 0);
		m_mainirq->in_w<IRQ_SOURCE_UNIBUS>(state);
	});

	CADR_DISK(config, m_disk_controller, 0);
	m_disk_controller->set_data_space(m_maincpu, AS_DATA);
	m_disk_controller->irq_callback().set(m_xbusirq, FUNC(input_merger_device::in_w<IRQ_SOURCE_DISK>));

	CADR_IOB(config, m_iob, 0);
	m_iob->irq_vector_callback().set(FUNC(cadr_state::unibus_irq_and_vector));

	CADR_TV_CONTROL(config, m_tv_control, 0);
	m_tv_control->irq_callback().set(m_xbusirq, FUNC(input_merger_device::in_w<IRQ_SOURCE_DISPLAY>));

	SOFTWARE_LIST(config, "hdd_list").set_original("cadr");
}


ROM_START(cadr)
	ROM_REGION64_BE(0x2000, "maincpu", ROMREGION_ERASE00)

	ROM_REGION64_BE(0x2000, "proms", ROMREGION_ERASE00)
	ROM_LOAD64_BYTE("cadr_1.bin.1b19", 0x002, 0x200, CRC(6a5a4183) SHA1(8957dc8ea542ef5bfac182889827a7da7365ef2e))
	ROM_LOAD64_BYTE("cadr_2.bin.1b17", 0x003, 0x200, CRC(63b63556) SHA1(bd30189c45c8bc17df9a8b210d6f60e056226019))
	ROM_LOAD64_BYTE("cadr_3.bin", 0x004, 0x200, CRC(b3a92fe1) SHA1(e1563331a3c23cb5b06b7a244c32dce2f3950a9a)) // 1C20 ?
	ROM_LOAD64_BYTE("cadr_4.bin", 0x005, 0x200, CRC(d8e9c9d1) SHA1(e2a4ae957f146b44b9efef150d7f984443df45f0)) // 1D16 ?
	ROM_LOAD64_BYTE("cadr_5.bin", 0x006, 0x200, CRC(ad31e93a) SHA1(0a0b9eca440fa2f7eee9307c17185489714ea571)) // 1E19 ?
	ROM_LOAD64_BYTE("cadr_6.bin", 0x007, 0x200, CRC(bf79cdd4) SHA1(65592629e2ec5b188610e45e25f4b9d265a408d5)) // 1E17 ?
	// PROMs at locations 1B20, 1B18, 1B16, 1D17, 1E20, 1E18 are not populated?
ROM_END

} // anonymous namespace

COMP(1978, cadr, 0, 0, cadr, cadr, cadr_state, empty_init, "MIT", "CADR", MACHINE_NOT_WORKING)
