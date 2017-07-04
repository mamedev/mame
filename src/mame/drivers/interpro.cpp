// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "emu.h"

#include "includes/interpro.h"

#include "debugger.h"

#include "interpro.lh"

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
	m_sreg_ctrl2 = CTRL2_COLDSTART | CTRL2_PWRENA | CTRL2_PWRUP;
}

void interpro_state::machine_reset()
{
	// flash rom requires the following values
	m_sreg_error = 0x00;
	m_sreg_status = 0x400;
	m_sreg_ctrl1 = CTRL1_FLOPRDY;
}

WRITE16_MEMBER(interpro_state::sreg_led_w)
{
	// 7-segment decode patterns (hex digits) borrowed from wico.cpp (mc14495)
	static const u8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71 };

	output().set_digit_value(0, patterns[data & 0xf]);
}

WRITE16_MEMBER(interpro_state::sreg_ctrl1_w)
{
	LOG_SYSTEM("system control register 1 write data 0x%x (%s)\n", data, machine().describe_context());

	// check if led decimal point changes state
	if ((data ^ m_sreg_ctrl1) & CTRL1_LEDDP)
		output().set_digit_value(0, (output().get_digit_value(0) + 0x80) & 0xff);

	m_sreg_ctrl1 = data;
}

WRITE16_MEMBER(interpro_state::sreg_ctrl2_w)
{
	LOG_SYSTEM("system control register 2 write data 0x%x (%s)\n", data, machine().describe_context());
	if (data & CTRL2_RESET)
	{
		m_sreg_ctrl1 &= ~CTRL2_COLDSTART;

		machine().schedule_soft_reset();
	}
	else
		m_sreg_ctrl1 = data & CTRL2_WMASK;
}

READ16_MEMBER(interpro_state::sreg_error_r)
{
	u16 result = m_sreg_error;

	// clear error register on read
	m_sreg_error = 0;

	return result;
}

READ8_MEMBER(interpro_state::idprom_r)
{
	LOG_IDPROM("idprom read offset 0x%x mask 0x%08x at %s\n", offset, mem_mask, machine().describe_context());

	// compute femtoseconds per cycle from main cpu clock
	u32 speed = 1'000'000'000'000'000 / m_maincpu->clock();

	static u8 idprom[] = {
		// module type id
		'M', 'S', 'M', 'T',     // board type MSMT/MPCB - detected by feature[3]
		'1', '4', '5',          // board number
		'0',                    // board revision

		// ECO bytes
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,

		// the following 8 bytes are "feature bytes"

		// for a 2700/2800/2500 system board, the first feature byte selects the variant
		//   model = (feature[0] & 0x2) ? (feature[0] & 0x8 ? 2700 : 2800) : 2500

		// 0x0a, // 2700 series
		0x02, // 2800 series
		// 0x00, // 2500 series

		0x00, 0x00,
		0x00, // board type, 0x80 = MPCB, 0x00 = MSMT

		// for the system boards, these bytes contain cpu clock speed (as femtoseconds per cycle, big-endian)
		(u8)(speed >> 24), (u8)(speed >> 16), (u8)(speed >> 8), (u8)(speed >> 0),

		// reserved bytes
		0xff, 0xff,

		// family
		// boot rom tests for family == 0x41 or 0x42
		// if so, speed read from feature bytes 2 & 3
		// if not, read speed from feature bytes 4-7

		//0x24, 0x00, // 2000

		// 0x31, 0x00, // 2400
		0x39, 0x00, // 2700/2800/2500 depending on first feature byte (0xa, 0x2, 0x0)
		// 0x40, 0x00, // 6700
		// 0x41, 0x00, // idprom reports as 2800 series cpu?
		//0x42, 0x00, // 6800 series

		// footprint and checksum
		0x55, 0xaa, 0x55, 0x00
	};

	switch (offset)
	{
	case 0x1f:
	{
		u8 sum = 0;

		// compute the checksum (sum of all bytes must be == 0x00)
		for (int i = 0; i < 0x20; i++)
			sum += idprom[i];

		return 0x100 - sum;
	}

	default:
		return idprom[offset];
	}
}

