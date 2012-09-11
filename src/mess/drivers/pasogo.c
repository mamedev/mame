/******************************************************************************
 PeT mess@utanet.at march 2008


 vadem vg230 (single chip pc) based handheld

although it is very related to standard pc hardware, it is different enough
to make the standard pc driver one level more complex, so own driver

******************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "imagedev/cartslot.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
/*
  rtc interrupt irq 2
 */

typedef struct _vg230_t vg230_t;
struct _vg230_t
{
	UINT8 index;
	UINT8 data[0x100];
	struct {
		UINT16 data;
	} bios_timer; // 1.19 MHz tclk signal
	struct {
		int seconds, minutes, hours, days;
		int alarm_seconds, alarm_minutes, alarm_hours, alarm_days;

		int onehertz_interrupt_on;
		int onehertz_interrupt_request;
		int alarm_interrupt_on;
		int alarm_interrupt_request;
	} rtc;
	struct {
		int write_protected;
	} pmu;
};

typedef struct _ems_t ems_t;
struct _ems_t
{
	UINT8 data;
	int index;
	struct {
		UINT8 data[2];
		int address;
		int type;
		int on;
	} mapper[26];
};

class pasogo_state : public driver_device
{
public:
	pasogo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;
	DECLARE_READ8_MEMBER(ems_r);
	DECLARE_WRITE8_MEMBER(ems_w);
	DECLARE_READ8_MEMBER(vg230_io_r);
	DECLARE_WRITE8_MEMBER(vg230_io_w);
	struct _vg230_t m_vg230;
	struct _ems_t m_ems;
	DECLARE_DRIVER_INIT(pasogo);
};


static TIMER_DEVICE_CALLBACK( vg230_timer )
{
	pasogo_state *state = timer.machine().driver_data<pasogo_state>();
	vg230_t *vg230 = &state->m_vg230;

	vg230->rtc.seconds+=1;
	if (vg230->rtc.seconds>=60)
	{
		vg230->rtc.seconds=00;
		vg230->rtc.minutes+=1;
		if (vg230->rtc.minutes>=60)
		{
			vg230->rtc.minutes=0;
			vg230->rtc.hours+=1;
			if (vg230->rtc.hours>=24)
			{
				vg230->rtc.hours=0;
				vg230->rtc.days=(vg230->rtc.days+1)&0xfff;
			}
		}
	}

	if (vg230->rtc.seconds==vg230->rtc.alarm_seconds
		&& vg230->rtc.minutes==vg230->rtc.alarm_minutes
		&& vg230->rtc.hours==vg230->rtc.alarm_hours
		&& (vg230->rtc.days&0x1f)==vg230->rtc.alarm_hours)
	{
		// generate alarm
	}
}

static void vg230_reset(running_machine &machine)
{
	pasogo_state *state = machine.driver_data<pasogo_state>();
	vg230_t *vg230 = &state->m_vg230;
	system_time systime;

	memset(vg230, 0, sizeof(*vg230));
	vg230->pmu.write_protected=TRUE;
	machine.base_datetime(systime);

	vg230->rtc.seconds= systime.local_time.second;
	vg230->rtc.minutes= systime.local_time.minute;
	vg230->rtc.hours = systime.local_time.hour;
	vg230->rtc.days = 0;

	vg230->bios_timer.data=0x7200; // HACK
}

static void vg230_init(running_machine &machine)
{
	vg230_reset(machine);
}


