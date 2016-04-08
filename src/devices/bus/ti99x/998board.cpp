// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************

    TI-99/8 main board logic

    This component implements the address decoder and mapper logic from the
    TI-99/8 console.

    The TI-99/8 defines a "logical address map" with 64 KiB (according to the
    16 address bits) and a "physical address map" with 16 MiB (according to the
    24 address bits of the mapper). Note that the mapper only uses 16 outgoing
    address lines and multiplexes the address bytes.

    Note: The TI-99/8's internal codename was "Armadillo"


    +-------+                        +--------+
    |  CPU  |========LogAddrBus======| Mapper |====PhysAddrBus==========
    |  TMS  |          ||            | AMIGO  |            ||
    |  9995 |      +----------+      |        |       +----------+
    |       |      | Logical  |      +--------+       | Physical |
    +-------+      | space    |            |          | space    |
                   | decoder  |            |          | decoder  |
                   | VAQUERRO |            |          | MOFETTA  |
                   +----------+            |          +----------+
                       |                   |               |
                   +--------------------+  |          +---------------------+
                   | Devices            |  |          | Devices             |
                   |            +-------+  |          |                     |
                   | ROM0       | SRAM  |  |          | DRAM  (POLLO)       |
                   | Video      | ----  |  |          | ROM1                |
                   | Speech     | Maps--+--+          | Cartridge port      |
                   | GROM       +-------+             | PEB                 |
                   | Sound              |             | Hexbus (OSO)        |
                   +--------------------+             +---------------------+

    Custom chips
    ------------
    The chipset of the TI-99/8 consists of five specifically programmed chips.
    All are nicknamed after some Spanish words (albeit sometimes misspelled)

    VAQUERRO: Logical Address Space decoder  ("Vaquero" = "Cowboy")
    MOFETTA : Physical Address Space decoder ("Mofeta"  = "Skunk")
    AMIGO   : Mapper                         ("Amigo"   = "Friend")
    OSO     : Hexbus adapter                 ("Oso"     = "Bear")
    POLLO   : DRAM controller (Not emulated) ("Pollo"   = "Chicken")

    See the comments for the respective chip implementation for details.


    ROM contents
    ------------
    The ROM0 chip is accessible at addresses 0000-1FFF in the logical address
    space of the compatibility mode. It contains the GPL interpreter. In
    native mode the ROM0 chip is invisible.

      ROM0
      offset  Logical address     Name
      -----------------------------------
      0000    0000-1FFF           ROM0


    The ROM1 chip contains 32 KiB of various system software. It is located in
    the physical address space, so it must be mapped into the logical address
    space by defining an appropriate map.

      ROM1
      offset  Physical address            Name
      ----------------------------------------------------------
      0000    FFA000-FFDFFF               ROM1
      4000    FF4000-FF5FFF @CRU>2700     Text-to-speech ROM/DSR
      6000    FF4000-FF5FFF @CRU>1700     Hexbus DSR

    The DSR portions have to be selected via the CRU bits >1700 or >2700.


    CRU map (I/O address space)
    ===========================
    0000-003e: TMS9901 system interface (see ti99_8.c)
    1700-17fe: Hexbus
    2000-26fe: Future external devices
    2700-27fe: Additional ROM ("internal DSR")
    2702     : System reset (when set to 1)
    2800-3ffe: Future external devices
    4000-fffe: Future external devices

    The TMS9995 offers the full 15-bit CRU address space. Devices designed for
    the TI-99/4A should only be accessed in the area 1000-1ffe. They will (by
    design) incompletely decode the CRU address and be mirrored in the higher
    areas.

    Note that the cartridge port of the TI-99/8 offers support for 16K ROM
    cartridges, but lacks CRU support.

    Michael Zapf, October 2010
    February 2012: Rewritten as class
    March 2016: Redesigned for custom chip emulation

    Informations taken from
    [1] ARMADILLO PRODUCT SPECIFICATIONS
    [2] TI-99/8 Graphics Programming Language interpreter

***************************************************************************/

// TODO:
// - PEB

#include "998board.h"

#define TRACE_CRU 0
#define TRACE_ADDRESS 0
#define TRACE_MAP 0
#define TRACE_OSO 0
#define TRACE_DECODE 0
#define TRACE_READY 0
#define TRACE_MEM 0
#define TRACE_CLOCK 0
#define TRACE_DETAIL 0
#define TRACE_MOFETTA 0
#define TRACE_AMIGO 0
#define TRACE_WS 0
#define TRACE_CPURY 0
#define TRACE_GROM 0
#define TRACE_PUNMAP 0

mainboard8_device::mainboard8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MAINBOARD8, "TI-99/8 Mainboard", tag, owner, clock, "ti998_mainboard", __FILE__),
	m_A14_set(false),
	m_pending_write(false),
	m_speech_ready(true),
	m_sound_ready(true),
	m_pbox_ready(true),
	m_ready(*this),
	m_console_reset(*this),
	m_hold_line(*this),
	m_vaquerro(*this, VAQUERRO_TAG),
	m_mofetta(*this, MOFETTA_TAG),
	m_amigo(*this, AMIGO_TAG),
	m_oso(*this, OSO_TAG)
{
}

// =============== CRU bus access ==================

READ8Z_MEMBER(mainboard8_device::crureadz)
{
	m_peb->crureadz(space, offset, value);
}

/*
    CRU handling. Mofetta is the only chip that bothers to handle it, beside the PEB
*/
WRITE8_MEMBER(mainboard8_device::cruwrite)
{
	m_mofetta->cruwrite(space, offset, data);
	m_peb->cruwrite(space, offset, data);
}

// =============== Memory bus access ==================

WRITE_LINE_MEMBER( mainboard8_device::dbin_in )
{
	m_dbin_level = (line_state)state;
}

SETOFFSET_MEMBER( mainboard8_device::setoffset )
{
	if (TRACE_ADDRESS) logerror("set %s %04x\n", (m_dbin_level==ASSERT_LINE)? "R" : "W", offset);

	// No data is waiting on the data bus
	m_pending_write = false;

	// Memory cycle begins
	m_vaquerro->memen_in(ASSERT_LINE);
	m_amigo->memen_in(ASSERT_LINE);

	// Save the logical address
	m_logical_address = offset;
	m_physical_address = 0;

	// In TI's bit order, A14 is the second line from the right side (2^1)
	m_A14_set = ((m_logical_address & 2)!=0); // Needed for clock_in

	// Check for match in logical space
	m_vaquerro->set_address(space, m_logical_address, m_dbin_level);

	// Select GROMs if addressed
	select_groms();

	// Speech select lines will always be asserted/cleared as soon as the address is available
	m_speech->wsq_w((m_vaquerro->spwt_out() == ASSERT_LINE)? FALSE : TRUE);
	m_speech->rsq_w((m_vaquerro->sprd_out() == ASSERT_LINE)? FALSE : TRUE);

	// If it is a logical space address, tell the mapper to stay inactive
	line_state lasreq = (line_state)m_vaquerro->lascsq_out();
	m_amigo->lascs_in(lasreq);
	m_mofetta->lascs_in(lasreq);

	// Need to set the address in any case so that the lines can be cleared
	m_amigo->set_address(space, m_logical_address);

	// AMIGO is the one to control the READY line to the CPU
	// MOFETTA does not contribute to READY
	m_ready(m_amigo->cpury_out());
}

WRITE_LINE_MEMBER( mainboard8_device::reset_console )
{
	m_console_reset(state);
}

WRITE_LINE_MEMBER( mainboard8_device::hold_cpu )
{
	m_hold_line(state);
}

/*
    HOLD Acknowledge from the CPU
*/
WRITE_LINE_MEMBER( mainboard8_device::holda_line )
{
	m_amigo->holda_in(state);
}

