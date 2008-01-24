/* ST-V Inits and SpeedUp Hacks */
/* stvinit.c */

/*
to be honest i think some of these cause more problems than they're worth ...
*/

#include "driver.h"
#include "machine/eeprom.h"
#include "cpu/sh2/sh2.h"
#include "machine/stvprot.h"

extern UINT32 *stv_workram_h;
extern UINT32 *stv_workram_l;
extern UINT32 *stv_backupram;
extern int stv_enable_slave_sh2;

extern int minit_boost,sinit_boost;
extern attotime minit_boost_timeslice, sinit_boost_timeslice;

DRIVER_INIT ( stv );

/*
IC-13 rom shifter routine,on 2000000-21fffff the game maps the rom bytes on the
ODD (in every sense) bytes.This gets the IC-13 rom status to good and ends a emulation
weird issue once and for all...
We need to remove this and add the whole thing into the ROM loading structure...
*/
static void ic13_shifter(void)
{
	UINT32 *rom = (UINT32 *)memory_region(REGION_USER1);
	UINT32 i;
	UINT32 *tmp = malloc_or_die(0x80000*2);

	for(i=(0);i<(0x100000-1);i+=8)
	{
		//mame_printf_debug("%08x\n",i);
		tmp[((i)/4)+0] = rom[(i/2)/4]; /*0.0 -> 2.1 -> 4.2*/
		tmp[((i)/4)+1] = rom[(i/2)/4]; /*1.0 -> 3.1 -> 5.2*/
	}

	for(i=(0);i<(0x100000-1);i+=8)
	{
		//mame_printf_debug("%08x\n",i);
		tmp[(i/4)+0] = ((tmp[(i/4)+0] & 0xff000000) >> 8) | ((tmp[(i/4)+0] & 0x00ff0000) >> 16);
		tmp[(i/4)+1] = ((tmp[(i/4)+1] & 0x0000ff00) << 8) | ((tmp[(i/4)+1] & 0x000000ff) >> 0);
	}

	for(i=(0);i<(0x100000-1);i+=4)
	{
		//mame_printf_debug("%08x\n",i);
		rom[i/4] = tmp[(i)/4];
	}

	for(i=(0x300000);i<(0x400000-1);i+=8)
	{
		//mame_printf_debug("%08x\n",i);
		tmp[((i-0x300000)/4)+0] = rom[(i/2)/4]; /*0.0 -> 2.1 -> 4.2*/
		tmp[((i-0x300000)/4)+1] = rom[(i/2)/4]; /*1.0 -> 3.1 -> 5.2*/
	}

	for(i=(0);i<(0x100000-1);i+=8)
	{
		//mame_printf_debug("%08x\n",i);
		tmp[(i/4)+0] = ((tmp[(i/4)+0] & 0xff000000) >> 8) | ((tmp[(i/4)+0] & 0x00ff0000) >> 16);
		tmp[(i/4)+1] = ((tmp[(i/4)+1] & 0x0000ff00) << 8) | ((tmp[(i/4)+1] & 0x000000ff) >> 0);
	}

	for(i=(0x100000);i<(0x200000-1);i+=4)
	{
		//mame_printf_debug("%08x\n",i);
		rom[i/4] = tmp[(i-0x100000)/4];
	}
	free(tmp);
}

DRIVER_INIT ( ic13 )
{
	ic13_shifter();
	DRIVER_INIT_CALL(stv);
}
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
		EEPROM_save(file);
	else
	{
		EEPROM_init(&eeprom_interface_93C46);

		if (file) EEPROM_load(file);
		else
		{
			if (stv_default_eeprom)	/* Set the EEPROM to Factory Defaults */
				EEPROM_set_data(stv_default_eeprom,stv_default_eeprom_length);
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
	if (activecpu_get_pc()==0x60154b4) cpu_spinuntil_int(); // bios menus..

	return stv_workram_h[0x0335d0/4];
}

static READ32_HANDLER( stv_speedup2_r )
{
	if (activecpu_get_pc()==0x6013af0) cpu_spinuntil_int(); // for use in japan

	return stv_workram_h[0x0335bc/4];
}

void install_stvbios_speedups(void)
{
/* idle skip bios? .. not 100% sure this is safe .. we'll see */
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60335d0, 0x60335d3, 0, 0, stv_speedup_r );
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60335bc, 0x60335bf, 0, 0, stv_speedup2_r );
}

static READ32_HANDLER( shienryu_slave_speedup_r )
{
 if (activecpu_get_pc()==0x06004410)
  cpu_spinuntil_time(ATTOTIME_IN_USEC(20)); // is this safe... we can't skip till vbl because its not a vbl wait loop

 return stv_workram_h[0x0ae8e4/4];
}


static READ32_HANDLER( shienryu_speedup_r )
{
	if (activecpu_get_pc()==0x060041C8) cpu_spinuntil_int(); // after you enable the sound cpu ...
	return stv_workram_h[0x0ae8e0/4];
}