READ8_MEMBER(interpro_state::slot0_r)
{
#if 0
	// a known graphics board idprom
	static u8 slot0[] = {
		0x00, 0x00, 0x00, 0x00, '9',  '6',  '3',  'A',  // board
		0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // eco
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // features
		0xff, 0xff,                                     // reserved
		0x22, 0x00,                                     // family
		0x55, 0xaa, 0x55,                               // footprint
		0x00                                            // checksum
	};
#else
	static u8 slot0[] = {
		'M', 'S', 'M', 'T',                             // board type
		'0',  '7',  '0',                                // board
		'B',                                            // board revision
		0x02, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // eco
		0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, // features
		0xff, 0xff,                                     // reserved
		0x05, 0x00,                                     // family
		0x55, 0xaa, 0x55,                               // footprint
		0x00                                            // checksum
	};
#endif
	switch (offset)
	{
	case 0x1f:
	{
		u8 sum = 0;

		// compute the checksum (sum of all bytes must be == 0x00)
		for (int i = 0; i < 0x20; i++)
			sum += slot0[i];

		return 0x100 - sum;
	}

	default:
		return slot0[offset];
	}
}

READ32_MEMBER(interpro_state::unmapped_r)
{
	// check if non-existent memory errors are enabled
	if (m_srarb->tmctrl_r(space, offset, mem_mask) & interpro_srarb_device::TMCTRL_ENNEM)
	{
		// flag non-existent memory error in system error register
		m_sreg_error |= (ERROR_SRXNEM | ERROR_SRXVALID);

		// tell ioga to raise a bus error
		m_ioga->bus_error(space, interpro_ioga_device::BINFO_BERR | interpro_ioga_device::BINFO_SNAPOK, offset << 2);
	}

	return space.unmap();
}

WRITE32_MEMBER(interpro_state::unmapped_w)
{
	// check if non-existent memory errors are enabled
	if (m_srarb->tmctrl_r(space, offset, mem_mask) & interpro_srarb_device::TMCTRL_ENNEM)
	{
		// flag non-existent memory error in system error register
		m_sreg_error |= (ERROR_SRXNEM | ERROR_SRXVALID);

		// tell ioga to raise a bus error
		m_ioga->bus_error(space, interpro_ioga_device::BINFO_BERR | interpro_ioga_device::BINFO_SNAPOK, offset << 2);
	}
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
	switch (offset >> 6)
	{
	case 0x0: return m_scsi->tcounter_lo_r(space, 0);
	case 0x1: return m_scsi->tcounter_hi_r(space, 0);
	case 0x2: return m_scsi->fifo_r(space, 0);
	case 0x3: return m_scsi->command_r(space, 0);
	case 0x4: return m_scsi->status_r(space, 0);
	case 0x5: return m_scsi->istatus_r(space, 0);
	case 0x6: return m_scsi->seq_step_r(space, 0);
	case 0x7: return m_scsi->fifo_flags_r(space, 0);
	case 0x8: return m_scsi->conf_r(space, 0);
	case 0xb: return m_scsi->conf2_r(space, 0);
	}

	logerror("scsi: read unmapped register 0x%x (%s)\n", offset >> 6, machine().describe_context());
	return space.unmap();
}

WRITE8_MEMBER(interpro_state::scsi_w)
{
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
	}

	logerror("scsi: unmapped register write 0x%02x data 0x%02x (%s)\n", offset >> 6, data, machine().describe_context());
}

DRIVER_INIT_MEMBER(interpro_state, ip2800)
{
	// FIXME: not all memory sizes are reported properly using fdm "5 inqhw" and "optimum_memory" commands

	// 16 = reports 16M, banks empty?
	// 32 = reports 16M, banks empty?
	// 64 = reports 128M, 16x8
	// 128 = reports 128M, 16x8
	// 256 = reports 256M, 32x8

	// grab the main memory space from the mmu
	address_space &space = m_mmu->space(0);

	// map the configured ram
	space.install_ram(0, m_ram->mask(), m_ram->pointer());
}

static SLOT_INTERFACE_START(interpro_scsi_devices)
	SLOT_INTERFACE("harddisk", NSCSI_HARDDISK)
	SLOT_INTERFACE("cdrom", NSCSI_CDROM)
	SLOT_INTERFACE_INTERNAL(INTERPRO_SCSI_ADAPTER_TAG, NCR53C90A)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START(interpro_scsi_adapter)
	MCFG_DEVICE_CLOCK(XTAL_24MHz)
	MCFG_NCR5390_IRQ_HANDLER(DEVWRITELINE(":" INTERPRO_IOGA_TAG, interpro_ioga_device, ir0_w))
	MCFG_NCR5390_DRQ_HANDLER(DEVWRITELINE(":" INTERPRO_IOGA_TAG, interpro_ioga_device, drq_scsi))
MACHINE_CONFIG_END

// these maps point the cpu virtual addresses to the mmu
static ADDRESS_MAP_START(clipper_insn_map, AS_PROGRAM, 32, interpro_state)
	AM_RANGE(0x00000000, 0xffffffff) AM_DEVREAD32(INTERPRO_MMU_TAG, cammu_device, insn_r, 0xffffffff)
ADDRESS_MAP_END

static ADDRESS_MAP_START(clipper_data_map, AS_DATA, 32, interpro_state)
	AM_RANGE(0x00000000, 0xffffffff) AM_DEVREADWRITE32(INTERPRO_MMU_TAG, cammu_device, data_r, data_w, 0xffffffff)
ADDRESS_MAP_END

static ADDRESS_MAP_START(interpro_common_map, 0, 32, interpro_state)
	AM_RANGE(0x40000000, 0x4000004f) AM_DEVICE(INTERPRO_MCGA_TAG, interpro_fmcc_device, map)
	AM_RANGE(0x4f007e00, 0x4f007eff) AM_DEVICE(INTERPRO_SGA_TAG, interpro_sga_device, map)

	AM_RANGE(0x7f000100, 0x7f00011f) AM_DEVICE8(INTERPRO_FDC_TAG, n82077aa_device, map, 0xff)
	AM_RANGE(0x7f000200, 0x7f0002ff) AM_DEVICE(INTERPRO_SRARB_TAG, interpro_srarb_device, map)

	AM_RANGE(0x7f000300, 0x7f000303) AM_READ16(sreg_error_r, 0xffff)
	AM_RANGE(0x7f000304, 0x7f000307) AM_READWRITE16(sreg_status_r, sreg_led_w, 0xffff)
	AM_RANGE(0x7f000308, 0x7f00030b) AM_READWRITE16(sreg_ctrl1_r, sreg_ctrl1_w, 0xffff)
	AM_RANGE(0x7f00030c, 0x7f00030f) AM_READWRITE16(sreg_ctrl2_r, sreg_ctrl2_w, 0xffff)

	AM_RANGE(0x7f00031c, 0x7f00031f) AM_READWRITE16(sreg_ctrl3_r, sreg_ctrl3_w, 0xffff)

	AM_RANGE(0x7f000400, 0x7f00040f) AM_DEVREADWRITE8(INTERPRO_SCC1_TAG, scc85c30_device, ba_cd_inv_r, ba_cd_inv_w, 0xff)
	AM_RANGE(0x7f000410, 0x7f00041f) AM_DEVREADWRITE8(INTERPRO_SCC2_TAG, scc85230_device, ba_cd_inv_r, ba_cd_inv_w, 0xff)
	AM_RANGE(0x7f000500, 0x7f0006ff) AM_READWRITE8(rtc_r, rtc_w, 0xff)
	AM_RANGE(0x7f000700, 0x7f00077f) AM_READ8(idprom_r, 0xff)
	AM_RANGE(0x7f001000, 0x7f001fff) AM_READWRITE8(scsi_r, scsi_w, 0x0000ff00)

	AM_RANGE(0x7f0fff00, 0x7f0fffff) AM_DEVICE(INTERPRO_IOGA_TAG, interpro_ioga_device, map)

	AM_RANGE(0x08000000, 0x08000fff) AM_NOP // bogus
	AM_RANGE(0x87000000, 0x8700007f) AM_READ8(slot0_r, 0xff)

	// 2800 (CBUS?) slots are mapped as follows
	AM_RANGE(0x87000000, 0x8700007f) AM_MIRROR(0x78000000) AM_READWRITE(unmapped_r, unmapped_w)
