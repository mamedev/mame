// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont, Juergen Buchmueller
/******************************************************************************
 *
 *  Acorn Archimedes
 *
 *  Skeleton: Juergen Buchmueller <pullmoll@t-online.de>, Jul 2000
 *  Enhanced: R. Belmont, June 2007
 *  Angelo Salese, August 2010
 *
 *  AKB10 - Archimedes 305
 *  AKB15 - Archimedes 310
 *  AKB20 - Archimedes 440
 *  AKB26 - Archimedes 410 (advertised but not known to be produced)
 *  AKB40 - Archimedes 410/1
 *  AKB42 - Archimedes 420/1
 *  AKB50 - Archimedes 440/1
 *  AKB01 - BBC A3000
 *  AKB50 - Archimedes 540
 *  ALB22 - Acorn A5000 2MB HD 80
 *  ALB24 - Acorn A5000 4MB HD 120
 *  AKB62 - Acorn A4 2MB FD
 *  AKB64 - Acorn A4 4MB HD 60
 *  AGB11 - Acorn A3010
 *  AGB22 - Acorn A3020 FD
 *  AGB23 - Acorn A3020 HD 60
 *  AGB33 - Acorn A020 HD 80
 *  AGC10 - Acorn A4000
 *  AGC20 - Acorn A4000 2MB HD 80
 *
 * Notes:
 * - Hold DEL down during boot reset the CMOS memory to the default values.
 * - default NVRAM is plainly wrong. Use the status/configure commands to set up properly
 *   (Scroll Lock is currently mapped with Right SHIFT, use this to move to next page of status).
 *   In order to load a floppy, you need at very least:
 *   configure floppies 2
 *   configure filesystem adfs
 *   configure monitortype 12
 *   Then reboot / reset the machine, and use cat to (attempt) to load a floppy contents.
 *
 *  TODO:
 *  - RISC OS Draw app uses unimplemented copro instructions
 *  - Move joystick ports into slot devices.
 *  - Add ABORT line support to the ARM core.
 *  - Hard disc controller.
 *  - Serial interface.
 *  - 82c711.
 *  - Podules expansions.
 *
 *
 *
=======================================================================================
 *
 *      Memory map (from http://b-em.bbcmicro.com/arculator/archdocs.txt)
 *
 *  0000000 - 1FFFFFF - logical RAM (32 meg)
 *  2000000 - 2FFFFFF - physical RAM (supervisor only - max 16MB - requires quad MEMCs)
 *  3000000 - 33FFFFF - IOC (IO controllers - supervisor only)
 *  3310000 - FDC - WD1772
 *  33A0000 - Econet - 6854
 *  33B0000 - Serial - 6551
 *  3240000 - 33FFFFF - internal expansion cards
 *  32D0000 - hard disc controller (not IDE) - HD63463
 *  3350010 - printer
 *  3350018 - latch A
 *  3350040 - latch B
 *  3270000 - external expansion cards
 *
 *  3400000 - 3FFFFFF - ROM (read - 12 meg - Arthur and RiscOS 2 512k, RiscOS 3 2MB)
 *  3400000 - 37FFFFF - Low ROM  (4 meg, I think this is expansion ROMs)
 *  3800000 - 3FFFFFF - High ROM (main OS ROM)
 *
 *  3400000 - 35FFFFF - VICD10 (write - supervisor only)
 *  3600000 - 3FFFFFF - MEMC (write - supervisor only)
 *
 *****************************************************************************/
/*
    DASM of code (bios 2 / RISC OS 2)
    0x380d4e0: MEMC: control to 0x10c (page size 32 kbytes, DRAM ram refresh only during flyback)
    0x380d4f0: VIDC: params (screen + sound frequency)
    0x380d51c: IOC: sets control to 0xff, clear IRQA and FIQ masks, sets IRQB mask to 0x80 (keyboard receive full irq)
    0x380d530: IOC: sets timer 0 to 0x4e20, go command
        0x380e0a8: work RAM physical check, max size etc.
    0x380e1f8: IOC: Disables DRAM ram refresh, sets timer 1 to 0x7ffe, go command, then it tests the latch of this timer, enables DRAM refresh
        0x380d00c: Set up default logical space
        0x380d16c: Set up case by case logical space


*/


#include "emu.h"

#include "cpu/arm/arm.h"
#include "formats/acorn_dsk.h"
#include "formats/apd_dsk.h"
#include "formats/jfd_dsk.h"
#include "formats/pc_dsk.h"
#include "imagedev/floppy.h"
#include "machine/acorn_ioc.h"
#include "machine/acorn_memc.h"
#include "machine/acorn_vidc.h"
#include "machine/archimedes_keyb.h"
#include "machine/pcf8583.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

namespace {

class aa310_state : public driver_device
{
public:
	aa310_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ioc(*this, "ioc")
		, m_memc(*this, "memc")
		, m_vidc(*this, "vidc")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_ram(*this, RAM_TAG)
		, m_joy(*this, "joy_p%u", 1U)
	{ }

