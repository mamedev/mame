#include "emu.h"
#include "sound/es5506.h"
#include "taito_en.h"

static int counter,vector_reg,imr_status;
static UINT16   es5510_dsp_ram[0x200];
static UINT32	es5510_gpr[0xc0];
static UINT32   es5510_dram[1<<24];
static UINT32   es5510_dol_latch;
static UINT32   es5510_dil_latch;
static UINT32   es5510_dadr_latch;
static UINT32	es5510_gpr_latch;
static UINT8    es5510_ram_sel;
static int timer_mode,m68681_imr;
static UINT32   *f3_shared_ram;

//static int es_tmp=1;

#define M68000_CLOCK	16000000
#define M68681_CLOCK	2000000 /* Actually X1, not the main clock */

enum { TIMER_SINGLESHOT, TIMER_PULSE };



static READ16_HANDLER(f3_68000_share_r)
{
	switch (offset & 3)
	{
	case 0: return (f3_shared_ram[offset/4]&0xff000000)>>16;
	case 1: return (f3_shared_ram[offset/4]&0x00ff0000)>>8;
	case 2: return (f3_shared_ram[offset/4]&0x0000ff00)>>0;
	case 3: return (f3_shared_ram[offset/4]&0x000000ff)<<8;
	}

	return 0;
}

static WRITE16_HANDLER(f3_68000_share_w)
{
	switch (offset & 3)
	{
	case 0: f3_shared_ram[offset/4] = (f3_shared_ram[offset/4]&0x00ffffff)|((data&0xff00)<<16);
	case 1: f3_shared_ram[offset/4] = (f3_shared_ram[offset/4]&0xff00ffff)|((data&0xff00)<<8);
	case 2: f3_shared_ram[offset/4] = (f3_shared_ram[offset/4]&0xffff00ff)|((data&0xff00)<<0);
	case 3: f3_shared_ram[offset/4] = (f3_shared_ram[offset/4]&0xffffff00)|((data&0xff00)>>8);
	}
}

static WRITE16_HANDLER( f3_es5505_bank_w )
{
	UINT32 max_banks_this_game=(space->machine().region("ensoniq.0")->bytes()/0x200000)-1;

#if 0
{
	static char count[10];
	count[data&7]++;
	popmessage("%x %x %x %x %x %x %x %x (%d)",count[0]&0xf,count[1]&0xf,count[2]&0xf,count[3]&0xf,count[4]&0xf,count[5]&0xf,count[6]&0xf,count[7]&0xf, max_banks_this_game);
}
#endif

	/* mask out unused bits */
	data &= max_banks_this_game;
	es5505_voice_bank_w(space->machine().device("ensoniq"),offset,data<<20);
}

static WRITE8_HANDLER( f3_volume_w )
{
	static UINT8 latch,ch[8];

	if(offset == 0)
		latch = data & 0x7;
	else
	{
		ch[latch] = data;
		if((latch & 6) == 6)
		{
			double ch_vol;

			ch_vol = (double)(ch[latch] & 0x3f);
			ch_vol/= 63.0;
			ch_vol*= 100.0;
			/* Left/Right panning trusted with Arabian Magic Sound Test menu. */
			es5505_set_channel_volume(space->machine().device("ensoniq"),(latch & 1) ^ 1,ch_vol);
		}
	}

	//popmessage("%02x %02x %02x %02x %02x %02x %02x %02x",ch[0],ch[1],ch[2],ch[3],ch[4],ch[5],ch[6],ch[7]);

	/* Channel 5 - Left Aux?  Always set to volume, but never used for panning */
	/* Channel 4 - Right Aux?  Always set to volume, but never used for panning */
	/* Channels 0, 1, 2, 3 - Unused */

}

static TIMER_DEVICE_CALLBACK( taito_en_timer_callback )
{
	/* Only cause IRQ if the mask is set to allow it */
	if (m68681_imr & 0x08)
	{
		device_set_input_line_vector(timer.machine().device("audiocpu"), 6, vector_reg);
		cputag_set_input_line(timer.machine(), "audiocpu", 6, ASSERT_LINE);
		imr_status |= 0x08;
	}
}

static READ16_HANDLER(f3_68681_r)
{
	if (offset == 0x05)
	{
		int ret = imr_status;
		imr_status = 0;
		return ret;
	}

	if (offset == 0x0e)
		return 1;

	/* IRQ ack */
	if (offset == 0x0f)
	{
		cputag_set_input_line(space->machine(), "audiocpu", 6, CLEAR_LINE);
		return 0;
	}

	return 0xff;
}

