// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An emulation of systems based on the Jazz computer architecture, originally
 * developed by Microsoft. Specific systems which implemented this architecture
 * include the MIPS Magnum/Millenium 4000 and Olivetti M700-10.
 *
 * References:
 *
 *   https://www.linux-mips.org/wiki/Jazz
 *   http://gunkies.org/wiki/MIPS_Magnum
 *   http://www.sensi.org/~alec/mips/mips-history.html
 *
 * TODO
 *   - everything (skeleton only)
 *
 * Unconfirmed parts lists from ARCSystem reference design (which appears to
 * be very similar or identical to the Jazz system) taken from:
 *   https://www.linux-mips.org/archives/riscy/1993-12/msg00013.html
 *
 *   Ref   Part                      Function
 *
 * System board:
 *
 *         Dallas DS1287             RTC and NVRAM
 *         Dallas DS1225Y            8k non-volatile SRAM
 *         WD16C552                  Dual serial and parallel port controller
 *         Intel N82077A             Floppy drive controller
 *         National DP83932BFV       Ethernet controller
 *         Intel 82358               EISA Bus Controller
 *         Intel 82357               EISA Integrated System Peripheral (ISP)
 *         Intel 82352 x 2           EISA Bus Buffer (EBB)
 *         Emulex FAS216             SCSI controller
 *         27C01                     128k EPROM
 *         28F020                    256k flash memory
 *         NEC μPD31432              ARC address path ASIC
 *         NEC μPD31431 x 2          ARC data path ASIC
 *         NEC μPD30400              R4000PC/50 CPU
 *
 * Audio board:
 *
 *         Crystal CS4215            Audio codec
 *         Altera FPGA x 4           Audio DMA
 *
 * Video board:
 *
 *         27C010                    128k EPROM
 *         IMS G364-11S              Video controller
 *         NEC μPD42274V-80 x 16     256kx4 VRAM (2MiB)
 */

#include "emu.h"

#include "includes/jazz.h"

#include "debugger.h"

#define VERBOSE 0
#include "logmacro.h"

void jazz_state::machine_start()
{
}

void jazz_state::machine_reset()
{
}

void jazz_state::init_common()
{
	// map the configured ram
	m_maincpu->space(AS_PROGRAM).install_ram(0, m_ram->mask(), m_ram->pointer());
}

void jazz_state::jazz_common_map(address_map &map)
{
	map(0x1fc00000, 0x1fc3ffff).r(m_flash, FUNC(amd_28f020_device::read));

	// FIXME: lots of guesswork and assumptions here for now
	map(0x80000000, 0x80000fff).m(m_mct_adr, FUNC(jazz_mct_adr_device::map));
	map(0x80001000, 0x80001fff).m(m_network, FUNC(dp83932c_device::map));
	//map(0x80002000, 0x80002fff).m(m_scsi, FUNC(ncr5390_device::map)).umask32(0x000000ff);
	map(0x80003000, 0x80003fff).m(m_fdc, FUNC(n82077aa_device::map));
	//map(0x80004000, 0x80004fff).rw(m_rtc, FUNC(mc146818_device::read), FUNC(mc146818_device::write));
	//map(0x80005000, 0x80005fff).m() // keyboard/mouse
	//map(0x80006000, 0x80006fff).m() // serial1
	//map(0x80007000, 0x80007fff).m() // serial2
	//map(0x80008000, 0x80008fff).m() // parallel
	map(0x80009000, 0x80009fff).ram().share("nvram"); // unprotected?
	map(0x8000a000, 0x8000afff).ram().share("nvram"); // protected?
	map(0x8000b000, 0x8000bfff).ram().share("nvram"); // read-only?  also sonic IO access?
	//map(0x8000c000, 0x8000cfff) // sound
	map(0x8000d000, 0x8000dfff).noprw(); // dummy dma device?

	map(0x8000f000, 0x8000f001).rw(FUNC(jazz_state::led_r), FUNC(jazz_state::led_w));

	//map(0x800e0000, 0x800e0000).rw(FUNC(jazz_state::dram_config_r), FUNC(jazz_state::dram_config_w));

	map(0xe0800000, 0xe0bfffff).ram().share("vram"); // framebuffer?

	map(0xfff00000, 0xfff3ffff).r(m_flash, FUNC(amd_28f020_device::read)); // mirror?
}

static void jazz_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("cdrom", NSCSI_CDROM);
}

