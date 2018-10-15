// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An emulation of systems designed and manufactured by MIPS Computer Systems,
 * all of which use MIPS R2000, R3000 or R6000 CPUs, and run the RISC/os
 * operating system.
 *
 * This driver is intended to eventually cover the following models:
 *
 *   Name        CPU      Clock  Slots    Disk  Package       Other
 *   M/500       R2000     5MHz  VME      ESDI
 *   M/800       R2000     8MHz  VME      ESDI
 *   M/1000      R2000    10MHz  VME      ESDI
 *   M/120-3     R2000    12MHz  PC-AT    SCSI  Deskside
 *   M/120-5     R2000    16MHz  PC-AT    SCSI  Deskside
 *   M/2000-6    R3000    20MHz  VMEx13   SMD   Rack Cabinet
 *   M/2000-8    R3000    25MHz  VMEx13   SMD   Rack Cabinet
 *   RC2030      R2000    16MHz           SCSI  Desktop
 *   RS2030      R2000    16MHz           SCSI  Desktop       aka M/12
 *   RC3230      R3000    25MHz  PC-ATx1  SCSI  Desktop
 *   RS3230      R3000    25MHz  PC-ATx1  SCSI  Desktop       aka M/20, Magnum 3000
 *   RC3240      R3000    20MHz  PC-ATx4  SCSI  Deskside      M/120 with CPU-board upgrade
 *   RC3330      R3000    33MHz  PC-AT    SCSI  Desktop
 *   RS3330      R3000    33MHz  PC-AT    SCSI  Desktop
 *   RC3260      R3000    25MHz  VMEx7    SCSI  Pedestal
 *   RC3360      R3000    33MHz  VME      SCSI  Pedestal
 *   RC6260      R6000    66MHz  VME      SCSI  Pedestal
 *   RC6280      R6000    66MHz  VMEx6    SMD   Data Center
 *   RC6380-100  R6000x1  66MHz  VME      SMD   Data Center
 *   RC6380-200  R6000x2  66MHz  VME      SMD   Data Center
 *   RC6380-400  R6000x4  66MHz  VME      SMD   Data Center
 *
 * Sources:
 *
 *   http://www.umips.net/
 *   http://www.geekdot.com/the-mips-rs2030/
 *   http://www.jp.netbsd.org/ports/mipsco/models.html
 *
 * TODO
 *   - everything (skeleton only)
 *
 *   Ref   Part                      Function
 *
 * System board:
 *
 * Video board:
 *
 */
/*
 * WIP notes
 *
 * nvram must be prepped with 0x74, 0x61 or 0x31 at offset 0x29
 *
 * Currently fails during IOP address path diagnostic. Bypass with:
 *   bp f7c51; g
 *   pc=f7c54; aw=0; g
 *
 * V50 internal peripherals:
 * base = 0xfe00
 * serial (sula): fe00
 *  timer (tula): fe08
 *    int (iula): fe10
 *    dma (dula): fe20 - unimplemented in uPD71037 mode (sctl & 2)
 *
 */

#include "emu.h"

#include "includes/mips.h"

#include "debugger.h"

#define VERBOSE 0
#include "logmacro.h"

void rx2030_state::machine_start()
{
}

void rx2030_state::machine_reset()
{
}

void rx2030_state::rx2030_init()
{
	// map the configured ram and vram
	m_maincpu->space(0).install_ram(0x00000000, 0x00000000 | m_ram->mask(), m_ram->pointer());

	if (m_vram)
		m_maincpu->space(0).install_ram(0x01000000, 0x01000000 | m_vram->mask(), m_vram->pointer());
}

void rx2030_state::iop_program_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram();

	map(0x20000, 0xbffff).lrw16("shared",
		[this](offs_t offset, u16 mem_mask)
		{
			offs_t const address = 0x1000 + (offset << 1);

			u16 const data = (m_ram->read(address + 1) << 8) | m_ram->read(address + 0);
			//logerror("shared_r 0x%04x data 0x%04x mask 0x%04x\n", address, data, mem_mask);
			return data;
		},
		[this](offs_t offset, u16 data, u16 mem_mask)
		{
			offs_t const address = 0x1000 + (offset << 1);

			//logerror("shared_w 0x%04x data 0x%04x mask 0x%04x\n", address, data, mem_mask);
			m_ram->write(address + 0, data);
			m_ram->write(address + 1, data >> 8);
		});
	map(0xc0000, 0xfffff).rom().region("eprom", 0);
}

