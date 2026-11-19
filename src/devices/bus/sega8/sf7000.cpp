// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***********************************************************************************************************

 Sega SF-7000 Super Control Station

 Disk drive unit for the SC-3000, attached through the cartridge slot.
 Contains 64K RAM, an IPL ROM, a uPD765A FDC with an internal 3" drive,
 an 8251 USART for RS-232 and an 8255 PPI for the Centronics port.

 ***********************************************************************************************************/

#include "emu.h"
#include "sf7000.h"

#include "softlist_dev.h"

#include "formats/sf7000_dsk.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SEGA8_SF7000, sega8_sf7000_device, "sf7000", "Sega SF-7000 Super Control Station")


//-------------------------------------------------
//  ROM( sf7000 )
//-------------------------------------------------

ROM_START( sf7000 )
	ROM_REGION( 0x2000, "ipl", 0 )
	ROM_LOAD( "ipl.rom", 0x0000, 0x2000, CRC(d76810b8) SHA1(77339a6db2593aadc638bed77b8e9bed5d9d87e3) )
ROM_END

const tiny_rom_entry *sega8_sf7000_device::device_rom_region() const
{
	return ROM_NAME( sf7000 );
}


//-------------------------------------------------
//  INPUT_PORTS( sf7000 )
//-------------------------------------------------

INPUT_PORTS_START( sf7000 )
	PORT_START("BAUD")
	PORT_CONFNAME( 0x07, 0x05, "Baud Rate" )
	PORT_CONFSETTING( 0x00, "9600 (jumper 1)" )
	PORT_CONFSETTING( 0x01, "4800 (jumper 2)" )
	PORT_CONFSETTING( 0x02, "2400 (jumper 3)" )
	PORT_CONFSETTING( 0x03, "1200 (jumper 4)" )
	PORT_CONFSETTING( 0x04, "600 (jumper 5)" )
	PORT_CONFSETTING( 0x05, "300 (jumper 6)" )
INPUT_PORTS_END

ioport_constructor sega8_sf7000_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( sf7000 );
}


//-------------------------------------------------
//  I8255 interface
//-------------------------------------------------

void sega8_sf7000_device::write_centronics_busy(int state)
{
	m_centronics_busy = state;
}

uint8_t sega8_sf7000_device::ppi_pa_r()
{
	/*
	    Signal  Description

	    PA0     INT from FDC
	    PA1     BUSY from Centronics printer
	    PA2     INDEX from FDD
	    PA3
	    PA4
	    PA5
	    PA6
	    PA7
	*/

	floppy_image_device *floppy = m_floppy0->get_device();
	uint8_t data = 0;

	data |= m_fdc->get_irq() ? 0x01 : 0x00;
	data |= m_centronics_busy << 1;
	data |= (floppy ? floppy->idx_r() : 0) << 2;

	return data;
}

void sega8_sf7000_device::ppi_pc_w(uint8_t data)
{
	/*
	    Signal  Description

	    PC0     /IN USE signal to FDD
	    PC1     /MOTOR ON signal to FDD
	    PC2     TC signal to FDC
	    PC3     RESET signal to FDC
	    PC4     not connected
	    PC5     READY signal to FDC
	    PC6     /ROM SEL (switch between IPL ROM and RAM)
	    PC7     /STROBE to Centronics printer
	*/

	floppy_image_device *floppy = m_floppy0->get_device();

	/* floppy motor */
	if (floppy)
		floppy->mon_w(BIT(data, 1));

	/* FDC terminal count */
	m_fdc->tc_w(BIT(data, 2));

	/* FDC reset */
	m_fdc->reset_w(BIT(data, 3));

	/* FDC ready */
	m_fdc->ready_w(!BIT(data, 5));

	/* ROM selection */
	m_romsel = !BIT(data, 6);

	/* printer strobe */
	m_centronics->write_strobe(BIT(data, 7));
}


//-------------------------------------------------
//  floppy formats
//-------------------------------------------------

void sega8_sf7000_device::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_SF7000_FORMAT);
}

