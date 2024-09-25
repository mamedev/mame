// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An emulation of systems designed and manufactured by MIPS Computer Systems,
 * all of which use MIPS R2000, R3000 or R6000 CPUs, and run the RISC/os
 * operating system.
 *
 * This driver is intended to eventually cover the following models:
 *
 *   Model       Board    CPU      Clock  Slots    Disk  Package       Other
 *   M/500       R2300    R2000     5MHz  VME      ESDI
 *   M/800       R2600    R2000     8MHz  VME      ESDI
 *   M/1000      R2800    R2000    10MHz  VME      ESDI
 *   M/120-3     R2400    R2000  12.5MHz  PC-AT    SCSI  Deskside      aka Intrepid
 *   M/120-5     R2400    R2000    16MHz  PC-AT    SCSI  Deskside
 *   M/180       R2400
 *   M/2000-6    R3200    R3000    20MHz  VMEx13   SMD   Rack Cabinet
 *   M/2000-8    R3200    R3000    25MHz  VMEx13   SMD   Rack Cabinet
 *   M/2000-?    RB3125   R3000    33MHz
 *   RC2030      I2000    R2000    16MHz           SCSI  Desktop       aka M/12, Jupiter
 *   RS2030      I2000    R2000    16MHz           SCSI  Desktop       aka M/12, Jupiter
 *   RC3230      R3030    R3000    25MHz  PC-ATx1  SCSI  Desktop       aka M/20, Pizazz
 *   RS3230      R3030    R3000    25MHz  PC-ATx1  SCSI  Desktop       aka M/20, Pizazz, Magnum 3000
 *   RC3240               R3000    25MHz  PC-ATx4  SCSI  Deskside      M/120 with CPU-board upgrade
 *   RC3330               R3000    33MHz  PC-AT    SCSI  Desktop
 *   RS3330               R3000    33MHz  PC-AT    SCSI  Desktop
 *   RC3260               R3000    25MHz  VMEx7    SCSI  Pedestal
 *   RC3360      RB3133   R3000    33MHz  VME      SCSI  Pedestal
 *   RC3370      RB3133
 *   RC6260      R6300    R6000    66MHz  VME      SCSI  Pedestal
 *   RC6280      R6300    R6000    66MHz  VMEx6    SMD   Data Center
 *   RC6380-100           R6000x1  66MHz  VME      SMD   Data Center
 *   RC6380-200           R6000x2  66MHz  VME      SMD   Data Center
 *   RC6380-400           R6000x4  66MHz  VME      SMD   Data Center
 *
 * Sources:
 *
 *   http://www.umips.net/
 *   http://www.geekdot.com/the-mips-rs2030/
 *   http://www.jp.netbsd.org/ports/mipsco/models.html
 *   http://www.prumpleffer.de/~miod/machineroom/machines/mips/magnum/index.html
 *   https://web.archive.org/web/20140518203135/http://no-l.org/pages/riscos.html
 *
 * TODO (rx3230)
 *   - verify/complete address maps
 *   - keyboard controller and interrupts
 *   - isa slot and colour graphics board
 *   - idprom
 *
 *   Ref   Part                      Function
 *
 * I2000 system board:
 *
 *         MIPS R2000A               Main CPU
 *         PACEMIPS PR2010A          Floating point unit
 *         33.3330 MHz crystal       CPU clock
 *         DL15CC200
 *         DDU7F-25                  Delay line (10 taps @ 2.5ns per tap)
 *         VLSI VL85C30-08PC         Serial port controller
 *         ST Z8038AB1 FIO           Parallel port controller
 *         1.8432 MHz crystal        Serial clock
 *         NEC D70216L-10            I/O processor
 *         Adaptec AIC-6250DL        SCSI controller
 *         AMD AM7990DCB/80          Ethernet controller
 *         20 MHz crystal
 *         2VP5U9                    DC/DC converter (Ethernet transceiver power?)
 *         WDC WD37C65BJW            Floppy controller
 *         Intel P8742AH             Keyboard controller
 *         Dallas DS1287             RTC and NVRAM
 *         buzzer                    Connected to keyboard controller?
 *
 *         P4C164-25PC               Cache RAM? (8Kx8, total 112KiB)
 *   U?-U?                           14 parts
 *
 *         27C512 64K EPROM          V50 IPL (64Kx8, total 256KiB)
 *   U139-U142                       4 parts
 *
 *         M5M4464                   V50 RAM (64Kx4, total 128KiB)
 *   U164-U167                       4 parts
 *
 *
 * Jupiter video board:
 *
 *         Idt75C458                 256x24 Color RAMDAC
 *         Bt438KC                   Clock generator
 *         108.180 MHz crystal
 *
 *         D41264V-12                Video RAM (64Kx4, total 1280KiB)
 *   U?-U?                           40 parts
 *
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
 * Rx2030 WIP
 *
 *   - keyboard reset failure
 *
 * V50 internal peripherals:
 * base = 0xfe00
 * serial (sula): fe00
 *  timer (tula): fe08
 *    int (iula): fe10
 *    dma (dula): fe20
 *
 * V50 IPL diagnostic routines
 *   NVRAM       f8dc4
 *   Ethernet ID f8b58
 *   Parallel    f8c5c
 *   Keyboard    f4cbc
 *   SCC         ec0fc
 *   Floppy      ee3ea
 *   SCSI        f426e
 *   LANCE       f8f68
 *
 * V50 interrupts:
 *   intp1 <- SCU
 *   intp2 <- CPU interface
 *   intp3 <- SCC
 *   intp4 <- FIO
 *   intp5 <- LANCE
 *   intp6 <- floppy?
 *   intp7 <- SCSI
 *
 * R2000 interrupts:
 *   int0 <- ?
 *   int1 <- iop keyboard
 *   int2 <- ?
 *   int4 <- iop clock
 *   int5 <- vblank
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