/*
    Clock line from the CPU. Forward to the custom chips.
*/
WRITE_LINE_MEMBER( mainboard8_device::clock_in )
{
	if (TRACE_CLOCK) logerror("CLKOUT = %d\n", state);

	// Propagate to Vaquerro; may change GGRDY (trailing edge) and the GROM select lines
	m_vaquerro->clock_in((line_state)state);

	// Set the incoming ready line of Amigo (Mapper) before the clock
	bool readycomb = ((m_vaquerro->ggrdy_out()==ASSERT_LINE) && m_speech_ready && m_sound_ready && m_pbox_ready);
	m_amigo->srdy_in(readycomb? ASSERT_LINE : CLEAR_LINE);

	// This may change the incoming READY lines of Vaquerro
	if (state==CLEAR_LINE) select_groms();

	m_amigo->clock_in((line_state)state);

	// Mofetta only needs the clock to produce the GROM clock
	m_mofetta->clock_in(state);

	m_mofetta->skdrcs_in(m_amigo->skdrcs_out());

	int gromclk = m_mofetta->gromclk_out();

	if (gromclk != m_gromclk)   // when it changed, propagate to the GROMs
	{
		m_gromclk = gromclk;
		for (int i=0; i < 8; i++)
		{
			if (i < 3)
			{
				m_sgrom[i]->gclock_in(gromclk);
				m_p3grom[i]->gclock_in(gromclk);
			}
			m_tsgrom[i]->gclock_in(gromclk);
			m_p8grom[i]->gclock_in(gromclk);
		}
		m_gromport->gclock_in(gromclk);
	}

	// Check video for writing
	if (m_pending_write && m_vaquerro->vdpwt_out()==ASSERT_LINE)
	{
		if (m_A14_set) m_video->register_write(*m_space, 0, m_latched_data);
		else m_video->vram_write(*m_space, 0, m_latched_data);
		m_pending_write = false;
		if (TRACE_MEM) logerror("Write %04x (video) <- %02x\n", m_logical_address, m_latched_data);
		cycle_end();
		return;
	}

	// Propagate the READY signal
	m_ready(m_amigo->cpury_out());

	// In case we're reading, the CPU will now do the READ operation.
	// Otherwise we must do the write operation now which we postponed before.

	if (m_pending_write && (state==CLEAR_LINE))
	{
		if (m_amigo->skdrcs_out()==ASSERT_LINE)
		{
			m_dram[m_physical_address & 0xffff] = m_latched_data;
			m_pending_write = false;
			if (TRACE_MEM) logerror("Write %04x (phys %06x, DRAM) <- %02x\n", m_logical_address, m_physical_address, m_latched_data);
		}

		if (m_mofetta->alccs_out()==ASSERT_LINE)
		{
			m_oso->write(*m_space, m_physical_address>>1, m_latched_data);
			m_pending_write = false;
			if (TRACE_MEM) logerror("Write %04x (phys %06x, OSO) <- %02x\n", m_logical_address, m_physical_address, m_latched_data);
		}

		if (m_mofetta->cmas_out()==ASSERT_LINE)
		{
			m_gromport->romgq_line(ASSERT_LINE);
			m_gromport->write(*m_space, m_physical_address & 0x3fff, m_latched_data);
			m_pending_write = false;
			if (TRACE_MEM) logerror("Write %04x (phys %06x, cartridge) <- %02x\n", m_logical_address, m_physical_address, m_latched_data);
		}
		else
		{
			m_gromport->romgq_line(CLEAR_LINE);
		}

		if (m_mofetta->dbc_out()==ASSERT_LINE)
		{
			m_peb->write(*m_space, m_physical_address, m_latched_data);
			m_pending_write = false;
			if (TRACE_MEM) logerror("Write %04x (phys %06x, PEB) <- %02x\n", m_logical_address, m_physical_address, m_latched_data);
		}
	}

	if (m_dbin_level==CLEAR_LINE && !m_pending_write)       // Memory cycle ends
		cycle_end();
}

void mainboard8_device::select_groms()
{
	// Select the GROM libs
	// Note that we must also deselect them again, so we have to visit each
	// one of them

	int select = m_vaquerro->gromcs_out();

	// Avoid to be called too often; this would have a bad penalty on emulation performance
	// This simple check actually increases bench performance from 120% to 240%
	if (select != m_prev_grom)
	{
		m_prev_grom = select;
		int lines = (m_dbin_level==ASSERT_LINE)? GROM_M_LINE : 0;
		if (m_A14_set) lines |= GROM_MO_LINE;

		for (int i=0; i < 8; i++)
		{
			if (i < 3)
			{
				m_sgrom[i]->set_lines(*m_space, lines, select & SGMSEL);
				m_p3grom[i]->set_lines(*m_space, lines, select & P3GSEL);
			}
			m_tsgrom[i]->set_lines(*m_space, lines, select & TSGSEL);
			m_p8grom[i]->set_lines(*m_space, lines, select & P8GSEL);
		}
		// Write to the cartridge port. The GROMs on cartridges are accesses as system GROMs
		if (select & SGMSEL) m_gromport->romgq_line(CLEAR_LINE);
		m_gromport->set_gromlines(*m_space, lines, select & SGMSEL);
	}

	// If we're planning to write to the GROMs, let's do it right now
	if (select !=0 && m_pending_write)
	{
		m_pending_write = false;
		switch (select)
		{
		case SGMSEL:
			for (int i=0; i < 3; i++)
			{
				m_sgrom[i]->write(*m_space, 0, m_latched_data);
			}
			if (TRACE_MEM) logerror("Write GS <- %02x\n", m_latched_data);
			m_gromport->write(*m_space, 0, m_latched_data);
			break;

		case TSGSEL:
			for (int i=0; i < 8; i++)
			{
				m_tsgrom[i]->write(*m_space, 0, m_latched_data);
			}
			if (TRACE_MEM) logerror("Write GT <- %02x\n", m_latched_data);
			break;

		case P8GSEL:
			for (int i=0; i < 8; i++)
			{
				m_p8grom[i]->write(*m_space, 0, m_latched_data);
			}
			if (TRACE_MEM) logerror("Write G8 <- %02x\n", m_latched_data);
			break;

		case P3GSEL:
			for (int i=0; i < 3; i++)
			{
				m_p3grom[i]->write(*m_space, 0, m_latched_data);
			}
			if (TRACE_MEM) logerror("Write G3 <- %02x\n", m_latched_data);
			break;

		default:
			logerror("Error: Multiple GROM libs selected: SGM=%d TSG=%d P8G=%d P3G=%d\n", (select & SGMSEL)!=0, (select & TSGSEL)!=0, (select & P8GSEL)!=0, (select & P3GSEL)!=0);
			break;
		}
	}
}

void mainboard8_device::set_paddress(int address)
{
	// Keep this value as the current address
	m_physical_address = (m_physical_address << 16) | address;
	if (TRACE_DETAIL) logerror("Setting physical address %06x\n", m_physical_address);

	m_mofetta->set_address(*m_space, address, m_dbin_level);
	m_peb->setaddress_dbin(*m_space, address, m_dbin_level);
}

WRITE_LINE_MEMBER( mainboard8_device::msast_in )
{
	if (TRACE_DETAIL) logerror("msast = %d\n", state);

	// Start physical space cycle on the trailing edge
	if (state==CLEAR_LINE)
	{
		m_mofetta->pmemen_in(ASSERT_LINE);
		m_peb->memen_in(ASSERT_LINE);
	}
	m_mofetta->msast_in(state);
	m_peb->msast_in(state);
}


