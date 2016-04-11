// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  11/04/2016
 *  Force SYS68K ISCSI-1 driver - This driver will be converted into a slot device once the VME bus driver exists.
 *  The ISCSI-1 board is a VME slave board that reads command and returns results through dual ported RAM to the VME bus.
 *
 * http://bitsavers.trailing-edge.com/pdf/forceComputers/800114_Force_Introduction_to_the_SYS68K_ISCSI-1_Oct86.pdf
 *
 * Address Map
 * ----------------------------------------------------------
 * Address Range     Description
 * ----------------------------------------------------------
 * 00 0000 - 00 0007 Initialisation vectors from system EPROM
 * 00 0008 - 00 1FFF Local SRAM
 * 00 2000 - 01 FFFF Dynamic Dual Port RAM 
 * C4 0000 - C4 001F SCSIbus controller
 * C8 0000 - C8 00FF DMAC
 * CC 0000 - CC 0007 FDC
 * CC 0009 - CC 0009 Control Register
 * D0 0000 - D0 003F PI/T
 * E0 0000 - E7 0000 EPROMs
 * ----------------------------------------------------------
 *
 * Interrupt sources
 * ----------------------------------------------------------
 * Description                  Device  Lvl  IRQ    VME board
 *                           /Board      Vector  Address
 * ----------------------------------------------------------
 * On board Sources
 * ABORT                        P3 p13  1    AV1
 * DMA controller               68450   2    DMAC/AV2
 * SCSI bus controller       NCR 5386S  3    AV3
 * Floppy Disk controller    WD 1772    4    AV4
 * Parallel interface and timer 68230   5    PIT timer
 *                              ---     6    ---
 * Parallel interface and timer 65230   7    PIT port
 * ----------------------------------------------------------
 *
 *  TODO:
 *  - Write and attach a 68450 DMA device
 *  - Write and attach a NCR5386S SCSI device
 *  - Find a floppy image and present it to the WD1772 floppy controller
 *  - Add VME bus driver
 *  - Let a controller CPU board (eg CPU-1 or CPU-30) boot from floppy or SCSI disk
 *
 ****************************************************************************/
#define TODO "Drivers for SCSI NCR5386s and DMA Motorola 68450 devices needed\n"

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/68230pit.h"
#include "machine/wd_fdc.h"
#include "machine/clock.h"

#define VERBOSE 0

#define LOG(x) do { if (VERBOSE) logerror x; } while (0)
#if VERBOSE == 2
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#define LLFORMAT "%I64%"
#else
#define FUNCNAME __PRETTY_FUNCTION__
#define LLFORMAT "%lld"
#endif

#define CPU_CRYSTAL XTAL_20MHz /* Jauch */
#define PIT_CRYSTAL XTAL_16MHz /* Jauch */

class fcscsi1_state : public driver_device
{
public:
fcscsi1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device (mconfig, type, tag),
		m_maincpu (*this, "maincpu")
		,m_fdc (*this, "fdc")
		,m_pit (*this, "pit")
		,m_tcr (0)
{
}

DECLARE_READ16_MEMBER (bootvect_r);
DECLARE_READ8_MEMBER (tcr_r);
DECLARE_WRITE8_MEMBER (tcr_w);

/* Dummy driver routines */
DECLARE_READ8_MEMBER (not_implemented_r);
DECLARE_WRITE8_MEMBER (not_implemented_w);

virtual void machine_start () override;

protected:

private:
required_device<cpu_device> m_maincpu;
required_device<wd1772_t> m_fdc;
required_device<pit68230_device> m_pit;

UINT8 m_tcr;
// Pointer to System ROMs needed by bootvect_r
UINT16  *m_sysrom;
};

static ADDRESS_MAP_START (fcscsi1_mem, AS_PROGRAM, 16, fcscsi1_state)
ADDRESS_MAP_UNMAP_HIGH
AM_RANGE (0x000000, 0x000007) AM_ROM AM_READ (bootvect_r)       /* Vectors mapped from System EPROM */
AM_RANGE (0x000008, 0x001fff) AM_RAM /* SRAM */
AM_RANGE (0x002000, 0x01ffff) AM_RAM /* Dual Ported RAM */
AM_RANGE (0xe00000, 0xe7ffff) AM_ROM /* System EPROM Area 32Kb DEBUGGER supplied */
AM_RANGE (0xd00000, 0xd0003f) AM_DEVREADWRITE8 ("pit", pit68230_device, read, write, 0x00ff)
//AM_RANGE (0xc40000, 0xc4001f) AM_DEVREADWRITE8("scsi", ncr5386s_t, read, write, 0x00ff) /* SCSI Controller interface - device support not yet available*/
//AM_RANGE (0xc80000, 0xc800ff) AM_DEVREADWRITE8("dma", m68450_t, read, write, 0x00ff)    /* DMA Controller interface - device support not yet available */
AM_RANGE (0xc40000, 0xc800ff) AM_READWRITE8 (not_implemented_r, not_implemented_w, 0xffff)  /* Dummy mapping af address area to display message */

