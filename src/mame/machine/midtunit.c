// license:???
// copyright-holders:Alex Pasadyn, Zsolt Vasvari, Kurt Mahan, Ernesto Corvi, Aaron Giles
/*************************************************************************

    Driver for Midway T-unit games.

**************************************************************************/

#include "emu.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/m6809/m6809.h"
#include "includes/midtunit.h"


/* compile-time constants */
#define ENABLE_ALL_JDREDD_LEVELS    0


/* constant definitions */
#define SOUND_ADPCM                 1
#define SOUND_ADPCM_LARGE           2
#define SOUND_DCS                   3


/*************************************
 *
 *  State saving
 *
 *************************************/

void midtunit_state::register_state_saving()
{
	save_item(NAME(m_cmos_write_enable));
	save_item(NAME(m_fake_sound_state));
	save_item(NAME(m_mk_prot_index));
	save_item(NAME(m_mk2_prot_data));
	save_item(NAME(m_nbajam_prot_queue));
	save_item(NAME(m_nbajam_prot_index));
	save_item(NAME(m_jdredd_prot_index));
	save_item(NAME(m_jdredd_prot_max));
}



/*************************************
 *
 *  CMOS reads/writes
 *
 *************************************/

WRITE16_MEMBER(midtunit_state::midtunit_cmos_enable_w)
{
	m_cmos_write_enable = 1;
}


WRITE16_MEMBER(midtunit_state::midtunit_cmos_w)
{
	if (1)/*m_cmos_write_enable*/
	{
		COMBINE_DATA(m_nvram+offset);
		m_cmos_write_enable = 0;
	}
	else
	{
		logerror("%08X:Unexpected CMOS W @ %05X\n", space.device().safe_pc(), offset);
		popmessage("Bad CMOS write");
	}
}


READ16_MEMBER(midtunit_state::midtunit_cmos_r)
{
	return m_nvram[offset];
}


/*************************************
 *
 *  Mortal Kombat (T-unit) protection
 *
 *************************************/

static const UINT8 mk_prot_values[] =
{
	0x13, 0x27, 0x0f, 0x1f, 0x3e, 0x3d, 0x3b, 0x37,
	0x2e, 0x1c, 0x38, 0x31, 0x22, 0x05, 0x0a, 0x15,
	0x2b, 0x16, 0x2d, 0x1a, 0x34, 0x28, 0x10, 0x21,
	0x03, 0x06, 0x0c, 0x19, 0x32, 0x24, 0x09, 0x13,
	0x27, 0x0f, 0x1f, 0x3e, 0x3d, 0x3b, 0x37, 0x2e,
	0x1c, 0x38, 0x31, 0x22, 0x05, 0x0a, 0x15, 0x2b,
	0x16, 0x2d, 0x1a, 0x34, 0x28, 0x10, 0x21, 0x03,
	0xff
};

READ16_MEMBER(midtunit_state::mk_prot_r)
{
	logerror("%08X:Protection R @ %05X = %04X\n", space.device().safe_pc(), offset, mk_prot_values[m_mk_prot_index] << 9);

	/* just in case */
	if (m_mk_prot_index >= sizeof(mk_prot_values))
	{
		logerror("%08X:Unexpected protection R @ %05X\n", space.device().safe_pc(), offset);
		m_mk_prot_index = 0;
	}

	return mk_prot_values[m_mk_prot_index++] << 9;
}

WRITE16_MEMBER(midtunit_state::mk_prot_w)
{
	if (ACCESSING_BITS_8_15)
	{
		int first_val = (data >> 9) & 0x3f;
		int i;

		/* find the desired first value and stop then */
		for (i = 0; i < sizeof(mk_prot_values); i++)
			if (mk_prot_values[i] == first_val)
			{
				m_mk_prot_index = i;
				break;
			}

		/* just in case */
		if (i == sizeof(mk_prot_values))
		{
			logerror("%08X:Unhandled protection W @ %05X = %04X\n", space.device().safe_pc(), offset, data);
			m_mk_prot_index = 0;
		}

		logerror("%08X:Protection W @ %05X = %04X\n", space.device().safe_pc(), offset, data);
	}
}



