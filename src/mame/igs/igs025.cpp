// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
/*

 IGS025 is some kind of state machine / logic device which the game
 uses for various security checks, and to determine the region of the
 game based on string sequences.

 The IGS025 can _NOT_ be swapped between games, so contains at least
 some game specific configuration / programming even if there is a
 large amount of common behavior between games.

*/

#include "emu.h"
#include "igs025.h"


igs025_device::igs025_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IGS025, tag, owner, clock)
	, m_execute_external(*this, DEVICE_SELF, FUNC(igs025_device::no_callback_setup))
	, m_source_cb(*this)
	, m_game_id(0)
	, m_region(0)

	, m_prot_hold(0)
	, m_prot_hilo(0)
	, m_prot_hilo_select(0)

	, m_prot_cmd(0)
	, m_prot_reg(0)
	, m_prot_ptr(0)
	, m_prot_swap(0)

	, m_prot_bs(0)
	, m_prot_cmd3(0)
{
}

void igs025_device::no_callback_setup()
{
	logerror("%s: igs025 trigger external callback with no external callback setup\n", machine().describe_context());
}


void igs025_device::device_start()
{
	m_execute_external.resolve();
	m_source_cb.resolve_safe(0);

	// Reset IGS025 stuff
	m_prot_hold = 0;
	m_prot_hilo = 0;
	m_prot_hilo_select = 0;
	m_prot_cmd = 0;
	m_prot_reg = 0;
	m_prot_ptr = 0;
	m_prot_swap = 0;
	m_prot_bs = 0;
	m_prot_cmd3 = 0;

	save_item(NAME(m_game_id));
	save_item(NAME(m_region));

	save_item(NAME(m_prot_hold));
	save_item(NAME(m_prot_hilo));
	save_item(NAME(m_prot_hilo_select));
	save_item(NAME(m_prot_cmd));
	save_item(NAME(m_prot_reg));
	save_item(NAME(m_prot_ptr));
	save_item(NAME(m_prot_swap));
	save_item(NAME(m_prot_bs));
	save_item(NAME(m_prot_cmd3));
}

void igs025_device::device_reset()
{
	// Reset IGS025 stuff
	m_prot_hold = 0;
	m_prot_hilo = 0;
	m_prot_hilo_select = 0;
	m_prot_cmd = 0;
	m_prot_reg = 0;
	m_prot_ptr = 0;
	m_prot_swap = 0;

	m_prot_bs = 0;
	m_prot_cmd3 = 0;
}

/****************************************/
/* WRITE */
/****************************************/

void igs025_device::killbld_igs025_prot_w(offs_t offset, uint16_t data)
{
	if (offset == 0)
	{
		m_prot_cmd = data;
	}
	else
	{
		switch (m_prot_cmd)
		{
			case 0x00:
				m_prot_reg = data;
				break;

			case 0x01: // drgw3
				if (data == 0x0002)
				{ // Execute command
					//logerror("execute\n");
					m_execute_external();
				}
				break;

			case 0x02: // killbld
				if (data == 0x0001)
				{ // Execute command
					//logerror("execute\n");
					m_execute_external();
					m_prot_reg++;
				}
				break;

			case 0x03:
				m_prot_swap = data;
				break;

			case 0x04:
				//m_prot_ptr = data; // Suspect. Not good for drgw3
				break;

			case 0x20:
			case 0x21:
			case 0x22:
			case 0x23:
			case 0x24:
			case 0x25:
			case 0x26:
			case 0x27:
				m_prot_ptr++;
				protection_calculate_hold(m_prot_cmd & 0x0f, data & 0xff);
				break;

		//  default:
		//      logerror("%s: ASIC25 W CMD %X  VAL %X\n", machine().describe_context(), m_prot_cmd, data);
		}
	}
}

void igs025_device::olds_w(offs_t offset, uint16_t data)
{
	if (offset == 0)
	{
		m_prot_cmd = data;
	}
	else
	{
		switch (m_prot_cmd)
		{
			case 0x00:
				m_prot_reg = data;
				break;

			case 0x02:
				m_prot_bs = ((data & 0x03) << 6) | ((data & 0x04) << 3) | ((data & 0x08) << 1);
				break;

			case 0x03:
				m_execute_external();

				m_prot_cmd3 = ((data >> 4) + 1) & 0x3;
				break;

			case 0x04:
				m_prot_ptr = data;
				break;

			case 0x20:
			case 0x21:
			case 0x22:
			case 0x23:
			case 0x24:
			case 0x25:
			case 0x26:
			case 0x27:
				m_prot_ptr++;
				protection_calculate_hold(m_prot_cmd & 0x0f, data & 0xff);
				break;

		//  default:
		//      logerror("%s: unemulated write mode!\n", machine().describe_context());
		}
	}
}


