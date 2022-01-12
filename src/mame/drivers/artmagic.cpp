// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Nicola Salmoria
/***************************************************************************

    Art & Magic hardware

    driver by Aaron Giles and Nicola Salmoria

    Games supported:
        * Cheese Chase
        * Ultimate Tennis
        * Stone Ball
        * Shooting Star (not emulated)

    Known bugs:
        * measured against a real PCB, the games run slightly too fast
          in spite of accurately measured VBLANK timings

    DIP locations verified for:
        * ultennis (manual+test mode)
        * cheesech (test mode)
        * stonebal (test mode)
        * stoneba2 (test mode)

***************************************************************************/

#include "emu.h"
#include "includes/artmagic.h"

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/tms34010/tms34010.h"
#include "machine/eeprompar.h"
#include "machine/mc68681.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "screen.h"
#include "speaker.h"


#define MASTER_CLOCK_40MHz      (XTAL(40'000'000))
#define MASTER_CLOCK_25MHz      (XTAL(25'000'000))


/*************************************
 *
 *  Interrupts
 *
 *************************************/

void artmagic_state::update_irq_state()
{
	m_maincpu->set_input_line(4, m_tms_irq  ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(5, m_hack_irq ? ASSERT_LINE : CLEAR_LINE);
}


WRITE_LINE_MEMBER(artmagic_state::m68k_gen_int)
{
	m_tms_irq = state;
	update_irq_state();
}



/*************************************
 *
 *  Machine init
 *
 *************************************/

void artmagic_state::machine_start()
{
	m_irq_off_timer = timer_alloc(TIMER_IRQ_OFF);

	save_item(NAME(m_tms_irq));
	save_item(NAME(m_hack_irq));
	save_item(NAME(m_prot_input_index));
	save_item(NAME(m_prot_output_index));
	save_item(NAME(m_prot_output_bit));
	save_item(NAME(m_prot_bit_index));
	save_item(NAME(m_prot_save));
	save_item(NAME(m_prot_input));
	save_item(NAME(m_prot_output));
}

void artmagic_state::machine_reset()
{
	m_tms_irq = m_hack_irq = 0;
	update_irq_state();
}



/*************************************
 *
 *  Misc control memory accesses
 *
 *************************************/

void artmagic_state::control_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_control[offset]);

	/* OKI banking here */
	if (offset == 0 && !BIT(data, 0))
	{
		m_oki->set_rom_bank(BIT(data, 4));
	}

	logerror("%06X:control_w(%d) = %04X\n", m_maincpu->pc(), offset, data);
}



/*************************************
 *
 *  Ultimate Tennis protection workarounds
 *
 *************************************/

void artmagic_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_IRQ_OFF:
		m_hack_irq = 0;
		update_irq_state();
		break;
	default:
		throw emu_fatalerror("Unknown id in artmagic_state::device_timer");
	}
}


uint16_t artmagic_state::ultennis_hack_r()
{
	/* IRQ5 points to: jsr (a5); rte */
	uint32_t pc = m_maincpu->pc();
	if (pc == 0x18c2 || pc == 0x18e4)
	{
		m_hack_irq = 1;
		update_irq_state();
		m_irq_off_timer->adjust(attotime::from_usec(1));
	}
	return ioport("300000")->read();
}



/*************************************
 *
 *  Game-specific protection
 *
 *************************************/

void artmagic_state::ultennis_protection()
{
	/* check the command byte */
	switch (m_prot_input[0])
	{
		case 0x00:  /* reset */
			m_prot_input_index = m_prot_output_index = 0;
			m_prot_output[0] = machine().rand();
			break;

		case 0x01:  /* 01 aaaa bbbb cccc dddd (xxxx) */
			if (m_prot_input_index == 9)
			{
				uint16_t a = m_prot_input[1] | (m_prot_input[2] << 8);
				uint16_t b = m_prot_input[3] | (m_prot_input[4] << 8);
				uint16_t c = m_prot_input[5] | (m_prot_input[6] << 8);
				uint16_t d = m_prot_input[7] | (m_prot_input[8] << 8);
				uint16_t x = a - b;
				if ((int16_t)x >= 0)
					x = (x * c) >> 16;
				else
					x = -(((uint16_t)-x * c) >> 16);
				x += d;
				m_prot_output[0] = x;
				m_prot_output[1] = x >> 8;
				m_prot_output_index = 0;
			}
			else if (m_prot_input_index >= 11)
				m_prot_input_index = 0;
			break;

		case 0x02:  /* 02 aaaa bbbb cccc (xxxxxxxx) */
			/*
			    Ultimate Tennis -- actual values from a board:

			        hex                             decimal
			        0041 0084 00c8 -> 00044142       65 132 200 -> 278850 = 65*65*66
			        001e 0084 00fc -> 0000e808       30 132 252 ->  59400 = 30*30*66
			        0030 007c 005f -> 00022e00       48 124  95 -> 142848 = 48*48*62
			        0024 00dd 0061 -> 00022ce0       36 221  97 -> 142560 = 36*36*110
			        0025 0096 005b -> 00019113       37 150  91 -> 102675 = 37*37*75
			        0044 00c9 004c -> 00070e40       68 201  76 -> 462400 = 68*68*100

			    question is: what is the 3rd value doing there?
			*/
			if (m_prot_input_index == 7)
			{
				uint16_t a = (int16_t)(m_prot_input[1] | (m_prot_input[2] << 8));
				uint16_t b = (int16_t)(m_prot_input[3] | (m_prot_input[4] << 8));
				/*uint16_t c = (int16_t)(m_prot_input[5] | (m_prot_input[6] << 8));*/
				uint32_t x = a * a * (b/2);
				m_prot_output[0] = x;
				m_prot_output[1] = x >> 8;
				m_prot_output[2] = x >> 16;
				m_prot_output[3] = x >> 24;
				m_prot_output_index = 0;
			}
			else if (m_prot_input_index >= 11)
				m_prot_input_index = 0;
			break;

		case 0x03:  /* 03 (xxxx) */
			if (m_prot_input_index == 1)
			{
				uint16_t x = m_prot_save;
				m_prot_output[0] = x;
				m_prot_output[1] = x >> 8;
				m_prot_output_index = 0;
			}
			else if (m_prot_input_index >= 3)
				m_prot_input_index = 0;
			break;

		case 0x04:  /* 04 aaaa */
			if (m_prot_input_index == 3)
			{
				uint16_t a = m_prot_input[1] | (m_prot_input[2] << 8);
				m_prot_save = a;
				m_prot_input_index = m_prot_output_index = 0;
			}
			break;

		default:
			logerror("protection command %02X: unknown\n", m_prot_input[0]);
			m_prot_input_index = m_prot_output_index = 0;
			break;
	}
}


