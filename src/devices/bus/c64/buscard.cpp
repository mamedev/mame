// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Batteries Included BusCard cartridge emulation

    SYS 61000 -> Enable BASIC 4.0
    SYS 61003 -> Disable BASIC 4.0
    SYS 61006 -> Enter Machine Language Monitor

**********************************************************************/

#include "emu.h"
#include "buscard.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(C64_BUSCARD, c64_buscard_device, "c64_buscard", "C64 BusCard cartridge")


//-------------------------------------------------
//  ROM( buscard )
//-------------------------------------------------

ROM_START( buscard )
	ROM_REGION( 0x8000, "rom", 0 )
	ROM_LOAD( "0.9.u1", 0x0000, 0x2000, CRC(175e8c96) SHA1(8fb4ba7e3d0b58dc01b66ef962955596f1b125b5) )
	//ROM_LOAD( "unpopulated.u13", 0x2000, 0x2000 )
	//ROM_LOAD( "unpopulated.u14", 0x4000, 0x2000 )
	//ROM_LOAD( "unpopulated.u15", 0x6000, 0x2000 )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *c64_buscard_device::device_rom_region() const
{
	return ROM_NAME( buscard );
}


//-------------------------------------------------
//  INPUT_PORTS( buscard )
//-------------------------------------------------

static INPUT_PORTS_START( buscard )
	PORT_START("S1")
	PORT_DIPNAME( 0x03, 0x00, "Device #4" ) PORT_DIPLOCATION("S1:1,2")
	PORT_DIPSETTING(    0x00, "Serial" )
	PORT_DIPSETTING(    0x01, "Parallel w/conv." )
	PORT_DIPSETTING(    0x02, "IEEE" )
	PORT_DIPSETTING(    0x03, "Parallel" )
	PORT_DIPNAME( 0x04, 0x04, "Device #5" ) PORT_DIPLOCATION("S1:3")
	PORT_DIPSETTING(    0x00, "IEEE" )
	PORT_DIPSETTING(    0x04, "Serial" )
	PORT_DIPNAME( 0x08, 0x08, "Device #6" ) PORT_DIPLOCATION("S1:4")
	PORT_DIPSETTING(    0x00, "IEEE" )
	PORT_DIPSETTING(    0x08, "Serial" )
	PORT_DIPNAME( 0x10, 0x10, "Device #7" ) PORT_DIPLOCATION("S1:5")
	PORT_DIPSETTING(    0x00, "IEEE" )
	PORT_DIPSETTING(    0x10, "Serial" )
	PORT_DIPNAME( 0x20, 0x20, "Device #8" ) PORT_DIPLOCATION("S1:6")
	PORT_DIPSETTING(    0x00, "IEEE" )
	PORT_DIPSETTING(    0x20, "Serial" )
	PORT_DIPNAME( 0x40, 0x40, "Device #9" ) PORT_DIPLOCATION("S1:7")
	PORT_DIPSETTING(    0x00, "IEEE" )
	PORT_DIPSETTING(    0x40, "Serial" )
	PORT_DIPNAME( 0x80, 0x80, "Device #10" ) PORT_DIPLOCATION("S1:8")
	PORT_DIPSETTING(    0x00, "IEEE" )
	PORT_DIPSETTING(    0x80, "Serial" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor c64_buscard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( buscard );
}


//-------------------------------------------------
//  PPI interface
//-------------------------------------------------

READ8_MEMBER( c64_buscard_device::ppi_pa_r )
{
	uint8_t data = 0xff;

	if (!m_te)
	{
		data = m_ieee1->read(space, 0);
	}

	if (m_dipsw)
	{
		data = m_s1->read();
	}

	return data;
}

WRITE8_MEMBER( c64_buscard_device::ppi_pa_w )
{
	m_ieee1->write(space, 0, data);

	m_centronics->write_data0(BIT(data, 0));
	m_centronics->write_data1(BIT(data, 1));
	m_centronics->write_data2(BIT(data, 2));
	m_centronics->write_data3(BIT(data, 3));
	m_centronics->write_data4(BIT(data, 4));
	m_centronics->write_data5(BIT(data, 5));
	m_centronics->write_data6(BIT(data, 6));
	m_centronics->write_data7(BIT(data, 7));
}

WRITE8_MEMBER( c64_buscard_device::ppi_pb_w )
{
	/*

	    bit     description

	    PB0     BASIC ROM bank bit 0
	    PB1     BASIC ROM bank bit 1
	    PB2
	    PB3     BASIC ROM enable
	    PB4
	    PB5
	    PB6     STROBE
	    PB7     DIP switch select

	*/

	m_bank = data & 0x03;
	m_basic = BIT(data, 3);

	m_centronics->write_strobe(BIT(data, 6));

	m_dipsw = BIT(data, 7);
}