// processors and memory
#include "cpu/mips/mips1.h"
#include "cpu/nec/v5x.h"
#include "machine/ram.h"

// i/o devices (common)
#include "machine/at_keybc.h"
#include "machine/z80scc.h"
#include "machine/upd765.h"
#include "machine/am79c90.h"

// i/o devices (rx2030)
#include "machine/mc146818.h"
#include "machine/z8038.h"
#include "machine/aic6250.h"

// i/o devices (rx3230)
#include "machine/timekpr.h"
#include "machine/ncr53c90.h"
#include "mips_rambo.h"

// busses and connectors
#include "machine/nscsi_bus.h"
#include "bus/nscsi/cd.h"
#include "bus/nscsi/hd.h"

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

class rx2030_state : public driver_device
{
public:
	rx2030_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_iop(*this, "iop")
		, m_ram(*this, "ram")
		, m_rom(*this, "rx2030")
		, m_rtc(*this, "rtc")
		, m_fio(*this, "fio")
		, m_kbdc(*this, "kbdc")
		, m_kbd(*this, "kbd")
		, m_scc(*this, "scc")
		, m_tty(*this, "tty%u", 0U)
		, m_fdc(*this, "fdc")
		, m_scsibus(*this, "scsi")
		, m_scsi(*this, "scsi:7:aic6250")
		, m_net(*this, "net")
		, m_buzzer(*this, "buzzer")
		, m_screen(*this, "screen")
		, m_ramdac(*this, "ramdac")
		, m_vram(*this, "vram")
	{
	}

	// machine config
	void rx2030(machine_config &config);
	void rs2030(machine_config &config);
	void rc2030(machine_config &config);

	void rx2030_init();

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// address maps
	void rx2030_map(address_map &map) ATTR_COLD;
	void rs2030_map(address_map &map) ATTR_COLD;

	void iop_program_map(address_map &map) ATTR_COLD;
	void iop_io_map(address_map &map) ATTR_COLD;

	u16 mmu_r(offs_t offset, u16 mem_mask = 0xffff);
	void mmu_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);

	u16 lance_r(offs_t offset, u16 mem_mask = 0xffff);
	void lance_w(offs_t offset, u16 data, u16 mem_mask = 0xffff);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect);

private:
	enum iop_interface_status_mask : u8
	{
		FPU_ABSENT = 0x01, // clear if FPU installed
		DBG_ABSENT = 0x02, // clear if debug board installed
		IOP_IRQ    = 0x04, // set when CPU interrupts IOP
		VID_ABSENT = 0x10, // clear if video board installed
		IOP_IACK   = 0x40, // set when IOP acknowledges CPU interrupt
		IOP_NERR   = 0x80, // clear when IOP receives parity error
	};

	// processors and memory
	required_device<r2000a_device> m_cpu;
	required_device<v50_device> m_iop;
	required_device<ram_device> m_ram;
	required_region_ptr<u16> m_rom;

	// i/o devices
	required_device<mc146818_device> m_rtc;
	required_device<z8038_device> m_fio;
	required_device<at_keyboard_controller_device> m_kbdc;
	required_device<pc_kbdc_device> m_kbd;
	required_device<z80scc_device> m_scc;
	required_device_array<rs232_port_device, 2> m_tty;
	required_device<wd37c65c_device> m_fdc;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<aic6250_device> m_scsi;
	required_device<am7990_device> m_net;
	required_device<speaker_sound_device> m_buzzer;

	// optional video board
	optional_device<screen_device> m_screen;
	optional_device<bt458_device> m_ramdac;
	optional_shared_ptr<u32> m_vram;

	// machine state
	u16 m_mmu[32]{};

	u8 m_iop_interface = 0;
};

class rx3230_state : public driver_device
{
public:
	rx3230_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_ram(*this, "ram")
		, m_rom(*this, "rx3230")
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
	void rx3230(machine_config &config);
	void rs3230(machine_config &config);
	void rc3230(machine_config &config);

