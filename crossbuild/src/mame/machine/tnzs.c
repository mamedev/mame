/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

  The I8742 MCU takes care of handling the coin inputs and the tilt switch.
  To simulate this, we read the status in the interrupt handler for the main
  CPU and update the counters appropriately. We also must take care of
  handling the coin/credit settings ourselves.

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "cpu/i8x41/i8x41.h"
#include "includes/tnzs.h"

static int mcu_type;
static int tnzs_input_select;

enum
{
	MCU_NONE_INSECTX,
	MCU_NONE_KAGEKI,
	MCU_NONE_TNZSB,
	MCU_NONE_KABUKIZ,
	MCU_EXTRMATN,
	MCU_ARKANOID,
	MCU_PLUMPOP,
	MCU_DRTOPPEL,
	MCU_CHUKATAI,
	MCU_TNZS
};

static int mcu_initializing,mcu_coinage_init,mcu_command,mcu_readcredits;
static int mcu_reportcoin;
static UINT8 mcu_coinage[4];
static UINT8 mcu_coinsA,mcu_coinsB,mcu_credits;



static READ8_HANDLER( mcu_tnzs_r )
{
	UINT8 data;

	if (offset == 0)
	{
		data = cpunum_get_reg(2, I8X41_DATA);
		cpu_yield();
	}
	else
	{
		data = cpunum_get_reg(2, I8X41_STAT);
		cpu_yield();
	}

//  logerror("PC %04x: read %02x from mcu $c00%01x\n", activecpu_get_previouspc(), data, offset);

	return data;
}

static WRITE8_HANDLER( mcu_tnzs_w )
{
//  logerror("PC %04x: write %02x to mcu $c00%01x\n", activecpu_get_previouspc(), data, offset);

	if (offset == 0)
		cpunum_set_reg(2, I8X41_DATA, data);
	else
		cpunum_set_reg(2, I8X41_CMND, data);
}


READ8_HANDLER( tnzs_port1_r )
{
	int data = 0;

	switch (tnzs_input_select & 0x0f)
	{
		case 0x0a:	data = input_port_4_r(0); break;
		case 0x0c:	data = input_port_2_r(0); break;
		case 0x0d:	data = input_port_3_r(0); break;
		default:	data = 0xff; break;
	}

//  logerror("I8742:%04x  Read %02x from port 1\n", activecpu_get_previouspc(), data);

	return data;
}

READ8_HANDLER( tnzs_port2_r )
{
	int data = input_port_4_r(0);

//  logerror("I8742:%04x  Read %02x from port 2\n", activecpu_get_previouspc(), data);

	return data;
}

WRITE8_HANDLER( tnzs_port2_w )
{
//  logerror("I8742:%04x  Write %02x to port 2\n", activecpu_get_previouspc(), data);

	coin_lockout_w( 0, (data & 0x40) );
	coin_lockout_w( 1, (data & 0x80) );
	coin_counter_w( 0, (~data & 0x10) );
	coin_counter_w( 1, (~data & 0x20) );

	tnzs_input_select = data;
}



READ8_HANDLER( arknoid2_sh_f000_r )
{
	int val;

//  logerror("PC %04x: read input %04x\n", activecpu_get_pc(), 0xf000 + offset);

	val = readinputport(7 + offset/2);
	if (offset & 1)
	{
		return ((val >> 8) & 0xff);
	}
	else
	{
		return val & 0xff;
	}
}


static void mcu_reset(void)
{
	mcu_initializing = 3;
	mcu_coinage_init = 0;
	mcu_coinage[0] = 1;
	mcu_coinage[1] = 1;
	mcu_coinage[2] = 1;
	mcu_coinage[3] = 1;
	mcu_coinsA = 0;
	mcu_coinsB = 0;
	mcu_credits = 0;
	mcu_reportcoin = 0;
	mcu_command = 0;
}

