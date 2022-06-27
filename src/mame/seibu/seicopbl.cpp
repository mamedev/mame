// license:LGPL-2.1+
// copyright-holders:Olivier Galibert, Angelo Salese, David Haywood, Tomasz Slanina
/********************************************************************************************

  COPDX bootleg simulation
    - Seibu Cup Soccer (bootleg)

  Notice that only the bare minimum is supported, which is what the bootleg device actually
  provides. Unlike the original device and many other Seibu customs, it has no DMA.
  Apparently it's an Actel PL84c FPGA programmed to be a Seibu COP clone.
  The internal operations are actually loaded via the ROMs, we use the original algorithm
  for the trigger until we find the proper hookup.

********************************************************************************************/

#include "emu.h"
#include "seicopbl.h"


DEFINE_DEVICE_TYPE(SEIBU_COP_BOOTLEG, seibu_cop_bootleg_device, "seibu_cop_boot", "Seibu COP (bootleg)")

uint16_t seibu_cop_bootleg_device::reg_lo_addr_r(offs_t offset)
{
	return m_reg[offset] & 0xffff;
}

uint16_t seibu_cop_bootleg_device::reg_hi_addr_r(offs_t offset)
{
	return m_reg[offset] >> 16;
}

void seibu_cop_bootleg_device::reg_lo_addr_w(offs_t offset, uint16_t data)
{
	m_reg[offset] = (m_reg[offset] & 0xffff0000) | (data & 0xffff);
}

void seibu_cop_bootleg_device::reg_hi_addr_w(offs_t offset, uint16_t data)
{
	m_reg[offset] = (m_reg[offset] & 0xffff) | (data << 16);
}

