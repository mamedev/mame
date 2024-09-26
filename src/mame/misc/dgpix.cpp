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
 - Let's Dance                         (c) 1999 dgPIX Entertainment Inc.
 - Beat Player 2000                    (c) 2000 dgPIX Entertainment Inc.
 - Fishing Maniac 2+                   (c) 2000 Saero Entertainment
 - Fishing Maniac 3                    (c) 2002 Saero Entertainment

 [*] the version number is written in the flash roms at the beginning of the game settings

Note: There is known to exist an alternate version of The X-Files titled The Sex Files which is undumped
The following additional games are also known to be undumped

- Fishing Maniac, using the VRender0 Minus Rev5 PCB, with a Flash Module Type-A REV2 dgPIX Entertainment Inc. 1999 subboard.
- Fishing Maniac 2, using the VRender0 Minus Rev5 PCB, with a Flash Module Type-A REV2 dgPIX Entertainment Inc. 1999 subboard.

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

VRender0 Minus Rev4 dgPIX Entertainment Inc. 1999
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

VRender0 Minus Rev5 dgPIX Entertainment Inc. 1999
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
      U100         - A custom programmed PIC microcontroller, rebadged as 'dgPIX-PR1' (DIP18)


*********************************************************************/

#include "emu.h"
#include "cpu/e132xs/e132xs.h"
#include "machine/intelfsh.h"
#include "machine/nvram.h"
#include "sound/ks0164.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class dgpix_state : public driver_device
{
public:
	dgpix_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_sound(*this, "ks0164"),
		m_flash(*this, "flash%u", 0),
		m_vblank(*this, "VBLANK")
	{ }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void dgpix_base(machine_config &config);

	void base_map(address_map &map) ATTR_COLD;

	u16 flash_r(offs_t offset);
	void flash_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	required_device<cpu_device> m_maincpu;
	required_device<ks0164_device> m_sound;
	optional_device_array<intel_28f320j5_device, 8> m_flash; // up to 8 but not all populated

private:
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void io_map(address_map &map) ATTR_COLD;

	void vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 vram_r(offs_t offset);

	void vbuffer_w(u32 data);

	void coin_w(u32 data);

	u32 vblank_r();

	void mpu401_data_w(offs_t, u32 data, u32 mem_mask);
	void mpu401_ctrl_w(offs_t, u32 data, u32 mem_mask);
	u32 mpu401_data_r(offs_t, u32 mem_mask);
	u32 mpu401_status_r();

	u16 flash_raw_rom_r(offs_t offset);

	required_ioport m_vblank;

	std::unique_ptr<u16[]> m_vram;
	int m_vbuffer;
	int m_old_vbuf;
};


class dgpix_typea_state : public dgpix_state
{
public:
	dgpix_typea_state(const machine_config &mconfig, device_type type, const char *tag) :
		dgpix_state(mconfig, type, tag)
	{ }

	void dgpix(machine_config &config);
	void dgpix_kdynastg(machine_config &config);

	void init_elfin();
	void init_jumpjump();
	void init_xfiles();
	void init_xfilesk();
	void init_kdynastg();
	void init_fmaniac2p();
	void init_fmaniac3();

private:
	void mem_map(address_map &map) ATTR_COLD;
};


class dgpix_bmkey_state : public dgpix_state
{
public:
	dgpix_bmkey_state(const machine_config &mconfig, device_type type, const char *tag) :
		dgpix_state(mconfig, type, tag),
		m_ks0164_bank(*this, "ks0164_bank")
	{ }

	void dgpix(machine_config &config);

	void init_letsdnce();
	void init_btplay2k();

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	void sound_bank_w(u16 data);

	required_memory_region m_ks0164_bank;
};

void dgpix_state::mpu401_data_w(offs_t, u32 data, u32 mem_mask)
{
	if(ACCESSING_BITS_0_7)
		m_sound->mpu401_data_w(data);
}

void dgpix_state::mpu401_ctrl_w(offs_t, u32 data, u32 mem_mask)
{
	if(ACCESSING_BITS_0_7)
		m_sound->mpu401_ctrl_w(data);
}

u32 dgpix_state::mpu401_data_r(offs_t, u32 mem_mask)
{
	if(ACCESSING_BITS_0_7)
		return m_sound->mpu401_data_r();
	return 0;
}

u32 dgpix_state::mpu401_status_r()
{
	return m_sound->mpu401_status_r();
}

