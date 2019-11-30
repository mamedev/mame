// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 1581/1563 Single Disk Drive emulation

**********************************************************************/

/*

    TODO:

    - drive not ready if ready_r() is connected to CIA

*/

#include "emu.h"
#include "c1581.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "u1"
#define M8520_TAG       "u5"
#define WD1772_TAG      "u4"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C1563, c1563_device, "c1563", "Commodore 1563 3.5\" Disk Drive")
DEFINE_DEVICE_TYPE(C1581, c1581_device, "c1581", "Commodore 1581 3.5\" Disk Drive")


//-------------------------------------------------
//  ROM( c1581 )
//-------------------------------------------------

ROM_START( c1581 )
	ROM_REGION( 0x8000, M6502_TAG, 0 )
	ROM_DEFAULT_BIOS("r1")
	ROM_SYSTEM_BIOS( 0, "beta", "Beta" )
	ROMX_LOAD( "beta.u2",          0x0000, 0x8000, CRC(ecc223cd) SHA1(a331d0d46ead1f0275b4ca594f87c6694d9d9594), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "r1", "Revision 1" )
	ROMX_LOAD( "318045-01.u2",     0x0000, 0x8000, CRC(113af078) SHA1(3fc088349ab83e8f5948b7670c866a3c954e6164), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "r2", "Revision 2" )
	ROMX_LOAD( "318045-02.u2",     0x0000, 0x8000, CRC(a9011b84) SHA1(01228eae6f066bd9b7b2b6a7fa3f667e41dad393), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "jiffydos", "JiffyDOS v6.01" )
	ROMX_LOAD( "jiffydos 1581.u2", 0x0000, 0x8000, CRC(98873d0f) SHA1(65bbf2be7bcd5bdcbff609d6c66471ffb9d04bfe), ROM_BIOS(3) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c1581_device::device_rom_region() const
{
	return ROM_NAME( c1581 );
}


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

const tiny_rom_entry *c1563_device::device_rom_region() const
{
	return ROM_NAME( c1563 );
}


//-------------------------------------------------
//  ADDRESS_MAP( c1581_mem )
//-------------------------------------------------

void c1581_device::c1581_mem(address_map &map)
{
	map(0x0000, 0x1fff).mirror(0x2000).ram();
	map(0x4000, 0x400f).mirror(0x1ff0).rw(M8520_TAG, FUNC(mos8520_device::read), FUNC(mos8520_device::write));
	map(0x6000, 0x6003).mirror(0x1ffc).rw(WD1772_TAG, FUNC(wd1772_device::read), FUNC(wd1772_device::write));
	map(0x8000, 0xffff).rom().region(M6502_TAG, 0);
}


//-------------------------------------------------
//  MOS8520_INTERFACE( cia_intf )
//-------------------------------------------------

WRITE_LINE_MEMBER( c1581_device::cnt_w )
{
	m_cnt_out = state;

	update_iec();
}

WRITE_LINE_MEMBER( c1581_device::sp_w )
{
	m_sp_out = state;

	update_iec();
}

READ8_MEMBER( c1581_device::cia_pa_r )
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

	uint8_t data = 0;

	// ready
	//data |= !m_floppy->ready_r() << 1;

	// device number
	data |= ((m_slot->get_address() - 8) & 0x03) << 3;

	// disk change
	data |= m_floppy->dskchg_r() << 7;

	return data;
}

WRITE8_MEMBER( c1581_device::cia_pa_w )
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
	m_leds[LED_POWER] = BIT(data, 5);

	// activity led
	m_leds[LED_ACT] = BIT(data, 6);
}

READ8_MEMBER( c1581_device::cia_pb_r )
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

	uint8_t data;

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

WRITE8_MEMBER( c1581_device::cia_pb_w )
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


//-------------------------------------------------
//  SLOT_INTERFACE( c1581_floppies )
//-------------------------------------------------

static void c1581_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD); // Chinon F-354-E
}


//-------------------------------------------------
//  FLOPPY_FORMATS( c1581_device::floppy_formats )
//-------------------------------------------------

FLOPPY_FORMATS_MEMBER( c1581_device::floppy_formats )
	FLOPPY_D81_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c1581_device::device_add_mconfig(machine_config &config)
{
	M6502(config, m_maincpu, 16_MHz_XTAL / 8);
	m_maincpu->set_addrmap(AS_PROGRAM, &c1581_device::c1581_mem);

	MOS8520(config, m_cia, 16_MHz_XTAL / 8);
	m_cia->irq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_cia->cnt_wr_callback().set(FUNC(c1581_device::cnt_w));
	m_cia->sp_wr_callback().set(FUNC(c1581_device::sp_w));
	m_cia->pa_rd_callback().set(FUNC(c1581_device::cia_pa_r));
	m_cia->pa_wr_callback().set(FUNC(c1581_device::cia_pa_w));
	m_cia->pb_rd_callback().set(FUNC(c1581_device::cia_pb_r));
	m_cia->pb_wr_callback().set(FUNC(c1581_device::cia_pb_w));

	WD1772(config, m_fdc, 16_MHz_XTAL / 2);
	FLOPPY_CONNECTOR(config, WD1772_TAG":0", c1581_floppies, "35dd", c1581_device::floppy_formats, true);
}


//-------------------------------------------------
//  INPUT_PORTS( c1581 )
//-------------------------------------------------

static INPUT_PORTS_START( c1581 )
	PORT_START("ADDRESS")
	PORT_DIPNAME( 0x03, 0x00, "Device Address" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x01, "9" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x03, "11" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c1581_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( c1581 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c1581_device - constructor
//-------------------------------------------------

c1581_device::c1581_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
		device_cbm_iec_interface(mconfig, *this),
		m_maincpu(*this, M6502_TAG),
		m_cia(*this, M8520_TAG),
		m_fdc(*this, WD1772_TAG),
		m_floppy(*this, WD1772_TAG":0:35dd"),
		m_address(*this, "ADDRESS"),
		m_leds(*this, "led%u", 0U),
		m_data_out(0),
		m_atn_ack(0),
		m_fast_ser_dir(0),
		m_sp_out(1),
		m_cnt_out(1)
{
}

c1581_device::c1581_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: c1581_device(mconfig, C1581, tag, owner, clock)
{
}


//-------------------------------------------------
//  c1563_device - constructor
//-------------------------------------------------

c1563_device::c1563_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: c1581_device(mconfig, C1563, tag, owner, clock) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c1581_device::device_start()
{
	m_leds.resolve();

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

void c1581_device::device_reset()
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

void c1581_device::cbm_iec_srq(int state)
{
	update_iec();
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void c1581_device::cbm_iec_atn(int state)
{
	update_iec();
}


//-------------------------------------------------
//  cbm_iec_data -
//-------------------------------------------------

void c1581_device::cbm_iec_data(int state)
{
	update_iec();
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void c1581_device::cbm_iec_reset(int state)
{
	if (!state)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  update_iec -
//-------------------------------------------------

void c1581_device::update_iec()
{
	m_cia->cnt_w(m_fast_ser_dir || m_bus->srq_r());
	m_cia->sp_w(m_fast_ser_dir || m_bus->data_r());

	int atn = m_bus->atn_r();
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
