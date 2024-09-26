// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/****************************************************************************

    Sealy HW running on VRender0+ SoC

    driver by Angelo Salese, based off original crystal.cpp by ElSemi

    TODO:
    - HY04 protection (controls tile RNG, 8bpp colors, a few program flow bits)
    - 8bpp colors are washed, data from flash ROMs is XORed with contents
      of NVRAM area 0x1400070b-80f in menghong, might be shared with
      HY04 as well.
    - EEPROM hookup;
    - extract password code when entering test mode in-game (assuming the
      0x485 workaround isn't enough);

=============================================================================

Crazy Dou Di Zhu II
Sealy, 2006

PCB Layout
----------

070405-fd-VER1.2
|--------------------------------------|
|       PAL        27C322.U36          |
|                               BATTERY|
|    M59PW1282     62256  14.31818MHz  |
|                             W9864G66 |
|                                      |
|J                     VRENDERZERO+    |
|A            W9864G66                 |
|M                            W9864G66 |
|M              8MHz                   |
|A    HY04    0260F8A                  |
|                     28.63636MHz      |
|                                      |
|                 VR1       TLDA1311   |
|                               TDA1519|
|  18WAY                  VOL  10WAY   |
|--------------------------------------|
Notes:
      0260F8A   - unknown TQFP44
      HY04      - rebadged DIP8 PIC - type unknown *
      W9864G66  - Winbond 64MBit DRAM
      M59PW1282 - ST Microelectronics 128MBit SOP44 FlashROM.
                  This is two 64MB SOP44 ROMs in one package

* The pins are:
  1 ground
  2 nothing
  3 data (only active for 1/4 second when the playing cards or "PASS" shows in game next to each player)
  4 nothing
  5 nothing
  6 clock
  7 +5V (could be VPP for programming voltage)
  8 +5V

=====

Meng Hong Lou (Dream of the Red Chamber)
Sealy, 2004?

Red PCB, very similar to crzyddz2

****************************************************************************/

#include "emu.h"
#include "cpu/se3208/se3208.h"
#include "machine/ds1302.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "machine/vrender0.h"
#include "emupal.h"

#include <algorithm>


namespace {

class menghong_state : public driver_device
{
public:
	menghong_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_flash(*this, "flash"),
		m_mainbank(*this, "mainbank"),
		m_maincpu(*this, "maincpu"),
		m_vr0soc(*this, "vr0soc"),
//      m_nvram(*this, "nvram"),
		m_ds1302(*this, "rtc"),
		m_eeprom(*this, "eeprom"),
		m_prot_data(*this, "pic_data"),
		m_keys(*this, "KEY%u", 0U)
	{ }


	void crzyddz2(machine_config &config);
	void menghong(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint32_t> m_workram;
	required_region_ptr<uint32_t> m_flash;
	required_memory_bank m_mainbank;

	// devices
	required_device<se3208_device> m_maincpu;
	required_device<vrender0soc_device> m_vr0soc;
//  required_device<nvram_device> m_nvram;
	required_device<ds1302_device> m_ds1302;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_region_ptr <uint8_t> m_prot_data;

	// inputs
	required_ioport_array<5> m_keys;

	uint32_t m_bank;
	uint32_t m_maxbank;
	uint32_t m_flashcmd;

	void banksw_w(uint32_t data);
	uint32_t flashcmd_r();
	void flashcmd_w(uint32_t data);

	uint32_t key_r();

	void menghong_mem(address_map &map) ATTR_COLD;
	void crzyddz2_mem(address_map &map) ATTR_COLD;

	// pio
	uint32_t pioldat_r();
	uint32_t m_pio;
	void pioldat_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t pioedat_r();
	uint8_t m_prot;

	uint8_t menghong_shared_r(offs_t offset);
	void menghong_shared_w(offs_t offset, uint8_t data);
	uint8_t crzyddz2_shared_r(offs_t offset);
	void crzyddz2_shared_w(offs_t offset, uint8_t data);
	std::unique_ptr<uint8_t []> m_sharedram;
};



void menghong_state::banksw_w(uint32_t data)
{
	m_bank = (data >> 1) & 7;
	m_mainbank->set_entry(m_bank);
}

uint32_t menghong_state::flashcmd_r()
{
	if ((m_flashcmd & 0xff) == 0xff)
	{
		if (m_bank < m_maxbank)
		{
			uint32_t *ptr = (uint32_t*)(m_mainbank->base());
			return ptr[0];
		}
		else
			return 0xffffffff;
	}
	if ((m_flashcmd & 0xff) == 0x90)
	{
		if (m_bank < m_maxbank)
			return 0x00180089;  //Intel 128MBit
		else
			return 0xffffffff;
	}
	return 0;
}

void menghong_state::flashcmd_w(uint32_t data)
{
	m_flashcmd = data;
}



// Crazy Dou Di Zhu II
// To do: HY04 (pic?) protection, 93C46 hookup

uint8_t menghong_state::menghong_shared_r(offs_t offset)
{
	return m_sharedram[offset];
}

void menghong_state::menghong_shared_w(offs_t offset, uint8_t data)
{
	m_sharedram[offset] = data;

	if (offset == 0x2a0)
	{
		if (data == 0x09)
		{
			// enables game settings by pressing start on password screen
			m_sharedram[0x485] = 0x02;

			// start at 0x140071b, up to 0x806, rolls back at 0x70c
			// we conveniently use an handcrafted ROM here, created by guessing colors from
			// transparencies and shading.
			// This will be useful for comparison when the actual PIC data will be extracted.
			for (int i=0;i<0x100;i++)
				m_sharedram[i+0x70c] = m_prot_data[i];

			// MCU also has a part in providing RNG
			// hold service1 while selecting makes the user select "a set" (location in NVRAM tbd)
			// 0x14005ca player 1 tiles
			// 0x14005d9 player 1 discard pond
			// 0x14005f9 available tiles
			// 0x14005a5 cpu tiles
			// 0x14005b4 cpu discard pond
		}
	}
}

uint8_t menghong_state::crzyddz2_shared_r(offs_t offset)
{
	return m_sharedram[offset];
}

void menghong_state::crzyddz2_shared_w(offs_t offset, uint8_t data)
{
	m_sharedram[offset] = data;

	// State machine is unconfirmed
	if (offset == 0x7e3)
	{
		switch(data)
		{
			case 0x00:
				m_sharedram[0x650] = 0x00; // prints 93c46 error otherwise
				m_sharedram[0x651] = 0x03; // PC=2012188
				break;
			case 0xbb:
				// this actually affects color again, game checksums the NVRAM contents
				// at PC=0x2011f9a, expecting a value of 0x7ebe otherwise locks up
				// after Sealy logo. Every single value is added to the routine and left
				// shifted by 1 (including the two values above)
				for(int i=0;i<0x3f;i++)
					m_sharedram[i+0x652] = 0xff;
				m_sharedram[0x691] = 0x9b;
				break;
			// additional locking protection is also applied with RNG feature
			// PC=0x2036756 tight loops if R1=0
		}
	}
}

uint32_t menghong_state::pioldat_r()
{
	return m_pio;
}

void menghong_state::pioldat_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pio);
	//uint32_t RST = data & 0x01000000;
	//uint32_t CLK = data & 0x02000000;
	//uint32_t DAT = data & 0x10000000;

//  m_eeprom->cs_write(RST ? 1 : 0);
//  m_eeprom->di_write(DAT ? 1 : 0);
//  m_eeprom->clk_write(CLK ? 1 : 0);

	if (ACCESSING_BITS_8_15)
	{
		int mux = (m_pio >> 8) & 0x1f;
		if (mux == 0x1f)
		{
			m_prot = ((m_pio >> 8) & 0xc0) ^ 0x40;
			logerror("%s: pio = %08x, prot = %02x\n", machine().describe_context(), m_pio, m_prot);
		}
	}
}

uint32_t menghong_state::pioedat_r()
{
	return 0;//m_eeprom->do_read();
}

uint32_t menghong_state::key_r()
{
	int mux = (m_pio >> 8) & 0x1f;

	uint8_t data = 0x3f;
	for (int i = 0; i < m_keys.size(); ++i)
		if (!BIT(mux,i))
			data = m_keys[i]->read();

/*
crzyddz2    in      out
            00      40
            40      00
            c0      80
*/
// menghong Sealy logo pal offset is at 0x3ea7400, relevant code is at 2086034
//  m_prot = (m_pio >> 8) & 0xc0) ^ 0x40;
	m_prot = (machine().rand() & 0xc0);

	return 0xffffff00 | data | m_prot;
}

void menghong_state::menghong_mem(address_map &map)
{
	map(0x00000000, 0x003fffff).rom().nopw();

	map(0x01280000, 0x01280003).w(FUNC(menghong_state::banksw_w));
//  map(0x01400000, 0x0140ffff).ram().share("nvram");
	map(0x01400000, 0x0140ffff).rw(FUNC(menghong_state::menghong_shared_r), FUNC(menghong_state::menghong_shared_w));
	map(0x01500000, 0x01500003).portr("P1_P2");
	map(0x01500004, 0x01500007).r(FUNC(menghong_state::key_r));
	map(0x01500008, 0x0150000b).portr("SYSTEM");

	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));
	map(0x01802004, 0x01802007).rw(FUNC(menghong_state::pioldat_r), FUNC(menghong_state::pioldat_w));
	map(0x01802008, 0x0180200b).r(FUNC(menghong_state::pioedat_r));

	map(0x02000000, 0x027fffff).ram().share(m_workram);

	map(0x03000000, 0x04ffffff).m(m_vr0soc, FUNC(vrender0soc_device::audiovideo_map));

	map(0x05000000, 0x05ffffff).bankr(m_mainbank);
	map(0x05000000, 0x05000003).rw(FUNC(menghong_state::flashcmd_r), FUNC(menghong_state::flashcmd_w));
}

