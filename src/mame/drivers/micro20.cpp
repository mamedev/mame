// license:BSD-3-Clause
// copyright-holders: R. Belmont
/****************************************************************************

    micro20.cpp
    GMX Micro 20 single-board computer
    
    68020 + 68881 FPU
    
    800a5e = end of initial 68020 torture test
****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68681.h"
#include "machine/msm58321.h"
#include "machine/wd_fdc.h"
#include "machine/68230pit.h"
#include "softlist.h"

#define MAINCPU_TAG "maincpu"
#define DUART_A_TAG "duarta"
#define DUART_B_TAG "duartb"
#define RTC_TAG		"rtc"
#define FDC_TAG		"fdc"
#define PIT_TAG		"pit"

class micro20_state : public driver_device
{
public:
	micro20_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, MAINCPU_TAG),
		m_rom(*this, "bootrom"),
		m_mainram(*this, "mainram"),
		m_pit(*this, PIT_TAG)
	{
	}

	required_device<m68020_device> m_maincpu;
	required_memory_region m_rom;
	required_shared_ptr<uint32_t> m_mainram;
	required_device<pit68230_device> m_pit;
	
	virtual void machine_start() override;
	virtual void machine_reset() override;
	
	DECLARE_WRITE_LINE_MEMBER(m68k_reset_callback);
	
private:

};

void micro20_state::machine_start()
{
}

void micro20_state::machine_reset()
{
	uint32_t *pROM = (uint32_t *)m_rom->base();
	uint32_t *pRAM = (uint32_t *)m_mainram.target();
	
	pRAM[0] = pROM[0];
	pRAM[1] = pROM[1];
	m_maincpu->reset();
	
	m_maincpu->set_reset_callback(write_line_delegate(FUNC(micro20_state::m68k_reset_callback),this));
}

WRITE_LINE_MEMBER(micro20_state::m68k_reset_callback)
{
	// startup test explicitly checks if the m68k RESET opcode resets the 68230
	m_pit->reset();
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START(micro20_map, AS_PROGRAM, 32, micro20_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM AM_SHARE("mainram")
	AM_RANGE(0x00800000, 0x0083ffff) AM_ROM AM_REGION("bootrom", 0)
	AM_RANGE(0xffff8000, 0xffff8003) AM_DEVREADWRITE8(FDC_TAG, wd1772_t, status_r, cmd_w,    0xff000000)	
	AM_RANGE(0xffff8000, 0xffff8003) AM_DEVREADWRITE8(FDC_TAG, wd1772_t, track_r, track_w,   0x00ff0000)	
	AM_RANGE(0xffff8000, 0xffff8003) AM_DEVREADWRITE8(FDC_TAG, wd1772_t, sector_r, sector_w, 0x0000ff00)	
	AM_RANGE(0xffff8000, 0xffff8003) AM_DEVREADWRITE8(FDC_TAG, wd1772_t, data_r, data_w,     0x000000ff)	
	AM_RANGE(0xffff8080, 0xffff808f) AM_DEVREADWRITE8(DUART_A_TAG, mc68681_device, read, write, 0xffffffff)
	AM_RANGE(0xffff80a0, 0xffff80af) AM_DEVREADWRITE8(DUART_B_TAG, mc68681_device, read, write, 0xffffffff)	
	AM_RANGE(0xffff80c0, 0xffff80df) AM_DEVREADWRITE8(PIT_TAG, pit68230_device, read, write, 0xffffffff)
ADDRESS_MAP_END

static MACHINE_CONFIG_START( micro20, micro20_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(MAINCPU_TAG, M68020, XTAL_16_67MHz)
	MCFG_CPU_PROGRAM_MAP(micro20_map)
	
	MCFG_MC68681_ADD(DUART_A_TAG, XTAL_3_6864MHz)
//	MCFG_MC68681_IRQ_CALLBACK(WRITELINE(vt240_state, irq13_w))
//	MCFG_MC68681_A_TX_CALLBACK(DEVWRITELINE("host", rs232_port_device, write_txd))
//	MCFG_MC68681_B_TX_CALLBACK(DEVWRITELINE("printer", rs232_port_device, write_txd))
//	MCFG_MC68681_OUTPORT_CALLBACK(WRITE8(vt240_state, duartout_w))

	MCFG_MC68681_ADD(DUART_B_TAG, XTAL_3_6864MHz)
	
	MCFG_DEVICE_ADD(RTC_TAG, MSM58321, XTAL_32_768kHz)
	
	MCFG_WD1772_ADD(FDC_TAG, XTAL_16_67MHz / 2)
	
	MCFG_DEVICE_ADD(PIT_TAG, PIT68230, XTAL_16_67MHz / 2)
MACHINE_CONFIG_END

static INPUT_PORTS_START( micro20 )
INPUT_PORTS_END

/***************************************************************************

  Machine driver(s)

***************************************************************************/


ROM_START( micro20 )
	ROM_REGION32_BE(0x40000, "bootrom", 0)
	ROM_LOAD32_BYTE( "d00-07_u6_6791.bin",  0x000003, 0x010000, CRC(63d66ea1) SHA1(c5dfbc4d81920e1d2e981c52c1af3d486d382a35) ) 
	ROM_LOAD32_BYTE( "d08-15_u8_0dc6.bin",  0x000002, 0x010000, CRC(d62ef21f) SHA1(2779d430b1a0b835807627e707d46547b29ef579) ) 
	ROM_LOAD32_BYTE( "d16-23_u10_e5b0.bin", 0x000001, 0x010000, CRC(cd7acf86) SHA1(db994ed714a1079fbb66616355e8f18d2d1a2005) ) 
	ROM_LOAD32_BYTE( "d24-31_u13_d115.bin", 0x000000, 0x010000, CRC(3646d943) SHA1(97ee54063e2fe49fef2ff68d0f2e39345a75eac5) ) 
ROM_END

COMP( 1984, micro20,  0,        0,      micro20,  micro20, driver_device, 0,  "GMX", "Micro 20",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )


