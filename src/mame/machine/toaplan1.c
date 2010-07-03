/***************************************************************************
                ToaPlan game hardware from 1988-1991
                ------------------------------------
 ***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms32010/tms32010.h"
#include "sound/3812intf.h"
#include "includes/toaplan1.h"

#define CLEAR 0
#define ASSERT 1


/* List of possible regions for coinage (for games with unemulated sound CPU) */
enum {
	TOAPLAN1_REGION_JAPAN=0,
	TOAPLAN1_REGION_US,
	TOAPLAN1_REGION_WORLD,
	TOAPLAN1_REGION_OTHER
};

static UINT8 toaplan1_coins_for_credit[TOAPLAN1_REGION_OTHER+1][2][4];
static UINT8 toaplan1_credits_for_coin[TOAPLAN1_REGION_OTHER+1][2][4];


static int toaplan1_coin_count; /* coin count increments on startup ? , so dont count it */
static int toaplan1_intenable;

static int demonwld_dsp_on;
static int demonwld_dsp_BIO;
static int dsp_execute;                 /* Demon world */
static UINT32 dsp_addr_w, main_ram_seg; /* Demon world */

static UINT8 vimana_coins[2];
static UINT8 vimana_credits;
static UINT8 vimana_latch;


int toaplan1_unk_reset_port;

UINT8 *toaplan1_sharedram;



INTERRUPT_GEN( toaplan1_interrupt )
{
	if (toaplan1_intenable)
		cpu_set_input_line(device, 4, HOLD_LINE);
}

WRITE16_HANDLER( toaplan1_intenable_w )
{
	if (ACCESSING_BITS_0_7)
	{
		toaplan1_intenable = data & 0xff;
	}
}


WRITE16_HANDLER( demonwld_dsp_addrsel_w )
{
	/* This sets the main CPU RAM address the DSP should */
	/*  read/write, via the DSP IO port 0 */
	/* Top three bits of data need to be shifted left 9 places */
	/*  to select which memory bank from main CPU address */
	/*  space to use */
	/* Lower thirteen bits of this data is shifted left one position */
	/*  to move it to an even address word boundary */

	main_ram_seg = ((data & 0xe000) << 9);
	dsp_addr_w   = ((data & 0x1fff) << 1);
	logerror("DSP PC:%04x IO write %04x (%08x) at port 0\n",cpu_get_previouspc(space->cpu),data,main_ram_seg + dsp_addr_w);
}