ADDRESS_MAP_END

// these maps represent the real main, i/o and boot spaces of the system
static ADDRESS_MAP_START(interpro_main_map, 0, 32, interpro_state)
	AM_RANGE(0x00000000, 0x00ffffff) AM_RAM AM_SHARE(RAM_TAG)

	AM_RANGE(0x7f100000, 0x7f11ffff) AM_ROM AM_REGION(INTERPRO_EPROM_TAG, 0)
	AM_RANGE(0x7f180000, 0x7f1bffff) AM_ROM AM_REGION(INTERPRO_FLASH_TAG, 0)

	AM_IMPORT_FROM(interpro_common_map)
ADDRESS_MAP_END

static ADDRESS_MAP_START(interpro_io_map, 1, 32, interpro_state)
	AM_RANGE(0x00000000, 0x00001fff) AM_DEVICE(INTERPRO_MMU_TAG, cammu_device, map)

	AM_IMPORT_FROM(interpro_common_map)
ADDRESS_MAP_END

static ADDRESS_MAP_START(interpro_boot_map, 2, 32, interpro_state)
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
	MCFG_CPU_ADD(INTERPRO_CPU_TAG, CLIPPER_C400, XTAL_12_5MHz)
	MCFG_CPU_PROGRAM_MAP(clipper_insn_map)
	MCFG_CPU_DATA_MAP(clipper_data_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE(INTERPRO_IOGA_TAG, interpro_ioga_device, inta_cb)

	MCFG_DEVICE_ADD(INTERPRO_MMU_TAG, CAMMU_C4T, 0)
	MCFG_DEVICE_ADDRESS_MAP(0, interpro_main_map)
	MCFG_DEVICE_ADDRESS_MAP(1, interpro_io_map)
	MCFG_DEVICE_ADDRESS_MAP(2, interpro_boot_map)
	MCFG_CAMMU_SSW_CB(DEVREAD32(INTERPRO_CPU_TAG, clipper_device, ssw))

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16M")
	MCFG_RAM_EXTRA_OPTIONS("32M,64M,128M,256M")

	// TODO: work out serial port assignments for mouse, console, keyboard and ?
	// first serial controller and devices
	MCFG_SCC85C30_ADD(INTERPRO_SCC1_TAG, XTAL_4_9152MHz, 0, 0, 0, 0)

	MCFG_Z80SCC_OUT_TXDA_CB(DEVWRITELINE(INTERPRO_SERIAL1_TAG, rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_TXDB_CB(DEVWRITELINE(INTERPRO_SERIAL2_TAG, rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_INT_CB(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, ir11_w))

	// is this the keyboard port?
	MCFG_RS232_PORT_ADD(INTERPRO_SERIAL1_TAG, default_rs232_devices, nullptr) // "keyboard"
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, ctsa_w))

	// eprom uses this serial port for console (show_config calls it "port 2")
	MCFG_RS232_PORT_ADD(INTERPRO_SERIAL2_TAG, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, ctsb_w))

	// second serial controller and devices
	MCFG_SCC85230_ADD(INTERPRO_SCC2_TAG, XTAL_4_9152MHz, 0, 0, 0, 0)

	MCFG_Z80SCC_OUT_TXDA_CB(DEVWRITELINE(INTERPRO_SERIAL3_TAG, rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_TXDB_CB(DEVWRITELINE(INTERPRO_SERIAL4_TAG, rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_INT_CB(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, ir11_w))

	MCFG_RS232_PORT_ADD(INTERPRO_SERIAL3_TAG, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, ctsa_w))

	MCFG_RS232_PORT_ADD(INTERPRO_SERIAL4_TAG, default_rs232_devices, nullptr) //"terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, ctsb_w))

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
	MCFG_NSCSI_BUS_ADD(INTERPRO_SCSI_TAG)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":0", interpro_scsi_devices, "harddisk", false)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":1", interpro_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":2", interpro_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":3", interpro_scsi_devices, "cdrom", false)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":4", interpro_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":5", interpro_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":6", interpro_scsi_devices, nullptr, false)
	MCFG_NSCSI_ADD(INTERPRO_SCSI_TAG ":7", interpro_scsi_devices, INTERPRO_SCSI_ADAPTER_TAG, true)
	MCFG_DEVICE_CARD_MACHINE_CONFIG(INTERPRO_SCSI_ADAPTER_TAG, interpro_scsi_adapter)

	// i/o gate array
	MCFG_INTERPRO_IOGA_ADD(INTERPRO_IOGA_TAG)
	MCFG_INTERPRO_IOGA_NMI_CB(INPUTLINE(INTERPRO_CPU_TAG, INPUT_LINE_NMI))
	MCFG_INTERPRO_IOGA_IRQ_CB(INPUTLINE(INTERPRO_CPU_TAG, INPUT_LINE_IRQ0))
	//MCFG_INTERPRO_IOGA_DMA_CB(IOGA_DMA_CHANNEL_PLOTTER, unknown)

	MCFG_INTERPRO_IOGA_DMA_CB(IOGA_DMA_SCSI, DEVREAD8(INTERPRO_SCSI_DEVICE_TAG, ncr53c90a_device, mdma_r), DEVWRITE8(INTERPRO_SCSI_DEVICE_TAG, ncr53c90a_device, mdma_w))
	MCFG_INTERPRO_IOGA_DMA_CB(IOGA_DMA_FLOPPY, DEVREAD8(INTERPRO_FDC_TAG, n82077aa_device, mdma_r), DEVWRITE8(INTERPRO_FDC_TAG, n82077aa_device, mdma_w))
	MCFG_INTERPRO_IOGA_DMA_CB(IOGA_DMA_SERIAL, DEVREAD8(INTERPRO_SCC1_TAG, z80scc_device, da_r), DEVWRITE8(INTERPRO_SCC1_TAG, z80scc_device, da_w))
	MCFG_INTERPRO_IOGA_FDCTC_CB(DEVWRITELINE(INTERPRO_FDC_TAG, n82077aa_device, tc_line_w))
	MCFG_INTERPRO_IOGA_DMA_BUS(INTERPRO_CAMMU_TAG, 0)

	// memory control gate array
	MCFG_DEVICE_ADD(INTERPRO_MCGA_TAG, INTERPRO_FMCC, 0)

	// srx gate array
	MCFG_DEVICE_ADD(INTERPRO_SGA_TAG, INTERPRO_SGA, 0)
	MCFG_INTERPRO_SGA_BERR_CB(DEVWRITE32(INTERPRO_IOGA_TAG, interpro_ioga_device, bus_error))

	// srx arbiter gate array
	MCFG_DEVICE_ADD(INTERPRO_SRARB_TAG, INTERPRO_SRARB, 0)

	MCFG_DEFAULT_LAYOUT(layout_interpro)

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
