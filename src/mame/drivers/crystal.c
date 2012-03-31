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
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT32 *  m_workram;
	UINT32 *  m_textureram;
	UINT32 *  m_frameram;
	UINT32 *  m_sysregs;
	UINT32 *  m_vidregs;
//  UINT32 *  m_nvram;    // currently this uses generic nvram handling

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
	UINT32    *m_ResetPatch;

	device_t *m_maincpu;
	device_t *m_ds1302;
	device_t *m_vr0video;
};

static void IntReq( running_machine &machine, int num )
{
	crystal_state *state = machine.driver_data<crystal_state>();
	address_space *space = state->m_maincpu->memory().space(AS_PROGRAM);
	UINT32 IntEn = space->read_dword(0x01800c08);
	UINT32 IntPend = space->read_dword(0x01800c0c);
	if (IntEn & (1 << num))
	{
		IntPend |= (1 << num);
		space->write_dword(0x01800c0c, IntPend);
		device_set_input_line(state->m_maincpu, SE3208_INT, ASSERT_LINE);
	}
#ifdef IDLE_LOOP_SPEEDUP
	state->m_FlipCntRead = 0;
	device_resume(state->m_maincpu, SUSPEND_REASON_SPIN);
#endif
}

static READ32_HANDLER( FlipCount_r )
{
	crystal_state *state = space->machine().driver_data<crystal_state>();

#ifdef IDLE_LOOP_SPEEDUP
	UINT32 IntPend = space->read_dword(0x01800c0c);
	state->m_FlipCntRead++;
	if (state->m_FlipCntRead >= 16 && !IntPend && state->m_FlipCount != 0)
		device_suspend(state->m_maincpu, SUSPEND_REASON_SPIN, 1);
#endif
	return ((UINT32) state->m_FlipCount) << 16;
}

static WRITE32_HANDLER( FlipCount_w )
{
	crystal_state *state = space->machine().driver_data<crystal_state>();

	if (mem_mask & 0x00ff0000)
	{
		int fc = (data >> 16) & 0xff;
		if (fc == 1)
			state->m_FlipCount++;
		else if (fc == 0)
			state->m_FlipCount = 0;
	}
}

static READ32_HANDLER( Input_r )
{
	crystal_state *state = space->machine().driver_data<crystal_state>();

	if (offset == 0)
		return input_port_read(space->machine(), "P1_P2");
	else if (offset == 1)
		return input_port_read(space->machine(), "P3_P4");
	else if( offset == 2)
	{
		UINT8 Port4 = input_port_read(space->machine(), "SYSTEM");
		if (!(Port4 & 0x10) && ((state->m_OldPort4 ^ Port4) & 0x10))	//coin buttons trigger IRQs
			IntReq(space->machine(), 12);
		if (!(Port4 & 0x20) && ((state->m_OldPort4 ^ Port4) & 0x20))
			IntReq(space->machine(), 19);
		state->m_OldPort4 = Port4;
		return /*dips*/input_port_read(space->machine(), "DSW") | (Port4 << 16);
	}
	return 0;
}

static WRITE32_HANDLER( IntAck_w )
{
	crystal_state *state = space->machine().driver_data<crystal_state>();
	UINT32 IntPend = space->read_dword(0x01800c0c);

	if (mem_mask & 0xff)
	{
		IntPend &= ~(1 << (data & 0x1f));
		space->write_dword(0x01800c0c, IntPend);
		if (!IntPend)
			device_set_input_line(state->m_maincpu, SE3208_INT, CLEAR_LINE);
	}
	if (mem_mask & 0xff00)
		state->m_IntHigh = (data >> 8) & 7;
}

static IRQ_CALLBACK( icallback )
{
	crystal_state *state = device->machine().driver_data<crystal_state>();
	address_space *space = device->memory().space(AS_PROGRAM);
	UINT32 IntPend = space->read_dword(0x01800c0c);
	int i;

	for (i = 0; i < 32; ++i)
	{
		if (BIT(IntPend, i))
		{
			return (state->m_IntHigh << 5) | i;
		}
	}
	return 0;		//This should never happen
}

