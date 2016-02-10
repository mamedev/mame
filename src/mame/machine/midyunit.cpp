// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn, Zsolt Vasvari, Ernesto Corvi, Aaron Giles
// thanks-to:Kurt Mahan
/*************************************************************************

    Williams/Midway Y/Z-unit system

**************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/m6809/m6809.h"
#include "includes/midyunit.h"


/* constant definitions */
#define SOUND_NARC                  1
#define SOUND_CVSD_SMALL            2
#define SOUND_CVSD                  3
#define SOUND_ADPCM                 4
#define SOUND_YAWDIM                5



/*************************************
 *
 *  CMOS reads/writes
 *
 *************************************/

WRITE16_MEMBER(midyunit_state::midyunit_cmos_w)
{
	logerror("%08x:CMOS Write @ %05X\n", space.device().safe_pc(), offset);
	COMBINE_DATA(&m_cmos_ram[offset + m_cmos_page]);
}


READ16_MEMBER(midyunit_state::midyunit_cmos_r)
{
	return m_cmos_ram[offset + m_cmos_page];
}



/*************************************
 *
 *  CMOS enable and protection
 *
 *************************************/

WRITE16_MEMBER(midyunit_state::midyunit_cmos_enable_w)
{
	m_cmos_w_enable = (~data >> 9) & 1;

	logerror("%08x:Protection write = %04X\n", space.device().safe_pc(), data);

	/* only go down this path if we have a data structure */
	if (m_prot_data)
	{
		/* mask off the data */
		data &= 0x0f00;

		/* update the FIFO */
		m_prot_sequence[0] = m_prot_sequence[1];
		m_prot_sequence[1] = m_prot_sequence[2];
		m_prot_sequence[2] = data;

		/* special case: sequence entry 1234 means Strike Force, which is different */
		if (m_prot_data->reset_sequence[0] == 0x1234)
		{
			if (data == 0x500)
			{
				m_prot_result = space.read_word(TOBYTE(0x10a4390)) << 4;
				logerror("  desired result = %04X\n", m_prot_result);
			}
		}

		/* all other games use the same pattern */
		else
		{
			/* look for a reset */
			if (m_prot_sequence[0] == m_prot_data->reset_sequence[0] &&
				m_prot_sequence[1] == m_prot_data->reset_sequence[1] &&
				m_prot_sequence[2] == m_prot_data->reset_sequence[2])
			{
				logerror("Protection reset\n");
				m_prot_index = 0;
			}

			/* look for a clock */
			if ((m_prot_sequence[1] & 0x0800) != 0 && (m_prot_sequence[2] & 0x0800) == 0)
			{
				m_prot_result = m_prot_data->data_sequence[m_prot_index++];
				logerror("Protection clock (new data = %04X)\n", m_prot_result);
			}
		}
	}
}


READ16_MEMBER(midyunit_state::midyunit_protection_r)
{
	/* return the most recently clocked value */
	logerror("%08X:Protection read = %04X\n", space.device().safe_pc(), m_prot_result);
	return m_prot_result;
}



/*************************************
 *
 *  Generic input ports
 *
 *************************************/

IOPORT_ARRAY_MEMBER(midyunit_state::ports) { "IN0", "IN1", "IN2", "DSW", "UNK0", "UNK1" };

READ16_MEMBER(midyunit_state::midyunit_input_r)
{
	return m_ports[offset]->read();
}



/*************************************
 *
 *  Special Terminator 2 input ports
 *
 *************************************/

READ16_MEMBER(midyunit_state::term2_input_r)
{
	if (offset != 2)
		return m_ports[offset]->read();

	switch (m_term2_analog_select)
	{
		default:
		case 0:  return ioport("STICK0_X")->read();
		case 1:  return ioport("STICK0_Y")->read();
		case 2:  return ioport("STICK1_X")->read();
		case 3:  return ioport("STICK1_Y")->read();
	}
}