u16 dgpix_state::flash_r(offs_t offset)
{
	const auto bank = offset / 0x200000;

	if (!m_flash[bank])
		return 0;

	return m_flash[bank]->read(offset % 0x200000);
}

void dgpix_state::flash_w(offs_t offset, u16 data, u16 mem_mask)
{
	const auto bank = offset / 0x200000;

	if (!m_flash[bank])
		return;

	m_flash[bank]->write(offset % 0x200000, data);
}

u16 dgpix_state::flash_raw_rom_r(offs_t offset)
{
	const auto bank = offset / 0x200000;

	if (!m_flash[bank])
		return 0;

	return swapendian_int16(m_flash[bank]->read_raw(offset % 0x200000));
}

void dgpix_state::vram_w(offs_t offset, u16 data, u16 mem_mask)
{
	if ((mem_mask == 0xffff) && (~data & 0x8000))
		COMBINE_DATA(&m_vram[offset + (0x40000 / 2) * m_vbuffer]);
}

u16 dgpix_state::vram_r(offs_t offset)
{
	return m_vram[offset + (0x40000 / 2) * m_vbuffer];
}

void dgpix_state::vbuffer_w(u32 data)
{
	if (m_old_vbuf == 3 && (data & 3) == 2)
	{
		m_vbuffer ^= 1;
	}

	m_old_vbuf = data & 3;
}

void dgpix_state::coin_w(u32 data)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);
}

u32 dgpix_state::vblank_r()
{
	/* burn a bunch of cycles because this is polled frequently during busy loops */
	m_maincpu->eat_cycles(100);
	return m_vblank->read();
}

void dgpix_bmkey_state::sound_bank_w(u16 data)
{
	auto bank_addr = data * 0x400000;
	if (bank_addr >= m_ks0164_bank->bytes())
		return;

	// Each flash contains a new program for the CPU, letsdnce's stop audio command
	// in the test menu is called "change bank / sound off" so they seem linked.
	// Unsure if this is really hooked up to the CPU's reset pin or not or if there
	// is some other mechanism for signaling the program changed and to stop sound.
	const u32 bank_mask = 0x400000 - 1;
	m_sound->space().install_rom(0, bank_mask, ((1 << 23) - 1) ^ bank_mask, m_ks0164_bank->base() + bank_addr);
}

void dgpix_state::base_map(address_map &map)
{
	map(0x00000000, 0x007fffff).ram();
	map(0x40000000, 0x4003ffff).rw(FUNC(dgpix_typea_state::vram_r), FUNC(dgpix_typea_state::vram_w));
	map(0xfe000000, 0xffffffff).r(FUNC(dgpix_typea_state::flash_raw_rom_r));
}

void dgpix_typea_state::mem_map(address_map &map)
{
	base_map(map);

	map(0xe0000000, 0xe1ffffff).rw(FUNC(dgpix_typea_state::flash_r), FUNC(dgpix_typea_state::flash_w)).mirror(0x02000000);
}

void dgpix_bmkey_state::mem_map(address_map &map)
{
	base_map(map);

	map(0xe1000000, 0xe1000001).w(FUNC(dgpix_bmkey_state::sound_bank_w));
	map(0xe2000000, 0xe3ffffff).rw(FUNC(dgpix_bmkey_state::flash_r), FUNC(dgpix_bmkey_state::flash_w));
}

void dgpix_state::io_map(address_map &map)
{
	map(0x0200, 0x0203).nopr(); // used to sync with the protecion PIC? tested bits 0 and 1
	map(0x0400, 0x0403).rw(FUNC(dgpix_state::vblank_r), FUNC(dgpix_state::vbuffer_w));
	map(0x0a10, 0x0a13).portr("INPUTS");
	map(0x0200, 0x0203).w(FUNC(dgpix_state::coin_w));
	map(0x0c00, 0x0c03).nopw(); // writes only: 1, 0, 1 at startup
	map(0x0c80, 0x0c83).rw(FUNC(dgpix_state::mpu401_data_r), FUNC(dgpix_state::mpu401_data_w));
	map(0x0c84, 0x0c87).rw(FUNC(dgpix_state::mpu401_status_r), FUNC(dgpix_state::mpu401_ctrl_w));
}

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

