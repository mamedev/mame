/*
    CRYSTAL SYSTEM by Brezzasoft (2001)
    using VRender0 System on a Chip

    The VRender0 (http://www.mesdigital.com/english/Products/product_vrender0.asp) chip contains:
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

#include "driver.h"
#include "cpu/se3208/se3208.h"
#include "video/vrender0.h"
#include "machine/ds1302.h"
#include "sound/vrender0.h"

#define IDLE_LOOP_SPEEDUP

#ifdef IDLE_LOOP_SPEEDUP
static UINT8 FlipCntRead;
#endif

static UINT32 *workram,*textureram,*frameram;
static UINT32 *sysregs,*vidregs;
static UINT32 Bank;
static UINT8 FlipCount,IntHigh;
static UINT32 Timer0ctrl,Timer1ctrl,Timer2ctrl,Timer3ctrl;
static void *Timer0,*Timer1,*Timer2,*Timer3;
static UINT32 FlashCmd,PIO;
static UINT32 DMA0ctrl,DMA1ctrl;
static UINT8 OldPort4;
static UINT32 *ResetPatch;

static void IntReq(running_machine *machine, int num)
{
	UINT32 IntEn=program_read_dword_32le(0x01800c08);
	UINT32 IntPend=program_read_dword_32le(0x01800c0c);
	if(IntEn&(1<<num))
	{
		IntPend|=(1<<num);
		program_write_dword_32le(0x01800c0c,IntPend);
		cpunum_set_input_line(machine, 0,SE3208_INT,ASSERT_LINE);
	}
#ifdef IDLE_LOOP_SPEEDUP
	FlipCntRead=0;
	cpunum_resume(0,SUSPEND_REASON_SPIN);
#endif
}

static READ32_HANDLER(FlipCount_r)
{
#ifdef IDLE_LOOP_SPEEDUP
	UINT32 IntPend=program_read_dword_32le(0x01800c0c);
	FlipCntRead++;
	if(FlipCntRead>=16 && !IntPend && FlipCount!=0)
		cpunum_suspend(0,SUSPEND_REASON_SPIN,1);
#endif
	return ((UINT32) FlipCount)<<16;
}

static WRITE32_HANDLER(FlipCount_w)
{
	if((~mem_mask)&0x00ff0000)
	{
		int fc=(data>>16)&0xff;
		if(fc==1)
			FlipCount++;
		else if(fc==0)
			FlipCount=0;
	}
}

static READ32_HANDLER(Input_r)
{
	if(offset==0)
		return input_port_read_indexed(machine, 0)|(input_port_read_indexed(machine, 1)<<16);
	else if(offset==1)
		return input_port_read_indexed(machine, 2)|(input_port_read_indexed(machine, 3)<<16);
	else if(offset==2)
	{
		UINT8 Port4=input_port_read_indexed(machine, 4);
		if(!(Port4&0x10) && ((OldPort4^Port4)&0x10))	//coin buttons trigger IRQs
			IntReq(machine, 12);
		if(!(Port4&0x20) && ((OldPort4^Port4)&0x20))
			IntReq(machine, 19);
		OldPort4=Port4;
		return /*dips*/input_port_read_indexed(machine, 5)|(Port4<<16);
	}
	return 0;
}

static WRITE32_HANDLER(IntAck_w)
{
	UINT32 IntPend=program_read_dword_32le(0x01800c0c);
	if((~mem_mask)&0xff)
	{
		IntPend&=~(1<<(data&0x1f));
		program_write_dword_32le(0x01800c0c,IntPend);
		if(!IntPend)
			cpunum_set_input_line(machine, 0,SE3208_INT,CLEAR_LINE);
	}
	if((~mem_mask)&0xff00)
		IntHigh=(data>>8)&7;
}

static int icallback(int line)
{
	int i;
	UINT32 IntPend=program_read_dword_32le(0x01800c0c);

	for(i=0;i<32;++i)
	{
		if(IntPend&(1<<i))
		{
			return (IntHigh<<5)|i;
		}
	}
	return 0;		//This should never happen
}

static WRITE32_HANDLER(Banksw_w)
{
	Bank=(data>>1)&7;
	if(Bank<=2)
		memory_set_bankptr(1,memory_region(REGION_USER1)+Bank*0x1000000);
	else
		memory_set_bankptr(1,memory_region(REGION_USER2));
}

