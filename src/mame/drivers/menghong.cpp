// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/****************************************************************************

    Sealy HW running on VRender0+ SoC

    driver by Angelo Salese, based off original crystal.cpp by ElSemi

    TODO:
    - HY04 protection (controls tile RNG at very least)
    - 8bpp colors are washed, data from flash ROMs is XORed with contents
      of NVRAM area 0x1400070b-80f in menghong, might be shared with 
	  HY04 as well.
    - EEPROM hookup;
    - extract password code when entering test mode in-game;

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
#include "machine/nvram.h"
#include "machine/eepromser.h"
#include "machine/vrender0.h"
#include "machine/timer.h"
#include "emupal.h"

#include <algorithm>

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
//		m_nvram(*this, "nvram"),
		m_ds1302(*this, "rtc"),
		m_eeprom(*this, "eeprom")
	{ }


	void crzyddz2(machine_config &config);
	void menghong(machine_config &config);

private:
	/* memory pointers */
	required_shared_ptr<uint32_t> m_workram;
	optional_region_ptr<uint32_t> m_flash;
	optional_memory_bank m_mainbank;

	/* devices */
	required_device<se3208_device> m_maincpu;
	required_device<vrender0soc_device> m_vr0soc;
//	required_device<nvram_device> m_nvram;
	required_device<ds1302_device> m_ds1302;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;

	uint32_t    m_Bank;
	uint32_t    m_maxbank;
	uint32_t    m_FlashCmd;

	DECLARE_WRITE32_MEMBER(Banksw_w);
	DECLARE_READ32_MEMBER(FlashCmd_r);
	DECLARE_WRITE32_MEMBER(FlashCmd_w);

	DECLARE_READ32_MEMBER(crzyddz2_key_r);

	IRQ_CALLBACK_MEMBER(icallback);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	void menghong_mem(address_map &map);
	void crzyddz2_mem(address_map &map);

	// PIO
	DECLARE_READ32_MEMBER(PIOldat_r);
	uint32_t m_PIO;
	DECLARE_WRITE32_MEMBER(crzyddz2_PIOldat_w);
	DECLARE_READ32_MEMBER(crzyddz2_PIOedat_r);
	uint8_t m_crzyddz2_prot;
	
	DECLARE_READ8_MEMBER(menghong_shared_r);
	DECLARE_WRITE8_MEMBER(menghong_shared_w);
	DECLARE_READ8_MEMBER(crzyddz2_shared_r);
	DECLARE_WRITE8_MEMBER(crzyddz2_shared_w);
	uint8_t *m_sharedram;
};



IRQ_CALLBACK_MEMBER(menghong_state::icallback)
{
	return m_vr0soc->irq_callback();
}

WRITE32_MEMBER(menghong_state::Banksw_w)
{
	m_Bank = (data >> 1) & 7;
	m_mainbank->set_entry(m_Bank);
}

READ32_MEMBER(menghong_state::FlashCmd_r)
{
	if ((m_FlashCmd & 0xff) == 0xff)
	{
		if (m_Bank < m_maxbank)
		{
			uint32_t *ptr = (uint32_t*)(m_mainbank->base());
			return ptr[0];
		}
		else
			return 0xffffffff;
	}
	if ((m_FlashCmd & 0xff) == 0x90)
	{
		if (m_Bank < m_maxbank)
			return 0x00180089;  //Intel 128MBit
		else
			return 0xffffffff;
	}
	return 0;
}

WRITE32_MEMBER(menghong_state::FlashCmd_w)
{
	m_FlashCmd = data;
}



// Crazy Dou Di Zhu II
// To do: HY04 (pic?) protection, 93C46 hookup

READ8_MEMBER(menghong_state::menghong_shared_r)
{	
	return m_sharedram[offset];
}

