// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * TODO (rx3230)
 *   - verify/complete address maps
 *   - keyboard controller and interrupts
 *   - isa slot and colour graphics board
 *   - idprom
 *
 *   Ref   Part                      Function
 *
 * R3030 system board (Assy. No. 03-00082- rev J):
 *
 *   N2B1  IDT 79R3000-25G           CPU
 *   L6B1  IDT 79R3010L-25OJ         FPU
 *   C3A2  50.0000 MHz crystal
 *   G2B8  MIPS 32-00039-000         RAMBO DMA/timer ASIC?
 *   H8B8  MIPS 32-00038-000         Cache control/write buffer ASIC?
 *   H8A3  MIPS 32-00038-000         Cache control/write buffer ASIC?
 *   E3H7  NCR 53C94                 SCSI controller
 *   C410  Intel N82072              Floppy controller
 *   B510  Z85C3010VSC               Serial controller
 *   C232  AMD AM7990JC/80           Ethernet controller
 *         AMD AM7992BDC             Ethernet serial interface
 *         M48T02                    RTC and NVRAM (labelled B6B93)
 *         MCS-48?                   Keyboard controller
 *   A7A7  DP8530V                   Clock generator
 *         AM27C1024                 IPL EPROM (128KiB, MSW)
 *         50-314-003
 *         3230 RIGHT
 *         CKSM / B098BB9
 *         TMS27C210                 IPL EPROM (128KiB, LSW)
 *         50-314-003
 *         3230 LEFT
 *         CKSM / 045A
 *
 *
 * Colour graphics board (assy. no. 03-00087- rev D):
 *
 *   UF4   Bt459KG110                256x24 Color RAMDAC
 *   UC4   Bt435KPJ                  Clock generator?
 *   OSC3  108.1800 MHz crystal      Pixel clock
 *
 *         ?                         Video RAM (total 1280KiB?)
 *   UJ11-UM11                       8 parts
 *   UJ13-UM13                       8 parts
 */
/*
 * Rx3230 WIP
 *
 *   status: boots RISC/os from network, panics during installation
 *
 * R3000 interrupts
 *  0 <- lance, scc, slot, keyboard
 *  1 <- scsi
 *  2 <- timer
 *  3 <- fpu
 *  4 <- fdc
 *  5 <- parity error
 *
 * Keyboard controller output port
 *  4: select 1M/4M SIMMs?
 *
 * rs3230 -window -nomax -ui_active -tty1 terminal -numscreens 2
 * PON failures
 *   instruction cache functionality (failed)
 *   instruction cache mats+ (skipped)
 *   data cache block refill (failed)
 *   instruction cache block refill (skipped)
 *   id prom (failed)
 *   tod - loop <1 second real time?
 *   color frame buffer (skipped)
 *   dma controller chip
 *   scsi controller chip
 *   tlb (skipped) - all pass except tlb_n (requires cpu data cache)
 *   exception (skipped)
 *   parity (failed)
 *   dma parity (skipped)
 *   at serial board (skipped)
 */

#include "emu.h"

#include "mips.h"

// processors and memory
#include "cpu/mips/mips1.h"
#include "cpu/nec/v5x.h"
#include "machine/ram.h"

// I/O devices (common)
#include "machine/at_keybc.h"
#include "machine/z80scc.h"
#include "machine/upd765.h"
#include "machine/am79c90.h"

// I/O devices (R3030)
#include "machine/timekpr.h"
#include "machine/ncr53c90.h"
#include "mips_rambo.h"

// busses and connectors
#include "machine/nscsi_bus.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"
#include "bus/nscsi/tape.h"

#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pc_kbd/keyboards.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/hlemouse.h"

// video and audio
#include "screen.h"
#include "video/bt45x.h"
#include "video/bt459.h"
#include "sound/spkrdev.h"
#include "speaker.h"

#include "imagedev/floppy.h"

#define LOG_MMU     (1U << 1)
#define LOG_IOCB    (1U << 2)

#define VERBOSE 0
#include "logmacro.h"

namespace {

class mips_r3030_state : public driver_device
{
public:
	mips_r3030_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram")
		, m_rom(*this, "r3030")
		, m_rambo(*this, "rambo")
		, m_scsibus(*this, "scsi")
		, m_scsi(*this, "scsi:7:ncr53c94")
		, m_net(*this, "net")
		, m_scc(*this, "scc")
		, m_tty(*this, "tty%u", 0U)
		, m_rtc(*this, "rtc")
		, m_fdc(*this, "fdc")
		, m_kbdc(*this, "kbdc")
		, m_kbd(*this, "kbd")
		, m_buzzer(*this, "buzzer")
		, m_screen(*this, "screen")
		, m_ramdac(*this, "ramdac")
		, m_vram(*this, "vram")
	{
	}

