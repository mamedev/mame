// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Tomasz Slanina
/********************************************************************

 dgPIX VRender0 hardware

 Games Supported:
 ---------------------------------------------------------------------------
 - Elfin                               (c) 1999 dgPIX Entertainment Inc.
 - Jump Jump                           (c) 1999 dgPIX Entertainment Inc.
 - The X-Files (2 sets)                (c) 1999 dgPIX Entertainment Inc.
 - King of Dynast Gear (version 1.8)   (c) 1999 EZ Graphics [*]
 - Fishing Maniac 3                    (c) 2002 Saero Entertainment

 [*] the version number is written in the flash roms at the beginning of the game settings

Note: There is known to exist an alternate version of The X-Files titled The Sex Files which is undumped

 Original bugs:
 - In King of Dynast Gear, Roger's fast attack shows some blank lines
   in the sword "shadow" if you do it in the left direction

 Info about the cpu inside KS0164 sound chip:
 - "The 16-bit CPU core was Sequoia's design and was licensed to Samsung.
    It was a 16-bit core with a nearly perfectly orthogonal instruction set.
    You could even multiply the PC by the stack pointer if you wanted."
    AKA Samsung's Omniwave MULTIMEDIA AUDIO


 driver by Pierpaolo Prazzoli & Tomasz Slanina

 - Pierpaolo Prazzoli 2006.05.06
    - added Fishing Maniac 3

 - Pierpaolo Prazzoli 2006.01.16
   - added King of Dynast Gear (protection patched by Tomasz)
   - fixed frame buffer drawing
   - fixed flash roms reading
   - added game settings writing to flash roms
   - added coin counters

 - TS 2005.02.15
   Added double buffering

 - TS 2005.02.06
   Preliminary emulation of X-Files. VRender0- is probably just framebuffer.
   Patch in DRIVER_INIT removes call at RAM adr $8f30 - protection ?
   (without fix, game freezes at one of startup screens - like on real
   board  with  protection PIC removed)

*********************************************************************

PCB Layout
----------

Elfin
The X-Files (Korean region)
Fishing Maniac 3

VRenderO Minus Rev4 dgPIX Entertainment Inc. 1999
|-----------------------------------------------------|
|TDA1515                C-O-N-N-1                     |
|   DA1545A                                       C   |
|                                                 O   |
|  VOL1    K4E151611                  KS0164      N   |
|  VOL2    K4E151611                              N   |
|J                                    169NDK19    3   |
|A     20MHz                           CONN2          |
|M  KA4558                                            |
|M                                                    |
|A                                          KM6161002 |
|          E1-32XT                                    |
|                                           KM6161002 |
|                                                     |
|       ST7705C                             KM6161002 |
| B1             XCS05                                |
| B2 B3          14.31818MHz  LED           KM6161002 |
|-----------------------------------------------------|
Notes:
      ST7705C      - Reset/Watchdog IC (SOIC8)
      E1-32XT      - Hyperstone E1-32XT CPU (QFP144)
      169NDK19     - Xtal, 16.9344MHz
      CONN1,CONN2, - Connectors for joining main board to small sub-board
      CONN3
      XCS05        - Xilinx Spartan XCS05 FPGA (QFP100)
      B1,B2,B3     - Push Buttons for TEST, SERVICE and RESET
      KS0164       - Samsung Electronics KS0164 General Midi compliant 32-voice Wavetable Synthesizer Chip
                     with built-in 16bit CPU and MPU-401 compatibility. (QFP100)
      K4E151611    - Samsung K4E151611C-JC60 1M x16 CMOS EDO DRAM (SOJ44)
      KM6161002    - Samsung KM6161002CJ-12 64k x16 High-Speed CMOS SRAM (SOJ44)



The X-Files (uncensored version)
Jump Jump
King of Dynast Gear

VRenderO Minus Rev5 dgPIX Entertainment Inc. 1999
|-----------------------------------------------------|
|TDA1515                C-O-N-N-1                     |
|   DA1545A                                       C   |
|                                                 O   |
|  VOL1    K4E151611                  KS0164      N   |
|  VOL2    K4E151611                              N   |
|J                                    169NDK19    3   |
|A     20MHz                           CONN2          |
|M  KA4558                                            |
|M                                                    |
|A                                          KM6161002 |
|          E1-32XT                                    |
|                                           KM6161002 |
|                                                     |
|       ST7705C                             KM6161002 |
| B1             XCS05                                |
| B2 B3          14.31818MHz  LED           KM6161002 |
|-----------------------------------------------------|
Notes:
      ST7705C      - Reset/Watchdog IC (SOIC8)
      E1-32XT      - Hyperstone E1-32XT CPU (QFP144)
      169NDK19     - Xtal, 16.9344MHz
      CONN1,CONN2, - Connectors for joining main board to small sub-board
      CONN3
      XCS05        - Xilinx Spartan XCS05 FPGA (QFP100)
      B1,B2,B3     - Push Buttons for TEST, SERVICE and RESET
      KS0164       - Samsung Electronics KS0164 General Midi compliant 32-voice Wavetable Synthesizer Chip
                     with built-in 16bit CPU and MPU-401 compatibility. (QFP100)
      K4E151611    - Samsung K4E151611C-JC60 1M x16Bit CMOS EDO DRAM (SOJ44)
      KM6161002    - Samsung KM6161002CJ-12 64k x16Bit High-Speed CMOS SRAM (SOJ44)



Sub-Board
---------

Flash Module Type-A REV2 dgPIX Entertainment Inc. 1999
|---------------------------------------|
|            C-O-N-N-1            U100  |
|C                FLASH.U3      FLASH.U5|
|O        FLASH.U2       FLASH.U4       |
|N FLASH.U10                            |
|N                                      |
|3                FLASH.U7      FLASH.U9|
|  CONN2  FLASH.U6       FLASH.U8       |
|---------------------------------------|
Notes:
      FLASH        - Intel DA28F320J5 32M x8 StrataFlash surface-mounted FlashROM (SSOP56)
      CONN1,CONN2,
      CONN3        - Connectors for joining small sub-board to main board
      U100         - A custom programmed PIC (Programmable Interrupt Controller), rebadged as 'dgPIX-PR1' (DIP18)


*********************************************************************/