static void mcu_handle_coins(int coin)
{
	static int insertcoin;

	/* The coin inputs and coin counters are managed by the i8742 mcu. */
	/* Here we simulate it. */
	/* Credits are limited to 9, so more coins should be rejected */
	/* Coin/Play settings must also be taken into consideration */

	if (coin & 0x08)	/* tilt */
		mcu_reportcoin = coin;
	else if (coin && coin != insertcoin)
	{
		if (coin & 0x01)	/* coin A */
		{
//          logerror("Coin dropped into slot A\n");
			coin_counter_w(0,1); coin_counter_w(0,0); /* Count slot A */
			mcu_coinsA++;
			if (mcu_coinsA >= mcu_coinage[0])
			{
				mcu_coinsA -= mcu_coinage[0];
				mcu_credits += mcu_coinage[1];
				if (mcu_credits >= 9)
				{
					mcu_credits = 9;
					coin_lockout_global_w(1); /* Lock all coin slots */
				}
				else
				{
					coin_lockout_global_w(0); /* Unlock all coin slots */
				}
			}
		}
		if (coin & 0x02)	/* coin B */
		{
//          logerror("Coin dropped into slot B\n");
			coin_counter_w(1,1); coin_counter_w(1,0); /* Count slot B */
			mcu_coinsB++;
			if (mcu_coinsB >= mcu_coinage[2])
			{
				mcu_coinsB -= mcu_coinage[2];
				mcu_credits += mcu_coinage[3];
				if (mcu_credits >= 9)
				{
					mcu_credits = 9;
					coin_lockout_global_w(1); /* Lock all coin slots */
				}
				else
				{
					coin_lockout_global_w(0); /* Unlock all coin slots */
				}
			}
		}
		if (coin & 0x04)	/* service */
		{
//          logerror("Coin dropped into service slot C\n");
			mcu_credits++;
		}
		mcu_reportcoin = coin;
	}
	else
	{
		if (mcu_credits < 9)
			coin_lockout_global_w(0); /* Unlock all coin slots */
		mcu_reportcoin = 0;
	}
	insertcoin = coin;
}



static READ8_HANDLER( mcu_arknoid2_r )
{
	static const char mcu_startup[] = "\x55\xaa\x5a";

//  logerror("PC %04x: read mcu %04x\n", activecpu_get_pc(), 0xc000 + offset);

	if (offset == 0)
	{
		/* if the mcu has just been reset, return startup code */
		if (mcu_initializing)
		{
			mcu_initializing--;
			return mcu_startup[2 - mcu_initializing];
		}

		switch (mcu_command)
		{
			case 0x41:
				return mcu_credits;

			case 0xc1:
				/* Read the credit counter or the inputs */
				if (mcu_readcredits == 0)
				{
					mcu_readcredits = 1;
					if (mcu_reportcoin & 0x08)
					{
						mcu_initializing = 3;
						return 0xee;	/* tilt */
					}
					else return mcu_credits;
				}
				else return readinputport(2);	/* buttons */

			default:
				logerror("error, unknown mcu command\n");
				/* should not happen */
				return 0xff;
				break;
		}
	}
	else
	{
		/*
        status bits:
        0 = mcu is ready to send data (read from c000)
        1 = mcu has read data (from c000)
        2 = unused
        3 = unused
        4-7 = coin code
              0 = nothing
              1,2,3 = coin switch pressed
              e = tilt
        */
		if (mcu_reportcoin & 0x08) return 0xe1;	/* tilt */
		if (mcu_reportcoin & 0x01) return 0x11;	/* coin 1 (will trigger "coin inserted" sound) */
		if (mcu_reportcoin & 0x02) return 0x21;	/* coin 2 (will trigger "coin inserted" sound) */
		if (mcu_reportcoin & 0x04) return 0x31;	/* coin 3 (will trigger "coin inserted" sound) */
		return 0x01;
	}
}

