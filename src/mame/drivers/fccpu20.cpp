// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/***************************************************************************
 *
 *  Force SYS68K CPU-20 VME SBC drivers
 *
 *  24/12/2016
 *
 * Thanks to Al Kossow and his site http://www.bitsavers.org/ I got the information
 * required to start the work with this driver.
 *
 *
 *       ||
 * ||    ||  CPU-20
 * ||||--||_____________________________________________________________
 * ||||--||                                                             |
 * ||    ||                                                           _ |__
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |VME|
 *       ||                                                          | |   |
 *       ||                                                          | |P1 |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          |_|   |
 *       ||                                                            |___|
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |
 *       ||                                                            |___
 *       ||                                                           _|   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |VME|
 *       ||                                                          | |   |
 *       ||                                                          | |P2 |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          | |   |
 *       ||                                                          |_|   |
 *       ||                                                            |___|
 * ||    ||                                                              +
 * ||||--||                                                              |
 * ||||--||--------------------------------------------------------------+
 * ||
 *
 * History of Force Computers
 *---------------------------
 *
 * Misc links about Force Computes and this board:
 *------------------------------------------------
 * http://bitsavers.trailing-edge.com/pdf/forceComputers/
 *
 * Description(s)
 * -------------
 * CPU-20 has the following feature set
 *
 * Address Map
 * --------------------------------------------------------------------------
 *  Range                   Decscription
 * --------------------------------------------------------------------------
 * 00000000-0xxFFFFF        Shared DRAM D8-D32
 * 0yy00000-FAFFFFFF        VME A32 D8-D32     yy=xx+1
 * FB000000-FBFEFFFF        VME A24 D8-D32
 * FBFF0000-FBFFFFFF        VME A16 D8-D32
 * FC000000-FCFEFFFF        VME A24 D8-D16
 * FCFF0000-FCFFFFFF        VME A16 D8-D16
 *  .... TBC
 * --------------------------------------------------------------------------
 *
 * PIT #1 hardware wiring
 * ----------------------------------------------------------
 * PA0-PA3  TBC
 * PA4-PA7
 * H1-H4
 * PB0-PB2
 * PB3-PB4
 * PB5
 * PB6-PB7
 * PC0,PC1
 * PC4,PC7
 * PC2
 * PC3
 * PC5
 * PC6
 *
 * PIT #2 hardware setup wiring
 * ----------------------------------------------------------
 * PA0-PA7  TBC
 * H1-H4
 * PB0-PB2
 * PB3-PB7
 * PC0-PC1
 * PC2
 * PC3
 * PC4
 * PC5
 * PC6
 * PC7
 *
 *---------------------------------------------------------------------------
 *  TODO:
 *  - Find accurate documentation and adjust memory map
 *  - Add layouts and system description(s)
 *  - Write & add 68561 UART
 *  - Write & add VME device
 *  - Write & add 68153 BIM
 *  - Add 68230 PIT
 *  - Add variants of boards in the CPU-20 and CPU-21 family
 *  - Add FGA, DUSCC devices and CPU-22 variants
 *
 ****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"

#define LOG_GENERAL 0x01
#define LOG_SETUP   0x02
#define LOG_PRINTF  0x04

#define VERBOSE 0 // (LOG_PRINTF | LOG_SETUP  | LOG_GENERAL)

#define LOGMASK(mask, ...)   do { if (VERBOSE & mask) logerror(__VA_ARGS__); } while (0)
#define LOGLEVEL(mask, level, ...) do { if ((VERBOSE & mask) >= level) logerror(__VA_ARGS__); } while (0)

#define LOG(...)      LOGMASK(LOG_GENERAL, __VA_ARGS__)
#define LOGSETUP(...) LOGMASK(LOG_SETUP,   __VA_ARGS__)

#if VERBOSE & LOG_PRINTF
#define logerror printf
#endif

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

class cpu20_state : public driver_device
{
public:
cpu20_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device (mconfig, type, tag)
		, m_maincpu (*this, "maincpu")
	{
	}
	DECLARE_READ32_MEMBER (bootvect_r);
	DECLARE_WRITE32_MEMBER (bootvect_w);
	virtual void machine_start () override;
	virtual void machine_reset () override;

private:
	required_device<m68000_base_device> m_maincpu;
	// Pointer to System ROMs needed by bootvect_r and masking RAM buffer for post reset accesses
	uint32_t  *m_sysrom;
	uint32_t  m_sysram[2];
};