DRIVER_INIT(shienryu)
{
	stv_default_eeprom = shienryu_default_eeprom;
	stv_default_eeprom_length = sizeof(shienryu_default_eeprom);

	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60ae8e0, 0x60ae8e3, 0, 0, shienryu_speedup_r ); // after you enable sound cpu
	memory_install_read32_handler(1, ADDRESS_SPACE_PROGRAM, 0x60ae8e4, 0x60ae8e7, 0, 0, shienryu_slave_speedup_r ); // after you enable sound cpu

	DRIVER_INIT_CALL(stv);
}

static READ32_HANDLER( prikura_speedup_r )
{
	if (activecpu_get_pc()==0x6018642) cpu_spinuntil_int(); // after you enable the sound cpu ...
	return stv_workram_h[0x0b9228/4];
}

static void prikura_slave_speedup( UINT32 data )
{
	if ( activecpu_get_pc() == 0x06018c70 )
		if ( (data & 0x00800000) == 0 )
			cpunum_spinuntil_trigger(1, 1000);
}

DRIVER_INIT(prikura)
{
/*
 06018640: MOV.B   @R14,R0  // 60b9228
 06018642: TST     R0,R0
 06018644: BF      $06018640

    (loops for 263473 instructions)
*/
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60b9228, 0x60b922b, 0, 0, prikura_speedup_r );
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)prikura_slave_speedup );

	DRIVER_INIT_CALL(stv);

	minit_boost = sinit_boost = 0;
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);

}


static READ32_HANDLER( hanagumi_speedup_r )
{
	if (activecpu_get_pc()==0x06010162) cpu_spinuntil_int(); // title logos

	return stv_workram_h[0x94188/4];
}

static READ32_HANDLER( hanagumi_slave_off )
{
	/* just turn the slave off, i don't think the game needs it */
	cpunum_set_input_line(1, INPUT_LINE_HALT, ASSERT_LINE);

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
   	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x6094188, 0x609418b, 0, 0, hanagumi_speedup_r );
   	memory_install_read32_handler(1, ADDRESS_SPACE_PROGRAM, 0x6015438, 0x601543b, 0, 0, hanagumi_slave_off );

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
	if (activecpu_get_pc()==0x6021CF2) cpu_spinuntil_time(ATTOTIME_IN_USEC(400)); // spinuntilint breaks controls again .. urgh


	return stv_workram_h[0x0ffc10/4];
}

static void puyosun_slave_speedup( UINT32 data )
{
	if ( activecpu_get_pc() == 0x6023700 )
		if ( (data & 0x00800000) == 0 )
			cpunum_spinuntil_trigger(1, 1000);
}

DRIVER_INIT(puyosun)
{
   	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, puyosun_speedup_r ); // idle loop of main cpu
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)puyosun_slave_speedup );

	DRIVER_INIT_CALL(ic13);

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
	if (activecpu_get_pc()==0x060461A2) cpu_spinuntil_time(ATTOTIME_IN_USEC(20)); // spinuntilint breaks controls again .. urgh

	return stv_workram_h[0x0ffc10/4];
}

DRIVER_INIT(mausuke)
{
   	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, mausuke_speedup_r ); // idle loop of main cpu

	DRIVER_INIT_CALL(ic13);

	minit_boost = sinit_boost = 0;
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

static READ32_HANDLER( cottonbm_speedup_r )
{
	if (activecpu_get_pc()==0x06030EE4) cpu_spinuntil_time(ATTOTIME_IN_USEC(20)); // spinuntilint breaks lots of things

	return stv_workram_h[0x0ffc10/4];
}

static void cottonbm_slave_speedup( UINT32 data )
{
	if (activecpu_get_pc() == 0x6032b54)
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

				cpunum_spinuntil_trigger(1, 1000);
			}
		}
}


DRIVER_INIT(cottonbm)
{
   	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, cottonbm_speedup_r ); // idle loop of main cpu
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)cottonbm_slave_speedup );

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(10);
}

static READ32_HANDLER( cotton2_speedup_r )
{
	if (activecpu_get_pc()==0x06031c7c) cpu_spinuntil_time(ATTOTIME_IN_USEC(20)); // spinuntilint breaks lots of things

	return stv_workram_h[0x0ffc10/4];
}

static void cotton2_slave_speedup( UINT32 data )
{
	if (activecpu_get_pc() == 0x60338ec)
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

				cpunum_spinuntil_trigger(1, 1000);
			}
		}
}

DRIVER_INIT(cotton2)
{
   	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, cotton2_speedup_r ); // idle loop of main cpu
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)cotton2_slave_speedup );

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

static int dnmtdeka_pending_commands;

static READ32_HANDLER( dnmtdeka_speedup_r )
{
	if (activecpu_get_pc()==0x6027c92) cpu_spinuntil_int();//cpu_spinuntil_time(ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x0985a0/4];
}