READ8_MEMBER( c64_buscard_device::ppi_pc_r )
{
	/*

	    bit     description

	    PC0     BUSY
	    PC1
	    PC2     DAV
	    PC3     EOI
	    PC4
	    PC5     ATN
	    PC6     NRFD
	    PC7     NDAC

	*/

	uint8_t data = 0;

	data |= m_busy;

	data |= m_ieee2->dav_r() << 2;
	data |= m_ieee2->eoi_r() << 3;
	data |= m_ieee2->atn_r() << 5;
	data |= m_ieee2->nrfd_r() << 6;
	data |= m_ieee2->ndac_r() << 7;

	return data;
}

WRITE8_MEMBER( c64_buscard_device::ppi_pc_w )
{
	/*

	    bit     description

	    PC0
	    PC1     ATN
	    PC2     DAV
	    PC3     EOI
	    PC4     TE
	    PC5
	    PC6     NRFD
	    PC7     NDAC

	*/

	m_te = BIT(data, 4);
	m_ieee1->te_w(m_te);
	m_ieee2->te_w(m_te);

	m_ieee2->atn_w(BIT(data, 1));
	m_ieee2->dav_w(BIT(data, 2));
	m_ieee2->eoi_w(BIT(data, 3));
	m_ieee2->nrfd_w(BIT(data, 6));
	m_ieee2->ndac_w(BIT(data, 7));
}


//-------------------------------------------------
//  Centronics interface
//-------------------------------------------------

WRITE_LINE_MEMBER( c64_buscard_device::busy_w )
{
	m_busy = state;
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void c64_buscard_device::device_add_mconfig(machine_config &config)
{
	I8255A(config, m_ppi, 0);
	m_ppi->in_pa_callback().set(FUNC(c64_buscard_device::ppi_pa_r));
	m_ppi->out_pa_callback().set(FUNC(c64_buscard_device::ppi_pa_w));
	m_ppi->in_pb_callback().set_constant(0xff);
	m_ppi->out_pb_callback().set(FUNC(c64_buscard_device::ppi_pb_w));
	m_ppi->in_pc_callback().set(FUNC(c64_buscard_device::ppi_pc_r));
	m_ppi->out_pc_callback().set(FUNC(c64_buscard_device::ppi_pc_w));

	DS75160A(config, m_ieee1, 0);
	m_ieee1->read_callback().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	m_ieee1->write_callback().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));

	DS75161A(config, m_ieee2, 0);
	m_ieee2->in_ren().set(IEEE488_TAG, FUNC(ieee488_device::ren_r));
	m_ieee2->in_ifc().set(IEEE488_TAG, FUNC(ieee488_device::ifc_r));
	m_ieee2->in_ndac().set(IEEE488_TAG, FUNC(ieee488_device::ndac_r));
	m_ieee2->in_nrfd().set(IEEE488_TAG, FUNC(ieee488_device::nrfd_r));
	m_ieee2->in_dav().set(IEEE488_TAG, FUNC(ieee488_device::dav_r));
	m_ieee2->in_eoi().set(IEEE488_TAG, FUNC(ieee488_device::eoi_r));
	m_ieee2->in_atn().set(IEEE488_TAG, FUNC(ieee488_device::atn_r));
	m_ieee2->in_srq().set(IEEE488_TAG, FUNC(ieee488_device::srq_r));
	m_ieee2->out_ren().set(IEEE488_TAG, FUNC(ieee488_device::host_ren_w));
	m_ieee2->out_ifc().set(IEEE488_TAG, FUNC(ieee488_device::host_ifc_w));
	m_ieee2->out_ndac().set(IEEE488_TAG, FUNC(ieee488_device::host_ndac_w));
	m_ieee2->out_nrfd().set(IEEE488_TAG, FUNC(ieee488_device::host_nrfd_w));
	m_ieee2->out_dav().set(IEEE488_TAG, FUNC(ieee488_device::host_dav_w));
	m_ieee2->out_eoi().set(IEEE488_TAG, FUNC(ieee488_device::host_eoi_w));
	m_ieee2->out_atn().set(IEEE488_TAG, FUNC(ieee488_device::host_atn_w));
	m_ieee2->out_srq().set(IEEE488_TAG, FUNC(ieee488_device::host_srq_w));

	IEEE488(config, m_bus, 0);
	ieee488_slot_device::add_cbm_defaults(config, nullptr);

	CENTRONICS(config, m_centronics, centronics_devices, nullptr);
	m_centronics->busy_handler().set(FUNC(c64_buscard_device::busy_w));

	C64_EXPANSION_SLOT(config, m_exp, DERIVED_CLOCK(1, 1), c64_expansion_cards, nullptr);
	m_exp->set_passthrough();
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  c64_buscard_device - constructor
//-------------------------------------------------

c64_buscard_device::c64_buscard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, C64_BUSCARD, tag, owner, clock),
	device_c64_expansion_card_interface(mconfig, *this),
	m_ppi(*this, "u2"),
	m_ieee1(*this, "u3"),
	m_ieee2(*this, "u4"),
	m_bus(*this, IEEE488_TAG),
	m_centronics(*this, "p4"),
	m_exp(*this, "exp"),
	m_s1(*this, "S1"),
	m_rom(*this, "rom"),
	m_te(1),
	m_bank(3),
	m_basic(1),
	m_dipsw(1),
	m_busy(1)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void c64_buscard_device::device_start()
{
	m_ieee1->pe_w(0);
	m_ieee2->dc_w(0);

	// state saving
	save_item(NAME(m_te));
	save_item(NAME(m_bank));
	save_item(NAME(m_basic));
	save_item(NAME(m_dipsw));
	save_item(NAME(m_busy));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void c64_buscard_device::device_reset()
{
	m_ppi->reset();

	m_ieee2->ifc_w(0);
	m_ieee2->ifc_w(1);
}


//-------------------------------------------------
//  c64_cd_r - cartridge data read
//-------------------------------------------------

uint8_t c64_buscard_device::c64_cd_r(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	int cs = BIT(offset, 6) && BIT(offset, 7);

	if (sphi2 && !io1 && cs)
	{
		data = m_ppi->read(offset & 0x03);
	}

	if (!pd_pgm1(offset, sphi2))
	{
		data = m_rom->base()[offset & 0x1fff];
	}

	if (!pd_pgm234(offset, sphi2, 0x02))
	{
		data = m_rom->base()[0x2000 | (offset & 0x1fff)];
	}

	if (!pd_pgm234(offset, sphi2, 0x01))
	{
		data = m_rom->base()[0x4000 | (offset & 0x1fff)];
	}

	if (!pd_pgm234(offset, sphi2, 0x00))
	{
		data = m_rom->base()[0x6000 | (offset & 0x1fff)];
	}

	return m_exp->cd_r(offset, data, sphi2, ba, roml, romh, io1 | cs, io2);
}


//-------------------------------------------------
//  c64_cd_w - cartridge data write
//-------------------------------------------------

void c64_buscard_device::c64_cd_w(offs_t offset, uint8_t data, int sphi2, int ba, int roml, int romh, int io1, int io2)
{
	int cs = BIT(offset, 6) && BIT(offset, 7);

	if (sphi2 && !io1 && cs)
	{
		m_ppi->write(offset & 0x03, data);
	}

	m_exp->cd_w(offset, data, sphi2, ba, roml, romh, io1 | cs, io2);
}


//-------------------------------------------------
//  c64_game_r - cartridge GAME read
//-------------------------------------------------

int c64_buscard_device::c64_game_r(offs_t offset, int sphi2, int ba, int rw)
{
	return pd_pgm1(offset, sphi2) & m_exp->game_r(offset, sphi2, ba, rw, m_slot->loram(), m_slot->hiram());
}


//-------------------------------------------------
//  c64_exrom_r - cartridge EXROM read
//-------------------------------------------------

int c64_buscard_device::c64_exrom_r(offs_t offset, int sphi2, int ba, int rw)
{
	return (!pd_pgm1(offset, sphi2)) | m_exp->exrom_r(offset, sphi2, ba, rw, m_slot->loram(), m_slot->hiram());
}


//-------------------------------------------------
//  pd_pgm1 - ROM 1 enable
//-------------------------------------------------

bool c64_buscard_device::pd_pgm1(offs_t offset, int sphi2)
{
	if (sphi2 && m_slot->hiram())
	{
		if (offset >= 0xa000 && offset < 0xc000 && m_slot->loram() && !m_basic)
		{
			return 0;
		}

		if (offset >= 0xec00 && offset < 0xf000)
		{
			return 0;
		}
	}

	return 1;
}


//-------------------------------------------------
//  pd_pgm234 - ROM 2/3/4 enable
//-------------------------------------------------

bool c64_buscard_device::pd_pgm234(offs_t offset, int sphi2, int bank)
{
	return !(sphi2 && m_slot->hiram() && m_slot->loram() && offset >= 0xa000 && offset < 0xc000 && m_basic && (m_bank == bank));
}
