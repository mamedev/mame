// license:BSD-3-Clause
// copyright-holders:ElSemi, Angelo Salese
/*
    CRYSTAL SYSTEM by Brezzasoft (2001)
    using VRender0 System on a Chip

    The VRender0 (info at archive.org for http://www.mesdigital.com) chip contains:
        - CPU Core SE3208 (info at www.adc.co.kr) @ 43Mhz
        - 2 DMA chans
        - 4 Timers
        - 32 PIO pins
        - PWM output
        - 32 channels wavetable synth (8bit linear, 16bit linear and 8bit ulaw sample format)
        - Custom 2D video rendering device (texture mapping, alphablend, roz)

    The protection is a PIC device labeled SMART-IC in the ROM board, I'm not sure how
    it exactly works, but it supplies some opcodes that have been replaced with garbage
    in the main program. I don't know if it traps reads and returns the correct ones when
    reading from flash, or if it's interfaced by the main program after copying the program
    from flash to RAM and it provides the addresses and values to patch. I patch the flash
    program with the correct data

    MAME driver by ElSemi
    Additional work and refactoring by Angelo Salese

    TODO:
    - provide NVRAM defaults where applicable;
    - add an actual reset button (helps with inp record/playback);
    - donghaer: needs "raster effect" for 2 players mode split screen, but no
      interrupt is actually provided for the task so apparently not a timer
      related effect;
    - wulybuly: strips off main RAM to texture transfers except for text after
      the first couple of frames;
    - maldaiza: PIC protection.
    - urachamu: some animation timings seems off, like bat hit animation before starting a given game.
      They were actually too fast before adding 30 Hz vblank for interlace mode, even if the game don't
      really read crtc blanking reg or use any other interrupt but the coin ones;
    - urachamu: investigate what CDMA in test mode really do, assuming it's not a dud;

========================================================================================================

The Crystal of Kings
Brezza Soft Corporation (Japan), 2001

This game runs on a small cartridge-based PCB known as the 'Crystal System'
The main PCB is small (approx 6" square) and contains only a few components. All of the processing
work is done by the large IC in the middle of the PCB. The system looks a bit like IGS's PGM System, in
that it's housed in a plastic case and has a single slot for insertion of a game cart. However this
system and the game carts are approx. half the size of the PGM carts.
On bootup, the screen is black and the system outputs a vertical white line on the right side of the screen.
The HSync is approx 20kHz, the screen is out of sync on a standard 15kHz arcade monitor.
After approx. 15-20 seconds, the screen changes to white and has some vertical stripes on it and the HSync
changes to 15kHz. After a few more seconds the game boots to a white screen and a blue 'Brezza Soft' logo.
Without a cart plugged in, the screen stays at the first vertical line screen.

Main PCB Layout
---------------

Brezza Soft MAGIC EYES AMG0110B
  |----------------------------------------------------|
  |TDA1519   VOL     3.6V_BATT  SW1 SW2 SW3            |
|-|                                                    |
|                              DS1233                  |
|                                                |---| |
|                                       GM76256  |   | |
|        DA1133A                        |-----|  |   | |
|                HY57V651620  32.768kHz |MX27L|  |   | |
|                               DS1302  |1000 |  |   | |
|J                  |-------------|     |-----|  |   | |
|A                  |             |LED1          |   | |
|M                  |             |LED2          |CN1| |
|M      HY57V651620 | VRENDERZERO |              |   | |
|A                  |             |              |   | |
|                   |             |              |   | |
|                   |-------------| HY57V651620  |   | |
|                                                |   | |
|              14.31818MHz                       |   | |
|                              PAL               |---| |
|                                    TD62003           |
|-|                                                    |
  | |---------------| DSW(8)                           |
  |-|     CN2       |----------------------------------|
    |---------------|

Notes:
      GM76C256   : Hyundai GM76C256 32k x8 SRAM (SOP28)
      MX27L1000  : Macronix MX27L1000QC-12 128k x8 EEPROM (BIOS, PLCC32)
      PAL        : Atmel ATF16V8-10PC PAL (DIP20)
      HY57V651620: Hyundai HY57V651620 4M x16 SDRAM (SSOP54)
      DA1311A    : Philips DA1311A DAC (SOIC8)
      DS1233     : Dallas DS1233 master reset IC (SOIC4)
      DS1302     : Dallas DS1302 real time clock IC (DIP8)
      VRENDERZERO: MESGraphics VRenderZERO (all-in-one main CPU/graphics/sound, QFP240)
      SW1        : Push button reset switch
      SW2        : Push button service switch
      SW3        : Push button test switch
      TDA1519    : Philips TDA1519 dual 6W stereo power amplifier (SIP9)
      VOL        : Master volume potentiometer
      3.6V_BATT  : 3.6 Volt NiCad battery 65mAh (for RTC)
      TD62003    : Toshiba TD62003 PNP 50V 0.5A quad darlington switch, for driving coin meters (DIP16)
      CN1        : PCI-type slot for extension riser board (for game cart connection at 90 degrees to the main PCB)
      CN2        : IDC 34-way flat cable connector (purpose unknown)
      VSync      : 60Hz


Game cart PCB
-------------
Cart sticker: 'THE CRYSTAL OF KINGS      BCSV0000'
PCB printing: Brezza Soft MAGIC EYES AMG0111B
|------------------------------|
|                              |
|PIC16xxx?                     |
|           U8   U7   U6   U5  |
|3.57945Mhz                    |
|                              |
|                              |
|                              |
|                              |
|74HC138                       |
|           U4   U3   U2   U1  |
|                              |
|                              |
|                              |
|-|                          |-|
  |--------------------------|

Notes:
      The cart PCB is single sided and contains only...
      1x 3.579545MHz crystal
      1x 74HC138 logic chip
      1x 18 pin unknown chip (DIP18, surface scratched but it's probably a PIC16xxx, labelled 'dgSMART-PR3 MAGIC EYES')
      3x Intel E28F128J3A 128MBit surface mounted FlashROMs (TSOP56, labelled 'BREZZASOFT BCSV0004Fxx', xx=01, 02, 03)
         Note: there are 8 spaces total for FlashROMs. Only U1, U2 & U3 are populated in this cart.


*/