void artmagic_state::cheesech_protection()
{
	/* check the command byte */
	switch (m_prot_input[0])
	{
		case 0x00:  /* reset */
			m_prot_input_index = m_prot_output_index = 0;
			m_prot_output[0] = machine().rand();
			break;

		case 0x01:  /* 01 aaaa bbbb (xxxx) */
			if (m_prot_input_index == 5)
			{
				uint16_t a = m_prot_input[1] | (m_prot_input[2] << 8);
				uint16_t b = m_prot_input[3] | (m_prot_input[4] << 8);
				uint16_t c = 0x4000;      /* seems to be hard-coded */
				uint16_t d = 0x00a0;      /* seems to be hard-coded */
				uint16_t x = a - b;
				if ((int16_t)x >= 0)
					x = (x * c) >> 16;
				else
					x = -(((uint16_t)-x * c) >> 16);
				x += d;
				m_prot_output[0] = x;
				m_prot_output[1] = x >> 8;
				m_prot_output_index = 0;
			}
			else if (m_prot_input_index >= 7)
				m_prot_input_index = 0;
			break;

		case 0x03:  /* 03 (xxxx) */
			if (m_prot_input_index == 1)
			{
				uint16_t x = m_prot_save;
				m_prot_output[0] = x;
				m_prot_output[1] = x >> 8;
				m_prot_output_index = 0;
			}
			else if (m_prot_input_index >= 3)
				m_prot_input_index = 0;
			break;

		case 0x04:  /* 04 aaaa */
			if (m_prot_input_index == 3)
			{
				uint16_t a = m_prot_input[1] | (m_prot_input[2] << 8);
				m_prot_save = a;
				m_prot_input_index = m_prot_output_index = 0;
			}
			break;

		default:
			logerror("protection command %02X: unknown\n", m_prot_input[0]);
			m_prot_input_index = m_prot_output_index = 0;
			break;
	}
}


void artmagic_state::stonebal_protection()
{
	/* check the command byte */
	switch (m_prot_input[0])
	{
		case 0x01:  /* 01 aaaa bbbb cccc dddd (xxxx) */
			if (m_prot_input_index == 9)
			{
				uint16_t a = m_prot_input[1] | (m_prot_input[2] << 8);
				uint16_t b = m_prot_input[3] | (m_prot_input[4] << 8);
				uint16_t c = m_prot_input[5] | (m_prot_input[6] << 8);
				uint16_t d = m_prot_input[7] | (m_prot_input[8] << 8);
				uint16_t x = a - b;
				if ((int16_t)x >= 0)
					x = (x * d) >> 16;
				else
					x = -(((uint16_t)-x * d) >> 16);
				x += c;
				m_prot_output[0] = x;
				m_prot_output[1] = x >> 8;
				m_prot_output_index = 0;
			}
			else if (m_prot_input_index >= 11)
				m_prot_input_index = 0;
			break;

		case 0x02:  /* 02 aaaa (xx) */
			if (m_prot_input_index == 3)
			{
				/*uint16_t a = m_prot_input[1] | (m_prot_input[2] << 8);*/
				uint8_t x = 0xa5;
				m_prot_output[0] = x;
				m_prot_output_index = 0;
			}
			else if (m_prot_input_index >= 4)
				m_prot_input_index = 0;
			break;

		case 0x03:  /* 03 (xxxx) */
			if (m_prot_input_index == 1)
			{
				uint16_t x = m_prot_save;
				m_prot_output[0] = x;
				m_prot_output[1] = x >> 8;
				m_prot_output_index = 0;
			}
			else if (m_prot_input_index >= 3)
				m_prot_input_index = 0;
			break;

		case 0x04:  /* 04 aaaa */
			if (m_prot_input_index == 3)
			{
				uint16_t a = m_prot_input[1] | (m_prot_input[2] << 8);
				m_prot_save = a;
				m_prot_input_index = m_prot_output_index = 0;
			}
			break;

		default:
			logerror("protection command %02X: unknown\n", m_prot_input[0]);
			m_prot_input_index = m_prot_output_index = 0;
			break;
	}
}


