/* ST-V Inits and SpeedUp Hacks */
/* stvinit.c */

/*
to be honest i think some of these cause more problems than they're worth ...
*/

#include "driver.h"
#include "deprecat.h"
#include "machine/eeprom.h"
#include "cpu/sh2/sh2.h"
#include "machine/stvprot.h"
#include "includes/stv.h"

#define FIRST_SPEEDUP_SLOT	(2)			// in case we remove/alter the BIOS speedups later



/*
EEPROM write 0000 to address 2d
EEPROM write 0000 to address 2e
EEPROM write 0000 to address 2f
EEPROM write 0000 to address 30
EEPROM write ffff to address 31
EEPROM write ffff to address 32
EEPROM write ffff to address 33
EEPROM write ffff to address 34
EEPROM write ffff to address 35
EEPROM write ffff to address 36
EEPROM write ffff to address 37
EEPROM write ffff to address 38
EEPROM write ffff to address 39
EEPROM write ffff to address 3a
EEPROM write ffff to address 3b
EEPROM write ffff to address 3c
EEPROM write ffff to address 3d
EEPROM write ffff to address 3e
EEPROM write ffff to address 3f
*/
#if 0
static const UINT8 stv_default_eeprom[128] = {
    0x53,0x45,0xff,0xff,0xff,0xff,0x3b,0xe2,
    0x00,0x00,0x00,0x00,0x00,0x02,0x01,0x00,
    0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x08,
    0x08,0xfd,0x10,0x04,0x23,0x2a,0x00,0x00,
    0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0x3b,0xe2,0xff,0xff,0x00,0x00,
    0x00,0x01,0x01,0x00,0x01,0x01,0x00,0x00,
    0x00,0x00,0x00,0x08,0x08,0xfd,0x10,0x04,
    0x23,0x2a,0x00,0x00,0x00,0x00,0x00,0x00,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff
};
#endif

static const UINT8 shienryu_default_eeprom[128] = {
	0x53,0x45,0x47,0x41,0x3b,0xe2,0x5e,0x09,
	0x5e,0x09,0x00,0x00,0x00,0x00,0x00,0x02,
	0x01,0x00,0x01,0x01,0x00,0x00,0x00,0x00,
	0x00,0x08,0x18,0xfd,0x18,0x01,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0x5e,0x09,0x00,0x00,
	0x00,0x00,0x00,0x02,0x01,0x00,0x01,0x01,
	0x00,0x00,0x00,0x00,0x00,0x08,0x18,0xfd,
	0x18,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

static const UINT8 *stv_default_eeprom;
static int stv_default_eeprom_length;

NVRAM_HANDLER( stv )
{
	if (read_or_write)
		eeprom_save(file);
	else
	{
		eeprom_init(&eeprom_interface_93C46);

		if (file) eeprom_load(file);
		else
		{
			if (stv_default_eeprom)	/* Set the EEPROM to Factory Defaults */
				eeprom_set_data(stv_default_eeprom,stv_default_eeprom_length);
		}
	}
}

/*

06013AE8: MOV.L   @($D4,PC),R5
06013AEA: MOV.L   @($D8,PC),R0
06013AEC: MOV.W   @R5,R5
06013AEE: MOV.L   @R0,R0
06013AF0: AND     R10,R5
06013AF2: TST     R0,R0
06013AF4: BTS     $06013B00
06013AF6: EXTS.W  R5,R5
06013B00: EXTS.W  R5,R5
06013B02: TST     R5,R5
06013B04: BF      $06013B0A
06013B06: TST     R4,R4
06013B08: BT      $06013AE8

   (loops for 375868 instructions)

*/


static READ32_HANDLER( stv_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x60154b2) cpu_spinuntil_int(machine->activecpu); // bios menus..
	cpu_spinuntil_int(machine->activecpu);

	return stv_workram_h[0x0335d0/4];
}

static READ32_HANDLER( stv_speedup2_r )
{
	if (cpu_get_pc(machine->activecpu)==0x6013aee) cpu_spinuntil_int(machine->activecpu); // for use in japan

	cpu_spinuntil_int(machine->activecpu);

	return stv_workram_h[0x0335bc/4];
}

void install_stvbios_speedups(running_machine *machine)
{
	// flushes 0 & 1 on both CPUs are for the BIOS speedups
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, 0);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60154b2);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, 1);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6013aee);

	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, 0);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60154b2);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, 1);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6013aee);

/* idle skip bios? .. not 100% sure this is safe .. we'll see */
	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60335d0, 0x60335d3, 0, 0, stv_speedup_r );
	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60335bc, 0x60335bf, 0, 0, stv_speedup2_r );
}

static READ32_HANDLER( shienryu_slave_speedup_r )
{
 if (cpu_get_pc(machine->activecpu)==0x0600440e)
  cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(20)); // is this safe... we can't skip till vbl because its not a vbl wait loop

 return stv_workram_h[0x0ae8e4/4];
}


static READ32_HANDLER( shienryu_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x060041C6) cpu_spinuntil_int(machine->activecpu); // after you enable the sound cpu ...
	return stv_workram_h[0x0ae8e0/4];
}


DRIVER_INIT(shienryu)
{
	stv_default_eeprom = shienryu_default_eeprom;
	stv_default_eeprom_length = sizeof(shienryu_default_eeprom);

	// master
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60041c6);
	// slave
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x600440e);

	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60ae8e0, 0x60ae8e3, 0, 0, shienryu_speedup_r ); // after you enable sound cpu
	memory_install_read32_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x60ae8e4, 0x60ae8e7, 0, 0, shienryu_slave_speedup_r ); // after you enable sound cpu

	DRIVER_INIT_CALL(stv);
}