void igs025_device::drgw2_prot_w(offs_t offset, uint16_t data)
{
	if (offset == 0)
	{
		m_prot_cmd = data;
		return;
	}

	switch (m_prot_cmd)
	{
		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
			m_prot_ptr++;
			protection_calculate_hold(m_prot_cmd & 0x0f, data & 0xff);
			break;

	//  case 0x08: // Used only on init..
	//  case 0x09:
	//  case 0x0a:
	//  case 0x0b:
	//  case 0x0c:
	//  break;

	//  case 0x15: // ????
	//  case 0x17:
	//  case 0xf2:
	//  break;

	//  default:
	//      logerror("%s: warning, writing to igs003_reg %02x = %02x\n", machine().describe_context(), m_prot_cmd, data);
	}
}

/****************************************/
/* READ */
/****************************************/

uint16_t igs025_device::prot_r(offs_t offset)
{
	if (offset)
	{
		switch (m_prot_cmd)
		{
		case 0x00:
			return bitswap<8>((m_prot_swap + 1) & 0x7f, 0, 1, 2, 3, 4, 5, 6, 7); // drgw3

		case 0x01:
			return m_prot_reg & 0x7f;

		case 0x02:
			return m_prot_bs | 0x80;

		case 0x03:
			return m_prot_cmd3;

		case 0x05:
			switch (m_prot_ptr)
			{
			case 1:
				return 0x3f00 | ((m_game_id >> 0) & 0xff);

			case 2:
				return 0x3f00 | ((m_game_id >> 8) & 0xff);

			case 3:
				return 0x3f00 | ((m_game_id >> 16) & 0xff);

			case 4:
				return 0x3f00 | ((m_game_id >> 24) & 0xff);

			default: // >= 5
				return 0x3f00 | bitswap<8>(m_prot_hold, 5, 2, 9, 7, 10, 13, 12, 15);
			}

		case 0x40:
			if (!machine().side_effects_disabled())
				protection_calculate_hilo();
			return 0; // Read and then discarded

			//  default:
			//      logerror("%s: ASIC25 R CMD %X\n", machine().describe_context(), m_prot_cmd);

			// drgw2 notes
			//  case 0x13: // Read to $80eeb8
			//  case 0x1f: // Read to $80eeb8
			//  case 0xf4: // Read to $80eeb8
			//  case 0xf6: // Read to $80eeb8
			//  case 0xf8: // Read to $80eeb8
			//      return 0;

			//  default:
			//      logerror("%s: warning, reading with igs003_reg = %02x\n", machine().describe_context(), m_prot_cmd);


		}
	}

	return 0;
}


void igs025_device::protection_calculate_hold(int y, int z)
{
	const uint16_t old = m_prot_hold;

	m_prot_hold = ((old << 1) | (old >> 15));

	m_prot_hold ^= 0x2bad;
	m_prot_hold ^= BIT(z, y);
	m_prot_hold ^= BIT(old, 7) << 0;
	m_prot_hold ^= BIT(~old, 13) << 4;
	m_prot_hold ^= BIT(old, 3) << 11;

	m_prot_hold ^= (m_prot_hilo & ~0x0408) << 1;
}

void igs025_device::protection_calculate_hilo()
{
	m_prot_hilo_select++;

	if (m_prot_hilo_select > 0xeb)
	{
		m_prot_hilo_select = 0;
	}

	const uint8_t source = m_source_cb(m_region, m_prot_hilo_select);

	if (m_prot_hilo_select & 1)
	{
		m_prot_hilo = (m_prot_hilo & 0x00ff) | (source << 8);
	}
	else
	{
		m_prot_hilo = (m_prot_hilo & 0xff00) | source;
	}
}


DEFINE_DEVICE_TYPE(IGS025, igs025_device, "igs025", "IGS025")