READ_LINE_MEMBER(artmagic_state::prot_r)
{
	return m_prot_output_bit;
}


void artmagic_state::protection_bit_w(offs_t offset, uint16_t data)
{
	/* shift in the new bit based on the offset */
	m_prot_input[m_prot_input_index] <<= 1;
	m_prot_input[m_prot_input_index] |= offset;

	/* clock out the next bit based on the offset */
	m_prot_output_bit = m_prot_output[m_prot_output_index] & 0x01;
	m_prot_output[m_prot_output_index] >>= 1;

	/* are we done with a whole byte? */
	if (++m_prot_bit_index == 8)
	{
		/* add the data and process it */
		m_prot_input_index++;
		m_prot_output_index++;
		m_prot_bit_index = 0;

		/* update the protection state */
		(this->*m_protection_handler)();
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void artmagic_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x220000, 0x23ffff).ram();
	map(0x240000, 0x240fff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);
	map(0x300000, 0x300001).portr("300000");
	map(0x300002, 0x300003).portr("300002");
	map(0x300004, 0x300005).portr("300004");
	map(0x300006, 0x300007).portr("300006");
	map(0x300008, 0x300009).portr("300008");
	map(0x30000a, 0x30000b).portr("30000a");
	map(0x300000, 0x300003).w(FUNC(artmagic_state::control_w)).share("control");
	map(0x300004, 0x300007).w(FUNC(artmagic_state::protection_bit_w));
	map(0x360001, 0x360001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x380000, 0x380007).rw(m_tms, FUNC(tms34010_device::host_r), FUNC(tms34010_device::host_w));
}


void artmagic_state::stonebal_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x27ffff).ram();
	map(0x280000, 0x280fff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);
	map(0x300000, 0x300001).portr("300000");
	map(0x300002, 0x300003).portr("300002");
	map(0x300004, 0x300005).portr("300004");
	map(0x300006, 0x300007).portr("300006");
	map(0x300008, 0x300009).portr("300008");
	map(0x30000a, 0x30000b).portr("30000a");
	map(0x30000c, 0x30000d).portr("30000c");
	map(0x30000e, 0x30000f).portr("30000e");
	map(0x300000, 0x300003).w(FUNC(artmagic_state::control_w)).share("control");
	map(0x300004, 0x300007).w(FUNC(artmagic_state::protection_bit_w));
	map(0x340001, 0x340001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x380000, 0x380007).rw(m_tms, FUNC(tms34010_device::host_r), FUNC(tms34010_device::host_w));
}

// TODO: jumps to undefined area at PC=33a0 -> 230000, presumably protection device provides a code snippet
void artmagic_state::shtstar_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x200000, 0x27ffff).ram();
	map(0x280000, 0x280fff).rw("eeprom", FUNC(eeprom_parallel_28xx_device::read), FUNC(eeprom_parallel_28xx_device::write)).umask16(0x00ff);

	map(0x300000, 0x300001).nopr(); //.portr("300000");
	map(0x300000, 0x300003).w(FUNC(artmagic_state::control_w)).share("control");
	map(0x300004, 0x300007).w(FUNC(artmagic_state::protection_bit_w));
	map(0x340001, 0x340001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x380000, 0x380007).rw(m_tms, FUNC(tms34010_device::host_r), FUNC(tms34010_device::host_w));
	map(0x3c0000, 0x3c001f).rw("mainduart", FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
}


/*************************************
 *
 *  Slave CPU memory handlers
 *
 *************************************/

void artmagic_state::tms_map(address_map &map)
{
	map(0x00000000, 0x001fffff).ram().share("vram0");
	map(0x00400000, 0x005fffff).ram().share("vram1");
	map(0x00800000, 0x0080007f).rw(FUNC(artmagic_state::blitter_r), FUNC(artmagic_state::blitter_w));
	map(0x00c00000, 0x00c000ff).rw(m_tlc34076, FUNC(tlc34076_device::read), FUNC(tlc34076_device::write)).umask16(0x00ff);
	map(0xffe00000, 0xffffffff).ram();
}


void artmagic_state::stonebal_tms_map(address_map &map)
{
	map(0x00000000, 0x001fffff).ram().share("vram0");
	map(0x00400000, 0x005fffff).ram().share("vram1");
	map(0x00800000, 0x0080007f).rw(FUNC(artmagic_state::blitter_r), FUNC(artmagic_state::blitter_w));
	map(0x00c00000, 0x00c000ff).rw(m_tlc34076, FUNC(tlc34076_device::read), FUNC(tlc34076_device::write)).umask16(0x00ff);
	map(0xffc00000, 0xffffffff).ram();
}

/*************************************
 *
 *  Extra CPU memory handlers
 *   (Shooting Star)
 *
 *************************************/

/* see adp.c */
void artmagic_state::shtstar_subcpu_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x800141, 0x800141).w("aysnd", FUNC(ym2149_device::address_w));
	map(0x800143, 0x800143).rw("aysnd", FUNC(ym2149_device::data_r), FUNC(ym2149_device::data_w));
	map(0x800180, 0x80019f).rw("subduart", FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0xffc000, 0xffffff).ram();
}

