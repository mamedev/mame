// license:BSD-3-Clause
// copyright-holders:Robert Justice,68bit
/**************************************************************************

    SWTPC S/09 Mess driver
    Robert Justice ,2009-2014

    Emulates four different fixed combinations of hardware
    1. swtpc09
       MP-09 with SBUG rom, MP-ID, MP-S2, MP-T, DMAF2.
       Will boot Flex operating system
    2. swtpc09i
       MP-09 with SBUG rom + HDrom, MP-ID, MP-S2, MP-T, DMAF2, PIAIDE.
       Will boot Flex operating system
       TODO: finish ide part and get this one working.
    3. swtpc09u
       MP-09 with UniBUG rom, MP-ID, MP-S2, DMAF2.
       Will boot UniFlex operating system
    4. swtpc09d3
       MP-09 with UniBUG U3 rom, MP-ID, MP-S2, DMAF3.
       Will boot UniFlex operating system
       TODO: add Harddisk support, DMAF3 has WD1000 interface

***************************************************************************/

#include "emu.h"
#include "includes/swtpc09.h"
#include "bus/ss50/interface.h"
#include "machine/input_merger.h"
#include "formats/flex_dsk.h"
#include "formats/uniflex_dsk.h"


/**************************************************************************
 Address maps

 56K of RAM from 0x0000 to 0xdfff
 2K  of ROM from 0xf800 to 0xffff

 E000 - E003  S2   MC6850 ACIA1   (used for UniFlex console)
 E004 - E007  S2   MC6850 ACIA2   (used for Flex console)
 E014 - E015  DC5  Control reg.
 E016 - E017  DC5  Control reg. 2
 E018 - E01B  DC5  WD2797 FDC
 E040 - E043  MPT  MC6821 PIA + MK5009 timer/counter
 E080 - E08F  MPID MC6821 PIA
 E090 - E09F  MPID MC6840 PTM

 F000 - F01F  DMAF2 MC6844 DMAC
 F020 - F023  DMAF2 WD1791 FDC
 F024 - F03F  DMAF2 Drive select register
 F040 - F041  DMAF2 DMA Address register

 F800 - FFFF  ROM
 FFF0 - FFFF  DAT RAM (only for writes)


 for DMAF3 version
 F000 - F01F  DMAF3 MC6844 DMAC
 F020 - F023  DMAF3 WD1791 FDC
 F024 - F024  DMAF3 Drive select register
 F025 - F025  DMAF3 DMA Address register
 F030 - F03F  DMAF3 WD1000
 F040 - F04F  DMAF3 6522 VIA

***************************************************************************/

/* Address map is dynamically setup when DAT memory is written to  */
/* only ROM from FF00-FFFF and DAT memory at FFF0-FFFF (write only) is guaranteed always*/

void swtpc09_state::mp09_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(swtpc09_state::main_r), FUNC(swtpc09_state::main_w));
	map(0xff00, 0xff0f).mirror(0xf0).writeonly().share("dat");
}

void swtpc09_state::flex_dmaf2_mem(address_map &map)
{
	map(0x00000, 0xfffff).ram(); // by default everything is ram, 1MB ram emulated
	map(0xfe000, 0xfe7ff).rw(FUNC(swtpc09_state::unmapped_r), FUNC(swtpc09_state::unmapped_w));

	// 0xfe004, 0xfe005 ACIA
	map(0xfe000, 0xfe00f).rw("io0", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe010, 0xfe01f).rw("io1", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe020, 0xfe02f).rw("io2", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe030, 0xfe03f).rw("io3", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe040, 0xfe04f).rw("io4", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe050, 0xfe05f).rw("io5", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe060, 0xfe06f).rw("io6", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe070, 0xfe07f).rw("io7", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));

	// MPID
	map(0xfe080, 0xfe083).mirror(0xc).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xfe090, 0xfe097).mirror(0x8).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	// DMAF2
	map(0xff000, 0xff01f).rw(FUNC(swtpc09_state::m6844_r), FUNC(swtpc09_state::m6844_w));
	map(0xff020, 0xff023).rw(FUNC(swtpc09_state::dmaf2_fdc_r), FUNC(swtpc09_state::dmaf2_fdc_w));
	map(0xff024, 0xff03f).rw(FUNC(swtpc09_state::dmaf2_control_reg_r), FUNC(swtpc09_state::dmaf2_control_reg_w));

	map(0xff800, 0xfffff).rom().region("bankdev", 0xff800);
}

