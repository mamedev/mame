// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "emu.h"

#define NEW_SCSI 0

#include "includes/interpro.h"
#include "debugger.h"

#define VERBOSE 0
#if VERBOSE
#define LOG_SYSTEM(...) logerror(__VA_ARGS__)
#define LOG_IDPROM(...) logerror(__VA_ARGS__)
#else
#define LOG_SYSTEM(...) {}
#define LOG_IDPROM(...) {}
#endif

// machine start
void interpro_state::machine_start()
{
	m_system_reg[SREG_CTRL2] = CTRL2_COLDSTART | CTRL2_PWRENA | CTRL2_PWRUP;
}

void interpro_state::machine_reset()
{
	// flash rom requires the following values
	m_system_reg[SREG_ERROR] = 0x00;
	m_system_reg[SREG_STATUS] = 0x400;
	m_system_reg[SREG_CTRL1] = CTRL1_FLOPRDY;
}

WRITE16_MEMBER(interpro_state::system_w)
{
	switch (offset)
	{
	case SREG_LED:
		LOG_SYSTEM("LED value %d at %s\n", data, machine().describe_context());
		break;

	case SREG_STATUS: // not sure if writable?
		break;

	case SREG_CTRL1:
		LOG_SYSTEM("system control register 1 write data 0x%x pc %s\n", data, machine().describe_context());

		if ((data ^ m_system_reg[offset]) & CTRL1_LEDDP)
			LOG_SYSTEM("LED decimal point %s\n", data & CTRL1_LEDDP ? "on" : "off");

		m_system_reg[offset] = data;
		break;

	case SREG_CTRL2:
		LOG_SYSTEM("system control register 2 write data 0x%x at %s\n", data, machine().describe_context());
		if (data & CTRL2_RESET)
		{
			m_system_reg[SREG_CTRL2] &= ~CTRL2_COLDSTART;

			machine().schedule_soft_reset();
		}
		else
			m_system_reg[offset] = data & 0x0f; // top four bits are not persistent
		break;
	}
}

READ16_MEMBER(interpro_state::system_r)
{
	LOG_SYSTEM("system register read offset %d at %s\n", offset, machine().describe_context());
	switch (offset)
	{
	case SREG_ERROR:
	case SREG_STATUS:
	case SREG_CTRL1:
	case SREG_CTRL2:
	default:
		return m_system_reg[offset];
		break;
	}
}

READ32_MEMBER(interpro_state::idprom_r)
{
	LOG_IDPROM("idprom read offset 0x%x mask 0x%08x at %s\n", offset, mem_mask, machine().describe_context());

	// abitrary fake number for now, not working properly
	u32 speed = 20000000;
	u32 speed1 = speed >> 24;
	u32 speed2 = speed >> 16;
	u32 speed3 = speed >> 8;

	static uint8_t idprom[] = {
		// module type id
		'M', 'P', 'C', 'B',
		'0', '1', '4', '5',

		// ECO bytes
		0x87, 0x65, 0x43, 0x21,
		0xbb, 0xcc, 0xdd, 0xee,

		// the following 8 bytes are "feature bytes"
		// the feature bytes contain a 32 bit word which is divided by 40000
		// if they're empty, a default value of 50 000 000 is used
		// perhaps this is a system speed (50MHz)?
		0x2, 0x34, 0x56, 0x78,
		(u8)speed, (u8)speed3, (u8)speed2, (u8)speed1,

		// reserved bytes
		0xff, 0xff,

		// family
		// boot rom tests for family == 0x41 or 0x42
		// if so, speed read from feature bytes 2 & 3
		// if not, read speed from feature bytes 4-7
		0x41, 0x00, // 2800-series CPU
		//0x24, 0x00, // 2000-series system board

		// footprint and checksum
		0x55, 0xaa, 0x55, 0x00
	};

	switch (offset)
	{
	case 0x1f:
	{
		uint8_t sum = 0;

		// compute the checksum (sum of all bytes must be == 0x00)
		for (int i = 0; i < 0x20; i++)
			sum += idprom[i];

		return 0x100 - (sum & 0xff);
	}

	default:
		return idprom[offset];
	}
}

READ32_MEMBER(interpro_state::slot0_r)
{
	// a known graphics board idprom
	static uint8_t slot0[] = {
		0x00, 0x00, 0x00, 0x00, '9',  '6',  '3',  'A',  // board
		0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // eco
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // features
		0xff, 0xff,                                     // reserved
		0x22, 0x00,                                     // family
		0x55, 0xaa, 0x55, 0x00
	};

	return ((uint8_t *)&slot0)[offset % 32];
}

