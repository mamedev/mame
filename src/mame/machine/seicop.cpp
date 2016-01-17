// license:LGPL-2.1+
// copyright-holders:Olivier Galibert, Angelo Salese, David Haywood, Tomasz Slanina

/*
  OLD Seibu Cop simulation code.

  this is currently only used by the Seibu Cup Soccer BOOTLEG

*/

/********************************************************************************************

  COPX bootleg simulation
    - Seibu Cup Soccer (bootleg)

 *******************************************************************************************/

#include "emu.h"
#include "seicop.h"
#include "includes/legionna.h"


const device_type SEIBU_COP_BOOTLEG = &device_creator<seibu_cop_bootleg_device>;

seibu_cop_bootleg_device::seibu_cop_bootleg_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SEIBU_COP_BOOTLEG, "Seibu COP (Bootleg)", tag, owner, clock, "seibu_cop_boot", __FILE__),
	m_cop_mcu_ram(nullptr),
	m_raiden2cop(*this, ":raiden2cop")
{
}

#define seibu_cop_log logerror

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void seibu_cop_bootleg_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void seibu_cop_bootleg_device::device_start()
{
	m_cop_mcu_ram = reinterpret_cast<UINT16 *>(machine().root_device().memshare("cop_mcu_ram")->ptr());

}

//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void seibu_cop_bootleg_device::device_reset()
{
}



READ16_MEMBER( seibu_cop_bootleg_device::copdxbl_0_r )
{
	UINT16 retvalue = m_cop_mcu_ram[offset];

	switch(offset)
	{
		default:
		{
			logerror("%06x: COPX unhandled read returning %04x from offset %04x\n", space.device().safe_pc(), retvalue, offset*2);
			return retvalue;
		}



		//case (0x47e/2):
		//case (0x5b0/2):
		//case (0x5b4/2):
		//  return m_cop_mcu_ram[offset];

		case (0x700/2): return space.machine().root_device().ioport("DSW1")->read();
		case (0x704/2): return space.machine().root_device().ioport("PLAYERS12")->read();
		case (0x708/2): return space.machine().root_device().ioport("PLAYERS34")->read();
		case (0x70c/2): return space.machine().root_device().ioport("SYSTEM")->read();
		case (0x71c/2): return space.machine().root_device().ioport("DSW2")->read();
	}
}

WRITE16_MEMBER( seibu_cop_bootleg_device::copdxbl_0_w )
{
	legionna_state *state = space.machine().driver_data<legionna_state>();
	COMBINE_DATA(&m_cop_mcu_ram[offset]);

	switch(offset)
	{
		default:
		{
			logerror("%06x: COPX unhandled write data %04x at offset %04x\n", space.device().safe_pc(), data, offset*2);
			break;
		}


		case (0x500/2):
		case (0x502/2):
		case (0x504/2):
			switch(m_cop_mcu_ram[offset])
			{
				case 0x8100:
				{
					int raw_angle = (space.read_word(m_raiden2cop->cop_regs[0]+(0x34^2)) & 0xff);
					double angle = raw_angle * M_PI / 128;
					double amp = (65536 >> 5)*(space.read_word(m_raiden2cop->cop_regs[0]+(0x36^2)) & 0xff);
					int res;

				/* TODO: up direction, why? */
					if(raw_angle == 0xc0)
						amp*=2;

					res = int(amp*sin(angle)) << m_raiden2cop->cop_scale;

					space.write_dword(m_raiden2cop->cop_regs[0] + 0x10, res);

					break;
				}
				case 0x8900:
				{
					int raw_angle = (space.read_word(m_raiden2cop->cop_regs[0]+(0x34^2)) & 0xff);
					double angle = raw_angle * M_PI / 128;
					double amp = (65536 >> 5)*(space.read_word(m_raiden2cop->cop_regs[0]+(0x36^2)) & 0xff);
					int res;

					/* TODO: left direction, why? */
					if(raw_angle == 0x80)
						amp*=2;

					res = int(amp*cos(angle)) << m_raiden2cop->cop_scale;

					space.write_dword(m_raiden2cop->cop_regs[0] + 20, res);

					break;
				}
				case 0x0205:
				{
					UINT8 offs;

					offs = (offset & 3) * 4;
					int ppos = space.read_dword(m_raiden2cop->cop_regs[0] + 4 + offs);
					int npos = ppos + space.read_dword(m_raiden2cop->cop_regs[0] + 0x10 + offs);
					int delta = (npos >> 16) - (ppos >> 16);

					space.write_dword(m_raiden2cop->cop_regs[0] + 4 + offs, npos);
					space.write_word(m_raiden2cop->cop_regs[0] + 0x1c + offs, space.read_word(m_raiden2cop->cop_regs[0] + 0x1c + offs) + delta);

					break;
				}
				case 0x130e:
				case 0x138e:
				{
					int dy = space.read_dword(m_raiden2cop->cop_regs[1]+4) - space.read_dword(m_raiden2cop->cop_regs[0]+4);
					int dx = space.read_dword(m_raiden2cop->cop_regs[1]+8) - space.read_dword(m_raiden2cop->cop_regs[0]+8);

					m_raiden2cop->cop_status = 7;
					if(!dx) {
						m_raiden2cop->cop_status |= 0x8000;
						m_raiden2cop->cop_angle = 0;
					} else {
						m_raiden2cop->cop_angle = atan(double(dy)/double(dx)) * 128.0 / M_PI;
						if(dx<0)
							m_raiden2cop->cop_angle += 0x80;
					}

					m_raiden2cop->m_LEGACY_r0 = dy;
					m_raiden2cop->m_LEGACY_r1 = dx;

					if(m_cop_mcu_ram[offset] & 0x80)
						space.write_word(m_raiden2cop->cop_regs[0]+(0x34^2), m_raiden2cop->cop_angle);

					break;
				}
				case 0x3b30:
				case 0x3bb0:
				{
					int dy = m_raiden2cop->m_LEGACY_r0;
					int dx = m_raiden2cop->m_LEGACY_r1;

					dx >>= 16;
					dy >>= 16;
					m_raiden2cop->cop_dist = sqrt((double)(dx*dx+dy*dy));

					if(m_cop_mcu_ram[offset] & 0x80)
						space.write_word(m_raiden2cop->cop_regs[0]+(0x38), m_raiden2cop->cop_dist);

					break;
				}
				default:
					printf("%04x\n",m_cop_mcu_ram[offset]);
					break;
			}
			break;

		/*TODO: kludge on x-axis.*/
		case (0x660/2): { state->m_scrollram16[0] = m_cop_mcu_ram[offset] - 0x1f0; break; }
		case (0x662/2): { state->m_scrollram16[1] = m_cop_mcu_ram[offset]; break; }
		case (0x664/2): { state->m_scrollram16[2] = m_cop_mcu_ram[offset] - 0x1f0; break; }
		case (0x666/2): { state->m_scrollram16[3] = m_cop_mcu_ram[offset]; break; }
		case (0x668/2): { state->m_scrollram16[4] = m_cop_mcu_ram[offset] - 0x1f0; break; }
		case (0x66a/2): { state->m_scrollram16[5] = m_cop_mcu_ram[offset]; break; }
		case (0x66c/2): { state->m_scrollram16[6] = m_cop_mcu_ram[offset] - 0x1f0; break; }
		case (0x66e/2): { state->m_scrollram16[7] = m_cop_mcu_ram[offset]; break; }

		case (0x740/2):
		{
			state->soundlatch_byte_w(space, 0, data & 0xff);
			state->m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE );
			break;
		}
	}
}
