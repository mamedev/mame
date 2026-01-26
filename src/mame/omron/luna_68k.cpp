// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Omron Luna M68K systems.
 *
 * Sources:
 *   - https://wiki.netbsd.org/ports/luna68k/
 */
/*
 * WIP
 *
 * This driver is currently based on a VME-based Luna with a 25MHz 68030, which
 * differs from the systems supported by NetBSD. Boards installed are:
 *
 *  C25   CPU, FPU, serial, RTC, 68030 + 68882 @ 25MHz?
 *  IOC2  I/O controller (floppy, SCSI, serial), 68000 @ 10MHz?
 *  GPU8  graphics processor + serial, 68020 @ 20MHz? + 68881 @ 16MHz?
 *  DPU8  video/framebuffer, Bt458 @ 108MHz
 *  CMC   communications (GPIB, Ethernet, serial), 68020 @ 12.5MHz?
 *
 * This specific machine may be an SX-9100 Model 90?
 */
#include "emu.h"

#include "cpu/m68000/m68030.h"

// memory
#include "machine/ram.h"

// various hardware
#include "machine/mc146818.h"
#include "machine/z80sio.h"
#include "machine/am9513.h"

// busses and connectors
#include "bus/rs232/rs232.h"
#include "bus/nscsi/hd.h"

// ioc2
#include "cpu/m68000/m68000.h"
#include "machine/hd63450.h"
#include "machine/mb87030.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"
#include "machine/z80scc.h"
#include "machine/z8536.h"

// gpu
#include "cpu/m68000/m68020.h"
#include "machine/mc68681.h"
#include "machine/mc68901.h"
#include "video/bt45x.h"
#include "video/hd63484.h"
#include "machine/nvram.h"
#include "screen.h"

// cmc
#include "machine/am79c90.h"
#include "machine/tms9914.h"

#define VERBOSE 0
#include "logmacro.h"

namespace {

class luna_68k_state : public driver_device
{
public:
	// CPU interrupts
	enum {
		ICPU_IOC,
		ICPU_SCSII,
		ICPU_SCSIE,
		ICPU_DMA0,
		ICPU_DMA1,
	};

	luna_68k_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_rtc(*this, "rtc")
		, m_sio(*this, "sio")
		, m_stc(*this, "stc")
		, m_serial(*this, "serial%u", 0U)
		, m_cpu_boot(*this, "cpu_boot")
		// ioc2
		, m_ioc_cpu(*this, "ioc")
		, m_ioc_dma(*this, "dma%u", 0U)
		, m_ioc_spc(*this, "scsi%u:7:spc", 0U)
		, m_ioc_fdc(*this, "fdc")
		, m_ioc_floppy(*this, "fdc:0")
		, m_ioc_scc(*this, "scc")
		, m_ioc_cio(*this, "cio")
		, m_ioc_ram(*this, "ioc_ram")
		, m_ioc_boot(*this, "ioc_boot")
		// gpu
		, m_gpu_cpu(*this, "gpu")
		, m_gpu_acrtc(*this, "acrtc")
		, m_gpu_dac(*this, "dac")
		, m_gpu_mfp(*this, "mfp")
		, m_gpu_tty(*this, "tty")
		, m_gpu_duart(*this, "duart%u", 0U)
		, m_screen(*this, "screen")
		, m_framebuffer(*this, "framebuffer")
		// cmc
		, m_cmc_cpu(*this, "cmc")
		, m_cmc_eth(*this, "lance")
		, m_cmc_gpib(*this, "gpib")
		, m_cmc_stc(*this, "cmc_stc")
		, m_cmc_scc1(*this, "cmc_scc1")
		, m_cmc_scc2(*this, "cmc_scc2")
		, m_cmc_boot(*this, "cmc_boot")
	{
	}

