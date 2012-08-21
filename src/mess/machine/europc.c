#include "emu.h"
#include "includes/europc.h"
#include "includes/pc.h"
#include "machine/pcshare.h"
#include "machine/pit8253.h"
#include "video/pc_aga.h"
#include "coreutil.h"

/*

 */

/*
  europc
  fe107 bios checksum test
   memory test
  fe145
   irq vector init
  fe156
  fe169 fd774 // test of special europc registers 254 354
  fe16c fe817
  fe16f
   fec08 // test of special europc registers 800a rtc time or date error, rtc corrected
    fef66 0xf
    fdb3e 0x8..0xc
    fd7f8
     fdb5f
  fe172
   fecc5 // 801a video setup error
    fd6c9
   copyright output
  fe1b7
  fe1be di bits set mean output text!!!,
   (801a)
   0x8000 output
        1 rtc error
        2 rtc time or date error
        4 checksum error in setup
        8 rtc status corrected
       10 video setup error
       20 video ram bad
       40 monitor type not recogniced
       80 mouse port enabled
      100 joystick port enabled

  fe1e2 fdc0c cpu speed is 4.77 MHz
  fe1e5 ff9c0 keyboard processor error
  fe1eb fc617 external lpt1 at 0x3bc
  fe1ee fe8ee external coms at

  routines:
  fc92d output text at bp
  fdb3e rtc read reg cl
  fe8ee piep
  fe95e rtc write reg cl
   polls until jim 0xa is zero,
   output cl at jim 0xa
   write ah hinibble as lownibble into jim 0xa
   write ah lownibble into jim 0xa
  fef66 rtc read reg cl
   polls until jim 0xa is zero,
   output cl at jim 0xa
   read low 4 nibble at jim 0xa
   read low 4 nibble at jim 0xa
   return first nibble<<4|second nibble in ah
  ff046 seldom compares ret
  ffe87 0 -> ds

  469:
   bit 0: b0000 memory available
   bit 1: b8000 memory available
  46a: 00 jim 250 01 jim 350
 */

static struct {
	UINT8 data[16];
	UINT8 state;
	AGA_MODE mode;
} europc_jim = { { 0 } } ;

/*
  250..253 write only 00 be 00 10

  252 write 0 b0000 memory activ
  252 write 0x10 b8000 memory activ

  jim 04: 0:4.77 0x40:7.16
  pio 63: 11,19 4.77 51,59 7.16

  63 bit 6,7 clock select
  254 bit 6,7 clock select
  250 bit 0: mouse on
      bit 1: joystick on
  254..257 r/w memory ? JIM asic? ram behaviour

*/
WRITE8_HANDLER ( europc_jim_w )
{
	switch (offset)
	{
	case 2:
		if (!(data & 0x80))
		{
			switch (data)
			{
			case 0x1f:
			case 0x0b: europc_jim.mode = AGA_MONO; break;
			case 0xe: //80 columns?
			case 0xd: //40 columns?
			case 0x18:
			case 0x1a: europc_jim.mode = AGA_COLOR; break;
			default: europc_jim.mode = AGA_OFF; break;
			}
		}
//      mode= data&0x10?AGA_COLOR:AGA_MONO;
//      mode= data&0x10?AGA_COLOR:AGA_OFF;
		pc_aga_set_mode(space->machine(), europc_jim.mode);
		if (data & 0x80) europc_jim.state = 0;
		break;
	case 4:
		switch(data & 0xc0)
		{
		case 0x00: space->machine().device("maincpu")->set_clock_scale(1.0 / 2); break;
		case 0x40: space->machine().device("maincpu")->set_clock_scale(3.0 / 4); break;
		default: space->machine().device("maincpu")->set_clock_scale(1); break;
		}
		break;
	case 0xa:
		europc_rtc_w(space, 0, data);
		return;
	}
	logerror("jim write %.2x %.2x\n", offset, data);
	europc_jim.data[offset] = data;
}

READ8_HANDLER ( europc_jim_r )
{
	int data = 0;
	switch(offset)
	{
	case 4: case 5: case 6: case 7: data = europc_jim.data[offset]; break;
	case 0: case 1: case 2: case 3: data = 0; break;
	case 0xa: return europc_rtc_r(space, 0);
	}
	return data;
}

READ8_HANDLER ( europc_jim2_r )
{
	switch (europc_jim.state)
	{
	case 0: europc_jim.state++; return 0;
	case 1: europc_jim.state++; return 0x80;
	case 2:
		europc_jim.state = 0;
		switch (europc_jim.mode)
		{
		case AGA_COLOR: return 0x87; // for color;
		case AGA_MONO: return 0x90; //for mono
		case AGA_OFF: return 0x80; // for vram
//      return 0x97; //for error
		}
	}
	return 0;
}

/* port 2e0 polling!? at fd6e1 */

static struct {
	int port61; // bit 0,1 must be 0 for startup; reset?
} europc_pio= { 0 };

WRITE8_HANDLER( europc_pio_w )
{
	switch (offset)
	{
	case 1:
		europc_pio.port61=data;
//      if (data == 0x30) pc1640.port62 = (pc1640.port65 & 0x10) >> 4;
//      else if (data == 0x34) pc1640.port62 = pc1640.port65 & 0xf;
		pit8253_gate2_w(space->machine().device("pit8253"), BIT(data, 0));
		pc_speaker_set_spkrdata(space->machine(), BIT(data, 1));
		pc_keyb_set_clock(BIT(data, 6));
		break;
	}

	logerror("europc pio write %.2x %.2x\n", offset, data);
}


 READ8_HANDLER( europc_pio_r )
{
	int data = 0;
	switch (offset)
	{
	case 0:
		if (!(europc_pio.port61&0x80))
			data = pc_keyb_read();
		break;
	case 1:
		data = europc_pio.port61;
		break;
	case 2:
		if (pit8253_get_output(space->machine().device("pit8253"), 2))
			data |= 0x20;
		break;
	}
	return data;
}

