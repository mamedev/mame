// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 2040/3040/4040 Disk Drive emulation

**********************************************************************/

/*

    2040/3040 disk initialization
    -----------------------------
    You need to initialize each diskette before trying to access it
    or you will get a DISK ID MISMATCH error upon disk commands.
    On the 4040 this is done automatically by the DOS.

    open 15,8,15:print 15,"i":close 15

    List directory
    --------------
    directory / diR

    Format disk
    -----------
    header "label,id",d0,i01

    Load file
    ---------
    dload "name" / dL"name

    Save file
    ---------
    dsave "name" / dS"name

*/

/*

    TODO:

    - 2040/3040/4040 have a Shugart SA390 drive (FLOPPY_525_SSSD_35T)

    - 2040 DOS 1 FDC rom (jumps to 104d while getting block header)

        FE70: jsr  $104D
        104D: m6502_brk#$00

*/

#include "c2040.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "un1"
#define M6532_0_TAG     "uc1"
#define M6532_1_TAG     "ue1"
#define M6504_TAG       "uh3"
#define M6522_TAG       "um3"
#define M6530_TAG       "uk3"
#define FDC_TAG         "fdc"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C2040 = &device_creator<c2040_t>;
const device_type C3040 = &device_creator<c3040_t>;
const device_type C4040 = &device_creator<c4040_t>;


//-------------------------------------------------
//  ROM( c2040 )
//-------------------------------------------------

