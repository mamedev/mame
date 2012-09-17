/***************************************************************************

Namco System II

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m6805/m6805.h"
#include "includes/namcos2.h"
#include "machine/nvram.h"

static TIMER_CALLBACK( namcos2_posirq_tick );
static void InitC148(void);

static emu_timer *namcos2_posirq_timer;

void (*namcos2_kickstart)(running_machine &machine, int internal);

static unsigned mFinalLapProtCount;
static int namcos2_mcu_analog_ctrl;
static int namcos2_mcu_analog_data;
static int namcos2_mcu_analog_complete;
static UINT8 *namcos2_eeprom;
static int sendval;


// not shared
READ16_HANDLER( namcos2_flap_prot_r )
{
	static const UINT16 table0[8] = { 0x0000,0x0040,0x0440,0x2440,0x2480,0xa080,0x8081,0x8041 };
	static const UINT16 table1[8] = { 0x0040,0x0060,0x0060,0x0860,0x0864,0x08e4,0x08e5,0x08a5 };
	UINT16 data;

	switch( offset )
	{
	case 0:
		data = 0x0101;
		break;

	case 1:
		data = 0x3e55;
		break;

	case 2:
		data = table1[mFinalLapProtCount&7];
		data = (data&0xff00)>>8;
		break;

	case 3:
		data = table1[mFinalLapProtCount&7];
		mFinalLapProtCount++;
		data = data&0x00ff;
		break;

	case 0x3fffc/2:
		data = table0[mFinalLapProtCount&7];
		data = data&0xff00;
		break;

	case 0x3fffe/2:
		data = table0[mFinalLapProtCount&7];
		mFinalLapProtCount++;
		data = (data&0x00ff)<<8;
		break;

	default:
		data = 0;
	}
	return data;
}

/*************************************************************/
/* Perform basic machine initialisation                      */
/*************************************************************/

#define namcos2_eeprom_size 0x2000

static void
ResetAllSubCPUs( running_machine &machine, int state )
{
	machine.device("slave")->execute().set_input_line(INPUT_LINE_RESET, state);
	machine.device("mcu")->execute().set_input_line(INPUT_LINE_RESET, state);
	switch( machine.driver_data<namcos2_shared_state>()->m_gametype )
	{
	case NAMCOS21_SOLVALOU:
	case NAMCOS21_STARBLADE:
	case NAMCOS21_AIRCOMBAT:
	case NAMCOS21_CYBERSLED:
		machine.device("dspmaster")->execute().set_input_line(INPUT_LINE_RESET, state);
		machine.device("dspslave")->execute().set_input_line(INPUT_LINE_RESET, state);
		break;

//  case NAMCOS21_WINRUN91:
//  case NAMCOS21_DRIVERS_EYES:
	default:
		break;
	}
}

MACHINE_START_MEMBER(namcos2_shared_state,namcos2)
{
	namcos2_kickstart = NULL;
	namcos2_eeprom = auto_alloc_array(machine(), UINT8, namcos2_eeprom_size);
	machine().device<nvram_device>("nvram")->set_base(namcos2_eeprom, namcos2_eeprom_size);
	namcos2_posirq_timer = machine().scheduler().timer_alloc(FUNC(namcos2_posirq_tick));
}

MACHINE_RESET_MEMBER(namcos2_shared_state,namcos2)
{
	address_space &space = *machine().device("maincpu")->memory().space(AS_PROGRAM);
	mFinalLapProtCount = 0;
	namcos2_mcu_analog_ctrl = 0;
	namcos2_mcu_analog_data = 0xaa;
	namcos2_mcu_analog_complete = 0;
	sendval = 0;

	/* Initialise the bank select in the sound CPU */
	namcos2_sound_bankselect_w(space, 0, 0); /* Page in bank 0 */

	machine().device("audiocpu")->execute().set_input_line(INPUT_LINE_RESET, ASSERT_LINE );

	/* Place CPU2 & CPU3 into the reset condition */
	ResetAllSubCPUs( machine(), ASSERT_LINE );

	/* Initialise interrupt handlers */
	InitC148();

	/* reset POSIRQ timer */
	namcos2_posirq_timer->adjust(attotime::never);
}