#include "emu.h"
#include "cpu/se3208/se3208.h"
#include "machine/ds1302.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "machine/vrender0.h"

#include <algorithm>


namespace {

class crystal_state : public driver_device
{
public:
	crystal_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_reset_patch(*this, "reset_patch"),
		m_flash(*this, "flash"),
		m_mainbank(*this, "mainbank"),
		m_maincpu(*this, "maincpu"),
		m_vr0soc(*this, "vr0soc"),
		m_ds1302(*this, "rtc"),
		m_eeprom(*this, "eeprom"),
		m_dsw(*this, "DSW"),
		m_system(*this, "SYSTEM")
	{ }

	void init_topbladv();
	void init_officeye();
	void init_crysking();
	void init_evosocc();
	void init_donghaer();
	void init_maldaiza();

	void crystal(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint32_t> m_workram;
	optional_shared_ptr<uint32_t> m_reset_patch;
	optional_region_ptr<uint32_t> m_flash;

	optional_memory_bank m_mainbank;

	// devices
	required_device<se3208_device> m_maincpu;
	required_device<vrender0soc_device> m_vr0soc;
	required_device<ds1302_device> m_ds1302;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;

	required_ioport m_dsw;
	required_ioport m_system;

	uint32_t    m_bank;
	uint32_t    m_maxbank;
	uint32_t    m_flashcmd;
	std::unique_ptr<uint8_t[]> m_dummy_region;

	uint32_t system_input_r();
	void banksw_w(uint32_t data);
	uint32_t flashcmd_r();
	void flashcmd_w(uint32_t data);
	void coin_counters_w(uint8_t data);

	void patchreset();
	void crystal_mem(address_map &map) ATTR_COLD;

	// PIO
	uint32_t pioldat_r();
	void pioldat_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t pioedat_r();
	uint32_t m_pio;
};


uint32_t crystal_state::system_input_r()
{
	return ( m_system->read() << 16) | (m_dsw->read()) | 0xff00ff00;
}

void crystal_state::banksw_w(uint32_t data)
{
	m_bank = (data >> 1) & 7;
	m_mainbank->set_entry(m_bank);
}

uint32_t crystal_state::pioldat_r()
{
	return m_pio;
}

// PIO Latched output DATa Register
void crystal_state::pioldat_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t RST = data & 0x01000000;
	uint32_t CLK = data & 0x02000000;
	uint32_t DAT = data & 0x10000000;

