// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek
/*

    Galaxy Game

    Driver by Mariusz Wojcieszek

    First commercial video game.

*/

#include "emu.h"
#include "cpu/t11/t11.h"
#include "emupal.h"
#include "screen.h"


namespace {

#define MAX_POINTS 2048

class galaxygame_state : public driver_device
{
public:
	galaxygame_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette")
	{ }

	void galaxygame(machine_config &config);

	void init_galaxygame();

private:
	uint16_t m_clk = 0;

	uint16_t m_x = 0;
	uint16_t m_y = 0;

	int16_t m_mq = 0;
	int16_t m_ac = 0;

	struct
	{
		uint16_t x = 0;
		uint16_t y = 0;
	} m_point_work_list[MAX_POINTS], m_point_display_list[MAX_POINTS];

	int m_point_work_list_index = 0;
	int m_point_display_list_index = 0;
	int m_interrupt = 0;
	uint16_t ke_r(offs_t offset);
	void ke_w(offs_t offset, uint16_t data);
	uint16_t x_r();
	void x_w(uint16_t data);
	uint16_t y_r();
	void y_w(uint16_t data);
	void clk_w(uint16_t data);
	virtual void machine_reset() override ATTR_COLD;
	uint32_t screen_update_galaxygame(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(galaxygame_irq);
	uint8_t galaxygame_irq_callback(offs_t offset);
	required_device<t11_device> m_maincpu;
	required_device<palette_device> m_palette;
	void galaxygame_map(address_map &map) ATTR_COLD;
};

/*************************************
 *
 *  KE11 Extended Arithmetic Element
 *
 *************************************/

uint16_t galaxygame_state::ke_r(offs_t offset)
{
	uint16_t ret;

	switch( offset )
	{
		case 0: // DIV
			ret = 0;
			break;
		case 1: // AC
			ret = m_ac;
			break;
		case 2: // MQ
			ret = m_mq;
			break;
		case 7: // ASH
			ret = 0;
			break;
		default:
			ret = 0;
			logerror("Unhandled KE read (%d)\n", offset );
			break;
	}
	return ret;
}

void galaxygame_state::ke_w(offs_t offset, uint16_t data)
{
	switch( offset )
	{
		case 0: // DIV
			{
				if ( data != 0 )
				{
					int32_t dividend = (int32_t)((uint32_t)((uint16_t)m_ac << 16) | (uint16_t)(m_mq));
					m_mq = dividend / (int16_t)data;
					m_ac = dividend % (int16_t)data;
				}
				else
				{
					m_mq = 0;
					m_ac = 0;
				}
			}
			break;
		case 1: // AC
			m_ac = (int16_t)data;
			break;
		case 2: // MQ
			m_mq = (int16_t)data;
			if (m_mq < 0)
			{
				m_ac = -1;
			}
			else
			{
				m_ac = 0;
			}
			break;
		case 3: // X
			{
				int32_t mulres = (int32_t)m_mq*(int32_t)(int16_t)data;
				m_ac = mulres >> 16;
				m_mq = mulres & 0xffff;
			}
			break;
		case 6: // LSH
			{
				data &= 63;
				int32_t val = (int32_t)((uint32_t)((uint16_t)m_ac << 16) | (uint16_t)(m_mq));
				if ( data < 32 )
				{
					val = val << data;
				}
				else
				{
					val = val >> (64 - data);
				}
				m_mq = val & 0xffff;
				m_ac = (val >> 16) & 0xffff;
			}
			break;
		case 7: // ASH
			{
				data &= 63;
				int32_t val = (int32_t)((uint32_t)((uint16_t)m_ac << 16) | (uint16_t)(m_mq));
				if ( data < 32 )
				{
					val = val << data;
				}
				else
				{
					val = val >> (64 - data);
				}
				m_mq = val & 0xffff;
				m_ac = (val >> 16) & 0xffff;
			}
			break;
		default:
			logerror("Unhandled KE write (%d)\n", offset);
			break;
	}

}

/*************************************
 *
 *  Video
 *
 *************************************/

uint32_t galaxygame_state::screen_update_galaxygame(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);
	for (int i = 0; i < m_point_display_list_index; i++ )
	{
		bitmap.pix(m_point_display_list[i].x >> 7, m_point_display_list[i].y >> 7) = 1;
	}
	return 0;
}

uint16_t galaxygame_state::x_r()
{
	return m_x;
}

void galaxygame_state::x_w(uint16_t data)
{
	m_x = data;
}

uint16_t galaxygame_state::y_r()
{
	return m_y;
}

void galaxygame_state::y_w(uint16_t data)
{
	m_y = data;
	if ( data == 0x0101 )
	{
		// send points list to display device
		// seems to happen on first 0x0101 write after interrupt
		if ( m_interrupt )
		{
			for ( int i = 0; i < m_point_work_list_index ; i++ )
			{
				m_point_display_list[i].x = m_point_work_list[i].x;
				m_point_display_list[i].y = m_point_work_list[i].y;
			}
			m_point_display_list_index = m_point_work_list_index;
			m_point_work_list_index = 0;
			m_interrupt = 0;
		}
	}
	else
	{
		if ( m_point_work_list_index >= MAX_POINTS )
		{
			logerror("Work list overflow\n");
		}
		else
		{
			m_point_work_list[m_point_work_list_index].x = m_x;
			m_point_work_list[m_point_work_list_index].y = m_y;
			m_point_work_list_index++;
		}
	}
}

/*************************************
 *
 *  Inputs
 *
 *************************************/

static INPUT_PORTS_START( galaxygame )
	PORT_START("COINAC")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_COIN3 ) // 25 cents, left
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_COIN1 ) // 10 cents, left
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_COIN4 ) // 25 cents, right
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_COIN2 ) // 10 cents, right

	PORT_START("SR")
	PORT_DIPNAME( 0x8000, 0x0000, "Gravity" )
	PORT_DIPSETTING(      0x0000, "Positive Gravity" )
	PORT_DIPSETTING(      0x8000, "Negative Gravity" )
	PORT_DIPNAME( 0x4000, 0x0000, "Sun" )
	PORT_DIPSETTING(      0x0000, "Sun (& Gravity)" )
	PORT_DIPSETTING(      0x4000, "No Sun (& No Gravity)" )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) // fire
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) // hyper
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) // thrust
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)

	PORT_DIPNAME( 0x0080, 0x0000, "Speed" )
	PORT_DIPSETTING(      0x0000, "Slow Speed" )
	PORT_DIPSETTING(      0x0080, "Fast Speed" )
	PORT_DIPNAME( 0x0040, 0x0000, DEF_STR( Players ) )
	PORT_DIPSETTING(      0x0000, "Two Players" )
	PORT_DIPSETTING(      0x0040, "One Player" )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
