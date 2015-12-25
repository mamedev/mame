// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Seiko-Epson SED1330 LCD Controller emulation

**********************************************************************/

#include "emu.h"
#include "sed1330.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0


#define INSTRUCTION_SYSTEM_SET      0x40
#define INSTRUCTION_SLEEP_IN        0x53    // unimplemented
#define INSTRUCTION_DISP_ON         0x59
#define INSTRUCTION_DISP_OFF        0x58
#define INSTRUCTION_SCROLL          0x44
#define INSTRUCTION_CSRFORM         0x5d
#define INSTRUCTION_CGRAM_ADR       0x5c
#define INSTRUCTION_CSRDIR_RIGHT    0x4c
#define INSTRUCTION_CSRDIR_LEFT     0x4d
#define INSTRUCTION_CSRDIR_UP       0x4e
#define INSTRUCTION_CSRDIR_DOWN     0x4f
#define INSTRUCTION_HDOT_SCR        0x5a
#define INSTRUCTION_OVLAY           0x5b
#define INSTRUCTION_CSRW            0x46
#define INSTRUCTION_CSRR            0x47    // unimplemented
#define INSTRUCTION_MWRITE          0x42
#define INSTRUCTION_MREAD           0x43    // unimplemented


#define CSRDIR_RIGHT                0x00
#define CSRDIR_LEFT                 0x01
#define CSRDIR_UP                   0x02
#define CSRDIR_DOWN                 0x03


#define MX_OR                       0x00
#define MX_XOR                      0x01    // unimplemented
#define MX_AND                      0x02    // unimplemented
#define MX_PRIORITY_OR              0x03    // unimplemented


#define FC_OFF                      0x00
#define FC_SOLID                    0x01    // unimplemented
#define FC_FLASH_32                 0x02    // unimplemented
#define FC_FLASH_64                 0x03    // unimplemented



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
const device_type SED1330 = &device_creator<sed1330_device>;


// default address map
static ADDRESS_MAP_START( sed1330, AS_0, 8, sed1330_device )
	AM_RANGE(0x0000, 0xffff) AM_RAM
ADDRESS_MAP_END


// internal character generator ROM
ROM_START( sed1330 )
	ROM_REGION( 0x5c0, "gfx1", 0 ) // internal chargen ROM
	ROM_LOAD( "sed1330.bin", 0x000, 0x5c0, NO_DUMP )
ROM_END



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  readbyte - read a byte at the given address
//-------------------------------------------------

inline UINT8 sed1330_device::readbyte(offs_t address)
{
	return space().read_byte(address);
}


//-------------------------------------------------
//  writebyte - write a byte at the given address
//-------------------------------------------------

inline void sed1330_device::writebyte(offs_t address, UINT8 data)
{
	space().write_byte(address, data);
}


//-------------------------------------------------
//  increment_csr - increment cursor address
//-------------------------------------------------

