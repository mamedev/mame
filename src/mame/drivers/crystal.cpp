// license:BSD-3-Clause
// copyright-holders:ElSemi
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

    The protection is a PIC deviced labeled SMART-IC in the rom board, I'm not sure how
    it exactly works, but it supplies some opcodes that have been replaced with garbage
    in the main program. I don't know if it traps reads and returns the correct ones when
    reading from flash, or if it's interfaced by the main program after copying the program
    from flash to ram and it provides the addresses and values to patch. I patch the flash
    program with the correct data

    MAME driver by ElSemi

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



P's Attack (c) 2004 Uniana Co., Ltd

+----------54321---654321--654321---------------------------+
|VOL       TICKET  GUN_1P  GUN_2P                 +---------|
|                                                 |         |
+-+                                               |  256MB  |
  |       CC-DAC                                  | Compact |
+-+                                  EMUL*        |  Flash  |
|                                                 |         |
|5          +---+                                 +---------|
|6          |   |                                           |
|P          | R |   25.1750MHz              +--------------+|
|I          | A |                           |     42Pin*   ||
|N          | M |                           +--------------+|
|           |   |                           +--------------+|
|C          +---+       +------------+      |     SYS      ||
|O                      |            |      +--------------+|
|N          +---+       |            |                      |
|N          |   |       |VRenderZERO+|                      |
|E SERVICE  | R |       | MagicEyes  |  +-------+    62256* |
|C          | A |       |            |  |  RAM  |           |
|T TEST     | M |       |            |  +-------+    62256* |
|O          |   |       +------------+                      |
|R RESET    +---+                                           |
|                                   14.31818MHz             |
+-+                                                         |
  |                                EEPROM                   |
+-+                GAL                                 DSW  |
|                                                           |
|  VGA                           PIC               BAT3.6V* |
+-----------------------------------------------------------+

* denotes unpopulated device

RAM are Samsung K4S641632H-TC75
VGA is a standard PC 15 pin VGA connection
DSW is 2 switch dipswitch (switches 3-8 are unpopulated)
PIC is a Microchip PIC16C711-041/P (silkscreened on the PCB as COSTOM)
SYS is a ST M27C160 EPROM (silkscreened on the PCB as SYSTEM_ROM_32M)
GAL is a GAL16V8B (not dumped)
EMUL is an unpopulated 8 pin connector
EEPROM is a 93C86 16K 5.0v Serial EEPROM (2048x8-bit or 1024x16-bit)
CC-DAC is a TDA1311A Stereo Continuous Calibration DAC


 P's Attack non JAMMA standard 56pin Edge Connector Pinout:

                          56pin Edge Connector
          Solder Side            |             Parts Side
------------------------------------------------------------------
             GND             | A | 1 |             GND
             GND             | B | 2 |             GND
             +5              | C | 3 |             +5
             +5              | D | 4 |             +5
       Player 1 Start Lamp   | E | 5 |         Coin Lamp
             +12             | F | 6 |             +12
------------ KEY ------------| G | 7 |------------ KEY -----------
       Player 2 Start Lamp   | H | 8 |        Coin Counter
        L Speaker (-)        | J | 9 |        L Speaker (+)
        R Speaker (-)        | K | 10|        R Speaker (+)
     Video Vertical Sync     | L | 11|
        Video Green          | M | 12|        Video Red
        Video Sync           | N | 13|        Video Blue
        Service Switch       | P | 14|        Video GND
    Video Horizontal Sync    | R | 15|        Test Switch
                             | S | 16|        Coin Switch
       Start Player 2        | T | 17|        Start Player 1
                             | U | 18|
                             | V | 19|
                             | W | 20|
                             | X | 21|
                             | Y | 22|
                             | a | 23|
                             | b | 24|
                             | d | 25|
                             | e | 26|
             GND             | f | 27|             GND
             GND             | g | 28|             GND


TICKET is a 5 pin connector:

  1| LED
  2| GND
  3| OUT
  4| IN
  5| +12v

GUN_xP are 6 pin gun connectors (pins 3-6 match the UNICO sytle guns):

 GUN-1P: Left (Blue) Gun Connector Pinout

  1| GND
  2| Solenoid
  3| Sensor
  4| +5V
  5| Switch (Trigger)
  6| GND

 GUN-2P: Right (Pink) Gun Connector Pinout

  1| GND
  2| Solenoid
  3| Sensor
  4| +5V
  5| Switch (Trigger)
  6| GND

*/

#include "emu.h"
#include "cpu/se3208/se3208.h"
#include "machine/ds1302.h"
#include "machine/nvram.h"
#include "sound/vrender0.h"
#include "video/vrender0.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define IDLE_LOOP_SPEEDUP

class crystal_state : public driver_device
{
public:
	crystal_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_sysregs(*this, "sysregs"),
		m_workram(*this, "workram"),
		m_vidregs(*this, "vidregs"),
		m_textureram(*this, "textureram"),
		m_frameram(*this, "frameram"),
		m_reset_patch(*this, "reset_patch"),
		m_maincpu(*this, "maincpu"),
		m_vr0(*this, "vr0"),
		m_video(*this, "vrender"),
		m_ds1302(*this, "rtc"),
		m_screen(*this, "screen")
	{ }

	/* memory pointers */
	required_shared_ptr<uint32_t> m_sysregs;
	required_shared_ptr<uint32_t> m_workram;
	required_shared_ptr<uint32_t> m_vidregs;
	required_shared_ptr<uint32_t> m_textureram;
	required_shared_ptr<uint32_t> m_frameram;
	optional_shared_ptr<uint32_t> m_reset_patch; // not needed for trivrus

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<vr0video_device> m_vr0;
	required_device<vrender0_device> m_video;
	required_device<ds1302_device> m_ds1302;
	required_device<screen_device> m_screen;

#ifdef IDLE_LOOP_SPEEDUP
	uint8_t     m_FlipCntRead;
#endif

	uint32_t    m_Bank;
	uint8_t     m_FlipCount;
	uint8_t     m_IntHigh;
	emu_timer  *m_Timer[4];
	uint32_t    m_FlashCmd;
	uint32_t    m_PIO;
	uint32_t    m_DMActrl[2];
	uint8_t     m_OldPort4;

	uint32_t *TimerRegsPtr(int which) const;
	void TimerStart(int which);

	DECLARE_READ32_MEMBER(FlipCount_r);
	DECLARE_WRITE32_MEMBER(FlipCount_w);
	DECLARE_READ32_MEMBER(Input_r);
	DECLARE_WRITE32_MEMBER(IntAck_w);
	DECLARE_WRITE32_MEMBER(Banksw_w);
	DECLARE_WRITE32_MEMBER(Timer0_w);
	DECLARE_READ32_MEMBER(Timer0_r);
	DECLARE_WRITE32_MEMBER(Timer1_w);
	DECLARE_READ32_MEMBER(Timer1_r);
	DECLARE_WRITE32_MEMBER(Timer2_w);
	DECLARE_READ32_MEMBER(Timer2_r);
	DECLARE_WRITE32_MEMBER(Timer3_w);
	DECLARE_READ32_MEMBER(Timer3_r);
	DECLARE_READ32_MEMBER(FlashCmd_r);
	DECLARE_WRITE32_MEMBER(FlashCmd_w);
	DECLARE_READ32_MEMBER(PIO_r);
	DECLARE_WRITE32_MEMBER(PIO_w);
	DECLARE_READ32_MEMBER(DMA0_r);
	DECLARE_WRITE32_MEMBER(DMA0_w);
	DECLARE_READ32_MEMBER(DMA1_r);
	DECLARE_WRITE32_MEMBER(DMA1_w);
	void init_topbladv();
	void init_officeye();
	void init_crysking();
	void init_evosocc();
	void init_donghaer();
	void init_psattack();