	void aa5000a(machine_config &config);
	void aa305(machine_config &config);
	void aa310(machine_config &config);
	void aa3000(machine_config &config);
	void aa5000(machine_config &config);
	void aa4101(machine_config &config);
	void aa3020(machine_config &config);
	void aa4401(machine_config &config);
	void aa3010(machine_config &config);
	void aa4(machine_config &config);
	void aa4000(machine_config &config);
	void aa540(machine_config &config);
	void aa440(machine_config &config);
	void aa4201(machine_config &config);

private:
	uint32_t fdc_r(offs_t offset);
	void fdc_w(offs_t offset, uint32_t data);
	uint32_t peripheral2_r(offs_t offset);
	uint32_t peripheral5_r(offs_t offset);
	void peripheral5_w(offs_t offset, uint32_t data);
	uint32_t dram_r(offs_t offset, uint32_t mem_mask = ~0);
	void dram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	DECLARE_READ_LINE_MEMBER(floppy_ready_r);
	DECLARE_WRITE_LINE_MEMBER(sound_mute_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	static void floppy_formats(format_registration &fr);

	void aa310_mem(address_map &map);
	void aa310_arm_mem(address_map &map);

	required_device<arm_cpu_device> m_maincpu;
	required_device<acorn_ioc_device> m_ioc;
	required_device<acorn_memc_device> m_memc;
	required_device<acorn_vidc10_device> m_vidc;
	required_device<wd1772_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<ram_device> m_ram;
	optional_ioport_array<2> m_joy;

	floppy_image_device *m_selected_floppy;
	std::unique_ptr<uint32_t[]> m_dram;
};

uint32_t aa310_state::fdc_r(offs_t offset)
{
	return m_fdc->read(offset & 0x03);
}

void aa310_state::fdc_w(offs_t offset, uint32_t data)
{
	return m_fdc->write(offset & 0x03, data & 0xff);
}

uint32_t aa310_state::peripheral2_r(offs_t offset)
{
	// RTFM joystick interface routes here
	// TODO: slot interface for econet (reads registers 0 and 1 during boot)
	switch ((offset << 2) & 0xffff)
	{
		case 0x00:
			return 0xed; // ID for econet
		case 0x04:
			return m_joy[0].read_safe(0xff);
		case 0x08:
			// Top Banana reads there and do various checks,
			// disallowing player 1 joy use if they fails (?)
			return m_joy[1].read_safe(0xff);
	}

	return 0xffffffff;
}

uint32_t aa310_state::peripheral5_r(offs_t offset)
{
	switch ((offset << 2) & 0xfc)
	{
	case 0x18: return 0x00; // FDC latch B
	case 0x40: return 0x00; // FDC latch A
	case 0x50: return 0x00; // FDC type, an 82c711 returns 5 here
	case 0x70: return 0x0f; // monitor type, TBD
	case 0x74: return 0xff; // unknown
	case 0x78:              // serial joystick?
	case 0x7c:
		logerror("FDC: reading Joystick port %04x at PC=%08x\n",offset << 2, m_maincpu->pc());
		return 0xff;

	}

	return 0xffffffff;
}

void aa310_state::peripheral5_w(offs_t offset, uint32_t data)
{
	switch ((offset << 2) & 0xfc)
	{
	case 0x00:  // HD63463
		break;

	case 0x08:  // HD63463 DACK
		break;

	case 0x10:  // Printer port
	{
		// compared to RTFM they reversed bits 0-3 (or viceversa, dunno what came out first)
		// for pragmatic convenience we bitswap here, but this should really be a slot option at some point.
		// TODO: understand how player 2 inputs routes, related somehow to CONTROL bit 6 (cfr. blitz in SW list)
		// TODO: paradr2k polls here with bit 7 and fails detection (Vertical Twist)
		uint8_t cur_joy_in = bitswap<8>(m_joy[0].read_safe(0xff),7,6,5,4,0,1,2,3);
		uint8_t joy_serial_data = (data & 0xff) ^ 0xff;
		bool serial_on = false;

		if (joy_serial_data == 0x20)
			serial_on = true;
		else if (joy_serial_data & cur_joy_in)
			serial_on = true;

		// wants printer irq for some reason (connected on parallel?)
		m_ioc->il6_w(serial_on ? ASSERT_LINE : CLEAR_LINE);
		break;
	}
	case 0x18: // Latch B
		// xxx- ---- Not used
		// ---x ---- Printer Strobe
		// ---- x--- FDC Reset
		// ---- --x- FDC DDEN
		m_fdc->dden_w(BIT(data, 1));
		m_fdc->mr_w(BIT(data, 3));
		break;

	case 0x40: // Latch A
		// x--- ---- Not used
		// -x-- ---- Floppy disc INUSE
		// --x- ---- Floppy motor
		// ---x ---- Side select
		// ---- xxxx Floppy disc select

		// Debug code to display RISC OS 3 POST failures
		const int post_debug = 0;
		if (post_debug && BIT(data, 17))
		{
			static attotime last_time(0, 0);
			static int bitpos = 0;

			if (BIT(data, 16) && bitpos <= 32)
			{
				bool state = (machine().time() - last_time) > m_maincpu->clocks_to_attotime(2000000);

				switch (32 - bitpos)
				{
				// Status flags
				case  0: printf("00000001  %-4s   Self-test due to power on\n"          , state ? "On" : "Off");   break;
				case  1: printf("00000002  %-4s   Self-test due to interface hardware\n", state ? "On" : "Off");   break;
				case  2: printf("00000004  %-4s   Self-test due to test link\n"         , state ? "On" : "Off");   break;
				case  3: printf("00000008  %-4s   Long memory test performed\n"         , state ? "On" : "Off");   break;
				case  4: printf("00000010  %-4s   ARM 3 fitted\n"                       , state ? "On" : "Off");   break;
				case  5: printf("00000020  %-4s   Long memory test disabled\n"          , state ? "On" : "Off");   break;
				case  6: printf("00000040  %-4s   PC-style IO world detected\n"         , state ? "On" : "Off");   break;
				case  7: printf("00000080  %-4s   VRAM detected\n"                      , state ? "On" : "Off");   break;

				// Fault flags
				case  8: printf("00000100  %-4s   CMOS RAM checksum error\n"            , state ? "Fail" : "Pass");   break;
				case  9: printf("00000200  %-4s   ROM failed checksum test\n"           , state ? "Fail" : "Pass");   break;
				case 10: printf("00000400  %-4s   MEMC CAM mapping failed\n"            , state ? "Fail" : "Pass");   break;
				case 11: printf("00000800  %-4s   MEMC protection failed\n"             , state ? "Fail" : "Pass");   break;
				case 12: printf("00001000  %-4s   IOC register test failed\n"           , state ? "Fail" : "Pass");   break;
				case 14: printf("00004000  %-4s   VIDC Virq timing failed\n"            , state ? "Fail" : "Pass");   break;
				case 15: printf("00008000  %-4s   VIDC Sirq timing failed\n"            , state ? "Fail" : "Pass");   break;
				case 16: printf("00010000  %-4s   CMOS unreadable\n"                    , state ? "Fail" : "Pass");   break;
				case 17: printf("00020000  %-4s   RAM control line failure\n"           , state ? "Fail" : "Pass");   break;
				case 18: printf("00040000  %-4s   Long RAM test failure\n"              , state ? "Fail" : "Pass");   break;
				}

				bitpos++;
			}

			last_time = machine().time();
		}

		if (!BIT(data, 0))     m_selected_floppy = m_floppy0->get_device();
		if (!BIT(data, 1))     m_selected_floppy = m_floppy1->get_device();
		if (!BIT(data, 2))     m_selected_floppy = nullptr; // floppy 2
		if (!BIT(data, 3))     m_selected_floppy = nullptr; // floppy 3

		m_fdc->set_floppy(m_selected_floppy);

		if (m_selected_floppy)
		{
			m_selected_floppy->mon_w(BIT(data, 5));
			m_selected_floppy->ss_w(!(BIT(data, 4)));
		}
		break;
	}
}

READ_LINE_MEMBER( aa310_state::floppy_ready_r )
{
	if (m_selected_floppy)
		return !m_selected_floppy->ready_r();

	return 0;
}

WRITE_LINE_MEMBER( aa310_state::sound_mute_w )
{
	// popmessage("Muting sound, contact MAME/MESSdev");
}

uint32_t aa310_state::dram_r(offs_t offset, uint32_t mem_mask)
{
	return m_dram[offset];
}

void aa310_state::dram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dram[offset]);
}