static WRITE8_HANDLER( mcu_arknoid2_w )
{
	if (offset == 0)
	{
//      logerror("PC %04x: write %02x to mcu %04x\n", activecpu_get_pc(), data, 0xc000 + offset);
		if (mcu_command == 0x41)
		{
			mcu_credits = (mcu_credits + data) & 0xff;
		}
	}
	else
	{
		/*
        0xc1: read number of credits, then buttons
        0x54+0x41: add value to number of credits
        0x15: sub 1 credit (when "Continue Play" only)
        0x84: coin 1 lockout (issued only in test mode)
        0x88: coin 2 lockout (issued only in test mode)
        0x80: release coin lockout (issued only in test mode)
        during initialization, a sequence of 4 bytes sets coin/credit settings
        */
//      logerror("PC %04x: write %02x to mcu %04x\n", activecpu_get_pc(), data, 0xc000 + offset);

		if (mcu_initializing)
		{
			/* set up coin/credit settings */
			mcu_coinage[mcu_coinage_init++] = data;
			if (mcu_coinage_init == 4) mcu_coinage_init = 0;	/* must not happen */
		}

		if (data == 0xc1)
			mcu_readcredits = 0;	/* reset input port number */

		if (data == 0x15)
		{
			mcu_credits = (mcu_credits - 1) & 0xff;
			if (mcu_credits == 0xff) mcu_credits = 0;
		}
		mcu_command = data;
	}
}


static READ8_HANDLER( mcu_extrmatn_r )
{
	static const char mcu_startup[] = "\x5a\xa5\x55";

//  logerror("PC %04x: read mcu %04x\n", activecpu_get_pc(), 0xc000 + offset);

	if (offset == 0)
	{
		/* if the mcu has just been reset, return startup code */
		if (mcu_initializing)
		{
			mcu_initializing--;
			return mcu_startup[2 - mcu_initializing];
		}

		switch (mcu_command)
		{
			case 0x01:
				return readinputport(2) ^ 0xff;	/* player 1 joystick + buttons */

			case 0x02:
				return readinputport(3) ^ 0xff;	/* player 2 joystick + buttons */

			case 0x1a:
				return (readinputport(5) | (readinputport(6) << 1));

			case 0x21:
				return readinputport(4) & 0x0f;

			case 0x41:
				return mcu_credits;

			case 0xa0:
				/* Read the credit counter */
				if (mcu_reportcoin & 0x08)
				{
					mcu_initializing = 3;
					return 0xee;	/* tilt */
				}
				else return mcu_credits;

			case 0xa1:
				/* Read the credit counter or the inputs */
				if (mcu_readcredits == 0)
				{
					mcu_readcredits = 1;
					if (mcu_reportcoin & 0x08)
					{
						mcu_initializing = 3;
						return 0xee;	/* tilt */
//                      return 0x64;    /* theres a reset input somewhere */
					}
					else return mcu_credits;
				}
				/* buttons */
				else return ((readinputport(2) & 0xf0) | (readinputport(3) >> 4)) ^ 0xff;

			default:
				logerror("error, unknown mcu command\n");
				/* should not happen */
				return 0xff;
				break;
		}
	}
	else
	{
		/*
        status bits:
        0 = mcu is ready to send data (read from c000)
        1 = mcu has read data (from c000)
        2 = unused
        3 = unused
        4-7 = coin code
              0 = nothing
              1,2,3 = coin switch pressed
              e = tilt
        */
		if (mcu_reportcoin & 0x08) return 0xe1;	/* tilt */
		if (mcu_reportcoin & 0x01) return 0x11;	/* coin 1 (will trigger "coin inserted" sound) */
		if (mcu_reportcoin & 0x02) return 0x21;	/* coin 2 (will trigger "coin inserted" sound) */
		if (mcu_reportcoin & 0x04) return 0x31;	/* coin 3 (will trigger "coin inserted" sound) */
		return 0x01;
	}
}