void swtpc09_state::flex_dc5_piaide_mem(address_map &map)
{
	map(0x00000, 0xfffff).ram(); // by default everything is ram, 1MB ram emulated
	map(0xfe000, 0xfe7ff).rw(FUNC(swtpc09_state::unmapped_r), FUNC(swtpc09_state::unmapped_w));

	// 0xfe004, 0xfe005 ACIA
	map(0xfe000, 0xfe00f).rw("io0", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	// 0xfe014 0xfe018-0xfe01b DC5 FDC
	map(0xfe010, 0xfe01f).rw("io1", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe020, 0xfe02f).rw("io2", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe030, 0xfe03f).rw("io3", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe040, 0xfe04f).rw("io4", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe050, 0xfe05f).rw("io5", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	// PIA IDE
	map(0xfe060, 0xfe06f).rw("io6", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe070, 0xfe07f).rw("io7", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));

	// MPID
	map(0xfe080, 0xfe083).mirror(0xc).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xfe090, 0xfe097).mirror(0x8).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));

	// TODO How does this mapping change with the bank changes?
	map(0xfe800, 0xfefff).rom().region("bankdev", 0xfe800); //piaide rom

	map(0xff800, 0xfffff).rom().region("bankdev", 0xff800);
}

void swtpc09_state::uniflex_dmaf2_mem(address_map &map)
{
	map(0x00000, 0xfffff).ram(); // by default everything is ram, 1MB ram emulated
	map(0xfe000, 0xfffff).rw(FUNC(swtpc09_state::unmapped_r), FUNC(swtpc09_state::unmapped_w));

	// 0xfe000, 0xfe001 ACIA
	map(0xfe000, 0xfe00f).rw("io0", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe010, 0xfe01f).rw("io1", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe020, 0xfe02f).rw("io2", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe030, 0xfe03f).rw("io3", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe040, 0xfe04f).rw("io4", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe050, 0xfe05f).rw("io5", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe060, 0xfe06f).rw("io6", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe070, 0xfe07f).rw("io7", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));

	// MPID
	map(0xfe080, 0xfe083).mirror(0xc).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xfe090, 0xfe097).mirror(0x8).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));

	// DMAF2
	map(0xff000, 0xff01f).rw(FUNC(swtpc09_state::m6844_r), FUNC(swtpc09_state::m6844_w));
	map(0xff020, 0xff023).rw(FUNC(swtpc09_state::dmaf2_fdc_r), FUNC(swtpc09_state::dmaf2_fdc_w));
	map(0xff024, 0xff03f).rw(FUNC(swtpc09_state::dmaf2_control_reg_r), FUNC(swtpc09_state::dmaf2_control_reg_w));

	map(0xff800, 0xfffff).rom().region("bankdev", 0xff800);
}

void swtpc09_state::uniflex_dmaf3_mem(address_map &map)
{
	map(0x00000, 0xfffff).ram(); // by default everything is ram, 1MB ram emulated
	map(0xfe000, 0xfffff).rw(FUNC(swtpc09_state::unmapped_r), FUNC(swtpc09_state::unmapped_w));

	// 0xfe000, 0xfe001 ACIA
	map(0xfe000, 0xfe00f).rw("io0", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe010, 0xfe01f).rw("io1", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe020, 0xfe02f).rw("io2", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe030, 0xfe03f).rw("io3", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe040, 0xfe04f).rw("io4", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe050, 0xfe05f).rw("io5", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe060, 0xfe06f).rw("io6", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));
	map(0xfe070, 0xfe07f).rw("io7", FUNC(ss50_interface_port_device::read), FUNC(ss50_interface_port_device::write));

	// MPID
	map(0xfe080, 0xfe083).mirror(0xc).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xfe090, 0xfe097).mirror(0x8).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));

	// DMAF3
	map(0xff000, 0xff01f).rw(FUNC(swtpc09_state::m6844_r), FUNC(swtpc09_state::m6844_w));
	map(0xff020, 0xff023).rw(FUNC(swtpc09_state::dmaf3_fdc_r), FUNC(swtpc09_state::dmaf3_fdc_w));
	map(0xff024, 0xff024).rw(FUNC(swtpc09_state::dmaf3_control_reg_r), FUNC(swtpc09_state::dmaf3_control_reg_w));
	map(0xff025, 0xff025).rw(FUNC(swtpc09_state::dmaf3_dma_address_reg_r), FUNC(swtpc09_state::dmaf3_dma_address_reg_w));
	map(0xff040, 0xff04f).m(m_via, FUNC(via6522_device::map));

	// DMAF3 WD1000
	map(0xff030, 0xff03f).rw(FUNC(swtpc09_state::dmaf3_wd_r), FUNC(swtpc09_state::dmaf3_wd_w));

	map(0xff800, 0xfffff).rom().region("bankdev", 0xff800);
}