/*************************************************************/
/* EEPROM Load/Save and read/write handling                  */
/*************************************************************/

WRITE16_HANDLER( namcos2_68k_eeprom_w ){
	if( ACCESSING_BITS_0_7 )
	{
		namcos2_eeprom[offset] = data;
	}
}

READ16_HANDLER( namcos2_68k_eeprom_r ){
	return namcos2_eeprom[offset];
}

/*************************************************************/
/* 68000 Shared memory area - Data ROM area                  */
/*************************************************************/
READ16_HANDLER( namcos2_68k_data_rom_r ){
	UINT16 *ROM = (UINT16 *)space.machine().root_device().memregion("user1")->base();
	return ROM[offset];
}



/**************************************************************/
/* 68000 Shared serial communications processor (CPU5?)       */
/**************************************************************/

READ16_MEMBER( namcos2_state::serial_comms_ram_r ){
	return m_serial_comms_ram[offset];
}

WRITE16_MEMBER( namcos2_state::serial_comms_ram_w ){
	COMBINE_DATA( &m_serial_comms_ram[offset] );
}

READ16_MEMBER( namcos2_state::serial_comms_ctrl_r )
{
	UINT16 retval = m_serial_comms_ctrl[offset];

	switch(offset){
	case 0x00:
		retval |= 0x0004;	/* Set READY? status bit */
		break;

	default:
		break;
	}
	return retval;
}

WRITE16_MEMBER( namcos2_state::serial_comms_ctrl_w )
{
	COMBINE_DATA( &m_serial_comms_ctrl[offset] );
}

/*************************************************************/
/* 68000 Shared protection/random key generator              */
/*************************************************************

Custom chip ID numbers:

Game        Year    ID (dec)    ID (hex)
--------    ----    ---         -----
finallap    1987
assault     1988    unused
metlhawk    1988
ordyne      1988    176         $00b0
mirninja    1988    177         $00b1
phelios     1988    178         $00b2   readme says 179
dirtfoxj    1989    180         $00b4
fourtrax    1989
valkyrie    1989
finehour    1989    188         $00bc
burnforc    1989    189         $00bd
marvland    1989    190         $00be
kyukaidk    1990    191         $00bf
dsaber      1990    192         $00c0
finalap2    1990    318         $013e
rthun2      1990    319         $013f
gollygho    1990                $0143
cosmogng    1991    330         $014a
sgunner2    1991    346         $015a   ID out of order; gfx board is not standard
finalap3    1992    318         $013e   same as finalap2
suzuka8h    1992
sws92       1992    331         $014b
sws92g      1992    332         $014c
suzuk8h2    1993
sws93       1993    334         $014e
 *************************************************************/