WRITE8_MEMBER(menghong_state::menghong_shared_w)
{
	m_sharedram[offset] = data;
	
	if (offset == 0x2a0)
	{
		if (data == 0x09)
		{
			// enables game settings by pressing start on password screen
			m_sharedram[0x485] = 0x02;
			
			// start at 0x140071b, up to 0x806, rolls back at 0x70c
			// since we don't have any reference at the moment a plaintext attack is possible with:
			// 1. NOP the XOR opcode at 0x207f544 
			// 2. take whatever is the original value of the supposedly transparent color on a 
			//    given texture palette bank
			// 3. look back in the stack area for where the value is originally loaded
			// 4. soft reset the machine and checkout the address where this value should belong
			//    in this area
			// 5. finally check back if everything is right by injecting the given triplet in this
			//    area, being careful about the inverted Blue and Red from stack to this
			// Note: In the end this big table will be nuked in favour of an hand crafted ROM
			
			// operator mode big text
			m_sharedram[0x70e] = 0x2c;
			m_sharedram[0x70f] = 0xe6;
			m_sharedram[0x710] = 0x05;
			
			// title screen (overlaps with insert coin)
			m_sharedram[0x720] = 0xc0;
			// insert coin
			m_sharedram[0x721] = 0x3a; 
			m_sharedram[0x722] = 0xfb; 
			m_sharedram[0x723] = 0x19;

			// transparency on "coin" outline / blinking on "insert"
			m_sharedram[0x727] = 0x85;
			m_sharedram[0x728] = 0x0d;
			m_sharedram[0x729] = 0xfd;

			m_sharedram[0x72c] = 0xc0;
			m_sharedram[0x72d] = 0xb4;
			m_sharedram[0x72e] = 0xdc;

			m_sharedram[0x731] = 0xb3;
			m_sharedram[0x732] = 0x2e;
			m_sharedram[0x733] = 0xa7;

			// 3rd girl (yellow dress)
			m_sharedram[0x760] = 0x8d;
			m_sharedram[0x761] = 0x61;
			m_sharedram[0x762] = 0xe0;
			
			// 3rd girl shading
			m_sharedram[0x76c] = 0x47 ^ 0xff;
			m_sharedram[0x76d] = 0x75 ^ 0xff;
			m_sharedram[0x76e] = 0xdb ^ 0xff;

			// 1st girl (yellow necklace, violet dress)
			m_sharedram[0x770] = 0x31;
			m_sharedram[0x771] = 0x7f;
			m_sharedram[0x772] = 0x65;

			// 1st girl shading
			m_sharedram[0x77c] = 0xb5 ^ 0xff;
			m_sharedram[0x77d] = 0xd3 ^ 0xff;
			m_sharedram[0x77e] = 0xc7 ^ 0xff;
			// power up girl shading, shared with one above
			m_sharedram[0x77f] = 0x13 ^ 0xff;
			m_sharedram[0x780] = 0x85 ^ 0xff;
			// power up girl transparency, shared with one below
			m_sharedram[0x781] = 0x40;
			m_sharedram[0x782] = 0x50;
			// attract mode gals, shading
			m_sharedram[0x783] = 0x15 ^ 0xff;
			m_sharedram[0x784] = 0x14 ^ 0xff;
			m_sharedram[0x785] = 0x16 ^ 0xff;
			
			// attract mode gals, transparency
			m_sharedram[0x786] = 0x9e;
			m_sharedram[0x787] = 0x69;
			m_sharedram[0x788] = 0x77;

			// Sealy Logo
			m_sharedram[0x78e] = 0x8c;
			m_sharedram[0x78f] = 0x10;
			m_sharedram[0x790] = 0xac;

			// power-up on draw
			m_sharedram[0x791] = 0xd6;
			m_sharedram[0x792] = 0x0d; 
			m_sharedram[0x793] = 0x39;

			// gameplay credit/bet count (3rd value matches following)
			m_sharedram[0x794] = 0x70;
			m_sharedram[0x795] = 0x0b;
			// gals on title screen, shading
			m_sharedram[0x796] = 0xd5 ^ 0xff;
			m_sharedram[0x797] = 0x11 ^ 0xff;
			m_sharedram[0x798] = 0x94 ^ 0xff;
			
			// gals on title screen, transparency
			m_sharedram[0x799] = 0x03;
			m_sharedram[0x79a] = 0xc2;
			m_sharedram[0x79b] = 0xe3;
			
			// pre-title screen attract mode text
			m_sharedram[0x79e] = 0xbf;
			m_sharedram[0x79f] = 0xa9;
			m_sharedram[0x7a0] = 0x4c;

			// test mode
			m_sharedram[0x7a1] = 0xb1;
			m_sharedram[0x7a2] = 0x49;
			m_sharedram[0x7a3] = 0x19;

			// operator mode 4th item header kanji
			m_sharedram[0x7a4] = 0x0e;
			m_sharedram[0x7a5] = 0x71;
			m_sharedram[0x7a6] = 0x3a;

			// big/small kanji
			m_sharedram[0x7b9] = 0xa3;
			m_sharedram[0x7ba] = 0x1e;
			m_sharedram[0x7bb] = 0x19;

			// gameplay: Kanji for discard tile
			m_sharedram[0x7be] = 0x72;
			m_sharedram[0x7bf] = 0x22;
			m_sharedram[0x7c0] = 0x2a;

			// "WIN" text on big/small
			m_sharedram[0x7ce] = 0x6d;
			m_sharedram[0x7cf] = 0x93;
			m_sharedram[0x7d0] = 0xb2;

			// 2nd girl (with pearl necklace)
			// 3d99800
			m_sharedram[0x7d3] = 0xa3;
			m_sharedram[0x7d4] = 0x11;
			m_sharedram[0x7d5] = 0x83;
			
			// operator mode numbers
			m_sharedram[0x7dc] = 0x6a;
			m_sharedram[0x7dd] = 0xa5;
			m_sharedram[0x7de] = 0x39;
			// 2nd girl shading
			m_sharedram[0x7df] = 0x61 ^ 0xff;
			m_sharedram[0x7e0] = 0xc2 ^ 0xff;
			m_sharedram[0x7e1] = 0x95 ^ 0xff;

			// hand text descriptions
			m_sharedram[0x7e8] = 0x16;
			m_sharedram[0x7e9] = 0xb2;
			m_sharedram[0x7ea] = 0xa3;
			
			// tile drawing hand
			m_sharedram[0x7f1] = 0x00;
			m_sharedram[0x7f2] = 0x12;
			m_sharedram[0x7f3] = 0x9a;
			
			// input letters, gameplay
			m_sharedram[0x7fc] = 0x3f;
			m_sharedram[0x7fd] = 0x46;
			m_sharedram[0x7fe] = 0xfd;
		}
	}
}