void menghong_state::crzyddz2_mem(address_map &map)
{
	menghong_mem(map);
	map(0x01400000, 0x0140ffff).rw(FUNC(menghong_state::crzyddz2_shared_r), FUNC(menghong_state::crzyddz2_shared_w));
}

void menghong_state::machine_start()
{
	m_sharedram = make_unique_clear<uint8_t []>(0x10000);

	m_maxbank = (m_flash) ? m_flash.bytes() / 0x1000000 : 0;
	std::unique_ptr<uint8_t[]> dummy_region = std::make_unique<uint8_t[]>(0x1000000);
	std::fill_n(&dummy_region[0], 0x1000000, 0xff); // 0xff Filled at Unmapped area
	uint8_t *rom = (m_flash) ? (uint8_t *)&m_flash[0] : dummy_region.get();
	for (int i = 0; i < 8; i++)
	{
		if (i < m_maxbank)
			m_mainbank->configure_entry(i, rom + i * 0x1000000);
		else
			m_mainbank->configure_entry(i, dummy_region.get());
	}

	save_item(NAME(m_bank));
	save_item(NAME(m_flashcmd));
	save_item(NAME(m_pio));
	save_pointer(NAME(m_sharedram), 0x10000);
}

void menghong_state::machine_reset()
{
	m_bank = 0;
	m_mainbank->set_entry(m_bank);
	m_flashcmd = 0xff;

	m_prot = 0x00;
}

