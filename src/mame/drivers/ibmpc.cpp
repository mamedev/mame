// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic
/***************************************************************************

    drivers/pc.c

Driver file for IBM PC, IBM PC XT, and related machines.

    PC-XT memory map

    00000-9FFFF   RAM
    A0000-AFFFF   NOP       or videoram EGA/VGA
    B0000-B7FFF   videoram  MDA, page #0
    B8000-BFFFF   videoram  CGA and/or MDA page #1, T1T mapped RAM
    C0000-C7FFF   NOP       or ROM EGA/VGA
    C8000-C9FFF   ROM       XT HDC #1
    CA000-CBFFF   ROM       XT HDC #2
    D0000-EFFFF   NOP       or 'adapter RAM'
    F0000-FDFFF   NOP       or ROM Basic + other Extensions
    FE000-FFFFF   ROM

IBM PC 5150
===========

- Intel 8088 at 4.77 MHz derived from a 14.31818 MHz crystal
- Onboard RAM: Revision A mainboard - min. 16KB, max. 64KB
               Revision B mainboard - min. 64KB, max. 256KB
- Total RAM (using 5161 expansion chassis or ISA memory board): 512KB (rev 1 or rev 2 bios) or 640KB (rev 3 bios)
- Graphics: MDA, CGA, or MDA and CGA
- Cassette port
- Five ISA expansion slots (short type)
- PC/XT keyboard 83-key 'IBM Model F' (some early 'IBM Model M' keyboards can produce scancodes compatible with this as well, but it was an undocumented feature unsupported by IBM)
- Optional 8087 co-processor
- Optional up to 4 (2 internal, 2 external) 160KB single-sided or 360KB double-sided 5 1/4" floppy drives
- Optional 10MB hard disk (using 5161 expansion chassis)
- Optional Game port joystick ISA card (two analog joysticks with 2 buttons each)
- Optional Parallel port ISA card, Unidirectional (upgradable to bidirectional (as is on the ps/2) with some minor hacking; IBM had all the circuitry there but it wasn't hooked up properly)
- Optional Serial port ISA card


IBM PC-JR
=========


IBM PC-XT 5160
==============

- Intel 8088 at 4.77 MHz derived from a 14.31818 MHz crystal
- Onboard RAM: Revision A mainboard (P/N 6181655) - min. 64KB, max. 256KB (4x 64KB banks of 9 IMS2600 64kx1 drams plus parity); could be upgraded to 640KB with a hack; may support the weird 256k stacked-chip dram the AT rev 1 uses)
               Revision B mainboard - min. 256KB, max 640KB (2x 256KB, 2x 64KB banks)
- Total RAM (using 5161 expansion chassis, ISA memory board, or rev B mainboard): 640KB
- Graphics: MDA, CGA, MDA and CGA, EGA, EGA and MDA, or EGA and CGA
- One internal 360KB double-sided 5 1/4" floppy drive
- PC/XT keyboard 83-key 'IBM Model F' (BIOS revisions 1 and 2) (some early 'IBM Model M' keyboards can produce scancodes compatible with this as well, but it was an undocumented feature unsupported by IBM)
- 84-key 'AT/Enhanced' keyboard or 101-key 'IBM Model M' (BIOS revisions 3 and 4)
- Eight ISA expansion slots (short type)
- Optional 8087 co-processor
- Optional second internal 360KB double-sided 5 1/4" floppy drive, if no internal hard disk
- Optional 'half height' 720KB double density 3 1/2" floppy drive
- Optional 10MB hard disk via 5161 expansion chassis
- Optional 10MB or 20MB Seagate ST-412 hard disk (in place of second floppy drive)
- Optional up to 2 external 360KB double-sided 5 1/4" floppy drive
- Optional Game port joystick ISA card (two analog joysticks with 2 buttons each)
- Optional Parallel port ISA card, Unidirectional (upgradable to bidirectional (as is on the ps/2) with some minor hacking; IBM had all the circuitry there but it wasn't hooked up properly)
- Optional Serial port ISA card


IBM PC Jr:

TODO: Which clock signals are available in a PC Jr?
      - What clock is Y1? This eventually gets passed on to the CPU (ZM40?) and some other components by a 8284 (ZM8?).
      - Is the clock attached to the Video Gate Array (ZM36?) exactly 14MHz?

IBM CGA/MDA:
Several different font roms were available, depending on what region the card was purchased in;
Currently known: (probably exist for all the standard codepages)
5788005: US (code page 437)
???????: Greek (code page 737) NOT DUMPED!
???????: Estonian/Lithuanian/Latvian (code page 775) NOT DUMPED!
???????: Icelandic (code page 861, characters 0x8B-0x8D, 0x95, 0x97-0x98, 0x9B, 0x9D, 0xA4-0xA7 differ from cp437) NOT DUMPED!
4733197: Danish/Norwegian (code page 865, characters 0x9B and 0x9D differ from cp437)
???????: Hebrew (code page 862) NOT DUMPED!

=========================
IBM Roms thanks to Frode
=========================

1504036.bin: IBM 4860 PC/jr BIOS. visible in memory at F0000-F7FFF.
1504037.bin: IBM 4860 PC/jr BIOS. visible in memory at F8000-FFFFF.

  ROM_LOAD( "1504036.bin", 0xf0000, 0x8000, CRC(de8fa668) SHA1(459341e033be1199c107e56d33680170e144b689))
  ROM_LOAD( "1504037.bin", 0xf8000, 0x8000, CRC(04c05f17) SHA1(319423cb6bb02b399ecf6e0cb82015c16ada68f5))

5601JDA.bin: IBM 5511 PC/JX BIOS. visible in memory at F0000-FFFFF.
  ROM_LOAD( "5601jda.bin", 0xf0000, 0x10000, CRC(b1e12366) SHA1(751feb16b985aa4f1ec1437493ff77e2ebd5e6a6))

7396917.bin: IBM 5140 PC/Convertible BIOS. visible in memory at F0000-F7FFF.
7396918.bin: IBM 5140 PC/Convertible BIOS. visible in memory at F8000-FFFFF.
  ROM_LOAD( "7396917.bin", 0xf0000, 0x8000, CRC(95c35652) SHA1(2bdac30715dba114fbe0895b8b4723f8dc26a90d))
  ROM_LOAD( "7396918.bin", 0xf8000, 0x8000, CRC(1b4202b0) SHA1(4797ff853ba1675860f293b6368832d05e2f3ea9))

5700019.bin: IBM 5150 PC BASIC 1.0. visible in memory at F6000-F7FFF.
5700027.bin: IBM 5150 PC BASIC 1.0. visible in memory at F8000-F9FFF.
5700035.bin: IBM 5150 PC BASIC 1.0. visible in memory at FA000-FBFFF.
5700043.bin: IBM 5150 PC BASIC 1.0. visible in memory at FC000-FDFFF.
  ROM_LOAD( "5700019.bin", 0xf6000, 0x2000, CRC(b59e8f6c) SHA1(7a5db95370194c73b7921f2d69267268c69d2511))
  ROM_LOAD( "5700027.bin", 0xf8000, 0x2000, CRC(bfff99b8) SHA1(ca2f126ba69c1613b7b5a4137d8d8cf1db36a8e6))
  ROM_LOAD( "5700035.bin", 0xfa000, 0x2000, CRC(9fe4ec11) SHA1(89af8138185938c3da3386f97d3b0549a51de5ef))
  ROM_LOAD( "5700043.bin", 0xfc000, 0x2000, CRC(ea2794e6) SHA1(22fe58bc853ffd393d5e2f98defda7456924b04f))


5700051.bin: First early IBM 5150 PC BIOS. visible in memory at FE000-FFFFF.
  ROM_LOAD( "5700051.bin", 0xfe000, 0x2000, CRC(12d33fb8) SHA1(f046058faa016ad13aed5a082a45b21dea43d346))
5700671.bin: Second early IBM 5150 PC BIOS. visible in memory at FE000-FFFFF.
  ROM_LOAD( "5700671.bin", 0xfe000, 0x2000, CRC(b7d4ec46) SHA1(bdb06f846c4768f39eeff7e16b6dbff8cd2117d2))

5000019.bin: IBM 5150 PC BASIC 1.1. visible in memory at F6000-F7FFF.
5000021.bin: IBM 5150 PC BASIC 1.1. visible in memory at F8000-F9FFF.
5000022.bin: IBM 5150 PC BASIC 1.1. visible in memory at FA000-FBFFF.
5000023.bin: IBM 5150 PC BASIC 1.1. visible in memory at FC000-FDFFF.
  ROM_LOAD( "5000019.bin", 0xf6000, 0x2000, CRC(80d3cf5d) SHA1(64769b7a8b60ffeefa04e4afbec778069a2840c9))
  ROM_LOAD( "5000021.bin", 0xf8000, 0x2000, CRC(673a4acc) SHA1(082ae803994048e225150f771794ca305f73d731))
  ROM_LOAD( "5000022.bin", 0xfa000, 0x2000, CRC(aac3fc37) SHA1(c9e0529470edf04da093bb8c8ae2536c688c1a74))
  ROM_LOAD( "5000023.bin", 0xfc000, 0x2000, CRC(3062b3fc) SHA1(5134dd64721cbf093d059ee5d3fd09c7f86604c7))


5700476.0.bin: Late IBM 5150 PC BIOS. visible in memory at FE000-FFFFF. (1981 copyright)
  ROM_LOAD( "1501476.0.bin", 0xfe000, 0x2000, CRC(9b791d3e) SHA1(0c93f07e62cd27688f7f473e9787ef5308535fa0))
5700476.1.bin: Late IBM 5150 PC BIOS. visible in memory at FE000-FFFFF. (1982 copyright)
  ROM_LOAD( "1501476.1.bin", 0xfe000, 0x2000, CRC(e88792b3) SHA1(40fce6a94dda4328a8b608c7ae2f39d1dc688af4))

5000027.bin: Early IBM 5160 PC/XT BIOS/BASIC 1.1. visible in memory at F6000-F7FFF.
5000026.bin: Prototype IBM 5160 PC/XT BIOS/BASIC 1.1. visible in memory at F8000-FFFFF.
  ROM_LOAD( "5000027.bin", 0xf6000, 0x2000, CRC(80d3cf5d) SHA1(64769b7a8b60ffeefa04e4afbec778069a2840c9))
  ROM_LOAD( "5000026.bin", 0xf8000, 0x8000, CRC(3c9b0ac3) SHA1(271c9f4cef5029a1560075550b67c3395db09fef))

6359116.bin: Early IBM 5160 PC/XT BIOS/BASIC 1.1. visible in memory at F6000-F7FFF.
  ROM_LOAD( "6359116.bin", 0xf6000, 0x2000, CRC(80d3cf5d) SHA1(64769b7a8b60ffeefa04e4afbec778069a2840c9))

1501512.bin: Early IBM 5160 PC/XT BIOS/BASIC 1.1. visible in memory at F8000-FFFFF.
  ROM_LOAD( "1501512.bin", 0xf8000, 0x8000, CRC(79522c3d) SHA1(6bac726d8d033491d52507278aa388ec04cf8b7e))

62x0854.bin: First late IBM 5160 PC/XT BIOS/BASIC 1.1. visible in memory at F0000-F7FFF. (PROM)
62x0851.bin: First late IBM 5160 PC/XT BIOS/BASIC 1.1. visible in memory at F8000-FFFFF. (PROM)
  ROM_LOAD( "62x0854.bin", 0xf0000, 0x8000, CRC(b5fb0e83) SHA1(937b43759ffd472da4fb0fe775b3842f5fb4c3b3))
  ROM_LOAD( "62x0851.bin", 0xf8000, 0x8000, CRC(1054f7bd) SHA1(e7d0155813e4c650085144327581f05486ed1484))

62x0853.bin: First late IBM 5160 PC/XT BIOS/BASIC 1.1. visible in memory at F0000-F7FFF. (EPROM)
62x0852.bin: First late IBM 5160 PC/XT BIOS/BASIC 1.1. visible in memory at F8000-FFFFF. (EPROM)
  ROM_LOAD( "62x0853.bin", 0xf0000, 0x8000, CRC(b5fb0e83) SHA1(937b43759ffd472da4fb0fe775b3842f5fb4c3b3))
  ROM_LOAD( "62x0852.bin", 0xf8000, 0x8000, CRC(1054f7bd) SHA1(e7d0155813e4c650085144327581f05486ed1484))

68x4370.bin: Second late IBM 5160 PC/XT BIOS/BASIC 1.1. visible in memory at F0000-F7FFF. (PROM)
62x0890.bin: Second late IBM 5160 PC/XT BIOS/BASIC 1.1. visible in memory at F8000-FFFFF. (PROM)
  ROM_LOAD( "68x4370.bin", 0xf0000, 0x8000, CRC(758ff036) SHA1(045e27a70407d89b7956ecae4d275bd2f6b0f8e2))
  ROM_LOAD( "62x0890.bin", 0xf8000, 0x8000, CRC(4f417635) SHA1(daa61762d3afdd7262e34edf1a3d2df9a05bcebb))

62x0819.bin: Second late IBM 5160 PC/XT BIOS/BASIC 1.1. visible in memory at F0000-F7FFF. (EPROM)
59x7268.bin: Second late IBM 5160 PC/XT BIOS/BASIC 1.1. visible in memory at F8000-FFFFF. (EPROM)
  ROM_LOAD( "62x0819.bin", 0xf0000, 0x8000, CRC(758ff036) SHA1(045e27a70407d89b7956ecae4d275bd2f6b0f8e2))
  ROM_LOAD( "59x7268.bin", 0xf8000, 0x8000, CRC(4f417635) SHA1(daa61762d3afdd7262e34edf1a3d2df9a05bcebb))

78x7460.bin: IBM 5162 PC/XT 286 BIOS. visible in even memory at F0000-FFFFF (mirror at E0000-EFFFF).
78x7461.bin: IBM 5162 PC/XT 286 BIOS. visible in odd memory at F0000-FFFFF (mirror at E0000-EFFFF).
  ROM_LOAD16_BYTE( "78x7460.bin", 0xf0000, 0x8000, CRC(1db4bd8f) SHA1(7be669fbb998d8b4626fefa7cd1208d3b2a88c31))
  ROM_LOAD16_BYTE( "78x7461.bin", 0xf0001, 0x8000, CRC(be14b453) SHA1(ec7c10087dbd53f9c6d1174e8f14212e2aec1818))

6181028.bin: First 6MHz IBM 5170 PC/AT BIOS. visible in even memory at F0000-FFFFF.
6181029.bin: First 6MHz IBM 5170 PC/AT BIOS. visible in odd memory at F0000-FFFFF.
  ROM_LOAD16_BYTE( "6181028.bin", 0xf0000, 0x8000, CRC(f6573f2a) SHA1(3e52cfa6a6a62b4e8576f4fe076c858c220e6c1a))
  ROM_LOAD16_BYTE( "6181029.bin", 0xf0001, 0x8000, CRC(7075fbb2) SHA1(a7b885cfd38710c9bc509da1e3ba9b543a2760be))

6480090.bin: Second 6MHz IBM 5170 PC/AT BIOS. visible in even memory at F0000-FFFFF.
6480091.bin: Second 6MHz IBM 5170 PC/AT BIOS. visible in odd memory at F0000-FFFFF.
  ROM_LOAD16_BYTE( "6480090.bin", 0xf0000, 0x8000, CRC(99703aa9) SHA1(18022e93a0412c8477e58f8c61a87718a0b9ab0e))
  ROM_LOAD16_BYTE( "6480091.bin", 0xf0001, 0x8000, CRC(013ef44b) SHA1(bfa15d2180a1902cb6d38c6eed3740f5617afd16))

62x0820.bin: 8MHz IBM 5170 PC/AT BIOS. visible in even memory at F0000-FFFFF. (PROM)
62x0821.bin: 8MHz IBM 5170 PC/AT BIOS. visible in odd memory at F0000-FFFFF. (PROM)
  ROM_LOAD( "62x0820.bin", 0xf0000, 0x8000, CRC(e9cc3761) SHA1(ff9373c1a1f34a32fb6acdabc189c61b01acf9aa))
  ROM_LOAD( "62x0821.bin", 0xf0001, 0x8000, CRC(b5978ccb) SHA1(2a1aeb9ae3cd7e60fc4c383ca026208b82156810))

61x9266.bin: 8MHz IBM 5170 PC/AT BIOS. visible in even memory at F0000-FFFFF. (EPROM)
61x9265.bin: 8MHz IBM 5170 PC/AT BIOS. visible in odd memory at F0000-FFFFF. (EPROM)
  ROM_LOAD( "61x9265.bin", 0xf0001, 0x8000, CRC(c32713e4) SHA1(22ed4e2be9f948682891e2fd056a97dbea01203c))
  ROM_LOAD( "61x9266.bin", 0xf0000, 0x8000, CRC(4995be7a) SHA1(8e8e5c863ae3b8c55fd394e345d8cca48b6e575c))


5788005.bin: IBM MDA/CGA font. Not mapped in PC memory. (American manufacture, otherwise similar to the European manufacture)
  ROM_LOAD( "5788005.bin", 0x0000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f))

6359300.bin: IBM MDA/CGA font. Not mapped in PC memory. (European manufacture, otherwise similar to the American manufacture)
  ROM_LOAD( "6359300.bin", 0x0000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f))

4733197.bin: IBM MDA/CGA Alternative font. Not mapped in PC memory.
  ROM_LOAD( "4733197.bin", 0x0000, 0x2000, CRC(650c0f85) SHA1(0c8ac77248a9856d065b1c64658e712ea55b7507))

6277356.bin: IBM EGA ROM. Visible in memory at C3FFF-C0000.
  ROM_LOAD( "6277356.bin", 0x0000, 0x4000, CRC(dc146448) SHA1(dc0794499b3e499c5777b3aa39554bbf0f2cc19b))

8222554.bin: IBM VGA ROM. Visible in memory at C0000-C5FFF.
  ROM_LOAD( "8222554.bin", 0x0000, 0x6000, CRC(6c12d745) SHA1(c0156607100e377c6c9563f23967409dab3ab92b))

5000059.bin: First version Hard drive controller ROM. Visible in memory at C8000-C9FFF.
  ROM_LOAD( "5000059.bin", 0xc8000, 0x2000, CRC(03e0ee9a) SHA1(6691be4f6a8d690c696ad8b259708d3e7e87ad89))

6359121.bin: First version Hard drive controller ROM. Visible in memory at C8000-C9FFF.
  ROM_LOAD( "6359121.bin", 0xc8000, 0x2000, CRC(03e0ee9a) SHA1(6691be4f6a8d690c696ad8b259708d3e7e87ad89))

59x7291.bin: Second version Hard drive controller ROM. Visible in memory at C8000-C8FFF.
  ROM_LOAD( "59x7291.bin", 0xc8000, 0x1000, CRC(25920437) SHA1(de970bcc5c6f1b588fbc4c76617165ce8eb2bf1d))

104839e.bin: Hard drive controller Z80 firmware ROM. Not mapped in PC memory. (Mapped in Z80 microcontroller memory at 0000-7FFF)
  ROM_LOAD( "104839e.bin", 0x0000, 0x1000, CRC(3ad32fcc) SHA1(0127fa520aaee91285cb46a640ed835b4554e4b3))

6323581.bin: 3270 Keyboard adapter ROM. The first 0x800 bytes visible in memory at C0000-C07FF. The later 0x1800 bytes visible in memory at CA000-CB7FF.
  ROM_LOAD( "6323581.bin", 0xc0000, 0x2000, CRC(cf323cbd) SHA1(93c1ef2ede02772a46dab075c32e179faa045f81))

1504161.bin: 3270 Character ROM (pixels 0-7). Not mapped in PC memory.
1504162.bin: 3270 Character ROM (pixel 8). Not mapped in PC memory.
  ROM_LOAD( "1504161.bin", 0x0000, 0x2000, CRC(d9246cf5) SHA1(2eaed495893a4e6649b04d10dada7b5ef4abd140))
  ROM_LOAD( "1504162.bin", 0x2000, 0x2000, CRC(59e1dc32) SHA1(337b5cced203345a5acfb02532d6b5f526902ee7))

6137323.bin: Professional Graphics Controller, first half of x86 firmware. Not mapped in PC memory. (Mapped in 8088-2 microcontroller memory at 00000-07FFF)
6137322.bin: Professional Graphics Controller, second half of x86 firmware. Not mapped in PC memory. (Mapped in 8088-2 microcontroller memory at 08000-0FFFF and any address within (8-F)(0-F)(3/7/B/F)00-(8-F)(0-F)(3/7/B/F)FF)
___MISSING___ 6137560.bin: Professional Graphics Controller, CGA emulation font. Not mapped in PC memory.
  ROM_LOAD( "6137322.bin", 0x8000, 0x8000, CRC(5e6cc82f) SHA1(45b3ffb5a9c51986862f8d47b3e03dcaaf4073d5))
  ROM_LOAD( "6137323.bin", 0x0000, 0x8000, CRC(f564f342) SHA1(c5ef17fd1569043cb59f61faf828ea8b0ee95526))


XC215 C 0.bin: IBM Music feature card Z80 firmware ROM. Not mapped in PC memory. (Mapped in Z80 microcontroller memory at 0000-7FFF)
  ROM_LOAD( "xc215 c 0.bin", 0x0000, 0x8000, CRC(28c58a4f) SHA1(e7edf28d20e6c146e3144526c89cd6beea64663b))

XT U44 IBM.bin: IBM 5160 PC/XT Bank-selection decoding ROM (256x4 bit). Not mapped in PC memory.
  ROM_LOAD( "xt u44 ibm.bin",0x0000, 0x0080, CRC(60ebb6e5) SHA1(e96a83b2fd6a231235374272a99353ab362b8e37))

1503033.bin: IBM 5170 PC/AT keyboard controller i8042 firmware. Not mapped in PC memory.
  ROM_LOAD( "1503033.bin", 0x0000, 0x0800, CRC(5a81c0d2) SHA1(0100f8789fb4de74706ae7f9473a12ec2b9bd729))

1503099.bin: IBM AT 84-key Keyboard firmware. Not mapped in PC memory.
  ROM_LOAD( "1503099.bin", 0x0000, 0x0400, CRC(1e921f37) SHA1(5f722bdb3b57f5a532c02a5c3f78f30d785796f2))

1385001.bin: IBM Terminal 122-key Keyboard firmware. Not mapped in PC memory.
  ROM_LOAD( "1385001.bin", 0x0000, 0x0400, CRC(c19767e9) SHA1(a3701e4617383a4de0fd5e2e86c4b74beaf94a7b))

30F9580.bin: IBM PS/2 model 30 286 even.
30F9579.bin: IBM PS/2 model 30 286 odd.
  ROM_LOAD( "30f9579.bin", 0x0000, 0x10000, CRC(1448d3cb) SHA1(13fa26d895ce084278cd5ab1208fc16c80115ebe))
  ROM_LOAD( "30f9580.bin", 0x0000, 0x10000, CRC(9965a634) SHA1(c237b1760f8a4561ec47dc70fe2e9df664e56596))

90X7415.bin: IBM PS/2 model 25/30 external FDD support adapter. visible in memory at C8000-C9FFF.
  ROM_LOAD( "90x7415.bin", 0x0000, 0x2000, CRC(02d28556) SHA1(5543a8634f90a9141cf95f6a13c71be7778ee2a1))


***************************************************************************/


