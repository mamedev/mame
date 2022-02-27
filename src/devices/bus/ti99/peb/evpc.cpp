// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************
    SNUG Enhanced Video Processor Card (EVPC)

    This is an expansion card with an own v9938 video processor on board.
    Later releases (EVPC2) can also be equipped with a v9958.

    The EVPC is intended to be used
    1. with the TI-99/4A console
    2. with the SGCPU

    For option 1, the console-internal VDP (TMS9928A) must be removed. This,
    however, raises a problem, because the video interrupt must now be send
    from the EVPC in the Peripheral Expansion Box to the console, but there is
    no line in the PEB cable for this purpose.

    To solve this issue, a separate cable is led from the EVPC to the console
    which delivers the video interrupt, and also the GROM clock which is also
    lost due to the removal of the internal VDP.

    The SGCPU requires this card, as it does not offer any video processor.
    In this configuration, the video interrupt cable is not required.

    Also, the SGCPU does not offer a socket for the sound chip of the TI
    console, and accordingly, the EVPC also gives the sound chip a new home.
    Thus we assume that in the TI console (option 1) the sound chip has
    also been removed.

    The EVPC has one configuration option:
       VRAM may be set to 128K or 192K.

    Important note: The DSR (firmware) of the EVPC expects a memory expansion
    to be present; otherwise, the configuration (using CALL EVPC) will crash.
    There is no warning if the 32K expansion is not present.

    Michael Zapf

*****************************************************************************/

#include "emu.h"
#include "evpc.h"

#include "speaker.h"

#define LOG_WARN        (1U<<1)   // Warnings
#define LOG_CRU         (1U<<2)   // CRU access
#define LOG_MEM         (1U<<3)   // Memory access
#define LOG_ADDRESS     (1U<<4)   // Addresses

#define VERBOSE ( LOG_GENERAL | LOG_WARN )

#include "logmacro.h"
#define EVPC_SCREEN_TAG      "screen"

DEFINE_DEVICE_TYPE(TI99_EVPC, bus::ti99::peb::snug_enhanced_video_device, "ti99_evpc", "SNUG Enhanced Video Processor Card")