/* Input ports */
static INPUT_PORTS_START( swtpc09 )
	// The MP09 was available in 4MHz or 8MHz XTALs, running the 6809 at
	// 1MHz or 2MHz respectively. Also allow an overclock option.
	//
	// Run the 6809 at 2MHz (an 8MHz XTAL) so that manual polling of FDC can
	// keep up with higher rate operation. The MPU09 did have the option of
	// 1MHz or 2MHz operation.
	PORT_START("maincpu_clock")
	PORT_CONFNAME(0xffffff, 2000000, "CPU clock")
	PORT_CONFSETTING(1000000, "1.0 MHz")
	PORT_CONFSETTING(2000000, "2.0 MHz")
	PORT_CONFSETTING(4000000, "4.0 MHz")

	// The DMAF2/3 clock can be jumpered selected for 1 or 2MHz, for 5.24"
	// and 8" drives. Some other options are also provided here.
	//
	// single or double density 5.24"  -  1.0MHz
	// 'standard' 3.5"  -  1.2MHz
	// 3.5" hd  -  2.0MHz
	// 8" 360rpm  -  2.4MHz
	PORT_START("fdc_clock")
	PORT_DIPNAME(0xffffff, 2000000, "DMAF2/3 FDC clock")
	PORT_DIPSETTING(1000000, "1.0 MHz")
	PORT_DIPSETTING(1200000, "1.2 MHz")
	PORT_DIPSETTING(2000000, "2.0 MHz")
	PORT_DIPSETTING(2400000, "2.4 MHz")

	// The MP-ID board has jumpers to vary the baud rates generated. The
	// most useful is the Low/High rate jumper that provide a four times
	// rate increase.
	PORT_START("baud_rate_high")
	PORT_DIPNAME(0x1, 0, "High baud rate")
	PORT_DIPSETTING(0, "Low (x1)")
	PORT_DIPSETTING(1, "High (x4)")

	// Debug aid to hard code the density. The FLEX format uses single
	// density for track zero. Many virtual disks 'fix' the format to be
	// purely double density and often without properly implementing
	// driver support for that. This setting is an aid to report
	// unexpected usage, and it attempts to correct that. It is possible
	// to patch the software to work with these pure double density disks.
	PORT_START("floppy_expected_density")
	PORT_CONFNAME(0xff, 0, "DMAF2/3 expected density")
	PORT_CONFSETTING(0, "-")
	PORT_CONFSETTING(1, "single density")
	PORT_CONFSETTING(2, "double density, with single density track zero")
	PORT_CONFSETTING(3, "double density")

	// Debug aid, to check that the sector head or side is set as expected
	// for the sector ID being read for the FLEX floppy disk format. Many
	// FLEX disk images were developed for vitural machines that have
	// little regard for the actual head and can work off the sector ID
	// and their drivers can set the head incorrectly. E.g. a disk with 18
	// sectors per side might have a drive set to switch heads for sectors
	// above 10. Another issue is that double density disk images are
	// often 'fixed' so that they are pure double density without being
	// single density onthe first track, and the drivers might still set
	// the head based track zero being single density. This aid is not
	// intended to be a substitute for fixing the drivers but it does help
	// work through the issues while patching the disks.
	PORT_START("floppy_expected_sectors")
	PORT_CONFNAME(0xff, 0, "DMAF2/3 expected sectors per side")
	PORT_CONFSETTING(0, "-")
	PORT_CONFSETTING(10, "10") // 5 1/4" single density 256B
	PORT_CONFSETTING(15, "15") // 8" single density 256B
	PORT_CONFSETTING(18, "18") // 5 1/4" double density 256B
	PORT_CONFSETTING(26, "26") // 8" double density 256B
	PORT_CONFSETTING(36, "36") // 3.5" 1.4M QD 256B
	// The track zero expected sectors if different from the above. FLEX
	// 6800 disks did format track zero in single density and if the
	// driver sticks to that and if using a double density disk then set a
	// single density size here.
	PORT_START("floppy_track_zero_expected_sectors")
	PORT_CONFNAME(0xff, 0, "DMAF2/3 track zero expected sectors per side")
	PORT_CONFSETTING(0, "-")
	PORT_CONFSETTING(10, "10") // 5 1/4" single density 256B
	PORT_CONFSETTING(15, "15") // 8" single density 256B
	PORT_CONFSETTING(18, "18") // 5 1/4" double density 256B
	PORT_CONFSETTING(26, "26") // 8" double density 256B
	PORT_CONFSETTING(36, "36") // 3.5" 1.4M QD 256B

	PORT_START("sbug_double_density")
	PORT_CONFNAME(0x1, 0, "SBUG patch for double density (SSO) disk boot")
	PORT_CONFSETTING(0, "No - single density")
	PORT_CONFSETTING(1, "Yes - double density")

	// The PIA IDE PROM includes initialization code that patches FLEX
	// after the disk boot code has loaded FLEX, and then it jumps to
	// 0xc850 to cold start FLEX. Have seen 0xcd00 being the cold start
	// address, so add an option to patch the PROM for that.
	PORT_START("piaide_flex_boot_cd00")
	PORT_CONFNAME(0x1, 0, "PIA IDE PROM patch FLEX entry to 0xcd00")
	PORT_CONFSETTING(0, "No - FLEX entry 0xc850")
	PORT_CONFSETTING(1, "Yes - FLEX entry 0xcd00")