READ8_MEMBER( mainboard8_device::read )
{
	UINT8 value = 0;
	const char* what;

	// =================================================
	//   Logical space
	// =================================================
	if (m_amigo->mapper_accessed())
	{
		value = m_amigo->read(space, 0);
		what = "mapper";
		goto readdone;
	}

	if (m_amigo->sramcs_out()==ASSERT_LINE)
	{
		value = m_sram[m_logical_address & 0x07ff];
		what = "SRAM";
		goto readdone;
	}

	if (m_vaquerro->lascsq_out()==ASSERT_LINE)
	{
		// VDP access
		if (m_vaquerro->vdprd_out()==ASSERT_LINE)
		{
			value = m_A14_set? m_video->register_read(space, 0) : m_video->vram_read(space, 0);
			what = "video";
			goto readdone;
		}

		// System ROM0
		if (m_vaquerro->sromcs_out()==ASSERT_LINE)
		{
			value = m_rom0[m_logical_address & 0x1fff];
			what = "ROM0";
			goto readdone;
		}

		// Speech
		if (m_vaquerro->sprd_out()==ASSERT_LINE)
		{
			value = m_speech->status_r(space, 0) & 0xff;
			what = "speech";
			goto readdone;
		}

		// GROMs
		switch (m_vaquerro->gromcs_out())
		{
		case SGMSEL:
			for (int i=0; i < 3; i++)
			{
				m_sgrom[i]->readz(space, 0, &value);
			}
			m_gromport->readz(space, 0, &value);
			if (TRACE_GROM && !m_A14_set) logerror("GS>%04x\n", m_sgrom[0]->debug_get_address()-1);
			what = "system GROM";
			goto readdone;

		case TSGSEL:
			for (int i=0; i < 8; i++)
			{
				m_tsgrom[i]->readz(space, 0, &value);
			}
			if (TRACE_GROM && !m_A14_set) logerror("GT>%04x\n", m_tsgrom[0]->debug_get_address()-1);
			what = "TTS GROM";
			goto readdone;

		case P8GSEL:
			for (int i=0; i < 8; i++)
			{
				m_p8grom[i]->readz(space, 0, &value);
			}
			if (TRACE_GROM && !m_A14_set) logerror("G8>%04x\n", m_p8grom[0]->debug_get_address()-1);
			what = "P8 GROM";
			goto readdone;

		case P3GSEL:
			for (int i=0; i < 3; i++)
			{
				m_p3grom[i]->readz(space, 0, &value);
			}
			if (TRACE_GROM && !m_A14_set) logerror("G3>%04x\n", m_p3grom[0]->debug_get_address()-1);
			what = "P3 GROM";
			goto readdone;
		default:
			break;
		}

		// These messages appear in fact every time that a GPL command writes
		// an immediate value to a write-only address (like 9400) because the
		// GPL interpreter always tries to load the value from the provided memory address first

		logerror("Read %04x (unmapped) ignored\n", m_logical_address);

		// Memory cycle ends
		cycle_end();
		return 0;
	}
	else
	{
		// =================================================
		//   Physical space
		// =================================================
		if (m_amigo->skdrcs_out()==ASSERT_LINE)
		{
			value = m_dram[m_physical_address & 0xffff];
			what = "DRAM";
			goto readdonephys;
		}

		if (m_mofetta->rom1cs_out()==ASSERT_LINE)
		{
			int address = (m_physical_address & 0x1fff);
			if (m_mofetta->rom1am_out()==ASSERT_LINE) address |= 0x4000;
			if (m_mofetta->rom1al_out()==ASSERT_LINE) address |= 0x2000;
			value = m_rom1[address];

			if (TRACE_MEM) logerror("Read %04x (ROM1@%04x) -> %02x\n", m_logical_address, address, value);
			cycle_end();
			return value;
		}

		if (m_mofetta->alccs_out()==ASSERT_LINE)
		{
			value = m_oso->read(*m_space, m_physical_address>>1);
			what = "OSO";
			goto readdonephys;
		}

		if (m_mofetta->prcs_out()==ASSERT_LINE)
		{
			value = m_pascalrom[m_physical_address & 0x3fff];
			what = "PASCAL";
			goto readdonephys;
		}

		if (m_mofetta->cmas_out()==ASSERT_LINE)
		{
			m_gromport->romgq_line(ASSERT_LINE);
			m_gromport->readz(*m_space, m_physical_address & 0x3fff, &value);
			what = "Cartridge";
			goto readdonephys;
		}

		if (m_mofetta->dbc_out()==ASSERT_LINE)
		{
			m_peb->readz(*m_space, m_physical_address & 0xffff, &value);
			what = "PEB";
			goto readdonephys;
		}

		if (TRACE_PUNMAP) logerror("Read %04x (phys %06x, unmapped) ignored\n", m_logical_address, m_physical_address);

		// Memory cycle ends
		cycle_end();
		return 0;
	}


readdone:
	if (TRACE_MEM) logerror("Read %04x (%s) -> %02x\n", m_logical_address, what, value);
	cycle_end();
	return value;

readdonephys:
	if (TRACE_MEM) logerror("Read %04x (phys %06x, %s) -> %02x\n", m_logical_address, m_physical_address, what, value);
	cycle_end();
	return value;
}

void mainboard8_device::cycle_end()
{
	// Memory cycle ends
	m_vaquerro->memen_in(CLEAR_LINE);
	m_amigo->memen_in(CLEAR_LINE);
	m_mofetta->pmemen_in(CLEAR_LINE);
	m_peb->memen_in(CLEAR_LINE);
}

/*
    When writing, the emulation relies on a push mechanism; the write
    operation follows the setaddress operation immediately.

    If the READY line is pulled down due to the mapping process, we must
    store the data bus value until the physical address is available.
*/
WRITE8_MEMBER( mainboard8_device::write )
{
	m_latched_data = data;
	m_space = &space;
	m_pending_write = true;

	// Some logical space devices can be written immediately
	// GROMs and video must wait to be selected
	if (m_amigo->mapper_accessed())
	{
		if (TRACE_MEM) logerror("Write %04x (mapper) <- %02x\n", m_logical_address, data);
		m_amigo->write(space, 0, data);
		m_pending_write = false;
	}

	// Sound ... we have to put it before SRAM because in compatibility mode the
	// sound address lies within the SRAM area
	if (m_vaquerro->sccs_out()==ASSERT_LINE)
	{
		if (TRACE_MEM) logerror("Write %04x (sound) <- %02x\n", m_logical_address, data);
		m_sound->write(space, 0, data);         // Sound chip will lower READY after this access
		m_pending_write = false;
	}
	else
	{
		// SRAM
		if (m_amigo->sramcs_out()==ASSERT_LINE)
		{
			if (TRACE_MEM) logerror("Write %04x (SRAM) <- %02x\n", m_logical_address, data);
			m_sram[m_logical_address & 0x07ff] = data;
			m_pending_write = false;
		}
	}

	// Speech
	if (m_vaquerro->spwt_out()==ASSERT_LINE)
	{
		if (TRACE_MEM) logerror("Write %04x (speech) <- %02x\n", m_logical_address, data);
		m_speech->data_w(space, 0, data);
		m_pending_write = false;
	}

	if (!m_pending_write)
		cycle_end();

	// The remaining physical devices will respond as soon as the physical address is completely set
}

/*
    Set 99/4A compatibility mode (CRUS=1)
*/
WRITE_LINE_MEMBER( mainboard8_device::crus_in )
{
	if (TRACE_CRU) logerror("%s CRUS\n", (state==1)? "Assert" : "Clear");
	m_vaquerro->crus_in(state);
	m_amigo->crus_in(state);
}

/*
    Set mapper /PTGEN line ("Pascal and Text-to-speech GROMs enable").
    Note that PTGEN is negative logic.
*/
WRITE_LINE_MEMBER( mainboard8_device::ptgen_in )
{
	if (TRACE_CRU) logerror("%s PTGEN*\n", (state==0)? "Assert" : "Clear");
	m_vaquerro->crusgl_in((state==0)? ASSERT_LINE : CLEAR_LINE);
}


/*
    READY lines, to be combined
*/
WRITE_LINE_MEMBER( mainboard8_device::system_grom_ready )
{
	m_vaquerro->sgmry(state);
}

WRITE_LINE_MEMBER( mainboard8_device::ptts_grom_ready )
{
	m_vaquerro->tsgry(state);
}

WRITE_LINE_MEMBER( mainboard8_device::p8_grom_ready )
{
	m_vaquerro->p8gry(state);
}

WRITE_LINE_MEMBER( mainboard8_device::p3_grom_ready )
{
	m_vaquerro->p3gry(state);
}

WRITE_LINE_MEMBER( mainboard8_device::sound_ready )
{
	m_sound_ready = state;
}

WRITE_LINE_MEMBER( mainboard8_device::speech_ready )
{
	// The TMS5200 implementation uses TRUE/FALSE, not ASSERT/CLEAR semantics
	m_speech_ready = (state==FALSE)? ASSERT_LINE : CLEAR_LINE;
}

WRITE_LINE_MEMBER( mainboard8_device::pbox_ready )
{
	m_pbox_ready = state;
}

WRITE_LINE_MEMBER( mainboard8_device::ggrdy_in )
{
	m_amigo->srdy_in((state==ASSERT_LINE && m_speech_ready && m_sound_ready && m_pbox_ready)? ASSERT_LINE : CLEAR_LINE);
}

static const char *const glib0[] = { SYSGROM0_TAG, SYSGROM1_TAG, SYSGROM2_TAG };
static const char *const glib1[] = { GLIB10_TAG, GLIB11_TAG, GLIB12_TAG, GLIB13_TAG, GLIB14_TAG, GLIB15_TAG, GLIB16_TAG, GLIB17_TAG };
static const char *const glib2[] = { GLIB20_TAG, GLIB21_TAG, GLIB22_TAG, GLIB23_TAG, GLIB24_TAG, GLIB25_TAG, GLIB26_TAG, GLIB27_TAG };
static const char *const glib3[] = { GLIB30_TAG, GLIB31_TAG, GLIB32_TAG };