READ16_HANDLER( namcos2_68k_key_r )
{
	switch (space.machine().driver_data<namcos2_shared_state>()->m_gametype)
	{
	case NAMCOS2_ORDYNE:
		switch(offset)
		{
		case 2: return 0x1001;
		case 3: return 0x1;
		case 4: return 0x110;
		case 5: return 0x10;
		case 6: return 0xB0;
		case 7: return 0xB0;
		}
		break;

	case NAMCOS2_STEEL_GUNNER_2:
		switch( offset )
		{
			case 4: return 0x15a;
		}
	    break;

	case NAMCOS2_MIRAI_NINJA:
		switch(offset)
		{
		case 7: return 0xB1;
		}
	    break;

	case NAMCOS2_PHELIOS:
		switch(offset)
		{
		case 0: return 0xF0;
		case 1: return 0xFF0;
		case 2: return 0xB2;
		case 3: return 0xB2;
		case 4: return 0xF;
		case 5: return 0xF00F;
		case 7: return 0xB2;
		}
	    break;

	case NAMCOS2_DIRT_FOX_JP:
		switch(offset)
		{
		case 1: return 0xB4;
		}
		break;

	case NAMCOS2_FINEST_HOUR:
		switch(offset)
		{
		case 7: return 0xBC;
		}
	    break;

	case NAMCOS2_BURNING_FORCE:
		switch(offset)
		{
		case 1: return 0xBD;
		}
		break;

	case NAMCOS2_MARVEL_LAND:
		switch(offset)
		{
		case 0: return 0x10;
		case 1: return 0x110;
		case 4: return 0xBE;
		case 6: return 0x1001;
		case 7: return (sendval==1)?0xBE:1;
		}
	    break;

	case NAMCOS2_DRAGON_SABER:
		switch(offset)
		{
		case 2: return 0xC0;
		}
		break;

	case NAMCOS2_ROLLING_THUNDER_2:
		switch(offset)
		{
		case 4:
			if( sendval == 1 ){
		        sendval = 0;
		        return 0x13F;
			}
			break;
		case 7:
			if( sendval == 1 ){
			    sendval = 0;
			    return 0x13F;
			}
			break;
		case 2: return 0;
		}
		break;

	case NAMCOS2_COSMO_GANG:
		switch(offset)
		{
		case 3: return 0x14A;
		}
		break;

	case NAMCOS2_SUPER_WSTADIUM:
		switch(offset)
		{
	//  case 3: return 0x142;
		case 4: return 0x142;
	//  case 3: popmessage("blah %08x",space.device().safe_pc());
		default: return space.machine().rand();
		}
		break;

	case NAMCOS2_SUPER_WSTADIUM_92:
		switch(offset)
		{
		case 3: return 0x14B;
		}
		break;

	case NAMCOS2_SUPER_WSTADIUM_92T:
		switch(offset)
		{
		case 3: return 0x14C;
		}
		break;

	case NAMCOS2_SUPER_WSTADIUM_93:
		switch(offset)
		{
		case 3: return 0x14E;
		}
		break;

	case NAMCOS2_SUZUKA_8_HOURS_2:
		switch(offset)
		{
		case 3: return 0x14D;
		case 2: return 0;
		}
		break;

	case NAMCOS2_GOLLY_GHOST:
		switch(offset)
		{
		case 0: return 2;
		case 1: return 2;
		case 2: return 0;
		case 4: return 0x143;
		}
		break;


	case NAMCOS2_BUBBLE_TROUBLE:
		switch(offset)
		{
		case 0: return 2; // not verified
		case 1: return 2; // not verified
		case 2: return 0; // not verified
		case 4: return 0x141;
		}
		break;
	}



	return space.machine().rand()&0xffff;
}

WRITE16_HANDLER( namcos2_68k_key_w )
{
	int gametype = space.machine().driver_data<namcos2_shared_state>()->m_gametype;
	if( gametype == NAMCOS2_MARVEL_LAND && offset == 5 )
	{
		if (data == 0x615E) sendval = 1;
	}
	if( gametype == NAMCOS2_ROLLING_THUNDER_2 && offset == 4 )
	{
		if (data == 0x13EC) sendval = 1;
	}
	if( gametype == NAMCOS2_ROLLING_THUNDER_2 && offset == 7 )
	{
		if (data == 0x13EC) sendval = 1;
	}
	if( gametype == NAMCOS2_MARVEL_LAND && offset == 6 )
	{
		if (data == 0x1001) sendval = 0;
	}
}

/*************************************************************/
/* 68000 Interrupt/IO Handlers - CUSTOM 148 - NOT SHARED     */
/*************************************************************/

#define NO_OF_LINES 	256
#define FRAME_TIME		(1.0/60.0)
#define LINE_LENGTH 	(FRAME_TIME/NO_OF_LINES)

static UINT16  namcos2_68k_master_C148[0x20];
static UINT16  namcos2_68k_slave_C148[0x20];
static UINT16  namcos2_68k_gpu_C148[0x20];


