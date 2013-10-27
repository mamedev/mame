// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor 55 21056-00 Xebec Interface Host Adapter emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "lux21056.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80_TAG         "5a"
#define Z80DMA_TAG      "6a"
#define SASIBUS_TAG     "sasi"

#define STAT_DIR \
    BIT(m_stat, 6)



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type LUXOR_55_21056 = &device_creator<luxor_55_21056_device>;


//-------------------------------------------------
//  ROM( luxor_55_21056 )
//-------------------------------------------------

ROM_START( luxor_55_21056 )
	ROM_REGION( 0x800, Z80_TAG, 0 )
	ROM_SYSTEM_BIOS( 0, "st4038", "Seagate ST4038 (CHS: 733,5,17,512)" )
	ROMX_LOAD( "st4038.6c", 0x000, 0x800, CRC(4c803b87) SHA1(1141bb51ad9200fc32d92a749460843dc6af8953), ROM_BIOS(1) ) // Seagate ST4038 (http://stason.org/TULARC/pc/hard-drives-hdd/seagate/ST4038-1987-31MB-5-25-FH-MFM-ST412.html)
	ROM_SYSTEM_BIOS( 1, "st225", "Seagate ST225 (CHS: 615,4,17,512)" )
	ROMX_LOAD( "st225.6c",  0x000, 0x800, CRC(c9f68f81) SHA1(7ff8b2a19f71fe0279ab3e5a0a5fffcb6030360c), ROM_BIOS(2) ) // Seagate ST225 (http://stason.org/TULARC/pc/hard-drives-hdd/seagate/ST225-21MB-5-25-HH-MFM-ST412.html)
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *luxor_55_21056_device::device_rom_region() const
{
	return ROM_NAME( luxor_55_21056 );
}


//-------------------------------------------------
//  ADDRESS_MAP( luxor_55_21056_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( luxor_55_21056_mem, AS_PROGRAM, 8, luxor_55_21056_device )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x07ff) AM_MIRROR(0x1800) AM_ROM AM_REGION(Z80_TAG, 0)
	AM_RANGE(0x2000, 0x3fff) AM_RAM
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( luxor_55_21056_io )
//-------------------------------------------------

static ADDRESS_MAP_START( luxor_55_21056_io, AS_IO, 8, luxor_55_21056_device )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xf8)
	AM_RANGE(0x00, 0x00) AM_MIRROR(0xf0) AM_DEVREADWRITE(Z80DMA_TAG, z80dma_device, read, write)
	AM_RANGE(0x08, 0x08) AM_READ(sasi_status_r)
	AM_RANGE(0x18, 0x18) AM_WRITE(stat_w)
	AM_RANGE(0x28, 0x28) AM_READ(out_r)
	AM_RANGE(0x38, 0x38) AM_WRITE(inp_w)
	AM_RANGE(0x48, 0x48) AM_READWRITE(sasi_data_r, sasi_data_w)
	AM_RANGE(0x58, 0x58) AM_READWRITE(rdy_reset_r, rdy_reset_w)
	AM_RANGE(0x68, 0x68) AM_READWRITE(sasi_sel_r, sasi_sel_w)
	AM_RANGE(0x78, 0x78) AM_READWRITE(sasi_rst_r, sasi_rst_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  z80_daisy_config daisy_chain
//-------------------------------------------------

static const z80_daisy_config daisy_chain[] =
{
	{ Z80DMA_TAG },
	{ NULL }
};


//-------------------------------------------------
//  Z80DMA_INTERFACE( dma_intf )
//-------------------------------------------------

READ8_MEMBER( luxor_55_21056_device::memory_read_byte )
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

WRITE8_MEMBER( luxor_55_21056_device::memory_write_byte )
{
	return m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}

READ8_MEMBER( luxor_55_21056_device::io_read_byte )
{
	return m_maincpu->space(AS_IO).read_byte(offset);
}

WRITE8_MEMBER( luxor_55_21056_device::io_write_byte )
{
	return m_maincpu->space(AS_IO).write_byte(offset, data);
}

static Z80DMA_INTERFACE( dma_intf )
{
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_HALT),
	DEVCB_CPU_INPUT_LINE(Z80_TAG, INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, luxor_55_21056_device, memory_read_byte),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, luxor_55_21056_device, memory_write_byte),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, luxor_55_21056_device, io_read_byte),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, luxor_55_21056_device, io_write_byte),
};