void mainboard8_device::device_start()
{
	logerror("Starting main board\n");
	// Lines going to the main driver class, then to the CPU
	m_ready.resolve_safe();         // READY
	m_console_reset.resolve_safe(); // RESET
	m_hold_line.resolve_safe();     // HOLD

	// Setting up the links to the GROMs
	for (int i=0; i < 8; i++)
	{
		if (i < 3)
		{
			m_sgrom[i] = downcast<tmc0430_device*>(machine().device(glib0[i]));
			m_p3grom[i] = downcast<tmc0430_device*>(machine().device(glib3[i]));
		}
		m_tsgrom[i] = downcast<tmc0430_device*>(machine().device(glib1[i]));
		m_p8grom[i] = downcast<tmc0430_device*>(machine().device(glib2[i]));
	}

	// Link to speech synthesizer
	m_speech = downcast<cd2501ecd_device*>(machine().device(SPEECHSYN_TAG));

	// Link to sound chip
	m_sound = downcast<sn76496_base_device*>(machine().device(TISOUNDCHIP_TAG));

	// Link to video
	m_video = downcast<tms9118_device*>(machine().device(VDP_TAG));

	// Link to cartridge port
	m_gromport = downcast<gromport_device*>(machine().device(GROMPORT_TAG));

	// Link to PEB
	m_peb = downcast<peribox_device*>(machine().device(PERIBOX_TAG));

	// Configure RAM and AMIGO
	m_sram = std::make_unique<UINT8[]>(SRAM_SIZE);
	m_dram = std::make_unique<UINT8[]>(DRAM_SIZE);

	m_amigo->connect_sram(m_sram.get());

	m_rom0  = machine().root_device().memregion(ROM0_REG)->base();
	m_rom1  = machine().root_device().memregion(ROM1_REG)->base();
	m_pascalrom  = machine().root_device().memregion(PASCAL_REG)->base();
}

void mainboard8_device::device_reset()
{
	logerror("Resetting main board\n");
	m_last_ready = CLEAR_LINE;
	m_speech_ready = true;
	m_sound_ready = true;
	m_pbox_ready = true;
	m_pending_write = false;
	m_prev_grom = 0;
	m_A14_set = false;
}

MACHINE_CONFIG_FRAGMENT( ti998_mainboard )
	MCFG_DEVICE_ADD(VAQUERRO_TAG, VAQUERRO, 0)
	MCFG_DEVICE_ADD(MOFETTA_TAG, MOFETTA, 0)
	MCFG_DEVICE_ADD(AMIGO_TAG, AMIGO, 0)
	MCFG_DEVICE_ADD(OSO_TAG, OSO, 0)
MACHINE_CONFIG_END

machine_config_constructor mainboard8_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ti998_mainboard );
}
const device_type MAINBOARD8 = &device_creator<mainboard8_device>;


/***************************************************************************
  ===== VAQUERRO: Logical Address Space decoder =====

    Logical address space (LAS)
    ===========================
    The LAS is the address space as seen by the TMS 9995 CPU. It is 64 KiB large.
    The LAS can be configured in two ways:
    - the native (99/8) mode
    - and the compatibility mode (99/4A)

    Both modes are selected by CRU bit 20 on base 0000 (named "CRUS").

    The console starts up in compatibility mode.

    The compatibility mode organizes the LAS in a similar way as the TI-99/4A.
    This means that machine language programs should run with no or only minor
    changes. In particular, game cartridges work without problems.

    The native mode rearranges the address space and puts memory-mapped devices
    to other positions.

    TI-99/4A compatibility mode (CRUS=1)
    ------------------------------------
    0000-1fff: 2 KiB ROM0
    2000-7fff: Free area
    8000-87ff: 2 KiB SRAM
      8000-81ff: mapper files (8 files with 16*4 bytes each)
      8200-82ff: Free RAM
      8300-83ff: Scratch-pad RAM as in the 99/4A
      8400-840f: Sound chip
    8800-880f: VDP read port (data, status)
    8810-881f: Mapper access port
    8820-8bff: Free area
    8c00-8c0f: VDP write port (data, address)
    8c10-8fff: Free area
    9000-900f: Speech synthesizer read (on-board)
    9010-93ff: Free area
    9400-940f: Speech synthesizer write (on-board)
    9410-97ff: Free area
    9800-980f: System GROM read (data, address)
    9810-9bff: Free area
    9c00-9c0f: System GROM write (data, address)
    9c10-fffb: Free area
    fffc-ffff: NMI vector

    TI-99/8 native mode (CRUS=0)
    ----------------------------
    0000-efff: Free area
    f000-f7ff: 2 KiB SRAM
      f000-f1ff: mapper files (8 files with 16*4 bytes each)
      f200-f7ff: Free RAM
    f800-f80f: Sound chip
    f810-f81f: VDP read (data, status) and write (data, address)
    f820-f82f: Speech synthesizer read/write
    f830-f83f: System GROM read/write
    f840-f86f: Free area
    f870-f87f: Mapper access port
    f880-fffb: Free area
    fffc-ffff: NMI vector

    Note that ROM0 is not visible in the native mode.

    If CRU bit 21 (PTGEN*) is set to 0, Pascal GROMs appear in the LAS in either
    mode. It is highly recommended to use native mode when turning on these
    GROMs, because the area where they appear may be occupied by a program in
    99/4A mode.

    Pascal and Text-to-speech GROM enabled (PTGEN*=0)
    -------------------------------------------------
    f840-f84f: Text-to-speech GROM read/write
    f850-f85f: P-Code library #1 GROM read/write
    f860-f86f: P-Code library #2 GROM read/write


***************************************************************************/

vaquerro_device::vaquerro_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: device_t(mconfig, VAQUERRO, "Logical Address Space Decoder", tag, owner, clock, "ti998_vaquerro", __FILE__),
	m_crus(ASSERT_LINE),
	m_crugl(ASSERT_LINE),
	m_ggrdy(ASSERT_LINE)
{
}

SETADDRESS_DBIN_MEMBER( vaquerro_device::set_address )
{
	// Do the decoding
	// state = dbin, offset = address

	bool reading = (state==ASSERT_LINE);
	bool sgfap = false;
	bool tsgfap = false;
	bool p8gfap = false;
	bool p3gfap = false;
	bool vfap = false;

	m_a14 = ((offset & 2)!=0)? ASSERT_LINE : CLEAR_LINE; // Needed for clock_in

	m_dbin_level = (line_state)state;

	int offbase = (offset & 0xfff1);

	// ===================   TI (compatibility) mode   ======================
	if (m_crus == ASSERT_LINE)
	{
		if (TRACE_DETAIL) logerror("Compatibility mode\n");

		// SPRD (Speech read)
		m_sprd = ((offbase==0x9000) && reading);

		// SPWT (Speech write)
		m_spwt = ((offbase==0x9400) && !reading);

		// Sound
		m_sccs = ((offbase==0x8400)&& !reading);

		// ROM0
		m_sromcs = (((offset & 0xe000)==0x0000) && reading);

		// Video select
		vfap = ((offbase==0x8800) && reading) || ((offbase==0x8c00) && !reading);

		// System GROM
		sgfap = ((offbase==0x9800) && reading) || ((offbase==0x9c00) && !reading);
	}
	// ======================   Native mode    ======================
	else
	{
		if (TRACE_DETAIL) logerror("Native mode\n");

		// SPRD (Speech read)
		m_sprd = ((offbase==0xf820) && reading);

		// SPWT (Speech write)
		m_spwt = ((offbase==0xf820) && !reading);

		// Sound
		m_sccs = ((offbase==0xf800) && !reading);

		// Video
		vfap = (offbase==0xf810);

		// System GROM (read and write)
		sgfap = (offbase==0xf830);
	}

	// These lines are not decoded for compatibility or native mode, only
	// the line CRUGL determines whether they become visible.
	tsgfap = (offbase==0xf840) && m_crugl;
	p8gfap = (offbase==0xf850) && m_crugl;
	p3gfap = (offbase==0xf860) && m_crugl;

	// The LASREQ line says whether Vaquerro does the job, or whether it is Mofetta's turn.
	m_grom_or_video = sgfap || tsgfap || p8gfap || p3gfap || vfap ;

	m_lasreq = (m_sprd || m_spwt || m_sccs || m_sromcs || m_grom_or_video);

	if (TRACE_DETAIL) logerror("sgfap=%d tsgfap=%d p8gfap=%d p3gfap=%d vfap=%d\n", sgfap, tsgfap, p8gfap, p3gfap, vfap);

	// Pass the selection to the wait state generators
	// and pick up the current select line states
	m_sgmws.select_in(sgfap);
	m_tsgws.select_in(tsgfap);
	m_p8gws.select_in(p8gfap);
	m_p3gws.select_in(p3gfap);
	m_vidws.select_in(vfap);

	m_gromsel = m_sgmws.select_out() | m_tsgws.select_out() | m_p8gws.select_out() | m_p3gws.select_out();

	m_vdprd = (reading && (m_vidws.select_out()!=0));
	m_vdpwt = (!reading && (m_vidws.select_out()!=0));

	if (m_grom_or_video)
	{
		// We don't see the current selection now; only with next clock pulse.
		m_mainboard->ggrdy_in(m_sry);
		if (TRACE_READY) logerror("GGRDY = %d\n", m_sry);
	}
}

