// license:BSD-3-Clause
// copyright-holders:eziochiu
/***********************************************************************
 PGM IGS027A ARM protection - kovgsyx / kovzscs bootleg variants

 These bootlegs replace the original IGS027A with an NXP LPC2132
 ARM7TDMI-S microcontroller. Communication between the 68k and ARM
 uses an I2C-based handshake through the LPC2132's VIC and I2C
 peripherals.
 ***********************************************************************/

#include "emu.h"
#include "pgmprot_igs027a_type1_kovgsyx.h"

void pgm_arm_type1_kovgsyx_state::machine_reset()
{
	pgm_arm_type1_state::machine_reset();
	m_kovgsyx_i2c1conset = 0;
	m_kovgsyx_i2c1dat = 0;
	m_kovgsyx_i2c1sclh = 0;
	m_kovgsyx_i2c1_status = 0x08;
	m_kovgsyx_handshake_done = 0;
}

void pgm_arm_type1_kovgsyx_state::pgm_decode_kovgsyx_samples()
{
	u8 *src = (u8 *)(memregion("ics")->base() + 0x800000);

	for (int i = 0; i < 0x400000; i += 2)
	{
		src[i + 0x000001] = src[i + 0x800000];
		src[i + 0x400001] = src[i + 0xc00000];
	}
}

void pgm_arm_type1_kovgsyx_state::pgm_decode_kovgsyx_program()
{
	u16 *src = (u16 *)(memregion("maincpu")->base() + 0x100000);
	std::vector<u16> dst(0x400000);

	for (int i = 0; i < 0x400000 / 2; i++)
	{
		int j = bitswap<24>((i + 0x80000) & 0x1fffff, 23, 22, 21, 20, 19,  18, 16, 1, 3, 5, 7, 9, 11, 13, 15, 17, 14, 12, 10, 8, 6, 4, 0, 2);

		dst[i] = bitswap<16>(src[j], 15, 0, 10, 12, 3, 4, 11, 5, 2, 13, 9, 6, 1, 14, 8, 7);
	}

	memcpy( src, &dst[0], 0x400000 );
}

void pgm_arm_type1_kovgsyx_state::pgm_decode_kovzscs_program()
{
	u16 *src = (u16 *)(memregion("maincpu")->base() + 0x100000);
	std::vector<u16> dst(0x400000);

	for (int i = 0; i < 0x400000 / 2; i++)
	{
		int j = bitswap<24>((i + 0x80000) & 0x1fffff, 23, 22, 21, 20, 19, 2, 1, 18, 17, 16, 15, 14, 13, 12, 11, 10, 0, 3, 4, 5, 6, 7, 8, 9);
		dst[i] = src[j];
	}

	memcpy( src, &dst[0], 0x400000 );
}


void pgm_arm_type1_kovgsyx_state::kovgsyx_asic27a_write_word(offs_t offset, u16 data)
{
	// defer the actual write via synchronize callback so both CPUs are caught up first
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(pgm_arm_type1_kovgsyx_state::kovgsyx_asic27a_write_sync), this), (offset << 16) | data);
}

void pgm_arm_type1_kovgsyx_state::kovgsyx_asic27a_write_sync(s32 param)
{
	int offset = param >> 16;
	u16 data = param & 0xffff;
	if (offset & 1)
	{
		m_kovgsyx_highlatch_68k_w = data;
		m_lpc2132_vic->set_irq(16, ASSERT_LINE);
		m_kovgsyx_lowlatch_arm_w = 0;
	}
	else
	{
		m_kovgsyx_lowlatch_68k_w = data;
		m_kovgsyx_highlatch_arm_w = 0;
	}
}

u16 pgm_arm_type1_kovgsyx_state::kovgsyx_asic27a_read_word(offs_t offset)
{
	if (offset & 1)
		return m_kovgsyx_highlatch_arm_w;
	else
		return m_kovgsyx_lowlatch_arm_w;
}