	// machine config
	void luna(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// address maps
	void cpu_map(address_map &map) ATTR_COLD;
	void cpu_cpuspace_map(address_map &map) ATTR_COLD;

	void ioc_cpu_map(address_map &map) ATTR_COLD;
	void ioc_cpuspace_map(address_map &map) ATTR_COLD;

	void gpu_cpu_map(address_map &map) ATTR_COLD;
	void hd63484_map(address_map &map) ATTR_COLD;

	void cmc_cpu_map(address_map &map) ATTR_COLD;

private:
	int m_ioc_interrupt_hack;
	u32 m_cpu_leds;
	u32 m_cpu_interrupts;
	u8 m_m2i;
	u8 m_i2m;

	[[maybe_unused]] u32 bus_error_r(offs_t offset);
	u16 ioc_ram_r(offs_t offset);
	void ioc_ram_w(offs_t offset, u16 data, u16 mem_mask);
	void cpu_reset_cb(int);
	void ioc_reset_cb(int);
	void cmc_boot_w(offs_t offset, u32 data);
	void cpu_leds_w(u32 data);
	void cpu_interrupt_set(int level, int state);

	void m2i_w(u32 data);
	u8 m2i_r();
	void m2i_int_clear(u8 data);
	void i2m_w(u8 data);
	u32 i2m_r();
	void i2m_int_clear(u32 data);
	u8 cpu_vector_r();
	void level1_w(int state);
	u8 level1_ack_r();
	void level5_w(int state);
	u8 level5_ack_r();
	u8 ioc_cio_vector_r();

	void gpu_params_w(u32 data);
	void gpu_slot_w(u32 data);

	// cpu
	required_device<m68030_device> m_cpu;
	required_device<ds1287_device> m_rtc;
	required_device<upd7201_device> m_sio;
	required_device<am9513_device> m_stc;
	required_device_array<rs232_port_device, 2> m_serial;
	memory_view m_cpu_boot;

	// ioc2
	required_device<m68000_device> m_ioc_cpu;
	required_device_array<hd63450_device, 2> m_ioc_dma;
	required_device_array<mb89352_device, 2> m_ioc_spc;
	required_device<mb8877_device> m_ioc_fdc;
	required_device<floppy_connector> m_ioc_floppy;
	required_device<z80scc_device> m_ioc_scc;
	required_device<z8536_device> m_ioc_cio;
	required_shared_ptr<u16> m_ioc_ram;
	memory_view m_ioc_boot;

	// gpu
	required_device<m68020fpu_device> m_gpu_cpu;
	required_device<hd63484_device> m_gpu_acrtc;
	required_device<bt458_device> m_gpu_dac;
	required_device<mc68901_device> m_gpu_mfp;
	required_device<rs232_port_device> m_gpu_tty;
	required_device_array<mc68681_device, 2> m_gpu_duart;
	required_device<screen_device> m_screen;
	required_shared_ptr<u32> m_framebuffer;
	void acrtc_display(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, int x, uint16_t data);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 crtc_flags_r();
	std::array<u32, 64> m_gpu_params;
	u32 m_gpu_slot;
	u32 m_gpu_size;

	// cmc
	required_device<m68020_device> m_cmc_cpu;
	required_device<am7990_device> m_cmc_eth;
	required_device<tms9914_device> m_cmc_gpib;
	required_device<am9513_device> m_cmc_stc;
	required_device<z80scc_device> m_cmc_scc1, m_cmc_scc2;
	memory_view m_cmc_boot;
};

void luna_68k_state::machine_start()
{
	save_item(NAME(m_m2i));
	save_item(NAME(m_i2m));
	save_item(NAME(m_ioc_interrupt_hack));
	save_item(NAME(m_cpu_leds));
	save_item(NAME(m_cpu_interrupts));
	save_item(NAME(m_gpu_params));
	save_item(NAME(m_gpu_slot));
	save_item(NAME(m_gpu_size));
}

void luna_68k_state::machine_reset()
{
	// there must be a select/motor/side select on somewhere
	auto *floppy = m_ioc_floppy->get_device();
	m_ioc_fdc->set_floppy(floppy);
	if (floppy)
	{
		floppy->ss_w(0);
		floppy->mon_w(0);
	}

	m_m2i = 0;
	m_i2m = 0;
	m_ioc_interrupt_hack = 2;
	m_cpu_leds = 0xffffffff;
	m_cpu_interrupts = 0;

	// Overlay the roms at 0 on reset
	m_cpu_boot.select(0);
	m_ioc_boot.select(0);
	m_cmc_boot.select(0);

	m_sio->ctsb_w(0);

	std::fill(m_gpu_params.begin(), m_gpu_params.end(), 0);
	m_gpu_slot = 0;
	m_gpu_size = 0;
}

void luna_68k_state::gpu_params_w(u32 data)
{
	m_gpu_params[m_gpu_size++] = data;
	if(m_gpu_size == m_gpu_params.size())
		m_gpu_size--;
}

void luna_68k_state::gpu_slot_w(u32 data)
{
	if(m_gpu_size) {
		std::string s = util::string_format("%02x:", m_gpu_slot);
		for(u32 i=0; i != m_gpu_size; i++)
			s += util::string_format(" %08x", m_gpu_params[i]);
		logerror("gpu command %s\n", s);
	}
	m_gpu_size = 0;
	m_gpu_slot = data;
}

u32 luna_68k_state::bus_error_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		LOG("bus_error_r 0x%x (%s)\n", offset << 2, machine().describe_context());
		m_cpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	}

	return 0;
}

// ioc/cpu shared ram, cpu is 32-bits wide, ioc is 16, hence the need for trampolines
u16 luna_68k_state::ioc_ram_r(offs_t offset)
{
	return m_ioc_ram[offset];
}

void luna_68k_state::ioc_ram_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_ioc_ram[offset] = (m_ioc_ram[offset] & ~mem_mask) | (data & mem_mask);
}

// For three of the four cpu, the rom overlays the ram at 0 until a
// reset opcode is reached, maybe.  It could also be om ram write,
// but the opcode is easier :-)