bool namcos2_shared_state::is_system21()
{
	switch (m_gametype)
	{
		case NAMCOS21_AIRCOMBAT:
		case NAMCOS21_STARBLADE:
		case NAMCOS21_CYBERSLED:
		case NAMCOS21_SOLVALOU:
		case NAMCOS21_WINRUN91:
		case NAMCOS21_DRIVERS_EYES:
			return 1;
		default:
			return 0;
	}
}

static void InitC148(void)
{
	int loop;

	for(loop = 0; loop < 0x20; loop++)
	{
		namcos2_68k_master_C148[loop] = 0;
		namcos2_68k_slave_C148[loop] = 0;
		namcos2_68k_gpu_C148[loop] = 0;
	}
}

static UINT16
ReadWriteC148( address_space &space, offs_t offset, UINT16 data, int bWrite )
{
	offs_t addr = ((offset * 2) + 0x1c0000) & 0x1fe000;
	device_t *altcpu = NULL;
	UINT16 *pC148Reg = NULL;
	UINT16 *pC148RegAlt = NULL;
	UINT16 result = 0;

	if (&space.device() == space.machine().device("maincpu"))
	{
		pC148Reg = namcos2_68k_master_C148;
		altcpu = space.machine().device("slave");
		pC148RegAlt = namcos2_68k_slave_C148;
	}
	else if (&space.device() == space.machine().device("slave"))
	{
		pC148Reg = namcos2_68k_slave_C148;
		altcpu = space.machine().device("maincpu");
		pC148RegAlt = namcos2_68k_master_C148;
	}
	else if (&space.device() == space.machine().device("gpu"))
	{
		pC148Reg = namcos2_68k_gpu_C148;
		altcpu = space.machine().device("maincpu");
		pC148RegAlt = namcos2_68k_master_C148;
	}

	if( bWrite )
	{
		int reg = (addr >> 13) & 0x1f;

		// If writing an IRQ priority register, clear any pending IRQs.
		// Dirt Fox and Winning Run require this behaviour
		if (reg < 8)
			space.device().execute().set_input_line(pC148Reg[reg], CLEAR_LINE);

		pC148Reg[reg] = data & 0x0007;
	}

	switch(addr)
	{
	case 0x1c0000: break; /* ? NAMCOS2_C148_0 */
	case 0x1c2000: break; /* ? NAMCOS2_C148_1 */
	case 0x1c4000: break; /* ? NAMCOS2_C148_2 */

	/* IRQ level */
	case 0x1c6000: break; /* NAMCOS2_C148_CPUIRQ */
	case 0x1c8000: break; /* NAMCOS2_C148_EXIRQ */
	case 0x1ca000: break; /* NAMCOS2_C148_POSIRQ */
	case 0x1cc000: break; /* NAMCOS2_C148_SERIRQ */
	case 0x1ce000: break; /* NAMCOS2_C148_VBLANKIRQ */

	case 0x1d0000: /* ? NAMCOS2_C148_0 */
		if( bWrite )
		{
			// mame_printf_debug( "cpu(%d) RAM[0x%06x] = 0x%x\n", cpu, addr, data );
			/* Dubious to assert IRQ for other CPU here, but Starblade seems to rely on it.
               It fails to show large polygons otherwise. */
			altcpu->execute().set_input_line(pC148RegAlt[NAMCOS2_C148_CPUIRQ], ASSERT_LINE);
		}
		break;

	case 0x1d2000: break; /* ? NAMCOS2_C148_1 */

	case 0x1d4000: /* ? NAMCOS2_C148_2 */
		if( bWrite )
		{
			// mame_printf_debug( "cpu(%d) RAM[0x%06x] = 0x%x\n", cpu, addr, data );
			/* Dubious to assert IRQ for other CPU here: Rolling Thunder 2 and Fine Hour break. */
			// altcpu->execute().set_input_line(pC148RegAlt[NAMCOS2_C148_CPUIRQ], ASSERT_LINE);
		}
		break;


	/* IRQ ack */
	case 0x1d6000: /* NAMCOS2_C148_CPUIRQ */
		// if( bWrite ) mame_printf_debug( "cpu(%d) RAM[0x%06x] = 0x%x\n", cpu, addr, data );
		space.device().execute().set_input_line(pC148Reg[NAMCOS2_C148_CPUIRQ], CLEAR_LINE);
		break;

	case 0x1d8000: /* NAMCOS2_C148_EXIRQ */
		// if( bWrite ) mame_printf_debug( "cpu(%d) RAM[0x%06x] = 0x%x\n", cpu, addr, data );
		space.device().execute().set_input_line(pC148Reg[NAMCOS2_C148_EXIRQ], CLEAR_LINE);
		break;

	case 0x1da000: /* NAMCOS2_C148_POSIRQ */
		// if( bWrite ) mame_printf_debug( "cpu(%d) RAM[0x%06x] = 0x%x\n", cpu, addr, data );
		space.device().execute().set_input_line(pC148Reg[NAMCOS2_C148_POSIRQ], CLEAR_LINE);
		break;

	case 0x1dc000: /* NAMCOS2_C148_SERIRQ */
		// if( bWrite ) mame_printf_debug( "cpu(%d) RAM[0x%06x] = 0x%x\n", cpu, addr, data );
		space.device().execute().set_input_line(pC148Reg[NAMCOS2_C148_SERIRQ], CLEAR_LINE);
		break;

	case 0x1de000: /* NAMCOS2_C148_VBLANKIRQ */
		space.device().execute().set_input_line(pC148Reg[NAMCOS2_C148_VBLANKIRQ], CLEAR_LINE);
		break;


	case 0x1e0000: /* EEPROM Status Register */
		result = ~0; /* Only BIT0 used: 1=EEPROM READY 0=EEPROM BUSY */
		break;

	case 0x1e2000: /* Sound CPU Reset control */
		if (&space.device() == space.machine().device("maincpu")) /* ? */
		{
			if (data & 0x01)
			{
				/* Resume execution */
				space.machine().device("audiocpu")->execute().set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
				space.device().execute().yield();
			}
			else
			{
				/* Suspend execution */
				space.machine().device("audiocpu")->execute().set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			}
			if (namcos2_kickstart != NULL)
			{
				//printf( "dspkick=0x%x\n", data );
				if (data & 0x04)
				{
					(*namcos2_kickstart)(space.machine(), 1);
				}
			}
		}
		break;

	case 0x1e4000: /* Alt 68000 & IO CPU Reset */
		if (&space.device() == space.machine().device("maincpu")) /* ? */
		{
			if (data & 0x01)
			{ /* Resume execution */
				ResetAllSubCPUs(space.machine(), CLEAR_LINE);
				/* Give the new CPU an immediate slice of the action */
				space.device().execute().yield();
			}
			else
			{ /* Suspend execution */
				ResetAllSubCPUs(space.machine(), ASSERT_LINE);
			}
		}
		break;

	case 0x1e6000: /* Watchdog reset kicker */
		/* watchdog_reset_w(0,0); */
		break;

	default:
		break;
	}
	return result;
}