void artmagic_state::shtstar_guncpu_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
}

void artmagic_state::shtstar_guncpu_io_map(address_map &map)
{
	map(0xc000, 0xcfff).ram();
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( cheesech )
	PORT_START("300000")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("300002")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("300004")
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SWB:8" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x0006, 0x0004, DEF_STR( Language ) )     PORT_DIPLOCATION("SWB:6,7")
	PORT_DIPSETTING(      0x0000, DEF_STR( French ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Italian ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( German ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Lives ))         PORT_DIPLOCATION("SWB:4,5")
	PORT_DIPSETTING(      0x0008, "3" )
	PORT_DIPSETTING(      0x0018, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPSETTING(      0x0010, "6" )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Demo_Sounds ))   PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x00c0, 0x0040, DEF_STR( Difficulty ))    PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(      0x00c0, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("300006")
	PORT_DIPNAME( 0x0007, 0x0007, "Right Coinage" )     PORT_DIPLOCATION("SWA:6,7,8")
	PORT_DIPSETTING(      0x0002, DEF_STR( 6C_1C ))
	PORT_DIPSETTING(      0x0006, DEF_STR( 5C_1C ))
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_4C ))
	PORT_DIPNAME( 0x0038, 0x0038, "Left Coinage"  )     PORT_DIPLOCATION("SWA:3,4,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ))
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play )) PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SWA:1" )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("300008")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN4 )
	PORT_BIT( 0x00f0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("30000a")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(artmagic_state, prot_r)    // protection data
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_CUSTOM )     // protection ready
	PORT_BIT( 0x00fc, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( ultennis )
	PORT_INCLUDE(cheesech)

	PORT_MODIFY("300004")
	PORT_DIPNAME( 0x0001, 0x0001, "Button Layout" )         PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(      0x0001, "Triangular" )
	PORT_DIPSETTING(      0x0000, "Linear" )
	PORT_DIPNAME( 0x0002, 0x0002, "Start Set At" )          PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(      0x0000, "0-0" )
	PORT_DIPSETTING(      0x0002, "4-4" )
	PORT_DIPNAME( 0x0004, 0x0004, "Sets Per Match" )        PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(      0x0004, "1" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPNAME( 0x0018, 0x0008, "Game Duratiob" )         PORT_DIPLOCATION("SWB:4,5")
	PORT_DIPSETTING(      0x0018, "5 Lost Points" )
	PORT_DIPSETTING(      0x0008, "6 Lost Points" )
	PORT_DIPSETTING(      0x0010, "7 Lost Points" )
	PORT_DIPSETTING(      0x0000, "8 Lost Points" )
INPUT_PORTS_END


static INPUT_PORTS_START( stonebal )
	PORT_INCLUDE(cheesech)

	PORT_MODIFY("300004")
	PORT_SERVICE_DIPLOC(  0x0001, IP_ACTIVE_LOW, "SWA:1" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Free_Play ))     PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x001c, 0x001c, "Left Coinage" )          PORT_DIPLOCATION("SWA:3,4,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(      0x0004, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_6C ))
	PORT_DIPNAME( 0x00e0, 0x00e0, "Right Coinage" )         PORT_DIPLOCATION("SWA:6,7,8")
	PORT_DIPSETTING(      0x0040, DEF_STR( 6C_1C ))
	PORT_DIPSETTING(      0x0060, DEF_STR( 5C_1C ))
	PORT_DIPSETTING(      0x0080, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(      0x00a0, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(      0x00c0, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_4C ))

	PORT_MODIFY("300006")
	PORT_DIPNAME( 0x0003, 0x0002, DEF_STR( Difficulty ))    PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(      0x0003, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0004, 0x0000, DEF_STR( Demo_Sounds ))   PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x0038, 0x0038, "Match Time" )            PORT_DIPLOCATION("SWB:4,5,6")
	PORT_DIPSETTING(      0x0030, "60s" )
	PORT_DIPSETTING(      0x0028, "70s" )
	PORT_DIPSETTING(      0x0020, "80s" )
	PORT_DIPSETTING(      0x0018, "90s" )
	PORT_DIPSETTING(      0x0038, "100s" )
	PORT_DIPSETTING(      0x0010, "110s" )
	PORT_DIPSETTING(      0x0008, "120s" )
	PORT_DIPSETTING(      0x0000, "130s" )
	PORT_DIPNAME( 0x0040, 0x0040, "Free Match Time" )       PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0000, "Short" )
	PORT_DIPNAME( 0x0080, 0x0080, "Game Mode" )             PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(      0x0080, "4 Players" )
	PORT_DIPSETTING(      0x0000, "2 Players" )

	PORT_START("30000c")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_START3 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("30000e")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_START4 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( stoneba2 )
	PORT_INCLUDE(stonebal)

	PORT_MODIFY("300006")
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SWB:8" )        /* Listed as "Unused" */
INPUT_PORTS_END