	void rx3230_init();

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// address maps
	void rx3230_map(address_map &map) ATTR_COLD;
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

void rx2030_state::machine_start()
{
	save_item(NAME(m_mmu));
	save_item(NAME(m_iop_interface));
}

void rx2030_state::machine_reset()
{
	m_cpu->set_input_line(INPUT_LINE_RESET, 1);
}

void rx2030_state::rx2030_init()
{
	m_iop_interface = IOP_NERR | DBG_ABSENT;

	// map the configured ram and vram
	m_cpu->space(0).install_ram(0x00000000, m_ram->mask(), m_ram->pointer());

	// page zero of prom space is mapped to ram page zero
	m_cpu->space(0).install_rom(0x1fc00000, 0x1fc00fff, m_ram->pointer());

	if (!m_vram)
		m_iop_interface |= VID_ABSENT;

	/*
	 * HACK: the prom bfs code broadcasts to the network address (i.e. the
	 * host portion is "all zeroes"), instead of to the standard "all ones".
	 * This makes it very difficult to receive the bfs request in a modern host
	 * OS; the patch changes the code to broadcast to the standard broadcast
	 * address instead.
	 *
	 * Technique is identical to that described for the rx3230 below.
	 */
	switch (system_bios())
	{
	case 1:
		m_rom[0x1ab68 >> 1] = 0x0624;
		m_rom[0x1ab6a >> 1] = 0xffff;
		break;

	case 2:
		m_rom[0x1a7f8 >> 1] = 0x0624;
		m_rom[0x1a7fa >> 1] = 0xffff;
		break;
	}
}

u16 rx2030_state::mmu_r(offs_t offset, u16 mem_mask)
{
	offs_t const address = (m_mmu[(offset >> 11) & 0x1f] << 12) | ((offset << 1) & 0xfff);

	u16 const data = (m_ram->read(BYTE4_XOR_BE(address + 1)) << 8) | m_ram->read(BYTE4_XOR_BE(address + 0));

	LOGMASKED(LOG_MMU, "mmu_r offset 0x%06x reg %d page 0x%04x mapped 0x%06x data 0x%04x\n",
		(offset << 1), (offset >> 11) & 0x1f, m_mmu[(offset >> 11) & 0x1f], address, data);

	return data;
}

void rx2030_state::mmu_w(offs_t offset, u16 data, u16 mem_mask)
{
	offs_t const address = (m_mmu[(offset >> 11) & 0x1f] << 12) | ((offset << 1) & 0xfff);

	LOGMASKED(LOG_MMU, "mmu_w offset 0x%06x reg %d page 0x%04x mapped 0x%06x data 0x%04x (%s)\n",
		(offset << 1), (offset >> 11) & 0x1f, m_mmu[(offset >> 11) & 0x1f], address, data, machine().describe_context());

	if (ACCESSING_BITS_0_7)
		m_ram->write(BYTE4_XOR_BE(address + 0), data);

	if (ACCESSING_BITS_8_15)
		m_ram->write(BYTE4_XOR_BE(address + 1), data >> 8);
}

void rx2030_state::iop_program_map(address_map &map)
{
	// 00000:1ffff  128k ram (64kx4, 4 parts)
	// 20000:3ffff  128k shared (32x4k mapped pages, bits 0-11 offset, bits 12-16 mmu register)
	// 80000:fffff  512k eprom (256k x 2 copies)

	map(0x00000, 0x1ffff).ram();
	map(0x20000, 0x3ffff).rw(FUNC(rx2030_state::mmu_r), FUNC(rx2030_state::mmu_w));

	map(0x80000, 0xbffff).rom().region("rx2030", 0).mirror(0x40000);
}

void rx2030_state::iop_io_map(address_map &map)
{
	map(0x0000, 0x003f).lrw16(
		NAME([this] (offs_t offset, u16 mem_mask) { return m_mmu[offset]; }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) { m_mmu[offset] = data; }));

	map(0x0040, 0x0043).m(m_fdc, FUNC(wd37c65c_device::map)).umask16(0xff);
	map(0x0044, 0x0045).w(m_fdc, FUNC(wd37c65c_device::dor_w)).umask16(0xff);
	map(0x0048, 0x0049).w(m_fdc, FUNC(wd37c65c_device::ccr_w)).umask16(0xff);
	//map(0x004c, 0x004d).r(m_fdc, FUNC(?)).umask16(0xff);

	map(0x0080, 0x0083).rw(m_scsi, FUNC(aic6250_device::read), FUNC(aic6250_device::write)).umask16(0xff);

	/*
	 * HACK: Substitute the keyboard "set defaults" command for the "reset"
	 * command to avoid an issue where the keyboard is still busy performing
	 * the reset and does not accept commands being sent to it to change the
	 * scan code set. Possibly caused by imperfect V50 timing and/or memory
	 * wait states that make the IOP code execute more slowly than emulated.
	 */
	map(0x00c0, 0x00c1).lrw8(NAME([this] () { return m_kbdc->data_r(); }), NAME([this] (u8 data) { m_kbdc->data_w(data == 0xff ? 0xf6 : data); })).umask16(0xff);
	map(0x00c4, 0x00c5).rw(m_kbdc, FUNC(at_keyboard_controller_device::status_r), FUNC(at_keyboard_controller_device::command_w)).umask16(0xff);