static WRITE8_HANDLER( mcu_extrmatn_w )
{
	if (offset == 0)
	{
//      logerror("PC %04x: write %02x to mcu %04x\n", activecpu_get_pc(), data, 0xc000 + offset);
		if (mcu_command == 0x41)
		{
			mcu_credits = (mcu_credits + data) & 0xff;
		}
	}
	else
	{
		/*
        0xa0: read number of credits
        0xa1: read number of credits, then buttons
        0x01: read player 1 joystick + buttons
        0x02: read player 2 joystick + buttons
        0x1a: read coin switches
        0x21: read service & tilt switches
        0x4a+0x41: add value to number of credits
        0x84: coin 1 lockout (issued only in test mode)
        0x88: coin 2 lockout (issued only in test mode)
        0x80: release coin lockout (issued only in test mode)
        during initialization, a sequence of 4 bytes sets coin/credit settings
        */

//      logerror("PC %04x: write %02x to mcu %04x\n", activecpu_get_pc(), data, 0xc000 + offset);

		if (mcu_initializing)
		{
			/* set up coin/credit settings */
			mcu_coinage[mcu_coinage_init++] = data;
			if (mcu_coinage_init == 4) mcu_coinage_init = 0;	/* must not happen */
		}

		if (data == 0xa1)
			mcu_readcredits = 0;	/* reset input port number */

		/* Dr Toppel decrements credits differently. So handle it */
		if ((data == 0x09) && (mcu_type == MCU_DRTOPPEL || mcu_type == MCU_PLUMPOP))
			mcu_credits = (mcu_credits - 1) & 0xff;		/* Player 1 start */
		if ((data == 0x18) && (mcu_type == MCU_DRTOPPEL || mcu_type == MCU_PLUMPOP))
			mcu_credits = (mcu_credits - 2) & 0xff;		/* Player 2 start */

		mcu_command = data;
	}
}



/*********************************

TNZS sync bug kludge

In all TNZS versions there is code like this:

0C5E: ld   ($EF10),a
0C61: ld   a,($EF10)
0C64: inc  a
0C65: ret  nz
0C66: jr   $0C61

which is sometimes executed by the main cpu when it writes to shared RAM a
command for the second CPU. The intended purpose of the code is to wait an
acknowledge from the sub CPU: the sub CPU writes FF to the same location
after reading the command.

However the above code is wrong. The "ret nz" instruction means that the
loop will be exited only when the contents of $EF10 are *NOT* $FF!!
On the real board, this casues little harm: the main CPU will just write
the command, read it back and, since it's not $FF, return immediately. There
is a chance that the command might go lost, but this will cause no major
harm, the worse that can happen is that the background tune will not change.

In MAME, however, since CPU interleaving is not perfect, it can happen that
the main CPU ends its timeslice after writing to EF10 but before reading it
back. In the meantime, the sub CPU will run, read the command and write FF
there - therefore causing the main CPU to enter an endless loop.

Unlike the usual sync problems in MAME, which can be fixed by increasing the
interleave factor, in this case increasing it will actually INCREASE the
chance of entering the endless loop - because it will increase the chances of
the main CPU ending its timeslice at the wrong moment.

So what we do here is catch writes by the main CPU to the RAM location, and
process them using a timer, in order to
a) force a resync of the two CPUs
b) make sure the main CPU will be the first one to run after the location is
   changed

Since the answer from the sub CPU is ignored, we don't even need to boost
interleave.

*********************************/

static TIMER_CALLBACK( kludge_callback )
{
	tnzs_sharedram[0x0f10] = param;
}

static WRITE8_HANDLER( tnzs_sync_kludge_w )
{
	timer_call_after_resynch(NULL, data,kludge_callback);
}




DRIVER_INIT( plumpop )
{
	mcu_type = MCU_PLUMPOP;
}

DRIVER_INIT( extrmatn )
{
	UINT8 *RAM = memory_region(REGION_CPU1);

	mcu_type = MCU_EXTRMATN;

	/* there's code which falls through from the fixed ROM to bank #7, I have to */
	/* copy it there otherwise the CPU bank switching support will not catch it. */
	memcpy(&RAM[0x08000],&RAM[0x2c000],0x4000);
}

DRIVER_INIT( arknoid2 )
{
	UINT8 *RAM = memory_region(REGION_CPU1);

	mcu_type = MCU_ARKANOID;

	/* there's code which falls through from the fixed ROM to bank #2, I have to */
	/* copy it there otherwise the CPU bank switching support will not catch it. */
	memcpy(&RAM[0x08000],&RAM[0x18000],0x4000);
}