READ8_MEMBER( pasogo_state::vg230_io_r )
{
	vg230_t *vg230 = &m_vg230;
	int log=TRUE;
	UINT8 data=0;

	vg230->bios_timer.data+=0x100; //HACK
	if (offset&1)
	{
		data=vg230->data[vg230->index];
		switch (vg230->index)
		{
			case 0x09: break;
			case 0x0a:
				if (vg230->data[9]&1)
					data=ioport("JOY")->read();
				else
					data=0xff;
				break;

			case 0x30:
				data=vg230->bios_timer.data&0xff;
				break;

			case 0x31:
				data=vg230->bios_timer.data>>8;
				log=FALSE;
				break;

			case 0x70: data=vg230->rtc.seconds; log=FALSE; break;
			case 0x71: data=vg230->rtc.minutes; log=FALSE; break;
			case 0x72: data=vg230->rtc.hours; log=FALSE; break;
			case 0x73: data=vg230->rtc.days; break;
			case 0x74: data=vg230->rtc.days>>8; break;
			case 0x79: /*rtc status*/log=FALSE; break;
			case 0x7a:
				data&=~3;
				if (vg230->rtc.alarm_interrupt_request) data|=1<<1;
				if (vg230->rtc.onehertz_interrupt_request) data|=1<<0;
				break;

			case 0xc1:
				data&=~1;
				if (vg230->pmu.write_protected) data|=1;
				vg230->pmu.write_protected=FALSE;
				log=FALSE;
				break;
		}

		if (log)
			logerror("%.5x vg230 %02x read %.2x\n",(int) m_maincpu->pc(),vg230->index,data);
      //    data=machine.root_device().memregion("maincpu")->base()[0x4000+offset];
	}
	else
		data=vg230->index;

	return data;
}
WRITE8_MEMBER( pasogo_state::vg230_io_w )
{
	vg230_t *vg230 = &m_vg230;
	int log=TRUE;

	if (offset&1)
	{
		//  machine.root_device().memregion("maincpu")->base()[0x4000+offset]=data;
		vg230->data[vg230->index]=data;
		switch (vg230->index)
		{
			case 0x09: break;
			case 0x70: vg230->rtc.seconds=data&0x3f; break;
			case 0x71: vg230->rtc.minutes=data&0x3f; break;
			case 0x72: vg230->rtc.hours=data&0x1f;break;
			case 0x73: vg230->rtc.days=(vg230->rtc.days&~0xff)|data; break;
			case 0x74: vg230->rtc.days=(vg230->rtc.days&0xff)|((data&0xf)<<8); break;
			case 0x75: vg230->rtc.alarm_seconds=data&0x3f; break;
			case 0x76: vg230->rtc.alarm_minutes=data&0x3f; break;
			case 0x77: vg230->rtc.alarm_hours=data&0x1f; break;
			case 0x78: vg230->rtc.days=data&0x1f; break;
			case 0x79:
				vg230->rtc.onehertz_interrupt_on=data&1;
				vg230->rtc.alarm_interrupt_on=data&2;
				log=FALSE;
				break;

			case 0x7a:
				if (data&2)
				{
					vg230->rtc.alarm_interrupt_request=FALSE; vg230->rtc.onehertz_interrupt_request=FALSE; /* update interrupt */
				}
				break;
		}

		if (log)
			logerror("%.5x vg230 %02x write %.2x\n",(int)m_maincpu->pc(),vg230->index,data);
	}
	else
		vg230->index=data;
}

READ8_MEMBER( pasogo_state::ems_r )
{
	ems_t *ems = &m_ems;
	UINT8 data=0;

	switch (offset)
	{
		case 0: data=ems->data; break;
		case 2: case 3: data=ems->mapper[ems->index].data[offset&1]; break;
	}
	return data;
}