static TIMER_CALLBACK( Timer0cb )
{
	if(!(Timer0ctrl&2))
		Timer0ctrl&=~1;
	IntReq(machine, 0);
}

static WRITE32_HANDLER(Timer0_w)
{
	if(((data^Timer0ctrl)&1) && (data&1))	//Timer activate
	{
		int PD=(data>>8)&0xff;
		int TCV=program_read_dword_32le(0x01801404);
		attotime period = attotime_mul(ATTOTIME_IN_HZ(43000000), (PD + 1) * (TCV + 1));

		if(Timer0ctrl&2)
			timer_adjust_periodic(Timer0,period,0,period);
		else
			timer_adjust_oneshot(Timer0,period,0);
	}
	COMBINE_DATA(&Timer0ctrl);
}

static READ32_HANDLER(Timer0_r)
{
	return Timer0ctrl;
}

static TIMER_CALLBACK( Timer1cb )
{
	if(!(Timer1ctrl&2))
		Timer1ctrl&=~1;
	IntReq(machine, 1);
}

static WRITE32_HANDLER(Timer1_w)
{
	if(((data^Timer1ctrl)&1) && (data&1))	//Timer activate
	{
		int PD=(data>>8)&0xff;
		int TCV=program_read_dword_32le(0x0180140C);
		attotime period = attotime_mul(ATTOTIME_IN_HZ(43000000), (PD + 1) * (TCV + 1));

		if(Timer1ctrl&2)
			timer_adjust_periodic(Timer1,period,0,period);
		else
			timer_adjust_oneshot(Timer1,period,0);
	}
	COMBINE_DATA(&Timer1ctrl);
}

static READ32_HANDLER(Timer1_r)
{
	return Timer1ctrl;
}

static TIMER_CALLBACK( Timer2cb )
{
	if(!(Timer2ctrl&2))
		Timer2ctrl&=~1;
	IntReq(machine, 9);
}

static WRITE32_HANDLER(Timer2_w)
{
	if(((data^Timer2ctrl)&1) && (data&1))	//Timer activate
	{
		int PD=(data>>8)&0xff;
		int TCV=program_read_dword_32le(0x01801414);
		attotime period = attotime_mul(ATTOTIME_IN_HZ(43000000), (PD + 1) * (TCV + 1));

		if(Timer2ctrl&2)
			timer_adjust_periodic(Timer2,period,0,period);
		else
			timer_adjust_oneshot(Timer2,period,0);
	}
	COMBINE_DATA(&Timer2ctrl);
}

static READ32_HANDLER(Timer2_r)
{
	return Timer2ctrl;
}

static TIMER_CALLBACK( Timer3cb )
{
	if(!(Timer3ctrl&2))
		Timer3ctrl&=~1;
	IntReq(machine, 10);
}

static WRITE32_HANDLER(Timer3_w)
{
	if(((data^Timer3ctrl)&1) && (data&1))	//Timer activate
	{
		int PD=(data>>8)&0xff;
		int TCV=program_read_dword_32le(0x0180141C);
		attotime period = attotime_mul(ATTOTIME_IN_HZ(43000000), (PD + 1) * (TCV + 1));

		if(Timer3ctrl&2)
			timer_adjust_periodic(Timer3,period,0,period);
		else
			timer_adjust_oneshot(Timer3,period,0);
	}
	COMBINE_DATA(&Timer3ctrl);
}

static READ32_HANDLER(Timer3_r)
{
	return Timer3ctrl;
}

static READ32_HANDLER(FlashCmd_r)
{
	if((FlashCmd&0xff)==0xff)
	{
		if(Bank<=2)
		{
			UINT32 *ptr=(UINT32*)(memory_region(REGION_USER1)+Bank*0x1000000);
			return ptr[0];
		}
		else
			return 0xffffffff;
	}
	if((FlashCmd&0xff)==0x90)
	{
		if(Bank<=2)
            return 0x00180089;	//Intel 128MBit
		else
			return 0xffffffff;
	}
	return 0;
}

static WRITE32_HANDLER(FlashCmd_w)
{
	FlashCmd=data;
}

static READ32_HANDLER(PIO_r)
{
	return PIO;
}