WRITE16_HANDLER( namcos2_68k_master_C148_w )
{
	(void)ReadWriteC148(space, offset, data, 1);
}

READ16_HANDLER( namcos2_68k_master_C148_r )
{
	return ReadWriteC148(space, offset, 0, 0);
}

WRITE16_HANDLER( namcos2_68k_slave_C148_w )
{
	(void)ReadWriteC148(space, offset, data, 1);
}

READ16_HANDLER( namcos2_68k_slave_C148_r )
{
	return ReadWriteC148(space, offset, 0, 0);
}

WRITE16_HANDLER( namcos2_68k_gpu_C148_w )
{
	(void)ReadWriteC148(space, offset, data, 1);
}

READ16_HANDLER( namcos2_68k_gpu_C148_r )
{
	return ReadWriteC148(space, offset, 0, 0);
}


static int GetPosIRQScanline( running_machine &machine )
{
	namcos2_shared_state *state = machine.driver_data<namcos2_shared_state>();
	if (state->is_system21()) return 0;
	return downcast<namcos2_state *>(state)->get_pos_irq_scanline();
}

static TIMER_CALLBACK( namcos2_posirq_tick )
{
	namcos2_shared_state *state = machine.driver_data<namcos2_shared_state>();
	if (state->is_system21()) {
		if (namcos2_68k_gpu_C148[NAMCOS2_C148_POSIRQ]) {
			machine.primary_screen->update_partial(param);
			machine.device("gpu")->execute().set_input_line(namcos2_68k_gpu_C148[NAMCOS2_C148_POSIRQ] , ASSERT_LINE);
		}
		return;
	}

	if (namcos2_68k_master_C148[NAMCOS2_C148_POSIRQ]|namcos2_68k_slave_C148[NAMCOS2_C148_POSIRQ]) {
		machine.primary_screen->update_partial(param);
		if (namcos2_68k_master_C148[NAMCOS2_C148_POSIRQ]) machine.device("maincpu")->execute().set_input_line(namcos2_68k_master_C148[NAMCOS2_C148_POSIRQ] , ASSERT_LINE);
		if (namcos2_68k_slave_C148[NAMCOS2_C148_POSIRQ]) machine.device("slave")->execute().set_input_line(namcos2_68k_slave_C148[NAMCOS2_C148_POSIRQ] , ASSERT_LINE);
	}
}