static READ32_HANDLER( prikura_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x6018640) cpu_spinuntil_int(machine->activecpu); // after you enable the sound cpu ...
	return stv_workram_h[0x0b9228/4];
}

static void prikura_slave_speedup( UINT32 data )
{
	if ( cpu_get_pc(Machine->activecpu) == 0x06018c6e )
		if ( (data & 0x00800000) == 0 )
			cpu_spinuntil_trigger(Machine->cpu[1], 1000);
}

DRIVER_INIT(prikura)
{
/*
 06018640: MOV.B   @R14,R0  // 60b9228
 06018642: TST     R0,R0
 06018644: BF      $06018640

    (loops for 263473 instructions)
*/
	// master
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6018640);
	// slave
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6018c6e);

	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60b9228, 0x60b922b, 0, 0, prikura_speedup_r );
	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)prikura_slave_speedup );

	DRIVER_INIT_CALL(stv);

	minit_boost = sinit_boost = 0;
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);

}


static READ32_HANDLER( hanagumi_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x06010160) cpu_spinuntil_int(machine->activecpu); // title logos

	return stv_workram_h[0x94188/4];
}

static READ32_HANDLER( hanagumi_slave_off )
{
	/* just turn the slave off, i don't think the game needs it */
	cpu_set_input_line(machine->cpu[1], INPUT_LINE_HALT, ASSERT_LINE);

	return stv_workram_h[0x015438/4];
}

DRIVER_INIT(hanagumi)
{
/*
    06013E1E: NOP
    0601015E: MOV.L   @($6C,PC),R3
    06010160: MOV.L   @R3,R0  (6094188)
    06010162: TST     R0,R0
    06010164: BT      $0601015A
    0601015A: JSR     R14
    0601015C: NOP
    06013E20: MOV.L   @($34,PC),R3
    06013E22: MOV.B   @($00,R3),R0
    06013E24: TST     R0,R0
    06013E26: BT      $06013E1C
    06013E1C: RTS
    06013E1E: NOP

   (loops for 288688 instructions)
*/
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6010160);

   	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x6094188, 0x609418b, 0, 0, hanagumi_speedup_r );
   	memory_install_read32_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x6015438, 0x601543b, 0, 0, hanagumi_slave_off );

	DRIVER_INIT_CALL(stv);
}




/* these idle loops might change if the interrupts change / are fixed because i don't really think these are vblank waits... */

/* puyosun

CPU0: Aids Screen

06021CF0: MOV.B   @($13,GBR),R0 (60ffc13)
06021CF2: CMP/PZ  R0
06021CF4: BT      $06021CF0
   (loops for xxx instructions)

   this is still very slow .. but i don't think it can be sped up further


*/

static READ32_HANDLER( puyosun_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x6021CF0) cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(400)); // spinuntilint breaks controls again .. urgh


	return stv_workram_h[0x0ffc10/4];
}

static void puyosun_slave_speedup( UINT32 data )
{
	if ( cpu_get_pc(Machine->activecpu) == 0x60236fe )
		if ( (data & 0x00800000) == 0 )
			cpu_spinuntil_trigger(Machine->cpu[1], 1000);
}

DRIVER_INIT(puyosun)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6021cf0);

	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60236fe);

   	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, puyosun_speedup_r ); // idle loop of main cpu
	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)puyosun_slave_speedup );

	DRIVER_INIT_CALL(stv);

	minit_boost = sinit_boost = 0;
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

/* mausuke

CPU0 Data East Logo:
060461A0: MOV.B   @($13,GBR),R0  (60ffc13)
060461A2: CMP/PZ  R0
060461A4: BT      $060461A0
   (loops for 232602 instructions)

*/

static READ32_HANDLER( mausuke_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x060461A0) cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(20)); // spinuntilint breaks controls again .. urgh

	return stv_workram_h[0x0ffc10/4];
}

DRIVER_INIT(mausuke)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60461A0);

   	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, mausuke_speedup_r ); // idle loop of main cpu

	DRIVER_INIT_CALL(stv);

	minit_boost = sinit_boost = 0;
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

#if 0
static READ32_HANDLER( cottonbm_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x06030EE2) cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(20)); // spinuntilint breaks lots of things

	return stv_workram_h[0x0ffc10/4];
}

static void cottonbm_slave_speedup( UINT32 data )
{
	if (cpu_get_pc(Machine->activecpu) == 0x6032b52)
		if ( (data & 0x00800000) == 0 )
		{
			if (
		   (stv_workram_h[0x0ffc44/4] != 0x260fbe34) &&
		   (stv_workram_h[0x0ffc48/4] != 0x260fbe34) &&
		   (stv_workram_h[0x0ffc44/4] != 0x260fbe2c) &&
		   (stv_workram_h[0x0ffc48/4] != 0x260fbe2c)
			)
			{
				logerror("cpu1 skip %08x %08x\n",stv_workram_h[0x0ffc44/4],stv_workram_h[0x0ffc48/4]);

				cpu_spinuntil_trigger(Machine->cpu[1], 1000);
			}
		}
}
#endif

DRIVER_INIT(cottonbm)
{
//  cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
//  cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6030ee2);
//  cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+1);
//  cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6032b52);

//  cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)cottonbm_slave_speedup );
//      memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, cottonbm_speedup_r ); // idle loop of main cpu

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(10);
}