	map(0x0100, 0x0107).rw(m_scc, FUNC(z80scc_device::ab_dc_r), FUNC(z80scc_device::ab_dc_w)).umask16(0xff);

	map(0x0140, 0x0143).rw(m_net, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w));

	map(0x0180, 0x018b).lr8(
			[] (offs_t offset)
			{
				// Ethernet MAC address (LSB first)
				static u8 const mac[] = { 0x00, 0x00, 0x6b, 0x12, 0x34, 0x56 };

				return mac[offset];
			}, "mac_r").umask16(0xff);

	// iop tests bits 0x04, 0x10 and 0x20
	map(0x01c0, 0x01c1).lr8(NAME([this] () { return m_iop_interface; })); // maybe?

	map(0x0200, 0x0201).rw(m_fio, FUNC(z8038_device::fifo_r<1>), FUNC(z8038_device::fifo_w<1>)).umask16(0xff);
	map(0x0202, 0x0203).rw(m_fio, FUNC(z8038_device::reg_r<1>), FUNC(z8038_device::reg_w<1>)).umask16(0xff);

	map(0x0240, 0x0241).w(m_rtc, FUNC(mc146818_device::address_w)).umask16(0xff00);
	map(0x0280, 0x0281).rw(m_rtc, FUNC(mc146818_device::data_r), FUNC(mc146818_device::data_w)).umask16(0xff00);

	map(0x02c0, 0x2c1).lw8([this](u8 data)
	{
		switch (data)
		{
		case 0: LOG("cpu interrupt 0 asserted\n"); m_cpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE); break;
		case 1: LOG("cpu interrupt 1 asserted\n"); m_cpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE); break;
		case 2: LOG("cpu interrupt 2 asserted\n"); m_cpu->set_input_line(INPUT_LINE_IRQ2, ASSERT_LINE); break;
		case 3: LOG("cpu interrupt 4 asserted\n"); m_cpu->set_input_line(INPUT_LINE_IRQ4, ASSERT_LINE); break;

		case 4:
			if (m_cpu->suspended())
			{
				LOG("cpu reset released\n");
				m_cpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
			}
			else
				m_iop->set_input_line(INPUT_LINE_IRQ2, CLEAR_LINE);
			m_iop_interface |= IOP_IACK;
			m_iop_interface &= ~IOP_IRQ;
			break;

		default:
			LOG("cpu interface command 0x%02x\n", data);
			break;

		//case 5: break; // unknown
		//case 6:
		//case 7:
			// something to do with shared memory access?
			//break;
		}
	}, "cpu_interface_w").umask16(0xff);

	map(0x0380, 0x0381).lw8(NAME([this](u8 data) { logerror("led_w 0x%02x\n", data); })).umask16(0xff00);
}

void rx2030_state::rx2030_map(address_map &map)
{
	map(0x02000000, 0x02000003).lrw8(
		NAME([this]() { return m_iop_interface; }),
		[this](u8 data)
		{
			switch (data)
			{
			case 0: LOG("cpu interrupt 0 cleared\n"); m_cpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE); break;
			case 1: LOG("cpu interrupt 1 cleared\n"); m_cpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE); break;
			case 2: LOG("cpu interrupt 2 cleared\n"); m_cpu->set_input_line(INPUT_LINE_IRQ2, CLEAR_LINE); break;
			case 3: LOG("cpu interrupt 4 cleared\n"); m_cpu->set_input_line(INPUT_LINE_IRQ4, CLEAR_LINE); break;
				break;

			case 4:
				if (VERBOSE & LOG_IOCB)
				{
					static char const *const iop_commands[] =
					{
						"IOP",     "UART0", "UART1", "NVRAM", "LED",   "CLOCK",  "TOD",   "SCSI0",
						"SCSI1",   "SCSI2", "SCSI3", "SCSI4", "SCSI5", "SCSI6",  "SCSI7", "FLOPPY0",
						"FLOPPY1", "LANCE", "PP",    "KYBD",  "MOUSE", "BUZZER", "UNK22", "UNK23"
					};
					static char const *const iop_lance[] =
					{
						"", "PROBE", "INIT", "STOP", "STRT", "RECV", "XMIT", "XMIT_DONE",
						"STAT", "INIT_DONE", "RESET",  "DBG_ON",  "DBG_OFF", "MISS"
					};

					for (int iocb = 0; iocb < 24; iocb++)
					{
						// check if command semaphore set
						if (m_ram->read(0x1000 + iocb * 16 + 10) || m_ram->read(0x1000 + iocb * 16 + 11))
						{
							u32 const iocb_cmdparam = m_ram->read(0x1000 + iocb * 16 + 0)
								| (m_ram->read(0x1000 + iocb * 16 + 1) << 8)
								| (m_ram->read(0x1000 + iocb * 16 + 2) << 16)
								| (m_ram->read(0x1000 + iocb * 16 + 3) << 24);

							u16 const iop_cmd = m_ram->read(0x1000 + iocb_cmdparam + 2) | (m_ram->read(0x1000 + iocb_cmdparam + 3) << 8);

							switch (iocb)
							{
							case 5: // clock
								LOGMASKED(LOG_IOCB, "iocb %s command 0x%04x (%s)\n",
									iop_commands[iocb], m_ram->read(0x1000 + iocb_cmdparam + 6) | (m_ram->read(0x1000 + iocb_cmdparam + 7) << 8),
									machine().describe_context());
								break;

							case 17: // lance
								LOGMASKED(LOG_IOCB, "iocb %s command %s (%s)\n",
									iop_commands[iocb], iop_lance[iocb_cmdparam],
									machine().describe_context());
								break;

							case 19: // keyboard
								LOGMASKED(LOG_IOCB, "iocb %s command 0x%04x data 0x%02x (%s)\n",
									iop_commands[iocb], iop_cmd,
									m_ram->read(0x1000 + iocb_cmdparam + 7),
									machine().describe_context());
								break;

							default:
								LOGMASKED(LOG_IOCB, "iocb %s command 0x%04x (%s)\n",
									iop_commands[iocb], iop_cmd,
									machine().describe_context());
								break;
							}
						}
					}
				}

				// interrupt the iop
				m_iop_interface &= ~IOP_IACK;
				m_iop_interface |= IOP_IRQ;
				m_iop->set_input_line(INPUT_LINE_IRQ2, ASSERT_LINE);
				break;

			case 6: LOG("led on\n"); break;
			case 7: LOG("led off\n"); break;

			default:
				LOG("iop interface command 0x%02x (%s)\n", data, machine().describe_context());
				break;
			}
		}, "iop_interface_w"
	).umask32(0xff);
}

