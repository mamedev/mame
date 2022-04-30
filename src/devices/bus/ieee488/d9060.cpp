// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 9060/9090 Hard Disk Drive emulation

**********************************************************************/

/*

    Use the CHDMAN utility to create a 5MB image for D9060:

    $ chdman createhd -o tm602s.chd -chs 153,4,32 -ss 256

    or a 10MB image for D9090:

    $ chdman createhd -o tm603s.chd -chs 153,6,32 -ss 256

    Start the PET emulator with the D9060 attached on the IEEE-488 bus,
    with the new CHD mounted:

    $ mame pet8032 -ieee8 d9060 -hard tm602s.chd
    $ mame pet8032 -ieee8 d9090 -hard tm603s.chd

    Enter 'HEADER "LABEL",D0,I01' to format the hard drive.
    Wait up to 1 hour and 20 minutes.

*/

#include "emu.h"
#include "d9060.h"
#include "bus/scsi/d9060hd.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_DOS_TAG   "7e"
#define M6532_0_TAG     "7f"
#define M6532_1_TAG     "7g"

#define M6502_HDC_TAG   "4a"
#define M6522_TAG       "4b"

#define AM2910_TAG      "9d"

#define SASIBUS_TAG     "sasi"

enum
{
	LED_POWER = 0,
	LED_READY,
	LED_ERROR
};



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(D9060, d9060_device, "d9060", "Commodore D9060")
DEFINE_DEVICE_TYPE(D9090, d9090_device, "d9090", "Commodore D9090")


//-------------------------------------------------
//  ROM( d9060 )
//-------------------------------------------------

ROM_START( d9060 )
	ROM_REGION( 0x4000, M6502_DOS_TAG, 0 )
	ROM_DEFAULT_BIOS("rc")
	ROM_SYSTEM_BIOS( 0, "ra", "Revision A" )
	ROMX_LOAD( "300516-001.7c", 0x0000, 0x2000, NO_DUMP, ROM_BIOS(0) )
	ROMX_LOAD( "300517-001.7d", 0x2000, 0x2000, CRC(566df630) SHA1(b1602dfff408b165ee52a6a4ca3e2ec27e689ba9), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "rb", "Revision B" )
	ROMX_LOAD( "300516-002.7c", 0x0000, 0x2000, CRC(2d758a14) SHA1(c959cc9dde84fc3d64e95e58a0a096a26d8107fd), ROM_BIOS(1) )
	ROMX_LOAD( "300517-002.7d", 0x2000, 0x2000, CRC(f0382bc3) SHA1(0b0a8dc520f5b41ffa832e4a636b3d226ccbb7f1), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "rc", "Revision C" )
	ROMX_LOAD( "300516-003.7c", 0x0000, 0x2000, CRC(d6a3e88f) SHA1(bb1ddb5da94a86266012eca54818aa21dc4cef6a), ROM_BIOS(2) )
	ROMX_LOAD( "300517-003.7d", 0x2000, 0x2000, CRC(2a9ad4ad) SHA1(4c17d014de48c906871b9b6c7d037d8736b1fd52), ROM_BIOS(2) )

	ROM_REGION( 0x800, M6502_HDC_TAG, 0 )
	ROM_LOAD( "300515-001.4c", 0x000, 0x800, CRC(99e096f7) SHA1(a3d1deb27bf5918b62b89c27fa3e488eb8f717a4) ) // Revision A
	ROM_LOAD( "300515-002.4c", 0x000, 0x800, CRC(49adf4fb) SHA1(59dafbd4855083074ba8dc96a04d4daa5b76e0d6) ) // Revision B

	ROM_REGION( 0x1400, AM2910_TAG, 0 )
	ROM_LOAD( "44_1.5b", 0x0000, 0x0400, CRC(67a49dd3) SHA1(be39f55bc0ff9a508ec55224c52549856ad3270a) ) // 82S137
	ROM_LOAD( "44_2.6b", 0x0400, 0x0400, CRC(f6d1bdbc) SHA1(bca7a96a60144c36eff1124485ec26df7cfa8143) ) // 82S137
	ROM_LOAD( "44_3.7b", 0x0800, 0x0400, CRC(68af3a1f) SHA1(d3823a0d23e828a2dff6d89211fbcd7034112298) ) // 82S137
	ROM_LOAD( "44_4.8b", 0x0c00, 0x0400, CRC(a767b1dc) SHA1(fff11b662c74040981822ccdcc8cbd0e22b517a9) ) // 82S137
	ROM_LOAD( "44_5.9b", 0x1000, 0x0400, CRC(85743073) SHA1(95a48835bc2bd1c79fb1a63778de3807fb731ac6) ) // 82S137
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *d9060_device_base::device_rom_region() const
{
	return ROM_NAME( d9060 );
}