static READ32_HANDLER( cotton2_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x06031c7a) cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(20)); // spinuntilint breaks lots of things

	return stv_workram_h[0x0ffc10/4];
}

static void cotton2_slave_speedup( UINT32 data )
{
	if (cpu_get_pc(Machine->activecpu) == 0x60338ea)
		if ( (data & 0x00800000) == 0 )
		{
			if (
			(stv_workram_h[0x0ffc44/4] != 0x260fd264) &&
			(stv_workram_h[0x0ffc48/4] != 0x260fd264) &&
			(stv_workram_h[0x0ffc44/4] != 0x260fd25c) &&
			(stv_workram_h[0x0ffc48/4] != 0x260fd25c)
			)
			{
				logerror("cpu1 skip %08x %08x\n",stv_workram_h[0x0ffc44/4],stv_workram_h[0x0ffc48/4]);

				cpu_spinuntil_trigger(Machine->cpu[1], 1000);
			}
		}
}

DRIVER_INIT(cotton2)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6031c7a);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60338ea);

   	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, cotton2_speedup_r ); // idle loop of main cpu
	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)cotton2_slave_speedup );

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

static int dnmtdeka_pending_commands;

static READ32_HANDLER( dnmtdeka_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x6027c90) cpu_spinuntil_int(machine->activecpu);//cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x0985a0/4];
}

static WRITE32_HANDLER(dnmtdeka_cmd_write)
{
	COMBINE_DATA(&stv_workram_h[0x0e0ad4/4 + offset]);
	if ( (cpunum_get_active() == 0) && (cpu_get_pc(machine->activecpu) == 0x00000d04) )
		return;

	if ( data != 0 ) dnmtdeka_pending_commands++;
	//logerror( "CMD: Written by cpu=%d, at = %08X, offset = %08X, data = %08X, commands = %d\n", cpunum_get_active(), cpu_get_pc(machine->activecpu), offset, data, dnmtdeka_pending_commands );
	cpuexec_trigger(machine, 1000);
}

static READ32_HANDLER(dnmtdeka_cmd_read)
{
	//logerror( "CMD: Read by cpu=%d, at = %08X, offset = %08X, data = %08X, commands = %d\n", cpunum_get_active(), cpu_get_pc(machine->activecpu), offset, stv_workram_h[0xe0bd0/4 + offset], dnmtdeka_pending_commands );
	if ( cpu_get_pc(machine->activecpu) == 0x060051f2 )
	{
		if ( stv_workram_h[0x0e0ad4/4 + offset] == 0 )
		{
			if ( dnmtdeka_pending_commands == 0 )
				cpu_spinuntil_trigger(machine->activecpu, 1000);
		}
		else
		{
			dnmtdeka_pending_commands--;
		}
	}

	return stv_workram_h[0x0e0ad4/4 + offset];
}


DRIVER_INIT(dnmtdeka)
{
	// install all 3 speedups on both master and slave
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6027c90);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+1);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0xd04);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+2);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60051f2);

	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6027c90);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+1);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0xd04);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+2);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60051f2);

	dnmtdeka_pending_commands = 0;

	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60985a0, 0x60985a3, 0, 0, dnmtdeka_speedup_r ); // idle loop of main cpu
	memory_install_readwrite32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x060e0ad4, 0x060e0bc3, 0, 0, dnmtdeka_cmd_read, dnmtdeka_cmd_write );

	DRIVER_INIT_CALL(stv);
}

static int diehard_pending_commands;

static READ32_HANDLER(diehard_speedup_r)
{
	if ( cpu_get_pc(machine->activecpu) == 0x06027c98 ) cpu_spinuntil_int(machine->activecpu);
	return stv_workram_h[0x000986ac/4];
}

static WRITE32_HANDLER(diehard_cmd_write)
{
	COMBINE_DATA(&stv_workram_h[0xe0bd0/4 + offset]);
	if ( (cpunum_get_active() == 0) && (cpu_get_pc(machine->activecpu) == 0x00000d04) )
		return;

	if ( data != 0 ) diehard_pending_commands++;
	//logerror( "CMD: Written by cpu=%d, at = %08X, offset = %08X, data = %08X, commands = %d\n", cpunum_get_active(), cpu_get_pc(machine->activecpu), offset, data, diehard_pending_commands );
	cpuexec_trigger(machine, 1000);
}

static READ32_HANDLER(diehard_cmd_read)
{
	//logerror( "CMD: Read by cpu=%d, at = %08X, offset = %08X, data = %08X, commands = %d\n", cpunum_get_active(), cpu_get_pc(machine->activecpu), offset, stv_workram_h[0xe0bd0/4 + offset], diehard_pending_commands );
	if ( cpu_get_pc(machine->activecpu) == 0x060051f2 )
	{
		if ( stv_workram_h[0xe0bd0/4 + offset] == 0 )
		{
			if ( diehard_pending_commands == 0 )
				cpu_spinuntil_trigger(machine->activecpu, 1000);
		}
	}

	return stv_workram_h[0xe0bd0/4 + offset];
}

static READ32_HANDLER(diehard_cmd_ack_read)
{
	//logerror( "CMDACK: Read by cpu=%d, at = %08X, offset = %08X, data = %08X, commands = %d\n", cpunum_get_active(), cpu_get_pc(machine->activecpu), offset, stv_workram_h[0x000e0dd8/4], diehard_pending_commands );
	if ( (stv_workram_h[0x000e0dd8/4] & 0xff000000) == 0 &&
		 diehard_pending_commands == 0 )
	{
		cpuexec_trigger(machine, 1000);
	}
	return stv_workram_h[0x000e0dd8/4];
}