void rx2030_state::rs2030_map(address_map &map)
{
	rx2030_map(map);

	// video hardware
	map(0x01000000, 0x011fffff).ram().share("vram");
	map(0x01ffff00, 0x01ffffff).m(m_ramdac, FUNC(bt458_device::map)).umask32(0xff);

	//map(0x01ff1000, 0x01ff1001).w() // graphics register?
	//map(0x01ff0080, 0x01ff0081).w() // graphics register?
}

u16 rx2030_state::lance_r(offs_t offset, u16 mem_mask)
{
	u16 const data =
		(m_ram->read(BYTE4_XOR_BE(offset + 1)) << 8) |
		m_ram->read(BYTE4_XOR_BE(offset + 0));

	return data;
}

void rx2030_state::lance_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_ram->write(BYTE4_XOR_BE(offset + 0), data);

	if (ACCESSING_BITS_8_15)
		m_ram->write(BYTE4_XOR_BE(offset + 1), data >> 8);
}

static void mips_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("cdrom", NSCSI_CDROM);
}

static void mips_rs232_devices(device_slot_interface &device)
{
	default_rs232_devices(device);

	device.option_add("mouse", MSYSTEMS_HLE_SERIAL_MOUSE);
}

void rx2030_state::rx2030(machine_config &config)
{
	R2000A(config, m_cpu, 33.333_MHz_XTAL / 2, 32768, 32768);
	m_cpu->set_fpu(mips1_device_base::MIPS_R2010A);
	m_cpu->in_brcond<0>().set([]() { return 1; }); // writeback complete

	V50(config, m_iop, 20_MHz_XTAL);
	m_iop->set_addrmap(AS_PROGRAM, &rx2030_state::iop_program_map);
	m_iop->set_addrmap(AS_IO, &rx2030_state::iop_io_map);
	m_iop->tout2_cb().set(m_buzzer, FUNC(speaker_sound_device::level_w));

	// general dma configuration
	m_iop->out_hreq_cb().set(m_iop, FUNC(v50_device::hack_w));
	m_iop->in_mem16r_cb().set(FUNC(rx2030_state::mmu_r));
	m_iop->out_mem16w_cb().set(FUNC(rx2030_state::mmu_w));

	// dma channel 1: scsi
	m_iop->in_io16r_cb<1>().set(m_scsi, FUNC(aic6250_device::dma16_r));
	m_iop->out_io16w_cb<1>().set(m_scsi, FUNC(aic6250_device::dma16_w));
	m_iop->out_dack_cb<1>().set(m_scsi, FUNC(aic6250_device::back_w));

	RAM(config, m_ram);
	m_ram->set_default_size("16M");
	m_ram->set_extra_options("4M,8M,12M");
	m_ram->set_default_value(0);

	// rtc and nvram
	MC146818(config, m_rtc, 32.768_kHz_XTAL);

	// parallel port
	Z8038(config, m_fio, 0);
	m_fio->out_int_cb<1>().set_inputline(m_iop, INPUT_LINE_IRQ4);

	// keyboard connector
	PC_KBDC(config, m_kbd, pc_at_keyboards, nullptr);
	m_kbd->out_clock_cb().set(m_kbdc, FUNC(at_keyboard_controller_device::kbd_clk_w));
	m_kbd->out_data_cb().set(m_kbdc, FUNC(at_keyboard_controller_device::kbd_data_w));

	// keyboard controller
	AT_KEYBOARD_CONTROLLER(config, m_kbdc, 12_MHz_XTAL);
	//m_kbdc->hot_res().set_inputline(m_maincpu, INPUT_LINE_RESET);
	m_kbdc->kbd_clk().set(m_kbd, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_kbdc->kbd_data().set(m_kbd, FUNC(pc_kbdc_device::data_write_from_mb));
	m_kbdc->set_default_bios_tag("award15");

	SCC85C30(config, m_scc, 1.8432_MHz_XTAL);
	m_scc->configure_channels(m_scc->clock(), m_scc->clock(), m_scc->clock(), m_scc->clock());
	m_scc->out_int_callback().set_inputline(m_iop, INPUT_LINE_IRQ3);

	// scc channel A (tty0)
	RS232_PORT(config, m_tty[0], mips_rs232_devices, nullptr);
	m_tty[0]->cts_handler().set(m_scc, FUNC(z80scc_device::ctsa_w));
	m_tty[0]->dcd_handler().set(m_scc, FUNC(z80scc_device::dcda_w));
	m_tty[0]->rxd_handler().set(m_scc, FUNC(z80scc_device::rxa_w));
	m_scc->out_rtsa_callback().set(m_tty[0], FUNC(rs232_port_device::write_rts));
	m_scc->out_txda_callback().set(m_tty[0], FUNC(rs232_port_device::write_txd));

	// scc channel B (tty1)
	RS232_PORT(config, m_tty[1], mips_rs232_devices, nullptr);
	m_tty[1]->cts_handler().set(m_scc, FUNC(z80scc_device::ctsb_w));
	m_tty[1]->dcd_handler().set(m_scc, FUNC(z80scc_device::dcdb_w));
	m_tty[1]->rxd_handler().set(m_scc, FUNC(z80scc_device::rxb_w));
	m_scc->out_rtsb_callback().set(m_tty[1], FUNC(rs232_port_device::write_rts));
	m_scc->out_txdb_callback().set(m_tty[1], FUNC(rs232_port_device::write_txd));

	// floppy controller and drive
	WD37C65C(config, m_fdc, 16_MHz_XTAL);
	m_fdc->intrq_wr_callback().set_inputline(m_iop, INPUT_LINE_IRQ6);
	//m_fdc->drq_wr_callback().set();
	FLOPPY_CONNECTOR(config, "fdc:0", "35hd", FLOPPY_35_HD, true, floppy_image_device::default_pc_floppy_formats).enable_sound(false);

	// scsi bus and devices
	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsi:0", mips_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", mips_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", mips_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", mips_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", mips_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", mips_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", mips_scsi_devices, nullptr);

	// scsi host adapter (clock assumed)
	NSCSI_CONNECTOR(config, "scsi:7").option_set("aic6250", AIC6250).clock(10_MHz_XTAL).machine_config(
		[this](device_t *device)
		{
			aic6250_device &adapter = downcast<aic6250_device &>(*device);

			adapter.int_cb().set_inputline(m_iop, INPUT_LINE_IRQ7).invert();
			adapter.breq_cb().set(m_iop, FUNC(v50_device::dreq_w<1>));
		});

	// ethernet
	AM7990(config, m_net);
	m_net->intr_out().set_inputline(m_iop, INPUT_LINE_IRQ5).invert();
	m_net->dma_in().set(FUNC(rx2030_state::lance_r));
	m_net->dma_out().set(FUNC(rx2030_state::lance_w));

	// buzzer
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_buzzer);
	m_buzzer->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void rx2030_state::rc2030(machine_config &config)
{
	rx2030(config);

	m_cpu->set_addrmap(AS_PROGRAM, &rx2030_state::rx2030_map);

	m_tty[1]->set_default_option("terminal");
}

void rx2030_state::rs2030(machine_config &config)
{
	rx2030(config);

	m_cpu->set_addrmap(AS_PROGRAM, &rx2030_state::rs2030_map);

	m_kbd->set_default_option(STR_KBD_MICROSOFT_NATURAL);
	m_tty[0]->set_default_option("mouse");

	// video hardware (1280x1024x8bpp @ 60Hz), 40 parts vram
	u32 const pixclock = 108'189'000;

	// timing from VESA 1280x1024 @ 60Hz
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(pixclock, 1688, 248, 1528, 1066, 38, 1062);
	m_screen->set_screen_update(FUNC(rx2030_state::screen_update));
	m_screen->screen_vblank().set_inputline(m_cpu, INPUT_LINE_IRQ5);

	BT458(config, m_ramdac, pixclock);
}

u32 rx2030_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	/*
	 * The graphics board has 1280KiB of video ram fitted, which is organised
	 * such that each 1280 pixel line occupies 2048 bytes of the address space;
	 * the remaining 768 addresses are presumably not mapped to anything.
	 */
	u32 *pixel_pointer = m_vram;

	for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
	{
		for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x += 4)
		{
			u32 const pixel_data = *pixel_pointer++;

			bitmap.pix(y, x + 0) = m_ramdac->pen_color((pixel_data >> 24) & 0xff);
			bitmap.pix(y, x + 1) = m_ramdac->pen_color((pixel_data >> 16) & 0xff);
			bitmap.pix(y, x + 2) = m_ramdac->pen_color((pixel_data >> 8) & 0xff);
			bitmap.pix(y, x + 3) = m_ramdac->pen_color((pixel_data >> 0) & 0xff);
		}

		// compensate by 2048 - 1280 pixels per line
		pixel_pointer += 0xc0;
	}

	return 0;
}