static WRITE32_HANDLER(PIO_w)
{
	UINT32 RST=data&0x01000000;
	UINT32 CLK=data&0x02000000;
	UINT32 DAT=data&0x10000000;

	DS1302_RST(RST?1:0);
	DS1302_DAT(DAT?1:0);
	DS1302_CLK(CLK?1:0);

	if(DS1302_RD())
		program_write_dword_32le(0x01802008,program_read_dword_32le(0x01802008)|0x10000000);
	else
		program_write_dword_32le(0x01802008,program_read_dword_32le(0x01802008)&(~0x10000000));

	COMBINE_DATA(&PIO);
}

static READ32_HANDLER(DMA0_r)
{
	return DMA0ctrl;
}

static WRITE32_HANDLER(DMA0_w)
{
	if(((data^DMA0ctrl)&(1<<10)) && (data&(1<<10)))	//DMAOn
	{
		UINT32 CTR=data;
		UINT32 SRC=program_read_dword_32le(0x01800804);
		UINT32 DST=program_read_dword_32le(0x01800808);
		UINT32 CNT=program_read_dword_32le(0x0180080C);
		int i;

		if(CTR&0x2)	//32 bits
		{
			for(i=0;i<CNT;++i)
			{
				UINT32 v=program_read_dword_32le(SRC+i*4);
				program_write_dword_32le(DST+i*4,v);
			}
		}
		else if(CTR&0x1)	//16 bits
		{
			for(i=0;i<CNT;++i)
			{
				UINT16 v=program_read_word_32le(SRC+i*2);
				program_write_word_32le(DST+i*2,v);
			}
		}
		else	//8 bits
		{
			for(i=0;i<CNT;++i)
			{
				UINT8 v=program_read_byte_32le(SRC+i);
				program_write_byte_32le(DST+i,v);
			}
		}
		data&=~(1<<10);
		program_write_dword_32le(0x0180080C,0);
		IntReq(machine, 7);
	}
	COMBINE_DATA(&DMA0ctrl);
}

static READ32_HANDLER(DMA1_r)
{
	return DMA1ctrl;
}

static WRITE32_HANDLER(DMA1_w)
{
	if(((data^DMA1ctrl)&(1<<10)) && (data&(1<<10)))	//DMAOn
	{
		UINT32 CTR=data;
		UINT32 SRC=program_read_dword_32le(0x01800814);
		UINT32 DST=program_read_dword_32le(0x01800818);
		UINT32 CNT=program_read_dword_32le(0x0180081C);
		int i;

		if(CTR&0x2)	//32 bits
		{
			for(i=0;i<CNT;++i)
			{
				UINT32 v=program_read_dword_32le(SRC+i*4);
				program_write_dword_32le(DST+i*4,v);
			}
		}
		else if(CTR&0x1)	//16 bits
		{
			for(i=0;i<CNT;++i)
			{
				UINT16 v=program_read_word_32le(SRC+i*2);
				program_write_word_32le(DST+i*2,v);
			}
		}
		else	//8 bits
		{
			for(i=0;i<CNT;++i)
			{
				UINT8 v=program_read_byte_32le(SRC+i);
				program_write_byte_32le(DST+i,v);
			}
		}
		data&=~(1<<10);
		program_write_dword_32le(0x0180081C,0);
		IntReq(machine, 8);
	}
	COMBINE_DATA(&DMA1ctrl);
}


