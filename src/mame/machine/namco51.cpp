// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Namco 51XX

    This custom chip is a Fujitsu MB8843 MCU programmed to act as an I/O
    device with built-in coin management. It is also apparently used as a
    protection device. It keeps track of the players scores, and checks
    if a high score has been obtained or bonus lives should be awarded.
    The main CPU has a range of commands to increment/decrement the score
    by various fixed amounts.

    The device is used to its full potential only by Bosconian; Xevious
    uses it too, but only to do a protection check on startup.

    CMD = command from main CPU
    ANS = answer to main CPU

    The chip reads/writes the I/O ports when the /IRQ is pulled down.
    Pin 21 determines whether a read or write should happen (1=R, 0=W).

               +------+
             EX|1   42|Vcc
              X|2   41|K3
         /RESET|3   40|K2
           /IRQ|4   39|K1
             SO|5   38|K0
             SI|6   37|R15
        /SC /TO|7   36|R14
            /TC|8   35|R13
             P0|9   34|R12
             P1|10  33|R11
             P2|11  32|R10
             P3|12  31|R9
             O0|13  30|R8
             O1|14  29|R7
             O2|15  28|R6
             O3|16  27|R5
             O4|17  26|R4
             O5|18  25|R3
             O6|19  24|R2
             O7|20  23|R1
            GND|21  22|R0
               +------+

    commands:
    00: nop
    01 + 4 arguments: set coinage (xevious, possibly because of a bug, is different)
    02: go in "credit" mode and enable start buttons
    03: disable joystick remapping
    04: enable joystick remapping
    05: go in "switch" mode
    06: nop
    07: nop

***************************************************************************/

#include "emu.h"
#include "namco51.h"


#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)


#define READ_PORT(num)           m_in_##num(space, 0)
#define WRITE_PORT(num,data)     m_out_##num(space, 0, data)

WRITE8_MEMBER( namco_51xx_device::write )
{
	data &= 0x07;

	LOG(("%s: custom 51XX write %02x\n",machine().describe_context(),data));

	if (m_coincred_mode)
	{
		switch (m_coincred_mode--)
		{
			case 4: m_coins_per_cred[0] = data; break;
			case 3: m_creds_per_coin[0] = data; break;
			case 2: m_coins_per_cred[1] = data; break;
			case 1: m_creds_per_coin[1] = data; break;
		}
	}
	else
	{
		switch (data)
		{
			case 0: // nop
				break;

			case 1: // set coinage
				m_coincred_mode = 4;
				/* this is a good time to reset the credits counter */
				m_credits = 0;

				{
					/* kludge for a possible bug in Xevious */
					static const game_driver *namcoio_51XX_driver = nullptr;
					static int namcoio_51XX_kludge = 0;

					/* Only compute namcoio_51XX_kludge when gamedrv changes */
					if (namcoio_51XX_driver != &machine().system())
					{
						namcoio_51XX_driver = &machine().system();
						if (strcmp(namcoio_51XX_driver->name, "xevious") == 0 ||
							strcmp(namcoio_51XX_driver->parent, "xevious") == 0)
							namcoio_51XX_kludge = 1;
						else
							namcoio_51XX_kludge = 0;
					}

					if (namcoio_51XX_kludge)
					{
						m_coincred_mode = 6;
						m_remap_joy = 1;
					}
				}
				break;

			case 2: // go in "credits" mode and enable start buttons
				m_mode = 1;
				m_in_count = 0;
				break;

			case 3: // disable joystick remapping
				m_remap_joy = 0;
				break;

			case 4: // enable joystick remapping
				m_remap_joy = 1;
				break;

			case 5: // go in "switch" mode
				m_mode = 0;
				m_in_count = 0;
				break;

			default:
				logerror("unknown 51XX command %02x\n",data);
				break;
		}
	}
}


