// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  Motorola MVME-350 6U Intelligent Tape Controller driver, initially derived 
 *  from hk68v10.c
 *
 *  31/08/2015
 *
 * I baught this board from http://www.retrotechnology.com without documentation.
 * It has a Motorola 68010 CPU @ 10MHz, 128 Mb RAM and two 2764 EPROMS with 
 * QIC-02 tape controller firmware. The board also populates a 68230 PIT and loads
 * of descrete TTL components.
 *
 *                                                                                   
 *       ||                                                                          
 * ||    ||                                                                          
 * ||||--||                                                                          
 * ||||--||                                                                          
 * ||    ||____________________________________________________________   ___ 
 *       ||                                                            |_|   |
 *       ||                                                            | |   |
 *       ||                                                            | |   |
 *       ||                                                            | |   |
 *       ||                                                            | |   |
 *       ||                                                            | |   |
 *       ||                                                            | |VME|
 *       ||                                                            | |   |
 *       ||                                                            | |P1 |
 *       ||                                                            | |   |
 *       ||                                                            | |   |
 *       ||                                                            | |   |
 *       ||                                                            | |   |
 *       ||                                                            | |   |
 *       ||                                                            | |   |
 *       ||                                                            |_|   |
 *       ||                                                              |___|
 *       ||                                                              |  
 *       ||                                                              |  
 *       ||                                                              |  
 *       ||                                                              |  
 *       ||                                                              |  
 *       ||                                                              |  
 *       ||                                                              |  
 *       ||                                                              |  
 *       ||                                                              |___
 *       ||                                                             _|   |
 *       ||                                                            | |   |
 *       ||                                                            | |   |
 *       ||                                                            | |   |
 *       ||                                                            | |   |
 *       ||                                                            | |VME|
 *       ||                                                            | |   |
 *       ||                                                            | |P2 |
 *       ||                                                            | |   |
 *       ||                                                            | |   |
 *       ||                                                            | |   |
 *       ||                                                            | |   |
 *       ||                                                            | |   |
 *       ||                                                            | |   |
 *       ||                                                            | |   |
 *       ||                                                            | |   |
 *       ||                                                            |_|   |
 *       ||                                                              |___|
 * ||    ||------------------------------------------------------------+-+             
 * ||||--||                                                            
 * ||||--||                                                            
 * ||
 *
 * History of Motorola VME division
 *---------------------------------
 * TBD
 *
 * Misc links about Motorola VME division and this board: TBD
 *
 * Address Map 
 * --------------------------------------------------------------------------
 * Local     to VME       Decscription
 * --------------------------------------------------------------------------
 * 0x000000               Up to 128Kb System ROM with RESET vector
 * 0x020000               RAM with vectors
 * 0x020500               RAM Top of stack
 * 0x040000               PIT device
 * --------------------------------------------------------------------------
 *
 * Interrupt sources MVME
 * ----------------------------------------------------------
 * Description                  Device  Lvl  IRQ    VME board
 *                           /Board      Vector  Address
 * ----------------------------------------------------------
 * On board Sources                     
 *  
 * Off board Sources (other VME boards)
 *
 * ----------------------------------------------------------
 *
 * DMAC Channel Assignments
 * ----------------------------------------------------------
 * Channel         M10            V10
 * ----------------------------------------------------------
 *
 *
 *  TODO:
 *  - Dump the ROMs (DONE)
 *  - Setup a working address map
 *  - Get documentation for VME interface
 *  - Add VME bus driver
 *  - Hook up a CPU board that supports boot from tape (ie MVME-162, MVME 147)
 *  - Get a tape file with a bootable data on it.
 *
 ****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/68230pit.h"

#define LOG(x) x