READ16_HANDLER( demonwld_dsp_r )
{
	/* DSP can read data from main CPU RAM via DSP IO port 1 */

	const address_space *mainspace;
	UINT16 input_data = 0;
	switch (main_ram_seg) {
		case 0xc00000:	mainspace = cputag_get_address_space(space->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
						input_data = memory_read_word(mainspace, main_ram_seg + dsp_addr_w);
						break;
		default:		logerror("DSP PC:%04x Warning !!! IO reading from %08x (port 1)\n",cpu_get_previouspc(space->cpu),main_ram_seg + dsp_addr_w);
	}
	logerror("DSP PC:%04x IO read %04x at %08x (port 1)\n",cpu_get_previouspc(space->cpu),input_data,main_ram_seg + dsp_addr_w);
	return input_data;
}

WRITE16_HANDLER( demonwld_dsp_w )
{
	const address_space *mainspace;

	/* Data written to main CPU RAM via DSP IO port 1 */
	dsp_execute = 0;
	switch (main_ram_seg) {
		case 0xc00000:	if ((dsp_addr_w < 3) && (data == 0)) dsp_execute = 1;
						mainspace = cputag_get_address_space(space->machine, "maincpu", ADDRESS_SPACE_PROGRAM);
						memory_write_word(mainspace, main_ram_seg + dsp_addr_w, data);
						break;
		default:		logerror("DSP PC:%04x Warning !!! IO writing to %08x (port 1)\n",cpu_get_previouspc(space->cpu),main_ram_seg + dsp_addr_w);
	}
	logerror("DSP PC:%04x IO write %04x at %08x (port 1)\n",cpu_get_previouspc(space->cpu),data,main_ram_seg + dsp_addr_w);
}

WRITE16_HANDLER( demonwld_dsp_bio_w )
{
	/* data 0xffff  means inhibit BIO line to DSP and enable */
	/*              communication to main processor */
	/*              Actually only DSP data bit 15 controls this */
	/* data 0x0000  means set DSP BIO line active and disable */
	/*              communication to main processor*/
	logerror("DSP PC:%04x IO write %04x at port 3\n",cpu_get_previouspc(space->cpu),data);
	if (data & 0x8000) {
		demonwld_dsp_BIO = CLEAR_LINE;
	}
	if (data == 0) {
		if (dsp_execute) {
			logerror("Turning 68000 on\n");
			cputag_set_input_line(space->machine, "maincpu", INPUT_LINE_HALT, CLEAR_LINE);
			dsp_execute = 0;
		}
		demonwld_dsp_BIO = ASSERT_LINE;
	}
}

READ16_HANDLER ( demonwld_BIO_r )
{
	return demonwld_dsp_BIO;
}


static void demonwld_dsp(running_machine *machine, int enable)
{
	demonwld_dsp_on = enable;
	if (enable)
	{
		logerror("Turning DSP on and 68000 off\n");
		cputag_set_input_line(machine, "dsp", INPUT_LINE_HALT, CLEAR_LINE);
		cputag_set_input_line(machine, "dsp", 0, ASSERT_LINE); /* TMS32010 INT */
		cputag_set_input_line(machine, "maincpu", INPUT_LINE_HALT, ASSERT_LINE);
	}
	else
	{
		logerror("Turning DSP off\n");
		cputag_set_input_line(machine, "dsp", 0, CLEAR_LINE); /* TMS32010 INT */
		cputag_set_input_line(machine, "dsp", INPUT_LINE_HALT, ASSERT_LINE);
	}
}
static STATE_POSTLOAD( demonwld_restore_dsp )
{
	demonwld_dsp(machine, demonwld_dsp_on);
}

WRITE16_HANDLER( demonwld_dsp_ctrl_w )
{
#if 0
	logerror("68000:%08x  Writing %08x to %08x.\n",cpu_get_pc(space->cpu) ,data ,0xe0000a + offset);
#endif

	if (ACCESSING_BITS_0_7)
	{
		switch (data)
		{
			case 0x00:	demonwld_dsp(space->machine, 1); break;	/* Enable the INT line to the DSP */
			case 0x01:	demonwld_dsp(space->machine, 0); break;	/* Inhibit the INT line to the DSP */
			default:	logerror("68000:%04x  Writing unknown command %08x to %08x\n",cpu_get_previouspc(space->cpu) ,data ,0xe0000a + offset); break;
		}
	}
	else
	{
		logerror("68000:%04x  Writing unknown command %08x to %08x\n",cpu_get_previouspc(space->cpu) ,data ,0xe0000a + offset);
	}
}


READ16_HANDLER( samesame_port_6_word_r )
{
	/* Bit 0x80 is secondary CPU (HD647180) ready signal */
	logerror("PC:%04x Warning !!! IO reading from $14000a\n",cpu_get_previouspc(space->cpu));
	return (0x80 | input_port_read(space->machine, "TJUMP")) & 0xff;
}

READ16_HANDLER ( vimana_system_port_r )
{
	int data, p, r, d, slot, reg, dsw;

    UINT8 vimana_region[16] = { TOAPLAN1_REGION_JAPAN, TOAPLAN1_REGION_US   , TOAPLAN1_REGION_WORLD, TOAPLAN1_REGION_JAPAN,
    							TOAPLAN1_REGION_JAPAN, TOAPLAN1_REGION_JAPAN, TOAPLAN1_REGION_JAPAN, TOAPLAN1_REGION_US   ,
    							TOAPLAN1_REGION_JAPAN, TOAPLAN1_REGION_OTHER, TOAPLAN1_REGION_OTHER, TOAPLAN1_REGION_OTHER,
    							TOAPLAN1_REGION_OTHER, TOAPLAN1_REGION_OTHER, TOAPLAN1_REGION_OTHER, TOAPLAN1_REGION_JAPAN };

	slot = -1;
	d = input_port_read(space->machine, "DSWA");
	r = input_port_read(space->machine, "TJUMP");
	p = input_port_read(space->machine, "SYSTEM");
	vimana_latch ^= p;
	data = (vimana_latch & p);

	/* simulate the mcu keeping track of credits based on region and coinage settings */
	/* latch so it doesn't add more than one coin per keypress */
	if (d & 0x04)   /* "test mode" ON */
	{
		vimana_coins[0] = vimana_coins[1] = 0;
		vimana_credits = 0;
	}
	else            /* "test mode" OFF */
	{
		if (data & 0x02)      /* TILT */
		{
			vimana_coins[0] = vimana_coins[1] = 0;
			vimana_credits = 0;
		}
		if (data & 0x01)      /* SERVICE1 */
		{
			vimana_credits++ ;
		}
		if (data & 0x08)      /* COIN1 */
		{
			slot = 0;
		}
		if (data & 0x10)      /* COIN2 */
		{
			slot = 1 ;
		}

		if (slot != -1)
		{
			reg = vimana_region[r];
			dsw = (d & 0xf0) >> (4 + 2 * slot);
			vimana_coins[slot]++;
			if (vimana_coins[slot] >= toaplan1_coins_for_credit[reg][slot][dsw])
			{
				vimana_credits += toaplan1_credits_for_coin[reg][slot][dsw];
				vimana_coins[slot] -= toaplan1_coins_for_credit[reg][slot][dsw];
			}
			coin_counter_w(space->machine, slot, 1);
			coin_counter_w(space->machine, slot, 0);
		}

		if (vimana_credits >= 9)
			vimana_credits = 9;
	}

	coin_lockout_global_w(space->machine, (vimana_credits >= 9));

	vimana_latch = p;

	return p & 0xffff;
}

READ16_HANDLER( vimana_mcu_r )
{
	int data = 0 ;
	switch (offset)
	{
		case 0:  data = 0xff; break;
		case 1:  data = 0x00; break;
		case 2:  data = vimana_credits; break;
	}
	return data & 0xff;
}

WRITE16_HANDLER( vimana_mcu_w )
{
	switch (offset)
	{
		case 0:	break;
		case 1:	break;
		case 2:	if (ACCESSING_BITS_0_7)
				{
					vimana_credits = data & 0xff;
					coin_lockout_global_w(space->machine, (vimana_credits >= 9));
				}
				break;
	}
}

READ16_HANDLER( toaplan1_shared_r )
{
	return toaplan1_sharedram[offset] & 0xff;
}

WRITE16_HANDLER( toaplan1_shared_w )
{
	if (ACCESSING_BITS_0_7)
	{
		toaplan1_sharedram[offset] = data & 0xff;
	}
}


WRITE16_HANDLER( toaplan1_reset_sound )
{
	/* Reset the secondary CPU and sound chip during soft resets */

	if (ACCESSING_BITS_0_7 && (data == 0))
	{
		logerror("PC:%04x  Resetting Sound CPU and Sound chip (%08x)\n", cpu_get_previouspc(space->cpu), data);
		devtag_reset(space->machine, "ymsnd");
		device_t *audiocpu = space->machine->device("audiocpu");
		if (audiocpu != NULL && audiocpu->type() == Z80)
			cpu_set_input_line(audiocpu, INPUT_LINE_RESET, PULSE_LINE);
	}
}


WRITE8_HANDLER( rallybik_coin_w )
{
	switch (data) {
		case 0x08: if (toaplan1_coin_count) { coin_counter_w(space->machine, 0,1); coin_counter_w(space->machine, 0,0); } break;
		case 0x09: if (toaplan1_coin_count) { coin_counter_w(space->machine, 2,1); coin_counter_w(space->machine, 2,0); } break;
		case 0x0a: if (toaplan1_coin_count) { coin_counter_w(space->machine, 1,1); coin_counter_w(space->machine, 1,0); } break;
		case 0x0b: if (toaplan1_coin_count) { coin_counter_w(space->machine, 3,1); coin_counter_w(space->machine, 3,0); } break;
		case 0x0c: coin_lockout_w(space->machine, 0,1); coin_lockout_w(space->machine, 2,1); break;
		case 0x0d: coin_lockout_w(space->machine, 0,0); coin_lockout_w(space->machine, 2,0); break;
		case 0x0e: coin_lockout_w(space->machine, 1,1); coin_lockout_w(space->machine, 3,1); break;
		case 0x0f: coin_lockout_w(space->machine, 1,0); coin_lockout_w(space->machine, 3,0); toaplan1_coin_count=1; break;
		default:   logerror("PC:%04x  Writing unknown data (%04x) to coin count/lockout port\n",cpu_get_previouspc(space->cpu),data); break;
	}
}

WRITE8_HANDLER( toaplan1_coin_w )
{
	logerror("Z80 writing %02x to coin control\n",data);
	/* This still isnt too clear yet. */
	/* Coin C has no coin lock ? */
	/* Are some outputs for lights ? (no space on JAMMA for it though) */

	switch (data) {
		case 0xee: coin_counter_w(space->machine, 1,1); coin_counter_w(space->machine, 1,0); break; /* Count slot B */
		case 0xed: coin_counter_w(space->machine, 0,1); coin_counter_w(space->machine, 0,0); break; /* Count slot A */
	/* The following are coin counts after coin-lock active (faulty coin-lock ?) */
		case 0xe2: coin_counter_w(space->machine, 1,1); coin_counter_w(space->machine, 1,0); coin_lockout_w(space->machine, 1,1); break;
		case 0xe1: coin_counter_w(space->machine, 0,1); coin_counter_w(space->machine, 0,0); coin_lockout_w(space->machine, 0,1); break;

		case 0xec: coin_lockout_global_w(space->machine, 0); break;	/* ??? count games played */
		case 0xe8: break;	/* ??? Maximum credits reached with coin/credit ratio */
		case 0xe4: break;	/* ??? Reset coin system */

		case 0x0c: coin_lockout_global_w(space->machine, 0); break;	/* Unlock all coin slots */
		case 0x08: coin_lockout_w(space->machine, 2,0); break;	/* Unlock coin slot C */
		case 0x09: coin_lockout_w(space->machine, 0,0); break;	/* Unlock coin slot A */
		case 0x0a: coin_lockout_w(space->machine, 1,0); break;	/* Unlock coin slot B */

		case 0x02: coin_lockout_w(space->machine, 1,1); break;	/* Lock coin slot B */
		case 0x01: coin_lockout_w(space->machine, 0,1); break;	/* Lock coin slot A */
		case 0x00: coin_lockout_global_w(space->machine, 1); break;	/* Lock all coin slots */
		default:   logerror("PC:%04x  Writing unknown data (%04x) to coin count/lockout port\n",cpu_get_previouspc(space->cpu),data); break;
	}
}

WRITE16_HANDLER( samesame_coin_w )
{
	if (ACCESSING_BITS_0_7)
	{
		toaplan1_coin_w(space, offset, data & 0xff);
	}
	if (ACCESSING_BITS_8_15 && (data&0xff00))
	{
		logerror("PC:%04x  Writing unknown MSB data (%04x) to coin count/lockout port\n",cpu_get_previouspc(space->cpu),data);
	}
}


static void toaplan1_init_coinage_tables(running_machine *machine)
{
	/* Japan */
	toaplan1_coins_for_credit[TOAPLAN1_REGION_JAPAN][0][0] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_JAPAN][0][0] = 1;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_JAPAN][0][1] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_JAPAN][0][1] = 2;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_JAPAN][0][2] = 2; toaplan1_credits_for_coin[TOAPLAN1_REGION_JAPAN][0][2] = 1;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_JAPAN][0][3] = 2; toaplan1_credits_for_coin[TOAPLAN1_REGION_JAPAN][0][3] = 3;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_JAPAN][1][0] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_JAPAN][1][0] = 1;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_JAPAN][1][1] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_JAPAN][1][1] = 2;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_JAPAN][1][2] = 2; toaplan1_credits_for_coin[TOAPLAN1_REGION_JAPAN][1][2] = 1;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_JAPAN][1][3] = 2; toaplan1_credits_for_coin[TOAPLAN1_REGION_JAPAN][1][3] = 3;
	/* US = Japan */
	toaplan1_coins_for_credit[TOAPLAN1_REGION_US   ][0][0] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_US   ][0][0] = 1;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_US   ][0][1] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_US   ][0][1] = 2;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_US   ][0][2] = 2; toaplan1_credits_for_coin[TOAPLAN1_REGION_US   ][0][2] = 1;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_US   ][0][3] = 2; toaplan1_credits_for_coin[TOAPLAN1_REGION_US   ][0][3] = 3;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_US   ][1][0] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_US   ][1][0] = 1;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_US   ][1][1] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_US   ][1][1] = 2;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_US   ][1][2] = 2; toaplan1_credits_for_coin[TOAPLAN1_REGION_US   ][1][2] = 1;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_US   ][1][3] = 2; toaplan1_credits_for_coin[TOAPLAN1_REGION_US   ][1][3] = 3;
	/* World */
	toaplan1_coins_for_credit[TOAPLAN1_REGION_WORLD][0][0] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_WORLD][0][0] = 1;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_WORLD][0][1] = 2; toaplan1_credits_for_coin[TOAPLAN1_REGION_WORLD][0][1] = 1;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_WORLD][0][2] = 3; toaplan1_credits_for_coin[TOAPLAN1_REGION_WORLD][0][2] = 1;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_WORLD][0][3] = 4; toaplan1_credits_for_coin[TOAPLAN1_REGION_WORLD][0][3] = 1;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_WORLD][1][0] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_WORLD][1][0] = 2;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_WORLD][1][1] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_WORLD][1][1] = 3;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_WORLD][1][2] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_WORLD][1][2] = 4;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_WORLD][1][3] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_WORLD][1][3] = 6;
	/* Other - as it is an unknown or invalid setting, I set everything to 1C_1C */
	toaplan1_coins_for_credit[TOAPLAN1_REGION_OTHER][0][0] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_OTHER][0][0] = 1;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_OTHER][0][1] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_OTHER][0][1] = 1;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_OTHER][0][2] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_OTHER][0][2] = 1;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_OTHER][0][3] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_OTHER][0][3] = 1;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_OTHER][1][0] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_OTHER][1][0] = 1;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_OTHER][1][1] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_OTHER][1][1] = 1;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_OTHER][1][2] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_OTHER][1][2] = 1;
	toaplan1_coins_for_credit[TOAPLAN1_REGION_OTHER][1][3] = 1; toaplan1_credits_for_coin[TOAPLAN1_REGION_OTHER][1][3] = 1;
}