#include "emu.h"
#include "machine/nvram.h"
#include "cpu/e132xs/e132xs.h"


class dgpix_state : public driver_device
{
public:
	dgpix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vblank(*this, "VBLANK") { }

	required_device<cpu_device> m_maincpu;
	required_ioport m_vblank;

	std::unique_ptr<UINT32[]> m_vram;
	int m_vbuffer;
	int m_flash_roms;
	int m_old_vbuf;
	UINT32 m_flash_cmd;
	INT32 m_first_offset;

	DECLARE_READ32_MEMBER(flash_r);
	DECLARE_WRITE32_MEMBER(flash_w);
	DECLARE_WRITE32_MEMBER(vram_w);
	DECLARE_READ32_MEMBER(vram_r);
	DECLARE_WRITE32_MEMBER(vbuffer_w);
	DECLARE_WRITE32_MEMBER(coin_w);
	DECLARE_READ32_MEMBER(vblank_r);

	DECLARE_DRIVER_INIT(elfin);
	DECLARE_DRIVER_INIT(jumpjump);
	DECLARE_DRIVER_INIT(xfiles);
	DECLARE_DRIVER_INIT(xfilesk);
	DECLARE_DRIVER_INIT(kdynastg);
	DECLARE_DRIVER_INIT(fmaniac3);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	UINT32 screen_update_dgpix(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


READ32_MEMBER(dgpix_state::flash_r)
{
	UINT32 *ROM = (UINT32 *)memregion("flash")->base();

	if(offset >= (0x2000000 - m_flash_roms * 0x400000) / 4)
	{
		if(m_flash_cmd == 0x90900000)
		{
			//read maker ID and chip ID
			return 0x00890014;
		}
		else if(m_flash_cmd == 0x00700000)
		{
			//read status
			return 0x80<<16;
		}
		else if(m_flash_cmd == 0x70700000)
		{
			//read status and ?
			return 0x82<<16;
		}
		else if(m_flash_cmd == 0xe8e80000)
		{
			//read status ?
			return 0x80<<16;
		}
	}

	return ROM[offset];
}

WRITE32_MEMBER(dgpix_state::flash_w)
{
	if(m_flash_cmd == 0x20200000)
	{
		// erase game settings
		if(data == 0xd0d00000)
		{
			// point to game settings
			UINT8 *rom = (UINT8 *)memregion("flash")->base() + offset*4;

			// erase one block
			memset(rom, 0xff, 0x10000);

			m_flash_cmd = 0;
		}
	}
	else if(m_flash_cmd == 0x0f0f0000)
	{
		if(data == 0xd0d00000 && offset == m_first_offset)
		{
			// finished
			m_flash_cmd = 0;
			m_first_offset = -1;
		}
		else
		{
			UINT16 *rom = (UINT16 *)memregion("flash")->base();

			// write game settings

			if(ACCESSING_BITS_0_15)
				rom[BYTE_XOR_BE(offset*2 + 1)] = data & 0xffff;
			else
				rom[BYTE_XOR_BE(offset*2 + 0)] = (data & 0xffff0000) >> 16;
		}
	}
	else
	{
		m_flash_cmd = data;

		if(m_flash_cmd == 0x0f0f0000 && m_first_offset == -1)
		{
			m_first_offset = offset;
		}
	}
}

WRITE32_MEMBER(dgpix_state::vram_w)
{
	UINT32 *dest = &m_vram[offset+(0x40000/4)*m_vbuffer];

	if (mem_mask == 0xffffffff)
	{
		if (~data & 0x80000000)
			*dest = (*dest & 0x0000ffff) | (data & 0xffff0000);

		if (~data & 0x00008000)
			*dest = (*dest & 0xffff0000) | (data & 0x0000ffff);
	}
	else if (((mem_mask == 0xffff0000) && (~data & 0x80000000)) ||
				((mem_mask == 0x0000ffff) && (~data & 0x00008000)))
		COMBINE_DATA(dest);
}

READ32_MEMBER(dgpix_state::vram_r)
{
	return m_vram[offset+(0x40000/4)*m_vbuffer];
}

WRITE32_MEMBER(dgpix_state::vbuffer_w)
{
	if(m_old_vbuf == 3 && (data & 3) == 2)
	{
		m_vbuffer ^= 1;
	}

	m_old_vbuf = data & 3;
}

WRITE32_MEMBER(dgpix_state::coin_w)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);
}

