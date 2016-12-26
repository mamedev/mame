// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom

#include "emu.h"
#include "includes/mzr8105.h"
#include "bus/vme/mzr8300.h"
#include "cpu/m68000/m68000.h"
#include "machine/clock.h"

static ADDRESS_MAP_START (mzr8105_mem, AS_PROGRAM, 16, mzr8105_state)
	ADDRESS_MAP_UNMAP_HIGH
/* The ROMs contains an OS9 bootloader. It is position independent but reset vector suggests that it sits flat on adress 0 (zero) */
	AM_RANGE (0x000000, 0x003fff) AM_ROM AM_REGION("roms", 0x000000) /* System EPROM Area 16Kb OS9 DEBUG - not verified     */
	AM_RANGE (0x004000, 0x01ffff) AM_ROM AM_REGION("roms", 0x004000)/* System EPROM Area 112Kb for System ROM - not verified    */
	AM_RANGE (0x020000, 0x03ffff) AM_RAM /* Not verified */
//	AM_RANGE (0x100000, 0xfeffff)  AM_READWRITE(vme_a24_r, vme_a24_w) /* VMEbus Rev B addresses (24 bits) - not verified */
//	AM_RANGE (0xff0000, 0xffffff)  AM_READWRITE(vme_a16_r, vme_a16_w) /* VMEbus Rev B addresses (16 bits) - not verified */
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START (mzr8105)
INPUT_PORTS_END

static SLOT_INTERFACE_START(mzr8105_vme_cards)
	SLOT_INTERFACE("mzr8300", VME_MZR8300)
SLOT_INTERFACE_END	

/*
 * Machine configuration
 */
MACHINE_CONFIG_START (mzr8105, mzr8105_state)
	MCFG_CPU_ADD ("maincpu", M68000, XTAL_10MHz)
	MCFG_CPU_PROGRAM_MAP (mzr8105_mem)
	MCFG_VME_P1_DEVICE_ADD("vme")
	MCFG_VME_P1_SLOT_ADD ("vme", "slot1", mzr8105_vme_cards, nullptr)
MACHINE_CONFIG_END

/* ROM definitions */
/* mzr8300 UPD7201 init sequence
 * :upd B Reg 02 <- 04 Interrupt vector
 * :upd B Reg 06 <- 00 Sync byte
 * :upd B Reg 07 <- 00 Sync byte
 * :upd B Reg 03 <- c1 Rx 8 bit chars + Rx enable
 * :upd B Reg 04 <- 44 16x clock, 1 stop bit ( == async mode), no parity
 * :upd B Reg 05 <- 68 Tx 8 bit chars + Tx enable
 */
ROM_START (mzr8105)
	ROM_REGION (0x20000, "roms", 0)
	ROM_LOAD16_BYTE ("mzros9LB.bin", 0x000000, 0x2000, CRC (7c6a354d) SHA1 (2721eb649c8046dbcb517a36a97dc0816cd133f2))
	ROM_LOAD16_BYTE ("mzros9HB.bin", 0x000001, 0x2000, CRC (d18e69a6) SHA1 (a00b68f4d649bcc09a29361f8692e52be12b3792))
ROM_END

/* Driver */
/*    YEAR  NAME          PARENT  COMPAT   MACHINE         INPUT     CLASS          INIT COMPANY                  FULLNAME          FLAGS */
COMP (1987, mzr8105,      0,      0,       mzr8105,        mzr8105, driver_device,  0,   "Mizar Inc",             "Mizar VME8105",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_TYPE_COMPUTER )