void rx3230_state::rx3230_map(address_map &map)
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

	map(0x1fc00000, 0x1fc3ffff).rom().region("rx3230", 0);
	map(0x1ff00000, 0x1ff3ffff).rom().region("rx3230", 0); // mirror
}

void rx3230_state::rs3230_map(address_map &map)
{
	rx3230_map(map);

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

void rx3230_state::machine_start()
{
	save_item(NAME(m_int_reg));
	save_item(NAME(m_int0_state));
	save_item(NAME(m_int1_state));
}

void rx3230_state::machine_reset()
{
	m_int_reg = INT_CLR;
	m_int0_state = 1;
	m_int1_state = 1;
}

void rx3230_state::rx3230_init()
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

void rx3230_state::rx3230(machine_config &config)
{
	R3000A(config, m_cpu, 50_MHz_XTAL / 2, 32768, 32768);
	m_cpu->set_addrmap(AS_PROGRAM, &rx3230_state::rx3230_map);
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
			adapter.irq_handler_cb().set(*this, FUNC(rx3230_state::irq_w<INT_SCSI>)).invert();
			adapter.drq_handler_cb().set(m_rambo, FUNC(mips_rambo_device::drq_w<0>));
		});

	// ethernet
	AM7990(config, m_net);
	m_net->intr_out().set(FUNC(rx3230_state::irq_w<INT_NET>));
	m_net->dma_in().set(FUNC(rx3230_state::lance_r));
	m_net->dma_out().set(FUNC(rx3230_state::lance_w));

	SCC85C30(config, m_scc, 9.8304_MHz_XTAL); // TODO: clock working but unverified
	m_scc->out_int_callback().set(FUNC(rx3230_state::irq_w<INT_SCC>)).invert();

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
	//m_kbdc->kbd_irq().set(FUNC(rx3230_state::irq_w<INT_KBD>));

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