INPUT_PORTS_END

/*************************************
 *
 *  Machine
 *
 *************************************/

void galaxygame_state::clk_w(uint16_t data)
{
	m_clk = data;
}

void galaxygame_state::galaxygame_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0xfec0, 0xfecf).rw(FUNC(galaxygame_state::ke_r), FUNC(galaxygame_state::ke_w));
	map(0xff52, 0xff53).rw(FUNC(galaxygame_state::y_r), FUNC(galaxygame_state::y_w)); // 177522 Y
	map(0xff54, 0xff55).portr("COINAC"); // 177524 COINAC
	map(0xff5a, 0xff5b).rw(FUNC(galaxygame_state::x_r), FUNC(galaxygame_state::x_w)); // 177532 X
	map(0xff5c, 0xff5d).portr("SR");     // 177534 SR
	map(0xff66, 0xff67).w(FUNC(galaxygame_state::clk_w));        // 177546 KW11 line frequency clock
}


uint8_t galaxygame_state::galaxygame_irq_callback(offs_t offset)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return 0x40;
}

INTERRUPT_GEN_MEMBER(galaxygame_state::galaxygame_irq)
{
	if ( m_clk & 0x40 )
	{
		device.execute().set_input_line(0, ASSERT_LINE);
		m_interrupt = 1;
	}
}