	// machine config
	void r3030(machine_config &config) ATTR_COLD;
	void rs3230(machine_config &config) ATTR_COLD;
	void rc3230(machine_config &config) ATTR_COLD;

	void r3030_init() ATTR_COLD;

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// address maps
	void r3030_map(address_map &map) ATTR_COLD;
	void rs3230_map(address_map &map) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

	u16 lance_r(offs_t offset, u16 mem_mask = 0xffff);
	void lance_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);

	template <u8 Source> void irq_w(int state);

private:
	// processors and memory
	required_device<r3000a_device> m_cpu;
	required_device<ram_device> m_ram;
	required_region_ptr<u32> m_rom;

	// i/o devices
	required_device<mips_rambo_device> m_rambo;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<ncr53c94_device> m_scsi;
	required_device<am7990_device> m_net;
	required_device<z80scc_device> m_scc;
	required_device_array<rs232_port_device, 2> m_tty;
	required_device<m48t02_device> m_rtc;
	required_device<i82072_device> m_fdc;
	required_device<at_keyboard_controller_device> m_kbdc;
	required_device<pc_kbdc_device> m_kbd;
	required_device<speaker_sound_device> m_buzzer;

	// optional colour video board
	optional_device<screen_device> m_screen;
	optional_device<bt459_device> m_ramdac;
	optional_device<ram_device> m_vram;

	enum int_reg_mask : u8
	{
		INT_SLOT = 0x01, // expansion slot
		INT_KBD  = 0x02, // keyboard controller
		INT_SCC  = 0x04, // serial controller
		INT_SCSI = 0x08, // scsi controller
		INT_NET  = 0x10, // ethernet controller
		INT_DRS  = 0x20, // data rate select
		INT_DSR  = 0x40, // data set ready
		INT_CEB  = 0x80, // modem call indicator

		INT_CLR  = 0xff,
	};

	enum gfx_reg_mask : u8
	{
		GFX_H_BLANK   = 0x10,
		GFX_V_BLANK   = 0x20,
		GFX_COLOR_RSV = 0xce, // reserved
	};

	u8 m_int_reg = 0;
	int m_int0_state = 0;
	int m_int1_state = 0;
};

void mips_r3030_state::r3030_map(address_map &map)
{
	map(0x00000000, 0x07ffffff).noprw(); // silence ram

	//map(0x10000000, 0x13ffffff); // restricted AT I/O space
	//map(0x14000000, 0x17ffffff); // restricted AT memory space

	map(0x16080004, 0x16080007).nopr(); // silence graphics register

	map(0x18000000, 0x1800003f).m(m_scsi, FUNC(ncr53c94_device::map)).umask32(0xff);
	map(0x19000000, 0x19000003).rw(m_kbdc, FUNC(at_keyboard_controller_device::data_r), FUNC(at_keyboard_controller_device::data_w)).umask32(0xff);
	map(0x19000004, 0x19000007).rw(m_kbdc, FUNC(at_keyboard_controller_device::status_r), FUNC(at_keyboard_controller_device::command_w)).umask32(0xff);
	map(0x19800000, 0x19800003).lr8(NAME([this]() { return m_int_reg; })).umask32(0xff);
	map(0x1a000000, 0x1a000007).rw(m_net, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w)).umask32(0xffff);
	map(0x1b000000, 0x1b00001f).rw(m_scc, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask32(0xff); // TODO: order?

	map(0x1c000000, 0x1c000fff).m(m_rambo, FUNC(mips_rambo_device::map));

	map(0x1d000000, 0x1d001fff).rw(m_rtc, FUNC(m48t02_device::read), FUNC(m48t02_device::write)).umask32(0xff);
	map(0x1e000000, 0x1e000007).m(m_fdc, FUNC(i82072_device::map)).umask32(0xff);
	//map(0x1e800000, 0x1e800003).umask32(0xff); // fdc tc

	map(0x1fc00000, 0x1fc3ffff).rom().region("r3030", 0);
	map(0x1ff00000, 0x1ff3ffff).rom().region("r3030", 0); // mirror
}

