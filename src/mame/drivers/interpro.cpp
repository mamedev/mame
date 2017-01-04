// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#include "includes/interpro.h"
#include "debugger.h"

#define VERBOSE 0
#if VERBOSE
#define LOG_EMERALD(...) logerror(__VA_ARGS__)
#define LOG_MCGA(...) logerror(__VA_ARGS__)
#define LOG_IDPROM(...) logerror(__VA_ARGS__)
#else
#define LOG_EMERALD(...) {}
#define LOG_MCGA(...) {}
#define LOG_IDPROM(...) {}
#endif


/*
* MCGA Control Register definitions.
*/
#define MCGA_CTRL_OPTMASK	0x00000003
#define MCGA_CTRL_CBITFRCRD	0x00000004
#define MCGA_CTRL_CBITFRCSUB	0x00000008
#define MCGA_CTRL_ENREFRESH	0x00000010
#define MCGA_CTRL_ENMSBE	0x00000100
#define MCGA_CTRL_ENMMBE	0x00000200 // multi-master bus enable?
#define MCGA_CTRL_ENECC		0x00000400
#define MCGA_CTRL_WRPROT	0x00008000
/*
* MCGA Error Register definitions.
*/
#define MCGA_ERROR_SYNDMASK	0x000000ff
#define MCGA_ERROR_SYNDSHIFT	0
#define MCGA_ERROR_SYND(X)	(((X) & MCGA_ERROR_SYNDMASK) >> \
					MCGA_ERROR_SYNDSHIFT)
#define MCGA_ERROR_MMBE		0x00000100
#define MCGA_ERROR_MSBE		0x00000200
#define MCGA_ERROR_ADDRMASK	0x00001C00
#define MCGA_ERROR_ADDRSHIFT	7
#define MCGA_ERROR_ADDR(X)	(((X) & MCGA_ERROR_ADDRMASK) >> \
					MCGA_ERROR_ADDRSHIFT)
#define MCGA_ERROR_VALID	0x00008000

/*
* MCGA Control Register definitions.
*/
#define MCGA_MEMSIZE_ADDRMASK	0x0000007F
#define MCGA_MEMSIZE_ADDRSHIFT	24
#define MCGA_MEMSIZE_ADDR(X)	(((X) & MCGA_MEMSIZE_ADDRMASK) << \
					MCGA_MEMSIZE_ADDRSHIFT)

#define E_SREG_LED 0
#define E_SREG_ERROR 0
#define E_SREG_STATUS 1
#define E_SREG_CTRL1 2
#define E_SREG_CTRL2 3

/*
* Error Register Bit Definitions
* WARNING: Some definitions apply only to certain hardware
*  (ie: E_SERR_SRX* is only valid on 6600 class machines)
*/
#define E_SERR_BPID4		0x0001
#define E_SERR_SRXMMBE		0x0002
#define E_SERR_SRXHOG		0x0004
#define E_SERR_SRXNEM		0x0008
#define E_SERR_SRXVALID		0x0010
#define E_SERR_CBUSNMI		0x0020
#define E_SERR_CBGMASK		0x00c0
#define E_SERR_CBGSHIFT		6
#define E_SERR_BG_MASK		0x0070
#define E_SERR_BG_SHIFT		4
#define E_SERR_BUSHOG		0x0080
#define E_SERR_BG(X)		(((X) & E_SERR_BG_MASK) >> E_SERR_BG_SHIFT)
#define CBUS_ID(X)		(((X) & E_SERR_CBGMASK) >> E_SERR_CBGSHIFT)

/*
* Status Register Bit Definitions
*/
#define E_STAT_YELLOW_ZONE	0x0001
#define E_STAT_SRNMI		0x0002
#define E_STAT_PWRLOSS		0x0004
#define E_STAT_RED_ZONE		0x0008
#define E_STAT_BP_MASK		0x00f0
#define E_STAT_BP_SHIFT		4
#define E_STAT_BP(X)		(((X) & E_STAT_BP_MASK) >> E_STAT_BP_SHIFT)

/*
* Control/Status Register 1 Bit Definitions
*/
#define E_CTRL1_FLOPLOW		0x0001
#define E_CTRL1_FLOPRDY		0x0002
#define E_CTRL1_LEDENA		0x0004
#define E_CTRL1_LEDDP		0x0008
#define E_CTRL1_ETHLOOP		0x0010
#define E_CTRL1_ETHDTR		0x0020
#define E_CTRL1_ETHRMOD		0x0040
#define E_CTRL1_CLIPRESET	0x0040
#define E_CTRL1_FIFOACTIVE	0x0080

