// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam floppy disk controller emulation

**********************************************************************/

/*

    Note: the 3.5" controllers are not compatible with 5.25" drives,
    and vice versa, due to differences in disk geometry and sector
    interleaving.

    640KB DSQD 5.25" disks are rumored to have also been used on the
    Adam, but no images of these are confirmed to exist and they are
    not supported by any extant FDC variant.


                              5.25"                   3.5"
                        160KB       320KB       720KB       1.44MB
    Device shortname    (SSDD)      (DSDD)      (DSDD)      (DSHD)

    adam_fdc            Yes         No          No          No
    adam_fdc_320kb      Yes         Yes         No          No
    adam_fdc_a720dipi   No          No          Yes         No
    adam_fdc_fp720at    No          No          Yes         No
    adam_fdc_mihddd     No          No          Yes         Yes

*/

#include "emu.h"
#include "fdc.h"

#include "machine/clock.h"

#include "formats/adam_dsk.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6801_TAG       "u6"
#define WD2793_TAG      "u11"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ADAM_FDC, adam_fdc_device, "adam_fdc", "Adam FDC")
DEFINE_DEVICE_TYPE(ADAM_FDC_320KB, adam_fdc_320kb_device, "adam_fdc_320kb", "Adam FDC (320KB DSDD modification)")
DEFINE_DEVICE_TYPE(ADAM_FDC_A720DIPI, adam_fdc_a720dipi_device, "adam_fdc_a720dipi", "Adam FDC (720KB 3.5\" A720DIPI)")
DEFINE_DEVICE_TYPE(ADAM_FDC_FP720AT, adam_fdc_fp720at_device, "adam_fdc_fp720at", "Adam FDC (720KB 3.5\" FastPack 720A(T))")
DEFINE_DEVICE_TYPE(ADAM_FDC_MIHDDD, adam_fdc_mihddd_device, "adam_fdc_mihddd", "Adam FDC (1.44MB 3.5\" Micro Innovations HD-DD)")


//-------------------------------------------------
//  ROM( adam_fdc )
//-------------------------------------------------

ROM_START( adam_fdc )
	ROM_REGION( 0x1000, M6801_TAG, 0 )
	//ROM_SYSTEM_BIOS( n, "ssdd", "Coleco 160KB SSDD" )
	ROM_LOAD( "adam disk u10 ad 31 rev a 09-27-84.u10", 0x0000, 0x1000, CRC(4b0b7143) SHA1(1cb68891c3af80e99efad7e309136ca37244f060) )
ROM_END


//-------------------------------------------------
//  ROM( adam_fdc_320kb )
//-------------------------------------------------

ROM_START( adam_fdc_320kb )
	ROM_REGION( 0x1000, M6801_TAG, 0 )
	ROM_DEFAULT_BIOS( "320ta" )
	ROM_SYSTEM_BIOS( 0, "320ta", "Minh Ta?" )
	ROMX_LOAD( "320ta.u10", 0x0000, 0x1000, CRC(dcd865b3) SHA1(dde583e0d18ce4406e9ea44ab34d083e73ee30e2), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "dbl24", "DBL" )
	ROMX_LOAD( "dbl2-4.u10", 0x0000, 0x1000, CRC(5df49f15) SHA1(43d5710e4fb05f520e813869a049585b41ada86b), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "pmhd", "PMHD" ) // formerly listed as 1.44MB 3.5" Powermate High Density but actually a bugfixed version of "dbl24"
	ROMX_LOAD( "pmhdfdc.u10", 0x0000, 0x1000, CRC(fed4006c) SHA1(bc8dd00dd5cde9500a4cd7dc1e4d74330184472a), ROM_BIOS(2) )
ROM_END


//-------------------------------------------------
//  ROM( adam_fdc_a720dipi )
//-------------------------------------------------

ROM_START( adam_fdc_a720dipi )
	ROM_REGION( 0x1000, M6801_TAG, 0 )
	//ROM_SYSTEM_BIOS( n, "a720dipi", "720KB 3.5\" A720DIPI 7607 MMSG" )
	ROM_LOAD( "a720dipi 7607 mmsg =c= 1988.u10", 0x0000, 0x1000, CRC(5f248557) SHA1(15b3aaebba38af84f6a1a6ccdf840ca3d58635da) )
