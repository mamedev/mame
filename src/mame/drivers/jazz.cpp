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
 *   - make it boot Windows NT
 *   - big-endian support for RISC/os
 *   - EISA bus and slots
 *   - slotify and improve graphics board
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
	// HACK: make sure the RTC is running
	m_rtc->write_direct(0x0a, 0x20);
}

void jazz_state::init_common()
{
	// map the configured ram and vram
	m_cpu->space(0).install_ram(0x00000000, 0x00000000 | m_ram->mask(), m_ram->pointer());
	// FIXME: video ram must be written in emulated cpu order
	m_cpu->space(0).install_ram(0x40000000, 0x40000000 | m_vram->mask(), m_vram->pointer());
}

void jazz_state::jazz_common_map(address_map &map)
{
	map(0x1fc00000, 0x1fc3ffff).r(m_flash, FUNC(amd_28f020_device::read));

	// NOTE: defaults to console on serial1 if no video rom found
	map(0x60000000, 0x6000013f).lr8("video_rom",
		[](offs_t offset)
		{
			/*
			 * Board_Identifier, PROM_Stride, PROM_Width, PROM_Size
			 *   0=JazzVideoG300, 1=JazzVideoG364, 2=JazzVideoVxl, 0x10=MipsVideoG364, 0x11=MaximumJazzVideo (jazzvideog364 osc period == 123077, MipsVideoG364 = 200000)
			 * Test_Byte_0 - Test_Byte_3
			 * VideoMemorySize, VideoControlSize
			 * CodeOffset, CodeSize
			 * ID string
			 */
			/*
			static u8 const jazz_g364[] =
			{
			    0x01, 0x08, 0x01, 0x20,  'J',  'a',  'z',  'z',
			    0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00,
			    0x00, 0x02, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
			     'J',  'a',  'z',  'z',  ' ',  'G',  '3',  '6',
			     '4', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			};

			static u8 const mips_g364[] =
			{
			    0x10, 0x08, 0x01, 0x20,  'J',  'a',  'z',  'z',
			    0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00,
			    0x00, 0x02, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
			     'M',  'i',  'p',  's',  ' ',  'G',  '3',  '6',
			     '4', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			};
			*/

			// FIXME: this is the MIPS G364 with the PROM fields zeroed to
			// avoid a startup error, and should be replaced with the real
			// signature above when an actual PROM dump is located.
			static u8 const bogus_g364[] =
			{
				0x10, 0x00, 0x00, 0x00,  'J',  'a',  'z',  'z',
				0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00,
				0x00, 0x02, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00,
				 'M',  'i',  'p',  's',  ' ',  'G',  '3',  '6',
				 '4', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			};

			return bogus_g364[offset];
		}).umask64(0xff);

	map(0x60080000, 0x60081fff).m(m_cvc, FUNC(g364_device::map));
	map(0x60180000, 0x60180007).lw32("g364_reset", [this](u32 data) { m_cvc->reset(); });

	map(0x80000000, 0x80000fff).m(m_mct_adr, FUNC(jazz_mct_adr_device::map));

	map(0x80001000, 0x800010ff).m(m_net, FUNC(dp83932c_device::map)).umask32(0x0000ffff);
	map(0x80002000, 0x8000200f).m(m_scsi, FUNC(ncr53c94_device::map));
	map(0x80003000, 0x8000300f).m(m_fdc, FUNC(n82077aa_device::map));

	// LE: only reads 4000
	// BE: read 400d, write 400d, write 400c
	map(0x80004000, 0x8000400f).lrw8("rtc",
		[this](offs_t offset) { return m_rtc->read(1); },
		[this](offs_t offset, u8 data) { m_rtc->write(1, data); }).umask64(0xff);
	map(0x80005000, 0x80005007).rw(m_kbdc, FUNC(ps2_keyboard_controller_device::data_r), FUNC(ps2_keyboard_controller_device::data_w)).umask64(0x00ff);
	map(0x80005000, 0x80005007).rw(m_kbdc, FUNC(ps2_keyboard_controller_device::status_r), FUNC(ps2_keyboard_controller_device::command_w)).umask64(0xff00);
	map(0x80006000, 0x80006007).rw(m_ace[0], FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
	map(0x80007000, 0x80007007).rw(m_ace[1], FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
	map(0x80008000, 0x80008007).rw(m_lpt, FUNC(pc_lpt_device::read), FUNC(pc_lpt_device::write)).umask64(0xffffffff);
	map(0x80009000, 0x8000afff).ram().share("nvram"); // 9000-9fff unprotected/a000-afff protected?
	map(0x8000b000, 0x8000b007).lr8("mac",
		[](offs_t offset)
		{
			// mac address and checksum
			static u8 const mac[] = { 0x00, 0x00, 0x6b, 0x12, 0x34, 0x56, 0x00, 0xf7 };

			return mac[offset];
		});
	// 3 4k pages of nvram: read/write, protected, read-only
	// last page holds ethernet mac and checksum in bytes 0-7

	//map(0x8000c000, 0x8000cfff) // sound
	//map(0x8000d000, 0x8000dfff).noprw(); // dummy dma device?
	map(0x8000d600, 0x8000d607).nopw();

	map(0x8000f000, 0x8000f007).lrw8(
		"led",
		[this]() { return m_led; },
		[this](u8 data)
		{
			logerror("led 0x%02x (%s)\n", data, machine().describe_context());
			m_led = data;
		}).umask64(0xff);

	// lots of byte data written to 800
	//map(0x800e0000, 0x800fffff).m() // dram config

	map(0x90000000, 0x90ffffff).m(m_isp, FUNC(i82357_device::map));

	//map(0x91000000, 0x91ffffff).m();
	//map(0x92000000, 0x92ffffff).m(); // EISA I/O ports?
	//map(0x93000000, 0x93ffffff).m(); // EISA memory

	map(0xf0000000, 0xf0000007).r(m_mct_adr, FUNC(jazz_mct_adr_device::isr_r)).umask64(0xffff);
	map(0xf0000000, 0xf0000007).rw(m_mct_adr, FUNC(jazz_mct_adr_device::imr_r), FUNC(jazz_mct_adr_device::imr_w)).umask64(0xffff0000);

	map(0xfff00000, 0xfff3ffff).r(m_flash, FUNC(amd_28f020_device::read)); // mirror?
}

static void jazz_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
	device.option_add("cdrom", NSCSI_CDROM);
}

FLOPPY_FORMATS_MEMBER(jazz_state::floppy_formats)
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

void jazz_state::jazz(machine_config &config)
{
	// FIXME: slow the cpu clock to get past session manager bugcheck
	R4000(config, m_cpu, 50_MHz_XTAL / 5);
	m_cpu->set_addrmap(0, &jazz_state::jazz_common_map);

	RAM(config, m_ram);
	m_ram->set_default_size("16M");
	m_ram->set_extra_options("32M,64M,128M,256M");
	m_ram->set_default_value(0);

	RAM(config, m_vram);
	m_vram->set_default_size("2M");
	m_vram->set_default_value(0);

	// local bus dma, timer and interrupt controller
	JAZZ_MCT_ADR(config, m_mct_adr, 0);
	m_mct_adr->set_bus(m_cpu, 0);
	m_mct_adr->out_int_dma_cb().set_inputline(m_cpu, INPUT_LINE_IRQ0);
	m_mct_adr->out_int_device_cb().set_inputline(m_cpu, INPUT_LINE_IRQ1);
	m_mct_adr->out_int_timer_cb().set_inputline(m_cpu, INPUT_LINE_IRQ4);
	m_mct_adr->eisa_iack_cb().set(m_isp, FUNC(i82357_device::eisa_irq_ack));

	// scsi bus and devices
	NSCSI_BUS(config, m_scsibus);
	NSCSI_CONNECTOR(config, "scsi:0", jazz_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", jazz_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", jazz_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", jazz_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", jazz_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", jazz_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", jazz_scsi_devices, "cdrom");

	// scsi host adapter
	NSCSI_CONNECTOR(config, "scsi:7").option_set("ncr53c94", NCR53C94).clock(24_MHz_XTAL).machine_config(
		[this](device_t *device)
		{
			ncr53c94_device &adapter = downcast<ncr53c94_device &>(*device);

			adapter.irq_handler_cb().set(m_mct_adr, FUNC(jazz_mct_adr_device::irq<5>));;
			adapter.drq_handler_cb().set(m_mct_adr, FUNC(jazz_mct_adr_device::drq<0>));

			subdevice<jazz_mct_adr_device>(":mct_adr")->dma_r_cb<0>().set(adapter, FUNC(ncr53c94_device::dma_r));
			subdevice<jazz_mct_adr_device>(":mct_adr")->dma_w_cb<0>().set(adapter, FUNC(ncr53c94_device::dma_w));
		});

	// floppy controller and drive
	N82077AA(config, m_fdc, 24_MHz_XTAL);
	m_fdc->intrq_wr_callback().set(m_mct_adr, FUNC(jazz_mct_adr_device::irq<1>));
	m_fdc->drq_wr_callback().set(m_mct_adr, FUNC(jazz_mct_adr_device::drq<1>));
	FLOPPY_CONNECTOR(config, "fdc:0", "35hd", FLOPPY_35_HD, true, jazz_state::floppy_formats).enable_sound(false);
	m_mct_adr->dma_r_cb<1>().set(m_fdc, FUNC(n82077aa_device::dma_r));
	m_mct_adr->dma_w_cb<1>().set(m_fdc, FUNC(n82077aa_device::dma_w));

	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->set_epoch(1980);

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);

	AMD_28F020(config, m_flash);

	// pc keyboard connector
	pc_kbdc_device &kbd_con(PC_KBDC(config, "kbd_con", 0));
	kbd_con.out_clock_cb().set(m_kbdc, FUNC(ps2_keyboard_controller_device::kbd_clk_w));
	kbd_con.out_data_cb().set(m_kbdc, FUNC(ps2_keyboard_controller_device::kbd_data_w));

	// keyboard port
	pc_kbdc_slot_device &kbd(PC_KBDC_SLOT(config, "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL));
	kbd.set_pc_kbdc_slot(&kbd_con);

    // auxiliary connector
    pc_kbdc_device &aux_con(PC_KBDC(config, "aux_con", 0));
    aux_con.out_clock_cb().set(m_kbdc, FUNC(ps2_keyboard_controller_device::aux_clk_w));
    aux_con.out_data_cb().set(m_kbdc, FUNC(ps2_keyboard_controller_device::aux_data_w));

    // auxiliary port
    pc_kbdc_slot_device &aux(PC_KBDC_SLOT(config, "aux", ps2_mice, STR_HLE_PS2_MOUSE));
    aux.set_pc_kbdc_slot(&aux_con);

	// keyboard controller
	PS2_KEYBOARD_CONTROLLER(config, m_kbdc, 12_MHz_XTAL);
	// FIXME: reset is probably routed through the MCT-ADR
	m_kbdc->hot_res().set([this](int state) { logerror("reset %d\n", state); });
	m_kbdc->kbd_clk().set(kbd_con, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_kbdc->kbd_data().set(kbd_con, FUNC(pc_kbdc_device::data_write_from_mb));
	m_kbdc->kbd_irq().set(m_mct_adr, FUNC(jazz_mct_adr_device::irq<6>));;
	m_kbdc->aux_clk().set(aux_con, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_kbdc->aux_data().set(aux_con, FUNC(pc_kbdc_device::data_write_from_mb));
	m_kbdc->aux_irq().set(m_mct_adr, FUNC(jazz_mct_adr_device::irq<7>));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(78643200, 1280, 0, 1280, 1024, 0, 1024);
	m_screen->set_screen_update("g364", FUNC(g364_device::screen_update));
	m_screen->screen_vblank().set(m_mct_adr, FUNC(jazz_mct_adr_device::irq<3>)); // maybe?

	G364(config, m_cvc, 5_MHz_XTAL); // FIXME: guessed clock
	m_cvc->set_screen(m_screen);
	m_cvc->set_vram(m_vram);

	// WD16C552 (two 16550 + pc_lpt)
	NS16550(config, m_ace[0], 4233600);
	rs232_port_device &serial0(RS232_PORT(config, "serial0", default_rs232_devices, nullptr));

	m_ace[0]->out_dtr_callback().set(serial0, FUNC(rs232_port_device::write_dtr));
	m_ace[0]->out_rts_callback().set(serial0, FUNC(rs232_port_device::write_rts));
	m_ace[0]->out_tx_callback().set(serial0, FUNC(rs232_port_device::write_txd));
	m_ace[0]->out_int_callback().set(m_mct_adr, FUNC(jazz_mct_adr_device::irq<8>));

	serial0.cts_handler().set(m_ace[0], FUNC(ns16550_device::cts_w));
	serial0.dcd_handler().set(m_ace[0], FUNC(ns16550_device::dcd_w));
	serial0.dsr_handler().set(m_ace[0], FUNC(ns16550_device::dsr_w));
	serial0.ri_handler().set(m_ace[0], FUNC(ns16550_device::ri_w));
	serial0.rxd_handler().set(m_ace[0], FUNC(ns16550_device::rx_w));

	NS16550(config, m_ace[1], 8_MHz_XTAL);
	rs232_port_device &serial1(RS232_PORT(config, "serial1", default_rs232_devices, nullptr));

	m_ace[1]->out_dtr_callback().set(serial1, FUNC(rs232_port_device::write_dtr));
	m_ace[1]->out_rts_callback().set(serial1, FUNC(rs232_port_device::write_rts));
	m_ace[1]->out_tx_callback().set(serial1, FUNC(rs232_port_device::write_txd));
	m_ace[1]->out_int_callback().set(m_mct_adr, FUNC(jazz_mct_adr_device::irq<9>));

	serial1.cts_handler().set(m_ace[1], FUNC(ns16550_device::cts_w));
	serial1.dcd_handler().set(m_ace[1], FUNC(ns16550_device::dcd_w));
	serial1.dsr_handler().set(m_ace[1], FUNC(ns16550_device::dsr_w));
	serial1.ri_handler().set(m_ace[1], FUNC(ns16550_device::ri_w));
	serial1.rxd_handler().set(m_ace[1], FUNC(ns16550_device::rx_w));

	PC_LPT(config, m_lpt, 0);
	m_lpt->irq_handler().set(m_mct_adr, FUNC(jazz_mct_adr_device::irq<0>));

	// TODO: sound, interrupt 2, drq 2(l) & 3(r)

	// buzzer
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_buzzer);
	m_buzzer->add_route(ALL_OUTPUTS, "mono", 0.50);

	DP83932C(config, m_net, 20_MHz_XTAL);
	m_net->out_int_cb().set(m_mct_adr, FUNC(jazz_mct_adr_device::irq<4>));
	m_net->set_ram(RAM_TAG);

	I82357(config, m_isp, 14.318181_MHz_XTAL);
	m_isp->out_rtc_cb().set(m_rtc, FUNC(mc146818_device::write));
	m_isp->out_int_cb().set_inputline(m_cpu, INPUT_LINE_IRQ2);
	m_isp->out_nmi_cb().set_inputline(m_cpu, INPUT_LINE_IRQ3);
	m_isp->out_spkr_cb().set(m_buzzer, FUNC(speaker_sound_device::level_w));

	// TODO: 4 EISA slots
}

void jazz_state::mmr4000be(machine_config &config)
{
	jazz(config);

	m_cpu->set_config(r4000_device::CONFIG_BE, r4000_device::CONFIG_BE);
}

void jazz_state::mmr4000le(machine_config &config)
{
	jazz(config);

	m_cpu->set_config(0, r4000_device::CONFIG_BE);
}

ROM_START(mmr4000be)
	ROM_REGION64_BE(0x40000, "flash", 0)
	ROM_SYSTEM_BIOS(0, "riscos", "R4000 RISC/os PROM")
	ROMX_LOAD("riscos.bin", 0x00000, 0x40000, CRC(cea6bc8f) SHA1(3e47b4ad5d1a0c7aac649e6aef3df1bf86fc938b), ROM_BIOS(0))
ROM_END

ROM_START(mmr4000le)
	ROM_REGION64_LE(0x40000, "flash", 0)
	ROM_SYSTEM_BIOS(0, "ntprom", "R4000 Windows NT PROM")
	ROMX_LOAD("ntprom.bin", 0x00000, 0x40000, CRC(d91018d7) SHA1(316de17820192c89b8ee6d9936ab8364a739ca53), ROM_BIOS(0))
ROM_END

/*    YEAR   NAME       PARENT  COMPAT  MACHINE    INPUT  CLASS       INIT         COMPANY  FULLNAME                 FLAGS */
COMP( 1992,  mmr4000be, 0,      0,      mmr4000be, 0,     jazz_state, init_common, "MIPS",  "Magnum R4000 (big)",    MACHINE_IS_SKELETON)
COMP( 1992,  mmr4000le, 0,      0,      mmr4000le, 0,     jazz_state, init_common, "MIPS",  "Magnum R4000 (little)", MACHINE_IS_SKELETON)