static WRITE32_HANDLER(dnmtdeka_cmd_write)
{
	COMBINE_DATA(&stv_workram_h[0x0e0ad4/4 + offset]);
	if ( (cpu_getactivecpu() == 0) && (activecpu_get_pc() == 0x00000d06) )
		return;

	if ( data != 0 ) dnmtdeka_pending_commands++;
	//logerror( "CMD: Written by cpu=%d, at = %08X, offset = %08X, data = %08X, commands = %d\n", cpu_getactivecpu(), activecpu_get_pc(), offset, data, dnmtdeka_pending_commands );
	cpu_trigger(Machine, 1000);
}

static READ32_HANDLER(dnmtdeka_cmd_read)
{
	//logerror( "CMD: Read by cpu=%d, at = %08X, offset = %08X, data = %08X, commands = %d\n", cpu_getactivecpu(), activecpu_get_pc(), offset, stv_workram_h[0xe0bd0/4 + offset], dnmtdeka_pending_commands );
	if ( activecpu_get_pc() == 0x060051f4 )
	{
		if ( stv_workram_h[0x0e0ad4/4 + offset] == 0 )
		{
			if ( dnmtdeka_pending_commands == 0 )
				cpu_spinuntil_trigger(1000);
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
	dnmtdeka_pending_commands = 0;

	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60985a0, 0x60985a3, 0, 0, dnmtdeka_speedup_r ); // idle loop of main cpu
	memory_install_write32_handler(0, ADDRESS_SPACE_PROGRAM, 0x060e0ad4, 0x060e0bc3, 0, 0, dnmtdeka_cmd_write );
	memory_install_read32_handler(1, ADDRESS_SPACE_PROGRAM, 0x060e0ad4, 0x060e0bc3, 0, 0, dnmtdeka_cmd_read );

	DRIVER_INIT_CALL(ic13);
}

static int diehard_pending_commands;

static READ32_HANDLER(diehard_speedup_r)
{
	if ( activecpu_get_pc() == 0x06027c9a ) cpu_spinuntil_int();
	return stv_workram_h[0x000986ac/4];
}

static WRITE32_HANDLER(diehard_cmd_write)
{
	COMBINE_DATA(&stv_workram_h[0xe0bd0/4 + offset]);
	if ( (cpu_getactivecpu() == 0) && (activecpu_get_pc() == 0x00000d06) )
		return;

	if ( data != 0 ) diehard_pending_commands++;
	//logerror( "CMD: Written by cpu=%d, at = %08X, offset = %08X, data = %08X, commands = %d\n", cpu_getactivecpu(), activecpu_get_pc(), offset, data, diehard_pending_commands );
	cpu_trigger(Machine, 1000);
}

static READ32_HANDLER(diehard_cmd_read)
{
	//logerror( "CMD: Read by cpu=%d, at = %08X, offset = %08X, data = %08X, commands = %d\n", cpu_getactivecpu(), activecpu_get_pc(), offset, stv_workram_h[0xe0bd0/4 + offset], diehard_pending_commands );
	if ( activecpu_get_pc() == 0x060051f4 )
	{
		if ( stv_workram_h[0xe0bd0/4 + offset] == 0 )
		{
			if ( diehard_pending_commands == 0 )
				cpu_spinuntil_trigger(1000);
		}
	}

	return stv_workram_h[0xe0bd0/4 + offset];
}

static READ32_HANDLER(diehard_cmd_ack_read)
{
	//logerror( "CMDACK: Read by cpu=%d, at = %08X, offset = %08X, data = %08X, commands = %d\n", cpu_getactivecpu(), activecpu_get_pc(), offset, stv_workram_h[0x000e0dd8/4], diehard_pending_commands );
	if ( (stv_workram_h[0x000e0dd8/4] & 0xff000000) == 0 &&
		 diehard_pending_commands == 0 )
	{
		cpu_trigger(Machine, 1000);
	}
	return stv_workram_h[0x000e0dd8/4];
}

static WRITE32_HANDLER(diehard_cmd_ack_write)
{
	//logerror( "CMDACK: Write by cpu=%d, at = %08X, offset = %08X, data = %08X, commands = %d\n", cpu_getactivecpu(), activecpu_get_pc(), offset, data, diehard_pending_commands );
	if ( diehard_pending_commands > 0 )
	{
		diehard_pending_commands--;
	}
	COMBINE_DATA(&stv_workram_h[0x000e0dd8/4]);
}

static WRITE32_HANDLER(diehard_cmd_ack_write_cpu0)
{
	//logerror( "CMDACK: Write by cpu=%d, at = %08X, offset = %08X, data = %08X, commands = %d\n", cpu_getactivecpu(), activecpu_get_pc(), offset, data, diehard_pending_commands );
	COMBINE_DATA(&stv_workram_h[0x000e0dd8/4]);
	cpu_trigger(Machine, 1000);
}

DRIVER_INIT(diehard)
{
	diehard_pending_commands = 0;

	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x060986ac, 0x060986af, 0, 0, diehard_speedup_r );
	memory_install_write32_handler(0, ADDRESS_SPACE_PROGRAM, 0x060e0bd0, 0x060e0dcf, 0, 0, diehard_cmd_write );
	memory_install_read32_handler(1, ADDRESS_SPACE_PROGRAM, 0x060e0bd0, 0x060e0dcf, 0, 0, diehard_cmd_read );
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x060e0dd8, 0x060e0dd8, 0, 0, diehard_cmd_ack_read );
	memory_install_write32_handler(0, ADDRESS_SPACE_PROGRAM, 0x060e0dd8, 0x060e0dd8, 0, 0, diehard_cmd_ack_write_cpu0 );
	memory_install_write32_handler(1, ADDRESS_SPACE_PROGRAM, 0x060e0dd8, 0x060e0dd8, 0, 0, diehard_cmd_ack_write );


	DRIVER_INIT_CALL(ic13);

}