	DECLARE_READ32_MEMBER(trivrus_input_r);
	DECLARE_WRITE32_MEMBER(trivrus_input_w);
	uint8_t m_trivrus_input;

	DECLARE_READ32_MEMBER(crzyddz2_key_r);
	DECLARE_WRITE32_MEMBER(crzyddz2_PIO_w);
	uint8_t m_crzyddz2_prot;

	virtual void machine_start() override;
	virtual void machine_reset() override;
	uint32_t screen_update_crystal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_crystal);
	INTERRUPT_GEN_MEMBER(crystal_interrupt);
	TIMER_CALLBACK_MEMBER(Timercb);
	IRQ_CALLBACK_MEMBER(icallback);
	void crystal_banksw_postload();
	void IntReq( int num );
	inline void Timer_w( address_space &space, int which, uint32_t data, uint32_t mem_mask );
	inline void DMA_w( address_space &space, int which, uint32_t data, uint32_t mem_mask );
	void PatchReset(  );
	uint16_t GetVidReg( address_space &space, uint16_t reg );
	void SetVidReg( address_space &space, uint16_t reg, uint16_t val );
	void crospuzl(machine_config &config);
	void crystal(machine_config &config);
	void crzyddz2(machine_config &config);
	void trivrus(machine_config &config);
	void crospuzl_mem(address_map &map);
	void crystal_mem(address_map &map);
	void crzyddz2_mem(address_map &map);
	void trivrus_mem(address_map &map);
};

void crystal_state::IntReq( int num )
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint32_t IntEn = space.read_dword(0x01800c08);
	uint32_t IntPend = space.read_dword(0x01800c0c);
	if (IntEn & (1 << num))
	{
		IntPend |= (1 << num);
		space.write_dword(0x01800c0c, IntPend);
		m_maincpu->set_input_line(SE3208_INT, ASSERT_LINE);
	}
#ifdef IDLE_LOOP_SPEEDUP
	m_FlipCntRead = 0;
	m_maincpu->resume(SUSPEND_REASON_SPIN);
#endif
}

READ32_MEMBER(crystal_state::FlipCount_r)
{
#ifdef IDLE_LOOP_SPEEDUP
	uint32_t IntPend = space.read_dword(0x01800c0c);
	m_FlipCntRead++;
	if (m_FlipCntRead >= 16 && !IntPend && m_FlipCount != 0)
		m_maincpu->suspend(SUSPEND_REASON_SPIN, 1);
#endif
	return ((uint32_t) m_FlipCount) << 16;
}

WRITE32_MEMBER(crystal_state::FlipCount_w)
{
	if (ACCESSING_BITS_16_23)
	{
		int fc = (data >> 16) & 0xff;
		if (fc == 1)
			m_FlipCount++;
		else if (fc == 0)
			m_FlipCount = 0;
	}
}

READ32_MEMBER(crystal_state::Input_r)
{
	if (offset == 0)
		return ioport("P1_P2")->read();
	else if (offset == 1)
		return ioport("P3_P4")->read();
	else if( offset == 2)
	{
		uint8_t Port4 = ioport("SYSTEM")->read();
		if (!(Port4 & 0x10) && ((m_OldPort4 ^ Port4) & 0x10))   //coin buttons trigger IRQs
			IntReq(12);
		if (!(Port4 & 0x20) && ((m_OldPort4 ^ Port4) & 0x20))
			IntReq(19);
		m_OldPort4 = Port4;
		return /*dips*/ioport("DSW")->read() | (Port4 << 16);
	}
	return 0;
}

WRITE32_MEMBER(crystal_state::IntAck_w)
{
	uint32_t IntPend = space.read_dword(0x01800c0c);

	if (ACCESSING_BITS_0_7)
	{
		IntPend &= ~(1 << (data & 0x1f));
		space.write_dword(0x01800c0c, IntPend);
		if (!IntPend)
			m_maincpu->set_input_line(SE3208_INT, CLEAR_LINE);
	}
	if (ACCESSING_BITS_8_15)
		m_IntHigh = (data >> 8) & 7;
}

IRQ_CALLBACK_MEMBER(crystal_state::icallback)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint32_t IntPend = space.read_dword(0x01800c0c);
	int i;

	for (i = 0; i < 32; ++i)
	{
		if (BIT(IntPend, i))
		{
			return (m_IntHigh << 5) | i;
		}
	}
	return 0;       //This should never happen
}

WRITE32_MEMBER(crystal_state::Banksw_w)
{
	m_Bank = (data >> 1) & 7;
	if (m_Bank <= 2)
		membank("bank1")->set_base(memregion("user1")->base() + m_Bank * 0x1000000);
	else
		membank("bank1")->set_base(memregion("user2")->base());
}

uint32_t *crystal_state::TimerRegsPtr(int which) const
{
	return &m_sysregs[0x1400/4 + which * 8/4];
}

void crystal_state::TimerStart(int which)
{
	uint32_t *regs = TimerRegsPtr(which);

	int PD = (regs[0] >> 8) & 0xff;
	int TCV = regs[1] & 0xffff;
	attotime period = attotime::from_hz(43000000) * ((PD + 1) * (TCV + 1));
	m_Timer[which]->adjust(period);

//  printf("timer %d start, PD = %x TCV = %x period = %s\n", which, PD, TCV, period.as_string());
}

TIMER_CALLBACK_MEMBER(crystal_state::Timercb)
{
	int which = (int)(uintptr_t)ptr;
	static const int num[] = { 0, 1, 9, 10 };

	uint32_t *regs = TimerRegsPtr(which);

	if (regs[0] & 2)
		TimerStart(which);
	else
		regs[0] &= ~1;

	IntReq(num[which]);
}

void crystal_state::Timer_w( address_space &space, int which, uint32_t data, uint32_t mem_mask )
{
	uint32_t *regs = TimerRegsPtr(which);

	uint32_t old = regs[0];
	data = COMBINE_DATA(&regs[0]);

	if ((data ^ old) & 1)
	{
		if (data & 1)
		{
			TimerStart(which);
		}
		else
		{
			// Timer stop
			m_Timer[which]->adjust(attotime::never);
//          printf("timer %d stop\n", which);
		}
	}
}

WRITE32_MEMBER(crystal_state::Timer0_w)
{
	Timer_w(space, 0, data, mem_mask);
}

READ32_MEMBER(crystal_state::Timer0_r)
{
	return *TimerRegsPtr(0);
}

WRITE32_MEMBER(crystal_state::Timer1_w)
{
	Timer_w(space, 1, data, mem_mask);
}

READ32_MEMBER(crystal_state::Timer1_r)
{
	return *TimerRegsPtr(1);
}

WRITE32_MEMBER(crystal_state::Timer2_w)
{
	Timer_w(space, 2, data, mem_mask);
}

READ32_MEMBER(crystal_state::Timer2_r)
{
	return *TimerRegsPtr(2);
}

WRITE32_MEMBER(crystal_state::Timer3_w)
{
	Timer_w(space, 3, data, mem_mask);
}

READ32_MEMBER(crystal_state::Timer3_r)
{
	return *TimerRegsPtr(3);
}

