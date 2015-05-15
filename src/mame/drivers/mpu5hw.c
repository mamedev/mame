// license:BSD-3-Clause
// copyright-holders:David Haywood
/* MPU5 hardware emulation */

/* This file contains the hardware emulation, the mpu5.c contains the set listings */

/*
    Many of the games in here also seem to exist on other hardware.

    MPU5

    Skeleton Driver

     -- there are a wide range of titles running on this hardware

     -- the driver does nothing, and currently only serves to act as a placeholder to document what existed on this hardware

     -- the main CPU is a 68340, which is a 32-bit 680xx variant with modified opcodes etc. (CPU32 core)

     -- Much of the communication is done via a 68681 DUART.

     -- The ASIC acts as the main I/O control, including an interface to a Z89321 DSP and a 16bit DAC.

     -- Help wanted, the MFME sources (which are based on MAME anyway) should be of some help here, if somebody
        in the FM emu community wants to adopt this driver they're welcome to it.

     -- As a result of games being on multiple systems, and some of the original sets being a mess there could be one or two
        out of position here (eg MPU4 video instead of MPU5) or with missing roms if there was extra hardware (nothing has been
        removed from the rom loading comments, so if there were extra roms present they're still commented)

        Some duplicate roms have been commented out for now, please don't remove these lines until the sets are properly sorted.

        Some games weren't even in the right zips, Eg the Red Hot Fever (MPU4) cotnained a mislabled MPU5 'Raise The Roof' set
        with extra roms, probably actually from the MPU4 Red Hot Fever.  The game names are usually stored somewhat as plain
        ASCII so spotting such problems is easy enough.

        In general things have been added here if the rom structure and initial code looks like the MPU5 boot code



    15/07/11 - rom loading for most games added, still some missing tho and clones still need sorting out properly.
*/


#include "emu.h"

#include "machine/sec.h"
#include "mpu5.lh"

#include "machine/68340.h"

class mpu5_state : public driver_device
{
public:
	mpu5_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }
	UINT32* m_cpuregion;
	UINT32* m_mainram;
	SEC sec;

	UINT8 m_led_strobe_temp;
	UINT8 m_led_strobe;
	UINT8 m_pic_clk;
	bool  m_pic_transfer_in_progress;
	UINT8 m_pic_bit1;
	UINT8 m_pic_data;
	UINT8 m_pic_clocked_bits;
	UINT8 m_pic_stored_input;
	UINT8 m_pic_output_bit;
	UINT8 m_input_strobe;

	DECLARE_READ32_MEMBER(mpu5_mem_r);
	DECLARE_WRITE32_MEMBER(mpu5_mem_w);

	DECLARE_READ32_MEMBER(asic_r32);
	DECLARE_READ8_MEMBER(asic_r8);
	DECLARE_WRITE32_MEMBER(asic_w32);
	DECLARE_WRITE8_MEMBER(asic_w8);

	DECLARE_READ32_MEMBER(pic_r);
	DECLARE_WRITE32_MEMBER(pic_w);

protected:

	// devices
	required_device<m68340cpu_device> m_maincpu;
	virtual void machine_start();
};

READ8_MEMBER(mpu5_state::asic_r8)
{
	switch (offset)
	{
		case 0x01:
		{
			return 0x99;
		}

		case 0x02:
		{
			//send init and ready for now - need to work on full DSP
			return 0x85;
		}

		case 0x0b:
		{
			return 0;
		}
		default:
		{
			int pc = space.device().safe_pc();
			logerror("%08x maincpu read from ASIC - offset %01x\n", pc, offset);
			return 0;
		}
	}
}


READ32_MEMBER(mpu5_state::asic_r32)
{
	UINT32 retdata = 0;
	if (mem_mask&0xff000000) retdata |= asic_r8(space,(offset*4)+0) <<24;
	if (mem_mask&0x00ff0000) retdata |= asic_r8(space,(offset*4)+1) <<16;
	if (mem_mask&0x0000ff00) retdata |= asic_r8(space,(offset*4)+2) <<8;
	if (mem_mask&0x000000ff) retdata |= asic_r8(space,(offset*4)+3) <<0;
	return retdata;
}

READ32_MEMBER(mpu5_state::mpu5_mem_r)
{
	int pc = space.device().safe_pc();
	int addr = offset *4;
	int cs = m68340_get_cs(m_maincpu, addr);

	switch ( cs )
	{
		case 2:
		{
			switch (addr & 0xf0)
			{
				case 0xd0:
				{
					logerror("%08x PIC read\n", pc);
					break;
				}
				case 0xe0:
				{
					logerror("%08x DUART read\n", pc);
					break;
				}

				case 0xf0:
				{
					return asic_r32(space, offset&3,mem_mask);
				}

				default:
				logerror("%08x maincpu read access offset %08x mem_mask %08x cs %d\n", pc, offset*4, mem_mask, cs);
				break;
			}
		}
		break;

		case 3:
		case 4:
			offset &=0x3fff;
			return (m_mainram[offset]);

		case 1:if (offset < 0x100000) // make sure to log an error instead of crashing when reading beyond end of region
			return m_cpuregion[offset];


		default:
			logerror("%08x maincpu read access offset %08x mem_mask %08x cs %d\n", pc, offset*4, mem_mask, cs);

	}

	return 0x0000;
}

