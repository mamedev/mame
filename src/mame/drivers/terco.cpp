// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/*
 * The 4490 CNC Mill Control unit
 *__________________________________________________________________________________________________________
 *                                                                                                          |
 *__________________________________________________________________________________________________________|
 *
 * History of Terco
 *------------------
 * 
 * Misc links about the boards supported by this driver.
 *-----------------------------------------------------
 *
 *  TODO:
 * ------
 *  - Add PCB layouts     
 *  - Keyboard            
 *  - Display/CRT         
 *  - Clickable Artwork   
 *  - Cassette i/f
 *  - Expansion bus
 */

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
//#include "machine/6850acia.h"

#define LOG_SETUP   (1U <<  1)
#define LOG_READ    (1U <<  2)
#define LOG_BCD     (1U <<  3)

//#define VERBOSE (LOG_BCD|LOG_SETUP)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGSETUP(...) LOGMASKED(LOG_SETUP,   __VA_ARGS__)
#define LOGR(...)     LOGMASKED(LOG_READ,    __VA_ARGS__)
#define LOGBCD(...)   LOGMASKED(LOG_BCD,     __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

/* Terco CNC Control Station 4490 */
class t4490_state : public driver_device
{
public:
	t4490_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	,m_maincpu(*this, "maincpu")
	,m_pia1(*this, "pia1")
	,m_pia2(*this, "pia2")
	  //	,m_acia(*this, "acia")
	{ }
	required_device<m6800_cpu_device> m_maincpu;
	virtual void machine_reset() override { m_maincpu->reset(); LOG(("--->%s()\n", FUNCNAME)); };
protected:
	required_device<pia6821_device> m_pia1;
	required_device<pia6821_device> m_pia2;
//	required_device<acia6850_device> m_acia;
};

static ADDRESS_MAP_START( t4490_map, AS_PROGRAM, 8, t4490_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x3000, 0x3fff) AM_ROM AM_REGION("maincpu", 0x3000)
	AM_RANGE(0x9500, 0x95ff) AM_RAM
	AM_RANGE(0x9036, 0x9037) AM_DEVREADWRITE("pia1", pia6821_device, read, write)
	AM_RANGE(0x903a, 0x903b) AM_DEVREADWRITE("pia2", pia6821_device, read, write)
//	AM_RANGE(0xc820, 0xc823) AM_DEVREADWRITE("acia", acia6850_device, read, write)
	AM_RANGE(0xa000, 0xffff) AM_ROM AM_REGION("maincpu", 0xa000)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( t4426 )
INPUT_PORTS_END

static INPUT_PORTS_START( t4490 )
INPUT_PORTS_END

static MACHINE_CONFIG_START( t4490, t4490_state )
	MCFG_CPU_ADD("maincpu", M6800, XTAL_8MHz/4) // divided by a MC6875
	MCFG_CPU_PROGRAM_MAP(t4490_map)

	/* devices */
	MCFG_DEVICE_ADD("pia1", PIA6821, 0)
	MCFG_DEVICE_ADD("pia2", PIA6821, 0)
MACHINE_CONFIG_END

ROM_START( t4490 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "terco4490-3861104.bin", 0x3000, 0x1000, CRC(d5fd17cc) SHA1(9a3564fa69b897ec51b49ad34f2d2696cb78ee9b) )
	ROM_LOAD( "terco4490-A861104.bin", 0xa000, 0x1000, CRC(65b8e7d0) SHA1(633217fc4aa301d87790bb8744b72ef030a4c262) )
	ROM_LOAD( "terco4490-B861104.bin", 0xb000, 0x1000, CRC(5a0ce3f2) SHA1(7ec455b9075454ce5943011a1dfb5725857168f5) )
	ROM_LOAD( "terco4490-C861104.bin", 0xc000, 0x1000, CRC(0627c68c) SHA1(bf733d3ffad3f1e75684e833afc9d10d33ca870f) )
	ROM_LOAD( "terco4490-D861104.bin", 0xd000, 0x1000, CRC(2156476d) SHA1(0d70c6285541746ef15cad0d47b2d752e228abfc) )
	ROM_LOAD( "terco4490-E861104.bin", 0xe000, 0x1000, CRC(b317fa37) SHA1(a2e037a3a88b5d780067a86e52c6f7c103711a98) )
	ROM_LOAD( "terco4490-F861104.bin", 0xf000, 0x1000, CRC(a45bc3e7) SHA1(e12efa9a4c72e4bce1d59ad359ee66d7c3babfa6) )
ROM_END

//    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   CLASS            INIT  COMPANY             FULLNAME                    FLAGS
COMP( 1986, t4490,	0,          0,      t4490,      t4490,  driver_device,   0,    "Terco AB",         "Terco 4490 Mill CNC Control 4490",  MACHINE_IS_SKELETON )
