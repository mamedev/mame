// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn, Zsolt Vasvari, Ernesto Corvi, Aaron Giles
// thanks-to:Kurt Mahan
/*************************************************************************

    Driver for Midway T-unit games.

**************************************************************************/

#include "emu.h"
#include "midtunit.h"

#include "cpu/m6809/m6809.h"
#include "cpu/tms34010/tms34010.h"


#define LOG_PROT    (1U << 1)
#define LOG_CMOS    (1U << 2)
#define LOG_SOUND   (1U << 3)

#define LOG_ALL     (LOG_PROT | LOG_CMOS | LOG_SOUND)

#define VERBOSE (0)
#include "logmacro.h"

#define LOGPROT(...)  LOGMASKED(LOG_PROT,  __VA_ARGS__)
#define LOGCMOS(...)  LOGMASKED(LOG_CMOS,  __VA_ARGS__)
#define LOGSOUND(...) LOGMASKED(LOG_SOUND, __VA_ARGS__)


/*************************************
 *
 *  State saving
 *
 *************************************/

void midtunit_base_state::machine_start()
{
	// register for state saving
	save_item(NAME(m_cmos_write_enable));
}

void midtunit_adpcm_state::machine_start()
{
	midtunit_base_state::machine_start();

	// register for state saving
	save_item(NAME(m_fake_sound_state));
	save_item(NAME(m_mk_prot_index));
	save_item(NAME(m_nbajam_prot_queue));
	save_item(NAME(m_nbajam_prot_index));
	save_item(NAME(m_jdredd_prot_index));
	save_item(NAME(m_jdredd_prot_max));
}

void mk2_state::machine_start()
{
	midtunit_base_state::machine_start();

	// register for state saving
	save_item(NAME(m_mk2_prot_data));
}



/*************************************
 *
 *  CMOS reads/writes
 *
 *************************************/

void midtunit_base_state::cmos_enable_w(uint16_t data)
{
	m_cmos_write_enable = 1;
}


void midtunit_base_state::cmos_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (1)// m_cmos_write_enable
	{
		COMBINE_DATA(m_nvram+offset);
		m_cmos_write_enable = 0;
	}
	else
	{
		LOGCMOS("%08X:Unexpected CMOS W @ %05X\n", m_maincpu->pc(), offset);
		popmessage("Bad CMOS write");
	}
}


uint16_t midtunit_base_state::cmos_r(offs_t offset)
{
	return m_nvram[offset];
}


/*************************************
 *
 *  Mortal Kombat (T-unit) protection
 *
 *************************************/

static const uint8_t mk_prot_values[] =
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

uint16_t midtunit_adpcm_state::mk_prot_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		LOGPROT("%s:Protection R @ %05X = %04X\n", machine().describe_context(), offset, mk_prot_values[m_mk_prot_index] << 9);

	// just in case
	if (m_mk_prot_index >= sizeof(mk_prot_values))
	{
		if (!machine().side_effects_disabled())
			LOGPROT("%s:Unexpected protection R @ %05X\n", machine().describe_context(), offset);
		m_mk_prot_index = 0;
	}

	uint16_t const result = mk_prot_values[m_mk_prot_index] << 9;
	if (!machine().side_effects_disabled())
		m_mk_prot_index++;

	return result;
}

void midtunit_adpcm_state::mk_prot_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		int const first_val = (data >> 9) & 0x3f;
		int i;

		// find the desired first value and stop then
		for (i = 0; i < sizeof(mk_prot_values); i++)
			if (mk_prot_values[i] == first_val)
			{
				m_mk_prot_index = i;
				break;
			}

		// just in case
		if (i == sizeof(mk_prot_values))
		{
			LOGPROT("%s:Unhandled protection W @ %05X = %04X\n", machine().describe_context(), offset, data);
			m_mk_prot_index = 0;
		}

		LOGPROT("%s:Protection W @ %05X = %04X\n", machine().describe_context(), offset, data);
	}
}



/*************************************
 *
 *  MK Turbo Ninja protection
 *
 *************************************/

