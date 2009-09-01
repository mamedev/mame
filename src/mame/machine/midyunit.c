/*************************************************************************

    Williams/Midway Y/Z-unit system

**************************************************************************/

#include "driver.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/m6809/m6809.h"
#include "audio/williams.h"
#include "midyunit.h"


/* constant definitions */
#define SOUND_NARC					1
#define SOUND_CVSD_SMALL			2
#define SOUND_CVSD					3
#define SOUND_ADPCM					4
#define SOUND_YAWDIM				5


/* protection data types */
struct protection_data
{
	UINT16	reset_sequence[3];
	UINT16	data_sequence[100];
};
static const struct protection_data *prot_data;
static UINT16 prot_result;
static UINT16 prot_sequence[3];
static UINT8 prot_index;


/* input-related variables */
static UINT8	term2_analog_select;

/* CMOS-related variables */
       UINT16 *midyunit_cmos_ram;
       UINT32 	midyunit_cmos_page;
static UINT8	cmos_w_enable;

/* sound-related variables */
static UINT8	chip_type;

/* hack-related variables */
static UINT16 *t2_hack_mem;



/*************************************
 *
 *  CMOS reads/writes
 *
 *************************************/

WRITE16_HANDLER( midyunit_cmos_w )
{
	logerror("%08x:CMOS Write @ %05X\n", cpu_get_pc(space->cpu), offset);
	COMBINE_DATA(&midyunit_cmos_ram[offset + midyunit_cmos_page]);
}


READ16_HANDLER( midyunit_cmos_r )
{
	return midyunit_cmos_ram[offset + midyunit_cmos_page];
}



/*************************************
 *
 *  CMOS enable and protection
 *
 *************************************/

WRITE16_HANDLER( midyunit_cmos_enable_w )
{
	cmos_w_enable = (~data >> 9) & 1;

	logerror("%08x:Protection write = %04X\n", cpu_get_pc(space->cpu), data);

	/* only go down this path if we have a data structure */
	if (prot_data)
	{
		/* mask off the data */
		data &= 0x0f00;

		/* update the FIFO */
		prot_sequence[0] = prot_sequence[1];
		prot_sequence[1] = prot_sequence[2];
		prot_sequence[2] = data;

		/* special case: sequence entry 1234 means Strike Force, which is different */
		if (prot_data->reset_sequence[0] == 0x1234)
		{
			if (data == 0x500)
			{
				prot_result = memory_read_word(space, TOBYTE(0x10a4390)) << 4;
				logerror("  desired result = %04X\n", prot_result);
			}
		}

		/* all other games use the same pattern */
		else
		{
			/* look for a reset */
			if (prot_sequence[0] == prot_data->reset_sequence[0] &&
				prot_sequence[1] == prot_data->reset_sequence[1] &&
				prot_sequence[2] == prot_data->reset_sequence[2])
			{
				logerror("Protection reset\n");
				prot_index = 0;
			}

			/* look for a clock */
			if ((prot_sequence[1] & 0x0800) != 0 && (prot_sequence[2] & 0x0800) == 0)
			{
				prot_result = prot_data->data_sequence[prot_index++];
				logerror("Protection clock (new data = %04X)\n", prot_result);
			}
		}
	}
}


READ16_HANDLER( midyunit_protection_r )
{
	/* return the most recently clocked value */
	logerror("%08X:Protection read = %04X\n", cpu_get_pc(space->cpu), prot_result);
	return prot_result;
}



/*************************************
 *
 *  Generic input ports
 *
 *************************************/

READ16_HANDLER( midyunit_input_r )
{
	static const char *const portnames[] = { "IN0", "IN1", "IN2", "DSW", "UNK0", "UNK1" };

	return input_port_read(space->machine, portnames[offset]);
}



/*************************************
 *
 *  Special Terminator 2 input ports
 *
 *************************************/

static READ16_HANDLER( term2_input_r )
{
	static const char *const portnames[] = { "IN0", "IN1", NULL, "DSW", "UNK0", "UNK1" };

	if (offset != 2)
		return input_port_read(space->machine, portnames[offset]);

	switch (term2_analog_select)
	{
		default:
		case 0:  return input_port_read(space->machine, "STICK0_X");
		case 1:  return input_port_read(space->machine, "STICK0_Y");
		case 2:  return input_port_read(space->machine, "STICK1_X");
		case 3:  return input_port_read(space->machine, "STICK1_Y");
	}
}

static WRITE16_HANDLER( term2_sound_w )
{
	if (offset == 0)
		term2_analog_select = (data >> 12) & 3;

	williams_adpcm_reset_w((~data & 0x100) >> 1);
	williams_adpcm_data_w(data);
}



/*************************************
 *
 *  Special Terminator 2 hack
 *
 *************************************/

static WRITE16_HANDLER( term2_hack_w )
{
    if (offset == 0 && cpu_get_pc(space->cpu) == 0xffce5230)
    {
        t2_hack_mem[offset] = 0;
        return;
    }
	COMBINE_DATA(&t2_hack_mem[offset]);
}

static WRITE16_HANDLER( term2la2_hack_w )
{
    if (offset == 0 && cpu_get_pc(space->cpu) == 0xffce4b80)
    {
        t2_hack_mem[offset] = 0;
        return;
    }
	COMBINE_DATA(&t2_hack_mem[offset]);
}

static WRITE16_HANDLER( term2la1_hack_w )
{
    if (offset == 0 && cpu_get_pc(space->cpu) == 0xffce33f0)
    {
        t2_hack_mem[offset] = 0;
        return;
    }
	COMBINE_DATA(&t2_hack_mem[offset]);
}



/*************************************
 *
 *  Generic driver init
 *
 *************************************/

static UINT8 *cvsd_protection_base;
static WRITE8_HANDLER( cvsd_protection_w )
{
	/* because the entire CVSD ROM is banked, we have to make sure that writes */
	/* go to the proper location (i.e., bank 0); currently bank 0 always lives */
	/* in the 0x10000-0x17fff space, so we just need to add 0x8000 to get the  */
	/* proper offset */
	cvsd_protection_base[offset] = data;
}


