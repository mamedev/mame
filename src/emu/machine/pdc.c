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

#define Z80_TAG       "u19"

#define FDC_TAG         "fdc"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type PDC = &device_creator<pdc_device>;

//-------------------------------------------------
//  ROM( PDC )
//-------------------------------------------------

ROM_START( pdc )
        ROM_REGION( 0x4000, "rom", 0 )
        ROM_LOAD( "pdc-97d9988.u17", 0x0000, 0x4000, CRC(d96ccaa6) SHA1(e1a465c2274a63e81dba7a71fc8b30f10c03baf0) )
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
//	AM_RANGE(0x0000, 0x3fff) AM_RAM AM_SHARE("pdc_ram")
ADDRESS_MAP_END

//-------------------------------------------------
//  ADDRESS_MAP( pdc_io )
//-------------------------------------------------

static ADDRESS_MAP_START( pdc_io, AS_IO, 8, pdc_device )
	//AM_RANGE(0x10, 0x11) AM_DEVICE(FDC_TAG, upd765a_device, map)
ADDRESS_MAP_END

//-------------------------------------------------
//  SLOT_INTERFACE( pdc_floppies )
//-------------------------------------------------

static SLOT_INTERFACE_START( pdc_floppies )
	SLOT_INTERFACE( "35hd", FLOPPY_35_HD )
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
//	MCFG_UPD765A_ADD("upd765a", true, false)
	MCFG_UPD765A_ADD(FDC_TAG, true, false)
//	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG, pdc_floppies, "35hd", pdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG":0", pdc_floppies, "35hd", pdc_device::floppy_formats)
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
	m_fdc(*this, FDC_TAG),
	m_floppy(*this, FDC_TAG ":0"),
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

