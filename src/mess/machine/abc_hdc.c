/**********************************************************************

    Luxor ABC 850 Winchester controller card emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "abc_hdc.h"
#include "machine/scsibus.h"
#include "machine/scsicb.h"
#include "machine/scsihd.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80_TAG			"z80"
#define SASIBUS_TAG		"sasi"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type ABC_HDC = &device_creator<abc_hdc_device>;


//-------------------------------------------------
//  ROM( abc_hdc )
//-------------------------------------------------

ROM_START( abc_hdc )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	// ABC 850
	ROM_SYSTEM_BIOS(0, "ro202", "Rodime RO202 (CHS: 321,4,17,512)" )
	ROMX_LOAD( "rodi202.bin",  0x0000, 0x0800, CRC(337b4dcf) SHA1(791ebeb4521ddc11fb9742114018e161e1849bdf), ROM_BIOS(1) ) // Rodime RO202 (http://stason.org/TULARC/pc/hard-drives-hdd/rodime/RO202-11MB-5-25-FH-MFM-ST506.html)
	ROM_SYSTEM_BIOS(1, "basf6185", "BASF 6185 (CHS: 440,6,32,256)" )
	ROMX_LOAD( "basf6185.bin", 0x0000, 0x0800, CRC(06f8fe2e) SHA1(e81f2a47c854e0dbb096bee3428d79e63591059d), ROM_BIOS(2) ) // BASF 6185 (http://stason.org/TULARC/pc/hard-drives-hdd/basf-magnetics/6185-22MB-5-25-FH-MFM-ST412.html)
	// ABC 852
	ROM_SYSTEM_BIOS(2, "nec5126", "NEC 5126 (CHS: 615,4,17,512)" )
	ROMX_LOAD( "nec5126.bin",  0x0000, 0x1000, CRC(17c247e7) SHA1(7339738b87751655cb4d6414422593272fe72f5d), ROM_BIOS(3) ) // NEC 5126 (http://stason.org/TULARC/pc/hard-drives-hdd/nec/D5126-20MB-5-25-HH-MFM-ST506.html)
	// ABC 856
	ROM_SYSTEM_BIOS(3, "micr1325", "Micropolis 1325 (CHS: 1024,8,33,256)" )
	ROMX_LOAD( "micr1325.bin", 0x0000, 0x0800, CRC(084af409) SHA1(342b8e214a8c4c2b014604e53c45ef1bd1c69ea3), ROM_BIOS(4) ) // Micropolis 1325 (http://stason.org/TULARC/pc/hard-drives-hdd/micropolis/1325-69MB-5-25-FH-MFM-ST506.html)
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *abc_hdc_device::device_rom_region() const
{
	return ROM_NAME( abc_hdc );
}


//-------------------------------------------------
//  ADDRESS_MAP( abc_hdc_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( abc_hdc_mem, AS_PROGRAM, 8, abc_hdc_device )
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION(Z80_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( abc_hdc_io )
//-------------------------------------------------

static ADDRESS_MAP_START( abc_hdc_io, AS_IO, 8, abc_hdc_device )
ADDRESS_MAP_END


//-------------------------------------------------
//  z80_daisy_config daisy_chain
//-------------------------------------------------

static const z80_daisy_config daisy_chain[] =
{
	{ NULL }
};


//-------------------------------------------------
//  SCSICB_interface sasi_intf
//-------------------------------------------------

static const SCSICB_interface sasi_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  MACHINE_DRIVER( abc_hdc )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( abc_hdc )
	MCFG_CPU_ADD(Z80_TAG, Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(abc_hdc_mem)
	MCFG_CPU_IO_MAP(abc_hdc_io)
	MCFG_CPU_CONFIG(daisy_chain)

	MCFG_SCSIBUS_ADD(SASIBUS_TAG)
	MCFG_SCSIDEV_ADD(SASIBUS_TAG ":harddisk0", SCSIHD, SCSI_ID_0)
	MCFG_SCSICB_ADD(SASIBUS_TAG ":host", sasi_intf)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor abc_hdc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( abc_hdc );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc_hdc_device - constructor
//-------------------------------------------------

abc_hdc_device::abc_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ABC_HDC, "ABC HDC", tag, owner, clock),
	  device_abcbus_card_interface(mconfig, *this),
	  m_maincpu(*this, Z80_TAG),
	  m_sasibus(*this, SASIBUS_TAG)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc_hdc_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void abc_hdc_device::device_reset()
{
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abcbus_cs -
//-------------------------------------------------

void abc_hdc_device::abcbus_cs(UINT8 data)
{
}