static WRITE32_HANDLER(diehard_cmd_ack_write)
{
	//logerror( "CMDACK: Write by cpu=%d, at = %08X, offset = %08X, data = %08X, commands = %d\n", cpunum_get_active(), cpu_get_pc(machine->activecpu), offset, data, diehard_pending_commands );
	if ( diehard_pending_commands > 0 )
	{
		diehard_pending_commands--;
	}
	COMBINE_DATA(&stv_workram_h[0x000e0dd8/4]);
}

static WRITE32_HANDLER(diehard_cmd_ack_write_cpu0)
{
	//logerror( "CMDACK: Write by cpu=%d, at = %08X, offset = %08X, data = %08X, commands = %d\n", cpunum_get_active(), cpu_get_pc(machine->activecpu), offset, data, diehard_pending_commands );
	COMBINE_DATA(&stv_workram_h[0x000e0dd8/4]);
	cpuexec_trigger(machine, 1000);
}

DRIVER_INIT(diehard)
{
	diehard_pending_commands = 0;

	// install all 3 speedups on both master and slave
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6027c98);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+1);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0xd04);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+2);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60051f2);

	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6027c98);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+1);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0xd04);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+2);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60051f2);


	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x060986ac, 0x060986af, 0, 0, diehard_speedup_r );
	memory_install_write32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x060e0bd0, 0x060e0dcf, 0, 0, diehard_cmd_write );
	memory_install_read32_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x060e0bd0, 0x060e0dcf, 0, 0, diehard_cmd_read );
	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x060e0dd8, 0x060e0ddb, 0, 0, diehard_cmd_ack_read );
	memory_install_write32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x060e0dd8, 0x060e0ddb, 0, 0, diehard_cmd_ack_write_cpu0 );
	memory_install_write32_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x060e0dd8, 0x060e0ddb, 0, 0, diehard_cmd_ack_write );


	DRIVER_INIT_CALL(stv);
}

static READ32_HANDLER( fhboxers_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x060041c2) cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(20));

	return stv_workram_h[0x00420c/4];
}

static READ32_HANDLER( fhboxers_speedup2_r )
{
	if (cpu_get_pc(machine->activecpu)==0x0600bb0a) cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(20));


	return stv_workram_h[0x090740/4];
}

static READ32_HANDLER( fhboxers_speedup3_r )
{
	if (cpu_get_pc(machine->activecpu)==0x0600b31e )
		cpu_spinuntil_int(machine->activecpu);

	return stv_workram_h[0x90bb4/4];
}

DRIVER_INIT(fhboxers)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60041c2);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+1);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x600bb0a);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+2);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x600b31e);

   	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x600420c, 0x600420f, 0, 0, fhboxers_speedup_r ); // idle loop of main cpu
   	memory_install_read32_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x6090740, 0x6090743, 0, 0, fhboxers_speedup2_r ); // idle loop of second cpu
  	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x06090bb4, 0x06090bb7, 0, 0, fhboxers_speedup3_r ); // idle loop of main cpu

	DRIVER_INIT_CALL(stv);
}




static READ32_HANDLER( groovef_hack1_r )
{
	if(cpu_get_pc(machine->activecpu) == 0x6005e7c) stv_workram_h[0x0fffcc/4] = 0x00000000;
//  popmessage("1 %08x",cpu_get_pc(machine->activecpu));
	return stv_workram_h[0x0fffcc/4];
}

static READ32_HANDLER( groovef_hack2_r )
{
	if(cpu_get_pc(machine->activecpu) == 0x6005e86) stv_workram_h[0x0ca6cc/4] = 0x00000000;
//  popmessage("2 %08x",cpu_get_pc(machine->activecpu));
	return stv_workram_h[0x0ca6cc/4];
}

static READ32_HANDLER( groovef_speedup_r )
{
//  logerror ("groove speedup \n");
	if (cpu_get_pc(machine->activecpu)==0x060a4970)
	{
		cpu_spinuntil_int(machine->activecpu); // title logos
//      logerror ("groove speedup skipping\n");

	}

	return stv_workram_h[0x0c64ec/4];
}

#ifdef UNUSED_FUNCTION
static READ32_HANDLER( groovef_second_cpu_off_r )
{
    if (cpu_get_pc(machine->activecpu)==0x060060c2)     cpu_set_input_line(machine->cpu[1], INPUT_LINE_HALT, ASSERT_LINE);
    return 0;
}
#endif

static void groovef_slave_speedup( UINT32 data )
{
	if ( cpu_get_pc(Machine->activecpu) == 0x060060c2 )
		if ( (data & 0x00800000) == 0 )
			cpu_spinuntil_trigger(Machine->cpu[1], 1000);
}

DRIVER_INIT( groovef )
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6005e7c);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+1);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6005e86);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+2);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60a4970);

	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60060c2);

	/* prevent game from hanging on startup -- todo: remove these hacks */
	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60ca6cc, 0x60ca6cf, 0, 0, groovef_hack2_r );
	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60fffcc, 0x60fffcf, 0, 0, groovef_hack1_r );

	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60c64ec, 0x60c64ef, 0, 0, groovef_speedup_r );
//  memory_install_read32_handler(machine, 1, ADDRESS_SPACE_PROGRAM, 0x60060dc, 0x60060df, 0, 0, groovef_second_cpu_off_r ); // not a good idea, needs it for ai.
	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)groovef_slave_speedup );

	DRIVER_INIT_CALL(stv);

	minit_boost = sinit_boost = 0;
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