static ADDRESS_MAP_START( crystal_mem, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM AM_WRITENOP

	AM_RANGE(0x01200000, 0x0120000f) AM_READ(Input_r)
	AM_RANGE(0x01280000, 0x01280003) AM_WRITE(Banksw_w)
	AM_RANGE(0x01400000, 0x0140ffff) AM_RAM AM_BASE(&generic_nvram32) AM_SIZE(&generic_nvram_size)

	AM_RANGE(0x01801400, 0x01801403) AM_READ(Timer0_r) AM_WRITE(Timer0_w)
	AM_RANGE(0x01801408, 0x0180140b) AM_READ(Timer1_r) AM_WRITE(Timer1_w)
	AM_RANGE(0x01801410, 0x01801413) AM_READ(Timer2_r) AM_WRITE(Timer2_w)
	AM_RANGE(0x01801418, 0x0180141b) AM_READ(Timer3_r) AM_WRITE(Timer3_w)
	AM_RANGE(0x01802004, 0x01802007) AM_WRITE(PIO_w) AM_READ(PIO_r)

	AM_RANGE(0x01800800, 0x01800803) AM_READ(DMA0_r) AM_WRITE(DMA0_w)
	AM_RANGE(0x01800810, 0x01800813) AM_READ(DMA1_r) AM_WRITE(DMA1_w)

	AM_RANGE(0x01800c04, 0x01800c07) AM_WRITE(IntAck_w)
	AM_RANGE(0x01800000, 0x0180ffff) AM_RAM AM_BASE(&sysregs)
	AM_RANGE(0x02000000, 0x027fffff) AM_RAM AM_BASE(&workram)

	AM_RANGE(0x030000a4, 0x030000a7) AM_READ(FlipCount_r) AM_WRITE(FlipCount_w)

	AM_RANGE(0x03000000, 0x0300ffff) AM_RAM AM_BASE(&vidregs)
	AM_RANGE(0x03800000, 0x03ffffff) AM_RAM AM_BASE(&textureram)
	AM_RANGE(0x04000000, 0x047fffff) AM_RAM AM_BASE(&frameram)
	AM_RANGE(0x04800000, 0x04800fff) AM_READ(VR0_Snd_Read) AM_WRITE(VR0_Snd_Write)

	AM_RANGE(0x05000000, 0x05000003) AM_READ(FlashCmd_r) AM_WRITE(FlashCmd_w)
	AM_RANGE(0x05000000, 0x05ffffff) AM_READ(SMH_BANK1)

	AM_RANGE(0x44414F4C, 0x44414F7F) AM_RAM AM_BASE(&ResetPatch)

ADDRESS_MAP_END

static void PatchReset(void)
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

	memcpy(ResetPatch,Patch,sizeof(Patch));
#else
	static const UINT8 Patch[] =
	{
		0x01,0xEA,0xC0,0x40,0x0A,0x40,0x06,0xE9,
		0x20,0x2A,0xC0,0x40,0x0A,0x40,0x06,0xE9,
		0x20,0x3A,0xD0,0xA1,0xFA,0xD4,0xF4,0xDE
	};

	memcpy(ResetPatch,Patch,sizeof(Patch));
#endif
}

static MACHINE_RESET(crystal)
{
	memset(sysregs,0,0x10000);
	memset(vidregs,0,0x10000);
	FlipCount=0;
	IntHigh=0;
	cpunum_set_irq_callback(0,icallback);
	Bank=0;
	memory_set_bankptr(1,memory_region(REGION_USER1)+0);
	FlashCmd=0xff;
	OldPort4=0;

	DMA0ctrl=0;
	DMA1ctrl=0;

	Timer0ctrl=0;
	Timer1ctrl=0;
	Timer2ctrl=0;
	Timer3ctrl=0;

	Timer0=timer_alloc(Timer0cb, NULL);
	timer_adjust_oneshot(Timer0,attotime_never,0);

	Timer1=timer_alloc(Timer1cb, NULL);
	timer_adjust_oneshot(Timer1,attotime_never,0);

	Timer2=timer_alloc(Timer2cb, NULL);
	timer_adjust_oneshot(Timer2,attotime_never,0);

	Timer3=timer_alloc(Timer3cb, NULL);
	timer_adjust_oneshot(Timer3,attotime_never,0);

	VR0_Snd_Set_Areas(textureram,frameram);
#ifdef IDLE_LOOP_SPEEDUP
	FlipCntRead=0;
#endif

	PatchReset();
}

static VIDEO_START(crystal)
{
}

static UINT16 GetVidReg(UINT16 reg)
{
	return program_read_word_32le(0x03000000+reg);
}

static void SetVidReg(UINT16 reg,UINT16 val)
{
	program_write_word_32le(0x03000000+reg,val);
}