WRITE16_MEMBER(midyunit_state::term2_sound_w)
{
	/* Flash Lamp Output Data */
	if  ( ((data & 0x800) != 0x800) && ((data & 0x400) == 0x400 ) )
	{
	output().set_value("Left_Flash_1", data & 0x1);
	output().set_value("Left_Flash_2", (data & 0x2) >> 1);
	output().set_value("Left_Flash_3", (data & 0x4) >> 2);
	output().set_value("Left_Flash_4", (data & 0x8) >> 3);
	output().set_value("Right_Flash_1", (data & 0x10) >> 4);
	output().set_value("Right_Flash_2", (data & 0x20) >> 5);
	output().set_value("Right_Flash_3", (data & 0x40) >> 6);
	output().set_value("Right_Flash_4", (data & 0x80) >> 7);
	}

	/* Gun Output Data */
	if  ( ((data & 0x800) == 0x800) && ((data & 0x400) != 0x400 ) )
	{
	output().set_value("Left_Gun_Recoil", data & 0x1);
	output().set_value("Right_Gun_Recoil", (data & 0x2) >> 1);
	output().set_value("Left_Gun_Green_Led", (~data & 0x20) >> 5);
	output().set_value("Left_Gun_Red_Led", (~data & 0x10) >> 4);
	output().set_value("Right_Gun_Green_Led", (~data & 0x80) >> 7);
	output().set_value("Right_Gun_Red_Led", (~data & 0x40) >> 6);
	}

	if (offset == 0)
		m_term2_analog_select = (data >> 12) & 3;

	m_adpcm_sound->reset_write((~data & 0x100) >> 1);
	m_adpcm_sound->write(space, offset, data);
}



/*************************************
 *
 *  Special Terminator 2 hack
 *
 *************************************/

WRITE16_MEMBER(midyunit_state::term2_hack_w)
{
	if (offset == 1 && space.device().safe_pc() == 0xffce6520)
	{
		m_t2_hack_mem[offset] = 0;
		return;
	}
	COMBINE_DATA(&m_t2_hack_mem[offset]);
}

WRITE16_MEMBER(midyunit_state::term2la3_hack_w)
{
	if (offset == 0 && space.device().safe_pc() == 0xffce5230)
	{
		m_t2_hack_mem[offset] = 0;
		return;
	}
	COMBINE_DATA(&m_t2_hack_mem[offset]);
}

WRITE16_MEMBER(midyunit_state::term2la2_hack_w)
{
	if (offset == 0 && space.device().safe_pc() == 0xffce4b80)
	{
		m_t2_hack_mem[offset] = 0;
		return;
	}
	COMBINE_DATA(&m_t2_hack_mem[offset]);
}

WRITE16_MEMBER(midyunit_state::term2la1_hack_w)
{
	if (offset == 0 && space.device().safe_pc() == 0xffce33f0)
	{
		m_t2_hack_mem[offset] = 0;
		return;
	}
	COMBINE_DATA(&m_t2_hack_mem[offset]);
}



/*************************************
 *
 *  Generic driver init
 *
 *************************************/

WRITE8_MEMBER(midyunit_state::cvsd_protection_w)
{
	/* because the entire CVSD ROM is banked, we have to make sure that writes */
	/* go to the proper location (i.e., bank 0); currently bank 0 always lives */
	/* in the 0x10000-0x17fff space, so we just need to add 0x8000 to get the  */
	/* proper offset */
	m_cvsd_protection_base[offset] = data;
}