void aa310_state::machine_start()
{
	// LK14 and LK15 are used to configure the installed RAM, different configurations
	// create different memory mirrors that RISC OS uses to detect the available RAM.
	int dram_size = m_ram->size() / 1024;
	m_dram = nullptr;
	switch (dram_size)
	{
	case 512:   // 512 kB
	case 1024:  // 1 MB
		// for configurations with less than 2 MB it is necessary to use a custom handler in order to emulate the correct memory mirrors.
		m_memc->output_dram_rowcol(true);
		m_memc->space(0).install_readwrite_handler(0x02000000,0x021fffff, dram_size == 512 ? 0x000ff7ff : 0x001ff7ff, 0x00e00000, 0,
				read32s_delegate (*this, FUNC(aa310_state::dram_r)),
				write32s_delegate(*this, FUNC(aa310_state::dram_w)));

		// for better performance we allocate 2 MB and mask out unused lines in the handlers
		m_dram = std::make_unique<uint32_t[]>(2048 * 1024 / 4);
		save_pointer(NAME(m_dram), 2048 * 1024 / 4);
		break;
	case 2048:  // 2 MB
		m_memc->space(0).install_ram(0x02000000, 0x021fffff, 0x00e00000, m_ram->pointer());
		break;
	case 4096:  // 4 MB
		m_memc->space(0).install_ram(0x02000000, 0x023fffff, 0x00c00000, m_ram->pointer());
		break;
	case 4096 * 2:  // 8 MB
		m_memc->space(0).install_ram(0x02000000, 0x027fffff, 0x00800000, m_ram->pointer());
		break;
	case 4096 * 3:  // 12 MB
		m_memc->space(0).install_ram(0x02000000, 0x02bfffff, 0x00000000, m_ram->pointer());
		break;
	case 4096 * 4:  // 16 MB
		m_memc->space(0).install_ram(0x02000000, 0x02ffffff, 0x00000000, m_ram->pointer());
		break;
	default:
		fatalerror("Archimedes %d kB RAM not supported", dram_size);
		break;
	}
}

void aa310_state::machine_reset()
{
	m_selected_floppy = m_floppy0->get_device();
}

void aa310_state::aa310_arm_mem(address_map &map)
{
	map(0x00000000, 0x01ffffff).rw(m_memc, FUNC(acorn_memc_device::logical_r), FUNC(acorn_memc_device::logical_w));
	map(0x02000000, 0x03ffffff).rw(m_memc, FUNC(acorn_memc_device::high_mem_r), FUNC(acorn_memc_device::high_mem_w));
}

void aa310_state::aa310_mem(address_map &map)
{
	map(0x00000000, 0x01ffffff).rw(m_memc, FUNC(acorn_memc_device::logical_r), FUNC(acorn_memc_device::logical_w));
	map(0x02000000, 0x02ffffff).noprw();    // installed in machine_start
	map(0x03000000, 0x033fffff).m(m_ioc, FUNC(acorn_ioc_device::map));
	map(0x03400000, 0x035fffff).rom().region("extension", 0x000000).w(m_vidc, FUNC(acorn_vidc10_device::write));
	map(0x03600000, 0x037fffff).rom().region("extension", 0x200000).w(m_memc, FUNC(acorn_memc_device::registers_w));
	map(0x03800000, 0x03ffffff).rom().region("maincpu", 0).w(m_memc, FUNC(acorn_memc_device::page_w));
}


