// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************************

MIT CADR emulation

**********************************************************************************/
#include "emu.h"
#include "cadr_disk.h"
#include "cadr_display.h"
#include "cadr_iob.h"

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
		, m_iob(*this, "iob")
		, m_display(*this, "display")
	{ }

	void cadr(machine_config &config);

protected:
	void machine_start() override;
	void machine_reset() override;

private:
	static constexpr u8 IRQ_SOURCE_DISK = 0;
	static constexpr u8 IRQ_SOURCE_DISPLAY = 1;
	static constexpr u8 IRQ_SOURCE_IOB = 2;
	static constexpr u8 IRQ_SOURCE_UNIBUS = 3;
	
	void mem_map(address_map &map);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) { return 0; }
	u32 disk_controller_r(offs_t offset);
	void disk_controller_w(offs_t offset, u32 data);
	TIMER_CALLBACK_MEMBER(tv_60hz_callback);
	void unibus_irq_and_vector(u16 vector);

	required_device<cadr_disk_device> m_disk_controller;
	required_device<cadr_cpu_device> m_maincpu;
	required_device<input_merger_any_high_device> m_mainirq;
	required_device<cadr_iob_device> m_iob;
	required_device<cadr_display_device> m_display;
	u32 m_irq_and_vector;
};


void cadr_state::mem_map(address_map &map)
{
	// Xbus memory

	// The system 100 boot program cannot handle more memory?
	map(0x000000, 0x1fffff).ram(); // 128KB - ~4MB
	// Xbus I/O
	// 3c0000 - video
	// up to 3c7fff ?
	map(0x3c0000, 0x3dfffb).lrw32(
		NAME([] (offs_t offset, u32 mem_mask) {
			printf("3c0000-3dffff: read %08x (%o)\n", 0x3c0000 + offset, 0x3c0000 + offset);
			return 0;
		}),
		NAME([] (offs_t offset, u32 data, u32 mem_mask) {
			printf("3c0000-3dffff: write %08x(%o), data %08x (%o)\n", 0x3c0000 + offset, 0x3c0000 + offset, data, data);
		})		
	);

	map(0x3c0000, 0x3c7fff).rw(m_display, FUNC(cadr_display_device::video_ram_read), FUNC(cadr_display_device::video_ram_write));

	// 3d0000 ?
	// 3dff00 ?

	// 3dfff0-3dfff7 - TV
	// 3dfff1 ?
	// 3dfff2 ?
	// 3dfff3 ?
	map(0x3dfff0, 0x3dfff0).rw(m_display, FUNC(cadr_display_device::status_r), FUNC(cadr_display_device::status_w));
	map(0x3dfff1, 0x3dfff7).lrw32(
		NAME([] (offs_t offset, u32 mem_mask) {
			printf("0x3dfff1-0x3dfff7: read %08x (%o)\n", 0x3dfff1 + offset, 0x3dfff1 + offset);
			return 1 << 4/*0*/;
		}),
		NAME([] (offs_t offset, u32 data, u32 mem_mask) {
			printf("0x3dfff1-0x3dfff7: write %08x(%o), data %08x (%o)\n", 0x3dfff1 + offset, 0x3dfff1 + offset, data, data);
		})		
	);

	// 3dfff9 / 17377771
	map(0x3dfff9, 0x3dfff9).r(m_disk_controller, FUNC(cadr_disk_device::memory_address_r));
	// 3dfffc / 17377774
	map(0x3dfffc, 0x3dfffc).rw(m_disk_controller, FUNC(cadr_disk_device::status_r), FUNC(cadr_disk_device::command_w));
	// 3dfffd / 17377775
	map(0x3dfffd, 0x3dfffd).rw(m_disk_controller, FUNC(cadr_disk_device::command_list_r), FUNC(cadr_disk_device::command_list_w));
	// 3dfffe / 17377776
	map(0x3dfffe, 0x3dfffe).rw(m_disk_controller, FUNC(cadr_disk_device::disk_address_r), FUNC(cadr_disk_device::disk_address_w));
	// 3dffff / 17377777
	map(0x3dffff, 0x3dffff).rw(m_disk_controller, FUNC(cadr_disk_device::error_correction_r), FUNC(cadr_disk_device::start_w));

	// Unibus - 16 bit bus

	// Unibus - 16bit bus
	// To catch unknown writes
	map(0x3e0000, 0x3fffff).lrw32(
		NAME([] (offs_t offset, u32 mem_mask) {
			printf("3e0000-3fffff: unibus read %08x (%o)\n", 0x3e0000 + offset, 0x3e0000 + offset);
			return 0;
		}),
		NAME([] (offs_t offset, u32 data, u32 mem_mask) {
			printf("3e0000-3fffff: unibus write %08x (%o), data %04x (%o)\n", 0x3e0000 + offset, 0x3e0000 + offset, data, data);
		})		
	);

	map(0x3ff420, 0x3ff43f).rw(m_iob, FUNC(cadr_iob_device::read), FUNC(cadr_iob_device::write));

	// FIXME: When using 16bit handlers a write to offset 5 causes 2 writes, 1 to offset 0 and 1 to offset 1
	map(0x3ff600, 0x3ff60f).rw(m_maincpu, FUNC(cadr_cpu_device::diag_r), FUNC(cadr_cpu_device::diag_w));
	// 3ff610 - interrupt status
	map(0x3ff610, 0x3ff610).lrw32(
		NAME([this] (u32 mem_mask) {
//			printf("3ff610: unibus irq read 0x%x\n", m_irq_and_vector);
			// Not sure if this is the right location
			m_mainirq->in_w<IRQ_SOURCE_UNIBUS>(CLEAR_LINE);
			m_irq_and_vector &= ~0x8001;
			return m_irq_and_vector;
		}),
		NAME([this] (u32 data, u32 mem_mask) {
//			printf("3ff610: unibus irq write, data %04x (%o)\n", data, data);
			m_irq_and_vector = (m_irq_and_vector & ~0xfffffc01) | (data & 0xfffffc01); // usim uses mask 3c01
		})
	);
	// 3ff611 - interrupt stim
	// 3ff612 - clear bus error
}