void rx3230_state::rc3230(machine_config &config)
{
	rx3230(config);

	m_cpu->set_addrmap(AS_PROGRAM, &rx3230_state::rx3230_map);

	m_kbd->set_default_option(STR_KBD_MICROSOFT_NATURAL);
	//m_tty[1]->set_default_option("terminal");
}

void rx3230_state::rs3230(machine_config &config)
{
	rx3230(config);

	m_kbd->set_default_option(STR_KBD_MICROSOFT_NATURAL);

	// FIXME: colour video board disabled for now
	if (false)
	{
		m_cpu->set_addrmap(AS_PROGRAM, &rx3230_state::rs3230_map);

		// video hardware (1280x1024x8bpp @ 60Hz), 16 parts vram
		u32 const pixclock = 108'180'000;

		// timing from VESA 1280x1024 @ 60Hz
		m_screen->set_raw(pixclock, 1688, 248, 1528, 1066, 38, 1062);
		m_screen->set_screen_update(FUNC(rx3230_state::screen_update));
		//m_screen->screen_vblank().set_inputline(m_cpu, INPUT_LINE_IRQ5);

		BT459(config, m_ramdac, pixclock);

		RAM(config, m_vram);
		m_vram->set_default_size("2M");
		m_vram->set_default_value(0);
	}
}

template <u8 Source> void rx3230_state::irq_w(int state)
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

u32 rx3230_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
{
	m_ramdac->screen_update(screen, bitmap, cliprect, m_vram->pointer());

	return 0;
}

u16 rx3230_state::lance_r(offs_t offset, u16 mem_mask)
{
	u16 const data =
		(m_ram->read(BYTE4_XOR_BE(offset + 0)) << 8) |
		m_ram->read(BYTE4_XOR_BE(offset + 1));

	return data;
}

void rx3230_state::lance_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_ram->write(BYTE4_XOR_BE(offset + 1), data);

	if (ACCESSING_BITS_8_15)
		m_ram->write(BYTE4_XOR_BE(offset + 0), data >> 8);
}