	m_ds1302->ce_w(RST ? 1 : 0);
	m_ds1302->io_w(DAT ? 1 : 0);
	m_ds1302->sclk_w(CLK ? 1 : 0);

	COMBINE_DATA(&m_pio);
}

// PIO External DATa Register
uint32_t crystal_state::pioedat_r()
{
	return m_ds1302->io_r() << 28;
}

uint32_t crystal_state::flashcmd_r()
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

void crystal_state::flashcmd_w(uint32_t data)
{
	m_flashcmd = data;
}

void crystal_state::coin_counters_w(uint8_t data)
{
	// Both signals are sent when setting is "1 shooter"
	// Only evosocc and crysking allow the user to change this setting.
	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);
}

void crystal_state::crystal_mem(address_map &map)
{
	map(0x00000000, 0x0001ffff).rom().nopw();

	map(0x01200000, 0x01200003).portr("P1_P2");
	map(0x01200000, 0x01200000).w(FUNC(crystal_state::coin_counters_w));
	map(0x01200004, 0x01200007).portr("P3_P4");
	map(0x01200008, 0x0120000b).r(FUNC(crystal_state::system_input_r));

	map(0x01280000, 0x01280003).w(FUNC(crystal_state::banksw_w));
	map(0x01400000, 0x0140ffff).ram().share("nvram");

	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));
	map(0x01802004, 0x01802007).rw(FUNC(crystal_state::pioldat_r), FUNC(crystal_state::pioldat_w));
	map(0x01802008, 0x0180200b).r(FUNC(crystal_state::pioedat_r));

	// mirror is accessed by donghaer on later levels
	map(0x02000000, 0x027fffff).mirror(0x00800000).ram().share("workram");

	map(0x03000000, 0x04ffffff).m(m_vr0soc, FUNC(vrender0soc_device::audiovideo_map));

	map(0x05000000, 0x05ffffff).bankr("mainbank");
	map(0x05000000, 0x05000003).rw(FUNC(crystal_state::flashcmd_r), FUNC(crystal_state::flashcmd_w));

	map(0x44414f4c, 0x44414f7f).ram().share("reset_patch");
}

void crystal_state::patchreset()
{
	if (!m_reset_patch)
		return;

	//The test menu reset routine seems buggy
	//it reads the reset vector from 0x02000000 but it should be
	//read from 0x00000000. At 0x2000000 there is the bios signature
	//"LOADED VER....", so it jumps to "LOAD" in hex (0x44414F4C)
	//I'll add some code there that makes the game stay in a loop
	//reading the flip register so the idle skip works

/*
Loop1:
    LDI 1,%R2
    LDI     0x30000a6,%R1
    STS %R2,(%R1,0x0)

loop:
    LDI 0x30000a6,%R1
    LDSU    (%R1,0x0),%R2
    CMP     %R2,0x0000
    JNZ loop

    JMP Loop1
*/


#if 1
	static const uint32_t patch[] =
	{
		0x40c0ea01,
		0xe906400a,
		0x40c02a20,
		0xe906400a,
		0xa1d03a20,
		0xdef4d4fa
	};

	memcpy(m_reset_patch, patch, sizeof(patch));
#else
	static const uint8_t patch[] =
	{
		0x01,0xea,0xc0,0x40,0x0a,0x40,0x06,0xe9,
		0x20,0x2a,0xc0,0x40,0x0a,0x40,0x06,0xe9,
		0x20,0x3a,0xd0,0xa1,0xfa,0xd4,0xf4,0xde
	};

	memcpy(m_reset_patch, patch, sizeof(patch));
#endif
}