static INPUT_PORTS_START( shtstar )

	PORT_START("3c0000")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("3c0002")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("3c0004")
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SWB:8" )        /* Listed as "Unused" */
	PORT_DIPNAME( 0x0006, 0x0004, DEF_STR( Language ) )     PORT_DIPLOCATION("SWB:6,7")
	PORT_DIPSETTING(      0x0000, DEF_STR( French ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Italian ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( English ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( German ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Lives ))         PORT_DIPLOCATION("SWB:4,5")
	PORT_DIPSETTING(      0x0008, "3" )
	PORT_DIPSETTING(      0x0018, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPSETTING(      0x0010, "6" )
	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Demo_Sounds ))   PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_DIPNAME( 0x00c0, 0x0040, DEF_STR( Difficulty ))    PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(      0x00c0, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Very_Hard ) )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("3c0006")
	PORT_DIPNAME( 0x0007, 0x0007, "Right Coinage" )     PORT_DIPLOCATION("SWA:6,7,8")
	PORT_DIPSETTING(      0x0002, DEF_STR( 6C_1C ))
	PORT_DIPSETTING(      0x0006, DEF_STR( 5C_1C ))
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(      0x0005, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_4C ))
	PORT_DIPNAME( 0x0038, 0x0038, "Left Coinage"  )     PORT_DIPLOCATION("SWA:3,4,5")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ))
	PORT_DIPSETTING(      0x0020, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ))
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Free_Play )) PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ))
	PORT_DIPSETTING(      0x0000, DEF_STR( On ))
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SWA:1" )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("3c0008")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN4 )
	PORT_BIT( 0x00f0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("3c000a")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(artmagic_state, prot_r)    // protection data
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_CUSTOM )     // protection ready
	PORT_BIT( 0x00fc, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void artmagic_state::artmagic(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, MASTER_CLOCK_25MHz/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &artmagic_state::main_map);

	TMS34010(config, m_tms, MASTER_CLOCK_40MHz);
	m_tms->set_addrmap(AS_PROGRAM, &artmagic_state::tms_map);
	m_tms->set_halt_on_reset(true);
	m_tms->set_pixel_clock(MASTER_CLOCK_40MHz/6);
	m_tms->set_pixels_per_clock(1);
	m_tms->set_scanline_rgb32_callback(FUNC(artmagic_state::scanline));
	m_tms->output_int().set(FUNC(artmagic_state::m68k_gen_int));
	m_tms->set_shiftreg_in_callback(FUNC(artmagic_state::to_shiftreg));
	m_tms->set_shiftreg_out_callback(FUNC(artmagic_state::from_shiftreg));

	config.set_maximum_quantum(attotime::from_hz(6000));

	EEPROM_2816(config, "eeprom").write_time(attotime::from_usec(1)); // FIXME: false-readback polling should make this unnecessary

	/* video hardware */
	TLC34076(config, m_tlc34076, tlc34076_device::TLC34076_6_BIT);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(MASTER_CLOCK_40MHz/6, 428, 0, 320, 313, 0, 256);
	screen.set_screen_update("tms", FUNC(tms34010_device::tms340x0_rgb32));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, MASTER_CLOCK_40MHz/3/10, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 0.65);
}


void artmagic_state::cheesech(machine_config &config)
{
	artmagic(config);

	m_oki->reset_routes();
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}


void artmagic_state::stonebal(machine_config &config)
{
	artmagic(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &artmagic_state::stonebal_map);

	m_tms->set_addrmap(AS_PROGRAM, &artmagic_state::stonebal_tms_map);

	m_oki->reset_routes();
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.45);
}

void artmagic_state::shtstar(machine_config &config)
{
	artmagic(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &artmagic_state::shtstar_map);

	MC68681(config, "mainduart", 3686400);

	/* sub cpu*/
	m68000_device &subcpu(M68000(config, "subcpu", MASTER_CLOCK_25MHz/2));
	subcpu.set_addrmap(AS_PROGRAM, &artmagic_state::shtstar_subcpu_map);

	MC68681(config, "subduart", 3686400);

	YM2149(config, "aysnd", 3686400/2).add_route(ALL_OUTPUTS, "mono", 0.10);

	/*gun board cpu*/
	i80c31_device &guncpu(I80C31(config, "guncpu", 6000000));
	guncpu.set_addrmap(AS_PROGRAM, &artmagic_state::shtstar_guncpu_map);
	guncpu.set_addrmap(AS_IO, &artmagic_state::shtstar_guncpu_io_map);
	guncpu.port_in_cb<1>().set_constant(0); // ?
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( cheesech )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for 68000 code */
	ROM_LOAD16_BYTE( "u102",     0x00000, 0x40000, CRC(1d6e07c5) SHA1(8650868cce47f685d22131aa28aad45033cb0a52) )
	ROM_LOAD16_BYTE( "u101",     0x00001, 0x40000, CRC(30ae9f95) SHA1(fede5d271aabb654c1efc077253d81ba23786f22) )

	ROM_REGION16_LE( 0x100000, "gfx", 0 )
	ROM_LOAD16_BYTE( "u134", 0x00000, 0x80000, CRC(354ba4a6) SHA1(68e7df750efb21c716ba8b8ed4ca15a8cdc9141b) )
	ROM_LOAD16_BYTE( "u135", 0x00001, 0x80000, CRC(97348681) SHA1(7e74685041cd5e8fbd45731284cf316dc3ffec60) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "u151", 0x00000, 0x80000, CRC(65d5ebdb) SHA1(0d905b9a60b86e51de3bdcf6eeb059fe29606431) )
ROM_END

/* There is known to exist an Ultimate Tennis with ROMs labeled A&M001C1293 13B, A&M001C1293 12B, and A&M001C1293 14A and are confirmed to be same data as below */
ROM_START( ultennis )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for 68000 code */
	ROM_LOAD16_BYTE( "a+m001b1093_13b_u102.u102", 0x00000, 0x40000, CRC(ec31385e) SHA1(244e78619c549712d5541fb252656afeba639bb7) ) /* labeled  A&M001B1093  13B  U102 */
	ROM_LOAD16_BYTE( "a+m001b1093_12b_u101.u101", 0x00001, 0x40000, CRC(08a7f655) SHA1(b8a4265472360b68bed71d6c175fc54dff088c1d) ) /* labeled  A&M001B1093  12B  U101 */

	ROM_REGION16_LE( 0x200000, "gfx", 0 )
	ROM_LOAD( "a+m-001-01-a.ic133", 0x000000, 0x200000, CRC(29d9204d) SHA1(0b2b77a55b8c2877c2e31b63156505584d4ee1f0) ) /* mask ROM labeled as  A&M-001-01-A  (C)1993 ART & MAGIC */

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "a+m001b1093_14a_u151.u151", 0x00000,  0x40000, CRC(4e19ca89) SHA1(ac7e17631ec653f83c4912df6f458b0e1df88096) ) /* labeled  A&M001B1093  14A  U151 */
ROM_END