READ32_MEMBER(crystal_state::FlashCmd_r)
{
	if ((m_FlashCmd & 0xff) == 0xff)
	{
		if (m_Bank <= 2)
		{
			uint32_t *ptr = (uint32_t*)(memregion("user1")->base() + m_Bank * 0x1000000);
			return ptr[0];
		}
		else
			return 0xffffffff;
	}
	if ((m_FlashCmd & 0xff) == 0x90)
	{
		if (m_Bank <= 2)
			return 0x00180089;  //Intel 128MBit
		else
			return 0xffffffff;
	}
	return 0;
}

WRITE32_MEMBER(crystal_state::FlashCmd_w)
{
	m_FlashCmd = data;
}

READ32_MEMBER(crystal_state::PIO_r)
{
	return m_PIO;
}

WRITE32_MEMBER(crystal_state::PIO_w)
{
	uint32_t RST = data & 0x01000000;
	uint32_t CLK = data & 0x02000000;
	uint32_t DAT = data & 0x10000000;

	m_ds1302->ce_w(RST ? 1 : 0);
	m_ds1302->io_w(DAT ? 1 : 0);
	m_ds1302->sclk_w(CLK ? 1 : 0);

	if (m_ds1302->io_r())
		space.write_dword(0x01802008, space.read_dword(0x01802008) | 0x10000000);
	else
		space.write_dword(0x01802008, space.read_dword(0x01802008) & (~0x10000000));

	COMBINE_DATA(&m_PIO);
}

void crystal_state::DMA_w( address_space &space, int which, uint32_t data, uint32_t mem_mask )
{
	if (((data ^ m_DMActrl[which]) & (1 << 10)) && (data & (1 << 10)))   //DMAOn
	{
		uint32_t CTR = data;
		uint32_t SRC = space.read_dword(0x01800804 + which * 0x10);
		uint32_t DST = space.read_dword(0x01800808 + which * 0x10);
		uint32_t CNT = space.read_dword(0x0180080C + which * 0x10);
		int i;

		if (CTR & 0x2)  //32 bits
		{
			for (i = 0; i < CNT; ++i)
			{
				uint32_t v = space.read_dword(SRC + i * 4);
				space.write_dword(DST + i * 4, v);
			}
		}
		else if (CTR & 0x1) //16 bits
		{
			for (i = 0; i < CNT; ++i)
			{
				uint16_t v = space.read_word(SRC + i * 2);
				space.write_word(DST + i * 2, v);
			}
		}
		else    //8 bits
		{
			for (i = 0; i < CNT; ++i)
			{
				uint8_t v = space.read_byte(SRC + i);
				space.write_byte(DST + i, v);
			}
		}
		data &= ~(1 << 10);
		space.write_dword(0x0180080C + which * 0x10, 0);
		IntReq(7 + which);
	}
	COMBINE_DATA(&m_DMActrl[which]);
}

READ32_MEMBER(crystal_state::DMA0_r)
{
	return m_DMActrl[0];
}

WRITE32_MEMBER(crystal_state::DMA0_w)
{
	DMA_w(space, 0, data, mem_mask);
}

READ32_MEMBER(crystal_state::DMA1_r)
{
	return m_DMActrl[1];
}

WRITE32_MEMBER(crystal_state::DMA1_w)
{
	DMA_w(space, 1, data, mem_mask);
}


void crystal_state::crystal_mem(address_map &map)
{
	map(0x00000000, 0x0001ffff).rom().nopw();

	map(0x01200000, 0x0120000f).r(FUNC(crystal_state::Input_r));
	map(0x01280000, 0x01280003).w(FUNC(crystal_state::Banksw_w));
	map(0x01400000, 0x0140ffff).ram().share("nvram");

	map(0x01800000, 0x0180ffff).ram().share("sysregs");

	map(0x01800800, 0x01800803).rw(FUNC(crystal_state::DMA0_r), FUNC(crystal_state::DMA0_w));
	map(0x01800810, 0x01800813).rw(FUNC(crystal_state::DMA1_r), FUNC(crystal_state::DMA1_w));

	map(0x01800c04, 0x01800c07).w(FUNC(crystal_state::IntAck_w));

	map(0x01801400, 0x01801403).rw(FUNC(crystal_state::Timer0_r), FUNC(crystal_state::Timer0_w));
	map(0x01801408, 0x0180140b).rw(FUNC(crystal_state::Timer1_r), FUNC(crystal_state::Timer1_w));
	map(0x01801410, 0x01801413).rw(FUNC(crystal_state::Timer2_r), FUNC(crystal_state::Timer2_w));
	map(0x01801418, 0x0180141b).rw(FUNC(crystal_state::Timer3_r), FUNC(crystal_state::Timer3_w));
	map(0x01802004, 0x01802007).rw(FUNC(crystal_state::PIO_r), FUNC(crystal_state::PIO_w));

	map(0x02000000, 0x027fffff).ram().share("workram");

	map(0x03000000, 0x0300ffff).ram().share("vidregs");
	map(0x030000a4, 0x030000a7).rw(FUNC(crystal_state::FlipCount_r), FUNC(crystal_state::FlipCount_w));
	map(0x03800000, 0x03ffffff).ram().share("textureram");
	map(0x04000000, 0x047fffff).ram().share("frameram");
	map(0x04800000, 0x04800fff).rw("vrender", FUNC(vrender0_device::vr0_snd_read), FUNC(vrender0_device::vr0_snd_write));

	map(0x05000000, 0x05ffffff).bankr("bank1");
	map(0x05000000, 0x05000003).rw(FUNC(crystal_state::FlashCmd_r), FUNC(crystal_state::FlashCmd_w));

	map(0x44414F4C, 0x44414F7F).ram().share("reset_patch");
}

// Trivia R Us
// To do: touch panel, RTC

READ32_MEMBER(crystal_state::trivrus_input_r)
{
	switch (m_trivrus_input)
	{
		case 1: return ioport("IN1")->read();
		case 2: return ioport("IN2")->read();
		case 3: return ioport("IN3")->read();
		case 4: return ioport("IN4")->read();
		case 5: return ioport("IN5")->read();
		case 6: return ioport("DSW")->read();
	}
	logerror("%s: unknown input %02x read\n", machine().describe_context(), m_trivrus_input);
	return 0xffffffff;
}

WRITE32_MEMBER(crystal_state::trivrus_input_w)
{
	if (ACCESSING_BITS_0_7)
		m_trivrus_input = data & 0xff;
}