void namcos2_adjust_posirq_timer( running_machine &machine, int scanline )
{
	namcos2_posirq_timer->adjust(machine.primary_screen->time_until_pos(scanline, 80), scanline);
}

INTERRUPT_GEN( namcos2_68k_master_vblank )
{
	namcos2_shared_state *state = device->machine().driver_data<namcos2_shared_state>();
	if (!state->is_system21()) namcos2_adjust_posirq_timer(device->machine(), GetPosIRQScanline(device->machine()));
	device->execute().set_input_line(namcos2_68k_master_C148[NAMCOS2_C148_VBLANKIRQ], HOLD_LINE);
}

INTERRUPT_GEN( namcos2_68k_slave_vblank )
{
	namcos2_shared_state *state = device->machine().driver_data<namcos2_shared_state>();
	if (!state->is_system21()) namcos2_adjust_posirq_timer(device->machine(), GetPosIRQScanline(device->machine()));
	device->execute().set_input_line(namcos2_68k_slave_C148[NAMCOS2_C148_VBLANKIRQ], HOLD_LINE);
}

INTERRUPT_GEN( namcos2_68k_gpu_vblank )
{
	/* only used by namcos21 */
	int scanline = GetPosIRQScanline(device->machine());
	scanline = 0x50+0x89; /* HACK for Winning Run */

	//printf( "namcos2_68k_gpu_vblank(%d)\n",namcos2_68k_gpu_C148[NAMCOS2_C148_POSIRQ] );
	namcos2_adjust_posirq_timer(device->machine(), scanline);
	device->execute().set_input_line(namcos2_68k_gpu_C148[NAMCOS2_C148_VBLANKIRQ], HOLD_LINE);
}

/**************************************************************/
/*  Sound sub-system                                          */
/**************************************************************/

WRITE8_HANDLER( namcos2_sound_bankselect_w )
{
	UINT8 *RAM=space.machine().root_device().memregion("audiocpu")->base();
	UINT32 max = (space.machine().root_device().memregion("audiocpu")->bytes() - 0x10000) / 0x4000;
	int bank = ( data >> 4 ) % max;	/* 991104.CAB */
	space.machine().root_device().membank(BANKED_SOUND_ROM)->set_base(&RAM[ 0x10000 + ( 0x4000 * bank ) ] );
}

/**************************************************************/
/*                                                            */
/*  68705 IO CPU Support functions                            */
/*                                                            */
/**************************************************************/

