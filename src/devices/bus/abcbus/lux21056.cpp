// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor 55 21056-00 Xebec Interface Host Adapter emulation

*********************************************************************/

/*

    Use the CHDMAN utility to create a 10MB image for ABC 850:

    $ chdman createhd -o ro202.chd -chs 321,4,32 -ss 256
    $ chdman createhd -o basf6186.chd -chs 440,4,32 -ss 256

    or a 20MB image for ABC 852:

    $ chdman createhd -o basf6185.chd -chs 440,6,32 -ss 256
    $ chdman createhd -o nec5126.chd -chs 615,4,32 -ss 256

    or a 60MB image for ABC 856:

    $ chdman createhd -o micr1325.chd -chs 1024,8,32 -ss 256

    Start the abc800 emulator with the ABC 850 attached on the ABC bus,
    with the new CHD and a UFD-DOS floppy mounted:

    $ mess abc800m -bus abc850 -flop1 ufd631 -hard ro202.chd
    $ mess abc800m -bus abc850 -bus:abc850:io2 xebec,bios=basf6186 -flop1 ufd631 -hard basf6186.chd

    or with the ABC 852 attached:

    $ mess abc800m -bus abc852 -flop1 ufd631 -hard basf6185.chd
    $ mess abc800m -bus abc852 -bus:abc852:io2 xebec,bios=nec5126 -flop1 ufd631 -hard nec5126.chd

    or with the ABC 856 attached:

    $ mess abc800m -bus abc856 -flop1 ufd631 -hard micr1325.chd

    Configure the floppy controller for use with an ABC 850:

    - Drive 0 Sides: Double
    - Drive 1 Sides: Double
    - Drive 0 Tracks: 40 or 80 depending on the UFD DOS image used
    - Drive 1 Tracks: 40 or 80 depending on the UFD DOS image used
    - Card Address: 44 (ABC 832/834/850)

    Reset the emulated machine by pressing F3.

    You should now see the following text at the top of the screen:

    DOS ar UFD-DOS ver. 19
    DR_: motsvarar MF_:

    Enter "BYE" to get into the UFD-DOS command prompt.
    Enter "DOSGEN,F HD0:" to start the formatting utility.
    Enter "J", and enter "J" to confirm the formatting.

    If you have a 20MB image, format the second partition by entering "DOSGEN,F HD1:", "J", and "J".

    If you have a 60MB image, format the third partition by entering "DOSGEN,F HD2:", "J", and "J",
    and format the fourth partition by entering "DOSGEN,F HD3:", "J", and "J".

    You can now list your freshly created partitions by entering "LIB".

    Or skip all of the above and use the preformatted images in the software list:

    $ mess abc800m -bus abc850 -flop1 ufd631 -hard abc850
    $ mess abc800m -bus abc852 -flop1 ufd631 -hard abc852
    $ mess abc800m -bus abc856 -flop1 ufd631 -hard abc856

*/

#include "lux21056.h"
#include "bus/scsi/s1410.h"



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
	ROM_REGION( 0x2000, Z80_TAG, 0 )
	// ABC 850
	ROM_SYSTEM_BIOS( 0, "ro202", "Rodime RO202 (CHS: 321,4,32,256)" )
	ROMX_LOAD( "rodi202.bin", 0x0000, 0x0800, CRC(337b4dcf) SHA1(791ebeb4521ddc11fb9742114018e161e1849bdf), ROM_BIOS(1) ) // Rodime RO202 (http://stason.org/TULARC/pc/hard-drives-hdd/rodime/RO202-11MB-5-25-FH-MFM-ST506.html)
	ROM_SYSTEM_BIOS( 1, "basf6186", "BASF 6186 (CHS: 440,4,32,256)" )
	ROMX_LOAD( "basf6186.bin", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(2) ) // BASF 6186 (http://stason.org/TULARC/pc/hard-drives-hdd/basf-magnetics/6186-14MB-5-25-FH-MFM-ST412.html)
	// ABC 852
	ROM_SYSTEM_BIOS( 2, "basf6185", "BASF 6185 (CHS: 440,6,32,256)" )
	ROMX_LOAD( "basf6185.bin", 0x0000, 0x0800, CRC(06f8fe2e) SHA1(e81f2a47c854e0dbb096bee3428d79e63591059d), ROM_BIOS(3) ) // BASF 6185 (http://stason.org/TULARC/pc/hard-drives-hdd/basf-magnetics/6185-22MB-5-25-FH-MFM-ST412.html)
	ROM_SYSTEM_BIOS( 3, "nec5126", "NEC 5126 (CHS: 615,4,32,256)" )
	ROMX_LOAD( "nec5126.bin", 0x0000, 0x1000, CRC(17c247e7) SHA1(7339738b87751655cb4d6414422593272fe72f5d), ROM_BIOS(4) ) // NEC 5126 (http://stason.org/TULARC/pc/hard-drives-hdd/nec/D5126-20MB-5-25-HH-MFM-ST506.html)
	// ABC 856
	ROM_SYSTEM_BIOS( 4, "micr1325", "Micropolis 1325 (CHS: 1024,8,32,256)" )
	ROMX_LOAD( "micr1325.bin", 0x0000, 0x0800, CRC(084af409) SHA1(342b8e214a8c4c2b014604e53c45ef1bd1c69ea3), ROM_BIOS(5) ) // Micropolis 1325 (http://stason.org/TULARC/pc/hard-drives-hdd/micropolis/1325-69MB-5-25-FH-MFM-ST506.html)
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
	AM_RANGE(0x0000, 0x0fff) AM_MIRROR(0x1000) AM_ROM AM_REGION(Z80_TAG, 0)
	AM_RANGE(0x2000, 0x27ff) AM_MIRROR(0x1800) AM_RAM
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
//  Z80DMA
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