/*
* Control/Status Register 2 Bit Definitions
*/
#define E_CTRL2_PWRUP		0x0001
#define E_CTRL2_PWRENA		0x0002
#define E_CTRL2_HOLDOFF		0x0004
#define E_CTRL2_EXTNMIENA	0x0008
#define E_CTRL2_COLDSTART	0x0010
#define E_CTRL2_RESET		0x0020
#define E_CTRL2_BUSENA		0x0040
#define E_CTRL2_FRCPARITY	0x0080
#define E_CTRL2_FLASHEN		0x0080
#define E_CTRL2_WMASK		0x000f
#define E_SET_CTRL2(X)		E_SREG_CTRL2 = (E_SREG_CTRL2 & \
					E_CTRL2_WMASK) | (X)
#define E_CLR_CTRL2(X)		E_SREG_CTRL2 &= E_CTRL2_WMASK & ~(X)

// machine start
void interpro_state::machine_start()
{
	m_emerald_reg[E_SREG_CTRL2] = E_CTRL2_COLDSTART | E_CTRL2_PWRENA | E_CTRL2_PWRUP;
}

void interpro_state::machine_reset()
{
	// flash rom requires the following values
	m_emerald_reg[E_SREG_ERROR] = 0x00;
	m_emerald_reg[E_SREG_STATUS] = 0x400;
	m_emerald_reg[E_SREG_CTRL1] = E_CTRL1_FLOPRDY;

	m_mcga[0] = 0x00ff;  // 0x00
	m_mcga[2] = MCGA_CTRL_ENREFRESH | MCGA_CTRL_CBITFRCSUB | MCGA_CTRL_CBITFRCRD;  // 0x08 ctrl
	//m_mcga[4] = 0x8000;  // 0x10 error
	m_mcga[10] = 0x00ff; // 0x28
	m_mcga[14] = 0x0340; // 0x38 memsize
}

WRITE16_MEMBER(interpro_state::emerald_w)
{
	switch (offset)
	{
	case E_SREG_LED:
		LOG_EMERALD("led: value %d at pc 0x%08x\n", data, space.device().safe_pc());
		m_led->a_w(data);
		break;

	case E_SREG_STATUS: // not sure if writable?
		break;

	case E_SREG_CTRL1:
		LOG_EMERALD("emerald write offset %d data 0x%x pc 0x%08x\n", offset, data, space.device().safe_pc());

		if ((data ^ m_emerald_reg[offset]) & E_CTRL1_LEDDP)
			LOG_EMERALD("emerald led decimal point %s\n", data & E_CTRL1_LEDDP ? "on" : "off");

		m_emerald_reg[offset] = data;
		break;

	case E_SREG_CTRL2:
		LOG_EMERALD("emerald write offset %d data 0x%x pc 0x%08x\n", offset, data, space.device().safe_pc());
		if (data & E_CTRL2_RESET)
		{
			m_emerald_reg[E_SREG_CTRL2] &= ~E_CTRL2_COLDSTART;

			machine().schedule_soft_reset();
		}
		else
			m_emerald_reg[offset] = data & 0x0f; // top four bits are not persistent
		break;
	}
}

READ16_MEMBER(interpro_state::emerald_r)
{
	LOG_EMERALD("emerald read offset %d pc 0x%08x\n", offset, space.device().safe_pc());
	switch (offset)
	{
	case E_SREG_ERROR:
	case E_SREG_STATUS:
	case E_SREG_CTRL1:
	case E_SREG_CTRL2:
	default:
		return m_emerald_reg[offset];
		break;
	}
}

WRITE16_MEMBER(interpro_state::mcga_w)
{
	/*
	read  MEMSIZE 0x38 mask 0xffff
	read  0x00 mask 0x0000
	write CBSUB   0x20 mask 0x00ff data 0
	write FRCRD   0x18 mask 0x00ff data 0
	read  ERROR   0x10 mask 0xffff
	read  0x00 mask 0xffff

	(0x38 >> 8) & 0xF == 3?

	if (0x00 != 0xFF) -> register reset error

	0x00 = 0x0055 (test value & 0xff)
	r7 = 0x00 & 0xff
	*/
	LOG_MCGA("mcga write offset = 0x%08x, mask = 0x%08x, data = 0x%08x, pc = 0x%08x\n", offset, mem_mask, data, space.device().safe_pc());
	switch (offset)
	{
	case 0x02: // MCGA_CTRL
		// HACK: set or clear error status depending on ENMMBE bit
		if (data & MCGA_CTRL_ENMMBE)
			m_mcga[4] |= MCGA_ERROR_VALID;
//		else
//			m_mcga[4] &= ~MCGA_ERROR_VALID;

	default:
		m_mcga[offset] = data;
		break;
	}
}

READ16_MEMBER(interpro_state::mcga_r)
{
	LOG_MCGA("mcga read offset = 0x%08x, mask = 0x%08x, pc = 0x%08x\n", offset, mem_mask, space.device().safe_pc());

	switch (offset)
	{
	default:
		return m_mcga[offset];
	}
}

