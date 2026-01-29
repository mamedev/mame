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

#include "luna_kbd.h"
#include "luna_68k_ioc.h"
#include "luna_68k_video.h"
#include "luna_68k_cmc.h"

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68030.h"
#include "machine/ram.h"
#include "machine/mc146818.h"
#include "machine/z80sio.h"
#include "machine/am9513.h"

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
		, m_ioc(*this, "ioc")
		, m_video(*this, "videocard")
		, m_cmc(*this, "cmc")
		, m_cpu(*this, "cpu")
		, m_rtc(*this, "rtc")
		, m_sio(*this, "sio")
		, m_stc(*this, "stc")
		, m_serial(*this, "serial%u", 0U)
		, m_cpu_boot(*this, "cpu_boot")
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

private:
	u32 m_cpu_leds;
	u32 m_cpu_interrupts;

	[[maybe_unused]] u32 bus_error_r(offs_t offset);
	void cpu_reset_cb(int);
	void cpu_leds_w(u32 data);
	void cpu_interrupt_set(int level, int state);

	u8 cpu_vector_r();
	void level1_w(int state);
	u8 level1_ack_r();
	void level5_w(int state);
	u8 level5_ack_r();

	required_device<luna_68k_ioc_device> m_ioc;
	required_device<luna_68k_video_connector> m_video;
	required_device<luna_68k_cmc_device> m_cmc;
	required_device<m68030_device> m_cpu;
	required_device<ds1287_device> m_rtc;
	required_device<upd7201_device> m_sio;
	required_device<am9513_device> m_stc;
	required_device_array<rs232_port_device, 2> m_serial;
	memory_view m_cpu_boot;

	// gpu
	//	void acrtc_display(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, int x, uint16_t data);
	//	u32 crtc_flags_r();

	// cmc
};

void luna_68k_state::machine_start()
{
	save_item(NAME(m_cpu_leds));
	save_item(NAME(m_cpu_interrupts));

	m_cpu->space(0).install_device(0, 0xffffffff, *m_ioc, &luna_68k_ioc_device::vme_map);
	m_cpu->space(0).install_device(0, 0xffffffff, *m_cmc, &luna_68k_cmc_device::vme_map);
	m_video->install_vme_map(m_cpu->space(0));
}

void luna_68k_state::machine_reset()
{
	m_cpu_leds = 0xffffffff;
	m_cpu_interrupts = 0;

	// Overlay the roms at 0 on reset
	m_cpu_boot.select(0);

	m_sio->ctsb_w(0);
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

// For three of the four cpu, the rom overlays the ram at 0 until a
// reset opcode is reached, maybe.  It could also be on ram write,
// but the opcode is easier :-)

void luna_68k_state::cpu_reset_cb(int)
{
	m_cpu_boot.disable();
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
		return 0x50 | (m_ioc->get_i2m() & 0xf);
	}
	if(BIT(m_cpu_interrupts, ICPU_SCSII))
		return 0x68;
	if(BIT(m_cpu_interrupts, ICPU_SCSIE))
		return 0x6c;

	// Not sure the dma interrupts exist
	if(BIT(m_cpu_interrupts, ICPU_DMA0))
		return 0x42;
	if(BIT(m_cpu_interrupts, ICPU_DMA1))
		return 0x78;

	return 24; // Spurious interrupt
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
	// Firmware handles up to 512M, kernel stops at 256M, but
	// diagnostics and kernel boot take multiple eternities to run.

	// DMA can handle up to 64M (24-bits worth of 32-bit words).
	// There doesn't seem to be code for bounce buffers in the kernel.

	// "Normal" ram size is 16M, fast boot for testing is 4M.
	// Hardware limit may have been 32M, maybe more.  There is at
	// least one real system booting with 24M.

	// 16M is done as 4 SIMMs for RAM arranged as two groups of 4,
	// soldered. There are 8 potential slots.  That limits naturally
	// to 32M, but it's unknown how many address lines max the simms
	// can have.

	map(0x00000000, 0x003fffff).ram(); 
	map(0x00000000, 0x00000007).view(m_cpu_boot);
	m_cpu_boot[0](0x00000000, 0x00000007).rom().region("eprom", 0);

	// 2* is 24-bits "short" VME space

	map(0x20200000, 0x20200003).lr32(NAME([]() -> u32 { return 1; }));

	// if < 0x40, swap page size is 0x400 instead of 0x1000, and the disk cache breaks
	map(0x30000ba0, 0x30000ba3).lr32(NAME([]() { return 0x40 << 16; }));

	map(0x40000000, 0x4001ffff).rom().region("eprom", 0).mirror(0x01000000);

	map(0x50000000, 0x50000007).rw(m_sio, FUNC(upd7201_device::ba_cd_r), FUNC(upd7201_device::ba_cd_w)).umask32(0xff00ff00);
	map(0x58000000, 0x5800007f).rw(m_rtc, FUNC(ds1287_device::read_direct), FUNC(ds1287_device::write_direct)).umask32(0xff00ff00);
	map(0x60000000, 0x60000003).rw(m_stc, FUNC(am9513_device::read16), FUNC(am9513_device::write16));

	map(0x70000000, 0x70000003).portr("DIPS");
	map(0x78000000, 0x78000003).w(FUNC(luna_68k_state::cpu_leds_w));
	map(0x78000000, 0x78000003).lr32(NAME([]() { return 0x00000002; })); // 20000000 = Abort, 80000000 = SYSfail, ~00000002 = parity error

	map(0x80000000, 0xffffffff).r(FUNC(luna_68k_state::bus_error_r));
}