WRITE8_MEMBER(interpro_state::rtc_w)
{
	switch (offset)
	{
	case 0x00:
		// write to RTC register
		m_rtc->write(space, 1, data);
		break;

	case 0x40:
		// set RTC read/write address
		m_rtc->write(space, 0, data);
		break;

	default:
		logerror("rtc: write to unknown offset 0x%02x data 0x%02x at %s\n", offset, data, machine().describe_context());
		break;
	}
}

READ8_MEMBER(interpro_state::rtc_r)
{
	switch (offset)
	{
	case 0x00:
		// read from RTC register
		return m_rtc->read(space, 1);

		// read from InterPro system ID PROM (contains MAC address)
	case 0x40: return 0x12;
	case 0x41: return 0x34;
	case 0x42: return 0x56;

	default:
		logerror("rtc: read from unknown offset 0x%02x at %s\n", offset, machine().describe_context());
		return 0xff;
	}
}

// these wrappers handle the two alternative scsi drivers, as well as the weird memory mapping on the InterPro
// that maps consecutive registers at offsets of 0x100 (and with lsb = 1, but we're ignoring that for now

READ8_MEMBER(interpro_state::scsi_r)
{
#if NEW_SCSI
	switch (offset >> 6)
	{
	case 0x0: return m_scsi->tcount_lo_r(space, 0);
	case 0x1: return m_scsi->tcount_hi_r(space, 0);
	case 0x2: return m_scsi->fifo_r(space, 0);
	case 0x3: return m_scsi->command_r(space, 0);
	case 0x4: return m_scsi->status_r(space, 0);
	case 0x5: return m_scsi->istatus_r(space, 0);
	case 0x6: return m_scsi->seq_step_r(space, 0);
	case 0x7: return m_scsi->fifo_flags_r(space, 0);
	case 0x8: return m_scsi->conf_r(space, 0);
	case 0xb: return m_scsi->conf2_r(space, 0);
	case 0xc: return m_scsi->conf3_r(space, 0);
	}

	logerror("read unmapped scsi adapter register 0x%x\n", offset >> 6);
	return 0x00;
#else
	return m_scsi->read(space, offset >> 6, mem_mask);
#endif
}

WRITE8_MEMBER(interpro_state::scsi_w)
{
#if NEW_SCSI
	switch (offset >> 6)
	{
	case 0: m_scsi->tcount_lo_w(space, 0, data); return;
	case 1: m_scsi->tcount_hi_w(space, 0, data); return;
	case 2: m_scsi->fifo_w(space, 0, data); return;
	case 3: m_scsi->command_w(space, 0, data); return;
	case 4: m_scsi->bus_id_w(space, 0, data); return;
	case 5: m_scsi->timeout_w(space, 0, data); return;
	case 6: m_scsi->sync_period_w(space, 0, data); return;
	case 7: m_scsi->sync_offset_w(space, 0, data); return;
	case 8: m_scsi->conf_w(space, 0, data); return;
	case 9: m_scsi->clock_w(space, 0, data); return;
	case 0xa: m_scsi->test_w(space, 0, data); return;
	case 0xb: m_scsi->conf2_w(space, 0, data); return;
	case 0xc: m_scsi->conf3_w(space, 0, data); return;
	case 0xf: m_scsi->fifo_align_w(space, 0, data); return;
	}

	logerror("unmapped scsi register write 0x%02x data 0x%02x\n", offset >> 6, data);
#else
	m_scsi->write(space, offset >> 6, data, mem_mask);
#endif
}

READ8_MEMBER(interpro_state::scsi_dma_r)
{
#if NEW_SCSI
	return m_scsi->dma_r();
#else
	u8 data = 0;

	m_scsi->dma_read_data(1, &data);

	return data;
#endif
}

WRITE8_MEMBER(interpro_state::scsi_dma_w)
{
#if NEW_SCSI
	m_scsi->dma_w(data);
#else
	m_scsi->dma_write_data(1, &data);
#endif
}

DRIVER_INIT_MEMBER(interpro_state, ip2800)
{
}