#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/ram.h"
#include "bus/isa/isa.h"
#include "bus/isa/isa_cards.h"
#include "machine/pc_lpt.h"
#include "bus/pc_kbd/keyboards.h"
#include "includes/genpc.h"
#include "softlist.h"

class ibmpc_state : public driver_device
{
public:
	ibmpc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_maincpu(*this, "maincpu") { }

	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START( pc8_map, AS_PROGRAM, 8, ibmpc_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x9ffff) AM_RAMBANK("bank10")
	AM_RANGE(0xa0000, 0xbffff) AM_NOP
	AM_RANGE(0xc0000, 0xc7fff) AM_ROM
	AM_RANGE(0xc8000, 0xcffff) AM_ROM
	AM_RANGE(0xd0000, 0xeffff) AM_NOP
	AM_RANGE(0xf0000, 0xfffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(pc8_io, AS_IO, 8, ibmpc_state )
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static INPUT_PORTS_START( ibm5150 )
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START(cga)
	DEVICE_INPUT_DEFAULTS("DSW0",0x30, 0x20)
DEVICE_INPUT_DEFAULTS_END

//static DEVICE_INPUT_DEFAULTS_START(ega)
//  DEVICE_INPUT_DEFAULTS("DSW0",0x30, 0x00)
//DEVICE_INPUT_DEFAULTS_END


static MACHINE_CONFIG_START( ibm5150, ibmpc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8088, XTAL_14_31818MHz/3)
	MCFG_CPU_PROGRAM_MAP(pc8_map)
	MCFG_CPU_IO_MAP(pc8_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

	MCFG_IBM5150_MOTHERBOARD_ADD("mb","maincpu")
	MCFG_DEVICE_INPUT_DEFAULTS(cga)

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, "cga", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, "com", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, "fdc_xt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, "hdc", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa5", pc_isa8_cards, nullptr, false)

	/* keyboard */
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_KEYTRONIC_PC3270)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("disk_list","ibm5150")
	MCFG_SOFTWARE_LIST_ADD("cass_list","ibm5150_cass")
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( ibm5140, ibm5150 )
	/* software lists */
	MCFG_DEVICE_REMOVE( "disk_list" )
	MCFG_SOFTWARE_LIST_ADD("disk_list","ibm5140")
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( ibm5160, ibmpc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8088, XTAL_14_31818MHz/3)
	MCFG_CPU_PROGRAM_MAP(pc8_map)
	MCFG_CPU_IO_MAP(pc8_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("mb:pic8259", pic8259_device, inta_cb)

	MCFG_IBM5160_MOTHERBOARD_ADD("mb","maincpu")
	MCFG_DEVICE_INPUT_DEFAULTS(cga)

	MCFG_ISA8_SLOT_ADD("mb:isa", "isa1", pc_isa8_cards, "cga", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa2", pc_isa8_cards, "com", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa3", pc_isa8_cards, "fdc_xt", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa4", pc_isa8_cards, "hdc", false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa5", pc_isa8_cards, nullptr, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa6", pc_isa8_cards, nullptr, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa7", pc_isa8_cards, nullptr, false)
	MCFG_ISA8_SLOT_ADD("mb:isa", "isa8", pc_isa8_cards, nullptr, false)

	/* keyboard */
	MCFG_PC_KBDC_SLOT_ADD("mb:pc_kbdc", "kbd", pc_xt_keyboards, STR_KBD_IBM_PC_XT_83)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("640K")

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("pc_disk_list","ibm5150")
	MCFG_SOFTWARE_LIST_ADD("xt_disk_list","ibm5160_flop")
MACHINE_CONFIG_END


ROM_START( ibm5150 )
	ROM_REGION(0x100000,"maincpu", 0)
//  ROM_LOAD("600963.u12", 0xc8000, 0x02000, CRC(f3daf85f) SHA1(3bd29538832d3084cbddeec92593988772755283))  /* Tandon/Western Digital Fixed Disk Adapter 600963-001__TYPE_5.U12.2764.bin - Meant for an IBM PC or XT which lacked bios support for HDDs */

	/* Xebec 1210 and 1220 Z80-based ST409/ST412 MFM controllers */
//  ROM_LOAD("5000059.12d", 0xc8000, 0x02000, CRC(03e0ee9a) SHA1(6691be4f6a8d690c696ad8b259708d3e7e87ad89)) /* Xebec 1210 IBM OEM Fixed Disk Adapter - Revision 1, supplied with rev1 and rev2 XT, and PC/3270. supports 4 hdd types, selectable by jumpers (which on the older XTs were usually soldered to one setting) */
//  ROM_LOAD("62x0822.12d", 0xc8000, 0x02000, CRC(4cdd2193) SHA1(fe8f88333b5e13e170bf637a9a0090383dee454d)) /* Xebec 1210 IBM OEM Fixed Disk Adapter 62X0822__(M)_AMI_8621MAB__S68B364-P__(C)IBM_CORP_1982,1985__PHILIPPINES.12D.2364.bin - Revision 2, supplied with rev3 and rev4 XT. supports 4 hdd types, selectable by jumpers (which unlike the older XTs, were changeable without requiring soldering )*/
//  ROM_LOAD("unknown.12d", 0xc8000, 0x02000, NO_DUMP ) /* Xebec 1210 Non-IBM/Retail Fixed Disk Adapter - supports 4 hdd types, selectable by jumpers (changeable without soldering) */
//  ROM_LOAD("unknown.???", 0xc8000, 0x02000, NO_DUMP ) /* Xebec 1220 Non-IBM/Retail Fixed Disk Adapter plus Floppy Disk Adapter - supports 4 hdd types, selectable by jumpers (changeable without soldering) */

	/* IBM PC 5150 (rev 3: 1501-476 10/27/82) 5-screw case 64-256k MB w/1501981 CGA Card, ROM Basic 1.1 */
	ROM_DEFAULT_BIOS( "rev3" )

	ROM_SYSTEM_BIOS( 0, "rev3", "IBM PC 5150 1501476 10/27/82" )
	ROMX_LOAD("5000019.u29", 0xf6000, 0x2000, CRC(80d3cf5d) SHA1(64769b7a8b60ffeefa04e4afbec778069a2840c9), ROM_BIOS(1))        /* ROM Basic 1.1 F6000-F7FFF; IBM P/N: 5000019, FRU: 6359109 */
	ROMX_LOAD("5000021.u30", 0xf8000, 0x2000, CRC(673a4acc) SHA1(082ae803994048e225150f771794ca305f73d731), ROM_BIOS(1))        /* ROM Basic 1.1 F8000-F9FFF; IBM P/N: 5000021, FRU: 6359111 */
	ROMX_LOAD("5000022.u31", 0xfa000, 0x2000, CRC(aac3fc37) SHA1(c9e0529470edf04da093bb8c8ae2536c688c1a74), ROM_BIOS(1))        /* ROM Basic 1.1 FA000-FBFFF; IBM P/N: 5000022, FRU: 6359112 */
	ROMX_LOAD("5000023.u32", 0xfc000, 0x2000, CRC(3062b3fc) SHA1(5134dd64721cbf093d059ee5d3fd09c7f86604c7), ROM_BIOS(1))        /* ROM Basic 1.1 FC000-FDFFF; IBM P/N: 5000023, FRU: 6359113 */
	ROMX_LOAD("1501476.u33", 0xfe000, 0x2000, CRC(e88792b3) SHA1(40fce6a94dda4328a8b608c7ae2f39d1dc688af4), ROM_BIOS(1))

	/* IBM PC 5150 (rev 1: 04/24/81) 2-screw case 16-64k MB w/MDA Card, ROM Basic 1.0 */
	ROM_SYSTEM_BIOS( 1, "rev1", "IBM PC 5150 5700051 04/24/81" )
	ROMX_LOAD("5700019.u29", 0xf6000, 0x2000, CRC(b59e8f6c) SHA1(7a5db95370194c73b7921f2d69267268c69d2511), ROM_BIOS(2))        /* ROM Basic 1.0 F6000-F7FFF */
	ROMX_LOAD("5700027.u30", 0xf8000, 0x2000, CRC(bfff99b8) SHA1(ca2f126ba69c1613b7b5a4137d8d8cf1db36a8e6), ROM_BIOS(2))        /* ROM Basic 1.0 F8000-F9FFF */
	ROMX_LOAD("5700035.u31", 0xfa000, 0x2000, CRC(9fe4ec11) SHA1(89af8138185938c3da3386f97d3b0549a51de5ef), ROM_BIOS(2))        /* ROM Basic 1.0 FA000-FBFFF */
	ROMX_LOAD("5700043.u32", 0xfc000, 0x2000, CRC(ea2794e6) SHA1(22fe58bc853ffd393d5e2f98defda7456924b04f), ROM_BIOS(2))        /* ROM Basic 1.0 FC000-FDFFF */
	ROMX_LOAD("5700051.u33", 0xfe000, 0x2000, CRC(12d33fb8) SHA1(f046058faa016ad13aed5a082a45b21dea43d346), ROM_BIOS(2))

	/* IBM PC 5150 (rev 2: 10/19/81) 2-screw case, 16-64k MB w/MDA Card, ROM Basic 1.0 */
	ROM_SYSTEM_BIOS( 2, "rev2", "IBM PC 5150 5700671 10/19/81" )
	ROMX_LOAD("5700019.u29", 0xf6000, 0x2000, CRC(b59e8f6c) SHA1(7a5db95370194c73b7921f2d69267268c69d2511), ROM_BIOS(3))        /* ROM Basic 1.0 F6000-F7FFF */
	ROMX_LOAD("5700027.u30", 0xf8000, 0x2000, CRC(bfff99b8) SHA1(ca2f126ba69c1613b7b5a4137d8d8cf1db36a8e6), ROM_BIOS(3))        /* ROM Basic 1.0 F8000-F9FFF */
	ROMX_LOAD("5700035.u31", 0xfa000, 0x2000, CRC(9fe4ec11) SHA1(89af8138185938c3da3386f97d3b0549a51de5ef), ROM_BIOS(3))        /* ROM Basic 1.0 FA000-FBFFF */
	ROMX_LOAD("5700043.u32", 0xfc000, 0x2000, CRC(ea2794e6) SHA1(22fe58bc853ffd393d5e2f98defda7456924b04f), ROM_BIOS(3))        /* ROM Basic 1.0 FC000-FDFFF */
	ROMX_LOAD("5700671.u33", 0xfe000, 0x2000, CRC(b7d4ec46) SHA1(bdb06f846c4768f39eeff7e16b6dbff8cd2117d2), ROM_BIOS(3))

	/* Z80 on the Xebec 1210 and 1220 Hard Disk Controllers */
//  ROM_REGION(0x10000, "cpu1", 0)
//  ROM_LOAD("104839re.12a", 0x0000, 0x1000, CRC(3ad32fcc) SHA1(0127fa520aaee91285cb46a640ed835b4554e4b3))  /* Xebec 1210 IBM OEM Hard Disk Controller, silkscreened "104839RE // COPYRIGHT // XEBEC 1986" - Common for both XEBEC 1210 IBM OEM revisions. Some cards have the rom marked 104839E instead (John Eliott's card is like this), but contents are the same. */
//  /* Other versions probably exist for the non-IBM/Retail 1210 and the 1220 */

	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END
ROM_START( ibm5155 )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD("5000027.u19", 0xf0000, 0x8000, CRC(fc982309) SHA1(2aa781a698a21c332398d9bc8503d4f580df0a05))
	ROM_LOAD("1501512.u18", 0xf8000, 0x8000, CRC(79522c3d) SHA1(6bac726d8d033491d52507278aa388ec04cf8b7e))
	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END
ROM_START( ibm5140 )
	ROM_REGION(0x100000,"maincpu", 0)
	ROM_LOAD("7396917.bin", 0xf0000, 0x08000, CRC(95c35652) SHA1(2bdac30715dba114fbe0895b8b4723f8dc26a90d))
	ROM_LOAD("7396918.bin", 0xf8000, 0x08000, CRC(1b4202b0) SHA1(4797ff853ba1675860f293b6368832d05e2f3ea9))
	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END
#ifdef UNUSED_DEFINITION
ROM_START( ibmpca )
	ROM_REGION(0x100000,"maincpu",0)
	ROM_LOAD("basicc11.f6", 0xf6000, 0x2000, CRC(80d3cf5d) SHA1(64769b7a8b60ffeefa04e4afbec778069a2840c9))
	ROM_LOAD("basicc11.f8", 0xf8000, 0x2000, CRC(673a4acc) SHA1(082ae803994048e225150f771794ca305f73d731))
	ROM_LOAD("basicc11.fa", 0xfa000, 0x2000, CRC(aac3fc37) SHA1(c9e0529470edf04da093bb8c8ae2536c688c1a74))
	ROM_LOAD("basicc11.fc", 0xfc000, 0x2000, CRC(3062b3fc) SHA1(5134dd64721cbf093d059ee5d3fd09c7f86604c7))
	ROM_LOAD("pc081682.bin", 0xfe000, 0x2000, CRC(5c3f0256) SHA1(b42c78abd0a9c630a2f972ad2bae46d83c3a2a09))

	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END
#endif


ROM_START( ibm5160 )
	ROM_REGION16_LE(0x100000,"maincpu", 0)
//  ROM_LOAD("600963.u12", 0xc8000, 0x02000, CRC(f3daf85f) SHA1(3bd29538832d3084cbddeec92593988772755283))  /* Tandon/Western Digital Fixed Disk Adapter 600963-001__TYPE_5.U12.2764.bin */

	/* PC/3270 has a 3270 keyboard controller card, plus a rom on that card to tell the pc how to run it.
	    * Unlike the much more complex keyboard controller used in the AT/3270, this one only has one rom,
	      a motorola made "(M)1503828 // XE // 8434A XM // SC81155P" custom (an MCU?; the more complicated
	      3270/AT keyboard card uses this same exact chip), an 8254, and some logic chips.
	      Thanks to high resolution pictures provided by John Elliott, I can see that the location of the
	              chips is unlabeled (except for by absolute pin position on the back), and there are no pals or proms.
	    * The board is stickered "2683114 // 874999 // 8446 SU" on the front.
	    * The board has a single DE-9 connector where the keyboard dongle connects to.
	    * The keyboard dongle has two connectors on it: a DIN-5 connector which connects to the Motherboard's
	      keyboard port, plus an RJ45-lookalike socket which the 3270 keyboard connects to.
	    * The rom is mapped very strangely to avoid hitting the hard disk controller:
	      The first 0x800 bytes appear at C0000-C07FF, and the last 0x1800 bytes appear at 0xCA000-CB7FF
	*/
//  ROM_LOAD("6323581.bin", 0xc0000, 0x00800, CRC(cf323cbd) SHA1(93c1ef2ede02772a46dab075c32e179faa045f81))
//  ROM_LOAD("6323581.bin", 0xca000, 0x01800, CRC(cf323cbd) SHA1(93c1ef2ede02772a46dab075c32e179faa045f81) ROM_SKIP(0x800))

	/* Xebec 1210 and 1220 Z80-based ST409/ST412 MFM controllers */
//  ROM_LOAD("5000059.12d", 0xc8000, 0x02000, CRC(03e0ee9a) SHA1(6691be4f6a8d690c696ad8b259708d3e7e87ad89)) /* Xebec 1210 IBM OEM Fixed Disk Adapter - Revision 1, supplied with rev1 and rev2 XT, and PC/3270. supports 4 hdd types, selectable by jumpers (which on the older XTs were usually soldered to one setting) */
//  ROM_LOAD("62x0822.12d", 0xc8000, 0x02000, CRC(4cdd2193) SHA1(fe8f88333b5e13e170bf637a9a0090383dee454d)) /* Xebec 1210 IBM OEM Fixed Disk Adapter 62X0822__(M)_AMI_8621MAB__S68B364-P__(C)IBM_CORP_1982,1985__PHILIPPINES.12D.2364.bin - Revision 2, supplied with rev3 and rev4 XT. supports 4 hdd types, selectable by jumpers (which unlike the older XTs, were changeable without requiring soldering )*/
//  ROM_LOAD("unknown.12d", 0xc8000, 0x02000, NO_DUMP ) /* Xebec 1210 Non-IBM/Retail Fixed Disk Adapter - supports 4 hdd types, selectable by jumpers (changeable without soldering) */
//  ROM_LOAD("unknown.???", 0xc8000, 0x02000, NO_DUMP ) /* Xebec 1220 Non-IBM/Retail Fixed Disk Adapter plus Floppy Disk Adapter - supports 4 hdd types, selectable by jumpers (changeable without soldering) */


	ROM_DEFAULT_BIOS( "rev4" )

	ROM_SYSTEM_BIOS( 0, "rev1", "IBM XT 5160 08/16/82" )    /* ROM at u18 marked as BAD_DUMP for now, as current dump, while likely correct, was regenerated from a number of smaller dumps, and needs a proper redump. */
	ROMX_LOAD("5000027.u19", 0xf0000, 0x8000, CRC(fc982309) SHA1(2aa781a698a21c332398d9bc8503d4f580df0a05), ROM_BIOS(1) )
	ROMX_LOAD("5000026.u18", 0xf8000, 0x8000, BAD_DUMP CRC(3c9b0ac3) SHA1(271c9f4cef5029a1560075550b67c3395db09fef), ROM_BIOS(1) ) /* This is probably a good dump, and works fine, but as it was manually regenerated based on a partial dump, it needs to be reverified. It's a very rare rom revision and may have only appeared on XT prototypes. */

	ROM_SYSTEM_BIOS( 1, "rev2", "IBM XT 5160 11/08/82" )    /* Same as PC 5155 BIOS and PC/3270 BIOS */
	ROMX_LOAD("5000027.u19", 0xf0000, 0x8000, CRC(fc982309) SHA1(2aa781a698a21c332398d9bc8503d4f580df0a05), ROM_BIOS(2) ) /* silkscreen "MK37050N-4 // 5000027" - FRU: 6359116 - Contents repeat 4 times; Alt Silkscreen (from yesterpc.com): "(M) // 5000027 // (C) 1983 IBM CORP // X E // 8425B NM"*/
	ROMX_LOAD("1501512.u18", 0xf8000, 0x8000, CRC(79522c3d) SHA1(6bac726d8d033491d52507278aa388ec04cf8b7e), ROM_BIOS(2) ) /* silkscreen "MK38036N-25 // 1501512 // ZA // (C)IBM CORP // 1981,1983 // D MALAYSIA // 8438 AP"*/

	ROM_SYSTEM_BIOS( 2, "rev3", "IBM XT 5160 01/10/86" )    /* Has enhanced keyboard support and a 3.5" drive */
	ROMX_LOAD("62x0854.u19", 0xf0000, 0x8000, CRC(b5fb0e83) SHA1(937b43759ffd472da4fb0fe775b3842f5fb4c3b3), ROM_BIOS(3) )
	ROMX_LOAD("62x0851.u18", 0xf8000, 0x8000, CRC(1054f7bd) SHA1(e7d0155813e4c650085144327581f05486ed1484), ROM_BIOS(3) )

	ROM_SYSTEM_BIOS( 3, "rev4", "IBM XT 5160 05/09/86" )    /* Minor bugfixes to keyboard code, supposedly */
	ROMX_LOAD("68x4370.u19", 0xf0000, 0x8000, CRC(758ff036) SHA1(045e27a70407d89b7956ecae4d275bd2f6b0f8e2), ROM_BIOS(4))
	ROMX_LOAD("62x0890.u18", 0xf8000, 0x8000, CRC(4f417635) SHA1(daa61762d3afdd7262e34edf1a3d2df9a05bcebb), ROM_BIOS(4))

//  ROM_SYSTEM_BIOS( 4, "xtdiag", "IBM XT 5160 w/Supersoft Diagnostics" )    /* ROMs marked as BAD_DUMP for now. We expect the data to be in a different ROM chip layout */
//  ROMX_LOAD("basicc11.f6", 0xf6000, 0x2000, BAD_DUMP CRC(80d3cf5d) SHA1(64769b7a8b60ffeefa04e4afbec778069a2840c9), ROM_BIOS(5) )
//  ROMX_LOAD("basicc11.f8", 0xf8000, 0x2000, BAD_DUMP CRC(673a4acc) SHA1(082ae803994048e225150f771794ca305f73d731), ROM_BIOS(5) )
//  ROMX_LOAD("basicc11.fa", 0xfa000, 0x2000, BAD_DUMP CRC(aac3fc37) SHA1(c9e0529470edf04da093bb8c8ae2536c688c1a74), ROM_BIOS(5) )
//  ROMX_LOAD("basicc11.fc", 0xfc000, 0x2000, BAD_DUMP CRC(3062b3fc) SHA1(5134dd64721cbf093d059ee5d3fd09c7f86604c7), ROM_BIOS(5) )
//  ROMX_LOAD("xtdiag.bin", 0xfe000, 0x2000, CRC(4e89a4d8) SHA1(39a28fb2fe9f1aeea24ed2c0255cebca76e37ed7), ROM_BIOS(5) )

	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */

	/* Z80 ROM on the Xebec 1210 and 1220 Hard Disk Controllers */
//  ROM_REGION(0x10000, "cpu1", 0)
//  ROM_LOAD("104839re.12a", 0x0000, 0x1000, CRC(3ad32fcc) SHA1(0127fa520aaee91285cb46a640ed835b4554e4b3))  /* Xebec 1210 IBM OEM Hard Disk Controller, silkscreened "104839RE // COPYRIGHT // XEBEC 1986" - Common for both XEBEC 1210 IBM OEM revisions. Some cards have the rom marked 104839E instead (John Eliott's card is like this), but contents are the same. */
//  /* Other versions probably exist for the non-IBM/Retail 1210 and the 1220 */


	/* PC/3270 and AT/3270 have a set of two (optionally 3) 3270PGC programmable graphics controller cards on them which
	       have 2 extra roms, plus a number of custom chips and at least one MCU.
	   Information on these three boards plus the keyboard interface can be found at:
	   http://www.seasip.info/VintagePC/5271.html
	    *** The descriptions below are based on the cards from the AT/3270, which are slightly
	        different from the PC/3270 cards. Changes between the PC/3270 and AT/3270 video cards are
	        listed:
	        1. The card lip edge is changed on the APA and PS cards to allow them to be placed
	           in a 16-bit ISA slot.
	        2. The Main Display board is exactly the same PCB (hence cannot be placed in a 16-bit
	           ISA slot), but the socket to the left of the 8255 is now populated, and has a lot of
	           rework wires connected to various places on the PCB.
	        3. The APA board was completely redone, and no longer has an 8254 (though it does have an
	           empty socket) and has ?twice as much memory on it? (not sure about this). The APA
	           board also now connects to both main board connectors 1 and 3, instead of only
	           connector 1.
	        4. The PS board has been minorly redone to allow clearance for a 16-bit ISA connector,
	           but no other significant chip changes were made. The connector 3 still exists on the
	           board but is unpopulated. Connector 2 still connects to the Main display board as
	           before.

	    ** The Main Display Board (with one 48-pin custom, 3 40 pin customs at least one of which is
	       an MCU, four 2016BP-10 srams, an 8254 and an 8255 on it, two crystals (16.257MHz and
	       21.676MHz) plus two mask roms ) is stickered "61X6579 // 983623 // 6390 SU" on the front.
	    *  The pcb is trace-marked "6320987" on both the front and back.
	    *  The card has a DE-9 connector on it for a monitor.
	    *  The customs are marked:
	       "1503192 // TC15G008P-0009 // JAPAN       8549A" (40 pins, at U52)
	       "1503193 // TC15G008AP-0020 // JAPAN       8610A" (48 pins, at U29)
	       "(M)1503194 // XE KGA005 // 8616N XM // SC81156P" (40 pins, at U36, likely an MCU)
	       "S8613 // SCN2672B // C4N40 A // CP3303" (40 pins, at U24, also possibly an MCU)

	    ** The All Points Addressable (Frame buffer?) card (with 2 48-pin customs on it which are
	       probably gate arrays and not MCUs, an empty socket (28 pins, U46), an Intel Id2147H-3,
	       a bank of twelve 16k*4-bit inmos ims2620p-15 DRAMs (tms4416 equivalent), and an Intel
	       D2147K 4096*1 byte SRAM) is stickered
	       "6487836 // A24969 // 6400 SU" on the back.
	    *  The pcb is trace-marked "EC 999040" on the back, and silkscreened "RC 2682819" on the front
	    *  The customs are marked:
	       "6323259 // TC15G008AP-0028 // JAPAN       8606A" (48 pins, at U67)
	       "6323260 // TC15G022AP-0018 // JAPAN       8606A" (48 pins, at U45)

	    ** The optional Programmable Symbol Card (with an AMD AM9128-10PC, and six tms4416-15NL DRAMS,
	       and a fleet of discrete logic chips, but no roms, pals, or proms) is stickered
	       "6347750 // A24866 // 6285 SU" on the front.
	    *  The PCB is trace-marked "PROGAMMABLE SYMBOL P/N 6347751 // ASSY. NO. 6347750" on the front,
	       and trace-marked "|||CIM0286 ECA2466 // 94V-O" on the back.
	*/
//  ROM_REGION(0x4000,"gfx2", 0)
//      ROM_LOAD("1504161.u11", 0x00000, 0x2000, CRC(d9246cf5) SHA1(2eaed495893a4e6649b04d10dada7b5ef4abd140)) /* silkscreen: "AMI 8613MAJ // 9591-041 // S2364B // 1504161 // PHILIPPINES" - Purpose: Pixels 0 through 7 of built-in 3270 terminal font*/
//      ROM_LOAD("1504162.u26", 0x02000, 0x2000, CRC(59e1dc32) SHA1(337b5cced203345a5acfb02532d6b5f526902ee7)) /* silkscreen: "AMI 8607MAH // 9591-042 // S2364B // 1504162 // PHILIPPINES" - Purpose: Pixel 8 of built-in 3270 terminal font*/
ROM_END



/***************************************************************************

  Game driver(s)

***************************************************************************/

/*     YEAR     NAME        PARENT      COMPAT  MACHINE     INPUT       INIT        COMPANY     FULLNAME */
COMP(  1981,    ibm5150,    0,          0,      ibm5150,    ibm5150, driver_device, 0,    "International Business Machines",  "IBM PC 5150" , 0)
COMP(  1982,    ibm5155,    ibm5150,    0,      ibm5150,    ibm5150, driver_device, 0,    "International Business Machines",  "IBM PC 5155" , 0)
COMP(  1985,    ibm5140,    ibm5150,    0,      ibm5140,    ibm5150, driver_device, 0,    "International Business Machines",  "IBM PC 5140 Convertible" , MACHINE_NOT_WORKING)

// xt class (pc but 8086)
COMP(  1982,    ibm5160,    ibm5150,    0,      ibm5160,    ibm5150, driver_device, 0,    "International Business Machines",  "IBM XT 5160" , 0)