/*************************************
 *
 *  MK Turbo Ninja protection
 *
 *************************************/

READ16_MEMBER(midtunit_state::mkturbo_prot_r)
{
	/* the security GAL overlays a counter of some sort at 0xfffff400 in ROM &space.
	 * A startup protection check expects to read back two different values in succession */
	return machine().rand();
}



/*************************************
 *
 *  Mortal Kombat 2 protection
 *
 *************************************/

READ16_MEMBER(midtunit_state::mk2_prot_const_r)
{
	return 2;
}

READ16_MEMBER(midtunit_state::mk2_prot_r)
{
	return m_mk2_prot_data;
}

READ16_MEMBER(midtunit_state::mk2_prot_shift_r)
{
	return m_mk2_prot_data >> 1;
}

WRITE16_MEMBER(midtunit_state::mk2_prot_w)
{
	COMBINE_DATA(&m_mk2_prot_data);
}



/*************************************
 *
 *  NBA Jam protection
 *
 *************************************/

static const UINT32 nbajam_prot_values[128] =
{
	0x21283b3b, 0x2439383b, 0x31283b3b, 0x302b3938, 0x31283b3b, 0x302b3938, 0x232f2f2f, 0x26383b3b,
	0x21283b3b, 0x2439383b, 0x312a1224, 0x302b1120, 0x312a1224, 0x302b1120, 0x232d283b, 0x26383b3b,
	0x2b3b3b3b, 0x2e2e2e2e, 0x39383b1b, 0x383b3b1b, 0x3b3b3b1b, 0x3a3a3a1a, 0x2b3b3b3b, 0x2e2e2e2e,
	0x2b39383b, 0x2e2e2e2e, 0x393a1a18, 0x383b1b1b, 0x3b3b1b1b, 0x3a3a1a18, 0x2b39383b, 0x2e2e2e2e,
	0x01202b3b, 0x0431283b, 0x11202b3b, 0x1021283b, 0x11202b3b, 0x1021283b, 0x03273b3b, 0x06302b39,
	0x09302b39, 0x0c232f2f, 0x19322e06, 0x18312a12, 0x19322e06, 0x18312a12, 0x0b31283b, 0x0e26383b,
	0x03273b3b, 0x06302b39, 0x11202b3b, 0x1021283b, 0x13273938, 0x12243938, 0x03273b3b, 0x06302b39,
	0x0b31283b, 0x0e26383b, 0x19322e06, 0x18312a12, 0x1b332f05, 0x1a302b11, 0x0b31283b, 0x0e26383b,
	0x21283b3b, 0x2439383b, 0x31283b3b, 0x302b3938, 0x31283b3b, 0x302b3938, 0x232f2f2f, 0x26383b3b,
	0x21283b3b, 0x2439383b, 0x312a1224, 0x302b1120, 0x312a1224, 0x302b1120, 0x232d283b, 0x26383b3b,
	0x2b3b3b3b, 0x2e2e2e2e, 0x39383b1b, 0x383b3b1b, 0x3b3b3b1b, 0x3a3a3a1a, 0x2b3b3b3b, 0x2e2e2e2e,
	0x2b39383b, 0x2e2e2e2e, 0x393a1a18, 0x383b1b1b, 0x3b3b1b1b, 0x3a3a1a18, 0x2b39383b, 0x2e2e2e2e,
	0x01202b3b, 0x0431283b, 0x11202b3b, 0x1021283b, 0x11202b3b, 0x1021283b, 0x03273b3b, 0x06302b39,
	0x09302b39, 0x0c232f2f, 0x19322e06, 0x18312a12, 0x19322e06, 0x18312a12, 0x0b31283b, 0x0e26383b,
	0x03273b3b, 0x06302b39, 0x11202b3b, 0x1021283b, 0x13273938, 0x12243938, 0x03273b3b, 0x06302b39,
	0x0b31283b, 0x0e26383b, 0x19322e06, 0x18312a12, 0x1b332f05, 0x1a302b11, 0x0b31283b, 0x0e26383b
};