void seibu_cop_bootleg_device::cmd_trigger_w(offs_t offset, uint16_t data)
{
	//printf("%04x %08x %08x\n",data,m_reg[0],m_reg[1]);
	uint8_t offs;

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
			int16_t dir_offset;

			div = m_host_space->read_word(m_reg[4] + offs);
			dir_offset = m_host_space->read_word(m_reg[4] + offs + 8);
			if (div == 0) { div = 1; }

			m_host_space->write_word((m_reg[6] + offs + 4), ((m_host_space->read_word(m_reg[5] + offs + 4) + dir_offset) / div));
			break;
		}

		/*
		    read32 10(r0)
		    add32 4(r0)
		    addmem32 4(r0)
		    addmem16 1c(r0)
		    write16h 1c(r0)
		*/
		// 0x0204 variant used from time to time (goal post collision)
		case 0x0204:
		case 0x0205:
		{
			int ppos = m_host_space->read_dword(m_reg[0] + 4 + offs);
			int npos = ppos + m_host_space->read_dword(m_reg[0] + 0x10 + offs);
			int delta = (npos >> 16) - (ppos >> 16);

			m_host_space->write_dword(m_reg[0] + 4 + offs, npos);
			m_host_space->write_word(m_reg[0] + 0x1c + offs, m_host_space->read_word(m_reg[0] + 0x1c + offs) + delta);
			break;
		}

		// jumping is done with this
		case 0x0905:
		{
			//printf("%08x %08x\n",m_reg[0],offs);

			int val = m_host_space->read_dword(m_reg[0] + 16 + offs);
			int delta = m_host_space->read_dword(m_reg[0] + 0x28 + offs);

			//printf("%08x + %08x = ",val,delta);
			val += delta;
			//printf("%08x\n",val);

			m_host_space->write_dword(m_reg[0] + 16 + offs, val);

			break;
		}

		/*
		    0x138e
		    write16h 8(r0)
		    sub32 8(r1)
		    ? 4(r0)
		    sub32 4(r1)
		    ? 36(r0)
		    addmem16 34(r0)
		    addmem16 34(r0)
		    sub32 34(r0)
		    0xe38e
		    write16h 8(r0)
		    sub32 8(r2)
		    ? 4(r0)
		    sub32 4(r2)
		    ? 36(r0)
		    addmem16 34(r0)
		    addmem16 34(r0)
		    sub32 34(r0)

		*/
		// normal tackle
		case 0x118e:
		case 0x130e:
		case 0x138e:
		// tackling ball hit?
		case 0x330e:
		case 0xe30e:
		case 0xe18e:
		{
			int target_reg = ((data & 0xf000) == 0xe000) ? 2 : 1;
			int sy = (m_host_space->read_dword(m_reg[0]+4) >> 16);
			int sx = (m_host_space->read_dword(m_reg[0]+8) >> 16);
			int dy = (m_host_space->read_dword(m_reg[target_reg]+4) >> 16);
			int dx = (m_host_space->read_dword(m_reg[target_reg]+8) >> 16);

			#if 0
			if(data == 0xe30e)
			{
				if(dx != 0 && m_reg[0] == 0x111f30)
					printf("%08x %08x | %08x %08x\n",sx,sy,dx,dy);
			}
			#endif
			dy -= sy;
			dx -= sx;

			#if 0
			if(data == 0xe30e)
			{
				if(dx != 0 && m_reg[0] == 0x111f30)
					printf("%08x %08x\n",dx,dy);
			}
			#endif

			//m_status = 7;
			if(!dx)
			{
				m_status = 0x8000;
				m_angle = 0;
			}
			else
			{
				m_status = 0;

				m_angle =  atan(double(dy)/double(dx)) * 128.0 / M_PI;

				//printf("%f\n",atan(double(dy)/double(dx)));

				if(dx<0)
				{
					m_angle += 0x80;
				}
			}

			m_dy = dy;
			m_dx = dx;

			// TODO: for some reason with 0x118e tackling go in inverted direction with this on
			// TODO: even worse, calculating 0xe18e makes the game pretty slow
			if(data == 0x118e || data == 0xe18e)
			{
				return;
			}

			if(data & 0x80)
				m_host_space->write_byte(m_reg[0]+(0x37), m_angle & 0xff);

			break;
		}

		case 0x3bb0:
		{
			int dy = m_dy;
			int dx = m_dx;

			m_dist = sqrt((double)(dx*dx+dy*dy));

			// TODO: is this right?
			m_host_space->write_word(m_reg[0]+(0x38), m_dist);

			break;
		}

		case 0x42c2:
		{
			int div = m_host_space->read_word(m_reg[0] + (0x34));

			if (!div)
			{
				m_status |= 0x8000;
				m_host_space->write_word(m_reg[0] + (0x38), 0);
				break;
			}
			// TODO: scaling is wrong
			m_host_space->write_word(m_reg[0] + (0x38), (m_dist << (5 - m_scale)) / div);

			//m_host_space->write_dword(m_reg[0] + (0x38), m_dist / (div << 10));
			break;
		}

		// shoot/pass is done with this
		/*
		    0x5105
		    sub32 (r0)
		    write16h 8(r0)
		    addmem32 4(r0)
		    outputs to 0x046/0x047 (d104_move_offset ?)
		*/
		case 0x5105:
		{
			int val = m_host_space->read_dword(m_reg[0]);
			val += m_host_space->read_word(m_reg[0] + 8);
			m_host_space->write_dword(m_reg[0] + 4,val);
			break;
		}
		/*
		    0x5905
		    write16h 10(r2)
		    sub32 8(r0)
		    addmem32 4(r1)
		*/
		case 0x5905:
		{
			int val = m_host_space->read_word(m_reg[2] + 0x10 + offs);
			val -= m_host_space->read_dword(m_reg[0] + 8 + offs);
			m_host_space->write_dword(m_reg[1] + 4 + offs,val);
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
			[[maybe_unused]] uint16_t sin_offs; //= m_host_space->read_dword(m_reg[0]+(0x34));
			sin_offs = m_host_space->read_byte(m_reg[0]+(0x35));
			sin_offs |= m_host_space->read_byte(m_reg[0]+(0x37)) << 8;
			int raw_angle = (m_host_space->read_word(m_reg[0]+(0x34^2)) & 0xff);
			double angle = raw_angle * M_PI / 128;
			double amp = (65536 >> 5)*(m_host_space->read_word(m_reg[0]+(0x36^2)) & 0xff);
			int res;


			/* TODO: up direction, why? */
			if(raw_angle == 0xc0)
				amp*=2;

			res = int(amp*sin(angle)) << m_scale;

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

			res = int(amp*cos(angle)) << m_scale;

			m_host_space->write_dword(m_reg[0] + 20, res);

			break;
		}

		/*
		    sub32 4(r2)
		    write16h (r3)
		    addmem32 4(r1)
		    outputs to 0x046/0x047 (d104_move_offset ?)
		*/
		case 0xd104:
		{
			int val = m_host_space->read_dword(m_reg[2]);
			val += m_host_space->read_word(m_reg[3] + 8);
			m_host_space->write_dword(m_reg[1] + 4,val);
			break;
		}
	}

}

uint16_t seibu_cop_bootleg_device::status_r()
{
	return m_status;
}

uint16_t seibu_cop_bootleg_device::dist_r()
{
	return m_dist;
}

uint16_t seibu_cop_bootleg_device::angle_r()
{
	return m_angle;
}