void luna_68k_state::cpu_reset_cb(int)
{
	m_cpu_boot.disable();
}

void luna_68k_state::ioc_reset_cb(int)
{
	m_ioc_boot.disable();
}

// There's no reset opcode in the cmc case, ram write seems to be the
// the most obvious option.  But it could also be something with
// whatever is at e00000.
void luna_68k_state::cmc_boot_w(offs_t offset, u32 data)
{
	m_cmc_boot.disable();
	m_cmc_cpu->space(AS_PROGRAM).write_dword(offset, data);
}

void luna_68k_state::cpu_leds_w(u32 data)
{
	if(m_cpu_leds == data)
		return;
	m_cpu_leds = data;

	if(0) {
		// Looks like a 1->0 transition on bit 24 (the RUN led) powers off
		u8 digit = (data >> 16) & 0xf;
		logerror("CPU leds %c %c%c%c%c%c%c%c%c %08x\n",
				 BIT(data, 23) ? digit >= 10 ? 'a' + (digit-10) : '0' + digit : '.',
				 BIT(data, 24) ? '.' : '#',
				 BIT(data, 25) ? '.' : '#',
				 BIT(data, 26) ? '.' : '#',
				 BIT(data, 27) ? '.' : '#',
				 BIT(data, 28) ? '.' : '#',
				 BIT(data, 29) ? '.' : '#',
				 BIT(data, 30) ? '.' : '#',
				 BIT(data, 31) ? '.' : '#',
				 data
				 );
	}
}

void luna_68k_state::m2i_w(u32 data)
{
	logerror("m2i_w %08x\n", data);
	m_m2i = data;
	m_ioc_cpu->set_input_line(1, ASSERT_LINE);
}

u8 luna_68k_state::m2i_r()
{
	return m_m2i;
}

void luna_68k_state::m2i_int_clear(u8 data)
{
	logerror("m2i interrupt clear %d\n", data);
	m_ioc_cpu->set_input_line(1, CLEAR_LINE);
}

void luna_68k_state::i2m_w(u8 data)
{
	logerror("%s i2m_w %02x\n", machine().time().to_string(), data);
	m_i2m = data;

	// First two clears happen before the interrupt is raised, wtf?
	if (m_ioc_interrupt_hack > 0) {
		m_ioc_interrupt_hack --;
		return;
	}

	cpu_interrupt_set(ICPU_IOC, 1);
}

u32 luna_68k_state::i2m_r()
{
	return m_i2m;
}

void luna_68k_state::i2m_int_clear(u32 data)
{
	logerror("%s i2m interrupt clear %d\n", machine().time().to_string(), data);
	cpu_interrupt_set(ICPU_IOC, 0);
}

void luna_68k_state::cpu_interrupt_set(int level, int state)
{
	//  logerror("cpu_interrupt_set(%d, %d)\n", level, state);
	if(state)
		m_cpu_interrupts |= 1 << level;
	else
		m_cpu_interrupts &= ~(1 << level);

	// Interrupt level is unknown
	//   scsi is 4
	m_cpu->set_input_line(4, m_cpu_interrupts ? ASSERT_LINE : CLEAR_LINE);
}

u8 luna_68k_state::cpu_vector_r()
{
	//  logerror("Interrupt taken, mask=%02x\n", m_cpu_interrupts);
	if(BIT(m_cpu_interrupts, ICPU_IOC)) {
		// Code tries to clear the interrupt at 30000838 but the mmu misroutes the address (to 838, in ram)?
		cpu_interrupt_set(ICPU_IOC, 0);
		return 0x50 | (m_i2m & 0xf);
	}
	if(BIT(m_cpu_interrupts, ICPU_SCSII))
	   return 0x68;
	if(BIT(m_cpu_interrupts, ICPU_SCSIE))
	   return 0x6c;
	if(BIT(m_cpu_interrupts, ICPU_DMA0)) {
		logerror("dma0 vector read\n");
		return 0x42;
	}
	if(BIT(m_cpu_interrupts, ICPU_DMA1))
	   return 0x78;

	return 24; // Spurious interrupt
}

u8 luna_68k_state::ioc_cio_vector_r()
{
	return 0x60 | (m_ioc_cio->intack_r() & 0xf);
}

u32 luna_68k_state::crtc_flags_r()
{
	return m_screen->vblank() ? 0x2000 : 0;
}

void luna_68k_state::acrtc_display(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, int x, uint16_t data)
{
	if(data)
		bitmap.pix(y, x) = 257;
	//      logerror("%d %d\n", x, y);
}

u32 luna_68k_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for(int y=0; y != 1024; y++) {
		u16 *dest = &bitmap.pix(y);
		for(int x=0; x != 1280; x++)
			*dest++ = 4;
	}
	m_gpu_acrtc->update_screen(screen, bitmap, cliprect);
	return 0;
}

void luna_68k_state::level1_w(int state)
{
	if(state && BIT(m_cpu_leds, 21))
		m_cpu->set_input_line(1, ASSERT_LINE);
}