WRITE_LINE_MEMBER( vaquerro_device::crusgl_in )
{
	m_crugl = (state==ASSERT_LINE);
}

WRITE_LINE_MEMBER( vaquerro_device::crus_in )
{
	m_crus = (line_state)state;
}

WRITE_LINE_MEMBER( vaquerro_device::memen_in )
{
	m_memen = (state==ASSERT_LINE);
}

/*
    Called by Mofetta
*/
READ_LINE_MEMBER( vaquerro_device::lascsq_out )
{
	return (m_lasreq && m_memen)? ASSERT_LINE : CLEAR_LINE;
}

/*
    Incoming ready lines from the GROM library
*/
WRITE_LINE_MEMBER( vaquerro_device::sgmry )
{
	if (TRACE_READY) logerror("Incoming SGMRY = %d\n", state);
	m_sgmws.ready_in((line_state)state);
}

WRITE_LINE_MEMBER( vaquerro_device::tsgry )
{
	if (TRACE_READY) logerror("Incoming TSGRY = %d\n", state);
	m_tsgws.ready_in((line_state)state);
}

WRITE_LINE_MEMBER( vaquerro_device::p8gry )
{
	if (TRACE_READY) logerror("Incoming 8GRY = %d\n", state);
	m_p8gws.ready_in((line_state)state);
}

WRITE_LINE_MEMBER( vaquerro_device::p3gry )
{
	if (TRACE_READY) logerror("Incoming P3GRY = %d\n", state);
	m_p3gws.ready_in((line_state)state);
}

/*
    Outgoing READY
*/
READ_LINE_MEMBER( vaquerro_device::ggrdy_out )
{
	if (TRACE_READY) logerror("GGRDY out = %d\n", m_ggrdy);
	return m_ggrdy;
}

/*
    Select lines
*/

// =========================

int vaquerro_device::gromcs_out()
{
	return m_gromsel;
}

// =========================

READ_LINE_MEMBER( vaquerro_device::vdprd_out )
{
	return (m_vdprd && m_memen)? ASSERT_LINE : CLEAR_LINE;
}

READ_LINE_MEMBER( vaquerro_device::vdpwt_out )
{
	return (m_vdpwt && m_memen)? ASSERT_LINE : CLEAR_LINE;
}

READ_LINE_MEMBER( vaquerro_device::sprd_out )
{
	return (m_sprd && m_memen)? ASSERT_LINE : CLEAR_LINE;
}

READ_LINE_MEMBER( vaquerro_device::spwt_out )
{
	return (m_spwt && m_memen)? ASSERT_LINE : CLEAR_LINE;
}

READ_LINE_MEMBER( vaquerro_device::sromcs_out )
{
	return (m_sromcs && m_memen)? ASSERT_LINE : CLEAR_LINE;
}

READ_LINE_MEMBER( vaquerro_device::sccs_out )
{
	return (m_sccs && m_memen)? ASSERT_LINE : CLEAR_LINE;
}

/*
    Incoming clock signal.

    The Vaquerro has a Wait State generation logic circuit for the video
    processor and all 4 GROM libraries. Each one has its separate generator.
    The GROMs get a 16 cycle wait period after their access, while the video
    processors gets an 8 cycle wait period. If during that period another
    access occurs, the system READY line will be cleared, triggering wait
    states in the CPU.
*/
WRITE_LINE_MEMBER( vaquerro_device::clock_in )
{
	line_state level = (line_state)state;

	// Propagate to the wait state generators (note that we need both clock levels)
	m_sgmws.clock_in(level);
	m_tsgws.clock_in(level);
	m_p8gws.clock_in(level);
	m_p3gws.clock_in(level);
	m_vidws.clock_in(level);

	// Collect the selections
	// Each one has its own indication, defined at init time
	m_gromsel = m_sgmws.select_out() | m_tsgws.select_out() | m_p8gws.select_out() | m_p3gws.select_out();

	bool reading = (m_dbin_level==ASSERT_LINE);

	m_vdprd = (reading && (m_vidws.select_out()!=0));
	m_vdpwt = (!reading && (m_vidws.select_out()!=0));

	// Get the READY levels from the GROMs
	if (level==CLEAR_LINE)
	{
		m_sry = m_sgmws.ready_out() || m_tsgws.ready_out() || m_p8gws.ready_out() || m_p3gws.ready_out() || m_vidws.ready_out();
		if (TRACE_WS) logerror("ready_out = (%d, %d, %d, %d, %d)\n", m_sgmws.ready_out(), m_tsgws.ready_out(), m_p8gws.ready_out(), m_p3gws.ready_out(),m_vidws.ready_out());
	}

	// If the output gate is closed, propagate ASSERT_LINE (pulled up)
	m_ggrdy = (!m_grom_or_video || m_sry)? ASSERT_LINE : CLEAR_LINE;
}


void vaquerro_device::device_start()
{
	logerror("Starting\n");
	m_mainboard = downcast<mainboard8_device*>(owner());
	m_sgmws.init(SGMSEL);
	m_tsgws.init(TSGSEL);
	m_p8gws.init(P8GSEL);
	m_p3gws.init(P3GSEL);
	m_vidws.init(VIDSEL);
}

void vaquerro_device::device_reset()
{
	m_ggrdy = ASSERT_LINE;
	m_vdpwt = m_vdprd = CLEAR_LINE;
	m_gromsel = 0;
	m_sgmws.treset_in(ASSERT_LINE);
	m_tsgws.treset_in(ASSERT_LINE);
	m_p8gws.treset_in(ASSERT_LINE);
	m_p3gws.treset_in(ASSERT_LINE);
	m_vidws.treset_in(ASSERT_LINE);
}

/*
    Wait state generation logic inside Vaquerro

    Analysis of the logic diagram of the Vaquerro delivers the following
    behavior (for the first GROM library; similar behavior applies for the
    other libraries). Note that the CLKOUT line is inverted.

    1. When the GROMs are unselected by address (SGFAP), the SRY line is Z
       (System READY). The GROM ready line (SGMRY) has no effect.
    2. When SGFAP is asserted while the internal counter is off, the
       READY line changes from Z to Low and the GROM select line (SGCS) is
       asserted (both immediately, before the next tick edge). SGMRY has no effect.
       The circuit state is constant during further clock ticks.
    3. After being selected, when SGMRY is asserted (GROM is ready), SRY
       changes to High on the next trailing edge. This will allow the
       CPU to complete the GROM access on the next cycle,
       and the address bus will change, typically deselecting the GROMs.
       Until this deselection, the circuit state remains constant
       (SGCS asserted, READY=H).
    4. When the GROMs are deselected, SRY changes to Z, and SGCS is cleared
       (immediately). A counter is started at 0 that is incremented on each clock
       tick (leading edge).
    5. When SGFAP is asserted while the counter is less that 15, SRY changes
       to Low immediately. SGCS remains cleared, so the GROMs are not selected.
       While SGFAP stays cleared, the counter completes its way to 15,
       then 0, and turns off.
    6. When the counter reaches 15, it returns to 0 on the next tick
       (leading). On the following trailing edge, the GROM select line is
       asserted, while the READY line remains Low.
    7. Continue at 3.
*/
void waitstate_generator::select_in(bool addressed)
{
	m_addressed = addressed;
}

int waitstate_generator::select_out()
{
	return (!m_counting && m_addressed)? m_selvalue : 0;
}