void midyunit_state::init_generic(int bpp, int sound, int prot_start, int prot_end)
{
	offs_t gfx_chunk = m_gfx_rom.bytes() / 4;
	UINT8 d1, d2, d3, d4, d5, d6;
	UINT8 *base;
	int i;

	/* load graphics ROMs */
	base = memregion("gfx1")->base();
	switch (bpp)
	{
		case 4:
			for (i = 0; i < m_gfx_rom.bytes(); i += 2)
			{
				d1 = ((base[0 * gfx_chunk + (i + 0) / 4]) >> (2 * ((i + 0) % 4))) & 3;
				d2 = ((base[1 * gfx_chunk + (i + 0) / 4]) >> (2 * ((i + 0) % 4))) & 3;
				d3 = ((base[0 * gfx_chunk + (i + 1) / 4]) >> (2 * ((i + 1) % 4))) & 3;
				d4 = ((base[1 * gfx_chunk + (i + 1) / 4]) >> (2 * ((i + 1) % 4))) & 3;

				m_gfx_rom[i + 0] = d1 | (d2 << 2);
				m_gfx_rom[i + 1] = d3 | (d4 << 2);
			}
			break;

		case 6:
			for (i = 0; i < m_gfx_rom.bytes(); i += 2)
			{
				d1 = ((base[0 * gfx_chunk + (i + 0) / 4]) >> (2 * ((i + 0) % 4))) & 3;
				d2 = ((base[1 * gfx_chunk + (i + 0) / 4]) >> (2 * ((i + 0) % 4))) & 3;
				d3 = ((base[2 * gfx_chunk + (i + 0) / 4]) >> (2 * ((i + 0) % 4))) & 3;
				d4 = ((base[0 * gfx_chunk + (i + 1) / 4]) >> (2 * ((i + 1) % 4))) & 3;
				d5 = ((base[1 * gfx_chunk + (i + 1) / 4]) >> (2 * ((i + 1) % 4))) & 3;
				d6 = ((base[2 * gfx_chunk + (i + 1) / 4]) >> (2 * ((i + 1) % 4))) & 3;

				m_gfx_rom[i + 0] = d1 | (d2 << 2) | (d3 << 4);
				m_gfx_rom[i + 1] = d4 | (d5 << 2) | (d6 << 4);
			}
			break;

		case 8:
			for (i = 0; i < m_gfx_rom.bytes(); i += 4)
			{
				m_gfx_rom[i + 0] = base[0 * gfx_chunk + i / 4];
				m_gfx_rom[i + 1] = base[1 * gfx_chunk + i / 4];
				m_gfx_rom[i + 2] = base[2 * gfx_chunk + i / 4];
				m_gfx_rom[i + 3] = base[3 * gfx_chunk + i / 4];
			}
			break;
	}

	/* load sound ROMs and set up sound handlers */
	m_chip_type = sound;
	switch (sound)
	{
		case SOUND_CVSD_SMALL:
			machine().device("cvsd:cpu")->memory().space(AS_PROGRAM).install_write_handler(prot_start, prot_end, write8_delegate(FUNC(midyunit_state::cvsd_protection_w), this));
			m_cvsd_protection_base = memregion("cvsd:cpu")->base() + 0x10000 + (prot_start - 0x8000);
			break;

		case SOUND_CVSD:
			machine().device("cvsd:cpu")->memory().space(AS_PROGRAM).install_ram(prot_start, prot_end);
			break;

		case SOUND_ADPCM:
			machine().device("adpcm:cpu")->memory().space(AS_PROGRAM).install_ram(prot_start, prot_end);
			break;

		case SOUND_NARC:
			machine().device("narcsnd:cpu0")->memory().space(AS_PROGRAM).install_ram(prot_start, prot_end);
			break;

		case SOUND_YAWDIM:
			break;
	}
}



/*************************************
 *
 *  Z-unit init
 *
 *  music: 6809 driving YM2151, DAC
 *  effects: 6809 driving CVSD, DAC
 *
 *************************************/

DRIVER_INIT_MEMBER(midyunit_state,narc)
{
	/* common init */
	init_generic(8, SOUND_NARC, 0xcdff, 0xce29);
}



/*************************************
 *
 *  Y-unit init (CVSD)
 *
 *  music: 6809 driving YM2151, DAC, and CVSD
 *
 *************************************/


/********************** Trog **************************/

DRIVER_INIT_MEMBER(midyunit_state,trog)
{
	/* protection */
	static const struct protection_data trog_protection_data =
	{
		{ 0x0f00, 0x0f00, 0x0f00 },
		{ 0x3000, 0x1000, 0x2000, 0x0000,
			0x2000, 0x3000,
			0x3000, 0x1000,
			0x0000, 0x0000, 0x2000, 0x3000, 0x1000, 0x1000, 0x2000 }
	};
	m_prot_data = &trog_protection_data;

	/* common init */
	init_generic(4, SOUND_CVSD_SMALL, 0x9eaf, 0x9ed9);
}