static WRITE32_HANDLER( Banksw_w )
{
	crystal_state *state = space->machine().driver_data<crystal_state>();

	state->m_Bank = (data >> 1) & 7;
	if (state->m_Bank <= 2)
		memory_set_bankptr(space->machine(), "bank1", space->machine().region("user1")->base() + state->m_Bank * 0x1000000);
	else
		memory_set_bankptr(space->machine(), "bank1", space->machine().region("user2")->base());
}

static TIMER_CALLBACK( Timercb )
{
	crystal_state *state = machine.driver_data<crystal_state>();
	int which = (int)(FPTR)ptr;
	static const int num[] = { 0, 1, 9, 10 };

	if (!(state->m_Timerctrl[which] & 2))
		state->m_Timerctrl[which] &= ~1;

	IntReq(machine, num[which]);
}

INLINE void Timer_w( address_space *space, int which, UINT32 data, UINT32 mem_mask )
{
	crystal_state *state = space->machine().driver_data<crystal_state>();

	if (((data ^ state->m_Timerctrl[which]) & 1) && (data & 1))	//Timer activate
	{
		int PD = (data >> 8) & 0xff;
		int TCV = space->read_dword(0x01801404 + which * 8);
		attotime period = attotime::from_hz(43000000) * ((PD + 1) * (TCV + 1));

		if (state->m_Timerctrl[which] & 2)
			state->m_Timer[which]->adjust(period, 0, period);
		else
			state->m_Timer[which]->adjust(period);
	}
	COMBINE_DATA(&state->m_Timerctrl[which]);
}

static WRITE32_HANDLER( Timer0_w )
{
	Timer_w(space, 0, data, mem_mask);
}

static READ32_HANDLER( Timer0_r )
{
	crystal_state *state = space->machine().driver_data<crystal_state>();
	return state->m_Timerctrl[0];
}

static WRITE32_HANDLER( Timer1_w )
{
	Timer_w(space, 1, data, mem_mask);
}

static READ32_HANDLER( Timer1_r )
{
	crystal_state *state = space->machine().driver_data<crystal_state>();
	return state->m_Timerctrl[1];
}

static WRITE32_HANDLER( Timer2_w )
{
	Timer_w(space, 2, data, mem_mask);
}

static READ32_HANDLER( Timer2_r )
{
	crystal_state *state = space->machine().driver_data<crystal_state>();
	return state->m_Timerctrl[2];
}

static WRITE32_HANDLER( Timer3_w )
{
	Timer_w(space, 3, data, mem_mask);
}

static READ32_HANDLER( Timer3_r )
{
	crystal_state *state = space->machine().driver_data<crystal_state>();
	return state->m_Timerctrl[3];
}

static READ32_HANDLER( FlashCmd_r )
{
	crystal_state *state = space->machine().driver_data<crystal_state>();

	if ((state->m_FlashCmd & 0xff) == 0xff)
	{
		if (state->m_Bank <= 2)
		{
			UINT32 *ptr = (UINT32*)(space->machine().region("user1")->base() + state->m_Bank * 0x1000000);
			return ptr[0];
		}
		else
			return 0xffffffff;
	}
	if ((state->m_FlashCmd & 0xff) == 0x90)
	{
		if (state->m_Bank <= 2)
            return 0x00180089;	//Intel 128MBit
		else
			return 0xffffffff;
	}
	return 0;
}

static WRITE32_HANDLER( FlashCmd_w )
{
	crystal_state *state = space->machine().driver_data<crystal_state>();
	state->m_FlashCmd = data;
}

static READ32_HANDLER( PIO_r )
{
	crystal_state *state = space->machine().driver_data<crystal_state>();
	return state->m_PIO;
}

static WRITE32_HANDLER( PIO_w )
{
	crystal_state *state = space->machine().driver_data<crystal_state>();
	UINT32 RST = data & 0x01000000;
	UINT32 CLK = data & 0x02000000;
	UINT32 DAT = data & 0x10000000;

	if (!RST)
		state->m_ds1302->reset();

	ds1302_dat_w(state->m_ds1302, 0, DAT ? 1 : 0);
	ds1302_clk_w(state->m_ds1302, 0, CLK ? 1 : 0);

	if (ds1302_read(state->m_ds1302, 0))
		space->write_dword(0x01802008, space->read_dword(0x01802008) | 0x10000000);
	else
		space->write_dword(0x01802008, space->read_dword(0x01802008) & (~0x10000000));

	COMBINE_DATA(&state->m_PIO);
}

