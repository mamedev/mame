// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari GT hardware

    driver by Aaron Giles

    Games supported:
        * T-Mek (1994) [2 sets]
        * Primal Rage (1994) [2 sets]

    Known bugs:
        * protection devices unknown

****************************************************************************

    Memory map (TBA)

    Primal Rage ROMs (A051512-)   -
               location  label
               ------------------------
               25L       136102-1041B 2DFC
               27L       136102-1042B 83FD
               28L       136102-1043B 61FE
               29L       136102-1044B 67FF
               25N       136102-0050A E8C6?
               27N       136102-0051A 4410
               28N       136102-0052A 8CD3?
               23P       136102-1045B FE9D?
               25R       not populated
               27R       not populated
               28R       not populated
               27V       not populated
               27W       not populated
               23V       not populated
               23W       not populated
               19V       not populated
               19W       not populated
               16V       not populated
               16W       not populated
               12V       not populated
               12W       not populated
               9V        not populated
               9W        not populated
               5V        not populated
               5W        not populated
               2V        136102-1100A DFE0
               2W        136102-1101A F984
               13B       not populated

***************************************************************************/


#include "emu.h"
#include "machine/atarigen.h"
#include "video/atarirle.h"
#include "cpu/m68000/m68000.h"
#include "includes/atarigt.h"


#define LOG_PROTECTION      (0)
#define HACK_TMEK_CONTROLS  (0)


/*************************************
 *
 *  Initialization
 *
 *************************************/

void atarigt_state::update_interrupts()
{
	m_maincpu->set_input_line(3, m_sound_int_state    ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(4, m_video_int_state    ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(6, m_scanline_int_state ? ASSERT_LINE : CLEAR_LINE);
}


MACHINE_RESET_MEMBER(atarigt_state,atarigt)
{
	atarigen_state::machine_reset();
	scanline_timer_reset(*m_screen, 8);
}



/*************************************
 *
 *  CAGE sound interrupts
 *
 *************************************/

WRITE8_MEMBER(atarigt_state::cage_irq_callback)
{
	if (data)
		sound_int_gen(m_maincpu);
	else
		sound_int_ack_w(space,0,0);
}

/*************************************
 *
 *  Input ports
 *
 *************************************/

READ32_MEMBER(atarigt_state::special_port2_r)
{
	int temp = ioport("SERVICE")->read();
	temp ^= 0x0001;     /* /A2DRDY always high for now */
	temp ^= 0x0008;     /* A2D.EOC always high for now */
	return (temp << 16) | temp;
}


READ32_MEMBER(atarigt_state::special_port3_r)
{
	int temp = ioport("COIN")->read();
	if (m_video_int_state) temp ^= 0x0001;
	if (m_scanline_int_state) temp ^= 0x0002;
	return (temp << 16) | temp;
}


inline void atarigt_state::compute_fake_pots(int *pots)
{
#if (HACK_TMEK_CONTROLS)
	int fake = ioport("FAKE")->read();

	pots[0] = pots[1] = pots[2] = pots[3] = 0x80;

	if (fake & 0x01)            /* up */
	{
		if (fake & 0x04)        /* up and left */
			pots[3] = 0x00;
		else if (fake & 0x08)   /* up and right */
			pots[1] = 0x00;
		else                    /* up only */
			pots[1] = pots[3] = 0x00;
	}
	else if (fake & 0x02)       /* down */
	{
		if (fake & 0x04)        /* down and left */
			pots[3] = 0xff;
		else if (fake & 0x08)   /* down and right */
			pots[1] = 0xff;
		else                    /* down only */
			pots[1] = pots[3] = 0xff;
	}
	else if (fake & 0x04)       /* left only */
		pots[1] = 0xff, pots[3] = 0x00;
	else if (fake & 0x08)       /* right only */
		pots[3] = 0xff, pots[1] = 0x00;
#endif
}


READ32_MEMBER(atarigt_state::analog_port0_r)
{
#if (HACK_TMEK_CONTROLS)
	int pots[4];
	compute_fake_pots(pots);
	return (pots[0] << 24) | (pots[3] << 8);
#else
	return (ioport("AN1")->read() << 24) | (ioport("AN2")->read() << 8);
#endif
}


READ32_MEMBER(atarigt_state::analog_port1_r)
{
#if (HACK_TMEK_CONTROLS)
	int pots[4];
	compute_fake_pots(pots);
	return (pots[2] << 24) | (pots[1] << 8);
#else
	return (ioport("AN3")->read() << 24) | (ioport("AN4")->read() << 8);
#endif
}



/*************************************
 *
 *  Output ports
 *
 *************************************/

WRITE32_MEMBER(atarigt_state::latch_w)
{
	/*
	    D13 = 68.DISA
	    D12 = ERASE
	    D11 = /MOGO
	    D8  = VCR
	    D5  = /XRESET
	    D4  = /SNDRES
	    D3  = CC.L
	    D0  = CC.R
	*/

	/* upper byte */
	if (ACCESSING_BITS_24_31)
	{
		/* bits 13-11 are the MO control bits */
		m_rle->control_write(space, offset, (data >> 27) & 7);
	}

	if (ACCESSING_BITS_16_23)
	{
		//cage_reset_w(space, data & 0x00100000);
		coin_counter_w(machine(), 0, data & 0x00080000);
		coin_counter_w(machine(), 1, data & 0x00010000);
	}
}


WRITE32_MEMBER(atarigt_state::mo_command_w)
{
	COMBINE_DATA(m_mo_command);
	if (ACCESSING_BITS_0_15)
		m_rle->command_write(space, offset, ((data & 0xffff) == 2) ? ATARIRLE_COMMAND_CHECKSUM : ATARIRLE_COMMAND_DRAW);
}


WRITE32_MEMBER(atarigt_state::led_w)
{
//  logerror("LED = %08X & %08X\n", data, mem_mask);
}



/*************************************
 *
 *  Sound I/O
 *
 *************************************/

READ32_MEMBER(atarigt_state::sound_data_r)
{
	UINT32 result = 0;

	if (ACCESSING_BITS_0_15)
		result |= m_cage->control_r();
	if (ACCESSING_BITS_16_31)
		result |= m_cage->main_r() << 16;
	return result;
}


WRITE32_MEMBER(atarigt_state::sound_data_w)
{
	if (ACCESSING_BITS_0_15)
		m_cage->control_w(data);
	if (ACCESSING_BITS_16_31)
		m_cage->main_w(data >> 16);
}



/*************************************
 *
 *  T-Mek protection
 *
 *************************************/



void atarigt_state::tmek_update_mode(offs_t offset)
{
	/* pop us into the readseq */
	for (int i = 0; i < ADDRSEQ_COUNT - 1; i++)
		m_protaddr[i] = m_protaddr[i + 1];
	m_protaddr[ADDRSEQ_COUNT - 1] = offset;
}


void atarigt_state::tmek_protection_w(address_space &space, offs_t offset, UINT16 data)
{
/*
    T-Mek init:
        ($387C0) = $0001
        Read ($38010), add to memory
        Write $3C0 bytes to low half of words from $38000-$3877E
        Read ($38488)
*/

	if (LOG_PROTECTION) logerror("%06X:Protection W@%06X = %04X\n", space.device().safe_pcbase(), offset, data);

	/* track accesses */
	tmek_update_mode(offset);

	switch (offset)
	{
		case 0xdb0000:
			m_ignore_writes = (data == 0x18);
			break;
	}
}

void atarigt_state::tmek_protection_r(address_space &space, offs_t offset, UINT16 *data)
{
	if (LOG_PROTECTION) logerror("%06X:Protection R@%06X\n", space.device().safe_pcbase(), offset);

	/* track accesses */
	tmek_update_mode(offset);

	/* handle specific reads */
	switch (offset)
	{
		/* status register; the code spins on this waiting for the high bit to be set */
		case 0xdb8700:
		case 0xdb87c0:
//          if (m_protmode != 0)
			{
				*data = -1;//0x8000;
			}
			break;
	}
}



/*************************************
 *
 *  Primal Rage protection
 *
 *************************************/

void atarigt_state::primrage_update_mode(offs_t offset)
{
	/* pop us into the readseq */
	for (int i = 0; i < ADDRSEQ_COUNT - 1; i++)
		m_protaddr[i] = m_protaddr[i + 1];
	m_protaddr[ADDRSEQ_COUNT - 1] = offset;

	/* check for particular sequences */
	if (!m_protmode)
	{
		/* this is from the code at $20f90 */
		if (m_protaddr[1] == 0xdcc7c4 && m_protaddr[2] == 0xdcc7c4 && m_protaddr[3] == 0xdc4010)
		{
			if (LOG_PROTECTION) logerror("prot:Entering mode 1\n");
			m_protmode = 1;
		}

		/* this is from the code at $27592 */
		if (m_protaddr[0] == 0xdcc7ca && m_protaddr[1] == 0xdcc7ca && m_protaddr[2] == 0xdcc7c6 && m_protaddr[3] == 0xdc4022)
		{
			if (LOG_PROTECTION) logerror("prot:Entering mode 2\n");
			m_protmode = 2;
		}

		/* this is from the code at $3d8dc */
		if (m_protaddr[0] == 0xdcc7c0 && m_protaddr[1] == 0xdcc7c0 && m_protaddr[2] == 0xdc80f2 && m_protaddr[3] == 0xdc7af2)
		{
			if (LOG_PROTECTION) logerror("prot:Entering mode 3\n");
			m_protmode = 3;
		}
	}
}



void atarigt_state::primrage_protection_w(address_space &space, offs_t offset, UINT16 data)
{
	if (LOG_PROTECTION)
	{
	UINT32 pc = space.device().safe_pcbase();
	switch (pc)
	{
		/* protection code from 20f90 - 21000 */
		case 0x20fba:
			if (offset % 16 == 0) logerror("\n   ");
			logerror("W@%06X(%04X) ", offset, data);
			break;

		/* protection code from 27592 - 27664 */
		case 0x275f6:
			logerror("W@%06X(%04X) ", offset, data);
			break;

		/* protection code from 3d8dc - 3d95a */
		case 0x3d908:
		case 0x3d932:
		case 0x3d938:
		case 0x3d93e:
			logerror("W@%06X(%04X) ", offset, data);
			break;
		case 0x3d944:
			logerror("W@%06X(%04X) - done\n", offset, data);
			break;

		/* protection code from 437fa - 43860 */
		case 0x43830:
		case 0x43838:
			logerror("W@%06X(%04X) ", offset, data);
			break;

		/* catch anything else */
		default:
			logerror("%06X:Unknown protection W@%06X = %04X\n", space.device().safe_pcbase(), offset, data);
			break;
	}
	}

/* mask = 0x78fff */

	/* track accesses */
	primrage_update_mode(offset);

	/* check for certain read sequences */
	if (m_protmode == 1 && offset >= 0xdc7800 && offset < 0xdc7800 + sizeof(m_protdata) * 2)
		m_protdata[(offset - 0xdc7800) / 2] = data;

	if (m_protmode == 2)
	{
		int temp = (offset - 0xdc7800) / 2;
		if (LOG_PROTECTION) logerror("prot:mode 2 param = %04X\n", temp);
		m_protresult = temp * 0x6915 + 0x6915;
	}

	if (m_protmode == 3)
	{
		if (offset == 0xdc4700)
		{
			if (LOG_PROTECTION) logerror("prot:Clearing mode 3\n");
			m_protmode = 0;
		}
	}
}



void atarigt_state::primrage_protection_r(address_space &space, offs_t offset, UINT16 *data)
{
	/* track accesses */
	primrage_update_mode(offset);

if (LOG_PROTECTION)
{
	UINT32 pc = space.device().safe_pcbase();
	UINT32 p1, p2, a6;
	switch (pc)
	{
		/* protection code from 20f90 - 21000 */
		case 0x20f90:
			logerror("Known Protection @ 20F90: R@%06X ", offset);
			break;
		case 0x20f98:
		case 0x20fa0:
			logerror("R@%06X ", offset);
			break;
		case 0x20fcc:
			logerror("R@%06X - done\n", offset);
			break;

		/* protection code from 27592 - 27664 */
		case 0x275bc:
			break;
		case 0x275cc:
			a6 = space.device().state().state_int(M68K_A6);
			p1 = (space.read_word(a6+8) << 16) | space.read_word(a6+10);
			p2 = (space.read_word(a6+12) << 16) | space.read_word(a6+14);
			logerror("Known Protection @ 275BC(%08X, %08X): R@%06X ", p1, p2, offset);
			break;
		case 0x275d2:
		case 0x275d8:
		case 0x275de:
		case 0x2761e:
		case 0x2762e:
			logerror("R@%06X ", offset);
			break;
		case 0x2763e:
			logerror("R@%06X - done\n", offset);
			break;

		/* protection code from 3d8dc - 3d95a */
		case 0x3d8f4:
			a6 = space.device().state().state_int(M68K_A6);
			p1 = (space.read_word(a6+12) << 16) | space.read_word(a6+14);
			logerror("Known Protection @ 3D8F4(%08X): R@%06X ", p1, offset);
			break;
		case 0x3d8fa:
		case 0x3d90e:
			logerror("R@%06X ", offset);
			break;

		/* protection code from 437fa - 43860 */
		case 0x43814:
			a6 = space.device().state().state_int(M68K_A6);
			p1 = space.read_dword(a6+14) & 0xffffff;
			logerror("Known Protection @ 43814(%08X): R@%06X ", p1, offset);
			break;
		case 0x4381c:
		case 0x43840:
			logerror("R@%06X ", offset);
			break;
		case 0x43848:
			logerror("R@%06X - done\n", offset);
			break;

		/* catch anything else */
		default:
			logerror("%06X:Unknown protection R@%06X\n", space.device().safe_pcbase(), offset);
			break;
	}
}

	/* handle specific reads */
	switch (offset)
	{
		/* status register; the code spins on this waiting for the high bit to be set */
		case 0xdc4700:
//          if (m_protmode != 0)
			{
				*data = 0x8000;
			}
			break;

		/* some kind of result register */
		case 0xdcc7c2:
			if (m_protmode == 2)
			{
				*data = m_protresult;
				m_protmode = 0;
			if (LOG_PROTECTION) logerror("prot:Clearing mode 2\n");
			}
			break;

		case 0xdcc7c4:
			if (m_protmode == 1)
			{
				m_protmode = 0;
			if (LOG_PROTECTION) logerror("prot:Clearing mode 1\n");
			}
			break;
	}
}



/*************************************
 *
 *  Protection/color RAM
 *
 *************************************/

READ32_MEMBER(atarigt_state::colorram_protection_r)
{
	offs_t address = 0xd80000 + offset * 4;
	UINT32 result32 = 0;
	UINT16 result;

	if (ACCESSING_BITS_16_31)
	{
		result = atarigt_colorram_r(address);
		(this->*m_protection_r)(space, address, &result);
		result32 |= result << 16;
	}
	if (ACCESSING_BITS_0_15)
	{
		result = atarigt_colorram_r(address + 2);
		(this->*m_protection_r)(space, address + 2, &result);
		result32 |= result;
	}

	return result32;
}


WRITE32_MEMBER(atarigt_state::colorram_protection_w)
{
	offs_t address = 0xd80000 + offset * 4;

	if (ACCESSING_BITS_16_31)
	{
		if (!m_ignore_writes)
			atarigt_colorram_w(address, data >> 16, mem_mask >> 16);
		(this->*m_protection_w)(space, address, data >> 16);
	}
	if (ACCESSING_BITS_0_15)
	{
		if (!m_ignore_writes)
			atarigt_colorram_w(address + 2, data, mem_mask);
		(this->*m_protection_w)(space, address + 2, data);
	}
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 32, atarigt_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0xc00000, 0xc00003) AM_READWRITE(sound_data_r, sound_data_w)
	AM_RANGE(0xd00014, 0xd00017) AM_READ(analog_port0_r)
	AM_RANGE(0xd0001c, 0xd0001f) AM_READ(analog_port1_r)
	AM_RANGE(0xd20000, 0xd20fff) AM_DEVREADWRITE8("eeprom", atari_eeprom_device, read, write, 0xff00ff00)
	AM_RANGE(0xd40000, 0xd4ffff) AM_DEVWRITE("eeprom", atari_eeprom_device, unlock_write)
	AM_RANGE(0xd72000, 0xd75fff) AM_DEVWRITE("playfield", tilemap_device, write) AM_SHARE("playfield")
	AM_RANGE(0xd76000, 0xd76fff) AM_DEVWRITE("alpha", tilemap_device, write) AM_SHARE("alpha")
	AM_RANGE(0xd78000, 0xd78fff) AM_RAM AM_SHARE("rle")
	AM_RANGE(0xd7a200, 0xd7a203) AM_WRITE(mo_command_w) AM_SHARE("mo_command")
	AM_RANGE(0xd70000, 0xd7ffff) AM_RAM
	AM_RANGE(0xd80000, 0xdfffff) AM_READWRITE(colorram_protection_r, colorram_protection_w) AM_SHARE("colorram")
	AM_RANGE(0xe04000, 0xe04003) AM_WRITE(led_w)
	AM_RANGE(0xe08000, 0xe08003) AM_WRITE(latch_w)
	AM_RANGE(0xe0a000, 0xe0a003) AM_WRITE16(scanline_int_ack_w, 0xffffffff)
	AM_RANGE(0xe0c000, 0xe0c003) AM_WRITE16(video_int_ack_w, 0xffffffff)
	AM_RANGE(0xe0e000, 0xe0e003) AM_WRITENOP//watchdog_reset_w },
	AM_RANGE(0xe80000, 0xe80003) AM_READ_PORT("P1_P2")
	AM_RANGE(0xe82000, 0xe82003) AM_READ(special_port2_r)
	AM_RANGE(0xe82004, 0xe82007) AM_READ(special_port3_r)
	AM_RANGE(0xf80000, 0xffffff) AM_RAM
ADDRESS_MAP_END



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( common )
	PORT_START("P1_P2")             /* 68.SW (A1=0, A1=1) */
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)

	PORT_START("SERVICE")       /* 68.STATUS (A2=0) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SPECIAL )  /* /A2DRDY */
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_TILT )     /* TILT */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SPECIAL )  /* /XIRQ23 */
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_SPECIAL ) /* A2D.EOC */
	PORT_BIT( 0x0030, IP_ACTIVE_LOW, IPT_UNUSED )   /* NC */
	PORT_SERVICE( 0x0040, IP_ACTIVE_LOW )           /* SELFTEST */
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")    /* VBLANK */
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN")          /* 68.STATUS (A2=1) */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_SPECIAL )  /* /VBIRQ */
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_SPECIAL )  /* /4MSIRQ */
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SPECIAL )  /* /XIRQ0 */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SPECIAL )  /* /XIRQ1 */
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SPECIAL )  /* /SERVICER */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SPECIAL )  /* /SER.L */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN2 )    /* COINR */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN1 )    /* COINL */
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( tmek )
	PORT_INCLUDE( common )