static WRITE16_HANDLER(f3_68681_w)
{
	timer_device *timer;
	switch (offset) {
		case 0x04: /* ACR */
			switch ((data>>4)&7) {
				case 0:
					logerror("Counter:  Unimplemented external IP2\n");
					break;
				case 1:
					logerror("Counter:  Unimplemented TxCA - 1X clock of channel A\n");
					break;
				case 2:
					logerror("Counter:  Unimplemented TxCB - 1X clock of channel B\n");
					break;
				case 3:
					logerror("Counter:  X1/Clk - divided by 16, counter is %04x, so interrupt every %d cycles\n",counter,(M68000_CLOCK/M68681_CLOCK)*counter*16);
					timer_mode=TIMER_SINGLESHOT;
					timer = space->machine().device<timer_device>("timer_68681");
					timer->adjust(downcast<cpu_device *>(&space->device())->cycles_to_attotime((M68000_CLOCK/M68681_CLOCK)*counter*16));
					break;
				case 4:
					logerror("Timer:  Unimplemented external IP2\n");
					break;
				case 5:
					logerror("Timer:  Unimplemented external IP2/16\n");
					break;
				case 6:
					logerror("Timer:  X1/Clk, counter is %04x, so interrupt every %d cycles\n",counter,(M68000_CLOCK/M68681_CLOCK)*counter);
					timer_mode=TIMER_PULSE;
					timer = space->machine().device<timer_device>("timer_68681");
					timer->adjust(downcast<cpu_device *>(&space->device())->cycles_to_attotime((M68000_CLOCK/M68681_CLOCK)*counter), 0, downcast<cpu_device *>(&space->device())->cycles_to_attotime((M68000_CLOCK/M68681_CLOCK)*counter));
					break;
				case 7:
					logerror("Timer:  Unimplemented X1/Clk - divided by 16\n");
					break;
			}
			break;

		case 0x05: /* IMR */
			logerror("68681:  %02x %02x\n",offset,data&0xff);
			m68681_imr=data&0xff;
			break;

		case 0x06: /* CTUR */
			counter=((data&0xff)<<8)|(counter&0xff);
			break;
		case 0x07: /* CTLR */
			counter=(counter&0xff00)|(data&0xff);
			break;
		case 0x08: break; /* MR1B (Mode register B) */
		case 0x09: break; /* CSRB (Clock select register B) */
		case 0x0a: break; /* CRB (Command register B) */
		case 0x0b: break; /* TBB (Transmit buffer B) */
		case 0x0c: /* IVR (Interrupt vector) */
			vector_reg=data&0xff;
			break;
		default:
			logerror("68681:  %02x %02x\n",offset,data&0xff);
			break;
	}
}

static READ16_HANDLER(es5510_dsp_r)
{
//  logerror("%06x: DSP read offset %04x (data is %04x)\n",cpu_get_pc(&space->device()),offset,es5510_dsp_ram[offset]);
//  if (es_tmp) return es5510_dsp_ram[offset];
/*
    switch (offset) {
        case 0x00: return (es5510_gpr_latch>>16)&0xff;
        case 0x01: return (es5510_gpr_latch>> 8)&0xff;
        case 0x02: return (es5510_gpr_latch>> 0)&0xff;
        case 0x03: return 0;
    }
*/
//  offset<<=1;

//if (offset<7 && es5510_dsp_ram[0]!=0xff) return space->machine().rand()%0xffff;

	switch(offset)
	{
		case 0x09: return (es5510_dil_latch >> 16) & 0xff;
		case 0x0a: return (es5510_dil_latch >> 8) & 0xff;
		case 0x0b: return (es5510_dil_latch >> 0) & 0xff; //TODO: docs says that this always returns 0
	}

	if (offset==0x12) return 0;

//  if (offset>4)
	if (offset==0x16) return 0x27;

	return es5510_dsp_ram[offset];
}

static WRITE16_HANDLER(es5510_dsp_w)
{
	UINT8 *snd_mem = (UINT8 *)space->machine().region("ensoniq.0")->base();

//  if (offset>4 && offset!=0x80  && offset!=0xa0  && offset!=0xc0  && offset!=0xe0)
//      logerror("%06x: DSP write offset %04x %04x\n",cpu_get_pc(&space->device()),offset,data);

	COMBINE_DATA(&es5510_dsp_ram[offset]);

	switch (offset) {
		case 0x00: es5510_gpr_latch=(es5510_gpr_latch&0x00ffff)|((data&0xff)<<16); break;
		case 0x01: es5510_gpr_latch=(es5510_gpr_latch&0xff00ff)|((data&0xff)<< 8); break;
		case 0x02: es5510_gpr_latch=(es5510_gpr_latch&0xffff00)|((data&0xff)<< 0); break;

		/* 0x03 to 0x08 INSTR Register */
		/* 0x09 to 0x0b DIL Register (r/o) */

		case 0x0c: es5510_dol_latch=(es5510_dol_latch&0x00ffff)|((data&0xff)<<16); break;
		case 0x0d: es5510_dol_latch=(es5510_dol_latch&0xff00ff)|((data&0xff)<< 8); break;
		case 0x0e: es5510_dol_latch=(es5510_dol_latch&0xffff00)|((data&0xff)<< 0); break; //TODO: docs says that this always returns 0xff

		case 0x0f:
			es5510_dadr_latch=(es5510_dadr_latch&0x00ffff)|((data&0xff)<<16);
			if(es5510_ram_sel)
				es5510_dil_latch = es5510_dram[es5510_dadr_latch];
			else
				es5510_dram[es5510_dadr_latch] = es5510_dol_latch;
			break;

		case 0x10: es5510_dadr_latch=(es5510_dadr_latch&0xff00ff)|((data&0xff)<< 8); break;
		case 0x11: es5510_dadr_latch=(es5510_dadr_latch&0xffff00)|((data&0xff)<< 0); break;

		/* 0x12 Host Control */

		case 0x14: es5510_ram_sel = data & 0x80; /* bit 6 is i/o select, everything else is undefined */break;

		/* 0x16 Program Counter (test purpose, r/o?) */
		/* 0x17 Internal Refresh counter (test purpose) */
		/* 0x18 Host Serial Control */
		/* 0x1f Halt enable (w) / Frame Counter (r) */

		case 0x80: /* Read select - GPR + INSTR */
	//      logerror("ES5510:  Read GPR/INSTR %06x (%06x)\n",data,es5510_gpr[data]);

			/* Check if a GPR is selected */
			if (data<0xc0) {
				//es_tmp=0;
				es5510_gpr_latch=es5510_gpr[data];
			}// else es_tmp=1;
			break;

		case 0xa0: /* Write select - GPR */
	//      logerror("ES5510:  Write GPR %06x %06x (0x%04x:=0x%06x\n",data,es5510_gpr_latch,data,snd_mem[es5510_gpr_latch>>8]);
			if (data<0xc0)
				es5510_gpr[data]=snd_mem[es5510_gpr_latch>>8];
			break;

		case 0xc0: /* Write select - INSTR */
	//      logerror("ES5510:  Write INSTR %06x %06x\n",data,es5510_gpr_latch);
			break;

		case 0xe0: /* Write select - GPR + INSTR */
	//      logerror("ES5510:  Write GPR/INSTR %06x %06x\n",data,es5510_gpr_latch);
			break;
	}
}