void rx2030_state::iop_io_map(address_map &map)
{
	// 0x180-18a ? .umask16(0xff)
	// 0x1c0 ?
	// 0x044
	// 0x2c0

	map(0x0000, 0x003f).lrw16("mmu",
		[this](offs_t offset, u16 mem_mask)
		{
			return m_mmu[offset];
		},
		[this](offs_t offset, u16 data, u16 mem_mask)
		{
			//logerror("mmu_w %d data 0x%04x\n", offset, data);

			m_mmu[offset] = data;
		});

	map(0x00c0, 0x00c1).rw(m_kbdc, FUNC(at_keyboard_controller_device::data_r), FUNC(at_keyboard_controller_device::data_w)).umask16(0xff);
	map(0x00c4, 0x00c5).rw(m_kbdc, FUNC(at_keyboard_controller_device::status_r), FUNC(at_keyboard_controller_device::command_w)).umask16(0xff);

	map(0x0080, 0x0083).rw(m_scsi, FUNC(wd33c93_device::read), FUNC(wd33c93_device::write)).umask16(0xff);

	map(0x0100, 0x0107).rw(m_scc, FUNC(z80scc_device::ba_cd_inv_r), FUNC(z80scc_device::ba_cd_inv_w)).umask16(0xff);
	map(0x0140, 0x0143).rw(m_net, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w));
	//map(0x0200, 0x0203).rw(m_lpt, FUNC(::read), FUNC(::write));

	map(0x0240, 0x0241).lw8("rtc", [this](address_space &space, offs_t offset, u8 data) { m_rtc->write(space, 0, data); }).umask16(0xff00);
	map(0x0280, 0x0281).lrw8("rtc",
		[this](address_space &space, offs_t offset) { return m_rtc->read(space, 1); },
		[this](address_space &space, offs_t offset, u8 data) { m_rtc->write(space, 1, data); }).umask16(0xff00);

	map(0x0380, 0x0381).lw8("led", [this](u8 data) { logerror("led_w 0x%02x\n", data); }).umask16(0xff00);
}

void rx2030_state::cpu_map(address_map &map)
{
	//map(0x01ff0000, 0x01ffffff).m() // ramdac
	//map(0x1fc00000, 0x1fc3ffff).rom().region("eprom", 0);

	//map(0x10003000, 0x1003ffff).rom().region("eprom", 0); // mirror?
}