DRIVER_INIT( drtoppel )
{
	UINT8 *RAM = memory_region(REGION_CPU1);

	mcu_type = MCU_DRTOPPEL;

	/* there's code which falls through from the fixed ROM to bank #2, I have to */
	/* copy it there otherwise the CPU bank switching support will not catch it. */
	memcpy(&RAM[0x08000],&RAM[0x18000],0x4000);

	/* drtoppel writes to the palette RAM area even if it has PROMs! We have to patch it out. */
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xf800, 0xfbff, 0, 0, MWA8_NOP);
}

DRIVER_INIT( chukatai )
{
	mcu_type = MCU_CHUKATAI;
}

DRIVER_INIT( tnzs )
{
	UINT8 *RAM = memory_region(REGION_CPU1);
	mcu_type = MCU_TNZS;

	/* there's code which falls through from the fixed ROM to bank #7, I have to */
	/* copy it there otherwise the CPU bank switching support will not catch it. */
	memcpy(&RAM[0x08000],&RAM[0x2c000],0x4000);

	/* we need to install a kludge to avoid problems with a bug in the original code */
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xef10, 0xef10, 0, 0, tnzs_sync_kludge_w);
}

DRIVER_INIT( tnzsb )
{
	UINT8 *RAM = memory_region(REGION_CPU1);
	mcu_type = MCU_NONE_TNZSB;

	/* there's code which falls through from the fixed ROM to bank #7, I have to */
	/* copy it there otherwise the CPU bank switching support will not catch it. */
	memcpy(&RAM[0x08000],&RAM[0x2c000],0x4000);

	/* we need to install a kludge to avoid problems with a bug in the original code */
	memory_install_write8_handler(0, ADDRESS_SPACE_PROGRAM, 0xef10, 0xef10, 0, 0, tnzs_sync_kludge_w);
}

DRIVER_INIT( kabukiz )
{
	mcu_type = MCU_NONE_KABUKIZ;
}

DRIVER_INIT( insectx )
{
	mcu_type = MCU_NONE_INSECTX;

	/* this game has no mcu, replace the handler with plain input port handlers */
	memory_install_read8_handler(1, ADDRESS_SPACE_PROGRAM, 0xc000, 0xc000, 0, 0, input_port_2_r );
	memory_install_read8_handler(1, ADDRESS_SPACE_PROGRAM, 0xc001, 0xc001, 0, 0, input_port_3_r );
	memory_install_read8_handler(1, ADDRESS_SPACE_PROGRAM, 0xc002, 0xc002, 0, 0, input_port_4_r );
}

DRIVER_INIT( kageki )
{
	mcu_type = MCU_NONE_KAGEKI;
}


READ8_HANDLER( tnzs_mcu_r )
{
	switch (mcu_type)
	{
		case MCU_TNZS:
		case MCU_CHUKATAI:
			return mcu_tnzs_r(offset);
			break;
		case MCU_ARKANOID:
			return mcu_arknoid2_r(offset);
			break;
		case MCU_EXTRMATN:
		case MCU_DRTOPPEL:
		case MCU_PLUMPOP:
			return mcu_extrmatn_r(offset);
			break;
		default:
			return 0xff;
			break;
	}
}

WRITE8_HANDLER( tnzs_mcu_w )
{
	switch (mcu_type)
	{
		case MCU_TNZS:
		case MCU_CHUKATAI:
			mcu_tnzs_w(offset,data);
			break;
		case MCU_ARKANOID:
			mcu_arknoid2_w(offset,data);
			break;
		case MCU_EXTRMATN:
		case MCU_DRTOPPEL:
		case MCU_PLUMPOP:
			mcu_extrmatn_w(offset,data);
			break;
		default:
			break;
	}
}