WRITE8_MEMBER( pasogo_state::ems_w )
{
	ems_t *ems = &m_ems;
	char bank[10];

	switch (offset)
	{
	case 0:
		ems->data=data;
		switch (data&~3)
		{
		case 0x80: ems->index=0; break;
		case 0x84: ems->index=1; break;
		case 0x88: ems->index=2; break;
		case 0x8c: ems->index=3; break;
		case 0x90: ems->index=4; break;
		case 0x94: ems->index=5; break;
		case 0x98: ems->index=6; break;
		case 0x9c: ems->index=7; break;
		case 0xa0: ems->index=8; break;
		case 0xa4: ems->index=9; break;
		case 0xa8: ems->index=10; break;
		case 0xac: ems->index=11; break;
		case 0xb0: ems->index=12; break;
		case 0xb4: ems->index=13; break;
      //  case 0xb8: ems->index=14; break;
      //  case 0xbc: ems->index=15; break;
		case 0xc0: ems->index=14; break;
		case 0xc4: ems->index=15; break;
		case 0xc8: ems->index=16; break;
		case 0xcc: ems->index=17; break;
		case 0xd0: ems->index=18; break;
		case 0xd4: ems->index=19; break;
		case 0xd8: ems->index=20; break;
		case 0xdc: ems->index=21; break;
		case 0xe0: ems->index=22; break;
		case 0xe4: ems->index=23; break;
		case 0xe8: ems->index=24; break;
		case 0xec: ems->index=25; break;
		}
		break;

	case 2:
	case 3:
		ems->mapper[ems->index].data[offset&1]=data;
		ems->mapper[ems->index].address=(ems->mapper[ems->index].data[0]<<14)|((ems->mapper[ems->index].data[1]&0xf)<<22);
		ems->mapper[ems->index].on=ems->mapper[ems->index].data[1]&0x80;
		ems->mapper[ems->index].type=(ems->mapper[ems->index].data[1]&0x70)>>4;
		logerror("%.5x ems mapper %d(%05x)on:%d type:%d address:%07x\n",(int)m_maincpu->pc(),ems->index, ems->data<<12,
			ems->mapper[ems->index].on, ems->mapper[ems->index].type, ems->mapper[ems->index].address );

		switch (ems->mapper[ems->index].type)
		{
		case 0: /*external*/
		case 1: /*ram*/
			sprintf(bank,"bank%d",ems->index+1);
			membank( bank )->set_base( memregion("maincpu")->base() + (ems->mapper[ems->index].address&0xfffff) );
			break;
		case 3: /* rom 1 */
		case 4: /* pc card a */
		case 5: /* pc card b */
		default:
			break;
		case 2:
			sprintf(bank,"bank%d",ems->index+1);
			membank( bank )->set_base( memregion("user1")->base() + (ems->mapper[ems->index].address&0xfffff) );
			break;
		}
		break;
	}
}

static ADDRESS_MAP_START(pasogo_mem, AS_PROGRAM, 8, pasogo_state)
//  AM_RANGE(0x00000, 0xfffff) AM_UNMAP AM_MASK(0xfffff)
//  AM_RANGE( 0x4000, 0x7fff) AM_READWRITE(gmaster_io_r, gmaster_io_w)
	ADDRESS_MAP_GLOBAL_MASK(0xffFFF)
	AM_RANGE(0x00000, 0x7ffff) AM_RAM
	AM_RANGE(0x80000, 0x83fff) AM_RAMBANK("bank1")
	AM_RANGE(0x84000, 0x87fff) AM_RAMBANK("bank2")
	AM_RANGE(0x88000, 0x8bfff) AM_RAMBANK("bank3")
	AM_RANGE(0x8c000, 0x8ffff) AM_RAMBANK("bank4")
	AM_RANGE(0x90000, 0x93fff) AM_RAMBANK("bank5")
	AM_RANGE(0x94000, 0x97fff) AM_RAMBANK("bank6")
	AM_RANGE(0x98000, 0x9bfff) AM_RAMBANK("bank7")
	AM_RANGE(0x9c000, 0x9ffff) AM_RAMBANK("bank8")
	AM_RANGE(0xa0000, 0xa3fff) AM_RAMBANK("bank9")
	AM_RANGE(0xa4000, 0xa7fff) AM_RAMBANK("bank10")
	AM_RANGE(0xa8000, 0xabfff) AM_RAMBANK("bank11")
	AM_RANGE(0xac000, 0xaffff) AM_RAMBANK("bank12")
	AM_RANGE(0xb0000, 0xb3fff) AM_RAMBANK("bank13")
	AM_RANGE(0xb4000, 0xb7fff) AM_RAMBANK("bank14")
