// license:GPL-2.0+
// copyright-holders:Brandon Munger
/**********************************************************************

    ROLM 9751 9005 Peripheral device controller emulation

**********************************************************************/

/*
	Notes
*/

#include "pdc.h"


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80_TAG		"pdc_z80" // U19
#define FDC_TAG         "fdc"
#define HDC_TAG		"hdc"
#define FDCDMA_TAG	"i8237dma"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PDC = &device_creator<pdc_device>;

//-------------------------------------------------
//  ROM( PDC )
//-------------------------------------------------

ROM_START( pdc )
        ROM_REGION( 0x4000, "rom", 0 )
        ROM_LOAD( "97d9988.27128.pdc.u17", 0x0000, 0x4000, CRC(d96ccaa6) SHA1(e1a465c2274a63e81dba7a71fc8b30f10c03baf0) ) // Label: "97D9988" 27128 @U17
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *pdc_device::device_rom_region() const
{
        return ROM_NAME( pdc );
}

//-------------------------------------------------
//  ADDRESS_MAP( pdc_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( pdc_mem, AS_PROGRAM, 8, pdc_device )
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION("rom", 0)
	AM_RANGE(0x8000, 0x9FFF) AM_RAM AM_SHARE("pdc_ram") // HM6264ALP-12 SRAM 8KB
	AM_RANGE(0xC000, 0xC7FF) AM_RAM // HM6116P-2 SRAM 2KB
ADDRESS_MAP_END

//-------------------------------------------------
//  ADDRESS_MAP( pdc_io )
//-------------------------------------------------

static ADDRESS_MAP_START( pdc_io, AS_IO, 8, pdc_device )
	AM_RANGE(0x00, 0x07) AM_READWRITE(p0_7_r,p0_7_w) AM_MIRROR(0xFF00)
	AM_RANGE(0x21, 0x2F) AM_READWRITE(fdd_68k_r,fdd_68k_w) AM_MIRROR(0xFF00)
	AM_RANGE(0x38, 0x38) AM_READ(p38_r) AM_MIRROR(0xFF00) // Possibly UPD765 interrupt
	AM_RANGE(0x39, 0x39) AM_READ(p39_r) AM_MIRROR(0xFF00) // HDD related
	AM_RANGE(0x40, 0x41) AM_DEVREADWRITE(HDC_TAG, hdc9224_device,read,write) AM_MIRROR(0xFF00)
	AM_RANGE(0x42, 0x43) AM_DEVICE(FDC_TAG, upd765a_device, map) AM_MIRROR(0xFF00)
	AM_RANGE(0x50, 0x53) AM_WRITE(p50_53_w) AM_MIRROR(0xFF00)
	AM_RANGE(0x60, 0x6f) AM_DEVREADWRITE(FDCDMA_TAG,am9517a_device,read,write) AM_MIRROR(0xFF00)
ADDRESS_MAP_END

//-------------------------------------------------
//  SLOT_INTERFACE( pdc_floppies )
//-------------------------------------------------

static SLOT_INTERFACE_START( pdc_floppies )
	SLOT_INTERFACE( "35hd", FLOPPY_35_HD )
SLOT_INTERFACE_END

//-------------------------------------------------
//  SLOT_INTERFACE( pdc_harddisks )
//-------------------------------------------------

static SLOT_INTERFACE_START( pdc_harddisks )
        SLOT_INTERFACE( "generic", MFMHD_GENERIC )    // Generic hard disk (self-adapting to image)
        SLOT_INTERFACE( "st213", MFMHD_ST213 )        // Seagate ST-213 (10 MB)
        SLOT_INTERFACE( "st225", MFMHD_ST225 )        // Seagate ST-225 (20 MB)
        SLOT_INTERFACE( "st251", MFMHD_ST251 )        // Seagate ST-251 (40 MB)
SLOT_INTERFACE_END

//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( pdc_device::floppy_formats )
        FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