AM_RANGE (0xcc0000, 0xcc0007) AM_DEVREADWRITE8("fdc", wd1772_t, read, write, 0x00ff)      /* Floppy Controller interface */
AM_RANGE (0xcc0008, 0xcc0009) AM_READWRITE8 (tcr_r, tcr_w, 0x00ff) /* The Control Register, SCSI ID and FD drive select bits */
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START (fcscsi1)
INPUT_PORTS_END

/* Start it up */
void fcscsi1_state::machine_start ()
{
	LOG (("machine_start\n"));

	/* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
	m_sysrom = (UINT16*)(memregion ("maincpu")->base () + 0xe00000);

}

/* Boot vector handler, the PCB hardwires the first 8 bytes from 0x80000 to 0x0 */
READ16_MEMBER (fcscsi1_state::bootvect_r){
	return m_sysrom [offset];
}

/* The Control Register - descretelly implemented on the PCB 
Bit #: 7 6 5 4 3 2 1 0
       \ \ \ \ \ \ \ \ Floppy Disk Side Select
        \ \ \ \ \ \ \ Floppy Disk Drive Select 0
         \ \ \ \ \ \ Floppy Disk Drive Select 1
          \ \ \ \ \ Floppy Disk Drive Select 2
           \ \ \ \ Floppy Disk Drive Select 3
            \ \ \ ISCSI-l I.D. Bit #0
             \ \ ISCSI-l I.D. Bit #1
              \ ISCSI-l 1.D. Bit #2 
*/

READ8_MEMBER (fcscsi1_state::tcr_r){
	LOG(("%s\n", FUNCNAME));
	return (UINT8) m_tcr;
}

WRITE8_MEMBER (fcscsi1_state::tcr_w){
	LOG(("%s [%02x]\n", FUNCNAME, data));
	m_tcr = data;
	return;
}

READ8_MEMBER (fcscsi1_state::not_implemented_r){
	static int been_here = 0;
	if (!been_here++){
		logerror(TODO);
		printf(TODO);
	}
	return (UINT8) 0;
}

WRITE8_MEMBER (fcscsi1_state::not_implemented_w){
	static int been_here = 0;
	if (!been_here++){
		logerror(TODO);
		printf(TODO);
	}
	return;
}
/*
 * Machine configuration
 */
static MACHINE_CONFIG_START (fcscsi1, fcscsi1_state)
/* basic machine hardware */
MCFG_CPU_ADD ("maincpu", M68010, CPU_CRYSTAL / 2) /* 7474 based frequency divide by 2 */
MCFG_CPU_PROGRAM_MAP (fcscsi1_mem)

/* FDC  */
MCFG_WD1772_ADD("fdc", CPU_CRYSTAL / 2) /* Same clock divider as for the CPU */

/* PIT Parallel Interface and Timer device */
MCFG_DEVICE_ADD ("pit", PIT68230, PIT_CRYSTAL / 2) /* 7474 based frequency divide by 2 */
MACHINE_CONFIG_END

/* ROM definitions */
ROM_START (fcscsi1)
ROM_REGION (0x1000000, "maincpu", 0)

/* Besta ROM:s */
ROM_LOAD16_BYTE ("besta88_scsi_lower.ROM", 0xe00001, 0x4000, CRC (fb3ab364) SHA1 (d79112100f1c4beaf358e006efd4dde5e300b0ba))
ROM_LOAD16_BYTE ("besta88_scsi_upper.ROM", 0xe00000, 0x4000, CRC (41f9cdf4) SHA1 (66b998bbf9459f0a613718260e05e97749532073))
ROM_END

/* Driver */
/*    YEAR  NAME          PARENT  COMPAT   MACHINE         INPUT     CLASS          INIT COMPANY                  FULLNAME          FLAGS */
COMP( 1986, fcscsi1,       0,      0,      fcscsi1,      fcscsi1, driver_device,     0,  "Force Computers Gmbh",  "SYS68K/SCSI-1",   MACHINE_IS_SKELETON )