void d9060_device_base::main_mem(address_map &map)
{
	map(0x0000, 0x007f).mirror(0x0100).m(m_riot0, FUNC(mos6532_new_device::ram_map));
	map(0x0080, 0x00ff).mirror(0x0100).m(m_riot1, FUNC(mos6532_new_device::ram_map));
	map(0x0200, 0x021f).mirror(0x0d60).m(m_riot0, FUNC(mos6532_new_device::io_map));
	map(0x0280, 0x029f).mirror(0x0d60).m(m_riot1, FUNC(mos6532_new_device::io_map));
	map(0x1000, 0x13ff).mirror(0x0c00).ram().share("share1");
	map(0x2000, 0x23ff).mirror(0x0c00).ram().share("share2");
	map(0x3000, 0x33ff).mirror(0x0c00).ram().share("share3");
	map(0x4000, 0x43ff).mirror(0x0c00).ram().share("share4");
	map(0xc000, 0xffff).rom().region(M6502_DOS_TAG, 0);
}


void d9060_device_base::hdc_mem(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x007f).mirror(0x300).ram();
	map(0x0080, 0x008f).mirror(0x370).m(m_via, FUNC(via6522_device::map));
	map(0x0400, 0x07ff).ram().share("share1");
	map(0x0800, 0x0bff).ram().share("share2");
	map(0x0c00, 0x0fff).ram().share("share3");
	map(0x1000, 0x13ff).ram().share("share4");
	map(0x1800, 0x1fff).rom().region(M6502_HDC_TAG, 0);
}


//-------------------------------------------------
//  riot6532 0
//-------------------------------------------------

uint8_t d9060_device_base::dio_r()
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


void d9060_device_base::dio_w(uint8_t data)
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
//  riot6532 1
//-------------------------------------------------

uint8_t d9060_device_base::riot1_pa_r()
{
	/*

	    bit     description

	    PA0
	    PA1
	    PA2
	    PA3
	    PA4
	    PA5     EOII
	    PA6     DAVI
	    PA7     _ATN

	*/

	uint8_t data = 0;

	// end or identify in
	data |= m_bus->eoi_r() << 5;

	// data valid in
	data |= m_bus->dav_r() << 6;

	// attention
	data |= !m_bus->atn_r() << 7;

	return data;
}

void d9060_device_base::riot1_pa_w(uint8_t data)
{
	/*

	    bit     description

	    PA0     ATNA
	    PA1     DACO
	    PA2     RFDO
	    PA3     EOIO
	    PA4     DAVO
	    PA5
	    PA6
	    PA7

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

uint8_t d9060_device_base::riot1_pb_r()
{
	/*

	    bit     description

	    PB0     device #
	    PB1     device #
	    PB2     device #
	    PB3
	    PB4
	    PB5
	    PB6     DACI
	    PB7     RFDI

	*/

	uint8_t data = 0;

	// device number selection
	data |= m_slot->get_address() - 8;

	// data accepted in
	data |= m_bus->ndac_r() << 6;

	// ready for data in
	data |= m_bus->nrfd_r() << 7;

	return data;
}