//-------------------------------------------------
//  MACHINE_DRIVER( pdc )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( pdc )
	/* CPU - Zilog Z0840006PSC */
        MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_10MHz / 2)
        MCFG_CPU_PROGRAM_MAP(pdc_mem)
	MCFG_CPU_IO_MAP(pdc_io)
	//MCFG_QUANTUM_PERFECT_CPU(M6502_TAG)

	/* Floppy Disk Controller - uPD765a - NEC D765AC-2 */
	MCFG_UPD765A_ADD(FDC_TAG, true, true)
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(pdc_device, fdc_irq))
	MCFG_UPD765_DRQ_CALLBACK(DEVWRITELINE(FDCDMA_TAG, am9517a_device, dreq0_w)) //MCFG_DEVCB_INVERT

	// Floppy disk drive
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG":0", pdc_floppies, "35hd", pdc_device::floppy_formats)
	//MCFG_FLOPPY_DRIVE_ADD(FDC_TAG":0", pdc_floppies, "35hd", floppy_image_device::default_floppy_formats)

	/* DMA Controller - Intel P8237A-5 */
	/* Channel 0: uPD765a Floppy Disk Controller */
	/* Channel 1: M68K main system memory */
	MCFG_DEVICE_ADD(FDCDMA_TAG, AM9517A, XTAL_10MHz / 2)
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(pdc_device, i8237_hreq_w))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(pdc_device, i8237_eop_w))
	MCFG_I8237_IN_MEMR_CB(READ8(pdc_device, i8237_dma_mem_r))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(pdc_device, i8237_dma_mem_w))
	MCFG_I8237_IN_IOR_0_CB(READ8(pdc_device, i8237_fdc_dma_r))
	MCFG_I8237_OUT_IOW_0_CB(WRITE8(pdc_device, i8237_fdc_dma_w))
	MCFG_I8237_IN_IOR_1_CB(READ8(pdc_device, m68k_dma_r))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(pdc_device, m68k_dma_w))
        // MCFG_AM9517A_OUT_DACK_0_CB(WRITELINE(pdc_device, fdc_dack_w))

	/* Hard Disk Controller - HDC9224 */
	MCFG_DEVICE_ADD(HDC_TAG, HDC9224, 0)
	MCFG_MFM_HARDDISK_CONN_ADD("h1", pdc_harddisks, NULL, MFM_BYTE, 3000, 20, MFMHD_GEN_FORMAT)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor pdc_device::device_mconfig_additions() const
{
        return MACHINE_CONFIG_NAME( pdc );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  pdc_device - constructor
//-------------------------------------------------

pdc_device::pdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, PDC, "ROLM PDC", tag, owner, clock, "pdc", __FILE__),
        m_pdccpu(*this, Z80_TAG),
	m_dma8237(*this, FDCDMA_TAG),
	m_fdc(*this, FDC_TAG),
	//m_floppy(*this, FDC_TAG ":0"),
	//m_floppy(*this, FDC_TAG ":0:35hd"),
	m_hdc9224(*this, HDC_TAG),
	m_pdc_ram(*this, "pdc_ram"),
	m_m68k_r_cb(*this),
	m_m68k_w_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pdc_device::device_start()
{
//	m_fdc->set_floppy(m_floppy);
//	floppy_image_device *m_floppy;
//	m_floppy = machine().device<floppy_connector>("fdc:0")->get_device();

}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pdc_device::device_reset()
{
	/* Reset registers */
	reg_p38 = 0;
	reg_p38 |= 4; /* ready for 68k ram DMA */
	//reg_p38 |= 0x20; // no idea at all - bit 5 (32)
	//m_fdc->ready_w(true);
	//m_floppy->mon_w(0);
	//m_fdc->set_floppy(m_floppy);
	//m_fdc->tc_w(1);
	//m_floppy->ready_w(true);

	/* Reset CPU */
        m_pdccpu->reset();

	/* Resolve callbacks */
	m_m68k_r_cb.resolve_safe(0);
	m_m68k_w_cb.resolve_safe();

	//machine().device<floppy_connector>(FDC_TAG":0")->get_device()->mon_w(false);
	//subdevice<floppy_connector>("fdc:0")->get_device()->mon_w(true);
	m_fdc->set_rate(500000) ;
}

