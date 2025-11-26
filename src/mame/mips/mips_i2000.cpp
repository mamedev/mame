// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
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

// I/O devices (I2000)
#include "machine/mc146818.h"
#include "machine/z8038.h"
#include "machine/aic6250.h"

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

class mips_i2000_state : public driver_device
{
public:
	mips_i2000_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu(*this, "cpu")
		, m_iop(*this, "iop")
		, m_ram(*this, "ram")
		, m_rom(*this, "i2000")
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
	void i2000(machine_config &config) ATTR_COLD;
	void rs2030(machine_config &config) ATTR_COLD;
	void rc2030(machine_config &config) ATTR_COLD;

	void i2000_init() ATTR_COLD;

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// address maps
	void i2000_map(address_map &map) ATTR_COLD;
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

void mips_i2000_state::machine_start()
{
	save_item(NAME(m_mmu));
	save_item(NAME(m_iop_interface));
}

void mips_i2000_state::machine_reset()
{
	m_cpu->set_input_line(INPUT_LINE_RESET, 1);
}

void mips_i2000_state::i2000_init()
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
	 * Technique is identical to that described for the r3030 below.
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

u16 mips_i2000_state::mmu_r(offs_t offset, u16 mem_mask)
{
	offs_t const address = (m_mmu[(offset >> 11) & 0x1f] << 12) | ((offset << 1) & 0xfff);

	u16 const data = (m_ram->read(BYTE4_XOR_BE(address + 1)) << 8) | m_ram->read(BYTE4_XOR_BE(address + 0));

	LOGMASKED(LOG_MMU, "mmu_r offset 0x%06x reg %d page 0x%04x mapped 0x%06x data 0x%04x\n",
		(offset << 1), (offset >> 11) & 0x1f, m_mmu[(offset >> 11) & 0x1f], address, data);

	return data;
}

void mips_i2000_state::mmu_w(offs_t offset, u16 data, u16 mem_mask)
{
	offs_t const address = (m_mmu[(offset >> 11) & 0x1f] << 12) | ((offset << 1) & 0xfff);

	LOGMASKED(LOG_MMU, "mmu_w offset 0x%06x reg %d page 0x%04x mapped 0x%06x data 0x%04x (%s)\n",
		(offset << 1), (offset >> 11) & 0x1f, m_mmu[(offset >> 11) & 0x1f], address, data, machine().describe_context());

	if (ACCESSING_BITS_0_7)
		m_ram->write(BYTE4_XOR_BE(address + 0), data);

	if (ACCESSING_BITS_8_15)
		m_ram->write(BYTE4_XOR_BE(address + 1), data >> 8);
}

void mips_i2000_state::iop_program_map(address_map &map)
{
	// 00000:1ffff  128k ram (64kx4, 4 parts)
	// 20000:3ffff  128k shared (32x4k mapped pages, bits 0-11 offset, bits 12-16 mmu register)
	// 80000:fffff  512k eprom (256k x 2 copies)

	map(0x00000, 0x1ffff).ram();
	map(0x20000, 0x3ffff).rw(FUNC(mips_i2000_state::mmu_r), FUNC(mips_i2000_state::mmu_w));

	map(0x80000, 0xbffff).rom().region("i2000", 0).mirror(0x40000);
}

void mips_i2000_state::iop_io_map(address_map &map)
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

void mips_i2000_state::i2000_map(address_map &map)
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

void mips_i2000_state::rs2030_map(address_map &map)
{
	i2000_map(map);

	// video hardware
	map(0x01000000, 0x011fffff).ram().share("vram");
	map(0x01ffff00, 0x01ffffff).m(m_ramdac, FUNC(bt458_device::map)).umask32(0xff);

	//map(0x01ff1000, 0x01ff1001).w() // graphics register?
	//map(0x01ff0080, 0x01ff0081).w() // graphics register?
}

u16 mips_i2000_state::lance_r(offs_t offset, u16 mem_mask)
{
	u16 const data =
		(m_ram->read(BYTE4_XOR_BE(offset + 1)) << 8) |
		m_ram->read(BYTE4_XOR_BE(offset + 0));

	return data;
}

void mips_i2000_state::lance_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_ram->write(BYTE4_XOR_BE(offset + 0), data);