static const UINT32 nbajamte_prot_values[128] =
{
	0x00000000, 0x04081020, 0x08102000, 0x0c183122, 0x10200000, 0x14281020, 0x18312204, 0x1c393326,
	0x20000001, 0x24081021, 0x28102000, 0x2c183122, 0x30200001, 0x34281021, 0x38312204, 0x3c393326,
	0x00000102, 0x04081122, 0x08102102, 0x0c183122, 0x10200000, 0x14281020, 0x18312204, 0x1c393326,
	0x20000103, 0x24081123, 0x28102102, 0x2c183122, 0x30200001, 0x34281021, 0x38312204, 0x3c393326,
	0x00010204, 0x04091224, 0x08112204, 0x0c193326, 0x10210204, 0x14291224, 0x18312204, 0x1c393326,
	0x20000001, 0x24081021, 0x28102000, 0x2c183122, 0x30200001, 0x34281021, 0x38312204, 0x3c393326,
	0x00010306, 0x04091326, 0x08112306, 0x0c193326, 0x10210204, 0x14291224, 0x18312204, 0x1c393326,
	0x20000103, 0x24081123, 0x28102102, 0x2c183122, 0x30200001, 0x34281021, 0x38312204, 0x3c393326,
	0x00000000, 0x01201028, 0x02213018, 0x03012030, 0x04223138, 0x05022110, 0x06030120, 0x07231108,
	0x08042231, 0x09243219, 0x0a251229, 0x0b050201, 0x0c261309, 0x0d060321, 0x0e072311, 0x0f273339,
	0x10080422, 0x1128140a, 0x1229343a, 0x13092412, 0x142a351a, 0x150a2532, 0x160b0502, 0x172b152a,
	0x180c2613, 0x192c363b, 0x1a2d160b, 0x1b0d0623, 0x1c2e172b, 0x1d0e0703, 0x1e0f2733, 0x1f2f371b,
	0x20100804, 0x2130182c, 0x2231381c, 0x23112834, 0x2432393c, 0x25122914, 0x26130924, 0x2733190c,
	0x28142a35, 0x29343a1d, 0x2a351a2d, 0x2b150a05, 0x2c361b0d, 0x2d160b25, 0x2e172b15, 0x2f373b3d,
	0x30180c26, 0x31381c0e, 0x32393c3e, 0x33192c16, 0x343a3d1e, 0x351a2d36, 0x361b0d06, 0x373b1d2e,
	0x381c2e17, 0x393c3e3f, 0x3a3d1e0f, 0x3b1d0e27, 0x3c3e1f2f, 0x3d1e0f07, 0x3e1f2f37, 0x3f3f3f1f
};

READ16_MEMBER(midtunit_state::nbajam_prot_r)
{
	int result = m_nbajam_prot_queue[m_nbajam_prot_index];
	if (m_nbajam_prot_index < 4)
		m_nbajam_prot_index++;
	return result;
}

WRITE16_MEMBER(midtunit_state::nbajam_prot_w)
{
	int table_index = (offset >> 6) & 0x7f;
	UINT32 protval = m_nbajam_prot_table[table_index];

	m_nbajam_prot_queue[0] = data;
	m_nbajam_prot_queue[1] = ((protval >> 24) & 0xff) << 9;
	m_nbajam_prot_queue[2] = ((protval >> 16) & 0xff) << 9;
	m_nbajam_prot_queue[3] = ((protval >> 8) & 0xff) << 9;
	m_nbajam_prot_queue[4] = ((protval >> 0) & 0xff) << 9;
	m_nbajam_prot_index = 0;
}



/*************************************
 *
 *  Judge Dredd protection
 *
 *************************************/