INLINE void DMA_w( address_space *space, int which, UINT32 data, UINT32 mem_mask )
{
	crystal_state *state = space->machine().driver_data<crystal_state>();

	if (((data ^ state->m_DMActrl[which]) & (1 << 10)) && (data & (1 << 10)))	//DMAOn
	{
		UINT32 CTR = data;
		UINT32 SRC = space->read_dword(0x01800804 + which * 0x10);
		UINT32 DST = space->read_dword(0x01800808 + which * 0x10);
		UINT32 CNT = space->read_dword(0x0180080C + which * 0x10);
		int i;

		if (CTR & 0x2)	//32 bits
		{
			for (i = 0; i < CNT; ++i)
			{
				UINT32 v = space->read_dword(SRC + i * 4);
				space->write_dword(DST + i * 4, v);
			}
		}
		else if (CTR & 0x1)	//16 bits
		{
			for (i = 0; i < CNT; ++i)
			{
				UINT16 v = space->read_word(SRC + i * 2);
				space->write_word(DST + i * 2, v);
			}
		}
		else	//8 bits
		{
			for (i = 0; i < CNT; ++i)
			{
				UINT8 v = space->read_byte(SRC + i);
				space->write_byte(DST + i, v);
			}
		}
		data &= ~(1 << 10);
		space->write_dword(0x0180080C + which * 0x10, 0);
		IntReq(space->machine(), 7 + which);
	}
	COMBINE_DATA(&state->m_DMActrl[which]);
}

static READ32_HANDLER( DMA0_r )
{
	crystal_state *state = space->machine().driver_data<crystal_state>();
	return state->m_DMActrl[0];
}

static WRITE32_HANDLER( DMA0_w )
{
	DMA_w(space, 0, data, mem_mask);
}

static READ32_HANDLER( DMA1_r )
{
	crystal_state *state = space->machine().driver_data<crystal_state>();
	return state->m_DMActrl[1];
}

static WRITE32_HANDLER( DMA1_w )
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
	AM_RANGE(0x01800000, 0x0180ffff) AM_RAM AM_BASE_MEMBER(crystal_state, m_sysregs)
	AM_RANGE(0x02000000, 0x027fffff) AM_RAM AM_BASE_MEMBER(crystal_state, m_workram)

	AM_RANGE(0x030000a4, 0x030000a7) AM_READWRITE(FlipCount_r, FlipCount_w)

	AM_RANGE(0x03000000, 0x0300ffff) AM_RAM AM_BASE_MEMBER(crystal_state, m_vidregs)
	AM_RANGE(0x03800000, 0x03ffffff) AM_RAM AM_BASE_MEMBER(crystal_state, m_textureram)
	AM_RANGE(0x04000000, 0x047fffff) AM_RAM AM_BASE_MEMBER(crystal_state, m_frameram)
	AM_RANGE(0x04800000, 0x04800fff) AM_DEVREADWRITE("vrender", vr0_snd_read, vr0_snd_write)

	AM_RANGE(0x05000000, 0x05000003) AM_READWRITE(FlashCmd_r, FlashCmd_w)
	AM_RANGE(0x05000000, 0x05ffffff) AM_ROMBANK("bank1")

	AM_RANGE(0x44414F4C, 0x44414F7F) AM_RAM AM_BASE_MEMBER(crystal_state, m_ResetPatch)

ADDRESS_MAP_END

static void PatchReset( running_machine &machine )
{
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

	crystal_state *state = machine.driver_data<crystal_state>();

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

	memcpy(state->m_ResetPatch, Patch, sizeof(Patch));
#else
	static const UINT8 Patch[] =
	{
		0x01,0xEA,0xC0,0x40,0x0A,0x40,0x06,0xE9,
		0x20,0x2A,0xC0,0x40,0x0A,0x40,0x06,0xE9,
		0x20,0x3A,0xD0,0xA1,0xFA,0xD4,0xF4,0xDE
	};

	memcpy(state->m_ResetPatch, Patch, sizeof(Patch));
#endif
}

