/***************************************************************************

    QX-10

    Preliminary driver by Mariusz Wojcieszek

    Status:
    Driver boots and load CP/M from floppy image. Needs upd7220 for gfx
    and keyboard hooked to upd7021.

    Done:
    - preliminary memory map
    - floppy (upd765)
    - DMA
    - Interrupts (pic8295)

    banking:
    - 0x1c = 0
    AM_RANGE(0x0000,0x1fff) ROM
    AM_RANGE(0x2000,0xdfff) NOP
    AM_RANGE(0xe000,0xffff) RAM
    - 0x1c = 1 0x20 = 1
    AM_RANGE(0x0000,0x7fff) RAM (0x18 selects bank)
    AM_RANGE(0x8000,0x87ff) CMOS
    AM_RANGE(0x8800,0xdfff) NOP or previous bank?
    AM_RANGE(0xe000,0xffff) RAM
    - 0x1c = 1 0x20 = 0
    AM_RANGE(0x0000,0xdfff) RAM (0x18 selects bank)
    AM_RANGE(0xe000,0xffff) RAM
****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/upd7201.h"
#include "machine/mc146818.h"
#include "machine/i8255.h"
#include "machine/8237dma.h"
#include "video/upd7220.h"
#include "machine/upd765.h"
#include "machine/ram.h"

#define MAIN_CLK	15974400

/*
    Driver data
*/

class qx10_state : public driver_device
{
public:
	qx10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_pit_1(*this, "pit8253_1"),
	m_pit_2(*this, "pit8253_2"),
	m_pic_m(*this, "pic8259_master"),
	m_pic_s(*this, "pic8259_slave"),
	m_scc(*this, "upd7201"),
	m_ppi(*this, "i8255"),
	m_dma_1(*this, "8237dma_1"),
	m_dma_2(*this, "8237dma_2"),
	m_fdc(*this, "upd765"),
	m_hgdc(*this, "upd7220"),
	m_rtc(*this, "rtc"),
	m_vram_bank(0),
	m_video_ram(*this, "video_ram"){ }

	required_device<device_t> m_pit_1;
	required_device<device_t> m_pit_2;
	required_device<device_t> m_pic_m;
	required_device<device_t> m_pic_s;
	required_device<upd7201_device> m_scc;
	required_device<i8255_device> m_ppi;
	required_device<i8237_device> m_dma_1;
	required_device<i8237_device> m_dma_2;
	required_device<device_t> m_fdc;
	required_device<upd7220_device> m_hgdc;
	required_device<mc146818_device> m_rtc;
	UINT8 m_vram_bank;
	required_shared_ptr<UINT8> m_video_ram;

	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();

	void update_memory_mapping();

	DECLARE_WRITE8_MEMBER( qx10_18_w );
	DECLARE_WRITE8_MEMBER( prom_sel_w );
	DECLARE_WRITE8_MEMBER( cmos_sel_w );
	DECLARE_WRITE_LINE_MEMBER( qx10_upd765_interrupt );
	DECLARE_WRITE_LINE_MEMBER( drq_w );
	DECLARE_WRITE8_MEMBER( fdd_motor_w );
	DECLARE_READ8_MEMBER( qx10_30_r );
	DECLARE_READ8_MEMBER( gdc_dack_r );
	DECLARE_WRITE8_MEMBER( gdc_dack_w );
	DECLARE_WRITE_LINE_MEMBER( tc_w );
	DECLARE_READ8_MEMBER( mc146818_data_r );
	DECLARE_WRITE8_MEMBER( mc146818_data_w );
	DECLARE_WRITE8_MEMBER( mc146818_offset_w );
	DECLARE_WRITE_LINE_MEMBER( qx10_pic8259_master_set_int_line );
	DECLARE_READ8_MEMBER( get_slave_ack );
	DECLARE_READ8_MEMBER( upd7201_r );
	DECLARE_WRITE8_MEMBER( upd7201_w );
	DECLARE_READ8_MEMBER( vram_bank_r );
	DECLARE_WRITE8_MEMBER( vram_bank_w );
	DECLARE_READ8_MEMBER( vram_r );
	DECLARE_WRITE8_MEMBER( vram_w );

	UINT8 *m_char_rom;

	int		m_mc146818_offset;

	/* FDD */
	int		m_fdcint;
	int		m_fdcmotor;
	int		m_fdcready;

	/* memory */
	int		m_membank;
	int		m_memprom;
	int		m_memcmos;
	UINT8	m_cmosram[0x800];

	UINT8 m_color_mode;

	struct{
		UINT8 repeat,enable;
		int repeat_start_time,repeat_interval;
		UINT8 led[8];
		UINT8 rx;
	}m_keyb;

	struct{
		UINT8 rx;
	}m_rs232c;

	virtual void palette_init();
};

