// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev

/*
 * Besta HCPU30 board, skeleton driver.
 *
 * Supported by SysV R3 "Bestix" port and also by Linux port,
 * see https://github.com/shattered/linux-m68k
 *
 * 68030 @ 33 MHz - primary CPU
 * 68882 @ ?? Mhz - FPU
 * 68020 @ ?? MHz - I/O CPU (using shared memory region)
 *
 * 4 or 16 MB of DRAM
 * 8 or 32 KB of NVRAM
 *
 * WD33C93 - SCSI
 * DP8473 - Floppy
 * i82586? - Ethernet
 * 62421 - Real-time clock
 */

#include "emu.h"
#include "vme_hcpu30.h"

//#define LOG_GENERAL (1U <<  0)
#define LOG_SETUP   (1U <<  1)
#define LOG_INT     (1U <<  2)

//#define VERBOSE (LOG_GENERAL | LOG_SETUP | LOG_INT)
//#define LOG_OUTPUT_FUNC printf

#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP, __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,   __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif


#define DUSCC_CLOCK XTAL(14'745'600) /* XXX Unverified */
#define RS232P1_TAG      "rs232p1"
#define RS232P2_TAG      "rs232p2"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(VME_HCPU30, vme_hcpu30_card_device, "hcpu30", "Besta HCPU30 CPU board")

void vme_hcpu30_card_device::hcpu30_mem(address_map &map)
{
	map(0x00000000, 0x00000007).ram().w(this, FUNC(vme_hcpu30_card_device::bootvect_w));   /* After first write we act as RAM */
	map(0x00000000, 0x00000007).rom().r(this, FUNC(vme_hcpu30_card_device::bootvect_r));   /* ROM mirror just during reset */
	map(0x00000008, 0x001fffff).ram(); // local bus DRAM, 4MB
	map(0x00200000, 0x00201fff).ram(); // AM_SHARE("iocpu")
	map(0xff000000, 0xff007fff).rom().mirror(0x8000).region("user1", 0);
	map(0xff020000, 0xff027fff).ram().mirror(0x8000); // SRAM 32KB
	map(0xffff8000, 0xffff8fff).unmaprw(); // shared memory with iocpu
	map(0xffff9000, 0xffff9fff).ram();   // SRAM, optional: battery-backed
	map(0xfffff100, 0xfffff11f).unmaprw(); // VME bus configuration (accessed after DIP switch read)
	map(0xfffff120, 0xfffff123).portr("SA1");
	map(0xfffff200, 0xfffff2ff).unmaprw();
	map(0xfffff300, 0xfffff3ff).rw("duscc", FUNC(duscc68562_device::read), FUNC(duscc68562_device::write));
	map(0xfffff600, 0xfffff6ff).unmaprw();
	map(0xfffff700, 0xfffff7ff).unmaprw();
}

static INPUT_PORTS_START(hcpu30)
	PORT_START("SA1")
	PORT_DIPNAME(0x03000000, 0x00000000, "Console port speed")
	PORT_DIPSETTING(0x00000000, "9600")
	PORT_DIPSETTING(0x01000000, "19200")
	PORT_DIPSETTING(0x02000000, "38400")
	PORT_DIPSETTING(0x03000000, "4800")
	PORT_DIPNAME(0x04000000, 0x04000000, "Boot into...")
	PORT_DIPSETTING(0x00000000, "UNIX")
	PORT_DIPSETTING(0x04000000, "Monitor")
	PORT_DIPNAME(0x10000000, 0x00000000, "VME bus width")
	PORT_DIPSETTING(0x00000000, "32 bits")
	PORT_DIPSETTING(0x10000000, "16 bits")
	PORT_DIPNAME(0x20000000, 0x00000000, "VME bus free")
	PORT_DIPSETTING(0x00000000, "ROR")
	PORT_DIPSETTING(0x20000000, "not ROR")
	PORT_DIPNAME(0x40000000, 0x00000000, "Cache burst mode")
	PORT_DIPSETTING(0x00000000, "Off")
	PORT_DIPSETTING(0x40000000, "On")
	PORT_DIPNAME(0x80000000, 0x00000000, "Undefined")
	PORT_DIPSETTING(0x00000000, "Off")
	PORT_DIPSETTING(0x80000000, "On")