uint16_t seibu_cop_bootleg_device::d104_move_r(offs_t offset)
{
	return m_d104_move_offset >> offset*16;
}

void seibu_cop_bootleg_device::d104_move_w(offs_t offset, uint16_t data)
{
	if(offset == 1)
		m_d104_move_offset = (m_d104_move_offset & 0xffff0000) | (data & 0xffff);
	else
		m_d104_move_offset = (m_d104_move_offset & 0xffff) | (data << 16);
}

uint16_t seibu_cop_bootleg_device::prng_max_r()
{
	return m_prng_max;
}

void seibu_cop_bootleg_device::prng_max_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_prng_max);
}

uint16_t seibu_cop_bootleg_device::prng_r()
{
	return m_host_cpu->total_cycles() % (m_prng_max + 1);
}

uint16_t seibu_cop_bootleg_device::scale_r()
{
	return m_scale;
}

void seibu_cop_bootleg_device::scale_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scale);
	m_scale &= 3;
}


// anything that is read thru ROM range 0xc**** is replacement code, therefore on this HW they are latches.
void seibu_cop_bootleg_device::seibucopbl_map(address_map &map)
{
	map(0x01e, 0x01f).ram(); // angle step, PC=0xc0186
	map(0x028, 0x02b).ram(); // DMA fill latches
	map(0x02c, 0x02d).rw(FUNC(seibu_cop_bootleg_device::prng_max_r), FUNC(seibu_cop_bootleg_device::prng_max_w));
	map(0x040, 0x043).ram(); // n/a
	map(0x044, 0x045).rw(FUNC(seibu_cop_bootleg_device::scale_r), FUNC(seibu_cop_bootleg_device::scale_w));
	map(0x046, 0x049).rw(FUNC(seibu_cop_bootleg_device::d104_move_r), FUNC(seibu_cop_bootleg_device::d104_move_w));
	map(0x04a, 0x04f).ram(); // n/a
	map(0x050, 0x05f).ram(); // n/a
	map(0x070, 0x07f).ram(); // DMA registers, PC=0xc0034

	map(0x0a0, 0x0af).rw(FUNC(seibu_cop_bootleg_device::reg_hi_addr_r), FUNC(seibu_cop_bootleg_device::reg_hi_addr_w));
	map(0x0b0, 0x0b3).ram(); // unknown, not in original COP
	map(0x0c0, 0x0cf).rw(FUNC(seibu_cop_bootleg_device::reg_lo_addr_r), FUNC(seibu_cop_bootleg_device::reg_lo_addr_w));

	map(0x100, 0x105).w(FUNC(seibu_cop_bootleg_device::cmd_trigger_w));
	map(0x1a0, 0x1a7).r(FUNC(seibu_cop_bootleg_device::prng_r));
	map(0x1b0, 0x1b1).r(FUNC(seibu_cop_bootleg_device::status_r));
	map(0x1b2, 0x1b3).r(FUNC(seibu_cop_bootleg_device::dist_r));
	map(0x1b4, 0x1b5).r(FUNC(seibu_cop_bootleg_device::angle_r));
}

seibu_cop_bootleg_device::seibu_cop_bootleg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SEIBU_COP_BOOTLEG, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_host_cpu(*this, finder_base::DUMMY_TAG),
	m_space_config("regs", ENDIANNESS_BIG, 16, 9, 0, address_map_constructor(FUNC(seibu_cop_bootleg_device::seibucopbl_map), this))
{
}



device_memory_interface::space_config_vector seibu_cop_bootleg_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void seibu_cop_bootleg_device::device_start()
{
//  m_cop_mcu_ram = reinterpret_cast<uint16_t *>(machine().root_device().memshare("cop_mcu_ram")->ptr());

}

//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void seibu_cop_bootleg_device::device_reset()
{
	m_host_space = &m_host_cpu->space(AS_PROGRAM);
}

//-------------------------------------------------
//  read_word - read a word at the given address
//-------------------------------------------------

inline uint16_t seibu_cop_bootleg_device::read_word(offs_t address)
{
	return space().read_word(address << 1);
}

//-------------------------------------------------
//  write_word - write a word at the given address
//-------------------------------------------------

inline void seibu_cop_bootleg_device::write_word(offs_t address, uint16_t data)
{
	space().write_word(address << 1, data);
}

uint16_t seibu_cop_bootleg_device::read(offs_t offset)
{
	return read_word(offset);
}

void seibu_cop_bootleg_device::write(offs_t offset, uint16_t data)
{
	write_word(offset,data);
}