static void init_generic(running_machine *machine, int bpp, int sound, int prot_start, int prot_end)
{
	offs_t gfx_chunk = midyunit_gfx_rom_size / 4;
	UINT8 d1, d2, d3, d4, d5, d6;
	UINT8 *base;
	int i;

	/* load graphics ROMs */
	base = memory_region(machine, "gfx1");
	switch (bpp)
	{
		case 4:
			for (i = 0; i < midyunit_gfx_rom_size; i += 2)
			{
				d1 = ((base[0 * gfx_chunk + (i + 0) / 4]) >> (2 * ((i + 0) % 4))) & 3;
				d2 = ((base[1 * gfx_chunk + (i + 0) / 4]) >> (2 * ((i + 0) % 4))) & 3;
				d3 = ((base[0 * gfx_chunk + (i + 1) / 4]) >> (2 * ((i + 1) % 4))) & 3;
				d4 = ((base[1 * gfx_chunk + (i + 1) / 4]) >> (2 * ((i + 1) % 4))) & 3;

				midyunit_gfx_rom[i + 0] = d1 | (d2 << 2);
				midyunit_gfx_rom[i + 1] = d3 | (d4 << 2);
			}
			break;

		case 6:
			for (i = 0; i < midyunit_gfx_rom_size; i += 2)
			{
				d1 = ((base[0 * gfx_chunk + (i + 0) / 4]) >> (2 * ((i + 0) % 4))) & 3;
				d2 = ((base[1 * gfx_chunk + (i + 0) / 4]) >> (2 * ((i + 0) % 4))) & 3;
				d3 = ((base[2 * gfx_chunk + (i + 0) / 4]) >> (2 * ((i + 0) % 4))) & 3;
				d4 = ((base[0 * gfx_chunk + (i + 1) / 4]) >> (2 * ((i + 1) % 4))) & 3;
				d5 = ((base[1 * gfx_chunk + (i + 1) / 4]) >> (2 * ((i + 1) % 4))) & 3;
				d6 = ((base[2 * gfx_chunk + (i + 1) / 4]) >> (2 * ((i + 1) % 4))) & 3;

				midyunit_gfx_rom[i + 0] = d1 | (d2 << 2) | (d3 << 4);
				midyunit_gfx_rom[i + 1] = d4 | (d5 << 2) | (d6 << 4);
			}
			break;

		case 8:
			for (i = 0; i < midyunit_gfx_rom_size; i += 4)
			{
				midyunit_gfx_rom[i + 0] = base[0 * gfx_chunk + i / 4];
				midyunit_gfx_rom[i + 1] = base[1 * gfx_chunk + i / 4];
				midyunit_gfx_rom[i + 2] = base[2 * gfx_chunk + i / 4];
				midyunit_gfx_rom[i + 3] = base[3 * gfx_chunk + i / 4];
			}
			break;
	}

	/* load sound ROMs and set up sound handlers */
	chip_type = sound;
	switch (sound)
	{
		case SOUND_CVSD_SMALL:
			williams_cvsd_init(machine);
			memory_install_write8_handler(cputag_get_address_space(machine, "cvsdcpu", ADDRESS_SPACE_PROGRAM), prot_start, prot_end, 0, 0, cvsd_protection_w);
			cvsd_protection_base = memory_region(machine, "cvsdcpu") + 0x10000 + (prot_start - 0x8000);
			break;

		case SOUND_CVSD:
			williams_cvsd_init(machine);
			memory_install_readwrite8_handler(cputag_get_address_space(machine, "cvsdcpu", ADDRESS_SPACE_PROGRAM), prot_start, prot_end, 0, 0, (read8_space_func)SMH_BANK(9), (write8_space_func)SMH_BANK(9));
			memory_set_bankptr(machine, 9, auto_alloc_array(machine, UINT8, 0x80));
			break;

		case SOUND_ADPCM:
			williams_adpcm_init(machine);
			memory_install_readwrite8_handler(cputag_get_address_space(machine, "adpcm", ADDRESS_SPACE_PROGRAM), prot_start, prot_end, 0, 0, (read8_space_func)SMH_BANK(9), (write8_space_func)SMH_BANK(9));
			memory_set_bankptr(machine, 9, auto_alloc_array(machine, UINT8, 0x80));
			break;

		case SOUND_NARC:
			williams_narc_init(machine);
			memory_install_readwrite8_handler(cputag_get_address_space(machine, "narc1cpu", ADDRESS_SPACE_PROGRAM), prot_start, prot_end, 0, 0, (read8_space_func)SMH_BANK(9), (write8_space_func)SMH_BANK(9));
			memory_set_bankptr(machine, 9, auto_alloc_array(machine, UINT8, 0x80));
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

DRIVER_INIT( narc )
{
	/* common init */
	init_generic(machine, 8, SOUND_NARC, 0xcdff, 0xce29);
}



/*************************************
 *
 *  Y-unit init (CVSD)
 *
 *  music: 6809 driving YM2151, DAC, and CVSD
 *
 *************************************/


/********************** Trog **************************/

DRIVER_INIT( trog )
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
	prot_data = &trog_protection_data;

	/* common init */
	init_generic(machine, 4, SOUND_CVSD_SMALL, 0x9eaf, 0x9ed9);
}


/********************** Smash TV **********************/

DRIVER_INIT( smashtv )
{
	/* common init */
	init_generic(machine, 6, SOUND_CVSD_SMALL, 0x9cf6, 0x9d21);
}


/********************** High Impact Football **********************/

DRIVER_INIT( hiimpact )
{
	/* protection */
	static const struct protection_data hiimpact_protection_data =
	{
		{ 0x0b00, 0x0b00, 0x0b00 },
		{ 0x2000, 0x4000, 0x4000, 0x0000, 0x6000, 0x6000, 0x2000, 0x4000,
		  0x2000, 0x4000, 0x2000, 0x0000, 0x4000, 0x6000, 0x2000 }
	};
	prot_data = &hiimpact_protection_data;

	/* common init */
	init_generic(machine, 6, SOUND_CVSD, 0x9b79, 0x9ba3);
}


/********************** Super High Impact Football **********************/

DRIVER_INIT( shimpact )
{
	/* protection */
	static const struct protection_data shimpact_protection_data =
	{
		{ 0x0f00, 0x0e00, 0x0d00 },
		{ 0x0000, 0x4000, 0x2000, 0x5000, 0x2000, 0x1000, 0x4000, 0x6000,
		  0x3000, 0x0000, 0x2000, 0x5000, 0x5000, 0x5000, 0x2000 }
	};
	prot_data = &shimpact_protection_data;

	/* common init */
	init_generic(machine, 6, SOUND_CVSD, 0x9c06, 0x9c15);
}


/********************** Strike Force **********************/

DRIVER_INIT( strkforc )
{
	/* protection */
	static const struct protection_data strkforc_protection_data =
	{
		{ 0x1234 }
	};
	prot_data = &strkforc_protection_data;

	/* common init */
	init_generic(machine, 4, SOUND_CVSD_SMALL, 0x9f7d, 0x9fa7);
}



/*************************************
 *
 *  Y-unit init (ADPCM)
 *
 *  music: 6809 driving YM2151, DAC, and OKIM6295
 *
 *************************************/


/********************** Mortal Kombat **********************/

DRIVER_INIT( mkyunit )
{
	/* protection */
	static const struct protection_data mk_protection_data =
	{
		{ 0x0d00, 0x0c00, 0x0900 },
		{ 0x4600, 0xf600, 0xa600, 0x0600, 0x2600, 0x9600, 0xc600, 0xe600,
		  0x8600, 0x7600, 0x8600, 0x8600, 0x9600, 0xd600, 0x6600, 0xb600,
		  0xd600, 0xe600, 0xf600, 0x7600, 0xb600, 0xa600, 0x3600 }
	};
	prot_data = &mk_protection_data;

	/* common init */
	init_generic(machine, 6, SOUND_ADPCM, 0xfb9c, 0xfbc6);
}

DRIVER_INIT( mkyawdim )
{
	/* common init */
	init_generic(machine, 6, SOUND_YAWDIM, 0, 0);
}


/*************************************
 *
 *  MK Turbo Ninja protection
 *
 *************************************/

static READ16_HANDLER( mkturbo_prot_r )
{
	/* the security GAL overlays a counter of some sort at 0xfffff400 in ROM space.
     * A startup protection check expects to read back two different values in succession */
	return mame_rand(space->machine);
}

DRIVER_INIT( mkyturbo )
{
	/* protection */
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0xfffff400, 0xfffff40f, 0, 0, mkturbo_prot_r);

	DRIVER_INIT_CALL(mkyunit);
}