static ADDRESS_MAP_START( f3_sound_map, AS_PROGRAM, 16, driver_device )
	AM_RANGE(0x000000, 0x00ffff) AM_RAM AM_MIRROR(0x30000) AM_SHARE("share1")
	AM_RANGE(0x140000, 0x140fff) AM_READWRITE_LEGACY(f3_68000_share_r, f3_68000_share_w)
	AM_RANGE(0x200000, 0x20001f) AM_DEVREADWRITE_LEGACY("ensoniq", es5505_r, es5505_w)
	AM_RANGE(0x260000, 0x2601ff) AM_READWRITE_LEGACY(es5510_dsp_r, es5510_dsp_w)
	AM_RANGE(0x280000, 0x28001f) AM_READWRITE_LEGACY(f3_68681_r, f3_68681_w)
	AM_RANGE(0x300000, 0x30003f) AM_WRITE_LEGACY(f3_es5505_bank_w)
	AM_RANGE(0x340000, 0x340003) AM_WRITE8_LEGACY(f3_volume_w,0xff00) /* 8 channel volume control */
	AM_RANGE(0xc00000, 0xc1ffff) AM_ROMBANK("bank1")
	AM_RANGE(0xc20000, 0xc3ffff) AM_ROMBANK("bank2")
	AM_RANGE(0xc40000, 0xc7ffff) AM_ROMBANK("bank3")
	AM_RANGE(0xff0000, 0xffffff) AM_RAM AM_SHARE("share1")	// mirror
ADDRESS_MAP_END

SOUND_RESET( taito_f3_soundsystem_reset )
{
	/* Sound cpu program loads to 0xc00000 so we use a bank */
	UINT16 *ROM = (UINT16 *)machine.region("audiocpu")->base();
	UINT16 *sound_ram = (UINT16 *)machine.memory().shared("share1")->ptr();
	memory_set_bankptr(machine, "bank1",&ROM[0x80000]);
	memory_set_bankptr(machine, "bank2",&ROM[0x90000]);
	memory_set_bankptr(machine, "bank3",&ROM[0xa0000]);

	sound_ram[0]=ROM[0x80000]; /* Stack and Reset vectors */
	sound_ram[1]=ROM[0x80001];
	sound_ram[2]=ROM[0x80002];
	sound_ram[3]=ROM[0x80003];

	/* reset CPU to catch any banking of startup vectors */
	machine.device("audiocpu")->reset();
	//cputag_set_input_line(machine, "audiocpu", INPUT_LINE_RESET, ASSERT_LINE);

	f3_shared_ram = (UINT32 *)machine.memory().shared("f3_shared")->ptr();
}

static const es5505_interface es5505_taito_f3_config =
{
	"ensoniq.0",	/* Bank 0: Unused by F3 games? */
	"ensoniq.0",	/* Bank 1: All games seem to use this */
	NULL /* irq */
};

MACHINE_CONFIG_FRAGMENT( taito_f3_sound )
	MCFG_TIMER_ADD("timer_68681", taito_en_timer_callback)

	MCFG_SOUND_RESET( taito_f3_soundsystem_reset )

	MCFG_CPU_ADD("audiocpu",  M68000, 16000000)
	MCFG_CPU_PROGRAM_MAP(f3_sound_map)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
	MCFG_SOUND_ADD("ensoniq", ES5505, 30476100/2)
	MCFG_SOUND_CONFIG(es5505_taito_f3_config)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END