static void crystal_banksw_postload(running_machine &machine)
{
	crystal_state *state = machine.driver_data<crystal_state>();

	if (state->m_Bank <= 2)
		memory_set_bankptr(machine, "bank1", machine.region("user1")->base() + state->m_Bank * 0x1000000);
	else
		memory_set_bankptr(machine, "bank1", machine.region("user2")->base());
}

static MACHINE_START( crystal )
{
	crystal_state *state = machine.driver_data<crystal_state>();
	int i;

	state->m_maincpu = machine.device("maincpu");
	state->m_ds1302 = machine.device("rtc");
	state->m_vr0video = machine.device("vr0");

	device_set_irq_callback(machine.device("maincpu"), icallback);
	for (i = 0; i < 4; i++)
		state->m_Timer[i] = machine.scheduler().timer_alloc(FUNC(Timercb), (void*)(FPTR)i);

	PatchReset(machine);

#ifdef IDLE_LOOP_SPEEDUP
	state->save_item(NAME(state->m_FlipCntRead));
#endif

	state->save_item(NAME(state->m_Bank));
	state->save_item(NAME(state->m_FlipCount));
	state->save_item(NAME(state->m_IntHigh));
	state->save_item(NAME(state->m_Timerctrl));
	state->save_item(NAME(state->m_FlashCmd));
	state->save_item(NAME(state->m_PIO));
	state->save_item(NAME(state->m_DMActrl));
	state->save_item(NAME(state->m_OldPort4));
	machine.save().register_postload(save_prepost_delegate(FUNC(crystal_banksw_postload), &machine));
}

static MACHINE_RESET( crystal )
{
	crystal_state *state = machine.driver_data<crystal_state>();
	int i;

	memset(state->m_sysregs, 0, 0x10000);
	memset(state->m_vidregs, 0, 0x10000);
	state->m_FlipCount = 0;
	state->m_IntHigh = 0;
	device_set_irq_callback(machine.device("maincpu"), icallback);
	state->m_Bank = 0;
	memory_set_bankptr(machine, "bank1", machine.region("user1")->base() + 0);
	state->m_FlashCmd = 0xff;
	state->m_OldPort4 = 0;

	state->m_DMActrl[0] = 0;
	state->m_DMActrl[1] = 0;

	for (i = 0; i < 4; i++)
	{
		state->m_Timerctrl[i] = 0;
		state->m_Timer[i]->adjust(attotime::never);
	}

	vr0_snd_set_areas(machine.device("vrender"), state->m_textureram, state->m_frameram);
#ifdef IDLE_LOOP_SPEEDUP
	state->m_FlipCntRead = 0;
#endif

	PatchReset(machine);
}

static UINT16 GetVidReg( address_space *space, UINT16 reg )
{
	return space->read_word(0x03000000 + reg);
}

static void SetVidReg( address_space *space, UINT16 reg, UINT16 val )
{
	space->write_word(0x03000000 + reg, val);
}


static SCREEN_UPDATE_IND16( crystal )
{
	crystal_state *state = screen.machine().driver_data<crystal_state>();
	address_space *space = screen.machine().device("maincpu")->memory().space(AS_PROGRAM);
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
		Front = (UINT16*) (state->m_frameram + B1 / 4);
		Back  = (UINT16*) (state->m_frameram + B0 / 4);
	}
	else
	{
		Front = (UINT16*) (state->m_frameram + B0 / 4);
		Back  = (UINT16*) (state->m_frameram + B1 / 4);
	}

	Visible  = (UINT16*) Front;
	DrawDest = (UINT16 *) state->m_frameram;


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
		DoFlip = vrender0_ProcessPacket(state->m_vr0video, 0x03800000 + head * 64, DrawDest, (UINT8*)state->m_textureram);
		head++;
		head &= 0x7ff;
		if (DoFlip)
			break;
	}

	if (DoFlip)
		SetVidReg(space, 0x8e, GetVidReg(space, 0x8e) ^ 1);

	srcline = (UINT16 *) Visible;
	for (y = 0; y < 240; y++)
		memcpy(&bitmap.pix16(y), &srcline[y * 512], width * 2);

	return 0;
}