uint16_t midtunit_adpcm_state::mkturbo_prot_r()
{
	// the security GAL overlays a counter of some sort at 0xfffff400 in ROM &space.
	// A startup protection check expects to read back two different values in succession
	return machine().rand();
}



/*************************************
 *
 *  Mortal Kombat 2 protection
 *
 *************************************/

uint16_t mk2_state::mk2_prot_const_r()
{
	return 2;
}

uint16_t mk2_state::mk2_prot_r()
{
	return m_mk2_prot_data;
}

uint16_t mk2_state::mk2_prot_shift_r()
{
	return m_mk2_prot_data >> 1;
}

void mk2_state::mk2_prot_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_mk2_prot_data);
}



/*************************************
 *
 *  NBA Jam protection
 *
 *************************************/

static const uint32_t nbajam_prot_values[128] =
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

static const uint32_t nbajamte_prot_values[128] =
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

uint16_t midtunit_adpcm_state::nbajam_prot_r()
{
	uint16_t const result = m_nbajam_prot_queue[m_nbajam_prot_index];
	if (!machine().side_effects_disabled())
	{
		if (m_nbajam_prot_index < 4)
			m_nbajam_prot_index++;
	}
	return result;
}

void midtunit_adpcm_state::nbajam_prot_w(offs_t offset, uint16_t data)
{
	int const table_index = (offset >> 6) & 0x7f;
	uint32_t const protval = m_nbajam_prot_table[table_index];

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

static const uint8_t jdredd_prot_values_10740[] =
{
	0x14,0x2A,0x15,0x0A,0x25,0x32,0x39,0x1C,
	0x2E,0x37,0x3B,0x1D,0x2E,0x37,0x1B,0x0D,
	0x26,0x33,0x39,0x3C,0x1E,0x2F,0x37,0x3B,
	0x3D,0x3E,0x3F,0x1F,0x2F,0x17,0x0B,0x25,
	0x32,0x19,0x0C,0x26,0x33,0x19,0x2C,0x16,
	0x2B,0x15,0x0A,0x05,0x22,0x00
};

static const uint8_t jdredd_prot_values_13240[] =
{
	0x28
};

static const uint8_t jdredd_prot_values_76540[] =
{
	0x04,0x08
};

static const uint8_t jdredd_prot_values_77760[] =
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

static const uint8_t jdredd_prot_values_80020[] =
{
	0x3A,0x1D,0x2E,0x37,0x00,0x00,0x2C,0x1C,
	0x39,0x33,0x00,0x00,0x00,0x00,0x00,0x00
};

void midtunit_adpcm_state::jdredd_prot_w(offs_t offset, uint16_t data)
{
	LOGPROT("%s:jdredd_prot_w(%04X,%04X)\n", machine().describe_context(), offset*16, data);

	switch (offset)
	{
		case 0x1074:
			m_jdredd_prot_index = 0;
			m_jdredd_prot_table = jdredd_prot_values_10740;
			m_jdredd_prot_max = sizeof(jdredd_prot_values_10740);
			LOGPROT("-- reset prot table 10740\n");
			break;

		case 0x1324:
			m_jdredd_prot_index = 0;
			m_jdredd_prot_table = jdredd_prot_values_13240;
			m_jdredd_prot_max = sizeof(jdredd_prot_values_13240);
			LOGPROT("-- reset prot table 13240\n");
			break;

		case 0x7654:
			m_jdredd_prot_index = 0;
			m_jdredd_prot_table = jdredd_prot_values_76540;
			m_jdredd_prot_max = sizeof(jdredd_prot_values_76540);
			LOGPROT("-- reset prot table 76540\n");
			break;

		case 0x7776:
			m_jdredd_prot_index = 0;
			m_jdredd_prot_table = jdredd_prot_values_77760;
			m_jdredd_prot_max = sizeof(jdredd_prot_values_77760);
			LOGPROT("-- reset prot table 77760\n");
			break;

		case 0x8002:
			m_jdredd_prot_index = 0;
			m_jdredd_prot_table = jdredd_prot_values_80020;
			m_jdredd_prot_max = sizeof(jdredd_prot_values_80020);
			LOGPROT("-- reset prot table 80020\n");
			break;
	}
}

uint16_t midtunit_adpcm_state::jdredd_prot_r(offs_t offset)
{
	uint16_t result = 0xffff;

	if (m_jdredd_prot_table && m_jdredd_prot_index < m_jdredd_prot_max)
	{
		result = m_jdredd_prot_table[m_jdredd_prot_index] << 9;
		if (!machine().side_effects_disabled())
			m_jdredd_prot_index++;
	}

	if (!machine().side_effects_disabled())
		LOGPROT("%s:jdredd_prot_r(%04X) = %04X\n", machine().describe_context(), offset*16, result);
	return result;
}


/*************************************
 *
 *  T-unit init (ADPCM)
 *
 *  music: 6809 driving YM2151, DAC, and OKIM6295
 *
 *************************************/

void midtunit_adpcm_state::init_mktunit()
{
	// protection
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x1b00000, 0x1b6ffff, read16sm_delegate(*this, FUNC(midtunit_adpcm_state::mk_prot_r)));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x1b00000, 0x1b6ffff, write16s_delegate(*this, FUNC(midtunit_adpcm_state::mk_prot_w)));

	m_hidden_ram = std::make_unique<uint8_t[]>(43);
	save_pointer(NAME(m_hidden_ram), 43);

	// sound chip protection (hidden RAM)
	m_adpcm_sound->get_cpu()->space(AS_PROGRAM).install_ram(0xfb9c, 0xfbc6, m_hidden_ram.get());
}