#if NEW_SCSI
static SLOT_INTERFACE_START(interpro_scsi_devices)
	SLOT_INTERFACE("harddisk", NSCSI_HARDDISK)
	SLOT_INTERFACE("cdrom", NSCSI_CDROM)
	SLOT_INTERFACE_INTERNAL(INTERPRO_SCSI_ADAPTER_TAG, NCR53C94)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT(interpro_scsi_adapter)
	MCFG_DEVICE_CLOCK(XTAL_12_5MHz)
	MCFG_NCR5390_IRQ_HANDLER(DEVWRITELINE(":" INTERPRO_IOGA_TAG, interpro_ioga_device, ir0_w))
	MCFG_NCR5390_DRQ_HANDLER(DEVWRITELINE(":" INTERPRO_IOGA_TAG, interpro_ioga_device, drq_scsi))
MACHINE_CONFIG_END
#endif

// these maps point the cpu virtual addresses to the mmu
static ADDRESS_MAP_START(clipper_insn_map, AS_PROGRAM, 32, interpro_state)
	AM_RANGE(0x00000000, 0xffffffff) AM_DEVREAD32(INTERPRO_MMU_TAG, cammu_device, insn_r, 0xffffffff)
ADDRESS_MAP_END

static ADDRESS_MAP_START(clipper_data_map, AS_DATA, 32, interpro_state)
	AM_RANGE(0x00000000, 0xffffffff) AM_DEVREADWRITE32(INTERPRO_MMU_TAG, cammu_device, data_r, data_w, 0xffffffff)
ADDRESS_MAP_END

// these maps represent the real main, i/o and boot spaces of the system
static ADDRESS_MAP_START(interpro_main_map, AS_0, 32, interpro_state)
	AM_RANGE(0x00000000, 0x00ffffff) AM_RAM // 16M RAM
	
	AM_RANGE(0x40000000, 0x4000003f) AM_DEVICE(INTERPRO_MCGA_TAG, interpro_fmcc_device, map)
	AM_RANGE(0x4f007e00, 0x4f007eff) AM_DEVICE(INTERPRO_SGA_TAG, interpro_sga_device, map)

	AM_RANGE(0x7f000100, 0x7f00011f) AM_DEVICE8(INTERPRO_FDC_TAG, n82077aa_device, map, 0xff)
	AM_RANGE(0x7f000200, 0x7f0002ff) AM_DEVICE(INTERPRO_SRARB_TAG, interpro_srarb_device, map)
	AM_RANGE(0x7f000300, 0x7f00030f) AM_READWRITE16(system_r, system_w, 0xffff)
	AM_RANGE(0x7f000400, 0x7f00040f) AM_DEVREADWRITE8(INTERPRO_SCC1_TAG, scc85c30_device, ba_cd_inv_r, ba_cd_inv_w, 0xff)
	AM_RANGE(0x7f000410, 0x7f00041f) AM_DEVREADWRITE8(INTERPRO_SCC2_TAG, scc85230_device, ba_cd_inv_r, ba_cd_inv_w, 0xff)
	AM_RANGE(0x7f000500, 0x7f0006ff) AM_READWRITE8(rtc_r, rtc_w, 0xff)
	AM_RANGE(0x7f000700, 0x7f00077f) AM_READ(idprom_r)
	AM_RANGE(0x7f001000, 0x7f001fff) AM_READWRITE8(scsi_r, scsi_w, 0x0000ff00)

	AM_RANGE(0x7f0fff00, 0x7f0fffff) AM_DEVICE(INTERPRO_IOGA_TAG, interpro_ioga_device, map)

	AM_RANGE(0x7f100000, 0x7f11ffff) AM_ROM AM_REGION(INTERPRO_EPROM_TAG, 0)
	AM_RANGE(0x7f180000, 0x7f1bffff) AM_ROM AM_REGION(INTERPRO_FLASH_TAG, 0)

	AM_RANGE(0x08000000, 0x08000fff) AM_NOP // bogus
	AM_RANGE(0x8f000000, 0x8f0fffff) AM_NOP // AM_READ(slot0_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START(interpro_io_map, AS_1, 32, interpro_state)
	AM_RANGE(0x00000000, 0x00001fff) AM_DEVICE(INTERPRO_MMU_TAG, cammu_device, map)

	AM_RANGE(0x40000000, 0x4000004f) AM_DEVICE(INTERPRO_MCGA_TAG, interpro_fmcc_device, map)
	AM_RANGE(0x4f007e00, 0x4f007eff) AM_DEVICE(INTERPRO_SGA_TAG, interpro_sga_device, map)

	AM_RANGE(0x7f000100, 0x7f00011f) AM_DEVICE8(INTERPRO_FDC_TAG, n82077aa_device, map, 0xff)
	AM_RANGE(0x7f000200, 0x7f0002ff) AM_DEVICE(INTERPRO_SRARB_TAG, interpro_srarb_device, map)
	AM_RANGE(0x7f000300, 0x7f00030f) AM_READWRITE16(system_r, system_w, 0xffff)
	AM_RANGE(0x7f000400, 0x7f00040f) AM_DEVREADWRITE8(INTERPRO_SCC1_TAG, scc85c30_device, ba_cd_inv_r, ba_cd_inv_w, 0xff)
	AM_RANGE(0x7f000410, 0x7f00041f) AM_DEVREADWRITE8(INTERPRO_SCC2_TAG, scc85230_device, ba_cd_inv_r, ba_cd_inv_w, 0xff)
	AM_RANGE(0x7f000500, 0x7f0006ff) AM_READWRITE8(rtc_r, rtc_w, 0xff)
	AM_RANGE(0x7f000700, 0x7f00077f) AM_READ(idprom_r)
	AM_RANGE(0x7f001000, 0x7f001fff) AM_READWRITE8(scsi_r, scsi_w, 0x0000ff00)

	AM_RANGE(0x7f0fff00, 0x7f0fffff) AM_DEVICE(INTERPRO_IOGA_TAG, interpro_ioga_device, map)

	AM_RANGE(0x08000000, 0x08000fff) AM_NOP // bogus
	AM_RANGE(0x8f000000, 0x8f0fffff) AM_NOP // AM_READ(slot0_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START(interpro_boot_map, AS_2, 32, interpro_state)
	AM_RANGE(0x00000000, 0x00001fff) AM_RAM
