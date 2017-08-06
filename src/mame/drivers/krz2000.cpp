// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    krz2000.cpp - Kurzweil K2000 series

    Skeleton driver by R. Belmont
    
    Hardware in brief:
    	TMP68301 CPU @ 16 MHz
    	uPD72064 FDC
    	85C30 SCSI
		M37450 on I/O board to handle panel/display/keyboard scanning
		HD6303 on I/O board to manage reverb DSP program loading

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/tmp68301.h"
#include "machine/upd765.h"
#include "machine/ncr5380n.h"
#include "machine/nscsi_cd.h"
#include "machine/nscsi_hd.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

class k2000_state : public driver_device
{
public:
	k2000_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;

	virtual void machine_start() override;
	virtual void machine_reset() override;
};

void k2000_state::machine_start()
{
}

void k2000_state::machine_reset()
{
}

static ADDRESS_MAP_START( k2000_map, AS_PROGRAM, 16, k2000_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x100000, 0x11ffff) AM_RAM
	AM_RANGE(0xfffc00, 0xffffff) AM_DEVREADWRITE("tmp68301", tmp68301_device, regs_r, regs_w)  // TMP68301 Registers
ADDRESS_MAP_END

static MACHINE_CONFIG_START( k2000 )
	MCFG_CPU_ADD("maincpu", M68301, XTAL_12MHz)
	MCFG_CPU_PROGRAM_MAP(k2000_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("tmp68301",tmp68301_device,irq_callback)

	MCFG_DEVICE_ADD("tmp68301", TMP68301, 0) 
	
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
MACHINE_CONFIG_END

static INPUT_PORTS_START( k2000 )
INPUT_PORTS_END

ROM_START( k2000 )
	ROM_REGION(0x140000, "maincpu", 0)
	ROM_LOAD16_BYTE( "k2j-k2rj_eng_hi__v2.0j_3b69__(c)_1993_yca.tms27c040.u6", 0x000000, 0x080000, CRC(35c17fc3) SHA1(b91deec0127669b46af05a2acaa212e29e49abfb) ) 
	ROM_LOAD16_BYTE( "k2j-k2rj_eng_lo__v2.0j_0db0__(c)_1993_yca.tms27c040.u3", 0x000001, 0x080000, CRC(11c7f436) SHA1(c2afe84b58d71932f223097ea01812eb513bd740) ) 
	ROM_LOAD16_BYTE( "k2j-k2rj_su_hi__v12ts_5e89__(c)_1993_yca.m27c1001.u5", 0x100000, 0x020000, CRC(16e0bdb7) SHA1(962fa10896f6a95210d752be28f02640869893a4) ) 
	ROM_LOAD16_BYTE( "k2j-k2rj_su_lo__v12ts_2f52__(c)_1993_yca.m27c1001.u2", 0x100001, 0x020000, CRC(cb11e837) SHA1(bcdf3d5abe8c53727a142008acb2755ed0ecc6ea) ) 

	ROM_REGION(0x2000, "pals", 0)
	ROM_LOAD( "pseudo_v4d.u11.gal16v8b.jed", 0x000000, 0x000bd0, CRC(43561132) SHA1(a0c567c81022bc7fb83023d89556ccd5aa1ab36d) ) 
	ROM_LOAD( "sndram_v1.u50.gal16v8b.jed", 0x001000, 0x000bd0, CRC(cabc9335) SHA1(968fa5baa43c7589c901f09b12085437834aeb37) ) 
ROM_END

CONS( 1990, k2000, 0, 0, k2000, k2000, k2000_state, 0, "Kurzweil Music Systems", "K2000", MACHINE_NOT_WORKING|MACHINE_NO_SOUND )