static UPD7220_DISPLAY_PIXELS( hgdc_display_pixels )
{
	qx10_state *state = device->machine().driver_data<qx10_state>();
	int xi,gfx[3];
	UINT8 pen;

	if(state->m_color_mode)
	{
		gfx[0] = state->m_video_ram[(address * 2) + 0x00000];
		gfx[1] = state->m_video_ram[(address * 2) + 0x20000];
		gfx[2] = state->m_video_ram[(address * 2) + 0x40000];
	}
	else
	{
		gfx[0] = state->m_video_ram[address];
		gfx[1] = 0;
		gfx[2] = 0;
	}

	for(xi=0;xi<8;xi++)
	{
		pen = ((gfx[0] >> (7-xi)) & 1) ? 1 : 0;
		pen|= ((gfx[1] >> (7-xi)) & 1) ? 2 : 0;
		pen|= ((gfx[2] >> (7-xi)) & 1) ? 4 : 0;

		bitmap.pix16(y, x + xi) = pen;
	}
}

static UPD7220_DRAW_TEXT_LINE( hgdc_draw_text )
{
	qx10_state *state = device->machine().driver_data<qx10_state>();
	int x;
	int xi,yi;
	int tile;
	int attr;
	UINT8 color;
	UINT8 tile_data;
	UINT8 pen;

	for( x = 0; x < pitch; x++ )
	{
		tile = state->m_video_ram[(addr+x)*2];
		attr = state->m_video_ram[(addr+x)*2+1];

		color = (state->m_color_mode) ? 1 : (attr & 4) ? 2 : 1; /* TODO: color mode */

		for( yi = 0; yi < lr; yi++)
		{
			tile_data = (state->m_char_rom[tile*16+yi]);

			if(attr & 8)
				tile_data^=0xff;

			if(cursor_on && cursor_addr == addr+x) //TODO
				tile_data^=0xff;

			if(attr & 0x80 && device->machine().primary_screen->frame_number() & 0x10) //TODO: check for blinking interval
				tile_data=0;

			for( xi = 0; xi < 8; xi++)
			{
				int res_x,res_y;

				if(yi >= 16)
					pen = 0;
				else
					pen = ((tile_data >> xi) & 1) ? color : 0;

				res_x = x * 8 + xi;
				res_y = y * lr + yi;

				if(res_x > screen_max_x || res_y > screen_max_y)
					continue;

				if(pen)
					bitmap.pix16(res_y, res_x) = pen;
			}
		}
	}
}

/*
    Memory
*/
void qx10_state::update_memory_mapping()
{
	int drambank = 0;

	if (m_membank & 1)
	{
		drambank = 0;
	}
	else if (m_membank & 2)
	{
		drambank = 1;
	}
	else if (m_membank & 4)
	{
		drambank = 2;
	}
	else if (m_membank & 8)
	{
		drambank = 3;
	}

	if (!m_memprom)
	{
		membank("bank1")->set_base(machine().root_device().memregion("maincpu")->base());
	}
	else
	{
		membank("bank1")->set_base(machine().device<ram_device>(RAM_TAG)->pointer() + drambank*64*1024);
	}
	if (m_memcmos)
	{
		membank("bank2")->set_base(m_cmosram);
	}
	else
	{
		membank("bank2")->set_base(machine().device<ram_device>(RAM_TAG)->pointer() + drambank*64*1024 + 32*1024);
	}
}

WRITE8_MEMBER( qx10_state::qx10_18_w )
{
	m_membank = (data >> 4) & 0x0f;
	update_memory_mapping();
}

WRITE8_MEMBER( qx10_state::prom_sel_w )
{
	m_memprom = data & 1;
	update_memory_mapping();
}

WRITE8_MEMBER( qx10_state::cmos_sel_w )
{
	m_memcmos = data & 1;
	update_memory_mapping();
}

/*
    FDD
*/

static const floppy_interface qx10_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(default),
	NULL,
	NULL
};

WRITE_LINE_MEMBER( qx10_state::qx10_upd765_interrupt )
{
	m_fdcint = state;

	//logerror("Interrupt from upd765: %d\n", state);
	// signal interrupt
	pic8259_ir6_w(m_pic_m, state);
};

WRITE_LINE_MEMBER( qx10_state::drq_w )
{
	i8237_dreq0_w(m_dma_1, !state);
}

static const struct upd765_interface qx10_upd765_interface =
{
	DEVCB_DRIVER_LINE_MEMBER(qx10_state, qx10_upd765_interrupt),
	DEVCB_DRIVER_LINE_MEMBER(qx10_state, drq_w),
	NULL,
	UPD765_RDY_PIN_CONNECTED,
	{FLOPPY_0,FLOPPY_1, NULL, NULL}
};

