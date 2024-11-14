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

    $ mame abc800m -bus abc850 -flop1 ufd631 -hard ro202.chd
    $ mame abc800m -bus abc850 -bus:abc850:io2 xebec,bios=basf6186 -flop1 ufd631 -hard basf6186.chd

    or with the ABC 852 attached:

    $ mame abc800m -bus abc852 -flop1 ufd631 -hard basf6185.chd
    $ mame abc800m -bus abc852 -bus:abc852:io2 xebec,bios=nec5126 -flop1 ufd631 -hard nec5126.chd

    or with the ABC 856 attached:

    $ mame abc800m -bus abc856 -flop1 ufd631 -hard micr1325.chd

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

    $ mame abc800m -bus abc850 -flop1 ufd631 -hard abc850
    $ mame abc800m -bus abc852 -flop1 ufd631 -hard abc852
    $ mame abc800m -bus abc856 -flop1 ufd631 -hard abc856

*/

#include "emu.h"
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

DEFINE_DEVICE_TYPE(LUXOR_55_21056, luxor_55_21056_device, "lux21056", "Luxor 55 21056")


//-------------------------------------------------
//  ROM( luxor_55_21056 )
//-------------------------------------------------

ROM_START( luxor_55_21056 )
	ROM_REGION( 0x2000, Z80_TAG, 0 )
	// ABC 850
	ROM_SYSTEM_BIOS( 0, "ro202", "Rodime RO202 (CHS: 321,4,32,256)" )
	ROMX_LOAD( "rodi202.bin", 0x0000, 0x0800, CRC(337b4dcf) SHA1(791ebeb4521ddc11fb9742114018e161e1849bdf), ROM_BIOS(0) ) // Rodime RO202 (http://stason.org/TULARC/pc/hard-drives-hdd/rodime/RO202-11MB-5-25-FH-MFM-ST506.html)
	ROM_SYSTEM_BIOS( 1, "basf6186", "BASF 6186 (CHS: 440,4,32,256)" )
	ROMX_LOAD( "basf6186.bin", 0x0000, 0x0800, NO_DUMP, ROM_BIOS(1) ) // BASF 6186 (http://stason.org/TULARC/pc/hard-drives-hdd/basf-magnetics/6186-14MB-5-25-FH-MFM-ST412.html)
	// ABC 852
	ROM_SYSTEM_BIOS( 2, "basf6185", "BASF 6185 (CHS: 440,6,32,256)" )
	ROMX_LOAD( "basf6185.bin", 0x0000, 0x0800, CRC(06f8fe2e) SHA1(e81f2a47c854e0dbb096bee3428d79e63591059d), ROM_BIOS(2) ) // BASF 6185 (http://stason.org/TULARC/pc/hard-drives-hdd/basf-magnetics/6185-22MB-5-25-FH-MFM-ST412.html)
	ROM_SYSTEM_BIOS( 3, "nec5126", "NEC 5126 (CHS: 615,4,32,256)" )
	ROMX_LOAD( "nec5126.bin", 0x0000, 0x1000, CRC(17c247e7) SHA1(7339738b87751655cb4d6414422593272fe72f5d), ROM_BIOS(3) ) // NEC 5126 (http://stason.org/TULARC/pc/hard-drives-hdd/nec/D5126-20MB-5-25-HH-MFM-ST506.html)
	// ABC 856
	ROM_SYSTEM_BIOS( 4, "micr1325", "Micropolis 1325 (CHS: 1024,8,32,256)" )
	ROMX_LOAD( "micr1325.bin", 0x0000, 0x0800, CRC(084af409) SHA1(342b8e214a8c4c2b014604e53c45ef1bd1c69ea3), ROM_BIOS(4) ) // Micropolis 1325 (http://stason.org/TULARC/pc/hard-drives-hdd/micropolis/1325-69MB-5-25-FH-MFM-ST506.html)
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *luxor_55_21056_device::device_rom_region() const
{
	return ROM_NAME( luxor_55_21056 );
}


//-------------------------------------------------
//  ADDRESS_MAP( luxor_55_21056_mem )
//-------------------------------------------------

void luxor_55_21056_device::luxor_55_21056_mem(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x3fff);
	map(0x0000, 0x0fff).mirror(0x1000).rom().region(Z80_TAG, 0);
	map(0x2000, 0x27ff).mirror(0x1800).ram();
}