//  AM_RANGE(0xb8000, 0xbffff) AM_RAM
	AM_RANGE(0xb8000, 0xbffff) AM_RAMBANK("bank28")
	AM_RANGE(0xc0000, 0xc3fff) AM_RAMBANK("bank15")
	AM_RANGE(0xc4000, 0xc7fff) AM_RAMBANK("bank16")
	AM_RANGE(0xc8000, 0xcbfff) AM_RAMBANK("bank17")
	AM_RANGE(0xcc000, 0xcffff) AM_RAMBANK("bank18")
	AM_RANGE(0xd0000, 0xd3fff) AM_RAMBANK("bank19")
	AM_RANGE(0xd4000, 0xd7fff) AM_RAMBANK("bank20")
	AM_RANGE(0xd8000, 0xdbfff) AM_RAMBANK("bank21")
	AM_RANGE(0xdc000, 0xdffff) AM_RAMBANK("bank22")
	AM_RANGE(0xe0000, 0xe3fff) AM_RAMBANK("bank23")
	AM_RANGE(0xe4000, 0xe7fff) AM_RAMBANK("bank24")
	AM_RANGE(0xe8000, 0xebfff) AM_RAMBANK("bank25")
	AM_RANGE(0xec000, 0xeffff) AM_RAMBANK("bank26")

	AM_RANGE(0xf0000, 0xfffff) AM_ROMBANK("bank27")
ADDRESS_MAP_END

static ADDRESS_MAP_START(pasogo_io, AS_IO, 8, pasogo_state)
//  ADDRESS_MAP_GLOBAL_MASK(0xfFFF)
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE_LEGACY("pic8259", pic8259_r, pic8259_w)
	AM_RANGE(0x26, 0x27) AM_READWRITE(vg230_io_r, vg230_io_w )
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE_LEGACY("pit8254", pit8253_r, pit8253_w)
	AM_RANGE(0x6c, 0x6f) AM_READWRITE(ems_r, ems_w )
ADDRESS_MAP_END

static INPUT_PORTS_START( pasogo )
PORT_START("JOY")
//  PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SELECT)  PORT_NAME("select")
//  PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START) PORT_NAME("start")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("O") /*?*/
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("X") /*?*/
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP   )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("a") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("b") PORT_CODE(KEYCODE_B)
INPUT_PORTS_END

/* palette in red, green, blue tribles */
static const unsigned char pasogo_palette[][3] =
{
	{ 0, 0, 0 },
	{ 45,45,43 },
	{ 130, 159, 166 },
	{ 255,255,255 }
};

static PALETTE_INIT( pasogo )
{
	int i;

	for ( i = 0; i < ARRAY_LENGTH(pasogo_palette); i++ )
	{
		palette_set_color_rgb(machine, i, pasogo_palette[i][0], pasogo_palette[i][1], pasogo_palette[i][2]);
	}
}

static SCREEN_UPDATE_IND16( pasogo )
{
	//static int width=-1,height=-1;
	UINT8 *rom = screen.machine().root_device().memregion("maincpu")->base()+0xb8000;
	static const UINT16 c[]={ 3, 0 };
	int x,y;
//  plot_box(bitmap, 0, 0, 64/*bitmap.width*/, bitmap.height, 0);
	int w=640;
	int h=240;
	if (0)
	{
		w=320;
		h=240;
		for (y=0;y<h; y++)
		{
			for (x=0;x<w;x+=4)
			{
				int a=(y&1)*0x2000;
				UINT8 d=rom[a+(y&~1)*80/2+x/4];
				UINT16 *line=&bitmap.pix16(y, x);
				*line++=(d>>6)&3;
				*line++=(d>>4)&3;
				*line++=(d>>2)&3;
				*line++=(d>>0)&3;
			}
		}
	}
	else
	{
		for (y=0;y<h; y++)
		{
			for (x=0;x<w;x+=8)
			{
				int a=(y&3)*0x2000;
				UINT8 d=rom[a+(y&~3)*80/4+x/8];
				UINT16 *line=&bitmap.pix16(y, x);
				*line++=c[(d>>7)&1];
				*line++=c[(d>>6)&1];
				*line++=c[(d>>5)&1];
				*line++=c[(d>>4)&1];
				*line++=c[(d>>3)&1];
				*line++=c[(d>>2)&1];
				*line++=c[(d>>1)&1];
				*line++=c[(d>>0)&1];
			}
		}
	}
#if 0
	if (w!=width || h!=height)
	{
		width=w; height=h;
//      machine.primary_screen->set_visible_area(0, width-1, 0, height-1);
		screen.set_visible_area(0, width-1, 0, height-1);
	}
#endif
	return 0;
}