static READ32_HANDLER( fhboxers_speedup_r )
{
	if (activecpu_get_pc()==0x060041c4) cpu_spinuntil_time(ATTOTIME_IN_USEC(20));

	return stv_workram_h[0x00420c/4];
}

static READ32_HANDLER( fhboxers_speedup2_r )
{
	if (activecpu_get_pc()==0x0600bb0c) cpu_spinuntil_time(ATTOTIME_IN_USEC(20));


	return stv_workram_h[0x090740/4];
}

static READ32_HANDLER( fhboxers_speedup3_r )
{
	if (activecpu_get_pc()==0x0600b320 )
		cpu_spinuntil_int();

	return stv_workram_h[0x90bb4/4];
}

DRIVER_INIT(fhboxers)
{
   	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x600420c, 0x600420f, 0, 0, fhboxers_speedup_r ); // idle loop of main cpu
   	memory_install_read32_handler(1, ADDRESS_SPACE_PROGRAM, 0x6090740, 0x6090743, 0, 0, fhboxers_speedup2_r ); // idle loop of second cpu
  	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x06090bb4, 0x06090bb7, 0, 0, fhboxers_speedup3_r ); // idle loop of main cpu

	DRIVER_INIT_CALL(ic13);
}


static READ32_HANDLER( bakubaku_speedup_r )
{
	if (activecpu_get_pc()==0x06036dc8) cpu_spinuntil_int(); // title logos

	return stv_workram_h[0x0833f0/4];
}

static READ32_HANDLER( bakubaku_speedup2_r )
{
	if (activecpu_get_pc()==0x06033762) 	cpunum_set_input_line(1, INPUT_LINE_HALT, ASSERT_LINE);

	return stv_workram_h[0x0033762/4];
}

#ifdef UNUSED_FUNCTION
static READ32_HANDLER( bakubaku_hangskip_r )
{
	if (activecpu_get_pc()==0x060335e4) stv_workram_h[0x0335e6/4] = 0x32300909;

	return stv_workram_h[0x033660/4];
}
#endif

DRIVER_INIT(bakubaku)
{
   	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60833f0, 0x60833f3, 0, 0, bakubaku_speedup_r ); // idle loop of main cpu
   	memory_install_read32_handler(1, ADDRESS_SPACE_PROGRAM, 0x60fdfe8, 0x60fdfeb, 0, 0, bakubaku_speedup2_r ); // turn off slave sh2, is it needed after boot ??
   	//memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x6033660, 0x6033663, 0, 0, bakubaku_hangskip_r ); // it waits for a ram address to change what should change it?

	DRIVER_INIT_CALL(ic13);
}

static READ32_HANDLER( groovef_hack1_r )
{
	if(activecpu_get_pc() == 0x6005e7e) stv_workram_h[0x0fffcc/4] = 0x00000000;
//  popmessage("1 %08x",activecpu_get_pc());
	return stv_workram_h[0x0fffcc/4];
}

static READ32_HANDLER( groovef_hack2_r )
{
	if(activecpu_get_pc() == 0x6005e88) stv_workram_h[0x0ca6cc/4] = 0x00000000;
//  popmessage("2 %08x",activecpu_get_pc());
	return stv_workram_h[0x0ca6cc/4];
}

static READ32_HANDLER( groovef_speedup_r )
{
//  logerror ("groove speedup \n");
	if (activecpu_get_pc()==0x060a4972)
	{
		cpu_spinuntil_int(); // title logos
//      logerror ("groove speedup skipping\n");

	}

	return stv_workram_h[0x0c64ec/4];
}
/*
static READ32_HANDLER( groovef_second_cpu_off_r )
{
    if (activecpu_get_pc()==0x060060c2)     cpunum_set_input_line(1, INPUT_LINE_HALT, ASSERT_LINE);
    return 0;
}
*/

static void groovef_slave_speedup( UINT32 data )
{
	if ( activecpu_get_pc() == 0x060060c4 )
		if ( (data & 0x00800000) == 0 )
			cpunum_spinuntil_trigger(1, 1000);
}

DRIVER_INIT( groovef )
{
	/* prevent game from hanging on startup -- todo: remove these hacks */
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60ca6cc, 0x60ca6cf, 0, 0, groovef_hack2_r );
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60fffcc, 0x60fffcf, 0, 0, groovef_hack1_r );

	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60c64ec, 0x60c64ef, 0, 0, groovef_speedup_r );