void midtunit_adpcm_state::init_mkturbo()
{
	// protection
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xfffff400, 0xfffff40f, read16smo_delegate(*this, FUNC(midtunit_adpcm_state::mkturbo_prot_r)));

	init_mktunit();
}


void midtunit_adpcm_state::init_nbajam_common(int te_protection)
{
	// protection
	if (!te_protection)
	{
		m_nbajam_prot_table = nbajam_prot_values;
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1b14020, 0x1b2503f, read16smo_delegate(*this, FUNC(midtunit_adpcm_state::nbajam_prot_r)));
		m_maincpu->space(AS_PROGRAM).install_write_handler(0x1b14020, 0x1b2503f, write16sm_delegate(*this, FUNC(midtunit_adpcm_state::nbajam_prot_w)));
	}
	else
	{
		m_nbajam_prot_table = nbajamte_prot_values;
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1b15f40, 0x1b37f5f, read16smo_delegate(*this, FUNC(midtunit_adpcm_state::nbajam_prot_r)));
		m_maincpu->space(AS_PROGRAM).install_write_handler(0x1b15f40, 0x1b37f5f, write16sm_delegate(*this, FUNC(midtunit_adpcm_state::nbajam_prot_w)));
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1b95f40, 0x1bb7f5f, read16smo_delegate(*this, FUNC(midtunit_adpcm_state::nbajam_prot_r)));
		m_maincpu->space(AS_PROGRAM).install_write_handler(0x1b95f40, 0x1bb7f5f, write16sm_delegate(*this, FUNC(midtunit_adpcm_state::nbajam_prot_w)));
	}

	m_hidden_ram = std::make_unique<uint8_t[]>(43);
	save_pointer(NAME(m_hidden_ram), 43);

	// sound chip protection (hidden RAM)
	if (!te_protection)
		m_adpcm_sound->get_cpu()->space(AS_PROGRAM).install_ram(0xfbaa, 0xfbd4, m_hidden_ram.get());
	else
		m_adpcm_sound->get_cpu()->space(AS_PROGRAM).install_ram(0xfbec, 0xfc16, m_hidden_ram.get());
}

void midtunit_adpcm_state::init_nbajam()
{
	init_nbajam_common(0);
}

void midtunit_adpcm_state::init_nbajamte()
{
	init_nbajam_common(1);
}