/* danchih hangs on the title screen without this hack .. */

/*  info from Saturnin Author

> seems to be fully playable, can you be more specific about the scu level 2
> dma stuff? i'd prefer a real solution than this hack, it could affect
other
> games too for all i know.

Unfortunalely I don't know much more about it : I got this info from a
person
who ran the SGL object files through objdump ...

0x060ffcbc _DMASt_SCU1
0x060ffcbd _DMASt_SCU2

But when I got games looping on thoses locations, the problem was related to
a
SCU interrupt (in indirect mode) which was registered but never triggered, a
bug in my SH2 core prevented the SR bits to be correctly filled in some
cases ...
When the interrupt is correctly triggered, I don't have these loops anymore,
and Hanafuda works without hack now (unless the sound ram one)


*/

static READ32_HANDLER( danchih_hack_r )
{
	logerror( "DMASt_SCU1: Read at PC=%08x, value = %08x\n", cpu_get_pc(machine->activecpu), stv_workram_h[0x0ffcbc/4] );
	if (cpu_get_pc(machine->activecpu)==0x06028b28) return 0x0e0c0000;

	return stv_workram_h[0x0ffcbc/4];
}

static READ32_HANDLER( danchih_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x06028c8e) cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x0ffc10/4];
}

static void danchih_slave_speedup( UINT32 data )
{
	if ( cpu_get_pc(Machine->activecpu) == 0x0602ae26 )
		if ( (data & 0x00800000) == 0 )
			cpu_spinuntil_trigger(Machine->cpu[1], 1000);
}

DRIVER_INIT( danchih )
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6028b28);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+1);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6028c8e);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x602ae26);

	/* prevent game from hanging on title screen -- todo: remove these hacks */
	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60ffcbc, 0x60ffcbf, 0, 0, danchih_hack_r );

	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, danchih_speedup_r );
	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)danchih_slave_speedup );

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(5);

}

/*
060011AE: AND     #$0F,R0
060011B0: MOV     #$5E,R1
060011B2: ADD     R5,R1
060011B4: MOV.B   R0,@R1
060011B6: MOV     R5,R0
060011B8: ADD     #$70,R0

060011BA: MOV.B   @(R0,R4),R0 <- reads 0x02020000,cause of the crash
060011BC: RTS
060011BE: NOP
060131AA: CMP/EQ  #$01,R0
060131AC: BT      $0601321C
060131AE: CMP/EQ  #$02,R0
060131B0: BT      $0601324A

TODO: understand where it gets 0x02020000,it must be 0x0000000

*/

static READ32_HANDLER( astrass_hack_r )
{
	/*PC reads at 0x60011ba if -debug is active?*/
	if(cpu_get_pc(machine->activecpu)==0x60011b8 || cpu_get_pc(machine->activecpu) == 0x60011ba) return 0x00000000;

	return stv_workram_h[0x000770/4];
}

static READ32_HANDLER( astrass_speedup_r )
{
	if(cpu_get_pc(machine->activecpu) == 0x0605b9da )
		cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(20));

	return stv_workram_h[0x8e4d8/4];
}

DRIVER_INIT( astrass )
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60011b8);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+1);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x605b9da);

	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x06000770, 0x06000773, 0, 0, astrass_hack_r );

	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x0608e4d8, 0x0608e4db, 0, 0, astrass_speedup_r );

	install_astrass_protection(machine);

	DRIVER_INIT_CALL(stv);
}

/* Treasure Hunt idle loop skipping */

static READ32_HANDLER(thunt_speedup_r)
{
	if (cpu_get_pc(machine->activecpu) == 0x0602A024) cpu_spinuntil_int(machine->activecpu);
	return stv_workram_h[0x00031424/4];
}

static READ32_HANDLER(thunt_speedup2_r)
{
	if (cpu_get_pc(machine->activecpu) == 0x06013EEA) cpu_spinuntil_int(machine->activecpu);
	return stv_workram_h[0x00075958/4];
}

static void thunt_slave_speedup(UINT32 data)
{
	if (cpu_get_pc(Machine->activecpu) == 0x0602AAF8)
		if ( (data & 0x00800000) == 0 )
			cpu_spinuntil_trigger(Machine->cpu[1], 1000);
}

DRIVER_INIT(thunt)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x602A024);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+1);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6013EEA);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x602AAF8);

/*
0602A024: MOV.L   @R6,R0    // 06031424
0602A026: TST     R0,R0
0602A028: BF      $0602A024
*/
	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x06031424, 0x06031427, 0, 0, thunt_speedup_r );

/*
06013EE8: MOV.L   @($10,PC),R0
06013EEA: MOV.B   @R0,R0
06013EEC: EXTU.B  R0,R0
06013EEE: TST     R0,R0
06013EF0: BT      $06013EF6
06013EF2: RTS
06013EF4: MOV     #$01,R0
*/
	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x06075958, 0x0607595b, 0, 0, thunt_speedup2_r );

/*
0602AAF8: MOV.B   @R11,R2
0602AAFA: EXTU.B  R2,R2
0602AAFC: AND     R13,R2
0602AAFE: CMP/EQ  R13,R2
0602AB00: BF      $0602AB28
0602AB28: BRA     $0602AAF8
*/

	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf *)thunt_slave_speedup);

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(1);
}

static READ32_HANDLER(sandor_speedup_r)
{
	if (cpu_get_pc(machine->activecpu) == 0x0602a0f8) cpu_spinuntil_int(machine->activecpu);
	return stv_workram_h[0x000314f8/4];
}

