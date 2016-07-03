// license:LGPL-2.1+
// copyright-holders:Olivier Galibert, Angelo Salese, David Haywood, Tomasz Slanina
/********************************************************************************************

  COPDX bootleg simulation
    - Seibu Cup Soccer (bootleg)

  Notice that only the bare minimum is supported, which is what the bootleg device actually 
  do.

********************************************************************************************/

#include "emu.h"
#include "seicop.h"
#include "includes/legionna.h"


const device_type SEIBU_COP_BOOTLEG = &device_creator<seibu_cop_bootleg_device>;

READ16_MEMBER(seibu_cop_bootleg_device::reg_lo_addr_r)
{
	return m_reg[offset] & 0xffff;
}

READ16_MEMBER(seibu_cop_bootleg_device::reg_hi_addr_r)
{
	return m_reg[offset] >> 16;
}

WRITE16_MEMBER(seibu_cop_bootleg_device::reg_lo_addr_w)
{
	m_reg[offset] = (m_reg[offset] & 0xffff0000) | (data & 0xffff);
}

WRITE16_MEMBER(seibu_cop_bootleg_device::reg_hi_addr_w)
{
	m_reg[offset] = (m_reg[offset] & 0xffff) | (data << 16);
}

WRITE16_MEMBER(seibu_cop_bootleg_device::cmd_trigger_w)
{
	//printf("%04x %08x %08x\n",data,m_reg[0],m_reg[1]);
	switch(data)
	{
		default:
			//printf("trigger write %04x\n",data);
			break;
		case 0x0000:
			break;

		case 0x0205:
		{
			UINT8 offs;

			offs = (offset & 3) * 4;
			int ppos = m_host_space->read_dword(m_reg[0] + 4 + offs);
			int npos = ppos + m_host_space->read_dword(m_reg[0] + 0x10 + offs);
			int delta = (npos >> 16) - (ppos >> 16);

			m_host_space->write_dword(m_reg[0] + 4 + offs, npos);
			m_host_space->write_word(m_reg[0] + 0x1c + offs, m_host_space->read_word(m_reg[0] + 0x1c + offs) + delta);
			break;
		}
		
		case 0xe18e:
		{
			int dy = m_host_space->read_dword(m_reg[1]+4) - m_host_space->read_dword(m_reg[0]+4);
			int dx = m_host_space->read_dword(m_reg[1]+8) - m_host_space->read_dword(m_reg[0]+8);
			UINT16 angle;
				
			//m_raiden2cop->cop_status = 7;
			if(!dx) {
				//m_raiden2cop->cop_status |= 0x8000;
				angle = 0;
			} else {
				angle = atan(double(dy)/double(dx)) * 128.0 / M_PI;
				if(dx<0)
					angle += 0x80;
			}

			m_dy = dy;
			m_dx = dx;

			//if(m_cop_mcu_ram[offset] & 0x80)
			m_host_space->write_word(m_reg[0]+(0x34^2), angle & 0xff);

			break;
		}
		
		case 0x3bb0:
		{
			int dy = m_dy;
			int dx = m_dx;
			UINT16 dist;

			dx >>= 16;
			dy >>= 16;
			dist = sqrt((double)(dx*dx+dy*dy));

			m_host_space->write_word(m_reg[0]+(0x38), dist);

			break;
		}
			
		case 0x8100:
		{
			int raw_angle = (m_host_space->read_word(m_reg[0]+(0x34^2)) & 0xff);
			double angle = raw_angle * M_PI / 128;
			double amp = (65536 >> 5)*(m_host_space->read_word(m_reg[0]+(0x36^2)) & 0xff);
			int res;

		/* TODO: up direction, why? */
			if(raw_angle == 0xc0)
				amp*=2;

			res = int(amp*sin(angle)) << 1;//m_raiden2cop->cop_scale;

			m_host_space->write_dword(m_reg[0] + 0x10, res);

			break;
		}
		
		case 0x8900:
		{
			int raw_angle = (m_host_space->read_word(m_reg[0]+(0x34^2)) & 0xff);
			double angle = raw_angle * M_PI / 128;
			double amp = (65536 >> 5)*(m_host_space->read_word(m_reg[0]+(0x36^2)) & 0xff);
			int res;

			/* TODO: left direction, why? */
			if(raw_angle == 0x80)
				amp*=2;

			res = int(amp*cos(angle)) << 1;//m_raiden2cop->cop_scale;

			m_host_space->write_dword(m_reg[0] + 20, res);

			break;
		}

	
	}
	
}

