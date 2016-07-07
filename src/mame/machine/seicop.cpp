// license:LGPL-2.1+
// copyright-holders:Olivier Galibert, Angelo Salese, David Haywood, Tomasz Slanina
/********************************************************************************************

  COPDX bootleg simulation
    - Seibu Cup Soccer (bootleg)

  Notice that only the bare minimum is supported, which is what the bootleg device actually 
  do.
  Apparently it's an Actel PL84c FPGA programmed to be a Seibu COP clone. 
  The internal operations are actually loaded via the ROMs, we use the original algorithm 
  for the trigger until we find the proper hookup.

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
	UINT8 offs;

	offs = (offset & 3) * 4;
	
	switch(data)
	{
		default:
			printf("trigger write %04x\n",data);
			break;
		case 0x0000:
			break;
		
		case 0xf105:
			break;

		case 0xdde5:
		{
			int div;
			INT16 dir_offset;

			div = m_host_space->read_word(m_reg[4] + offs);
			dir_offset = m_host_space->read_word(m_reg[4] + offs + 8);
			if (div == 0) { div = 1; }

			m_host_space->write_word((m_reg[6] + offs + 4), ((m_host_space->read_word(m_reg[5] + offs + 4) + dir_offset) / div));
			break;
		}
		case 0x0205:
		{
			int ppos = m_host_space->read_dword(m_reg[0] + 4 + offs);
			int npos = ppos + m_host_space->read_dword(m_reg[0] + 0x10 + offs);
			int delta = (npos >> 16) - (ppos >> 16);

			m_host_space->write_dword(m_reg[0] + 4 + offs, npos);
			m_host_space->write_word(m_reg[0] + 0x1c + offs, m_host_space->read_word(m_reg[0] + 0x1c + offs) + delta);
			break;
		}

		case 0x130e:
		case 0x138e:
		case 0xe30e:
		case 0xe18e:
		{
			int target_reg = data & 0x200 ? 2 : 1;
			int dy = (m_host_space->read_dword(m_reg[target_reg]+4) >> 16) - (m_host_space->read_dword(m_reg[0]+4) >> 16);
			int dx = (m_host_space->read_dword(m_reg[target_reg]+8) >> 16) - (m_host_space->read_dword(m_reg[0]+8) >> 16);
			
			//m_status = 7;
			if(!dy) {
				m_status = 0x8000;
				m_angle = 0;
			} else {
				m_status = 0;
				m_angle =  atan(double(dx)/double(dy)) * 128.0 / M_PI;

				if(dx<0)
				{
					m_angle += 0x80;
				}
			}

			m_dy = dy;
			m_dx = dx;

			if(data & 0x80)
				m_host_space->write_byte(m_reg[0]+(0x37), m_angle & 0xff);

			break;
		}
		
		case 0x3bb0:
		{
			int dy = m_dy;
			int dx = m_dx;

			dx >>= 16;
			dy >>= 16;
			m_dist = sqrt((double)(dx*dx+dy*dy));
	
			// TODO: is this right?
			m_host_space->write_word(m_reg[0]+(0x38), m_dist);

			break;
		}
		
		// TODO: wrong
		case 0x42c2:
		{
			int div = m_host_space->read_word(m_reg[0] + (0x34));

			if (!div)
			{
				m_status |= 0x8000;
				m_host_space->write_dword(m_reg[0] + (0x38), 0);
				break;
			}

			m_host_space->write_dword(m_reg[0] + (0x38), (m_dist << (5 - 1)) / div);
			break;
		}
		
		/*
	        00000-0ffff:
	        amp = x/256
	        ang = x & 255
	        s = sin(ang*2*pi/256)
	        val = trunc(s*amp)
	        if(s<0)
	        val--
	        if(s == 192)
	        val = -2*amp
	    */
		case 0x8100:
		{
			UINT16 sin_offs; //= m_host_space->read_dword(m_reg[0]+(0x34));
			sin_offs = m_host_space->read_byte(m_reg[0]+(0x35));
			sin_offs |= m_host_space->read_byte(m_reg[0]+(0x37)) << 8;
			int raw_angle = (m_host_space->read_word(m_reg[0]+(0x34^2)) & 0xff);
			double angle = raw_angle * M_PI / 128;
			double amp = (65536 >> 5)*(m_host_space->read_word(m_reg[0]+(0x36^2)) & 0xff);
			int res;
			

			/* TODO: up direction, why? */
			if(raw_angle == 0xc0)
				amp*=2;

			res = int(amp*sin(angle)) << 1;//m_raiden2cop->cop_scale;

			printf("%04x %02x %08x\n",sin_offs,raw_angle,res);

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

		case 0xd104:
		{
			//m_host_space->write_dword(m_reg[0] + 4 + offs, 0x04000000);
			break;
		}
	}
	
}

READ16_MEMBER(seibu_cop_bootleg_device::status_r)
{
	return m_status;
}

READ16_MEMBER(seibu_cop_bootleg_device::dist_r)
{
	return m_dist;
}

READ16_MEMBER(seibu_cop_bootleg_device::angle_r)
{
	return m_angle;
}

READ16_MEMBER(seibu_cop_bootleg_device::d104_move_r)
{
	return m_d104_move_offset >> offset*16;
}

WRITE16_MEMBER(seibu_cop_bootleg_device::d104_move_w)
{
	if(offset == 1)
		m_d104_move_offset = (m_d104_move_offset & 0xffff0000) | (data & 0xffff); 
	else
		m_d104_move_offset = (m_d104_move_offset & 0xffff) | (data << 16); 
}

// anything that is read thru ROM range 0xc**** is replacement code, therefore on this HW they are latches.
static ADDRESS_MAP_START( seibucopbl_map, AS_0, 16, seibu_cop_bootleg_device )
	AM_RANGE(0x01e, 0x01f) AM_RAM // angle step, PC=0xc0186
	AM_RANGE(0x046, 0x049) AM_READWRITE(d104_move_r,d104_move_w)
	AM_RANGE(0x070, 0x07f) AM_RAM // DMA registers, PC=0xc0034
	AM_RANGE(0x0a0, 0x0af) AM_READWRITE(reg_hi_addr_r,reg_hi_addr_w)
	AM_RANGE(0x0c0, 0x0cf) AM_READWRITE(reg_lo_addr_r,reg_lo_addr_w)
	AM_RANGE(0x100, 0x105) AM_WRITE(cmd_trigger_w)
	AM_RANGE(0x1b0, 0x1b1) AM_READ(status_r)
	AM_RANGE(0x1b2, 0x1b3) AM_READ(dist_r)
	AM_RANGE(0x1b4, 0x1b5) AM_READ(angle_r)
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
}