ADDRESS_MAP_END

FLOPPY_FORMATS_MEMBER(interpro_state::floppy_formats)
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START(interpro_floppies)
	SLOT_INTERFACE("525dd", FLOPPY_525_DD)
	SLOT_INTERFACE("35hd", FLOPPY_35_HD)
SLOT_INTERFACE_END

// input ports
static INPUT_PORTS_START(ip2800)
INPUT_PORTS_END

static MACHINE_CONFIG_START(ip2800)
	MCFG_CPU_ADD(INTERPRO_CPU_TAG, CLIPPER_C400, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP(clipper_insn_map)
	MCFG_CPU_DATA_MAP(clipper_data_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE(INTERPRO_IOGA_TAG, interpro_ioga_device, inta_cb)

	MCFG_DEVICE_ADD(INTERPRO_MMU_TAG, CAMMU_C4T, 0)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, interpro_main_map)
	MCFG_DEVICE_ADDRESS_MAP(AS_1, interpro_io_map)
	MCFG_DEVICE_ADDRESS_MAP(AS_2, interpro_boot_map)
	MCFG_CAMMU_SSW_CB(DEVREAD32(INTERPRO_CPU_TAG, clipper_device, ssw))

	// serial controllers and rs232 bus
	MCFG_SCC85C30_ADD(INTERPRO_SCC1_TAG, XTAL_4_9152MHz, 0, 0, 0, 0)

	MCFG_Z80SCC_OUT_TXDA_CB(DEVWRITELINE("rs232a", rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_TXDB_CB(DEVWRITELINE("rs232b", rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_INT_CB(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, ir11_w))

	MCFG_RS232_PORT_ADD("rs232a", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, ctsa_w))

	// the following port is known as "port 2"
	MCFG_RS232_PORT_ADD("rs232b", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, ctsb_w))

	MCFG_SCC85230_ADD(INTERPRO_SCC2_TAG, XTAL_4_9152MHz, 0, 0, 0, 0)

	// real-time clock/non-volatile memory
	MCFG_MC146818_ADD(INTERPRO_RTC_TAG, XTAL_32_768kHz)
	MCFG_MC146818_UTC(true)
	MCFG_MC146818_IRQ_HANDLER(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, ir9_w))

	// floppy
	MCFG_N82077AA_ADD(INTERPRO_FDC_TAG, n82077aa_device::MODE_PS2)
	MCFG_UPD765_INTRQ_CALLBACK(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, ir1_w))
	MCFG_UPD765_DRQ_CALLBACK(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, drq_floppy))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", interpro_floppies, "525dd", interpro_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", interpro_floppies, "35hd", interpro_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(false)

	// scsi
#if NEW_SCSI
	MCFG_NSCSI_BUS_ADD(INTERPRO_SCSI_TAG)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":0", interpro_scsi_devices, "harddisk", false)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":1", interpro_scsi_devices, "cdrom", false)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":2", interpro_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":3", interpro_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":4", interpro_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":5", interpro_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":6", interpro_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":7", interpro_scsi_devices, INTERPRO_SCSI_ADAPTER_TAG, true)
	MCFG_DEVICE_CARD_MACHINE_CONFIG(INTERPRO_SCSI_ADAPTER_TAG, interpro_scsi_adapter)