static INPUT_PORTS_START( btplay2k )
	PORT_INCLUDE( dgpix )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( letsdnce )
	PORT_INCLUDE( dgpix )

	PORT_MODIFY("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Up")
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Down")
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Left")
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Right")
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Up")
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Down")
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Left")
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Right")
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
INPUT_PORTS_END

void dgpix_state::video_start()
{
	m_vram = std::make_unique<u16[]>(0x40000);

	save_pointer(NAME(m_vram), 0x40000);
}

u32 dgpix_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		int x = cliprect.left();
		u16 const *src = &m_vram[(m_vbuffer ? 0 : 0x20000) | (y << 9) | x];
		u16 *dest = &bitmap.pix(y, x);

		for (; x <= cliprect.right(); x++)
		{
			*dest++ = *src++ & 0x7fff;
		}
	}

	return 0;
}

void dgpix_state::machine_start()
{
	save_item(NAME(m_vbuffer));
	save_item(NAME(m_old_vbuf));
}

void dgpix_state::machine_reset()
{
	m_vbuffer = 0;
	m_old_vbuf = 3;
}

void dgpix_bmkey_state::machine_reset()
{
	dgpix_state::machine_reset();
	sound_bank_w(0);
}

void dgpix_state::dgpix_base(machine_config &config)
{
	E132XT(config, m_maincpu, 20000000*4); /* 4x internal multiplier */
	m_maincpu->set_addrmap(AS_IO, &dgpix_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(512, 256);
	screen.set_visarea(0, 319, 0, 239);
	screen.set_screen_update(FUNC(dgpix_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::BGR_555);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	KS0164(config, m_sound, 16.9344_MHz_XTAL);
	m_sound->add_route(0, "lspeaker", 1.0);
	m_sound->add_route(1, "rspeaker", 1.0);

	INTEL_28F320J5(config, m_flash[6]);
	INTEL_28F320J5(config, m_flash[7]);
}

void dgpix_typea_state::dgpix(machine_config &config)
{
	dgpix_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &dgpix_typea_state::mem_map);
}

void dgpix_typea_state::dgpix_kdynastg(machine_config &config)
{
	dgpix(config);

	INTEL_28F320J5(config, m_flash[4]);
	INTEL_28F320J5(config, m_flash[5]);
}

void dgpix_bmkey_state::dgpix(machine_config &config)
{
	dgpix_base(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &dgpix_bmkey_state::mem_map);
}


/*

Elfin
dgPIX Entertainment Inc. 1999

PCB combo:
VRender0 Minus Rev4 dgPIX Entertainment Inc. 1999
Flash Module Type-A REV2 dgPIX Entertainment Inc. 1999

*/
ROM_START( elfin )
	/* Hyperstone CPU Code & Data */
	ROM_REGION32_LE( 0x400000, "flash0", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash1", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash2", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash3", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash4", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash5", ROMREGION_ERASE00 )

	ROM_REGION32_LE( 0x400000, "flash6", ROMREGION_ERASE00 )
	ROM_LOAD( "flash.u8", 0x0000000, 0x400000, CRC(eb56d7ca) SHA1(7c1cfcc68579cf3bdd9707da7d745a410223b8d9) )

	ROM_REGION32_LE( 0x400000, "flash7", ROMREGION_ERASE00 )
	ROM_LOAD( "flash.u9", 0x0000000, 0x400000, CRC(cbf64ef4) SHA1(1a231872ee14e6d718c3f8888185ede7483e79dd) ) /* game settings & highscores are saved in here */

	ROM_REGION( 0x400000, "ks0164", 0 ) /* sound ROM */
	ROM_LOAD16_WORD_SWAP( "flash.u10", 0x0000000, 0x400000, CRC(d378fe55) SHA1(5cc7bc5ae258cd48816857793a262e7c6c330795) )

	ROM_REGION( 0x1000, "cpu2", ROMREGION_ERASEFF ) /* PIC */
	ROM_LOAD( "elfin_pic",  0x0000, 0x1000, NO_DUMP ) // protected
ROM_END

/*

Jump Jump
dgPIX Entertainment Inc. 1999

PCB combo:
VRender0 Minus Rev5 dgPIX Entertainment Inc. 1999
Flash Module Type-A REV2 dgPIX Entertainment Inc. 1999

*/
ROM_START( jumpjump )
	/* Hyperstone CPU Code & Data */
	ROM_REGION32_LE( 0x400000, "flash0", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash1", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash2", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash3", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash4", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash5", ROMREGION_ERASE00 )

	ROM_REGION32_LE( 0x400000, "flash6", ROMREGION_ERASE00 )
	ROM_LOAD( "jumpjump.u8", 0x0000000, 0x400000, CRC(210dfd8b) SHA1(a1aee4ec8c01832e77d2e4e334a62c246d7e3635) )

	ROM_REGION32_LE( 0x400000, "flash7", ROMREGION_ERASE00 )
	ROM_LOAD( "jumpjump.u9", 0x0000000, 0x400000, CRC(16d1e352) SHA1(3c43974fb8d90b0c84472dd9f2167eb983142095) )

	ROM_REGION( 0x400000, "ks0164", 0 ) /* sound ROM */
	ROM_LOAD16_WORD_SWAP( "jumpjump.u10", 0x0000000, 0x400000, CRC(2152ecce) SHA1(522d389952a07fa0830ca8aaa6de3aacf834e32e) )

	ROM_REGION( 0x1000, "cpu2", ROMREGION_ERASEFF ) /* PIC */
	ROM_LOAD( "jumpjump_pic",  0x0000, 0x1000, NO_DUMP ) // protected - labeled S831D dgPIX-PR1
ROM_END

/*

The X-Files
dgPIX Entertainment Inc. 1999

PCB combo:
VRender0 Minus Rev5 dgPIX Entertainment Inc. 1999
Flash Module Type-A REV2 dgPIX Entertainment Inc. 1999

Uncensored World version

*/
ROM_START( xfiles )
	/* Hyperstone CPU Code & Data */
	ROM_REGION32_LE( 0x400000, "flash0", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash1", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash2", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash3", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash4", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash5", ROMREGION_ERASE00 )

	ROM_REGION32_LE( 0x400000, "flash6", ROMREGION_ERASE00 )
	ROM_LOAD( "flash.u8",  0x0000000, 0x400000, CRC(231ad82a) SHA1(a1cc5c4122605e564d51137f1dca2afa82616202) )

	ROM_REGION32_LE( 0x400000, "flash7", ROMREGION_ERASE00 )
	ROM_LOAD( "flash.u9",  0x0000000, 0x400000, CRC(d68994b7) SHA1(c1752d6795f7aaa6beef73643327205a1c32f0f5) )

	ROM_REGION( 0x400000, "ks0164", 0 ) /* sound ROM */
	ROM_LOAD16_WORD_SWAP( "flash.u10", 0x0000000, 0x400000, CRC(1af33cda) SHA1(9bbcfb07a4a5bcff3efc1c7bcc51bc16c47ca9e6) )

	ROM_REGION( 0x1000, "cpu2", 0 ) /* PIC */
	ROM_LOAD( "xfiles_pic",  0x0000, 0x1000, NO_DUMP ) // protected
ROM_END

/*

The X-Files
dgPIX Entertainment Inc. 1999

PCB combo:
VRender0 Minus Rev4 dgPIX Entertainment Inc. 1999
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
	/* Hyperstone CPU Code & Data */
	ROM_REGION32_LE( 0x400000, "flash0", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash1", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash2", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash3", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash4", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash5", ROMREGION_ERASE00 )

	ROM_REGION32_LE( 0x400000, "flash6", ROMREGION_ERASE00 )
	ROM_LOAD( "u8.bin",  0x0000000, 0x400000, CRC(3b2c2bc1) SHA1(1c07fb5bd8a8c9b5fb169e6400fef845f3aee7aa) )

	ROM_REGION32_LE( 0x400000, "flash7", ROMREGION_ERASE00 )
	ROM_LOAD( "u9.bin",  0x0000000, 0x400000, CRC(6ecdd1eb) SHA1(e26c9711e589865cc75ec693d382758fa52528b8) )

	ROM_REGION( 0x400000, "ks0164", 0 ) /* sound ROM */
	ROM_LOAD16_WORD_SWAP( "u10.bin", 0x0000000, 0x400000, CRC(f2ef1eb9) SHA1(d033d140fce6716d7d78509aa5387829f0a1404c) )

	ROM_REGION( 0x1000, "cpu2", 0 ) /* PIC */
	ROM_LOAD( "xfilesk_pic",  0x0000, 0x1000, NO_DUMP ) // protected - same PIC as parent??
ROM_END

/*

King of Dynast Gear
EZ Graphics, 1999

PCB combo:
VRender0 Minus Rev5 dgPIX Entertainment Inc. 1999
Flash Module Type-A REV2 dgPIX Entertainment Inc. 1999

*/
ROM_START( kdynastg )
	/* Hyperstone CPU Code & Data */
	ROM_REGION32_LE( 0x400000, "flash0", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash1", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash2", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash3", ROMREGION_ERASE00 )

	ROM_REGION32_LE( 0x400000, "flash4", ROMREGION_ERASE00 )
	ROM_LOAD( "flash.u6",  0x0000000, 0x400000, CRC(280dd64e) SHA1(0e23b227b1183fb5591c3a849b5a5fe7faa23cc8) )

	ROM_REGION32_LE( 0x400000, "flash5", ROMREGION_ERASE00 )
	ROM_LOAD( "flash.u7",  0x0000000, 0x400000, CRC(f9125894) SHA1(abaad31f7a02143ea7029e47e6baf2976365f70c) )

	ROM_REGION32_LE( 0x400000, "flash6", ROMREGION_ERASE00 )
	ROM_LOAD( "flash.u8",  0x0000000, 0x400000, CRC(1016b61c) SHA1(eab4934e1f41cc26259e5187a94ceebd45888a94) )

	ROM_REGION32_LE( 0x400000, "flash7", ROMREGION_ERASE00 )
	ROM_LOAD( "flash.u9",  0x0000000, 0x400000, CRC(093d9243) SHA1(2a643acc7144193aaa3606a84b0c67aadb4c543b) )

	ROM_REGION( 0x400000, "ks0164", 0 ) /* sound ROM */
	ROM_LOAD16_WORD_SWAP( "flash.u10", 0x0000000, 0x400000, CRC(3f103cb1) SHA1(2ff9bd73f3005f09d872018b81c915b01d6703f5) )

	ROM_REGION( 0x1000, "cpu2", 0 ) /* PIC */
	ROM_LOAD( "kdynastg_pic",  0x0000, 0x1000, NO_DUMP ) // protected
ROM_END

/*

Let's Dance
dgPIX Entertainment Inc. 1999

PCB combo:
VRender0 Minus Rev5 dgPIX Entertainment Inc. 1999
BMkey Flash Rev2 dgPIX Entertainment 1999

*/
ROM_START( letsdnce )
	ROM_REGION32_LE( 0x400000, "flash0", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash1", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash2", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash3", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash4", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash5", ROMREGION_ERASE00 )

	ROM_REGION32_LE( 0x400000, "flash6", ROMREGION_ERASE00 )
	ROM_LOAD( "flash.u9",  0x0000000, 0x400000, CRC(90e181d6) SHA1(c698b842c045f95a5f3a5483b5e5d12ca06c8f08) )

	ROM_REGION32_LE( 0x400000, "flash7", ROMREGION_ERASE00 )
	ROM_LOAD( "flash.u10", 0x0000000, 0x400000, CRC(1416acb3) SHA1(097b89d5cebeaa29742abec8ed84b50313f0b387) )

	ROM_REGION32_LE( 0x1c00000, "ks0164_bank", 0 ) /* sound ROMs */
	ROM_LOAD( "flash.u20", 0x0000000, 0x400000, CRC(e88ccc12) SHA1(6d988fe337a166f6e77ed67e83de7a64688958d1) )
	ROM_LOAD( "flash.u21", 0x0400000, 0x400000, CRC(ffbbde83) SHA1(a6307d782024cd1b6c9fd83ffeef64c31c6bd22d) )
	ROM_LOAD( "flash.u22", 0x0800000, 0x400000, CRC(068c376a) SHA1(622db2b76d84d53bc235fd77aea85cdb2d8c286c) )
	ROM_LOAD( "flash.u23", 0x0c00000, 0x400000, CRC(ab033dc7) SHA1(e46c0902bc5cc2608011bbb27fb136d9ccae1789) )
	ROM_LOAD( "flash.u24", 0x1000000, 0x400000, CRC(23a556d8) SHA1(8ed58febbb1c51f4315494859afa04c916155471) )
	ROM_LOAD( "flash.u25", 0x1400000, 0x400000, CRC(2a0f0e06) SHA1(9b1d6978a72c354aa1ef97ca4aa5902985f0aaed) )
	ROM_LOAD( "flash.u26", 0x1800000, 0x400000, CRC(2a0c396b) SHA1(a8eead3de11c85997c930f000ed48c783bcee07c) )

	ROM_REGION( 0x1000, "cpu2", ROMREGION_ERASEFF ) /* PIC */
	ROM_LOAD( "letsdnce_pic",  0x0000, 0x1000, NO_DUMP ) // protected - labeled S831D dgPIX-PR1

	ROM_REGION( 0x117, "pals", 0 )
	ROM_LOAD( "palce16v8h.u11",  0x0000, 0x117, CRC(47e474c9) SHA1(f13ef4050072ab000a45140c180a3b97dacd8675) )
ROM_END

/*

Beat Player 2000
dgPIX Entertainment Inc. 2000

PCB combo:
VRender0 Minus Rev5 dgPIX Entertainment Inc. 1999
BMkey Flash Rev2 dgPIX Entertainment 1999

*/
ROM_START( btplay2k )
	ROM_REGION32_LE( 0x400000, "flash0", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash1", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash2", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash3", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash4", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash5", ROMREGION_ERASE00 )

	ROM_REGION32_LE( 0x400000, "flash6", ROMREGION_ERASE00 )
	ROM_LOAD( "flash.u9",  0x0000000, 0x400000, CRC(28d5a2cb) SHA1(69082810849031379018babe6d87c5528e97cfba) )

	ROM_REGION32_LE( 0x400000, "flash7", ROMREGION_ERASE00 )
	ROM_LOAD( "flash.u10", 0x0000000, 0x400000, CRC(4f6c963b) SHA1(775df9c33ff73a85bf478e695f09577d3a07c997) )

	ROM_REGION32_LE( 0x1c00000, "ks0164_bank", 0 )
	ROM_LOAD( "flash.u20", 0x0000000, 0x400000, CRC(cbbb5c11) SHA1(dfc9eeadb077efb86277ff73fbd0e51608c2a2eb) )
	ROM_LOAD( "flash.u21", 0x0400000, 0x400000, CRC(f1dc0d33) SHA1(6e1e4f8da2a8ea59703e8684613c05a05c60d0ac) )
	ROM_LOAD( "flash.u22", 0x0800000, 0x400000, CRC(b783feb6) SHA1(86ca43262a4ccd64f4bd079ad8eaa0a3c113db1f) )
	ROM_LOAD( "flash.u23", 0x0c00000, 0x400000, CRC(fa298e11) SHA1(9c54d4f37fd2ed367b6f9fdb01c361b25b6f2048) )
	ROM_LOAD( "flash.u24", 0x1000000, 0x400000, CRC(29827f0c) SHA1(050e6ed33cf38d5ed45b7d05d039e618f06b5c5b) )
	ROM_LOAD( "flash.u25", 0x1400000, 0x400000, CRC(81b974fa) SHA1(30c11fa926437f144fa6929df2eb85751777bcac) )
	ROM_LOAD( "flash.u26", 0x1800000, 0x400000, CRC(6ff2f3ec) SHA1(8193851b5fdf5248d328f14c0edc2501d12233f3) )

	ROM_REGION( 0x1000, "cpu2", ROMREGION_ERASEFF ) /* PIC */
	ROM_LOAD( "btplay2k_pic",  0x0000, 0x1000, NO_DUMP ) // protected - labeled S831D dgPIX-PR1

	ROM_REGION( 0x117, "pals", 0 )
	ROM_LOAD( "palce16v8h.u11",  0x0000, 0x117, CRC(47e474c9) SHA1(f13ef4050072ab000a45140c180a3b97dacd8675) )
ROM_END


/*

Fishing Maniac 2+
Saero Entertainment, 2000

PCB combo:
VRender0 Minus Rev4 dgPIX Entertainment Inc. 1999
Flash Module Type-A REV2 dgPIX Entertainment Inc. 1999

U100 18 pin socket for the PIC chip is unused

*/
ROM_START( fmaniac2p )
	/* Hyperstone CPU Code & Data */
	ROM_REGION32_LE( 0x400000, "flash0", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash1", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash2", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash3", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash4", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash5", ROMREGION_ERASE00 )

	ROM_REGION32_LE( 0x400000, "flash6", ROMREGION_ERASE00 )
	ROM_LOAD( "flash.u8", 0x0000000, 0x400000, CRC(acc7da30) SHA1(8c4ffbf646777104a0e108c9d1b49446c12adddc) )

	ROM_REGION32_LE( 0x400000, "flash7", ROMREGION_ERASE00 )
	ROM_LOAD( "flash.u9", 0x0000000, 0x400000, CRC(b7f7079d) SHA1(2c97d106a8a28e6fa2a18fb324412533e24f46cc) )

	ROM_REGION( 0x400000, "ks0164", 0 ) /* sound ROM */
	ROM_LOAD16_WORD_SWAP( "flash.u10", 0x0000000, 0x400000, CRC(9a4b15fa) SHA1(64a7c6eff049b15a005a9f2d87b340c81c2ee69c) )

	ROM_REGION( 0x1000, "cpu2", ROMREGION_ERASEFF ) /* PIC */
	// not present
ROM_END

/*

Fishing Maniac 3
Saero Entertainment, 2002

PCB combo:
VRender0 Minus Rev4 dgPIX Entertainment Inc. 1999
Flash Module Type-A REV2 dgPIX Entertainment Inc. 1999

U100 18 pin socket for the PIC chip is unused

*/
ROM_START( fmaniac3 )
	/* Hyperstone CPU Code & Data */
	ROM_REGION32_LE( 0x400000, "flash0", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash1", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash2", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash3", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash4", ROMREGION_ERASE00 )
	ROM_REGION32_LE( 0x400000, "flash5", ROMREGION_ERASE00 )

	ROM_REGION32_LE( 0x400000, "flash6", ROMREGION_ERASE00 )
	ROM_LOAD( "flash.u8", 0x0000000, 0x400000, CRC(dc08a224) SHA1(4d14145eb84ad13674296f81e90b9d60403fa0de) )

	ROM_REGION32_LE( 0x400000, "flash7", ROMREGION_ERASE00 )
	ROM_LOAD( "flash.u9", 0x0000000, 0x400000, CRC(c1fee95f) SHA1(0ed5ed9fa18e7da9242a6df2c210c46de25a2281) )

	ROM_REGION( 0x400000, "ks0164", 0 ) /* sound ROM */
	ROM_LOAD16_WORD_SWAP( "flash.u10", 0x0000000, 0x400000, CRC(dfeb91a0) SHA1(a4a79073c3f6135957ea8a4a66a9c71a3a39893c) )

	ROM_REGION( 0x1000, "cpu2", ROMREGION_ERASEFF ) /* PIC */
	// not present
ROM_END

void dgpix_typea_state::init_elfin()
{
	u8 *rom = memregion("flash7")->base();

	rom[BYTE4_XOR_LE(0x3a9e94)] = 0;
	rom[BYTE4_XOR_LE(0x3a9e95)] = 3;
	rom[BYTE4_XOR_LE(0x3a9e96)] = 0;
	rom[BYTE4_XOR_LE(0x3a9e97)] = 3;
	rom[BYTE4_XOR_LE(0x3a9e98)] = 0;
	rom[BYTE4_XOR_LE(0x3a9e99)] = 3;
}

void dgpix_typea_state::init_jumpjump()
{
	u8 *rom = memregion("flash7")->base();

	rom[BYTE4_XOR_LE(0x3a829a)] = 0;
	rom[BYTE4_XOR_LE(0x3a829b)] = 3;
	rom[BYTE4_XOR_LE(0x3a829c)] = 0;
	rom[BYTE4_XOR_LE(0x3a829d)] = 3;
	rom[BYTE4_XOR_LE(0x3a829e)] = 0;
	rom[BYTE4_XOR_LE(0x3a829f)] = 3;
}

void dgpix_typea_state::init_xfiles()
{
	u8 *rom = memregion("flash7")->base();

	rom[BYTE4_XOR_LE(0x3a9a2a)] = 0;
	rom[BYTE4_XOR_LE(0x3a9a2b)] = 3;
	rom[BYTE4_XOR_LE(0x3a9a2c)] = 0;
	rom[BYTE4_XOR_LE(0x3a9a2d)] = 3;
	rom[BYTE4_XOR_LE(0x3a9a2e)] = 0;
	rom[BYTE4_XOR_LE(0x3a9a2f)] = 3;
}

void dgpix_typea_state::init_xfilesk()
{
	u8 *rom = memregion("flash7")->base();

	rom[BYTE4_XOR_LE(0x3aa92e)] = 0;
	rom[BYTE4_XOR_LE(0x3aa92f)] = 3;
	rom[BYTE4_XOR_LE(0x3aa930)] = 0;
	rom[BYTE4_XOR_LE(0x3aa931)] = 3;
	rom[BYTE4_XOR_LE(0x3aa932)] = 0;
	rom[BYTE4_XOR_LE(0x3aa933)] = 3;

//  protection related ?
//  m_maincpu->space(AS_PROGRAM).nop_read(0xf0c8b440, 0xf0c8b447);
}

void dgpix_typea_state::init_kdynastg()
{
	u8 *rom = memregion("flash7")->base();

	rom[BYTE4_XOR_LE(0x3aaa10)] = 0; // 129f0 - nopped call
	rom[BYTE4_XOR_LE(0x3aaa11)] = 3;
	rom[BYTE4_XOR_LE(0x3aaa12)] = 0;
	rom[BYTE4_XOR_LE(0x3aaa13)] = 3;
	rom[BYTE4_XOR_LE(0x3aaa14)] = 0;
	rom[BYTE4_XOR_LE(0x3aaa15)] = 3;

	rom[BYTE4_XOR_LE(0x3a45c8)] = 0; // c5a8 - added ret
	rom[BYTE4_XOR_LE(0x3a45c9)] = 5;

//  protection related ?
//  m_maincpu->space(AS_PROGRAM).nop_read(0x12341234, 0x12341243);
}

void dgpix_bmkey_state::init_letsdnce()
{
	u8 *rom = memregion("flash7")->base();

	rom[BYTE4_XOR_LE(0x3a9eb2)] = 0;
	rom[BYTE4_XOR_LE(0x3a9eb3)] = 3;
	rom[BYTE4_XOR_LE(0x3a9eb4)] = 0;
	rom[BYTE4_XOR_LE(0x3a9eb5)] = 3;
	rom[BYTE4_XOR_LE(0x3a9eb6)] = 0;
	rom[BYTE4_XOR_LE(0x3a9eb7)] = 3;
}

void dgpix_bmkey_state::init_btplay2k()
{
	u8 *rom = memregion("flash7")->base();

	rom[BYTE4_XOR_LE(0x3a7914)] = 0;
	rom[BYTE4_XOR_LE(0x3a7915)] = 3;
	rom[BYTE4_XOR_LE(0x3a7916)] = 0;
	rom[BYTE4_XOR_LE(0x3a7917)] = 3;
	rom[BYTE4_XOR_LE(0x3a7918)] = 0;
	rom[BYTE4_XOR_LE(0x3a7919)] = 3;
}

} // anonymous namespace


GAME( 1999, elfin,     0,      dgpix,          dgpix,    dgpix_typea_state, init_elfin,     ROT0, "dgPIX Entertainment Inc.", "Elfin",                             MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1999, jumpjump,  0,      dgpix,          dgpix,    dgpix_typea_state, init_jumpjump,  ROT0, "dgPIX Entertainment Inc.", "Jump Jump",                         MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1999, xfiles,    0,      dgpix,          dgpix,    dgpix_typea_state, init_xfiles,    ROT0, "dgPIX Entertainment Inc.", "The X-Files",                       MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1999, xfilesk,   xfiles, dgpix,          dgpix,    dgpix_typea_state, init_xfilesk,   ROT0, "dgPIX Entertainment Inc.", "The X-Files (Censored, Korea)",     MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1999, kdynastg,  0,      dgpix_kdynastg, dgpix,    dgpix_typea_state, init_kdynastg,  ROT0, "EZ Graphics",              "King of Dynast Gear (version 1.8)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1999, letsdnce,  0,      dgpix,          letsdnce, dgpix_bmkey_state, init_letsdnce,  ROT0, "dgPIX Entertainment Inc.", "Let's Dance",                       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2000, btplay2k,  0,      dgpix,          btplay2k, dgpix_bmkey_state, init_btplay2k,  ROT0, "dgPIX Entertainment Inc.", "Beat Player 2000",                  MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2000, fmaniac2p, 0,      dgpix,          dgpix,    dgpix_typea_state, empty_init,     ROT0, "Saero Entertainment",      "Fishing Maniac 2+",                 MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 2002, fmaniac3,  0,      dgpix,          dgpix,    dgpix_typea_state, empty_init,     ROT0, "Saero Entertainment",      "Fishing Maniac 3",                  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
