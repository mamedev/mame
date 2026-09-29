// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    PDC Clipper 3" floppy disk drive emulation

    1541 derived DOS with the disk controller replaced by a uPD765A
    formatting 40 tracks of 5 x 1024 byte MFM sectors.

**********************************************************************/

#include "emu.h"
#include "clipper_fdd.h"

#include "formats/clipper_dsk.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6502_TAG       "maincpu"
#define M6522_TAG       "via"
#define UPD765_TAG      "fdc"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CLIPPER_FDD, clipper_fdd_device, "clipper_fdd", "PDC Clipper 3\" Floppy Disk Drive")


//-------------------------------------------------
//  ROM( clipper_fdd )
//-------------------------------------------------

ROM_START( clipper_fdd )
	ROM_REGION( 0x4000, M6502_TAG, 0 )
	ROM_LOAD( "fdc.bin", 0x0000, 0x2000, CRC(44b0b1fc) SHA1(effcf165cb4ea32540a8a8c12781303dc36fa4b2) )
	ROM_LOAD( "fdc_12.bin", 0x2000, 0x2000, CRC(397a2219) SHA1(7eefcc871a805f45be4ba016fe9fc7d25318c431) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *clipper_fdd_device::device_rom_region() const
{
	return ROM_NAME( clipper_fdd );
}


//-------------------------------------------------
//  ADDRESS_MAP( mem_map )
//-------------------------------------------------

void clipper_fdd_device::mem_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1400, 0x1401).mirror(0x03fe).m(m_fdc, FUNC(upd765a_device::map));
	map(0x1800, 0x180f).mirror(0x03f0).m(m_via, FUNC(via6522_device::map));
	map(0x1c00, 0x1c00).mirror(0x03ff).rw(FUNC(clipper_fdd_device::latch_r), FUNC(clipper_fdd_device::latch_w));
	map(0x8000, 0xbfff).rw(FUNC(clipper_fdd_device::tc_r), FUNC(clipper_fdd_device::tc_w));
	map(0xc000, 0xffff).rom().region(M6502_TAG, 0);
}


//-------------------------------------------------
//  via_pb_r -
//-------------------------------------------------

uint8_t clipper_fdd_device::via_pb_r()
{
	/*

	    bit     description

	    PB0     DATA IN
	    PB1
	    PB2     CLK IN
	    PB3
	    PB4
	    PB5     J1
	    PB6     J2
	    PB7     ATN IN

	*/

	uint8_t data = !m_bus->data_r() && !atn_ack();

	data |= !m_bus->clk_r() << 2;
	data |= ((m_slot->get_address() - 8) & 0x03) << 5;
	data |= !m_bus->atn_r() << 7;

	return data;
}


//-------------------------------------------------
//  via_pb_w -
//-------------------------------------------------

void clipper_fdd_device::via_pb_w(uint8_t data)
{
	/*

	    bit     description

	    PB0
	    PB1     DATA OUT
	    PB2
	    PB3     CLK OUT
	    PB4     ATNA
	    PB5
	    PB6
	    PB7

	*/

	m_iec_data = !BIT(data, 1);
	m_iec_clk = !BIT(data, 3);
	m_atna = BIT(data, 4);

	m_iec_sync_timer->adjust(attotime::zero);
}


//-------------------------------------------------
//  iec_sync_tick -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(clipper_fdd_device::iec_sync_tick)
{
	m_via->write_ca1(!m_bus->atn_r());

	m_bus->clk_w(this, m_iec_clk);
	m_bus->data_w(this, m_iec_data && !atn_ack());
}


//-------------------------------------------------
//  latch_r -
//-------------------------------------------------

uint8_t clipper_fdd_device::latch_r()
{
	return m_latch;
}


//-------------------------------------------------
//  latch_w -
//-------------------------------------------------

void clipper_fdd_device::latch_w(uint8_t data)
{
	m_latch = data;

	m_led = BIT(data, 3);
}


//-------------------------------------------------
//  tc_r -
//-------------------------------------------------

uint8_t clipper_fdd_device::tc_r()
{
	if (!machine().side_effects_disabled())
	{
		m_fdc->tc_w(1);
		m_fdc->tc_w(0);
	}

	return 0xff;
}


//-------------------------------------------------
//  tc_w -
//-------------------------------------------------

void clipper_fdd_device::tc_w(uint8_t data)
{
	m_fdc->tc_w(1);
	m_fdc->tc_w(0);
}


//-------------------------------------------------
//  floppy_formats -
//-------------------------------------------------

void clipper_fdd_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_CLIPPER_FORMAT);
}

static void clipper_floppies(device_slot_interface &device)
{
	device.option_add("3ssdd", FLOPPY_3_SSDD);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void clipper_fdd_device::device_add_mconfig(machine_config &config)
{
	M6502(config, m_maincpu, XTAL(16'000'000)/16);
	m_maincpu->set_addrmap(AS_PROGRAM, &clipper_fdd_device::mem_map);

	MOS6522(config, m_via, XTAL(16'000'000)/16);
	m_via->readpb_handler().set(FUNC(clipper_fdd_device::via_pb_r));
	m_via->writepb_handler().set(FUNC(clipper_fdd_device::via_pb_w));
	m_via->irq_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	UPD765A(config, m_fdc, XTAL(8'000'000), true, true);
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, M6502_SET_OVERFLOW);

	FLOPPY_CONNECTOR(config, m_floppy, clipper_floppies, "3ssdd", clipper_fdd_device::floppy_formats, true).enable_sound(true);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  clipper_fdd_device - constructor
//-------------------------------------------------

clipper_fdd_device::clipper_fdd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CLIPPER_FDD, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this),
	m_maincpu(*this, M6502_TAG),
	m_via(*this, M6522_TAG),
	m_fdc(*this, UPD765_TAG),
	m_floppy(*this, UPD765_TAG":0"),
	m_led(*this, "led0"),
	m_iec_clk(0),
	m_iec_data(0),
	m_atna(0),
	m_latch(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void clipper_fdd_device::device_start()
{
	m_iec_sync_timer = timer_alloc(FUNC(clipper_fdd_device::iec_sync_tick), this);
	m_mtr_on_timer = timer_alloc(FUNC(clipper_fdd_device::mtr_on_tick), this);

	save_item(NAME(m_iec_clk));
	save_item(NAME(m_iec_data));
	save_item(NAME(m_atna));
	save_item(NAME(m_latch));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void clipper_fdd_device::device_reset()
{
	m_mtr_on_timer->adjust(attotime::zero);
}


//-------------------------------------------------
//  mtr_on_tick -
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(clipper_fdd_device::mtr_on_tick)
{
	if (m_floppy->get_device())
		m_floppy->get_device()->mon_w(0);
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void clipper_fdd_device::cbm_iec_atn(int state)
{
	m_iec_sync_timer->adjust(attotime::zero);
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void clipper_fdd_device::cbm_iec_reset(int state)
{
	if (!state)
	{
		reset();
	}
}