static READ32_HANDLER(sandor_speedup2_r)
{
	if (cpu_get_pc(machine->activecpu) == 0x06013fbe) cpu_spinuntil_int(machine->activecpu);
	return stv_workram_h[0x00075a2c/4];
}


static void sandor_slave_speedup(UINT32 data)
{
	if (cpu_get_pc(Machine->activecpu) == 0x0602abcc)
		if ( (data & 0x00800000) == 0 )
			cpu_spinuntil_trigger(Machine->cpu[1], 1000);
}

DRIVER_INIT(sandor)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x602a0f8);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+1);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6013fbe);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x602abcc);

	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x060314f8, 0x060314fb, 0, 0, sandor_speedup_r );
	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x06075a2c, 0x06075a2f, 0, 0, sandor_speedup2_r );
	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf *)sandor_slave_speedup);
	DRIVER_INIT_CALL(stv);
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(1);

}

static READ32_HANDLER(grdforce_speedup_r)
{
	if ( cpu_get_pc(machine->activecpu) == 0x06041E32 ) cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x000ffc10/4];
}

static void grdforce_slave_speedup( UINT32 data )
{
	if (cpu_get_pc(Machine->activecpu) == 0x06043aa2)
		if ( (data & 0x00800000) == 0 )
		{
			if (
			(stv_workram_h[0x0ffc44/4] != 0x260fd258) &&
			(stv_workram_h[0x0ffc48/4] != 0x260fd258) &&
			(stv_workram_h[0x0ffc44/4] != 0x260fd25c) &&
			(stv_workram_h[0x0ffc48/4] != 0x260fd25c)
			)
			{
				logerror("cpu1 skip %08x %08x\n",stv_workram_h[0x0ffc44/4],stv_workram_h[0x0ffc48/4]);

				//cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(200));
				cpu_spinuntil_trigger(Machine->cpu[1], 1000);
			}
		}
}

DRIVER_INIT(grdforce)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6041e32);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6043aa2);
/*
06041E2C: MOV.L   @($03C8,GBR),R0
06041E2E: JSR     R0
06041E30: NOP
06041A44: RTS
06041A46: NOP
06041E32: MOV.B   @($13,GBR),R0 //060ffc13
06041E34: CMP/PZ  R0
06041E36: BT      $06041E2C
*/
	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x060ffc10, 0x060ffc13, 0, 0, grdforce_speedup_r );

	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf *)grdforce_slave_speedup);

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

static READ32_HANDLER( batmanfr_speedup_r )
{
	//logerror( "batmanfr speedup: pc = %08x, mem = %08x\n", cpu_get_pc(machine->activecpu), stv_workram_h[0x0002acf0/4] );
	if ( cpu_get_pc(machine->activecpu) != 0x060121c0 )
		cpu_spinuntil_int(machine->activecpu);

	return stv_workram_h[0x0002acf0/4];
}

static void batmanfr_slave_speedup( UINT32 data )
{
	if (cpu_get_pc(Machine->activecpu) == 0x060125bc )
		if ( (data & 0x00800000) == 0 )
			cpu_spinuntil_trigger(Machine->cpu[1], 1000);
}


DRIVER_INIT(batmanfr)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60121c0);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60125bc);

	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x0602acf0, 0x0602acf3, 0, 0, batmanfr_speedup_r );
	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)batmanfr_slave_speedup );

	DRIVER_INIT_CALL(stv);

	minit_boost = sinit_boost = 0;
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

static void colmns97_slave_speedup( UINT32 data )
{
	if (cpu_get_pc(Machine->activecpu) == 0x060298a2 )
		if ( (data & 0x00800000) == 0 )
			if ( (stv_workram_h[0x0ffc48/4] != 0x260ef3fc) )
			{
				logerror("cpu1 skip %08x %08x\n",stv_workram_h[0x0ffc44/4],stv_workram_h[0x0ffc48/4]);
				cpu_spinuntil_trigger(Machine->cpu[1], 1000);
			}

}

DRIVER_INIT(colmns97)
{
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60298a2);

	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)colmns97_slave_speedup );

	DRIVER_INIT_CALL(stv);

	minit_boost = sinit_boost = 0;

}

static READ32_HANDLER(winterht_speedup_r)
{
	if ( cpu_get_pc(machine->activecpu) == 0x06098aea ) cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(20));//cpu_spinuntil_int(machine->activecpu);
	return stv_workram_h[0x000ffc10/4];
}

static void winterht_slave_speedup( UINT32 data )
{
	if (cpu_get_pc(Machine->activecpu) == 0x0609ae4e )
		if ( (data & 0x00800000) == 0 )
			cpu_spinuntil_trigger(Machine->cpu[1], 1000);
}

DRIVER_INIT(winterht)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6098aea);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x609ae4e);

	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x060ffc10, 0x060ffc13, 0, 0, winterht_speedup_r );

	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)winterht_slave_speedup );

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(2);
}

static READ32_HANDLER(seabass_speedup_r)
{
	if ( cpu_get_pc(machine->activecpu) == 0x0602cbfa ) cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x000ffc10/4];
}

static void seabass_slave_speedup( UINT32 data )
{
	if (cpu_get_pc(Machine->activecpu) == 0x060321ee )
		if ( (data & 0x00800000) == 0 )
			cpu_spinuntil_trigger(Machine->cpu[1], 1000);
}