//  memory_install_read32_handler(1, ADDRESS_SPACE_PROGRAM, 0x60060dc, 0x60060df, 0, 0, groovef_second_cpu_off_r ); // not a good idea, needs it for ai.
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)groovef_slave_speedup );

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
	logerror( "DMASt_SCU1: Read at PC=%08x, value = %08x\n", activecpu_get_pc(), stv_workram_h[0x0ffcbc/4] );
	if (activecpu_get_pc()==0x06028b2a) return 0x0e0c0000;

	return stv_workram_h[0x0ffcbc/4];
}

static READ32_HANDLER( danchih_speedup_r )
{
	if (activecpu_get_pc()==0x06028c90) cpu_spinuntil_time(ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x0ffc10/4];
}

static void danchih_slave_speedup( UINT32 data )
{
	if ( activecpu_get_pc() == 0x0602ae28 )
		if ( (data & 0x00800000) == 0 )
			cpunum_spinuntil_trigger(1, 1000);
}

DRIVER_INIT( danchih )
{
	/* prevent game from hanging on title screen -- todo: remove these hacks */
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60ffcbc, 0x60ffcbf, 0, 0, danchih_hack_r );

	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, danchih_speedup_r );
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)danchih_slave_speedup );

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
	if(activecpu_get_pc()==0x60011bc) return 0x00000000;

	return stv_workram_h[0x000770/4];
}

static READ32_HANDLER( astrass_speedup_r )
{
	if(activecpu_get_pc() == 0x0605b9dc )
		cpu_spinuntil_time(ATTOTIME_IN_USEC(20));

	return stv_workram_h[0x8e4d8/4];
}

DRIVER_INIT( astrass )
{
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x6000770, 0x6000773, 0, 0, astrass_hack_r );

	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x0608e4d8, 0x0608e4db, 0, 0, astrass_speedup_r );

	install_astrass_protection();

	DRIVER_INIT_CALL(ic13);
}

/* Treasure Hunt idle loop skipping */

static READ32_HANDLER(thunt_speedup_r)
{
	if (activecpu_get_pc() == 0x0602A026) cpu_spinuntil_int();
	return stv_workram_h[0x00031424/4];
}

static READ32_HANDLER(thunt_speedup2_r)
{
	if (activecpu_get_pc() == 0x06013EEC) cpu_spinuntil_int();
	return stv_workram_h[0x00075958/4];
}

static void thunt_slave_speedup(UINT32 data)
{
	if (activecpu_get_pc() == 0x0602AAFA)
		if ( (data & 0x00800000) == 0 )
			cpunum_spinuntil_trigger(1, 1000);
}

DRIVER_INIT(thunt)
{
/*
0602A024: MOV.L   @R6,R0    // 06031424
0602A026: TST     R0,R0
0602A028: BF      $0602A024
*/
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x06031424, 0x06031427, 0, 0, thunt_speedup_r );

/*
06013EE8: MOV.L   @($10,PC),R0
06013EEA: MOV.B   @R0,R0
06013EEC: EXTU.B  R0,R0
06013EEE: TST     R0,R0
06013EF0: BT      $06013EF6
06013EF2: RTS
06013EF4: MOV     #$01,R0
*/
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x06075958, 0x0607595b, 0, 0, thunt_speedup2_r );

/*
0602AAF8: MOV.B   @R11,R2
0602AAFA: EXTU.B  R2,R2
0602AAFC: AND     R13,R2
0602AAFE: CMP/EQ  R13,R2
0602AB00: BF      $0602AB28
0602AB28: BRA     $0602AAF8
*/

	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf *)thunt_slave_speedup);

	DRIVER_INIT_CALL(ic13);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(1);
}

static READ32_HANDLER(sandor_speedup_r)
{
	if (activecpu_get_pc() == 0x0602a0fa) cpu_spinuntil_int();
	return stv_workram_h[0x000314f8/4];
}

static READ32_HANDLER(sandor_speedup2_r)
{
	if (activecpu_get_pc() == 0x06013fc0) cpu_spinuntil_int();
	return stv_workram_h[0x00075a2c/4];
}


static void sandor_slave_speedup(UINT32 data)
{
	if (activecpu_get_pc() == 0x0602abce)
		if ( (data & 0x00800000) == 0 )
			cpunum_spinuntil_trigger(1, 1000);
}

DRIVER_INIT(sandor)
{
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x060314f8, 0x060314fb, 0, 0, sandor_speedup_r );
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x06075a2c, 0x06075a2f, 0, 0, sandor_speedup2_r );
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf *)sandor_slave_speedup);
	DRIVER_INIT_CALL(ic13);
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(1);

}

static READ32_HANDLER(grdforce_speedup_r)
{
	if ( activecpu_get_pc() == 0x06041E34 ) cpu_spinuntil_time(ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x000ffc10/4];
}