// realtime clock and nvram
static struct {
	/*
       reg 0: seconds
       reg 1: minutes
       reg 2: hours
       reg 3: day 1 based
       reg 4: month 1 based
       reg 5: year bcd (no century, values bigger 88? are handled as 1900, else 2000)
       reg 6:
       reg 7:
       reg 8:
       reg 9:
       reg a:
       reg b: 0x10 written
        bit 0,1: 0 video startup mode: 0=specialadapter, 1=color40, 2=color80, 3=monochrom
        bit 2: internal video on
        bit 4: color
        bit 6,7: clock
       reg c:
        bit 0,1: language/country
       reg d: xor checksum
       reg e:
       reg 0f: 01 status ok, when not 01 written
    */
	UINT8 data[0x10];
	int reg;
	int state;
	emu_timer *timer;
} europc_rtc;

void europc_rtc_set_time(running_machine &machine)
{
	system_time systime;

	/* get the current date/time from the core */
	machine.current_datetime(systime);

	europc_rtc.data[0] = dec_2_bcd(systime.utc_time.second);
	europc_rtc.data[1] = dec_2_bcd(systime.utc_time.minute);
	europc_rtc.data[2] = dec_2_bcd(systime.utc_time.hour);

	europc_rtc.data[3] = dec_2_bcd(systime.utc_time.mday);
	europc_rtc.data[4] = dec_2_bcd(systime.utc_time.month + 1);
	europc_rtc.data[5] = dec_2_bcd(systime.utc_time.year % 100);
}

static TIMER_CALLBACK(europc_rtc_timer)
{
	int month, year;
	europc_rtc.data[0]=bcd_adjust(europc_rtc.data[0]+1);
	if (europc_rtc.data[0]>=0x60)
	{
		europc_rtc.data[0]=0;
		europc_rtc.data[1]=bcd_adjust(europc_rtc.data[1]+1);
		if (europc_rtc.data[1]>=0x60)
		{
			europc_rtc.data[1]=0;
			europc_rtc.data[2]=bcd_adjust(europc_rtc.data[2]+1);
			if (europc_rtc.data[2]>=0x24)
			{
				europc_rtc.data[2]=0;
				europc_rtc.data[3]=bcd_adjust(europc_rtc.data[3]+1);
				month=bcd_2_dec(europc_rtc.data[4]);
				year=bcd_2_dec(europc_rtc.data[5])+2000; // save for julian_days_in_month_calculation
				if (europc_rtc.data[3]> gregorian_days_in_month(month, year))
				{
					europc_rtc.data[3]=1;
					europc_rtc.data[4]=bcd_adjust(europc_rtc.data[4]+1);
					if (europc_rtc.data[4]>0x12)
					{
						europc_rtc.data[4]=1;
						europc_rtc.data[5]=bcd_adjust(europc_rtc.data[5]+1)&0xff;
					}
				}
			}
		}
	}
}

void europc_rtc_init(running_machine &machine)
{
	memset(&europc_rtc,0,sizeof(europc_rtc));
	europc_rtc.data[0xf]=1;

	europc_rtc.timer = machine.scheduler().timer_alloc(FUNC(europc_rtc_timer));
	europc_rtc.timer->adjust(attotime::zero, 0, attotime(1,0));
}

 READ8_HANDLER( europc_rtc_r )
{
	int data=0;
	switch (europc_rtc.state)
	{
	case 1:
		data=(europc_rtc.data[europc_rtc.reg]&0xf0)>>4;
		europc_rtc.state++;
		break;
	case 2:
		data=europc_rtc.data[europc_rtc.reg]&0xf;
		europc_rtc.state=0;
//      logerror("rtc read %x %.2x\n",europc_rtc.reg, europc_rtc.data[europc_rtc.reg]);
		break;
	}
	return data;
}

WRITE8_HANDLER( europc_rtc_w )
{
	switch (europc_rtc.state)
	{
	case 0:
		europc_rtc.reg=data;
		europc_rtc.state=1;
		break;
	case 1:
		europc_rtc.data[europc_rtc.reg]=(europc_rtc.data[europc_rtc.reg]&~0xf0)|((data&0xf)<<4);
		europc_rtc.state++;
		break;
	case 2:
		europc_rtc.data[europc_rtc.reg]=(europc_rtc.data[europc_rtc.reg]&~0xf)|(data&0xf);
		europc_rtc.state=0;
//      logerror("rtc written %x %.2x\n",europc_rtc.reg, europc_rtc.data[europc_rtc.reg]);
		break;
	}
}

static void europc_rtc_load_stream(emu_file *file)
{
	file->read(europc_rtc.data, sizeof(europc_rtc.data));
}

static void europc_rtc_save_stream(emu_file *file)
{
	file->write(europc_rtc.data, sizeof(europc_rtc.data));
}

NVRAM_HANDLER( europc_rtc )
{
	if (file == NULL)
	{
		/* init only */
		/* europc_rtc_set_time(machine); */
	}
	else if (read_or_write)
	{
		europc_rtc_save_stream(file);
	}
	else
	{
		europc_rtc_load_stream(file);
		europc_rtc_set_time(machine);
	}
}