//-------------------------------------------------
//  I8237 DMA
//-------------------------------------------------

WRITE_LINE_MEMBER(pdc_device::i8237_hreq_w)
{
	m_pdccpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
	m_dma8237->hack_w(state);
}

WRITE_LINE_MEMBER(pdc_device::i8237_eop_w)
{
	m_fdc->tc_w(state);
	reg_p38 |= 4; /* ready for 68k ram DMA */
	if(state) m_dma8237->dreq1_w(0);
}

READ8_MEMBER(pdc_device::i8237_dma_mem_r)
{
	return m_pdccpu->space(AS_PROGRAM).read_byte(offset);
}

WRITE8_MEMBER(pdc_device::i8237_dma_mem_w)
{
	m_pdccpu->space(AS_PROGRAM).write_byte(offset,data);
}

READ8_MEMBER(pdc_device::i8237_fdc_dma_r)
{
	UINT8 ret = m_fdc->dma_r();
	logerror("PDC: 8237 DMA CHANNEL 0 READ ADDRESS: %08X, DATA: %02X\n", offset, ret );
	return ret;
}

WRITE8_MEMBER(pdc_device::i8237_fdc_dma_w)
{
	logerror("PDC: 8237 DMA CHANNEL 0 WRITE ADDRESS: %08X, DATA: %02X\n", offset, data );
	m_fdc->dma_w(data);
}

READ8_MEMBER(pdc_device::m68k_dma_r)
{
	UINT32 address;
	UINT8 data;

	address = fdd_68k_dma_r_address++;
	data =  m_m68k_r_cb(address);
	logerror("PDC: 8237 DMA CHANNEL 1 READ ADDRESS: %08X, DATA: %02X\n", address, data );
	return data;
}

WRITE8_MEMBER(pdc_device::m68k_dma_w)
{
	logerror("PDC: 8237 DMA CHANNEL 1 WRITE ADDRESS: %08X, DATA: %02X\n", fdd_68k_dma_w_address, data );
	m_m68k_w_cb(data);
	fdd_68k_dma_w_address++;
}

WRITE_LINE_MEMBER(pdc_device::hdd_irq)
{
	m_pdccpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);
}

WRITE_LINE_MEMBER(pdc_device::fdc_irq)
{
	b_fdc_irq = state != 0;
}
READ8_MEMBER(pdc_device::p0_7_r)
{
	switch(offset)
	{
		case 0: /* Port 0: Old style command low byte [0x5FF041B0] */
			logerror("PDC: Port 0x00 READ: %02X\n", reg_p0);
			return reg_p0;
		case 1: /* Port 1: Old style command high byte [0x5FF041B0] */
			logerror("PDC: Port 0x01 READ: %02X\n", reg_p1);
			return reg_p1;
		case 2: /* Port 2: FDD command address low byte [0x5FF0C0B0][0x5FF0C1B0] */
			logerror("PDC: Port 0x02 READ: %02X\n", reg_p2);
			fdd_68k_dma_r_address = (fdd_68k_dma_r_address & (0xFF<<9)) | (reg_p2 << 1);
			return reg_p2;
		case 3: /* Port 3: FDD command address high byte [0x5FF0C0B0][0x5FF0C1B0] */
			logerror("PDC: Port 0x03 READ: %02X\n", reg_p3);
			fdd_68k_dma_r_address = (fdd_68k_dma_r_address & (0xFF<<1)) | (reg_p3 << 9);
			return reg_p3;
		case 6: /* Port 6: FDD data destination address low byte [0x5FF080B0] */
			logerror("PDC: Port 0x06 READ: %02X\n", reg_p6);
			return reg_p6;
		case 7: /* Port 7: FDD data destination address high byte [0x5FF080B0] */
			logerror("PDC: Port 0x07 READ: %02X\n", reg_p7);
			return reg_p7;
		default:
			logerror("(!)PDC: Port %02X READ: \n", offset);
			return 0;
	}
}