READ8_MEMBER(menghong_state::crzyddz2_shared_r)
{
	return m_sharedram[offset];
}

WRITE8_MEMBER(menghong_state::crzyddz2_shared_w)
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

READ32_MEMBER(menghong_state::PIOldat_r)
{
	return m_PIO;
}

WRITE32_MEMBER(menghong_state::crzyddz2_PIOldat_w)
{
	COMBINE_DATA(&m_PIO);
	//uint32_t RST = data & 0x01000000;
	//uint32_t CLK = data & 0x02000000;
	//uint32_t DAT = data & 0x10000000;

//  m_eeprom->cs_write(RST ? 1 : 0);
//  m_eeprom->di_write(DAT ? 1 : 0);
//  m_eeprom->clk_write(CLK ? 1 : 0);

	if (ACCESSING_BITS_8_15)
	{
		int mux = (m_PIO >> 8) & 0x1f;
		if (mux == 0x1f)
		{
			m_crzyddz2_prot = ((m_PIO >> 8) & 0xc0) ^ 0x40;
			logerror("%s: PIO = %08x, prot = %02x\n", machine().describe_context(), m_PIO, m_crzyddz2_prot);
		}
	}
}

READ32_MEMBER(menghong_state::crzyddz2_PIOedat_r)
{
	return 0;//m_eeprom->do_read();
}

READ32_MEMBER(menghong_state::crzyddz2_key_r)
{
	static const char *const key_names[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4" };

	int mux = (m_PIO >> 8) & 0x1f;

	uint8_t data = 0x3f;
	for (int i = 0; i < sizeof(key_names)/sizeof(key_names[0]); ++i)
		if (!BIT(mux,i))
			data =  ioport(key_names[i])->read();

/*
crzyddz2    in      out
            00      40
            40      00
            c0      80
*/
// menghong Sealy logo pal offset is at 0x3ea7400, relevant code is at 2086034
//  m_crzyddz2_prot = (m_PIO >> 8) & 0xc0) ^ 0x40;
	m_crzyddz2_prot = (machine().rand() & 0xc0);

	return 0xffffff00 | data | m_crzyddz2_prot;
}

void menghong_state::menghong_mem(address_map &map)
{
	map(0x00000000, 0x003fffff).rom().nopw();

	map(0x01280000, 0x01280003).w(FUNC(menghong_state::Banksw_w));
//	map(0x01400000, 0x0140ffff).ram().share("nvram");
	map(0x01400000, 0x0140ffff).rw(FUNC(menghong_state::menghong_shared_r), FUNC(menghong_state::menghong_shared_w));
	map(0x01500000, 0x01500003).portr("P1_P2");
	map(0x01500004, 0x01500007).r(FUNC(menghong_state::crzyddz2_key_r));
	map(0x01500008, 0x0150000b).portr("SYSTEM");

	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));
	map(0x01802004, 0x01802007).rw(FUNC(menghong_state::PIOldat_r), FUNC(menghong_state::crzyddz2_PIOldat_w));
	map(0x01802008, 0x0180200b).r(FUNC(menghong_state::crzyddz2_PIOedat_r));

	map(0x02000000, 0x027fffff).ram().share("workram");

	map(0x03000000, 0x04ffffff).m(m_vr0soc, FUNC(vrender0soc_device::audiovideo_map));

	map(0x05000000, 0x05ffffff).bankr("mainbank");
	map(0x05000000, 0x05000003).rw(FUNC(menghong_state::FlashCmd_r), FUNC(menghong_state::FlashCmd_w));
}