/********************** Smash TV **********************/

DRIVER_INIT_MEMBER(midyunit_state,smashtv)
{
	/* common init */
	init_generic(6, SOUND_CVSD_SMALL, 0x9cf6, 0x9d21);
}


/********************** High Impact Football **********************/

DRIVER_INIT_MEMBER(midyunit_state,hiimpact)
{
	/* protection */
	static const struct protection_data hiimpact_protection_data =
	{
		{ 0x0b00, 0x0b00, 0x0b00 },
		{ 0x2000, 0x4000, 0x4000, 0x0000, 0x6000, 0x6000, 0x2000, 0x4000,
			0x2000, 0x4000, 0x2000, 0x0000, 0x4000, 0x6000, 0x2000 }
	};
	m_prot_data = &hiimpact_protection_data;

	/* common init */
	init_generic(6, SOUND_CVSD, 0x9b79, 0x9ba3);
}


/********************** Super High Impact Football **********************/

DRIVER_INIT_MEMBER(midyunit_state,shimpact)
{
	/* protection */
	static const struct protection_data shimpact_protection_data =
	{
		{ 0x0f00, 0x0e00, 0x0d00 },
		{ 0x0000, 0x4000, 0x2000, 0x5000, 0x2000, 0x1000, 0x4000, 0x6000,
			0x3000, 0x0000, 0x2000, 0x5000, 0x5000, 0x5000, 0x2000 }
	};
	m_prot_data = &shimpact_protection_data;

	/* common init */
	init_generic(6, SOUND_CVSD, 0x9c06, 0x9c15);
}


/********************** Strike Force **********************/

DRIVER_INIT_MEMBER(midyunit_state,strkforc)
{
	/* protection */
	static const struct protection_data strkforc_protection_data =
	{
		{ 0x1234 }
	};
	m_prot_data = &strkforc_protection_data;

	/* common init */
	init_generic(4, SOUND_CVSD_SMALL, 0x9f7d, 0x9fa7);
}



/*************************************
 *
 *  Y-unit init (ADPCM)
 *
 *  music: 6809 driving YM2151, DAC, and OKIM6295
 *
 *************************************/


/********************** Mortal Kombat **********************/

DRIVER_INIT_MEMBER(midyunit_state,mkyunit)
{
	/* protection */
	static const struct protection_data mk_protection_data =
	{
		{ 0x0d00, 0x0c00, 0x0900 },
		{ 0x4600, 0xf600, 0xa600, 0x0600, 0x2600, 0x9600, 0xc600, 0xe600,
			0x8600, 0x7600, 0x8600, 0x8600, 0x9600, 0xd600, 0x6600, 0xb600,
			0xd600, 0xe600, 0xf600, 0x7600, 0xb600, 0xa600, 0x3600 }
	};
	m_prot_data = &mk_protection_data;

	/* common init */
	init_generic(6, SOUND_ADPCM, 0xfb9c, 0xfbc6);
}

DRIVER_INIT_MEMBER(midyunit_state,mkyawdim)
{
	/* common init */
	init_generic(6, SOUND_YAWDIM, 0, 0);
}


/*************************************
 *
 *  MK Turbo Ninja protection
 *
 *************************************/

READ16_MEMBER(midyunit_state::mkturbo_prot_r)
{
	/* the security GAL overlays a counter of some sort at 0xfffff400 in ROM space.
	 * A startup protection check expects to read back two different values in succession */
	return machine().rand();
}

DRIVER_INIT_MEMBER(midyunit_state,mkyturbo)
{
	/* protection */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xfffff400, 0xfffff40f, read16_delegate(FUNC(midyunit_state::mkturbo_prot_r),this));

	DRIVER_INIT_CALL(mkyunit);
}

/********************** Terminator 2 **********************/