ROM_START( ultennisj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for 68000 code */
	ROM_LOAD16_BYTE( "a+m001d0194-13c-u102-japan.u102", 0x00000, 0x40000, CRC(65cee452) SHA1(49259e8faf289d6d80769f6d44e9d61d15e431c6) ) /* labeled  A&M001D0194  13C  U102 */
	ROM_LOAD16_BYTE( "a+m001d0194-12c-u101-japan.u101", 0x00001, 0x40000, CRC(5f4b0ca0) SHA1(57e9ed60cc0e53eeb4e08c4003138d3bdaec3de7) ) /* labeled  A&M001D0194  12C  U101 */

	ROM_REGION16_LE( 0x200000, "gfx", 0 )
	ROM_LOAD( "a+m-001-01-a.ic133", 0x000000, 0x200000, CRC(29d9204d) SHA1(0b2b77a55b8c2877c2e31b63156505584d4ee1f0) ) /* mask ROM labeled as  A&M-001-01-A  (C)1993 ART & MAGIC */

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "a+m001c1293-14a-u151.u151", 0x00000,  0x40000, CRC(4e19ca89) SHA1(ac7e17631ec653f83c4912df6f458b0e1df88096) ) /* labeled  A&M001C1293  14A  U151 */
ROM_END


/*
Stone Ball
Art & Magic, 1994

PCB No: AM007B1094 0454
CPUs  : TMS34010FNL-40, MC68000P12
SND   : OKI M6295
OSC   : 40.000MHz, 32.000MHz, 25.000MHz
DIP SW: 8 position (x2)

RAM   : MCM54260 (x2, 40 pin SOJ)
        TMS44251 (x4, 28 pin ZIP)

OTHER :
        CSI CAT28C16 EEPROM (24 pin DIP)
        ADV476KN80E (28 pin DIP)
        8 PALs
        1 PIC 16C54
        Black Box - inside is....
                                 XILINX XC3030 (x2, 84 Pin PLCC)
                                 3V Battery (Suicidal?)
                                 74HC14 Logic Chip
                                 10 Pin header (probably for re-programming the XC3030's
                                               after it suicides....)


ROMs  :
                             Byte
Filename      Type           C'sum
---------------------------------------------------
u1801.bin     27C4001        344Eh     OKI Samples

u101.bin      27C2001        617Ah  \  Main Program
u102.bin      27C2001        8F04h  /

u1600.bin     32M Mask       1105h  \
u1601.bin     32M Mask       8642h  /  Gfx

*/

ROM_START( stonebal )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for 68000 code */
	ROM_LOAD16_BYTE( "u102",     0x00000, 0x40000, CRC(712feda1) SHA1(c5b385f425786566fa274fe166a7116615a8ce86) ) /* 4 Players kit, v1-20 13/12/1994 */
	ROM_LOAD16_BYTE( "u101",     0x00001, 0x40000, CRC(4f1656a9) SHA1(720717ae4166b3ec50bb572197a8c6c96b284648) )

	ROM_REGION16_LE( 0x400000, "gfx", 0 )
	ROM_LOAD( "u1600.bin", 0x000000, 0x200000, CRC(d2ffe9ff) SHA1(1c5dcbd8208e45458da9db7621f6b8602bca0fae) )
	ROM_LOAD( "u1601.bin", 0x200000, 0x200000, CRC(dbe893f0) SHA1(71a8a022decc0ff7d4c65f7e6e0cbba9e0b5582c) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "sb_snd_9-9-94.u1801", 0x00000, 0x80000, CRC(d98f7378) SHA1(700df7f29c039b96791c2704a67f01a722dc96dc) ) /* labeled  SB snd 9/9/94 */
ROM_END