INPUT_PORTS_END


static DEVICE_INPUT_DEFAULTS_START( dc5 )
	DEVICE_INPUT_DEFAULTS("address_mode", 0xf, 1)
DEVICE_INPUT_DEFAULTS_END


FLOPPY_FORMATS_MEMBER( swtpc09_state::floppy_flex_formats )
	FLOPPY_MFI_FORMAT,
	FLOPPY_FLEX_FORMAT
FLOPPY_FORMATS_END

FLOPPY_FORMATS_MEMBER( swtpc09_state::floppy_uniflex_formats )
	FLOPPY_MFI_FORMAT,
	FLOPPY_UNIFLEX_FORMAT
FLOPPY_FORMATS_END


// todo: implement floppy controller cards as slot devices and do this properly
static void swtpc09_flex_floppies(device_slot_interface &device)
{
	device.option_add("sssd35", FLOPPY_525_SSSD_35T); // 35 trks ss sd 5.25
	device.option_add("sssd",   FLOPPY_525_SSSD);     // 40 trks ss sd 5.25
	device.option_add("dssd35", FLOPPY_525_SD_35T);   // 35 trks ds sd 5.25
	device.option_add("dssd",   FLOPPY_525_SD);       // 40 trks ds sd 5.25
	device.option_add("ssdd",   FLOPPY_525_SSDD);     // 40 trks ss dd 5.25
	device.option_add("dd",     FLOPPY_525_DD);       // 40 trks ds dd 5.25
	device.option_add("ssqd",   FLOPPY_525_SSQD);     // 80 trks ss dd 5.25
	device.option_add("qd",     FLOPPY_525_QD);       // 80 trks ds dd 5.25
	device.option_add("8sssd",  FLOPPY_8_SSSD);       // 77 trks ss sd 8"
	device.option_add("8dssd",  FLOPPY_8_DSSD);       // 77 trks ds sd 8"
	device.option_add("8ssdd",  FLOPPY_8_SSDD);       // 77 trks ss dd 8"
	device.option_add("8dsdd",  FLOPPY_8_DSDD);       // 77 trks ds dd 8"
	device.option_add("35hd",   FLOPPY_35_HD);        // 1.44mb disk
}