#else
	MCFG_DEVICE_ADD(INTERPRO_SCSI_TAG, SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD(INTERPRO_SCSI_TAG ":" SCSI_PORT_DEVICE1, "harddisk", SCSIHD, SCSI_ID_0)
	MCFG_SCSIDEV_ADD(INTERPRO_SCSI_TAG ":" SCSI_PORT_DEVICE2, "cdrom", SCSICD, SCSI_ID_3)

	MCFG_DEVICE_ADD(INTERPRO_SCSI_ADAPTER_TAG, NCR539X, XTAL_12_5MHz)
	MCFG_LEGACY_SCSI_PORT(INTERPRO_SCSI_TAG)
	MCFG_NCR539X_OUT_IRQ_CB(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, ir0_w))
	MCFG_NCR539X_OUT_DRQ_CB(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, drq_scsi))
#endif

	// i/o gate array
	MCFG_INTERPRO_IOGA_ADD(INTERPRO_IOGA_TAG)
	MCFG_INTERPRO_IOGA_NMI_CB(INPUTLINE(INTERPRO_CPU_TAG, INPUT_LINE_NMI))
	MCFG_INTERPRO_IOGA_IRQ_CB(INPUTLINE(INTERPRO_CPU_TAG, INPUT_LINE_IRQ0))
	//MCFG_INTERPRO_IOGA_DMA_CB(IOGA_DMA_CHANNEL_PLOTTER, unknown)

	// use driver helper functions to wrap scsi adapter dma read/write
	MCFG_INTERPRO_IOGA_DMA_CB(IOGA_DMA_SCSI, DEVREAD8("", interpro_state, scsi_dma_r), DEVWRITE8("", interpro_state, scsi_dma_w))

	MCFG_INTERPRO_IOGA_DMA_CB(IOGA_DMA_FLOPPY, DEVREAD8(INTERPRO_FDC_TAG, n82077aa_device, mdma_r), DEVWRITE8(INTERPRO_FDC_TAG, n82077aa_device, mdma_w))
	MCFG_INTERPRO_IOGA_DMA_CB(IOGA_DMA_SERIAL, DEVREAD8(INTERPRO_SCC1_TAG, z80scc_device, da_r), DEVWRITE8(INTERPRO_SCC1_TAG, z80scc_device, da_w))
	MCFG_INTERPRO_IOGA_FDCTC_CB(DEVWRITELINE(INTERPRO_FDC_TAG, n82077aa_device, tc_line_w))
	MCFG_INTERPRO_IOGA_DMA_BUS(INTERPRO_CAMMU_TAG, AS_0)

	// memory control gate array
	MCFG_DEVICE_ADD(INTERPRO_MCGA_TAG, INTERPRO_FMCC, 0)

	// srx gate array
	MCFG_DEVICE_ADD(INTERPRO_SGA_TAG, INTERPRO_SGA, 0)
	MCFG_INTERPRO_SGA_BERR_CB(DEVWRITE32(INTERPRO_IOGA_TAG, interpro_ioga_device, bus_error))

	// srx arbiter gate array
	MCFG_DEVICE_ADD(INTERPRO_SRARB_TAG, INTERPRO_SRARB, 0)

MACHINE_CONFIG_END

ROM_START(ip2800)
	ROM_REGION(0x0020000, INTERPRO_EPROM_TAG, 0)
	ROM_SYSTEM_BIOS(0, "IP2830", "IP2830")
	ROMX_LOAD("ip2830_eprom.bin", 0x00000, 0x20000, CRC(467ce7bd) SHA1(53faee40d5df311f53b24c930e434cbf94a5c4aa), ROM_BIOS(1))

	ROM_REGION(0x0040000, INTERPRO_FLASH_TAG, 0)
	ROM_LOAD_OPTIONAL("ip2830_flash.bin", 0x00000, 0x40000, CRC(a0c0899f) SHA1(dda6fbca81f9885a1a76ca3c25e80463a83a0ef7))
ROM_END

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT   CLASS           INIT     COMPANY         FULLNAME         FLAGS */
COMP( 1993, ip2800,     0,      0,      ip2800,     ip2800, interpro_state, ip2800, "Intergraph",   "InterPro 2800", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