WRITE8_MEMBER( qx10_state::fdd_motor_w )
{
	m_fdcmotor = 1;

	floppy_mon_w(floppy_get_device(machine(), 0), CLEAR_LINE);
	floppy_drive_set_ready_state(floppy_get_device(machine(), 0), 1,1);
	// motor off controlled by clock
};

READ8_MEMBER( qx10_state::qx10_30_r )
{
	floppy_image_legacy *floppy1,*floppy2;

	floppy1 = flopimg_get_image(floppy_get_device(machine(), 0));
	floppy2 = flopimg_get_image(floppy_get_device(machine(), 1));

	return m_fdcint |
		   /*m_fdcmotor*/ 0 << 1 |
		   ((floppy1 != NULL) || (floppy2 != NULL) ? 1 : 0) << 3 |
		   m_membank << 4;
};

/*
    DMA8237
*/
WRITE_LINE_DEVICE_HANDLER( dma_hrq_changed )
{
	/* Assert HLDA */
	i8237_hlda_w(device, state);
}

READ8_MEMBER( qx10_state::gdc_dack_r )
{
	logerror("GDC DACK read\n");
	return 0;
}

WRITE8_MEMBER( qx10_state::gdc_dack_w )
{
	logerror("GDC DACK write %02x\n", data);
}

WRITE_LINE_MEMBER( qx10_state::tc_w )
{
	/* floppy terminal count */
	upd765_tc_w(m_fdc, !state);
}

/*
    8237 DMA (Master)
    Channel 1: Floppy disk
    Channel 2: GDC
    Channel 3: Option slots
*/
static UINT8 memory_read_byte(address_space *space, offs_t address) { return space->read_byte(address); }
static void memory_write_byte(address_space *space, offs_t address, UINT8 data) { space->write_byte(address, data); }

static I8237_INTERFACE( qx10_dma8237_1_interface )
{
	DEVCB_DEVICE_LINE("8237dma_1", dma_hrq_changed),
	DEVCB_DRIVER_LINE_MEMBER(qx10_state, tc_w),
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, memory_read_byte),
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, memory_write_byte),
	{ DEVCB_DEVICE_HANDLER("upd765", upd765_dack_r), DEVCB_DRIVER_MEMBER(qx10_state, gdc_dack_r),/*DEVCB_DEVICE_HANDLER("upd7220", upd7220_dack_r)*/ DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_DEVICE_HANDLER("upd765", upd765_dack_w), DEVCB_DRIVER_MEMBER(qx10_state, gdc_dack_w),/*DEVCB_DEVICE_HANDLER("upd7220", upd7220_dack_w)*/ DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL }
};

/*
    8237 DMA (Slave)
    Channel 1: Option slots #1
    Channel 2: Option slots #2
    Channel 3: Option slots #3
    Channel 4: Option slots #4
*/
static I8237_INTERFACE( qx10_dma8237_2_interface )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL }
};

/*
    8255
*/
static I8255_INTERFACE(qx10_i8255_interface)
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

/*
    MC146818
*/
READ8_MEMBER( qx10_state::mc146818_data_r )
{
	return m_rtc->read(space, m_mc146818_offset);
};

WRITE8_MEMBER( qx10_state::mc146818_data_w )
{
	m_rtc->write(space, m_mc146818_offset, data);
};

WRITE8_MEMBER( qx10_state::mc146818_offset_w )
{
	m_mc146818_offset = data;
};

/*
    UPD7201
    Channel A: Keyboard
    Channel B: RS232
*/

static UPD7201_INTERFACE(qx10_upd7201_interface)
{
	DEVCB_NULL,					/* interrupt */
	{
		{
			0,					/* receive clock */
			0,					/* transmit clock */
			DEVCB_NULL,			/* receive DRQ */
			DEVCB_NULL,			/* transmit DRQ */
			DEVCB_NULL,			/* receive data */
			DEVCB_NULL,			/* transmit data */
			DEVCB_NULL,			/* clear to send */
			DEVCB_NULL,			/* data carrier detect */
			DEVCB_NULL,			/* ready to send */
			DEVCB_NULL,			/* data terminal ready */
			DEVCB_NULL,			/* wait */
			DEVCB_NULL			/* sync output */
		}, {
			0,					/* receive clock */
			0,					/* transmit clock */
			DEVCB_NULL,			/* receive DRQ */
			DEVCB_NULL,			/* transmit DRQ */
			DEVCB_NULL,			/* receive data */
			DEVCB_NULL,			/* transmit data */
			DEVCB_NULL,			/* clear to send */
			DEVCB_NULL,			/* data carrier detect */
			DEVCB_NULL,			/* ready to send */
			DEVCB_NULL,			/* data terminal ready */
			DEVCB_NULL,			/* wait */
			DEVCB_NULL			/* sync output */
		}
	}
};