ROM_START( stonebal2 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for 68000 code */
	ROM_LOAD16_BYTE( "u102.bin", 0x00000, 0x40000, CRC(b3c4f64f) SHA1(6327e9f3cd9deb871a6910cf1f006c8ee143e859) ) /* 2 Players kit, v1-20 7/11/1994 */
	ROM_LOAD16_BYTE( "u101.bin", 0x00001, 0x40000, CRC(fe373f74) SHA1(bafac4bbd1aae4ccc4ae16205309483f1bbdd464) )

	ROM_REGION16_LE( 0x400000, "gfx", 0 )
	ROM_LOAD( "u1600.bin", 0x000000, 0x200000, CRC(d2ffe9ff) SHA1(1c5dcbd8208e45458da9db7621f6b8602bca0fae) )
	ROM_LOAD( "u1601.bin", 0x200000, 0x200000, CRC(dbe893f0) SHA1(71a8a022decc0ff7d4c65f7e6e0cbba9e0b5582c) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "sb_snd_9-9-94.u1801", 0x00000, 0x80000, CRC(d98f7378) SHA1(700df7f29c039b96791c2704a67f01a722dc96dc) ) /* labeled  SB snd 9/9/94 */
ROM_END


ROM_START( stonebal2o )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for 68000 code */
	ROM_LOAD16_BYTE( "sb_o_2p_24-10.u102", 0x00000, 0x40000, CRC(ab58c6b2) SHA1(6e29646d4b0802733d04e722909c03b87761c759) ) /* 2 Players kit, v1-20 21/10/1994 */
	ROM_LOAD16_BYTE( "sb_e_2p_24-10.u101", 0x00001, 0x40000, CRC(ea967835) SHA1(12655f0dc44981f4a49ed45f271d5eb24f2cc5c6) ) /* Yes the Odd / Even labels are backwards & chips dated 24/10 */

	ROM_REGION16_LE( 0x400000, "gfx", 0 )
	ROM_LOAD( "u1600.bin", 0x000000, 0x200000, CRC(d2ffe9ff) SHA1(1c5dcbd8208e45458da9db7621f6b8602bca0fae) )
	ROM_LOAD( "u1601.bin", 0x200000, 0x200000, CRC(dbe893f0) SHA1(71a8a022decc0ff7d4c65f7e6e0cbba9e0b5582c) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "sb_snd_9-9-94.u1801", 0x00000, 0x80000, CRC(d98f7378) SHA1(700df7f29c039b96791c2704a67f01a722dc96dc) ) /* labeled  SB snd 9/9/94 */
ROM_END

/*

Shooting Star:

- Small pcb "gewehr controller" (gun controller) adp 1994
    romless MCU 80c31BH-3 16P philips
    27c256 eprom "2207 7b42c5"
    LC36648L-10
    8 connectors
    3X led
    osc 12.0000M

- common adp cpu board adp 1994.12
    MC68ec0000FN8
    2x 27c1001 eproms
    2x mk48t08b-15 timekeeper RAM

- common adp i/o board (see adp.c ) with MC68681 and YM2149F

- lamp board with triacs

- a couple of tiny boards with some logic parts

- Art & Magic jamma pcb,  11 connectors (jamma not connected)   "am005c0494 0310"
    MC68000p12
    TMS34010fnl-40
    fpga actel a1020a  pl84c  16b.u110
    cpld xilinx xc7236a 0 25  15b.u111
    MC68681p
    ramdac adv476kn80e 03-56 os
    Oki M6295
    25.000mhz
    40.000mhz

    13 pals/gals (not dumped) labelled:
        a&m005c0494 06a u126
        a&m005c0494 03a u125
        a&m005c0494 02a u307
        a&m005c0494 18a u306
        a&m005c0494 17a u305
        a&m005c0494 05a u206
        a&m005c0494 01a u205
        a&m005c0494 04a u601
        a&m005c0494 08a u916
        a&m005c0494 07a u705
        a&m005c0494 10a u917
        a&m005c0494 09a u903
        a&m005c0494 11a u112

    27c2001 near oki  "a&m005c0494 12a"
    2x 27c010 68k labelled u101 and u102
    2x 27c040 "a&m005c0494 13a"  and "a&m005c0494 14a"


Shooting Star
Nova (Art & Magic ?), 1994

PCB No: AM005C0494 0310
CPUs  : TMS34010FNL-40, MC68000P12
SND   : OKI M6295
OSC   : 40.000MHz, 25.000MHz


*/


ROM_START( shtstar )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rom.u102", 0x00000, 0x20000, CRC(cce9877e) SHA1(3e2b3b29d5dd73bfe0c7faf84309b50adbcded3b) )
	ROM_LOAD16_BYTE( "rom.u101", 0x00001, 0x20000, CRC(3a330d9d) SHA1(0f3cd75e9e5483e3cf51f0c4eb4f15b6c3b33b67) )

	ROM_REGION( 0x40000, "subcpu", 0 )
	ROM_LOAD16_BYTE( "shooting_star_f1_i.u2",  0x00000, 0x20000, CRC(2780d8d6) SHA1(a8db3a9771f6918eb8bb3b94db82ca8ada2aae7d) )
	ROM_LOAD16_BYTE( "shooting_star_f1_ii.u7", 0x00001, 0x20000, CRC(0d127d9c) SHA1(e9d209901e55a743a4916c850083caa23c5ebb39) )

	/* 80c31 MCU */
	ROM_REGION( 0x10000, "guncpu", 0 )
	ROM_LOAD( "2207_7b42c5.u6", 0x00000, 0x8000, CRC(6dd4b4ed) SHA1(b37e9e5ddfb5d88c5412dc79643adfc4362fbb46) )

	ROM_REGION16_LE( 0x100000, "gfx", 0 )
	ROM_LOAD( "a+m005c0494_13a.u134", 0x00000, 0x80000, CRC(f101136a) SHA1(9ff7275e0c1fc41f3d97ae0bd628581e2803910a) )
	ROM_LOAD( "a+m005c0494_14a.u135", 0x80000, 0x80000, CRC(3e847f8f) SHA1(c99159951303b7f752305fa8e7e6d4bfb4fc54ba) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "a+m005c0494_12a.u151", 0x00000, 0x40000, CRC(2df3db1e) SHA1(d2e588db577de6fd527cd496f5eae9964d557da3) )

	ROM_REGION( 0x1a00,  "plds", 0 )
	ROM_LOAD( "a+m005c0494_06a.u126",   0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "a+m005c0494_03a.u125",   0x0200, 0x0200, NO_DUMP )
	ROM_LOAD( "a+m005c0494_02a.u307",   0x0400, 0x0200, NO_DUMP )
	ROM_LOAD( "a+m005c0494_18a.u306",   0x0600, 0x0200, NO_DUMP )
	ROM_LOAD( "a+m005c0494_17a.u305",   0x0800, 0x0200, NO_DUMP )
	ROM_LOAD( "a+m005c0494_05a.u206",   0x0a00, 0x0200, NO_DUMP )
	ROM_LOAD( "a+m005c0494_01a.u205",   0x0c00, 0x0200, NO_DUMP )
	ROM_LOAD( "a+m005c0494_04a.u601",   0x0e00, 0x0200, NO_DUMP )
	ROM_LOAD( "a+m005c0494_08a.u916",   0x1000, 0x0200, NO_DUMP )
	ROM_LOAD( "a+m005c0494_07a.u705",   0x1200, 0x0200, NO_DUMP )
	ROM_LOAD( "a+m005c0494_10a.u917",   0x1400, 0x0200, NO_DUMP )
	ROM_LOAD( "a+m005c0494_09a.u903",   0x1600, 0x0200, NO_DUMP )
	ROM_LOAD( "a+m005c0494_11a.u112",   0x1800, 0x0200, NO_DUMP )

ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

void artmagic_state::decrypt_ultennis()
{
	/* set up the parameters for the blitter data decryption which will happen at runtime */
	for (int i = 0; i < 16; i++)
	{
		m_xor[i] = 0x0462;
		if (i & 1) m_xor[i] ^= 0x0011;
		if (i & 2) m_xor[i] ^= 0x2200;
		if (i & 4) m_xor[i] ^= 0x4004;
		if (i & 8) m_xor[i] ^= 0x0880;
	}
}


void artmagic_state::decrypt_cheesech()
{
	/* set up the parameters for the blitter data decryption which will happen at runtime */
	for (int i = 0; i < 16; i++)
	{
		m_xor[i] = 0x0891;
		if (i & 1) m_xor[i] ^= 0x1100;
		if (i & 2) m_xor[i] ^= 0x0022;
		if (i & 4) m_xor[i] ^= 0x0440;
		if (i & 8) m_xor[i] ^= 0x8008;
	}
}


void artmagic_state::init_ultennis()
{
	decrypt_ultennis();
	m_is_stoneball = 0;
	m_protection_handler = &artmagic_state::ultennis_protection;

	/* additional (protection?) hack */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x300000, 0x300001, read16smo_delegate(*this, FUNC(artmagic_state::ultennis_hack_r)));
}