static INPUT_PORTS_START( aa310 )
	// standard Atari/Commodore DB9
	// TODO: 10 different joystick configurations (!), some of them supports multiple buttons as well
	PORT_START("joy_p1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("joy_p2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x60, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END

void aa310_state::floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();

	fr.add(FLOPPY_ACORN_ADFS_NEW_FORMAT);
	fr.add(FLOPPY_ACORN_ADFS_OLD_FORMAT);
	fr.add(FLOPPY_APD_FORMAT);
	fr.add(FLOPPY_JFD_FORMAT);
}

static void aa310_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("35hd", FLOPPY_35_HD);
}


void aa310_state::aa310(machine_config &config)
{
	/* basic machine hardware */
	ARM(config, m_maincpu, 24_MHz_XTAL / 3); /* ARM2 8 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &aa310_state::aa310_arm_mem);
	m_maincpu->set_copro_type(arm_cpu_device::copro_type::VL86C020);

	ACORN_MEMC(config, m_memc, 24_MHz_XTAL / 3, m_vidc);
	m_memc->set_addrmap(0, &aa310_state::aa310_mem);
	m_memc->sirq_w().set(m_ioc, FUNC(acorn_ioc_device::il1_w));

	ACORN_IOC(config, m_ioc, 24_MHz_XTAL / 3);
	m_ioc->fiq_w().set_inputline(m_maincpu, ARM_FIRQ_LINE);
	m_ioc->irq_w().set_inputline(m_maincpu, ARM_IRQ_LINE);
	m_ioc->kout_w().set("keyboard", FUNC(archimedes_keyboard_device::kin_w));
	m_ioc->peripheral_r<1>().set(FUNC(aa310_state::fdc_r));
	m_ioc->peripheral_w<1>().set(FUNC(aa310_state::fdc_w));
	m_ioc->peripheral_r<2>().set(FUNC(aa310_state::peripheral2_r));
	m_ioc->peripheral_w<2>().set_log("IOC: Econet W");
	m_ioc->peripheral_r<3>().set_log("IOC: Serial R");
	m_ioc->peripheral_w<3>().set_log("IOC: Serial W");
	m_ioc->peripheral_r<4>().set_log("IOC: Podule R");
	m_ioc->peripheral_w<4>().set_log("IOC: Podule W");
	m_ioc->peripheral_r<5>().set(FUNC(aa310_state::peripheral5_r));
	m_ioc->peripheral_w<5>().set(FUNC(aa310_state::peripheral5_w));
	m_ioc->peripheral_r<6>().set_log("IOC: External Expansion R");
	m_ioc->peripheral_w<6>().set_log("IOC: External Expansion W");
	m_ioc->gpio_r<0>().set("i2cmem", FUNC(pcf8583_device::sda_r));
	m_ioc->gpio_w<0>().set("i2cmem", FUNC(pcf8583_device::sda_w));
	m_ioc->gpio_w<1>().set("i2cmem", FUNC(pcf8583_device::scl_w));
	m_ioc->gpio_r<2>().set(FUNC(aa310_state::floppy_ready_r));
	m_ioc->gpio_w<5>().set(FUNC(aa310_state::sound_mute_w));

	ARCHIMEDES_KEYBOARD(config, "keyboard").kout().set(m_ioc, FUNC(acorn_ioc_device::kin_w));

	PCF8583(config, "i2cmem", 32.768_kHz_XTAL);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.screen_vblank().set(m_ioc, FUNC(acorn_ioc_device::ir_w));

	ACORN_VIDC10(config, m_vidc, 24_MHz_XTAL);
	m_vidc->set_screen("screen");
	m_vidc->vblank().set(m_memc, FUNC(acorn_memc_device::vidrq_w));
	m_vidc->sound_drq().set(m_memc, FUNC(acorn_memc_device::sndrq_w));

	RAM(config, m_ram).set_default_size("1M");

	wd1772_device& fdc(WD1772(config, m_fdc, 24_MHz_XTAL / 3));
	fdc.set_disable_motor_control(true);
	fdc.intrq_wr_callback().set(m_ioc, FUNC(acorn_ioc_device::fh1_w));
	fdc.drq_wr_callback().set(m_ioc, FUNC(acorn_ioc_device::fh0_w));
	FLOPPY_CONNECTOR(config, m_floppy0, aa310_floppies, "35dd", aa310_state::floppy_formats).enable_sound(true);

	// rarely had 2nd FDD installed, space was used for HDD
	FLOPPY_CONNECTOR(config, m_floppy1, aa310_floppies, nullptr, aa310_state::floppy_formats).enable_sound(true);

	SOFTWARE_LIST(config, "flop_list").set_original("archimedes");

	/* Expansion slots - 2-card backplane */
}

void aa310_state::aa305(machine_config &config)
{
	aa310(config);
	m_ram->set_default_size("512K").set_extra_options("1M");
}

void aa310_state::aa440(machine_config &config)
{
	aa310(config);
	m_ram->set_default_size("4M");

	/* 20MB HDD */

	/* Expansion slots - 4-card backplane */
}

void aa310_state::aa3000(machine_config &config)
{
	aa310(config);
	m_ram->set_default_size("1M").set_extra_options("2M");
}

void aa310_state::aa4101(machine_config &config)
{
	aa310(config);
	m_ram->set_default_size("1M").set_extra_options("2M,4M");

	/* Expansion slots - 4-card backplane */
}

void aa310_state::aa4201(machine_config &config)
{
	aa310(config);
	m_ram->set_default_size("2M").set_extra_options("4M");

	/* 20MB HDD */

	/* Expansion slots - 4-card backplane */
}

void aa310_state::aa4401(machine_config &config)
{
	aa310(config);
	m_ram->set_default_size("4M").set_extra_options("8M");

	/* 50MB HDD */

	/* Expansion slots - 4-card backplane */
}