WRITE_LINE_MEMBER( luxor_55_21056_device::write_sasi_bsy )
{
	m_sasi_bsy = state;

	if (m_sasi_bsy)
	{
		m_sasibus->write_sel(!m_sasi_bsy);
	}
}

WRITE_LINE_MEMBER( luxor_55_21056_device::write_sasi_io )
{
	m_sasi_io = state;

	if (!m_sasi_io)
	{
		m_sasi_data_out->write(m_sasi_data);
	}
	else
	{
		m_sasi_data_out->write(0);
	}
}

WRITE_LINE_MEMBER( luxor_55_21056_device::write_sasi_req )
{
	m_sasi_req = state;

	if (m_sasi_req)
	{
		m_sasibus->write_ack(!m_sasi_req);
	}
}

WRITE_LINE_MEMBER( luxor_55_21056_device::write_sasi_cd )
{
	m_sasi_cd = state;
}

WRITE_LINE_MEMBER( luxor_55_21056_device::write_sasi_msg )
{
	m_sasi_msg = state;
}

//-------------------------------------------------
//  MACHINE_DRIVER( luxor_55_21056 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( luxor_55_21056 )
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_8MHz/2)
	MCFG_CPU_PROGRAM_MAP(luxor_55_21056_mem)
	MCFG_CPU_IO_MAP(luxor_55_21056_io)
	MCFG_CPU_CONFIG(daisy_chain)

	MCFG_DEVICE_ADD(Z80DMA_TAG, Z80DMA, XTAL_8MHz/2)
	MCFG_Z80DMA_OUT_BUSREQ_CB(INPUTLINE(Z80_TAG, INPUT_LINE_HALT))
	MCFG_Z80DMA_OUT_INT_CB(INPUTLINE(Z80_TAG, INPUT_LINE_IRQ0))
	MCFG_Z80DMA_IN_MREQ_CB(READ8(luxor_55_21056_device, memory_read_byte))
	MCFG_Z80DMA_OUT_MREQ_CB(WRITE8(luxor_55_21056_device, memory_write_byte))
	MCFG_Z80DMA_IN_IORQ_CB(READ8(luxor_55_21056_device, io_read_byte))
	MCFG_Z80DMA_OUT_IORQ_CB(WRITE8(luxor_55_21056_device, io_write_byte))

	MCFG_DEVICE_ADD(SASIBUS_TAG, SCSI_PORT, 0)
	MCFG_SCSI_DATA_INPUT_BUFFER("sasi_data_in")
	MCFG_SCSI_REQ_HANDLER(WRITELINE(luxor_55_21056_device, write_sasi_req))
	MCFG_SCSI_IO_HANDLER(WRITELINE(luxor_55_21056_device, write_sasi_io))
	MCFG_SCSI_CD_HANDLER(WRITELINE(luxor_55_21056_device, write_sasi_cd))
	MCFG_SCSI_MSG_HANDLER(WRITELINE(luxor_55_21056_device, write_sasi_msg))
	MCFG_SCSI_BSY_HANDLER(WRITELINE(luxor_55_21056_device, write_sasi_bsy))
	MCFG_SCSIDEV_ADD(SASIBUS_TAG ":" SCSI_PORT_DEVICE1, "harddisk", S1410, SCSI_ID_0)

	MCFG_SCSI_OUTPUT_LATCH_ADD("sasi_data_out", SASIBUS_TAG)
	MCFG_DEVICE_ADD("sasi_data_in", INPUT_BUFFER, 0)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor luxor_55_21056_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( luxor_55_21056 );
}


//-------------------------------------------------
//  INPUT_PORTS( luxor_55_21046 )
//-------------------------------------------------