ROM_START( c2040 ) // schematic 320806, DOS 1.0
	ROM_REGION( 0x3000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS("dos12")
	ROM_LOAD( "901468-xx.ul1", 0x1000, 0x1000, NO_DUMP )
	ROM_LOAD( "901468-xx.uh1", 0x2000, 0x1000, NO_DUMP )

	ROM_REGION( 0x400, M6504_TAG, 0 )
	ROM_LOAD( "901466-01.uk3", 0x000, 0x400, CRC(9d1e25ce) SHA1(d539858f839f96393f218307df7394362a84a26a) )

	ROM_REGION( 0x800, "gcr", 0)
	ROM_LOAD( "901467.uk6",    0x000, 0x800, CRC(a23337eb) SHA1(97df576397608455616331f8e837cb3404363fa2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c2040_t::device_rom_region() const
{
	return ROM_NAME( c2040 );
}


//-------------------------------------------------
//  ROM( c3040 )
//-------------------------------------------------

ROM_START( c3040 ) // schematic 320806, DOS 1.2
	ROM_REGION( 0x3000, M6502_TAG, 0 )
	ROM_LOAD( "901468-06.ul1", 0x1000, 0x1000, CRC(25b5eed5) SHA1(4d9658f2e6ff3276e5c6e224611a66ce44b16fc7) )
	ROM_LOAD( "901468-07.uh1", 0x2000, 0x1000, CRC(9b09ae83) SHA1(6a51c7954938439ca8342fc295bda050c06e1791) )

	ROM_REGION( 0x400, M6504_TAG, 0 )
	ROM_LOAD( "901466-02.uk3", 0x000, 0x400, CRC(9d1e25ce) SHA1(d539858f839f96393f218307df7394362a84a26a) )

	ROM_REGION( 0x800, "gcr", 0)
	ROM_LOAD( "901467.uk6",    0x000, 0x800, CRC(a23337eb) SHA1(97df576397608455616331f8e837cb3404363fa2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c3040_t::device_rom_region() const
{
	return ROM_NAME( c3040 );
}


//-------------------------------------------------
//  ROM( c4040 )
//-------------------------------------------------

ROM_START( c4040 ) // schematic ?
	ROM_REGION( 0x3000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS("dos20r2")
	ROM_SYSTEM_BIOS( 0, "dos20r1", "DOS 2.0 Revision 1" )
	ROMX_LOAD( "901468-11.uj1", 0x0000, 0x1000, CRC(b7157458) SHA1(8415f3159dea73161e0cef7960afa6c76953b6f8), ROM_BIOS(1) )
	ROMX_LOAD( "901468-12.ul1", 0x1000, 0x1000, CRC(02c44ff9) SHA1(e8a94f239082d45f64f01b2d8e488d18fe659cbb), ROM_BIOS(1) )
	ROMX_LOAD( "901468-13.uh1", 0x2000, 0x1000, CRC(cbd785b3) SHA1(6ada7904ac9d13c3f1c0a8715f9c4be1aa6eb0bb), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "dos20r2", "DOS 2.0 Revision 2" )
	ROMX_LOAD( "901468-14.uj1", 0x0000, 0x1000, CRC(bc4d4872) SHA1(ffb992b82ec913ddff7be964d7527aca3e21580c), ROM_BIOS(2) )
	ROMX_LOAD( "901468-15.ul1", 0x1000, 0x1000, CRC(b6970533) SHA1(f702d6917fe8a798740ba4d467b500944ae7b70a), ROM_BIOS(2) )
	ROMX_LOAD( "901468-16.uh1", 0x2000, 0x1000, CRC(1f5eefb7) SHA1(04b918cf4adeee8015b43383d3cea7288a7d0aa8), ROM_BIOS(2) )

	ROM_REGION( 0x400, M6504_TAG, 0 )
	// RIOT DOS 2
	ROM_LOAD( "901466-04.uk3", 0x000, 0x400, CRC(0ab338dc) SHA1(6645fa40b81be1ff7d1384e9b52df06a26ab0bfb) )

	ROM_REGION( 0x800, "gcr", 0)
	ROM_LOAD( "901467.uk6",    0x000, 0x800, CRC(a23337eb) SHA1(97df576397608455616331f8e837cb3404363fa2) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *c4040_t::device_rom_region() const
{
	return ROM_NAME( c4040 );
}


//-------------------------------------------------
//  ADDRESS_MAP( c2040_main_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( c2040_main_mem, AS_PROGRAM, 8, c2040_t )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x007f) AM_MIRROR(0x0100) AM_DEVICE(M6532_0_TAG, mos6532_t, ram_map)
	AM_RANGE(0x0080, 0x00ff) AM_MIRROR(0x0100) AM_DEVICE(M6532_1_TAG, mos6532_t, ram_map)
	AM_RANGE(0x0200, 0x021f) AM_MIRROR(0x0d60) AM_DEVICE(M6532_0_TAG, mos6532_t, io_map)
	AM_RANGE(0x0280, 0x029f) AM_MIRROR(0x0d60) AM_DEVICE(M6532_1_TAG, mos6532_t, io_map)
	AM_RANGE(0x1000, 0x13ff) AM_MIRROR(0x0c00) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x2000, 0x23ff) AM_MIRROR(0x0c00) AM_RAM AM_SHARE("share2")
	AM_RANGE(0x3000, 0x33ff) AM_MIRROR(0x0c00) AM_RAM AM_SHARE("share3")
	AM_RANGE(0x4000, 0x43ff) AM_MIRROR(0x0c00) AM_RAM AM_SHARE("share4")
	AM_RANGE(0x5000, 0x7fff) AM_ROM AM_REGION(M6502_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( c2040_fdc_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( c2040_fdc_mem, AS_PROGRAM, 8, c2040_t )
	ADDRESS_MAP_GLOBAL_MASK(0x1fff)
	AM_RANGE(0x0000, 0x003f) AM_MIRROR(0x0300) AM_DEVICE(M6530_TAG, mos6530_t, ram_map)
	AM_RANGE(0x0040, 0x004f) AM_MIRROR(0x0330) AM_DEVICE(M6522_TAG, via6522_device, map)
	AM_RANGE(0x0080, 0x008f) AM_MIRROR(0x0330) AM_DEVICE(M6530_TAG, mos6530_t, io_map)
	AM_RANGE(0x0400, 0x07ff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x0800, 0x0bff) AM_RAM AM_SHARE("share2")
	AM_RANGE(0x0c00, 0x0fff) AM_RAM AM_SHARE("share3")
	AM_RANGE(0x1000, 0x13ff) AM_RAM AM_SHARE("share4")
	AM_RANGE(0x1c00, 0x1fff) AM_ROM AM_REGION(M6504_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  riot6532 uc1
//-------------------------------------------------

READ8_MEMBER( c2040_t::dio_r )
{
	/*

	    bit     description

	    PA0     DI0
	    PA1     DI1
	    PA2     DI2
	    PA3     DI3
	    PA4     DI4
	    PA5     DI5
	    PA6     DI6
	    PA7     DI7

	*/

	return m_bus->dio_r();
}

WRITE8_MEMBER( c2040_t::dio_w )
{
	/*

	    bit     description

	    PB0     DO0
	    PB1     DO1
	    PB2     DO2
	    PB3     DO3
	    PB4     DO4
	    PB5     DO5
	    PB6     DO6
	    PB7     DO7

	*/

	m_bus->dio_w(this, data);
}


//-------------------------------------------------
//  riot6532 ue1
//-------------------------------------------------

READ8_MEMBER( c2040_t::riot1_pa_r )
{
	/*

	    bit     description

	    PA0     ATNA
	    PA1     DACO
	    PA2     RFDO
	    PA3     EOIO
	    PA4     DAVO
	    PA5     EOII
	    PA6     DAVI
	    PA7     _ATN

	*/

	UINT8 data = 0;

	// end or identify in
	data |= m_bus->eoi_r() << 5;

	// data valid in
	data |= m_bus->dav_r() << 6;

	// attention
	data |= !m_bus->atn_r() << 7;

	return data;
}

WRITE8_MEMBER( c2040_t::riot1_pa_w )
{
	/*

	    bit     description

	    PA0     ATNA
	    PA1     DACO
	    PA2     RFDO
	    PA3     EOIO
	    PA4     DAVO
	    PA5     EOII
	    PA6     DAVI
	    PA7     _ATN

	*/

	// attention acknowledge
	m_atna = BIT(data, 0);

	// data accepted out
	m_daco = BIT(data, 1);

	// not ready for data out
	m_rfdo = BIT(data, 2);

	// end or identify out
	m_bus->eoi_w(this, BIT(data, 3));

	// data valid out
	m_bus->dav_w(this, BIT(data, 4));

	update_ieee_signals();
}

READ8_MEMBER( c2040_t::riot1_pb_r )
{
	/*

	    bit     description

	    PB0     DEVICE NUMBER SELECTION
	    PB1     DEVICE NUMBER SELECTION
	    PB2     DEVICE NUMBER SELECTION
	    PB3
	    PB4
	    PB5
	    PB6     DACI
	    PB7     RFDI

	*/

	UINT8 data = 0;

	// device number selection
	data |= m_slot->get_address() - 8;

	// data accepted in
	data |= m_bus->ndac_r() << 6;

	// ready for data in
	data |= m_bus->nrfd_r() << 7;

	return data;
}

WRITE8_MEMBER( c2040_t::riot1_pb_w )
{
	/*

	    bit     description

	    PB0
	    PB1
	    PB2
	    PB3     ACT LED 1
	    PB4     ACT LED 0
	    PB5     ERR LED
	    PB6
	    PB7

	*/

	// activity led 1
	output_set_led_value(LED_ACT1, BIT(data, 3));

	// activity led 0
	output_set_led_value(LED_ACT0, BIT(data, 4));

	// error led
	output_set_led_value(LED_ERR, BIT(data, 5));
}


WRITE8_MEMBER( c2040_t::via_pb_w )
{
	/*

	    bit     description

	    PB0     S1A
	    PB1     S1B
	    PB2     S0A
	    PB3     S0B
	    PB4     MTR1
	    PB5     MTR0
	    PB6
	    PB7

	*/

	// spindle motor 1
	m_fdc->mtr1_w(BIT(data, 4));

	// spindle motor 0
	m_fdc->mtr0_w(BIT(data, 5));

	// stepper motor 1
	m_fdc->stp1_w(data & 0x03);

	// stepper motor 0
	m_fdc->stp0_w((data >> 2) & 0x03);
}


//-------------------------------------------------
//  SLOT_INTERFACE( c2040_floppies )
//-------------------------------------------------

static SLOT_INTERFACE_START( c2040_floppies )
	SLOT_INTERFACE( "525ssqd", FLOPPY_525_SSQD )
SLOT_INTERFACE_END


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( c2040_t::floppy_formats )
	FLOPPY_C3040_FORMAT,
	FLOPPY_G64_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( c3040_t::floppy_formats )
	FLOPPY_C3040_FORMAT,
	FLOPPY_G64_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( c4040_t::floppy_formats )
	FLOPPY_C4040_FORMAT,
	FLOPPY_G64_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c2040 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c2040 )
	// DOS
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_16MHz/16)
	MCFG_CPU_PROGRAM_MAP(c2040_main_mem)

	MCFG_DEVICE_ADD(M6532_0_TAG, MOS6532n, XTAL_16MHz/16)
	MCFG_MOS6530n_IN_PA_CB(READ8(c2040_t, dio_r))
	MCFG_MOS6530n_OUT_PB_CB(WRITE8(c2040_t, dio_w))

	MCFG_DEVICE_ADD(M6532_1_TAG, MOS6532n, XTAL_16MHz/16)
	MCFG_MOS6530n_IN_PA_CB(READ8(c2040_t, riot1_pa_r))
	MCFG_MOS6530n_OUT_PA_CB(WRITE8(c2040_t, riot1_pa_w))
	MCFG_MOS6530n_IN_PB_CB(READ8(c2040_t, riot1_pb_r))
	MCFG_MOS6530n_OUT_PB_CB(WRITE8(c2040_t, riot1_pb_w))
	MCFG_MOS6530n_IRQ_CB(INPUTLINE(M6502_TAG, INPUT_LINE_IRQ0))

	// controller
	MCFG_CPU_ADD(M6504_TAG, M6504, XTAL_16MHz/16)
	MCFG_CPU_PROGRAM_MAP(c2040_fdc_mem)

	MCFG_DEVICE_ADD(M6522_TAG, VIA6522, XTAL_16MHz/16)
	MCFG_VIA6522_READPA_HANDLER(DEVREAD8(FDC_TAG, c2040_fdc_t, read))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(c2040_t, via_pb_w))
	MCFG_VIA6522_CA2_HANDLER(DEVWRITELINE(FDC_TAG, c2040_fdc_t, mode_sel_w))
	MCFG_VIA6522_CB2_HANDLER(DEVWRITELINE(FDC_TAG, c2040_fdc_t, rw_sel_w))

	MCFG_DEVICE_ADD(M6530_TAG, MOS6530n, XTAL_16MHz/16)
	MCFG_MOS6530n_OUT_PA_CB(DEVWRITE8(FDC_TAG, c2040_fdc_t, write))
	MCFG_MOS6530n_OUT_PB0_CB(DEVWRITELINE(FDC_TAG, c2040_fdc_t, drv_sel_w))
	MCFG_MOS6530n_OUT_PB1_CB(DEVWRITELINE(FDC_TAG, c2040_fdc_t, ds0_w))
	MCFG_MOS6530n_OUT_PB2_CB(DEVWRITELINE(FDC_TAG, c2040_fdc_t, ds1_w))
	MCFG_MOS6530n_OUT_PB7_CB(INPUTLINE(M6504_TAG, M6502_IRQ_LINE))
	MCFG_MOS6530n_IN_PB3_CB(DEVREADLINE(FDC_TAG, c2040_fdc_t, wps_r))

	MCFG_DEVICE_ADD(FDC_TAG, C2040_FDC, XTAL_16MHz)
	MCFG_C2040_SYNC_CALLBACK(DEVWRITELINE(M6530_TAG, mos6530_t, pb6_w))
	MCFG_C2040_READY_CALLBACK(DEVWRITELINE(M6522_TAG, via6522_device, write_ca1))
	MCFG_C2040_ERROR_CALLBACK(DEVWRITELINE(M6522_TAG, via6522_device, write_cb1))
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG":0", c2040_floppies, "525ssqd", c2040_t::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG":1", c2040_floppies, "525ssqd", c2040_t::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c2040_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c2040 );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c3040 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c3040 )
	// DOS
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_16MHz/16)
	MCFG_CPU_PROGRAM_MAP(c2040_main_mem)

	MCFG_DEVICE_ADD(M6532_0_TAG, MOS6532n, XTAL_16MHz/16)
	MCFG_MOS6530n_IN_PA_CB(READ8(c2040_t, dio_r))
	MCFG_MOS6530n_OUT_PB_CB(WRITE8(c2040_t, dio_w))

	MCFG_DEVICE_ADD(M6532_1_TAG, MOS6532n, XTAL_16MHz/16)
	MCFG_MOS6530n_IN_PA_CB(READ8(c2040_t, riot1_pa_r))
	MCFG_MOS6530n_OUT_PA_CB(WRITE8(c2040_t, riot1_pa_w))
	MCFG_MOS6530n_IN_PB_CB(READ8(c2040_t, riot1_pb_r))
	MCFG_MOS6530n_OUT_PB_CB(WRITE8(c2040_t, riot1_pb_w))
	MCFG_MOS6530n_IRQ_CB(INPUTLINE(M6502_TAG, INPUT_LINE_IRQ0))

	// controller
	MCFG_CPU_ADD(M6504_TAG, M6504, XTAL_16MHz/16)
	MCFG_CPU_PROGRAM_MAP(c2040_fdc_mem)

	MCFG_DEVICE_ADD(M6522_TAG, VIA6522, XTAL_16MHz/16)
	MCFG_VIA6522_READPA_HANDLER(DEVREAD8(FDC_TAG, c2040_fdc_t, read))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(c2040_t, via_pb_w))
	MCFG_VIA6522_CA2_HANDLER(DEVWRITELINE(FDC_TAG, c2040_fdc_t, mode_sel_w))
	MCFG_VIA6522_CB2_HANDLER(DEVWRITELINE(FDC_TAG, c2040_fdc_t, rw_sel_w))

	MCFG_DEVICE_ADD(M6530_TAG, MOS6530n, XTAL_16MHz/16)
	MCFG_MOS6530n_OUT_PA_CB(DEVWRITE8(FDC_TAG, c2040_fdc_t, write))
	MCFG_MOS6530n_OUT_PB0_CB(DEVWRITELINE(FDC_TAG, c2040_fdc_t, drv_sel_w))
	MCFG_MOS6530n_OUT_PB1_CB(DEVWRITELINE(FDC_TAG, c2040_fdc_t, ds0_w))
	MCFG_MOS6530n_OUT_PB2_CB(DEVWRITELINE(FDC_TAG, c2040_fdc_t, ds1_w))
	MCFG_MOS6530n_IN_PB3_CB(DEVREADLINE(FDC_TAG, c2040_fdc_t, wps_r))
	MCFG_MOS6530n_OUT_PB7_CB(INPUTLINE(M6504_TAG, M6502_IRQ_LINE))

	MCFG_DEVICE_ADD(FDC_TAG, C2040_FDC, XTAL_16MHz)
	MCFG_C2040_SYNC_CALLBACK(DEVWRITELINE(M6530_TAG, mos6530_t, pb6_w))
	MCFG_C2040_READY_CALLBACK(DEVWRITELINE(M6522_TAG, via6522_device, write_ca1))
	MCFG_C2040_ERROR_CALLBACK(DEVWRITELINE(M6522_TAG, via6522_device, write_cb1))
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG":0", c2040_floppies, "525ssqd", c3040_t::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG":1", c2040_floppies, "525ssqd", c3040_t::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c3040_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c3040 );
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( c4040 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c4040 )
	// DOS
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_16MHz/16)
	MCFG_CPU_PROGRAM_MAP(c2040_main_mem)

	MCFG_DEVICE_ADD(M6532_0_TAG, MOS6532n, XTAL_16MHz/16)
	MCFG_MOS6530n_IN_PA_CB(READ8(c2040_t, dio_r))
	MCFG_MOS6530n_OUT_PB_CB(WRITE8(c2040_t, dio_w))

	MCFG_DEVICE_ADD(M6532_1_TAG, MOS6532n, XTAL_16MHz/16)
	MCFG_MOS6530n_IN_PA_CB(READ8(c2040_t, riot1_pa_r))
	MCFG_MOS6530n_OUT_PA_CB(WRITE8(c2040_t, riot1_pa_w))
	MCFG_MOS6530n_IN_PB_CB(READ8(c2040_t, riot1_pb_r))
	MCFG_MOS6530n_OUT_PB_CB(WRITE8(c2040_t, riot1_pb_w))
	MCFG_MOS6530n_IRQ_CB(INPUTLINE(M6502_TAG, INPUT_LINE_IRQ0))

	// controller
	MCFG_CPU_ADD(M6504_TAG, M6504, XTAL_16MHz/16)
	MCFG_CPU_PROGRAM_MAP(c2040_fdc_mem)

	MCFG_DEVICE_ADD(M6522_TAG, VIA6522, XTAL_16MHz/16)
	MCFG_VIA6522_READPA_HANDLER(DEVREAD8(FDC_TAG, c2040_fdc_t, read))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(c2040_t, via_pb_w))
	MCFG_VIA6522_CA2_HANDLER(DEVWRITELINE(FDC_TAG, c2040_fdc_t, mode_sel_w))
	MCFG_VIA6522_CB2_HANDLER(DEVWRITELINE(FDC_TAG, c2040_fdc_t, rw_sel_w))

	MCFG_DEVICE_ADD(M6530_TAG, MOS6530n, XTAL_16MHz/16)
	MCFG_MOS6530n_OUT_PA_CB(DEVWRITE8(FDC_TAG, c2040_fdc_t, write))
	MCFG_MOS6530n_OUT_PB0_CB(DEVWRITELINE(FDC_TAG, c2040_fdc_t, drv_sel_w))
	MCFG_MOS6530n_OUT_PB1_CB(DEVWRITELINE(FDC_TAG, c2040_fdc_t, ds0_w))
	MCFG_MOS6530n_OUT_PB2_CB(DEVWRITELINE(FDC_TAG, c2040_fdc_t, ds1_w))
	MCFG_MOS6530n_IN_PB3_CB(DEVREADLINE(FDC_TAG, c2040_fdc_t, wps_r))
	MCFG_MOS6530n_OUT_PB7_CB(INPUTLINE(M6504_TAG, M6502_IRQ_LINE))

	MCFG_DEVICE_ADD(FDC_TAG, C2040_FDC, XTAL_16MHz)
	MCFG_C2040_SYNC_CALLBACK(DEVWRITELINE(M6530_TAG, mos6530_t, pb6_w))
	MCFG_C2040_READY_CALLBACK(DEVWRITELINE(M6522_TAG, via6522_device, write_ca1))
	MCFG_C2040_ERROR_CALLBACK(DEVWRITELINE(M6522_TAG, via6522_device, write_cb1))
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG":0", c2040_floppies, "525ssqd", c4040_t::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC_TAG":1", c2040_floppies, "525ssqd", c4040_t::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor c4040_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c4040 );
}


