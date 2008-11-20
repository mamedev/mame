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

	DRIVER_INIT_CALL(stv);
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

	DRIVER_INIT_CALL(stv);

	minit_boost = sinit_boost = 0;
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);

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

DRIVER_INIT(puyosun)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6021cf0);

	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60236fe);

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

DRIVER_INIT(mausuke)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60461A0);

	DRIVER_INIT_CALL(stv);

	minit_boost = sinit_boost = 0;
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

DRIVER_INIT(cottonbm)
{
//  cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
//  cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6030ee2);
//  cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+1);
//  cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6032b52);

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(10);
}

DRIVER_INIT(cotton2)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6031c7a);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60338ea);

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
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

	DRIVER_INIT_CALL(stv);
}

DRIVER_INIT(diehard)
{
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

	DRIVER_INIT_CALL(stv);
}

DRIVER_INIT(fhboxers)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60041c2);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+1);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x600bb0a);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+2);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x600b31e);

	DRIVER_INIT_CALL(stv);
}




static READ32_HANDLER( groovef_hack1_r )
{
	if(cpu_get_pc(space->cpu) == 0x6005e7c) stv_workram_h[0x0fffcc/4] = 0x00000000;
//  popmessage("1 %08x",cpu_get_pc(space->cpu));
	return stv_workram_h[0x0fffcc/4];
}

static READ32_HANDLER( groovef_hack2_r )
{
	if(cpu_get_pc(space->cpu) == 0x6005e86) stv_workram_h[0x0ca6cc/4] = 0x00000000;
//  popmessage("2 %08x",cpu_get_pc(space->cpu));
	return stv_workram_h[0x0ca6cc/4];
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
	memory_install_read32_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0x60ca6cc, 0x60ca6cf, 0, 0, groovef_hack2_r );
	memory_install_read32_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0x60fffcc, 0x60fffcf, 0, 0, groovef_hack1_r );

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
	logerror( "DMASt_SCU1: Read at PC=%08x, value = %08x\n", cpu_get_pc(space->cpu), stv_workram_h[0x0ffcbc/4] );
	if (cpu_get_pc(space->cpu)==0x06028b28) return 0x0e0c0000;

	return stv_workram_h[0x0ffcbc/4];
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
	memory_install_read32_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0x60ffcbc, 0x60ffcbf, 0, 0, danchih_hack_r );

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
	if(cpu_get_pc(space->cpu)==0x60011b8 || cpu_get_pc(space->cpu) == 0x60011ba) return 0x00000000;

	return stv_workram_h[0x000770/4];
}

DRIVER_INIT( astrass )
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60011b8);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+1);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x605b9da);

	memory_install_read32_handler(cpu_get_address_space(machine->cpu[0], ADDRESS_SPACE_PROGRAM), 0x06000770, 0x06000773, 0, 0, astrass_hack_r );

	install_astrass_protection(machine);

	DRIVER_INIT_CALL(stv);
}

DRIVER_INIT(thunt)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x602A024);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+1);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6013EEA);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x602AAF8);

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(1);
}

DRIVER_INIT(sandor)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x602a0f8);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT+1);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6013fbe);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x602abcc);

	DRIVER_INIT_CALL(stv);
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(1);

}

DRIVER_INIT(grdforce)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6041e32);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6043aa2);

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

DRIVER_INIT(batmanfr)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60121c0);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60125bc);

	DRIVER_INIT_CALL(stv);

	minit_boost = sinit_boost = 0;
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

DRIVER_INIT(colmns97)
{
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60298a2);

	DRIVER_INIT_CALL(stv);

	minit_boost = sinit_boost = 0;

}

DRIVER_INIT(winterht)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6098aea);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x609ae4e);

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(2);
}

DRIVER_INIT(seabass)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x602cbfa);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60321ee);

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(5);
}

DRIVER_INIT(vfremix)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x602c30c);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x604c332);

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(20);
}

DRIVER_INIT(sss)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6026398);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6028cd6);

	install_standard_protection(machine);

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

DRIVER_INIT(othellos)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x602bcbe);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x602d92e);

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);

}

DRIVER_INIT(sasissu)
{
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60710be);

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(2);
}

DRIVER_INIT(gaxeduel)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6012ee4);

	DRIVER_INIT_CALL(stv);
}

DRIVER_INIT(suikoenb)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6013f7a);

	DRIVER_INIT_CALL(stv);
}


DRIVER_INIT(sokyugrt)
{
	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

DRIVER_INIT(znpwfv)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6012ec2);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x60175a6);

	DRIVER_INIT_CALL(stv);
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_NSEC(500);
}

DRIVER_INIT(twcup98)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x605edde);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6062bca);

	DRIVER_INIT_CALL(stv);
	install_standard_protection(machine);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(5);
}

DRIVER_INIT(smleague)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6063bf4);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6062bca);

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

DRIVER_INIT(finlarch)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6064d60);

	DRIVER_INIT_CALL(stv);

}

DRIVER_INIT(maruchan)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x601ba46);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x601ba46);

	DRIVER_INIT_CALL(stv);

	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(50);
}

DRIVER_INIT(pblbeach)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x605eb78);

	DRIVER_INIT_CALL(stv);
}

DRIVER_INIT(shanhigw)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6020c5c);

	DRIVER_INIT_CALL(stv);
}

DRIVER_INIT(elandore)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x604eac0);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x605340a);

	install_standard_protection(machine);

	DRIVER_INIT_CALL(stv);
	minit_boost_timeslice = sinit_boost_timeslice = ATTOTIME_IN_USEC(0);

}

DRIVER_INIT(rsgun)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6034d04);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x6036152);

	install_standard_protection(machine);

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

DRIVER_INIT(nameclv3)
{
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[0], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x601eb4c);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_SELECT, FIRST_SPEEDUP_SLOT);
	cpu_set_info_int(machine->cpu[1], CPUINFO_INT_SH2_PCFLUSH_ADDR, 0x602b80e);

	DRIVER_INIT_CALL(stv);
}