void crystal_state::machine_start()
{
	patchreset();

	if (m_mainbank)
	{
		m_maxbank = (m_flash) ? m_flash.bytes() / 0x1000000 : 0;
		m_dummy_region = std::make_unique<uint8_t[]>(0x1000000);
		std::fill_n(&m_dummy_region[0], 0x1000000, 0xff); // 0xff filled at unmapped area
		uint8_t *rom = (m_flash) ? (uint8_t *)&m_flash[0] : &m_dummy_region[0];
		for (int i = 0; i < 8; i++)
		{
			if ((i < m_maxbank))
				m_mainbank->configure_entry(i, rom + i * 0x1000000);
			else
				m_mainbank->configure_entry(i, m_dummy_region.get());
		}
	}

	save_item(NAME(m_bank));
	save_item(NAME(m_flashcmd));
	save_item(NAME(m_pio));
}

void crystal_state::machine_reset()
{
	m_bank = 0;
	m_mainbank->set_entry(m_bank);
	m_flashcmd = 0xff;

	patchreset();
}

INPUT_CHANGED_MEMBER(crystal_state::coin_inserted)
{
	if (oldval)
	{
		uint8_t coin_chute = (uint8_t)(uintptr_t)param & 1;
		m_vr0soc->IntReq(coin_chute ? 19 : 12);
	}
}

static INPUT_PORTS_START( crystal )
	PORT_START("P1_P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3_P4")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, crystal_state, coin_inserted, 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, crystal_state, coin_inserted, 1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Pause ) )       PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Free_Play ) )   PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )     PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )     PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )     PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )     PORT_DIPLOCATION("DSW:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )     PORT_DIPLOCATION("DSW:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Test ) )        PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( topbladv )
	PORT_INCLUDE( crystal )

	PORT_MODIFY("P1_P2")
	PORT_BIT( 0x000000f0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P3_P4")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNUSED )

	// TODO: coin 2 insertion is fuzzy, may be BTANB
	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, crystal_state, coin_inserted, 0)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, crystal_state, coin_inserted, 1)
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )
INPUT_PORTS_END


static INPUT_PORTS_START( officeye )
	PORT_INCLUDE( crystal )

	// TODO: player 2 start doesn't work ingame, it only skips attract mode items,
	// or be valid in test mode when prompted to press start
	// lamps test also gives only p1 and p3, and after crediting up only p1 and p3 starts are lighted on ($01320000 address writes)
	// I guess game is currently in 2 players mode, is there anything that enables 3 players mode?
	// notice that this is valid for both urachamu and officeye
	PORT_MODIFY("P1_P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Red")
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Green")
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Blue")
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Red")
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("P3 Red")
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Green")
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("P3 Green")
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Blue")
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3) PORT_NAME("P3 Blue")
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("P3_P4")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0000000f, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( urachamu )
	PORT_INCLUDE( officeye )

	// oddly they reversed red and blue
	PORT_MODIFY("P1_P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Blue")
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Red")
	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Blue")
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3) PORT_NAME("P3 Blue")
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Red")
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("P3 Red")
INPUT_PORTS_END