#if (HACK_TMEK_CONTROLS)
	PORT_START("FAKE")      /* single digital joystick */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
#else

	PORT_START("AN1")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("AN2")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("AN3")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("AN4")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)
#endif
INPUT_PORTS_END


static INPUT_PORTS_START( primrage )
	PORT_INCLUDE( common )

	/* primrage uses one of the 4 action buttons as start */
	PORT_MODIFY( "P1_P2" )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x01000000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START("AN1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("AN4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout pflayout =
{
	8,8,
	RGN_FRAC(1,3),
	5,
	{ 0, 0, 1, 2, 3 },
	{ RGN_FRAC(1,3)+0, RGN_FRAC(1,3)+4, 0, 4, RGN_FRAC(1,3)+8, RGN_FRAC(1,3)+12, 8, 12 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};


static const gfx_layout pftoplayout =
{
	8,8,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(2,3)+0, RGN_FRAC(2,3)+4, 0, 0, 0, 0 },
	{ 3, 2, 1, 0, 11, 10, 9, 8 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};


static const gfx_layout anlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, 8, 12, 16, 20, 24, 28 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8 },
	32*8
};


static GFXDECODE_START( atarigt )
	GFXDECODE_ENTRY( "gfx1", 0, pflayout, 0x000, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, anlayout, 0x000, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, pftoplayout, 0x000, 64 )
GFXDECODE_END