void luna_68k_state::cpu_cpuspace_map(address_map &map)
{
	map(0xfffffff3, 0xfffffff3).r(FUNC(luna_68k_state::level1_ack_r));
	map(0xfffffff5, 0xfffffff5).r(FUNC(luna_68k_state::cpu_vector_r));
	map(0xfffffff7, 0xfffffff7).lr8(NAME([]() { return m68000_base_device::autovector(3); }));
	map(0xfffffff9, 0xfffffff9).r(FUNC(luna_68k_state::cpu_vector_r));
	map(0xfffffffb, 0xfffffffb).r(FUNC(luna_68k_state::level5_ack_r));
	map(0xfffffffd, 0xfffffffd).lr8(NAME([]() { return m68000_base_device::autovector(6); }));
	map(0xffffffff, 0xffffffff).lr8(NAME([]() { return m68000_base_device::autovector(7); }));
}

static void keyboard_devices(device_slot_interface &device)
{
	device.option_add("keyboard", LUNA_KEYBOARD);
}

void luna_68k_state::luna(machine_config &config)
{
	M68030(config, m_cpu, 50_MHz_XTAL / 2);
	m_cpu->set_addrmap(AS_PROGRAM, &luna_68k_state::cpu_map);
	m_cpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &luna_68k_state::cpu_cpuspace_map);
	m_cpu->reset_cb().set(*this, FUNC(luna_68k_state::cpu_reset_cb));

	DS1287(config, m_rtc, 32'768);

	UPD7201(config, m_sio, XTAL(9'830'000)); // D9.83B0
	m_sio->out_int_callback().set_inputline(m_cpu, M68K_IRQ_6);

	// console
	RS232_PORT(config, m_serial[0], default_rs232_devices, nullptr);
	m_sio->out_txda_callback().set(m_serial[0], FUNC(rs232_port_device::write_txd));
	m_sio->out_dtra_callback().set(m_serial[0], FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsa_callback().set(m_serial[0], FUNC(rs232_port_device::write_rts));
	m_serial[0]->rxd_handler().set(m_sio, FUNC(upd7201_device::rxa_w));
	m_serial[0]->cts_handler().set(m_sio, FUNC(upd7201_device::ctsa_w));

	// keyboard
	RS232_PORT(config, m_serial[1], keyboard_devices, "keyboard");
	m_sio->out_txdb_callback().set(m_serial[1], FUNC(rs232_port_device::write_txd));
	m_sio->out_dtrb_callback().set(m_serial[1], FUNC(rs232_port_device::write_dtr));
	m_sio->out_rtsb_callback().set(m_serial[1], FUNC(rs232_port_device::write_rts));
	m_serial[1]->rxd_handler().set(m_sio, FUNC(upd7201_device::rxb_w));
	m_serial[1]->cts_handler().set(m_sio, FUNC(upd7201_device::ctsb_w));

	AM9513(config, m_stc, XTAL(9'830'000)/2);
	m_stc->fout_cb().set(m_stc, FUNC(am9513_device::gate1_w)); // assumption based on a common configuration
	m_stc->out1_cb().set_inputline(m_cpu, M68K_IRQ_7);
	m_stc->out2_cb().set(FUNC(luna_68k_state::level5_w));
	m_stc->out3_cb().set(FUNC(luna_68k_state::level1_w));
	m_stc->out4_cb().set(m_sio, FUNC(upd7201_device::rxca_w));
	m_stc->out4_cb().append(m_sio, FUNC(upd7201_device::txca_w));
	m_stc->out5_cb().set(m_sio, FUNC(upd7201_device::rxcb_w));
	m_stc->out5_cb().append(m_sio, FUNC(upd7201_device::txcb_w));

	LUNA_68K_IOC(config, m_ioc);
	m_ioc->set_main_space_tag(m_cpu, AS_PROGRAM);
	m_ioc->interrupt_cb().set([this](int state) { cpu_interrupt_set(ICPU_IOC, state); });
	m_ioc->interrupt_scsii_cb().set([this](int state) { cpu_interrupt_set(ICPU_SCSII, state); });
	m_ioc->interrupt_scsie_cb().set([this](int state) { cpu_interrupt_set(ICPU_SCSIE, state); });
	m_ioc->interrupt_dma0_cb().set([this](int state) { cpu_interrupt_set(ICPU_DMA0, state); });
	m_ioc->interrupt_dma1_cb().set([this](int state) { cpu_interrupt_set(ICPU_DMA1, state); });

	LUNA_68K_VIDEO_CONNECTOR(config, m_video, luna_68k_video_intf, "gpu");

	LUNA_68K_CMC(config, m_cmc);
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
INPUT_PORTS_END

ROM_START(luna)
	ROM_REGION32_BE(0x20000, "eprom", 0)
	ROM_LOAD16_WORD_SWAP("0283__ac__8117__1.05.ic88", 0x00000, 0x20000, CRC(c46dec54) SHA1(22ef9274f4ef85d446d56cce13a68273dc55f10a))

	// Force firmware to reinitialize nvram at first boot
	ROM_REGION(64, "rtc", ROMREGION_ERASEFF)
ROM_END

} // anonymous namespace

/*   YEAR   NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS           INIT         COMPANY  FULLNAME  FLAGS */
COMP(1989?, luna, 0,      0,      luna,    luna,  luna_68k_state, empty_init,  "Omron", "Luna",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
