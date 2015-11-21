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
There are only two known games running on this system, Crystal of Kings and Evolution Soccer.
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
#include "video/vrender0.h"
#include "machine/ds1302.h"
#include "sound/vrender0.h"
#include "machine/nvram.h"

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
		m_ds1302(*this, "rtc") { }

	/* memory pointers */
	required_shared_ptr<UINT32> m_sysregs;
	required_shared_ptr<UINT32> m_workram;
	required_shared_ptr<UINT32> m_vidregs;
	required_shared_ptr<UINT32> m_textureram;
	required_shared_ptr<UINT32> m_frameram;
	optional_shared_ptr<UINT32> m_reset_patch; // not needed for trivrus
//  UINT32 *  m_nvram;    // currently this uses generic nvram handling

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<vr0video_device> m_vr0;
	required_device<ds1302_device> m_ds1302;

#ifdef IDLE_LOOP_SPEEDUP
	UINT8     m_FlipCntRead;
#endif

	UINT32    m_Bank;
	UINT8     m_FlipCount;
	UINT8     m_IntHigh;
	UINT32    m_Timerctrl[4];
	emu_timer *m_Timer[4];
	UINT32    m_FlashCmd;
	UINT32    m_PIO;
	UINT32    m_DMActrl[2];
	UINT8     m_OldPort4;

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
	DECLARE_DRIVER_INIT(topbladv);
	DECLARE_DRIVER_INIT(officeye);
	DECLARE_DRIVER_INIT(crysking);
	DECLARE_DRIVER_INIT(evosocc);
	DECLARE_DRIVER_INIT(donghaer);

	DECLARE_READ32_MEMBER(trivrus_input_r);
	DECLARE_WRITE32_MEMBER(trivrus_input_w);
	UINT8 m_trivrus_input;

	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_crystal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_crystal(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(crystal_interrupt);
	TIMER_CALLBACK_MEMBER(Timercb);
	IRQ_CALLBACK_MEMBER(icallback);
	void crystal_banksw_postload();
	void IntReq( int num );
	inline void Timer_w( address_space &space, int which, UINT32 data, UINT32 mem_mask );
	inline void DMA_w( address_space &space, int which, UINT32 data, UINT32 mem_mask );
	void PatchReset(  );
	UINT16 GetVidReg( address_space &space, UINT16 reg );
	void SetVidReg( address_space &space, UINT16 reg, UINT16 val );
};

void crystal_state::IntReq( int num )
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	UINT32 IntEn = space.read_dword(0x01800c08);
	UINT32 IntPend = space.read_dword(0x01800c0c);
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
	UINT32 IntPend = space.read_dword(0x01800c0c);
	m_FlipCntRead++;
	if (m_FlipCntRead >= 16 && !IntPend && m_FlipCount != 0)
		m_maincpu->suspend(SUSPEND_REASON_SPIN, 1);
#endif
	return ((UINT32) m_FlipCount) << 16;
}

WRITE32_MEMBER(crystal_state::FlipCount_w)
{
	if (mem_mask & 0x00ff0000)
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
		UINT8 Port4 = ioport("SYSTEM")->read();
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
	UINT32 IntPend = space.read_dword(0x01800c0c);

	if (mem_mask & 0xff)
	{
		IntPend &= ~(1 << (data & 0x1f));
		space.write_dword(0x01800c0c, IntPend);
		if (!IntPend)
			m_maincpu->set_input_line(SE3208_INT, CLEAR_LINE);
	}
	if (mem_mask & 0xff00)
		m_IntHigh = (data >> 8) & 7;
}