static INPUT_PORTS_START( wulybuly )
	PORT_INCLUDE( crystal )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Test ) )        PORT_DIPLOCATION("DSW:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )     PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void crystal_state::crystal(machine_config &config)
{
	SE3208(config, m_maincpu, 14318180 * 3); // TODO : different between each PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &crystal_state::crystal_mem);
	m_maincpu->iackx_cb().set(m_vr0soc, FUNC(vrender0soc_device::irq_callback));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	VRENDER0_SOC(config, m_vr0soc, 14318180 * 3);
	m_vr0soc->set_host_cpu_tag(m_maincpu);

	DS1302(config, m_ds1302, 32.768_kHz_XTAL);
}


#define CRYSBIOS \
	ROM_REGION( 0x20000, "maincpu", 0 )  \
	ROM_SYSTEM_BIOS( 0, "amg0110b", "AMG0110B PCB" ) \
	ROMX_LOAD("mx27l1000.u14",  0x000000, 0x020000, CRC(beff39a9) SHA1(b6f6dda58d9c82273f9422c1bd623411e58982cb), ROM_BIOS(0)) \
	ROM_SYSTEM_BIOS( 1, "amg0110d", "AMG0110D PCB" ) /* newer? */ \
	ROMX_LOAD("mx27l1000-alt.u14",  0x000000, 0x020000, CRC(1e8175c8) SHA1(f60c016be2ff11e47b2192acddb92676043af501), ROM_BIOS(1))

ROM_START( crysbios )
	CRYSBIOS

	ROM_REGION32_LE( 0x1000000, "flash", ROMREGION_ERASEFF )
ROM_END

ROM_START( crysking )
	CRYSBIOS

	ROM_REGION32_LE( 0x3000000, "flash", 0 )
	ROM_LOAD("bcsv0004f01.u1",  0x0000000, 0x1000000, CRC(8feff120) SHA1(2ea42fa893bff845b5b855e2556789f8354e9066) )
	ROM_LOAD("bcsv0004f02.u2",  0x1000000, 0x1000000, CRC(0e799845) SHA1(419674ce043cb1efb18303f4cb7fdbbae642ee39) )
	ROM_LOAD("bcsv0004f03.u3",  0x2000000, 0x1000000, CRC(659e2d17) SHA1(342c98f3f695ef4dea8b533612451c4d2fb58809) )
ROM_END

ROM_START( evosocc )
	CRYSBIOS

	ROM_REGION32_LE( 0x3000000, "flash", 0 )
	ROM_LOAD("bcsv0001u01",  0x0000000, 0x1000000, CRC(2581a0ea) SHA1(ee483ac60a3ed00a21cb515974cec4af19916a7d) )
	ROM_LOAD("bcsv0001u02",  0x1000000, 0x1000000, CRC(47ef1794) SHA1(f573706c17d1342b9b7aed9b40b8b648f0bf58db) )
	ROM_LOAD("bcsv0001u03",  0x2000000, 0x1000000, CRC(f396a2ec) SHA1(f305eb10856fb5d4c229a6b09d6a2fb21b24ce66) )
ROM_END

ROM_START( topbladv )
	CRYSBIOS

	ROM_REGION( 0x4300, "pic", 0 ) // pic16c727 - we don't have a core for this
	ROM_LOAD("top_blade_v_pic16c727.bin",  0x000000, 0x4300, CRC(9cdea57b) SHA1(884156085f9e780cdf719aedc2e8a0fd5983613b) )

	ROM_REGION32_LE( 0x1000000, "flash", 0 )
	ROM_LOAD("flash.u1",  0x0000000, 0x1000000, CRC(bd23f640) SHA1(1d22aa2c828642bb7c1dfea4e13f777f95acc701) )
ROM_END

ROM_START( officeye )
	ROM_REGION( 0x20000, "maincpu", 0 ) // bios (not the standard one)
	ROM_LOAD("bios.u14",  0x000000, 0x020000, CRC(ffc57e90) SHA1(6b6a17fd4798dea9c7b880f3063be8494e7db302) )

	ROM_REGION( 0x4280, "pic", 0 ) // pic16f84a - we don't have a core for this
	ROM_LOAD("office_yeo_in_cheon_ha_pic16f84a.bin",  0x000000, 0x4280, CRC(7561cdf5) SHA1(eade592823a110019b4af81a7dc56d01f7d6589f) )

	ROM_REGION32_LE( 0x2000000, "flash", 0 )
	ROM_LOAD("flash.u1",  0x0000000, 0x1000000, CRC(d3f3eec4) SHA1(ea728415bd4906964b7d37f4379a8a3bd42a1c2d) )
	ROM_LOAD("flash.u2",  0x1000000, 0x1000000, CRC(e4f85d0a) SHA1(2ddfa6b3a30e69754aa9d96434ff3d37784bfa57) )
ROM_END

ROM_START( donghaer )
	CRYSBIOS

	ROM_REGION( 0x4280, "pic", 0 ) // pic16f84a - we don't have a core for this (or the dump in this case)
	ROM_LOAD("donghaer_pic16f84a.bin",  0x000000, 0x4280, NO_DUMP )

	ROM_REGION32_LE( 0x2000000, "flash", 0 )
	ROM_LOAD( "u1",           0x0000000, 0x1000000, CRC(61217ad7) SHA1(2593f1356aa850f4f9aa5d00bec822aa59c59224) )
	ROM_LOAD( "u2",           0x1000000, 0x1000000, CRC(6d82f1a5) SHA1(036bd45f0daac1ffeaa5ad9774fc1b56e3c75ff9) )
ROM_END

ROM_START( wulybuly )
	CRYSBIOS

	ROM_REGION( 0x4280, "pic", ROMREGION_ERASEFF ) // empty socket

	ROM_REGION32_LE( 0x1000000, "flash", 0 )
	ROM_LOAD( "u1",           0x0000000, 0x1000000,  CRC(7406f5db) SHA1(dd53afb08d0567241d08d2422c672d429ef9b78f) )

	ROM_REGION( 0x10000, "nvram", 0 ) // otherwise it defaults to completely invalid coinage
	ROM_LOAD( "nvram",  0x0000000, 0x10000, CRC(5908b829) SHA1(c23180f1b3a80e29c61e25584ffa929e41479b56) )
ROM_END

ROM_START( urachamu )
	CRYSBIOS

	ROM_REGION( 0x4280, "pic", ROMREGION_ERASEFF ) // empty socket

	ROM_REGION32_LE( 0x4000000, "flash", 0 )
	ROM_LOAD( "u1",           0x0000000, 0x1000000,  CRC(f341d6fc) SHA1(23ecd9f3e5e20fc2a293cc735c8c4d60d01b68c0) )
	ROM_LOAD( "u2",           0x1000000, 0x1000000,  CRC(ad81d61f) SHA1(5872c1d96e25d1b2a1b8a35c8ff163d67cfdabe4) )
	ROM_LOAD( "u3",           0x2000000, 0x1000000,  CRC(3c7e32a4) SHA1(8a26e745eccc00a2c622062c8ad027781dfc1969) )
	ROM_LOAD( "u4",           0x3000000, 0x1000000,  CRC(c5505627) SHA1(5229f435b4cf218d50ae4a4ae65a6ae13d7b7080) )

	// without this game defaults in free play mode, with no demo sound and 0 lives count.
	ROM_REGION( 0x10000, "nvram", 0 )
	ROM_LOAD( "nvram",        0x00000, 0x10000, CRC(66df826d) SHA1(09260b76cb17082c841e7e37b89cb076e54ffb8d) )
ROM_END

ROM_START( maldaiza )
	CRYSBIOS

	ROM_REGION( 0x4280, "pic", ROMREGION_ERASEFF )
	ROM_LOAD("maldaliza_pic16f84a.bin",  0x000000, 0x4280, NO_DUMP )

	ROM_REGION32_LE( 0x2000000, "flash", 0 )
	ROM_LOAD( "u1",           0x0000000, 0x1000000,  CRC(f484d12b) SHA1(29641cda9138b5bf02c2ece34f8289385fd2ba29) )
	ROM_LOAD( "u2",           0x1000000, 0x1000000,  CRC(86175ebf) SHA1(c1800f7339dafd3ec6c0302eebc8406582a46b04) ) // $ff filled
ROM_END

/* note on PIC protection from ElSemi (for actually emulating it instead of patching)

The PIC uses a software UART bit banged on a single output pin of the main CPU:
the data port is bit 0x20000000 on the PIO register, the same register where the EEPROM control lines are. The serial data is transmitted at 8 data bits, even parity, 1 stop bit. It's probably
tricky to get it working properly because it doesn't rely on a clock signal, and so, the PIC and main CPU must run in parallel, and the bit lengths must match. The PIC bit delay routine is just a loop.
Also it seems that bit 0x40000000 is the PIC reset.

*/

void crystal_state::init_crysking()
{
	uint16_t *rom = (uint16_t*) memregion("flash")->base();

	//patch the data feed by the protection

	rom[WORD_XOR_LE(0x7bb6 / 2)] = 0xdf01;
	rom[WORD_XOR_LE(0x7bb8 / 2)] = 0x9c00;

	rom[WORD_XOR_LE(0x976a / 2)] = 0x901c;
	rom[WORD_XOR_LE(0x976c / 2)] = 0x9001;

	rom[WORD_XOR_LE(0x8096 / 2)] = 0x90fc;
	rom[WORD_XOR_LE(0x8098 / 2)] = 0x9001;

	rom[WORD_XOR_LE(0x8a52 / 2)] = 0x4000;    //NOP
	rom[WORD_XOR_LE(0x8a54 / 2)] = 0x403c;    //NOP
}

void crystal_state::init_evosocc()
{
	uint16_t *rom = (uint16_t*) memregion("flash")->base();
	rom += 0x1000000 * 2 / 2;

	rom[WORD_XOR_LE(0x97388e / 2)] = 0x90fc;  //PUSH R2..R7
	rom[WORD_XOR_LE(0x973890 / 2)] = 0x9001;  //PUSH R0

	rom[WORD_XOR_LE(0x971058 / 2)] = 0x907c;  //PUSH R2..R6
	rom[WORD_XOR_LE(0x971060 / 2)] = 0x9001; //PUSH R0

	rom[WORD_XOR_LE(0x978036 / 2)] = 0x900c;  //PUSH R2-R3
	rom[WORD_XOR_LE(0x978038 / 2)] = 0x8303;  //LD    (%SP,0xC),R3

	rom[WORD_XOR_LE(0x974ed0 / 2)] = 0x90fc;  //PUSH R7-R6-R5-R4-R3-R2
	rom[WORD_XOR_LE(0x974ed2 / 2)] = 0x9001;  //PUSH R0
}

void crystal_state::init_topbladv()
{
	// patches based on analysis of PIC dump
	uint16_t *rom = (uint16_t*) memregion("flash")->base();
	/*
	    PIC Protection data:
	    - RAM ADDR - --PATCH--
	    62 0f 02 02 fc 90 01 90
	    68 6a 02 02 04 90 01 90
	    2c cf 03 02 e9 df c2 c3
	    00 e0 03 02 01 90 00 92
	*/

	rom[WORD_XOR_LE(0x12d7a / 2)] = 0x90fc; //PUSH R7-R6-R5-R4-R3-R2
	rom[WORD_XOR_LE(0x12d7c / 2)] = 0x9001; //PUSH R0

	rom[WORD_XOR_LE(0x18880 / 2)] = 0x9004; //PUSH R2
	rom[WORD_XOR_LE(0x18882 / 2)] = 0x9001; //PUSH R0

	rom[WORD_XOR_LE(0x2fe18 / 2)] = 0x9001; //PUSH R0
	rom[WORD_XOR_LE(0x2fe1a / 2)] = 0x9200; //PUSH SR

	rom[WORD_XOR_LE(0x2ed44 / 2)] = 0xdfe9; //CALL 0x3cf00
	rom[WORD_XOR_LE(0x2ed46 / 2)] = 0xc3c2; //MOV %SR0,%DR1

}

void crystal_state::init_officeye()
{
	// patches based on analysis of PIC dump
	uint16_t *rom = (uint16_t*) memregion("flash")->base();

	/*
	    PIC Protection data:
	    - RAM ADDR - --PATCH--
	    0a 83 01 02 1c 90 01 90
	    50 85 01 02 7c 90 01 90
	    4c 99 05 02 04 90 01 90
	    3a c1 01 02 1c 90 01 90
	*/

	rom[WORD_XOR_LE(0x9c9e / 2)] = 0x901c;  //PUSH R4-R3-R2
	rom[WORD_XOR_LE(0x9ca0 / 2)] = 0x9001;  //PUSH R0

	rom[WORD_XOR_LE(0x9ee4 / 2)] = 0x907c;  //PUSH R6-R5-R4-R3-R2
	rom[WORD_XOR_LE(0x9ee6 / 2)] = 0x9001;  //PUSH R0

	rom[WORD_XOR_LE(0x4b2e0 / 2)] = 0x9004; //PUSH R2
	rom[WORD_XOR_LE(0x4b2e2 / 2)] = 0x9001; //PUSH R0

	rom[WORD_XOR_LE(0xdace / 2)] = 0x901c;  //PUSH R4-R3-R2
	rom[WORD_XOR_LE(0xdad0 / 2)] = 0x9001;  //PUSH R0
}

void crystal_state::init_donghaer()
{
	uint16_t *rom = (uint16_t*)memregion("flash")->base();

	rom[WORD_XOR_LE(0x037a2 / 2)] = 0x9004; // PUSH %R2
	rom[WORD_XOR_LE(0x037a4 / 2)] = 0x8202; // LD   (%SP,0x8),%R2

	rom[WORD_XOR_LE(0x03834 / 2)] = 0x9001; // PUSH %R0
	rom[WORD_XOR_LE(0x03836 / 2)] = 0x9200; // PUSH %SR

	rom[WORD_XOR_LE(0x0ac9e / 2)] = 0x9004; // PUSH %R2
	rom[WORD_XOR_LE(0x0aca0 / 2)] = 0x4081; // LERI 0x81

	rom[WORD_XOR_LE(0x19c70 / 2)] = 0x900c; // PUSH %R2-%R3
	rom[WORD_XOR_LE(0x19c72 / 2)] = 0x9001; // PUSH %R0
}

void crystal_state::init_maldaiza()
{
	uint16_t *rom = (uint16_t*)memregion("flash")->base();
	rom[WORD_XOR_LE(0x09b12 / 2)] = 0x9004; // PUSH %R2
	rom[WORD_XOR_LE(0x09b14 / 2)] = 0x8202; // LD   (%SP,0x8),%R2
	// ...
}

} // Anonymous namespace