void artmagic_state::init_cheesech()
{
	decrypt_cheesech();
	m_is_stoneball = 0;
	m_protection_handler = &artmagic_state::cheesech_protection;
}


void artmagic_state::init_stonebal()
{
	decrypt_ultennis();
	m_is_stoneball = 1; /* blits 1 line high are NOT encrypted, also different first pixel decrypt */
	m_protection_handler = &artmagic_state::stonebal_protection;
}

void artmagic_state::init_shtstar()
{
	/* wrong */
	decrypt_ultennis();
	m_is_stoneball =0;
	m_protection_handler = &artmagic_state::stonebal_protection;
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1993, ultennis,   0,        artmagic, ultennis, artmagic_state, init_ultennis, ROT0, "Art & Magic", "Ultimate Tennis", MACHINE_SUPPORTS_SAVE )
GAME( 1993, ultennisj,  ultennis, artmagic, ultennis, artmagic_state, init_ultennis, ROT0, "Art & Magic (Banpresto license)", "Ultimate Tennis (v 1.4, Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, cheesech,   0,        cheesech, cheesech, artmagic_state, init_cheesech, ROT0, "Art & Magic", "Cheese Chase", MACHINE_SUPPORTS_SAVE )
GAME( 1994, stonebal,   0,        stonebal, stonebal, artmagic_state, init_stonebal, ROT0, "Art & Magic", "Stone Ball (4 Players, v1-20 13/12/1994)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, stonebal2,  stonebal, stonebal, stoneba2, artmagic_state, init_stonebal, ROT0, "Art & Magic", "Stone Ball (2 Players, v1-20 7/11/1994)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, stonebal2o, stonebal, stonebal, stoneba2, artmagic_state, init_stonebal, ROT0, "Art & Magic", "Stone Ball (2 Players, v1-20 21/10/1994)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, shtstar,    0,        shtstar,  shtstar,  artmagic_state, init_shtstar,  ROT0, "Nova", "Shooting Star", MACHINE_NOT_WORKING )