static void grdforce_slave_speedup( UINT32 data )
{
	if (activecpu_get_pc() == 0x06043aa4)
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

				//cpu_spinuntil_time(ATTOTIME_IN_USEC(200));
				cpunum_spinuntil_trigger(1, 1000);
			}
		}
}

DRIVER_INIT(grdforce)
{
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
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x060ffc10, 0x060ffc13, 0, 0, grdforce_speedup_r );

	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf *)grdforce_slave_speedup);

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

static READ32_HANDLER( batmanfr_speedup_r )
{
	//logerror( "batmanfr speedup: pc = %08x, mem = %08x\n", activecpu_get_pc(), stv_workram_h[0x0002acf0/4] );
	if ( activecpu_get_pc() != 0x060121c2 )
		cpu_spinuntil_int();

	return stv_workram_h[0x0002acf0/4];
}

static void batmanfr_slave_speedup( UINT32 data )
{
	if (activecpu_get_pc() == 0x060125be )
		if ( (data & 0x00800000) == 0 )
			cpunum_spinuntil_trigger(1, 1000);
}


DRIVER_INIT(batmanfr)
{
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x0602acf0, 0x0602acf3, 0, 0, batmanfr_speedup_r );
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)batmanfr_slave_speedup );

	DRIVER_INIT_CALL(stv);

	minit_boost = sinit_boost = 0;
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

static void colmns97_slave_speedup( UINT32 data )
{
	if (activecpu_get_pc() == 0x060298a4 )
		if ( (data & 0x00800000) == 0 )
			if ( (stv_workram_h[0x0ffc48/4] != 0x260ef3fc) )
			{
				logerror("cpu1 skip %08x %08x\n",stv_workram_h[0x0ffc44/4],stv_workram_h[0x0ffc48/4]);
				cpunum_spinuntil_trigger(1, 1000);
			}

}

DRIVER_INIT(colmns97)
{
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)colmns97_slave_speedup );

	DRIVER_INIT_CALL(ic13);

	minit_boost = sinit_boost = 0;

}

static READ32_HANDLER(winterht_speedup_r)
{
	if ( activecpu_get_pc() == 0x06098aec ) cpu_spinuntil_time(ATTOTIME_IN_USEC(20));//cpu_spinuntil_int();
	return stv_workram_h[0x000ffc10/4];
}

static void winterht_slave_speedup( UINT32 data )
{
	if (activecpu_get_pc() == 0x0609ae50 )
		if ( (data & 0x00800000) == 0 )
			cpunum_spinuntil_trigger(1, 1000);
}

DRIVER_INIT(winterht)
{
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x060ffc10, 0x060ffc13, 0, 0, winterht_speedup_r );

	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)winterht_slave_speedup );

	DRIVER_INIT_CALL(ic13);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(2);
}

static READ32_HANDLER(seabass_speedup_r)
{
	if ( activecpu_get_pc() == 0x0602cbfc ) cpu_spinuntil_time(ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x000ffc10/4];
}

static void seabass_slave_speedup( UINT32 data )
{
	if (activecpu_get_pc() == 0x060321f0 )
		if ( (data & 0x00800000) == 0 )
			cpunum_spinuntil_trigger(1, 1000);
}


DRIVER_INIT(seabass)
{
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x060ffc10, 0x060ffc13, 0, 0, seabass_speedup_r );

	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)seabass_slave_speedup );

	DRIVER_INIT_CALL(ic13);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(5);
}

static void vfremix_slave_speedup( UINT32 data )
{
	if (activecpu_get_pc() == 0x0604C334 )
		if ( (data & 0x00800000) == 0 )
			cpunum_spinuntil_trigger(1, 1000);
}

static READ32_HANDLER(vfremix_speedup_r)
{
	if ( activecpu_get_pc() == 0x0602c30e ) cpu_spinuntil_int();
	return stv_workram_h[0x00074f98/4];
}

DRIVER_INIT(vfremix)
{
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x06074f98, 0x06074f9b, 0, 0, vfremix_speedup_r );
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)vfremix_slave_speedup );

	DRIVER_INIT_CALL(ic13);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(20);
}


static READ32_HANDLER(sss_speedup_r)
{
	if ( activecpu_get_pc() == 0x0602639a ) cpu_spinuntil_int();
	return stv_workram_h[0x000ffc10/4];
}

static void sss_slave_speedup( UINT32 data )
{
	if (activecpu_get_pc() == 0x06028cd8 )
		if ( (data & 0x00800000) == 0 )
			cpunum_spinuntil_trigger(1, 1000);
}

DRIVER_INIT(sss)
{
	install_standard_protection();
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x060ffc10, 0x060ffc13, 0, 0, sss_speedup_r );
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)sss_slave_speedup );

	DRIVER_INIT_CALL(ic13);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

static READ32_HANDLER(othellos_speedup_r)
{
	if ( activecpu_get_pc() == 0x0602bcc0 ) cpu_spinuntil_time(ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x000ffc10/4];
}