void crystal_state::trivrus_mem(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom().nopw();

//  0x01280000 & 0x0000ffff (written at boot)
	map(0x01500000, 0x01500003).rw(FUNC(crystal_state::trivrus_input_r), FUNC(crystal_state::trivrus_input_w));
//  0x01500010 & 0x000000ff = sec
//  0x01500010 & 0x00ff0000 = min
//  0x01500014 & 0x000000ff = hour
//  0x01500014 & 0x00ff0000 = day
//  0x01500018 & 0x000000ff = month
//  0x0150001c & 0x000000ff = year - 2000
	map(0x01600000, 0x01607fff).ram().share("nvram");

	map(0x01800000, 0x0180ffff).ram().share("sysregs");

	map(0x01800800, 0x01800803).rw(FUNC(crystal_state::DMA0_r), FUNC(crystal_state::DMA0_w));
	map(0x01800810, 0x01800813).rw(FUNC(crystal_state::DMA1_r), FUNC(crystal_state::DMA1_w));

	map(0x01800c04, 0x01800c07).w(FUNC(crystal_state::IntAck_w));

	map(0x01801400, 0x01801403).rw(FUNC(crystal_state::Timer0_r), FUNC(crystal_state::Timer0_w));
	map(0x01801408, 0x0180140b).rw(FUNC(crystal_state::Timer1_r), FUNC(crystal_state::Timer1_w));
	map(0x01801410, 0x01801413).rw(FUNC(crystal_state::Timer2_r), FUNC(crystal_state::Timer2_w));
	map(0x01801418, 0x0180141b).rw(FUNC(crystal_state::Timer3_r), FUNC(crystal_state::Timer3_w));
	map(0x01802004, 0x01802007).rw(FUNC(crystal_state::PIO_r), FUNC(crystal_state::PIO_w));

	map(0x02000000, 0x027fffff).ram().share("workram");


	map(0x03000000, 0x0300ffff).ram().share("vidregs");
	map(0x030000a4, 0x030000a7).rw(FUNC(crystal_state::FlipCount_r), FUNC(crystal_state::FlipCount_w));
	map(0x03800000, 0x03ffffff).ram().share("textureram");
	map(0x04000000, 0x047fffff).ram().share("frameram");
	map(0x04800000, 0x04800fff).rw("vrender", FUNC(vrender0_device::vr0_snd_read), FUNC(vrender0_device::vr0_snd_write));

	map(0x05000000, 0x05ffffff).bankr("bank1");
	map(0x05000000, 0x05000003).rw(FUNC(crystal_state::FlashCmd_r), FUNC(crystal_state::FlashCmd_w));

//  AM_RANGE(0x44414F4C, 0x44414F7F) AM_RAM AM_SHARE("reset_patch")
}

void crystal_state::crospuzl_mem(address_map &map)
{
	trivrus_mem(map);

	map(0x01500000, 0x01500003).r(FUNC(crystal_state::FlashCmd_r));
	map(0x01500100, 0x01500103).w(FUNC(crystal_state::FlashCmd_w));
	map(0x01510000, 0x01510003).portr("IN0");
	map(0x01511000, 0x01511003).portr("IN1");
	map(0x01512000, 0x01512003).portr("IN2");
	map(0x01513000, 0x01513003).portr("IN3");
}

// Crazy Dou Di Zhu II
// To do: HY04 (pic?) protection

WRITE32_MEMBER(crystal_state::crzyddz2_PIO_w)
{
	COMBINE_DATA(&m_PIO);

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

READ32_MEMBER(crystal_state::crzyddz2_key_r)
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
//  m_crzyddz2_prot = (m_PIO >> 8) & 0xc0) ^ 0x40;
	m_crzyddz2_prot = (machine().rand() & 0xc0);

	return 0xffffff00 | data | m_crzyddz2_prot;
}

void crystal_state::crzyddz2_mem(address_map &map)
{
	map(0x00000000, 0x00ffffff).rom().nopw();

	map(0x01280000, 0x01280003).w(FUNC(crystal_state::Banksw_w));
	map(0x01400000, 0x0140ffff).ram().share("nvram");
	map(0x01500000, 0x01500003).portr("P1_P2");
	map(0x01500004, 0x01500007).r(FUNC(crystal_state::crzyddz2_key_r));

	map(0x01800000, 0x0180ffff).ram().share("sysregs");

	map(0x01800800, 0x01800803).rw(FUNC(crystal_state::DMA0_r), FUNC(crystal_state::DMA0_w));
	map(0x01800810, 0x01800813).rw(FUNC(crystal_state::DMA1_r), FUNC(crystal_state::DMA1_w));

	map(0x01800c04, 0x01800c07).w(FUNC(crystal_state::IntAck_w));

	map(0x01801400, 0x01801403).rw(FUNC(crystal_state::Timer0_r), FUNC(crystal_state::Timer0_w));
	map(0x01801408, 0x0180140b).rw(FUNC(crystal_state::Timer1_r), FUNC(crystal_state::Timer1_w));
	map(0x01801410, 0x01801413).rw(FUNC(crystal_state::Timer2_r), FUNC(crystal_state::Timer2_w));
	map(0x01801418, 0x0180141b).rw(FUNC(crystal_state::Timer3_r), FUNC(crystal_state::Timer3_w));
	map(0x01802004, 0x01802007).rw(FUNC(crystal_state::PIO_r), FUNC(crystal_state::crzyddz2_PIO_w));

	map(0x02000000, 0x027fffff).ram().share("workram");


	map(0x03000000, 0x0300ffff).ram().share("vidregs");
	map(0x030000a4, 0x030000a7).rw(FUNC(crystal_state::FlipCount_r), FUNC(crystal_state::FlipCount_w));
	map(0x03800000, 0x03ffffff).ram().share("textureram");
	map(0x04000000, 0x047fffff).ram().share("frameram");
	map(0x04800000, 0x04800fff).rw("vrender", FUNC(vrender0_device::vr0_snd_read), FUNC(vrender0_device::vr0_snd_write));

	map(0x05000000, 0x05ffffff).bankr("bank1");
	map(0x05000000, 0x05000003).rw(FUNC(crystal_state::FlashCmd_r), FUNC(crystal_state::FlashCmd_w));

//  AM_RANGE(0x44414F4C, 0x44414F7F) AM_RAM AM_SHARE("reset_patch")
}


void crystal_state::PatchReset(  )
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
	static const uint32_t Patch[] =
	{
		0x40c0ea01,
		0xe906400a,
		0x40c02a20,
		0xe906400a,
		0xa1d03a20,
		0xdef4d4fa
	};

	memcpy(m_reset_patch, Patch, sizeof(Patch));
#else
	static const uint8_t Patch[] =
	{
		0x01,0xEA,0xC0,0x40,0x0A,0x40,0x06,0xE9,
		0x20,0x2A,0xC0,0x40,0x0A,0x40,0x06,0xE9,
		0x20,0x3A,0xD0,0xA1,0xFA,0xD4,0xF4,0xDE
	};

	memcpy(m_reset_patch, Patch, sizeof(Patch));
#endif
}

void crystal_state::crystal_banksw_postload()
{
	if (m_Bank <= 2)
		membank("bank1")->set_base(memregion("user1")->base() + m_Bank * 0x1000000);
	else
		membank("bank1")->set_base(memregion("user2")->base());
}

void crystal_state::machine_start()
{
	int i;

	for (i = 0; i < 4; i++)
		m_Timer[i] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(crystal_state::Timercb),this), (void*)(uintptr_t)i);

	PatchReset();

#ifdef IDLE_LOOP_SPEEDUP
	save_item(NAME(m_FlipCntRead));
#endif

	save_item(NAME(m_Bank));
	save_item(NAME(m_FlipCount));
	save_item(NAME(m_IntHigh));
	save_item(NAME(m_FlashCmd));
	save_item(NAME(m_PIO));
	save_item(NAME(m_DMActrl));
	save_item(NAME(m_OldPort4));
	save_item(NAME(m_trivrus_input));
	machine().save().register_postload(save_prepost_delegate(FUNC(crystal_state::crystal_banksw_postload), this));
}

void crystal_state::machine_reset()
{
	memset(m_sysregs, 0, 0x10000);
	memset(m_vidregs, 0, 0x10000);

	m_FlipCount = 0;
	m_IntHigh = 0;
	m_Bank = 0;
	membank("bank1")->set_base(memregion("user1")->base() + 0);
	m_FlashCmd = 0xff;
	m_OldPort4 = 0;

	m_DMActrl[0] = 0;
	m_DMActrl[1] = 0;

	for (int i = 0; i < 4; i++)
	{
		*TimerRegsPtr(i) = 0xff << 8;
		m_Timer[i]->adjust(attotime::never);
	}

	m_video->set_areas(m_textureram, m_frameram);
#ifdef IDLE_LOOP_SPEEDUP
	m_FlipCntRead = 0;
#endif

	PatchReset();

	m_crzyddz2_prot = 0x00;
}