class mvme350_state : public driver_device
{
public:
mvme350_state(const machine_config &mconfig, device_type type, const char *tag) :
        driver_device (mconfig, type, tag),
                m_maincpu (*this, "maincpu"),
                m_pit(*this, "pit")
{
}

DECLARE_READ16_MEMBER (vme_a24_r);
DECLARE_WRITE16_MEMBER (vme_a24_w);
DECLARE_READ16_MEMBER (vme_a16_r);
DECLARE_WRITE16_MEMBER (vme_a16_w);
virtual void machine_start ();
virtual void machine_reset ();
protected:

private:
        required_device<cpu_device> m_maincpu;
	required_device<pit68230_device> m_pit;

};

static ADDRESS_MAP_START (mvme350_mem, AS_PROGRAM, 16, mvme350_state)
ADDRESS_MAP_UNMAP_HIGH
AM_RANGE (0x000000, 0x01ffff) AM_ROM /* 128 Mb ROM */
AM_RANGE (0x020000, 0x03ffff) AM_RAM /* 128 Mb RAM */
//AM_RANGE(0x060000,  0x060035) AM_DEVREADWRITE8("pit", pit68230_device, data_r, data_w, 0x00ff)
//AM_RANGE(0x100000, 0xfeffff)  AM_READWRITE(vme_a24_r, vme_a24_w) /* VMEbus Rev B addresses (24 bits) - not verified */
//AM_RANGE(0xff0000, 0xffffff)  AM_READWRITE(vme_a16_r, vme_a16_w) /* VMEbus Rev B addresses (16 bits) - not verified */
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START (mvme350)
INPUT_PORTS_END

/* Start it up */
void mvme350_state::machine_start ()
{
        LOG (logerror ("machine_start\n"));
}

void mvme350_state::machine_reset ()
{
        LOG (logerror ("machine_reset\n"));
}

#if 0
/* Dummy VME access methods until the VME bus device is ready for use */
READ16_MEMBER (mvme350_state::vme_a24_r){
        LOG (logerror ("vme_a24_r\n"));
        return (UINT16) 0;
}

WRITE16_MEMBER (mvme350_state::vme_a24_w){
        LOG (logerror ("vme_a24_w\n"));
}

READ16_MEMBER (mvme350_state::vme_a16_r){
        LOG (logerror ("vme_16_r\n"));
        return (UINT16) 0;
}

WRITE16_MEMBER (mvme350_state::vme_a16_w){
        LOG (logerror ("vme_a16_w\n"));
}
#endif

/*
 * Machine configuration
 */
static MACHINE_CONFIG_START (mvme350, mvme350_state)
        /* basic machine hardware */
        MCFG_CPU_ADD ("maincpu", M68010, XTAL_10MHz)
        MCFG_CPU_PROGRAM_MAP (mvme350_mem)
        /* PIT Parallel Interface and Timer device, assuming strapped for on board clock */
	MCFG_DEVICE_ADD("pit", PIT68230, XTAL_16MHz / 2)

MACHINE_CONFIG_END

/* ROM definitions */
ROM_START (mvme350)
ROM_REGION (0x1000000, "maincpu", 0)

ROM_LOAD16_BYTE ("mvme350U40v2.3.bin", 0x0000, 0x4000, CRC (bcef82ef) SHA1 (e6fdf26e4714cbaeb3e97d7b5acf02d64d8ad744))
ROM_LOAD16_BYTE ("mvme350U47v2.3.bin", 0x0001, 0x4000, CRC (582ce095) SHA1 (d0929dbfeb0cfda63df6b5bc29ee27fbf665def7))

/*
 * System ROM information
 *
 * The ROMs known commands from different sources:
 *
 *  NONE
 * 
 *  This is controller board which sets up the board and then executes a STOP instruction
 * awaiting a CPU on the VME bus to request its services.
 */
ROM_END

/* Driver */
/*    YEAR  NAME          PARENT  COMPAT   MACHINE         INPUT     CLASS          INIT COMPANY                  FULLNAME          FLAGS */
COMP (1985, mvme350,      0,      0,       mvme350,        mvme350, driver_device, 0,   "Motorola",   "MVME-350", MACHINE_IS_SKELETON )