void mips_r3030_state::rs3230_map(address_map &map)
{
	r3030_map(map);

	map(0x10000000, 0x12ffffff).lrw32(
		NAME([this](offs_t offset)
		{
			u32 const ram_offset = ((offset >> 13) * 0x500) + ((offset & 0x1ff) << 2);

			u32 const data =
				u32(m_vram->read(ram_offset | 0)) << 24 |
				u32(m_vram->read(ram_offset | 1)) << 16 |
				u32(m_vram->read(ram_offset | 2)) << 8 |
				u32(m_vram->read(ram_offset | 3)) << 0;

			return data;
		}),
		NAME([this](offs_t offset, u32 data)
		{
			u32 const ram_offset = ((offset >> 13) * 0x500) + ((offset & 0x1ff) << 2);

			m_vram->write(ram_offset | 0, data >> 24);
			m_vram->write(ram_offset | 1, data >> 16);
			m_vram->write(ram_offset | 2, data >> 8);
			m_vram->write(ram_offset | 3, data >> 0);
		}));

	map(0x14000000, 0x14000003).rw(m_ramdac, FUNC(bt459_device::address_lo_r), FUNC(bt459_device::address_lo_w)).umask32(0xff);
	map(0x14080000, 0x14080003).rw(m_ramdac, FUNC(bt459_device::address_hi_r), FUNC(bt459_device::address_hi_w)).umask32(0xff);
	map(0x14100000, 0x14100003).rw(m_ramdac, FUNC(bt459_device::register_r), FUNC(bt459_device::register_w)).umask32(0xff);
	map(0x14180000, 0x14180003).rw(m_ramdac, FUNC(bt459_device::palette_r), FUNC(bt459_device::palette_w)).umask32(0xff);

	map(0x16080004, 0x16080007).lr8(NAME([this] ()
	{
		u8 const data = (m_screen->vblank() ? GFX_V_BLANK : 0) | (m_screen->hblank() ? GFX_H_BLANK : 0);

		return data;
	})).umask32(0xff); // also write 0

	//map(0x16000004, 0x16000007).w(); // write 0x00000001
	//map(0x16100000, 0x16100003).w(); // write 0xffffffff
}

void mips_r3030_state::machine_start()
{
	save_item(NAME(m_int_reg));
	save_item(NAME(m_int0_state));
	save_item(NAME(m_int1_state));
}

void mips_r3030_state::machine_reset()
{
	m_int_reg = INT_CLR;
	m_int0_state = 1;
	m_int1_state = 1;
}

void mips_r3030_state::r3030_init()
{
	// map the configured ram
	m_cpu->space(0).install_ram(0x00000000, m_ram->mask(), m_ram->pointer());

	/*
	 * HACK: the prom bootp code broadcasts to the network address (i.e. the
	 * host portion is "all zeroes"), instead of to the standard "all ones".
	 * This makes it very difficult to receive the bootp request in a host OS,
	 * so this patch changes the code to broadcast to the standard broadcast
	 * address instead.
	 *
	 * 0xbfc1f1b0: addu  r6,0,0
	 *             jal   $bfc0be10   # set host portion from r6
	 *
	 * This patch changes the first instruction to one which loads r6 with
	 * 0xffffffff, which is then or'd into the host part of the address, i.e.:
	 *
	 *             addiu r6,0,-$1
	 */
	m_rom[0x1f1b0 >> 2] = 0x2406ffff;
}