static void swtpc09_uniflex_floppies(device_slot_interface &device)
{
	device.option_add("8dssd",  FLOPPY_8_DSSD);       // 8 inch ds sd
	device.option_add("8dsdd",  FLOPPY_8_DSDD);       // 8 inch ds dd
 }


/***************************************************************************
 Machine definitions
****************************************************************************/

/* Machine driver */
/* MPU09, MPID, MPS2 */
void swtpc09_state::swtpc09_base(machine_config &config)
{
	/* basic machine hardware */
	MC6809(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &swtpc09_state::mp09_mem);

	ADDRESS_MAP_BANK(config, m_bankdev, 0);
	m_bankdev->set_endianness(ENDIANNESS_BIG);
	m_bankdev->set_data_width(8);
	m_bankdev->set_addr_width(20);
	m_bankdev->set_addrmap(AS_PROGRAM, &swtpc09_state::flex_dmaf2_mem);

	// IO0 at 0xe000 is used for the MP-S2 ACIA.
	ss50_interface_port_device &io0(SS50_INTERFACE(config, "io0", ss50_default_2rs_devices, nullptr));
	io0.irq_cb().set("mainirq", FUNC(input_merger_device::in_w<0>));
	io0.firq_cb().set("mainfirq", FUNC(input_merger_device::in_w<0>));
	// IO1 at 0xe010 is used for the DC5 FDC.
	ss50_interface_port_device &io1(SS50_INTERFACE(config, "io1", ss50_default_2rs_devices, nullptr));
	io1.irq_cb().set("mainirq", FUNC(input_merger_device::in_w<1>));
	io1.firq_cb().set("mainfirq", FUNC(input_merger_device::in_w<1>));
	ss50_interface_port_device &io2(SS50_INTERFACE(config, "io2", ss50_default_2rs_devices, nullptr));
	io2.irq_cb().set("mainirq", FUNC(input_merger_device::in_w<2>));
	io2.firq_cb().set("mainfirq", FUNC(input_merger_device::in_w<2>));
	ss50_interface_port_device &io3(SS50_INTERFACE(config, "io3", ss50_default_2rs_devices, nullptr));
	io3.irq_cb().set("mainirq", FUNC(input_merger_device::in_w<3>));
	io3.firq_cb().set("mainfirq", FUNC(input_merger_device::in_w<3>));
	ss50_interface_port_device &io4(SS50_INTERFACE(config, "io4", ss50_default_2rs_devices, nullptr));
	io4.irq_cb().set("mainirq", FUNC(input_merger_device::in_w<4>));
	io4.firq_cb().set("mainfirq", FUNC(input_merger_device::in_w<4>));
	ss50_interface_port_device &io5(SS50_INTERFACE(config, "io5", ss50_default_2rs_devices, nullptr));
	io5.irq_cb().set("mainirq", FUNC(input_merger_device::in_w<5>));
	io5.firq_cb().set("mainfirq", FUNC(input_merger_device::in_w<5>));
	// IO6 at 0xe060 is used for PIA-IDE
	ss50_interface_port_device &io6(SS50_INTERFACE(config, "io6", ss50_default_2rs_devices, nullptr));
	io6.irq_cb().set("mainirq", FUNC(input_merger_device::in_w<6>));
	io6.firq_cb().set("mainfirq", FUNC(input_merger_device::in_w<6>));
	ss50_interface_port_device &io7(SS50_INTERFACE(config, "io7", ss50_default_2rs_devices, nullptr));
	io7.irq_cb().set("mainirq", FUNC(input_merger_device::in_w<7>));
	io7.firq_cb().set("mainfirq", FUNC(input_merger_device::in_w<7>));

	// IO8 at 0xe080 is used internally by the MPID board PIA.
	PIA6821(config, m_pia, 0);
	m_pia->readpa_handler().set(FUNC(swtpc09_state::pia0_a_r));
	m_pia->readca1_handler().set(FUNC(swtpc09_state::pia0_ca1_r));
	m_pia->irqa_handler().set(FUNC(swtpc09_state::pia0_irq_a));

	// IO9 at 0xe090 is used internally by the MPID board 6840 timer.
	PTM6840(config, m_ptm, 2000000);
	m_ptm->set_external_clocks(50, 0, 50);
	m_ptm->o1_callback().set(FUNC(swtpc09_state::ptm_o1_callback));
	m_ptm->o3_callback().set(FUNC(swtpc09_state::ptm_o3_callback));
	m_ptm->irq_callback().set(FUNC(swtpc09_state::ptm_irq));

	// MP-ID baud rate generator
	MC14411(config, m_brg, 1.8432_MHz_XTAL);
	// 9600b / 38400b
	m_brg->out_f<1>().set("io0", FUNC(ss50_interface_port_device::f150_9600_w));
	m_brg->out_f<1>().append("io1", FUNC(ss50_interface_port_device::f150_9600_w));
	m_brg->out_f<1>().append("io2", FUNC(ss50_interface_port_device::f150_9600_w));
	m_brg->out_f<1>().append("io3", FUNC(ss50_interface_port_device::f150_9600_w));
	m_brg->out_f<1>().append("io4", FUNC(ss50_interface_port_device::f150_9600_w));
	m_brg->out_f<1>().append("io5", FUNC(ss50_interface_port_device::f150_9600_w));
	m_brg->out_f<1>().append("io6", FUNC(ss50_interface_port_device::f150_9600_w));
	m_brg->out_f<1>().append("io7", FUNC(ss50_interface_port_device::f150_9600_w));
	// 4800b / 19200b
	m_brg->out_f<3>().set("io0", FUNC(ss50_interface_port_device::f600_4800_w));
	m_brg->out_f<3>().append("io1", FUNC(ss50_interface_port_device::f600_4800_w));
	m_brg->out_f<3>().append("io2", FUNC(ss50_interface_port_device::f600_4800_w));
	m_brg->out_f<3>().append("io3", FUNC(ss50_interface_port_device::f600_4800_w));
	m_brg->out_f<3>().append("io4", FUNC(ss50_interface_port_device::f600_4800_w));
	m_brg->out_f<3>().append("io5", FUNC(ss50_interface_port_device::f600_4800_w));
	m_brg->out_f<3>().append("io6", FUNC(ss50_interface_port_device::f600_4800_w));
	m_brg->out_f<3>().append("io7", FUNC(ss50_interface_port_device::f600_4800_w));
	// 1200b / 4800b
	m_brg->out_f<7>().set("io0", FUNC(ss50_interface_port_device::f600_1200_w));
	m_brg->out_f<7>().append("io1", FUNC(ss50_interface_port_device::f600_1200_w));
	m_brg->out_f<7>().append("io2", FUNC(ss50_interface_port_device::f600_1200_w));
	m_brg->out_f<7>().append("io3", FUNC(ss50_interface_port_device::f600_1200_w));
	m_brg->out_f<7>().append("io4", FUNC(ss50_interface_port_device::f600_1200_w));
	m_brg->out_f<7>().append("io5", FUNC(ss50_interface_port_device::f600_1200_w));
	m_brg->out_f<7>().append("io6", FUNC(ss50_interface_port_device::f600_1200_w));
	m_brg->out_f<7>().append("io7", FUNC(ss50_interface_port_device::f600_1200_w));
	// 300b / 1200b
	m_brg->out_f<9>().set("io0", FUNC(ss50_interface_port_device::f300_w));
	m_brg->out_f<9>().append("io1", FUNC(ss50_interface_port_device::f300_w));
	m_brg->out_f<9>().append("io2", FUNC(ss50_interface_port_device::f300_w));
	m_brg->out_f<9>().append("io3", FUNC(ss50_interface_port_device::f300_w));
	m_brg->out_f<9>().append("io4", FUNC(ss50_interface_port_device::f300_w));
	m_brg->out_f<9>().append("io5", FUNC(ss50_interface_port_device::f300_w));
	m_brg->out_f<9>().append("io6", FUNC(ss50_interface_port_device::f300_w));
	m_brg->out_f<9>().append("io7", FUNC(ss50_interface_port_device::f300_w));
	// 110b / 440b
	m_brg->out_f<13>().set("io0", FUNC(ss50_interface_port_device::f110_w));
	m_brg->out_f<13>().append("io1", FUNC(ss50_interface_port_device::f110_w));
	m_brg->out_f<13>().append("io2", FUNC(ss50_interface_port_device::f110_w));
	m_brg->out_f<13>().append("io3", FUNC(ss50_interface_port_device::f110_w));
	m_brg->out_f<13>().append("io4", FUNC(ss50_interface_port_device::f110_w));
	m_brg->out_f<13>().append("io5", FUNC(ss50_interface_port_device::f110_w));
	m_brg->out_f<13>().append("io6", FUNC(ss50_interface_port_device::f110_w));
	m_brg->out_f<13>().append("io7", FUNC(ss50_interface_port_device::f110_w));

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set(FUNC(swtpc09_state::io_irq_w));
	INPUT_MERGER_ANY_HIGH(config, "mainfirq").output_handler().set_inputline(m_maincpu, M6809_FIRQ_LINE);

	subdevice<ss50_interface_port_device>("io0")->set_default_option("mps2");
}