uint16_t crystal_state::GetVidReg( address_space &space, uint16_t reg )
{
	return space.read_word(0x03000000 + reg);
}

void crystal_state::SetVidReg( address_space &space, uint16_t reg, uint16_t val )
{
	space.write_word(0x03000000 + reg, val);
}


uint32_t crystal_state::screen_update_crystal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int xres = 320;
	int yres = 240;

	// probably more registers around here control height / interlace enable etc.
	// 0x341c looks like height, but doesn't change for interlace mode.
	xres = m_sysregs[0x340c / 4]+1;
	if (xres > 640) xres = 640;

	// force double height if 640 wide (probably a reg for this)
	if (xres == 640) yres = 480;


	rectangle visarea;
	visarea.set(0, xres-1, 0, yres-1);
	m_screen->configure(xres, yres, visarea, m_screen->frame_period().attoseconds() );



	address_space &space = m_maincpu->space(AS_PROGRAM);
	int DoFlip;

	uint32_t B0 = 0x0;
	uint32_t B1 = (GetVidReg(space, 0x90) & 0x8000) ? 0x400000 : 0x100000;
	uint16_t *Front, *Back;
	uint16_t *Visible, *DrawDest;
	uint16_t *srcline;
	int y;
	uint16_t head, tail;
	uint32_t width = screen.width();

	if (GetVidReg(space, 0x8e) & 1)
	{
		Front = (uint16_t*) (m_frameram + B1 / 4);
		Back  = (uint16_t*) (m_frameram + B0 / 4);
	}
	else
	{
		Front = (uint16_t*) (m_frameram + B0 / 4);
		Back  = (uint16_t*) (m_frameram + B1 / 4);
	}

	Visible  = (uint16_t*) Front;
	// ERROR: This cast is NOT endian-safe without the use of BYTE/WORD/DWORD_XOR_* macros!
	DrawDest = reinterpret_cast<uint16_t *>(m_frameram.target());


	if (GetVidReg(space, 0x8c) & 0x80)
		DrawDest = Front;
	else
		DrawDest = Back;

//  DrawDest = Visible;

	srcline = (uint16_t *) DrawDest;

	DoFlip = 0;
	head = GetVidReg(space, 0x82);
	tail = GetVidReg(space, 0x80);
	while ((head & 0x7ff) != (tail & 0x7ff))
	{
		// ERROR: This cast is NOT endian-safe without the use of BYTE/WORD/DWORD_XOR_* macros!
		DoFlip = m_vr0->vrender0_ProcessPacket(0x03800000 + head * 64, DrawDest, reinterpret_cast<uint8_t*>(m_textureram.target()));
		head++;
		head &= 0x7ff;
		if (DoFlip)
			break;
	}

	if (DoFlip)
		SetVidReg(space, 0x8e, GetVidReg(space, 0x8e) ^ 1);

	srcline = (uint16_t *) Visible;
	for (y = 0; y < screen.height(); y++)
		memcpy(&bitmap.pix16(y), &srcline[y * 1024], width * 2);

	return 0;
}

WRITE_LINE_MEMBER(crystal_state::screen_vblank_crystal)
{
	// rising edge
	if (state)
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		uint16_t head, tail;
		int DoFlip = 0;

		head = GetVidReg(space, 0x82);
		tail = GetVidReg(space, 0x80);
		while ((head & 0x7ff) != (tail & 0x7ff))
		{
			uint16_t Packet0 = space.read_word(0x03800000 + head * 64);
			if (Packet0 & 0x81)
				DoFlip = 1;
			head++;
			head &= 0x7ff;
			if (DoFlip)
				break;
		}
		SetVidReg(space, 0x82, head);
		if (DoFlip)
		{
			if (m_FlipCount)
				m_FlipCount--;

		}
	}
}

INTERRUPT_GEN_MEMBER(crystal_state::crystal_interrupt)
{
	IntReq(24);      //VRender0 VBlank
}

static INPUT_PORTS_START(crystal)
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
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Pause ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Test ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



static INPUT_PORTS_START(officeye)
	PORT_START("P1_P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Red")  // RED
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Green")  // GREEN
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Blue") // BLUE
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	//PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_START2 ) // where is start2?
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Red")  // RED
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3) PORT_NAME("P3 Red")
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Green")  // GREEN
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3) PORT_NAME("P3 Green")
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Blue") // BLUE
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3) PORT_NAME("P3 Blue")
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3_P4")
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00ff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff000000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Pause ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Test ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START(trivrus)
	PORT_START("IN1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("Up")        PORT_CODE(KEYCODE_UP)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("Left/True") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("Down")      PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Enter/Exit")
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Next")
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_OTHER   ) PORT_NAME("Right/False") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_COIN1   ) PORT_IMPULSE(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Sound")
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_BIT( 0x000000ff, IP_ACTIVE_LOW, IPT_OTHER )PORT_CODE(KEYCODE_9)
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN5")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_SERVICE1 ) // Free Game
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )   // Setup
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Interlace?" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Serial?" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )       // hangs at boot
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Touch Screen" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START(crospuzl)
	PORT_START("IN0")
	PORT_DIPNAME( 0x01, 0x01, "DSW1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "IN3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("PCB-SW1")
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

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
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_SERVICE1       ) // .. 3  (inc secret code / credit)
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_SERVICE3       ) // .. 4  (exit secret screen / clear credits)
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_SERVICE4       ) //       (reset and clear ram?)
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNKNOWN        )
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


MACHINE_CONFIG_START(crystal_state::crystal)

	MCFG_DEVICE_ADD("maincpu", SE3208, 43000000)
	MCFG_DEVICE_PROGRAM_MAP(crystal_mem)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", crystal_state,  crystal_interrupt)
	MCFG_DEVICE_IRQ_ACKNOWLEDGE_DRIVER(crystal_state, icallback)

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)
	MCFG_SCREEN_UPDATE_DRIVER(crystal_state, screen_update_crystal)
	MCFG_SCREEN_VBLANK_CALLBACK(WRITELINE(*this, crystal_state, screen_vblank_crystal))
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("vr0", VIDEO_VRENDER0, 0, "maincpu")

	MCFG_PALETTE_ADD_RRRRRGGGGGGBBBBB("palette")

	MCFG_DEVICE_ADD("rtc", DS1302, 32.768_kHz_XTAL)

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	MCFG_SOUND_VRENDER0_ADD("vrender", 0)
	MCFG_VR0_REGBASE(0x04800000)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(crystal_state::trivrus)
	crystal(config);
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(trivrus_mem)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(crystal_state::crospuzl)
	crystal(config);
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(crospuzl_mem)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(crystal_state::crzyddz2)
	crystal(config);
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(crzyddz2_mem)
MACHINE_CONFIG_END


ROM_START( crysbios )
	ROM_REGION( 0x20000, "maincpu", 0 ) // bios
	ROM_LOAD("mx27l1000.u14",  0x000000, 0x020000, CRC(beff39a9) SHA1(b6f6dda58d9c82273f9422c1bd623411e58982cb) )

	ROM_REGION32_LE( 0x3000000, "user1", ROMREGION_ERASEFF ) // Flash

	ROM_REGION( 0x1000000, "user2",   ROMREGION_ERASEFF ) // Unmapped flash
ROM_END