//-------------------------------------------------
//  ADDRESS_MAP( luxor_55_21056_io )
//-------------------------------------------------

void luxor_55_21056_device::luxor_55_21056_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xf8);
	map(0x00, 0x00).mirror(0xf0).rw(Z80DMA_TAG, FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x08, 0x08).r(FUNC(luxor_55_21056_device::sasi_status_r));
	map(0x18, 0x18).w(FUNC(luxor_55_21056_device::stat_w));
	map(0x28, 0x28).r(FUNC(luxor_55_21056_device::out_r));
	map(0x38, 0x38).w(FUNC(luxor_55_21056_device::inp_w));
	map(0x48, 0x48).rw(FUNC(luxor_55_21056_device::sasi_data_r), FUNC(luxor_55_21056_device::sasi_data_w));
	map(0x58, 0x58).rw(FUNC(luxor_55_21056_device::rdy_reset_r), FUNC(luxor_55_21056_device::rdy_reset_w));
	map(0x68, 0x68).rw(FUNC(luxor_55_21056_device::sasi_sel_r), FUNC(luxor_55_21056_device::sasi_sel_w));
	map(0x78, 0x78).rw(FUNC(luxor_55_21056_device::sasi_rst_r), FUNC(luxor_55_21056_device::sasi_rst_w));
}


//-------------------------------------------------
//  z80_daisy_config daisy_chain
//-------------------------------------------------

static const z80_daisy_config daisy_chain[] =
{
	{ Z80DMA_TAG },
	{ nullptr }
};


//-------------------------------------------------
//  Z80DMA
//-------------------------------------------------

uint8_t luxor_55_21056_device::memory_read_byte(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

void luxor_55_21056_device::memory_write_byte(offs_t offset, uint8_t data)
{
	return m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}

uint8_t luxor_55_21056_device::io_read_byte(offs_t offset)
{
	return m_maincpu->space(AS_IO).read_byte(offset);
}

void luxor_55_21056_device::io_write_byte(offs_t offset, uint8_t data)
{
	return m_maincpu->space(AS_IO).write_byte(offset, data);
}


void luxor_55_21056_device::write_sasi_bsy(int state)
{
	m_sasi_bsy = state;

	if (m_sasi_bsy)
	{
		m_sasibus->write_sel(0);
	}
}

void luxor_55_21056_device::write_sasi_io(int state)
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

void luxor_55_21056_device::write_sasi_req(int state)
{
	m_sasi_req = state;

	if (m_sasi_req)
	{
		m_req = 0;
		m_sasibus->write_ack(!m_req);
	}
}

void luxor_55_21056_device::write_sasi_cd(int state)
{
	m_sasi_cd = state;
}

void luxor_55_21056_device::write_sasi_msg(int state)
{
	m_sasi_msg = state;
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void luxor_55_21056_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_maincpu, XTAL(8'000'000)/2);
	m_maincpu->set_memory_map(&luxor_55_21056_device::luxor_55_21056_mem);
	m_maincpu->set_io_map(&luxor_55_21056_device::luxor_55_21056_io);
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->busack_cb().set(m_dma, FUNC(z80dma_device::bai_w));

	Z80DMA(config, m_dma, XTAL(8'000'000)/2);
	m_dma->out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSRQ);
	m_dma->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_dma->in_mreq_callback().set(FUNC(luxor_55_21056_device::memory_read_byte));
	m_dma->out_mreq_callback().set(FUNC(luxor_55_21056_device::memory_write_byte));
	m_dma->in_iorq_callback().set(FUNC(luxor_55_21056_device::io_read_byte));
	m_dma->out_iorq_callback().set(FUNC(luxor_55_21056_device::io_write_byte));

	SCSI_PORT(config, m_sasibus);
	m_sasibus->set_data_input_buffer(m_sasi_data_in);
	m_sasibus->req_handler().set(FUNC(luxor_55_21056_device::write_sasi_req));
	m_sasibus->io_handler().set(FUNC(luxor_55_21056_device::write_sasi_io));
	m_sasibus->cd_handler().set(FUNC(luxor_55_21056_device::write_sasi_cd));
	m_sasibus->msg_handler().set(FUNC(luxor_55_21056_device::write_sasi_msg));
	m_sasibus->bsy_handler().set(FUNC(luxor_55_21056_device::write_sasi_bsy));
	m_sasibus->set_slot_device(1, "harddisk", S1410, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_0));

	OUTPUT_LATCH(config, m_sasi_data_out);
	m_sasibus->set_output_latch(*m_sasi_data_out);

	INPUT_BUFFER(config, m_sasi_data_in);
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