/* joystick input mapping

  The joystick is parsed and a number corresponding to the direction is returned,
  according to the following table:

          0
        7   1
      6   8   2
        5   3
          4

  The values for directions impossible to obtain on a joystick have not been
  verified on Namco original hardware, but they are the same in all the bootlegs,
  so we can assume they are right.
*/
static const int joy_map[16] =
/*  LDRU, LDR, LDU,  LD, LRU, LR,  LU,    L, DRU,  DR,  DU,   D,  RU,   R,   U, center */
{    0xf, 0xe, 0xd, 0x5, 0xc, 0x9, 0x7, 0x6, 0xb, 0x3, 0xa, 0x4, 0x1, 0x2, 0x0, 0x8 };


READ8_MEMBER( namco_51xx_device::read )
{
	LOG(("%s: custom 51XX read\n",machine().describe_context()));

	if (m_mode == 0) /* switch mode */
	{
		switch ((m_in_count++) % 3)
		{
			default:
			case 0: return READ_PORT(0) | (READ_PORT(1) << 4);
			case 1: return READ_PORT(2) | (READ_PORT(3) << 4);
			case 2: return 0;   // nothing?
		}
	}
	else    /* credits mode */
	{
		switch ((m_in_count++) % 3)
		{
			default:
			case 0: // number of credits in BCD format
				{
					int in,toggle;

					in = ~(READ_PORT(0) | (READ_PORT(1) << 4));
					toggle = in ^ m_lastcoins;
					m_lastcoins = in;

					if (m_coins_per_cred[0] > 0)
					{
						if (m_credits >= 99)
						{
							WRITE_PORT(1,1);  // coin lockout
						}
						else
						{
							WRITE_PORT(1,0);  // coin lockout
							/* check if the user inserted a coin */
							if (toggle & in & 0x10)
							{
								m_coins[0]++;
								WRITE_PORT(0,0x04);   // coin counter
								WRITE_PORT(0,0x0c);
								if (m_coins[0] >= m_coins_per_cred[0])
								{
									m_credits += m_creds_per_coin[0];
									m_coins[0] -= m_coins_per_cred[0];
								}
							}
							if (toggle & in & 0x20)
							{
								m_coins[1]++;
								WRITE_PORT(0,0x08);   // coin counter
								WRITE_PORT(0,0x0c);
								if (m_coins[1] >= m_coins_per_cred[1])
								{
									m_credits += m_creds_per_coin[1];
									m_coins[1] -= m_coins_per_cred[1];
								}
							}
							if (toggle & in & 0x40)
							{
								m_credits++;
							}
						}
					}
					else m_credits = 100;    // free play

					if (m_mode == 1)
					{
						int on = (machine().first_screen()->frame_number() & 0x10) >> 4;

						if (m_credits >= 2)
							WRITE_PORT(0,0x0c | 3*on);    // lamps
						else if (m_credits >= 1)
							WRITE_PORT(0,0x0c | 2*on);    // lamps
						else
							WRITE_PORT(0,0x0c);   // lamps off

						/* check for 1 player start button */
						if (toggle & in & 0x04)
						{
							if (m_credits >= 1)
							{
								m_credits--;
								m_mode = 2;
								WRITE_PORT(0,0x0c);   // lamps off
							}
						}
						/* check for 2 players start button */
						else if (toggle & in & 0x08)
						{
							if (m_credits >= 2)
							{
								m_credits -= 2;
								m_mode = 2;
								WRITE_PORT( 0,0x0c);  // lamps off
							}
						}
					}
				}

				if (~READ_PORT( 1) & 0x08)    /* check test mode switch */
					return 0xbb;

				return (m_credits / 10) * 16 + m_credits % 10;

			case 1:
				{
					int joy = READ_PORT(2) & 0x0f;
					int in,toggle;

					in = ~READ_PORT(0);
					toggle = in ^ m_lastbuttons;
					m_lastbuttons = (m_lastbuttons & 2) | (in & 1);

					/* remap joystick */
					if (m_remap_joy) joy = joy_map[joy];

					/* fire */
					joy |= ((toggle & in & 0x01)^1) << 4;
					joy |= ((in & 0x01)^1) << 5;

					return joy;
				}

			case 2:
				{
					int joy = READ_PORT(3) & 0x0f;
					int in,toggle;

					in = ~READ_PORT(0);
					toggle = in ^ m_lastbuttons;
					m_lastbuttons = (m_lastbuttons & 1) | (in & 2);

					/* remap joystick */
					if (m_remap_joy) joy = joy_map[joy];

					/* fire */
					joy |= ((toggle & in & 0x02)^2) << 3;
					joy |= ((in & 0x02)^2) << 4;

					return joy;
				}
		}
	}
}