static void othellos_slave_speedup( UINT32 data )
{
	if (activecpu_get_pc() == 0x0602d930 )
		if ( (data & 0x00800000) == 0 )
			if ( (stv_workram_h[0x0ffc48/4] != 0x260fd25c ) )
			{
				logerror("cpu1 skip %08x %08x\n",stv_workram_h[0x0ffc44/4],stv_workram_h[0x0ffc48/4]);
				cpunum_spinuntil_trigger(1, 1000);
			}
}

DRIVER_INIT(othellos)
{
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x060ffc10, 0x060ffc13, 0, 0, othellos_speedup_r );
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)othellos_slave_speedup );

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);

}

static void sasissu_slave_speedup( UINT32 data )
{
	if ( activecpu_get_pc() == 0x060710C0 )
		if ( (data & 0x00800000) == 0 )
			cpunum_spinuntil_trigger(1, 1000);
}

DRIVER_INIT(sasissu)
{
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)sasissu_slave_speedup );

	DRIVER_INIT_CALL(ic13);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(2);
}

static READ32_HANDLER(gaxeduel_speedup_r)
{
	if ( activecpu_get_pc() == 0x06012ee6 ) cpu_spinuntil_int();
	return stv_workram_l[0x000f4068 / 4];
}

DRIVER_INIT(gaxeduel)
{
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x002f4068, 0x002f406b, 0, 0, gaxeduel_speedup_r);
	DRIVER_INIT_CALL(ic13);
}

static READ32_HANDLER(suikoenb_speedup_r)
{
	if ( activecpu_get_pc() == 0x06013f7c ) cpu_spinuntil_int();
	return stv_workram_h[0x000705d0 / 4];
}

DRIVER_INIT(suikoenb)
{
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x060705d0, 0x060705d3, 0, 0, suikoenb_speedup_r);
	DRIVER_INIT_CALL(ic13);
}

static void sokyugrt_slave_speedup( UINT32 data )
{
	logerror( "SlaveSH2: Idle loop skip, data = %08X\n", data );
	if (stv_enable_slave_sh2)
		if ( activecpu_get_pc() == 0x0605eec2 )
			if ( (data & 0x00800000) == 0 )
				cpunum_spinuntil_trigger(1, 1000);
}

static READ32_HANDLER(sokyugrt_speedup_r)
{
	if ( activecpu_get_pc() == 0x0605d9dc ) cpu_spinuntil_int();
	return stv_workram_h[0x000788cc / 4];
}

DRIVER_INIT(sokyugrt)
{
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)sokyugrt_slave_speedup );
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x060788cc, 0x060788cf, 0, 0, sokyugrt_speedup_r);
	DRIVER_INIT_CALL(ic13);
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);

}

static READ32_HANDLER( znpwfv_speedup_r )
{
	if (activecpu_get_pc()==0x6012ec4) cpu_spinuntil_time(ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x0ffc10/4];
}

static void znpwfv_slave_speedup( UINT32 data )
{
	if ( activecpu_get_pc() == 0x060175a8 )
		if ( (data & 0x00800000) == 0 )
		{
			if (
			(stv_workram_h[0x0ffc44/4] != 0x260f359c) &&
			(stv_workram_h[0x0ffc48/4] != 0x260f359c) &&
			(stv_workram_h[0x0ffc48/4] != 0x260f3598)
			)
			{
				logerror("cpu1 skip %08x %08x\n",stv_workram_h[0x0ffc44/4],stv_workram_h[0x0ffc48/4]);

				cpunum_spinuntil_trigger(1, 1000);
			}
		}
}

DRIVER_INIT(znpwfv)
{
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)znpwfv_slave_speedup );
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, znpwfv_speedup_r );

	DRIVER_INIT_CALL(ic13);
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_NSEC(500);
}

static READ32_HANDLER( twcup98_speedup_r )
{
	if (activecpu_get_pc()==0x605ede0) cpu_spinuntil_time(ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x0ffc10/4];
}

static void twcup98_slave_speedup( UINT32 data )
{
	if ( activecpu_get_pc() == 0x06062bcc )
		if ( (data & 0x00800000) == 0 )
			cpunum_spinuntil_trigger(1, 1000);
}

DRIVER_INIT(twcup98)
{
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)twcup98_slave_speedup );
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, twcup98_speedup_r );

	DRIVER_INIT_CALL(ic13);
	install_standard_protection();

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(5);
}

static READ32_HANDLER( smleague_speedup_r )
{
	if (activecpu_get_pc()==0x6063bf6) cpu_spinuntil_time(ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x0ffc10/4];
}

static void smleague_slave_speedup( UINT32 data )
{
	if ( activecpu_get_pc() == 0x06062bcc )
		if ( (data & 0x00800000) == 0 )
			cpunum_spinuntil_trigger(1, 1000);
}

DRIVER_INIT(smleague)
{
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)smleague_slave_speedup );
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, smleague_speedup_r );

	DRIVER_INIT_CALL(ic13);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