ROM_START( crysking )
	ROM_REGION( 0x20000, "maincpu", 0 ) // bios
	ROM_LOAD("mx27l1000.u14",  0x000000, 0x020000, CRC(beff39a9) SHA1(b6f6dda58d9c82273f9422c1bd623411e58982cb))

	ROM_REGION32_LE( 0x3000000, "user1", 0 ) // Flash
	ROM_LOAD("bcsv0004f01.u1",  0x0000000, 0x1000000, CRC(8feff120) SHA1(2ea42fa893bff845b5b855e2556789f8354e9066) )
	ROM_LOAD("bcsv0004f02.u2",  0x1000000, 0x1000000, CRC(0e799845) SHA1(419674ce043cb1efb18303f4cb7fdbbae642ee39) )
	ROM_LOAD("bcsv0004f03.u3",  0x2000000, 0x1000000, CRC(659e2d17) SHA1(342c98f3f695ef4dea8b533612451c4d2fb58809) )

	ROM_REGION( 0x1000000, "user2",   ROMREGION_ERASEFF ) // Unmapped flash
ROM_END

ROM_START( evosocc )
	ROM_REGION( 0x20000, "maincpu", 0 ) // bios
	ROM_LOAD("mx27l1000.u14",  0x000000, 0x020000, CRC(beff39a9) SHA1(b6f6dda58d9c82273f9422c1bd623411e58982cb))

	ROM_REGION32_LE( 0x3000000, "user1", 0 ) // Flash
	ROM_LOAD("bcsv0001u01",  0x0000000, 0x1000000, CRC(2581a0ea) SHA1(ee483ac60a3ed00a21cb515974cec4af19916a7d) )
	ROM_LOAD("bcsv0001u02",  0x1000000, 0x1000000, CRC(47ef1794) SHA1(f573706c17d1342b9b7aed9b40b8b648f0bf58db) )
	ROM_LOAD("bcsv0001u03",  0x2000000, 0x1000000, CRC(f396a2ec) SHA1(f305eb10856fb5d4c229a6b09d6a2fb21b24ce66) )

	ROM_REGION( 0x1000000, "user2",   ROMREGION_ERASEFF ) // Unmapped flash
ROM_END

ROM_START( topbladv )
	ROM_REGION( 0x20000, "maincpu", 0 ) // bios
	ROM_LOAD("mx27l1000.u14",  0x000000, 0x020000, CRC(beff39a9) SHA1(b6f6dda58d9c82273f9422c1bd623411e58982cb))

	ROM_REGION( 0x4300, "pic", 0 ) // pic16c727 - we don't have a core for this
	ROM_LOAD("top_blade_v_pic16c727.bin",  0x000000, 0x4300, CRC(9cdea57b) SHA1(884156085f9e780cdf719aedc2e8a0fd5983613b) )


	ROM_REGION32_LE( 0x3000000, "user1", 0 ) // Flash
	ROM_LOAD("flash.u1",  0x0000000, 0x1000000, CRC(bd23f640) SHA1(1d22aa2c828642bb7c1dfea4e13f777f95acc701) )

	ROM_REGION( 0x1000000, "user2",   ROMREGION_ERASEFF ) // Unmapped flash
ROM_END

ROM_START( officeye )
	ROM_REGION( 0x20000, "maincpu", 0 ) // bios (not the standard one)
	ROM_LOAD("bios.u14",  0x000000, 0x020000, CRC(ffc57e90) SHA1(6b6a17fd4798dea9c7b880f3063be8494e7db302) )

	ROM_REGION( 0x4280, "pic", 0 ) // pic16f84a - we don't have a core for this
	ROM_LOAD("office_yeo_in_cheon_ha_pic16f84a.bin",  0x000000, 0x4280, CRC(7561cdf5) SHA1(eade592823a110019b4af81a7dc56d01f7d6589f) )


	ROM_REGION32_LE( 0x3000000, "user1", 0 ) // Flash
	ROM_LOAD("flash.u1",  0x0000000, 0x1000000, CRC(d3f3eec4) SHA1(ea728415bd4906964b7d37f4379a8a3bd42a1c2d) )
	ROM_LOAD("flash.u2",  0x1000000, 0x1000000, CRC(e4f85d0a) SHA1(2ddfa6b3a30e69754aa9d96434ff3d37784bfa57) )

	ROM_REGION( 0x1000000, "user2",   ROMREGION_ERASEFF ) // Unmapped flash
ROM_END

ROM_START( donghaer )
	ROM_REGION( 0x20000, "maincpu", 0 ) // bios
	ROM_LOAD("mx27l1000.u14",  0x000000, 0x020000, CRC(beff39a9) SHA1(b6f6dda58d9c82273f9422c1bd623411e58982cb))

	ROM_REGION( 0x4280, "pic", 0 ) // pic16f84a - we don't have a core for this (or the dump in this case)
	ROM_LOAD("donghaer_pic16f84a.bin",  0x000000, 0x4280, NO_DUMP )

	ROM_REGION32_LE( 0x3000000, "user1", 0 ) // Flash
	ROM_LOAD( "u1",           0x0000000, 0x1000000, CRC(61217ad7) SHA1(2593f1356aa850f4f9aa5d00bec822aa59c59224) )
	ROM_LOAD( "u2",           0x1000000, 0x1000000, CRC(6d82f1a5) SHA1(036bd45f0daac1ffeaa5ad9774fc1b56e3c75ff9) )

	ROM_REGION( 0x1000000, "user2",   ROMREGION_ERASEFF ) // Unmapped flash
ROM_END

ROM_START( trivrus )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "u4", 0x00000, 0x80000, CRC(2d2e9a11) SHA1(73e7b19a032eae21312ca80f8c42cc16725496a7) )

	ROM_REGION32_LE( 0x3000000, "user1", ROMREGION_ERASEFF ) // Flash
	ROM_LOAD( "u3", 0x000000, 0x1000010, CRC(ba901707) SHA1(e281ba07024cd19ef1ab72d2197014f7b1f4d30f) )

	ROM_REGION( 0x1000000, "user2", ROMREGION_ERASEFF ) // Unmapped flash
ROM_END

ROM_START( crospuzl ) /* This PCB uses ADC 'Amazon-LF' SoC, EISC CPU core - However PCBs have been see with a standard VRenderZERO+ MagicEyes EISC chip */
	ROM_REGION( 0x80010, "maincpu", 0 )
	ROM_LOAD("en29lv040a.u5",  0x000000, 0x80010, CRC(d50e8500) SHA1(d681cd18cd0e48854c24291d417d2d6d28fe35c1) )

	ROM_REGION32_LE( 0x8400010, "user1", ROMREGION_ERASEFF ) // Flash
	// mostly empty, but still looks good
	ROM_LOAD("k9f1g08u0a.riser",  0x000000, 0x8400010, CRC(7f3c88c3) SHA1(db3169a7b4caab754e9d911998a2ece13c65ce5b) )

	ROM_REGION( 0x1000000, "user2", ROMREGION_ERASEFF ) // Unmapped flash
ROM_END

ROM_START( psattack )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD("5.sys",  0x000000, 0x200000, CRC(f09878e4) SHA1(25b8dbac47d3911615c8874746e420ece13e7181) )

	ROM_REGION( 0x4010, "pic16c711", 0 )
	ROM_LOAD("16c711.pic",  0x0000, 0x137b, CRC(617d8292) SHA1(d32d6054ce9db2e31efaf41015afcc78ed32f6aa) ) // raw dump
	ROM_LOAD("16c711.bin",  0x0000, 0x4010, CRC(b316693f) SHA1(eba1f75043bd415268eedfdb95c475e73c14ff86) ) // converted to binary

	DISK_REGION( "cfcard" )
	DISK_IMAGE_READONLY( "psattack", 0, SHA1(e99cd0dafc33ec13bf56061f81dc7c0a181594ee) )

	// keep driver happy
	ROM_REGION32_LE( 0x3000000, "user1", ROMREGION_ERASEFF )
	ROM_REGION( 0x1000000, "user2",   ROMREGION_ERASEFF )