INPUT_PORTS_END

ROM_START(hcpu30)
	ROM_REGION32_BE(0x8000, "user1", ROMREGION_ERASEFF)
	ROM_LOAD("hcpu30.27c256.dat", 0x0000, 0x8000, CRC(d24da66e) SHA1(5431b0559b168a995e725b35e1465a0b8ee8aa72))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *vme_hcpu30_card_device::device_rom_region() const
{
	return ROM_NAME(hcpu30);
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor vme_hcpu30_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hcpu30);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(vme_hcpu30_card_device::device_add_mconfig)
	MCFG_DEVICE_ADD("maincpu", M68030, 2*16670000)
	MCFG_DEVICE_PROGRAM_MAP(hcpu30_mem)

	MCFG_DUSCC68562_ADD("duscc", DUSCC_CLOCK, 0, 0, 0, 0)
	MCFG_DUSCC_OUT_TXDA_CB(WRITELINE(RS232P1_TAG, rs232_port_device, write_txd))
	MCFG_DUSCC_OUT_DTRA_CB(WRITELINE(RS232P1_TAG, rs232_port_device, write_dtr))
	MCFG_DUSCC_OUT_RTSA_CB(WRITELINE(RS232P1_TAG, rs232_port_device, write_rts))
	MCFG_DUSCC_OUT_TXDB_CB(WRITELINE(RS232P2_TAG, rs232_port_device, write_txd))
	MCFG_DUSCC_OUT_DTRB_CB(WRITELINE(RS232P2_TAG, rs232_port_device, write_dtr))
	MCFG_DUSCC_OUT_RTSB_CB(WRITELINE(RS232P2_TAG, rs232_port_device, write_rts))
//  MCFG_DUSCC_OUT_INT_CB(WRITELINE()

	MCFG_DEVICE_ADD (RS232P1_TAG, RS232_PORT, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER (WRITELINE ("duscc", duscc68562_device, rxa_w))
	MCFG_RS232_CTS_HANDLER (WRITELINE ("duscc", duscc68562_device, ctsa_w))

	MCFG_DEVICE_ADD (RS232P2_TAG, RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER (WRITELINE ("duscc", duscc68562_device, rxb_w))
	MCFG_RS232_CTS_HANDLER (WRITELINE ("duscc", duscc68562_device, ctsb_w))
MACHINE_CONFIG_END

/* Boot vector handler, the PCB hardwires the first 8 bytes from 0xff800000 to 0x0 at reset */
READ32_MEMBER(vme_hcpu30_card_device::bootvect_r)
{
	LOG("%s\n", FUNCNAME);
	return m_sysrom[offset];
}

WRITE32_MEMBER(vme_hcpu30_card_device::bootvect_w)
{
	LOG("%s\n", FUNCNAME);
	m_sysram[offset % ARRAY_LENGTH(m_sysram)] &= ~mem_mask;
	m_sysram[offset % ARRAY_LENGTH(m_sysram)] |= (data & mem_mask);
	m_sysrom = &m_sysram[0]; // redirect all upcoming accesses to masking RAM until reset.
}

vme_hcpu30_card_device::vme_hcpu30_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_vme_card_interface(mconfig, *this)
	, m_maincpu(*this, "maincpu")
	, m_dusccterm(*this, "duscc")
{
	LOG("%s %s\n", tag, FUNCNAME);
	m_slot = 1;
}

vme_hcpu30_card_device::vme_hcpu30_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: vme_hcpu30_card_device(mconfig, VME_HCPU30, tag, owner, clock)
{
}

void vme_hcpu30_card_device::device_start()
{
	LOG("%s %s\n", tag(), FUNCNAME);

	set_vme_device();

	save_pointer (NAME (m_sysrom), sizeof(m_sysrom));
	save_pointer (NAME (m_sysram), sizeof(m_sysram));

	/* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
	m_sysrom = (uint32_t*)(memregion ("user1")->base());
}

void vme_hcpu30_card_device::device_reset()
{
	LOG("%s %s\n", tag(), FUNCNAME);
}
