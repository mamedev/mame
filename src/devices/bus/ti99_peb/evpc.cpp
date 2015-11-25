// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************
    SNUG Enhanced Video Processor Card (evpc)
    based on v9938 (may also be equipped with v9958)
    Can be used with TI-99/4A as an add-on card; internal VDP must be removed

    The SGCPU ("TI-99/4P") only runs with EVPC.
    Michael Zapf

    October 2010: Rewritten as device
    February 2012: Rewritten as class

    FIXME: Locks up on startup when HFDC is present. This can be avoided
    by using another controller (like bwg) or doing a soft reset.

*****************************************************************************/

#include "evpc.h"

#define EVPC_CRU_BASE 0x1400
#define VERBOSE 1
#define LOG logerror

#define NOVRAM_SIZE 256

snug_enhanced_video_device::snug_enhanced_video_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: ti_expansion_card_device(mconfig, TI99_EVPC, "SNUG Enhanced Video Processor Card", tag, owner, clock, "ti99_evpc", __FILE__),
	device_nvram_interface(mconfig, *this),
	m_dsrrom(nullptr),
	m_RAMEN(false),
	m_dsr_page(0),
	m_novram(nullptr)
{
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void snug_enhanced_video_device::nvram_default()
{
	memset(m_novram, 0, NOVRAM_SIZE);
}

//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

void snug_enhanced_video_device::nvram_read(emu_file &file)
{
	file.read(m_novram, NOVRAM_SIZE);
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

void snug_enhanced_video_device::nvram_write(emu_file &file)
{
	file.write(m_novram, NOVRAM_SIZE);
}


/*
    Read a byte in evpc DSR space
    0x4000 - 0x5eff   DSR (paged)
    0x5f00 - 0x5fef   NOVRAM
    0x5ff0 - 0x5fff   Palette
*/
READ8Z_MEMBER(snug_enhanced_video_device::readz)
{
	if (m_selected)
	{
		if ((offset & m_select_mask)==m_select_value)
		{
			if ((offset & 0x1ff0)==0x1ff0)              // Palette control
			{
				switch (offset & 0x000f)
				{
				case 0:
					/* Palette Read Address Register */
					*value = m_palette.write_index;
					break;

				case 2:
					/* Palette Read Color Value */
					if (m_palette.read)
					{
						switch (m_palette.state)
						{
						case 0:
							*value = m_palette.color[m_palette.read_index].red;
							break;
						case 1:
							*value = m_palette.color[m_palette.read_index].green;
							break;
						case 2:
							*value = m_palette.color[m_palette.read_index].blue;
							break;
						}
						m_palette.state++;
						if (m_palette.state == 3)
						{
							m_palette.state = 0;
							m_palette.read_index++;
						}
					}
					break;

				case 4:
					/* Palette Read Pixel Mask */
					*value = m_palette.mask;
					break;
				case 6:
					/* Palette Read Address Register for Color Value */
					if (m_palette.read)
						*value = 0;
					else
						*value = 3;
					break;
				}
			}
			else
			{
				if ((offset & 0x1f00)==0x1f00)
				{
					if (m_RAMEN)  // NOVRAM hides DSR
					{
						*value = m_novram[offset & 0x00ff];
					}
					else  // DSR
					{
						*value = m_dsrrom[(offset&0x1fff) | (m_dsr_page<<13)];
					}
				}
				else
				{
					*value = m_dsrrom[(offset&0x1fff) | (m_dsr_page<<13)];
				}
			}
		}
	}
}

/*
    Write a byte in evpc DSR space
    0x4000 - 0x5eff   DSR (paged)
    0x5f00 - 0x5fef   NOVRAM
    0x5ff0 - 0x5fff   Palette
*/
WRITE8_MEMBER(snug_enhanced_video_device::write)
{
	if (m_selected)
	{
		if ((offset & m_select_mask)==m_select_value)
		{
			if ((offset & 0x1ff0)==0x1ff0)
			{
				/* PALETTE */
				if (VERBOSE>5) LOG("palette write %04x <- %02x\n", offset&0xffff, data);
				switch (offset & 0x000f)
				{
				case 0x08:
					/* Palette Write Address Register */
					if (VERBOSE>5) LOG("EVPC palette address write (for write access)\n");
					m_palette.write_index = data;
					m_palette.state = 0;
					m_palette.read = 0;
					break;

				case 0x0a:
					/* Palette Write Color Value */
					if (VERBOSE>5) LOG("EVPC palette color write\n");
					if (!m_palette.read)
					{
						switch (m_palette.state)
						{
						case 0:
							m_palette.color[m_palette.write_index].red = data;
							break;
						case 1:
							m_palette.color[m_palette.write_index].green = data;
							break;
						case 2:
							m_palette.color[m_palette.write_index].blue = data;
							break;
						}
						m_palette.state++;
						if (m_palette.state == 3)
						{
							m_palette.state = 0;
							m_palette.write_index++;
						}
						//evpc_palette.dirty = 1;
					}
					break;

				case 0x0c:
					/* Palette Write Pixel Mask */
					if (VERBOSE>5) LOG("EVPC palette mask write\n");
					m_palette.mask = data;
					break;

				case 0x0e:
					/* Palette Write Address Register for Color Value */
					if (VERBOSE>5) LOG("EVPC palette address write (for read access)\n");
					m_palette.read_index = data;
					m_palette.state = 0;
					m_palette.read = 1;
					break;
				}
			}
			else
			{
				if ((offset & 0x1f00)==0x1f00)
				{
					if (m_RAMEN)
					{
						// NOVRAM
						m_novram[offset & 0x00ff] = data;
					}
				}
			}
		}
	}
}

/*
    The CRU read handler. Read EVPC DIP switches
    0: Video timing (PAL/NTSC)
    1: -
    2: charset
    3: RAM shift
    4: -
    5: -
    6: -
    7: DIP or NOVRAM
    Logic is inverted
*/
READ8Z_MEMBER(snug_enhanced_video_device::crureadz)
{
	if ((offset & 0xff00)==EVPC_CRU_BASE)
	{
		if ((offset & 0x00f0)==0) // offset 0 delivers bits 0-7 (address 00-0f)
		{
			*value = ~(ioport("EVPC-SW1")->read() | (ioport("EVPC-SW3")->read()<<2)
				| (ioport("EVPC-SW4")->read()<<3) | (ioport("EVPC-SW8")->read()<<7));
		}
	}
}

/*
    The CRU write handler.
    Bit 0: Turn on DSR ROM
    Bit 1: DSR page select (bit 0)
    Bit 2: -
    Bit 3: RAM enable
    Bit 4: DSR page select (bit 2)
    Bit 5: DSR page select (bit 1)
    Bit 6: -
    Bit 7: -
*/
WRITE8_MEMBER(snug_enhanced_video_device::cruwrite)
{
	if ((offset & 0xff00)==EVPC_CRU_BASE)
	{
		int bit = (offset >> 1) & 0x0f;
		switch (bit)
		{
		case 0:
			m_selected = (data!=0);
			if (VERBOSE>4) LOG("evpc: Map DSR = %d\n", m_selected);
			break;

		case 1:
			if (data!=0)
				m_dsr_page |= 1;
			else
				m_dsr_page &= ~1;
			break;

		case 3:
			m_RAMEN = (data!=0);
			break;

		case 4:
			if (data!=0)
				m_dsr_page |= 4;
			else
				m_dsr_page &= ~4;
			break;

		case 5:
			if (data!=0)
				m_dsr_page |= 2;
			else
				m_dsr_page &= ~2;
			break;

		case 2:
		case 6:
		case 7:
			break;
		}
	}
}

void snug_enhanced_video_device::device_start()
{
	m_dsrrom = memregion(DSRROM)->base();
	m_novram = global_alloc_array(UINT8, NOVRAM_SIZE);
}

void snug_enhanced_video_device::device_reset()
{
	if (VERBOSE>5) LOG("evpc: reset\n");
	m_select_mask = 0x7e000;
	m_select_value = 0x74000;
	m_dsr_page = 0;
	m_RAMEN = false;
	m_selected = false;
}

void snug_enhanced_video_device::device_stop()
{
	global_free_array(m_novram);
}

ROM_START( ti99_evpc )
	ROM_REGION(0x10000, DSRROM, 0)
	ROM_LOAD("evpcdsr.bin", 0, 0x10000, CRC(a062b75d) SHA1(6e8060f86e3bb9c36f244d88825e3fe237bfe9a9)) /* evpc DSR ROM */
ROM_END

/*
    Input ports for the EPVC
*/
INPUT_PORTS_START( ti99_evpc )
	PORT_START( "EVPC-SW1" )
	PORT_DIPNAME( 0x01, 0x00, "EVPC video mode" ) PORT_CONDITION( "EVPC-SW8", 0x01, EQUALS, 0x00 )
		PORT_DIPSETTING(    0x00, "PAL" )
		PORT_DIPSETTING(    0x01, "NTSC" )

	PORT_START( "EVPC-SW3" )
	PORT_DIPNAME( 0x01, 0x00, "EVPC charset" ) PORT_CONDITION( "EVPC-SW8", 0x01, EQUALS, 0x00 )
		PORT_DIPSETTING(    0x00, DEF_STR( International ))
		PORT_DIPSETTING(    0x01, DEF_STR( German ))

	PORT_START( "EVPC-SW4" )
	PORT_DIPNAME( 0x01, 0x00, "EVPC VDP RAM" ) PORT_CONDITION( "EVPC-SW8", 0x01, EQUALS, 0x00 )
		PORT_DIPSETTING(    0x00, "shifted" )
		PORT_DIPSETTING(    0x01, "not shifted" )

	PORT_START( "EVPC-SW8" )
	PORT_DIPNAME( 0x01, 0x00, "EVPC Configuration" )
		PORT_DIPSETTING(    0x00, "DIP" )
		PORT_DIPSETTING(    0x01, "NOVRAM" )
INPUT_PORTS_END

const rom_entry *snug_enhanced_video_device::device_rom_region() const
{
	return ROM_NAME( ti99_evpc );
}

ioport_constructor snug_enhanced_video_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ti99_evpc);
}

const device_type TI99_EVPC = &device_creator<snug_enhanced_video_device>;