// Each board is fitted with an ASIC that does most of the heavy lifting, including sound playback.
WRITE8_MEMBER(mpu5_state::asic_w8)
{
	switch (offset)
	{
		case 0x03:
		{
			if (m_led_strobe_temp != data)
			{
				m_led_strobe_temp = data;

				switch (m_led_strobe_temp)
				{
					case 0x00:
					m_led_strobe = 0;
					break;
					case 0x01:
					m_led_strobe = 1;
					break;
					case 0x02:
					m_led_strobe = 2;
					break;
					case 0x04:
					m_led_strobe = 3;
					break;
					case 0x08:
					m_led_strobe = 4;
					break;
					case 0x10:
					m_led_strobe = 5;
					break;
					case 0x20:
					m_led_strobe = 6;
					break;
					case 0x40:
					m_led_strobe = 7;
					break;
					case 0x80:
					m_led_strobe = 8;
					break;
				}
			}
			break;
		}

		case 0x09:
		{
			//Assume SEC fitted for now
			sec.write_data_line(~data&0x01);
			sec.write_clock_line(~data&0x02);
			sec.write_cs_line(~data&0x04);
		}
		case 0x0b:
		{
			output_set_value("statuslamp1", ((data&0x10) != 0));

			output_set_value("statuslamp2", ((data&0x20) != 0));

			if (data & 0x40)
			{
//              m_dsp_pin =1;
			}
		}
		break;
		default:
		{
			int pc = space.device().safe_pc();
			logerror("%08x maincpu write to ASIC - offset %01x data %02x\n", pc, offset, data);
		}
	}
}


WRITE32_MEMBER(mpu5_state::asic_w32)
{
	if (mem_mask&0xff000000) asic_w8(space,(offset*4)+0, (data>>24)&0xff);
	if (mem_mask&0x00ff0000) asic_w8(space,(offset*4)+1, (data>>16)&0xff);
	if (mem_mask&0x0000ff00) asic_w8(space,(offset*4)+2, (data>>8) &0xff);
	if (mem_mask&0x000000ff) asic_w8(space,(offset*4)+3, (data>>0) &0xff);
}


READ32_MEMBER(mpu5_state::pic_r)
{
	int pc = space.device().safe_pc();
	logerror("%08x maincpu read from PIC - offset %01x\n", pc, offset);
	return m_pic_output_bit;
}

WRITE32_MEMBER(mpu5_state::pic_w)
{
	switch (offset)
	{
		case 0x04:
		{
			if (m_pic_clk)
			{
				m_pic_transfer_in_progress = true;
			}
			m_pic_bit1 = (data & 0x01);
			break;
		}

		case 0x06:
		case 0x07:
		{
			if ( (!data) && (m_pic_transfer_in_progress) && (m_pic_clk))
			{
				//clock in the stored bit (rudimentary protection here)
				m_pic_data = (m_pic_data << 1);
				m_pic_data |= m_pic_bit1;
				m_pic_stored_input <<= 1;
				m_pic_clocked_bits ++;

				if (m_pic_clocked_bits >=8)
				{
					m_pic_data =0;
					m_pic_clocked_bits =0;

					if (m_input_strobe <4)
					{
						m_input_strobe +=1;
					}

				}
			}
			else
			{
				m_pic_output_bit = BIT(m_pic_stored_input,7);
			}
			m_pic_transfer_in_progress = false;
			m_pic_clk = (data != 0);
			break;
		}
		default:
		{
			int pc = space.device().safe_pc();
			logerror("%08x maincpu write to PIC - offset %01x data %02x\n", pc, offset, data);
			break;
		}
	}

}

WRITE32_MEMBER(mpu5_state::mpu5_mem_w)
{
	int pc = space.device().safe_pc();
	int addr = offset *4;
	int cs = m68340_get_cs(m_maincpu, addr);

	switch ( cs )
	{
		case 2:
		{
			switch (addr & 0xf0)
			{
				case 0xd0:
				{
					pic_w(space, (addr& 0x0f),data,mem_mask);
					break;
				}
				case 0xe0:
				{
					logerror("%08x DUART write\n", pc);
					break;
				}

				case 0xf0:
				{
					asic_w32(space, offset&3,data,mem_mask);
					break;
				}

				default:
					logerror("%08x maincpu write access offset %08x data %08x mem_mask %08x cs %d\n", pc, offset*4, data, mem_mask, cs);
				break;
			}
		}
		break;

		case 3:
		case 4:
			offset &=0x3fff;
			COMBINE_DATA(&m_mainram[offset]);
			break;



		default:
			logerror("%08x maincpu write access offset %08x data %08x mem_mask %08x cs %d\n", pc, offset*4, data, mem_mask, cs);

	}

}

static ADDRESS_MAP_START( mpu5_map, AS_PROGRAM, 32, mpu5_state )
	AM_RANGE(0x00000000, 0xffffffff) AM_READWRITE(mpu5_mem_r, mpu5_mem_w)
ADDRESS_MAP_END

INPUT_PORTS_START(  mpu5 )
INPUT_PORTS_END


void mpu5_state::machine_start()
{
	m_cpuregion = (UINT32*)memregion( "maincpu" )->base();
	m_mainram = (UINT32*)auto_alloc_array_clear(machine(), UINT32, 0x10000);
	m_pic_output_bit =0;
}


MACHINE_CONFIG_START( mpu5, mpu5_state )
	MCFG_CPU_ADD("maincpu", M68340, 16000000)    // ?
	MCFG_CPU_PROGRAM_MAP(mpu5_map)

	MCFG_DEFAULT_LAYOUT(layout_mpu5)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	/* unknown sound */
MACHINE_CONFIG_END