INPUT_PORTS_START( luxor_55_21056 )
	PORT_START("S1")
	PORT_DIPNAME( 0x3f, 0x24, "Card Address" )
	PORT_DIPSETTING(    0x20, "32" )
	PORT_DIPSETTING(    0x21, "33" )
	PORT_DIPSETTING(    0x22, "34" )
	PORT_DIPSETTING(    0x23, "35" )
	PORT_DIPSETTING(    0x24, "36 (ABC 850)" )
	PORT_DIPSETTING(    0x25, "37" )
	PORT_DIPSETTING(    0x26, "38" )
	PORT_DIPSETTING(    0x27, "39" )
	PORT_DIPSETTING(    0x28, "40" )
	PORT_DIPSETTING(    0x29, "41" )
	PORT_DIPSETTING(    0x2a, "42" )
	PORT_DIPSETTING(    0x2b, "43" )
	PORT_DIPSETTING(    0x2c, "44" )
	PORT_DIPSETTING(    0x2d, "45" )
	PORT_DIPSETTING(    0x2e, "46" )
	PORT_DIPSETTING(    0x2f, "47" )

	PORT_START("S2")
	PORT_DIPNAME( 0x01, 0x00, "PROM Size" )
	PORT_DIPSETTING(    0x00, "2 KB" )
	PORT_DIPSETTING(    0x01, "8 KB" )

	PORT_START("S3")
	PORT_DIPNAME( 0x01, 0x00, "RAM Size" )
	PORT_DIPSETTING(    0x00, "2 KB" )
	PORT_DIPSETTING(    0x01, "8 KB" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor luxor_55_21056_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( luxor_55_21056 );
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
		m_sasibus(*this, SASIBUS_TAG),
		m_sasi_data_out(*this, "sasi_data_out"),
		m_sasi_data_in(*this, "sasi_data_in"),
		m_s1(*this, "S1"),
		m_cs(false),
		m_rdy(0),
		m_sasi_req(0),
		m_sasi_io(0),
		m_sasi_cd(0),
		m_sasi_msg(0),
		m_sasi_bsy(0), m_inp(0), m_out(0),
		m_stat(0),
		m_sasi_data(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void luxor_55_21056_device::device_start()
{
	// state saving
	save_item(NAME(m_cs));
	save_item(NAME(m_rdy));
	save_item(NAME(m_inp));
	save_item(NAME(m_out));
	save_item(NAME(m_stat));
	save_item(NAME(m_sasi_req));
	save_item(NAME(m_sasi_io));
	save_item(NAME(m_sasi_cd));
	save_item(NAME(m_sasi_msg));
	save_item(NAME(m_sasi_bsy));
	save_item(NAME(m_sasi_data));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void luxor_55_21056_device::device_reset()
{
	m_maincpu->reset();

	m_cs = false;
	m_stat = 0;
	m_sasi_data = 0;

	set_rdy(m_rdy);
}



//**************************************************************************
//  ABC BUS INTERFACE
//**************************************************************************

//-------------------------------------------------
//  abcbus_cs -
//-------------------------------------------------

void luxor_55_21056_device::abcbus_cs(UINT8 data)
{
	m_cs = (data == m_s1->read());
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
		data |= m_rdy ^ STAT_DIR;
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

		if (m_rdy) set_rdy(!m_rdy);
	}

	return data;
}


//-------------------------------------------------
//  abcbus_out -
//-------------------------------------------------

void luxor_55_21056_device::abcbus_out(UINT8 data)
{
	if (m_cs)
	{
		m_out = data;

		if (STAT_DIR && !m_rdy) set_rdy(!m_rdy);
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
		device_reset();
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

	data |= m_rdy ^ STAT_DIR;

	data |= !m_sasi_req << 1;
	data |= !m_sasi_io << 2;
	data |= !m_sasi_cd << 3;
	data |= !m_sasi_msg << 4;
	data |= !m_sasi_bsy << 5;

	return data ^ 0xff;
}


//-------------------------------------------------
//  stat_w -
//-------------------------------------------------

WRITE8_MEMBER( luxor_55_21056_device::stat_w )
{
	m_stat = data;

	set_rdy(m_rdy);
}


//-------------------------------------------------
//  out_r -
//-------------------------------------------------

READ8_MEMBER( luxor_55_21056_device::out_r )
{
	UINT8 data = m_out;

	if (STAT_DIR && m_rdy) set_rdy(!m_rdy);

	return data;
}


//-------------------------------------------------
//  inp_w -
//-------------------------------------------------

WRITE8_MEMBER( luxor_55_21056_device::inp_w )
{
	m_inp = data;

	if (!STAT_DIR && !m_rdy) set_rdy(!m_rdy);
}


//-------------------------------------------------
//  sasi_data_r -
//-------------------------------------------------

READ8_MEMBER( luxor_55_21056_device::sasi_data_r )
{
	UINT8 data = m_sasi_data_in->read();

	m_sasibus->write_ack(!m_sasi_req);

	return data;
}


//-------------------------------------------------
//  sasi_data_w -
//-------------------------------------------------

WRITE8_MEMBER( luxor_55_21056_device::sasi_data_w )
{
	m_sasi_data = data;

	if (!m_sasi_io)
	{
		m_sasi_data_out->write(m_sasi_data);
	}

	m_sasibus->write_ack(!m_sasi_req);
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
	set_rdy(0);
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
	m_sasibus->write_sel(!m_sasi_bsy);
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
	m_sasibus->write_rst(1);
	m_sasibus->write_rst(0);
}


//-------------------------------------------------
//  set_rdy -
//-------------------------------------------------

void luxor_55_21056_device::set_rdy(int state)
{
	m_rdy = state;

	m_dma->rdy_w(m_rdy ^ STAT_DIR);
}