/*
    Should be low by default.
*/
line_state waitstate_generator::ready_out()
{
	return (m_ready && !m_counting && m_generate)? ASSERT_LINE : CLEAR_LINE;
}

bool waitstate_generator::is_counting()
{
	return m_counting;
}

bool waitstate_generator::is_generating()
{
	return m_generate;
}

bool waitstate_generator::is_ready()
{
	return m_ready;
}

/*
    READY in. This may only show an effect with the next trailing edge of CLKOUT.
*/
void grom_waitstate_generator::ready_in(line_state ready)
{
	m_ready = (ready==ASSERT_LINE);
}

void grom_waitstate_generator::clock_in(line_state clkout)
{
	if (clkout == ASSERT_LINE)
	{
		if (m_counting) m_counter++;
	}
	else
	{
		if (m_counting && m_counter==16)
		{
			m_counter = 0;
			m_counting = false;
		}
		else
		{
			if (!m_addressed && m_generate) m_counting = true;
			m_generate = ((m_addressed || m_counting) && (m_counter != 15));
		}
	}
}

void waitstate_generator::treset_in(line_state reset)
{
	if (reset==ASSERT_LINE)
	{
		m_counter = 0;
		m_generate = m_counting = m_addressed = false;
	}
}

void video_waitstate_generator::clock_in(line_state clkout)
{
	if (clkout == ASSERT_LINE)
	{
		if (m_counting) m_counter++;
	}
	else
	{
		if (m_counting && m_counter==7)
		{
			m_counter = 0;
			m_counting = false;
		}
		else
		{
			if (!m_addressed && m_generate) m_counting = true;
			m_generate = ((m_addressed || m_counting) && (m_counter != 6));
		}
	}
}

const device_type VAQUERRO = &device_creator<vaquerro_device>;

/***************************************************************************
  ===== MOFETTA: Physical Address Space decoder =====

     Physical address space (PAS)
     ============================
     The PAS is 24 bits wide and accessed via the custom mapper chip nicknamed
     "Amigo". The mapper exchanges map definitions with SRAM (see LAS). That
     means, a map can be prepared in SRAM, and for activating it, the mapper
     is accessed on its port, telling it to load or save a map.

     000000-00ffff: 64 KiB console DRAM
     010000-efffff: undefined

     f00000-f03fff: PASCAL support ROM (not mentioned in [1])

     f04000-feffff: undefined
     ff0000       : unmapped (code for mapper)
     ff0001-ff3fff: undefined
     ff4000-ff5fff: DSR ROM in Peripheral Box, Hexbus DSR (CRU 1700) or additional ROM (CRU 2700)
     ff6000-ff9fff: Cartridge ROM space
     ffa000-ffdfff: 16 KiB ROM1
     ffe000-ffe00f: Interrupt level sense
     ffe010-ffffff: undefined


***************************************************************************/

enum
{
	UNDEF=0,
	DRAM,
	PASCAL,
	INTERNAL
};

mofetta_device::mofetta_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, MOFETTA, "Physical Address Space Decoder", tag, owner, clock, "ti998_mofetta", __FILE__),
	m_gotfirstword(false)
{
}

SETADDRESS_DBIN_MEMBER( mofetta_device::set_address )
{
	if (!m_gotfirstword)
	{
		// Store the first word and wait for clearing of MSAST
		if (TRACE_MOFETTA) logerror("Got the upper word of the address: %04x\n", offset);
		m_address_latch = offset;
	}
	else
	{
		// Second part - now decode the address
		if (TRACE_MOFETTA) logerror("Got the lower word of the address: %04x\n", offset);

		bool acs, tcs, rcs, acsx;
		bool reading = (state==ASSERT_LINE);
		int offbase = (offset & 0xe000);

		// PASCAL ROM select (16K)
		m_prcs = (m_prefix == 0xf0) && ((offset & 0xc000) == 0x0000);

		// Hexbus select
		acs = (m_prefix == 0xff) && (offbase == 0x4000) && m_alcpg;

		// Internal DSR select (ff4000-ff5fff @ CRU>2700)
		tcs = (m_prefix == 0xff) && (offbase == 0x4000) && m_txspg;

		// Hexbus select (ff4000-ff5fef @ CRU>1700), excluding OSO
		acsx = acs && ((offset & 0x1ff0)!=0x1ff0);

		// Upper 16K of ROM1
		m_rom1am = !((offbase == 0xa000) || (offbase == 0xc000));

		// ROM select
		rcs = (m_prefix == 0xff) && reading && !m_rom1am;

		// ROM1 select (containing 16K ROM, 8K TTS, 8K ACS)
		m_rom1cs = tcs || rcs || acsx;

		// Accessing OSO (ff5ff0 @ CRU>1700)
		m_alccs = acs && ((offset & 0x1ff0)==0x1ff0);

		// Second half of ROM or ACS
		m_rom1al = reading && (m_prefix == 0xff) && ((offbase == 0xc000) || acs);

		// Cartridge port (ff6000-ff9fff)
		m_cmas = (m_prefix == 0xff) && ((offbase == 0x6000) || (offbase == 0x8000));

		m_gotfirstword = false;
	}
}

/*
    Mofetta delivers the GROMCLK. In the 99/4A, this clock is produced by the VDP.
    Apart from that, Mofetta does not need the CLKOUT.
*/
WRITE_LINE_MEMBER( mofetta_device::clock_in )
{
	if (state == CLEAR_LINE)    // TODO: Correct edge?
	{
		m_gromclock_count++;
		if (m_gromclock_count >=3)
		{
			m_gromclk_up = !m_gromclk_up;
			m_gromclock_count = 0;
		}
	}
}

READ_LINE_MEMBER( mofetta_device::alccs_out )
{
	return (m_alccs && m_pmemen)? ASSERT_LINE : CLEAR_LINE;
}

READ_LINE_MEMBER( mofetta_device::gromclk_out )
{
	return m_gromclk_up? ASSERT_LINE : CLEAR_LINE;
}

READ_LINE_MEMBER( mofetta_device::rom1cs_out )
{
	return (m_rom1cs && m_pmemen)? ASSERT_LINE : CLEAR_LINE;
}

READ_LINE_MEMBER( mofetta_device::rom1am_out )
{
	return (m_rom1am && m_pmemen)? ASSERT_LINE : CLEAR_LINE;
}

READ_LINE_MEMBER( mofetta_device::rom1al_out )
{
	return (m_rom1al && m_pmemen)? ASSERT_LINE : CLEAR_LINE;
}

READ_LINE_MEMBER( mofetta_device::prcs_out )
{
	return (m_prcs && m_pmemen)? ASSERT_LINE : CLEAR_LINE;
}

READ_LINE_MEMBER( mofetta_device::cmas_out )
{
	return (m_cmas && m_pmemen)? ASSERT_LINE : CLEAR_LINE;
}

/*
    Asserted when a PEB access occurs
*/
READ_LINE_MEMBER( mofetta_device::dbc_out )
{
	return (m_lasreq || m_cmas || m_rom1cs || m_skdrcs || !m_pmemen)? CLEAR_LINE : ASSERT_LINE;
}

WRITE8_MEMBER(mofetta_device::cruwrite)
{
	if ((offset & 0xff00)==0x2700)
	{
		if ((offset & 0x0002)!=0)
		{
			// SWRST (Software reset)
			// Value seems to be irrelevant
			if (TRACE_CRU) logerror("Doing a software reset by SBO 2702\n");
			m_mainboard->reset_console(ASSERT_LINE);
			m_mainboard->reset_console(CLEAR_LINE);
		}
		else
		{
			m_txspg = (data!=0);        // CRU>2700
			if (TRACE_CRU)  logerror("Turning %s CRU>2700\n", m_txspg? "on" : "off");
		}
	}
	else
	{
		if ((offset & 0xff00)==0x1700)
		{
			m_alcpg = (data!=0);        // CRU>1700
			if (TRACE_CRU) logerror("Turning %s CRU>1700\n", m_alcpg? "on" : "off");
		}
	}
}

/*
    Setting or clearing the MSAST line.
*/
WRITE_LINE_MEMBER( mofetta_device::msast_in )
{
	if (state == ASSERT_LINE)
	{
		if (m_msast == CLEAR_LINE)      // Leading edge
		{
			m_gotfirstword = true; // Process first word
			// We now have the first part, containing the flags and the upper byte.
			m_prefix = m_address_latch & 0xff;
		}
	}
	// TODO: Evaluate the first three bits
	m_msast = (line_state)state;
}