static INPUT_PORTS_START(crzyddz2)
	PORT_START("P1_P2") // 1500002 & 1500000
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) // up
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) // down  (next secret code)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) // left  (inc secret code)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) // right (dec secret code)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1        ) // A
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2        ) // B
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3        ) // C     (bet)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON4        ) // D
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START1 ) // start (secret code screen)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Operator Mode")
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_SERVICE ) // .. 1  (secret code screen / service mode)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_SERVICE2 ) // not service mode
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Clear RAM")
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0000ffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x010000, 0x010000, "DSWA" )
	PORT_DIPSETTING(    0x010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x020000, 0x020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x040000, 0x040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x080000, 0x080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x100000, 0x100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x200000, 0x200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x400000, 0x400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x800000, 0x800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// 1500004 (multiplexed by 1802005)
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A         )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E         )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I         )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M         )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN       ) // kan
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) // start?

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B         )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F         )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J         )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N         )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH     ) // ?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET       ) // ? + C

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C         )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G         )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K         )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI       ) // chi
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE     ) // ?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN           ) // nothing

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D         )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H         )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L         )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON       ) // pon
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN           ) // nothing
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN           ) // nothing

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN           ) // nothing
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_RON       ) // ron
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) // ?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN           ) // nothing
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG       ) // big
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL     ) // small + D
INPUT_PORTS_END

