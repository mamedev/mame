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
#include "cpu/m68000/m68000.h"
#include "cpu/tms34010/tms34010.h"
#include "cpu/mcs51/mcs51.h"
#include "video/tlc34076.h"
#include "includes/artmagic.h"
#include "sound/okim6295.h"
#include "machine/nvram.h"


#define MASTER_CLOCK_40MHz      (XTAL_40MHz)
#define MASTER_CLOCK_25MHz      (XTAL_25MHz)


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

WRITE16_MEMBER(artmagic_state::control_w)
{
	COMBINE_DATA(&m_control[offset]);

	/* OKI banking here */
	if (offset == 0)
	{
		m_oki->set_bank_base((((data >> 4) & 1) * 0x40000) % m_oki->region()->bytes());
	}

	logerror("%06X:control_w(%d) = %04X\n", space.device().safe_pc(), offset, data);
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
		assert_always(FALSE, "Unknown id in artmagic_state::device_timer");
	}
}


READ16_MEMBER(artmagic_state::ultennis_hack_r)
{
	/* IRQ5 points to: jsr (a5); rte */
	UINT32 pc = space.device().safe_pc();
	if (pc == 0x18c2 || pc == 0x18e4)
	{
		m_hack_irq = 1;
		update_irq_state();
		timer_set(attotime::from_usec(1), TIMER_IRQ_OFF);
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
				UINT16 a = m_prot_input[1] | (m_prot_input[2] << 8);
				UINT16 b = m_prot_input[3] | (m_prot_input[4] << 8);
				UINT16 c = m_prot_input[5] | (m_prot_input[6] << 8);
				UINT16 d = m_prot_input[7] | (m_prot_input[8] << 8);
				UINT16 x = a - b;
				if ((INT16)x >= 0)
					x = (x * c) >> 16;
				else
					x = -(((UINT16)-x * c) >> 16);
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
				UINT16 a = (INT16)(m_prot_input[1] | (m_prot_input[2] << 8));
				UINT16 b = (INT16)(m_prot_input[3] | (m_prot_input[4] << 8));
				/*UINT16 c = (INT16)(m_prot_input[5] | (m_prot_input[6] << 8));*/
				UINT32 x = a * a * (b/2);
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
				UINT16 x = m_prot_save;
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
				UINT16 a = m_prot_input[1] | (m_prot_input[2] << 8);
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
				UINT16 a = m_prot_input[1] | (m_prot_input[2] << 8);
				UINT16 b = m_prot_input[3] | (m_prot_input[4] << 8);
				UINT16 c = 0x4000;      /* seems to be hard-coded */
				UINT16 d = 0x00a0;      /* seems to be hard-coded */
				UINT16 x = a - b;
				if ((INT16)x >= 0)
					x = (x * c) >> 16;
				else
					x = -(((UINT16)-x * c) >> 16);
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
				UINT16 x = m_prot_save;
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
				UINT16 a = m_prot_input[1] | (m_prot_input[2] << 8);
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
				UINT16 a = m_prot_input[1] | (m_prot_input[2] << 8);
				UINT16 b = m_prot_input[3] | (m_prot_input[4] << 8);
				UINT16 c = m_prot_input[5] | (m_prot_input[6] << 8);
				UINT16 d = m_prot_input[7] | (m_prot_input[8] << 8);
				UINT16 x = a - b;
				if ((INT16)x >= 0)
					x = (x * d) >> 16;
				else
					x = -(((UINT16)-x * d) >> 16);
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
				/*UINT16 a = m_prot_input[1] | (m_prot_input[2] << 8);*/
				UINT8 x = 0xa5;
				m_prot_output[0] = x;
				m_prot_output_index = 0;
			}
			else if (m_prot_input_index >= 4)
				m_prot_input_index = 0;
			break;

		case 0x03:  /* 03 (xxxx) */
			if (m_prot_input_index == 1)
			{
				UINT16 x = m_prot_save;
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
				UINT16 a = m_prot_input[1] | (m_prot_input[2] << 8);
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


CUSTOM_INPUT_MEMBER(artmagic_state::prot_r)
{
	return m_prot_output_bit;
}


WRITE16_MEMBER(artmagic_state::protection_bit_w)
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

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, artmagic_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x220000, 0x23ffff) AM_RAM
	AM_RANGE(0x240000, 0x240fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x300000, 0x300001) AM_READ_PORT("300000")
	AM_RANGE(0x300002, 0x300003) AM_READ_PORT("300002")
	AM_RANGE(0x300004, 0x300005) AM_READ_PORT("300004")
	AM_RANGE(0x300006, 0x300007) AM_READ_PORT("300006")
	AM_RANGE(0x300008, 0x300009) AM_READ_PORT("300008")
	AM_RANGE(0x30000a, 0x30000b) AM_READ_PORT("30000a")
	AM_RANGE(0x300000, 0x300003) AM_WRITE(control_w) AM_SHARE("control")
	AM_RANGE(0x300004, 0x300007) AM_WRITE(protection_bit_w)
	AM_RANGE(0x360000, 0x360001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x380000, 0x380007) AM_DEVREADWRITE("tms", tms34010_device, host_r, host_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( stonebal_map, AS_PROGRAM, 16, artmagic_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x200000, 0x27ffff) AM_RAM
	AM_RANGE(0x280000, 0x280fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x300000, 0x300001) AM_READ_PORT("300000")
	AM_RANGE(0x300002, 0x300003) AM_READ_PORT("300002")
	AM_RANGE(0x300004, 0x300005) AM_READ_PORT("300004")
	AM_RANGE(0x300006, 0x300007) AM_READ_PORT("300006")
	AM_RANGE(0x300008, 0x300009) AM_READ_PORT("300008")
	AM_RANGE(0x30000a, 0x30000b) AM_READ_PORT("30000a")
	AM_RANGE(0x30000c, 0x30000d) AM_READ_PORT("30000c")
	AM_RANGE(0x30000e, 0x30000f) AM_READ_PORT("30000e")
	AM_RANGE(0x300000, 0x300003) AM_WRITE(control_w) AM_SHARE("control")
	AM_RANGE(0x300004, 0x300007) AM_WRITE(protection_bit_w)
	AM_RANGE(0x340000, 0x340001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x380000, 0x380007) AM_DEVREADWRITE("tms", tms34010_device, host_r, host_w)
ADDRESS_MAP_END

READ16_MEMBER(artmagic_state::unk_r)
{
	return machine().rand();
}

static ADDRESS_MAP_START( shtstar_map, AS_PROGRAM, 16, artmagic_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x200000, 0x27ffff) AM_RAM
	AM_RANGE(0x280000, 0x280fff) AM_RAM AM_SHARE("nvram")

	AM_RANGE(0x3c0000, 0x3c0001) AM_READ_PORT("3c0000")
	AM_RANGE(0x3c0002, 0x3c0003) AM_READ_PORT("3c0002")
	AM_RANGE(0x3c0004, 0x3c0005) AM_READ_PORT("3c0004")
	AM_RANGE(0x3c0006, 0x3c0007) AM_READ_PORT("3c0006")
	AM_RANGE(0x3c0008, 0x3c0009) AM_READ_PORT("3c0008")
	AM_RANGE(0x3c000a, 0x3c000b) AM_READ_PORT("3c000a")

	AM_RANGE(0x3c0012, 0x3c0013) AM_READ(unk_r)
	AM_RANGE(0x3c0014, 0x3c0015) AM_NOP

	AM_RANGE(0x300000, 0x300003) AM_WRITE(control_w) AM_SHARE("control")
	AM_RANGE(0x3c0004, 0x3c0007) AM_WRITE(protection_bit_w)
	AM_RANGE(0x340000, 0x340001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x380000, 0x380007) AM_DEVREADWRITE("tms", tms34010_device, host_r, host_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Slave CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( tms_map, AS_PROGRAM, 16, artmagic_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_SHARE("vram0")
	AM_RANGE(0x00400000, 0x005fffff) AM_RAM AM_SHARE("vram1")
	AM_RANGE(0x00800000, 0x0080007f) AM_READWRITE(artmagic_blitter_r, artmagic_blitter_w)
	AM_RANGE(0x00c00000, 0x00c000ff) AM_DEVREADWRITE8("tlc34076", tlc34076_device, read, write, 0x00ff)
	AM_RANGE(0xc0000000, 0xc00001ff) AM_DEVREADWRITE("tms", tms34010_device, io_register_r, io_register_w)
	AM_RANGE(0xffe00000, 0xffffffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( stonebal_tms_map, AS_PROGRAM, 16, artmagic_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_SHARE("vram0")
	AM_RANGE(0x00400000, 0x005fffff) AM_RAM AM_SHARE("vram1")
	AM_RANGE(0x00800000, 0x0080007f) AM_READWRITE(artmagic_blitter_r, artmagic_blitter_w)
	AM_RANGE(0x00c00000, 0x00c000ff) AM_DEVREADWRITE8("tlc34076", tlc34076_device, read, write, 0x00ff)
	AM_RANGE(0xc0000000, 0xc00001ff) AM_DEVREADWRITE("tms", tms34010_device, io_register_r, io_register_w)
	AM_RANGE(0xffc00000, 0xffffffff) AM_RAM
ADDRESS_MAP_END

/*************************************
 *
 *  Extra CPU memory handlers
 *   (Shooting Star)
 *
 *************************************/

/* see adp.c */
static ADDRESS_MAP_START( shtstar_subcpu_map, AS_PROGRAM, 16, artmagic_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0xffc000, 0xffffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( shtstar_guncpu_map, AS_PROGRAM, 8, artmagic_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( shtstar_guncpu_io_map, AS_IO, 8, artmagic_state )
	AM_RANGE(0xc000, 0xcfff) AM_RAM
ADDRESS_MAP_END


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
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, artmagic_state,prot_r, NULL)    /* protection data */
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_SPECIAL )     /* protection ready */
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
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, artmagic_state,prot_r, NULL)    /* protection data */
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_SPECIAL )     /* protection ready */
	PORT_BIT( 0x00fc, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( artmagic, artmagic_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, MASTER_CLOCK_25MHz/2)
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_CPU_ADD("tms", TMS34010, MASTER_CLOCK_40MHz)
	MCFG_CPU_PROGRAM_MAP(tms_map)
	MCFG_TMS340X0_HALT_ON_RESET(TRUE) /* halt on reset */
	MCFG_TMS340X0_PIXEL_CLOCK(MASTER_CLOCK_40MHz/6) /* pixel clock */
	MCFG_TMS340X0_PIXELS_PER_CLOCK(1) /* pixels per clock */
	MCFG_TMS340X0_SCANLINE_RGB32_CB(artmagic_state, scanline)              /* scanline update (rgb32) */
	MCFG_TMS340X0_OUTPUT_INT_CB(WRITELINE(artmagic_state, m68k_gen_int))
	MCFG_TMS340X0_TO_SHIFTREG_CB(artmagic_state, to_shiftreg)           /* write to shiftreg function */
	MCFG_TMS340X0_FROM_SHIFTREG_CB(artmagic_state, from_shiftreg)          /* read from shiftreg function */

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))
	MCFG_NVRAM_ADD_1FILL("nvram")

	/* video hardware */
	MCFG_TLC34076_ADD("tlc34076", TLC34076_6_BIT)


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK_40MHz/6, 428, 0, 320, 313, 0, 256)
	MCFG_SCREEN_UPDATE_DEVICE("tms", tms34010_device, tms340x0_rgb32)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", MASTER_CLOCK_40MHz/3/10, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.65)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( cheesech, artmagic )

	MCFG_SOUND_MODIFY("oki")
	MCFG_SOUND_ROUTES_RESET()
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( stonebal, artmagic )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(stonebal_map)

	MCFG_CPU_MODIFY("tms")
	MCFG_CPU_PROGRAM_MAP(stonebal_tms_map)

	MCFG_SOUND_MODIFY("oki")
	MCFG_SOUND_ROUTES_RESET()
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.45)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( shtstar, artmagic )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(shtstar_map)

	/* sub cpu*/
	MCFG_CPU_ADD("subcpu", M68000, MASTER_CLOCK_25MHz/2)
	MCFG_CPU_PROGRAM_MAP(shtstar_subcpu_map)

	/*gun board cpu*/
	MCFG_CPU_ADD("guncpu", I80C31, 6000000)
	MCFG_CPU_IO_MAP(shtstar_guncpu_io_map)
	MCFG_CPU_PROGRAM_MAP(shtstar_guncpu_map)

MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( cheesech )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for 68000 code */
	ROM_LOAD16_BYTE( "u102",     0x00000, 0x40000, CRC(1d6e07c5) SHA1(8650868cce47f685d22131aa28aad45033cb0a52) )
	ROM_LOAD16_BYTE( "u101",     0x00001, 0x40000, CRC(30ae9f95) SHA1(fede5d271aabb654c1efc077253d81ba23786f22) )

	ROM_REGION16_LE( 0x100000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "u134", 0x00000, 0x80000, CRC(354ba4a6) SHA1(68e7df750efb21c716ba8b8ed4ca15a8cdc9141b) )
	ROM_LOAD16_BYTE( "u135", 0x00001, 0x80000, CRC(97348681) SHA1(7e74685041cd5e8fbd45731284cf316dc3ffec60) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "u151", 0x00000, 0x80000, CRC(65d5ebdb) SHA1(0d905b9a60b86e51de3bdcf6eeb059fe29606431) )
ROM_END


ROM_START( ultennis )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for 68000 code */
	ROM_LOAD16_BYTE( "utu102.bin", 0x00000, 0x40000, CRC(ec31385e) SHA1(244e78619c549712d5541fb252656afeba639bb7) )
	ROM_LOAD16_BYTE( "utu101.bin", 0x00001, 0x40000, CRC(08a7f655) SHA1(b8a4265472360b68bed71d6c175fc54dff088c1d) )

	ROM_REGION16_LE( 0x200000, "gfx1", 0 )
	ROM_LOAD( "utu133.bin", 0x000000, 0x200000, CRC(29d9204d) SHA1(0b2b77a55b8c2877c2e31b63156505584d4ee1f0) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "utu151.bin", 0x00000,  0x40000, CRC(4e19ca89) SHA1(ac7e17631ec653f83c4912df6f458b0e1df88096) )
ROM_END


ROM_START( ultennisj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for 68000 code */
	ROM_LOAD16_BYTE( "a&m001d0194-13c-u102-japan.u102", 0x00000, 0x40000, CRC(65cee452) SHA1(49259e8faf289d6d80769f6d44e9d61d15e431c6) )
	ROM_LOAD16_BYTE( "a&m001d0194-12c-u101-japan.u101", 0x00001, 0x40000, CRC(5f4b0ca0) SHA1(57e9ed60cc0e53eeb4e08c4003138d3bdaec3de7) )

	ROM_REGION16_LE( 0x200000, "gfx1", 0 )
	ROM_LOAD( "a&m-001-01-a.ic133", 0x000000, 0x200000, CRC(29d9204d) SHA1(0b2b77a55b8c2877c2e31b63156505584d4ee1f0) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "a&m001c1293-14a-u151.u151", 0x00000,  0x40000, CRC(4e19ca89) SHA1(ac7e17631ec653f83c4912df6f458b0e1df88096) )
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
	ROM_LOAD16_BYTE( "u102",     0x00000, 0x40000, CRC(712feda1) SHA1(c5b385f425786566fa274fe166a7116615a8ce86) )
	ROM_LOAD16_BYTE( "u101",     0x00001, 0x40000, CRC(4f1656a9) SHA1(720717ae4166b3ec50bb572197a8c6c96b284648) )

	ROM_REGION16_LE( 0x400000, "gfx1", 0 )
	ROM_LOAD( "u1600.bin", 0x000000, 0x200000, CRC(d2ffe9ff) SHA1(1c5dcbd8208e45458da9db7621f6b8602bca0fae) )
	ROM_LOAD( "u1601.bin", 0x200000, 0x200000, CRC(dbe893f0) SHA1(71a8a022decc0ff7d4c65f7e6e0cbba9e0b5582c) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "u1801.bin", 0x00000, 0x80000, CRC(d98f7378) SHA1(700df7f29c039b96791c2704a67f01a722dc96dc) )
ROM_END


ROM_START( stonebal2 )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 64k for 68000 code */
	ROM_LOAD16_BYTE( "u102.bin", 0x00000, 0x40000, CRC(b3c4f64f) SHA1(6327e9f3cd9deb871a6910cf1f006c8ee143e859) )
	ROM_LOAD16_BYTE( "u101.bin", 0x00001, 0x40000, CRC(fe373f74) SHA1(bafac4bbd1aae4ccc4ae16205309483f1bbdd464) )

	ROM_REGION16_LE( 0x400000, "gfx1", 0 )
	ROM_LOAD( "u1600.bin", 0x000000, 0x200000, CRC(d2ffe9ff) SHA1(1c5dcbd8208e45458da9db7621f6b8602bca0fae) )
	ROM_LOAD( "u1601.bin", 0x200000, 0x200000, CRC(dbe893f0) SHA1(71a8a022decc0ff7d4c65f7e6e0cbba9e0b5582c) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "u1801.bin", 0x00000, 0x80000, CRC(d98f7378) SHA1(700df7f29c039b96791c2704a67f01a722dc96dc) )
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

	ROM_REGION16_LE( 0x100000, "gfx1", 0 )
	ROM_LOAD( "a&m005c0494_13a.u134", 0x00000, 0x80000, CRC(f101136a) SHA1(9ff7275e0c1fc41f3d97ae0bd628581e2803910a) )
	ROM_LOAD( "a&m005c0494_14a.u135", 0x80000, 0x80000, CRC(3e847f8f) SHA1(c99159951303b7f752305fa8e7e6d4bfb4fc54ba) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "a&m005c0494_12a.u151", 0x00000, 0x40000, CRC(2df3db1e) SHA1(d2e588db577de6fd527cd496f5eae9964d557da3) )

	ROM_REGION( 0x1a00,  "plds", 0 )
	ROM_LOAD( "a&m005c0494_06a.u126",   0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "a&m005c0494_03a.u125",   0x0200, 0x0200, NO_DUMP )
	ROM_LOAD( "a&m005c0494_02a.u307",   0x0400, 0x0200, NO_DUMP )
	ROM_LOAD( "a&m005c0494_18a.u306",   0x0600, 0x0200, NO_DUMP )
	ROM_LOAD( "a&m005c0494_17a.u305",   0x0800, 0x0200, NO_DUMP )
	ROM_LOAD( "a&m005c0494_05a.u206",   0x0a00, 0x0200, NO_DUMP )
	ROM_LOAD( "a&m005c0494_01a.u205",   0x0c00, 0x0200, NO_DUMP )
	ROM_LOAD( "a&m005c0494_04a.u601",   0x0e00, 0x0200, NO_DUMP )
	ROM_LOAD( "a&m005c0494_08a.u916",   0x1000, 0x0200, NO_DUMP )
	ROM_LOAD( "a&m005c0494_07a.u705",   0x1200, 0x0200, NO_DUMP )
	ROM_LOAD( "a&m005c0494_10a.u917",   0x1400, 0x0200, NO_DUMP )
	ROM_LOAD( "a&m005c0494_09a.u903",   0x1600, 0x0200, NO_DUMP )
	ROM_LOAD( "a&m005c0494_11a.u112",   0x1800, 0x0200, NO_DUMP )

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


DRIVER_INIT_MEMBER(artmagic_state,ultennis)
{
	decrypt_ultennis();
	m_is_stoneball = 0;
	m_protection_handler = &artmagic_state::ultennis_protection;

	/* additional (protection?) hack */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x300000, 0x300001, read16_delegate(FUNC(artmagic_state::ultennis_hack_r),this));
}


DRIVER_INIT_MEMBER(artmagic_state,cheesech)
{
	decrypt_cheesech();
	m_is_stoneball = 0;
	m_protection_handler = &artmagic_state::cheesech_protection;
}


DRIVER_INIT_MEMBER(artmagic_state,stonebal)
{
	decrypt_ultennis();
	m_is_stoneball = 1; /* blits 1 line high are NOT encrypted, also different first pixel decrypt */
	m_protection_handler = &artmagic_state::stonebal_protection;
}

DRIVER_INIT_MEMBER(artmagic_state,shtstar)
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

GAME( 1993, ultennis, 0,        artmagic, ultennis, artmagic_state, ultennis, ROT0, "Art & Magic", "Ultimate Tennis", MACHINE_SUPPORTS_SAVE )
GAME( 1993, ultennisj,ultennis, artmagic, ultennis, artmagic_state, ultennis, ROT0, "Art & Magic (Banpresto license)", "Ultimate Tennis (v 1.4, Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, cheesech, 0,        cheesech, cheesech, artmagic_state, cheesech, ROT0, "Art & Magic", "Cheese Chase", MACHINE_SUPPORTS_SAVE )
GAME( 1994, stonebal, 0,        stonebal, stonebal, artmagic_state, stonebal, ROT0, "Art & Magic", "Stone Ball (4 Players)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, stonebal2,stonebal, stonebal, stoneba2, artmagic_state, stonebal, ROT0, "Art & Magic", "Stone Ball (2 Players)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, shtstar, 0, shtstar, shtstar, artmagic_state, shtstar, ROT0, "Nova", "Shooting Star", MACHINE_NOT_WORKING )
