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
//        AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x6000) AM_RAM
//        AM_RANGE(0x1800, 0x180f) AM_MIRROR(0x63f0) AM_DEVREADWRITE(M6522_0_TAG, via6522_device, read, write)
//        AM_RANGE(0x1c00, 0x1c0f) AM_MIRROR(0x63f0) AM_DEVREADWRITE(M6522_1_TAG, via6522_device, read, write)
//        AM_RANGE(0x8000, 0xbfff) AM_MIRROR(0x4000) AM_ROM AM_REGION(M6502_TAG, 0)
	AM_RANGE(0x0000, 0x3fff) AM_ROM AM_REGION("rom", 0)
	AM_RANGE(0x8000, 0x9FFF) AM_RAM AM_SHARE("pdc_ram") // HM6264ALP-12 SRAM 8KB
	AM_RANGE(0xC000, 0xC7FF) AM_RAM // HM6116P-2 SRAM 2KB
//	AM_RANGE(0x0000, 0x3fff) AM_RAM AM_SHARE("pdc_ram")
ADDRESS_MAP_END

//-------------------------------------------------
//  ADDRESS_MAP( pdc_io )
//-------------------------------------------------

static ADDRESS_MAP_START( pdc_io, AS_IO, 8, pdc_device )
	AM_RANGE(0x40, 0x41) AM_DEVREADWRITE(HDC_TAG, hdc9224_device,read,write) AM_MIRROR(0xFF00)
	AM_RANGE(0x42, 0x43) AM_DEVICE(FDC_TAG, upd765a_device, map) AM_MIRROR(0xFF00)
	AM_RANGE(0xd0, 0xdf) AM_DEVREADWRITE(FDCDMA_TAG,am9517a_device,read,write) AM_MIRROR(0xFF00)
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
	// CPU - Zilog Z0840006PSC
        MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_10MHz / 2)
        MCFG_CPU_PROGRAM_MAP(pdc_mem)
	MCFG_CPU_IO_MAP(pdc_io)
//        MCFG_QUANTUM_PERFECT_CPU(M6502_TAG)

//        MCFG_DEVICE_ADD(M6522_0_TAG, VIA6522, XTAL_16MHz/16)
//        MCFG_VIA6522_READPA_HANDLER(READ8(c1541_base_t, via0_pa_r))
//        MCFG_VIA6522_READPB_HANDLER(READ8(c1541_base_t, via0_pb_r))
//        MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(c1541_base_t, via0_pa_w))
//        MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(c1541_base_t, via0_pb_w))
//        MCFG_VIA6522_CB2_HANDLER(WRITELINE(c1541_base_t, via0_ca2_w))
//        MCFG_VIA6522_IRQ_HANDLER(WRITELINE(c1541_base_t, via0_irq_w))

//        MCFG_DEVICE_ADD(M6522_1_TAG, VIA6522, XTAL_16MHz/16)
//        MCFG_VIA6522_READPA_HANDLER(DEVREAD8(C64H156_TAG, c64h156_device, yb_r))
//        MCFG_VIA6522_READPB_HANDLER(READ8(c1541_base_t, via1_pb_r))
//        MCFG_VIA6522_WRITEPA_HANDLER(DEVWRITE8(C64H156_TAG, c64h156_device, yb_w))
//        MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(c1541_base_t, via1_pb_w))
//        MCFG_VIA6522_CA2_HANDLER(DEVWRITELINE(C64H156_TAG, c64h156_device, soe_w))
//        MCFG_VIA6522_CB2_HANDLER(DEVWRITELINE(C64H156_TAG, c64h156_device, oe_w))
//        MCFG_VIA6522_IRQ_HANDLER(WRITELINE(c1541_base_t, via1_irq_w))

//        MCFG_DEVICE_ADD(C64H156_TAG, C64H156, XTAL_16MHz)
//        MCFG_64H156_ATN_CALLBACK(WRITELINE(c1541_base_t, atn_w))
//        MCFG_64H156_BYTE_CALLBACK(WRITELINE(c1541_base_t, byte_w))
//        MCFG_FLOPPY_DRIVE_ADD(C64H156_TAG":0", c1540_floppies, "525ssqd", c1541_base_t::floppy_formats)

	// uPD765a FDC - NEC D765AC-2
//	MCFG_UPD765A_ADD("upd765a", true, false)
	MCFG_UPD765A_ADD(FDC_TAG, true, true)