//-------------------------------------------------
//  INPUT_PORTS( c2040 )
//-------------------------------------------------

static INPUT_PORTS_START( c2040 )
	PORT_START("ADDRESS")
	PORT_DIPNAME( 0x07, 0x00, "Device Address" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x01, "9" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x03, "11" )
	PORT_DIPSETTING(    0x04, "12" )
	PORT_DIPSETTING(    0x05, "13" )
	PORT_DIPSETTING(    0x06, "14" )
	PORT_DIPSETTING(    0x07, "15" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c2040_t::device_input_ports() const
{
	return INPUT_PORTS_NAME( c2040 );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  update_ieee_signals -
//-------------------------------------------------

inline void c2040_t::update_ieee_signals()
{
	int atn = m_bus->atn_r();
	int nrfd = !(!(!(atn && m_atna) && m_rfdo) || !(atn || m_atna));
	int ndac = !(m_daco || !(atn || m_atna));

	m_bus->nrfd_w(this, nrfd);
	m_bus->ndac_w(this, ndac);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c2040_t - constructor
//-------------------------------------------------

c2040_t::c2040_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_ieee488_interface(mconfig, *this),
	m_maincpu(*this, M6502_TAG),
	m_fdccpu(*this, M6504_TAG),
	m_riot0(*this, M6532_0_TAG),
	m_riot1(*this, M6532_1_TAG),
	m_miot(*this, M6530_TAG),
	m_via(*this, M6522_TAG),
	m_floppy0(*this, FDC_TAG":0:525ssqd"),
	m_floppy1(*this, FDC_TAG":1:525ssqd"),
	m_fdc(*this, FDC_TAG),
	m_gcr(*this, "gcr"),
	m_address(*this, "ADDRESS"),
	m_rfdo(1),
	m_daco(1),
	m_atna(1), m_ifc(0)
{
}

c2040_t::c2040_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, C2040, "C2040", tag, owner, clock, "c2040", __FILE__),
	device_ieee488_interface(mconfig, *this),
	m_maincpu(*this, M6502_TAG),
	m_fdccpu(*this, M6504_TAG),
	m_riot0(*this, M6532_0_TAG),
	m_riot1(*this, M6532_1_TAG),
	m_miot(*this, M6530_TAG),
	m_via(*this, M6522_TAG),
	m_floppy0(*this, FDC_TAG":0:525ssqd"),
	m_floppy1(*this, FDC_TAG":1:525ssqd"),
	m_fdc(*this, FDC_TAG),
	m_gcr(*this, "gcr"),
	m_address(*this, "ADDRESS"),
	m_rfdo(1),
	m_daco(1),
	m_atna(1), m_ifc(0)
{
}


//-------------------------------------------------
//  c3040_t - constructor
//-------------------------------------------------

c3040_t::c3040_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	c2040_t(mconfig, C3040, "C3040", tag, owner, clock, "c3040", __FILE__) { }


//-------------------------------------------------
//  c4040_t - constructor
//-------------------------------------------------

c4040_t::c4040_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	c2040_t(mconfig, C4040, "C4040", tag, owner, clock, "c4040", __FILE__) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c2040_t::device_start()
{
	// install image callbacks
	m_fdc->set_floppy(m_floppy0, m_floppy1);

	// register for state saving
	save_item(NAME(m_rfdo));
	save_item(NAME(m_daco));
	save_item(NAME(m_atna));
	save_item(NAME(m_ifc));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c2040_t::device_reset()
{
	m_maincpu->reset();

	// toggle M6502 SO
	m_maincpu->set_input_line(M6502_SET_OVERFLOW, ASSERT_LINE);
	m_maincpu->set_input_line(M6502_SET_OVERFLOW, CLEAR_LINE);

	m_fdccpu->reset();

	m_riot0->reset();
	m_riot1->reset();
	m_miot->reset();
	m_via->reset();

	m_riot1->pa7_w(0);

	// turn off spindle motors
	m_fdc->mtr0_w(1);
	m_fdc->mtr1_w(1);
}


//-------------------------------------------------
//  ieee488_atn -
//-------------------------------------------------

void c2040_t::ieee488_atn(int state)
{
	update_ieee_signals();

	m_riot1->pa7_w(!state);
}


//-------------------------------------------------
//  ieee488_ifc -
//-------------------------------------------------

void c2040_t::ieee488_ifc(int state)
{
	if (!m_ifc && state)
	{
		device_reset();
	}

	m_ifc = state;
}