u8 luna_68k_state::level1_ack_r()
{
	m_cpu->set_input_line(1, CLEAR_LINE);
	return m68000_base_device::autovector(1);
}

void luna_68k_state::level5_w(int state)
{
	if(state)
		m_cpu->set_input_line(5, ASSERT_LINE);
}

u8 luna_68k_state::level5_ack_r()
{
	m_cpu->set_input_line(5, CLEAR_LINE);
	return m68000_base_device::autovector(5);
}

void luna_68k_state::cpu_map(address_map &map)
{
	map(0x00000000, 0x00ffffff>>2).ram(); // 8 SIMMs for RAM arranged as two groups of 4, soldered
	map(0x00000000, 0x00000007).view(m_cpu_boot);
	m_cpu_boot[0](0x00000000, 0x00000007).rom().region("eprom", 0);

	map(0x20200000, 0x20200003).lr32(NAME([]() -> u32 { return 1; }));
	map(0x20280000, 0x2029ffff).rw(FUNC(luna_68k_state::ioc_ram_r), FUNC(luna_68k_state::ioc_ram_w));

	//  map(0x30000000, 0x3fffffff).r(FUNC(luna_68k_state::bus_error_r));
	map(0x30000834, 0x30000837).rw(FUNC(luna_68k_state::i2m_r), FUNC(luna_68k_state::m2i_w));
	map(0x30000838, 0x3000083b).w(FUNC(luna_68k_state::i2m_int_clear));

	map(0x30000900, 0x300009ff).rw(m_ioc_dma[0], FUNC(hd63450_device::read), FUNC(hd63450_device::write));
	map(0x30000b80, 0x30000b81); // bit 3 is direction for ioc dma0, but why?

	map(0x30000ba0, 0x30000ba3).lr32(NAME([]() { return 0x10; }));
	map(0x30000d00, 0x30000d1f).m(m_ioc_spc[0], FUNC(mb89352_device::map)).umask32(0x00ff00ff);
	map(0x30000d20, 0x30000d3f).m(m_ioc_spc[1], FUNC(mb89352_device::map)).umask32(0x00ff00ff);
	map(0x30001200, 0x300012ff).rw(m_ioc_dma[1], FUNC(hd63450_device::read), FUNC(hd63450_device::write));
	map(0x40000000, 0x4001ffff).rom().region("eprom", 0).mirror(0x01000000);

	map(0x50000000, 0x50000007).rw(m_sio, FUNC(upd7201_device::ba_cd_r), FUNC(upd7201_device::ba_cd_w)).umask32(0xff00ff00);
	map(0x58000000, 0x5800007f).rw(m_rtc, FUNC(ds1287_device::read_direct), FUNC(ds1287_device::write_direct)).umask32(0xff00ff00);
	map(0x60000000, 0x60000003).rw(m_stc, FUNC(am9513_device::read16), FUNC(am9513_device::write16));

	map(0x70000000, 0x70000003).portr("DIPS");
	map(0x78000000, 0x78000003).w(FUNC(luna_68k_state::cpu_leds_w));
	map(0x78000000, 0x78000003).lr32(NAME([]() { return 0x00000002; })); // 20000000 = Abort, 80000000 = SYSfail, ~00000002 = parity error

	map(0xd01f8000, 0xd01f8007).rw(m_gpu_acrtc, FUNC(hd63484_device::read16), FUNC(hd63484_device::write16)).umask32(0x0000ffff);
	map(0xd01ff000, 0xd01ff00f).m(m_gpu_dac, FUNC(bt458_device::map)).umask32(0x000000ff);

}

void luna_68k_state::cpu_cpuspace_map(address_map &map)
{
	map(0xfffffff3, 0xfffffff3).r(FUNC(luna_68k_state::level1_ack_r));
	map(0xfffffff5, 0xfffffff5).r(FUNC(luna_68k_state::cpu_vector_r));
	map(0xfffffff9, 0xfffffff9).r(FUNC(luna_68k_state::cpu_vector_r));
	map(0xfffffffb, 0xfffffffb).r(FUNC(luna_68k_state::level5_ack_r));
	map(0xffffffff, 0xffffffff).lr8(NAME([]() { return m68000_base_device::autovector(7); }));
}