void d9060_device_base::riot1_pb_w(uint8_t data)
{
	/*

	    bit     description

	    PB0
	    PB1
	    PB2
	    PB3
	    PB4     DRIVE RDY
	    PB5     PWR ON AND NO ERRORS
	    PB6
	    PB7

	*/

	// ready led
	m_leds[LED_READY] = BIT(data, 4);

	// power led
	m_leds[LED_POWER] = BIT(data, 5);

	// error led
	m_leds[LED_ERROR] = !BIT(data, 5);
}


void d9060_device_base::via_pb_w(uint8_t data)
{
	/*

	    bit     description

	    PB0     SEL
	    PB1     RST
	    PB2     C/D
	    PB3     BUSY
	    PB4     J14 (1=9060, 0=9090)
	    PB5     J13
	    PB6     I/O
	    PB7     MSG

	*/

	m_sasibus->write_sel(BIT(data, 0));
	m_sasibus->write_rst(BIT(data, 1));
}

WRITE_LINE_MEMBER( d9060_device_base::ack_w )
{
	m_sasibus->write_ack(!state);
}

WRITE_LINE_MEMBER( d9060_device_base::enable_w )
{
	m_enable = state;

	if( !m_enable )
	{
		m_sasi_data_out->write( m_data );
	}
	else
	{
		m_sasi_data_out->write( 0 );
	}
}

void d9060_device_base::scsi_data_w(uint8_t data)
{
	m_data = data;

	if( !m_enable )
	{
		m_sasi_data_out->write( m_data );
	}
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void d9060_device_base::device_add_mconfig(machine_config &config)
{
	// DOS
	M6502(config, m_maincpu, XTAL(4'000'000)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &d9060_device::main_mem);

	MOS6532_NEW(config, m_riot0, XTAL(4'000'000)/4);
	m_riot0->pa_rd_callback().set(FUNC(d9060_device_base::dio_r));
	m_riot0->pb_wr_callback().set(FUNC(d9060_device_base::dio_w));

	MOS6532_NEW(config, m_riot1, XTAL(4'000'000)/4);
	m_riot1->pa_rd_callback().set(FUNC(d9060_device_base::riot1_pa_r));
	m_riot1->pa_wr_callback().set(FUNC(d9060_device_base::riot1_pa_w));
	m_riot1->pb_rd_callback().set(FUNC(d9060_device_base::riot1_pb_r));
	m_riot1->pb_wr_callback().set(FUNC(d9060_device_base::riot1_pb_w));
	m_riot1->irq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	// controller
	M6502(config, m_hdccpu, XTAL(4'000'000)/4);
	m_hdccpu->set_addrmap(AS_PROGRAM, &d9060_device::hdc_mem);

	MOS6522(config, m_via, XTAL(4'000'000)/4);
	m_via->writepa_handler().set(FUNC(d9060_device_base::scsi_data_w));
	m_via->writepb_handler().set(FUNC(d9060_device_base::via_pb_w));
	m_via->ca2_handler().set(FUNC(d9060_device_base::ack_w));
	m_via->cb2_handler().set(FUNC(d9060_device_base::enable_w));
	m_via->irq_handler().set_inputline(m_hdccpu, M6502_IRQ_LINE);

	SCSI_PORT(config, m_sasibus);
	m_sasibus->req_handler().set(M6522_TAG, FUNC(via6522_device::write_ca1));
	m_sasibus->cd_handler().set(M6522_TAG, FUNC(via6522_device::write_pb2));
	m_sasibus->bsy_handler().set(M6522_TAG, FUNC(via6522_device::write_pb3));
	m_sasibus->io_handler().set(M6522_TAG, FUNC(via6522_device::write_pb6));
	m_sasibus->msg_handler().set(M6522_TAG, FUNC(via6522_device::write_pb7));
	m_sasibus->data0_handler().set(M6522_TAG, FUNC(via6522_device::write_pa0));
	m_sasibus->data1_handler().set(M6522_TAG, FUNC(via6522_device::write_pa1));
	m_sasibus->data2_handler().set(M6522_TAG, FUNC(via6522_device::write_pa2));
	m_sasibus->data3_handler().set(M6522_TAG, FUNC(via6522_device::write_pa3));
	m_sasibus->data4_handler().set(M6522_TAG, FUNC(via6522_device::write_pa4));
	m_sasibus->data5_handler().set(M6522_TAG, FUNC(via6522_device::write_pa5));
	m_sasibus->data6_handler().set(M6522_TAG, FUNC(via6522_device::write_pa6));
	m_sasibus->data7_handler().set(M6522_TAG, FUNC(via6522_device::write_pa7));

	OUTPUT_LATCH(config, m_sasi_data_out);
	m_sasibus->set_output_latch(*m_sasi_data_out);

	m_sasibus->set_slot_device(1, "harddisk", D9060HD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_0));
}