DRIVER_INIT(seabass)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x602cbfa);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60321ee);

	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x060ffc10, 0x060ffc13, 0, 0, seabass_speedup_r );

	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)seabass_slave_speedup );

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(5);
}

static void vfremix_slave_speedup( UINT32 data )
{
	if (cpu_get_pc(Machine->activecpu) == 0x0604C332 )
		if ( (data & 0x00800000) == 0 )
			cpu_spinuntil_trigger(Machine->cpu[1], 1000);
}

static READ32_HANDLER(vfremix_speedup_r)
{
	if ( cpu_get_pc(machine->activecpu) == 0x0602c30c ) cpu_spinuntil_int(machine->activecpu);
	return stv_workram_h[0x00074f98/4];
}

DRIVER_INIT(vfremix)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x602c30c);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x604c332);

	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x06074f98, 0x06074f9b, 0, 0, vfremix_speedup_r );
	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)vfremix_slave_speedup );

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(20);
}


static READ32_HANDLER(sss_speedup_r)
{
	if ( cpu_get_pc(machine->activecpu) == 0x06026398 ) cpu_spinuntil_int(machine->activecpu);
	return stv_workram_h[0x000ffc10/4];
}

static void sss_slave_speedup( UINT32 data )
{
	if (cpu_get_pc(Machine->activecpu) == 0x06028cd6 )
		if ( (data & 0x00800000) == 0 )
			cpu_spinuntil_trigger(Machine->cpu[1], 1000);
}

DRIVER_INIT(sss)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6026398);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6028cd6);

	install_standard_protection(machine);
	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x060ffc10, 0x060ffc13, 0, 0, sss_speedup_r );
	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)sss_slave_speedup );

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

static READ32_HANDLER(othellos_speedup_r)
{
	if ( cpu_get_pc(machine->activecpu) == 0x0602bcbe ) cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x000ffc10/4];
}

static void othellos_slave_speedup( UINT32 data )
{
	if (cpu_get_pc(Machine->activecpu) == 0x0602d92e )
		if ( (data & 0x00800000) == 0 )
			if ( (stv_workram_h[0x0ffc48/4] != 0x260fd25c ) )
			{
				logerror("cpu1 skip %08x %08x\n",stv_workram_h[0x0ffc44/4],stv_workram_h[0x0ffc48/4]);
				cpu_spinuntil_trigger(Machine->cpu[1], 1000);
			}
}

DRIVER_INIT(othellos)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x602bcbe);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x602d92e);

	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x060ffc10, 0x060ffc13, 0, 0, othellos_speedup_r );
	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)othellos_slave_speedup );

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);

}

static void sasissu_slave_speedup( UINT32 data )
{
	if ( cpu_get_pc(Machine->activecpu) == 0x060710be )
		if ( (data & 0x00800000) == 0 )
			cpu_spinuntil_trigger(Machine->cpu[1], 1000);
}

DRIVER_INIT(sasissu)
{
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60710be);

	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)sasissu_slave_speedup );

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(2);
}

static READ32_HANDLER(gaxeduel_speedup_r)
{
	if ( cpu_get_pc(machine->activecpu) == 0x06012ee4 ) cpu_spinuntil_int(machine->activecpu);
	return stv_workram_l[0x000f4068 / 4];
}

DRIVER_INIT(gaxeduel)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6012ee4);

	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x002f4068, 0x002f406b, 0, 0, gaxeduel_speedup_r);
	DRIVER_INIT_CALL(stv);
}

static READ32_HANDLER(suikoenb_speedup_r)
{
	if ( cpu_get_pc(machine->activecpu) == 0x06013f7a ) cpu_spinuntil_int(machine->activecpu);
	return stv_workram_h[0x000705d0 / 4];
}

DRIVER_INIT(suikoenb)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6013f7a);

	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x060705d0, 0x060705d3, 0, 0, suikoenb_speedup_r);
	DRIVER_INIT_CALL(stv);
}


DRIVER_INIT(sokyugrt)
{
	DRIVER_INIT_CALL(stv);
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);

}

static READ32_HANDLER( znpwfv_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x6012ec2) cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x0ffc10/4];
}

static void znpwfv_slave_speedup( UINT32 data )
{
	if ( cpu_get_pc(Machine->activecpu) == 0x060175a6 )
		if ( (data & 0x00800000) == 0 )
		{
			if (
			(stv_workram_h[0x0ffc44/4] != 0x260f359c) &&
			(stv_workram_h[0x0ffc48/4] != 0x260f359c) &&
			(stv_workram_h[0x0ffc48/4] != 0x260f3598)
			)
			{
				logerror("cpu1 skip %08x %08x\n",stv_workram_h[0x0ffc44/4],stv_workram_h[0x0ffc48/4]);

				cpu_spinuntil_trigger(Machine->cpu[1], 1000);
			}
		}
}

DRIVER_INIT(znpwfv)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6012ec2);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60175a6);

	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)znpwfv_slave_speedup );
	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, znpwfv_speedup_r );

	DRIVER_INIT_CALL(stv);
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_NSEC(500);
}

static READ32_HANDLER( twcup98_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x605edde) cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x0ffc10/4];
}

static void twcup98_slave_speedup( UINT32 data )
{
	if ( cpu_get_pc(Machine->activecpu) == 0x06062bca )
		if ( (data & 0x00800000) == 0 )
			cpu_spinuntil_trigger(Machine->cpu[1], 1000);
}

DRIVER_INIT(twcup98)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x605edde);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6062bca);

	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)twcup98_slave_speedup );
	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, twcup98_speedup_r );

	DRIVER_INIT_CALL(stv);
	install_standard_protection(machine);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(5);
}