/* MPU09, MPID, MPS2, DMAF2 */
void swtpc09_state::swtpc09(machine_config &config)
{
	swtpc09_base(config);

	// DMAF2
	FD1797(config, m_fdc, 2000000);
	FLOPPY_CONNECTOR(config, "fdc:0", swtpc09_flex_floppies, "dd", swtpc09_state::floppy_flex_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", swtpc09_flex_floppies, "dd", swtpc09_state::floppy_flex_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", swtpc09_flex_floppies, "dd", swtpc09_state::floppy_flex_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", swtpc09_flex_floppies, "dd", swtpc09_state::floppy_flex_formats).enable_sound(true);

	m_fdc->intrq_wr_callback().set(FUNC(swtpc09_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(swtpc09_state::fdc_drq_w));
	m_fdc->sso_wr_callback().set(FUNC(swtpc09_state::fdc_sso_w));
}

/* MPU09, MPID, MPS2, DC5, MPT, PIAIDE*/
void swtpc09_state::swtpc09i(machine_config &config)
{
	swtpc09_base(config);
	m_bankdev->set_addrmap(AS_PROGRAM, &swtpc09_state::flex_dc5_piaide_mem);

	subdevice<ss50_interface_port_device>("io1")->set_default_option("dc5");
	subdevice<ss50_interface_port_device>("io1")->set_option_device_input_defaults("dc5", DEVICE_INPUT_DEFAULTS_NAME(dc5));
	subdevice<ss50_interface_port_device>("io4")->set_default_option("mpt");
	subdevice<ss50_interface_port_device>("io6")->set_default_option("piaide");
}


void swtpc09_state::swtpc09u(machine_config &config)
{
	swtpc09_base(config);
	m_bankdev->set_addrmap(AS_PROGRAM, &swtpc09_state::uniflex_dmaf2_mem);

	// DMAF2
	FD1797(config, m_fdc, 2400000);
	FLOPPY_CONNECTOR(config, "fdc:0", swtpc09_uniflex_floppies, "8dsdd", swtpc09_state::floppy_uniflex_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", swtpc09_uniflex_floppies, "8dsdd", swtpc09_state::floppy_uniflex_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", swtpc09_uniflex_floppies, "8dsdd", swtpc09_state::floppy_uniflex_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", swtpc09_uniflex_floppies, "8dsdd", swtpc09_state::floppy_uniflex_formats).enable_sound(true);

	m_fdc->intrq_wr_callback().set(FUNC(swtpc09_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(swtpc09_state::fdc_drq_w));
	m_fdc->sso_wr_callback().set(FUNC(swtpc09_state::fdc_sso_w));
}


/* MPU09, MPID, MPS2 DMAF3 */
void swtpc09_state::swtpc09d3(machine_config &config)
{
	swtpc09_base(config);
	m_pia->set_clock(2000000);
	m_bankdev->set_addrmap(AS_PROGRAM, &swtpc09_state::uniflex_dmaf3_mem);

	// DMAF3
	FD1797(config, m_fdc, 2400000);
	FLOPPY_CONNECTOR(config, "fdc:0", swtpc09_uniflex_floppies, "8dsdd", swtpc09_state::floppy_uniflex_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", swtpc09_uniflex_floppies, "8dsdd", swtpc09_state::floppy_uniflex_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:2", swtpc09_uniflex_floppies, "8dsdd", swtpc09_state::floppy_uniflex_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:3", swtpc09_uniflex_floppies, "8dsdd", swtpc09_state::floppy_uniflex_formats).enable_sound(true);

	m_fdc->intrq_wr_callback().set(FUNC(swtpc09_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(swtpc09_state::fdc_drq_w));
	m_fdc->sso_wr_callback().set(FUNC(swtpc09_state::fdc_sso_w));

	via6522_device &via(VIA6522(config, "via", 4_MHz_XTAL / 4));
	via.readpa_handler().set(FUNC(swtpc09_state::dmaf3_via_read_porta));
	via.readpb_handler().set(FUNC(swtpc09_state::dmaf3_via_read_portb));
	via.writepa_handler().set(FUNC(swtpc09_state::dmaf3_via_write_porta));
	//via.ca1_handler().set(FUNC(swtpc09_state::dmaf3_via_write_ca1));
	via.irq_handler().set(FUNC(swtpc09_state::dmaf3_via_irq));
}


/* ROM definition */
ROM_START( swtpc09 )
	ROM_REGION( 0x100000, "bankdev", 0 )
	ROM_LOAD ( "sbugh1-8.bin", 0xff800, 0x0800, CRC(10a045a7) SHA1(de547b77653951c7424a069520d52c5b0432e98d) )
ROM_END

ROM_START( swtpc09i )
	ROM_REGION( 0x100000, "bankdev", 0 )
	ROM_LOAD ( "hd-rom.bin", 0xfe800, 0x0800, CRC(b898b4d7) SHA1(2806633eda7da4e9a243fc534f15526ee928b6bc) )
	ROM_LOAD ( "sbugh1-8.bin", 0xff800, 0x0800, CRC(10a045a7) SHA1(de547b77653951c7424a069520d52c5b0432e98d) )
ROM_END

ROM_START( swtpc09u )
	ROM_REGION( 0x100000, "bankdev", 0 )
	ROM_LOAD ( "unibug.bin", 0xff800, 0x00800, CRC(92e1cbf2) SHA1(db00f17ee9accdbfa1775fe0162d3556159b8e70) )
ROM_END

ROM_START( swtpc09d3 )
	ROM_REGION( 0x100000, "bankdev", 0 )
	ROM_LOAD ( "uos3.bin", 0xff800, 0x00800, CRC(e95eb3e0) SHA1(3e971d3b7e143bc87e4b506e18a8c928c089c25a) )
ROM_END

/* Driver */

//    YEAR  NAME       PARENT   COMPAT  MACHINE    INPUT    CLASS          INIT            COMPANY  FULLNAME                    FLAGS
COMP( 1980, swtpc09,   0,       0,      swtpc09,   swtpc09, swtpc09_state, init_swtpc09,   "SWTPC", "swtpc S/09 Sbug",          MACHINE_NO_SOUND_HW )
COMP( 1980, swtpc09i,  swtpc09, 0,      swtpc09i,  swtpc09, swtpc09_state, init_swtpc09i,  "SWTPC", "swtpc S/09 Sbug + piaide", MACHINE_NO_SOUND_HW )
COMP( 1980, swtpc09u,  swtpc09, 0,      swtpc09u,  swtpc09, swtpc09_state, init_swtpc09u,  "SWTPC", "swtpc S/09 UNIBug + DMAF2", MACHINE_NO_SOUND_HW )
COMP( 1980, swtpc09d3, swtpc09, 0,      swtpc09d3, swtpc09, swtpc09_state, init_swtpc09d3, "SWTPC", "swtpc S/09 UNIBug + DMAF3", MACHINE_NO_SOUND_HW )
