/**********************************************************************

    Commodore 1581/1563 Single Disk Drive emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

/*
	
	TODO:

	- wd_fdc drops the busy bit too soon, c1581 aborts reading the sector ID after the first CRC byte @CD17

*/

#include "c1581.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "u1"
#define M8520_TAG       "u5"
#define WD1772_TAG      "u4"


enum
{
	LED_POWER = 0,
	LED_ACT
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type C1563 = &device_creator<c1563_device>;
const device_type C1581 = &device_creator<c1581_device>;


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void base_c1581_device::device_config_complete()
{
	switch (m_variant)
	{
	default:
	case TYPE_1581:
		m_shortname = "c1581";
		break;

	case TYPE_1563:
		m_shortname = "c1563";
		break;
	}
}


//-------------------------------------------------
//  ROM( c1581 )
//-------------------------------------------------

ROM_START( c1581 )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS("r1")
	ROM_SYSTEM_BIOS( 0, "beta", "Beta" )
	ROMX_LOAD( "beta.u2",          0x0000, 0x8000, CRC(ecc223cd) SHA1(a331d0d46ead1f0275b4ca594f87c6694d9d9594), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "r1", "Revision 1" )
	ROMX_LOAD( "318045-01.u2",     0x0000, 0x8000, CRC(113af078) SHA1(3fc088349ab83e8f5948b7670c866a3c954e6164), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "r2", "Revision 2" )
	ROMX_LOAD( "318045-02.u2",     0x0000, 0x8000, CRC(a9011b84) SHA1(01228eae6f066bd9b7b2b6a7fa3f667e41dad393), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos 1581.u2", 0x0000, 0x8000, CRC(98873d0f) SHA1(65bbf2be7bcd5bdcbff609d6c66471ffb9d04bfe), ROM_BIOS(4) )
ROM_END


//-------------------------------------------------
//  ROM( c1563 )
//-------------------------------------------------

ROM_START( c1563 )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_LOAD( "1563-rom.bin", 0x0000, 0x8000, CRC(1d184687) SHA1(2c5111a9c15be7b7955f6c8775fea25ec10c0ca0) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *base_c1581_device::device_rom_region() const
{
	switch (m_variant)
	{
	default:
	case TYPE_1581:
		return ROM_NAME( c1581 );

	case TYPE_1563:
		return ROM_NAME( c1563 );
	}
}


//-------------------------------------------------
//  ADDRESS_MAP( c1581_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( c1581_mem, AS_PROGRAM, 8, base_c1581_device )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x4000, 0x400f) AM_MIRROR(0x1ff0) AM_DEVREADWRITE(M8520_TAG, mos8520_device, read, write)
	AM_RANGE(0x6000, 0x6003) AM_MIRROR(0x1ffc) AM_DEVREADWRITE(WD1772_TAG, wd1772_t, read, write)
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION(M6502_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  MOS8520_INTERFACE( cia_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( base_c1581_device::cnt_w )
{
	m_cnt_out = state;

	update_iec();
}

WRITE_LINE_MEMBER( base_c1581_device::sp_w )
{
	m_sp_out = state;

	update_iec();
}

READ8_MEMBER( base_c1581_device::cia_pa_r )
{
	/*

	    bit     description

	    PA0
	    PA1     /RDY
	    PA2
	    PA3     DEV# SEL (SW1)
	    PA4     DEV# SEL (SW1)
	    PA5
	    PA6
	    PA7     /DISK CHNG

	*/

	UINT8 data = 0;

	// ready
	data |= !m_floppy->ready_r() << 1;

	// device number
	data |= (m_address - 8) << 3;

	// disk change
	data |= m_floppy->dskchg_r() << 7;

	return data;
}

WRITE8_MEMBER( base_c1581_device::cia_pa_w )
{
	/*

	    bit     description

	    PA0     SIDE0
	    PA1
	    PA2     /MOTOR
	    PA3
	    PA4
	    PA5     POWER LED
	    PA6     ACT LED
	    PA7

	*/

	// side select
	m_floppy->ss_w(BIT(data, 0));

	// motor
	m_floppy->mon_w(BIT(data, 2));

	// power led
	output_set_led_value(LED_POWER, BIT(data, 5));

	// activity led
	output_set_led_value(LED_ACT, BIT(data, 6));
}

READ8_MEMBER( base_c1581_device::cia_pb_r )
{
	/*

	    bit     description

	    PB0     DATA IN
	    PB1
	    PB2     CLK IN
	    PB3
	    PB4
	    PB5
	    PB6     /WPRT
	    PB7     ATN IN

	*/

	UINT8 data = 0;

	// data in
	data = !m_bus->data_r();

	// clock in
	data |= !m_bus->clk_r() << 2;

	// write protect
	data |= !m_floppy->wpt_r() << 6;

	// attention in
	data |= !m_bus->atn_r() << 7;

	return data;
}

WRITE8_MEMBER( base_c1581_device::cia_pb_w )
{
	/*

	    bit     description

	    PB0
	    PB1     DATA OUT
	    PB2
	    PB3     CLK OUT
	    PB4     ATN ACK
	    PB5     FAST SER DIR
	    PB6
	    PB7

	*/

	// data out
	m_data_out = BIT(data, 1);

	// clock out
	m_bus->clk_w(this, !BIT(data, 3));

	// attention acknowledge
	m_atn_ack = BIT(data, 4);

	// fast serial direction
	m_fast_ser_dir = BIT(data, 5);

	update_iec();
}

static MOS8520_INTERFACE( cia_intf )
{
	DEVCB_CPU_INPUT_LINE(M6502_TAG, INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c1581_device, cnt_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, base_c1581_device, sp_w),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c1581_device, cia_pa_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c1581_device, cia_pa_w),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c1581_device, cia_pb_r),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, base_c1581_device, cia_pb_w)
};


//-------------------------------------------------
//  SLOT_INTERFACE( c1581_floppies )
//-------------------------------------------------

static SLOT_INTERFACE_START( c1581_floppies )
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD ) // Chinon F-354-E
SLOT_INTERFACE_END


//-------------------------------------------------
//  FLOPPY_FORMATS( c1581_device::floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( base_c1581_device::floppy_formats )
	FLOPPY_D81_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  MACHINE_DRIVER( c1581 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( c1581 )
	MCFG_CPU_ADD(M6502_TAG, M6502, XTAL_16MHz/8)
	MCFG_CPU_PROGRAM_MAP(c1581_mem)

	MCFG_MOS8520_ADD(M8520_TAG, XTAL_16MHz/8, 0, cia_intf)
	MCFG_WD1772x_ADD(WD1772_TAG, XTAL_16MHz/2)

	MCFG_FLOPPY_DRIVE_ADD(WD1772_TAG":0", c1581_floppies, "35dd", 0, base_c1581_device::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor base_c1581_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( c1581 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  base_c1581_device - constructor
//-------------------------------------------------

base_c1581_device::base_c1581_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant)
	: device_t(mconfig, type, name, tag, owner, clock),
		device_cbm_iec_interface(mconfig, *this),
		m_maincpu(*this, M6502_TAG),
		m_cia(*this, M8520_TAG),
		m_fdc(*this, WD1772_TAG),
		m_floppy(*this, WD1772_TAG":0:35dd"),
		m_data_out(0),
		m_atn_ack(0),
		m_fast_ser_dir(0),
		m_sp_out(1),
		m_cnt_out(1),
		m_variant(variant)
{
}


//-------------------------------------------------
//  c1563_device - constructor
//-------------------------------------------------

c1563_device::c1563_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: base_c1581_device(mconfig, C1563, "C1563", tag, owner, clock, TYPE_1563) { }


//-------------------------------------------------
//  c1581_device - constructor
//-------------------------------------------------

c1581_device::c1581_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: base_c1581_device(mconfig, C1581, "C1581", tag, owner, clock, TYPE_1581) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void base_c1581_device::device_start()
{
	// state saving
	save_item(NAME(m_data_out));
	save_item(NAME(m_atn_ack));
	save_item(NAME(m_fast_ser_dir));
	save_item(NAME(m_sp_out));
	save_item(NAME(m_cnt_out));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void base_c1581_device::device_reset()
{
	m_maincpu->reset();

	m_cia->reset();
	m_fdc->reset();

	m_fdc->set_floppy(m_floppy);
	m_fdc->dden_w(0);

	m_sp_out = 1;
	m_cnt_out = 1;

	update_iec();
}


//-------------------------------------------------
//  cbm_iec_srq -
//-------------------------------------------------

void base_c1581_device::cbm_iec_srq(int state)
{
	update_iec();
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void base_c1581_device::cbm_iec_atn(int state)
{
	update_iec();
}


//-------------------------------------------------
//  cbm_iec_data -
//-------------------------------------------------

void base_c1581_device::cbm_iec_data(int state)
{
	update_iec();
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void base_c1581_device::cbm_iec_reset(int state)
{
	if (!state)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  update_iec -
//-------------------------------------------------

void base_c1581_device::update_iec()
{
	int atn = m_bus->atn_r();

	m_cia->cnt_w(m_fast_ser_dir || m_bus->srq_r());
	m_cia->sp_w(m_fast_ser_dir || m_bus->data_r());
	m_cia->flag_w(atn);

	// serial data
	int data = !m_data_out && !(m_atn_ack && !atn);

	if (m_fast_ser_dir) data &= m_sp_out;

	m_bus->data_w(this, data);

	// fast clock
	int srq = 1;

	if (m_fast_ser_dir) srq &= m_cnt_out;

	m_bus->srq_w(this, srq);
}