static READ32_HANDLER( finlarch_speedup_r )
{
	if (activecpu_get_pc()==0x6064d62) cpu_spinuntil_int();
	return stv_workram_h[0x0ffc10/4];
}


DRIVER_INIT(finlarch)
{
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, finlarch_speedup_r );

	DRIVER_INIT_CALL(ic13);

}

static READ32_HANDLER( maruchan_speedup_r )
{
	if (activecpu_get_pc()==0x06012a54) cpu_spinuntil_time(ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x0ffc10/4];
}

static void maruchan_slave_speedup( UINT32 data )
{
	if ( activecpu_get_pc() == 0x0601ba48 )
		if ( (data & 0x00800000) == 0 )
			if (
			(stv_workram_h[0x0ffc48/4] != 0x260ef3c8) &&
			(stv_workram_h[0x0ffc48/4] != 0x260ef3c4)
			)
			{
				logerror("cpu1 skip %08x %08x\n",stv_workram_h[0x0ffc44/4],stv_workram_h[0x0ffc48/4]);

				cpunum_spinuntil_trigger(1, 1000);
			}

}

DRIVER_INIT(maruchan)
{
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)maruchan_slave_speedup );
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, maruchan_speedup_r );

	DRIVER_INIT_CALL(ic13);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

static READ32_HANDLER( pblbeach_speedup_r )
{
	if (activecpu_get_pc()==0x0605eb7a)
		if (stv_workram_h[0x006c398/4] != 0)
			cpu_spinuntil_int();
	return stv_workram_h[0x006c398/4];
}

DRIVER_INIT(pblbeach)
{
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x0606c398, 0x0606c39b, 0, 0, pblbeach_speedup_r );

	DRIVER_INIT_CALL(ic13);
}

static READ32_HANDLER( shanhigw_speedup_r )
{
	if (activecpu_get_pc()==0x06020c5e)
			cpu_spinuntil_int();
	return stv_workram_h[0x95cd8/4];
}

DRIVER_INIT(shanhigw)
{
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x06095cd8, 0x06095cdb, 0, 0, shanhigw_speedup_r );

	DRIVER_INIT_CALL(stv);
}

static READ32_HANDLER( elandore_speedup_r )
{
	if (activecpu_get_pc()==0x0604eac2) cpu_spinuntil_time(ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x0ffc10/4];
}


static void elandore_slave_speedup(UINT32 data)
{
	if (activecpu_get_pc() == 0x0605340c)
		if ( (data & 0x00800000) == 0 )
			if ( (stv_workram_h[0x0ffc48/4] != 0x260ee018) )
			{
				logerror("cpu1 skip %08x %08x\n",stv_workram_h[0x0ffc44/4],stv_workram_h[0x0ffc48/4]);

				cpunum_spinuntil_trigger(1, 1000);
			}
}

DRIVER_INIT(elandore)
{
	install_standard_protection();
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, elandore_speedup_r );
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf *)elandore_slave_speedup);
	DRIVER_INIT_CALL(stv);
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(0);

}

static READ32_HANDLER( rsgun_speedup_r )
{
	if (activecpu_get_pc()==0x06034d06) cpu_spinuntil_time(ATTOTIME_IN_USEC(20));
	return stv_workram_h[0x0ffc10/4];
}

static void rsgun_slave_speedup(UINT32 data)
{
	if (activecpu_get_pc() == 0x06036154)
		if ( (data & 0x00800000) == 0 )
			if ((stv_workram_h[0x0ffc48/4] != 0x260efc50))
			{
				logerror("cpu1 skip %08x %08x\n",stv_workram_h[0x0ffc44/4],stv_workram_h[0x0ffc48/4]);

				cpunum_spinuntil_trigger(1, 1000);
			}
}

DRIVER_INIT(rsgun)
{
	install_standard_protection();
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60ffc10, 0x60ffc13, 0, 0, rsgun_speedup_r );
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf *)rsgun_slave_speedup);

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(20);

}

DRIVER_INIT(ffreveng)
{
	install_standard_protection();
	DRIVER_INIT_CALL(stv);
}

DRIVER_INIT(decathlt)
{
	install_decathlt_protection();
	DRIVER_INIT_CALL(ic13);
}

static READ32_HANDLER( nameclv3_speedup_r )
{
	if (activecpu_get_pc()==0x601eb4e) cpu_spinuntil_time(ATTOTIME_IN_USEC(30));
	return stv_workram_h[0x0452c0/4];
}

static void nameclv3_slave_speedup( UINT32 data )
{
	if ( activecpu_get_pc() == 0x0602B810 )
		if ( (data & 0x00800000) == 0 )
			cpunum_spinuntil_trigger(1, 1000);
}

DRIVER_INIT(nameclv3)
{
	memory_install_read32_handler(0, ADDRESS_SPACE_PROGRAM, 0x60452c0, 0x60452c3, 0, 0, nameclv3_speedup_r );
	cpunum_set_info_fct(1, CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK, (genf*)nameclv3_slave_speedup );
	DRIVER_INIT_CALL(stv);
}