u32 pgm_arm_type1_kovgsyx_state::kovgsyx_lpc2132_read_long(offs_t offset)
{
	if ((offset & 0x3FFFC0) == 0x7F000)
	{
		switch (offset & 0x3F)
		{
			case 0x20:
				return m_kovgsyx_pll_enabled;

			case 0x21:
				return m_kovgsyx_pll_config;

			case 0x22:
			{
				u32 status = 0;

				if (m_kovgsyx_pll_enabled)
				{
					m_kovgsyx_pll_lock_timer++;

					if (m_kovgsyx_pll_lock_timer > 10)
					{
						status |= 0x0400;
					}
				}

				status |= (m_kovgsyx_pll_enabled & 0x03) << 8;
				status |= (m_kovgsyx_pll_config & 0x1F);

				return status;
			}

			case 0x23:
				return 0;
		}
	}

	if ((offset & 0x3FFFC0) == 0xA000)
	{
		switch (offset & 0x3F)
		{
			case 0x00:
				return m_kovgsyx_i2c1conset;

			case 0x01:
				return m_kovgsyx_i2c1_status;

			case 0x02:
				return m_kovgsyx_i2c1dat;

			case 0x04:
				return m_kovgsyx_i2c1sclh;

			default:
				return 0;
		}
	}

	return 0;
}

void pgm_arm_type1_kovgsyx_state::kovgsyx_lpc2132_write_long(offs_t offset, u32 data, u32 mem_mask)
{
	if ((offset & 0x3FFFC0) == 0x7F000 || (offset & 0x3FFFC0) == 0x7F040)
	{
		switch (offset & 0x7F)
		{
			case 0x20:
			{
				m_kovgsyx_pll_enabled = data & 0x03;
				if (m_kovgsyx_pll_enabled)
					m_kovgsyx_pll_lock_timer = 0;
				break;
			}
			case 0x21:
				m_kovgsyx_pll_config = data & 0xFF;
				break;
			case 0x23:
				break;
			case 0x50:
				if (data & 0x04)
				{
					m_lpc2132_vic->set_irq(16, CLEAR_LINE);
				}
				break;
		}
	}

	if ((offset & 0x3FFFC0) == 0xA000)
	{
		switch (offset & 0x3F)
		{
			case 0x00:
			{
				m_kovgsyx_i2c1conset = data;

				if (m_kovgsyx_handshake_done && (data & 0x00200000) && !(data & 0x00400000))
				{
					if (data & 0x04000000)
					{
						m_kovgsyx_highlatch_arm_w = (m_kovgsyx_i2c1sclh >> 16) & 0xffff;
						m_lpc2132_vic->set_irq(16, CLEAR_LINE);
					}
					else
					{
						m_kovgsyx_lowlatch_arm_w = (m_kovgsyx_i2c1sclh >> 16) & 0xffff;
					}
				}

				if (m_kovgsyx_handshake_done && (data & 0x00400000) && !(data & 0x00200000))
				{
					if (data & 0x04000000)
					{
						m_kovgsyx_i2c1sclh = m_kovgsyx_highlatch_68k_w << 16;
					}
					else
					{
						m_kovgsyx_i2c1sclh = m_kovgsyx_lowlatch_68k_w << 16;
					}
				}

				if (data & 0x400000) m_kovgsyx_i2c1_status = 0x08;
				if (data & 0x200000) m_kovgsyx_i2c1_status = 0x10;
				if ((data & 0x600000) == 0x600000) m_kovgsyx_i2c1_status = 0x28;

				break;
			}
			case 0x04:
			{
				m_kovgsyx_i2c1sclh = data;
				m_kovgsyx_i2c1_status = 0x28;
				break;
			}
			case 0x02:
			{
				m_kovgsyx_i2c1dat = data;
				break;
			}
			case 0x06:
				break;
		}
	}
}

void pgm_arm_type1_kovgsyx_state::kovgsyx_handshake_callback(u32 data)
{
	if (data != 0 && !m_kovgsyx_handshake_done)
	{
		m_kovgsyx_handshake_done = 1;
	}
}