void midtunit_adpcm_state::init_jdreddp()
{
	// looks like the watchdog needs to be disabled
	m_maincpu->space(AS_PROGRAM).nop_write(0x01d81060, 0x01d8107f);

	// protection
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0x1b00000, 0x1bfffff, read16sm_delegate(*this, FUNC(midtunit_adpcm_state::jdredd_prot_r)), write16sm_delegate(*this, FUNC(midtunit_adpcm_state::jdredd_prot_w)));

	m_hidden_ram = std::make_unique<uint8_t[]>(43);
	save_pointer(NAME(m_hidden_ram), 43);

	// sound chip protection (hidden RAM)
	m_adpcm_sound->get_cpu()->space(AS_PROGRAM).install_ram(0xfbcf, 0xfbf9, m_hidden_ram.get());
}



/*************************************
 *
 *  T-unit init (DCS)
 *
 *  music: ADSP2105
 *
 *************************************/

void mk2_state::init_mk2()
{
	// protection
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x00f20c60, 0x00f20c7f, write16s_delegate(*this, FUNC(mk2_state::mk2_prot_w)));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x00f42820, 0x00f4283f, write16s_delegate(*this, FUNC(mk2_state::mk2_prot_w)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x01a190e0, 0x01a190ff, read16smo_delegate(*this, FUNC(mk2_state::mk2_prot_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x01a191c0, 0x01a191df, read16smo_delegate(*this, FUNC(mk2_state::mk2_prot_shift_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x01a3d0c0, 0x01a3d0ff, read16smo_delegate(*this, FUNC(mk2_state::mk2_prot_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x01d9d1e0, 0x01d9d1ff, read16smo_delegate(*this, FUNC(mk2_state::mk2_prot_const_r)));
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x01def920, 0x01def93f, read16smo_delegate(*this, FUNC(mk2_state::mk2_prot_const_r)));
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

void midtunit_adpcm_state::machine_reset()
{
	midtunit_base_state::machine_reset();

	// reset sound
	m_adpcm_sound->reset_write(1);
	m_adpcm_sound->reset_write(0);
}

void mk2_state::machine_reset()
{
	midtunit_base_state::machine_reset();

	// reset sound
	m_dcs->reset_w(0);
	m_dcs->reset_w(1);
}



/*************************************
 *
 *  Sound write handlers
 *
 *************************************/

uint16_t midtunit_adpcm_state::sound_state_r()
{
//  LOGSOUND("%s:Sound status read\n", machine().describe_context());

	if (m_fake_sound_state)
	{
		if (!machine().side_effects_disabled())
			m_fake_sound_state--;
		return 0;
	}
	return ~0;
}

uint16_t midtunit_adpcm_state::sound_r()
{
	if (!machine().side_effects_disabled())
		LOGSOUND("%08X:Sound data read\n", m_maincpu->pc());

	return ~0;
}

void midtunit_adpcm_state::sound_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// check for out-of-bounds accesses
	if (!offset)
	{
		LOGSOUND("%08X:Unexpected write to sound (lo) = %04X\n", m_maincpu->pc(), data);
		return;
	}

	// call through based on the sound type
	if (ACCESSING_BITS_0_7 && ACCESSING_BITS_8_15)
	{
		m_adpcm_sound->reset_write(~data & 0x100);
		m_adpcm_sound->write(data & 0xff);

		// the games seem to check for $82 loops, so this should be just barely enough
		m_fake_sound_state = 128;
	}
}

uint16_t mk2_state::dcs_state_r()
{
//  LOGSOUND("%s:Sound status read\n", machine().describe_context());

	return m_dcs->control_r() >> 4;
}

uint16_t mk2_state::dcs_r()
{
	if (!machine().side_effects_disabled())
		LOGSOUND("%08X:Sound data read\n", m_maincpu->pc());

	return m_dcs->data_r() & 0xff;
}

void mk2_state::dcs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// check for out-of-bounds accesses
	if (!offset)
	{
		LOGSOUND("%08X:Unexpected write to sound (lo) = %04X\n", m_maincpu->pc(), data);
		return;
	}

	// call through based on the sound type
	if (ACCESSING_BITS_0_7 && ACCESSING_BITS_8_15)
	{
		LOGSOUND("%08X:Sound write = %04X\n", m_maincpu->pc(), data);
		m_dcs->reset_w(data & 0x100);
		m_dcs->data_w(data & 0xff);
	}
}