void aa310_state::aa540(machine_config &config)
{
	aa310(config);
	m_maincpu->set_clock(52_MHz_XTAL / 2); // ARM3

	m_ram->set_default_size("4M").set_extra_options("8M,12M,16M");

	/* 100MB HDD */

	/* Expansion slots - 4-card backplane */
}

void aa310_state::aa5000(machine_config &config)
{
	aa310(config);
	m_maincpu->set_clock(50_MHz_XTAL / 2); // ARM3

	m_ram->set_default_size("2M").set_extra_options("4M");

	/* 80MB HDD */

	/* Expansion slots - 4-card backplane */
}

void aa310_state::aa4(machine_config &config)
{
	aa5000(config);
	m_maincpu->set_clock(24_MHz_XTAL); // ARM3

	/* video hardware */
	screen_device &screen(SCREEN(config.replace(), "screen", SCREEN_TYPE_LCD));
	screen.screen_vblank().set(m_ioc, FUNC(acorn_ioc_device::ir_w));

	ACORN_VIDC10_LCD(config.replace(), m_vidc, 24_MHz_XTAL);
	m_vidc->set_screen("screen");
	m_vidc->vblank().set(m_memc, FUNC(acorn_memc_device::vidrq_w));
	m_vidc->sound_drq().set(m_memc, FUNC(acorn_memc_device::sndrq_w));

	/* 765 FDC */

	/* 60MB HDD */
}

void aa310_state::aa5000a(machine_config &config)
{
	aa5000(config);
	m_maincpu->set_clock(33000000); // ARM3
}

void aa310_state::aa3010(machine_config &config)
{
	aa310(config);
	m_maincpu->set_clock(72_MHz_XTAL / 6); // ARM250

	m_ram->set_default_size("1M").set_extra_options("2M");
}

void aa310_state::aa3020(machine_config &config)
{
	aa3010(config);
	m_ram->set_default_size("2M").set_extra_options("4M");
}

void aa310_state::aa4000(machine_config &config)
{
	aa3010(config);
	m_ram->set_default_size("2M").set_extra_options("4M");

	/* 80MB HDD */

	/* Expansion slots - 4-card backplane */
}