/********************** Terminator 2 **********************/

static void term2_init_common(running_machine *machine, write16_space_func hack_w)
{
	/* protection */
	static const struct protection_data term2_protection_data =
	{
		{ 0x0f00, 0x0f00, 0x0f00 },
		{ 0x4000, 0xf000, 0xa000 }
	};
	prot_data = &term2_protection_data;

	/* common init */
	init_generic(machine, 6, SOUND_ADPCM, 0xfa8d, 0xfa9c);

	/* special inputs */
	memory_install_read16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x01c00000, 0x01c0005f, 0, 0, term2_input_r);
	memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x01e00000, 0x01e0001f, 0, 0, term2_sound_w);

	/* HACK: this prevents the freeze on the movies */
	/* until we figure whats causing it, this is better than nothing */
	t2_hack_mem = memory_install_write16_handler(cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM), 0x010aa0e0, 0x010aa0ff, 0, 0, hack_w);
}

DRIVER_INIT( term2 ) { term2_init_common(machine, term2_hack_w); }
DRIVER_INIT( term2la2 ) { term2_init_common(machine, term2la2_hack_w); }
DRIVER_INIT( term2la1 ) { term2_init_common(machine, term2la1_hack_w); }



/********************** Total Carnage **********************/

DRIVER_INIT( totcarn )
{
	/* protection */
	static const struct protection_data totcarn_protection_data =
	{
		{ 0x0f00, 0x0f00, 0x0f00 },
		{ 0x4a00, 0x6a00, 0xda00, 0x6a00, 0x9a00, 0x4a00, 0x2a00, 0x9a00, 0x1a00,
		  0x8a00, 0xaa00 }
	};
	prot_data = &totcarn_protection_data;

	/* common init */
	init_generic(machine, 6, SOUND_ADPCM, 0xfc04, 0xfc2e);
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

MACHINE_RESET( midyunit )
{
	/* reset sound */
	switch (chip_type)
	{
		case SOUND_NARC:
			williams_narc_reset_w(1);
			williams_narc_reset_w(0);
			break;

		case SOUND_CVSD:
		case SOUND_CVSD_SMALL:
			williams_cvsd_reset_w(1);
			williams_cvsd_reset_w(0);
			break;

		case SOUND_ADPCM:
			williams_adpcm_reset_w(1);
			williams_adpcm_reset_w(0);
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

WRITE16_HANDLER( midyunit_sound_w )
{
	/* check for out-of-bounds accesses */
	if (offset)
	{
		logerror("%08X:Unexpected write to sound (hi) = %04X\n", cpu_get_pc(space->cpu), data);
		return;
	}

	/* call through based on the sound type */
	if (ACCESSING_BITS_0_7 && ACCESSING_BITS_8_15)
		switch (chip_type)
		{
			case SOUND_NARC:
				williams_narc_data_w(data);
				break;

			case SOUND_CVSD_SMALL:
			case SOUND_CVSD:
				williams_cvsd_reset_w((~data & 0x100) >> 8);
				williams_cvsd_data_w(space->machine, (data & 0xff) | ((data & 0x200) >> 1));
				break;

			case SOUND_ADPCM:
				williams_adpcm_reset_w((~data & 0x100) >> 8);
				williams_adpcm_data_w(data);
				break;

			case SOUND_YAWDIM:
				soundlatch_w(space, 0, data);
				cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_NMI, PULSE_LINE);
				break;
		}
}