ROM_END


//-------------------------------------------------
//  ROM( adam_fdc_fp720at )
//-------------------------------------------------

ROM_START( adam_fdc_fp720at )
	ROM_REGION( 0x1000, M6801_TAG, 0 )
	//ROM_SYSTEM_BIOS( n, "fp720at", "720KB 3.5\" FastPack 720A(T)" )
	ROM_LOAD( "fastpack 720a,t.u10", 0x0000, 0x1000, CRC(8f952c88) SHA1(e593a89d7c6e7ea99e7ce376ffa2732d7b646d49) )
ROM_END


//-------------------------------------------------
//  ROM( adam_fdc_mihddd )
//-------------------------------------------------

ROM_START( adam_fdc_mihddd )
	ROM_REGION( 0x1000, M6801_TAG, 0 )
	ROM_LOAD( "1440k micro innovations hd-dd.u10", 0x0000, 0x1000, CRC(2efec8c0) SHA1(f6df22339c93dca938b65d0cbe23abcad89ec230) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *adam_fdc_device::device_rom_region() const
{
	return ROM_NAME( adam_fdc );
}

const tiny_rom_entry *adam_fdc_320kb_device::device_rom_region() const
{
	return ROM_NAME( adam_fdc_320kb );
}

const tiny_rom_entry *adam_fdc_a720dipi_device::device_rom_region() const
{
	return ROM_NAME( adam_fdc_a720dipi );
}

const tiny_rom_entry *adam_fdc_fp720at_device::device_rom_region() const
{
	return ROM_NAME( adam_fdc_fp720at );
}

const tiny_rom_entry *adam_fdc_mihddd_device::device_rom_region() const
{
	return ROM_NAME( adam_fdc_mihddd );
}


//-------------------------------------------------
//  ADDRESS_MAP( fdc6801_mem )
//-------------------------------------------------

void adam_fdc_device::adam_fdc_mem(address_map &map)
{
	map(0x0400, 0x07ff).ram().writeonly().share("ram");
	map(0x0800, 0x0800).mirror(0x3ff).r(WD2793_TAG, FUNC(wd2793_device::status_r));
	map(0x1400, 0x17ff).ram().readonly().share("ram");
	map(0x1800, 0x1800).mirror(0x3ff).w(WD2793_TAG, FUNC(wd2793_device::cmd_w));
	map(0x2800, 0x2800).mirror(0x3ff).r(WD2793_TAG, FUNC(wd2793_device::track_r));
	map(0x3800, 0x3800).mirror(0x3ff).w(WD2793_TAG, FUNC(wd2793_device::track_w));
	map(0x4800, 0x4800).mirror(0x3ff).r(WD2793_TAG, FUNC(wd2793_device::sector_r));
	map(0x5800, 0x5800).mirror(0x3ff).w(WD2793_TAG, FUNC(wd2793_device::sector_w));
	map(0x6800, 0x6800).mirror(0x3ff).r(WD2793_TAG, FUNC(wd2793_device::data_r));
	map(0x6c00, 0x6fff).r(FUNC(adam_fdc_device::read_data_r));
	map(0x7800, 0x7800).mirror(0x3ff).w(WD2793_TAG, FUNC(wd2793_device::data_w));
	map(0x7c00, 0x7fff).r(FUNC(adam_fdc_device::write_data_r));
	map(0x8000, 0x8fff).mirror(0x7000).rom().region(M6801_TAG, 0);
}


//-------------------------------------------------
//  floppy_formats
//-------------------------------------------------

void adam_fdc_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_ADAM_FORMAT);
}

static void adam_160kb_floppies(device_slot_interface &device)
{
	device.option_add("525ssdd", FLOPPY_525_SSDD);
}

static void adam_320kb_floppies(device_slot_interface &device)
{
	device.option_add("525ssdd", FLOPPY_525_SSDD);
	device.option_add("525dsdd", FLOPPY_525_DD);
}

static void adam_720kb_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