//	MCFG_UPD765_INTRQ_CALLBACK(DEVWRITELINE("ctc", z80ctc_device, trg3))
	MCFG_UPD765_INTRQ_CALLBACK(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_UPD765_DRQ_CALLBACK(DEVWRITELINE(FDCDMA_TAG, am9517a_device, dreq0_w)) MCFG_DEVCB_INVERT

	// Floppy disk drive
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG":0", pdc_floppies, "35hd", pdc_device::floppy_formats)

	// FDC DMA Controller - Intel P8237A-5
	MCFG_DEVICE_ADD(FDCDMA_TAG, AM9517A, XTAL_10MHz / 2)
//	MCFG_I8237_IN_MEMR_CB(READ8(pdc_device, memory_read_byte))
//	MCFG_I8237_OUT_MEMW_CB(WRITE8(pdc_device, memory_write_byte))
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(pdc_device, i8237_hreq_w))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(pdc_device, i8237_eop_w))
	MCFG_I8237_IN_MEMR_CB(READ8(pdc_device, i8237_dma_mem_r))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(pdc_device, i8237_dma_mem_w))
	MCFG_I8237_IN_IOR_0_CB(READ8(pdc_device, i8237_fdc_dma_r))
	MCFG_I8237_OUT_IOW_0_CB(WRITE8(pdc_device, i8237_fdc_dma_w))
        // MCFG_AM9517A_OUT_DACK_0_CB(WRITELINE(pdc_device, fdc_dack_w))

	// HDC9224 HDC
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
//  c1541_base_t - constructor
//-------------------------------------------------

//pdc_device::pdc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
pdc_device::pdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        //device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_t(mconfig, PDC, "ROLM PDC", tag, owner, clock, "pdc", __FILE__),
        //device_cbm_iec_interface(mconfig, *this),
        //device_c64_floppy_parallel_interface(mconfig, *this),
        m_pdccpu(*this, Z80_TAG),
        //m_via0(*this, M6522_0_TAG),
        //m_via1(*this, M6522_1_TAG),
        //m_ga(*this, C64H156_TAG),
        //m_floppy(*this, C64H156_TAG":0:525ssqd"),
//	m_fdc(*this, FDC_TAG ":35hd")
	m_dma8237(*this, FDCDMA_TAG),
	m_fdc(*this, FDC_TAG),
	m_floppy(*this, FDC_TAG ":0"),
	m_hdc9224(*this, HDC_TAG),
//	m_harddisk(*this, "h1"),
        //m_address(*this, "ADDRESS"),
        //m_data_out(1),
        //m_via0_irq(CLEAR_LINE),
        //m_via1_irq(CLEAR_LINE)
	m_pdc_ram(*this, "pdc_ram")
{
}

//-------------------------------------------------
//  c1540_t - constructor
//-------------------------------------------------

//pdc_device::pdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
//        : pdc_device_t(mconfig, PDC, "ROLM PDC", tag, owner, clock, "pdc", __FILE__) { }

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void pdc_device::device_start()
{
        // install image callbacks
//        m_ga->set_floppy(m_floppy);

        // register for state saving
//        save_item(NAME(m_data_out));
//        save_item(NAME(m_via0_irq));
//        save_item(NAME(m_via1_irq));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void pdc_device::device_reset()
{
//	UINT8 *rom = memregion("rom")->base();
//	UINT8 *ram = m_pdc_ram;
//	memcpy(ram, rom, 0x4000);

        m_pdccpu->reset();

//        m_via0->reset();
//        m_via1->reset();

        // initialize gate array
//        m_ga->accl_w(0);
//        m_ga->ted_w(1);
}

//-------------------------------------------------
//  I8237 DMA
//-------------------------------------------------

//READ8_MEMBER(pdc_device::memory_read_byte)
//{
//        address_space& prog_space = m_pdccpu->space(AS_PROGRAM);
//        return prog_space.read_byte(offset);
//}

//WRITE8_MEMBER(pdc_device::memory_write_byte)
//{
//        address_space& prog_space = m_pdccpu->space(AS_PROGRAM);
//        return prog_space.write_byte(offset, data);
//}

WRITE_LINE_MEMBER(pdc_device::i8237_hreq_w)
{
	m_pdccpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
	m_dma8237->hack_w(state);
}

WRITE_LINE_MEMBER(pdc_device::i8237_eop_w)
{
	m_fdc->tc_w(state);
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
	return ret;
}

WRITE8_MEMBER(pdc_device::i8237_fdc_dma_w)
{
	m_fdc->dma_w(data);
}

