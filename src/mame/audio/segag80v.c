// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Sega vector hardware

*************************************************************************/

#include "emu.h"
#include "includes/segag80v.h"
#include "sound/samples.h"

/* History:



 * 4/25/99 Tac-Scan now makes Credit Noises with $2c                    (Jim Hernandez)
 * 4/9/99 Zektor Discrete Sound Support mixed with voice samples.       (Jim Hernandez)
          Zektor uses some Eliminator sounds.

 * 2/5/99 Extra Life sound constant found $1C after fixing main driver. (Jim Hernandez)
 * 1/29/99 Supports Tac Scan new 44.1 kHz sample set.                    (Jim Hernandez)

 * -Stuff to do -
 * Find hex bit for warp.wav sound calls.
 *
 * 2/05/98 now using the new sample_*() functions. BW
 *
 */

/*
    Tac/Scan sound constants

    There are some sounds that are unknown:
    $09 Tunnel Warp Sound?
    $0a
    $0b Formation Change
        $0c
        $0e
        $0f
        $1c 1up (Extra Life)
        $2c Credit
        $30 - $3f  Hex numbers for ship position flight sounds
    $41

    Some sound samples are missing:
    - I use the one bullet and one explosion sound for all 3 for example.

Star Trk Sounds (USB Loaded from 5400 in main EPROMs)

8     PHASER
a     PHOTON
e     TARGETING
10    DENY
12    SHEILD HIT
14    ENTERPRISE HIT
16    ENT EXPLOSION
1a    KLINGON EXPLOSION
1c    DOCK
1e    STARBASE HIT
11    STARBASE RED
22    STARBASE EXPLOSION
24    SMALL BONUS
25    LARGE BONUS
26    STARBASE INTRO
27    KLINGON INTRO
28    ENTERPRISE INTRO
29    PLAYER CHANGE
2e    KLINGON FIRE
4,5   IMPULSE
6,7   WARP
c,d   RED ALERT
18,2f WARP SUCK
19,2f SAUCER EXIT
2c,21 NOMAD MOTION
2d,21 NOMAD STOPPED
2b    COIN DROP MUSIC
2a    HIGH SCORE MUSIC


Eliminator Sound Board (800-3174)
---------------------------------

inputs
0x3c-0x3f

d0 speech ready

outputs ( 0 = ON)

0x3e (076)

d7      torpedo 2
d6      torpedo 1
d5      bounce
d4      explosion 3
d3      explosion 2
d2      explosion 1
d1      fireball
d0      -

0x3f (077)

d7      background msb
d6      background lsb
d5      enemy ship
d4      skitter
d3      thrust msb
d2      thrust lsb
d1      thrust hi
d0      thrust lo

Space Fury Sound Board (800-0241)
---------------------------------

0x3e (076) (0 = ON)

d7      partial warship, low frequency oscillation
d6      star spin
d5      -
d4      -
d3      -
d2      thrust, low frequency noise
d1      fire, metalic buzz
d0      craft scale, rising tone

0x3f (077)

d7      -
d6      -
d5      docking bang
d4      large explosion
d3      small explosion, low frequency noise
d2      fireball
d1      shot
d0      crafts joining

*/

WRITE8_MEMBER(segag80v_state::elim1_sh_w)
{
	data ^= 0xff;

	/* Play fireball sample */
	if (data & 0x02)
		m_samples->start(0, 0);

	/* Play explosion samples */
	if (data & 0x04)
		m_samples->start(1, 10);
	if (data & 0x08)
		m_samples->start(1, 9);
	if (data & 0x10)
		m_samples->start(1, 8);

	/* Play bounce sample */
	if (data & 0x20)
	{
		if (m_samples->playing(2))
			m_samples->stop(2);
		m_samples->start(2, 1);
	}

	/* Play lazer sample */
	if (data & 0xc0)
	{
		if (m_samples->playing(3))
			m_samples->stop(3);
		m_samples->start(3, 5);
	}
}

WRITE8_MEMBER(segag80v_state::elim2_sh_w)
{
	data ^= 0xff;

	/* Play thrust sample */
	if (data & 0x0f)
		m_samples->start(4, 6);
	else
		m_samples->stop(4);

	/* Play skitter sample */
	if (data & 0x10)
		m_samples->start(5, 2);

	/* Play eliminator sample */
	if (data & 0x20)
		m_samples->start(6, 3);

	/* Play electron samples */
	if (data & 0x40)
		m_samples->start(7, 7);
	if (data & 0x80)
		m_samples->start(7, 4);
}


WRITE8_MEMBER(segag80v_state::zektor1_sh_w)
{
	data ^= 0xff;

	/* Play fireball sample */
	if (data & 0x02)
				m_samples->start(0, 0);

	/* Play explosion samples */
	if (data & 0x04)
				m_samples->start(1, 10);
	if (data & 0x08)
					m_samples->start(1, 9);
	if (data & 0x10)
					m_samples->start(1, 8);

	/* Play bounce sample */
	if (data & 0x20)
	{
				if (m_samples->playing(2))
						m_samples->stop(2);
				m_samples->start(2, 1);
	}

	/* Play lazer sample */
	if (data & 0xc0)
	{
		if (m_samples->playing(3))
			m_samples->stop(3);
				m_samples->start(3, 5);
	}
}

WRITE8_MEMBER(segag80v_state::zektor2_sh_w)
{
	data ^= 0xff;

	/* Play thrust sample */
	if (data & 0x0f)
			m_samples->start(4, 6);
	else
		m_samples->stop(4);

	/* Play skitter sample */
	if (data & 0x10)
				m_samples->start(5, 2);

	/* Play eliminator sample */
	if (data & 0x20)
				m_samples->start(6, 3);

	/* Play electron samples */
	if (data & 0x40)
				m_samples->start(7, 40);
	if (data & 0x80)
				m_samples->start(7, 41);
}



WRITE8_MEMBER(segag80v_state::spacfury1_sh_w)
{
	data ^= 0xff;

	/* craft growing */
	if (data & 0x01)
		m_samples->start(1, 0);

	/* craft moving */
	if (data & 0x02)
	{
		if (!m_samples->playing(2))
			m_samples->start(2, 1, true);
	}
	else
		m_samples->stop(2);

	/* Thrust */
	if (data & 0x04)
	{
		if (!m_samples->playing(3))
			m_samples->start(3, 4, true);
	}
	else
		m_samples->stop(3);

	/* star spin */
	if (data & 0x40)
		m_samples->start(4, 8);

	/* partial warship? */
	if (data & 0x80)
		m_samples->start(4, 9);

}

WRITE8_MEMBER(segag80v_state::spacfury2_sh_w)
{
	data ^= 0xff;

	/* craft joining */
	if (data & 0x01)
		m_samples->start(5, 2);

	/* ship firing */
	if (data & 0x02)
	{
		if (m_samples->playing(6))
			m_samples->stop(6);
		m_samples->start(6, 3);

	}

	/* fireball */
	if (data & 0x04)
		m_samples->start(7, 6);

	/* small explosion */
	if (data & 0x08)
		m_samples->start(7, 6);
	/* large explosion */
	if (data & 0x10)
		m_samples->start(7, 5);

	/* docking bang */
	if (data & 0x20)
		m_samples->start(0, 7);

}