void mips_r3030_state::r3030(machine_config &config)
{
	R3000A(config, m_cpu, 50_MHz_XTAL / 2, 32768, 32768);
	m_cpu->set_addrmap(AS_PROGRAM, &mips_r3030_state::r3030_map);
	m_cpu->set_fpu(mips1_device_base::MIPS_R3010A);
	m_cpu->in_brcond<0>().set([]() { return 1; }); // writeback complete

	// 32 SIMM slots, 8-128MB memory, banks of 8 1MB or 4MB SIMMs
	RAM(config, m_ram);
	m_ram->set_default_size("32M");
	m_ram->set_extra_options("16M,64M,128M");
	m_ram->set_default_value(0);

	MIPS_RAMBO(config, m_rambo, 25_MHz_XTAL / 4);
	m_rambo->timer_out().set_inputline(m_cpu, INPUT_LINE_IRQ2);
	m_rambo->irq_out().set_inputline(m_cpu, INPUT_LINE_IRQ1);
	m_rambo->parity_out().set_inputline(m_cpu, INPUT_LINE_IRQ5);
	//m_rambo->buzzer_out().set(m_buzzer, FUNC(speaker_sound_device::level_w));
	m_rambo->set_ram(m_ram);
	m_rambo->dma_r<0>().set("scsi:7:ncr53c94", FUNC(ncr53c94_device::dma16_swap_r));
	m_rambo->dma_w<0>().set("scsi:7:ncr53c94", FUNC(ncr53c94_device::dma16_swap_w));

	// scsi bus and devices
	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsi:0", mips_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", mips_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", mips_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", mips_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", mips_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", mips_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", mips_scsi_devices, nullptr);

	// scsi host adapter
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr53c94", NCR53C94).clock(24_MHz_XTAL).machine_config(
		[this](device_t *device)
		{
			ncr53c94_device &adapter = downcast<ncr53c94_device &>(*device);

			adapter.set_busmd(ncr53c94_device::busmd_t::BUSMD_1);
			adapter.irq_handler_cb().set(*this, FUNC(mips_r3030_state::irq_w<INT_SCSI>)).invert();
			adapter.drq_handler_cb().set(m_rambo, FUNC(mips_rambo_device::drq_w<0>));
		});

	// ethernet
	AM7990(config, m_net);
	m_net->intr_out().set(FUNC(mips_r3030_state::irq_w<INT_NET>));
	m_net->dma_in().set(FUNC(mips_r3030_state::lance_r));
	m_net->dma_out().set(FUNC(mips_r3030_state::lance_w));

	SCC85C30(config, m_scc, 9.8304_MHz_XTAL); // TODO: clock working but unverified
	m_scc->out_int_callback().set(FUNC(mips_r3030_state::irq_w<INT_SCC>)).invert();

	// scc channel A (tty0)
	RS232_PORT(config, m_tty[0], default_rs232_devices, nullptr);
	m_tty[0]->cts_handler().set(m_scc, FUNC(z80scc_device::ctsa_w));
	m_tty[0]->dcd_handler().set(m_scc, FUNC(z80scc_device::dcda_w));
	m_tty[0]->rxd_handler().set(m_scc, FUNC(z80scc_device::rxa_w));
	m_scc->out_rtsa_callback().set(m_tty[0], FUNC(rs232_port_device::write_rts));
	m_scc->out_txda_callback().set(m_tty[0], FUNC(rs232_port_device::write_txd));

	// scc channel B (tty1)
	RS232_PORT(config, m_tty[1], default_rs232_devices, nullptr);
	m_tty[1]->cts_handler().set(m_scc, FUNC(z80scc_device::ctsb_w));
	m_tty[1]->dcd_handler().set(m_scc, FUNC(z80scc_device::dcdb_w));
	m_tty[1]->rxd_handler().set(m_scc, FUNC(z80scc_device::rxb_w));
	m_scc->out_rtsb_callback().set(m_tty[1], FUNC(rs232_port_device::write_rts));
	m_scc->out_txdb_callback().set(m_tty[1], FUNC(rs232_port_device::write_txd));

	M48T02(config, m_rtc);

	// floppy controller and drive
	I82072(config, m_fdc, 16_MHz_XTAL);
	m_fdc->intrq_wr_callback().set_inputline(m_cpu, INPUT_LINE_IRQ4);
	//m_fdc->drq_wr_callback().set();
	FLOPPY_CONNECTOR(config, "fdc:0", "35hd", FLOPPY_35_HD, true, floppy_image_device::default_pc_floppy_formats).enable_sound(false);

	// keyboard connector
	PC_KBDC(config, m_kbd, pc_at_keyboards, nullptr);
	m_kbd->out_clock_cb().set(m_kbdc, FUNC(at_keyboard_controller_device::kbd_clk_w));
	m_kbd->out_data_cb().set(m_kbdc, FUNC(at_keyboard_controller_device::kbd_data_w));

	// keyboard controller
	AT_KEYBOARD_CONTROLLER(config, m_kbdc, 12_MHz_XTAL); // TODO: confirm
	m_kbdc->kbd_clk().set(m_kbd, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_kbdc->kbd_data().set(m_kbd, FUNC(pc_kbdc_device::data_write_from_mb));
	//m_kbdc->kbd_irq().set(FUNC(mips_r3030_state::irq_w<INT_KBD>));

	// buzzer
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_buzzer);
	m_buzzer->add_route(ALL_OUTPUTS, "mono", 0.50);

	// motherboard monochrome video (1152x900 @ 72Hz)
	u32 const pixclock = 74'649'600;

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(pixclock, 1152, 0, 1152, 900, 0, 900);
	m_screen->set_screen_update(m_rambo.finder_tag(), FUNC(mips_rambo_device::screen_update));

	// TODO: slot - motherboard can accept either the colour graphics board, or
	// a riser which presents an ISA 16-bit slot.
}