void luna_68k_state::ioc_cpu_map(address_map &map)
{
	// am8530h-6pc scc @ 4.9152MHz
	// mb89352 x 2 scsi
	// mb8877a
	// hd63450ps10 x 2 dma
	// z0853606psc cio

	map(0x000000, 0x01ffff).ram(); // HM62256LP-10x4 (32768x4) - 128KB
	map(0x000000, 0x000fff).view(m_ioc_boot);
	m_ioc_boot[0](0x000000, 0x000fff).rom().region("ioc", 0);
	map(0x100000, 0x11ffff).ram().share(m_ioc_ram); // HM62256LP-10x8 (32768x4) - 128KB

	map(0xf86001, 0xf86001).w(FUNC(luna_68k_state::m2i_int_clear));
	map(0xf87000, 0xf87000).w(FUNC(luna_68k_state::i2m_w));
	map(0xf87001, 0xf87001).r(FUNC(luna_68k_state::m2i_r));
	map(0xfc0000, 0xfcffff).rom().region("ioc", 0);
	map(0xfe0000, 0xfe0fff).rom().region("ioc", 0);
	map(0xfef300, 0xfef307).rw(m_ioc_fdc, FUNC(mb8877_device::read), FUNC(mb8877_device::write)).umask16(0x00ff);

	map(0xfef400, 0xfef400).rw(m_ioc_scc, FUNC(z80scc_device::da_r), FUNC(z80scc_device::da_w));
	map(0xfef401, 0xfef401).rw(m_ioc_scc, FUNC(z80scc_device::ca_r), FUNC(z80scc_device::ca_w));
	map(0xfef402, 0xfef402).rw(m_ioc_scc, FUNC(z80scc_device::db_r), FUNC(z80scc_device::db_w));
	map(0xfef403, 0xfef403).rw(m_ioc_scc, FUNC(z80scc_device::cb_r), FUNC(z80scc_device::cb_w));

	map(0xfef600, 0xfef607).rw(m_ioc_cio, FUNC(z8536_device::read), FUNC(z8536_device::write)).umask16(0x00ff);
}

void luna_68k_state::ioc_cpuspace_map(address_map &map)
{
	map(0xfffff0, 0xffffff).m(m_ioc_cpu, FUNC(m68000_device::autovectors_map));
	map(0xfffff5, 0xfffff5).r(FUNC(luna_68k_state::ioc_cio_vector_r)); // Actual level unknown, just not 2 or 7
}

void luna_68k_state::gpu_cpu_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).rom().region("gpu", 0);

	map(0x50000000, 0x5003ffff).ram();

	map(0x70000000, 0x70000003).w(FUNC(luna_68k_state::gpu_params_w));
	map(0x70000320, 0x70000323).w(FUNC(luna_68k_state::gpu_slot_w));
	map(0x70000130, 0x70000133).r(FUNC(luna_68k_state::crtc_flags_r));

	// Tests is there's ram at 80800000, 80c00000 and 81000000,
	// records it as 4M, 8M, 12M or 28M available
	map(0x80000000, 0x80bfffff).ram().share(m_framebuffer); // M5M41000BJ  1mb  (1m x 1) dynamic RAM (8x12) - 12MB

	map(0xb0001000, 0xb0001003).nopw();

	map(0xb0004000, 0xb0004003).portr("GPU");
	map(0xb0080000, 0xb008001f).rw(m_gpu_mfp, FUNC(mc68901_device::read), FUNC(mc68901_device::write));
	map(0xb0081000, 0xb008100f).rw(m_gpu_duart[0], FUNC(mc68681_device::read), FUNC(mc68681_device::write));
	map(0xb0082000, 0xb008200f).rw(m_gpu_duart[1], FUNC(mc68681_device::read), FUNC(mc68681_device::write));

	map(0xb0090000, 0xb00900ff).ram().share("gpu_nvram"); // MBM2212-20 256x4 NVRAM x 2 - 256B

	map(0xc0000000, 0xc000ffff).ram(); // M5M5178P-55 64kb (8k x 8) static RAM (2x4)   -  64kB
	map(0xf0000000, 0xf003ffff).ram(); // M5M5258P-35 256kb (64k x 4) static RAM (x8)  - 256kB
}

void luna_68k_state::cmc_cpu_map(address_map &map)
{
	map(0x00000000, 0x0003ffff).ram(); // NEC D43256AC-10L x 8 (32768x8)
	map(0x00000000, 0x00000007).view(m_cmc_boot);
	m_cmc_boot[0](0x00000000, 0x00000007).rom().region("cmc", 0).w(FUNC(luna_68k_state::cmc_boot_w));

	map(0x00400000, 0x00407fff).rom().region("cmc", 0);
}

void luna_68k_state::hd63484_map(address_map &map)
{
	map(0x00000, 0xfffff).ram();
}

static DEVICE_INPUT_DEFAULTS_START(terminal)
	DEVICE_INPUT_DEFAULTS("RS232_RXBAUD", 0xff, RS232_BAUD_19200)
	DEVICE_INPUT_DEFAULTS("RS232_TXBAUD", 0xff, RS232_BAUD_19200)