static const UINT8 jdredd_prot_values_10740[] =
{
	0x14,0x2A,0x15,0x0A,0x25,0x32,0x39,0x1C,
	0x2E,0x37,0x3B,0x1D,0x2E,0x37,0x1B,0x0D,
	0x26,0x33,0x39,0x3C,0x1E,0x2F,0x37,0x3B,
	0x3D,0x3E,0x3F,0x1F,0x2F,0x17,0x0B,0x25,
	0x32,0x19,0x0C,0x26,0x33,0x19,0x2C,0x16,
	0x2B,0x15,0x0A,0x05,0x22,0x00
};

static const UINT8 jdredd_prot_values_13240[] =
{
	0x28
};

static const UINT8 jdredd_prot_values_76540[] =
{
	0x04,0x08
};

static const UINT8 jdredd_prot_values_77760[] =
{
	0x14,0x2A,0x14,0x2A,0x35,0x2A,0x35,0x1A,
	0x35,0x1A,0x2D,0x1A,0x2D,0x36,0x2D,0x36,
	0x1B,0x36,0x1B,0x36,0x2C,0x36,0x2C,0x18,
	0x2C,0x18,0x31,0x18,0x31,0x22,0x31,0x22,
	0x04,0x22,0x04,0x08,0x04,0x08,0x10,0x08,
	0x10,0x20,0x10,0x20,0x00,0x20,0x00,0x00,
	0x00,0x00,0x01,0x00,0x01,0x02,0x01,0x02,
	0x05,0x02,0x05,0x0B,0x05,0x0B,0x16,0x0B,
	0x16,0x2C,0x16,0x2C,0x18,0x2C,0x18,0x31,
	0x18,0x31,0x22,0x31,0x22,0x04,0x22,0x04,
	0x08,0x04,0x08,0x10,0x08,0x10,0x20,0x10,
	0x20,0x00,0x00
};

static const UINT8 jdredd_prot_values_80020[] =
{
	0x3A,0x1D,0x2E,0x37,0x00,0x00,0x2C,0x1C,
	0x39,0x33,0x00,0x00,0x00,0x00,0x00,0x00
};

WRITE16_MEMBER(midtunit_state::jdredd_prot_w)
{
	logerror("%08X:jdredd_prot_w(%04X,%04X)\n", space.device().safe_pcbase(), offset*16, data);

	switch (offset)
	{
		case TOWORD(0x10740):
			m_jdredd_prot_index = 0;
			m_jdredd_prot_table = jdredd_prot_values_10740;
			m_jdredd_prot_max = sizeof(jdredd_prot_values_10740);
			logerror("-- reset prot table 10740\n");
			break;

		case TOWORD(0x13240):
			m_jdredd_prot_index = 0;
			m_jdredd_prot_table = jdredd_prot_values_13240;
			m_jdredd_prot_max = sizeof(jdredd_prot_values_13240);
			logerror("-- reset prot table 13240\n");
			break;

		case TOWORD(0x76540):
			m_jdredd_prot_index = 0;
			m_jdredd_prot_table = jdredd_prot_values_76540;
			m_jdredd_prot_max = sizeof(jdredd_prot_values_76540);
			logerror("-- reset prot table 76540\n");
			break;

		case TOWORD(0x77760):
			m_jdredd_prot_index = 0;
			m_jdredd_prot_table = jdredd_prot_values_77760;
			m_jdredd_prot_max = sizeof(jdredd_prot_values_77760);
			logerror("-- reset prot table 77760\n");
			break;

		case TOWORD(0x80020):
			m_jdredd_prot_index = 0;
			m_jdredd_prot_table = jdredd_prot_values_80020;
			m_jdredd_prot_max = sizeof(jdredd_prot_values_80020);
			logerror("-- reset prot table 80020\n");
			break;
	}
}

READ16_MEMBER(midtunit_state::jdredd_prot_r)
{
	UINT16 result = 0xffff;

	if (m_jdredd_prot_table && m_jdredd_prot_index < m_jdredd_prot_max)
		result = m_jdredd_prot_table[m_jdredd_prot_index++] << 9;

	logerror("%08X:jdredd_prot_r(%04X) = %04X\n", space.device().safe_pcbase(), offset*16, result);
	return result;
}