static ADDRESS_MAP_START (cpu20_mem, AS_PROGRAM, 32, cpu20_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE (0x00000000, 0x00000007) AM_ROM AM_READ  (bootvect_r)   /* ROM mirror just during reset */
	AM_RANGE (0x00000000, 0x00000007) AM_RAM AM_WRITE (bootvect_w)   /* After first write we act as RAM */
	AM_RANGE (0x00000008, 0x003fffff) AM_RAM /* RAM  installed in machine start */
	AM_RANGE (0xff040000, 0xff04ffff) AM_RAM /* RAM  installed in machine start */
	AM_RANGE (0xff000000, 0xff00ffff) AM_ROM AM_REGION("roms", 0x0000)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START (cpu20)
INPUT_PORTS_END

/* Start it up */
void cpu20_state::machine_start ()
{
	LOGSETUP("%s\n", FUNCNAME);

	save_pointer (NAME (m_sysrom), sizeof(m_sysrom));
	save_pointer (NAME (m_sysram), sizeof(m_sysram));

	/* Setup pointer to bootvector in ROM for bootvector handler bootvect_r */
	m_sysrom = (uint32_t*)(memregion ("roms")->base());
}

void cpu20_state::machine_reset ()
{
	LOGSETUP("%s\n", FUNCNAME);

	/* Reset pointer to bootvector in ROM for bootvector handler bootvect_r */
	if (m_sysrom == &m_sysram[0]) /* Condition needed because memory map is not setup first time */
		m_sysrom = (uint32_t*)(memregion ("roms")->base());
}

#if 0
/*                                                                              setup board ID */
DRIVER_INIT_MEMBER( cpu20_state, cpu20x )      { LOGSETUP("%s\n", FUNCNAME); m_board_id = 0x50; }
DRIVER_INIT_MEMBER( cpu20_state, cpu20xa )     { LOGSETUP("%s\n", FUNCNAME); m_board_id = 0x50; }
DRIVER_INIT_MEMBER( cpu20_state, cpu20za )     { LOGSETUP("%s\n", FUNCNAME); m_board_id = 0x50; }
DRIVER_INIT_MEMBER( cpu20_state, cpu20zbe )    { LOGSETUP("%s\n", FUNCNAME); m_board_id = 0x50; }
DRIVER_INIT_MEMBER( cpu20_state, cpu20be8 )    { LOGSETUP("%s\n", FUNCNAME); m_board_id = 0x50; }
DRIVER_INIT_MEMBER( cpu20_state, cpu20be16 )   { LOGSETUP("%s\n", FUNCNAME); m_board_id = 0x50; }
DRIVER_INIT_MEMBER( cpu20_state, cpu20lite4 )  { LOGSETUP("%s\n", FUNCNAME); m_board_id = 0x50; }
DRIVER_INIT_MEMBER( cpu20_state, cpu20lite8 )  { LOGSETUP("%s\n", FUNCNAME); m_board_id = 0x50; }
DRIVER_INIT_MEMBER( cpu20_state, cpu33 )       { LOGSETUP("%s\n", FUNCNAME); m_board_id = 0x68; } // 0x60 skips FGA prompt
#endif

/* Boot vector handler, the PCB hardwires the first 8 bytes from 0xff800000 to 0x0 at reset*/
READ32_MEMBER (cpu20_state::bootvect_r){
	LOG("%s\n", FUNCNAME);
	return m_sysrom[offset];
}

WRITE32_MEMBER (cpu20_state::bootvect_w){
	LOG("%s\n", FUNCNAME);
	m_sysram[offset % sizeof(m_sysram)] &= ~mem_mask;
	m_sysram[offset % sizeof(m_sysram)] |= (data & mem_mask);
	m_sysrom = &m_sysram[0]; // redirect all upcomming accesses to masking RAM until reset.
}

#if 0
void cpu20_state::update_irq_to_maincpu()
{
	LOGINT(("%s()\n", FUNCNAME);
	LOGINT((" - fga_irq_level: %02x\n", fga_irq_level));
	LOGINT((" - fga_irq_state: %02x\n", fga_irq_state));
	switch (fga_irq_level & 0x07)
	{
	case 1: m_maincpu->set_input_line(M68K_IRQ_1, fga_irq_state); break;
	case 2: m_maincpu->set_input_line(M68K_IRQ_2, fga_irq_state); break;
	case 3: m_maincpu->set_input_line(M68K_IRQ_3, fga_irq_state); break;
	case 4: m_maincpu->set_input_line(M68K_IRQ_4, fga_irq_state); break;
	case 5: m_maincpu->set_input_line(M68K_IRQ_5, fga_irq_state); break;
	case 6: m_maincpu->set_input_line(M68K_IRQ_6, fga_irq_state); break;
	case 7: m_maincpu->set_input_line(M68K_IRQ_7, fga_irq_state); break;
	default: logerror("Programmatic error in %s, please report\n", FUNCNAME);
	}
}
#endif

/*
 * Machine configuration
 */
static MACHINE_CONFIG_START (cpu20, cpu20_state)
	/* basic machine hardware */
	MCFG_CPU_ADD ("maincpu", M68020, XTAL_16MHz) /* Crytstal not verified */
	MCFG_CPU_PROGRAM_MAP (cpu20_mem)
MACHINE_CONFIG_END

/* ROM definitions */
ROM_START (fccpu20) /* This is an original rom dump */
	ROM_REGION32_BE(0x10000, "roms", 0)
// Boots with Board ID set to: 0x36 (FGA002 BOOT on terminal P4, "Wait until harddisk is up to speed " on terminal P1)
	ROM_LOAD32_BYTE("L.BIN",  0x000002, 0x4000, CRC (174ab801) SHA1 (0d7b8ed29d5fdd4bd2073005008120c5f20128dd))
	ROM_LOAD32_BYTE("LL.BIN", 0x000003, 0x4000, CRC (9fd9e3e4) SHA1 (e5a7c87021e6be412dd5a8166d9f62b681169eda))
	ROM_LOAD32_BYTE("U.BIN",  0x000001, 0x4000, CRC (d1afe4c0) SHA1 (b5baf9798d73632f7bb843cbc4b306c8c03f4296))
	ROM_LOAD32_BYTE("UU.BIN", 0x000000, 0x4000, CRC (b54d623b) SHA1 (49b272184a04570b09004de71fae0ed0d1bf5929))
ROM_END

/*
 * System ROM information
 *
 * xxxxxxx bootprom version xxx is released mmm dd, yyyy, coprighted by FORCE Computers Gmbh
 */
/* Driver */
/*    YEAR  NAME          PARENT  COMPAT   MACHINE         INPUT     CLASS          INIT         COMPANY                   FULLNAME                FLAGS */
COMP (1986, fccpu20,      0,       0,      cpu20,          cpu20,    driver_device,      0,      "Force Computers Gmbh",   "SYS68K/CPU-20",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_TYPE_COMPUTER )
