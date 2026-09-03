// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    CMD FD-2000/FD-4000 disk drive emulation

**********************************************************************/

#include "emu.h"
#include "fd2000.h"

#include "formats/d2m_dsk.h"
#include "formats/d81_dsk.h"

#include "fd2000.lh"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define G65SC02PI2_TAG  "maincpu"
#define R65C02P4_TAG    "maincpu"
#define G65SC22P2_TAG   "via"
#define DP8473V_TAG     "fdc"
#define PC8477AV1_TAG   "fdc"
#define DS1216E_TAG     "rtc"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(FD2000, fd2000_device, "fd2000", "FD-2000 Disk Drive")
DEFINE_DEVICE_TYPE(FD4000, fd4000_device, "fd4000", "FD-4000 Disk Drive")


//-------------------------------------------------
//  ROM( fd2000 )
//-------------------------------------------------

ROM_START( fd2000 )
	ROM_REGION( 0x8000, G65SC02PI2_TAG, 0 )
	ROM_DEFAULT_BIOS( "v140" )
	ROM_SYSTEM_BIOS( 0, "v134", "Version 1.34" )
	ROMX_LOAD( "cmd fd-2000 dos v1.34 fd-350026.u2", 0x0000, 0x8000, CRC(859a5edc) SHA1(487fa82a7977e5208d5088f3580f34e8c89560d1), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v140", "Version 1.40" )
	ROMX_LOAD( "cmd fd-2000 dos v1.40 cs 33cc6f.u2", 0x0000, 0x8000, CRC(4e6ca15c) SHA1(0c61ba58269baf2b8aadf3bbc4648c7a5a6d2128), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  ROM( fd4000 )
//-------------------------------------------------

ROM_START( fd4000 )
	ROM_REGION( 0x8000, R65C02P4_TAG, 0 )
	ROM_DEFAULT_BIOS( "v140" )
	ROM_SYSTEM_BIOS( 0, "v134", "Version 1.34" )
	ROMX_LOAD( "cmd fd-4000 dos v1.34 fd-350022.u2", 0x0000, 0x8000, CRC(1f4820c1) SHA1(7a2966662e7840fd9377549727ccba62e4349c6f), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v140", "Version 1.40" )
	ROMX_LOAD( "cmd fd-4000 dos v1.40 fd-350022.u2", 0x0000, 0x8000, CRC(b563ef10) SHA1(d936d76fd8b50ce4c65f885703653d7c1bd7d3c9), ROM_BIOS(1) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *fd2000_device::device_rom_region() const
{
	return ROM_NAME( fd2000 );
}

const tiny_rom_entry *fd4000_device::device_rom_region() const
{
	return ROM_NAME( fd4000 );
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( fd2000 )
	PORT_START("PB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Swap") PORT_WRITE_LINE_DEVICE_MEMBER(G65SC22P2_TAG, FUNC(via6522_device::write_ca1))

	PORT_START("ADDRESS")
	PORT_DIPNAME( 0x0f, 0x00, "Device Address" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x01, "9" )
	PORT_DIPSETTING(    0x02, "10" )
	PORT_DIPSETTING(    0x03, "11" )
	PORT_DIPSETTING(    0x04, "12" )
	PORT_DIPSETTING(    0x05, "13" )
	PORT_DIPSETTING(    0x06, "14" )
	PORT_DIPSETTING(    0x07, "15" )
INPUT_PORTS_END

ioport_constructor fd2000_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( fd2000 );
}


//-------------------------------------------------
//  ADDRESS_MAP( fd2000_mem )
//-------------------------------------------------

void fd2000_device::fd2000_mem(address_map &map)
{
	map(0x0000, 0x3fff).ram();
	map(0x4000, 0x400f).mirror(0xbf0).m(m_via, FUNC(via6522_device::map));
	map(0x4e00, 0x4e07).mirror(0x1f8).m(m_fdc, FUNC(dp8473_device::map));
	map(0x5000, 0x7fff).ram();
	map(0x8000, 0xffff).r(FUNC(fd2000_device::rtc_r));
}


//-------------------------------------------------
//  ADDRESS_MAP( fd4000_mem )
//-------------------------------------------------

void fd4000_device::fd4000_mem(address_map &map)
{
	map(0x0000, 0x3fff).ram();
	map(0x4000, 0x400f).mirror(0xbf0).m(m_via, FUNC(via6522_device::map));
	map(0x4e00, 0x4e07).mirror(0x1f8).m(m_fdc, FUNC(pc8477a_device::map));
	map(0x5000, 0x7fff).ram();
	map(0x8000, 0xffff).r(FUNC(fd4000_device::rtc_r));
}


uint8_t fd2000_device::via_pa_r()
{
	/*

	    bit     description

	    0       DATA IN
	    1
	    2       CLK IN
	    3
	    4
	    5
	    6
	    7       ATN IN

	*/

	uint8_t data = 0;

	data |= !m_bus->data_r();
	data |= !m_bus->clk_r() << 2;
	data |= !m_bus->atn_r() << 7;

	return data;
}

void fd2000_device::via_pa_w(uint8_t data)
{
	/*

	    bit     description

	    0
	    1       DATA OUT
	    2
	    3       CLK OUT
	    4		ATNA
	    5       FAST DIR
	    6
	    7

	*/

	m_iec_data = BIT(data, 1);
	m_iec_clk = BIT(data, 3);
	m_atn_ack = BIT(data, 4);
	m_fst_dir = BIT(data, 5);

	m_iec_sync_timer->adjust(attotime::zero);
}

uint8_t fd2000_device::via_pb_r()
{
	/*

	    bit     description

	    0
	    1
	    2		DIPSW2
	    3		DIPSW4
	    4		DIPSW3
	    5
	    6
	    7       FDC INTRQ

	*/

	int addr = m_slot->get_address() - 8;

	uint8_t data = 0;
	data |= BIT(addr, 2) << 2;
	data |= BIT(addr, 0) << 3;
	data |= BIT(addr, 1) << 4;

	// FDC interrupt
	data |= m_fdc->get_irq() << 7;

	return data;
}

void fd2000_device::via_pb_w(uint8_t data)
{
	/*

	    bit     description

	    0		density select (1=ED/0=HD)
	    1		PWRLED
	    2
	    3
	    4
	    5       ERRLED
	    6       ACTLED
	    7

	*/

	m_leds[LED_PWR] = BIT(data, 1);
	m_leds[LED_ERR] = BIT(data, 5);
	m_leds[LED_ACT] = BIT(data, 6);
}

void fd2000_device::via_cb1_w(int state)
{
	m_fst_clk = state;

	m_iec_sync_timer->adjust(attotime::zero);
}

void fd2000_device::via_cb2_w(int state)
{
	m_fst_data = state;

	m_iec_sync_timer->adjust(attotime::zero);
}

TIMER_CALLBACK_MEMBER(fd2000_device::iec_sync_tick)
{
	m_via->write_cb1(m_fst_dir || m_bus->srq_r());
	m_via->write_cb2(m_fst_dir || m_bus->data_r());

	m_bus->atn_w(this, m_iec_atn);
	m_bus->clk_w(this, !m_iec_clk);

	bool data_out = !m_iec_data && !(m_atn_ack && !m_bus->atn_r());
	if (m_fst_dir)
		data_out &= m_fst_data;
	m_bus->data_w(this, data_out);

	bool srq_out = 1;
	if (m_fst_dir)
		srq_out &= m_fst_clk;
	m_bus->srq_w(this, srq_out);
}

uint8_t fd2000_device::rtc_r(offs_t offset)
{
	if (m_rtc->ceo_r())
		return m_rtc->read(offset);
	else
		m_rtc->read(offset);

	return m_rom->base()[offset];
}

void fd4000_device::mtr0_w(int state)
{
	printf("mtr0_w state: %d\n", state);
	m_maincpu->set_unscaled_clock(XTAL(24'000'000)/(state ? 6 : 12));
}

static void fd2000_floppies(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD); // TEAC FD-235HF
}

static void fd4000_floppies(device_slot_interface &device)
{
	device.option_add("35ed", FLOPPY_35_ED); // TEAC FD-235J
}

void fd2000_device::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_D81_FORMAT);
	fr.add(FLOPPY_D1M_FORMAT);
	fr.add(FLOPPY_D2M_FORMAT);
}

void fd4000_device::floppy_formats(format_registration &fr)
{
	fd2000_device::floppy_formats(fr);
	fr.add(FLOPPY_D4M_FORMAT);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void fd2000_device::add_common_devices(machine_config &config)
{
	W65C02(config, m_maincpu, XTAL(24'000'000)/12);

	R65C22(config, m_via, XTAL(24'000'000)/12);
	m_via->irq_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);
	m_via->readpa_handler().set(FUNC(fd2000_device::via_pa_r));
	m_via->readpb_handler().set(FUNC(fd2000_device::via_pb_r));
	m_via->writepa_handler().set(FUNC(fd2000_device::via_pa_w));
	m_via->writepb_handler().set(FUNC(fd2000_device::via_pb_w));
	m_via->cb1_handler().set(FUNC(fd2000_device::via_cb1_w));
	m_via->cb2_handler().set(FUNC(fd2000_device::via_cb2_w));

	DS1216E(config, m_rtc);

	config.set_default_layout(layout_fd2000);
}

void fd2000_device::device_add_mconfig(machine_config &config)
{
	add_common_devices(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &fd2000_device::fd2000_mem);
	
	DP8473(config, m_fdc, XTAL(24'000'000));

	FLOPPY_CONNECTOR(config, DP8473V_TAG":1", fd2000_floppies, "35hd", fd2000_device::floppy_formats, true).enable_sound(true);
}

void fd4000_device::device_add_mconfig(machine_config &config)
{
	add_common_devices(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &fd4000_device::fd4000_mem);

	PC8477A(config, m_fdc, XTAL(24'000'000));
	m_fdc->mtr0_wr_callback().set(FUNC(fd4000_device::mtr0_w));

	FLOPPY_CONNECTOR(config, PC8477AV1_TAG":1", fd4000_floppies, "35ed", fd4000_device::floppy_formats, true).enable_sound(true);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  fd2000_device - constructor
//-------------------------------------------------

fd2000_device::fd2000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: fd2000_device(mconfig, FD2000, tag, owner, clock)
{
}

fd2000_device::fd2000_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_cbm_iec_interface(mconfig, *this),
	m_maincpu(*this, G65SC02PI2_TAG),
	m_via(*this, G65SC22P2_TAG),
	m_rtc(*this, DS1216E_TAG),
	m_rom(*this, G65SC02PI2_TAG),
	m_fdc(*this, DP8473V_TAG),
	m_leds(*this, "led%u", 0U),
	m_pb(*this, "PB")
{
}


//-------------------------------------------------
//  fd4000_device - constructor
//-------------------------------------------------

fd4000_device::fd4000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: fd2000_device(mconfig, FD4000, tag, owner, clock) { }


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void fd2000_device::device_start()
{
	m_iec_sync_timer = timer_alloc(FUNC(fd2000_device::iec_sync_tick), this);

	// state saving
	save_item(NAME(m_fst_dir));
	save_item(NAME(m_fst_clk));
	save_item(NAME(m_fst_data));
	save_item(NAME(m_atn_ack));
	save_item(NAME(m_iec_atn));
	save_item(NAME(m_iec_clk));
	save_item(NAME(m_iec_data));
}


//-------------------------------------------------
//  cbm_iec_srq -
//-------------------------------------------------

void fd2000_device::cbm_iec_srq(int state)
{
	m_iec_sync_timer->adjust(attotime::zero);
}


//-------------------------------------------------
//  cbm_iec_atn -
//-------------------------------------------------

void fd2000_device::cbm_iec_atn(int state)
{
	m_via->write_ca2(state);

	m_iec_sync_timer->adjust(attotime::zero);
}


//-------------------------------------------------
//  cbm_iec_data -
//-------------------------------------------------

void fd2000_device::cbm_iec_data(int state)
{
	m_iec_sync_timer->adjust(attotime::zero);
}


//-------------------------------------------------
//  cbm_iec_reset -
//-------------------------------------------------

void fd2000_device::cbm_iec_reset(int state)
{
	if (!state)
	{
		reset();
	}
}