WRITE8_MEMBER(pdc_device::p0_7_w)
{
	switch(offset)
	{
		case 4: /* Port 4: FDD command completion status low byte [0x5FF030B0] */
			logerror("PDC: Port 0x04 WRITE: %02X\n", data);
			reg_p4 = data;
			break;
		case 5: /* Port 5: FDD command completion status high byte [0x5FF030B0] */
			logerror("PDC: Port 0x05 WRITE: %02X\n", data);
			reg_p5 = data;
			break;
		default:
			logerror("(!)PDC: Port %02X WRITE: %02X\n", offset, data);
			break;
	}
}

READ8_MEMBER(pdc_device::fdd_68k_r)
{
	UINT8 address = offset + 0x21;
	switch(address)
	{
		default:
			logerror("(!)PDC: Port %02X READ: \n", address);
			return 0;
	}
}
WRITE8_MEMBER(pdc_device::fdd_68k_w)
{
	UINT8 address = offset + 0x21;
	switch(address)
	{
		case 0x21: /* Port 21: ?? */
			logerror("PDC: Port 0x21 WRITE: %02X\n", data);
			logerror("PDC: Resetting 0x38 bit 1\n");
			reg_p38 &= ~2; // Clear bit 1
			reg_p21 = data;
			break;
		case 0x23: /* Port 23: FDD 68k DMA high byte */
			/* The address is << 1 on the 68k side */
			fdd_68k_dma_w_address = (fdd_68k_dma_w_address & (0xFF<<1)) | (data << 9);
			logerror("PDC: Port %02X WRITE: %02X\n", address, data);
			break;
		case 0x24: /* Port 24: FDD 68k DMA low byte */
			/* The address is << 1 on the 68k side */
			fdd_68k_dma_w_address = (fdd_68k_dma_w_address & (0xFF<<9)) | (data << 1);
			logerror("PDC: Port %02X WRITE: %02X\n", address, data);
			break;
		case 0x26:
			switch(data)
			{
				case 0x80:
					m_dma8237->dreq1_w(1);
					reg_p38 &= ~4; // Clear bit 4
					logerror("PDC: Port 0x26 WRITE: 0x80, DMA REQ CH 1\n");
					break;
			}
			break;
		case 0x2C:
			switch(data)
			{
				case 0xFF:
					m_dma8237->dreq1_w(0);
					logerror("PDC: Port 0x2C WRITE: 0xFF, DMA REQ CH 1 OFF\n");
					break;
			}
			break;
		default:
			logerror("(!)PDC: Port %02X WRITE: %02X, PC: %X\n", address, data, space.device().safe_pc());
			break;
	}
}

WRITE8_MEMBER(pdc_device::p38_w)
{
	logerror("PDC: Port 0x38 WRITE: %i\n", data);
	//reg_p38 |= data;
	reg_p38 = data;
}

READ8_MEMBER(pdc_device::p38_r)
{
	reg_p38 ^= 0x20; /* Invert bit 5 (32) */
	//UINT8 retn;
	logerror("PDC: Port 0x38 READ: %02X, PC: %X\n", reg_p38, space.device().safe_pc());
	//retn = reg_p38;
	//reg_p38 &= ~2; // Clear bit 1
	//return retn;
	return reg_p38;
}

READ8_MEMBER(pdc_device::p39_r)
{
	UINT8 data = 1;
	if(b_fdc_irq) data |= 8; // Set bit 3
	logerror("PDC: Port 0x39 READ: %02X, PC: %X\n", data, space.device().safe_pc());
	return data;
}

WRITE8_MEMBER(pdc_device::p50_53_w)
{
	UINT8 address = 0x50 + offset;
	switch(address)
	{
		case 0x53: /* Port 53: Almost certainly not FDD motor control, but seems to work */
			switch(data)
			{
				case 0x80:
					logerror("PDC: FDD Motor on.\n");
					m_fdc->subdevice<floppy_connector>("0")->get_device()->mon_w(0);
					break;
				default:
					logerror("PDC: Port 0x53 WRITE: %x\n", data);
			}
			break;
		default:
			logerror("PDC: Port %02x WRITE: %x\n", address, data);
	}
}
