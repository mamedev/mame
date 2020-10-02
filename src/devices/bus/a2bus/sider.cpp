// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    sider.cpp

    Implementation of the First Class Peripherals / Advanced Tech Services / Xebec Sider cards

    Sider 1: based on Xebec SASI hardware, with Xebec firmware.  Requires HDD with 256 byte sectors
    and special formatting - not compatible with standard .po/.hdv type images.

    Sider 2: Same hardware, new firmware that's fully ProDOS compliant including 512 byte sectors.
    Sectors are interleaved: the first byte is the byte that would be at 0, the second byte is the byte that would be at $100,
    the third is the byte at $1, the fourth at $101, and so on.  So this is also not compatible with standard images.

    This command-line program will convert a standard image to work with Sider 2.  First parameter is the input filename,
    second parameter is the output filename.  No error checking is done, if the input file doesn't exist it'll probably crash.

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    static char block[512];
    static char blockout[512];

    int main(int argc, char *argv[])
    {
      FILE *in, *out;

      in = fopen(argv[1], "rb");
      out = fopen(argv[2], "wb");

      int len = 0;
      fseek(in, 0, SEEK_END);
      len = ftell(in);
      fseek(in, 0, SEEK_SET);

      for (int bnum = 0; bnum < (len / 512); bnum++)
      {
        fread(block, 512, 1, in);
        for (int byte = 0; byte < 256; byte++)
        {
          blockout[byte*2] = block[byte];
          blockout[(byte*2)+1] = block[byte+256];
        }
        fwrite(blockout, 512, 1, out);
        printf("block %d\n", bnum);
      }

      fclose(out);
      fclose(in);
      return 0;
    }

    --------------------------------------

    $C0(n+8)X space:
    $0: read state / write latch
    $1: read data / write control

    State:
    b3: /Busy
    b4: /Message
    b5: I/O
    b6: C/D
    b7: REQ

    Control:
    b4: drive contents of output latch onto the SASI bus (reverse logic; latch is driven when this bit is clear)
    b5: reset bus
    b6: /BSY
    b7: /SEL

*********************************************************************/

#include "emu.h"
#include "sider.h"
#include "bus/scsi/scsihd.h"
#include "bus/scsi/s1410.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(A2BUS_SIDER2, a2bus_sider2card_device, "a2sider2", "First Class Peripherals Sider 2 SASI Card")
DEFINE_DEVICE_TYPE(A2BUS_SIDER1, a2bus_sider1card_device, "a2sider1", "First Class Peripherals Sider 1 SASI Card")

#define SASI_ROM_REGION  "sasi_rom"

ROM_START( sider2 )
	ROM_REGION(0x800, SASI_ROM_REGION, 0)
	ROM_LOAD( "atsv22_a02a9baad2262a8a49ecea843f602124.bin", 0x000000, 0x000800, CRC(97773a0b) SHA1(8d0a5d6ce3b9a236771126033c4aba6c0cc5e704) )
ROM_END

ROM_START( sider1 )
	ROM_REGION(0x800, SASI_ROM_REGION, 0)
	ROM_LOAD( "xebec-103684c-1986.bin", 0x000000, 0x000800, CRC(9e62e15f) SHA1(7f50b5e00cac4960204f50448a6e2d623b1a41e2) )
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_sider_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, m_sasibus);
	NSCSI_CONNECTOR(config, "sasibus:0", default_scsi_devices, "s1410", false);
	NSCSI_CONNECTOR(config, "sasibus:1", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "sasibus:2", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "sasibus:3", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "sasibus:4", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "sasibus:5", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "sasibus:6", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "sasibus:7", default_scsi_devices, "scsicb", true).option_add_internal("scsicb", NSCSI_CB);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_sider2card_device::device_rom_region() const
{
	return ROM_NAME( sider2 );
}

const tiny_rom_entry *a2bus_sider1card_device::device_rom_region() const
{
	return ROM_NAME( sider1 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_sider_device::a2bus_sider_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_sasibus(*this, "sasibus"),
	m_sasi(*this, "sasibus:7:scsicb"),
	m_rom(*this, SASI_ROM_REGION),
	m_latch(0xff),
	m_control(0)
{
}

a2bus_sider2card_device::a2bus_sider2card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_sider_device(mconfig, A2BUS_SIDER2, tag, owner, clock)
{
}

a2bus_sider1card_device::a2bus_sider1card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_sider_device(mconfig, A2BUS_SIDER1, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_sider_device::device_start()
{
	save_item(NAME(m_latch));
	save_item(NAME(m_control));
}

void a2bus_sider_device::device_reset()
{
	m_latch = 0xff;
	m_control = 0;
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_sider_device::read_c0nx(uint8_t offset)
{
	u8 rv = 0;

	switch (offset)
	{
		case 0:
	//      b3: /Busy
	//      b4: /Message
	//      b5: I/O
	//      b6: C/D
	//      b7: REQ
			rv |= m_sasi->req_r() ? 0x80 : 0;
			rv |= m_sasi->cd_r() ? 0x40 : 0;
			rv |= m_sasi->io_r() ? 0x20 : 0;
			rv |= m_sasi->msg_r() ? 0x10 : 0;
			rv |= m_sasi->bsy_r() ? 0x08 : 0;
			return rv;

		case 1:
			rv = m_sasi->read();
			if ((m_sasi->req_r()) && (m_sasi->io_r()))
			{
				m_sasi->ack_w(1);
				m_sasi->ack_w(0);
			}
			return rv;

		default:
			logerror("Read c0n%x (%s)\n", offset, machine().describe_context().c_str());
			break;
	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_sider_device::write_c0nx(uint8_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_latch = data;
			if (!(m_control & 0x10))
			{
				m_sasi->write(m_latch);

				if (m_sasi->req_r())
				{
					m_sasi->ack_w(1);
					m_sasi->write(0);   // stop driving the data lines
					m_sasi->ack_w(0);
				}
			}
			break;

		case 1:
//          b4: drive contents of output latch onto the SASI bus
//          b5: reset bus
//          b6: /BSY
//          b7: /SEL (error in note on A2DP's schematic; BSY and SEL are the same polarity from the 6502's POV)
			m_control = data;
			m_sasi->sel_w((data & 0x80) ? 1 : 0);
			m_sasi->bsy_w((data & 0x40) ? 1 : 0);
			if (data & 0x20)
			{
				m_sasibus->reset();
			}
			else if (!(data & 0x10))
			{
				m_sasi->write(m_latch);
			}
			break;

		default:
			logerror("Write %02x to c0n%x (%s)\n", data, offset, machine().describe_context().c_str());
			break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t a2bus_sider_device::read_cnxx(uint8_t offset)
{
	// slot images at $700
	return m_rom[offset + 0x700];
}

void a2bus_sider_device::write_cnxx(uint8_t offset, uint8_t data)
{
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

uint8_t a2bus_sider_device::read_c800(uint16_t offset)
{
	return m_rom[offset];
}

/*-------------------------------------------------
    write_c800 - called for writes to this card's c800 space
-------------------------------------------------*/
void a2bus_sider_device::write_c800(uint16_t offset, uint8_t data)
{
}