DEVICE_INPUT_DEFAULTS_END

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK)
		.machine_config([](device_t *hd) {
							static_cast<nscsi_harddisk_device *>(hd)->set_default_model_name("CDC     94161-156");
						});
}
static void luna_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void luna_68k_state::luna(machine_config &config)
{
	M68030(config, m_cpu, 50_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &luna_68k_state::cpu_map);
	m_cpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &luna_68k_state::cpu_cpuspace_map);
	m_cpu->reset_cb().set(*this, FUNC(luna_68k_state::cpu_reset_cb));

	DS1287(config, m_rtc, 32'768);

	UPD7201(config, m_sio, 9'830'000); // D9.83B0
	m_sio->out_int_callback().set_inputline(m_cpu, M68K_IRQ_6);

	// console
	RS232_PORT(config, m_serial[0], default_rs232_devices, "terminal");
	m_serial[0]->set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));
	m_sio->out_txda_callback().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_sio->out_dtra_callback().set(m_serial[0], FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsa_callback().set(m_serial[0], FUNC(rs232_port_device::write_rts));
	m_serial[0]->rxd_handler().set(m_sio, FUNC(upd7201_device::rxa_w));
	m_serial[0]->cts_handler().set(m_sio, FUNC(upd7201_device::ctsa_w));

	// keyboard
	RS232_PORT(config, m_serial[1], default_rs232_devices, nullptr);
	m_sio->out_txdb_callback().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_sio->out_dtrb_callback().set(m_serial[1], FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsb_callback().set(m_serial[1], FUNC(rs232_port_device::write_rts));
	m_serial[1]->rxd_handler().set(m_sio, FUNC(upd7201_device::rxb_w));
	m_serial[1]->cts_handler().set(m_sio, FUNC(upd7201_device::ctsb_w));

	AM9513(config, m_stc, 9'830'000); // FIXME: clock? sources?
	m_stc->fout_cb().set(m_stc, FUNC(am9513_device::gate1_w)); // assumption based on a common configuration
	m_stc->out1_cb().set_inputline(m_cpu, M68K_IRQ_7);
	m_stc->out2_cb().set(FUNC(luna_68k_state::level5_w));
	m_stc->out3_cb().set(FUNC(luna_68k_state::level1_w));
	m_stc->out4_cb().set(m_sio, FUNC(upd7201_device::rxca_w));
	m_stc->out4_cb().append(m_sio, FUNC(upd7201_device::txca_w));
	m_stc->out5_cb().set(m_sio, FUNC(upd7201_device::rxcb_w));
	m_stc->out5_cb().append(m_sio, FUNC(upd7201_device::txcb_w));

	// IOC2
	M68000(config, m_ioc_cpu, 10_MHz_XTAL);
	m_ioc_cpu->set_addrmap(AS_PROGRAM, &luna_68k_state::ioc_cpu_map);
	m_ioc_cpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &luna_68k_state::ioc_cpuspace_map);
	m_ioc_cpu->reset_cb().set(*this, FUNC(luna_68k_state::ioc_reset_cb));

	HD63450(config, m_ioc_dma[0], 20'000'000 / 2, m_cpu);
	m_ioc_dma[0]->set_clocks(attotime::from_nsec(80), attotime::from_nsec(80), attotime::from_nsec(80), attotime::from_nsec(80));
	m_ioc_dma[0]->set_burst_clocks(attotime::from_nsec(80), attotime::from_nsec(80), attotime::from_nsec(80), attotime::from_nsec(80));
	m_ioc_dma[0]->irq_callback().set([this](int state) { cpu_interrupt_set(ICPU_DMA0, state); });
	HD63450(config, m_ioc_dma[1], 20'000'000 / 2, m_cpu);
	m_ioc_dma[1]->set_clocks(attotime::from_nsec(80), attotime::from_nsec(80), attotime::from_nsec(80), attotime::from_nsec(80));
	m_ioc_dma[1]->set_burst_clocks(attotime::from_nsec(80), attotime::from_nsec(80), attotime::from_nsec(80), attotime::from_nsec(80));
	m_ioc_dma[1]->irq_callback().set([this](int state) { cpu_interrupt_set(ICPU_DMA1, state); });


	// internal SCSI
	NSCSI_BUS(config, "scsi0");
	NSCSI_CONNECTOR(config, "scsi0:0", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:1", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:2", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:3", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:4", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:5", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi0:6", scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi0:7").option_set("spc", MB89352).machine_config(
		[this, dma=m_ioc_dma[0].target()](device_t *device)
		{
			mb89352_device &spc = downcast<mb89352_device &>(*device);

			spc.set_clock(10'000'000);
			spc.out_irq_callback().set([this](int state) { cpu_interrupt_set(ICPU_SCSII, state); });
			spc.out_dreq_callback().set(*dma, FUNC(hd63450_device::drq2_w));
			dma->dma8_read<2>().set(spc, FUNC(mb89352_device::dma_r));
			dma->dma8_write<2>().set(spc, FUNC(mb89352_device::dma_w));
		});

	// external SCSI
	NSCSI_BUS(config, "scsi1");
	NSCSI_CONNECTOR(config, "scsi1:0", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:1", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:2", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:3", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:4", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:5", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:6", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi1:7").option_set("spc", MB89352).machine_config(
		[this](device_t *device)
		{
			mb89352_device &spc = downcast<mb89352_device &>(*device);

			spc.set_clock(10'000'000);
			spc.out_irq_callback().set([this](int state) { cpu_interrupt_set(ICPU_SCSIE, state); });
		});

	MB8877(config, m_ioc_fdc, 32_MHz_XTAL / 4);
	FLOPPY_CONNECTOR(config, m_ioc_floppy, luna_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);

	SCC8530(config, m_ioc_scc, 4.9152_MHz_XTAL); // AM8530H-6PC
	m_ioc_scc->configure_channels(4'915'200, 4'915'200, 4'915'200, 4'915'200);
	Z8536(config, m_ioc_cio, 10'000'000);
	m_ioc_cio->irq_wr_cb().set_inputline(m_ioc_cpu, 2);

	// GPU
	M68020FPU(config, m_gpu_cpu, 33'340'000 / 2);
	m_gpu_cpu->set_addrmap(AS_PROGRAM, &luna_68k_state::gpu_cpu_map);

	HD63484(config, m_gpu_acrtc, 108_MHz_XTAL/32); // Some equivalent hidden somewhere
	m_gpu_acrtc->set_screen(m_screen);
	m_gpu_acrtc->set_addrmap(0, &luna_68k_state::hd63484_map);
	m_gpu_acrtc->set_display_callback(FUNC(luna_68k_state::acrtc_display));

	BT458(config, m_gpu_dac, 108_MHz_XTAL);

	MC68901(config, m_gpu_mfp, 3.6864_MHz_XTAL);
	m_gpu_mfp->set_timer_clock(3.6864_MHz_XTAL);
	m_gpu_mfp->out_tdo_cb().set(m_gpu_mfp, FUNC(mc68901_device::tc_w));
	m_gpu_mfp->out_tdo_cb().append(m_gpu_mfp, FUNC(mc68901_device::rc_w));
	//m_gpu_mfp->out_irq_cb().set_inputline(m_gpu_cpu, M68K_IRQ_7);

	RS232_PORT(config, m_gpu_tty, default_rs232_devices, nullptr);
	m_gpu_mfp->out_so_cb().set(m_gpu_tty, FUNC(rs232_port_device::write_txd));
	m_gpu_tty->rxd_handler().set(m_gpu_mfp, FUNC(mc68901_device::si_w));

	MC68681(config, m_gpu_duart[0], 3.6864_MHz_XTAL);
	MC68681(config, m_gpu_duart[1], 3.6864_MHz_XTAL);

	NVRAM(config, "gpu_nvram");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// VBlank may be connected to mfp interrupt 0

	m_screen->set_raw(108_MHz_XTAL, 1728, 0, 1280, 1056, 0, 1024);
	m_screen->set_screen_update(FUNC(luna_68k_state::screen_update));
	m_screen->set_palette(m_gpu_dac);

	// CMC
	M68020(config, m_cmc_cpu, 25_MHz_XTAL/2);
	m_cmc_cpu->set_addrmap(AS_PROGRAM, &luna_68k_state::cmc_cpu_map);

	AM7990(config, m_cmc_eth); // Linked to an AM7992BCD,
	TMS9914(config, m_cmc_gpib, 25_MHz_XTAL/5);
	AM9513(config, m_cmc_stc, 25_MHz_XTAL/2); // FIXME: clock unknown

	SCC8530(config, m_cmc_scc1, 4.9152_MHz_XTAL); // AM8530H-6PC
	SCC8530(config, m_cmc_scc2, 4.9152_MHz_XTAL); // AM8530H-6PC
	m_cmc_scc1->configure_channels(4'915'200, 4'915'200, 4'915'200, 4'915'200);
	m_cmc_scc2->configure_channels(4'915'200, 4'915'200, 4'915'200, 4'915'200);

}