void jazz_state::jazz(machine_config &config)
{
	m_maincpu->set_addrmap(AS_PROGRAM, &jazz_state::jazz_common_map);

	RAM(config, m_ram, 0);
	m_ram->set_default_size("8M");
	m_ram->set_extra_options("16M,32M,64M,128M,256M");
	m_ram->set_default_value(0);

	RAM(config, m_vram);
	m_vram->set_default_size("2M");
	m_vram->set_default_value(0);

	// FIXME: may require big and little endian variants
	JAZZ_MCT_ADR(config, m_mct_adr, 0);

	// scsi bus and devices
	NSCSI_BUS(config, m_scsibus, 0);

	nscsi_connector &harddisk(NSCSI_CONNECTOR(config, "scsi:0", 0));
	jazz_scsi_devices(harddisk);
	harddisk.set_default_option("harddisk");

	nscsi_connector &cdrom(NSCSI_CONNECTOR(config, "scsi:6", 0));
	jazz_scsi_devices(cdrom);
	cdrom.set_default_option("cdrom");

	jazz_scsi_devices(NSCSI_CONNECTOR(config, "scsi:1", 0));
	jazz_scsi_devices(NSCSI_CONNECTOR(config, "scsi:2", 0));
	jazz_scsi_devices(NSCSI_CONNECTOR(config, "scsi:3", 0));
	jazz_scsi_devices(NSCSI_CONNECTOR(config, "scsi:4", 0));
	jazz_scsi_devices(NSCSI_CONNECTOR(config, "scsi:5", 0));

	// scsi host adapter
	nscsi_connector &adapter(NSCSI_CONNECTOR(config, "scsi:7", 0));
	adapter.option_add_internal("host", NCR53C90A);
	adapter.set_default_option("host");
	adapter.set_fixed(true);

	N82077AA(config, m_fdc, 24_MHz_XTAL);

	MC146818(config, m_rtc, 32.768_kHz_XTAL);

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);

	AMD_28F020(config, m_flash);

	// pc keyboard controller?
	pc_kbdc_device &kbdc(PC_KBDC(config, "pc_kbdc", 0));
	kbdc.out_clock_cb().set(m_kbdc, FUNC(at_keyboard_controller_device::kbd_clk_w));
	kbdc.out_data_cb().set(m_kbdc, FUNC(at_keyboard_controller_device::kbd_data_w));

	// keyboard port
	pc_kbdc_slot_device &kbd(PC_KBDC_SLOT(config, "kbd", 0));
	pc_at_keyboards(kbd);
	kbd.set_default_option(STR_KBD_IBM_PC_AT_84);
	kbd.set_pc_kbdc_slot(&kbdc);

	// at keyboard controller
	AT_KEYBOARD_CONTROLLER(config, m_kbdc, 12_MHz_XTAL);
	m_kbdc->hot_res().set_inputline(m_maincpu, INPUT_LINE_RESET);
	m_kbdc->kbd_clk().set(kbdc, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_kbdc->kbd_data().set(kbdc, FUNC(pc_kbdc_device::data_write_from_mb));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(78643200, 1280, 0, 1280, 1024, 0, 1024);
	m_screen->set_screen_update("g364", FUNC(g364_device::screen_update));

	G364(config, m_ramdac, 5_MHz_XTAL); // FIXME: guessed clock
	m_ramdac->set_screen(m_screen);
	m_ramdac->set_vram(m_vram);

	DP83932C(config, m_network, 20_MHz_XTAL);
	m_network->set_ram(RAM_TAG);
}

void jazz_state::mmr4000be(machine_config &config)
{
	R4000BE(config, m_maincpu, 50_MHz_XTAL);

	jazz(config);
}

void jazz_state::mmr4000le(machine_config &config)
{
	R4000LE(config, m_maincpu, 50_MHz_XTAL);

	jazz(config);
}

ROM_START(mmr4000be)
	ROM_REGION32_BE(0x40000, "flash", 0)
	ROM_SYSTEM_BIOS(0, "riscos", "R4000 RISC/os PROM")
	ROMX_LOAD("riscos.bin", 0x00000, 0x40000, CRC(cea6bc8f) SHA1(3e47b4ad5d1a0c7aac649e6aef3df1bf86fc938b), ROM_BIOS(0))
ROM_END

ROM_START(mmr4000le)
	ROM_REGION32_LE(0x40000, "flash", 0)
	ROM_SYSTEM_BIOS(0, "ntprom", "R4000 Windows NT PROM")
	ROMX_LOAD("ntprom.bin", 0x00000, 0x40000, CRC(d91018d7) SHA1(316de17820192c89b8ee6d9936ab8364a739ca53), ROM_BIOS(0))
ROM_END

/*    YEAR   NAME       PARENT  COMPAT  MACHINE    INPUT  CLASS       INIT         COMPANY  FULLNAME                 FLAGS */
COMP( 1992,  mmr4000be, 0,      0,      mmr4000be, 0,     jazz_state, init_common, "MIPS",  "Magnum R4000 (big)",    MACHINE_IS_SKELETON)
COMP( 1992,  mmr4000le, 0,      0,      mmr4000le, 0,     jazz_state, init_common, "MIPS",  "Magnum R4000 (little)", MACHINE_IS_SKELETON)