inline void sed1330_device::increment_csr()
{
	switch (m_cd)
	{
	case CSRDIR_RIGHT:
		m_csr++;
		break;

	case CSRDIR_LEFT:
		m_csr--;
		break;

	case CSRDIR_UP:
		m_csr -= m_ap;
		break;

	case CSRDIR_DOWN:
		m_csr += m_ap;
		break;
	}
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sed1330_device - constructor
//-------------------------------------------------

sed1330_device::sed1330_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SED1330, "SED1330", tag, owner, clock, "sed1330", __FILE__),
		device_memory_interface(mconfig, *this),
		device_video_interface(mconfig, *this),
		m_bf(0),
		m_space_config("videoram", ENDIANNESS_LITTLE, 8, 16, 0, nullptr, *ADDRESS_MAP_NAME(sed1330))
{
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *sed1330_device::device_rom_region() const
{
	return ROM_NAME( sed1330 );
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sed1330_device::device_start()
{
	// register for state saving
	save_item(NAME(m_bf));
	save_item(NAME(m_ir));
	save_item(NAME(m_dor));
	save_item(NAME(m_pbc));
	save_item(NAME(m_d));
	save_item(NAME(m_sleep));
	save_item(NAME(m_sag));
	save_item(NAME(m_m0));
	save_item(NAME(m_m1));
	save_item(NAME(m_m2));
	save_item(NAME(m_ws));
	save_item(NAME(m_iv));
	save_item(NAME(m_wf));
	save_item(NAME(m_fx));
	save_item(NAME(m_fy));
	save_item(NAME(m_cr));
	save_item(NAME(m_tcr));
	save_item(NAME(m_lf));
	save_item(NAME(m_ap));
	save_item(NAME(m_sad1));
	save_item(NAME(m_sad2));
	save_item(NAME(m_sad3));
	save_item(NAME(m_sad4));
	save_item(NAME(m_sl1));
	save_item(NAME(m_sl2));
	save_item(NAME(m_hdotscr));
	save_item(NAME(m_csr));
	save_item(NAME(m_cd));
	save_item(NAME(m_crx));
	save_item(NAME(m_cry));
	save_item(NAME(m_cm));
	save_item(NAME(m_fc));
	save_item(NAME(m_fp));
	save_item(NAME(m_mx));
	save_item(NAME(m_dm));
	save_item(NAME(m_ov));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void sed1330_device::device_reset()
{
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

const address_space_config *sed1330_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : nullptr;
}


//-------------------------------------------------
//  status_r -
//-------------------------------------------------

READ8_MEMBER( sed1330_device::status_r )
{
	if (LOG) logerror("SED1330 '%s' Status Read: %s\n", tag(), m_bf ? "busy" : "ready");

	return m_bf << 6;
}


//-------------------------------------------------
//  command_w -
//-------------------------------------------------

WRITE8_MEMBER( sed1330_device::command_w )
{
	m_ir = data;
	m_pbc = 0;

	switch (m_ir)
	{
#if 0
	case INSTRUCTION_SLEEP_IN:
		break;
#endif
	case INSTRUCTION_CSRDIR_RIGHT:
	case INSTRUCTION_CSRDIR_LEFT:
	case INSTRUCTION_CSRDIR_UP:
	case INSTRUCTION_CSRDIR_DOWN:
		m_cd = data & 0x03;

		if (LOG)
		{
			switch (m_cd)
			{
			case CSRDIR_RIGHT:  logerror("SED1330 '%s' Cursor Direction: Right\n", tag());  break;
			case CSRDIR_LEFT:   logerror("SED1330 '%s' Cursor Direction: Left\n", tag());   break;
			case CSRDIR_UP:     logerror("SED1330 '%s' Cursor Direction: Up\n", tag());     break;
			case CSRDIR_DOWN:   logerror("SED1330 '%s' Cursor Direction: Down\n", tag());   break;
			}
		}
		break;
	}
}


//-------------------------------------------------
//  data_r -
//-------------------------------------------------

READ8_MEMBER( sed1330_device::data_r )
{
	UINT8 data = readbyte(m_csr);

	if (LOG) logerror("SED1330 '%s' Memory Read %02x from %04x\n", tag(), data, m_csr);

	increment_csr();

	return data;
}


//-------------------------------------------------
//  data_w -
//-------------------------------------------------

WRITE8_MEMBER( sed1330_device::data_w )
{
	switch (m_ir)
	{
	case INSTRUCTION_SYSTEM_SET:
		switch (m_pbc)
		{
		case 0:
			m_m0 = BIT(data, 0);
			m_m1 = BIT(data, 1);
			m_m2 = BIT(data, 2);
			m_ws = BIT(data, 3);
			m_iv = BIT(data, 5);

			if (LOG)
			{
				logerror("SED1330 '%s' %s CG ROM\n", tag(), BIT(data, 0) ? "External" : "Internal");
				logerror("SED1330 '%s' D6 Correction: %s\n", tag(), BIT(data, 1) ? "enabled" : "disabled");
				logerror("SED1330 '%s' Character Height: %u\n", tag(), BIT(data, 2) ? 16 : 8);
				logerror("SED1330 '%s' %s Panel Drive\n", tag(), BIT(data, 3) ? "Dual" : "Single");
				logerror("SED1330 '%s' Screen Top-Line Correction: %s\n", tag(), BIT(data, 5) ? "disabled" : "enabled");
			}
			break;

		case 1:
			m_fx = (data & 0x07) + 1;
			m_wf = BIT(data, 7);

			if (LOG)
			{
				logerror("SED1330 '%s' Horizontal Character Size: %u\n", tag(), m_fx);
				logerror("SED1330 '%s' %s AC Drive\n", tag(), BIT(data, 7) ? "2-frame" : "16-line");
			}
			break;

		case 2:
			m_fy = (data & 0x0f) + 1;
			if (LOG) logerror("SED1330 '%s' Vertical Character Size: %u\n", tag(), m_fy);
			break;

		case 3:
			m_cr = data + 1;
			if (LOG) logerror("SED1330 '%s' Visible Characters Per Line: %u\n", tag(), m_cr);
			break;

		case 4:
			m_tcr = data + 1;
			if (LOG) logerror("SED1330 '%s' Total Characters Per Line: %u\n", tag(), m_tcr);
			break;

		case 5:
			m_lf = data + 1;
			if (LOG) logerror("SED1330 '%s' Frame Height: %u\n", tag(), m_lf);
			break;

		case 6:
			m_ap = (m_ap & 0xff00) | data;
			break;

		case 7:
			m_ap = (data << 8) | (m_ap & 0xff);
			if (LOG) logerror("SED1330 '%s' Virtual Screen Width: %u\n", tag(), m_ap);
			break;

		default:
			logerror("SED1330 '%s' Invalid parameter byte %02x\n", tag(), data);
		}
		break;

	case INSTRUCTION_DISP_ON:
	case INSTRUCTION_DISP_OFF:
		m_d = BIT(data, 0);
		m_fc = data & 0x03;
		m_fp = data >> 2;
		if (LOG)
		{
			logerror("SED1330 '%s' Display: %s\n", tag(), BIT(data, 0) ? "enabled" : "disabled");

			switch (m_fc)
			{
			case FC_OFF:        logerror("SED1330 '%s' Cursor: disabled\n", tag()); break;
			case FC_SOLID:      logerror("SED1330 '%s' Cursor: solid\n", tag());        break;
			case FC_FLASH_32:   logerror("SED1330 '%s' Cursor: fFR/32\n", tag());       break;
			case FC_FLASH_64:   logerror("SED1330 '%s' Cursor: fFR/64\n", tag());       break;
			}

			switch (m_fp & 0x03)
			{
			case FC_OFF:        logerror("SED1330 '%s' Display Page 1: disabled\n", tag());     break;
			case FC_SOLID:      logerror("SED1330 '%s' Display Page 1: enabled\n", tag());      break;
			case FC_FLASH_32:   logerror("SED1330 '%s' Display Page 1: flash fFR/32\n", tag()); break;
			case FC_FLASH_64:   logerror("SED1330 '%s' Display Page 1: flash fFR/64\n", tag()); break;
			}

			switch ((m_fp >> 2) & 0x03)
			{
			case FC_OFF:        logerror("SED1330 '%s' Display Page 2/4: disabled\n", tag());       break;
			case FC_SOLID:      logerror("SED1330 '%s' Display Page 2/4: enabled\n", tag());        break;
			case FC_FLASH_32:   logerror("SED1330 '%s' Display Page 2/4: flash fFR/32\n", tag());   break;
			case FC_FLASH_64:   logerror("SED1330 '%s' Display Page 2/4: flash fFR/64\n", tag());   break;
			}

			switch ((m_fp >> 4) & 0x03)
			{
			case FC_OFF:        logerror("SED1330 '%s' Display Page 3: disabled\n", tag());     break;
			case FC_SOLID:      logerror("SED1330 '%s' Display Page 3: enabled\n", tag());      break;
			case FC_FLASH_32:   logerror("SED1330 '%s' Display Page 3: flash fFR/32\n", tag()); break;
			case FC_FLASH_64:   logerror("SED1330 '%s' Display Page 3: flash fFR/64\n", tag()); break;
			}
		}
		break;

	case INSTRUCTION_SCROLL:
		switch (m_pbc)
		{
		case 0:
			m_sad1 = (m_sad1 & 0xff00) | data;
			break;

		case 1:
			m_sad1 = (data << 8) | (m_sad1 & 0xff);
			if (LOG) logerror("SED1330 '%s' Display Page 1 Start Address: %04x\n", tag(), m_sad1);
			break;

		case 2:
			m_sl1 = data + 1;
			if (LOG) logerror("SED1330 '%s' Display Block 1 Screen Lines: %u\n", tag(), m_sl1);
			break;

		case 3:
			m_sad2 = (m_sad2 & 0xff00) | data;
			break;

		case 4:
			m_sad2 = (data << 8) | (m_sad2 & 0xff);
			if (LOG) logerror("SED1330 '%s' Display Page 2 Start Address: %04x\n", tag(), m_sad2);
			break;

		case 5:
			m_sl2 = data + 1;
			if (LOG) logerror("SED1330 '%s' Display Block 2 Screen Lines: %u\n", tag(), m_sl2);
			break;

		case 6:
			m_sad3 = (m_sad3 & 0xff00) | data;
			break;

		case 7:
			m_sad3 = (data << 8) | (m_sad3 & 0xff);
			if (LOG) logerror("SED1330 '%s' Display Page 3 Start Address: %04x\n", tag(), m_sad3);
			break;

		case 8:
			m_sad4 = (m_sad4 & 0xff00) | data;
			break;

		case 9:
			m_sad4 = (data << 8) | (m_sad4 & 0xff);
			if (LOG) logerror("SED1330 '%s' Display Page 4 Start Address: %04x\n", tag(), m_sad4);
			break;

		default:
			logerror("SED1330 '%s' Invalid parameter byte %02x\n", tag(), data);
		}
		break;

	case INSTRUCTION_CSRFORM:
		switch (m_pbc)
		{
		case 0:
			m_crx = (data & 0x0f) + 1;
			if (LOG) logerror("SED1330 '%s' Horizontal Cursor Size: %u\n", tag(), m_crx);
			break;

		case 1:
			m_cry = (data & 0x0f) + 1;
			m_cm = BIT(data, 7);
			if (LOG)
			{
				logerror("SED1330 '%s' Vertical Cursor Location: %u\n", tag(), m_cry);
				logerror("SED1330 '%s' Cursor Shape: %s\n", tag(), BIT(data, 7) ? "Block" : "Underscore");
			}
			break;

		default:
			logerror("SED1330 '%s' Invalid parameter byte %02x\n", tag(), data);
		}
		break;

	case INSTRUCTION_CGRAM_ADR:
		switch (m_pbc)
		{
		case 0:
			m_sag = (m_sag & 0xff00) | data;
			break;

		case 1:
			m_sag = (data << 8) | (m_sag & 0xff);
			if (LOG) logerror("SED1330 '%s' Character Generator RAM Start Address: %04x\n", tag(), m_sag);
			break;

		default:
			logerror("SED1330 '%s' Invalid parameter byte %02x\n", tag(), data);
		}
		break;

	case INSTRUCTION_HDOT_SCR:
		m_hdotscr = data & 0x07;
		if (LOG) logerror("SED1330 '%s' Horizontal Dot Scroll: %u\n", tag(), m_hdotscr);
		break;

	case INSTRUCTION_OVLAY:
		m_mx = data & 0x03;
		m_dm = (data >> 2) & 0x03;
		m_ov = BIT(data, 4);

		if (LOG)
		{
			switch (m_mx)
			{
			case MX_OR:             logerror("SED1330 '%s' Display Composition Method: OR\n", tag());               break;
			case MX_XOR:            logerror("SED1330 '%s' Display Composition Method: Exclusive-OR\n", tag()); break;
			case MX_AND:            logerror("SED1330 '%s' Display Composition Method: AND\n", tag());          break;
			case MX_PRIORITY_OR:    logerror("SED1330 '%s' Display Composition Method: Priority-OR\n", tag());  break;
			}

			logerror("SED1330 '%s' Display Page 1 Mode: %s\n", tag(), BIT(data, 2) ? "Graphics" : "Text");
			logerror("SED1330 '%s' Display Page 3 Mode: %s\n", tag(), BIT(data, 3) ? "Graphics" : "Text");
			logerror("SED1330 '%s' Display Composition Layers: %u\n", tag(), BIT(data, 4) ? 3 : 2);
		}
		break;

	case INSTRUCTION_CSRW:
		switch (m_pbc)
		{
		case 0:
			m_csr = (m_csr & 0xff00) | data;
			break;

		case 1:
			m_csr = (data << 8) | (m_csr & 0xff);
			if (LOG) logerror("SED1330 '%s' Cursor Address %04x\n", tag(), m_csr);
			break;

		default:
			logerror("SED1330 '%s' Invalid parameter byte %02x\n", tag(), data);
		}
		break;
#if 0
	case INSTRUCTION_CSRR:
		break;
#endif
	case INSTRUCTION_MWRITE:
		if (LOG) logerror("SED1330 '%s' Memory Write %02x to %04x (row %u col %u line %u)\n", tag(), data, m_csr, m_csr/80/8, m_csr%80, m_csr/80);

		writebyte(m_csr, data);

		increment_csr();
		break;
#if 0
	case INSTRUCTION_MREAD:
		break;
#endif
	default:
		logerror("SED1330 '%s' Unsupported instruction %02x\n", tag(), m_ir);
	}

	m_pbc++;
}


//-------------------------------------------------
//  draw_text_scanline -
//-------------------------------------------------

void sed1330_device::draw_text_scanline(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, UINT16 va)
{
	int sx, x;

	for (sx = 0; sx < m_cr; sx++)
	{
		if ((va + sx) == m_csr)
		{
			if (m_fc == FC_OFF) continue;

			if (m_cm)
			{
				// block cursor
				if (y % m_fy < m_cry)
				{
					for (x = 0; x < m_crx; x++)
					{
						bitmap.pix16(y, (sx * m_fx) + x) = 1;
					}
				}
			}
			else
			{
				// underscore cursor
				if (y % m_fy == m_cry)
				{
					for (x = 0; x < m_crx; x++)
					{
						bitmap.pix16(y, (sx * m_fx) + x) = 1;
					}
				}
			}
		}
	}
}


//-------------------------------------------------
//  draw_graphics_scanline -
//-------------------------------------------------

void sed1330_device::draw_graphics_scanline(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, UINT16 va)
{
	int sx, x;

	for (sx = 0; sx < m_cr; sx++)
	{
		UINT8 data = readbyte(va++);

		for (x = 0; x < m_fx; x++)
		{
			bitmap.pix16(y, (sx * m_fx) + x) = BIT(data, 7);
			data <<= 1;
		}
	}
}


//-------------------------------------------------
//  update_graphics -
//-------------------------------------------------

void sed1330_device::update_graphics(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
}


//-------------------------------------------------
//  update_text -
//-------------------------------------------------

void sed1330_device::update_text(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y;

	if (m_ws)
	{
		for (y = 0; y < m_sl1; y++)
		{
			UINT16 sad1 = m_sad1 + ((y / m_fy) * m_ap);
			UINT16 sad2 = m_sad2 + (y * m_ap);
			UINT16 sad3 = m_sad3 + ((y / m_fy) * m_ap);
			UINT16 sad4 = m_sad4 + (y * m_ap);

			// draw graphics display page 2 scanline
			draw_graphics_scanline(bitmap, cliprect, y, sad2);

			// draw text display page 1 scanline
			draw_text_scanline(bitmap, cliprect, y, sad1);

			// draw graphics display page 4 scanline
			draw_graphics_scanline(bitmap, cliprect, y + m_sl1, sad4);

			// draw text display page 3 scanline
			draw_text_scanline(bitmap, cliprect, y + m_sl1, sad3);
		}
	}
}


//-------------------------------------------------
//  screen_update -
//-------------------------------------------------

UINT32 sed1330_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_d)
	{
		if (m_dm)
		{
			update_graphics(bitmap, cliprect);
		}
		else
		{
			update_text(bitmap, cliprect);
		}
	}
	return 0;
}