namespace bus::ti99::peb {

#define NOVRAM_SIZE 256
#define EVPC_CRU_BASE 0x1400
#define SOUNDCHIP_TAG "soundchip"

snug_enhanced_video_device::snug_enhanced_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
	device_t(mconfig, TI99_EVPC, tag, owner, clock),
	device_ti99_peribox_card_interface(mconfig, *this),
	device_nvram_interface(mconfig, *this),
	m_dsr_page(0),
	m_inDsrArea(false),
	m_novram_accessed(false),
	m_palette_accessed(false),
	m_RAMEN(false),
	m_sound_accessed(false),
	m_video_accessed(false),
	m_intlevel(0),
	m_dsrrom(nullptr),
	m_novram(nullptr),
	m_video(*this, TIGEN_V9938_TAG),
	m_sound(*this, SOUNDCHIP_TAG),
	m_colorbus(*this, COLORBUS_TAG),
	m_console_conn(*this, ":" TI99_EVPC_CONN_TAG)
{
}

void snug_enhanced_video_device::setaddress_dbin(offs_t offset, int state)
{
	// Do not allow setaddress for the debugger. It will mess up the
	// setaddress/memory access pairs when the CPU enters wait states.
	if (machine().side_effects_disabled()) return;

	LOGMASKED(LOG_ADDRESS, "set address %04x, %s\n", offset, (state==ASSERT_LINE)? "read" : "write");

	m_address = offset;
	bool reading = (state==ASSERT_LINE);
	int offbase = (m_address & 0x7fc01); // The 7 represents the AMA/B/C lines

	// Sound
	m_sound_accessed = ((m_address & 0x7ff01)==0x78400) && !reading;

	// Video space
	// 8800 / 8802 / 8804 / 8806
	// 8c00 / 8c02 / 8c04 / 8c06
	//
	// Bits 1000 1w00 0000 0xx0
	// Mask 1111 1000 0000 0001
	m_video_accessed = ((offbase==0x78800) && reading) || ((offbase==0x78c00) && !reading);

	// Read a byte in evpc DSR space
	// 0x4000 - 0x5eff   DSR (paged)
	// 0x5f00 - 0x5fef   NOVRAM
	// 0x5ff0 - 0x5fff   Palette
	m_inDsrArea = in_dsr_space(m_address, true);
	m_novram_accessed = ((m_address & 0x7ff00)==0x75f00);
	m_palette_accessed = ((m_address & 0x7fff0)==0x75ff0);

	// Note that we check the selection in reverse order so that the overlap is avoided
}

//-------------------------------------------------
//  nvram_default - called to initialize NVRAM to
//  its default state
//-------------------------------------------------

void snug_enhanced_video_device::nvram_default()
{
	memset(m_novram.get(), 0, NOVRAM_SIZE);
}

//-------------------------------------------------
//  nvram_read - called to read NVRAM from the
//  .nv file
//-------------------------------------------------

bool snug_enhanced_video_device::nvram_read(util::read_stream &file)
{
	size_t actual;
	return !file.read(m_novram.get(), NOVRAM_SIZE, actual) && actual == NOVRAM_SIZE;
}

//-------------------------------------------------
//  nvram_write - called to write NVRAM to the
//  .nv file
//-------------------------------------------------

bool snug_enhanced_video_device::nvram_write(util::write_stream &file)
{
	size_t actual;
	return !file.write(m_novram.get(), NOVRAM_SIZE, actual) && actual == NOVRAM_SIZE;
}

/*
    Read a byte in evpc DSR space, NOVRAM, Palette, or video
    0x4000 - 0x5eff   DSR (paged)
    0x5f00 - 0x5fef   NOVRAM
    0x5ff0 - 0x5fff   Palette (5ff0, 5ff2, 5ff4, 5ff6)
*/
void snug_enhanced_video_device::readz(offs_t offset, uint8_t *value)
{
	if (m_selected && m_inDsrArea)
	{
		if (m_palette_accessed)
		{
			switch (m_address & 0x000f)
			{
			case 0:
				// Palette Read Address Register
				*value = m_palette.write_index;
				break;

			case 2:
				// Palette Read Color Value
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
				// Palette Read Pixel Mask
				*value = m_palette.mask;
				break;
			case 6:
				// Palette Read Address Register for Color Value
				if (m_palette.read)
					*value = 0;
				else
					*value = 3;
				break;
			}
			return;
		}

		if (m_novram_accessed && m_RAMEN)
		{
			// NOVRAM
			*value = m_novram[offset & 0x00ff];
			return;
		}

		// DSR space
		*value = m_dsrrom[(offset & 0x1fff) | (m_dsr_page<<13)];
		return;
	}

	if (m_video_accessed)
	{
		*value = m_video->read(m_address>>1);
	}
}

/*
    Write a byte in evpc DSR space
    0x4000 - 0x5eff   DSR (paged)
    0x5f00 - 0x5fef   NOVRAM
    0x5ff0 - 0x5fff   Palette (5ff8, 5ffa, 5ffc, 5ffe)
*/
void snug_enhanced_video_device::write(offs_t offset, uint8_t data)
{
	if (m_selected && m_inDsrArea)
	{
		if (m_palette_accessed)
		{
			// Palette
			LOGMASKED(LOG_MEM, "palette write %04x <- %02x\n", offset&0xffff, data);
			switch (m_address & 0x000f)
			{
			case 0x08:
				// Palette Write Address Register
				LOGMASKED(LOG_MEM, "EVPC palette address write (for write access)\n");
				m_palette.write_index = data;
				m_palette.state = 0;
				m_palette.read = 0;
				break;

			case 0x0a:
				// Palette Write Color Value
				LOGMASKED(LOG_MEM, "EVPC palette color write\n");
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
				// Palette Write Pixel Mask
				LOGMASKED(LOG_MEM, "EVPC palette mask write\n");
				m_palette.mask = data;
				break;

			case 0x0e:
				// Palette Write Address Register for Color Value
				LOGMASKED(LOG_MEM, "EVPC palette address write (for read access)\n");
				m_palette.read_index = data;
				m_palette.state = 0;
				m_palette.read = 1;
				break;

			}
			return;
		}

		if (m_novram_accessed && m_RAMEN)
		{
			// NOVRAM
			m_novram[offset & 0x00ff] = data;
			return;
		}
	}

	if (m_video_accessed)
	{
		m_video->write(m_address>>1, data);
	}

	if (m_sound_accessed)
	{
		m_sound->write(data);
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
void snug_enhanced_video_device::crureadz(offs_t offset, uint8_t *value)
{
	if ((offset & 0xff00)==EVPC_CRU_BASE)
	{
		switch ((offset>>1) & 7)
		{
		case 0: *value = ~(ioport("EVPC-SW1")->read()); break;
		case 2: *value = ~(ioport("EVPC-SW3")->read()); break;
		case 3: *value = ~(ioport("EVPC-SW4")->read()); break;
		case 7: *value = ~(ioport("EVPC-SW8")->read()); break;
		default: *value = ~0; break;
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
void snug_enhanced_video_device::cruwrite(offs_t offset, uint8_t data)
{
	if ((offset & 0xff00)==EVPC_CRU_BASE)
	{
		int bit = (offset >> 1) & 0x0f;
		switch (bit)
		{
		case 0:
			m_selected = (data!=0);
			LOGMASKED(LOG_CRU, "Map DSR = %d\n", m_selected);
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

/*
    READY line for the sound chip
*/
WRITE_LINE_MEMBER( snug_enhanced_video_device::ready_line )
{
	m_slot->set_ready(state);
}

void snug_enhanced_video_device::device_start()
{
	m_dsrrom = memregion(TI99_DSRROM)->base();
	m_novram = std::make_unique<uint8_t[]>(NOVRAM_SIZE);
	save_item(NAME(m_address));
	save_item(NAME(m_dsr_page));
	save_item(NAME(m_inDsrArea));
	save_item(NAME(m_novram_accessed));
	save_item(NAME(m_palette_accessed));
	save_item(NAME(m_RAMEN));
	save_item(NAME(m_sound_accessed));
	save_item(NAME(m_video_accessed));
	save_item(NAME(m_intlevel));
}

void snug_enhanced_video_device::device_reset()
{
	m_dsr_page = 0;
	m_RAMEN = false;
	m_selected = false;
	m_video->set_vram_size((ioport("EVPC-MEM")->read()==0)? 0x20000 : 0x30000);
}

void snug_enhanced_video_device::device_stop()
{
	m_novram = nullptr;
}

/*
    This is the extra cable running from the EVPC card right into the TI console.
    It delivers the VDP interrupt and the GROM clock.

    For the SGCPU, the signal is delivered by the LCP line.
*/
WRITE_LINE_MEMBER( snug_enhanced_video_device::video_interrupt_in )
{
	// This method is frequently called without level change, so we only
	// react on changes
	if (state != m_intlevel)
	{
		m_intlevel = state;
		if (m_console_conn != nullptr) m_console_conn->vclock_line(state);
		else m_slot->lcp_line(state);
	}
}

ROM_START( ti99_evpc )
	ROM_REGION(0x10000, TI99_DSRROM, 0)
	ROM_LOAD("evpc_dsr.u21", 0, 0x10000, CRC(a062b75d) SHA1(6e8060f86e3bb9c36f244d88825e3fe237bfe9a9)) /* evpc DSR ROM */
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

	PORT_START( "EVPC-MEM" )
	PORT_CONFNAME( 0x01, 0x00, "EVPC video memory" )
		PORT_DIPSETTING(    0x00, "128K" )
		PORT_DIPSETTING(    0x01, "192K" )
INPUT_PORTS_END

const tiny_rom_entry *snug_enhanced_video_device::device_rom_region() const
{
	return ROM_NAME( ti99_evpc );
}

ioport_constructor snug_enhanced_video_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ti99_evpc);
}

void snug_enhanced_video_device::device_add_mconfig(machine_config& config)
{
	// video hardware
	V9938(config, m_video, XTAL(21'477'272)); // typical 9938 clock, not verified

	m_video->int_cb().set(FUNC(snug_enhanced_video_device::video_interrupt_in));
	m_video->set_screen(EVPC_SCREEN_TAG);
	m_video->set_vram_size(0x20000); // gets changed at device_reset, but give it a default value to avoid assert
	screen_device& screen(SCREEN(config, EVPC_SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(21'477'272),
		v99x8_device::HTOTAL,
		0,
		v99x8_device::HVISIBLE - 1,
		v99x8_device::VTOTAL_NTSC * 2,
		v99x8_device::VERTICAL_ADJUST * 2,
		v99x8_device::VVISIBLE_NTSC * 2 - 1 - v99x8_device::VERTICAL_ADJUST * 2);
	screen.set_screen_update(TIGEN_V9938_TAG, FUNC(v99x8_device::screen_update));

	// Sound hardware
	SPEAKER(config, "sound_out").front_center();
	sn94624_device& soundgen(SN94624(config, SOUNDCHIP_TAG, 3579545/8));
	soundgen.ready_cb().set(FUNC(snug_enhanced_video_device::ready_line));
	soundgen.add_route(ALL_OUTPUTS, "sound_out", 0.75);

	// Mouse connected to the color bus of the v9938; default: none
	V9938_COLORBUS(config, m_colorbus, 0, ti99_colorbus_options, nullptr);
}

} // end namespace bus::ti99::peb