//-------------------------------------------------
//  INPUT_PORTS( d9060 )
//-------------------------------------------------

static INPUT_PORTS_START( d9060 )
	PORT_START("ADDRESS")
	PORT_DIPNAME( 0x07, 0x01, "Device Address" )
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

ioport_constructor d9060_device_base::device_input_ports() const
{
	return INPUT_PORTS_NAME( d9060 );
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  update_ieee_signals -
//-------------------------------------------------

inline void d9060_device_base::update_ieee_signals()
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
//  d9060_device_base - constructor
//-------------------------------------------------

d9060_device_base::d9060_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint32_t variant)
	: device_t(mconfig, type, tag, owner, clock)
	, device_ieee488_interface(mconfig, *this)
	, m_maincpu(*this, M6502_DOS_TAG)
	, m_hdccpu(*this, M6502_HDC_TAG)
	, m_riot0(*this, M6532_0_TAG)
	, m_riot1(*this, M6532_1_TAG)
	, m_via(*this, M6522_TAG)
	, m_sasibus(*this, SASIBUS_TAG)
	, m_sasi_data_out(*this, "sasi_data_out")
	, m_address(*this, "ADDRESS")
	, m_leds(*this, "led%u", 0U)
	, m_rfdo(1)
	, m_daco(1)
	, m_atna(1)
	, m_ifc(0)
	, m_enable(0)
	, m_data(0)
	, m_variant(variant)
{
}


//-------------------------------------------------
//  d9060_device - constructor
//-------------------------------------------------

d9060_device::d9060_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: d9060_device_base(mconfig, D9060, tag, owner, clock, TYPE_9060)
{
}


//-------------------------------------------------
//  d9090_device - constructor
//-------------------------------------------------

d9090_device::d9090_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: d9060_device_base(mconfig, D9090, tag, owner, clock, TYPE_9090)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void d9060_device_base::device_start()
{
	m_leds.resolve();

	// state saving
	save_item(NAME(m_rfdo));
	save_item(NAME(m_daco));
	save_item(NAME(m_atna));
	save_item(NAME(m_enable));

	m_via->write_pb4(!(m_variant == TYPE_9090)); // J14 (6 HEADS)
	m_via->write_pb5(!(m_variant == TYPE_9060)); // J13 (4 HEADS)
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void d9060_device_base::device_reset()
{
	m_maincpu->set_input_line(M6502_SET_OVERFLOW, ASSERT_LINE);
	m_maincpu->set_input_line(M6502_SET_OVERFLOW, CLEAR_LINE);

	m_hdccpu->set_input_line(M6502_SET_OVERFLOW, ASSERT_LINE);

	m_riot1->pa7_w(1);
}


//-------------------------------------------------
//  ieee488_atn - attention
//-------------------------------------------------

void d9060_device_base::ieee488_atn(int state)
{
	update_ieee_signals();

	m_riot1->pa7_w(state);
}


//-------------------------------------------------
//  ieee488_ifc - interface clear
//-------------------------------------------------

void d9060_device_base::ieee488_ifc(int state)
{
	if (!m_ifc && state)
	{
		device_reset();
	}

	m_ifc = state;
}
