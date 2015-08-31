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
 * Misc links about Motorola VME division and this board:
 *
 * Address Map 
 * --------------------------------------------------------------------------
 * Local     to VME       Decscription
 * --------------------------------------------------------------------------
 * 0xffffff                                    To of A24  
 * 0xfc0000            
 * 0x200000                                    (top of 2 meg RAM) 
 * 0x100000                                    (top of 1 meg RAM)
 *                        Exception Vectors
 * 0x000000                                    (bottom of memory) 
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

#define LOG(x) x

class mvme350_state : public driver_device
{
public:
mvme350_state(const machine_config &mconfig, device_type type, const char *tag) :
        driver_device (mconfig, type, tag),
        m_maincpu (*this, "maincpu")
{
}

        //DECLARE_READ16_MEMBER (bootvect_r);
        //DECLARE_WRITE16_MEMBER (bootvect_w);
DECLARE_READ16_MEMBER (vme_a24_r);
DECLARE_WRITE16_MEMBER (vme_a24_w);
DECLARE_READ16_MEMBER (vme_a16_r);
DECLARE_WRITE16_MEMBER (vme_a16_w);
virtual void machine_start ();
virtual void machine_reset ();
protected:

private:
required_device<cpu_device> m_maincpu;

// Pointer to System ROMs needed by bootvect_r and masking RAM buffer for post reset accesses
// UINT16  *m_sysrom;
// UINT16  m_sysram[4];
};

static ADDRESS_MAP_START (mvme350_mem, AS_PROGRAM, 16, mvme350_state)
ADDRESS_MAP_UNMAP_HIGH
//AM_RANGE (0x000000, 0x000007) AM_ROM AM_READ  (bootvect_r)       /* ROM mirror just durin reset */
//AM_RANGE (0x000000, 0x000007) AM_RAM AM_WRITE (bootvect_w)       /* After first write we act as RAM */
//AM_RANGE (0x000008, 0x0fffff) AM_RAM /* 1 Mb RAM */
AM_RANGE (0x000000, 0x01ffff) AM_ROM /* 128 Mb ROM */
AM_RANGE (0x020000, 0x03ffff) AM_RAM /* 128 Mb RAM */
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

        /* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
        //	m_sysrom = (UINT16*)(memregion ("maincpu")->base () + 0x0fc0000);
}

/* Support CPU resets 

  TODO: Investigate why the user need two 'softreset' commands for the below to work.
        If only one 'softreset' is given the reset PC gets the RAM content, not the intended ROM vector.
        Race conditions? Wrong call order in memory system? Debugger prefetch accesses? 
*/
void mvme350_state::machine_reset ()
{
        LOG (logerror ("machine_reset\n"));

        /* Reset pointer to bootvector in ROM for bootvector handler bootvect_r */
        //        if (m_sysrom == &m_sysram[0]) /* Condition needed because memory map is not setup first time */
	//        m_sysrom = (UINT16*)(memregion ("maincpu")->base () + 0x0fc0000);
}

#if 0
/* Boot vector handler, the PCB hardwires the first 8 bytes from 0xfc0000 to 0x0 at reset*/
READ16_MEMBER (mvme350_state::bootvect_r){
        //LOG (logerror ("bootvect_r %s\n", m_sysrom != &m_sysram[0] ? "as reset" : "as swapped"));
        return m_sysrom [offset];
}

WRITE16_MEMBER (mvme350_state::bootvect_w){
        LOG (logerror("bootvect_w offset %08x, mask %08x, data %04x\n", offset, mem_mask, data));
        m_sysram[offset % sizeof(m_sysram)] &= ~mem_mask;
        m_sysram[offset % sizeof(m_sysram)] |= (data & mem_mask);
        m_sysrom = &m_sysram[0]; // redirect all upcomming accesses to masking RAM until reset. 
}

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

/* Terminal Port config */

MACHINE_CONFIG_END

/* ROM definitions */
ROM_START (mvme350)
ROM_REGION (0x1000000, "maincpu", 0)

ROM_LOAD16_BYTE ("hk68kv10U23.bin", 0xFC0001, 0x2000, CRC (632aa026) SHA1 (f2b1ed0cc38dfbeb1602c013e00757015400720d))
ROM_LOAD16_BYTE ("hk68kv10U12.bin", 0xFC0000, 0x2000, CRC (f2d688e9) SHA1 (e68699965645f0ce53de47625163c3eb02c8b727))

/*
 * System ROM information
 *
 * The ROMs known commands from different sources:
 *
 *  TBD
 *
 */
ROM_END

/* Driver */
/*    YEAR  NAME          PARENT  COMPAT   MACHINE         INPUT     CLASS          INIT COMPANY                  FULLNAME          FLAGS */
COMP (1985, mvme350,      0,      0,       mvme350,        mvme350, driver_device, 0,   "Motorola",   "MVME-350", MACHINE_IS_SKELETON )