WRITE_LINE_MEMBER( mofetta_device::pmemen_in )
{
	m_pmemen = (state==ASSERT_LINE);
}

WRITE_LINE_MEMBER( mofetta_device::lascs_in )
{
	m_lasreq = (state==ASSERT_LINE);
}

WRITE_LINE_MEMBER( mofetta_device::skdrcs_in )
{
	m_skdrcs = (state==ASSERT_LINE);
}

void mofetta_device::device_start()
{
	logerror("Starting\n");
	m_mainboard = downcast<mainboard8_device*>(owner());
}

void mofetta_device::device_reset()
{
	m_gotfirstword = false;
	m_alcpg = false;
	m_txspg = false;
	m_prefix = 0;
}


const device_type MOFETTA = &device_creator<mofetta_device>;

/***************************************************************************

  ==============================
    Mapper (codename "Amigo")
  ==============================

    Unfortunately, we do not have logic diagrams for Amigo, so we have to
    guess how it is actually working.

    Initial setting of mapper (as defined in the power-up routine, TI-99/4A mode)

    0   00ff0000 -> Unmapped; logical address 0000...0fff = ROM0
    1   00ff0000 -> Unmapped; logical address 1000...1fff = ROM0
    2   00000800 -> DRAM; 2000 = 000800, 2fff = 0017ff
    3   00001800 -> DRAM; 3000 = 001800, 3fff = 0027ff
    4   00ff4000 -> DSR space (internal / ioport)
    5   00ff5000 -> DSR space (internal / ioport)
    6   00ff6000 -> Cartridge space (6000..6fff)
    7   00ff7000 -> Cartridge space (7000..7fff)
    8   00ff0000 -> Unmapped; device ports (VDP) and SRAM
    9   00ff0000 -> Unmapped; device ports (Speech, GROM)
    A   00002800 -> DRAM; a000 = 002800, afff = 0037ff
    B   00003800 -> DRAM; b000 = 003800, bfff = 0047ff
    C   00004800 -> DRAM; c000 = 004800, cfff = 0057ff
    D   00005800 -> DRAM; d000 = 005800, dfff = 0067ff
    E   00006800 -> DRAM; e000 = 006800, efff = 0077ff
    F   00007800 -> DRAM; f000 = 007800, ffff = 0087ff

    Format of map table entry

    +--+---+---+---+---+---+---+---+ +-----------+ +----------+ +---------+
    | W| X | R | 0 | 0 | 0 | 0 | 0 | | Upper (8) | | High (8) | | Low (8) |
    +--+---+---+---+---+---+---+---+ +-----------+ +----------+ +---------+

    W: Write protection if set to 1
    X: Execute protection if set to 1
    R: Read protection if set to 1

    When a protection violation occurs, the tms9901 INT1* pin is pulled low
    (active).  The pin remains low until the mapper status register is read.

    Address handling
    ----------------
    Physical address is (Upper * 2^16) + (High * 2^8) + Low

    The mapper calculates the actual physical address by looking up the
    table entry from the first four bits of the logical address and then
    *adding* the remaining 12 bits of the logical address on the map value.

    The value 0xff0000 is used to indicate a non-mapped area.

    Mapper control register
    -----------------------
    The mapper control register is used to initiate a map load/save operation.

    +---+---+---+---+---+---+---+---+
    | 0 | 0 | 0 | 0 | Map File  | RW|
    +---+---+---+---+---+---+---+---+

    The map file is a number from 0-7 indicating the set of map values for the
    operation, which means the location in SRAM where the next 64 values are
    loaded from or stored into.

    RW = 1: load from SRAM into mapper
    RW = 0: store from mapper into SRAM

    When read, the mapper register returns the violation flags:
    +---+---+---+---+---+---+---+---+
    | W | X | R | 0 | 0 | 0 | 0 | 0 |
    +---+---+---+---+---+---+---+---+

***************************************************************************/

amigo_device::amigo_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: device_t(mconfig, AMIGO, "Address space mapper", tag, owner, clock, "ti998_amigo", __FILE__),
	m_logical_space(true),
	m_crus(ASSERT_LINE)
{
}

enum
{
	IDLE = 0,
	CREATE_PADDR,
	ADDR_MSW,
	ADDR_LSW,
	SRAMLOAD,
	SRAMSAVE
};

/*
    Incoming READY line (SRDY)
*/
WRITE_LINE_MEMBER( amigo_device::srdy_in )
{
	if (TRACE_READY) logerror("Incoming SRDY = %d\n", state);
	m_srdy = (line_state)state;

	// If the access is going to logical space, pass through the READY line
	if (m_logical_space)
	{
		if (TRACE_CPURY) logerror("Setting CPURY = %d (SRDY)\n", m_ready_out);
		m_ready_out = m_srdy;
	}
}

WRITE_LINE_MEMBER( amigo_device::memen_in )
{
	m_memen = (state==ASSERT_LINE);
}

/*
    Polled from the mainboard
*/
READ_LINE_MEMBER( amigo_device::cpury_out )
{
	return m_ready_out;
}

/*
    Polled from the mainboard
*/
READ_LINE_MEMBER( amigo_device::sramcs_out )
{
	return m_sram_accessed && m_memen? ASSERT_LINE : CLEAR_LINE;
}

/*
    SKDRCS line (maybe "Sixty-four Kilobyte DRam Chip Select"). We assume that
    Amigo asserts the select line not before the whole address was written.
    This is actually more than needed because we only have 64K DRAM (which
    would only need a 16 bit address), and Amigo itself selects it.
*/
READ_LINE_MEMBER( amigo_device::skdrcs_out )
{
	return m_dram_accessed && (m_amstate == IDLE) && m_memen? ASSERT_LINE : CLEAR_LINE;
}

/*
    Incoming CRUS line. Needed to set the mapper config addresses.
*/
WRITE_LINE_MEMBER( amigo_device::crus_in )
{
	m_crus = (line_state)state;
}

/*
    Incoming LASCS line.
*/
WRITE_LINE_MEMBER( amigo_device::lascs_in )
{
	m_logical_space = (state==ASSERT_LINE);
}

/*
    The logical address bus has been set. The Amigo chip now has to map this
    address to a physical address. There are three phases (3 clock ticks):
    1. Sample the logical address lines, determine the map register
       (first four bits), create the physical address by adding the remaining
       12 bits and the map register contents
    2. Set the physical address bus with the first 16 bits of the physical
       address. Assert the MSAST line.
    3. Set the physical address bus with the second 16 bits of the physical
       address. Clear the MSAST line. Forward any incoming READY=0 to the CPU.
*/
SETOFFSET_MEMBER( amigo_device::set_address )
{
	// Check whether the mapper itself is accessed
	int mapaddr = (m_crus==ASSERT_LINE)? 0x8810 : 0xf870;
	m_mapper_accessed = ((offset & 0xfff1)==mapaddr);

	// or SRAM
	int sramaddr = (m_crus==ASSERT_LINE)? 0x8000 : 0xf000;
	m_sram_accessed = ((offset & 0xf800)==sramaddr);

	m_logical_space |= (m_mapper_accessed || m_sram_accessed);

	// Is the address not in the logical address space?
	if (!m_logical_space)
	{
		if (TRACE_AMIGO) logerror("Amigo decoding; %04x is a physical address.\n", offset);
		// Build the physical address
		// The first three bits are the protection bits (Write, Execute, Read)
		// Theoretically, the addition of the logical address could mess up those
		// first three bits, but the physical address is only 24 bits wide, so we
		// have a space of 5 zeros between the protection bits and the address.
		// We should just clear those five bits after the addition.

		m_physical_address = ((offset & 0x0fff) + m_base_register[(offset >> 12) & 0x000f]) & 0x00ffffff;

		// TODO: Process flags

		// Is it DRAM?
		m_dram_accessed = (m_physical_address & 0x00ff0000)==0;

		// This takes one clock pulse.
		m_amstate = CREATE_PADDR;

		// Pull down READY
		m_ready_out = CLEAR_LINE;

		if (TRACE_CPURY) logerror("Setting CPURY = %d (PAS)\n", m_ready_out);
	}
	else
	{
		// This was a logical space access. Pass through READY.
		m_dram_accessed = false;
		m_amstate = IDLE;
		m_ready_out = m_srdy;
		if (TRACE_CPURY) logerror("Setting CPURY = %d (LAS)\n", m_ready_out);
	}
}