/*
    Timer 0
    Counter CLK                         Gate                    OUT             Operation
    0       Keyboard clock (1200bps)    Memory register D0      Speaker timer   Speaker timer (100ms)
    1       Keyboard clock (1200bps)    +5V                     8259A (10E) IR5 Software timer
    2       Clock 1,9668MHz             Memory register D7      8259 (12E) IR1  Software timer
*/

static const struct pit8253_config qx10_pit8253_1_config =
{
	{
		{ 1200,         DEVCB_NULL,     DEVCB_NULL },
		{ 1200,         DEVCB_LINE_VCC, DEVCB_NULL },
		{ MAIN_CLK / 8, DEVCB_NULL,     DEVCB_NULL },
	}
};

/*
    Timer 1
    Counter CLK                 Gate        OUT                 Operation
    0       Clock 1,9668MHz     +5V         Speaker frequency   1kHz
    1       Clock 1,9668MHz     +5V         Keyboard clock      1200bps (Clock / 1664)
    2       Clock 1,9668MHz     +5V         RS-232C baud rate   9600bps (Clock / 208)
*/
static const struct pit8253_config qx10_pit8253_2_config =
{
	{
		{ MAIN_CLK / 8, DEVCB_LINE_VCC, DEVCB_NULL },
		{ MAIN_CLK / 8, DEVCB_LINE_VCC, DEVCB_NULL },
		{ MAIN_CLK / 8, DEVCB_LINE_VCC, DEVCB_NULL },
	}
};


/*
    Master PIC8259
    IR0     Power down detection interrupt
    IR1     Software timer #1 interrupt
    IR2     External interrupt INTF1
    IR3     External interrupt INTF2
    IR4     Keyboard/RS232 interrupt
    IR5     CRT/lightpen interrupt
    IR6     Floppy controller interrupt
*/

WRITE_LINE_MEMBER( qx10_state::qx10_pic8259_master_set_int_line )
{
	machine().device("maincpu")->execute().set_input_line(0, state ? HOLD_LINE : CLEAR_LINE);
}

READ8_MEMBER( qx10_state::get_slave_ack )
{
	if (offset==7) { // IRQ = 7
		return pic8259_acknowledge(m_pic_s);
	}
	return 0x00;
}

static const struct pic8259_interface qx10_pic8259_master_config =
{
	DEVCB_DRIVER_LINE_MEMBER(qx10_state, qx10_pic8259_master_set_int_line),
	DEVCB_LINE_VCC,
	DEVCB_DRIVER_MEMBER(qx10_state, get_slave_ack)
};

/*
    Slave PIC8259
    IR0     Printer interrupt
    IR1     External interrupt #1
    IR2     Calendar clock interrupt
    IR3     External interrupt #2
    IR4     External interrupt #3
    IR5     Software timer #2 interrupt
    IR6     External interrupt #4
    IR7     External interrupt #5

*/

static const struct pic8259_interface qx10_pic8259_slave_config =
{
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir7_w),
	DEVCB_LINE_GND,
	DEVCB_NULL
};

static IRQ_CALLBACK( irq_callback )
{
	return pic8259_acknowledge(device->machine().driver_data<qx10_state>()->m_pic_m );
}

READ8_MEMBER( qx10_state::upd7201_r )
{
	if((offset & 2) == 0)
	{
		return m_keyb.rx;
	}
	//printf("R [%02x]\n",offset);

	return m_rs232c.rx;
}

WRITE8_MEMBER( qx10_state::upd7201_w )
{
	if((offset & 2) == 0) //keyb TX
	{
		switch(data & 0xe0)
		{
			case 0x00:
				m_keyb.repeat_start_time = 300+(data & 0x1f)*25;
				printf("keyb Set repeat start time, %d ms\n",m_keyb.repeat_start_time);
				break;
			case 0x20:
				m_keyb.repeat_interval = 30+(data & 0x1f)*5;
				printf("keyb Set repeat interval, %d ms\n",m_keyb.repeat_interval);
				break;
			case 0x40:
				m_keyb.led[(data & 0xe) >> 1] = data & 1;
				printf("keyb Set led %02x %s\n",((data & 0xe) >> 1),data & 1 ? "on" : "off");
				m_keyb.rx = (data & 0xf) | 0xc0;
				pic8259_ir4_w(machine().device("pic8259_master"), 1);
				break;
			case 0x60:
				printf("keyb Read LED status\n");
				// 0x80 + data
				break;
			case 0x80:
				printf("keyb Read SW status\n");
				// 0xc0 + data
				break;
			case 0xa0:
				m_keyb.repeat = data & 1;
				//printf("keyb repeat flag issued %s\n",data & 1 ? "on" : "off");
				break;
			case 0xc0:
				m_keyb.enable = data & 1;
				printf("keyb Enable flag issued %s\n",data & 1 ? "on" : "off");
				break;
			case 0xe0:
				printf("keyb Reset Issued, diagnostic is %s\n",data & 1 ? "on" : "off");
				m_keyb.rx = 0;
				break;
		}
	}
	else //RS-232c TX
	{
		//printf("RS-232c W %02x\n",data);
		if(data == 0x01) //cheap, but needed for working inputs in "The QX-10 Diagnostic"
			m_rs232c.rx = 0x04;
		else if(data == 0x00)
			m_rs232c.rx = 0xfe;
		else
			m_rs232c.rx = 0xff;
	}

}