void rx2030_state::rx2030(machine_config &config)
{
	// FIXME: main cpu disabled for now
	R2000(config, m_maincpu, 16.6698_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &rx2030_state::cpu_map);
	m_maincpu->in_brcond0().set([]() { return ASSERT_LINE; });
	m_maincpu->set_disable();

	// TODO: mouse connects to v50 serial?
	V53(config, m_iop, 20_MHz_XTAL / 2);
	m_iop->set_addrmap(AS_PROGRAM, &rx2030_state::iop_program_map);
	m_iop->set_addrmap(AS_IO, &rx2030_state::iop_io_map);

	RAM(config, m_ram);
	m_ram->set_default_size("16M");
	m_ram->set_extra_options("4M,8M,12M");
	m_ram->set_default_value(0);

	// rtc and nvram
	MC146818(config, m_rtc, 32.768_kHz_XTAL);

	// TODO: ethernet id prom
	// TODO: parallel port

	// keyboard
	pc_kbdc_device &kbdc(PC_KBDC(config, "pc_kbdc", 0));
	kbdc.out_clock_cb().set(m_kbdc, FUNC(at_keyboard_controller_device::kbd_clk_w));
	kbdc.out_data_cb().set(m_kbdc, FUNC(at_keyboard_controller_device::kbd_data_w));

	PC_KBDC_SLOT(config, m_kbd, 0);
	pc_at_keyboards(*m_kbd);
	m_kbd->set_pc_kbdc_slot(&kbdc);

	AT_KEYBOARD_CONTROLLER(config, m_kbdc, 12_MHz_XTAL);
	//m_kbdc->hot_res().set_inputline(m_maincpu, INPUT_LINE_RESET);
	m_kbdc->kbd_clk().set(kbdc, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_kbdc->kbd_data().set(kbdc, FUNC(pc_kbdc_device::data_write_from_mb));

	// TODO: verify scc device and clock
	SCC85C30(config, m_scc, 4.9152_MHz_XTAL);
	m_scc->configure_channels(1'843'200, 1'843'200, 1'843'200, 1'843'200);
	//m_scc->out_int_callback().set(m_iop, FUNC(v53_device::intp1_w));

	// scc channel A (tty0)
	RS232_PORT(config, m_tty[0], default_rs232_devices, nullptr);
	m_tty[0]->cts_handler().set(m_scc, FUNC(z80scc_device::ctsa_w));
	m_tty[0]->dcd_handler().set(m_scc, FUNC(z80scc_device::dcda_w));
	m_tty[0]->rxd_handler().set(m_scc, FUNC(z80scc_device::rxa_w));
	m_scc->out_rtsa_callback().set(m_tty[0], FUNC(rs232_port_device::write_rts));
	m_scc->out_txda_callback().set(m_tty[0], FUNC(rs232_port_device::write_txd));
	//m_scc->out_wreqa_callback().set(m_ioga, FUNC(interpro_ioga_device::drq_serial1)).invert();

	// scc channel B (tty1)
	RS232_PORT(config, m_tty[1], default_rs232_devices, nullptr);
	m_tty[1]->cts_handler().set(m_scc, FUNC(z80scc_device::ctsb_w));
	m_tty[1]->dcd_handler().set(m_scc, FUNC(z80scc_device::dcdb_w));
	m_tty[1]->rxd_handler().set(m_scc, FUNC(z80scc_device::rxb_w));
	m_scc->out_rtsb_callback().set(m_tty[1], FUNC(rs232_port_device::write_rts));
	m_scc->out_txdb_callback().set(m_tty[1], FUNC(rs232_port_device::write_txd));
	//m_scc->out_wreqb_callback().set(m_ioga, FUNC(interpro_ioga_device::drq_serial2)).invert();

	// floppy controller and drive
	WD37C65C(config, m_fdc, 16_MHz_XTAL);
	//m_fdc->intrq_wr_callback().set(m_mct_adr, FUNC(jazz_mct_adr_device::irq<1>));
	//m_fdc->drq_wr_callback().set(m_mct_adr, FUNC(jazz_mct_adr_device::drq<1>));
	FLOPPY_CONNECTOR(config, "fdc:0", "35hd", FLOPPY_35_HD, true, &FLOPPY_PC_FORMAT).enable_sound(false);
	//m_mct_adr->dma_r_cb<1>().set(m_fdc, FUNC(n82077aa_device::dma_r));
	//m_mct_adr->dma_w_cb<1>().set(m_fdc, FUNC(n82077aa_device::dma_w));

	// scsi controller and port
	scsi_port_device &scsi_port(SCSI_PORT(config, "scsi_port"));
	WD33C93(config, m_scsi);
	m_scsi->set_scsi_port(scsi_port);
	//m_scsi->irq_cb().set_inputline()

	// ethernet
	AM7990(config, m_net);
	//m_net->dma_in().set(FUNC(rx2030_state::lance_dma_r));
	//m_net->dma_out().set(FUNC(rx2030_state::lance_dma_w));
}

void rx2030_state::rc2030(machine_config &config)
{
	rx2030(config);

	// no keyboard
	m_kbd->set_default_option(nullptr);

	m_tty[1]->set_default_option("terminal");
}

void rx2030_state::rs2030(machine_config &config)
{
	rx2030(config);

	m_kbd->set_default_option(STR_KBD_MICROSOFT_NATURAL);

	// video hardware (1280x1024x8bpp @ 60Hz), 40 parts vram
	const u32 pixclock = 108'189'000;

	// timing from VESA 1280x1024 @ 60Hz
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(pixclock, 1688, 248, 1528, 1066, 38, 1062);
	m_screen->set_screen_update(FUNC(rx2030_state::screen_update));
	m_screen->screen_vblank().set([this](int state) {});

	BT458(config, m_ramdac, pixclock);
	RAM(config, m_vram, 0).set_default_size("2M");
}

u32 rx2030_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u8 *pixel_data = m_vram->pointer();

	for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
		for (int x = screen.visible_area().min_x; x <= screen.visible_area().max_x; x++)
			bitmap.pix(y, x) = m_ramdac->pen_color(*pixel_data++);

	return 0;
}

ROM_START(rx2030)
	ROM_REGION(0x40000, "eprom", 0)

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
ROM_END
#define rom_rc2030 rom_rx2030
#define rom_rs2030 rom_rx2030

/*   YEAR   NAME       PARENT  COMPAT  MACHINE    INPUT  CLASS         INIT         COMPANY  FULLNAME  FLAGS */
COMP(1989,  rc2030,    0,      0,      rc2030,    0,     rx2030_state, rx2030_init, "MIPS",  "RC2030", MACHINE_IS_SKELETON )
COMP(1989,  rs2030,    0,      0,      rs2030,    0,     rx2030_state, rx2030_init, "MIPS",  "RS2030", MACHINE_IS_SKELETON )
