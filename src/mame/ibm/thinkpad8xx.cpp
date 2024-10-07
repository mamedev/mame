// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************
Skeleton driver for IBM ThinkPad Power Series.
The IBM ThinkPad Power Series (800/820/821/822/823/850/851/860) is a laptop series from the ThinkPad line
that was manufactured by IBM. It is based on the PowerPC architecture.
All of the PowerPC ThinkPads could run Windows NT 3.51 and 4.0, AIX 4.1.x, and Solaris Desktop 2.5.1
PowerPC Edition. It is also possible to run certain PowerPC versions of Linux on the 800 Series.
830 and 850 models can also run OS/2 Warp PowerPC Edition.

This has no chance of running until MAME's PowerPC CPU core supports little Endian mode and motherboards
wired for little Endian operating systems.

Hardware for the 850 model:
-SCSI hard disk and SCSI CD-ROM drive (with a NCR 53C810 PCI-SCSI I/O Processor).
-Two PC Card expansion slots (with a Ricoh RF53C366L PC Card interface controller and a
 MAX780 Dual-Slot PCMCIA Analog Power Controller).
-Video:
   -IBM 85G7815 (by Seiko/Epson).
   -Western Digital WD90C24A SVGA LCD controller.
   -Two Hitachi HM51S4260 262144 x 16bit DRAM (1MB of video display memory).
   -10.4" 640×480 or 800×600 screen.
-Video capture:
   -Brooktree Bt812 NTSC/PAL to RGB/YCrCb Decoder.
   -Two Hitachi HM530281 high speed 331776 x 8bit Frame buffer DRAM.
   -ASCII V7310AS (アスキー, Asukii) Video Capture Device.
-Crystal CS4231 16bit stereo codec for audio.
-Two DRAM DIMMs slots.
-Hitachi H8/338 (HD6473388) for main board supervision.
-CPU:
    -IBM PowerPC 603e @ 100MHz (PPCI603eFC100BPQ).
    -Two 32k x 36bits IBM043614 burst SRAM (256k L2 cache total).
    -IDT71216 240K (16K x 15bit) cache-tag RAM.
    -33.333 MHz xtal (tripled for 100MHz system clock).
-Dallas DS1585S RTC.
-Intel S82378ZB PCIset.
-National Semiconductor DP87322VF (SuperI/O III, Floppy Disk Controller with Dual UARTs,
 Enhanced Parallel Port, and IDE Interface).
-Motorola XPC105ARX66CD (PowerPC PCI Bridge/Memory Controller).
-Other ICs: S-MOS 85G7814, S-MOS 85G2680

More info: http://oldcomputer.info/portables/tp850/ibm_ppc_thinkpad_redbook.pdf

***********************************************************************************************************/

#include "emu.h"
#include "cpu/h8/h8325.h"
#include "cpu/powerpc/ppc.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class thinkpad8xx_state : public driver_device
{
public:
	thinkpad8xx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }


	void thinkpad850(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
};


static INPUT_PORTS_START(thinkpad8xx)
INPUT_PORTS_END

void thinkpad8xx_state::thinkpad850(machine_config &config)
{
	PPC603(config, m_maincpu, 33.333_MHz_XTAL * 3); // IBM PPCI603eFC100BPQ

	// All BIOS ROM chip lines are routed through the S-MOS 85G7814

	H8325(config, "mcu", XTAL(10'000'000)); // Actually an H8/338 (HD6473388: 48k-byte ROM; 2k-byte RAM), unknown clock

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SOFTWARE_LIST(config, "thinkpad8xx").set_original("thinkpad8xx");
}


ROM_START(thinkpad850)
	ROM_DEFAULT_BIOS("v101")
	ROM_SYSTEM_BIOS( 0, "v100", "v1.00 (91G0610, 07-03-1995)" )
	ROM_SYSTEM_BIOS( 1, "v101", "v1.01 (91G1671, 09-10-1996)" )

	ROM_REGION( 0x80000, "maincpu", 0 )
	ROMX_LOAD( "91g0610_ibm_dakota_v100_mbm29f040a.u21", 0x00000, 0x80000, CRC(169a79c4) SHA1(da74a2f346b732add62d08ca5f34f192cae5d033), ROM_BIOS(0) )
	ROMX_LOAD( "91g1671_ibm_dakota_v101_mbm29f040a.u21", 0x00000, 0x80000, CRC(5210dbd6) SHA1(8e0bbbe130e6fdb06ef307bb5addbcb993a8a41f), ROM_BIOS(1) ) // Needed for installing Windows NT

	ROM_REGION(0xe000, "mcu", 0)
	ROM_LOAD( "hd6473388.u15", 0x0000, 0xe000, NO_DUMP )
ROM_END

} // anonymous namespace

//    YEAR, NAME,        PARENT, COMPAT, MACHINE,     INPUT,       CLASS,             INIT,       COMPANY, FULLNAME,       FLAGS
COMP( 1996, thinkpad850, 0,      0,      thinkpad850, thinkpad8xx, thinkpad8xx_state, empty_init, "IBM",   "ThinkPad 850", MACHINE_IS_SKELETON )