WRITE_LINE_MEMBER( luxor_55_21056_device::sasi_bsy_w )
{
	if (state)
	{
		m_sasibus->scsi_sel_w(0);
	}
}

WRITE_LINE_MEMBER( luxor_55_21056_device::sasi_io_w )
{
	if (!state)
	{
		m_sasibus->scsi_data_w(m_sasi_data ^ 0xff);
	}
}

WRITE_LINE_MEMBER( luxor_55_21056_device::sasi_req_w )
{
	if (state)
	{
		m_req = 1;
		m_sasibus->scsi_ack_w(!m_req);
	}
}


//-------------------------------------------------
//  MACHINE_DRIVER( luxor_55_21056 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( luxor_55_21056 )
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_8MHz/2)
	MCFG_CPU_PROGRAM_MAP(luxor_55_21056_mem)
	MCFG_CPU_IO_MAP(luxor_55_21056_io)
	MCFG_CPU_CONFIG(daisy_chain)

	MCFG_Z80DMA_ADD(Z80DMA_TAG, XTAL_8MHz/2, dma_intf)

	MCFG_SCSIBUS_ADD(SASIBUS_TAG)
	MCFG_SCSIDEV_ADD(SASIBUS_TAG ":harddisk0", SCSIHD, SCSI_ID_0)
	MCFG_SCSICB_ADD(SASIBUS_TAG ":host")
	MCFG_SCSICB_BSY_HANDLER(DEVWRITELINE(DEVICE_SELF_OWNER, luxor_55_21056_device, sasi_bsy_w))
	MCFG_SCSICB_IO_HANDLER(DEVWRITELINE(DEVICE_SELF_OWNER, luxor_55_21056_device, sasi_io_w))
	MCFG_SCSICB_REQ_HANDLER(DEVWRITELINE(DEVICE_SELF_OWNER, luxor_55_21056_device, sasi_req_w))
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor luxor_55_21056_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( luxor_55_21056 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  luxor_55_21056_device - constructor
//-------------------------------------------------

luxor_55_21056_device::luxor_55_21056_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, LUXOR_55_21056, "Luxor 55 21056", tag, owner, clock, "lux21056", __FILE__),
		device_abcbus_card_interface(mconfig, *this),
		m_maincpu(*this, Z80_TAG),
		m_dma(*this, Z80DMA_TAG),
		m_sasibus(*this, SASIBUS_TAG ":host"),
		m_cs(0),
		m_rdy(0),
		m_req(1),
		m_stat(0),
		m_sasi_data(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void luxor_55_21056_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void luxor_55_21056_device::device_reset()
{
	m_stat = 0;
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abcbus_cs -
//-------------------------------------------------

void luxor_55_21056_device::abcbus_cs(UINT8 data)
{
}


//-------------------------------------------------
//  abcbus_stat -
//-------------------------------------------------

UINT8 luxor_55_21056_device::abcbus_stat()
{
	UINT8 data = 0xff;

	if (m_cs)
	{
		data = m_stat & 0xfe;
		data |= m_rdy;
	}

	return data;
}


//-------------------------------------------------
//  abcbus_inp -
//-------------------------------------------------

UINT8 luxor_55_21056_device::abcbus_inp()
{
	UINT8 data = 0xff;

	if (m_cs && !STAT_DIR)
	{
		data = m_inp;

		set_rdy(!m_rdy);
	}

	return data;
}


//-------------------------------------------------
//  abcbus_utp -
//-------------------------------------------------

void luxor_55_21056_device::abcbus_utp(UINT8 data)
{
	if (m_cs)
	{
		m_out = data;

		set_rdy(!m_rdy);
	}
}


//-------------------------------------------------
//  abcbus_c1 -
//-------------------------------------------------

void luxor_55_21056_device::abcbus_c1(UINT8 data)
{
	if (m_cs)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
}


//-------------------------------------------------
//  abcbus_c3 -
//-------------------------------------------------

void luxor_55_21056_device::abcbus_c3(UINT8 data)
{
	if (m_cs)
	{
		m_maincpu->reset();
	}
}


//-------------------------------------------------
//  sasi_status_r -
//-------------------------------------------------

READ8_MEMBER( luxor_55_21056_device::sasi_status_r )
{
	/*
	
	    bit     description
	
	    0       RDY
	    1       REQ
	    2       I/O
	    3       C/D
	    4       MSG
	    5       BSY
	    6       
	    7       
	
	*/	

	UINT8 data = 0;

	data |= m_rdy;

	data |= (!m_req || !m_sasibus->scsi_req_r()) << 1;
	data |= !m_sasibus->scsi_io_r() << 2;
	data |= m_sasibus->scsi_cd_r() << 3;
	data |= m_sasibus->scsi_msg_r() << 4;
	data |= m_sasibus->scsi_bsy_r() << 5;

	return data ^ 0xff;
}


//-------------------------------------------------
//  stat_w -
//-------------------------------------------------

WRITE8_MEMBER( luxor_55_21056_device::stat_w )
{
	m_stat = data;
}


//-------------------------------------------------
//  out_r -
//-------------------------------------------------

READ8_MEMBER( luxor_55_21056_device::out_r )
{
	UINT8 data = m_out;

	set_rdy(!m_rdy);

	return data;
}


//-------------------------------------------------
//  inp_w -
//-------------------------------------------------

WRITE8_MEMBER( luxor_55_21056_device::inp_w )
{
	m_inp = data;

	set_rdy(!m_rdy);
}


//-------------------------------------------------
//  sasi_data_r -
//-------------------------------------------------

READ8_MEMBER( luxor_55_21056_device::sasi_data_r )
{
	UINT8 data = m_sasibus->scsi_data_r();

	m_req = !m_sasibus->scsi_req_r();
	m_sasibus->scsi_ack_w(!m_req);

	return data ^ 0xff;
}


//-------------------------------------------------
//  sasi_data_w -
//-------------------------------------------------

WRITE8_MEMBER( luxor_55_21056_device::sasi_data_w )
{
	m_sasi_data = data;

	if (!m_sasibus->scsi_io_r())
	{
		m_sasibus->scsi_data_w(m_sasi_data ^ 0xff);
	}

	m_req = !m_sasibus->scsi_req_r();
	m_sasibus->scsi_ack_w(!m_req);
}


//-------------------------------------------------
//  rdy_reset_r -
//-------------------------------------------------

READ8_MEMBER( luxor_55_21056_device::rdy_reset_r )
{
	rdy_reset_w(space, offset, 0xff);

	return 0xff;
}


//-------------------------------------------------
//  rdy_reset_w -
//-------------------------------------------------

WRITE8_MEMBER( luxor_55_21056_device::rdy_reset_w )
{
	set_rdy(STAT_DIR);
}


//-------------------------------------------------
//  sasi_sel_r -
//-------------------------------------------------

READ8_MEMBER( luxor_55_21056_device::sasi_sel_r )
{
	sasi_sel_w(space, offset, 0xff);

	return 0xff;
}


//-------------------------------------------------
//  sasi_sel_w -
//-------------------------------------------------

WRITE8_MEMBER( luxor_55_21056_device::sasi_sel_w )
{
	m_sasibus->scsi_sel_w(m_sasibus->scsi_bsy_r());
}


//-------------------------------------------------
//  sasi_rst_r -
//-------------------------------------------------

READ8_MEMBER( luxor_55_21056_device::sasi_rst_r )
{
	sasi_rst_w(space, offset, 0xff);

	return 0xff;
}


//-------------------------------------------------
//  sasi_rst_w -
//-------------------------------------------------

WRITE8_MEMBER( luxor_55_21056_device::sasi_rst_w )
{
	m_sasibus->scsi_rst_w(1);
	m_sasibus->scsi_rst_w(0);
}


//-------------------------------------------------
//  set_rdy -
//-------------------------------------------------

void luxor_55_21056_device::set_rdy(int state)
{
	m_rdy = state;

	m_dma->rdy_w(m_rdy);
}