static const atari_rle_objects_config modesc =
{
	0,          /* left clip coordinate */
	0,          /* right clip coordinate */
	0x0000,     /* base palette entry */

	{{ 0x7fff,0,0,0,0,0,0,0 }}, /* mask for the code index */
	{{ 0,0x0ff0,0,0,0,0,0,0 }}, /* mask for the color */
	{{ 0,0,0xffc0,0,0,0,0,0 }}, /* mask for the X position */
	{{ 0,0,0,0xffc0,0,0,0,0 }}, /* mask for the Y position */
	{{ 0,0,0,0,0xffff,0,0,0 }}, /* mask for the scale factor */
	{{ 0x8000,0,0,0,0,0,0,0 }}, /* mask for the horizontal flip */
	{{ 0,0,0,0,0,0,0x00ff,0 }}, /* mask for the order */
	{{ 0,0x0e00,0,0,0,0,0,0 }}, /* mask for the priority */
	{{ 0,0x8000,0,0,0,0,0,0 }}  /* mask for the VRAM target */
};



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( atarigt, atarigt_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68EC020, ATARI_CLOCK_50MHz/2)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", atarigen_state, video_int_gen)
	MCFG_CPU_PERIODIC_INT_DRIVER(atarigen_state, scanline_int_gen, 250)

	MCFG_MACHINE_RESET_OVERRIDE(atarigt_state,atarigt)

	MCFG_ATARI_EEPROM_2816_ADD("eeprom")

	/* video hardware */
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", atarigt)
	MCFG_PALETTE_ADD("palette", 32768)

	MCFG_TILEMAP_ADD_CUSTOM("playfield", "gfxdecode", 2, atarigt_state, get_playfield_tile_info, 8,8, atarigt_playfield_scan, 128,64)
	MCFG_TILEMAP_ADD_STANDARD("alpha", "gfxdecode", 2, atarigt_state, get_alpha_tile_info, 8,8, SCAN_ROWS, 64, 32)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	/* note: these parameters are from published specs, not derived */
	/* the board uses a pair of GALs to determine H and V parameters */
	MCFG_SCREEN_RAW_PARAMS(ATARI_CLOCK_14MHz/2, 456, 0, 336, 262, 0, 240)
	MCFG_SCREEN_UPDATE_DRIVER(atarigt_state, screen_update_atarigt)

	MCFG_VIDEO_START_OVERRIDE(atarigt_state,atarigt)

	MCFG_ATARIRLE_ADD("rle", modesc)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tmek, atarigt )
	/* sound hardware */
	MCFG_DEVICE_ADD("cage", ATARI_CAGE, 0)
	MCFG_ATARI_CAGE_SPEEDUP(0x4fad)
	MCFG_ATARI_CAGE_IRQ_CALLBACK(WRITE8(atarigt_state,cage_irq_callback))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( primrage, atarigt )
	/* sound hardware */
	MCFG_DEVICE_ADD("cage", ATARI_CAGE, 0)
	MCFG_ATARI_CAGE_SPEEDUP(0x42f2)
	MCFG_ATARI_CAGE_IRQ_CALLBACK(WRITE8(atarigt_state,cage_irq_callback))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( primrage20, atarigt )
	/* sound hardware */
	MCFG_DEVICE_ADD("cage", ATARI_CAGE, 0)
	MCFG_ATARI_CAGE_SPEEDUP(0x48a4)
	MCFG_ATARI_CAGE_IRQ_CALLBACK(WRITE8(atarigt_state,cage_irq_callback))
MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( tmek )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "0044d", 0x00000, 0x20000, CRC(1cd62725) SHA1(7685794d9fbe3fe7a9978d12e489447b4fba5282) )
	ROM_LOAD32_BYTE( "0043d", 0x00001, 0x20000, CRC(82185051) SHA1(a21aad4f6ec948d9cd47efb89e7811c5c2e4850b) )
	ROM_LOAD32_BYTE( "0042d", 0x00002, 0x20000, CRC(ef9feda4) SHA1(9fb6e91d4c22e28ced61d0d1f28f5e43191c8762) )
	ROM_LOAD32_BYTE( "0041d", 0x00003, 0x20000, CRC(179da056) SHA1(5f7ddf44aab55beaf2c377b0c93279acb6273255) )

	ROM_REGION32_LE( 0x200000, "cageboot", 0 )  /* TMS320C31 boot ROM */
	ROM_LOAD32_BYTE( "0078c", 0x000000, 0x080000, CRC(ff5b979a) SHA1(deb8ee454b6b7c7bddb2ba0c808869e45b19e55f) )

	ROM_REGION32_LE( 0x1000000, "cage", 0 ) /* TMS320C31 sound ROMs */
	ROM_LOAD32_WORD( "0077",  0x400000, 0x200000, CRC(8f650f8b) SHA1(e3b48ff4e2093d709134b6bf62cecd101ab5cef4) )
	ROM_LOAD32_BYTE( "2501a", 0x400002, 0x080000, CRC(98e51103) SHA1(420d0aac6b1de1bd990b9e4219041192400299f8) )
	ROM_LOAD32_BYTE( "2500a", 0x400003, 0x080000, CRC(49c0136c) SHA1(1ad463b1e50df9843abb8c645cbe8a79e42cbb87) )
	ROM_LOAD32_BYTE( "2503a", 0x600002, 0x080000, CRC(4376f3eb) SHA1(fe3f1efec3e6b4da3d5a13611bad7e34306cc224) )
	ROM_LOAD32_BYTE( "2502a", 0x600003, 0x080000, CRC(a48e6a5f) SHA1(f9615ff587b60d07172fc44ce87ae0fb49cb02a0) )
	ROM_LOAD32_WORD( "0076",  0x800000, 0x200000, CRC(74dffe2d) SHA1(9436f69827050ad2f3be58f1cb57d7a06b75ab61) )
	ROM_LOAD32_WORD( "0074",  0x800002, 0x200000, CRC(8dfc6ce0) SHA1(5b0d4dd4cb7934f542e67217a2542a3c69558cea) )

	ROM_REGION( 0x300000, "gfx1", 0 )
	ROM_LOAD( "0250",  0x000000, 0x80000, CRC(56bd9f25) SHA1(a8161aeee274f28c41f82b6b3f63570970ee281d) ) /* playfield, planes 0-1 */
	ROM_LOAD( "0253a", 0x080000, 0x80000, CRC(23e2f83d) SHA1(804a17ce8768bd48cda853e55fc1f54ed7475968) )
	ROM_LOAD( "0251",  0x100000, 0x80000, CRC(0d3b08f7) SHA1(72ec2383011ef20e9054594279cc85fa55c3a9b2) ) /* playfield, planes 2-3 */
	ROM_LOAD( "0254a", 0x180000, 0x80000, CRC(448aea87) SHA1(8c9e367b2f8d06858d37a9239fb732c1379ec374) )
	ROM_LOAD( "0252",  0x200000, 0x80000, CRC(95a1c23b) SHA1(74eb69dcaebd7a7a03d8f7c9bf6183ece695e91d) ) /* playfield, planes 4-5 */
	ROM_LOAD( "0255a", 0x280000, 0x80000, CRC(f0fbb700) SHA1(3f0355b137f6426a07abab77f25e718c6102a16f) )

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "0045a", 0x000000, 0x20000, CRC(057a5304) SHA1(d44c0cf885a1324888b7e8118f124c0dae616859) ) /* alphanumerics */

	ROM_REGION16_BE( 0x1000000, "rle", 0 )
	ROM_LOAD16_BYTE( "0300", 0x000001, 0x100000, CRC(8367ddac) SHA1(9ca77962259284cef8a261b652ab1327817ee8d0) )
	ROM_LOAD16_BYTE( "0301", 0x000000, 0x100000, CRC(94524b5b) SHA1(db401fd7ba56658fcb614406672c02569d845930) )
	ROM_LOAD16_BYTE( "0302", 0x200001, 0x100000, CRC(c03f1aa7) SHA1(c68b52280d0695629c843b9c90f7a39713e063b0) )
	ROM_LOAD16_BYTE( "0303", 0x200000, 0x100000, CRC(3ac5b24f) SHA1(96c26cb3f17f4a383bf0a7be442c6199fbac8b4b) )
	ROM_LOAD16_BYTE( "0304", 0x400001, 0x100000, CRC(b053ef78) SHA1(30dd8c409ed7fbc12495829e680df9d7d1cf6c6c) )
	ROM_LOAD16_BYTE( "0305", 0x400000, 0x100000, CRC(b012b8e9) SHA1(89af30b49fad8424b00252c2ea3ef454a45a5622) )
	ROM_LOAD16_BYTE( "0306", 0x600001, 0x100000, CRC(d086f149) SHA1(92b5b7c01863a9fcc9b9b3990744da7ac107a324) )
	ROM_LOAD16_BYTE( "0307", 0x600000, 0x100000, CRC(49c1a541) SHA1(65169a8ed4cd5f77fec61252a72b7731d8e910e1) )
	ROM_LOAD16_BYTE( "0308", 0x800001, 0x100000, CRC(97033c8a) SHA1(c81d30a492dd0419193a68eea78ba5e6b12a3f9a) )
	ROM_LOAD16_BYTE( "0309", 0x800000, 0x100000, CRC(e095ecb3) SHA1(79b7d21096cc1abeb2d1bc45deab5dc42282a807) )
	ROM_LOAD16_BYTE( "0310", 0xa00001, 0x100000, CRC(e056a0c3) SHA1(0a87e4078371e1b52e9418a4824f2d37cb07a649) )
	ROM_LOAD16_BYTE( "0311", 0xa00000, 0x100000, CRC(05afb2dc) SHA1(db186bfde255aa57f8e80bdc92c9be6d8c366bb9) )
	ROM_LOAD16_BYTE( "0312", 0xc00001, 0x100000, CRC(cc224dae) SHA1(0d57382b53920172ceaba62a0f690fc04aedfddc) )
	ROM_LOAD16_BYTE( "0313", 0xc00000, 0x100000, CRC(a8cf049d) SHA1(d130e1f94d2a2819ed46c45834aa1b1cd86ab839) )
	ROM_LOAD16_BYTE( "0314", 0xe00001, 0x100000, CRC(4f01db8d) SHA1(c18c72f1ccbe6ff18576592548c960f9ce357016) )
	ROM_LOAD16_BYTE( "0315", 0xe00000, 0x100000, CRC(28e97d06) SHA1(ef115f393c568822cb2cb3cca92c7656e1ee07f9) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "0001a",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )  /* microcode for growth renderer */
	ROM_LOAD( "0001b",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "0001c",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END


ROM_START( tmek51p )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "prog0", 0x00000, 0x20000, CRC(df16ffc1) SHA1(9b63493bc9fae4c6c58883050921fed1ae8f0cf3) )
	ROM_LOAD32_BYTE( "prog1", 0x00001, 0x20000, CRC(a5ab6b62) SHA1(7c12c6f78e795b61c7dd40b871e7f1461c199cab) )
	ROM_LOAD32_BYTE( "prog2", 0x00002, 0x20000, CRC(bdcf5942) SHA1(21c54694bfe1e5663e67a54afed2a0f37b0f00de) )
	ROM_LOAD32_BYTE( "prog3", 0x00003, 0x20000, CRC(7b59022a) SHA1(7395063ff0ecda0453dc7d981ca0b90b8411b715) )

	ROM_REGION32_LE( 0x200000, "cageboot", 0 )  /* TMS320C31 boot ROM */
	ROM_LOAD32_BYTE( "0078c", 0x000000, 0x080000, CRC(ff5b979a) SHA1(deb8ee454b6b7c7bddb2ba0c808869e45b19e55f) )

	ROM_REGION32_LE( 0x1000000, "cage", 0 ) /* TMS320C31 sound ROMs */
	ROM_LOAD32_WORD( "0077",  0x400000, 0x200000, CRC(8f650f8b) SHA1(e3b48ff4e2093d709134b6bf62cecd101ab5cef4) )
	ROM_LOAD32_BYTE( "2501a", 0x400002, 0x080000, CRC(98e51103) SHA1(420d0aac6b1de1bd990b9e4219041192400299f8) )
	ROM_LOAD32_BYTE( "2500a", 0x400003, 0x080000, CRC(49c0136c) SHA1(1ad463b1e50df9843abb8c645cbe8a79e42cbb87) )
	ROM_LOAD32_BYTE( "2503a", 0x600002, 0x080000, CRC(4376f3eb) SHA1(fe3f1efec3e6b4da3d5a13611bad7e34306cc224) )
	ROM_LOAD32_BYTE( "2502a", 0x600003, 0x080000, CRC(a48e6a5f) SHA1(f9615ff587b60d07172fc44ce87ae0fb49cb02a0) )
	ROM_LOAD32_WORD( "0076",  0x800000, 0x200000, CRC(74dffe2d) SHA1(9436f69827050ad2f3be58f1cb57d7a06b75ab61) )
	ROM_LOAD32_WORD( "0074",  0x800002, 0x200000, CRC(8dfc6ce0) SHA1(5b0d4dd4cb7934f542e67217a2542a3c69558cea) )

	ROM_REGION( 0x300000, "gfx1", 0 )
	ROM_LOAD( "0250",  0x000000, 0x80000, CRC(56bd9f25) SHA1(a8161aeee274f28c41f82b6b3f63570970ee281d) ) /* playfield, planes 0-1 */
	ROM_LOAD( "0253a", 0x080000, 0x80000, CRC(23e2f83d) SHA1(804a17ce8768bd48cda853e55fc1f54ed7475968) )
	ROM_LOAD( "0251",  0x100000, 0x80000, CRC(0d3b08f7) SHA1(72ec2383011ef20e9054594279cc85fa55c3a9b2) ) /* playfield, planes 2-3 */
	ROM_LOAD( "0254a", 0x180000, 0x80000, CRC(448aea87) SHA1(8c9e367b2f8d06858d37a9239fb732c1379ec374) )
	ROM_LOAD( "0252",  0x200000, 0x80000, CRC(95a1c23b) SHA1(74eb69dcaebd7a7a03d8f7c9bf6183ece695e91d) ) /* playfield, planes 4-5 */
	ROM_LOAD( "0255a", 0x280000, 0x80000, CRC(f0fbb700) SHA1(3f0355b137f6426a07abab77f25e718c6102a16f) )

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "0045a", 0x000000, 0x20000, CRC(057a5304) SHA1(d44c0cf885a1324888b7e8118f124c0dae616859) ) /* alphanumerics */

	ROM_REGION16_BE( 0x1000000, "rle", 0 )
	ROM_LOAD16_BYTE( "0300", 0x000001, 0x100000, CRC(8367ddac) SHA1(9ca77962259284cef8a261b652ab1327817ee8d0) )
	ROM_LOAD16_BYTE( "0301", 0x000000, 0x100000, CRC(94524b5b) SHA1(db401fd7ba56658fcb614406672c02569d845930) )
	ROM_LOAD16_BYTE( "0302", 0x200001, 0x100000, CRC(c03f1aa7) SHA1(c68b52280d0695629c843b9c90f7a39713e063b0) )
	ROM_LOAD16_BYTE( "0303", 0x200000, 0x100000, CRC(3ac5b24f) SHA1(96c26cb3f17f4a383bf0a7be442c6199fbac8b4b) )
	ROM_LOAD16_BYTE( "0304", 0x400001, 0x100000, CRC(b053ef78) SHA1(30dd8c409ed7fbc12495829e680df9d7d1cf6c6c) )
	ROM_LOAD16_BYTE( "0305", 0x400000, 0x100000, CRC(b012b8e9) SHA1(89af30b49fad8424b00252c2ea3ef454a45a5622) )
	ROM_LOAD16_BYTE( "0306", 0x600001, 0x100000, CRC(d086f149) SHA1(92b5b7c01863a9fcc9b9b3990744da7ac107a324) )
	ROM_LOAD16_BYTE( "0307", 0x600000, 0x100000, CRC(49c1a541) SHA1(65169a8ed4cd5f77fec61252a72b7731d8e910e1) )
	ROM_LOAD16_BYTE( "0308", 0x800001, 0x100000, CRC(97033c8a) SHA1(c81d30a492dd0419193a68eea78ba5e6b12a3f9a) )
	ROM_LOAD16_BYTE( "0309", 0x800000, 0x100000, CRC(e095ecb3) SHA1(79b7d21096cc1abeb2d1bc45deab5dc42282a807) )
	ROM_LOAD16_BYTE( "0310", 0xa00001, 0x100000, CRC(e056a0c3) SHA1(0a87e4078371e1b52e9418a4824f2d37cb07a649) )
	ROM_LOAD16_BYTE( "0311", 0xa00000, 0x100000, CRC(05afb2dc) SHA1(db186bfde255aa57f8e80bdc92c9be6d8c366bb9) )
	ROM_LOAD16_BYTE( "0312", 0xc00001, 0x100000, CRC(cc224dae) SHA1(0d57382b53920172ceaba62a0f690fc04aedfddc) )
	ROM_LOAD16_BYTE( "0313", 0xc00000, 0x100000, CRC(a8cf049d) SHA1(d130e1f94d2a2819ed46c45834aa1b1cd86ab839) )
	ROM_LOAD16_BYTE( "0314", 0xe00001, 0x100000, CRC(4f01db8d) SHA1(c18c72f1ccbe6ff18576592548c960f9ce357016) )
	ROM_LOAD16_BYTE( "0315", 0xe00000, 0x100000, CRC(28e97d06) SHA1(ef115f393c568822cb2cb3cca92c7656e1ee07f9) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "0001a",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )  /* microcode for growth renderer */
	ROM_LOAD( "0001b",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "0001c",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END


ROM_START( tmek45 )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "0044c", 0x00000, 0x20000, CRC(6079fc3f) SHA1(f8caedd9708108ce7c2d1300661dadaf2e3a6319) )
	ROM_LOAD32_BYTE( "0043c", 0x00001, 0x20000, CRC(23d6388b) SHA1(6144c845ae28777809776d863c0ed5ee3dff5c58) )
	ROM_LOAD32_BYTE( "0042c", 0x00002, 0x20000, CRC(ba8745be) SHA1(139a3132ea2c69e37e63868402fcf10852953e9b) )
	ROM_LOAD32_BYTE( "0041c", 0x00003, 0x20000, CRC(0285bc17) SHA1(346d9fcbea4b22986be04971074531bc0c014c79) )

	ROM_REGION32_LE( 0x200000, "cageboot", 0 )  /* TMS320C31 boot ROM */
	ROM_LOAD32_BYTE( "0078b", 0x000000, 0x080000, CRC(a952771c) SHA1(49982ea864a99c07f45886ada7e2c9427a75f775) )

	ROM_REGION32_LE( 0x1000000, "cage", 0 ) /* TMS320C31 sound ROMs */
	ROM_LOAD32_WORD( "0077",  0x400000, 0x200000, CRC(8f650f8b) SHA1(e3b48ff4e2093d709134b6bf62cecd101ab5cef4) )
	ROM_LOAD32_BYTE( "2501a", 0x400002, 0x080000, CRC(98e51103) SHA1(420d0aac6b1de1bd990b9e4219041192400299f8) )
	ROM_LOAD32_BYTE( "2500a", 0x400003, 0x080000, CRC(49c0136c) SHA1(1ad463b1e50df9843abb8c645cbe8a79e42cbb87) )
	ROM_LOAD32_BYTE( "2503a", 0x600002, 0x080000, CRC(4376f3eb) SHA1(fe3f1efec3e6b4da3d5a13611bad7e34306cc224) )
	ROM_LOAD32_BYTE( "2502a", 0x600003, 0x080000, CRC(a48e6a5f) SHA1(f9615ff587b60d07172fc44ce87ae0fb49cb02a0) )
	ROM_LOAD32_WORD( "0076",  0x800000, 0x200000, CRC(74dffe2d) SHA1(9436f69827050ad2f3be58f1cb57d7a06b75ab61) )
	ROM_LOAD32_WORD( "0074",  0x800002, 0x200000, CRC(8dfc6ce0) SHA1(5b0d4dd4cb7934f542e67217a2542a3c69558cea) )

	ROM_REGION( 0x300000, "gfx1", 0 )
	ROM_LOAD( "0250",  0x000000, 0x80000, CRC(56bd9f25) SHA1(a8161aeee274f28c41f82b6b3f63570970ee281d) ) /* playfield, planes 0-1 */
	ROM_LOAD( "0253a", 0x080000, 0x80000, CRC(23e2f83d) SHA1(804a17ce8768bd48cda853e55fc1f54ed7475968) )
	ROM_LOAD( "0251",  0x100000, 0x80000, CRC(0d3b08f7) SHA1(72ec2383011ef20e9054594279cc85fa55c3a9b2) ) /* playfield, planes 2-3 */
	ROM_LOAD( "0254a", 0x180000, 0x80000, CRC(448aea87) SHA1(8c9e367b2f8d06858d37a9239fb732c1379ec374) )
	ROM_LOAD( "0252",  0x200000, 0x80000, CRC(95a1c23b) SHA1(74eb69dcaebd7a7a03d8f7c9bf6183ece695e91d) ) /* playfield, planes 4-5 */
	ROM_LOAD( "0255a", 0x280000, 0x80000, CRC(f0fbb700) SHA1(3f0355b137f6426a07abab77f25e718c6102a16f) )

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "0045a", 0x000000, 0x20000, CRC(057a5304) SHA1(d44c0cf885a1324888b7e8118f124c0dae616859) ) /* alphanumerics */

	ROM_REGION16_BE( 0x1000000, "rle", 0 )
	ROM_LOAD16_BYTE( "0300", 0x000001, 0x100000, CRC(8367ddac) SHA1(9ca77962259284cef8a261b652ab1327817ee8d0) )
	ROM_LOAD16_BYTE( "0301", 0x000000, 0x100000, CRC(94524b5b) SHA1(db401fd7ba56658fcb614406672c02569d845930) )
	ROM_LOAD16_BYTE( "0302", 0x200001, 0x100000, CRC(c03f1aa7) SHA1(c68b52280d0695629c843b9c90f7a39713e063b0) )
	ROM_LOAD16_BYTE( "0303", 0x200000, 0x100000, CRC(3ac5b24f) SHA1(96c26cb3f17f4a383bf0a7be442c6199fbac8b4b) )
	ROM_LOAD16_BYTE( "0304", 0x400001, 0x100000, CRC(b053ef78) SHA1(30dd8c409ed7fbc12495829e680df9d7d1cf6c6c) )
	ROM_LOAD16_BYTE( "0305", 0x400000, 0x100000, CRC(b012b8e9) SHA1(89af30b49fad8424b00252c2ea3ef454a45a5622) )
	ROM_LOAD16_BYTE( "0306", 0x600001, 0x100000, CRC(d086f149) SHA1(92b5b7c01863a9fcc9b9b3990744da7ac107a324) )
	ROM_LOAD16_BYTE( "0307", 0x600000, 0x100000, CRC(49c1a541) SHA1(65169a8ed4cd5f77fec61252a72b7731d8e910e1) )
	ROM_LOAD16_BYTE( "0308", 0x800001, 0x100000, CRC(97033c8a) SHA1(c81d30a492dd0419193a68eea78ba5e6b12a3f9a) )
	ROM_LOAD16_BYTE( "0309", 0x800000, 0x100000, CRC(e095ecb3) SHA1(79b7d21096cc1abeb2d1bc45deab5dc42282a807) )
	ROM_LOAD16_BYTE( "0310", 0xa00001, 0x100000, CRC(e056a0c3) SHA1(0a87e4078371e1b52e9418a4824f2d37cb07a649) )
	ROM_LOAD16_BYTE( "0311", 0xa00000, 0x100000, CRC(05afb2dc) SHA1(db186bfde255aa57f8e80bdc92c9be6d8c366bb9) )
	ROM_LOAD16_BYTE( "0312", 0xc00001, 0x100000, CRC(cc224dae) SHA1(0d57382b53920172ceaba62a0f690fc04aedfddc) )
	ROM_LOAD16_BYTE( "0313", 0xc00000, 0x100000, CRC(a8cf049d) SHA1(d130e1f94d2a2819ed46c45834aa1b1cd86ab839) )
	ROM_LOAD16_BYTE( "0314", 0xe00001, 0x100000, CRC(4f01db8d) SHA1(c18c72f1ccbe6ff18576592548c960f9ce357016) )
	ROM_LOAD16_BYTE( "0315", 0xe00000, 0x100000, CRC(28e97d06) SHA1(ef115f393c568822cb2cb3cca92c7656e1ee07f9) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "0001a",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )  /* microcode for growth renderer */
	ROM_LOAD( "0001b",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "0001c",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END


ROM_START( tmek44 )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "0044b", 0x00000, 0x20000, CRC(f7cc0590) SHA1(b60df7535b3ce0c94ca14a2430a3a414d77fc751) )
	ROM_LOAD32_BYTE( "0043b", 0x00001, 0x20000, CRC(9fb8e072) SHA1(5904bebf17f31922998f9b31e388abc2ac244385) )
	ROM_LOAD32_BYTE( "0042b", 0x00002, 0x20000, CRC(ce68a9b3) SHA1(47b7a0ac8cce3d40f3f7559ec1b137dfdeaf1d83) )
	ROM_LOAD32_BYTE( "0041b", 0x00003, 0x20000, CRC(b71ec759) SHA1(d4bed4bbab2c3bd278da4cd0f53580d7f66d8152) )

	ROM_REGION32_LE( 0x200000, "cageboot", 0 )  /* TMS320C31 boot ROM */
	ROM_LOAD32_BYTE( "0078a", 0x000000, 0x080000, CRC(314d736f) SHA1(b23946fde6ea47d6a6e3430a9df4b06d453a94c8) )

	ROM_REGION32_LE( 0x1000000, "cage", 0 ) /* TMS320C31 sound ROMs */
	ROM_LOAD32_WORD( "0077",  0x400000, 0x200000, CRC(8f650f8b) SHA1(e3b48ff4e2093d709134b6bf62cecd101ab5cef4) )
	ROM_LOAD32_BYTE( "2501a", 0x400002, 0x080000, CRC(98e51103) SHA1(420d0aac6b1de1bd990b9e4219041192400299f8) )
	ROM_LOAD32_BYTE( "2500a", 0x400003, 0x080000, CRC(49c0136c) SHA1(1ad463b1e50df9843abb8c645cbe8a79e42cbb87) )
	ROM_LOAD32_BYTE( "2503a", 0x600002, 0x080000, CRC(4376f3eb) SHA1(fe3f1efec3e6b4da3d5a13611bad7e34306cc224) )
	ROM_LOAD32_BYTE( "2502a", 0x600003, 0x080000, CRC(a48e6a5f) SHA1(f9615ff587b60d07172fc44ce87ae0fb49cb02a0) )
	ROM_LOAD32_WORD( "0076",  0x800000, 0x200000, CRC(74dffe2d) SHA1(9436f69827050ad2f3be58f1cb57d7a06b75ab61) )
	ROM_LOAD32_WORD( "0074",  0x800002, 0x200000, CRC(8dfc6ce0) SHA1(5b0d4dd4cb7934f542e67217a2542a3c69558cea) )

	ROM_REGION( 0x300000, "gfx1", 0 )
	ROM_LOAD( "0250",  0x000000, 0x80000, CRC(56bd9f25) SHA1(a8161aeee274f28c41f82b6b3f63570970ee281d) ) /* playfield, planes 0-1 */
	ROM_LOAD( "0253a", 0x080000, 0x80000, CRC(23e2f83d) SHA1(804a17ce8768bd48cda853e55fc1f54ed7475968) )
	ROM_LOAD( "0251",  0x100000, 0x80000, CRC(0d3b08f7) SHA1(72ec2383011ef20e9054594279cc85fa55c3a9b2) ) /* playfield, planes 2-3 */
	ROM_LOAD( "0254a", 0x180000, 0x80000, CRC(448aea87) SHA1(8c9e367b2f8d06858d37a9239fb732c1379ec374) )
	ROM_LOAD( "0252",  0x200000, 0x80000, CRC(95a1c23b) SHA1(74eb69dcaebd7a7a03d8f7c9bf6183ece695e91d) ) /* playfield, planes 4-5 */
	ROM_LOAD( "0255a", 0x280000, 0x80000, CRC(f0fbb700) SHA1(3f0355b137f6426a07abab77f25e718c6102a16f) )

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "0045a", 0x000000, 0x20000, CRC(057a5304) SHA1(d44c0cf885a1324888b7e8118f124c0dae616859) ) /* alphanumerics */

	ROM_REGION16_BE( 0x1000000, "rle", 0 )
	ROM_LOAD16_BYTE( "0300", 0x000001, 0x100000, CRC(8367ddac) SHA1(9ca77962259284cef8a261b652ab1327817ee8d0) )
	ROM_LOAD16_BYTE( "0301", 0x000000, 0x100000, CRC(94524b5b) SHA1(db401fd7ba56658fcb614406672c02569d845930) )
	ROM_LOAD16_BYTE( "0302", 0x200001, 0x100000, CRC(c03f1aa7) SHA1(c68b52280d0695629c843b9c90f7a39713e063b0) )
	ROM_LOAD16_BYTE( "0303", 0x200000, 0x100000, CRC(3ac5b24f) SHA1(96c26cb3f17f4a383bf0a7be442c6199fbac8b4b) )
	ROM_LOAD16_BYTE( "0304", 0x400001, 0x100000, CRC(b053ef78) SHA1(30dd8c409ed7fbc12495829e680df9d7d1cf6c6c) )
	ROM_LOAD16_BYTE( "0305", 0x400000, 0x100000, CRC(b012b8e9) SHA1(89af30b49fad8424b00252c2ea3ef454a45a5622) )
	ROM_LOAD16_BYTE( "0306", 0x600001, 0x100000, CRC(d086f149) SHA1(92b5b7c01863a9fcc9b9b3990744da7ac107a324) )
	ROM_LOAD16_BYTE( "0307", 0x600000, 0x100000, CRC(49c1a541) SHA1(65169a8ed4cd5f77fec61252a72b7731d8e910e1) )
	ROM_LOAD16_BYTE( "0308", 0x800001, 0x100000, CRC(97033c8a) SHA1(c81d30a492dd0419193a68eea78ba5e6b12a3f9a) )
	ROM_LOAD16_BYTE( "0309", 0x800000, 0x100000, CRC(e095ecb3) SHA1(79b7d21096cc1abeb2d1bc45deab5dc42282a807) )
	ROM_LOAD16_BYTE( "0310", 0xa00001, 0x100000, CRC(e056a0c3) SHA1(0a87e4078371e1b52e9418a4824f2d37cb07a649) )
	ROM_LOAD16_BYTE( "0311", 0xa00000, 0x100000, CRC(05afb2dc) SHA1(db186bfde255aa57f8e80bdc92c9be6d8c366bb9) )
	ROM_LOAD16_BYTE( "0312", 0xc00001, 0x100000, CRC(cc224dae) SHA1(0d57382b53920172ceaba62a0f690fc04aedfddc) )
	ROM_LOAD16_BYTE( "0313", 0xc00000, 0x100000, CRC(a8cf049d) SHA1(d130e1f94d2a2819ed46c45834aa1b1cd86ab839) )
	ROM_LOAD16_BYTE( "0314", 0xe00001, 0x100000, CRC(4f01db8d) SHA1(c18c72f1ccbe6ff18576592548c960f9ce357016) )
	ROM_LOAD16_BYTE( "0315", 0xe00000, 0x100000, CRC(28e97d06) SHA1(ef115f393c568822cb2cb3cca92c7656e1ee07f9) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "0001a",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )  /* microcode for growth renderer */
	ROM_LOAD( "0001b",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "0001c",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END


ROM_START( tmek20 )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "pgm0", 0x00000, 0x20000, CRC(f5f7f7be) SHA1(66be472e7c0ef26e2ce2b45488a8e4cfc1b0f80a) )
	ROM_LOAD32_BYTE( "pgm1", 0x00001, 0x20000, CRC(284f7971) SHA1(5327f6368abd2ab9740a5150a8660c420f750476) )
	ROM_LOAD32_BYTE( "pgm2", 0x00002, 0x20000, CRC(ce9a77d4) SHA1(025143b59d85180286086940b05c8e5ea0b4a7fe) )
	ROM_LOAD32_BYTE( "pgm3", 0x00003, 0x20000, CRC(28b0e210) SHA1(7567671beecc7d30e9d4b61cf7d3448bb1dbb072) )

	ROM_REGION32_LE( 0x200000, "cageboot", 0 )  /* TMS320C31 boot ROM */
	ROM_LOAD32_BYTE( "0078", 0x000000, 0x080000, BAD_DUMP CRC(314d736f) SHA1(b23946fde6ea47d6a6e3430a9df4b06d453a94c8) ) // not dumped from this pcb, rom taken from another set instead

	ROM_REGION32_LE( 0x1000000, "cage", 0 ) /* TMS320C31 sound ROMs */
	ROM_LOAD32_WORD( "0077",  0x400000, 0x200000, CRC(8f650f8b) SHA1(e3b48ff4e2093d709134b6bf62cecd101ab5cef4) )
	ROM_LOAD32_BYTE( "2501a", 0x400002, 0x080000, CRC(98e51103) SHA1(420d0aac6b1de1bd990b9e4219041192400299f8) )
	ROM_LOAD32_BYTE( "2500a", 0x400003, 0x080000, CRC(49c0136c) SHA1(1ad463b1e50df9843abb8c645cbe8a79e42cbb87) )
	ROM_LOAD32_BYTE( "2503a", 0x600002, 0x080000, CRC(4376f3eb) SHA1(fe3f1efec3e6b4da3d5a13611bad7e34306cc224) )
	ROM_LOAD32_BYTE( "2502a", 0x600003, 0x080000, CRC(a48e6a5f) SHA1(f9615ff587b60d07172fc44ce87ae0fb49cb02a0) )
	ROM_LOAD32_WORD( "0076",  0x800000, 0x200000, CRC(74dffe2d) SHA1(9436f69827050ad2f3be58f1cb57d7a06b75ab61) )
	ROM_LOAD32_WORD( "0074",  0x800002, 0x200000, CRC(8dfc6ce0) SHA1(5b0d4dd4cb7934f542e67217a2542a3c69558cea) )

	ROM_REGION( 0x300000, "gfx1", 0 )
	ROM_LOAD( "0250",  0x000000, 0x80000, CRC(56bd9f25) SHA1(a8161aeee274f28c41f82b6b3f63570970ee281d) ) /* playfield, planes 0-1 */
	ROM_LOAD( "0253a", 0x080000, 0x80000, CRC(23e2f83d) SHA1(804a17ce8768bd48cda853e55fc1f54ed7475968) )
	ROM_LOAD( "0251",  0x100000, 0x80000, CRC(0d3b08f7) SHA1(72ec2383011ef20e9054594279cc85fa55c3a9b2) ) /* playfield, planes 2-3 */
	ROM_LOAD( "0254a", 0x180000, 0x80000, CRC(448aea87) SHA1(8c9e367b2f8d06858d37a9239fb732c1379ec374) )
	ROM_LOAD( "0252",  0x200000, 0x80000, CRC(95a1c23b) SHA1(74eb69dcaebd7a7a03d8f7c9bf6183ece695e91d) ) /* playfield, planes 4-5 */
	ROM_LOAD( "0255a", 0x280000, 0x80000, CRC(f0fbb700) SHA1(3f0355b137f6426a07abab77f25e718c6102a16f) )

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "alpha", 0x000000, 0x20000, CRC(8f57a604) SHA1(f076636430ff73ea11e4687ef7b21a7bac1d8e34) ) /* alphanumerics */

	ROM_REGION16_BE( 0x1000000, "rle", 0 )
	ROM_LOAD16_BYTE( "0300", 0x000001, 0x100000, CRC(8367ddac) SHA1(9ca77962259284cef8a261b652ab1327817ee8d0) )
	ROM_LOAD16_BYTE( "0301", 0x000000, 0x100000, CRC(94524b5b) SHA1(db401fd7ba56658fcb614406672c02569d845930) )
	ROM_LOAD16_BYTE( "0302", 0x200001, 0x100000, CRC(c03f1aa7) SHA1(c68b52280d0695629c843b9c90f7a39713e063b0) )
	ROM_LOAD16_BYTE( "0303", 0x200000, 0x100000, CRC(3ac5b24f) SHA1(96c26cb3f17f4a383bf0a7be442c6199fbac8b4b) )
	ROM_LOAD16_BYTE( "0304", 0x400001, 0x100000, CRC(b053ef78) SHA1(30dd8c409ed7fbc12495829e680df9d7d1cf6c6c) )
	ROM_LOAD16_BYTE( "0305", 0x400000, 0x100000, CRC(b012b8e9) SHA1(89af30b49fad8424b00252c2ea3ef454a45a5622) )
	ROM_LOAD16_BYTE( "0306", 0x600001, 0x100000, CRC(d086f149) SHA1(92b5b7c01863a9fcc9b9b3990744da7ac107a324) )
	ROM_LOAD16_BYTE( "0307", 0x600000, 0x100000, CRC(49c1a541) SHA1(65169a8ed4cd5f77fec61252a72b7731d8e910e1) )
	ROM_LOAD16_BYTE( "0308", 0x800001, 0x100000, CRC(97033c8a) SHA1(c81d30a492dd0419193a68eea78ba5e6b12a3f9a) )
	ROM_LOAD16_BYTE( "0309", 0x800000, 0x100000, CRC(e095ecb3) SHA1(79b7d21096cc1abeb2d1bc45deab5dc42282a807) )
	ROM_LOAD16_BYTE( "0310", 0xa00001, 0x100000, CRC(e056a0c3) SHA1(0a87e4078371e1b52e9418a4824f2d37cb07a649) )
	ROM_LOAD16_BYTE( "0311", 0xa00000, 0x100000, CRC(05afb2dc) SHA1(db186bfde255aa57f8e80bdc92c9be6d8c366bb9) )
	ROM_LOAD16_BYTE( "0312", 0xc00001, 0x100000, CRC(cc224dae) SHA1(0d57382b53920172ceaba62a0f690fc04aedfddc) )
	ROM_LOAD16_BYTE( "0313", 0xc00000, 0x100000, CRC(a8cf049d) SHA1(d130e1f94d2a2819ed46c45834aa1b1cd86ab839) )
	ROM_LOAD16_BYTE( "0314", 0xe00001, 0x100000, CRC(4f01db8d) SHA1(c18c72f1ccbe6ff18576592548c960f9ce357016) )
	ROM_LOAD16_BYTE( "0315", 0xe00000, 0x100000, CRC(28e97d06) SHA1(ef115f393c568822cb2cb3cca92c7656e1ee07f9) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "0001a",  0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )  /* microcode for growth renderer */
	ROM_LOAD( "0001b",  0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "0001c",  0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END


ROM_START( primrage )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136102-1044b.29l", 0x000000, 0x80000, CRC(35c9c34b) SHA1(4bd1d35cc7c68574819afd648405eedb8db25b4c) )
	ROM_LOAD32_BYTE( "136102-1043b.28l", 0x000001, 0x80000, CRC(86322829) SHA1(e0e72888def0931d078921f099bae6788738a291) )
	ROM_LOAD32_BYTE( "136102-1042b.27l", 0x000002, 0x80000, CRC(750e8095) SHA1(4660637136b1a25169d8c43646c8b87081763987) )
	ROM_LOAD32_BYTE( "136102-1041b.25l", 0x000003, 0x80000, CRC(6a90d283) SHA1(7c18c97cb5e5cdd26a52cd6bc099fbce87055311) )

	ROM_REGION32_LE( 0x200000, "cageboot", 0 )  /* TMS320C31 boot ROM */
	ROM_LOAD32_BYTE( "136102-1078a.11a", 0x000000, 0x080000, CRC(0656435f) SHA1(f8e498171e754eb8703dad6b2351509bbb27e06b) )

	ROM_REGION32_LE( 0x1000000, "cage", 0 ) /* TMS320C31 sound ROMs */
	ROM_LOAD32_WORD( "136102-0075",  0x400000, 0x200000, CRC(b685a88e) SHA1(998b8fe54971f6cd96e4c22b19e3831f29d8172d) )
	ROM_LOAD32_WORD( "136102-0077",  0x400002, 0x200000, CRC(3283cea8) SHA1(fb7333ca951053a56c501f2ce0eb197c8fcafaf7) )

	ROM_REGION( 0x300000, "gfx1", 0 )
	ROM_LOAD( "136102-0050a.25n", 0x000000, 0x80000, CRC(66896e8f) SHA1(7675b24c15ca0608f11f2a7b8d70717adb10924c) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136102-0051a.27n", 0x100000, 0x80000, CRC(fb5b3e7b) SHA1(f43fe4b5c4bbea10da46b60c644f586fb391355d) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136102-0052a.28n", 0x200000, 0x80000, CRC(cbe38670) SHA1(0780e599007851f6d37cdd8c701d01cb1ae48b9d) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136102-1045b.23p", 0x000000, 0x20000, CRC(1d3260bf) SHA1(85d9db8499cbe180c8d52710f3cfe64453a530ff) ) /* alphanumerics */

	ROM_REGION16_BE( 0x2000000, "rle", 0 )
	ROM_LOAD16_BYTE( "136102-1100a.2v",    0x0000001, 0x080000, CRC(6e9c80b5) SHA1(ec724011527dd8707c733211b1a6c51b22f580c7) )
	ROM_LOAD16_BYTE( "136102-1101a.2w",    0x0000000, 0x080000, CRC(bb7ee624) SHA1(0de6385aee7d25b41fd5bf232e44e5da536504ac) )
	ROM_LOAD16_BYTE( "136102-0332.mol1.0", 0x0800001, 0x100000, CRC(610cfcb4) SHA1(bed1bd0d11c0a7cc48d020fc0acec34daf48c5ac) )
	ROM_LOAD16_BYTE( "136102-0333.moh1.0", 0x0800000, 0x100000, CRC(3320448e) SHA1(aef42328bf72fca5c04bfed1ea41100bb5aafeaa) )
	ROM_LOAD16_BYTE( "136102-0334.mol1.1", 0x0a00001, 0x100000, CRC(be3acb6f) SHA1(664cb4cd4d325577ab0cbe0cf48870a9f4706573) )
	ROM_LOAD16_BYTE( "136102-0335.moh1.1", 0x0a00000, 0x100000, CRC(e4f6e87a) SHA1(2a3f8ff46b289c25cd4ca2a1369b14613f48e964) )
	ROM_LOAD16_BYTE( "136102-0336.mol1.2", 0x0c00001, 0x100000, CRC(a78a8525) SHA1(69c3da4d45b0f09f5bdabcedd238b82efab48a70) )
	ROM_LOAD16_BYTE( "136102-0337.moh1.2", 0x0c00000, 0x100000, CRC(73fdd050) SHA1(63c67187953d2dab93a260e548ef5965e7cba4e8) )
	ROM_LOAD16_BYTE( "136102-0338.mol1.3", 0x0e00001, 0x100000, CRC(fa19cae6) SHA1(7d0560516971f32835329a17450c7561631a27d1) )
	ROM_LOAD16_BYTE( "136102-0339.moh1.3", 0x0e00000, 0x100000, CRC(e0cd1393) SHA1(0de59d04165d64320512936c194db19cca6455fd) )
	ROM_LOAD16_BYTE( "136102-0316.mol0.0", 0x1000001, 0x100000, CRC(9301c672) SHA1(a8971049c857ae283a95b257dd0d6aaff6d787cd) )
	ROM_LOAD16_BYTE( "136102-0317.moh0.0", 0x1000000, 0x100000, CRC(9e3b831a) SHA1(b799e57bea9522cb83f9aa7ea38a17b1d8273b8d) )
	ROM_LOAD16_BYTE( "136102-0318.mol0.1", 0x1200001, 0x100000, CRC(8523db5d) SHA1(f2476aa26b1a7cbe7510994d92eb209fda65593d) )
	ROM_LOAD16_BYTE( "136102-0319.moh0.1", 0x1200000, 0x100000, CRC(42f22e4b) SHA1(2a1a6f0a7aca7b7b64bce0bd54eb4cb23a2336b1) )
	ROM_LOAD16_BYTE( "136102-0320.mol0.2", 0x1400001, 0x100000, CRC(21369d13) SHA1(28e03595c098fd9bec6f7316180d17905a51a51b) )
	ROM_LOAD16_BYTE( "136102-0321.moh0.2", 0x1400000, 0x100000, CRC(3b7d498a) SHA1(804e9e1567bf97e8dae3b9444428254ced8b60da) )
	ROM_LOAD16_BYTE( "136102-0322.mol0.3", 0x1600001, 0x100000, CRC(05e9f407) SHA1(fa25a893d4cb805df02d7d12df4dbabefb3114a2) )
	ROM_LOAD16_BYTE( "136102-0323.moh0.3", 0x1600000, 0x100000, CRC(603f3bb6) SHA1(d7c22dc900d9edc36d8f211df67a206d14637fab) )
	ROM_LOAD16_BYTE( "136102-0324.mol0.4", 0x1800001, 0x100000, CRC(3c37769f) SHA1(ca0306a439949d2a0305cc0cf05808a58e84084c) )
	ROM_LOAD16_BYTE( "136102-0325.moh0.4", 0x1800000, 0x100000, CRC(f43321e3) SHA1(8bb4dd4a5d5400b17052d50dca9078211dc6b861) )
	ROM_LOAD16_BYTE( "136102-0326.mol0.5", 0x1a00001, 0x100000, CRC(63d4ccea) SHA1(340fced6998a8ae6fd285d8fe666f5f1e4b6bfaf) )
	ROM_LOAD16_BYTE( "136102-0327.moh0.5", 0x1a00000, 0x100000, CRC(9f4806d5) SHA1(76e9f1a47e7fa45e834fa8739528f1e3c54b14dc) )
	ROM_LOAD16_BYTE( "136102-0328.mol0.6", 0x1c00001, 0x100000, CRC(a08d73e1) SHA1(25a58777f15e9550111447b47a98762fd6bb498d) )
	ROM_LOAD16_BYTE( "136102-0329.moh0.6", 0x1c00000, 0x100000, CRC(eff3d2cd) SHA1(8532568b5fd91d2b738947e9cd575a4eb2a03be2) )
	ROM_LOAD16_BYTE( "136102-0330.mol0.7", 0x1e00001, 0x100000, CRC(7bf6bb8f) SHA1(f34bd8a9c7f95436a1c816badc59673cd2a6969a) )
	ROM_LOAD16_BYTE( "136102-0331.moh0.7", 0x1e00000, 0x100000, CRC(c6a64dad) SHA1(ee54514463ab61cbaef70da064cf5de591e5861f) )

	ROM_REGION( 0x0600, "proms", 0 ) /* N82S147AN */
	ROM_LOAD( "136094-0001a.13s", 0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )    /* microcode for growth renderer */
	ROM_LOAD( "136094-0002a.14s", 0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "136094-0003a.15s", 0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )

	ROM_REGION( 0x0012, "mainbd_pals", 0 ) /* Dump attempted but security fuse blown */
	ROM_LOAD( "136101-0021a.22a.bin", 0x0000, 0x0001, NO_DUMP) /* GAL16V8B-15LP */
	ROM_LOAD( "136101-1025b.22e.bin", 0x0000, 0x0001, NO_DUMP) /* GAL16V8B-25LP */
	ROM_LOAD( "136101-0013a.23e.bin", 0x0000, 0x0001, NO_DUMP) /* GAL16V8B-15LP */
	ROM_LOAD( "136101-0018a.24e.bin", 0x0000, 0x0001, NO_DUMP) /* GAL16V8B-25LP */
	ROM_LOAD( "136101-0017a.25e.bin", 0x0000, 0x0001, NO_DUMP) /* GAL22V10B-10LP */
	ROM_LOAD( "136101-1220a.22u.bin", 0x0000, 0x0001, NO_DUMP) /* GAL16V8B-10LP */
	ROM_LOAD( "136094-0015a.17p.bin", 0x0000, 0x0001, NO_DUMP) /* GAL16V8B-25LP */
	ROM_LOAD( "136094-0007a.17s.bin", 0x0000, 0x0001, NO_DUMP) /* GAL16V8B-25LP */
	ROM_LOAD( "136101-1008a.13m.bin", 0x0000, 0x0001, NO_DUMP) /* GAL16V8B-25LP */
	ROM_LOAD( "136101-0011a.11k.bin", 0x0000, 0x0001, NO_DUMP) /* GAL16V8B-25LP */
	ROM_LOAD( "136101-1022a.12k.bin", 0x0000, 0x0001, NO_DUMP) /* GAL22V10B-15LP */
	ROM_LOAD( "136094-0014a.12s.bin", 0x0000, 0x0001, NO_DUMP) /* GAL16V8B-25LP */
	ROM_LOAD( "136094-0016a.11s.bin", 0x0000, 0x0001, NO_DUMP) /* GAL16V8B-25LP */
	ROM_LOAD( "136101-0009a.10m.bin", 0x0000, 0x0001, NO_DUMP) /* GAL16V8B-25LP */
	ROM_LOAD( "136101-0012a.9n.bin",  0x0000, 0x0001, NO_DUMP) /* GAL16V8B-25LP */
	ROM_LOAD( "136101-0010b.8k.bin",  0x0000, 0x0001, NO_DUMP) /* GAL16V8B-25LP */
	ROM_LOAD( "136101-0019a.7k.bin",  0x0000, 0x0001, NO_DUMP) /* GAL20V8B-25LP */
	ROM_LOAD( "136101-0006a.1s.bin",  0x0000, 0x0001, NO_DUMP) /* GAL16V8B-10LP */

	ROM_REGION( 0x0002, "rombd_pals", 0 ) /* Dump attempted but security fuse blown */
	ROM_LOAD( "136102-0261a.bnkdec.bin", 0x0000, 0x0001, NO_DUMP) /* GAL16V8B-15LP */
	ROM_LOAD( "136102-0260a.romdec.bin", 0x0000, 0x0001, NO_DUMP) /* GAL16V8B-10LP */

	ROM_REGION( 0x0003, "sndbd_pals", 0 ) /* Dump attempted but security fuse blown */
	ROM_LOAD( "136101-0070a.9f.bin",  0x0000, 0x0001, NO_DUMP) /* GAL16V8B-25LP */
	ROM_LOAD( "136102-0071a.10f.bin", 0x0000, 0x0001, NO_DUMP) /* GAL16V8B-25LP */
	ROM_LOAD( "136101-0073a.11f.bin", 0x0000, 0x0001, NO_DUMP) /* GAL16V8B-25LP */
ROM_END


ROM_START( primrage20 )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 8*64k for 68000 code */
	ROM_LOAD32_BYTE( "136102-0044b.29l", 0x000000, 0x80000, CRC(26139575) SHA1(22e59ab621d58e56969b64701fc59aec085193dd) )
	ROM_LOAD32_BYTE( "136102-0043b.28l", 0x000001, 0x80000, CRC(928d2447) SHA1(9bbbdbf056a7b986d985d79be889b9876a710631) )
	ROM_LOAD32_BYTE( "136102-0042b.27l", 0x000002, 0x80000, CRC(cd6062b9) SHA1(2973fb561ab68cd48ec132b6720c04d10bedfd19) )
	ROM_LOAD32_BYTE( "136102-0041b.25l", 0x000003, 0x80000, CRC(3008f6f0) SHA1(45aac457b4584ee3bd3561e3b2e34e49aa61fbc5) )

	ROM_REGION32_LE( 0x200000, "cageboot", 0 )  /* TMS320C31 boot ROM */
	ROM_LOAD32_BYTE( "136102-0078a.11a", 0x000000, 0x080000, CRC(91df8d8f) SHA1(6d361f88de604b8f11dd9bfe85ff18bcd322862d) )

	ROM_REGION32_LE( 0x1000000, "cage", 0 ) /* TMS320C31 sound ROMs */
	ROM_LOAD32_WORD( "136102-0075",  0x400000, 0x200000, CRC(b685a88e) SHA1(998b8fe54971f6cd96e4c22b19e3831f29d8172d) )
	ROM_LOAD32_WORD( "136102-0077",  0x400002, 0x200000, CRC(3283cea8) SHA1(fb7333ca951053a56c501f2ce0eb197c8fcafaf7) )

	ROM_REGION( 0x300000, "gfx1", 0 )
	ROM_LOAD( "136102-0050a.25n", 0x000000, 0x80000, CRC(66896e8f) SHA1(7675b24c15ca0608f11f2a7b8d70717adb10924c) ) /* playfield, planes 0-1 */
	ROM_LOAD( "136102-0051a.27n", 0x100000, 0x80000, CRC(fb5b3e7b) SHA1(f43fe4b5c4bbea10da46b60c644f586fb391355d) ) /* playfield, planes 2-3 */
	ROM_LOAD( "136102-0052a.28n", 0x200000, 0x80000, CRC(cbe38670) SHA1(0780e599007851f6d37cdd8c701d01cb1ae48b9d) ) /* playfield, planes 4-5 */

	ROM_REGION( 0x020000, "gfx2", 0 )
	ROM_LOAD( "136102-0045a.23p", 0x000000, 0x20000, CRC(c8b39b1c) SHA1(836c0ccf96b2beccacf6d8ac23981fc2d1f09803) ) /* alphanumerics */

	ROM_REGION16_BE( 0x2000000, "rle", 0 )
	ROM_LOAD16_BYTE( "136102-0100a.2v",    0x0000001, 0x080000, CRC(5299fb2a) SHA1(791378215ab6ffff3ab2ae7192ce9f88dae4090d) )
	ROM_LOAD16_BYTE( "136102-0101a.2w",    0x0000000, 0x080000, CRC(3e234711) SHA1(6a9f19db2b4c8c34d3d7b4984206e3d5c4398d7f) )
	ROM_LOAD16_BYTE( "136102-0332.mol1.0", 0x0800001, 0x100000, CRC(610cfcb4) SHA1(bed1bd0d11c0a7cc48d020fc0acec34daf48c5ac) )
	ROM_LOAD16_BYTE( "136102-0333.moh1.0", 0x0800000, 0x100000, CRC(3320448e) SHA1(aef42328bf72fca5c04bfed1ea41100bb5aafeaa) )
	ROM_LOAD16_BYTE( "136102-0334.mol1.1", 0x0a00001, 0x100000, CRC(be3acb6f) SHA1(664cb4cd4d325577ab0cbe0cf48870a9f4706573) )
	ROM_LOAD16_BYTE( "136102-0335.moh1.1", 0x0a00000, 0x100000, CRC(e4f6e87a) SHA1(2a3f8ff46b289c25cd4ca2a1369b14613f48e964) )
	ROM_LOAD16_BYTE( "136102-0336.mol1.2", 0x0c00001, 0x100000, CRC(a78a8525) SHA1(69c3da4d45b0f09f5bdabcedd238b82efab48a70) )
	ROM_LOAD16_BYTE( "136102-0337.moh1.2", 0x0c00000, 0x100000, CRC(73fdd050) SHA1(63c67187953d2dab93a260e548ef5965e7cba4e8) )
	ROM_LOAD16_BYTE( "136102-0338.mol1.3", 0x0e00001, 0x100000, CRC(fa19cae6) SHA1(7d0560516971f32835329a17450c7561631a27d1) )
	ROM_LOAD16_BYTE( "136102-0339.moh1.3", 0x0e00000, 0x100000, CRC(e0cd1393) SHA1(0de59d04165d64320512936c194db19cca6455fd) )
	ROM_LOAD16_BYTE( "136102-0316.mol0.0", 0x1000001, 0x100000, CRC(9301c672) SHA1(a8971049c857ae283a95b257dd0d6aaff6d787cd) )
	ROM_LOAD16_BYTE( "136102-0317.moh0.0", 0x1000000, 0x100000, CRC(9e3b831a) SHA1(b799e57bea9522cb83f9aa7ea38a17b1d8273b8d) )
	ROM_LOAD16_BYTE( "136102-0318.mol0.1", 0x1200001, 0x100000, CRC(8523db5d) SHA1(f2476aa26b1a7cbe7510994d92eb209fda65593d) )
	ROM_LOAD16_BYTE( "136102-0319.moh0.1", 0x1200000, 0x100000, CRC(42f22e4b) SHA1(2a1a6f0a7aca7b7b64bce0bd54eb4cb23a2336b1) )
	ROM_LOAD16_BYTE( "136102-0320.mol0.2", 0x1400001, 0x100000, CRC(21369d13) SHA1(28e03595c098fd9bec6f7316180d17905a51a51b) )
	ROM_LOAD16_BYTE( "136102-0321.moh0.2", 0x1400000, 0x100000, CRC(3b7d498a) SHA1(804e9e1567bf97e8dae3b9444428254ced8b60da) )
	ROM_LOAD16_BYTE( "136102-0322.mol0.3", 0x1600001, 0x100000, CRC(05e9f407) SHA1(fa25a893d4cb805df02d7d12df4dbabefb3114a2) )
	ROM_LOAD16_BYTE( "136102-0323.moh0.3", 0x1600000, 0x100000, CRC(603f3bb6) SHA1(d7c22dc900d9edc36d8f211df67a206d14637fab) )
	ROM_LOAD16_BYTE( "136102-0324.mol0.4", 0x1800001, 0x100000, CRC(3c37769f) SHA1(ca0306a439949d2a0305cc0cf05808a58e84084c) )
	ROM_LOAD16_BYTE( "136102-0325.moh0.4", 0x1800000, 0x100000, CRC(f43321e3) SHA1(8bb4dd4a5d5400b17052d50dca9078211dc6b861) )
	ROM_LOAD16_BYTE( "136102-0326.mol0.5", 0x1a00001, 0x100000, CRC(63d4ccea) SHA1(340fced6998a8ae6fd285d8fe666f5f1e4b6bfaf) )
	ROM_LOAD16_BYTE( "136102-0327.moh0.5", 0x1a00000, 0x100000, CRC(9f4806d5) SHA1(76e9f1a47e7fa45e834fa8739528f1e3c54b14dc) )
	ROM_LOAD16_BYTE( "136102-0328.mol0.6", 0x1c00001, 0x100000, CRC(a08d73e1) SHA1(25a58777f15e9550111447b47a98762fd6bb498d) )
	ROM_LOAD16_BYTE( "136102-0329.moh0.6", 0x1c00000, 0x100000, CRC(eff3d2cd) SHA1(8532568b5fd91d2b738947e9cd575a4eb2a03be2) )
	ROM_LOAD16_BYTE( "136102-0330.mol0.7", 0x1e00001, 0x100000, CRC(7bf6bb8f) SHA1(f34bd8a9c7f95436a1c816badc59673cd2a6969a) )
	ROM_LOAD16_BYTE( "136102-0331.moh0.7", 0x1e00000, 0x100000, CRC(c6a64dad) SHA1(ee54514463ab61cbaef70da064cf5de591e5861f) )

	ROM_REGION( 0x0600, "proms", 0 ) /* N82S147AN */
	ROM_LOAD( "136094-0001a.13s", 0x0000, 0x0200, CRC(a70ade3f) SHA1(f4a558b17767eed2683c768d1b441e75edcff967) )    /* microcode for growth renderer */
	ROM_LOAD( "136094-0002a.14s", 0x0200, 0x0200, CRC(f4768b4d) SHA1(a506fa5386ab0ea2851ff1f8474d4bfc66deaa70) )
	ROM_LOAD( "136094-0003a.15s", 0x0400, 0x0200, CRC(22a76ad4) SHA1(ce840c283bbd3a5f19dc8d91b19d1571eff51ff4) )
ROM_END



/*************************************
 *
 *  Driver initialization
 *
 *************************************/

WRITE32_MEMBER(atarigt_state::tmek_pf_w)
{
	offs_t pc = space.device().safe_pc();

	/* protected version */
	if (pc == 0x2EB3C || pc == 0x2EB48)
	{
		logerror("%06X:PFW@%06X = %08X & %08X (src=%06X)\n", space.device().safe_pc(), 0xd72000 + offset*4, data, mem_mask, (UINT32)space.device().state().state_int(M68K_A4) - 2);
		/* skip these writes to make more stuff visible */
		return;
	}

	/* unprotected version */
	if (pc == 0x25834 || pc == 0x25860)
		logerror("%06X:PFW@%06X = %08X & %08X (src=%06X)\n", space.device().safe_pc(), 0xd72000 + offset*4, data, mem_mask, (UINT32)space.device().state().state_int(M68K_A3) - 2);

	m_playfield_tilemap->write(space, offset, data, mem_mask);
}

DRIVER_INIT_MEMBER(atarigt_state,tmek)
{
	m_is_primrage = 0;

	/* setup protection */
	m_protection_r = &atarigt_state::tmek_protection_r;
	m_protection_w = &atarigt_state::tmek_protection_w;

	/* temp hack */
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xd72000, 0xd75fff, write32_delegate(FUNC(atarigt_state::tmek_pf_w),this));
}


DRIVER_INIT_MEMBER(atarigt_state,primrage)
{
	m_is_primrage = 1;

	/* install protection */
	m_protection_r = &atarigt_state::primrage_protection_r;
	m_protection_w = &atarigt_state::primrage_protection_w;
}

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1994, tmek,       0,        tmek,      tmek, atarigt_state,     tmek,       ROT0, "Atari Games", "T-MEK (v5.1, The Warlords)", MACHINE_UNEMULATED_PROTECTION )
GAME( 1994, tmek51p,    tmek,     tmek,      tmek, atarigt_state,     tmek,       ROT0, "Atari Games", "T-MEK (v5.1, prototype)", MACHINE_UNEMULATED_PROTECTION )
GAME( 1994, tmek45,     tmek,     tmek,      tmek, atarigt_state,     tmek,       ROT0, "Atari Games", "T-MEK (v4.5)", MACHINE_UNEMULATED_PROTECTION )
GAME( 1994, tmek44,     tmek,     tmek,      tmek, atarigt_state,     tmek,       ROT0, "Atari Games", "T-MEK (v4.4)", MACHINE_UNEMULATED_PROTECTION )
GAME( 1994, tmek20,     tmek,     tmek,      tmek, atarigt_state,     tmek,       ROT0, "Atari Games", "T-MEK (v2.0, prototype)", 0 )
GAME( 1994, primrage,   0,        primrage,  primrage, atarigt_state, primrage,   ROT0, "Atari Games", "Primal Rage (version 2.3)", MACHINE_UNEMULATED_PROTECTION )
GAME( 1994, primrage20, primrage, primrage20,primrage, atarigt_state, primrage, ROT0, "Atari Games", "Primal Rage (version 2.0)", MACHINE_UNEMULATED_PROTECTION )