void menghong_state::menghong(machine_config &config)
{
	SE3208(config, m_maincpu, 14318180 * 3); // TODO : different between each PCBs
	m_maincpu->set_addrmap(AS_PROGRAM, &menghong_state::menghong_mem);
	m_maincpu->iackx_cb().set(m_vr0soc, FUNC(vrender0soc_device::irq_callback));

	// HY04 running at 8 MHz

//  NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);

	VRENDER0_SOC(config, m_vr0soc, 14318180 * 3);
	m_vr0soc->set_host_cpu_tag(m_maincpu);
	m_vr0soc->set_external_vclk(28636360); // Assumed from the only available XTal on PCB

	DS1302(config, m_ds1302, 32.768_kHz_XTAL);
	EEPROM_93C46_16BIT(config, m_eeprom);
}

void menghong_state::crzyddz2(machine_config &config)
{
	menghong(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &menghong_state::crzyddz2_mem);
}

ROM_START( menghong )
	ROM_REGION32_LE( 0x1000000, "flash", 0 ) // Flash
	ROM_LOAD( "rom.u48", 0x000000, 0x1000000, CRC(e24257c4) SHA1(569d79a61ff6d35100ba5727069363146df9e0b7) )

	ROM_REGION( 0x0400000, "maincpu", 0 )
	ROM_LOAD( "060511_08-01-18.u49",  0x0000000, 0x0200000, CRC(b0c12107) SHA1(b1753757bbdb7d996df563ac6abdc6b46676704b) ) // 27C160
	ROM_RELOAD(                       0x0200000, 0x0200000 )

	ROM_REGION( 0x4280, "pic", 0 ) // hy04
	ROM_LOAD("menghong_hy04", 0x000000, 0x4280, NO_DUMP )

	ROM_REGION( 0x0100, "pic_data", ROMREGION_ERASEFF )
	ROM_LOAD("hy04_fake_data.bin", 0, 0x100, BAD_DUMP CRC(73cc964b) SHA1(39d223c550e38c97135322e43ccabb70f04964b9) )
ROM_END

ROM_START( crzyddz2 )
	ROM_REGION32_LE( 0x1000000, "flash", 0 ) // Flash
	ROM_LOAD( "rom.u48", 0x000000, 0x1000000, CRC(0f3a1987) SHA1(6cad943846c79db31226676c7391f32216cfff79) )

	ROM_REGION( 0x0400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "27c322.u49", 0x000000, 0x0400000, CRC(b3177f39) SHA1(2a28bf8045bd2e053d88549b79fbc11f30ef9a32) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x4280, "pic", 0 ) // hy04
	ROM_LOAD("hy04", 0x000000, 0x4280, NO_DUMP )

	ROM_REGION( 0x0100, "pic_data", ROMREGION_ERASEFF )
ROM_END

} // Anonymous namespace


GAME( 2004?,menghong, 0,        menghong, crzyddz2, menghong_state, empty_init,    ROT0, "Sealy", "Meng Hong Lou", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
GAME( 2006, crzyddz2, 0,        crzyddz2, crzyddz2, menghong_state, empty_init,    ROT0, "Sealy", "Crazy Dou Di Zhu II", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
