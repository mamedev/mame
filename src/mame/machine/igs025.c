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


igs025_device::igs025_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, IGS025, "IGS025", tag, owner, clock, "igs_025_022", __FILE__)
{
	m_execute_external =  igs025_execute_external(FUNC(igs025_device::no_callback_setup), this);
}

void igs025_device::device_config_complete()
{
}

void igs025_device::device_validity_check(validity_checker &valid) const
{
}

void igs025_device::no_callback_setup()
{
	printf("igs025 trigger external callback with no external callback setup\n");
}

	

void igs025_device::set_external_cb(device_t &device,igs025_execute_external newcb)
{
	//printf("set_external_cb\n");
	igs025_device &dev = downcast<igs025_device &>(device);
	dev.m_execute_external = newcb;
}


void igs025_device::device_start()
{
	// Reset IGS025 stuff
	m_kb_prot_hold = 0;
	m_kb_prot_hilo = 0;
	m_kb_prot_hilo_select = 0;
	m_kb_cmd = 0;
	m_kb_reg = 0;
	m_kb_ptr = 0;
	m_kb_swap = 0;

	m_execute_external.bind_relative_to(*owner());

	save_item(NAME(m_kb_prot_hold));
	save_item(NAME(m_kb_prot_hilo));
	save_item(NAME(m_kb_prot_hilo_select));
	save_item(NAME(m_kb_cmd));
	save_item(NAME(m_kb_reg));
	save_item(NAME(m_kb_ptr));
}

void igs025_device::device_reset()
{
	// Reset IGS025 stuff
	m_kb_prot_hold = 0;
	m_kb_prot_hilo = 0;
	m_kb_prot_hilo_select = 0;
	m_kb_cmd = 0;
	m_kb_reg = 0;
	m_kb_ptr = 0;
	m_kb_swap = 0;
}

void igs025_device::killbld_protection_calculate_hold(int y, int z)
{
	unsigned short old = m_kb_prot_hold;

	m_kb_prot_hold = ((old << 1) | (old >> 15));

	m_kb_prot_hold ^= 0x2bad;
	m_kb_prot_hold ^= BIT(z, y);
	m_kb_prot_hold ^= BIT( old,  7) <<  0;
	m_kb_prot_hold ^= BIT(~old, 13) <<  4;
	m_kb_prot_hold ^= BIT( old,  3) << 11;

	m_kb_prot_hold ^= (m_kb_prot_hilo & ~0x0408) << 1;
}

void igs025_device::killbld_protection_calculate_hilo()
{
	UINT8 source;

	m_kb_prot_hilo_select++;

	if (m_kb_prot_hilo_select > 0xeb) {
		m_kb_prot_hilo_select = 0;
	}

	source = m_kb_source_data[(ioport(":Region")->read() - m_kb_source_data_offset)][m_kb_prot_hilo_select];

	if (m_kb_prot_hilo_select & 1)
	{
		m_kb_prot_hilo = (m_kb_prot_hilo & 0x00ff) | (source << 8);
	}
	else
	{
		m_kb_prot_hilo = (m_kb_prot_hilo & 0xff00) | (source << 0);
	}
}

WRITE16_MEMBER(igs025_device::killbld_igs025_prot_w )
{
	if (offset == 0)
	{
		m_kb_cmd = data;
	}
	else
	{
		switch (m_kb_cmd)
		{
			case 0x00:
				m_kb_reg = data;
			break;

			case 0x01: // drgw3
			{
				if (data == 0x0002) { // Execute command
					//printf("execute\n");
					m_execute_external();	
				}
			}
			break;

			case 0x02: // killbld
			{
				if (data == 0x0001) { // Execute command
					//printf("execute\n");
					m_execute_external();	
					m_kb_reg++;
				}
			}
			break;

			case 0x03:
				m_kb_swap = data;
			break;

			case 0x04:
		//		m_kb_ptr = data; // Suspect. Not good for drgw3
			break;

			case 0x20:
			case 0x21:
			case 0x22:
			case 0x23:
			case 0x24:
			case 0x25:
			case 0x26:
			case 0x27:
				m_kb_ptr++;
				killbld_protection_calculate_hold(m_kb_cmd & 0x0f, data & 0xff);
			break;

		//	default:
		//		logerror("%06X: ASIC25 W CMD %X  VAL %X\n", space.device().safe_pc(), m_kb_cmd, data);
		}
	}
}

READ16_MEMBER(igs025_device::killbld_igs025_prot_r )
{
	if (offset)
	{
		switch (m_kb_cmd)
		{
			case 0x00:
				return BITSWAP8((m_kb_swap+1) & 0x7f, 0,1,2,3,4,5,6,7); // drgw3

			case 0x01:
				return m_kb_reg & 0x7f;

			case 0x05:
			{
				switch (m_kb_ptr)
				{
					case 1:
						return 0x3f00 | ioport(":Region")->read();

					case 2:
						return 0x3f00 | ((m_kb_game_id >> 8) & 0xff);

					case 3:
						return 0x3f00 | ((m_kb_game_id >> 16) & 0xff);

					case 4:
						return 0x3f00 | ((m_kb_game_id >> 24) & 0xff);

					default: // >= 5
						return 0x3f00 | BITSWAP8(m_kb_prot_hold, 5,2,9,7,10,13,12,15);
				}

				return 0;
			}

			case 0x40:
				killbld_protection_calculate_hilo();
				return 0; // Read and then discarded

		//	default:
		//		logerror("%06X: ASIC25 R CMD %X\n", space.device().safe_pc(), m_kb_cmd);
		}
	}

	return 0;
}




const device_type IGS025 = &device_creator<igs025_device>;