static READ32_HANDLER( smleague_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x6063bf4) cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x0ffc10/4];
}

static void smleague_slave_speedup( UINT32 data )
{
	if ( cpu_get_pc(Machine->activecpu) == 0x06062bca )
		if ( (data & 0x00800000) == 0 )
			cpu_spinuntil_trigger(Machine->cpu[1], 1000);
}

DRIVER_INIT(smleague)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6063bf4);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6062bca);

	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)smleague_slave_speedup );
	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, smleague_speedup_r );

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

static READ32_HANDLER( finlarch_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x6064d60) cpu_spinuntil_int(machine->activecpu);
	return stv_workram_h[0x0ffc10/4];
}


DRIVER_INIT(finlarch)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6064d60);

	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, finlarch_speedup_r );

	DRIVER_INIT_CALL(stv);

}

static READ32_HANDLER( maruchan_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x06012a52) cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x0ffc10/4];
}

static void maruchan_slave_speedup( UINT32 data )
{
	if ( cpu_get_pc(Machine->activecpu) == 0x0601ba46 )
		if ( (data & 0x00800000) == 0 )
			if (
			(stv_workram_h[0x0ffc48/4] != 0x260ef3c8) &&
			(stv_workram_h[0x0ffc48/4] != 0x260ef3c4)
			)
			{
				logerror("cpu1 skip %08x %08x\n",stv_workram_h[0x0ffc44/4],stv_workram_h[0x0ffc48/4]);

				cpu_spinuntil_trigger(Machine->cpu[1], 1000);
			}

}

DRIVER_INIT(maruchan)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x601ba46);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x601ba46);

	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)maruchan_slave_speedup );
	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, maruchan_speedup_r );

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

static READ32_HANDLER( pblbeach_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x0605eb78)
		if (stv_workram_h[0x006c398/4] != 0)
			cpu_spinuntil_int(machine->activecpu);
	return stv_workram_h[0x006c398/4];
}

DRIVER_INIT(pblbeach)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x605eb78);

	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x0606c398, 0x0606c39b, 0, 0, pblbeach_speedup_r );

	DRIVER_INIT_CALL(stv);
}

static READ32_HANDLER( shanhigw_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x06020c5c)
			cpu_spinuntil_int(machine->activecpu);
	return stv_workram_h[0x95cd8/4];
}

DRIVER_INIT(shanhigw)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6020c5c);

	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x06095cd8, 0x06095cdb, 0, 0, shanhigw_speedup_r );

	DRIVER_INIT_CALL(stv);
}

static READ32_HANDLER( elandore_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x0604eac0) cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x0ffc10/4];
}


static void elandore_slave_speedup(UINT32 data)
{
	if (cpu_get_pc(Machine->activecpu) == 0x0605340a)
		if ( (data & 0x00800000) == 0 )
			if ( (stv_workram_h[0x0ffc48/4] != 0x260ee018) )
			{
				logerror("cpu1 skip %08x %08x\n",stv_workram_h[0x0ffc44/4],stv_workram_h[0x0ffc48/4]);

				cpu_spinuntil_trigger(Machine->cpu[1], 1000);
			}
}

DRIVER_INIT(elandore)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x604eac0);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x605340a);

	install_standard_protection(machine);
	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, elandore_speedup_r );
	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf *)elandore_slave_speedup);
	DRIVER_INIT_CALL(stv);
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(0);

}

static READ32_HANDLER( rsgun_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x06034d04) cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x0ffc10/4];
}

static void rsgun_slave_speedup(UINT32 data)
{
	if (cpu_get_pc(Machine->activecpu) == 0x06036152)
		if ( (data & 0x00800000) == 0 )
			if ((stv_workram_h[0x0ffc48/4] != 0x260efc50))
			{
				logerror("cpu1 skip %08x %08x\n",stv_workram_h[0x0ffc44/4],stv_workram_h[0x0ffc48/4]);

				cpu_spinuntil_trigger(Machine->cpu[1], 1000);
			}
}

DRIVER_INIT(rsgun)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6034d04);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6036152);

	install_standard_protection(machine);
	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, rsgun_speedup_r );
	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf *)rsgun_slave_speedup);

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(20);

}

DRIVER_INIT(ffreveng)
{
	install_standard_protection(machine);
	DRIVER_INIT_CALL(stv);
}

DRIVER_INIT(decathlt)
{
	install_decathlt_protection(machine);
	DRIVER_INIT_CALL(stv);
}

static READ32_HANDLER( nameclv3_speedup_r )
{
	if (cpu_get_pc(machine->activecpu)==0x601eb4c) cpu_spinuntil_time(machine->activecpu, ATTOTIME_IN_USEC(30));
	return stv_workram_h[0x0452c0/4];
}

static void nameclv3_slave_speedup( UINT32 data )
{
	if ( cpu_get_pc(Machine->activecpu) == 0x0602B80e )
		if ( (data & 0x00800000) == 0 )
			cpu_spinuntil_trigger(Machine->cpu[1], 1000);
}

DRIVER_INIT(nameclv3)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x601eb4c);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x602b80e);

	memory_install_read32_handler(machine, 0, ADDRESS_SPACE_PROGRAM, 0x60452c0, 0x60452c3, 0, 0, nameclv3_speedup_r );
	cpu_set_info_fct(machine->cpu[1], CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)nameclv3_slave_speedup );
	DRIVER_INIT_CALL(stv);
}