static void adam_1440kb_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("35hd", FLOPPY_35_HD);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void adam_fdc_device::device_add_mconfig(machine_config &config)
{
	M6803(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &adam_fdc_device::adam_fdc_mem);
	m_maincpu->in_p1_cb().set(FUNC(adam_fdc_device::p1_r));
	m_maincpu->out_p1_cb().set(FUNC(adam_fdc_device::p1_w));
	m_maincpu->in_p2_cb().set(FUNC(adam_fdc_device::p2_r));
	m_maincpu->out_p2_cb().set(FUNC(adam_fdc_device::p2_w));

	WD2793(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	FLOPPY_CONNECTOR(config, m_connector, adam_160kb_floppies, "525ssdd", adam_fdc_device::floppy_formats).enable_sound(true);
}

void adam_fdc_320kb_device::device_add_mconfig(machine_config &config)
{
	adam_fdc_device::device_add_mconfig(config);

	FLOPPY_CONNECTOR(config.replace(), m_connector, adam_320kb_floppies, "525dsdd", adam_fdc_320kb_device::floppy_formats).enable_sound(true);
}

void adam_fdc_a720dipi_device::device_add_mconfig(machine_config &config)
{
	adam_fdc_device::device_add_mconfig(config);

	FLOPPY_CONNECTOR(config.replace(), m_connector, adam_720kb_floppies, "35dd", adam_fdc_a720dipi_device::floppy_formats).enable_sound(true);
}

void adam_fdc_fp720at_device::device_add_mconfig(machine_config &config)
{
	adam_fdc_device::device_add_mconfig(config);

	FLOPPY_CONNECTOR(config.replace(), m_connector, adam_720kb_floppies, "35dd", adam_fdc_fp720at_device::floppy_formats).enable_sound(true);
}

void adam_fdc_mihddd_device::device_add_mconfig(machine_config &config)
{
	adam_fdc_device::device_add_mconfig(config);

	// clocks are increased to keep up with 500 Kbps data rate of HD floppy disks
	m_maincpu->set_clock(8000000);
	m_fdc->set_clock(2000000);

	// external serial clock used to maintain ADAMnet rate
	CLOCK(config, "clock", 500000).signal_handler().set([this] (int state) { if (state) { m_maincpu->clock_serial(); } });

	FLOPPY_CONNECTOR(config.replace(), m_connector, adam_1440kb_floppies, "35hd", adam_fdc_mihddd_device::floppy_formats).enable_sound(true);
}


//-------------------------------------------------
//  INPUT_PORTS( adam_fdc )
//-------------------------------------------------

static INPUT_PORTS_START( adam_fdc )
	PORT_START("SW3")
	PORT_DIPNAME( 0x01, 0x00, "Drive Select" ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x00, "DS1" )
	PORT_DIPSETTING(    0x01, "DS2" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor adam_fdc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( adam_fdc );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  adam_fdc_device - constructor
//-------------------------------------------------

adam_fdc_device::adam_fdc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
		device_adamnet_card_interface(mconfig, *this),
		m_maincpu(*this, M6801_TAG),
		m_fdc(*this, WD2793_TAG),
		m_connector(*this, WD2793_TAG":0"),
		m_floppy(nullptr),
		m_ram(*this, "ram"),
		m_sw3(*this, "SW3")
{
}

adam_fdc_device::adam_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: adam_fdc_device(mconfig, ADAM_FDC, tag, owner, clock)
{
}


//-------------------------------------------------
//  adam_fdc_320kb_device - constructor
//-------------------------------------------------

adam_fdc_320kb_device::adam_fdc_320kb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: adam_fdc_device(mconfig, type, tag, owner, clock)
{
}

adam_fdc_320kb_device::adam_fdc_320kb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: adam_fdc_320kb_device(mconfig, ADAM_FDC_320KB, tag, owner, clock)
{
}


//-------------------------------------------------
//  adam_fdc_a720dipi_device - constructor
//-------------------------------------------------

adam_fdc_a720dipi_device::adam_fdc_a720dipi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: adam_fdc_320kb_device(mconfig, ADAM_FDC_A720DIPI, tag, owner, clock)
{
}


//-------------------------------------------------
//  adam_fdc_fp720at_device - constructor
//-------------------------------------------------

adam_fdc_fp720at_device::adam_fdc_fp720at_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: adam_fdc_320kb_device(mconfig, ADAM_FDC_FP720AT, tag, owner, clock)
{
}


//-------------------------------------------------
//  adam_fdc_mihddd_device - constructor
//-------------------------------------------------

adam_fdc_mihddd_device::adam_fdc_mihddd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: adam_fdc_320kb_device(mconfig, ADAM_FDC_MIHDDD, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adam_fdc_device::device_start()
{
}


//-------------------------------------------------
//  adamnet_reset_w -
//-------------------------------------------------

void adam_fdc_device::adamnet_reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state);

	if (state == ASSERT_LINE) m_fdc->reset();
}


//-------------------------------------------------
//  read_data_r -
//-------------------------------------------------

uint8_t adam_fdc_device::read_data_r(offs_t offset)
{
	uint8_t data = m_fdc->data_r();

	if (!machine().side_effects_disabled())
		m_ram[offset & 0x3ff] = data;

	return data;
}


//-------------------------------------------------
//  write_data_r -
//-------------------------------------------------

uint8_t adam_fdc_device::write_data_r(offs_t offset)
{
	uint8_t data = m_ram[offset & 0x3ff];

	if (!machine().side_effects_disabled())
		m_fdc->data_w(data);

	return data;
}


//-------------------------------------------------
//  p1_r -
//-------------------------------------------------

uint8_t adam_fdc_device::p1_r()
{
	/*

	    bit     description

	    0       disk in place
	    1
	    2       FDC DRQ
	    3
	    4
	    5
	    6
	    7       SW3 (0=DS1, 1=DS2)

	*/

	uint8_t data = 0x00;

	// disk in place
	floppy_image_device *floppy0 = m_connector->get_device();
	data |= (floppy0 != nullptr && floppy0->exists()) ? 0x00 : 0x01;

	// floppy data request
	data |= m_fdc->drq_r() ? 0x04 : 0x00;

	// drive select
	data |= m_sw3->read() << 7;

	return data;
}

uint8_t adam_fdc_a720dipi_device::p1_r()
{
	// signal inverted here because drive outputs it that way?
	return adam_fdc_device::p1_r() ^ 0x01;
}


//-------------------------------------------------
//  p1_w -
//-------------------------------------------------

void adam_fdc_device::p1_w(uint8_t data)
{
	/*

	    bit     description

	    0
	    1       FDC ENP
	    2
	    3       FDC _DDEN
	    4
	    5       DRIVE SELECT
	    6       MOTOR ON
	    7

	*/

	// write precompensation
	//m_fdc->enp_w(BIT(data, 1));

	// density select
	m_fdc->dden_w(BIT(data, 3));

	// drive select
	m_floppy = nullptr;

	if (BIT(data, 5))
	{
		m_floppy = m_connector->get_device();
	}

	m_fdc->set_floppy(m_floppy);

	// motor enable
	if (m_floppy) m_floppy->mon_w(!BIT(data, 6));
}

void adam_fdc_320kb_device::p1_w(uint8_t data)
{
	adam_fdc_device::p1_w(data);

	// side select
	if (m_floppy) m_floppy->ss_w(!BIT(data, 4));
}

void adam_fdc_mihddd_device::p1_w(uint8_t data)
{
	// keep DDEN low here
	adam_fdc_320kb_device::p1_w(data & 0xf7);

	// DD/HD select
	m_fdc->enmf_w(BIT(data, 3));
}


//-------------------------------------------------
//  p2_r -
//-------------------------------------------------

uint8_t adam_fdc_device::p2_r()
{
	/*

	    bit     description

	    0       mode bit 0
	    1       mode bit 1
	    2       mode bit 2
	    3       NET RXD
	    4

	*/

	uint8_t data = M6801_MODE_2;

	// NET RXD
	data |= m_bus->rxd_r(this) << 3;

	return data;
}


//-------------------------------------------------
//  p2_w -
//-------------------------------------------------

void adam_fdc_device::p2_w(uint8_t data)
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4       NET TXD

	*/

	m_bus->txd_w(this, BIT(data, 4));
}