static INTERRUPT_GEN( pasogo_interrupt )
{
//  cputag_set_input_line(machine, "maincpu", UPD7810_INTFE1, PULSE_LINE);
}

static IRQ_CALLBACK(pasogo_irq_callback)
{
	return pic8259_acknowledge( device->machine().device("pic8259"));
}

static MACHINE_RESET( pasogo )
{
	device_set_irq_callback(machine.device("maincpu"), pasogo_irq_callback);
}

//static const unsigned i86_address_mask = 0x000fffff;

static const struct pit8253_config pc_pit8254_config =
{
	{
		{
			4772720/4,				/* heartbeat IRQ */
			DEVCB_NULL,
			DEVCB_DEVICE_LINE("pic8259", pic8259_ir0_w)
		}, {
			4772720/4,				/* dram refresh */
			DEVCB_NULL,
			DEVCB_NULL
		}, {
			4772720/4,				/* pio port c pin 4, and speaker polling enough */
			DEVCB_NULL,
			DEVCB_NULL
		}
	}
};


static WRITE_LINE_DEVICE_HANDLER( pasogo_pic8259_set_int_line )
{
	cputag_set_input_line(device->machine(), "maincpu", 0, state ? HOLD_LINE : CLEAR_LINE);
}

static const struct pic8259_interface pasogo_pic8259_config =
{
	DEVCB_LINE(pasogo_pic8259_set_int_line),
	DEVCB_LINE_VCC,
	DEVCB_NULL
};


static MACHINE_CONFIG_START( pasogo, pasogo_state )

	MCFG_CPU_ADD("maincpu", I80188/*V30HL in vadem vg230*/, 10000000/*?*/)
	MCFG_CPU_PROGRAM_MAP(pasogo_mem)
	MCFG_CPU_IO_MAP( pasogo_io)
	MCFG_CPU_VBLANK_INT("screen", pasogo_interrupt)
//  MCFG_CPU_CONFIG(i86_address_mask)
	MCFG_MACHINE_RESET( pasogo )

	MCFG_PIT8254_ADD( "pit8254", pc_pit8254_config )

	MCFG_PIC8259_ADD( "pic8259", pasogo_pic8259_config )

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(640, 400)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 400-1)
	MCFG_SCREEN_UPDATE_STATIC(pasogo)

	MCFG_PALETTE_LENGTH(ARRAY_LENGTH(pasogo_palette))
	MCFG_PALETTE_INIT(pasogo)
#if 0
	MCFG_SPEAKER_STANDARD_MONO("gmaster")
	MCFG_SOUND_ADD("custom", CUSTOM, 0)
	MCFG_SOUND_CONFIG(gmaster_sound_interface)
	MCFG_SOUND_ROUTE(0, "gmaster", 0.50)
#endif

	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin")
	MCFG_CARTSLOT_MANDATORY
	MCFG_CARTSLOT_INTERFACE("pasogo_cart")
	MCFG_SOFTWARE_LIST_ADD("cart_list","pasogo")

	MCFG_TIMER_ADD_PERIODIC("vg230_timer", vg230_timer, attotime::from_hz(1))
MACHINE_CONFIG_END


ROM_START(pasogo)
	ROM_REGION(0x100000,"maincpu", ROMREGION_ERASEFF) // 1 megabyte dram?
	ROM_REGION(0x100000,"user1", ROMREGION_ERASEFF)
	ROM_CART_LOAD("cart", 0, 0x100000, ROM_NOMIRROR)
ROM_END


DRIVER_INIT_MEMBER(pasogo_state,pasogo)
{
	vg230_init(machine());
	memset(&m_ems, 0, sizeof(m_ems));
	membank( "bank27" )->set_base( machine().root_device().memregion("user1")->base() + 0x00000 );
	membank( "bank28" )->set_base( memregion("maincpu")->base() + 0xb8000/*?*/ );
}

//    YEAR   NAME    PARENT  COMPAT    MACHINE   INPUT     INIT      COMPANY  FULLNAME          FLAGS
CONS( 1996, pasogo,   0,      0,       pasogo,  pasogo, pasogo_state,    pasogo,   "KOEI", "PasoGo", GAME_NO_SOUND|GAME_NOT_WORKING)