static SCREEN_VBLANK(crystal)
{
	// rising edge
	if (vblank_on)
	{
		crystal_state *state = screen.machine().driver_data<crystal_state>();
		address_space *space = screen.machine().device("maincpu")->memory().space(AS_PROGRAM);
		UINT16 head, tail;
		int DoFlip = 0;

		head = GetVidReg(space, 0x82);
		tail = GetVidReg(space, 0x80);
		while ((head & 0x7ff) != (tail & 0x7ff))
		{
			UINT16 Packet0 = space->read_word(0x03800000 + head * 64);
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
			if (state->m_FlipCount)
				state->m_FlipCount--;

		}
	}
}

static INTERRUPT_GEN(crystal_interrupt)
{
	IntReq(device->machine(), 24);		//VRender0 VBlank
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

static const vr0_interface vr0_config =
{
	0x04800000
};

static const vr0video_interface vr0video_config =
{
	"maincpu"
};

static MACHINE_CONFIG_START( crystal, crystal_state )

	MCFG_CPU_ADD("maincpu", SE3208, 43000000)
	MCFG_CPU_PROGRAM_MAP(crystal_mem)
	MCFG_CPU_VBLANK_INT("screen", crystal_interrupt)

	MCFG_MACHINE_START(crystal)
	MCFG_MACHINE_RESET(crystal)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE_STATIC(crystal)
	MCFG_SCREEN_VBLANK_STATIC(crystal)

	MCFG_VIDEO_VRENDER0_ADD("vr0", vr0video_config)

	MCFG_PALETTE_INIT(RRRRR_GGGGGG_BBBBB)
	MCFG_PALETTE_LENGTH(65536)

	MCFG_DS1302_ADD("rtc")

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("vrender", VRENDER0, 0)
	MCFG_SOUND_CONFIG(vr0_config)
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

ROM_START( crysbios )
	ROM_REGION( 0x20000, "maincpu", 0 ) // bios
	ROM_LOAD("mx27l1000.u14",  0x000000, 0x020000, CRC(BEFF39A9) SHA1(b6f6dda58d9c82273f9422c1bd623411e58982cb) )

	ROM_REGION32_LE( 0x3000000, "user1", ROMREGION_ERASEFF ) // Flash

	ROM_REGION( 0x10000, "user2",	ROMREGION_ERASEFF )	//Unmapped flash
ROM_END

ROM_START( crysking )
	ROM_REGION( 0x20000, "maincpu", 0 ) // bios
	ROM_LOAD("mx27l1000.u14",  0x000000, 0x020000, CRC(BEFF39A9) SHA1(b6f6dda58d9c82273f9422c1bd623411e58982cb))

	ROM_REGION32_LE( 0x3000000, "user1", 0 ) // Flash
	ROM_LOAD("bcsv0004f01.u1",  0x0000000, 0x1000000, CRC(8FEFF120) SHA1(2ea42fa893bff845b5b855e2556789f8354e9066) )
	ROM_LOAD("bcsv0004f02.u2",  0x1000000, 0x1000000, CRC(0E799845) SHA1(419674ce043cb1efb18303f4cb7fdbbae642ee39) )
	ROM_LOAD("bcsv0004f03.u3",  0x2000000, 0x1000000, CRC(659E2D17) SHA1(342c98f3f695ef4dea8b533612451c4d2fb58809) )

	ROM_REGION( 0x10000, "user2",	ROMREGION_ERASEFF )	//Unmapped flash
ROM_END

ROM_START( evosocc )
	ROM_REGION( 0x20000, "maincpu", 0 ) // bios
	ROM_LOAD("mx27l1000.u14",  0x000000, 0x020000, CRC(BEFF39A9) SHA1(b6f6dda58d9c82273f9422c1bd623411e58982cb))

	ROM_REGION32_LE( 0x3000000, "user1", 0 ) // Flash
	ROM_LOAD("bcsv0001u01",  0x0000000, 0x1000000, CRC(2581A0EA) SHA1(ee483ac60a3ed00a21cb515974cec4af19916a7d) )
	ROM_LOAD("bcsv0001u02",  0x1000000, 0x1000000, CRC(47EF1794) SHA1(f573706c17d1342b9b7aed9b40b8b648f0bf58db) )
	ROM_LOAD("bcsv0001u03",  0x2000000, 0x1000000, CRC(F396A2EC) SHA1(f305eb10856fb5d4c229a6b09d6a2fb21b24ce66) )

	ROM_REGION( 0x10000, "user2",	ROMREGION_ERASEFF )	//Unmapped flash
ROM_END

ROM_START( topbladv )
	ROM_REGION( 0x20000, "maincpu", 0 ) // bios
	ROM_LOAD("mx27l1000.u14",  0x000000, 0x020000, CRC(BEFF39A9) SHA1(b6f6dda58d9c82273f9422c1bd623411e58982cb))

	ROM_REGION( 0x4300, "pic", 0 ) // pic16c727 - we don't have a core for this
	ROM_LOAD("top_blade_v_pic16c727.bin",  0x000000, 0x4300, CRC(9cdea57b) SHA1(884156085f9e780cdf719aedc2e8a0fd5983613b) )


	ROM_REGION32_LE( 0x1000000, "user1", 0 ) // Flash
	ROM_LOAD("flash.u1",  0x0000000, 0x1000000, CRC(bd23f640) SHA1(1d22aa2c828642bb7c1dfea4e13f777f95acc701) )

	ROM_REGION( 0x10000, "user2",	ROMREGION_ERASEFF )	//Unmapped flash
ROM_END


ROM_START( officeye )
	ROM_REGION( 0x20000, "maincpu", 0 ) // bios (not the standard one)
	ROM_LOAD("bios.u14",  0x000000, 0x020000, CRC(ffc57e90) SHA1(6b6a17fd4798dea9c7b880f3063be8494e7db302) )

	ROM_REGION( 0x4280, "pic", 0 ) // pic16f84a - we don't have a core for this
	ROM_LOAD("office_yeo_in_cheon_ha_pic16f84a.bin",  0x000000, 0x4280, CRC(7561cdf5) SHA1(eade592823a110019b4af81a7dc56d01f7d6589f) )


	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Flash
	ROM_LOAD("flash.u1",  0x0000000, 0x1000000, CRC(d3f3eec4) SHA1(ea728415bd4906964b7d37f4379a8a3bd42a1c2d) )
	ROM_LOAD("flash.u2",  0x1000000, 0x1000000, CRC(e4f85d0a) SHA1(2ddfa6b3a30e69754aa9d96434ff3d37784bfa57) )

	ROM_REGION( 0x10000, "user2",	ROMREGION_ERASEFF )	//Unmapped flash
ROM_END


ROM_START( donghaer )
	ROM_REGION( 0x20000, "maincpu", 0 ) // bios
	ROM_LOAD("mx27l1000.u14",  0x000000, 0x020000, CRC(BEFF39A9) SHA1(b6f6dda58d9c82273f9422c1bd623411e58982cb))

	ROM_REGION( 0x4280, "pic", 0 ) // pic16f84a - we don't have a core for this (or the dump in this case)
	ROM_LOAD("donghaer_pic16f84a.bin",  0x000000, 0x4280, NO_DUMP )

	ROM_REGION32_LE( 0x2000000, "user1", 0 ) // Flash
	ROM_LOAD( "u1",           0x0000000, 0x1000000, CRC(61217ad7) SHA1(2593f1356aa850f4f9aa5d00bec822aa59c59224) )
	ROM_LOAD( "u2",           0x1000000, 0x1000000, CRC(6d82f1a5) SHA1(036bd45f0daac1ffeaa5ad9774fc1b56e3c75ff9) )

	ROM_REGION( 0x10000, "user2",	ROMREGION_ERASEFF )	//Unmapped flash
ROM_END

static DRIVER_INIT(crysking)
{
	UINT16 *Rom = (UINT16*) machine.region("user1")->base();

	//patch the data feed by the protection

	Rom[WORD_XOR_LE(0x7bb6/2)] = 0xDF01;
	Rom[WORD_XOR_LE(0x7bb8/2)] = 0x9C00;

	Rom[WORD_XOR_LE(0x976a/2)] = 0x901C;
	Rom[WORD_XOR_LE(0x976c/2)] = 0x9001;

	Rom[WORD_XOR_LE(0x8096/2)] = 0x90FC;
	Rom[WORD_XOR_LE(0x8098/2)] = 0x9001;

	Rom[WORD_XOR_LE(0x8a52/2)] = 0x4000;	//NOP
	Rom[WORD_XOR_LE(0x8a54/2)] = 0x403c;	//NOP
}

static DRIVER_INIT(evosocc)
{
	UINT16 *Rom = (UINT16*) machine.region("user1")->base();
	Rom += 0x1000000 * 2 / 2;

	Rom[WORD_XOR_LE(0x97388E/2)] = 0x90FC;	//PUSH R2..R7
	Rom[WORD_XOR_LE(0x973890/2)] = 0x9001;	//PUSH R0

	Rom[WORD_XOR_LE(0x971058/2)] = 0x907C;	//PUSH R2..R6
	Rom[WORD_XOR_LE(0x971060/2)] = 0x9001; //PUSH R0

	Rom[WORD_XOR_LE(0x978036/2)] = 0x900C;	//PUSH R2-R3
	Rom[WORD_XOR_LE(0x978038/2)] = 0x8303;	//LD    (%SP,0xC),R3

	Rom[WORD_XOR_LE(0x974ED0/2)] = 0x90FC;	//PUSH R7-R6-R5-R4-R3-R2
	Rom[WORD_XOR_LE(0x974ED2/2)] = 0x9001;	//PUSH R0
}

static DRIVER_INIT(topbladv)
{
	UINT16 *Rom = (UINT16*) machine.region("user1")->base();

	Rom[WORD_XOR_LE(0x12d7a/2)] = 0x90FC;	//PUSH R7-R6-R5-R4-R3-R2
	Rom[WORD_XOR_LE(0x12d7c/2)] = 0x9001;	//PUSH R0

	Rom[WORD_XOR_LE(0x2fe18/2)] = 0x9001;	//PUSH R0
	Rom[WORD_XOR_LE(0x2fe1a/2)] = 0x9200;	//PUSH SR

	Rom[WORD_XOR_LE(0x18880/2)] = 0x9001;	//PUSH R0
	Rom[WORD_XOR_LE(0x18882/2)] = 0x9200;	//PUSH SR

	Rom[WORD_XOR_LE(0xDACE/2)] = 0x901C;	//PUSH R4-R3-R2
	Rom[WORD_XOR_LE(0xDAD0/2)] = 0x9001;	//PUSH R0

}

static DRIVER_INIT(officeye)
{
	UINT16 *Rom = (UINT16*) machine.region("user1")->base();

	Rom[WORD_XOR_LE(0x9c9e/2)] = 0x901C;	//PUSH R4-R3-R2
	Rom[WORD_XOR_LE(0x9ca0/2)] = 0x9001;	//PUSH R0

	Rom[WORD_XOR_LE(0x9EE4/2)] = 0x907C;	//PUSH R6-R5-R4-R3-R2
	Rom[WORD_XOR_LE(0x9EE6/2)] = 0x9001;	//PUSH R0

	Rom[WORD_XOR_LE(0x4B2E0/2)] = 0x9004;	//PUSH R2
	Rom[WORD_XOR_LE(0x4B2E2/2)] = 0x9001;	//PUSH R0

/*
    Rom[WORD_XOR_LE(0x18880/2)] = 0x9001; //PUSH R0
    Rom[WORD_XOR_LE(0x18882/2)] = 0x9200; //PUSH SR
 */
}



GAME( 2001, crysbios,        0, crystal,  crystal,         0, ROT0, "BrezzaSoft", "Crystal System BIOS", GAME_IS_BIOS_ROOT )
GAME( 2001, crysking, crysbios, crystal,  crystal,  crysking, ROT0, "BrezzaSoft", "The Crystal of Kings", 0 )
GAME( 2001, evosocc,  crysbios, crystal,  crystal,  evosocc,  ROT0, "Evoga", "Evolution Soccer", 0 )
GAME( 2003, topbladv, crysbios, topbladv, crystal,  topbladv, ROT0, "SonoKong / Expotato", "Top Blade V", GAME_NOT_WORKING ) // protection
GAME( 2001, officeye,        0, crystal,  crystal,  officeye, ROT0, "Danbi", "Office Yeo In Cheon Ha (version 1.2)", GAME_NOT_WORKING ) // protection
GAME( 2001, donghaer,        0, crystal,  crystal,  officeye, ROT0, "Danbi", "Donggul Donggul Haerong", GAME_NOT_WORKING )