IRQ_CALLBACK_MEMBER(crystal_state::icallback)
{
	address_space &space = device.memory().space(AS_PROGRAM);
	UINT32 IntPend = space.read_dword(0x01800c0c);
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

TIMER_CALLBACK_MEMBER(crystal_state::Timercb)
{
	int which = (int)(FPTR)ptr;
	static const int num[] = { 0, 1, 9, 10 };

	if (!(m_Timerctrl[which] & 2))
		m_Timerctrl[which] &= ~1;

	IntReq(num[which]);
}

void crystal_state::Timer_w( address_space &space, int which, UINT32 data, UINT32 mem_mask )
{
	if (((data ^ m_Timerctrl[which]) & 1) && (data & 1)) //Timer activate
	{
		int PD = (data >> 8) & 0xff;
		int TCV = space.read_dword(0x01801404 + which * 8);
		attotime period = attotime::from_hz(43000000) * ((PD + 1) * (TCV + 1));

		if (m_Timerctrl[which] & 2)
			m_Timer[which]->adjust(period, 0, period);
		else
			m_Timer[which]->adjust(period);
	}
	COMBINE_DATA(&m_Timerctrl[which]);
}

WRITE32_MEMBER(crystal_state::Timer0_w)
{
	Timer_w(space, 0, data, mem_mask);
}

READ32_MEMBER(crystal_state::Timer0_r)
{
	return m_Timerctrl[0];
}

WRITE32_MEMBER(crystal_state::Timer1_w)
{
	Timer_w(space, 1, data, mem_mask);
}

READ32_MEMBER(crystal_state::Timer1_r)
{
	return m_Timerctrl[1];
}

WRITE32_MEMBER(crystal_state::Timer2_w)
{
	Timer_w(space, 2, data, mem_mask);
}

READ32_MEMBER(crystal_state::Timer2_r)
{
	return m_Timerctrl[2];
}

WRITE32_MEMBER(crystal_state::Timer3_w)
{
	Timer_w(space, 3, data, mem_mask);
}

READ32_MEMBER(crystal_state::Timer3_r)
{
	return m_Timerctrl[3];
}

READ32_MEMBER(crystal_state::FlashCmd_r)
{
	if ((m_FlashCmd & 0xff) == 0xff)
	{
		if (m_Bank <= 2)
		{
			UINT32 *ptr = (UINT32*)(memregion("user1")->base() + m_Bank * 0x1000000);
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
	UINT32 RST = data & 0x01000000;
	UINT32 CLK = data & 0x02000000;
	UINT32 DAT = data & 0x10000000;

	m_ds1302->ce_w(RST ? 1 : 0);
	m_ds1302->io_w(DAT ? 1 : 0);
	m_ds1302->sclk_w(CLK ? 1 : 0);

	if (m_ds1302->io_r())
		space.write_dword(0x01802008, space.read_dword(0x01802008) | 0x10000000);
	else
		space.write_dword(0x01802008, space.read_dword(0x01802008) & (~0x10000000));

	COMBINE_DATA(&m_PIO);
}

void crystal_state::DMA_w( address_space &space, int which, UINT32 data, UINT32 mem_mask )
{
	if (((data ^ m_DMActrl[which]) & (1 << 10)) && (data & (1 << 10)))   //DMAOn
	{
		UINT32 CTR = data;
		UINT32 SRC = space.read_dword(0x01800804 + which * 0x10);
		UINT32 DST = space.read_dword(0x01800808 + which * 0x10);
		UINT32 CNT = space.read_dword(0x0180080C + which * 0x10);
		int i;

		if (CTR & 0x2)  //32 bits
		{
			for (i = 0; i < CNT; ++i)
			{
				UINT32 v = space.read_dword(SRC + i * 4);
				space.write_dword(DST + i * 4, v);
			}
		}
		else if (CTR & 0x1) //16 bits
		{
			for (i = 0; i < CNT; ++i)
			{
				UINT16 v = space.read_word(SRC + i * 2);
				space.write_word(DST + i * 2, v);
			}
		}
		else    //8 bits
		{
			for (i = 0; i < CNT; ++i)
			{
				UINT8 v = space.read_byte(SRC + i);
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


static ADDRESS_MAP_START( crystal_mem, AS_PROGRAM, 32, crystal_state )
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM AM_WRITENOP

	AM_RANGE(0x01200000, 0x0120000f) AM_READ(Input_r)
	AM_RANGE(0x01280000, 0x01280003) AM_WRITE(Banksw_w)
	AM_RANGE(0x01400000, 0x0140ffff) AM_RAM AM_SHARE("nvram")

	AM_RANGE(0x01801400, 0x01801403) AM_READWRITE(Timer0_r, Timer0_w)
	AM_RANGE(0x01801408, 0x0180140b) AM_READWRITE(Timer1_r, Timer1_w)
	AM_RANGE(0x01801410, 0x01801413) AM_READWRITE(Timer2_r, Timer2_w)
	AM_RANGE(0x01801418, 0x0180141b) AM_READWRITE(Timer3_r, Timer3_w)
	AM_RANGE(0x01802004, 0x01802007) AM_READWRITE(PIO_r, PIO_w)

	AM_RANGE(0x01800800, 0x01800803) AM_READWRITE(DMA0_r, DMA0_w)
	AM_RANGE(0x01800810, 0x01800813) AM_READWRITE(DMA1_r, DMA1_w)

	AM_RANGE(0x01800c04, 0x01800c07) AM_WRITE(IntAck_w)
	AM_RANGE(0x01800000, 0x0180ffff) AM_RAM AM_SHARE("sysregs")
	AM_RANGE(0x02000000, 0x027fffff) AM_RAM AM_SHARE("workram")

	AM_RANGE(0x030000a4, 0x030000a7) AM_READWRITE(FlipCount_r, FlipCount_w)

	AM_RANGE(0x03000000, 0x0300ffff) AM_RAM AM_SHARE("vidregs")
	AM_RANGE(0x03800000, 0x03ffffff) AM_RAM AM_SHARE("textureram")
	AM_RANGE(0x04000000, 0x047fffff) AM_RAM AM_SHARE("frameram")
	AM_RANGE(0x04800000, 0x04800fff) AM_DEVREADWRITE("vrender", vrender0_device, vr0_snd_read, vr0_snd_write)

	AM_RANGE(0x05000000, 0x05000003) AM_READWRITE(FlashCmd_r, FlashCmd_w)
	AM_RANGE(0x05000000, 0x05ffffff) AM_ROMBANK("bank1")

	AM_RANGE(0x44414F4C, 0x44414F7F) AM_RAM AM_SHARE("reset_patch")

ADDRESS_MAP_END

// Trivia R Us
// To do: touch panel, RTC

READ32_MEMBER(crystal_state::trivrus_input_r)
{
	switch (m_trivrus_input)
	{
		case 1:	return ioport("IN1")->read();
		case 2:	return ioport("IN2")->read();
		case 3:	return ioport("IN3")->read();
		case 4:	return ioport("IN4")->read();
		case 5:	return ioport("IN5")->read();
		case 6:	return ioport("DSW")->read();
	}
	logerror("%s: unknown input %02x read\n", machine().describe_context(), m_trivrus_input);
	return 0xffffffff;
}

WRITE32_MEMBER(crystal_state::trivrus_input_w)
{
	if (ACCESSING_BITS_0_7)
		m_trivrus_input = data & 0xff;
}

static ADDRESS_MAP_START( trivrus_mem, AS_PROGRAM, 32, crystal_state )
	AM_RANGE(0x00000000, 0x0007ffff) AM_ROM AM_WRITENOP

//	0x01280000 & 0x0000ffff (written at boot)
	AM_RANGE(0x01500000, 0x01500003) AM_READWRITE(trivrus_input_r, trivrus_input_w)
//	0x01500010 & 0x000000ff = sec
//	0x01500010 & 0x00ff0000 = min
//	0x01500014 & 0x000000ff = hour
//	0x01500014 & 0x00ff0000 = day
//	0x01500018 & 0x000000ff = month
//	0x0150001c & 0x000000ff = year - 2000
	AM_RANGE(0x01600000, 0x01607fff) AM_RAM AM_SHARE("nvram")

	AM_RANGE(0x01801400, 0x01801403) AM_READWRITE(Timer0_r, Timer0_w)
	AM_RANGE(0x01801408, 0x0180140b) AM_READWRITE(Timer1_r, Timer1_w)
	AM_RANGE(0x01801410, 0x01801413) AM_READWRITE(Timer2_r, Timer2_w)
	AM_RANGE(0x01801418, 0x0180141b) AM_READWRITE(Timer3_r, Timer3_w)
	AM_RANGE(0x01802004, 0x01802007) AM_READWRITE(PIO_r, PIO_w)

	AM_RANGE(0x01800800, 0x01800803) AM_READWRITE(DMA0_r, DMA0_w)
	AM_RANGE(0x01800810, 0x01800813) AM_READWRITE(DMA1_r, DMA1_w)

	AM_RANGE(0x01800c04, 0x01800c07) AM_WRITE(IntAck_w)
	AM_RANGE(0x01800000, 0x0180ffff) AM_RAM AM_SHARE("sysregs")
	AM_RANGE(0x02000000, 0x027fffff) AM_RAM AM_SHARE("workram")

	AM_RANGE(0x030000a4, 0x030000a7) AM_READWRITE(FlipCount_r, FlipCount_w)

	AM_RANGE(0x03000000, 0x0300ffff) AM_RAM AM_SHARE("vidregs")
	AM_RANGE(0x03800000, 0x03ffffff) AM_RAM AM_SHARE("textureram")
	AM_RANGE(0x04000000, 0x047fffff) AM_RAM AM_SHARE("frameram")
	AM_RANGE(0x04800000, 0x04800fff) AM_DEVREADWRITE("vrender", vrender0_device, vr0_snd_read, vr0_snd_write)

	AM_RANGE(0x05000000, 0x05000003) AM_READWRITE(FlashCmd_r, FlashCmd_w)
	AM_RANGE(0x05000000, 0x05ffffff) AM_ROMBANK("bank1")

//	AM_RANGE(0x44414F4C, 0x44414F7F) AM_RAM AM_SHARE("reset_patch")

ADDRESS_MAP_END


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
	static const UINT32 Patch[] =
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
	static const UINT8 Patch[] =
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
		m_Timer[i] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(crystal_state::Timercb),this), (void*)(FPTR)i);

	PatchReset();

#ifdef IDLE_LOOP_SPEEDUP
	save_item(NAME(m_FlipCntRead));
#endif

	save_item(NAME(m_Bank));
	save_item(NAME(m_FlipCount));
	save_item(NAME(m_IntHigh));
	save_item(NAME(m_Timerctrl));
	save_item(NAME(m_FlashCmd));
	save_item(NAME(m_PIO));
	save_item(NAME(m_DMActrl));
	save_item(NAME(m_OldPort4));
	save_item(NAME(m_trivrus_input));
	machine().save().register_postload(save_prepost_delegate(FUNC(crystal_state::crystal_banksw_postload), this));
}

void crystal_state::machine_reset()
{
	int i;

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

	for (i = 0; i < 4; i++)
	{
		m_Timerctrl[i] = 0;
		m_Timer[i]->adjust(attotime::never);
	}

	machine().device<vrender0_device>("vrender")->set_areas(m_textureram, m_frameram);
#ifdef IDLE_LOOP_SPEEDUP
	m_FlipCntRead = 0;
#endif

	PatchReset();
}

UINT16 crystal_state::GetVidReg( address_space &space, UINT16 reg )
{
	return space.read_word(0x03000000 + reg);
}

void crystal_state::SetVidReg( address_space &space, UINT16 reg, UINT16 val )
{
	space.write_word(0x03000000 + reg, val);
}


UINT32 crystal_state::screen_update_crystal(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int DoFlip;

	UINT32 B0 = 0x0;
	UINT32 B1 = (GetVidReg(space, 0x90) & 0x8000) ? 0x400000 : 0x100000;
	UINT16 *Front, *Back;
	UINT16 *Visible, *DrawDest;
	UINT16 *srcline;
	int y;
	UINT16 head, tail;
	UINT32 width = screen.width();

	if (GetVidReg(space, 0x8e) & 1)
	{
		Front = (UINT16*) (m_frameram + B1 / 4);
		Back  = (UINT16*) (m_frameram + B0 / 4);
	}
	else
	{
		Front = (UINT16*) (m_frameram + B0 / 4);
		Back  = (UINT16*) (m_frameram + B1 / 4);
	}

	Visible  = (UINT16*) Front;
	// ERROR: This cast is NOT endian-safe without the use of BYTE/WORD/DWORD_XOR_* macros!
	DrawDest = reinterpret_cast<UINT16 *>(m_frameram.target());


	if (GetVidReg(space, 0x8c) & 0x80)
		DrawDest = Front;
	else
		DrawDest = Back;

//  DrawDest = Visible;

	srcline = (UINT16 *) DrawDest;

	DoFlip = 0;
	head = GetVidReg(space, 0x82);
	tail = GetVidReg(space, 0x80);
	while ((head & 0x7ff) != (tail & 0x7ff))
	{
		// ERROR: This cast is NOT endian-safe without the use of BYTE/WORD/DWORD_XOR_* macros!
		DoFlip = m_vr0->vrender0_ProcessPacket(0x03800000 + head * 64, DrawDest, reinterpret_cast<UINT8*>(m_textureram.target()));
		head++;
		head &= 0x7ff;
		if (DoFlip)
			break;
	}

	if (DoFlip)
		SetVidReg(space, 0x8e, GetVidReg(space, 0x8e) ^ 1);

	srcline = (UINT16 *) Visible;
	for (y = 0; y < screen.height(); y++)
		memcpy(&bitmap.pix16(y), &srcline[y * 1024], width * 2);

	return 0;
}

void crystal_state::screen_eof_crystal(screen_device &screen, bool state)
{
	// rising edge
	if (state)
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		UINT16 head, tail;
		int DoFlip = 0;

		head = GetVidReg(space, 0x82);
		tail = GetVidReg(space, 0x80);
		while ((head & 0x7ff) != (tail & 0x7ff))
		{
			UINT16 Packet0 = space.read_word(0x03800000 + head * 64);
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
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0000ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Red")  // RED
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Green")  // GREEN
	PORT_BIT( 0x00080000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Blue") // BLUE
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_START2 ) // where is start3?
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
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_SERVICE1 )	// Free Game
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x08, IP_ACTIVE_LOW )	// Setup
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
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )		// hangs at boot
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


static MACHINE_CONFIG_START( crystal, crystal_state )

	MCFG_CPU_ADD("maincpu", SE3208, 43000000)
	MCFG_CPU_PROGRAM_MAP(crystal_mem)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", crystal_state,  crystal_interrupt)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(crystal_state, icallback)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE_DRIVER(crystal_state, screen_update_crystal)
	MCFG_SCREEN_VBLANK_DRIVER(crystal_state, screen_eof_crystal)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("vr0", VIDEO_VRENDER0, 0)
	MCFG_VIDEO_VRENDER0_CPU("maincpu")

	MCFG_PALETTE_ADD_RRRRRGGGGGGBBBBB("palette")

	MCFG_DS1302_ADD("rtc", XTAL_32_768kHz)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_VRENDER0_ADD("vrender", 0)
	MCFG_VR0_REGBASE(0x04800000)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

/*
    Top blade screen is 32 pixels wider
*/
static MACHINE_CONFIG_DERIVED( topbladv, crystal )

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(320+32, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 319+32, 0, 239)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( trivrus, crystal )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(trivrus_mem)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
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


DRIVER_INIT_MEMBER(crystal_state,crysking)
{
	UINT16 *Rom = (UINT16*) memregion("user1")->base();

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

DRIVER_INIT_MEMBER(crystal_state,evosocc)
{
	UINT16 *Rom = (UINT16*) memregion("user1")->base();
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

DRIVER_INIT_MEMBER(crystal_state,topbladv)
{
	// patches based on analysis of PIC dump
	UINT16 *Rom = (UINT16*) memregion("user1")->base();
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

DRIVER_INIT_MEMBER(crystal_state,officeye)
{
	// patches based on analysis of PIC dump
	UINT16 *Rom = (UINT16*) memregion("user1")->base();

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

DRIVER_INIT_MEMBER(crystal_state, donghaer)
{
	UINT16 *Rom = (UINT16*)memregion("user1")->base();

	Rom[WORD_XOR_LE(0x037A2 / 2)] = 0x9004; // PUSH %R2
	Rom[WORD_XOR_LE(0x037A4 / 2)] = 0x8202; // LD   (%SP,0x8),%R2

	Rom[WORD_XOR_LE(0x03834 / 2)] = 0x9001; // PUSH %R0
	Rom[WORD_XOR_LE(0x03836 / 2)] = 0x9200; // PUSH %SR

	Rom[WORD_XOR_LE(0x0AC9E / 2)] = 0x9004; // PUSH %R2
	Rom[WORD_XOR_LE(0x0ACA0 / 2)] = 0x4081; // LERI 0x81

	Rom[WORD_XOR_LE(0x19C70 / 2)] = 0x900C; // PUSH %R2-%R3
	Rom[WORD_XOR_LE(0x19C72 / 2)] = 0x9001; // PUSH %R0
}


GAME( 2001, crysbios,        0, crystal,  crystal, driver_device,         0, ROT0, "BrezzaSoft",          "Crystal System BIOS",                  MACHINE_IS_BIOS_ROOT )
GAME( 2001, crysking, crysbios, crystal,  crystal, crystal_state,  crysking, ROT0, "BrezzaSoft",          "The Crystal of Kings",                 0 )
GAME( 2001, evosocc,  crysbios, crystal,  crystal, crystal_state,  evosocc,  ROT0, "Evoga",               "Evolution Soccer",                     0 )
GAME( 2003, topbladv, crysbios, topbladv, crystal, crystal_state,  topbladv, ROT0, "SonoKong / Expotato", "Top Blade V",                          0 )
GAME( 2001, officeye,        0, crystal,  officeye,crystal_state,  officeye, ROT0, "Danbi",               "Office Yeo In Cheon Ha (version 1.2)", MACHINE_NOT_WORKING ) // still has some instability issues
GAME( 2001, donghaer,        0, crystal,  crystal, crystal_state,  donghaer, ROT0, "Danbi",               "Donggul Donggul Haerong",              MACHINE_NOT_WORKING )
GAME( 2009, trivrus,         0, trivrus,  trivrus, driver_device,         0, ROT0, "AGT",                 "Trivia R Us (v1.07)",                  0 )