/*
* ID Prom Structure
*/
struct IDprom {
	char	i_board[8];		/* string of board type */
	char	i_eco[8];		/* ECO flags */
	char	i_feature[8];		/* feature flags */
	char	i_reserved[2];		/* reserved (all 1's) */
	short	i_family;		/* family code */
	char	i_footprint[4];		/* footprint/checksum */
};


READ32_MEMBER(interpro_state::idprom_r)
{
	LOG_IDPROM("idprom read offset 0x%x mask 0x%08x at 0x%08x\n", offset, mem_mask, space.device().safe_pc());

	uint32_t speed = 70000000;

	static uint8_t idprom[] = {
#if 0
		// module type id
		'M', 'P', 'C', 'B',
		'*', '*', '*', '*',

		// ECO bytes
		0x87, 0x65, 0x43, 0x21,
		0xbb, 0xcc, 0xdd, 0xee,

		// the following 8 bytes are "feature bytes"
		// the feature bytes contain a 32 bit word which is divided by 40000
		// if they're empty, a default value of 50 000 000 is used
		// perhaps this is a system speed (50MHz)?
		0x12, 0x34, 0x56, 0x78,
		(speed >> 24) & 0xff, (speed >> 16) & 0xff, (speed >> 8) & 0xff, (speed >> 0) & 0xff,

		// reserved bytes
		0xff, 0xff, 

		// family
		// boot rom tests for family == 0x41 or 0x42
		// if so, speed read from feature bytes 2 & 3
		// if not, read speed from feature bytes 4-7
		0x00, 0x40,
#else
		// Carl Friend's 2020
		0x00, 0x00, 0x00, 0x00, '9',  '6',  '2',  'A',  // board
		0x00, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // eco
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // feature
		0xff, 0xff,										// reserved
		0x24, 0x00,										// family
#endif
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
	//static struct IDprom slot0 = { "bordtyp", "ecoflgs", "featurs", { '\xff', '\xff' }, 0x7ead, "chk" };

	// Carl Friend's Turqoise graphics board
	static uint8_t slot0[] = {
		0x00, 0x00, 0x00, 0x00, '9',  '6',  '3',  'A',	// board
		0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // eco
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // features
		0xff, 0xff,										// reserved
		0x22, 0x00,										// family
		0x55, 0xaa, 0x55, 0x00
	};

	return ((uint8_t *)&slot0)[offset];
}

WRITE8_MEMBER(interpro_state::interpro_rtc_w)
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
		logerror("rtc: write to unknown offset 0x%02x data 0x%02x at pc 0x%08x\n", offset, data, space.device().safe_pc());
		break;
	}
}

READ8_MEMBER(interpro_state::interpro_rtc_r)
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
		logerror("rtc: read from unknown offset 0x%02x at pc 0x%08x\n", offset, space.device().safe_pc());
		return 0xff;
	}
}

/*
MCGA: 40000xxx - refer iopsysreg.h
00: 		0005
08: CTRL	0010 // control register (hword)
10: ERROR	0400 // error register (hword)
18:		0000 // frcrd register (byte)
20:		0000 // cbsub register (byte)
28:		0088
30:		001B
38: MEMSIZE	0004

*/




// driver init
DRIVER_INIT_MEMBER(interpro_state, ip2800)
{
}

static ADDRESS_MAP_START(ip2800_map, AS_PROGRAM, 32, interpro_state)
	ADDRESS_MAP_UNMAP_LOW
	AM_RANGE(0x00000000, 0x00ffffff) AM_RAM // 16M RAM
	
	AM_RANGE(0x08000000, 0x0800001f) AM_RAM // bogus

	AM_RANGE(0x40000000, 0x4000003f) AM_READWRITE16(mcga_r, mcga_w, 0xffff)
	AM_RANGE(0x4f007e00, 0x4f007f7f) AM_RAM // treat SRX GA as ram for now

	AM_RANGE(0x7f000100, 0x7f00011f) AM_DEVICE8(INTERPRO_FDC_TAG, n82077aa_device, map, 0xff)

	AM_RANGE(0x7f000400, 0x7f00040f) AM_DEVREADWRITE8(INTERPRO_SCC1_TAG, scc85C30_device, ba_cd_inv_r, ba_cd_inv_w, 0xff)
	AM_RANGE(0x7f000410, 0x7f00041f) AM_DEVREADWRITE8(INTERPRO_SCC2_TAG, scc85230_device, ba_cd_inv_r, ba_cd_inv_w, 0xff)

	AM_RANGE(0x7f000300, 0x7f00030f) AM_READWRITE16(emerald_r, emerald_w, 0xffff)

	AM_RANGE(0x7f000500, 0x7f0006ff) AM_READWRITE8(interpro_rtc_r, interpro_rtc_w, 0xff)
	AM_RANGE(0x7F000700, 0x7F00077f) AM_READ(idprom_r)

	AM_RANGE(0x7F0FFF00, 0x7F0FFFFF) AM_DEVICE(INTERPRO_IOGA_TAG, interpro_ioga_device, map) 

	AM_RANGE(0x7F100000, 0x7F11FFFF) AM_ROM AM_REGION(INTERPRO_ROM_TAG, 0)
	AM_RANGE(0x7F180000, 0x7F1BFFFF) AM_ROM AM_REGION(INTERPRO_EEPROM_TAG, 0)

	AM_RANGE(0x8f007f80, 0x8f007fff) AM_READ(slot0_r)

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