#if ENABLE_ALL_JDREDD_LEVELS
static UINT16 *jdredd_hack;
READ16_MEMBER(midtunit_state::jdredd_hack_r)
{
	if (space.device().safe_pc() == 0xFFBA7EB0)
	{
		fprintf(stderr, "jdredd_hack_r\n");
		return 0;
	}

	return jdredd_hack[offset];
}
#endif



/*************************************
 *
 *  Generic driver init
 *
 *************************************/

void midtunit_state::init_tunit_generic(int sound)
{
	/* register for state saving */
	register_state_saving();

	/* load sound ROMs and set up sound handlers */
	m_chip_type = sound;


	/* default graphics functionality */
	m_gfx_rom_large = 0;
}



/*************************************
 *
 *  T-unit init (ADPCM)
 *
 *  music: 6809 driving YM2151, DAC, and OKIM6295
 *
 *************************************/

DRIVER_INIT_MEMBER(midtunit_state,mktunit)
{
	/* common init */
	init_tunit_generic(SOUND_ADPCM);

	/* protection */
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1b00000, 0x1b6ffff, read16_delegate(FUNC(midtunit_state::mk_prot_r),this), write16_delegate(FUNC(midtunit_state::mk_prot_w),this));

	/* sound chip protection (hidden RAM) */
	machine().device("adpcm:cpu")->memory().space(AS_PROGRAM).install_ram(0xfb9c, 0xfbc6);
}

DRIVER_INIT_MEMBER(midtunit_state,mkturbo)
{
	/* protection */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xfffff400, 0xfffff40f, read16_delegate(FUNC(midtunit_state::mkturbo_prot_r),this));

	DRIVER_INIT_CALL(mktunit);
}


void midtunit_state::init_nbajam_common(int te_protection)
{
	/* common init */
	init_tunit_generic(SOUND_ADPCM_LARGE);
	/* protection */
	if (!te_protection)
	{
		m_nbajam_prot_table = nbajam_prot_values;
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1b14020, 0x1b2503f, read16_delegate(FUNC(midtunit_state::nbajam_prot_r),this), write16_delegate(FUNC(midtunit_state::nbajam_prot_w),this));
	}
	else
	{
		m_nbajam_prot_table = nbajamte_prot_values;
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1b15f40, 0x1b37f5f, read16_delegate(FUNC(midtunit_state::nbajam_prot_r),this), write16_delegate(FUNC(midtunit_state::nbajam_prot_w),this));
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1b95f40, 0x1bb7f5f, read16_delegate(FUNC(midtunit_state::nbajam_prot_r),this), write16_delegate(FUNC(midtunit_state::nbajam_prot_w),this));
	}

	/* sound chip protection (hidden RAM) */
	if (!te_protection)
		machine().device("adpcm:cpu")->memory().space(AS_PROGRAM).install_ram(0xfbaa, 0xfbd4);
	else
		machine().device("adpcm:cpu")->memory().space(AS_PROGRAM).install_ram(0xfbec, 0xfc16);
}

DRIVER_INIT_MEMBER(midtunit_state,nbajam)
{
	init_nbajam_common(0);
}

DRIVER_INIT_MEMBER(midtunit_state,nbajamte)
{
	init_nbajam_common(1);
}

DRIVER_INIT_MEMBER(midtunit_state,jdreddp)
{
	/* common init */
	init_tunit_generic(SOUND_ADPCM_LARGE);

	/* looks like the watchdog needs to be disabled */
	m_maincpu->space(AS_PROGRAM).nop_write(0x01d81060, 0x01d8107f);

	/* protection */
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1b00000, 0x1bfffff, read16_delegate(FUNC(midtunit_state::jdredd_prot_r),this), write16_delegate(FUNC(midtunit_state::jdredd_prot_w),this));

	/* sound chip protection (hidden RAM) */
	machine().device("adpcm:cpu")->memory().space(AS_PROGRAM).install_read_bank(0xfbcf, 0xfbf9, "bank7");
	machine().device("adpcm:cpu")->memory().space(AS_PROGRAM).install_write_bank(0xfbcf, 0xfbf9, "bank9");
	membank("adpcm:bank9")->set_base(auto_alloc_array(machine(), UINT8, 0x80));