READ8_MEMBER( qx10_state::vram_bank_r )
{
	return m_vram_bank;
}

WRITE8_MEMBER( qx10_state::vram_bank_w )
{
	if(m_color_mode)
	{
		m_vram_bank = data & 7;
		if(data != 1 && data != 2 && data != 4)
			printf("%02x\n",data);
	}
}

static ADDRESS_MAP_START(qx10_mem, AS_PROGRAM, 8, qx10_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x7fff ) AM_RAMBANK("bank1")
	AM_RANGE( 0x8000, 0xdfff ) AM_RAMBANK("bank2")
	AM_RANGE( 0xe000, 0xffff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( qx10_io , AS_IO, 8, qx10_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE_LEGACY("pit8253_1", pit8253_r, pit8253_w)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE_LEGACY("pit8253_2", pit8253_r, pit8253_w)
	AM_RANGE(0x08, 0x09) AM_DEVREADWRITE_LEGACY("pic8259_master", pic8259_r, pic8259_w)
	AM_RANGE(0x0c, 0x0d) AM_DEVREADWRITE_LEGACY("pic8259_slave", pic8259_r, pic8259_w)
	AM_RANGE(0x10, 0x13) AM_READWRITE(upd7201_r, upd7201_w) //AM_DEVREADWRITE("upd7201", upd7201_device, cd_ba_r, cd_ba_w)
	AM_RANGE(0x14, 0x17) AM_DEVREADWRITE("i8255", i8255_device, read, write)
	AM_RANGE(0x18, 0x1b) AM_READ_PORT("DSW") AM_WRITE(qx10_18_w)
	AM_RANGE(0x1c, 0x1f) AM_WRITE(prom_sel_w)
	AM_RANGE(0x20, 0x23) AM_WRITE(cmos_sel_w)
	AM_RANGE(0x2c, 0x2c) AM_READ_PORT("CONFIG")
	AM_RANGE(0x2d, 0x2d) AM_READWRITE(vram_bank_r,vram_bank_w)
	AM_RANGE(0x30, 0x33) AM_READWRITE(qx10_30_r, fdd_motor_w)
	AM_RANGE(0x34, 0x34) AM_DEVREAD_LEGACY("upd765", upd765_status_r)
	AM_RANGE(0x35, 0x35) AM_DEVREADWRITE_LEGACY("upd765", upd765_data_r, upd765_data_w)
	AM_RANGE(0x38, 0x39) AM_DEVREADWRITE("upd7220", upd7220_device, read, write)
//  AM_RANGE(0x3a, 0x3a) GDC zoom
//  AM_RANGE(0x3b, 0x3b) GDC light pen req
	AM_RANGE(0x3c, 0x3c) AM_READWRITE(mc146818_data_r, mc146818_data_w)
	AM_RANGE(0x3d, 0x3d) AM_WRITE(mc146818_offset_w)
	AM_RANGE(0x40, 0x4f) AM_DEVREADWRITE_LEGACY("8237dma_1", i8237_r, i8237_w)
	AM_RANGE(0x50, 0x5f) AM_DEVREADWRITE_LEGACY("8237dma_2", i8237_r, i8237_w)
//  AM_RANGE(0xfc, 0xfd) Multi-Font comms
ADDRESS_MAP_END

/* Input ports */
/* TODO: shift break */
static INPUT_CHANGED( key_stroke )
{
	qx10_state *state = field.machine().driver_data<qx10_state>();

	if(newval && !oldval)
	{
		state->m_keyb.rx = (UINT8)(FPTR)(param) & 0x7f;
		pic8259_ir4_w(field.machine().device("pic8259_master"), 1);
	}

	if(oldval && !newval)
		state->m_keyb.rx = 0;
}

static INPUT_PORTS_START( qx10 )
/* TODO: Several buttons (namely the UNDO / STORE / RETRIEVE etc.) are presumably multiple keypresses */
	PORT_START("KEY0") // 0x00 - 0x07
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("UNDO") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x01)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("(H1)") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x02)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("STORE") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x03)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RETRIEVE") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x04)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PRINT") /*PORT_CODE(KEYCODE_5) PORT_CHAR('5')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x05)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("INDEX") /*PORT_CODE(KEYCODE_6) PORT_CHAR('6')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x06)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("MAIL") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x07)

	PORT_START("KEY1") // 0x08 - 0x0f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("(H2)") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x08)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("MENU") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x09)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CALC") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x0a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SCHED") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x0b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DRAW") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x0c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("(H3)") /*PORT_CODE(KEYCODE_5) PORT_CHAR('5')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x0d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("BOLD") /*PORT_CODE(KEYCODE_6) PORT_CHAR('6')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x0e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ITALIC") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x0f)

	PORT_START("KEY2") // 0x10 - 0x17
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_UNUSED) //PORT_CHANGED(key_stroke, 0x10)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_UNUSED) //PORT_CHANGED(key_stroke, 0x11)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_UNUSED) //PORT_CHANGED(key_stroke, 0x12)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_UNUSED) //PORT_CHANGED(key_stroke, 0x13)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_UNUSED) //PORT_CHANGED(key_stroke, 0x14)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ENTER (PAD)") PORT_CODE(KEYCODE_ENTER_PAD) /*PORT_CHAR(0xd)*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x15)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(". (PAD)") PORT_CODE(KEYCODE_DEL_PAD) /*PORT_CHAR('6')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x16)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0 (PAD)") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR('0') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x17)

	PORT_START("KEY3") // 0x18 - 0x1f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("= (PAD)") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x18)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6 (PAD)") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR('6') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x19)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5 (PAD)") PORT_CODE(KEYCODE_5_PAD) PORT_CHAR('5') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x1a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4 (PAD)") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR('4') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x1b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("(H5)") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x1c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("(H4)") /*PORT_CODE(KEYCODE_5) PORT_CHAR(0xd)*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x1d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("STYLE") /*PORT_CODE(KEYCODE_6) PORT_CHAR('6')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x1e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SIZE") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x1f)

	PORT_START("KEY4") // 0x20 - 0x27
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RSHIFT") PORT_CODE(KEYCODE_RSHIFT) PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x20)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_UNUSED) //PORT_CHANGED(key_stroke, 0x21)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_UNUSED) //PORT_CHANGED(key_stroke, 0x22)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_UNUSED) //PORT_CHANGED(key_stroke, 0x23)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_UNUSED) //PORT_CHANGED(key_stroke, 0x24)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3 (PAD)") PORT_CODE(KEYCODE_3_PAD) PORT_CHAR('3') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x25)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2 (PAD)") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR('2') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x26)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1 (PAD)") PORT_CODE(KEYCODE_1_PAD) PORT_CHAR('1') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x27)

	PORT_START("KEY5") // 0x28 - 0x2f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("+ (PAD)") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR('+') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x28)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9 (PAD)") PORT_CODE(KEYCODE_9_PAD) PORT_CHAR('9') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x29)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8 (PAD)") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR('8') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x2a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7 (PAD)") PORT_CODE(KEYCODE_7_PAD) PORT_CHAR('7') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x2b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("- (PAD)") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR('-') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x2c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("* (PAD)") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR('*') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x2d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/ (PAD)") PORT_CODE(KEYCODE_SLASH_PAD) PORT_CHAR('/') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x2e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DEC TAB") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x2f)

	PORT_START("KEY6") // 0x30 - 0x37
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("LSHIFT") PORT_CODE(KEYCODE_LSHIFT) PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x30)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("(H6)") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x31)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x32)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x33)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x34)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x35)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x36)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x37)

	/* TODO: check 0x3a - 0x3b */
	PORT_START("KEY7") // 0x38 - 0x3f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x38)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x39)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(",") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x3a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(".") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x3b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) /*PORT_CHAR('4')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x3c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) /*PORT_CHAR(0xd)*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x3d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) /*PORT_CHAR('6')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x3e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) /*PORT_CHAR('7')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x3f)

	PORT_START("KEY8") // 0x40 - 0x47
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_UNUSED) //PORT_CHANGED(key_stroke, 0x40)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("TAB REL") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x41)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("SHIFT LOCK") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x42)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x43)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x44)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x45)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x46)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x47)

	PORT_START("KEY9") // 0x48 - 0x4f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x48)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x49)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x4a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x4b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(";") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x4c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9-6") /*PORT_CODE(KEYCODE_5) PORT_CHAR(0xd)*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x4d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(0x0d) PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x4e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("? /") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x4f)

	PORT_START("KEYA") // 0x50 - 0x57
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RCTRL") PORT_CODE(KEYCODE_RCONTROL) /*PORT_CHAR('1')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x50)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x51)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x52)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x53)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x54)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x55)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x56)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x57)

	PORT_START("KEYB") // 0x58 - 0x5f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x58)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x59)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x5a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1/2 1/4") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x5b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("< [") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x5c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("> ]") /*PORT_CODE(KEYCODE_5) PORT_CHAR(0xd)*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x5d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("INSERT") PORT_CODE(KEYCODE_INSERT) /*PORT_CHAR('6')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x5e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("WORD") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x5f)

	PORT_START("KEYC") // 0x60 - 0x67
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("GRPH SHIFT") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x60)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x61)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x62)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x63)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x64)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x65)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x66)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x67)

	PORT_START("KEYD") // 0x68 - 0x6f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x68)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x69)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("-") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x6a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("=") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x6b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_BACKSLASH) /*PORT_CHAR('4')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x6c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("BACKSPACE") PORT_CODE(KEYCODE_BACKSPACE) /*PORT_CHAR(0xd)*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x6d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D-7") /*PORT_CODE(KEYCODE_6) PORT_CHAR('6')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x6e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("LINE") /*PORT_CODE(KEYCODE_7) PORT_CHAR('7')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x6f)

	PORT_START("KEYE") // 0x70 - 0x77
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("LCTRL") PORT_CODE(KEYCODE_LCONTROL) /*PORT_CHAR('1')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x70)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("COPY DISK") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x71)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("HELP") /*PORT_CODE(KEYCODE_2) PORT_CHAR('2')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x72)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("STOP") /*PORT_CODE(KEYCODE_3) PORT_CHAR('3')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x73)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("MAR SEL") /*PORT_CODE(KEYCODE_4) PORT_CHAR('4')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x74)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^") /*PORT_CODE(KEYCODE_5) PORT_CHAR(0xd)*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x75)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x76)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) /*PORT_CHAR('7')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x77)

	PORT_START("KEYF") // 0x78 - 0x7f
	PORT_BIT(0x01,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("TAB SET") /*PORT_CODE(KEYCODE_1) PORT_CHAR('1')*/ PORT_IMPULSE(1) PORT_CHANGED(key_stroke, 0x78)
	PORT_BIT(0x02,IP_ACTIVE_HIGH,IPT_UNUSED) //PORT_CHANGED(key_stroke, 0x79)
	PORT_BIT(0x04,IP_ACTIVE_HIGH,IPT_UNUSED) //PORT_CHANGED(key_stroke, 0x7a)
	PORT_BIT(0x08,IP_ACTIVE_HIGH,IPT_UNUSED) //PORT_CHANGED(key_stroke, 0x7b)
	PORT_BIT(0x10,IP_ACTIVE_HIGH,IPT_UNUSED) //PORT_CHANGED(key_stroke, 0x7c)
	PORT_BIT(0x20,IP_ACTIVE_HIGH,IPT_UNUSED) //PORT_CHANGED(key_stroke, 0x7d)
	PORT_BIT(0x40,IP_ACTIVE_HIGH,IPT_UNUSED) //PORT_CHANGED(key_stroke, 0x7e)
	PORT_BIT(0x80,IP_ACTIVE_HIGH,IPT_UNUSED) //PORT_CHANGED(key_stroke, 0x7f)

	/* TODO: All of those have unknown meaning */
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "DSW" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) ) //CMOS related
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

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x03, 0x02, "Video Board" )
	PORT_CONFSETTING( 0x02, "Monochrome" )
	PORT_CONFSETTING( 0x01, "Color" )
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