void mips_r3030_state::rc3230(machine_config &config)
{
	r3030(config);

	m_cpu->set_addrmap(AS_PROGRAM, &mips_r3030_state::r3030_map);

	m_kbd->set_default_option(STR_KBD_MICROSOFT_NATURAL);
	//m_tty[1]->set_default_option("terminal");
}

void mips_r3030_state::rs3230(machine_config &config)
{
	r3030(config);

	m_kbd->set_default_option(STR_KBD_MICROSOFT_NATURAL);

	// FIXME: colour video board disabled for now
	if (false)
	{
		m_cpu->set_addrmap(AS_PROGRAM, &mips_r3030_state::rs3230_map);

		// video hardware (1280x1024x8bpp @ 60Hz), 16 parts vram
		u32 const pixclock = 108'180'000;

		// timing from VESA 1280x1024 @ 60Hz
		m_screen->set_raw(pixclock, 1688, 248, 1528, 1066, 38, 1062);
		m_screen->set_screen_update(FUNC(mips_r3030_state::screen_update));
		//m_screen->screen_vblank().set_inputline(m_cpu, INPUT_LINE_IRQ5);

		BT459(config, m_ramdac, pixclock);

		RAM(config, m_vram);
		m_vram->set_default_size("2M");
		m_vram->set_default_value(0);
	}
}

template <u8 Source> void mips_r3030_state::irq_w(int state)
{
	if (state)
		m_int_reg |= Source;
	else
		m_int_reg &= ~Source;

	switch (Source)
	{
	case INT_SLOT:
	case INT_KBD:
	case INT_SCC:
	case INT_NET:
		if (m_int0_state != state)
		{
			m_int0_state = state;
			m_cpu->set_input_line(INPUT_LINE_IRQ0, !state);
		}
		break;

	case INT_SCSI:
		if (m_int1_state != state)
		{
			m_int1_state = state;
			m_cpu->set_input_line(INPUT_LINE_IRQ1, !state);
		}
		break;
	}
}

u32 mips_r3030_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	m_ramdac->screen_update(screen, bitmap, cliprect, m_vram->pointer());

	return 0;
}

u16 mips_r3030_state::lance_r(offs_t offset, u16 mem_mask)
{
	u16 const data =
		(m_ram->read(BYTE4_XOR_BE(offset + 0)) << 8) |
		m_ram->read(BYTE4_XOR_BE(offset + 1));

	return data;
}

void mips_r3030_state::lance_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_ram->write(BYTE4_XOR_BE(offset + 1), data);

	if (ACCESSING_BITS_8_15)
		m_ram->write(BYTE4_XOR_BE(offset + 0), data >> 8);
}

ROM_START(r3030)
	ROM_REGION32_BE(0x40000, "r3030", 0)
	ROM_SYSTEM_BIOS(0, "v5.40", "R3030 v5.40, Jun 1990")
	ROMX_LOAD("50-314-003__3230_left.bin",  0x00002, 0x20000, CRC(77ce42c9) SHA1(b2d5e5a386ed0ff840646647ba90b3c36732a7fe), ROM_BIOS(0) | ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2))
	ROMX_LOAD("50-314-003__3230_right.bin", 0x00000, 0x20000, CRC(5bc1ce2f) SHA1(38661234bf40b76395393459de49e48619b2b454), ROM_BIOS(0) | ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2))

	ROM_SYSTEM_BIOS(1, "v5.42", "R3030 v5.42, Mar 1991")
	ROMX_LOAD("unknown.bin", 0x00002, 0x20000, NO_DUMP, ROM_BIOS(1) | ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2))
	ROMX_LOAD("unknown.bin", 0x00000, 0x20000, NO_DUMP, ROM_BIOS(1) | ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2))

	//ROM_REGION(0x800, "i8042", 0)
	//ROM_LOAD("unknown.bin", 0x000, 0x800, NO_DUMP)

	//ROM_REGION(0x800, "rtc", 0)
	//ROM_LOAD("m48t02.bin", 0x000, 0x800, NO_DUMP)
ROM_END
#define rom_rc3230 rom_r3030
#define rom_rs3230 rom_r3030

} // anonymous namespace

/*   YEAR   NAME       PARENT  COMPAT  MACHINE    INPUT  CLASS             INIT         COMPANY  FULLNAME       FLAGS */
COMP(1990,  rc3230,    0,      0,      rc3230,    0,     mips_r3030_state, r3030_init, "MIPS",  "RC3230",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP(1990,  rs3230,    0,      0,      rs3230,    0,     mips_r3030_state, r3030_init, "MIPS",  "Magnum 3000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