void pgm_arm_type1_kovgsyx_state::kovgsyx_common_init()
{
	pgm_decode_kovlsqh2_tiles();

	pgm_decode_kovlsqh2_sprites(memregion("igs023:sprcol")->base() + 0x0000000);
	pgm_decode_kovlsqh2_sprites(memregion("igs023:sprcol")->base() + 0x0800000);
	pgm_decode_kovlsqh2_sprites(memregion("igs023:sprcol")->base() + 0x1000000);
	pgm_decode_kovlsqh2_sprites(memregion("igs023:sprcol")->base() + 0x1800000);
	pgm_decode_kovlsqh2_sprites(memregion("igs023:sprcol")->base() + 0x2000000);
	pgm_decode_kovlsqh2_sprites(memregion("igs023:sprcol")->base() + 0x2800000);
	pgm_decode_kovlsqh2_sprites(memregion("igs023:sprmask")->base() + 0x0000000);
	pgm_decode_kovlsqh2_sprites(memregion("igs023:sprmask")->base() + 0x0800000);

	pgm_decode_kovgsyx_samples();

	pgm_basic_init();

	m_kovgsyx_highlatch_arm_w = 0;
	m_kovgsyx_lowlatch_arm_w = 0;
	m_kovgsyx_highlatch_68k_w = 0;
	m_kovgsyx_lowlatch_68k_w = 0;
	m_kovgsyx_pll_lock_timer = 0;
	m_kovgsyx_pll_enabled = 0;
	m_kovgsyx_pll_config = 0;
	m_kovgsyx_i2c1sclh = 0;
	m_kovgsyx_i2c1conset = 0;
	m_kovgsyx_i2c1dat = 0;
	m_kovgsyx_i2c1_status = 0x08;
	m_kovgsyx_handshake_done = 0;

	save_item(NAME(m_kovgsyx_highlatch_arm_w));
	save_item(NAME(m_kovgsyx_lowlatch_arm_w));
	save_item(NAME(m_kovgsyx_highlatch_68k_w));
	save_item(NAME(m_kovgsyx_lowlatch_68k_w));
	save_item(NAME(m_kovgsyx_pll_lock_timer));
	save_item(NAME(m_kovgsyx_pll_enabled));
	save_item(NAME(m_kovgsyx_pll_config));
	save_item(NAME(m_kovgsyx_i2c1sclh));
	save_item(NAME(m_kovgsyx_i2c1conset));
	save_item(NAME(m_kovgsyx_i2c1dat));
	save_item(NAME(m_kovgsyx_i2c1_status));
	save_item(NAME(m_kovgsyx_handshake_done));
}

void pgm_arm_type1_kovgsyx_state::init_kovgsyx()
{
	pgm_decode_kovgsyx_program();
	kovgsyx_common_init();
}

void pgm_arm_type1_kovgsyx_state::init_kovzscs()
{
	pgm_decode_kovzscs_program();
	kovgsyx_common_init();
}


void pgm_arm_type1_kovgsyx_state::kovgsyx_arm7_map(address_map &map)
{
	map(0x00000000, 0x0000ffff).rom();
	map(0x40000000, 0x40003fff).ram(); // internal ram for asic
	map(0xe0000000, 0xe01fffff).rw(FUNC(pgm_arm_type1_kovgsyx_state::kovgsyx_lpc2132_read_long), FUNC(pgm_arm_type1_kovgsyx_state::kovgsyx_lpc2132_write_long));
	map(0xfffff000, 0xffffffff).m(m_lpc2132_vic, FUNC(lpc2132_vic_device::regs_map));
}

void pgm_arm_type1_kovgsyx_state::kovgsyx_map(address_map &map)
{
	pgm_mem(map);
	map(0x100000, 0x4fffff).bankrw("bank1");
	map(0x080000, 0x09ffff).rom().region("maincpu", 0x000000);
	map(0x500000, 0x5fffff).rom().region("maincpu", 0x400000);
	map(0x600000, 0x6fffff).rom().region("maincpu", 0x400000);
	map(0xd00000, 0xd003ff).ram();
	map(0xf00000, 0xffffff).rom().region("maincpu", 0x400000);
	map(0xd80000, 0xd80005).rw(FUNC(pgm_arm_type1_kovgsyx_state::kovgsyx_asic27a_read_word), FUNC(pgm_arm_type1_kovgsyx_state::kovgsyx_asic27a_write_word));
}

void pgm_arm_type1_kovgsyx_state::pgm_arm_type1_kovgsyx(machine_config &config)
{
	pgmbase(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &pgm_arm_type1_kovgsyx_state::kovgsyx_map);

	ARM7(config, m_prot, 60000000);  // 60MHz
	m_prot->set_addrmap(AS_PROGRAM, &pgm_arm_type1_kovgsyx_state::kovgsyx_arm7_map);

	LPC2132_VIC(config, m_lpc2132_vic);
	m_lpc2132_vic->irq_callback().set_inputline(m_prot, arm7_cpu_device::ARM7_IRQ_LINE);
	m_lpc2132_vic->def_vect_addr_callback().set(FUNC(pgm_arm_type1_kovgsyx_state::kovgsyx_handshake_callback));
}