INPUT_PORTS_START(luna)
	PORT_START("DIPS")
	PORT_DIPNAME(0x00000001, 0x00000000, "Autoboot")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00000001, "On")
	PORT_DIPNAME(0x00000002, 0x00000000, "Console on bitmap")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00000002, "On")
	PORT_DIPNAME(0x00000004, 0x00000000, "unk02")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00000004, "On")
	PORT_DIPNAME(0x00000008, 0x00000000, "unk03")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00000008, "On")
	PORT_DIPNAME(0x00000010, 0x00000000, "unk04")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00000010, "On")
	PORT_DIPNAME(0x00000020, 0x00000000, "unk05")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00000020, "On")
	PORT_DIPNAME(0x00000040, 0x00000000, "unk06")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00000040, "On")
	PORT_DIPNAME(0x00000080, 0x00000000, "Disable diagnostics")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00000080, "On")
	PORT_DIPNAME(0x00000100, 0x00000000, "unk08")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00000100, "On")
	PORT_DIPNAME(0x00000200, 0x00000000, "unk09")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00000200, "On")
	PORT_DIPNAME(0x00000400, 0x00000000, "unk0a")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00000400, "On")
	PORT_DIPNAME(0x00000800, 0x00000000, "unk0b")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00000800, "On")
	PORT_DIPNAME(0x00001000, 0x00000000, "unk0c")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00001000, "On")
	PORT_DIPNAME(0x00002000, 0x00000000, "unk0d")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00002000, "On")
	PORT_DIPNAME(0x00004000, 0x00000000, "unk0e")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00004000, "On")
	PORT_DIPNAME(0x00008000, 0x00000000, "unk0f")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00008000, "On")
	PORT_DIPNAME(0x00010000, 0x00000000, "Control line test")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00010000, "On")
	PORT_DIPNAME(0x00020000, 0x00000000, "FPP test")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00020000, "On")
	PORT_DIPNAME(0x00040000, 0x00000000, "RAM Data test")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00040000, "On")
	PORT_DIPNAME(0x00080000, 0x00000000, "Segment LED display test")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00080000, "On")
	PORT_DIPNAME(0x00100000, 0x00000000, "RAM Address test")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00100000, "On")
	PORT_DIPNAME(0x00200000, 0x00000000, "NO test")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00200000, "On")
	PORT_DIPNAME(0x00400000, 0x00000000, "Serial input test")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00400000, "On")
	PORT_DIPNAME(0x00800000, 0x00000000, "Serial output test")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x00800000, "On")
	PORT_DIPNAME(0x01000000, 0x00000000, "unk18")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x01000000, "On")
	PORT_DIPNAME(0x02000000, 0x00000000, "unk19")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x02000000, "On")
	PORT_DIPNAME(0x04000000, 0x00000000, "unk1a")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x04000000, "On")
	PORT_DIPNAME(0x08000000, 0x00000000, "unk1b")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x08000000, "On")
	PORT_DIPNAME(0x10000000, 0x00000000, "unk1c")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x10000000, "On")
	PORT_DIPNAME(0x20000000, 0x00000000, "unk1d")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x20000000, "On")
	PORT_DIPNAME(0x40000000, 0x00000000, "unk1e")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x40000000, "On")
	PORT_DIPNAME(0x80000000, 0x00000000, "unk1f")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x80000000, "On")

	PORT_START("GPU")
	PORT_DIPNAME(0x01000000, 0x00000000, "unk00")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x01000000, "On")
	PORT_DIPNAME(0x02000000, 0x00000000, "unk01")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x02000000, "On")
	PORT_DIPNAME(0x04000000, 0x00000000, "unk02")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x04000000, "On")
	PORT_DIPNAME(0x08000000, 0x00000000, "unk03")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x08000000, "On")
	PORT_DIPNAME(0x10000000, 0x00000000, "unk04")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x10000000, "On")
	PORT_DIPNAME(0x20000000, 0x00000000, "unk05")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x20000000, "On")
	PORT_DIPNAME(0x40000000, 0x00000000, "unk06")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x40000000, "On")
	PORT_DIPNAME(0x80000000, 0x00000000, "unk07")
	PORT_DIPSETTING(         0x00000000, "Off")
	PORT_DIPSETTING(         0x80000000, "On")