/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

static ADDRESS_MAP_START( namco_51xx_map_io, AS_IO, 8, namco_51xx_device )
//  AM_RANGE(MB88_PORTK,  MB88_PORTK)  AM_READ(namco_51xx_K_r)
//  AM_RANGE(MB88_PORTO,  MB88_PORTO)  AM_WRITE(namco_51xx_O_w)
//  AM_RANGE(MB88_PORTR0, MB88_PORTR0) AM_READ(namco_51xx_R0_r)
//  AM_RANGE(MB88_PORTR2, MB88_PORTR2) AM_READ(namco_51xx_R2_r)
ADDRESS_MAP_END


static MACHINE_CONFIG_FRAGMENT( namco_51xx )
	MCFG_CPU_ADD("mcu", MB8843, DERIVED_CLOCK(1,1))     /* parent clock, internally divided by 6 */
	MCFG_CPU_IO_MAP(namco_51xx_map_io)
	MCFG_DEVICE_DISABLE()
MACHINE_CONFIG_END


ROM_START( namco_51xx )
	ROM_REGION( 0x400, "mcu", 0 )
	ROM_LOAD( "51xx.bin",     0x0000, 0x0400, CRC(c2f57ef8) SHA1(50de79e0d6a76bda95ffb02fcce369a79e6abfec) )
ROM_END

const device_type NAMCO_51XX = &device_creator<namco_51xx_device>;

namco_51xx_device::namco_51xx_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, NAMCO_51XX, "Namco 51xx", tag, owner, clock, "namco51", __FILE__),
	m_cpu(*this, "mcu"),
	m_in_0(*this),
	m_in_1(*this),
	m_in_2(*this),
	m_in_3(*this),
	m_out_0(*this),
	m_out_1(*this),
	m_lastcoins(0),
	m_lastbuttons(0),
	m_mode(0),
	m_coincred_mode(0),
	m_remap_joy(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_51xx_device::device_start()
{
	/* resolve our read callbacks */
	m_in_0.resolve_safe(0);
	m_in_1.resolve_safe(0);
	m_in_2.resolve_safe(0);
	m_in_3.resolve_safe(0);

	/* resolve our write callbacks */
	m_out_0.resolve_safe();
	m_out_1.resolve_safe();

	save_item(NAME(m_lastcoins));
	save_item(NAME(m_lastbuttons));
	save_item(NAME(m_credits));
	save_item(NAME(m_coins));
	save_item(NAME(m_coins_per_cred));
	save_item(NAME(m_creds_per_coin));
	save_item(NAME(m_in_count));
	save_item(NAME(m_mode));
	save_item(NAME(m_coincred_mode));
	save_item(NAME(m_remap_joy));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void namco_51xx_device::device_reset()
{
	/* reset internal registers */
	m_credits = 0;
	m_coins[0] = 0;
	m_coins_per_cred[0] = 1;
	m_creds_per_coin[0] = 1;
	m_coins[1] = 0;
	m_coins_per_cred[1] = 1;
	m_creds_per_coin[1] = 1;
	m_in_count = 0;
}

//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor namco_51xx_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( namco_51xx  );
}

//-------------------------------------------------
//  device_rom_region - return a pointer to the
//  the device's ROM definitions
//-------------------------------------------------

const rom_entry *namco_51xx_device::device_rom_region() const
{
	return ROM_NAME(namco_51xx );
}