#if ENABLE_ALL_JDREDD_LEVELS
	/* how about the final levels? */
	jdredd_hack = m_maincpu->space(AS_PROGRAM).install_read_handler(0xFFBA7FF0, 0xFFBA7FFf, read16_delegate(FUNC(midtunit_state::jdredd_hack_r), this));
#endif
}



/*************************************
 *
 *  T-unit init (DCS)
 *
 *  music: ADSP2105
 *
 *************************************/

DRIVER_INIT_MEMBER(midtunit_state,mk2)
{
	/* common init */
	init_tunit_generic(SOUND_DCS);
	m_gfx_rom_large = 1;

	/* protection */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x00f20c60, 0x00f20c7f, write16_delegate(FUNC(midtunit_state::mk2_prot_w),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x00f42820, 0x00f4283f, write16_delegate(FUNC(midtunit_state::mk2_prot_w),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x01a190e0, 0x01a190ff, read16_delegate(FUNC(midtunit_state::mk2_prot_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x01a191c0, 0x01a191df, read16_delegate(FUNC(midtunit_state::mk2_prot_shift_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x01a3d0c0, 0x01a3d0ff, read16_delegate(FUNC(midtunit_state::mk2_prot_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x01d9d1e0, 0x01d9d1ff, read16_delegate(FUNC(midtunit_state::mk2_prot_const_r),this));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x01def920, 0x01def93f, read16_delegate(FUNC(midtunit_state::mk2_prot_const_r),this));
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

MACHINE_RESET_MEMBER(midtunit_state,midtunit)
{
	/* reset sound */
	switch (m_chip_type)
	{
		case SOUND_ADPCM:
		case SOUND_ADPCM_LARGE:
			m_adpcm_sound->reset_write(1);
			m_adpcm_sound->reset_write(0);
			break;

		case SOUND_DCS:
			m_dcs->reset_w(1);
			m_dcs->reset_w(0);
			break;
	}
}



/*************************************
 *
 *  Sound write handlers
 *
 *************************************/

READ16_MEMBER(midtunit_state::midtunit_sound_state_r)
{
/*  logerror("%08X:Sound status read\n", space.device().safe_pc());*/

	if (m_chip_type == SOUND_DCS)
		return m_dcs->control_r() >> 4;

	if (m_fake_sound_state)
	{
		m_fake_sound_state--;
		return 0;
	}
	return ~0;
}

READ16_MEMBER(midtunit_state::midtunit_sound_r)
{
	logerror("%08X:Sound data read\n", space.device().safe_pc());

	if (m_chip_type == SOUND_DCS)
		return m_dcs->data_r() & 0xff;

	return ~0;
}

WRITE16_MEMBER(midtunit_state::midtunit_sound_w)
{
	/* check for out-of-bounds accesses */
	if (!offset)
	{
		logerror("%08X:Unexpected write to sound (lo) = %04X\n", space.device().safe_pc(), data);
		return;
	}

	/* call through based on the sound type */
	if (ACCESSING_BITS_0_7 && ACCESSING_BITS_8_15)
		switch (m_chip_type)
		{
			case SOUND_ADPCM:
			case SOUND_ADPCM_LARGE:
				m_adpcm_sound->reset_write(~data & 0x100);
				m_adpcm_sound->write(space, offset, data & 0xff);

				/* the games seem to check for $82 loops, so this should be just barely enough */
				m_fake_sound_state = 128;
				break;

			case SOUND_DCS:
				logerror("%08X:Sound write = %04X\n", space.device().safe_pc(), data);
				m_dcs->reset_w(~data & 0x100);
				m_dcs->data_w(data & 0xff);
				/* the games seem to check for $82 loops, so this should be just barely enough */
				m_fake_sound_state = 128;
				break;
		}
}