static MACHINE_CONFIG_START(ip2800, interpro_state)
	MCFG_CPU_ADD(INTERPRO_CPU_TAG, CLIPPER, 10000000)
	MCFG_CPU_PROGRAM_MAP(ip2800_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE(INTERPRO_IOGA_TAG, interpro_ioga_device, inta_cb)

	MCFG_SCC85C30_ADD(INTERPRO_SCC1_TAG, XTAL_4_9152MHz, 0, 0, 0, 0) 

	MCFG_Z80SCC_OUT_TXDA_CB(DEVWRITELINE("rs232a", rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_TXDB_CB(DEVWRITELINE("rs232b", rs232_port_device, write_txd))
	MCFG_Z80SCC_OUT_INT_CB(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, ir11_w))

	MCFG_RS232_PORT_ADD("rs232a", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, rxa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, dcda_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, ctsa_w))

	MCFG_RS232_PORT_ADD("rs232b", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, rxb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, dcdb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE(INTERPRO_SCC1_TAG, z80scc_device, ctsb_w))

	MCFG_SCC85230_ADD(INTERPRO_SCC2_TAG, XTAL_4_9152MHz, 0, 0, 0, 0)

	MCFG_MC146818_ADD(INTERPRO_RTC_TAG, XTAL_32_768kHz)
	MCFG_MC146818_UTC(true)
	MCFG_MC146818_IRQ_HANDLER(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, ir9_w))

	MCFG_DEVICE_ADD(INTERPRO_LED_TAG, DM9368, 0)

	MCFG_N82077AA_ADD(INTERPRO_FDC_TAG, n82077aa_device::MODE_PS2)
	MCFG_UPD765_INTRQ_CALLBACK(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, ir1_w))
	MCFG_UPD765_DRQ_CALLBACK(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, drq))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", interpro_floppies, "525dd", interpro_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", interpro_floppies, "35hd", interpro_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(false)

	MCFG_DEVICE_ADD("scsiport", SCSI_PORT, 0)

	MCFG_DEVICE_ADD(INTERPRO_SCSI_TAG, NCR539X, 12500000)
	MCFG_LEGACY_SCSI_PORT("scsiport")
	MCFG_NCR539X_OUT_IRQ_CB(DEVWRITELINE(INTERPRO_IOGA_TAG, interpro_ioga_device, ir0_w))

	MCFG_INTERPRO_IOGA_ADD(INTERPRO_IOGA_TAG)
	MCFG_INTERPRO_IOGA_NMI_CB(INPUTLINE(INTERPRO_CPU_TAG, INPUT_LINE_NMI))
	MCFG_INTERPRO_IOGA_IRQ_CB(INPUTLINE(INTERPRO_CPU_TAG, INPUT_LINE_IRQ0))

	// use callbacks to tell the ioga what the dma read and write methods of each device are
	// MCFG_INTERPRO_IOGA_DMA_CALLBACK(channel, n82077aa_device, dma_r, dma_w)

MACHINE_CONFIG_END

ROM_START(ip2800)
	ROM_REGION(0x0020000, INTERPRO_ROM_TAG, 0)
	ROM_SYSTEM_BIOS(0, "IP2830", "IP2830")
	ROMX_LOAD("ip2830_rom.bin", 0x00000, 0x20000, CRC(467ce7bd) SHA1(53faee40d5df311f53b24c930e434cbf94a5c4aa), ROM_BIOS(1))

	ROM_REGION(0x0040000, INTERPRO_EEPROM_TAG, 0)
	ROM_LOAD_OPTIONAL("ip2830_eeprom.bin", 0x00000, 0x40000, CRC(a0c0899f) SHA1(dda6fbca81f9885a1a76ca3c25e80463a83a0ef7))
ROM_END

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT   CLASS			INIT	COMPANY         FULLNAME         FLAGS */
COMP( 1990, ip2800,          0, 0,      ip2800,		ip2800,	interpro_state, ip2800, "Intergraph",   "InterPro 2800", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