INPUT_PORTS_END

ROM_START(luna)
	ROM_REGION32_BE(0x20000, "eprom", 0)
	ROM_LOAD16_WORD_SWAP("0283__ac__8117__1.05.ic88", 0x00000, 0x20000, CRC(c46dec54) SHA1(22ef9274f4ef85d446d56cce13a68273dc55f10a))

	// Force firmware to reinitialize nvram at first boot
	ROM_REGION(64, "rtc", ROMREGION_ERASEFF)

	ROM_REGION16_BE(0x10000, "ioc", 0)
	ROM_LOAD16_BYTE("8145__h__3.24.ic108", 0x0000, 0x8000, CRC(d2dde582) SHA1(e34c15e43869be573272503d1f47e9e244536396))
	ROM_LOAD16_BYTE("8145__l__3.24.ic100", 0x0001, 0x8000, CRC(4863329b) SHA1(881623c3a64260f5cc1be066dbb47799d1f2ce14))

	ROM_REGION32_BE(0x40000, "gpu", 0)
	ROM_LOAD("jaw-2500__rom0__v1.21.rom0", 0x00000, 0x20000, CRC(3aa5dfa8) SHA1(e703402b6d2271c303c6abe8833281e994c244de))
	ROM_LOAD("jaw-2500__rom1__v1.21.rom1", 0x20000, 0x20000, CRC(9881eecd) SHA1(4a87417d9bf801e797bf504d18a6c6b5d3911706))

	ROM_REGION16_BE(0x8000, "cmc", 0)
	ROM_LOAD("8112_v1_1.ic54", 0x0000, 0x8000, CRC(b87e0122) SHA1(22290850761ed3dddb2369e062012679e2963fa3))
ROM_END

} // anonymous namespace

/*   YEAR   NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS           INIT         COMPANY  FULLNAME  FLAGS */
COMP(1989?, luna, 0,      0,      luna,    luna,  luna_68k_state, empty_init,  "Omron", "Luna",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