static ADDRESS_MAP_START( seibucopbl_map, AS_0, 16, seibu_cop_bootleg_device )
	AM_RANGE(0x070, 0x07f) AM_RAM // DMA registers, resolved at PC=0xc0034
	AM_RANGE(0x0a0, 0x0af) AM_READWRITE(reg_hi_addr_r,reg_hi_addr_w)
	AM_RANGE(0x0c0, 0x0cf) AM_READWRITE(reg_lo_addr_r,reg_lo_addr_w)
	AM_RANGE(0x100, 0x105) AM_WRITE(cmd_trigger_w)
ADDRESS_MAP_END

seibu_cop_bootleg_device::seibu_cop_bootleg_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SEIBU_COP_BOOTLEG, "Seibu COP (Bootleg)", tag, owner, clock, "seibu_cop_boot", __FILE__),
      device_memory_interface(mconfig, *this),
 		m_space_config("regs", ENDIANNESS_LITTLE, 16, 9, 0, nullptr, *ADDRESS_MAP_NAME(seibucopbl_map))
{
}



const address_space_config *seibu_cop_bootleg_device::memory_space_config(address_spacenum spacenum) const
{
	return (spacenum == AS_0) ? &m_space_config : nullptr;
}

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
//	m_cop_mcu_ram = reinterpret_cast<UINT16 *>(machine().root_device().memshare("cop_mcu_ram")->ptr());

}

//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void seibu_cop_bootleg_device::device_reset()
{
	m_host_cpu = machine().device<cpu_device>("maincpu");
	m_host_space = &m_host_cpu->space(AS_PROGRAM);
}

inline UINT16 seibu_cop_bootleg_device::read_word(offs_t address)
{
	return space().read_word(address << 1);
}

//-------------------------------------------------
//  write_word - write a word at the given address
//-------------------------------------------------

inline void seibu_cop_bootleg_device::write_word(offs_t address, UINT16 data)
{
	space().write_word(address << 1, data);
}

READ16_MEMBER( seibu_cop_bootleg_device::copdxbl_0_r )
{
	return read_word(offset);
}

WRITE16_MEMBER( seibu_cop_bootleg_device::copdxbl_0_w )
{
	write_word(offset,data);
	
	switch((offset+0x400)/2)
	{
		default:
		{
			//logerror("%06x: COPX unhandled write data %04x at offset %04x\n", space.device().safe_pc(), data, offset*2+0x400);
			break;
		}

		#if 0
		case (0x500/2):
		case (0x502/2):
		case (0x504/2):
			switch(m_cop_mcu_ram[offset])
			{
				case 0x130e:
				case 0x138e:
				{
					int dy = m_host_space.read_dword(m_reg[1]+4) - m_host_space.read_dword(m_reg[0]+4);
					int dx = m_host_space.read_dword(m_reg[1]+8) - m_host_space.read_dword(m_reg[0]+8);

					m_raiden2cop->cop_status = 7;
					if(!dx) {
						m_raiden2cop->cop_status |= 0x8000;
						m_raiden2cop->cop_angle = 0;
					} else {
						m_raiden2cop->cop_angle = atan(double(dy)/double(dx)) * 128.0 / M_PI;
						if(dx<0)
							m_raiden2cop->cop_angle += 0x80;
					}

					m_dy = dy;
					m_dx = dx;

					if(m_cop_mcu_ram[offset] & 0x80)
						m_host_space.write_word(m_reg[0]+(0x34^2), m_raiden2cop->cop_angle);

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
						m_host_space.write_word(m_reg[0]+(0x38), m_raiden2cop->cop_dist);

					break;
				}
				default:
					printf("%04x\n",m_cop_mcu_ram[offset]);
					break;
			}
			break;
		#endif

	}
}