void qx10_state::machine_start()
{
	machine().device("maincpu")->execute().set_irq_acknowledge_callback(irq_callback);
}

void qx10_state::machine_reset()
{
	i8237_dreq0_w(m_dma_1, 1);

	m_memprom = 0;
	m_memcmos = 0;
	m_membank = 0;
	update_memory_mapping();

	{
		int i;

		/* TODO: is there a bit that sets this up? */
		m_color_mode = ioport("CONFIG")->read() & 1;

		if(m_color_mode) //color
		{
			for ( i = 0; i < 8; i++ )
				palette_set_color_rgb(machine(), i, pal1bit((i >> 2) & 1), pal1bit((i >> 1) & 1), pal1bit((i >> 0) & 1));
		}
		else //monochrome
		{
			for ( i = 0; i < 8; i++ )
				palette_set_color_rgb(machine(), i, pal1bit(0), pal1bit(0), pal1bit(0));

			palette_set_color_rgb(machine(), 1, 0x00, 0x9f, 0x00);
			palette_set_color_rgb(machine(), 2, 0x00, 0xff, 0x00);
			m_vram_bank = 0;
		}
	}
}

/* F4 Character Displayer */
static const gfx_layout qx10_charlayout =
{
	8, 16,					/* 8 x 16 characters */
	RGN_FRAC(1,1),			/* 128 characters */
	1,					/* 1 bits per pixel */
	{ 0 },					/* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8, 8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16					/* every char takes 16 bytes */
};