ROM_START( aa305 )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "120", "Arthur 1.20 (25 Sep 1987)" ) // Parts 0277,022-02, 0277,023-03, 0277,024-02, 0277,025-02,
	ROMX_LOAD( "arthur120.bin", 0x000000, 0x80000, CRC(eb3fda57) SHA1(1181ff9c2c2f3d6d414054ec33b2260404bafc81), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "030", "Arthur 0.30 (17 Jun 1987)" ) // Parts 0276,322-01, 0276,323-01, 0276,324-01, 0276,325-01,
	ROMX_LOAD( "arthur030.bin", 0x000000, 0x80000, CRC(5df8ed42) SHA1(6aebd686d97dfdf6726fa5f3246ef35b840b286d), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "200", "RISC OS 2.00 (05 Oct 1988)" )
	ROMX_LOAD( "0283,022-01.ic24", 0x000000, 0x20000, CRC(52186922) SHA1(06a7ca871407fec7b55b56ed1b419d10ced03e72), ROM_BIOS(2) )
	ROMX_LOAD( "0283,023-01.ic25", 0x020000, 0x20000, CRC(b8284110) SHA1(021f29e09a5ad994f71992fb6118d78069c545bf), ROM_BIOS(2) )
	ROMX_LOAD( "0283,024-01.ic26", 0x040000, 0x20000, CRC(b80d1a22) SHA1(6b8cb24f7f7095c0c64e62697553fb5294146135), ROM_BIOS(2) )
	ROMX_LOAD( "0283,025-01.ic27", 0x060000, 0x20000, CRC(ad1d4715) SHA1(f9c5a771ae5dfbc4b19c95e2e7423166735e8697), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "201", "RISC OS 2.01 (05 Jul 1990)" ) // Parts 0270,601-01, 0270,602-01, 0270,603-01, 0270,604-01,
	ROMX_LOAD( "riscos201.bin", 0x000000, 0x080000, CRC(7cb5ea3f) SHA1(9ed7dee8553d96a6d58dd23d31682234f57beb62), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "300", "RISC OS 3.00 (25 Sep 1991)" )
	ROMX_LOAD( "0270,251-01.ic24", 0x000000, 0x80000, CRC(44af65d3) SHA1(7097f39722ed10f3b2c27805cb8866c14f878a7b), ROM_BIOS(4) )
	ROMX_LOAD( "0270,252-01.ic25", 0x080000, 0x80000, CRC(b4a28cba) SHA1(3b8c8fa5068ebab74d2c229c1582de67eef81ac9), ROM_BIOS(4) )
	ROMX_LOAD( "0270,253-01.ic26", 0x100000, 0x80000, CRC(a3661f66) SHA1(5d4ddd776945321a07c9e59b6b24b90815a3e861), ROM_BIOS(4) )
	ROMX_LOAD( "0270,254-01.ic27", 0x180000, 0x80000, CRC(3aae93ad) SHA1(13efdd4a09aad1a3a4cd39f81994d68be511232e), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "310", "RISC OS 3.10 (30 Apr 1992)" )
	ROMX_LOAD( "0296,041-01.ic24", 0x000000, 0x80000, CRC(3aab9849) SHA1(ce910fd0c53fef609a5ab70f251da798c10235c0), ROM_BIOS(5) )
	ROMX_LOAD( "0296,042-01.ic25", 0x080000, 0x80000, CRC(15d89664) SHA1(78f5d0e6f1e8ee603317807f53ff8fe65a3b3518), ROM_BIOS(5) )
	ROMX_LOAD( "0296,043-01.ic26", 0x100000, 0x80000, CRC(a81ceb7c) SHA1(46b870876bc1f68f242726415f0c49fef7be0c72), ROM_BIOS(5) )
	ROMX_LOAD( "0296,044-01.ic27", 0x180000, 0x80000, CRC(fc1badad) SHA1(fc1326cf949ec54ef6ef32cf928b996f35b69b50), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "311", "RISC OS 3.11 (29 Sep 1992)" )
	ROMX_LOAD( "0296,041-02.ic24", 0x000000, 0x80000, CRC(c1adde84) SHA1(12d060e0401dd0523d44453f947bdc55dd2c3240), ROM_BIOS(6) )
	ROMX_LOAD( "0296,042-02.ic25", 0x080000, 0x80000, CRC(15d89664) SHA1(78f5d0e6f1e8ee603317807f53ff8fe65a3b3518), ROM_BIOS(6) )
	ROMX_LOAD( "0296,043-02.ic26", 0x100000, 0x80000, CRC(a81ceb7c) SHA1(46b870876bc1f68f242726415f0c49fef7be0c72), ROM_BIOS(6) )
	ROMX_LOAD( "0296,044-02.ic27", 0x180000, 0x80000, CRC(707b0c6c) SHA1(345199a33fed23996374b9db8170a52ab63f0380), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 7, "319", "RISC OS 3.19 (09 Jun 1993)" ) // Parts 0296,241-01, 0296,242-01, 0296,243-01, 0296,244-01,
	ROMX_LOAD( "riscos319.bin", 0x000000, 0x200000, CRC(00c7a3d3) SHA1(be7a8cba5d6c6c0e1c4838712524056cf4b8c8cb), ROM_BIOS(7) )
	ROM_SYSTEM_BIOS( 8, "test", "Diagnostic Test ROMs" ) // Usage described in Archimedes 300 Series Service Manual
	ROMX_LOAD( "0276,146-01.ic24", 0x000000, 0x10000, CRC(9c45283c) SHA1(9eb5bd7ad0958f194a3416d79d7e01e4c45741e1), ROM_BIOS(8) | ROM_SKIP(3) )
	ROMX_LOAD( "0276,147-01.ic25", 0x000001, 0x10000, CRC(ad94e17f) SHA1(1c8e39c69d4ae1b674e0f732aaa62a4403998f41), ROM_BIOS(8) | ROM_SKIP(3) )
	ROMX_LOAD( "0276,148-01.ic26", 0x000002, 0x10000, CRC(1ab02f2d) SHA1(dd7d216967524e64d1a03076a6081461ec8528c3), ROM_BIOS(8) | ROM_SKIP(3) )
	ROMX_LOAD( "0276,149-01.ic27", 0x000003, 0x10000, CRC(5fd6a406) SHA1(790af8a4c74d0f6714d528f7502443ce5898a618), ROM_BIOS(8) | ROM_SKIP(3) )


	ROM_REGION32_LE( 0x400000, "extension", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROMX_LOAD( "cmos_arthur.bin",  0x0000, 0x0100, CRC(4be5a84f) SHA1(2a6e52af01e23665a884f4693e2a397d731f7e4a), ROM_BIOS(0) )
	ROMX_LOAD( "cmos_arthur.bin",  0x0000, 0x0100, CRC(4be5a84f) SHA1(2a6e52af01e23665a884f4693e2a397d731f7e4a), ROM_BIOS(1) )
	ROMX_LOAD( "cmos_riscos2.bin", 0x0000, 0x0100, CRC(763b14e3) SHA1(0ccd41648a798ba4a5d92c19c24b605a8927fb76), ROM_BIOS(2) )
	ROMX_LOAD( "cmos_riscos2.bin", 0x0000, 0x0100, CRC(763b14e3) SHA1(0ccd41648a798ba4a5d92c19c24b605a8927fb76), ROM_BIOS(3) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5), ROM_BIOS(4) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5), ROM_BIOS(5) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5), ROM_BIOS(6) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5), ROM_BIOS(7) )
ROM_END

#define rom_aa310 rom_aa305
#define rom_aa440 rom_aa305