void midyunit_state::term2_init_common(write16_delegate hack_w)
{
	/* protection */
	static const struct protection_data term2_protection_data =
	{
		{ 0x0f00, 0x0f00, 0x0f00 },
		{ 0x4000, 0xf000, 0xa000 }
	};
	m_prot_data = &term2_protection_data;

	/* common init */
	init_generic(6, SOUND_ADPCM, 0xfa8d, 0xfa9c);

	/* special inputs */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x01c00000, 0x01c0005f, read16_delegate(FUNC(midyunit_state::term2_input_r), this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x01e00000, 0x01e0001f, write16_delegate(FUNC(midyunit_state::term2_sound_w), this));

	/* HACK: this prevents the freeze on the movies */
	/* until we figure what's causing it, this is better than nothing */
	m_t2_hack_mem = m_maincpu->space(AS_PROGRAM).install_write_handler(0x010aa0e0, 0x010aa0ff, hack_w);
}

DRIVER_INIT_MEMBER(midyunit_state,term2)    { term2_init_common(write16_delegate(FUNC(midyunit_state::term2_hack_w),this)); }
DRIVER_INIT_MEMBER(midyunit_state,term2la3) { term2_init_common(write16_delegate(FUNC(midyunit_state::term2la3_hack_w),this)); }
DRIVER_INIT_MEMBER(midyunit_state,term2la2) { term2_init_common(write16_delegate(FUNC(midyunit_state::term2la2_hack_w),this)); }
DRIVER_INIT_MEMBER(midyunit_state,term2la1) { term2_init_common(write16_delegate(FUNC(midyunit_state::term2la1_hack_w),this)); }



/********************** Total Carnage **********************/

DRIVER_INIT_MEMBER(midyunit_state,totcarn)
{
	/* protection */
	static const struct protection_data totcarn_protection_data =
	{
		{ 0x0f00, 0x0f00, 0x0f00 },
		{ 0x4a00, 0x6a00, 0xda00, 0x6a00, 0x9a00, 0x4a00, 0x2a00, 0x9a00, 0x1a00,
			0x8a00, 0xaa00 }
	};
	m_prot_data = &totcarn_protection_data;

	/* common init */
	init_generic(6, SOUND_ADPCM, 0xfc04, 0xfc2e);
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

MACHINE_RESET_MEMBER(midyunit_state,midyunit)
{
	/* reset sound */
	switch (m_chip_type)
	{
		case SOUND_NARC:
			m_narc_sound->reset_write(1);
			m_narc_sound->reset_write(0);
			break;

		case SOUND_CVSD:
		case SOUND_CVSD_SMALL:
			m_cvsd_sound->reset_write(1);
			m_cvsd_sound->reset_write(0);
			break;

		case SOUND_ADPCM:
			m_adpcm_sound->reset_write(1);
			m_adpcm_sound->reset_write(0);
			break;

		case SOUND_YAWDIM:
			break;
	}
}



/*************************************
 *
 *  Sound write handlers
 *
 *************************************/

WRITE16_MEMBER(midyunit_state::midyunit_sound_w)
{
	/* check for out-of-bounds accesses */
	if (offset)
	{
		logerror("%08X:Unexpected write to sound (hi) = %04X\n", space.device().safe_pc(), data);
		return;
	}

	/* call through based on the sound type */
	if (ACCESSING_BITS_0_7 && ACCESSING_BITS_8_15)
		switch (m_chip_type)
		{
			case SOUND_NARC:
				m_narc_sound->write(space, offset, data);
				break;

			case SOUND_CVSD_SMALL:
			case SOUND_CVSD:
				m_cvsd_sound->reset_write((~data & 0x100) >> 8);
				m_cvsd_sound->write(space, offset, (data & 0xff) | ((data & 0x200) >> 1));
				break;

			case SOUND_ADPCM:
				m_adpcm_sound->reset_write((~data & 0x100) >> 8);
				m_adpcm_sound->write(space, offset, data);
				break;

			case SOUND_YAWDIM:
				soundlatch_byte_w(space, 0, data);
				m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
				break;
		}
}