static void sf7000_floppies(device_slot_interface &device)
{
	device.option_add("3ssdd", FLOPPY_3_SSDD);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void sega8_sf7000_device::device_add_mconfig(machine_config &config)
{
	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set(FUNC(sega8_sf7000_device::ppi_pa_r));
	m_ppi->out_pb_callback().set(m_cent_data_out, FUNC(output_latch_device::write));
	m_ppi->out_pc_callback().set(FUNC(sega8_sf7000_device::ppi_pc_w));
	m_ppi->tri_pc_callback().set_constant(0x8f); // /ROM SEL must be active after reset

	I8251(config, m_usart, 4.9152_MHz_XTAL / 2);
	m_usart->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_usart->dtr_handler().set(m_rs232, FUNC(rs232_port_device::write_dtr));
	m_usart->rts_handler().set(m_rs232, FUNC(rs232_port_device::write_rts));

	CLOCK(config, m_baudclk, 4.9152_MHz_XTAL / 1024);
	m_baudclk->signal_handler().set(m_usart, FUNC(i8251_device::write_txc));
	m_baudclk->signal_handler().append(m_usart, FUNC(i8251_device::write_rxc));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_usart, FUNC(i8251_device::write_rxd));
	m_rs232->cts_handler().set(m_usart, FUNC(i8251_device::write_cts));
	m_rs232->dsr_handler().set(m_usart, FUNC(i8251_device::write_dsr));

	UPD765A(config, m_fdc, 16_MHz_XTAL / 4); // clocked through SED9420C
	m_fdc->set_ready_line_connected(false);
	FLOPPY_CONNECTOR(config, m_floppy0, sf7000_floppies, "3ssdd", sega8_sf7000_device::floppy_formats);

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->busy_handler().set(FUNC(sega8_sf7000_device::write_centronics_busy));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	SOFTWARE_LIST(config, "flop_list").set_original("sf7000");
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sega8_sf7000_device - constructor
//-------------------------------------------------

sega8_sf7000_device::sega8_sf7000_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SEGA8_SF7000, tag, owner, clock),
	device_sega8_cart_interface(mconfig, *this),
	m_fdc(*this, "upd765"),
	m_floppy0(*this, "upd765:0"),
	m_ppi(*this, "upd9255"),
	m_usart(*this, "upd8251"),
	m_baudclk(*this, "baudclk"),
	m_rs232(*this, "rs232"),
	m_centronics(*this, "centronics"),
	m_cent_data_out(*this, "cent_data_out"),
	m_rom(*this, "ipl"),
	m_ram(*this, "ram", 0x10000, ENDIANNESS_LITTLE),
	m_baud(*this, "BAUD"),
	m_romsel(false),
	m_centronics_busy(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega8_sf7000_device::device_start()
{
	save_item(NAME(m_romsel));
	save_item(NAME(m_centronics_busy));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sega8_sf7000_device::device_reset()
{
	m_baudclk->set_unscaled_clock(4.9152_MHz_XTAL / (32 << m_baud->read()));
}


//-------------------------------------------------
//  memory
//-------------------------------------------------

bool sega8_sf7000_device::dsram() const
{
	return true;
}

uint8_t sega8_sf7000_device::read_cart(offs_t offset)
{
	if (m_romsel && offset < 0x4000)
		return m_rom[offset & (m_rom.length() - 1)];

	return m_ram[offset];
}

void sega8_sf7000_device::write_cart(offs_t offset, uint8_t data)
{
	m_ram[offset] = data;
}

uint8_t sega8_sf7000_device::read_ram(offs_t offset)
{
	return m_ram[0xc000 + offset];
}

void sega8_sf7000_device::write_ram(offs_t offset, uint8_t data)
{
	m_ram[0xc000 + offset] = data;
}


//-------------------------------------------------
//  I/O
//-------------------------------------------------

uint8_t sega8_sf7000_device::read_io(offs_t offset)
{
	if (BIT(offset, 4))
		return 0xff;

	switch ((offset >> 2) & 3)
	{
	case 0: return BIT(offset, 0) ? m_fdc->fifo_r() : m_fdc->msr_r();
	case 1: return m_ppi->read(offset & 3);
	case 2: return m_usart->read(offset & 1);
	}

	return 0xff;
}

void sega8_sf7000_device::write_io(offs_t offset, uint8_t data)
{
	if (BIT(offset, 4))
		return;

	switch ((offset >> 2) & 3)
	{
	case 0:
		if (BIT(offset, 0))
			m_fdc->fifo_w(data);
		break;
	case 1: m_ppi->write(offset & 3, data); break;
	case 2: m_usart->write(offset & 1, data); break;
	}
}