	if (ACCESSING_BITS_8_15)
		m_ram->write(BYTE4_XOR_BE(offset + 1), data >> 8);
}

void mips_i2000_state::i2000(machine_config &config)
{
	R2000A(config, m_cpu, 33.333_MHz_XTAL / 2, 32768, 32768);
	m_cpu->set_fpu(mips1_device_base::MIPS_R2010A);
	m_cpu->in_brcond<0>().set([]() { return 1; }); // writeback complete

	V50(config, m_iop, 20_MHz_XTAL);
	m_iop->set_addrmap(AS_PROGRAM, &mips_i2000_state::iop_program_map);
	m_iop->set_addrmap(AS_IO, &mips_i2000_state::iop_io_map);
	m_iop->tout2_cb().set(m_buzzer, FUNC(speaker_sound_device::level_w));

	// general dma configuration
	m_iop->out_hreq_cb().set(m_iop, FUNC(v50_device::hack_w));
	m_iop->in_mem16r_cb().set(FUNC(mips_i2000_state::mmu_r));
	m_iop->out_mem16w_cb().set(FUNC(mips_i2000_state::mmu_w));

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
	m_net->dma_in().set(FUNC(mips_i2000_state::lance_r));
	m_net->dma_out().set(FUNC(mips_i2000_state::lance_w));

	// buzzer
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_buzzer);
	m_buzzer->add_route(ALL_OUTPUTS, "mono", 0.50);
}

void mips_i2000_state::rc2030(machine_config &config)
{
	i2000(config);

	m_cpu->set_addrmap(AS_PROGRAM, &mips_i2000_state::i2000_map);

	m_tty[1]->set_default_option("terminal");
}

void mips_i2000_state::rs2030(machine_config &config)
{
	i2000(config);

	m_cpu->set_addrmap(AS_PROGRAM, &mips_i2000_state::rs2030_map);

	m_kbd->set_default_option(STR_KBD_MICROSOFT_NATURAL);
	m_tty[0]->set_default_option("mouse");

	// video hardware (1280x1024x8bpp @ 60Hz), 40 parts vram
	u32 const pixclock = 108'189'000;

	// timing from VESA 1280x1024 @ 60Hz
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(pixclock, 1688, 248, 1528, 1066, 38, 1062);
	m_screen->set_screen_update(FUNC(mips_i2000_state::screen_update));
	m_screen->screen_vblank().set_inputline(m_cpu, INPUT_LINE_IRQ5);

	BT458(config, m_ramdac, pixclock);
}

u32 mips_i2000_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, rectangle const &cliprect)
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

ROM_START(i2000)
	ROM_REGION16_LE(0x40000, "i2000", 0)
	ROM_SYSTEM_BIOS(0, "v4.32", "I2000 v4.32, Jan 1991")
	ROMX_LOAD("50-00121__005.u139", 0x00000, 0x10000, CRC(b2f42665) SHA1(81c83aa6b8865338fda5c03733ede91749997648), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("50-00120__005.u140", 0x00001, 0x10000, CRC(0ffa485e) SHA1(7cdfb81d1a547c5ccc88e1e0ef73d447cd03e9e2), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("50-00119__005.u141", 0x20001, 0x10000, CRC(68fb219d) SHA1(7161ad8e5e0207d8730e09753ca74bfec0e782f8), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("50-00118__005.u142", 0x20000, 0x10000, CRC(b59426d3) SHA1(3fc09b0368f731c2c07cf29b481f30c01e330929), ROM_BIOS(0) | ROM_SKIP(1))

	ROM_SYSTEM_BIOS(1, "v4.30", "I2000 v4.30, Jul 1989")
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
#define rom_rc2030 rom_i2000
#define rom_rs2030 rom_i2000

} // anonymous namespace

/*   YEAR   NAME       PARENT  COMPAT  MACHINE    INPUT  CLASS             INIT         COMPANY  FULLNAME       FLAGS */
COMP(1989,  rc2030,    0,      0,      rc2030,    0,     mips_i2000_state, i2000_init, "MIPS",  "RC2030",      0)
COMP(1989,  rs2030,    0,      0,      rs2030,    0,     mips_i2000_state, i2000_init, "MIPS",  "RS2030",      0)