INTERRUPT_GEN( arknoid2_interrupt )
{
	int coin;

	switch (mcu_type)
	{
		case MCU_ARKANOID:
		case MCU_EXTRMATN:
		case MCU_DRTOPPEL:
		case MCU_PLUMPOP:
			coin  = 0;
			coin |= ((readinputport(5) & 1) << 0);
			coin |= ((readinputport(6) & 1) << 1);
			coin |= ((readinputport(4) & 3) << 2);
			coin ^= 0x0c;
			mcu_handle_coins(coin);
			break;
		default:
			break;
	}

	cpunum_set_input_line(machine, 0, 0, HOLD_LINE);
}

MACHINE_RESET( tnzs )
{
	/* initialize the mcu simulation */
	switch (mcu_type)
	{
		case MCU_ARKANOID:
		case MCU_EXTRMATN:
		case MCU_DRTOPPEL:
		case MCU_PLUMPOP:
			mcu_reset();
			break;
		default:
			break;
	}

	/* preset the banks */
	{
		UINT8 *RAM;

		RAM = memory_region(REGION_CPU1);
		memory_set_bankptr(1,&RAM[0x18000]);

		RAM = memory_region(REGION_CPU2);
		memory_set_bankptr(2,&RAM[0x10000]);
	}
}


READ8_HANDLER( tnzs_sharedram_r )
{
	return tnzs_sharedram[offset];
}

WRITE8_HANDLER( tnzs_sharedram_w )
{
	tnzs_sharedram[offset] = data;
}



WRITE8_HANDLER( tnzs_bankswitch_w )
{
	UINT8 *RAM = memory_region(REGION_CPU1);

//  logerror("PC %04x: writing %02x to bankswitch\n", activecpu_get_pc(),data);

	/* bit 4 resets the second CPU */
	if (data & 0x10)
		cpunum_set_input_line(Machine, 1, INPUT_LINE_RESET, CLEAR_LINE);
	else
		cpunum_set_input_line(Machine, 1, INPUT_LINE_RESET, ASSERT_LINE);

	/* bits 0-2 select RAM/ROM bank */
	memory_set_bankptr (1, &RAM[0x10000 + 0x4000 * (data & 0x07)]);
}

WRITE8_HANDLER( tnzs_bankswitch1_w )
{
	UINT8 *RAM = memory_region(REGION_CPU2);

//  logerror("PC %04x: writing %02x to bankswitch 1\n", activecpu_get_pc(),data);

	switch (mcu_type)
	{
		case MCU_TNZS:
		case MCU_CHUKATAI:
				/* bit 2 resets the mcu */
				if (data & 0x04)
				{
					if (Machine->drv->cpu[2].type == CPU_I8X41)
						cpunum_set_input_line(Machine, 2, INPUT_LINE_RESET, PULSE_LINE);
				}
				/* Coin count and lockout is handled by the i8742 */
				break;
		case MCU_NONE_INSECTX:
				coin_lockout_w( 0, (~data & 0x04) );
				coin_lockout_w( 1, (~data & 0x08) );
				coin_counter_w( 0, (data & 0x10) );
				coin_counter_w( 1, (data & 0x20) );
				break;
		case MCU_NONE_TNZSB:
		case MCU_NONE_KABUKIZ:
				coin_lockout_w( 0, (~data & 0x10) );
				coin_lockout_w( 1, (~data & 0x20) );
				coin_counter_w( 0, (data & 0x04) );
				coin_counter_w( 1, (data & 0x08) );
				break;
		case MCU_NONE_KAGEKI:
				coin_lockout_global_w( (~data & 0x20) );
				coin_counter_w( 0, (data & 0x04) );
				coin_counter_w( 1, (data & 0x08) );
				break;
		case MCU_ARKANOID:
		case MCU_EXTRMATN:
		case MCU_DRTOPPEL:
		case MCU_PLUMPOP:
				/* bit 2 resets the mcu */
				if (data & 0x04)
					mcu_reset();
				break;
		default:
				break;
	}

	/* bits 0-1 select ROM bank */
	memory_set_bankptr (2, &RAM[0x10000 + 0x2000 * (data & 3)]);
}
