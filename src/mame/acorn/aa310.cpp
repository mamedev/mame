// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont, Juergen Buchmueller, Nigel Barnes
/******************************************************************************
 *
 *  Acorn Archimedes
 *
 *  AKB10 - Archimedes 305
 *  AKB15 - Archimedes 310
 *  AKB20 - Archimedes 440
 *  AKB26 - Archimedes 410 (advertised but not known to be produced)
 *  AKB01 - BBC A3000
 *  AKB40 - Archimedes 410/1
 *  AKB42 - Archimedes 420/1
 *  AKB44 - Archimedes 440/1
 *  AKB50 - Archimedes 540
 *  ALB22 - Acorn A5000 2MB HD 80
 *  ALB24 - Acorn A5000 4MB HD 120
 *  AKB62 - Acorn A4 2MB FD
 *  AKB64 - Acorn A4 4MB HD 60
 *  AGB11 - Acorn A3010
 *  AGB22 - Acorn A3020 FD
 *  AGB23 - Acorn A3020 HD 60
 *  AGB33 - Acorn A3020 HD 80
 *  AGC10 - Acorn A4000
 *  AGC20 - Acorn A4000 2MB HD 80
 *  UNX11 - R140 Workstation
 *  UNX22 - R225 Discless Workstation
 *  UNX26 - R260 8MB Workstation
 *
 *  Supplied IDE Hard Drives:
 *   60MB Connor CP2064
 *   85MB Conner CP30084E
 *   99MB Fujitsu M2616ET
 *  105MB Connor CFS105A
 *  210MB Connor CFS210A
 *
 * Notes:
 * - Hold DEL down during boot reset the CMOS memory to the default values.
 * - default NVRAM is plainly wrong. Use the status/configure commands to set up properly
 *   (Scroll Lock is currently mapped with Right SHIFT, use this to move to next page of status).
 *   In order to load a floppy, you need at very least:
 *   - configure floppies 1
 *   - configure filesystem adfs
 *   - configure monitortype 12
 *   Then reboot / reset the machine, and use cat to (attempt) to load a floppy contents.
 *
 *  TODO:
 *  - RISC OS Draw app uses unimplemented copro instructions
 *  - Add ABORT line support to the ARM core.
 *  - HD63463 Hard disc controller.
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
=======================================================================================
 *
 *  Archimedes IOC interrupts:
 *     IL0    Podule FIQ
 *     IL1    Sound Empty
 *     IL2    Serial
 *     IL3    HDD
 *     IL4    Disc Change
 *     IL5    Podule IRQ
 *     IL6    Printer Busy
 *     IL7    Serial Ring
 *     IF     Printer Ack
 *     IR     VBL
 *     POR    Reset
 *     FH0    Floppy DRQ
 *     FH1    Floppy IRQ
 *     FL     Econet
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

#include "bus/archimedes/econet/slot.h"
#include "bus/archimedes/podule/slot.h"
//#include "bus/archimedes/test/slot.h"
#include "bus/centronics/ctronics.h"
#include "bus/centronics/spjoy.h"
#include "bus/econet/econet.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "bus/nscsi/devices.h"
#include "bus/rs232/rs232.h"
#include "cpu/arm/arm.h"
#include "formats/acorn_dsk.h"
#include "formats/apd_dsk.h"
#include "formats/jfd_dsk.h"
#include "formats/st_dsk.h"
#include "imagedev/floppy.h"
#include "imagedev/harddriv.h"
#include "machine/acorn_bmu.h"
#include "machine/acorn_ioc.h"
#include "machine/acorn_lc.h"
#include "machine/acorn_memc.h"
#include "machine/acorn_vidc.h"
#include "machine/archimedes_keyb.h"
#include "machine/ds2401.h"
//#include "machine/hd63463.h"
#include "machine/i2cmem.h"
#include "machine/mc6854.h"
#include "machine/mos6551.h"
#include "machine/pcf8573.h"
#include "machine/pcf8583.h"
#include "machine/ram.h"
#include "machine/upc82c710.h"
#include "machine/upc82c711.h"
#include "machine/wd_fdc.h"
#include "machine/wd33c9x.h"
#include "machine/z80scc.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#define LOG_POST (1U << 1)

#define VERBOSE (LOG_POST)
#include "logmacro.h"


namespace {

class aabase_state : public driver_device
{
public:
	aabase_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ioc(*this, "ioc")
		, m_memc(*this, "memc")
		, m_vidc(*this, "vidc")
		, m_exp(*this, "exp")
		, m_podule(*this, "podule%u", 0U)
		, m_ram(*this, RAM_TAG)
		, m_joy(*this, "joy_p%u", 1U)
		, m_selected_floppy(nullptr)
	{ }

	void aabase(machine_config &config);

	void init_r225();
	void init_flop();
	void init_hd();
	void init_scsi();
	void init_ide();
	void init_a4();

protected:
	u32 dram_r(offs_t offset, u32 mem_mask = ~0);
	void dram_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	virtual void machine_start() override ATTR_COLD;

	static void floppy_formats(format_registration &fr);

	void arm_map(address_map &map) ATTR_COLD;
	virtual void memc_map(address_map &map) ATTR_COLD;

	void post_debug(int post_state);

	required_device<arm_cpu_device> m_maincpu;
	required_device<acorn_ioc_device> m_ioc;
	required_device<acorn_memc_device> m_memc;
	required_device<acorn_vidc10_device> m_vidc;
	required_device<archimedes_exp_device> m_exp;
	optional_device_array<archimedes_podule_slot_device, 4> m_podule;
	required_device<ram_device> m_ram;
	optional_ioport_array<2> m_joy;

	floppy_image_device *m_selected_floppy;
	std::unique_ptr<u32[]> m_dram;
};


class aa500_state : public aabase_state
{
public:
	aa500_state(const machine_config &mconfig, device_type type, const char *tag)
		: aabase_state(mconfig, type, tag)
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		//, m_hdc(*this, "hdc")
		, m_adlc(*this, "adlc")
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_i2cmem(*this, "i2cmem")
		, m_rtc(*this, "rtc")
	{ }

	void aa500(machine_config &config);
	void aa500d(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;

	u32 peripheral5_r(offs_t offset);
	void peripheral5_w(offs_t offset, u32 data);
	void peripheral6_w(offs_t offset, u32 data);

private:
	required_device<fd1793_device> m_fdc;
	optional_device_array<floppy_connector, 2> m_floppy;
	//required_device<hd63463_device> m_hdc;
	required_device<mc6854_device> m_adlc;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<i2c_pcf8570_device> m_i2cmem;
	required_device<pcf8573_device> m_rtc;
};


class aa310_state : public aabase_state
{
public:
	aa310_state(const machine_config &mconfig, device_type type, const char *tag)
		: aabase_state(mconfig, type, tag)
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		//, m_hdc(*this, "hdc")
		//, m_test(*this, "test")
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
	{ }

	void aa305(machine_config &config);
	void aa310(machine_config &config);
	void aa440(machine_config &config);
	void aa4101(machine_config &config);
	void aa4201(machine_config &config);
	void aa4401(machine_config &config);
	void ar140(machine_config &config);
	void aa540(machine_config &config);
	void ar225(machine_config &config);
	void ar260(machine_config &config);
	void aa3000(machine_config &config);
	void av20dev(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;

	u32 peripheral5_r(offs_t offset);
	void peripheral5_w(offs_t offset, u32 data);

private:
	required_device<wd1772_device> m_fdc;
	optional_device_array<floppy_connector, 4> m_floppy;
	//optional_device<hd63463_device> m_hdc;
	//optional_device<archimedes_test_slot_device> m_test;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
};


class aa680_state : public aabase_state
{
public:
	aa680_state(const machine_config &mconfig, device_type type, const char *tag)
		: aabase_state(mconfig, type, tag)
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_scsi(*this, "scsi:7:wd33c93a")
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_cent_ctrl_out(*this, "cent_ctrl_out")
		, m_cent_status_in(*this, "cent_status_in")
		, m_hex_display(0)
	{ }

	void am4(machine_config &config);
	void aa680(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;

	virtual void memc_map(address_map &map) override ATTR_COLD;

	u32 peripheral5_r(offs_t offset);
	void peripheral5_w(offs_t offset, u32 data);

private:
	required_device<fd1793_device> m_fdc;
	optional_device_array<floppy_connector, 2> m_floppy;
	required_device<wd33c93a_device> m_scsi;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<output_latch_device> m_cent_ctrl_out;
	required_device<input_buffer_device> m_cent_status_in;

	u8 m_hex_display;
};


class aa4000_state : public aabase_state
{
public:
	aa4000_state(const machine_config &mconfig, device_type type, const char *tag)
		: aabase_state(mconfig, type, tag)
		, m_i2cmem(*this,"i2cmem")
		, m_floppy(*this, "upc:fdc:%u", 0U)
		//, m_test(*this, "test")
		, m_ioeb_control(0)
	{ }

	void aa3010(machine_config &config);
	void aa3020(machine_config &config);
	void aa4000(machine_config &config);

	required_device<pcf8583_device> m_i2cmem;

protected:
	virtual void machine_reset() override ATTR_COLD;

	virtual void memc_map(address_map &map) override ATTR_COLD;

	u8 ioeb_r(offs_t offset);
	void ioeb_w(offs_t offset, u8 data);

	optional_device_array<floppy_connector, 4> m_floppy;
	//required_device<archimedes_test_slot_device> m_test;

	u8 m_ioeb_control;

};


class aa5000_state : public aa4000_state
{
public:
	aa5000_state(const machine_config &mconfig, device_type type, const char *tag)
		: aa4000_state(mconfig, type, tag)
		, m_ext_rom(*this, "extrom")
		, m_ext_region(*this, "extension")
	{ }

	void aa5000(machine_config &config);
	void aa5000a(machine_config &config);

protected:
	virtual void memc_map(address_map &map) override ATTR_COLD;

	u8 extension_r(offs_t offset);

	required_device<generic_slot_device> m_ext_rom;
	required_memory_region m_ext_region;
};

class aa4_state : public aa5000_state
{
public:
	aa4_state(const machine_config &mconfig, device_type type, const char *tag)
		: aa5000_state(mconfig, type, tag)
		, m_lc(*this, "lc")
		, m_bmu(*this, "bmu")
	{ }

	void aa4(machine_config &config);

protected:
	virtual void memc_map(address_map &map) override ATTR_COLD;

private:
	required_device<acorn_lc_device> m_lc;
	required_device<acorn_bmu_device> m_bmu;

};


void aabase_state::init_r225()
{
	u8 *cmos = memregion("i2cmem")->base();

	cmos[0x30] = 0x20; // *Configure Autoboot On
	cmos[0x36] = 0x01; // *Configure Netboot On
	cmos[0x45] = 0x17; // *Configure FileSystem Ram
	cmos[0x50] = 0x90; // *Configure Boot
	cmos[0xb4] = 0x24; // *Configure Romboard 1 1 1024  *Configure Romboard 1 2 1024
	cmos[0xc7] = 0x00; // *Configure Floppies 0
	cmos[0xd0] = 0x08; // *Configure RamFSSize 256K
}

void aabase_state::init_flop()
{
	u8 *cmos = memregion("i2cmem")->base();

	cmos[0xc7] = 0x01; // *Configure Floppies 1
}

void aabase_state::init_hd()
{
	//u8 *cmos = memregion("i2cmem")->base();

	//cmos[0x4b] = 0x54; // *Configure Drive 4
	//cmos[0x50] = 0x90; // *Configure Boot
	//cmos[0xc7] = 0x09; // *Configure HardDiscs 1
}

void aabase_state::init_scsi()
{
	//u8 *cmos = memregion("i2cmem")->base();

	//cmos[0x4b] = 0x54; // *Configure Drive 4
	//cmos[0xc7] = 0x08; // *Configure SCSIFSdiscs 1
}

void aabase_state::init_ide()
{
	u8 *cmos = memregion("i2cmem")->base();

	cmos[0x4b] = 0x54; // *Configure Drive 4
	cmos[0xc7] = 0x41; // *Configure IDEDiscs 1
}

void aabase_state::init_a4()
{
	u8 *cmos = memregion("i2cmem")->base();

	cmos[0x28] = 0x01; // !BatMgr
	cmos[0x4b] = 0x54; // *Configure Drive 4
	cmos[0xc7] = 0x41; // *Configure IDEDiscs 1
}


u32 aa310_state::peripheral5_r(offs_t offset)
{
	switch ((offset << 2) & 0xfc)
	{
	case 0x20: // HD63463
	case 0x24:
		return 0; //m_hdc->read(offset);

	case 0x08: // HD63463 DACK
	case 0x0c:
		return 0; //m_hdc->dma_r();

	case 0x18: // FDC latch B
		return 0x00;

	case 0x40: // FDC latch A
		return 0x00;

	case 0x50: // IOEB not present
		return 0x00;

	case 0x70: // Monitor Type, TBD
		return 0x0f;

	case 0x78: // Joystick (A3010 only)
		return 0xff;
	}

	return 0xffffffff;
}

void aa310_state::peripheral5_w(offs_t offset, u32 data)
{
	switch ((offset << 2) & 0xfc)
	{
	case 0x00:  // HD63463
	case 0x04:
		//m_hdc->write(offset, data);
		break;

	case 0x28:  // HD63463 DACK
	case 0x2c:
		//m_hdc->dma_w(data);
		break;

	case 0x10:  // Printer port
		m_cent_data_out->write(data & 0xff);
		break;

	case 0x18:  // Latch B
				// xxx- ---- Not used
				// ---x ---- Printer Strobe
				// ---- x--- FDC Reset
				// ---- --x- FDC DDEN
		m_fdc->dden_w(BIT(data, 1));
		m_fdc->mr_w(BIT(data, 3));
		m_centronics->write_strobe(BIT(data, 4));
		break;

	case 0x40:  // Latch A
				// x--- ---- Not used
				// -x-- ---- Floppy disc INUSE
				// --x- ---- Floppy motor
				// ---x ---- Side select
				// ---- xxxx Floppy disc select
		m_selected_floppy = nullptr;
		if (!BIT(data, 0)) m_selected_floppy = m_floppy[0]->get_device();
		if (!BIT(data, 1)) m_selected_floppy = m_floppy[1]->get_device();
		if (!BIT(data, 2)) m_selected_floppy = m_floppy[2]->get_device();
		if (!BIT(data, 3)) m_selected_floppy = m_floppy[3]->get_device();

		m_fdc->set_floppy(m_selected_floppy);

		if (m_selected_floppy)
		{
			m_selected_floppy->mon_w(BIT(data, 5));
			m_selected_floppy->ss_w(!BIT(data, 4));
		}

		// Debug code to display RISC OS 3 POST failures
		//if (BIT(data, 1))
		//{
			post_debug(BIT(data, 0));
		//}
		break;
	}
}

u32 aa500_state::peripheral5_r(offs_t offset)
{
	switch ((offset << 2) & 0xfc)
	{
	case 0x20: // HD63463
	case 0x24:
		return 0; //m_hdc->read(offset);

	case 0x08: // HD63463 DACK
	case 0x0c:
		return 0; //m_hdc->dma_r();

	case 0x18: // External Latch B
		return 0x00;
	}

	return 0xffffffff;
}

void aa500_state::peripheral5_w(offs_t offset, u32 data)
{
	switch ((offset << 2) & 0xfc)
	{
	case 0x00:  // HD63463
	case 0x04:
		//m_hdc->write(offset, data);
		break;

	case 0x28:  // HD63463 DACK
	case 0x2c:
		//m_hdc->dma_w(data);
		break;

	case 0x10:  // Printer Data
		m_cent_data_out->write(data & 0xff);
		break;

	case 0x18:  // External Latch B
				// x--- ---- Head Select 3
				// -x-- ---- analogue output Mute
				// --x- ---- analogue input Mute
				// ---x ---- Printer Strobe
				// ---- x--- FDC Controller reset
				// ---- -x-- FDC clock frequency control
				// ---- --xx Data Separator control
		m_fdc->dden_w(BIT(data, 1));
		//m_fdc->set_unscaled_clock(24_MHz_XTAL / (BIT(data, 2) ? 24 : 12));
		m_fdc->mr_w(BIT(data, 3));
		m_centronics->write_strobe(BIT(data, 4));
		break;
	}
}

void aa500_state::peripheral6_w(offs_t offset, u32 data)
{
	switch ((offset << 2) & 0xfc)
	{
	case 0x00:  // External Latch A
				// x--- ---- Disc Eject/Change Reset
				// -x-- ---- In Use control
				// --x- ---- Motor on/off control
				// ---x ---- Side Select
				// ---- xxxx Floppy Disc select
		m_selected_floppy = nullptr;
		if (!BIT(data, 0)) m_selected_floppy = m_floppy[0]->get_device();
		if (!BIT(data, 1)) m_selected_floppy = m_floppy[1]->get_device();
		if (!BIT(data, 2)) m_selected_floppy = nullptr;
		if (!BIT(data, 3)) m_selected_floppy = nullptr;

		m_fdc->set_floppy(m_selected_floppy);

		if (m_selected_floppy)
		{
			m_selected_floppy->mon_w(BIT(data, 5));
			m_selected_floppy->ss_w(!BIT(data, 4));
		}
		break;
	}
}


u32 aa680_state::peripheral5_r(offs_t offset)
{
	switch ((offset << 2) & 0xfc)
	{
	case 0x18:  // Printer Control
				// x--- ---- Select
				// -x-- ---- Printer error
				// --x- ---- Printer reset
				// ---x ---- Busy
				// ---- x--- Acknowledge
				// ---- -x-- Auto-line feed
				// ---- --x- Printer direction
				// ---- ---x Paper error
		return m_cent_status_in->read();
	}

	return 0xffffffff;
}

void aa680_state::peripheral5_w(offs_t offset, u32 data)
{
	switch ((offset << 2) & 0xfc)
	{
	case 0x10: // Printer Data
		m_cent_data_out->write(data & 0xff);
		break;

	case 0x18:  // Printer Control
				// xx-- ---- Reserved write zeros
				// --x- ---- Printer reset
				// ---x ---- Clear busy
				// ---- x--- Acknowledge
				// ---- -x-- Auto-line feed
				// ---- --x- Printer port direction
				// ---- ---x Printer strobe
		m_cent_ctrl_out->write(data ^ 0xff);
		break;

	case 0x40:  // Floppy Latch 1
				// x--- ---- reset
				// -x-- ---- Floppy disc INUSE
				// --x- ---- Floppy motor
				// ---x ---- Side select
				// ---- x--- Density
				// ---- -x-- Not used
				// ---- --xx Floppy disc select
		m_selected_floppy = nullptr;
		if (!BIT(data, 0)) m_selected_floppy = m_floppy[0]->get_device();
		if (!BIT(data, 1)) m_selected_floppy = m_floppy[1]->get_device();

		m_fdc->dden_w(BIT(data, 3));

		m_fdc->set_floppy(m_selected_floppy);

		if (m_selected_floppy)
		{
			m_selected_floppy->mon_w(BIT(data, 5));
			m_selected_floppy->ss_w(!BIT(data, 4));
		}

		m_fdc->mr_w(BIT(data, 7));
		break;
	}
}


u8 aa4000_state::ioeb_r(offs_t offset)
{
	static constexpr struct
	{
		u8 id;
		u8 hs;
	}
	mid[6] =
	{
		{0xe, 0x1}, // Normal
		{0xb, 0x4}, // Multiscan
		{0xe, 0x0}, // VGA
		{0xe, 0x0}, // Super VGA
		{0xf, 0x0}, // Hi-res mono
		{0xf, 0x0}  // LCD
	};

	u8 data = 0xff;
	int hs = 0;

	switch ((offset << 2) & 0xf8)
	{
	case 0x50: // ID register
		data = 0xf5;
		break;

	case 0x58: // Interrupt latch
		logerror("ioeb_r: interrupt latch\n");
		break;

	case 0x70: // MID register (monitor)
		//if (BIT(m_ioeb_control, 2))
		//  hs = !vidc_get_hs();
		//else
		//  hs = vidc_get_hs();

		if (hs)
			data = 0xf0 | mid[1].id | mid[1].hs;
		else
			data = 0xf0 | mid[1].id;
		break;

	case 0x78: // Joystick (A3010 only)
		data = m_joy[offset & 1].read_safe(0xff);
		break;

	default:
		logerror("ioeb_r: unknown %04x\n", offset << 2);
	}

	return data;
}

void aa4000_state::ioeb_w(offs_t offset, u8 data)
{
	switch ((offset << 2) & 0xf8)
	{
	case 0x48: // Control register
		switch (BIT(data, 0, 2))
		{
		case 0:
			m_vidc->set_unscaled_clock(24_MHz_XTAL);
			break;
		case 1:
			m_vidc->set_unscaled_clock(25.175_MHz_XTAL);
			break;
		case 2:
			m_vidc->set_unscaled_clock(36_MHz_XTAL);
			break;
		case 3:
			m_vidc->set_unscaled_clock(24_MHz_XTAL);
			break;
		}
		m_ioeb_control = data & 0x0f;
		break;

	case 0x58: // Interrupt latch
		//logerror("ioeb_w: interrupt latch %02x\n", data);
		break;

	default:
		logerror("ioeb_w: unknown %04x %04x\n", offset << 2, data);
	}
}


u8 aa5000_state::extension_r(offs_t offset)
{
	// test for valid extension rom loaded
	if (m_ext_rom->read_rom(3) == 0x87)
		return m_ext_rom->read_rom(offset & 0xffff);
	else
		return m_ext_region->base()[offset & 0xffff];
}


u32 aabase_state::dram_r(offs_t offset, u32 mem_mask)
{
	return m_dram[offset];
}

void aabase_state::dram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_dram[offset]);
}


void aabase_state::post_debug(int state)
{
	static attotime last_time(0, 0);
	static int bitpos = 0;

	if (state && bitpos <= 32)
	{
		bool bit = (machine().time() - last_time) > m_maincpu->clocks_to_attotime(2000000);

		switch (32 - bitpos)
		{
		// Status bits
		case  0: LOGMASKED(LOG_POST, "00000001  %-4s  Self-test due to power on\n"                , bit ? "Yes" : "No"); break;
		case  1: LOGMASKED(LOG_POST, "00000002  %-4s  Self-test due to interface hardware\n"      , bit ? "Yes" : "No"); break;
		case  2: LOGMASKED(LOG_POST, "00000004  %-4s  Self-test due to test link\n"               , bit ? "Yes" : "No"); break;
		case  3: LOGMASKED(LOG_POST, "00000008  %-4s  Long memory test performed\n"               , bit ? "Yes" : "No"); break;
		case  4: LOGMASKED(LOG_POST, "00000010  %-4s  ARM 3 fitted\n"                             , bit ? "Yes" : "No"); break;
		case  5: LOGMASKED(LOG_POST, "00000020  %-4s  Long memory test disabled\n"                , bit ? "Yes" : "No"); break;
		case  6: LOGMASKED(LOG_POST, "00000040  %-4s  PC-style IO world detected\n"               , bit ? "Yes" : "No"); break;
		case  7: LOGMASKED(LOG_POST, "00000080  %-4s  VRAM detected\n"                            , bit ? "Yes" : "No"); break;

		// Fault bits
		case  8: LOGMASKED(LOG_POST, "00000100  %-4s  CMOS RAM checksum error\n"                  , bit ? "Fail" : "Pass"); break;
		case  9: LOGMASKED(LOG_POST, "00000200  %-4s  ROM failed checksum test\n"                 , bit ? "Fail" : "Pass"); break;
		case 10: LOGMASKED(LOG_POST, "00000400  %-4s  MEMC CAM mapping failed\n"                  , bit ? "Fail" : "Pass"); break;
		case 11: LOGMASKED(LOG_POST, "00000800  %-4s  MEMC protection failed\n"                   , bit ? "Fail" : "Pass"); break;
		case 12: LOGMASKED(LOG_POST, "00001000  %-4s  IOC register test failed\n"                 , bit ? "Fail" : "Pass"); break;
		case 14: LOGMASKED(LOG_POST, "00004000  %-4s  VIDC (Virq interrupt) timing failed\n"      , bit ? "Fail" : "Pass"); break;
		case 15: LOGMASKED(LOG_POST, "00008000  %-4s  Sound (Sirq interrupt) timing failed\n"     , bit ? "Fail" : "Pass"); break;
		case 16: LOGMASKED(LOG_POST, "00010000  %-4s  CMOS RAM (clock/calendar chip) unreadable\n", bit ? "Fail" : "Pass"); break;
		case 17: LOGMASKED(LOG_POST, "00020000  %-4s  RAM control line failure\n"                 , bit ? "Fail" : "Pass"); break;
		case 18: LOGMASKED(LOG_POST, "00040000  %-4s  Long RAM test failure\n"                    , bit ? "Fail" : "Pass"); break;
		}

		bitpos++;
	}

	last_time = machine().time();
}


void aabase_state::machine_start()
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
			m_memc->space(0).install_readwrite_handler(0x02000000, 0x021fffff, dram_size == 512 ? 0x000ff7ff : 0x001ff7ff, 0x00e00000, 0,
				read32s_delegate(*this, FUNC(aabase_state::dram_r)),
				write32s_delegate(*this, FUNC(aabase_state::dram_w)));

			// for better performance we allocate 2 MB and mask out unused lines in the handlers
			m_dram = std::make_unique<u32[]>(2048 * 1024 / 4);
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


void aa500_state::machine_reset()
{
	m_selected_floppy = m_floppy[0]->get_device();
}


void aa310_state::machine_reset()
{
	m_selected_floppy = m_floppy[0]->get_device();
}


void aa680_state::machine_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	// install hexadecimal display
	program.install_read_tap(0x3400000, 0x3420003, "rom_shadow_bank_r", [this](offs_t offset, u32 &data, u32 mem_mask)
		{
			if (!machine().side_effects_disabled())
			{
				switch (offset & 0xfff00)
				{
				case 0x00000:
					if (((offset >> 4) & 0x0f) != m_hex_display)
					{
						m_hex_display = (offset >> 4) & 0x0f;
						logerror("Hexadecimal display: %X\n", m_hex_display);
					}
					break;

				case 0x20000:
					if (m_hex_display != 0x00)
					{
						m_hex_display = 0x00;
						logerror("Hexadecimal display: BLANK\n");
					}
					break;
				}
			}

			// return the original data
			return data;
		});

	m_selected_floppy = m_floppy[0]->get_device();
	m_hex_display = 0x00;
}


void aa4000_state::machine_reset()
{
	m_selected_floppy = m_floppy[0]->get_device();
	m_ioeb_control = 0x00;
}


void aabase_state::arm_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).rw(m_memc, FUNC(acorn_memc_device::logical_r), FUNC(acorn_memc_device::logical_w));
	map(0x02000000, 0x03ffffff).rw(m_memc, FUNC(acorn_memc_device::high_mem_r), FUNC(acorn_memc_device::high_mem_w));
}

void aabase_state::memc_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).rw(m_memc, FUNC(acorn_memc_device::logical_r), FUNC(acorn_memc_device::logical_w));
	map(0x02000000, 0x02ffffff).noprw(); // physical ram installed in machine_start
	map(0x03000000, 0x033fffff).m(m_ioc, FUNC(acorn_ioc_device::map));
	map(0x03000000, 0x0300ffff).rw(m_exp, FUNC(archimedes_exp_device::ms_r), FUNC(archimedes_exp_device::ms_w)).umask32(0x0000ffff);
	map(0x03400000, 0x035fffff).nopr().w(m_vidc, FUNC(acorn_vidc10_device::write));
	map(0x03600000, 0x037fffff).nopr().w(m_memc, FUNC(acorn_memc_device::registers_w));
	map(0x03800000, 0x039fffff).mirror(0x600000).rom().region("maincpu", 0).w(m_memc, FUNC(acorn_memc_device::page_w));
}

void aa680_state::memc_map(address_map &map)
{
	aabase_state::memc_map(map);
	map(0x03100000, 0x0313ffff).ram(); // scsi buffer
	map(0x031e0000, 0x031e0007).rw(m_scsi, FUNC(wd33c93a_device::indir_r), FUNC(wd33c93a_device::indir_w)).umask32(0x000000ff);
	//map(0x031e0200, 0x031e03ff).lw8(NAME([this](u8 data) { m_scsi_ctrl = data; })).umask32(0x000000ff);
	//map(0x031e0400, 0x031e05ff).lw16(NAME([this](u16 data) { m_scsi_addr = data; })).umask32(0x0000ffff);
	map(0x03400000, 0x035fffff).rom().region("maincpu", 0);
}

void aa4000_state::memc_map(address_map &map)
{
	aabase_state::memc_map(map);
	map(0x03010000, 0x03011fff).select(0x00180000).rw("upc", FUNC(upc82c711_device::io_r), FUNC(upc82c711_device::io_w)).umask32(0x0000ffff);
	map(0x03012000, 0x03029fff).select(0x00180000).rw("upc", FUNC(upc82c711_device::dack_tc0_r), FUNC(upc82c711_device::dack_tc0_w)).umask32(0x000000ff);
	map(0x0302a000, 0x0302bfff).select(0x00180000).rw("upc", FUNC(upc82c711_device::dack_tc1_r), FUNC(upc82c711_device::dack_tc1_w)).umask32(0x000000ff);
}

void aa5000_state::memc_map(address_map &map)
{
	aabase_state::memc_map(map);
	map(0x03010000, 0x03011fff).select(0x00180000).rw("upc", FUNC(upc82c710_device::io_r), FUNC(upc82c710_device::io_w)).umask32(0x0000ffff);
	map(0x03012000, 0x03029fff).select(0x00180000).rw("upc", FUNC(upc82c710_device::dack_tc0_r), FUNC(upc82c710_device::dack_tc0_w)).umask32(0x000000ff);
	map(0x0302a000, 0x0302bfff).select(0x00180000).rw("upc", FUNC(upc82c710_device::dack_tc1_r), FUNC(upc82c710_device::dack_tc1_w)).umask32(0x000000ff);
	map(0x03400000, 0x037fffff).r(FUNC(aa5000_state::extension_r)).umask32(0x000000ff); // 5th column ROM
}

void aa4_state::memc_map(address_map &map)
{
	aabase_state::memc_map(map);
	map(0x03010000, 0x03011fff).select(0x00180000).rw("upc", FUNC(upc82c711_device::io_r), FUNC(upc82c711_device::io_w)).umask32(0x0000ffff);
	map(0x03012000, 0x03029fff).select(0x00180000).rw("upc", FUNC(upc82c711_device::dack_tc0_r), FUNC(upc82c711_device::dack_tc0_w)).umask32(0x000000ff);
	map(0x0302a000, 0x0302bfff).select(0x00180000).rw("upc", FUNC(upc82c711_device::dack_tc1_r), FUNC(upc82c711_device::dack_tc1_w)).umask32(0x000000ff);
	map(0x0302c000, 0x0302ffff).select(0x00180000).rw(m_lc, FUNC(acorn_lc_device::read), FUNC(acorn_lc_device::write)).umask32(0x000000ff);
	map(0x03400000, 0x037fffff).r(FUNC(aa4_state::extension_r)).umask32(0x000000ff); // 5th column ROM
}


//static INPUT_PORTS_START( aa4 )
//  PORT_START("MONITOR")
//  PORT_CONFNAME( 0x07, 0x07, "Monitor Type" )
//  PORT_CONFSETTING( 0x02, "Colour SVGA" )
//  PORT_CONFSETTING( 0x05, "Mono VGA" )
//  PORT_CONFSETTING( 0x06, "Colour VGA" )
//  PORT_CONFSETTING( 0x07, "LCD" )
//INPUT_PORTS_END


static INPUT_PORTS_START( aa3010 )
	// A3010 joystick ports
	PORT_START("joy_p1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x60, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("joy_p2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x60, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


void aabase_state::floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
	// Archimedes formats
	fr.add(FLOPPY_ACORN_ADFS_NEW_FORMAT);
	fr.add(FLOPPY_APD_FORMAT);
	fr.add(FLOPPY_JFD_FORMAT);
	// BBC Micro formats
	fr.add(FLOPPY_ACORN_ADFS_OLD_FORMAT);
	fr.add(FLOPPY_ACORN_SSD_FORMAT);
	fr.add(FLOPPY_ACORN_DSD_FORMAT);
	// Atari ST formats
	fr.add(FLOPPY_ST_FORMAT);
	fr.add(FLOPPY_MSA_FORMAT);
}

static void aa310_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("35hd", FLOPPY_35_HD);
	device.option_add("525sd", FLOPPY_525_SD);
	device.option_add("525qd", FLOPPY_525_QD);
}


void aabase_state::aabase(machine_config &config)
{
	ARM(config, m_maincpu, 24_MHz_XTAL / 3); // ARM2
	m_maincpu->set_addrmap(AS_PROGRAM, &aabase_state::arm_map);
	m_maincpu->set_copro_type(arm_cpu_device::copro_type::VL86C020);

	ACORN_MEMC(config, m_memc, 24_MHz_XTAL / 3, m_vidc);
	m_memc->set_addrmap(0, &aabase_state::memc_map);
	m_memc->sirq_w().set(m_ioc, FUNC(acorn_ioc_device::il1_w));
	//m_memc->abort_w().set_inputline(m_maincpu, ARM_ABORT_LINE);

	ACORN_IOC(config, m_ioc, 24_MHz_XTAL / 3);
	m_ioc->fiq_w().set_inputline(m_maincpu, ARM_FIRQ_LINE);
	m_ioc->irq_w().set_inputline(m_maincpu, ARM_IRQ_LINE);
	m_ioc->kout_w().set("keyboard", FUNC(archimedes_keyboard_device::kin_w));
	m_ioc->peripheral_r<4>().set(m_exp, FUNC(archimedes_exp_device::ps4_r));
	m_ioc->peripheral_w<4>().set(m_exp, FUNC(archimedes_exp_device::ps4_w));
	m_ioc->peripheral_r<6>().set(m_exp, FUNC(archimedes_exp_device::ps6_r));
	m_ioc->peripheral_w<6>().set(m_exp, FUNC(archimedes_exp_device::ps6_w));

	ARCHIMEDES_KEYBOARD(config, "keyboard").kout().set(m_ioc, FUNC(acorn_ioc_device::kin_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.screen_vblank().set(m_ioc, FUNC(acorn_ioc_device::ir_w));

	ACORN_VIDC1A(config, m_vidc, 24_MHz_XTAL);
	m_vidc->set_screen("screen");
	m_vidc->vblank().set(m_memc, FUNC(acorn_memc_device::vidrq_w));
	m_vidc->sound_drq().set(m_memc, FUNC(acorn_memc_device::sndrq_w));

	RAM(config, m_ram).set_default_size("1M");

	SOFTWARE_LIST(config, "flop_list").set_original("archimedes");
	SOFTWARE_LIST(config, "hdd_list").set_original("archimedes_hdd");
	SOFTWARE_LIST(config, "rom_list").set_original("archimedes_rom").set_filter("ARC");

	ARCHIMEDES_EXPANSION_BUS(config, m_exp, 24_MHz_XTAL / 3);
	m_exp->out_fiq_callback().set(m_ioc, FUNC(acorn_ioc_device::il0_w));
	m_exp->out_irq_callback().set(m_ioc, FUNC(acorn_ioc_device::il5_w));
}

void aa500_state::aa500(machine_config &config)
{
	aabase(config);

	m_ioc->baud_w().set("acia", FUNC(mos6551_device::write_rxc));
	m_ioc->peripheral_r<1>().set([this](offs_t offset) { return m_fdc->read(offset & 0x03); });
	m_ioc->peripheral_w<1>().set([this](offs_t offset, u32 data) { m_fdc->write(offset & 0x03, data & 0xff); });
	m_ioc->peripheral_r<2>().set([this](offs_t offset) { return m_adlc->read(offset & 0x03); });
	m_ioc->peripheral_w<2>().set([this](offs_t offset, u32 data) { m_adlc->write(offset & 0x03, data & 0xff); });
	m_ioc->peripheral_r<3>().set("acia", FUNC(mos6551_device::read));
	m_ioc->peripheral_w<3>().set("acia", FUNC(mos6551_device::write));
	m_ioc->peripheral_r<5>().set(FUNC(aa500_state::peripheral5_r));
	m_ioc->peripheral_w<5>().set(FUNC(aa500_state::peripheral5_w));
	m_ioc->peripheral_r<6>().set_constant(0xff);
	m_ioc->peripheral_w<6>().set(FUNC(aa500_state::peripheral6_w));
	m_ioc->gpio_r<0>().set([this]() { return m_i2cmem->read_sda() & m_rtc->sda_r(); });
	m_ioc->gpio_w<0>().set(m_i2cmem, FUNC(i2c_pcf8570_device::write_sda));
	m_ioc->gpio_w<0>().append(m_rtc, FUNC(pcf8573_device::sda_w));
	m_ioc->gpio_w<1>().set(m_i2cmem, FUNC(i2c_pcf8570_device::write_scl));
	m_ioc->gpio_w<1>().append(m_rtc, FUNC(pcf8573_device::scl_w));
	//m_ioc->gpio_r<2>().set("rtc", FUNC(pcf8573_device::min_r));
	//m_ioc->gpio_r<3>().set("rtc", FUNC(pcf8573_device::sec_r));
	m_ioc->gpio_r<4>().set([this] { return m_selected_floppy ? m_selected_floppy->dskchg_r() : 1; });
	//m_ioc->gpio_w<5>().set([this] (int state) { logerror("%s: Sound Mute %d", machine().describe_context(), state); });

	ACORN_VIDC1(config.replace(), m_vidc, 24_MHz_XTAL);
	m_vidc->set_screen("screen");
	m_vidc->vblank().set(m_memc, FUNC(acorn_memc_device::vidrq_w));
	m_vidc->sound_drq().set(m_memc, FUNC(acorn_memc_device::sndrq_w));

	// TODO: implement A500 keyboard, uses M6500/11 MCU

	m_ram->set_default_size("4M");

	// PCF8570 and PCF8573 (pre PCF8583)
	I2C_PCF8570(config, m_i2cmem);
	PCF8573(config, m_rtc, 32.768_kHz_XTAL);

	FD1793(config, m_fdc, 24_MHz_XTAL / 24);
	m_fdc->intrq_wr_callback().set(m_ioc, FUNC(acorn_ioc_device::fh1_w));
	m_fdc->drq_wr_callback().set(m_ioc, FUNC(acorn_ioc_device::fh0_w));
	FLOPPY_CONNECTOR(config, m_floppy[0], aa310_floppies, "35dd", aabase_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], aa310_floppies, nullptr, aabase_state::floppy_formats).enable_sound(true);

	CENTRONICS(config, m_centronics, centronics_devices, nullptr);
	m_centronics->busy_handler().set(m_ioc, FUNC(acorn_ioc_device::il6_w));
	m_centronics->ack_handler().set(m_ioc, FUNC(acorn_ioc_device::if_w));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	MC6854(config, m_adlc);
	m_adlc->out_txd_cb().set("network", FUNC(econet_device::host_data_w));
	m_adlc->out_irq_cb().set(m_ioc, FUNC(acorn_ioc_device::fl_w));
	//m_adlc->out_rts_cb().

	econet_device &econet(ECONET(config, "network", 0));
	econet.clk_wr_callback().set(m_adlc, FUNC(mc6854_device::txc_w));
	econet.clk_wr_callback().append(m_adlc, FUNC(mc6854_device::rxc_w));
	econet.data_wr_callback().set(m_adlc, FUNC(mc6854_device::set_rx));

	mos6551_device &acia(MOS6551(config, "acia", 0));
	acia.set_xtal(1.8432_MHz_XTAL);
	acia.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	acia.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	acia.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	acia.irq_handler().set(m_ioc, FUNC(acorn_ioc_device::il2_w));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("acia", FUNC(mos6551_device::write_rxd));
	rs232.dcd_handler().set("acia", FUNC(mos6551_device::write_dcd));
	rs232.cts_handler().set("acia", FUNC(mos6551_device::write_cts));
	rs232.dsr_handler().set("acia", FUNC(mos6551_device::write_dsr));

	//HD63463(config, m_hdc, 24_MHz_XTAL / 3);
	//m_hdc->usel_callback().set([](u8 data) { return data - 1; });
	//m_hdc->irq_callback().set(m_ioc, FUNC(acorn_ioc_device::il3_w));
	//m_hdc->dreq_callback().set(m_ioc, FUNC(acorn_ioc_device::il4_w));

	//HARDDISK(config, "hdc:0", "st506_hdd"); // 20MB HDD
	//HARDDISK(config, "hdc:1", "st506_hdd");

	// expansion slots - 4-card backplane
	ARCHIMEDES_PODULE_SLOT(config, m_podule[0], m_exp, archimedes_exp_devices, nullptr);
	ARCHIMEDES_PODULE_SLOT(config, m_podule[1], m_exp, archimedes_exp_devices, nullptr);
	ARCHIMEDES_PODULE_SLOT(config, m_podule[2], m_exp, archimedes_exp_devices, nullptr);
	ARCHIMEDES_PODULE_SLOT(config, m_podule[3], m_exp, archimedes_exp_devices, nullptr);
}

void aa500_state::aa500d(machine_config &config)
{
	aa500(config);

	// expansion slots - 4-card backplane
	m_podule[0]->set_default_option(nullptr);     // RGB conversion card
	m_podule[1]->set_default_option(nullptr);     // prototype SCSI interface for Philips VP415
	m_podule[2]->set_default_option(nullptr);
	m_podule[3]->set_default_option(nullptr);
}

void aa310_state::aa310(machine_config &config)
{
	aabase(config);

	m_ioc->baud_w().set("acia", FUNC(mos6551_device::write_rxc));
	m_ioc->peripheral_r<1>().set([this](offs_t offset) { return m_fdc->read(offset & 0x03); });
	m_ioc->peripheral_w<1>().set([this](offs_t offset, u32 data) { m_fdc->write(offset & 0x03, data & 0xff); });
	m_ioc->peripheral_r<2>().set("econet", FUNC(archimedes_econet_slot_device::read));
	m_ioc->peripheral_w<2>().set("econet", FUNC(archimedes_econet_slot_device::write));
	m_ioc->peripheral_r<3>().set("acia", FUNC(mos6551_device::read));
	m_ioc->peripheral_w<3>().set("acia", FUNC(mos6551_device::write));
	m_ioc->peripheral_r<5>().set(FUNC(aa310_state::peripheral5_r));
	m_ioc->peripheral_w<5>().set(FUNC(aa310_state::peripheral5_w));
	m_ioc->gpio_r<0>().set("i2cmem", FUNC(pcf8583_device::sda_r));
	m_ioc->gpio_w<0>().set("i2cmem", FUNC(pcf8583_device::sda_w));
	m_ioc->gpio_w<1>().set("i2cmem", FUNC(pcf8583_device::scl_w));
	m_ioc->gpio_r<2>().set([this] { return m_selected_floppy ? !m_selected_floppy->ready_r() : 0; });
	//m_ioc->gpio_r<3>().set([this] () { logerror("%s: Reserved", machine().describe_context()); return 0; });
	//m_ioc->gpio_r<4>().set([this] () { logerror("%s: Aux IO connector", machine().describe_context()); return 0; });
	//m_ioc->gpio_w<5>().set([this] (int state) { logerror("%s: Sound Mute %s", machine().describe_context(), state); });

	m_ram->set_default_size("1M");

	PCF8583(config, "i2cmem", 32.768_kHz_XTAL);

	WD1772(config, m_fdc, 24_MHz_XTAL / 3);
	m_fdc->set_disable_motor_control(true);
	m_fdc->intrq_wr_callback().set(m_ioc, FUNC(acorn_ioc_device::fh1_w));
	m_fdc->drq_wr_callback().set(m_ioc, FUNC(acorn_ioc_device::fh0_w));
	FLOPPY_CONNECTOR(config, m_floppy[0], aa310_floppies, "35dd", aabase_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], aa310_floppies, nullptr, aabase_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[2], aa310_floppies, nullptr, aabase_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[3], aa310_floppies, nullptr, aabase_state::floppy_formats).enable_sound(true);

	CENTRONICS(config, m_centronics, centronics_devices, nullptr);
	m_centronics->option_add("spjoy", SERIAL_PORT_JOYSTICK);
	m_centronics->busy_handler().set(m_ioc, FUNC(acorn_ioc_device::il6_w));
	m_centronics->ack_handler().set(m_ioc, FUNC(acorn_ioc_device::if_w));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	mos6551_device &acia(MOS6551(config, "acia", 0));
	acia.set_xtal(1.8432_MHz_XTAL);
	acia.txd_handler().set("rs232", FUNC(rs232_port_device::write_txd));
	acia.rts_handler().set("rs232", FUNC(rs232_port_device::write_rts));
	acia.dtr_handler().set("rs232", FUNC(rs232_port_device::write_dtr));
	acia.irq_handler().set(m_ioc, FUNC(acorn_ioc_device::il2_w));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("acia", FUNC(mos6551_device::write_rxd));
	rs232.dcd_handler().set("acia", FUNC(mos6551_device::write_dcd));
	rs232.cts_handler().set("acia", FUNC(mos6551_device::write_cts));
	rs232.dsr_handler().set("acia", FUNC(mos6551_device::write_dsr));

	// expansion slots - 2-card backplane
	ARCHIMEDES_PODULE_SLOT(config, m_podule[0], m_exp, archimedes_exp_devices, nullptr);
	ARCHIMEDES_PODULE_SLOT(config, m_podule[2], m_exp, archimedes_exp_devices, nullptr);

	archimedes_econet_slot_device &econet(ARCHIMEDES_ECONET_SLOT(config, "econet", archimedes_econet_devices, nullptr));
	econet.efiq_handler().set(m_ioc, FUNC(acorn_ioc_device::fl_w));
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

	//HD63463(config, m_hdc, 24_MHz_XTAL / 3);
	//m_hdc->usel_callback().set([](u8 data) { return data - 1; });
	//m_hdc->irq_callback().set(m_ioc, FUNC(acorn_ioc_device::il3_w));
	//m_hdc->dreq_callback().set(m_ioc, FUNC(acorn_ioc_device::il4_w));

	//HARDDISK(config, "hdc:0", "st506_hdd"); // 20MB HDD
	//HARDDISK(config, "hdc:1", "st506_hdd");

	// expansion slots - 4-card backplane
	ARCHIMEDES_PODULE_SLOT(config, m_podule[1], m_exp, archimedes_exp_devices, nullptr);
	ARCHIMEDES_PODULE_SLOT(config, m_podule[3], m_exp, archimedes_exp_devices, nullptr);
}

void aa680_state::am4(machine_config &config)
{
	aa680(config);

	// expansion slots - 4-card backplane
	m_podule[0]->set_default_option("tube"); // Tube podule to load bootstrap from BBC Micro
	m_podule[1]->set_default_option(nullptr);
	m_podule[2]->set_default_option(nullptr);
	m_podule[3]->set_default_option(nullptr);
}

void aa680_state::aa680(machine_config &config)
{
	aabase(config);

	m_ioc->baud_w().set("scc", FUNC(scc8530_device::txca_w));
	m_ioc->baud_w().append("scc", FUNC(scc8530_device::txcb_w));
	m_ioc->peripheral_r<1>().set([this](offs_t offset) { return m_fdc->read(offset & 0x03); });
	m_ioc->peripheral_w<1>().set([this](offs_t offset, u32 data) { m_fdc->write(offset & 0x03, data & 0xff); });
	m_ioc->peripheral_r<2>().set_constant(0xff);
	m_ioc->peripheral_w<2>().set_nop();
	m_ioc->peripheral_r<3>().set("scc", FUNC(scc8530_device::ab_dc_r));
	m_ioc->peripheral_w<3>().set("scc", FUNC(scc8530_device::ab_dc_w));
	m_ioc->peripheral_r<5>().set(FUNC(aa680_state::peripheral5_r));
	m_ioc->peripheral_w<5>().set(FUNC(aa680_state::peripheral5_w));
	m_ioc->gpio_r<0>().set("i2cmem", FUNC(pcf8583_device::sda_r));
	m_ioc->gpio_w<0>().set("i2cmem", FUNC(pcf8583_device::sda_w));
	m_ioc->gpio_w<1>().set("i2cmem", FUNC(pcf8583_device::scl_w));
	m_ioc->gpio_r<2>().set([this] { return m_selected_floppy ? !m_selected_floppy->ready_r() : 0; });

	m_ram->set_default_size("8M");

	PCF8583(config, "i2cmem", 32.768_kHz_XTAL);

	FD1793(config, m_fdc, 96_MHz_XTAL / 96);
	m_fdc->intrq_wr_callback().set(m_ioc, FUNC(acorn_ioc_device::il4_w));
	m_fdc->drq_wr_callback().set(m_ioc, FUNC(acorn_ioc_device::fh0_w));
	FLOPPY_CONNECTOR(config, m_floppy[0], aa310_floppies, "35dd", aabase_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], aa310_floppies, nullptr, aabase_state::floppy_formats).enable_sound(true);

	CENTRONICS(config, m_centronics, centronics_devices, nullptr);
	m_centronics->perror_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit0));
	m_centronics->ack_handler().set(m_ioc, FUNC(acorn_ioc_device::if_w));
	m_centronics->ack_handler().append(m_cent_status_in, FUNC(input_buffer_device::write_bit3));
	m_centronics->busy_handler().set(m_ioc, FUNC(acorn_ioc_device::il6_w));
	m_centronics->busy_handler().append(m_cent_status_in, FUNC(input_buffer_device::write_bit4));
	m_centronics->fault_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit6));
	m_centronics->select_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit7));

	INPUT_BUFFER(config, m_cent_status_in);

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	OUTPUT_LATCH(config, m_cent_ctrl_out);
	m_cent_ctrl_out->bit_handler<0>().set(m_centronics, FUNC(centronics_device::write_strobe));
	m_cent_ctrl_out->bit_handler<1>().set(m_centronics, FUNC(centronics_device::write_select_in));
	m_cent_ctrl_out->bit_handler<1>().append(m_cent_status_in, FUNC(input_buffer_device::write_bit1));
	m_cent_ctrl_out->bit_handler<2>().set(m_centronics, FUNC(centronics_device::write_autofd));
	m_cent_ctrl_out->bit_handler<2>().append(m_cent_status_in, FUNC(input_buffer_device::write_bit2));
	m_cent_ctrl_out->bit_handler<3>().set(m_centronics, FUNC(centronics_device::write_ack));
	m_cent_ctrl_out->bit_handler<4>().set(m_centronics, FUNC(centronics_device::write_busy));

	scc8530_device &scc(SCC8530(config, "scc", 3.6864_MHz_XTAL));
	scc.out_txda_callback().set("rs232a", FUNC(rs232_port_device::write_txd));
	scc.out_dtra_callback().set("rs232a", FUNC(rs232_port_device::write_dtr));
	scc.out_rtsa_callback().set("rs232a", FUNC(rs232_port_device::write_rts));
	scc.out_txdb_callback().set("rs232b", FUNC(rs232_port_device::write_txd));
	scc.out_dtrb_callback().set("rs232b", FUNC(rs232_port_device::write_dtr));
	scc.out_rtsb_callback().set("rs232b", FUNC(rs232_port_device::write_rts));
	scc.out_int_callback().set(m_ioc, FUNC(acorn_ioc_device::il2_w));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232a", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set("scc", FUNC(scc8530_device::rxa_w));
	rs232a.dcd_handler().set("scc", FUNC(scc8530_device::dcda_w));
	rs232a.cts_handler().set("scc", FUNC(scc8530_device::ctsa_w));
	rs232a.dsr_handler().set("scc", FUNC(scc8530_device::synca_w));
	rs232a.ri_handler().set(m_ioc, FUNC(acorn_ioc_device::il7_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set("scc", FUNC(scc8530_device::rxb_w));
	rs232b.dcd_handler().set("scc", FUNC(scc8530_device::dcdb_w));
	rs232b.cts_handler().set("scc", FUNC(scc8530_device::ctsb_w));
	rs232b.dsr_handler().set("scc", FUNC(scc8530_device::syncb_w));

	// scsi 70MB HDD
	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, "harddisk", true); // Internal Hard Disc Drive
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr, false);   // External Hard Disc Drive #1
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr, false);   // External Hard Disc Drive #2
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr, false);   // External Hard Disc Drive #3
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr, false);   // External Hard Disc Drive #4 / Streamer #2
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr, false);   // Streamer #1
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr, false);   // Processor / Printer Device
	NSCSI_CONNECTOR(config, "scsi:7").option_set("wd33c93a", WD33C93A).clock(96_MHz_XTAL / 12)
		.machine_config([this](device_t *device)
		{
			wd33c93a_device &wd33c93(downcast<wd33c93a_device &>(*device));
			wd33c93.irq_cb().set(m_ioc, FUNC(acorn_ioc_device::il3_w));
			//wd33c93.drq_cb().set(*this, FUNC(aa680_state::scsi_drq));
		});

	// expansion slots - 4-card backplane - pre-installed podules would make it a Technical Publishing System
	ARCHIMEDES_PODULE_SLOT(config, m_podule[0], m_exp, archimedes_exp_devices, nullptr);
	ARCHIMEDES_PODULE_SLOT(config, m_podule[1], m_exp, archimedes_exp_devices, nullptr);  // Acorn AKA30 SCSI
	ARCHIMEDES_PODULE_SLOT(config, m_podule[2], m_exp, archimedes_exp_devices, nullptr);  // Acorn Laser Printer
	ARCHIMEDES_PODULE_SLOT(config, m_podule[3], m_exp, archimedes_exp_devices, "ether1"); // Acorn Lance Ethernet
}

void aa310_state::aa3000(machine_config &config)
{
	aa310(config);

	m_ram->set_default_size("1M").set_extra_options("2M");

	// expansion slots - mini
	ARCHIMEDES_PODULE_SLOT(config, m_podule[1], m_exp, archimedes_mini_exp_devices, nullptr);
	config.device_remove("podule2");
}

void aa310_state::aa4101(machine_config &config)
{
	aa310(config);
	m_ram->set_default_size("1M").set_extra_options("2M,4M");

	//HD63463(config, m_hdc, 24_MHz_XTAL / 3);
	//m_hdc->usel_callback().set([](u8 data) { return data - 1; });
	//m_hdc->irq_callback().set(m_ioc, FUNC(acorn_ioc_device::il3_w));
	//m_hdc->dreq_callback().set(m_ioc, FUNC(acorn_ioc_device::il3_w));

	//HARDDISK(config, "hdc:0", "st506_hdd");
	//HARDDISK(config, "hdc:1", "st506_hdd");

	// expansion slots - 4-card backplane
	ARCHIMEDES_PODULE_SLOT(config, m_podule[1], m_exp, archimedes_exp_devices, nullptr);
	ARCHIMEDES_PODULE_SLOT(config, m_podule[3], m_exp, archimedes_exp_devices, nullptr);
}

void aa310_state::aa4201(machine_config &config)
{
	aa4101(config);
	m_ram->set_default_size("2M").set_extra_options("4M");
}

void aa310_state::aa4401(machine_config &config)
{
	aa4201(config);
	m_ram->set_default_size("4M").set_extra_options("8M");
}

void aa310_state::ar140(machine_config &config)
{
	aa440(config);

	m_ram->set_default_size("4M").set_extra_options("8M");

	// expansion slots - 4-card backplane
	m_podule[0]->set_default_option("ether1"); // Acorn AKA25 Ethernet
	m_podule[1]->set_default_option(nullptr);
	m_podule[2]->set_default_option(nullptr);
	m_podule[3]->set_default_option(nullptr);
}

void aa310_state::aa540(machine_config &config)
{
	aa310(config);
	m_maincpu->set_clock(52_MHz_XTAL / 2); // ARM3
	m_maincpu->set_copro_type(arm_cpu_device::copro_type::VL86C020);

	m_ram->set_default_size("16M").set_extra_options("8M,16M");

	// expansion slots - 4-card backplane
	ARCHIMEDES_PODULE_SLOT(config, m_podule[1], m_exp, archimedes_exp_devices, nullptr); // Acorn AKA31 SCSI 100MB HDD
	ARCHIMEDES_PODULE_SLOT(config, m_podule[3], m_exp, archimedes_exp_devices, nullptr);

	//ARCHIMEDES_TEST_SLOT(config, m_test, archimedes_test_devices, nullptr);
}

void aa310_state::ar225(machine_config &config)
{
	aa540(config);

	m_ram->set_default_size("4M").set_extra_options("8M,16M");

	// discless
	m_floppy[0]->set_default_option(nullptr);
	m_floppy[1]->set_default_option(nullptr);

	// expansion slots - 4-card backplane
	m_podule[0]->set_default_option("ether1");       // Acorn AKA25 Ethernet
	m_podule[1]->set_default_option("rom_r225boot"); // Acorn AKA05 ROM (with DiscLess Bootstrap support)
	m_podule[2]->set_default_option(nullptr);
	m_podule[3]->set_default_option(nullptr);
}

void aa310_state::ar260(machine_config &config)
{
	aa540(config);

	m_ram->set_default_size("8M").set_extra_options("16M");

	// expansion slots - 4-card backplane
	m_podule[0]->set_default_option("ether1");     // Acorn AKA25 Ethernet
	m_podule[1]->set_default_option(nullptr);      // Acorn AKA31 SCSI
	m_podule[2]->set_default_option(nullptr);
	m_podule[3]->set_default_option(nullptr);
}

void aa310_state::av20dev(machine_config &config)
{
	// This is an internal development machine based on a R225/A540, used for testing
	// the new VIDC20 video controller.
	aa540(config);

	ARM_VIDC20(config.replace(), m_vidc, 24_MHz_XTAL);
	m_vidc->set_screen("screen");
	m_vidc->vblank().set(m_memc, FUNC(acorn_memc_device::vidrq_w));
	m_vidc->sound_drq().set(m_memc, FUNC(acorn_memc_device::sndrq_w));

	m_ram->set_default_size("4M").set_extra_options("8M,16M");

	// expansion slots - 4-card backplane
	m_podule[0]->set_default_option(nullptr);
	m_podule[1]->set_default_option(nullptr);
	m_podule[2]->set_default_option(nullptr);
	m_podule[3]->set_default_option(nullptr);
}

void aa4000_state::aa3010(machine_config &config)
{
	aabase(config);
	m_maincpu->set_clock(72_MHz_XTAL / 6); // ARM250

	m_ioc->baud_w().set("upc:serial1", FUNC(ns16450_device::clock_w));
	m_ioc->peripheral_r<1>().set([this] () { logerror("%s: IOC: Peripheral Select 1 R", machine().describe_context()); return 0; });
	m_ioc->peripheral_w<1>().set([this] (int state) { logerror("%s: IOC: Peripheral Select 1 W %d", machine().describe_context(), state); });
	m_ioc->peripheral_r<2>().set("econet", FUNC(archimedes_econet_slot_device::read));
	m_ioc->peripheral_w<2>().set("econet", FUNC(archimedes_econet_slot_device::write));
	m_ioc->peripheral_r<3>().set([this] () { logerror("%s: IOC: Peripheral Select 3 R", machine().describe_context()); return 0; });
	m_ioc->peripheral_w<3>().set([this] (int state) { logerror("%s: IOC: Peripheral Select 3 W %d", machine().describe_context(), state); });
	m_ioc->peripheral_r<5>().set(FUNC(aa4000_state::ioeb_r));
	m_ioc->peripheral_w<5>().set(FUNC(aa4000_state::ioeb_w));
	m_ioc->gpio_r<0>().set(m_i2cmem, FUNC(pcf8583_device::sda_r));
	m_ioc->gpio_w<0>().set(m_i2cmem, FUNC(pcf8583_device::sda_w));
	m_ioc->gpio_w<1>().set(m_i2cmem, FUNC(pcf8583_device::scl_w));
	m_ioc->gpio_r<2>().set_constant(1); // FDHden
	m_ioc->gpio_r<3>().set("idrom", FUNC(ds2401_device::read));
	m_ioc->gpio_w<3>().set("idrom", FUNC(ds2401_device::write));
	m_ioc->gpio_r<4>().set_constant(1); // Sintr
	//m_ioc->gpio_w<5>().set([this] (int state) { logerror("%s: Sound Mute %d", machine().describe_context(), state); });

	m_ram->set_default_size("1M").set_extra_options("2M");

	PCF8583(config, m_i2cmem, 32.768_kHz_XTAL);

	DS2401(config, "idrom", 0); // DS2400

	upc82c711_device &upc(UPC82C711(config, "upc", 24_MHz_XTAL));
	upc.irq4().set(m_ioc, FUNC(acorn_ioc_device::il2_w));
	upc.fintr().set(m_ioc, FUNC(acorn_ioc_device::il4_w));
	upc.pintr().set(m_ioc, FUNC(acorn_ioc_device::il6_w));
	upc.fdrq().set(m_ioc, FUNC(acorn_ioc_device::fh0_w));
	upc.txd1().set("rs232", FUNC(rs232_port_device::write_txd));
	upc.rts1().set("rs232", FUNC(rs232_port_device::write_rts));
	upc.dtr1().set("rs232", FUNC(rs232_port_device::write_dtr));

	FLOPPY_CONNECTOR(config, m_floppy[0], aa310_floppies, "35hd", aabase_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], aa310_floppies, nullptr, aabase_state::floppy_formats).enable_sound(true);

	subdevice<upd765_family_device>("upc:fdc")->idx_wr_callback().set(m_ioc, FUNC(acorn_ioc_device::if_w));
	subdevice<ata_interface_device>("upc:ide")->irq_handler().set(m_ioc, FUNC(acorn_ioc_device::il3_w));
	subdevice<centronics_device>("upc:parallel:centronics")->option_add("spjoy", SERIAL_PORT_JOYSTICK);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("upc:serial1", FUNC(ns16450_device::rx_w));
	rs232.cts_handler().set("upc:serial1", FUNC(ns16450_device::cts_w));
	rs232.dsr_handler().set("upc:serial1", FUNC(ns16450_device::dsr_w));
	rs232.dcd_handler().set("upc:serial1", FUNC(ns16450_device::dcd_w));
	rs232.ri_handler().set("upc:serial1", FUNC(ns16450_device::ri_w));

	// expansion slots - mini
	ARCHIMEDES_PODULE_SLOT(config, m_podule[1], m_exp, archimedes_mini_exp_devices, nullptr);

	archimedes_econet_slot_device &econet(ARCHIMEDES_ECONET_SLOT(config, "econet", archimedes_econet_devices, nullptr));
	econet.efiq_handler().set(m_ioc, FUNC(acorn_ioc_device::fl_w));

	//ARCHIMEDES_TEST_SLOT(config, m_test, archimedes_test_devices, nullptr);
}

void aa4000_state::aa3020(machine_config &config)
{
	aa3010(config);
	m_maincpu->set_clock(72_MHz_XTAL / 6); // ARM250

	m_ram->set_default_size("2M").set_extra_options("4M");

	// ide 60MB HDD
	subdevice<ata_interface_device>("upc:ide")->options(ata_devices, "hdd", nullptr, false);
}

void aa4000_state::aa4000(machine_config &config)
{
	aa3010(config);
	m_maincpu->set_clock(72_MHz_XTAL / 6); // ARM250

	m_ram->set_default_size("2M").set_extra_options("4M");

	// ide 80MB HDD
	subdevice<ata_interface_device>("upc:ide")->options(ata_devices, "hdd", nullptr, false);
}

void aa5000_state::aa5000(machine_config &config)
{
	aabase(config);
	m_maincpu->set_clock(50_MHz_XTAL / 2); // ARM3
	m_maincpu->set_copro_type(arm_cpu_device::copro_type::VL86C020);

	m_ioc->baud_w().set("upc:serial", FUNC(ns16450_device::clock_w));
	m_ioc->peripheral_r<1>().set([this] () { logerror("%s: IOC: Peripheral Select 1 R", machine().describe_context()); return 0; });
	m_ioc->peripheral_w<1>().set([this] (int state) { logerror("%s: IOC: Peripheral Select 1 W %d", machine().describe_context(), state); });
	m_ioc->peripheral_r<2>().set("econet", FUNC(archimedes_econet_slot_device::read));
	m_ioc->peripheral_w<2>().set("econet", FUNC(archimedes_econet_slot_device::write));
	m_ioc->peripheral_r<3>().set([this] () { logerror("%s: IOC: Peripheral Select 3 R", machine().describe_context()); return 0; });
	m_ioc->peripheral_w<3>().set([this] (int state) { logerror("%s: IOC: Peripheral Select 3 W %d", machine().describe_context(), state); });
	m_ioc->peripheral_r<5>().set(FUNC(aa5000_state::ioeb_r));
	m_ioc->peripheral_w<5>().set(FUNC(aa5000_state::ioeb_w));
	m_ioc->gpio_r<0>().set("i2cmem", FUNC(pcf8583_device::sda_r));
	m_ioc->gpio_w<0>().set("i2cmem", FUNC(pcf8583_device::sda_w));
	m_ioc->gpio_w<1>().set("i2cmem", FUNC(pcf8583_device::scl_w));
	m_ioc->gpio_r<2>().set_constant(1); // FDHden
	m_ioc->gpio_r<3>().set("idrom", FUNC(ds2401_device::read));
	m_ioc->gpio_w<3>().set("idrom", FUNC(ds2401_device::write));
	m_ioc->gpio_r<4>().set_constant(1); // Sintr
	//m_ioc->gpio_w<5>().set([this] (int state) { logerror("%s: Sound Mute %d", machine().describe_context(), state); });

	m_ram->set_default_size("2M").set_extra_options("4M");

	PCF8583(config, "i2cmem", 32.768_kHz_XTAL);

	DS2401(config, "idrom", 0); // DS2400

	upc82c710_device &upc(UPC82C710(config, "upc", 24_MHz_XTAL));
	upc.sintr().set(m_ioc, FUNC(acorn_ioc_device::il2_w));
	//upc.index().set(m_ioc, FUNC(acorn_ioc_device::if_w));
	upc.fintr().set(m_ioc, FUNC(acorn_ioc_device::il4_w));
	upc.pintr().set(m_ioc, FUNC(acorn_ioc_device::il6_w));
	upc.fdrq().set(m_ioc, FUNC(acorn_ioc_device::fh0_w));
	upc.txd().set("rs232", FUNC(rs232_port_device::write_txd));
	upc.rts().set("rs232", FUNC(rs232_port_device::write_rts));
	upc.dtr().set("rs232", FUNC(rs232_port_device::write_dtr));

	FLOPPY_CONNECTOR(config, m_floppy[0], aa310_floppies, "35hd", aabase_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppy[1], aa310_floppies, nullptr, aabase_state::floppy_formats).enable_sound(true);

	subdevice<upd765_family_device>("upc:fdc")->idx_wr_callback().set(m_ioc, FUNC(acorn_ioc_device::if_w));
	subdevice<ata_interface_device>("upc:ide")->irq_handler().set(m_ioc, FUNC(acorn_ioc_device::il3_w));
	subdevice<centronics_device>("upc:parallel:centronics")->option_add("spjoy", SERIAL_PORT_JOYSTICK);

	// ide 80MB HDD
	subdevice<ata_interface_device>("upc:ide")->options(ata_devices, "hdd", nullptr, false);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("upc:serial", FUNC(ns16450_device::rx_w));
	rs232.cts_handler().set("upc:serial", FUNC(ns16450_device::cts_w));
	rs232.dsr_handler().set("upc:serial", FUNC(ns16450_device::dsr_w));
	rs232.dcd_handler().set("upc:serial", FUNC(ns16450_device::dcd_w));
	rs232.ri_handler().set("upc:serial", FUNC(ns16450_device::ri_w));

	// expansion slots - 4-card backplane
	ARCHIMEDES_PODULE_SLOT(config, m_podule[0], m_exp, archimedes_exp_devices, nullptr);
	ARCHIMEDES_PODULE_SLOT(config, m_podule[1], m_exp, archimedes_exp_devices, nullptr);
	ARCHIMEDES_PODULE_SLOT(config, m_podule[2], m_exp, archimedes_exp_devices, nullptr);
	ARCHIMEDES_PODULE_SLOT(config, m_podule[3], m_exp, archimedes_exp_devices, nullptr);

	archimedes_econet_slot_device &econet(ARCHIMEDES_ECONET_SLOT(config, "econet", archimedes_econet_devices, nullptr));
	econet.efiq_handler().set(m_ioc, FUNC(acorn_ioc_device::fl_w));

	//ARCHIMEDES_TEST_SLOT(config, m_test, archimedes_test_devices, nullptr);

	// extension rom
	GENERIC_SOCKET(config, m_ext_rom, generic_linear_slot, "ext_rom", "bin,rom");
	subdevice<software_list_device>("rom_list")->set_filter("A5000,ARC");
}

void aa4_state::aa4(machine_config &config)
{
	aa3010(config);
	m_maincpu->set_clock(24_MHz_XTAL); // ARM3
	m_maincpu->set_copro_type(arm_cpu_device::copro_type::VL86C020);

	ACORN_BMU(config, m_bmu, 4.194304_MHz_XTAL);
	//bmu.battlo_callback().set(m_ioc, FUNC(acorn_ioc_device::il7_w));

	//m_ioc->peripheral_r<2>().set("lc", FUNC(lc_device::read));
	//m_ioc->peripheral_w<2>().set("lc", FUNC(lc_device::write));
	m_ioc->gpio_r<0>().set([this]() { return m_i2cmem->sda_r() & m_bmu->sda_r(); });
	m_ioc->gpio_w<0>().append(m_bmu, FUNC(acorn_bmu_device::sda_w));
	m_ioc->gpio_w<1>().append(m_bmu, FUNC(acorn_bmu_device::scl_w));

	// video hardware
	screen_device &screen(SCREEN(config.replace(), "screen", SCREEN_TYPE_LCD));
	screen.screen_vblank().set(m_ioc, FUNC(acorn_ioc_device::ir_w));

	ACORN_LC(config, m_lc, 24_MHz_XTAL);

	//ACORN_VIDC1A_LCD(config.replace(), m_vidc, 24_MHz_XTAL);
	//m_vidc->set_screen("screen");
	//m_vidc->vblank().set(m_memc, FUNC(acorn_memc_device::vidrq_w));
	//m_vidc->sound_drq().set(m_memc, FUNC(acorn_memc_device::sndrq_w));

	m_ram->set_default_size("2M").set_extra_options("4M");

	// ide 60MB HDD
	subdevice<ata_interface_device>("upc:ide")->options(ata_devices, "hdd", nullptr, false);

	// expansion slots - none
	config.device_remove("podule1");

	// extension rom
	GENERIC_SOCKET(config, m_ext_rom, generic_linear_slot, "ext_rom", "bin,rom");
	subdevice<software_list_device>("rom_list")->set_filter("A4,ARC");
}

void aa5000_state::aa5000a(machine_config &config)
{
	aa5000(config);
	m_maincpu->set_clock(33_MHz_XTAL); // ARM3
}


ROM_START( aa500 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "sn104", "#104: RISC OS 2.00 (12 Oct 1988)" ) // serial #41 has same ROMs
	ROMX_LOAD( "a500_riscos206_0.ic24", 0x000000, 0x20000, CRC(60910286) SHA1(9bede102207d45dda07b4282a4cc4b4d2212704a), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "a500_riscos206_1.ic25", 0x000001, 0x20000, CRC(3e1aaa54) SHA1(c648c691e083117f9bb2459e4675401824a851b0), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "a500_riscos206_2.ic26", 0x000002, 0x20000, CRC(3ae4e522) SHA1(030c6b2c0796655ad46732ce230b6f811f224684), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "a500_riscos206_3.ic27", 0x000003, 0x20000, CRC(8b60c990) SHA1(976f2b24913866abef9c6751591c2355ab472f87), ROM_BIOS(0) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 1, "sn46", "#46: RISC OS 2.00 (15 Sep 1988)" ) // ex Acorn, Paul Fellows' development machine - missing HDD image to soft load modules
	ROMX_LOAD( "a500_riscos200_0.ic24", 0x000000, 0x20000, CRC(9afa7fca) SHA1(a52f03e2ac3dd594fe0979c46d06601df5f690e7), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "a500_riscos200_1.ic25", 0x000001, 0x20000, CRC(4c0bd304) SHA1(36fa1c7c3a634494912581b6a71e7dd4ccdfc514), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "a500_riscos200_2.ic26", 0x000002, 0x20000, CRC(e47e98d4) SHA1(8430e0d9a4402c1a76c7c6d57826d914be49f130), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "a500_riscos200_3.ic27", 0x000003, 0x20000, CRC(e2c7d11d) SHA1(fa2d763566f7166009c04166fe6d330644201014), ROM_BIOS(1) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 2, "sn47", "#47: RISC OS 3.10 (30 Apr 1992)" )
	ROMX_LOAD( "a500_ros310_rom0.ic24", 0x000000, 0x80000, CRC(ad8e0873) SHA1(69a55e024a9f1f663b1bc0664d4d1631ed3899ee), ROM_BIOS(2) | ROM_SKIP(3) )
	ROMX_LOAD( "a500_ros310_rom1.ic25", 0x000001, 0x80000, CRC(0be31dfc) SHA1(c2a7aa6737931171507950025c3bc1008800e0f7), ROM_BIOS(2) | ROM_SKIP(3) )
	ROMX_LOAD( "a500_ros310_rom2.ic26", 0x000002, 0x80000, CRC(081c5825) SHA1(e42a356bfaa77a81ee9e7e2a8d58ff0f301583c2), ROM_BIOS(2) | ROM_SKIP(3) )
	ROMX_LOAD( "a500_ros310_rom3.ic27", 0x000003, 0x80000, CRC(c9bd793b) SHA1(534428a338b10d49b4fc2d9d6ab4c2463e3c3db5), ROM_BIOS(2) | ROM_SKIP(3) )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROMX_LOAD( "cmos_riscos2.bin", 0x0000, 0x0100, CRC(1ecf3369) SHA1(96163285797e0d54017d8d4ae87835328a4658bd), ROM_BIOS(0) )
	ROMX_LOAD( "cmos_riscos2.bin", 0x0000, 0x0100, CRC(1ecf3369) SHA1(96163285797e0d54017d8d4ae87835328a4658bd), ROM_BIOS(1) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(96ed59b2) SHA1(9dab30b4c3305e1142819687889fca334b532679), ROM_BIOS(2) )
ROM_END

ROM_START( aa500d )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "s249", "#249: Arthur 1.20 (25 Sep 1987)" ) // ex Logica, developers of Domesday software
	ROMX_LOAD( "a500_arthur_12_0.ic24", 0x000000, 0x10000, CRC(3d61a13c) SHA1(90b70a30a81c22ba7510cfec62730f77d6c7414a), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "a500_arthur_12_1.ic25", 0x000001, 0x10000, CRC(829d2856) SHA1(454847cafd9d6d37a756205e0109c3e8c463ab92), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "a500_arthur_12_2.ic26", 0x000002, 0x10000, CRC(a68bfab4) SHA1(db55995aeb18a7d73a8beee3bc7388e23cd8a02d), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "a500_arthur_12_3.ic27", 0x000003, 0x10000, CRC(8c56de3f) SHA1(a8b8d1d6638488fe2a328814ab4f2f4566b663b6), ROM_BIOS(0) | ROM_SKIP(3) )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROMX_LOAD( "cmos_arthur.bin",  0x0000, 0x0100, CRC(4fc66ddc) SHA1(f0eae9a535505d82ba3488ddb7895434df940d73), ROM_BIOS(0) )

	//DISK_REGION( "hdc:0:harddisk" )
	//DISK_IMAGE( "a500_domesday", 0, SHA1(094ff299d6564d7b04bed4562f52aaa4cfd503b3) )
ROM_END

ROM_START( aa305 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "030", "Arthur 0.30 (17 Jun 1987)" )
	ROMX_LOAD( "0276,322-01.rom", 0x000000, 0x20000, CRC(e6862d4c) SHA1(13d8470f1cb2c1d15530bc7fa8a95ecc4a371cf3), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "0276,323-01.rom", 0x000001, 0x20000, CRC(a9aeb4cf) SHA1(f37e744ba0e48861815683b24612b0dd69d6ea8b), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "0276,324-01.rom", 0x000002, 0x20000, CRC(7a175186) SHA1(efdee41d9811b05a726af9ba135e0178bb1749e5), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "0276,325-01.rom", 0x000003, 0x20000, CRC(7a8811b8) SHA1(7a6b5d7bb94ff5e2a31469ab024c8b2d4d295709), ROM_BIOS(0) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 1, "120", "Arthur 1.20 (25 Sep 1987)" )
	ROMX_LOAD( "0277,022-02.rom", 0x000000, 0x20000, CRC(03bfe550) SHA1(e4f3f1e37b84e716d75a32aa291a9189371daa1c), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "0277,023-02.rom", 0x000001, 0x20000, CRC(89ece77c) SHA1(e1979a8d3586c006e3837ff721cfa5439e6394bc), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "0277,024-02.rom", 0x000002, 0x20000, CRC(2302db86) SHA1(9fb3761571141bd51936daf03fa4f94fca177356), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "0277,025-02.rom", 0x000003, 0x20000, CRC(1546a0da) SHA1(078ef64aa8a7196d0ee4bca9926a74ade0a36173), ROM_BIOS(1) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 2, "200", "RISC OS 2.00 (05 Oct 1988)" )
	ROMX_LOAD( "0283,022-01.rom", 0x000000, 0x20000, CRC(24291ebf) SHA1(758adaf6f73b4041a680cdf9a0b2107da12ca5a0), ROM_BIOS(2) | ROM_SKIP(3) )
	ROMX_LOAD( "0283,023-01.rom", 0x000001, 0x20000, CRC(44a134f1) SHA1(2db7f06e692c3191b2e131d55a1cf997e226c7c6), ROM_BIOS(2) | ROM_SKIP(3) )
	ROMX_LOAD( "0283,024-01.rom", 0x000002, 0x20000, CRC(997f42b6) SHA1(779fcb13ce4107c27a003cdc848e8a5b7e039268), ROM_BIOS(2) | ROM_SKIP(3) )
	ROMX_LOAD( "0283,025-01.rom", 0x000003, 0x20000, CRC(6335dba2) SHA1(0e0631ee43acf3d005f24a4d5c11c5c60e6f29a2), ROM_BIOS(2) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 3, "201", "RISC OS 2.01 (05 Jul 1990)" )
	ROMX_LOAD( "0270,601-01.rom", 0x000000, 0x20000, CRC(29e2890b) SHA1(2ccdbda7494824180426d66cd38659f6ee55a045), ROM_BIOS(3) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,602-01.rom", 0x000001, 0x20000, CRC(dd1e4893) SHA1(2d39a5027fd164fd9409e38074c68731622406bb), ROM_BIOS(3) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,603-01.rom", 0x000002, 0x20000, CRC(985a8703) SHA1(87376fba36757b311f6c4178c2ac04d8a4ad063a), ROM_BIOS(3) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,604-01.rom", 0x000003, 0x20000, CRC(f23e4c8d) SHA1(27595da90d76d3b25b55e176e0688740c5ce39de), ROM_BIOS(3) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 4, "test", "Diagnostic Test ROMs" ) // Usage described in Archimedes 300 Series Service Manual
	ROMX_LOAD( "0276,146-01.rom", 0x000000, 0x10000, CRC(9c45283c) SHA1(9eb5bd7ad0958f194a3416d79d7e01e4c45741e1), ROM_BIOS(4) | ROM_SKIP(3) )
	ROMX_LOAD( "0276,147-01.rom", 0x000001, 0x10000, CRC(ad94e17f) SHA1(1c8e39c69d4ae1b674e0f732aaa62a4403998f41), ROM_BIOS(4) | ROM_SKIP(3) )
	ROMX_LOAD( "0276,148-01.rom", 0x000002, 0x10000, CRC(1ab02f2d) SHA1(dd7d216967524e64d1a03076a6081461ec8528c3), ROM_BIOS(4) | ROM_SKIP(3) )
	ROMX_LOAD( "0276,149-01.rom", 0x000003, 0x10000, CRC(5fd6a406) SHA1(790af8a4c74d0f6714d528f7502443ce5898a618), ROM_BIOS(4) | ROM_SKIP(3) )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROMX_LOAD( "cmos_arthur.bin",  0x0000, 0x0100, CRC(4fc66ddc) SHA1(f0eae9a535505d82ba3488ddb7895434df940d73), ROM_BIOS(0) )
	ROMX_LOAD( "cmos_arthur.bin",  0x0000, 0x0100, CRC(4fc66ddc) SHA1(f0eae9a535505d82ba3488ddb7895434df940d73), ROM_BIOS(1) )
	ROMX_LOAD( "cmos_riscos2.bin", 0x0000, 0x0100, CRC(1ecf3369) SHA1(96163285797e0d54017d8d4ae87835328a4658bd), ROM_BIOS(2) )
	ROMX_LOAD( "cmos_riscos2.bin", 0x0000, 0x0100, CRC(1ecf3369) SHA1(96163285797e0d54017d8d4ae87835328a4658bd), ROM_BIOS(3) )
ROM_END

ROM_START( aa310 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "120", "Arthur 1.20 (25 Sep 1987)" )
	ROMX_LOAD( "0277,022-02.rom", 0x000000, 0x20000, CRC(03bfe550) SHA1(e4f3f1e37b84e716d75a32aa291a9189371daa1c), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "0277,023-02.rom", 0x000001, 0x20000, CRC(89ece77c) SHA1(e1979a8d3586c006e3837ff721cfa5439e6394bc), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "0277,024-02.rom", 0x000002, 0x20000, CRC(2302db86) SHA1(9fb3761571141bd51936daf03fa4f94fca177356), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "0277,025-02.rom", 0x000003, 0x20000, CRC(1546a0da) SHA1(078ef64aa8a7196d0ee4bca9926a74ade0a36173), ROM_BIOS(0) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 1, "030", "Arthur 0.30 (17 Jun 1987)" )
	ROMX_LOAD( "0276,322-01.rom", 0x000000, 0x20000, CRC(e6862d4c) SHA1(13d8470f1cb2c1d15530bc7fa8a95ecc4a371cf3), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "0276,323-01.rom", 0x000001, 0x20000, CRC(a9aeb4cf) SHA1(f37e744ba0e48861815683b24612b0dd69d6ea8b), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "0276,324-01.rom", 0x000002, 0x20000, CRC(7a175186) SHA1(efdee41d9811b05a726af9ba135e0178bb1749e5), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "0276,325-01.rom", 0x000003, 0x20000, CRC(7a8811b8) SHA1(7a6b5d7bb94ff5e2a31469ab024c8b2d4d295709), ROM_BIOS(1) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 2, "200", "RISC OS 2.00 (05 Oct 1988)" )
	ROMX_LOAD( "0283,022-01.rom", 0x000000, 0x20000, CRC(24291ebf) SHA1(758adaf6f73b4041a680cdf9a0b2107da12ca5a0), ROM_BIOS(2) | ROM_SKIP(3) )
	ROMX_LOAD( "0283,023-01.rom", 0x000001, 0x20000, CRC(44a134f1) SHA1(2db7f06e692c3191b2e131d55a1cf997e226c7c6), ROM_BIOS(2) | ROM_SKIP(3) )
	ROMX_LOAD( "0283,024-01.rom", 0x000002, 0x20000, CRC(997f42b6) SHA1(779fcb13ce4107c27a003cdc848e8a5b7e039268), ROM_BIOS(2) | ROM_SKIP(3) )
	ROMX_LOAD( "0283,025-01.rom", 0x000003, 0x20000, CRC(6335dba2) SHA1(0e0631ee43acf3d005f24a4d5c11c5c60e6f29a2), ROM_BIOS(2) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 3, "201", "RISC OS 2.01 (05 Jul 1990)" )
	ROMX_LOAD( "0270,601-01.rom", 0x000000, 0x20000, CRC(29e2890b) SHA1(2ccdbda7494824180426d66cd38659f6ee55a045), ROM_BIOS(3) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,602-01.rom", 0x000001, 0x20000, CRC(dd1e4893) SHA1(2d39a5027fd164fd9409e38074c68731622406bb), ROM_BIOS(3) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,603-01.rom", 0x000002, 0x20000, CRC(985a8703) SHA1(87376fba36757b311f6c4178c2ac04d8a4ad063a), ROM_BIOS(3) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,604-01.rom", 0x000003, 0x20000, CRC(f23e4c8d) SHA1(27595da90d76d3b25b55e176e0688740c5ce39de), ROM_BIOS(3) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 4, "300", "RISC OS 3.00 (25 Sep 1991)" )
	ROMX_LOAD( "0270,251-01.rom", 0x000000, 0x80000, CRC(023115a9) SHA1(d3233f76d5750e04ef2bc39d5b2dfd96e6a03c45), ROM_BIOS(4) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,252-01.rom", 0x000001, 0x80000, CRC(6db01129) SHA1(4b801dcce4d268d5e4c2680efa23acb29e4f907f), ROM_BIOS(4) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,253-01.rom", 0x000002, 0x80000, CRC(d749a9f2) SHA1(c53c35b847d300989163f9e779590d1853e8adaf), ROM_BIOS(4) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,254-01.rom", 0x000003, 0x80000, CRC(5b13c523) SHA1(b815bdf31dd99e5b4f2d99790cc28ee8cd907b43), ROM_BIOS(4) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 5, "310", "RISC OS 3.10 (30 Apr 1992)" )
	ROMX_LOAD( "0296,041-01.rom", 0x000000, 0x80000, CRC(b7499ef8) SHA1(4ab53a53c531bfbecdd441c82d9f4c0517682dde), ROM_BIOS(5) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,042-01.rom", 0x000001, 0x80000, CRC(d55a854c) SHA1(c9308cee92cca2a626d8577ec99485ad58b8da2a), ROM_BIOS(5) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,043-01.rom", 0x000002, 0x80000, CRC(19bc549a) SHA1(88b02bd3df94b56284ffad1c24fa140249b7cb63), ROM_BIOS(5) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,044-01.rom", 0x000003, 0x80000, CRC(bf86f497) SHA1(a200dca6dbee7c0be25a7e5363a6a3e6455a3bf3), ROM_BIOS(5) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 6, "311", "RISC OS 3.11 (29 Sep 1992)" )
	ROMX_LOAD( "0296,041-02.rom", 0x000000, 0x80000, CRC(84185879) SHA1(2740312b32e9cb8ca6cba9f7b33b68dc0dfab810), ROM_BIOS(6) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,042-02.rom", 0x000001, 0x80000, CRC(c7584553) SHA1(144f8f55f06d6d0752f2f989f4f5c7cec38a43ea), ROM_BIOS(6) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,043-02.rom", 0x000002, 0x80000, CRC(ff5acf17) SHA1(f9c9d4eb2f465b44353257594e631d0e3706f651), ROM_BIOS(6) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,044-02.rom", 0x000003, 0x80000, CRC(e2a3480e) SHA1(5b48e8b66ba86568e2225d60f34e201dd5f5d52a), ROM_BIOS(6) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 7, "test", "Diagnostic Test ROMs" ) // Usage described in Archimedes 300 Series Service Manual
	ROMX_LOAD( "0276,146-01.rom", 0x000000, 0x10000, CRC(9c45283c) SHA1(9eb5bd7ad0958f194a3416d79d7e01e4c45741e1), ROM_BIOS(7) | ROM_SKIP(3) )
	ROMX_LOAD( "0276,147-01.rom", 0x000001, 0x10000, CRC(ad94e17f) SHA1(1c8e39c69d4ae1b674e0f732aaa62a4403998f41), ROM_BIOS(7) | ROM_SKIP(3) )
	ROMX_LOAD( "0276,148-01.rom", 0x000002, 0x10000, CRC(1ab02f2d) SHA1(dd7d216967524e64d1a03076a6081461ec8528c3), ROM_BIOS(7) | ROM_SKIP(3) )
	ROMX_LOAD( "0276,149-01.rom", 0x000003, 0x10000, CRC(5fd6a406) SHA1(790af8a4c74d0f6714d528f7502443ce5898a618), ROM_BIOS(7) | ROM_SKIP(3) )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROMX_LOAD( "cmos_arthur.bin",  0x0000, 0x0100, CRC(4fc66ddc) SHA1(f0eae9a535505d82ba3488ddb7895434df940d73), ROM_BIOS(0) )
	ROMX_LOAD( "cmos_arthur.bin",  0x0000, 0x0100, CRC(4fc66ddc) SHA1(f0eae9a535505d82ba3488ddb7895434df940d73), ROM_BIOS(1) )
	ROMX_LOAD( "cmos_riscos2.bin", 0x0000, 0x0100, CRC(1ecf3369) SHA1(96163285797e0d54017d8d4ae87835328a4658bd), ROM_BIOS(2) )
	ROMX_LOAD( "cmos_riscos2.bin", 0x0000, 0x0100, CRC(1ecf3369) SHA1(96163285797e0d54017d8d4ae87835328a4658bd), ROM_BIOS(3) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(96ed59b2) SHA1(9dab30b4c3305e1142819687889fca334b532679), ROM_BIOS(4) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(96ed59b2) SHA1(9dab30b4c3305e1142819687889fca334b532679), ROM_BIOS(5) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(96ed59b2) SHA1(9dab30b4c3305e1142819687889fca334b532679), ROM_BIOS(6) )
ROM_END

#define rom_aa440 rom_aa310

ROM_START( am4 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD32_BYTE( "m4_arf_0.bin", 0x000000, 0x10000, CRC(b54544c2) SHA1(f75d6b4c8506d1f14f9583b4175af0c8accd8562) )
	ROM_LOAD32_BYTE( "m4_arf_1.bin", 0x000001, 0x10000, CRC(cd6fe9be) SHA1(07acf52a9cc81939998f52836be477285571f332) )
	ROM_LOAD32_BYTE( "m4_arf_2.bin", 0x000002, 0x10000, CRC(575ffc0a) SHA1(816008aa5bd5cfbace07ae4b5e4691951beccf76) )
	ROM_LOAD32_BYTE( "m4_arf_3.bin", 0x000003, 0x10000, CRC(78feaa86) SHA1(476d9c0006b857b51fa272eef819ecfc8b4257d8) )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
ROM_END

ROM_START( aa680 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD32_BYTE( "0274,200-c_boot_rom_0.ic150", 0x000000, 0x20000, CRC(b04c206c) SHA1(9f83c20ba738c3a7dc63ded45151108fd75975bd) )
	ROM_LOAD32_BYTE( "0274,201-c_boot_rom_1.ic151", 0x000001, 0x20000, CRC(baf57404) SHA1(cf4ea48007f57f4e7d7ac3ae782d462fa34d04bf) )
	ROM_LOAD32_BYTE( "0274,202-c_boot_rom_2.ic152", 0x000002, 0x20000, CRC(c9adf722) SHA1(77c613c6b1b4bd49069a18713166ca8d1c926e02) )
	ROM_LOAD32_BYTE( "0274,203-c_boot_rom_3.ic153", 0x000003, 0x20000, CRC(a5d9eb4b) SHA1(2c521c7500dae27d4192067c3e4758180699f0eb) )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
ROM_END

ROM_START( aa3000 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "200", "RISC OS 2.00 (05 Oct 1988)" )
	ROMX_LOAD( "0283,022-01.rom", 0x000000, 0x20000, CRC(24291ebf) SHA1(758adaf6f73b4041a680cdf9a0b2107da12ca5a0), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "0283,023-01.rom", 0x000001, 0x20000, CRC(44a134f1) SHA1(2db7f06e692c3191b2e131d55a1cf997e226c7c6), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "0283,024-01.rom", 0x000002, 0x20000, CRC(997f42b6) SHA1(779fcb13ce4107c27a003cdc848e8a5b7e039268), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "0283,025-01.rom", 0x000003, 0x20000, CRC(6335dba2) SHA1(0e0631ee43acf3d005f24a4d5c11c5c60e6f29a2), ROM_BIOS(0) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 1, "201", "RISC OS 2.01 (05 Jul 1990)" )
	ROMX_LOAD( "0270,601-01.rom", 0x000000, 0x20000, CRC(29e2890b) SHA1(2ccdbda7494824180426d66cd38659f6ee55a045), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,602-01.rom", 0x000001, 0x20000, CRC(dd1e4893) SHA1(2d39a5027fd164fd9409e38074c68731622406bb), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,603-01.rom", 0x000002, 0x20000, CRC(985a8703) SHA1(87376fba36757b311f6c4178c2ac04d8a4ad063a), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,604-01.rom", 0x000003, 0x20000, CRC(f23e4c8d) SHA1(27595da90d76d3b25b55e176e0688740c5ce39de), ROM_BIOS(1) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 2, "300", "RISC OS 3.00 (25 Sep 1991)" )
	ROMX_LOAD( "0270,251-01.rom", 0x000000, 0x80000, CRC(023115a9) SHA1(d3233f76d5750e04ef2bc39d5b2dfd96e6a03c45), ROM_BIOS(2) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,252-01.rom", 0x000001, 0x80000, CRC(6db01129) SHA1(4b801dcce4d268d5e4c2680efa23acb29e4f907f), ROM_BIOS(2) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,253-01.rom", 0x000002, 0x80000, CRC(d749a9f2) SHA1(c53c35b847d300989163f9e779590d1853e8adaf), ROM_BIOS(2) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,254-01.rom", 0x000003, 0x80000, CRC(5b13c523) SHA1(b815bdf31dd99e5b4f2d99790cc28ee8cd907b43), ROM_BIOS(2) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 3, "310", "RISC OS 3.10 (30 Apr 1992)" )
	ROMX_LOAD( "0296,041-01.rom", 0x000000, 0x80000, CRC(b7499ef8) SHA1(4ab53a53c531bfbecdd441c82d9f4c0517682dde), ROM_BIOS(3) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,042-01.rom", 0x000001, 0x80000, CRC(d55a854c) SHA1(c9308cee92cca2a626d8577ec99485ad58b8da2a), ROM_BIOS(3) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,043-01.rom", 0x000002, 0x80000, CRC(19bc549a) SHA1(88b02bd3df94b56284ffad1c24fa140249b7cb63), ROM_BIOS(3) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,044-01.rom", 0x000003, 0x80000, CRC(bf86f497) SHA1(a200dca6dbee7c0be25a7e5363a6a3e6455a3bf3), ROM_BIOS(3) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 4, "311", "RISC OS 3.11 (29 Sep 1992)" )
	ROMX_LOAD( "0296,041-02.rom", 0x000000, 0x80000, CRC(84185879) SHA1(2740312b32e9cb8ca6cba9f7b33b68dc0dfab810), ROM_BIOS(4) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,042-02.rom", 0x000001, 0x80000, CRC(c7584553) SHA1(144f8f55f06d6d0752f2f989f4f5c7cec38a43ea), ROM_BIOS(4) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,043-02.rom", 0x000002, 0x80000, CRC(ff5acf17) SHA1(f9c9d4eb2f465b44353257594e631d0e3706f651), ROM_BIOS(4) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,044-02.rom", 0x000003, 0x80000, CRC(e2a3480e) SHA1(5b48e8b66ba86568e2225d60f34e201dd5f5d52a), ROM_BIOS(4) | ROM_SKIP(3) )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROMX_LOAD( "cmos_riscos2.bin", 0x0000, 0x0100, CRC(1ecf3369) SHA1(96163285797e0d54017d8d4ae87835328a4658bd), ROM_BIOS(0) )
	ROMX_LOAD( "cmos_riscos2.bin", 0x0000, 0x0100, CRC(1ecf3369) SHA1(96163285797e0d54017d8d4ae87835328a4658bd), ROM_BIOS(1) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(96ed59b2) SHA1(9dab30b4c3305e1142819687889fca334b532679), ROM_BIOS(2) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(96ed59b2) SHA1(9dab30b4c3305e1142819687889fca334b532679), ROM_BIOS(3) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(96ed59b2) SHA1(9dab30b4c3305e1142819687889fca334b532679), ROM_BIOS(4) )
ROM_END

#define rom_aa4101 rom_aa3000
#define rom_aa4201 rom_aa3000
#define rom_aa4401 rom_aa3000

ROM_START( ar140 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "200", "RISC OS 2.00 (05 Oct 1988)" )
	ROMX_LOAD( "0283,022-01.rom", 0x000000, 0x20000, CRC(24291ebf) SHA1(758adaf6f73b4041a680cdf9a0b2107da12ca5a0), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "0283,023-01.rom", 0x000001, 0x20000, CRC(44a134f1) SHA1(2db7f06e692c3191b2e131d55a1cf997e226c7c6), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "0283,024-01.rom", 0x000002, 0x20000, CRC(997f42b6) SHA1(779fcb13ce4107c27a003cdc848e8a5b7e039268), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "0283,025-01.rom", 0x000003, 0x20000, CRC(6335dba2) SHA1(0e0631ee43acf3d005f24a4d5c11c5c60e6f29a2), ROM_BIOS(0) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 1, "201", "RISC OS 2.01 (05 Jul 1990)" )
	ROMX_LOAD( "0270,601-01.rom", 0x000000, 0x20000, CRC(29e2890b) SHA1(2ccdbda7494824180426d66cd38659f6ee55a045), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,602-01.rom", 0x000001, 0x20000, CRC(dd1e4893) SHA1(2d39a5027fd164fd9409e38074c68731622406bb), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,603-01.rom", 0x000002, 0x20000, CRC(985a8703) SHA1(87376fba36757b311f6c4178c2ac04d8a4ad063a), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,604-01.rom", 0x000003, 0x20000, CRC(f23e4c8d) SHA1(27595da90d76d3b25b55e176e0688740c5ce39de), ROM_BIOS(1) | ROM_SKIP(3) )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROMX_LOAD( "cmos_riscos2.bin", 0x0000, 0x0100, CRC(1ecf3369) SHA1(96163285797e0d54017d8d4ae87835328a4658bd), ROM_BIOS(0) )
	ROMX_LOAD( "cmos_riscos2.bin", 0x0000, 0x0100, CRC(1ecf3369) SHA1(96163285797e0d54017d8d4ae87835328a4658bd), ROM_BIOS(1) )
ROM_END

ROM_START( aa540 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "201", "RISC OS 2.01 (05 Jul 1990)" )
	ROMX_LOAD( "0270,601-01.rom", 0x000000, 0x20000, CRC(29e2890b) SHA1(2ccdbda7494824180426d66cd38659f6ee55a045), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,602-01.rom", 0x000001, 0x20000, CRC(dd1e4893) SHA1(2d39a5027fd164fd9409e38074c68731622406bb), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,603-01.rom", 0x000002, 0x20000, CRC(985a8703) SHA1(87376fba36757b311f6c4178c2ac04d8a4ad063a), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,604-01.rom", 0x000003, 0x20000, CRC(f23e4c8d) SHA1(27595da90d76d3b25b55e176e0688740c5ce39de), ROM_BIOS(0) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 1, "300", "RISC OS 3.00 (25 Sep 1991)" )
	ROMX_LOAD( "0270,251-01.rom", 0x000000, 0x80000, CRC(023115a9) SHA1(d3233f76d5750e04ef2bc39d5b2dfd96e6a03c45), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,252-01.rom", 0x000001, 0x80000, CRC(6db01129) SHA1(4b801dcce4d268d5e4c2680efa23acb29e4f907f), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,253-01.rom", 0x000002, 0x80000, CRC(d749a9f2) SHA1(c53c35b847d300989163f9e779590d1853e8adaf), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,254-01.rom", 0x000003, 0x80000, CRC(5b13c523) SHA1(b815bdf31dd99e5b4f2d99790cc28ee8cd907b43), ROM_BIOS(1) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 2, "310", "RISC OS 3.10 (30 Apr 1992)" )
	ROMX_LOAD( "0296,041-01.rom", 0x000000, 0x80000, CRC(b7499ef8) SHA1(4ab53a53c531bfbecdd441c82d9f4c0517682dde), ROM_BIOS(2) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,042-01.rom", 0x000001, 0x80000, CRC(d55a854c) SHA1(c9308cee92cca2a626d8577ec99485ad58b8da2a), ROM_BIOS(2) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,043-01.rom", 0x000002, 0x80000, CRC(19bc549a) SHA1(88b02bd3df94b56284ffad1c24fa140249b7cb63), ROM_BIOS(2) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,044-01.rom", 0x000003, 0x80000, CRC(bf86f497) SHA1(a200dca6dbee7c0be25a7e5363a6a3e6455a3bf3), ROM_BIOS(2) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 3, "311", "RISC OS 3.11 (29 Sep 1992)" )
	ROMX_LOAD( "0296,041-02.rom", 0x000000, 0x80000, CRC(84185879) SHA1(2740312b32e9cb8ca6cba9f7b33b68dc0dfab810), ROM_BIOS(3) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,042-02.rom", 0x000001, 0x80000, CRC(c7584553) SHA1(144f8f55f06d6d0752f2f989f4f5c7cec38a43ea), ROM_BIOS(3) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,043-02.rom", 0x000002, 0x80000, CRC(ff5acf17) SHA1(f9c9d4eb2f465b44353257594e631d0e3706f651), ROM_BIOS(3) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,044-02.rom", 0x000003, 0x80000, CRC(e2a3480e) SHA1(5b48e8b66ba86568e2225d60f34e201dd5f5d52a), ROM_BIOS(3) | ROM_SKIP(3) )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROMX_LOAD( "cmos_riscos2.bin", 0x0000, 0x0100, CRC(1ecf3369) SHA1(96163285797e0d54017d8d4ae87835328a4658bd), ROM_BIOS(0) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(96ed59b2) SHA1(9dab30b4c3305e1142819687889fca334b532679), ROM_BIOS(1) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(96ed59b2) SHA1(9dab30b4c3305e1142819687889fca334b532679), ROM_BIOS(2) )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(96ed59b2) SHA1(9dab30b4c3305e1142819687889fca334b532679), ROM_BIOS(3) )
ROM_END

#define rom_ar225 rom_aa540
#define rom_ar260 rom_aa540

ROM_START( aa5000 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_DEFAULT_BIOS("311")
	ROM_SYSTEM_BIOS( 0, "300", "RISC OS 3.00 (25 Sep 1991)" )
	ROMX_LOAD( "0270,251-01.rom", 0x000000, 0x80000, CRC(023115a9) SHA1(d3233f76d5750e04ef2bc39d5b2dfd96e6a03c45), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,252-01.rom", 0x000001, 0x80000, CRC(6db01129) SHA1(4b801dcce4d268d5e4c2680efa23acb29e4f907f), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,253-01.rom", 0x000002, 0x80000, CRC(d749a9f2) SHA1(c53c35b847d300989163f9e779590d1853e8adaf), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "0270,254-01.rom", 0x000003, 0x80000, CRC(5b13c523) SHA1(b815bdf31dd99e5b4f2d99790cc28ee8cd907b43), ROM_BIOS(0) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 1, "310", "RISC OS 3.10 (30 Apr 1992)" )
	ROMX_LOAD( "0296,041-01.rom", 0x000000, 0x80000, CRC(b7499ef8) SHA1(4ab53a53c531bfbecdd441c82d9f4c0517682dde), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,042-01.rom", 0x000001, 0x80000, CRC(d55a854c) SHA1(c9308cee92cca2a626d8577ec99485ad58b8da2a), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,043-01.rom", 0x000002, 0x80000, CRC(19bc549a) SHA1(88b02bd3df94b56284ffad1c24fa140249b7cb63), ROM_BIOS(1) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,044-01.rom", 0x000003, 0x80000, CRC(bf86f497) SHA1(a200dca6dbee7c0be25a7e5363a6a3e6455a3bf3), ROM_BIOS(1) | ROM_SKIP(3) )
	ROM_SYSTEM_BIOS( 2, "311", "RISC OS 3.11 (29 Sep 1992)" )
	ROMX_LOAD( "0296,041-02.rom", 0x000000, 0x80000, CRC(84185879) SHA1(2740312b32e9cb8ca6cba9f7b33b68dc0dfab810), ROM_BIOS(2) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,042-02.rom", 0x000001, 0x80000, CRC(c7584553) SHA1(144f8f55f06d6d0752f2f989f4f5c7cec38a43ea), ROM_BIOS(2) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,043-02.rom", 0x000002, 0x80000, CRC(ff5acf17) SHA1(f9c9d4eb2f465b44353257594e631d0e3706f651), ROM_BIOS(2) | ROM_SKIP(3) )
	ROMX_LOAD( "0296,044-02.rom", 0x000003, 0x80000, CRC(e2a3480e) SHA1(5b48e8b66ba86568e2225d60f34e201dd5f5d52a), ROM_BIOS(2) | ROM_SKIP(3) )

	ROM_REGION( 0x10000, "extension", ROMREGION_ERASE00 )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROM_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(96ed59b2) SHA1(9dab30b4c3305e1142819687889fca334b532679) )

	DISK_REGION( "upc:ide:0:hdd" )
	DISK_IMAGE( "riscos311_apps", 0, SHA1(d69e2fb15d82f83d32786a29dd3321a37a9dbb36) )
ROM_END

#define rom_aa5000a rom_aa5000

ROM_START( aa4 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	// RISC OS 3.10 (30 Apr 1992)
	ROM_LOAD32_WORD( "0296,061-01.ic4",  0x000000, 0x100000, CRC(b77fe215) SHA1(57b19ea4b97a9b6a240aa61211c2c134cb295aa0) )
	ROM_LOAD32_WORD( "0296,062-01.ic15", 0x000002, 0x100000, CRC(d42e196e) SHA1(64243d39d1bca38b10761f66a8042c883bde87a4) )

	ROM_REGION( 0x10000, "extension", ROMREGION_ERASE00 )
	// Power Management
	ROM_LOAD( "0296,063-01.ic38", 0x00000, 0x10000, CRC(9ca3a6be) SHA1(75905b031f49960605d55c3e7350d309559ed440) )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROM_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(96ed59b2) SHA1(9dab30b4c3305e1142819687889fca334b532679) )

	DISK_REGION( "upc:ide:0:hdd" )
	DISK_IMAGE( "riscos311_apps", 0, SHA1(d69e2fb15d82f83d32786a29dd3321a37a9dbb36) )
ROM_END

ROM_START( aa3010 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	// RISC OS 3.11 (29 Sep 1992)
	ROM_LOAD32_WORD( "0296,061-02.ic17", 0x000000, 0x100000, CRC(552fc3aa) SHA1(b2f1911e53d7377f2e69e1a870139745d3df494b) )
	ROM_LOAD32_WORD( "0296,062-02.ic18", 0x000002, 0x100000, CRC(308d5a4a) SHA1(b309e1dd85670a06d77ec504dbbec6c42336329f) )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROM_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(96ed59b2) SHA1(9dab30b4c3305e1142819687889fca334b532679) )
ROM_END

ROM_START( aa3010_de )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	// RISC OS 3.19 (09 Jun 1993)
	ROM_LOAD32_WORD( "0296,241-01.ic17", 0x000000, 0x100000, CRC(8aaf7ff3) SHA1(bc00d90842f40259a48d8f0627d4129e2fa766fe) )
	ROM_LOAD32_WORD( "0296,242-01.ic18", 0x000002, 0x100000, CRC(0ddc807e) SHA1(b0fdb33869cc593123a04fe959c1528f76aac0b9) )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROM_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(96ed59b2) SHA1(9dab30b4c3305e1142819687889fca334b532679) )
ROM_END

ROM_START( aa3020 )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	// RISC OS 3.11 (29 Sep 1992)
	ROM_LOAD32_WORD( "0296,061-02.ic17", 0x000000, 0x100000, CRC(552fc3aa) SHA1(b2f1911e53d7377f2e69e1a870139745d3df494b) )
	ROM_LOAD32_WORD( "0296,062-02.ic18", 0x000002, 0x100000, CRC(308d5a4a) SHA1(b309e1dd85670a06d77ec504dbbec6c42336329f) )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROM_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(96ed59b2) SHA1(9dab30b4c3305e1142819687889fca334b532679) )

	DISK_REGION( "upc:ide:0:hdd" )
	DISK_IMAGE( "riscos311_apps", 0, SHA1(d69e2fb15d82f83d32786a29dd3321a37a9dbb36) )
ROM_END

#define rom_aa4000 rom_aa3020

ROM_START( av20dev )
	ROM_REGION( 0x200000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "320", "RISC OS 3.20 (10 Sep 1992)" )
	ROMX_LOAD( "riscos_vidc20-2_0.rom", 0x000000, 0x80000, CRC(2cdaa10b) SHA1(172bc66124da68e0556878945bfc7e6611cde38f), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "riscos_vidc20-2_1.rom", 0x000001, 0x80000, CRC(2dd7404e) SHA1(a4be1d6874650815435a7c02fb3c681f04b05a4d), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "riscos_vidc20-2_2.rom", 0x000002, 0x80000, CRC(7e9e307a) SHA1(c64d6bda19aedf1e009da6537c019b02666155ff), ROM_BIOS(0) | ROM_SKIP(3) )
	ROMX_LOAD( "riscos_vidc20-2_3.rom", 0x000003, 0x80000, CRC(dc59924c) SHA1(ebd0bdc07ef200640b39b90bd9f89c15ca396089), ROM_BIOS(0) | ROM_SKIP(3) )

	ROM_REGION( 0x100, "i2cmem", ROMREGION_ERASE00 )
	ROMX_LOAD( "cmos_riscos3.bin", 0x0000, 0x0100, CRC(96ed59b2) SHA1(9dab30b4c3305e1142819687889fca334b532679), ROM_BIOS(0) )
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT  COMPAT  MACHINE  INPUT     CLASS         INIT        COMPANY            FULLNAME                                  FLAGS
COMP( 1986, aa500,     0,      0,      aa500,   0,        aa500_state,  init_hd,    "Acorn Computers", "Acorn A500 Development System",          MACHINE_NOT_WORKING )
COMP( 1986, aa500d,    aa500,  0,      aa500d,  0,        aa500_state,  init_hd,    "Acorn Computers", "Acorn A500 Domesday Development System", MACHINE_NOT_WORKING )
COMP( 1987, aa305,     aa310,  0,      aa305,   0,        aa310_state,  init_flop,  "Acorn Computers", "Archimedes 305",                         MACHINE_NOT_WORKING )
COMP( 1987, aa310,     0,      0,      aa310,   0,        aa310_state,  init_flop,  "Acorn Computers", "Archimedes 310",                         MACHINE_NOT_WORKING )
COMP( 1987, aa440,     aa310,  0,      aa440,   0,        aa310_state,  init_hd,    "Acorn Computers", "Archimedes 440",                         MACHINE_NOT_WORKING )
COMP( 1988, am4,       0,      0,      am4,     0,        aa680_state,  empty_init, "Acorn Computers", "Acorn M4",                               MACHINE_NOT_WORKING )
COMP( 1988, aa680,     0,      0,      aa680,   0,        aa680_state,  empty_init, "Acorn Computers", "Acorn A680 UNIX Evaluation System",      MACHINE_NOT_WORKING )
COMP( 1989, aa3000,    aa310,  0,      aa3000,  0,        aa310_state,  init_flop,  "Acorn Computers", "BBC A3000",                              MACHINE_NOT_WORKING )
COMP( 1989, aa4101,    aa310,  0,      aa4101,  0,        aa310_state,  init_flop,  "Acorn Computers", "Archimedes 410/1",                       MACHINE_NOT_WORKING )
COMP( 1989, aa4201,    aa310,  0,      aa4201,  0,        aa310_state,  init_flop,  "Acorn Computers", "Archimedes 420/1",                       MACHINE_NOT_WORKING )
COMP( 1989, aa4401,    aa310,  0,      aa4401,  0,        aa310_state,  init_hd,    "Acorn Computers", "Archimedes 440/1",                       MACHINE_NOT_WORKING )
COMP( 1989, ar140,     aa310,  0,      ar140,   0,        aa310_state,  init_hd,    "Acorn Computers", "Acorn R140",                             MACHINE_NOT_WORKING )
COMP( 1990, aa540,     aa310,  0,      aa540,   0,        aa310_state,  init_scsi,  "Acorn Computers", "Archimedes 540",                         MACHINE_NOT_WORKING )
COMP( 1990, ar225,     aa310,  0,      ar225,   0,        aa310_state,  init_r225,  "Acorn Computers", "Acorn R225",                             MACHINE_NOT_WORKING )
COMP( 1990, ar260,     aa310,  0,      ar260,   0,        aa310_state,  init_scsi,  "Acorn Computers", "Acorn R260",                             MACHINE_NOT_WORKING )
COMP( 1992, av20dev,   aa310,  0,      av20dev, 0,        aa310_state,  init_scsi,  "Acorn Computers", "Acorn V20 (Development)",                MACHINE_NOT_WORKING )
COMP( 1991, aa5000,    0,      0,      aa5000,  0,        aa5000_state, init_ide,   "Acorn Computers", "Acorn A5000",                            MACHINE_NOT_WORKING )
COMP( 1992, aa4,       aa5000, 0,      aa4,     0,        aa4_state,    init_a4,    "Acorn Computers", "Acorn A4",                               MACHINE_NOT_WORKING )
COMP( 1992, aa3010,    0,      0,      aa3010,  aa3010,   aa4000_state, init_flop,  "Acorn Computers", "Acorn A3010",                            MACHINE_NOT_WORKING )
COMP( 1993, aa3010_de, aa3010, 0,      aa3010,  aa3010,   aa4000_state, init_flop,  "Acorn Computers", "Acorn A3010 (German)",                   MACHINE_NOT_WORKING )
COMP( 1992, aa3020,    aa3010, 0,      aa3020,  0,        aa4000_state, init_ide,   "Acorn Computers", "Acorn A3020",                            MACHINE_NOT_WORKING )
COMP( 1992, aa4000,    aa3010, 0,      aa4000,  0,        aa4000_state, init_ide,   "Acorn Computers", "Acorn A4000",                            MACHINE_NOT_WORKING )
COMP( 1993, aa5000a,   aa5000, 0,      aa5000a, 0,        aa5000_state, init_ide,   "Acorn Computers", "Acorn A5000 Alpha",                      MACHINE_NOT_WORKING )