ROM_START(rx2030)
	ROM_REGION16_LE(0x40000, "rx2030", 0)
	ROM_SYSTEM_BIOS(0, "v4.32", "Rx2030 v4.32, Jan 1991")
	ROMX_LOAD("50-00121__005.u139", 0x00000, 0x10000, CRC(b2f42665) SHA1(81c83aa6b8865338fda5c03733ede91749997648), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("50-00120__005.u140", 0x00001, 0x10000, CRC(0ffa485e) SHA1(7cdfb81d1a547c5ccc88e1e0ef73d447cd03e9e2), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("50-00119__005.u141", 0x20001, 0x10000, CRC(68fb219d) SHA1(7161ad8e5e0207d8730e09753ca74bfec0e782f8), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("50-00118__005.u142", 0x20000, 0x10000, CRC(b59426d3) SHA1(3fc09b0368f731c2c07cf29b481f30c01e330929), ROM_BIOS(0) | ROM_SKIP(1))

	ROM_SYSTEM_BIOS(1, "v4.30", "Rx2030 v4.30, Jul 1989")
	ROMX_LOAD("50-00121__003.u139", 0x00000, 0x10000, CRC(ebc580ac) SHA1(63f9a1d344d53f32ee769f5137820faf64ffa291), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("50-00120__003.u140", 0x00001, 0x10000, CRC(e1991721) SHA1(028d33be271c95f198473b650f7800f9ca4a60b2), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("50-00119__003.u141", 0x20001, 0x10000, CRC(c8469906) SHA1(69bbf4b5c415b2e2156a4467bf9cb30e79f586ef), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("50-00118__003.u142", 0x20000, 0x10000, CRC(18cc001a) SHA1(198023e92e1e3ba2fc8637f5dd6f370e7e023fdd), ROM_BIOS(1) | ROM_SKIP(1))

	/*
	 * The following isn't a real dump, but a hand-made nvram image that allows
	 * entry to the boot monitor. Variables can be adjusted via the monitor,
	 * and are laid out as follows:
	 *
	 *   Offset  Length  Variable
	 *    0x0e      4    netaddr
	 *    0x12      1    lbaud
	 *    0x13      1    rbaud
	 *    0x14     20    bootfile
	 *    0x28      1    bootmode
	 *    0x29      1    console
	 *    0x2a      1    ponmask? or something similar
	 *    0x2b      3    unused?
	 *    0x2e      4    resetepc
	 *    0x32      4    resetra
	 *    0x36      1    keyswtch
	 *    0x37      1    flag
	 *    0x38      8    unused?
	 *
	 */
	ROM_REGION(0x40, "rtc", 0)
	ROM_LOAD("ds1287.bin", 0x00, 0x40, CRC(28369bf3) SHA1(64f24e1d8fb7103ab0bd3023c66490447bdcbf89))
ROM_END
#define rom_rc2030 rom_rx2030
#define rom_rs2030 rom_rx2030

ROM_START(rx3230)
	ROM_REGION32_BE(0x40000, "rx3230", 0)
	ROM_SYSTEM_BIOS(0, "v5.40", "Rx3230 v5.40, Jun 1990")
	ROMX_LOAD("50-314-003__3230_left.bin",  0x00002, 0x20000, CRC(77ce42c9) SHA1(b2d5e5a386ed0ff840646647ba90b3c36732a7fe), ROM_BIOS(0) | ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2))
	ROMX_LOAD("50-314-003__3230_right.bin", 0x00000, 0x20000, CRC(5bc1ce2f) SHA1(38661234bf40b76395393459de49e48619b2b454), ROM_BIOS(0) | ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2))

	ROM_SYSTEM_BIOS(1, "v5.42", "Rx3230 v5.42, Mar 1991")
	ROMX_LOAD("unknown.bin", 0x00002, 0x20000, NO_DUMP, ROM_BIOS(1) | ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2))
	ROMX_LOAD("unknown.bin", 0x00000, 0x20000, NO_DUMP, ROM_BIOS(1) | ROM_GROUPWORD | ROM_REVERSE | ROM_SKIP(2))

	//ROM_REGION(0x800, "i8042", 0)
	//ROM_LOAD("unknown.bin", 0x000, 0x800, NO_DUMP)

	//ROM_REGION(0x800, "rtc", 0)
	//ROM_LOAD("m48t02.bin", 0x000, 0x800, NO_DUMP)
ROM_END
#define rom_rc3230 rom_rx3230
#define rom_rs3230 rom_rx3230

}

/*   YEAR   NAME       PARENT  COMPAT  MACHINE    INPUT  CLASS         INIT         COMPANY  FULLNAME       FLAGS */
COMP(1989,  rc2030,    0,      0,      rc2030,    0,     rx2030_state, rx2030_init, "MIPS",  "RC2030",      0)
COMP(1989,  rs2030,    0,      0,      rs2030,    0,     rx2030_state, rx2030_init, "MIPS",  "RS2030",      0)
COMP(1990,  rc3230,    0,      0,      rc3230,    0,     rx3230_state, rx3230_init, "MIPS",  "RC3230",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP(1990,  rs3230,    0,      0,      rs3230,    0,     rx3230_state, rx3230_init, "MIPS",  "Magnum 3000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