ROM_START( aa3000 )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "200", "RISC OS 2.00 (05 Oct 1988)" )
	ROMX_LOAD( "0283,022-01.ic14", 0x000000, 0x20000, CRC(52186922) SHA1(06a7ca871407fec7b55b56ed1b419d10ced03e72), ROM_BIOS(0) )
	ROMX_LOAD( "0283,023-01.ic15", 0x020000, 0x20000, CRC(b8284110) SHA1(021f29e09a5ad994f71992fb6118d78069c545bf), ROM_BIOS(0) )
	ROMX_LOAD( "0283,024-01.ic16", 0x040000, 0x20000, CRC(b80d1a22) SHA1(6b8cb24f7f7095c0c64e62697553fb5294146135), ROM_BIOS(0) )
	ROMX_LOAD( "0283,025-01.ic17", 0x060000, 0x20000, CRC(ad1d4715) SHA1(f9c5a771ae5dfbc4b19c95e2e7423166735e8697), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "201", "RISC OS 2.01 (05 Jul 1990)" ) // Parts 0270,601-01, 0270,602-01, 0270,603-01, 0270,604-01,
	ROMX_LOAD( "riscos201.bin", 0x000000, 0x080000, CRC(7cb5ea3f) SHA1(9ed7dee8553d96a6d58dd23d31682234f57beb62), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "300", "RISC OS 3.00 (25 Sep 1991)" )
	ROMX_LOAD( "0270,251-01.ic14", 0x000000, 0x80000, CRC(44af65d3) SHA1(7097f39722ed10f3b2c27805cb8866c14f878a7b), ROM_BIOS(2) )
	ROMX_LOAD( "0270,252-01.ic15", 0x080000, 0x80000, CRC(b4a28cba) SHA1(3b8c8fa5068ebab74d2c229c1582de67eef81ac9), ROM_BIOS(2) )
	ROMX_LOAD( "0270,253-01.ic16", 0x100000, 0x80000, CRC(a3661f66) SHA1(5d4ddd776945321a07c9e59b6b24b90815a3e861), ROM_BIOS(2) )
	ROMX_LOAD( "0270,254-01.ic17", 0x180000, 0x80000, CRC(3aae93ad) SHA1(13efdd4a09aad1a3a4cd39f81994d68be511232e), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "310", "RISC OS 3.10 (30 Apr 1992)" )
	ROMX_LOAD( "0296,041-01.ic14", 0x000000, 0x80000, CRC(3aab9849) SHA1(ce910fd0c53fef609a5ab70f251da798c10235c0), ROM_BIOS(3) )
	ROMX_LOAD( "0296,042-01.ic15", 0x080000, 0x80000, CRC(15d89664) SHA1(78f5d0e6f1e8ee603317807f53ff8fe65a3b3518), ROM_BIOS(3) )
	ROMX_LOAD( "0296,043-01.ic16", 0x100000, 0x80000, CRC(a81ceb7c) SHA1(46b870876bc1f68f242726415f0c49fef7be0c72), ROM_BIOS(3) )
	ROMX_LOAD( "0296,044-01.ic17", 0x180000, 0x80000, CRC(fc1badad) SHA1(fc1326cf949ec54ef6ef32cf928b996f35b69b50), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "311", "RISC OS 3.11 (29 Sep 1992)" )
	ROMX_LOAD( "0296,041-02.ic14", 0x000000, 0x80000, CRC(c1adde84) SHA1(12d060e0401dd0523d44453f947bdc55dd2c3240), ROM_BIOS(4) )
	ROMX_LOAD( "0296,042-02.ic15", 0x080000, 0x80000, CRC(15d89664) SHA1(78f5d0e6f1e8ee603317807f53ff8fe65a3b3518), ROM_BIOS(4) )
	ROMX_LOAD( "0296,043-02.ic16", 0x100000, 0x80000, CRC(a81ceb7c) SHA1(46b870876bc1f68f242726415f0c49fef7be0c72), ROM_BIOS(4) )
	ROMX_LOAD( "0296,044-02.ic17", 0x180000, 0x80000, CRC(707b0c6c) SHA1(345199a33fed23996374b9db8170a52ab63f0380), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "319", "RISC OS 3.19 (09 Jun 1993)" ) // Parts 0296,241-01, 0296,242-01, 0296,243-01, 0296,244-01,
	ROMX_LOAD( "riscos319.bin", 0x000000, 0x200000, CRC(00c7a3d3) SHA1(be7a8cba5d6c6c0e1c4838712524056cf4b8c8cb), ROM_BIOS(5) )


	ROM_REGION32_LE( 0x400000, "extension", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROMX_LOAD( "cmos_riscos2.bin", 0x0000, 0x0100, CRC(763b14e3) SHA1(0ccd41648a798ba4a5d92c19c24b605a8927fb76), ROM_BIOS(0) )
	ROMX_LOAD( "cmos_riscos2.bin", 0x0000, 0x0100, CRC(763b14e3) SHA1(0ccd41648a798ba4a5d92c19c24b605a8927fb76), ROM_BIOS(1) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5), ROM_BIOS(2) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5), ROM_BIOS(3) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5), ROM_BIOS(4) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5), ROM_BIOS(5) )
ROM_END

#define rom_aa4101 rom_aa3000 // ROMs in IC16, IC17, IC18, IC19
#define rom_aa4201 rom_aa3000 // ROMs in IC16, IC17, IC18, IC19
#define rom_aa4401 rom_aa3000 // ROMs in IC16, IC17, IC18, IC19
#define rom_aa540  rom_aa3000 // ROMs in IC47, IC48, IC49, IC50

ROM_START( aa5000 )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "300", "RISC OS 3.00 (25 Sep 1991)" )
	ROMX_LOAD( "0270,251-01.ic27", 0x000000, 0x80000, CRC(44af65d3) SHA1(7097f39722ed10f3b2c27805cb8866c14f878a7b), ROM_BIOS(0) )
	ROMX_LOAD( "0270,252-01.ic28", 0x080000, 0x80000, CRC(b4a28cba) SHA1(3b8c8fa5068ebab74d2c229c1582de67eef81ac9), ROM_BIOS(0) )
	ROMX_LOAD( "0270,253-01.ic29", 0x100000, 0x80000, CRC(a3661f66) SHA1(5d4ddd776945321a07c9e59b6b24b90815a3e861), ROM_BIOS(0) )
	ROMX_LOAD( "0270,254-01.ic30", 0x180000, 0x80000, CRC(3aae93ad) SHA1(13efdd4a09aad1a3a4cd39f81994d68be511232e), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "310", "RISC OS 3.10 (30 Apr 1992)" )
	ROMX_LOAD( "0296,041-01.ic27", 0x000000, 0x80000, CRC(3aab9849) SHA1(ce910fd0c53fef609a5ab70f251da798c10235c0), ROM_BIOS(1) )
	ROMX_LOAD( "0296,042-01.ic28", 0x080000, 0x80000, CRC(15d89664) SHA1(78f5d0e6f1e8ee603317807f53ff8fe65a3b3518), ROM_BIOS(1) )
	ROMX_LOAD( "0296,043-01.ic29", 0x100000, 0x80000, CRC(a81ceb7c) SHA1(46b870876bc1f68f242726415f0c49fef7be0c72), ROM_BIOS(1) )
	ROMX_LOAD( "0296,044-01.ic30", 0x180000, 0x80000, CRC(fc1badad) SHA1(fc1326cf949ec54ef6ef32cf928b996f35b69b50), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "311", "RISC OS 3.11 (29 Sep 1992)" )
	ROMX_LOAD( "0296,041-02.ic27", 0x000000, 0x80000, CRC(c1adde84) SHA1(12d060e0401dd0523d44453f947bdc55dd2c3240), ROM_BIOS(2) )
	ROMX_LOAD( "0296,042-02.ic28", 0x080000, 0x80000, CRC(15d89664) SHA1(78f5d0e6f1e8ee603317807f53ff8fe65a3b3518), ROM_BIOS(2) )
	ROMX_LOAD( "0296,043-02.ic29", 0x100000, 0x80000, CRC(a81ceb7c) SHA1(46b870876bc1f68f242726415f0c49fef7be0c72), ROM_BIOS(2) )
	ROMX_LOAD( "0296,044-02.ic30", 0x180000, 0x80000, CRC(707b0c6c) SHA1(345199a33fed23996374b9db8170a52ab63f0380), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "319", "RISC OS 3.19 (09 Jun 1993)" ) // Parts 0296,241-01, 0296,242-01, 0296,243-01, 0296,244-01,
	ROMX_LOAD( "riscos319.bin", 0x000000, 0x200000, CRC(00c7a3d3) SHA1(be7a8cba5d6c6c0e1c4838712524056cf4b8c8cb), ROM_BIOS(3) )


	ROM_REGION32_LE( 0x400000, "extension", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROM_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5) )