ROM_END

ROM_START( ddz )
	ROM_REGION( 0xc00000, "maincpu", 0 )
	ROM_LOAD("ddz.001.rom",  0x000000, 0x400000, CRC(b379f823) SHA1(531885b35d668d22c75a9759994f4aca6eacb046) )
	ROM_LOAD("ddz.002.rom",  0x400000, 0x400000, CRC(285c744d) SHA1(2f8bc70825e55e3114015cb263e786df35cde275) )
	ROM_LOAD("ddz.003.rom",  0x800000, 0x400000, CRC(61c9b5c9) SHA1(0438417398403456a1c49408881797a94aa86f49) )

	// keep driver happy
	ROM_REGION32_LE( 0x3000000, "user1", ROMREGION_ERASEFF )
	ROM_REGION( 0x1000000, "user2",   ROMREGION_ERASEFF )
ROM_END


/*
招级疯斗 - "Zhaoji Fengdou" - "Crazy Class"

Haze's notes:

fwiw, it's probably same PCB as the non-working 'ddz' in MAME, but different game.

there's some kind of encryption/scrambling going on, at the very least

Code:


Offset      0  1  2  3  4  5  6  7   8  9  A  B  C  D  E  F

0007BE60   00 00 00 99 03 AD AF 00  00 00 82 00 03 AD 64 63      ™ ­¯   ‚  ­dc
0007BE70   62 61 39 38 37 36 35 34  33 32 31 30 00 4E 61 4E   ba9876543210 NaN
0007BE80   00 66 6E 49 02 0E 85 06  02 0E 84 04 02 0E 83 EA    fnI  …   „   ƒê
0007BE90   02 0E 83 D6 02 0E 83 C8  02 0E 84 58 02 0E 84 12     ƒÖ  ƒÈ  „X  „
0007BEA0   66 65 28 00 30 00 65 73  61 62 20 64 61 62 20 3A   fe( 0 esab dab :
0007BEB0   66 74 6E 69 72 70 66 76  20 6E 69 20 67 75 62 00   ftnirpfv ni gub
0007BEC0   46 45 44 43 42 41 39 38  37 36 35 34 33 32 31 30   FEDCBA9876543210
0007BED0   00 29 6C 6C 75 6E 2E 00  00 00 8F 8E 02 0E 89 DC    )llun.    Ž  ‰Ü


if you reverse the letters you get 'bug in vfprintf : bad base'

so I suspect the data is in reverse order and maybe some blocks scrambled about.
*/


ROM_START( crzclass ) // PCB marked MAH-JONG
	ROM_REGION( 0xc00000, "maincpu", 0 )
	ROM_LOAD("tjf-mahjong-rom1.bin",  0x000000, 0x400000, CRC(0a8af816) SHA1(9f292e847873078ed2b7584f463633cf9086c7e8) ) // SHARP LH28F320BJD-TTL80
	ROM_LOAD("tjf-mahjong-rom2.bin",  0x400000, 0x400000, CRC(2a04e84a) SHA1(189b16fd4314fd2a5f8a1214618b5db83f8ac59a) ) // SHARP LH28F320BJD-TTL80
	ROM_LOAD("tjf-mahjong-rom3.bin",  0x800000, 0x400000, CRC(1cacf3f9) SHA1(e6c88c98aeb7df4098f8e20f412018617005724d) ) // SHARP LH28F320BJD-TTL80
	// rom4 not populated

	// keep driver happy
	ROM_REGION32_LE( 0x3000000, "user1", ROMREGION_ERASEFF )
	ROM_REGION( 0x1000000, "user2",   ROMREGION_ERASEFF )
ROM_END

/***************************************************************************

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

***************************************************************************/