static VIDEO_UPDATE(crystal)
{
	int DoFlip;


	UINT32 B0=0x0;
	UINT32 B1=(GetVidReg(0x90)&0x8000)?0x400000:0x100000;
	UINT16 *Front,*Back;
	UINT16 *Visible,*DrawDest;
	UINT16 *srcline;
	int y;
	UINT16 head,tail;

	if(GetVidReg(0x8e)&1)
	{
		Front=(UINT16*) (frameram+B1/4);
		Back=(UINT16*) (frameram+B0/4);
	}
	else
	{
		Front=(UINT16*) (frameram+B0/4);
		Back=(UINT16*) (frameram+B1/4);
	}

	Visible=(UINT16*) Front;
	DrawDest=(UINT16 *) frameram;


	if(GetVidReg(0x8c)&0x80)
		DrawDest=Front;
	else
		DrawDest=Back;

//  DrawDest=Visible;

	srcline=(UINT16 *) DrawDest;

	DoFlip=0;
	head=GetVidReg(0x82);
	tail=GetVidReg(0x80);
	while((head&0x7ff)!=(tail&0x7ff))
	{
		DoFlip=vrender0_ProcessPacket(0x03800000+head*64,DrawDest,(UINT8*)textureram);
		head++;
		head&=0x7ff;
		if(DoFlip)
			break;
	}

	if(DoFlip)
		SetVidReg(0x8e,GetVidReg(0x8e)^1);

	srcline=(UINT16 *) Visible;
	for(y=0;y<240;y++)
		memcpy(BITMAP_ADDR16(bitmap, y, 0), &srcline[y*512], 320*2);

	return 0;
}

static VIDEO_EOF(crystal)
{
	UINT16 head,tail;
	int DoFlip=0;

	head=GetVidReg(0x82);
	tail=GetVidReg(0x80);
	while((head&0x7ff)!=(tail&0x7ff))
	{
		UINT16 Packet0=program_read_word_32le(0x03800000+head*64);
		if(Packet0&0x81)
			DoFlip=1;
		head++;
		head&=0x7ff;
		if(DoFlip)
			break;
	}
	SetVidReg(0x82,head);
	if(DoFlip)
	{
		if(FlipCount)
			FlipCount--;

	}
}

static INTERRUPT_GEN(crystal_interrupt)
{
	IntReq(machine, 24);		//VRender0 VBlank
}

static INPUT_PORTS_START(crystal)
	PORT_START
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)

	PORT_START
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)

	PORT_START
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)

	PORT_START
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(4)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(3)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(4)

	PORT_START
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_LOW )

	PORT_START
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

static const struct VR0Interface vr0_interface =
{
	0x04800000
};


static MACHINE_DRIVER_START( crystal )
	MDRV_CPU_ADD(SE3208, 43000000)
	MDRV_CPU_PROGRAM_MAP(crystal_mem,0)
 	MDRV_CPU_VBLANK_INT("main", crystal_interrupt)

	MDRV_MACHINE_RESET(crystal)

	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(320, 240)
	MDRV_SCREEN_VISIBLE_AREA(0, 319, 0, 239)

	MDRV_VIDEO_START(crystal)
	MDRV_VIDEO_UPDATE(crystal)
	MDRV_VIDEO_EOF(crystal)

	MDRV_PALETTE_INIT(RRRRR_GGGGGG_BBBBB)
	MDRV_PALETTE_LENGTH(65536)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(VRENDER0, 0)
	MDRV_SOUND_CONFIG(vr0_interface)
	MDRV_SOUND_ROUTE(0, "left", 1.0)
	MDRV_SOUND_ROUTE(1, "right", 1.0)
MACHINE_DRIVER_END

ROM_START( crysbios )
	ROM_REGION( 0x20000, REGION_CPU1, 0 ) // bios
	ROM_LOAD("mx27l1000.u14",  0x000000, 0x020000, CRC(BEFF39A9) SHA1(b6f6dda58d9c82273f9422c1bd623411e58982cb) )

	ROM_REGION32_LE( 0x3000000, REGION_USER1, ROMREGION_ERASEFF ) // Flash

	ROM_REGION( 0x10000, REGION_USER2,	ROMREGION_ERASEFF )	//Unmapped flash
ROM_END