/*
    Read the mapper status bits
*/
READ8_MEMBER( amigo_device::read )
{
	// Read the protection status bits and reset them
	UINT8 value = m_protflag;
	m_protflag = 0;
	return value;
}

/*
    Configure the mapper. This is the only reason to write to the AMIGO.
*/
WRITE8_MEMBER( amigo_device::write )
{
	// Load or save map file
	if ((data & 0xf0)==0x00)
	{
		// Need to HOLD the CPU
		m_amstate = ((data & 1)==1)? SRAMLOAD : SRAMSAVE;
		m_sram_address = (data & 0x0e) << 5;
		m_hold_acknowledged = false;
		m_basereg = 0;
		m_mapvalue = 0;
		m_mainboard->hold_cpu(ASSERT_LINE);
	}
	else logerror("Invalid value written to Amigo: %02x\n", data);
}

WRITE_LINE_MEMBER( amigo_device::clock_in )
{
	if (state==CLEAR_LINE)
	{
		switch (m_amstate)
		{
		case IDLE:
			break;
		case CREATE_PADDR:
			// Address has been created
			m_amstate = ADDR_MSW;
			break;
		case ADDR_MSW:
			// Transmit the first word (without the protection bits)
			m_mainboard->set_paddress((m_physical_address >> 16) & 0x00ff);
			m_amstate = ADDR_LSW;
			break;
		case ADDR_LSW:
			m_mainboard->msast_in(ASSERT_LINE); // Pulse MSAST
			m_mainboard->msast_in(CLEAR_LINE);
			m_mainboard->set_paddress(m_physical_address & 0xffff);
			m_amstate = IDLE;
			m_ready_out = m_srdy;   // Propagate incoming READY
			break;

		case SRAMLOAD:
			if (m_hold_acknowledged) mapper_load();
			break;

		case SRAMSAVE:
			if (m_hold_acknowledged) mapper_save();
			break;

		default:
			logerror("Invalid state in mapper: %d\n", m_amstate);
		}
	}
}

void amigo_device::mapper_load()
{
	m_mapvalue = (m_mapvalue << 8)  | m_sram[m_sram_address++];

	if ((m_sram_address & 0x03)==0)
	{
		if (TRACE_MAP) logerror("Loaded basereg %02d = %08x\n", m_basereg, m_mapvalue);
		m_base_register[m_basereg++] = m_mapvalue;
	}
	if (m_basereg == 16)
	{
		m_amstate = IDLE;
		m_mainboard->hold_cpu(CLEAR_LINE);
	}
}

void amigo_device::mapper_save()
{
	if ((m_sram_address & 0x03)==0)
	{
		if (m_basereg == 16)
		{
			m_amstate = IDLE;
			m_mainboard->hold_cpu(CLEAR_LINE);
			return;
		}
		else
		{
			m_mapvalue = m_base_register[m_basereg];
			if (TRACE_MAP) logerror("Saving basereg %02d = %08x\n", m_basereg, m_mapvalue);
			m_basereg++;
		}
	}

	m_sram[m_sram_address++] = (m_mapvalue >> 24) & 0xff;
	m_mapvalue = m_mapvalue << 8;
}

WRITE_LINE_MEMBER( amigo_device::holda_in )
{
	if (TRACE_MAP) logerror("HOLD acknowledged = %d\n", state);
	m_hold_acknowledged = (state==ASSERT_LINE);
}

void amigo_device::device_start()
{
	logerror("Starting\n");
	m_mainboard = downcast<mainboard8_device*>(owner());
}

void amigo_device::device_reset()
{
	m_logical_space = true;
}

const device_type AMIGO = &device_creator<amigo_device>;

/***************************************************************************

  ===== OSO: Hexbus interface =====

  The Hexbus is a 4-bit peripheral bus with master/slave coordination. Bytes
  are written over the bus in two passes. Hexbus was the designated standard
  peripheral bus for TI computers before TI left the home computer market.

  Existing devices are floppy drive, RS232 serial adapter, and
  a "Wafertape" drive (kind of tape streamer)

  Registers:  Read   Write  Bits of register
  ----------------------------------------------------------------------------
  Data     :  5FF8     -    ADB3  ADB2  ADB1    ADB0    ADB3  ADB2  ADB1  ADB0
  Status   :  5FFA     -    HSKWT HSKRD BAVIAS  BAVAIS  SBAV  WBUSY RBUSY SHSK
  Control  :  5FFC   5FFA   WIEN  RIEN  BAVIAEN BAVAIEN BAVC  WEN   REN   CR7
  Xmit     :  5FFE   5FF8   XDR0  XDR1  XDR2    XDR3    XDR4  XDR5  XDR6  XDR7

  ADBx = Hexbus data bit X
  HSKWT = Set when a byte has been sent over the bus and HSK has been asserted
  HSKRD = Set when a byte has been received
  BAVIAS = set when the BAV* signal (bus available) transits to active state
  BAVAIS = set when the BAV* signal transits to inactive state (=1)
  SBAV = set when BAV* = 0 (active)
  WBUSY = set when a write action is in progress (two transfers @ 4 bits)
  Reset when HSKWT is set
  RBUSY = set when a read action is in progress (two transfers @ 4 bits)
  Reset when HSKRD is set
  SHSK = set when HSK* is active (0)

  WIEN = Enable interrupt for write completion
  RIEN = Enable interrupt for read completion
  BAVIAEN = BAVIA enable (slave mode)
  BAVAIEN = BAVAI enable (slave mode)
  BAVC = set BAV* line (0=active)
  WEN = set write enable (byte is written from xmit reg)
  REN = set read enable (latch HSK and read byte into data reg)
  CR7 = future extension
  XDRx = transmit register bit

  Hexbus connector (console)
  +---+---+---+---+
  | 4 | 3 | 2 | 1 |      4 = L;    3 = BAV*; 2 = ADB1; 1 = ADB0
  +---+---+---+---+
  | 8 | 7 | 6 | 5 |      8 = ADB3; 7 = ADB2; 6 = nc;   5 = HSK*
  +---+---+---+---+

  TODO: This is just a preliminary implementation to satisfy the operating
        system. When completed we can hopefully emulate a Hexbus floppy and
        use it in Extended Basic II which refuses to work with the PEB cards.
        The Hexbus should then be designed as a slot device.

****************************************************************************/

/* Status register bits */
enum
{
	HSKWT = 0x80,
	HSKRD = 0x40,
	BAVIAS = 0x20,
	BAVAIS = 0x10,
	SBAV = 0x08,
	WBUSY = 0x04,
	RBUSY = 0x02,
	SHSK = 0x01
};

oso_device::oso_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: device_t(mconfig, OSO, "Hexbus interface", tag, owner, clock, "oso", __FILE__), m_data(0), m_status(0), m_control(0), m_xmit(0)
{
}

READ8_MEMBER( oso_device::read )
{
	int value = 0;
	offset &= 0x03;
	switch (offset)
	{
	case 0:
		// read 5FF8: read data register
		if (TRACE_OSO) logerror("Read data register = %02x\n", value);
		value = m_data;
		break;
	case 1:
		// read 5FFA: read status register
		value = m_status;
		if (TRACE_OSO) logerror("Read status %02x\n", value);
		break;
	case 2:
		// read 5FFC: read control register
		value = m_control;
		if (TRACE_OSO) logerror("Read control register = %02x\n", value);
		break;
	case 3:
		// read 5FFE: read transmit register
		value = m_xmit;
		if (TRACE_OSO) logerror("Read transmit register = %02x\n", value);
		break;
	}
	return value;
}

WRITE8_MEMBER( oso_device::write )
{
	offset &= 0x03;
	switch (offset)
	{
	case 0:
		// write 5FF8: write transmit register
		if (TRACE_OSO) logerror("Write transmit register %02x\n", data);
		m_xmit = data;
		// We set the status register directly in order to prevent lock-ups
		// until we have a complete Hexbus implementation
		m_status |= HSKWT;
		break;
	case 1:
		// write 5FFA: write control register
		if (TRACE_OSO) logerror("Write control register %02x\n", data);
		m_control = data;
		break;
	default:
		// write 5FFC, 5FFE: undefined
		if (TRACE_OSO) logerror("Invalid write on %04x: %02x\n", (offset<<1) | 0x5ff0, data);
		break;
	}
}

void oso_device::device_start()
{
	logerror("Starting\n");
	m_status = m_xmit = m_control = m_data = 0;
}

const device_type OSO = &device_creator<oso_device>;