luxor_55_21056_device::luxor_55_21056_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, LUXOR_55_21056, tag, owner, clock),
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

void luxor_55_21056_device::abcbus_cs(uint8_t data)
{
	m_cs = (data == m_s1->read());
}


//-------------------------------------------------
//  abcbus_stat -
//-------------------------------------------------

uint8_t luxor_55_21056_device::abcbus_stat()
{
	uint8_t data = 0xff;

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

uint8_t luxor_55_21056_device::abcbus_inp()
{
	uint8_t data = 0xff;

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

void luxor_55_21056_device::abcbus_out(uint8_t data)
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

void luxor_55_21056_device::abcbus_c1(uint8_t data)
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

void luxor_55_21056_device::abcbus_c3(uint8_t data)
{
	if (m_cs)
	{
		device_reset();
	}
}


//-------------------------------------------------
//  sasi_status_r -
//-------------------------------------------------

uint8_t luxor_55_21056_device::sasi_status_r()
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

	uint8_t data = 0;

	data |= m_rdy ^ STAT_DIR;

	data |= (m_req || m_sasi_req) << 1;
	data |= m_sasi_io << 2;
	data |= !m_sasi_cd << 3;
	data |= !m_sasi_msg << 4;
	data |= !m_sasi_bsy << 5;

	return data ^ 0xff;
}


//-------------------------------------------------
//  stat_w -
//-------------------------------------------------

void luxor_55_21056_device::stat_w(uint8_t data)
{
	m_stat = data;

	set_rdy(m_rdy);
}


//-------------------------------------------------
//  out_r -
//-------------------------------------------------

uint8_t luxor_55_21056_device::out_r()
{
	uint8_t data = m_out;

	if (STAT_DIR && m_rdy) set_rdy(!m_rdy);

	return data;
}


//-------------------------------------------------
//  inp_w -
//-------------------------------------------------

void luxor_55_21056_device::inp_w(uint8_t data)
{
	m_inp = data;

	if (!STAT_DIR && !m_rdy) set_rdy(!m_rdy);
}


//-------------------------------------------------
//  sasi_data_r -
//-------------------------------------------------

uint8_t luxor_55_21056_device::sasi_data_r()
{
	uint8_t data = m_sasi_data_in->read();

	m_req = !m_sasi_req;
	m_sasibus->write_ack(!m_req);

	return data;
}


//-------------------------------------------------
//  sasi_data_w -
//-------------------------------------------------

void luxor_55_21056_device::sasi_data_w(uint8_t data)
{
	m_sasi_data = data;

	if (!m_sasi_io)
	{
		m_sasi_data_out->write(m_sasi_data);
	}

	m_req = !m_sasi_req;
	m_sasibus->write_ack(!m_req);
}


//-------------------------------------------------
//  rdy_reset_r -
//-------------------------------------------------

uint8_t luxor_55_21056_device::rdy_reset_r()
{
	if (!machine().side_effects_disabled())
		rdy_reset_w(0xff);

	return 0xff;
}


//-------------------------------------------------
//  rdy_reset_w -
//-------------------------------------------------

void luxor_55_21056_device::rdy_reset_w(uint8_t data)
{
	set_rdy(0);
}


//-------------------------------------------------
//  sasi_sel_r -
//-------------------------------------------------

uint8_t luxor_55_21056_device::sasi_sel_r()
{
	if (!machine().side_effects_disabled())
		sasi_sel_w(0xff);

	return 0xff;
}


//-------------------------------------------------
//  sasi_sel_w -
//-------------------------------------------------

void luxor_55_21056_device::sasi_sel_w(uint8_t data)
{
	m_sasibus->write_sel(!m_sasi_bsy);
}


//-------------------------------------------------
//  sasi_rst_r -
//-------------------------------------------------

uint8_t luxor_55_21056_device::sasi_rst_r()
{
	if (!machine().side_effects_disabled())
		sasi_rst_w(0xff);

	return 0xff;
}


//-------------------------------------------------
//  sasi_rst_w -
//-------------------------------------------------

void luxor_55_21056_device::sasi_rst_w(uint8_t data)
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