void galaxygame_state::machine_reset()
{
	m_maincpu->set_input_line(t11_device::VEC_LINE, ASSERT_LINE);

	m_clk = 0x00;
	m_point_work_list_index = 0;
	m_point_display_list_index = 0;
	m_interrupt = 0;
}

void galaxygame_state::galaxygame(machine_config &config)
{
	T11(config, m_maincpu, 3000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &galaxygame_state::galaxygame_map);
	m_maincpu->set_initial_mode(5 << 13);
	m_maincpu->in_iack().set(FUNC(galaxygame_state::galaxygame_irq_callback));
	m_maincpu->set_periodic_int(FUNC(galaxygame_state::galaxygame_irq), attotime::from_hz(60));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 512);
	screen.set_visarea(0, 511, 0, 511);
	screen.set_screen_update(FUNC(galaxygame_state::screen_update_galaxygame));
	screen.set_palette("palette");

	PALETTE(config, m_palette, palette_device::MONOCHROME);
}

ROM_START(galgame)
	// Original Galaxy Game listing, the one used in the 2nd hardware revision (blue dual cabinet)
	// PALX11 V413R 19-NOV-71 8:46
	ROM_REGION( 0x20000, "code", ROMREGION_ERASE00 )
	ROM_LOAD( "sw97.lst", 0x00000, 0x1f062, CRC(838018a5) SHA1(e3c47c5cf78299650b031ec49fde7d9e4024a759) )
ROM_END

/*************************************
 *
 *  Code loading
 *
 *************************************/

static uint8_t read_uint16(uint16_t *pval, int pos, const uint8_t* line, int linelen)
{
	*pval = 0;
	if (linelen < (pos + 6))
	{
		return 0;
	}

	for (int i = 0; i < 6; i++)
	{
		*pval <<= 3;
		*pval |= line[pos + i] - 0x30;
	}
	return 1;
}

static uint8_t read_uint8(uint8_t *pval, int pos, const uint8_t* line, int linelen)
{
	*pval = 0;
	if (linelen < (pos + 3))
	{
		return 0;
	}

	for (int i = 0; i < 3; i++)
	{
		*pval <<= 3;
		*pval |= line[pos + i] - 0x30;
	}
	return 1;
}

void galaxygame_state::init_galaxygame()
{
	address_space &main = m_maincpu->space(AS_PROGRAM);
	uint8_t *code = memregion("code")->base();

	int filepos = 0;

	//load lst file
	while (code[filepos] != 0)
	{
		uint8_t line[256];
		int linepos = 0;
		while (code[filepos] != 0x0d)
		{
			line[linepos++] = code[filepos++];
		}
		line[linepos] = 0;
		filepos += 2;
		int linelen = linepos;

		if (linelen == 0)
		{
			continue;
		}

		uint16_t address;
		if ((line[8] != ' ') && read_uint16(&address, 7, line, linelen))
		{
			if ((linelen >= 15+6) && (line[15] != ' '))
			{
				uint16_t val;
				read_uint16(&val, 15, line, linelen);
				main.write_word(address, val, 0xffff);
				address += 2;

				if ((linelen >= 22+6) && (line[22] != ' '))
				{
					read_uint16(&val, 22, line, linelen);
					main.write_word(address, val, 0xffff);
					address += 2;
				}

				if ((linelen >= 29+6) && (line[29] != ' '))
				{
					read_uint16(&val, 29, line, linelen);
					main.write_word(address, val, 0xffff);
					address += 2;
				}

			}
			else
			{
				if ((linelen >= 18+3) && (line[18] != ' '))
				{
					uint8_t val8;
					read_uint8(&val8, 18, line, linelen);
					main.write_byte(address, val8);
					address += 1;
				}
			}

		}
	}

	// set startup code
	main.write_word(0, 012700); /* MOV #0, R0 */
	main.write_word(2, 0);
	main.write_word(4, 0x8d00); /* MTPS R0 */
	main.write_word(6, 000167); /* JMP 0500*/
	main.write_word(8, 000500 - 10);
}

} // anonymous namespace


GAME(1971, galgame, 0, galaxygame, galaxygame, galaxygame_state, init_galaxygame, ROT270, "Computer Recreations, Inc", "Galaxy Game", MACHINE_NO_SOUND_HW )