ROM_START( crzyddz2 )
	ROM_REGION32_LE( 0x3000000, "user1", ROMREGION_ERASEFF ) // Flash
	ROM_LOAD( "rom.u48", 0x000000, 0x1000000, CRC(0f3a1987) SHA1(6cad943846c79db31226676c7391f32216cfff79) )

	ROM_REGION( 0x1000000, "maincpu", ROMREGION_ERASEFF )
	ROM_COPY( "user1",      0x000000, 0x000000, 0x1000000 ) // copy flash here
	ROM_LOAD( "27c322.u49", 0x000000, 0x0200000, CRC(b3177f39) SHA1(2a28bf8045bd2e053d88549b79fbc11f30ef9a32) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_CONTINUE(           0x000000, 0x0200000 )

	ROM_REGION( 0x4280, "pic", 0 ) // hy04
	ROM_LOAD("hy04", 0x000000, 0x4280, NO_DUMP )

	ROM_REGION( 0x1000000, "user2", ROMREGION_ERASEFF ) // Unmapped flash
ROM_END

/***************************************************************************

Meng Hong Lou (Dream of the Red Chamber)
Sealy, 2004?

Red PCB, very similar to crzyddz2

***************************************************************************/

ROM_START( menghong )
	ROM_REGION32_LE( 0x3000000, "user1", ROMREGION_ERASEFF ) // Flash
	ROM_LOAD( "rom.u48", 0x000000, 0x1000000, CRC(e24257c4) SHA1(569d79a61ff6d35100ba5727069363146df9e0b7) )

	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_COPY( "user1",      0x000000, 0x000000, 0x1000000 ) // copy flash here
	ROM_LOAD( "060511_08-01-18.u49",  0x000000, 0x0200000, CRC(b0c12107) SHA1(b1753757bbdb7d996df563ac6abdc6b46676704b) ) // 27C160

	ROM_REGION( 0x4280, "pic", 0 ) // hy04
	ROM_LOAD("menghong_hy04", 0x000000, 0x4280, NO_DUMP )

	ROM_REGION( 0x1000000, "user2", ROMREGION_ERASEFF ) // Unmapped flash
ROM_END


void crystal_state::init_crysking()
{
	uint16_t *Rom = (uint16_t*) memregion("user1")->base();

	//patch the data feed by the protection

	Rom[WORD_XOR_LE(0x7bb6/2)] = 0xDF01;
	Rom[WORD_XOR_LE(0x7bb8/2)] = 0x9C00;

	Rom[WORD_XOR_LE(0x976a/2)] = 0x901C;
	Rom[WORD_XOR_LE(0x976c/2)] = 0x9001;

	Rom[WORD_XOR_LE(0x8096/2)] = 0x90FC;
	Rom[WORD_XOR_LE(0x8098/2)] = 0x9001;

	Rom[WORD_XOR_LE(0x8a52/2)] = 0x4000;    //NOP
	Rom[WORD_XOR_LE(0x8a54/2)] = 0x403c;    //NOP
}

void crystal_state::init_evosocc()
{
	uint16_t *Rom = (uint16_t*) memregion("user1")->base();
	Rom += 0x1000000 * 2 / 2;

	Rom[WORD_XOR_LE(0x97388E/2)] = 0x90FC;  //PUSH R2..R7
	Rom[WORD_XOR_LE(0x973890/2)] = 0x9001;  //PUSH R0

	Rom[WORD_XOR_LE(0x971058/2)] = 0x907C;  //PUSH R2..R6
	Rom[WORD_XOR_LE(0x971060/2)] = 0x9001; //PUSH R0

	Rom[WORD_XOR_LE(0x978036/2)] = 0x900C;  //PUSH R2-R3
	Rom[WORD_XOR_LE(0x978038/2)] = 0x8303;  //LD    (%SP,0xC),R3

	Rom[WORD_XOR_LE(0x974ED0/2)] = 0x90FC;  //PUSH R7-R6-R5-R4-R3-R2
	Rom[WORD_XOR_LE(0x974ED2/2)] = 0x9001;  //PUSH R0
}

/* note on PIC protection from ElSemi (for actually emulating it instead of patching)

The PIC uses a software UART bit banged on a single output pin of the main CPU:
the data port is bit 0x20000000 on the PIO register, the same register where the EEPROM control lines are. The serial data is transmitted at 8 data bits, even parity, 1 stop bit. It's probably
tricky to get it working properly because it doesn't rely on a clock signal, and so, the pic and main cpu must run in parallel, and the bit lengths must match. The pic bit delay routine is just a loop.
also it seems that bit 0x40000000 is the PIC reset.

*/

void crystal_state::init_topbladv()
{
	// patches based on analysis of PIC dump
	uint16_t *Rom = (uint16_t*) memregion("user1")->base();
	/*
	    PIC Protection data:
	    - RAM ADDR - --PATCH--
	    62 0f 02 02 fc 90 01 90
	    68 6a 02 02 04 90 01 90
	    2c cf 03 02 e9 df c2 c3
	    00 e0 03 02 01 90 00 92
	*/

	Rom[WORD_XOR_LE(0x12d7a/2)]=0x90FC; //PUSH R7-R6-R5-R4-R3-R2
	Rom[WORD_XOR_LE(0x12d7c/2)]=0x9001; //PUSH R0

	Rom[WORD_XOR_LE(0x18880/2)]=0x9004; //PUSH R2
	Rom[WORD_XOR_LE(0x18882/2)]=0x9001; //PUSH R0

	Rom[WORD_XOR_LE(0x2fe18/2)]=0x9001; //PUSH R0
	Rom[WORD_XOR_LE(0x2fe1a/2)]=0x9200; //PUSH SR

	Rom[WORD_XOR_LE(0x2ED44/2)]=0xDFE9; //CALL 0x3cf00
	Rom[WORD_XOR_LE(0x2ED46/2)]=0xC3C2; //MOV %SR0,%DR1

}

void crystal_state::init_officeye()
{
	// patches based on analysis of PIC dump
	uint16_t *Rom = (uint16_t*) memregion("user1")->base();

	/*
	    PIC Protection data:
	    - RAM ADDR - --PATCH--
	    0a 83 01 02 1c 90 01 90
	    50 85 01 02 7c 90 01 90
	    4c 99 05 02 04 90 01 90
	    3a c1 01 02 1c 90 01 90
	*/

	Rom[WORD_XOR_LE(0x9c9e/2)]=0x901C;  //PUSH R4-R3-R2
	Rom[WORD_XOR_LE(0x9ca0/2)]=0x9001;  //PUSH R0

	Rom[WORD_XOR_LE(0x9EE4/2)]=0x907C;  //PUSH R6-R5-R4-R3-R2
	Rom[WORD_XOR_LE(0x9EE6/2)]=0x9001;  //PUSH R0

	Rom[WORD_XOR_LE(0x4B2E0/2)]=0x9004; //PUSH R2
	Rom[WORD_XOR_LE(0x4B2E2/2)]=0x9001; //PUSH R0

	Rom[WORD_XOR_LE(0xDACE/2)]=0x901c;  //PUSH R4-R3-R2
	Rom[WORD_XOR_LE(0xDAD0/2)]=0x9001;  //PUSH R0
}

void crystal_state::init_donghaer()
{
	uint16_t *Rom = (uint16_t*)memregion("user1")->base();

	Rom[WORD_XOR_LE(0x037A2 / 2)] = 0x9004; // PUSH %R2
	Rom[WORD_XOR_LE(0x037A4 / 2)] = 0x8202; // LD   (%SP,0x8),%R2

	Rom[WORD_XOR_LE(0x03834 / 2)] = 0x9001; // PUSH %R0
	Rom[WORD_XOR_LE(0x03836 / 2)] = 0x9200; // PUSH %SR

	Rom[WORD_XOR_LE(0x0AC9E / 2)] = 0x9004; // PUSH %R2
	Rom[WORD_XOR_LE(0x0ACA0 / 2)] = 0x4081; // LERI 0x81

	Rom[WORD_XOR_LE(0x19C70 / 2)] = 0x900C; // PUSH %R2-%R3
	Rom[WORD_XOR_LE(0x19C72 / 2)] = 0x9001; // PUSH %R0
}

void crystal_state::init_psattack()
{
}


GAME( 2001, crysbios, 0,        crystal,  crystal,  crystal_state, empty_init,    ROT0, "BrezzaSoft",          "Crystal System BIOS",                  MACHINE_IS_BIOS_ROOT )
GAME( 2001, crysking, crysbios, crystal,  crystal,  crystal_state, init_crysking, ROT0, "BrezzaSoft",          "The Crystal of Kings",                 0 )
GAME( 2001, evosocc,  crysbios, crystal,  crystal,  crystal_state, init_evosocc,  ROT0, "Evoga",               "Evolution Soccer",                     0 )
GAME( 2003, topbladv, crysbios, crystal,  crystal,  crystal_state, init_topbladv, ROT0, "SonoKong / Expotato", "Top Blade V",                          0 )
GAME( 2001, officeye, 0,        crystal,  officeye, crystal_state, init_officeye, ROT0, "Danbi",               "Office Yeo In Cheon Ha (version 1.2)", MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION ) // still has some instability issues
GAME( 2001, donghaer, 0,        crystal,  crystal,  crystal_state, init_donghaer, ROT0, "Danbi",               "Donggul Donggul Haerong",              MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
GAME( 2004?,menghong, 0,        crzyddz2, crzyddz2, crystal_state, empty_init,    ROT0, "Sealy",               "Meng Hong Lou",                        MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
GAME( 2006, crzyddz2, 0,        crzyddz2, crzyddz2, crystal_state, empty_init,    ROT0, "Sealy",               "Crazy Dou Di Zhu II",                  MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION )
GAME( 2009, trivrus,  0,        trivrus,  trivrus,  crystal_state, empty_init,    ROT0, "AGT",                 "Trivia R Us (v1.07)",                  0 ) // has a CF card instead of flash roms
GAME( 200?, crospuzl, 0,        crospuzl, crospuzl, crystal_state, empty_init,    ROT0, "<unknown>",           "Cross Puzzle",                         MACHINE_NOT_WORKING )

GAME( 2004, psattack, 0,        crystal,  crystal,  crystal_state, init_psattack, ROT0, "Uniana",              "P's Attack",                           MACHINE_IS_SKELETON )
// looks like the same kind of hw from strings in the ROM, but scrambled / encrypted?
GAME( 200?, ddz,      0,        crystal,  crystal,  crystal_state, empty_init,    ROT0, "IGS?",                "Dou Di Zhu",                           MACHINE_IS_SKELETON )
GAME( 200?, crzclass, 0,        crystal,  crystal,  crystal_state, empty_init,    ROT0, "TJF",                 "Zhaoji Fengdou",                       MACHINE_IS_SKELETON ) // 'Crazy Class'