GAME( 2001, crysbios, 0,        crystal,  crystal,  crystal_state, empty_init,    ROT0, "BrezzaSoft",          "Crystal System BIOS", MACHINE_IS_BIOS_ROOT )
GAME( 2001, crysking, crysbios, crystal,  crystal,  crystal_state, init_crysking, ROT0, "BrezzaSoft",          "The Crystal of Kings", 0 )
GAME( 2001, evosocc,  crysbios, crystal,  crystal,  crystal_state, init_evosocc,  ROT0, "Evoga",               "Evolution Soccer", 0 )
GAME( 2001, officeye, 0,        crystal,  officeye, crystal_state, init_officeye, ROT0, "Danbi",               "Office Yeo In Cheon Ha (version 1.2)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION ) // still has some instability issues
GAME( 2001, donghaer, crysbios, crystal,  crystal,  crystal_state, init_donghaer, ROT0, "Danbi",               "Donggul Donggul Haerong", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION ) // 2 players mode has GFX issues, seldomly hangs
GAME( 2002, urachamu, crysbios, crystal,  urachamu, crystal_state, empty_init,    ROT0, "GamToU",              "Urachacha Mudaeri (Korea)", 0 ) // lamps, verify game timings
GAME( 2003, topbladv, crysbios, crystal,  topbladv, crystal_state, init_topbladv, ROT0, "SonoKong / Expotato", "Top Blade V", 0 )
GAME( 200?, wulybuly, crysbios, crystal,  wulybuly, crystal_state, empty_init,    ROT0, "<unknown>",           "Wully Bully", MACHINE_NOT_WORKING )
GAME( 2002, maldaiza, crysbios, crystal,  crystal,  crystal_state, init_maldaiza, ROT0, "Big A Korea",         "Maldaliza", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION ) // PIC hookup