WRITE8_HANDLER( namcos2_mcu_analog_ctrl_w )
{
	namcos2_mcu_analog_ctrl = data & 0xff;

	/* Check if this is a start of conversion */
	/* Input ports 2 through 9 are the analog channels */

	if(data & 0x40)
	{
	/* Set the conversion complete flag */
		namcos2_mcu_analog_complete = 2;
		/* We convert instantly, good eh! */
		switch((data>>2) & 0x07)
		{
		case 0:
			namcos2_mcu_analog_data=space.machine().root_device().ioport("AN0")->read();
			break;
		case 1:
			namcos2_mcu_analog_data=space.machine().root_device().ioport("AN1")->read();
			break;
		case 2:
			namcos2_mcu_analog_data=space.machine().root_device().ioport("AN2")->read();
			break;
		case 3:
			namcos2_mcu_analog_data=space.machine().root_device().ioport("AN3")->read();
			break;
		case 4:
			namcos2_mcu_analog_data=space.machine().root_device().ioport("AN4")->read();
			break;
		case 5:
			namcos2_mcu_analog_data=space.machine().root_device().ioport("AN5")->read();
			break;
		case 6:
			namcos2_mcu_analog_data=space.machine().root_device().ioport("AN6")->read();
			break;
		case 7:
			namcos2_mcu_analog_data=space.machine().root_device().ioport("AN7")->read();
			break;
		default:
			output_set_value("anunk",data);
		}
#if 0
		/* Perform the offset handling on the input port */
		/* this converts it to a twos complement number */
		if( m_gametype == NAMCOS2_DIRT_FOX ||
			m_gametype == NAMCOS2_DIRT_FOX_JP )
		{
			namcos2_mcu_analog_data ^= 0x80;
		}
#endif
		/* If the interrupt enable bit is set trigger an A/D IRQ */
		if(data & 0x20)
		{
			generic_pulse_irq_line(space.machine().device("mcu"), HD63705_INT_ADCONV, 1);
		}
	}
}

READ8_HANDLER( namcos2_mcu_analog_ctrl_r )
{
	int data=0;

	/* ADEF flag is only cleared AFTER a read from control THEN a read from DATA */
	if(namcos2_mcu_analog_complete==2) namcos2_mcu_analog_complete=1;
	if(namcos2_mcu_analog_complete) data|=0x80;

	/* Mask on the lower 6 register bits, Irq EN/Channel/Clock */
	data|=namcos2_mcu_analog_ctrl&0x3f;
	/* Return the value */
	return data;
}

WRITE8_HANDLER( namcos2_mcu_analog_port_w )
{
}

READ8_HANDLER( namcos2_mcu_analog_port_r )
{
	if(namcos2_mcu_analog_complete==1) namcos2_mcu_analog_complete=0;
	return namcos2_mcu_analog_data;
}

WRITE8_HANDLER( namcos2_mcu_port_d_w )
{
	/* Undefined operation on write */
}

READ8_HANDLER( namcos2_mcu_port_d_r )
{
	/* Provides a digital version of the analog ports */
	int threshold = 0x7f;
	int data = 0;

	/* Read/convert the bits one at a time */
	if(space.machine().root_device().ioport("AN0")->read() > threshold) data |= 0x01;
	if(space.machine().root_device().ioport("AN1")->read() > threshold) data |= 0x02;
	if(space.machine().root_device().ioport("AN2")->read() > threshold) data |= 0x04;
	if(space.machine().root_device().ioport("AN3")->read() > threshold) data |= 0x08;
	if(space.machine().root_device().ioport("AN4")->read() > threshold) data |= 0x10;
	if(space.machine().root_device().ioport("AN5")->read() > threshold) data |= 0x20;
	if(space.machine().root_device().ioport("AN6")->read() > threshold) data |= 0x40;
	if(space.machine().root_device().ioport("AN7")->read() > threshold) data |= 0x80;

	/* Return the result */
	return data;
}

READ8_HANDLER( namcos2_input_port_0_r )
{
	int data = space.machine().root_device().ioport("MCUB")->read();
	return data;
}

READ8_HANDLER( namcos2_input_port_10_r )
{
	int data = space.machine().root_device().ioport("MCUH")->read();
	return data;
}

READ8_HANDLER( namcos2_input_port_12_r )
{
	int data = space.machine().root_device().ioport("MCUDI0")->read();
	return data;
}