static GFXDECODE_START( qx10 )
	GFXDECODE_ENTRY( "chargen", 0x0000, qx10_charlayout, 1, 1 )
GFXDECODE_END

void qx10_state::video_start()
{
	// allocate memory
	//m_video_ram = auto_alloc_array_clear(machine(), UINT8, 0x60000);

	// find memory regions
	m_char_rom = memregion("chargen")->base();
}

static UPD7220_INTERFACE( hgdc_intf )
{
	"screen",
	hgdc_display_pixels,
	hgdc_draw_text,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

void qx10_state::palette_init()
{
	// ...
}

READ8_MEMBER( qx10_state::vram_r )
{
	int bank = 0;

	if (m_vram_bank & 1)	 { bank = 0; } // B
	else if(m_vram_bank & 2) { bank = 1; } // G
	else if(m_vram_bank & 4) { bank = 2; } // R

	return m_video_ram[offset + (0x20000 * bank)];
}

WRITE8_MEMBER( qx10_state::vram_w )
{
	int bank = 0;

	if (m_vram_bank & 1)	 { bank = 0; } // B
	else if(m_vram_bank & 2) { bank = 1; } // G
	else if(m_vram_bank & 4) { bank = 2; } // R

	m_video_ram[offset + (0x20000 * bank)] = data;
}

static ADDRESS_MAP_START( upd7220_map, AS_0, 8, qx10_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1ffff)
	AM_RANGE(0x00000, 0x1ffff) AM_RAM AM_SHARE("video_ram")
ADDRESS_MAP_END


static MACHINE_CONFIG_START( qx10, qx10_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, MAIN_CLK / 4)
	MCFG_CPU_PROGRAM_MAP(qx10_mem)
	MCFG_CPU_IO_MAP(qx10_io)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DEVICE("upd7220", upd7220_device, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_GFXDECODE(qx10)
	MCFG_PALETTE_LENGTH(8)

	/* Devices */
	MCFG_PIT8253_ADD("pit8253_1", qx10_pit8253_1_config)
	MCFG_PIT8253_ADD("pit8253_2", qx10_pit8253_2_config)
	MCFG_PIC8259_ADD("pic8259_master", qx10_pic8259_master_config)
	MCFG_PIC8259_ADD("pic8259_slave", qx10_pic8259_slave_config)
	MCFG_UPD7201_ADD("upd7201", MAIN_CLK/4, qx10_upd7201_interface)
	MCFG_I8255_ADD("i8255", qx10_i8255_interface)
	MCFG_I8237_ADD("8237dma_1", MAIN_CLK/4, qx10_dma8237_1_interface)
	MCFG_I8237_ADD("8237dma_2", MAIN_CLK/4, qx10_dma8237_2_interface)
	MCFG_UPD7220_ADD("upd7220", MAIN_CLK/4, hgdc_intf, upd7220_map)
	MCFG_MC146818_ADD( "rtc", MC146818_STANDARD )
	MCFG_UPD765A_ADD("upd765", qx10_upd765_interface)
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(qx10_floppy_interface)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("256K")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( qx10 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v006", "v0.06")
	ROMX_LOAD( "ipl006.bin", 0x0000, 0x0800, CRC(3155056a) SHA1(67cc0ae5055d472aa42eb40cddff6da69ffc6553), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v003", "v0.03")
	ROMX_LOAD( "ipl003.bin", 0x0000, 0x0800, CRC(3cbc4008) SHA1(cc8c7d1aa0cca8f9753d40698b2dc6802fd5f890), ROM_BIOS(2))

	/* This is probably the i8039 program ROM for the Q10MF Multifont card, and the actual font ROMs are missing (6 * HM43128) */
	/* The first part of this rom looks like code for an embedded controller?
        From 0300 on, is a keyboard lookup table */
	ROM_REGION( 0x0800, "i8039", 0 )
	ROM_LOAD( "m12020a.3e", 0x0000, 0x0800, CRC(fa27f333) SHA1(73d27084ca7b002d5f370220d8da6623a6e82132))

	ROM_REGION( 0x1000, "chargen", 0 )
//  ROM_LOAD( "qge.2e",   0x0000, 0x0800, BAD_DUMP CRC(ed93cb81) SHA1(579e68bde3f4184ded7d89b72c6936824f48d10b))  //this one contains special characters only
	ROM_LOAD( "qge.2e",   0x0000, 0x1000, BAD_DUMP CRC(eb31a2d5) SHA1(6dc581bf2854a07ae93b23b6dfc9c7abd3c0569e))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY   FULLNAME       FLAGS */
COMP( 1983, qx10,  0,       0,	qx10,	qx10, driver_device,	 0,		  "Epson",   "QX-10",		GAME_NOT_WORKING | GAME_NO_SOUND )