void menghong_state::crzyddz2_mem(address_map &map)
{
	menghong_mem(map);
	map(0x01400000, 0x0140ffff).rw(FUNC(menghong_state::crzyddz2_shared_r), FUNC(menghong_state::crzyddz2_shared_w));
}

void menghong_state::machine_start()
{
	m_sharedram = auto_alloc_array_clear(machine(), uint8_t, 0x10000);
	
	if (m_mainbank)
	{
		m_maxbank = (m_flash) ? m_flash.bytes() / 0x1000000 : 0;
		uint8_t *dummy_region = auto_alloc_array(machine(), uint8_t, 0x1000000);
		std::fill_n(&dummy_region[0], 0x1000000, 0xff); // 0xff Filled at Unmapped area
		uint8_t *ROM = (m_flash) ? (uint8_t *)&m_flash[0] : dummy_region;
		for (int i = 0; i < 8; i++)
		{
			if ((i < m_maxbank))
				m_mainbank->configure_entry(i, ROM + i * 0x1000000);
			else
				m_mainbank->configure_entry(i, dummy_region);
		}
	}

	save_item(NAME(m_Bank));
	save_item(NAME(m_FlashCmd));
	save_item(NAME(m_PIO));
}

void menghong_state::machine_reset()
{
	m_Bank = 0;
	m_mainbank->set_entry(m_Bank);
	m_FlashCmd = 0xff;

	m_crzyddz2_prot = 0x00;
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

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_START1         ) // start (secret code screen)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_SERVICE2       ) // .. 2  (next secret code / stats)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_SERVICE        ) // .. 1  (secret code screen / service mode)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_SERVICE3       ) // .. 4  (exit secret screen / clear credits)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_SERVICE4       ) //       (reset and clear ram?)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN        )
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
	m_maincpu->set_irq_acknowledge_callback(FUNC(menghong_state::icallback));

	// HY04 running at 8 MHz

//	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);

	VRENDER0_SOC(config, m_vr0soc, 14318180 * 3);
	m_vr0soc->set_host_cpu_tag(m_maincpu);
	m_vr0soc->set_external_vclk(28636360); // Assumed from the only available XTal on PCB

	DS1302(config, m_ds1302, 32.768_kHz_XTAL);
	EEPROM_93C46_16BIT(config, "eeprom");
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
ROM_END

ROM_START( crzyddz2 )
	ROM_REGION32_LE( 0x1000000, "flash", 0 ) // Flash
	ROM_LOAD( "rom.u48", 0x000000, 0x1000000, CRC(0f3a1987) SHA1(6cad943846c79db31226676c7391f32216cfff79) )

	ROM_REGION( 0x0400000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "27c322.u49", 0x000000, 0x0400000, CRC(b3177f39) SHA1(2a28bf8045bd2e053d88549b79fbc11f30ef9a32) ) // 1ST AND 2ND HALF IDENTICAL

	ROM_REGION( 0x4280, "pic", 0 ) // hy04
	ROM_LOAD("hy04", 0x000000, 0x4280, NO_DUMP )
ROM_END

GAME( 2004?,menghong, 0,        menghong, crzyddz2, menghong_state, empty_init,    ROT0, "Sealy", "Meng Hong Lou", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
GAME( 2006, crzyddz2, 0,        crzyddz2, crzyddz2, menghong_state, empty_init,    ROT0, "Sealy", "Crazy Dou Di Zhu II", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