ROM_END

#define rom_aa5000a rom_aa5000

ROM_START( aa4 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	// RISC OS 3.10 (30 Apr 1992)
	ROM_LOAD32_WORD( "0296,061-01.ic4",  0x000000, 0x100000, CRC(b77fe215) SHA1(57b19ea4b97a9b6a240aa61211c2c134cb295aa0) )
	ROM_LOAD32_WORD( "0296,062-01.ic15", 0x000002, 0x100000, CRC(d42e196e) SHA1(64243d39d1bca38b10761f66a8042c883bde87a4) )


	ROM_REGION32_LE( 0x400000, "extension", ROMREGION_ERASE00 )
	/* Power Management */
	ROM_LOAD32_BYTE( "0296,063-01.ic38", 0x000003, 0x010000, CRC(9ca3a6be) SHA1(75905b031f49960605d55c3e7350d309559ed440) )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROM_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5) )
ROM_END

ROM_START( aa3010 )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASEFF )
	// RISC OS 3.11 (29 Sep 1992)
	ROM_LOAD32_WORD( "0296,061-02.ic17", 0x000000, 0x100000, CRC(552fc3aa) SHA1(b2f1911e53d7377f2e69e1a870139745d3df494b) )
	ROM_LOAD32_WORD( "0296,062-02.ic18", 0x000002, 0x100000, CRC(308d5a4a) SHA1(b309e1dd85670a06d77ec504dbbec6c42336329f) )


	ROM_REGION32_LE( 0x400000, "extension", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROM_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(0da2d31d) SHA1(4a5277f27e23f0eae9daa8cc5a4818a322f579a5))
ROM_END

#define rom_aa3020 rom_aa3010
#define rom_aa4000 rom_aa3010

} // anonymous namespace


/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME             FLAGS */
COMP( 1987, aa305,   aa310,  0,      aa305,   aa310, aa310_state, empty_init, "Acorn", "Archimedes 305",    MACHINE_NOT_WORKING)
COMP( 1987, aa310,   0,      0,      aa310,   aa310, aa310_state, empty_init, "Acorn", "Archimedes 310",    MACHINE_NOT_WORKING)
COMP( 1987, aa440,   aa310,  0,      aa440,   aa310, aa310_state, empty_init, "Acorn", "Archimedes 440",    MACHINE_NOT_WORKING)
COMP( 1989, aa3000,  aa310,  0,      aa3000,  aa310, aa310_state, empty_init, "Acorn", "BBC A3000",         MACHINE_NOT_WORKING)
COMP( 1989, aa4101,  aa310,  0,      aa4101,  aa310, aa310_state, empty_init, "Acorn", "Archimedes 410/1",  MACHINE_NOT_WORKING)
COMP( 1989, aa4201,  aa310,  0,      aa4201,  aa310, aa310_state, empty_init, "Acorn", "Archimedes 420/1",  MACHINE_NOT_WORKING)
COMP( 1989, aa4401,  aa310,  0,      aa4401,  aa310, aa310_state, empty_init, "Acorn", "Archimedes 440/1",  MACHINE_NOT_WORKING)
COMP( 1990, aa540,   aa310,  0,      aa540,   aa310, aa310_state, empty_init, "Acorn", "Archimedes 540",    MACHINE_NOT_WORKING)
COMP( 1991, aa5000,  0,      0,      aa5000,  aa310, aa310_state, empty_init, "Acorn", "Acorn A5000",       MACHINE_NOT_WORKING)
COMP( 1992, aa4,     aa5000, 0,      aa4,     aa310, aa310_state, empty_init, "Acorn", "Acorn A4",          MACHINE_NOT_WORKING)
COMP( 1992, aa3010,  aa4000, 0,      aa3010,  aa310, aa310_state, empty_init, "Acorn", "Acorn A3010",       MACHINE_NOT_WORKING)
COMP( 1992, aa3020,  aa4000, 0,      aa3020,  aa310, aa310_state, empty_init, "Acorn", "Acorn A3020",       MACHINE_NOT_WORKING)
COMP( 1992, aa4000,  0,      0,      aa4000,  aa310, aa310_state, empty_init, "Acorn", "Acorn A4000",       MACHINE_NOT_WORKING)
COMP( 1993, aa5000a, aa5000, 0,      aa5000a, aa310, aa310_state, empty_init, "Acorn", "Acorn A5000 Alpha", MACHINE_NOT_WORKING)