MACHINE_RESET( toaplan1 )
{
	toaplan1_intenable = 0;
	toaplan1_coin_count = 0;
	toaplan1_unk_reset_port = 0;
	coin_lockout_global_w(machine, 0);
}
void toaplan1_driver_savestate(running_machine *machine)
{
	state_save_register_global(machine, toaplan1_intenable);
	state_save_register_global(machine, toaplan1_coin_count);
	state_save_register_global(machine, toaplan1_unk_reset_port);
}

MACHINE_RESET( zerowing )	/* Hack for ZeroWing and OutZone. See the video driver */
{
	MACHINE_RESET_CALL(toaplan1);
	toaplan1_unk_reset_port = 1;
}

MACHINE_RESET( demonwld )
{
	MACHINE_RESET_CALL(toaplan1);
	dsp_addr_w = 0;
	main_ram_seg = 0;
	dsp_execute = 0;
}
void demonwld_driver_savestate(running_machine *machine)
{
	state_save_register_global(machine, demonwld_dsp_on);
	state_save_register_global(machine, dsp_addr_w);
	state_save_register_global(machine, main_ram_seg);
	state_save_register_global(machine, demonwld_dsp_BIO);
	state_save_register_global(machine, dsp_execute);
	state_save_register_postload(machine, demonwld_restore_dsp, NULL);
}

MACHINE_RESET( vimana )
{
	MACHINE_RESET_CALL(toaplan1);
	toaplan1_init_coinage_tables(machine);
	vimana_coins[0] = vimana_coins[1] = 0;
	vimana_credits = 0;
	vimana_latch = 0;
}
void vimana_driver_savestate(running_machine *machine)
{
	state_save_register_global(machine, vimana_coins[0]);
	state_save_register_global(machine, vimana_coins[1]);
	state_save_register_global(machine, vimana_credits);
	state_save_register_global(machine, vimana_latch);
}