static INPUT_PORTS_START(cadr)
INPUT_PORTS_END


void cadr_state::machine_start()
{
	u8 *maincpu = memregion("maincpu")->base();
	u8 *prom = memregion("proms")->base();

	for (int i = 0; i < 0x200; i++)
		for (int b = 0; b < 8; b++)
			maincpu[((511 - i) * 8) + b] = prom[(i * 8) + b];

	save_item(NAME(m_irq_and_vector));
}


void cadr_state::machine_reset()
{
	m_irq_and_vector = 0;
}


void cadr_state::unibus_irq_and_vector(u16 vector)
{
	// around 025470 when bit 0 is 1, MD<<21 is checked  0100000 
	m_irq_and_vector = (m_irq_and_vector & ~0x3fc) | 0x8000 | (vector & 0x3fc) | 0x01;
	m_mainirq->in_w<IRQ_SOURCE_UNIBUS>(ASSERT_LINE);
}


void cadr_state::cadr(machine_config &config)
{
	CADR(config, m_maincpu, 10'000'000); // Clock not correct
	m_maincpu->set_addrmap(AS_DATA, &cadr_state::mem_map);

	INPUT_MERGER_ANY_HIGH(config, m_mainirq).output_handler().set_inputline("maincpu", INPUT_LINE_IRQ0);

	CADR_DISK(config, m_disk_controller, 0);
	m_disk_controller->set_data_space(m_maincpu, AS_DATA);
	m_disk_controller->irq_callback().set(m_mainirq, FUNC(input_merger_device::in_w<IRQ_SOURCE_DISK>));

	CADR_IOB(config, m_iob, 0);
	m_iob->irq_vector_callback().set(FUNC(cadr_state::unibus_irq_and_vector));

	CADR_DISPLAY(config, m_display, 0);
	m_display->irq_callback().set(m_mainirq, FUNC(input_merger_device::in_w<IRQ_SOURCE_DISPLAY>));

	SOFTWARE_LIST(config, "hdd_list").set_original("cadr");
}


ROM_START(cadr)
	ROM_REGION64_BE(0x2000, "maincpu", ROMREGION_ERASE00)

	ROM_REGION64_BE(0x2000, "proms", ROMREGION_ERASE00)
	ROM_LOAD64_BYTE("cadr_1.bin", 0x002, 0x200, CRC(6a5a4183) SHA1(8957dc8ea542ef5bfac182889827a7da7365ef2e)) // 1B19 ?
	ROM_LOAD64_BYTE("cadr_2.bin", 0x003, 0x200, CRC(63b63556) SHA1(bd30189c45c8bc17df9a8b210d6f60e056226019)) // 1B17 ?
	ROM_LOAD64_BYTE("cadr_3.bin", 0x004, 0x200, CRC(b3a92fe1) SHA1(e1563331a3c23cb5b06b7a244c32dce2f3950a9a)) // 1C20 ?
	ROM_LOAD64_BYTE("cadr_4.bin", 0x005, 0x200, CRC(d8e9c9d1) SHA1(e2a4ae957f146b44b9efef150d7f984443df45f0)) // 1D16 ?
	ROM_LOAD64_BYTE("cadr_5.bin", 0x006, 0x200, CRC(ad31e93a) SHA1(0a0b9eca440fa2f7eee9307c17185489714ea571)) // 1E19 ?
	ROM_LOAD64_BYTE("cadr_6.bin", 0x007, 0x200, CRC(bf79cdd4) SHA1(65592629e2ec5b188610e45e25f4b9d265a408d5)) // 1E17 ?
	// PROMs at locations 1B20, 1B18, 1B16, 1D17, 1E20, !E18 are not populated?
ROM_END

} // anonymous namespace

COMP(1976, cadr, 0, 0, cadr, cadr, cadr_state, empty_init, "MIT", "CADR", MACHINE_NOT_WORKING)