ROM_START( crysking )
	ROM_REGION( 0x20000, REGION_CPU1, 0 ) // bios
	ROM_LOAD("mx27l1000.u14",  0x000000, 0x020000, CRC(BEFF39A9) SHA1(b6f6dda58d9c82273f9422c1bd623411e58982cb))

	ROM_REGION32_LE( 0x3000000, REGION_USER1, 0 ) // Flash
	ROM_LOAD("bcsv0004f01.u1",  0x0000000, 0x1000000, CRC(8FEFF120) SHA1(2ea42fa893bff845b5b855e2556789f8354e9066) )
	ROM_LOAD("bcsv0004f02.u2",  0x1000000, 0x1000000, CRC(0E799845) SHA1(419674ce043cb1efb18303f4cb7fdbbae642ee39) )
	ROM_LOAD("bcsv0004f03.u3",  0x2000000, 0x1000000, CRC(659E2D17) SHA1(342c98f3f695ef4dea8b533612451c4d2fb58809) )

	ROM_REGION( 0x10000, REGION_USER2,	ROMREGION_ERASEFF )	//Unmapped flash
ROM_END

ROM_START( evosocc )
	ROM_REGION( 0x20000, REGION_CPU1, 0 ) // bios
	ROM_LOAD("mx27l1000.u14",  0x000000, 0x020000, CRC(BEFF39A9) SHA1(b6f6dda58d9c82273f9422c1bd623411e58982cb))

	ROM_REGION32_LE( 0x3000000, REGION_USER1, 0 ) // Flash
	ROM_LOAD("bcsv0001u01",  0x0000000, 0x1000000, CRC(2581A0EA) SHA1(ee483ac60a3ed00a21cb515974cec4af19916a7d) )
	ROM_LOAD("bcsv0001u02",  0x1000000, 0x1000000, CRC(47EF1794) SHA1(f573706c17d1342b9b7aed9b40b8b648f0bf58db) )
	ROM_LOAD("bcsv0001u03",  0x2000000, 0x1000000, CRC(F396A2EC) SHA1(f305eb10856fb5d4c229a6b09d6a2fb21b24ce66) )

	ROM_REGION( 0x10000, REGION_USER2,	ROMREGION_ERASEFF )	//Unmapped flash
ROM_END


static DRIVER_INIT(crysking)
{
	UINT16 *Rom=(UINT16*) memory_region(REGION_USER1);

	//patch the data feed by the protection

	Rom[WORD_XOR_LE(0x7bb6/2)]=0xDF01;
	Rom[WORD_XOR_LE(0x7bb8/2)]=0x9C00;

	Rom[WORD_XOR_LE(0x976a/2)]=0x901C;
	Rom[WORD_XOR_LE(0x976c/2)]=0x9001;

	Rom[WORD_XOR_LE(0x8096/2)]=0x90FC;
	Rom[WORD_XOR_LE(0x8098/2)]=0x9001;

	Rom[WORD_XOR_LE(0x8a52/2)]=0x4000;	//NOP
	Rom[WORD_XOR_LE(0x8a54/2)]=0x403c;	//NOP
}

static DRIVER_INIT(evosocc)
{
	UINT16 *Rom=(UINT16*) memory_region(REGION_USER1);
	Rom+=0x1000000*2/2;

	Rom[WORD_XOR_LE(0x97388E/2)]=0x90FC;	//PUSH R2..R7
	Rom[WORD_XOR_LE(0x973890/2)]=0x9001;	//PUSH R0

	Rom[WORD_XOR_LE(0x971058/2)]=0x907C;	//PUSH R2..R6
	Rom[WORD_XOR_LE(0x971060/2)]=0x9001; //PUSH R0

	Rom[WORD_XOR_LE(0x978036/2)]=0x900C;	//PUSH R2-R3
	Rom[WORD_XOR_LE(0x978038/2)]=0x8303;	//LD    (%SP,0xC),R3

	Rom[WORD_XOR_LE(0x974ED0/2)]=0x90FC;	//PUSH R7-R6-R5-R4-R3-R2
	Rom[WORD_XOR_LE(0x974ED2/2)]=0x9001;	//PUSH R0
}

GAME( 2001, crysbios,        0, crystal, crystal,        0, ROT0, "Brezzasoft", "Crystal System BIOS", GAME_IS_BIOS_ROOT )
GAME( 2001, crysking, crysbios, crystal, crystal, crysking, ROT0, "Brezzasoft", "The Crystal of Kings", 0 )
GAME( 2001, evosocc,  crysbios, crystal, crystal,  evosocc, ROT0, "Evoga", "Evolution Soccer", 0 )