READ32_MEMBER(dgpix_state::vblank_r)
{
	/* burn a bunch of cycles because this is polled frequently during busy loops */
	space.device().execute().eat_cycles(100);
	return m_vblank->read();
}

static ADDRESS_MAP_START( cpu_map, AS_PROGRAM, 32, dgpix_state )
	AM_RANGE(0x00000000, 0x007fffff) AM_RAM
	AM_RANGE(0x40000000, 0x4003ffff) AM_READWRITE(vram_r, vram_w)
	AM_RANGE(0xe0000000, 0xe1ffffff) AM_READWRITE(flash_r, flash_w)
	AM_RANGE(0xe2000000, 0xe3ffffff) AM_READWRITE(flash_r, flash_w)
	AM_RANGE(0xffc00000, 0xffffffff) AM_ROM AM_REGION("flash", 0x1c00000) AM_SHARE("nvram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( io_map, AS_IO, 32, dgpix_state )
	AM_RANGE(0x0200, 0x0203) AM_READNOP // used to sync with the protecion PIC? tested bits 0 and 1
	AM_RANGE(0x0400, 0x0403) AM_READWRITE(vblank_r, vbuffer_w)
	AM_RANGE(0x0a10, 0x0a13) AM_READ_PORT("INPUTS")
	AM_RANGE(0x0200, 0x0203) AM_WRITE(coin_w)
	AM_RANGE(0x0c00, 0x0c03) AM_WRITENOP // writes only: 1, 0, 1 at startup
	AM_RANGE(0x0c80, 0x0c83) AM_WRITENOP // sound commands / latches
	AM_RANGE(0x0c80, 0x0c83) AM_READNOP //read at startup -> cmp 0xFE
	AM_RANGE(0x0c84, 0x0c87) AM_READNOP // sound status, checks bit 0x40 and 0x80
ADDRESS_MAP_END


static INPUT_PORTS_START( dgpix )
	PORT_START("VBLANK")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen") // value 2 is used by fmaniac3
	PORT_BIT( 0xfffffffc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_SERVICE_NO_TOGGLE( 0x00010000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

void dgpix_state::video_start()
{
	m_vram = std::make_unique<UINT32[]>(0x40000*2/4);

	save_pointer(NAME(m_vram.get()), 0x40000*2/4);
}

UINT32 dgpix_state::screen_update_dgpix(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int y;

	for (y = 0; y < 240; y++)
	{
		int x;
		UINT32 *src = &m_vram[(m_vbuffer ? 0 : 0x10000) | (y << 8)];
		UINT16 *dest = &bitmap.pix16(y);

		for (x = 0; x < 320; x += 2)
		{
			dest[0] = (*src >> 16) & 0x7fff;
			dest[1] = (*src >>  0) & 0x7fff;

			src++;
			dest += 2;
		}
	}

	return 0;
}

void dgpix_state::machine_start()
{
	save_item(NAME(m_vbuffer));
	save_item(NAME(m_flash_cmd));
	save_item(NAME(m_first_offset));
	save_item(NAME(m_old_vbuf));
}

void dgpix_state::machine_reset()
{
	m_vbuffer = 0;
	m_flash_cmd = 0;
	m_first_offset = -1;
	m_old_vbuf = 3;
}


static MACHINE_CONFIG_START( dgpix, dgpix_state )
	MCFG_CPU_ADD("maincpu", E132XT, 20000000*4) /* 4x internal multiplier */
	MCFG_CPU_PROGRAM_MAP(cpu_map)
	MCFG_CPU_IO_MAP(io_map)

/*
    unknown 16bit sound cpu, embedded inside the KS0164 sound chip
    running at 16.9MHz
*/

	MCFG_NVRAM_ADD_NO_FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE_DRIVER(dgpix_state, screen_update_dgpix)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_BBBBBGGGGGRRRRR("palette")


	/* sound hardware */
	// KS0164 sound chip
MACHINE_CONFIG_END


/*
Elfin
dgPIX Entertainment Inc. 1999

PCB combo:
VRenderO Minus Rev4 dgPIX Entertainment Inc. 1999
Flash Module Type-A REV2 dgPIX Entertainment Inc. 1999

*/
ROM_START( elfin )
	ROM_REGION32_BE( 0x2000000, "flash", ROMREGION_ERASE00 ) /* Hyperstone CPU Code & Data */
	/* 0 - 0x17fffff empty space */
	ROM_LOAD16_WORD_SWAP( "flash.u8", 0x1800000, 0x400000, CRC(eb56d7ca) SHA1(7c1cfcc68579cf3bdd9707da7d745a410223b8d9) )
	ROM_LOAD16_WORD_SWAP( "flash.u9", 0x1c00000, 0x400000, CRC(cbf64ef4) SHA1(1a231872ee14e6d718c3f8888185ede7483e79dd) ) /* game settings & highscores are saved in here */

	ROM_REGION( 0x400000, "cpu1", 0 ) /* sound rom */
	ROM_LOAD16_WORD_SWAP( "flash.u10", 0x000000, 0x400000, CRC(d378fe55) SHA1(5cc7bc5ae258cd48816857793a262e7c6c330795) )

	ROM_REGION( 0x1000, "cpu2", ROMREGION_ERASEFF ) /* PIC */
	ROM_LOAD( "elfin_pic",  0x0000, 0x1000, NO_DUMP ) // protected
ROM_END

/*
Jump Jump
dgPIX Entertainment Inc. 1999

PCB combo:
VRenderO Minus Rev5 dgPIX Entertainment Inc. 1999
Flash Module Type-A REV2 dgPIX Entertainment Inc. 1999

*/
ROM_START( jumpjump )
	ROM_REGION32_BE( 0x2000000, "flash", ROMREGION_ERASE00 ) /* Hyperstone CPU Code & Data */
	/* 0 - 0x17fffff empty space */
	ROM_LOAD16_WORD_SWAP( "jumpjump.u8", 0x1800000, 0x400000, CRC(210dfd8b) SHA1(a1aee4ec8c01832e77d2e4e334a62c246d7e3635) )
	ROM_LOAD16_WORD_SWAP( "jumpjump.u9", 0x1c00000, 0x400000, CRC(16d1e352) SHA1(3c43974fb8d90b0c84472dd9f2167eb983142095) )

	ROM_REGION( 0x400000, "cpu1", 0 ) /* sound rom */
	ROM_LOAD16_WORD_SWAP( "jumpjump.u10", 0x000000, 0x400000, CRC(2152ecce) SHA1(522d389952a07fa0830ca8aaa6de3aacf834e32e) )

	ROM_REGION( 0x1000, "cpu2", ROMREGION_ERASEFF ) /* PIC */
	ROM_LOAD( "jumpjump_pic",  0x0000, 0x1000, NO_DUMP ) // protected - labeled S831D dgPIX-PR1
ROM_END

/*

The X-Files
dgPIX Entertainment Inc. 1999

PCB combo:
VRenderO Minus Rev5 dgPIX Entertainment Inc. 1999
Flash Module Type-A REV2 dgPIX Entertainment Inc. 1999

Uncensored World version

*/
ROM_START( xfiles )
	ROM_REGION32_BE( 0x2000000, "flash", ROMREGION_ERASE00 ) /* Hyperstone CPU Code & Data */
	/* 0 - 0x17fffff empty space */
	ROM_LOAD16_WORD_SWAP( "flash.u8",  0x1800000, 0x400000, CRC(231ad82a) SHA1(a1cc5c4122605e564d51137f1dca2afa82616202) )
	ROM_LOAD16_WORD_SWAP( "flash.u9",  0x1c00000, 0x400000, CRC(d68994b7) SHA1(c1752d6795f7aaa6beef73643327205a1c32f0f5) )

	ROM_REGION( 0x400000, "cpu1", 0 ) /* sound rom */
	ROM_LOAD16_WORD_SWAP( "flash.u10", 0x0000000, 0x400000, CRC(1af33cda) SHA1(9bbcfb07a4a5bcff3efc1c7bcc51bc16c47ca9e6) )

	ROM_REGION( 0x1000, "cpu2", 0 ) /* PIC */
	ROM_LOAD( "xfiles_pic",  0x0000, 0x1000, NO_DUMP ) // protected
ROM_END

/*

The X-Files
dgPIX Entertainment Inc. 1999

PCB combo:
VRenderO Minus Rev4 dgPIX Entertainment Inc. 1999
Flash Module Type-A REV2 dgPIX Entertainment Inc. 1999

Contrary to what you might think on first hearing the title, this game
is like Match It 2 etc. However, the quality of the graphics
is outstanding, perhaps the most high quality seen in this "type" of game.
At the end of the level, you are presented with a babe, where you can use
the joystick and buttons to scroll up and down and zoom in for erm...
a closer inspection of the 'merchandise' ;-))

Censored version for the Korean market
Korean text on Mode Select screen and the following screen

*/
ROM_START( xfilesk )
	ROM_REGION32_BE( 0x2000000, "flash", ROMREGION_ERASE00 ) /* Hyperstone CPU Code & Data */
	/* 0 - 0x17fffff empty space */
	ROM_LOAD16_WORD_SWAP( "u8.bin",  0x1800000, 0x400000, CRC(3b2c2bc1) SHA1(1c07fb5bd8a8c9b5fb169e6400fef845f3aee7aa) )
	ROM_LOAD16_WORD_SWAP( "u9.bin",  0x1c00000, 0x400000, CRC(6ecdd1eb) SHA1(e26c9711e589865cc75ec693d382758fa52528b8) )

	ROM_REGION( 0x400000, "cpu1", 0 ) /* sound rom */
	ROM_LOAD16_WORD_SWAP( "u10.bin", 0x0000000, 0x400000, CRC(f2ef1eb9) SHA1(d033d140fce6716d7d78509aa5387829f0a1404c) )

	ROM_REGION( 0x1000, "cpu2", 0 ) /* PIC */
	ROM_LOAD( "xfilesk_pic",  0x0000, 0x1000, NO_DUMP ) // protected - same PIC as parent??
ROM_END

/*
King of Dynast Gear
EZ Graphics, 1999

PCB combo:
VRenderO Minus Rev5 dgPIX Entertainment Inc. 1999
Flash Module Type-A REV2 dgPIX Entertainment Inc. 1999

*/
ROM_START( kdynastg )
	ROM_REGION32_BE( 0x2000000, "flash", ROMREGION_ERASE00 )  /* Hyperstone CPU Code & Data */
	/* 0 - 0x0ffffff empty space */
	ROM_LOAD16_WORD_SWAP( "flash.u6",  0x1000000, 0x400000, CRC(280dd64e) SHA1(0e23b227b1183fb5591c3a849b5a5fe7faa23cc8) )
	ROM_LOAD16_WORD_SWAP( "flash.u7",  0x1400000, 0x400000, CRC(f9125894) SHA1(abaad31f7a02143ea7029e47e6baf2976365f70c) )
	ROM_LOAD16_WORD_SWAP( "flash.u8",  0x1800000, 0x400000, CRC(1016b61c) SHA1(eab4934e1f41cc26259e5187a94ceebd45888a94) )
	ROM_LOAD16_WORD_SWAP( "flash.u9",  0x1c00000, 0x400000, CRC(093d9243) SHA1(2a643acc7144193aaa3606a84b0c67aadb4c543b) )

	ROM_REGION( 0x400000, "cpu1", 0 ) /* sound rom */
	ROM_LOAD16_WORD_SWAP( "flash.u10", 0x0000000, 0x400000, CRC(3f103cb1) SHA1(2ff9bd73f3005f09d872018b81c915b01d6703f5) )

	ROM_REGION( 0x1000, "cpu2", 0 ) /* PIC */
	ROM_LOAD( "kdynastg_pic",  0x0000, 0x1000, NO_DUMP ) // protected
ROM_END

/*
Fishing Maniac 3
Saero Entertainment, 2002

PCB combo:
VRenderO Minus Rev4 dgPIX Entertainment Inc. 1999
Flash Module Type-A REV2 dgPIX Entertainment Inc. 1999

 U100 18 pin socket for the PIC chip is unused

*/
ROM_START( fmaniac3 )
	ROM_REGION32_BE( 0x2000000, "flash", ROMREGION_ERASE00 ) /* Hyperstone CPU Code & Data */
	/* 0 - 0x17fffff empty space */
	ROM_LOAD16_WORD_SWAP( "flash.u8", 0x1800000, 0x400000, CRC(dc08a224) SHA1(4d14145eb84ad13674296f81e90b9d60403fa0de) )
	ROM_LOAD16_WORD_SWAP( "flash.u9", 0x1c00000, 0x400000, CRC(c1fee95f) SHA1(0ed5ed9fa18e7da9242a6df2c210c46de25a2281) )

	ROM_REGION( 0x400000, "cpu1", 0 ) /* sound rom */
	ROM_LOAD16_WORD_SWAP( "flash.u10", 0x000000, 0x400000, CRC(dfeb91a0) SHA1(a4a79073c3f6135957ea8a4a66a9c71a3a39893c) )

	ROM_REGION( 0x1000, "cpu2", ROMREGION_ERASEFF ) /* PIC */
	// not present
ROM_END



DRIVER_INIT_MEMBER(dgpix_state,elfin)
{
	UINT8 *rom = (UINT8 *)memregion("flash")->base() + 0x1c00000;

	rom[BYTE4_XOR_BE(0x3a9e94)] = 3;
	rom[BYTE4_XOR_BE(0x3a9e95)] = 0;
	rom[BYTE4_XOR_BE(0x3a9e96)] = 3;
	rom[BYTE4_XOR_BE(0x3a9e97)] = 0;
	rom[BYTE4_XOR_BE(0x3a9e98)] = 3;
	rom[BYTE4_XOR_BE(0x3a9e99)] = 0;

	m_flash_roms = 2;
}

DRIVER_INIT_MEMBER(dgpix_state,jumpjump)
{
	UINT8 *rom = (UINT8 *)memregion("flash")->base() + 0x1c00000;

	rom[BYTE4_XOR_BE(0x3a829a)] = 3;
	rom[BYTE4_XOR_BE(0x3a829b)] = 0;
	rom[BYTE4_XOR_BE(0x3a829c)] = 3;
	rom[BYTE4_XOR_BE(0x3a829d)] = 0;
	rom[BYTE4_XOR_BE(0x3a829e)] = 3;
	rom[BYTE4_XOR_BE(0x3a829f)] = 0;

	m_flash_roms = 2;
}

DRIVER_INIT_MEMBER(dgpix_state,xfiles)
{
	UINT8 *rom = (UINT8 *)memregion("flash")->base() + 0x1c00000;

	rom[BYTE4_XOR_BE(0x3a9a2a)] = 3;
	rom[BYTE4_XOR_BE(0x3a9a2b)] = 0;
	rom[BYTE4_XOR_BE(0x3a9a2c)] = 3;
	rom[BYTE4_XOR_BE(0x3a9a2d)] = 0;
	rom[BYTE4_XOR_BE(0x3a9a2e)] = 3;
	rom[BYTE4_XOR_BE(0x3a9a2f)] = 0;

	m_flash_roms = 2;
}

DRIVER_INIT_MEMBER(dgpix_state,xfilesk)
{
	UINT8 *rom = (UINT8 *)memregion("flash")->base() + 0x1c00000;

	rom[BYTE4_XOR_BE(0x3aa92e)] = 3;
	rom[BYTE4_XOR_BE(0x3aa92f)] = 0;
	rom[BYTE4_XOR_BE(0x3aa930)] = 3;
	rom[BYTE4_XOR_BE(0x3aa931)] = 0;
	rom[BYTE4_XOR_BE(0x3aa932)] = 3;
	rom[BYTE4_XOR_BE(0x3aa933)] = 0;

//  protection related ?
//  m_maincpu->space(AS_PROGRAM).nop_read(0xf0c8b440, 0xf0c8b447);

	m_flash_roms = 2;
}

DRIVER_INIT_MEMBER(dgpix_state,kdynastg)
{
	UINT8 *rom = (UINT8 *)memregion("flash")->base() + 0x1c00000;

	rom[BYTE4_XOR_BE(0x3aaa10)] = 3; // 129f0 - nopped call
	rom[BYTE4_XOR_BE(0x3aaa11)] = 0;
	rom[BYTE4_XOR_BE(0x3aaa12)] = 3;
	rom[BYTE4_XOR_BE(0x3aaa13)] = 0;
	rom[BYTE4_XOR_BE(0x3aaa14)] = 3;
	rom[BYTE4_XOR_BE(0x3aaa15)] = 0;

	rom[BYTE4_XOR_BE(0x3a45c8)] = 5; // c5a8 - added ret
	rom[BYTE4_XOR_BE(0x3a45c9)] = 0;

//  protection related ?
//  m_maincpu->space(AS_PROGRAM).nop_read(0x12341234, 0x12341243);

	m_flash_roms = 4;
}

DRIVER_INIT_MEMBER(dgpix_state,fmaniac3)
{
	m_flash_roms = 2;
}

GAME( 1999, elfin,          0, dgpix, dgpix, dgpix_state, elfin,    ROT0, "dgPIX Entertainment Inc.", "Elfin",                             MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1999, jumpjump,       0, dgpix, dgpix, dgpix_state, jumpjump, ROT0, "dgPIX Entertainment Inc.", "Jump Jump",                         MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1999, xfiles,         0, dgpix, dgpix, dgpix_state, xfiles,   ROT0, "dgPIX Entertainment Inc.", "The X-Files",                       MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1999, xfilesk,   xfiles, dgpix, dgpix, dgpix_state, xfilesk,  ROT0, "dgPIX Entertainment Inc.", "The X-Files (Censored, Korea)",     MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1999, kdynastg,       0, dgpix, dgpix, dgpix_state, kdynastg, ROT0, "EZ Graphics",              "King of Dynast Gear (version 1.8)", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, fmaniac3,       0, dgpix, dgpix, dgpix_state, fmaniac3, ROT0, "Saero Entertainment",      "Fishing Maniac 3",                  MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
