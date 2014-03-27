/***************************************************************************

    machine/pc.c

    Functions to emulate general aspects of the machine
    (RAM, ROM, interrupts, I/O ports)

    The information herein is heavily based on
    'Ralph Browns Interrupt List'
    Release 52, Last Change 20oct96

***************************************************************************/

#include "includes/pc.h"
#include "bus/rs232/rs232.h"
#include "cpu/i86/i86.h"

#include "machine/i8255.h"
#include "machine/ins8250.h"
#include "machine/mc146818.h"
#include "machine/pic8259.h"

#include "video/pc_t1t.h"

#include "machine/pit8253.h"

#include "machine/pckeybrd.h"
#include "machine/pc_lpt.h"
#include "machine/pc_fdc.h"
#include "machine/upd765.h"
#include "includes/amstr_pc.h"
#include "imagedev/cassette.h"
#include "sound/speaker.h"

#include "machine/am9517a.h"
#include "machine/wd_fdc.h"

#include "machine/ram.h"

#include "coreutil.h"

#define VERBOSE_PIO 0   /* PIO (keyboard controller) */

#define PIO_LOG(N,M,A) \
	do { \
		if(VERBOSE_PIO>=N) \
		{ \
			if( M ) \
				logerror("%11.6f: %-24s",machine().time().as_double(),(char*)M ); \
			logerror A; \
		} \
	} while (0)

#define VERBOSE_DBG 0       /* general debug messages */

#define DBG_LOG(N,M,A) \
	do { \
	if(VERBOSE_DBG>=N) \
		{ \
			if( M ) \
				logerror("%11.6f at %s: %-24s",machine().time().as_double(),machine().describe_context(),(char*)M ); \
			logerror A; \
		} \
	} while (0)

/*************************************************************************
 *
 *      PC DMA stuff
 *
 *************************************************************************/

READ8_MEMBER(pc_state::pc_page_r)
{
	return 0xFF;
}


WRITE8_MEMBER(pc_state::pc_page_w)
{
	switch(offset % 4)
	{
	case 1:
		m_dma_offset[0][2] = data;
		break;
	case 2:
		m_dma_offset[0][3] = data;
		break;
	case 3:
		m_dma_offset[0][0] = m_dma_offset[0][1] = data;
		break;
	}
}


WRITE_LINE_MEMBER(pc_state::pc_dma_hrq_changed)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237->hack_w(state);
}


READ8_MEMBER(pc_state::pc_dma_read_byte)
{
	if(m_dma_channel == -1)
		return 0xff;
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0x0F0000;

	return prog_space.read_byte( page_offset + offset);
}


WRITE8_MEMBER(pc_state::pc_dma_write_byte)
{
	if(m_dma_channel == -1)
		return;
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0x0F0000;

	prog_space.write_byte( page_offset + offset, data);
}


READ8_MEMBER(pc_state::pc_dma8237_fdc_dack_r)
{
	return machine().device<pc_fdc_interface>("fdc")->dma_r();
}


READ8_MEMBER(pc_state::pc_dma8237_hdc_dack_r)
{
	return 0xff;
}


WRITE8_MEMBER(pc_state::pc_dma8237_fdc_dack_w)
{
	machine().device<pc_fdc_interface>("fdc")->dma_w(data);
}


WRITE8_MEMBER(pc_state::pc_dma8237_hdc_dack_w)
{
}


WRITE8_MEMBER(pc_state::pc_dma8237_0_dack_w)
{
	m_u73_q2 = 0;
	m_dma8237->dreq0_w( m_u73_q2 );
}

void pc_state::pc_eop_w(int channel, bool state)
{
	switch(channel)
	{
		case 2:
			machine().device<pc_fdc_interface>("fdc")->tc_w(state);
			break;
		case 0:
		case 1:
		case 3:
		default:
			break;
	}
}


WRITE_LINE_MEMBER(pc_state::pc_dma8237_out_eop)
{
	m_cur_eop = state == ASSERT_LINE;
	if(m_dma_channel != -1 && m_cur_eop)
		pc_eop_w(m_dma_channel, m_cur_eop ? ASSERT_LINE : CLEAR_LINE);
}

void pc_state::pc_select_dma_channel(int channel, bool state)
{
	if(!state) {
		m_dma_channel = channel;
		if(m_cur_eop)
			pc_eop_w(channel, ASSERT_LINE );

	} else if(m_dma_channel == channel) {
		m_dma_channel = -1;
		if(m_cur_eop)
			pc_eop_w(channel, CLEAR_LINE );
	}
}

WRITE_LINE_MEMBER(pc_state::pc_dack0_w){ pc_select_dma_channel(0, state); }
WRITE_LINE_MEMBER(pc_state::pc_dack1_w){ pc_select_dma_channel(1, state); }
WRITE_LINE_MEMBER(pc_state::pc_dack2_w){ pc_select_dma_channel(2, state); }
WRITE_LINE_MEMBER(pc_state::pc_dack3_w){ pc_select_dma_channel(3, state); }

I8237_INTERFACE( ibm5150_dma8237_config )
{
	DEVCB_DRIVER_LINE_MEMBER(pc_state,pc_dma_hrq_changed),
	DEVCB_DRIVER_LINE_MEMBER(pc_state,pc_dma8237_out_eop),
	DEVCB_DRIVER_MEMBER(pc_state, pc_dma_read_byte),
	DEVCB_DRIVER_MEMBER(pc_state, pc_dma_write_byte),
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_DRIVER_MEMBER(pc_state,pc_dma8237_fdc_dack_r), DEVCB_DRIVER_MEMBER(pc_state, pc_dma8237_hdc_dack_r) },
	{ DEVCB_DRIVER_MEMBER(pc_state, pc_dma8237_0_dack_w), DEVCB_NULL, DEVCB_DRIVER_MEMBER(pc_state,pc_dma8237_fdc_dack_w), DEVCB_DRIVER_MEMBER(pc_state, pc_dma8237_hdc_dack_w) },
	{ DEVCB_DRIVER_LINE_MEMBER(pc_state,pc_dack0_w), DEVCB_DRIVER_LINE_MEMBER(pc_state, pc_dack1_w), DEVCB_DRIVER_LINE_MEMBER(pc_state, pc_dack2_w), DEVCB_DRIVER_LINE_MEMBER(pc_state, pc_dack3_w) }
};




/*************************************************************************
 *
 *      PC Speaker related
 *
 *************************************************************************/
UINT8 pc_state::pc_speaker_get_spk()
{
	return m_pc_spkrdata & m_pit_out2;
}


void pc_state::pc_speaker_set_spkrdata(UINT8 data)
{
	m_pc_spkrdata = data ? 1 : 0;
	m_speaker->level_w(pc_speaker_get_spk());
}


/*************************************************************
 *
 * pit8253 configuration
 *
 *************************************************************/

WRITE_LINE_MEMBER(pc_state::ibm5150_pit8253_out1_changed)
{
	/* Trigger DMA channel #0 */
	if ( m_out1 == 0 && state == 1 && m_u73_q2 == 0 )
	{
		m_u73_q2 = 1;
		m_dma8237->dreq0_w( m_u73_q2 );
	}
	m_out1 = state;
}


WRITE_LINE_MEMBER(pc_state::ibm5150_pit8253_out2_changed)
{
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(pc_speaker_get_spk());
}


/**********************************************************
 *
 * COM hardware
 *
 **********************************************************/

/* called when a interrupt is set/cleared from com hardware */
WRITE_LINE_MEMBER(pc_state::pc_com_interrupt_1)
{
	m_pic8259->ir4_w(state);
}

WRITE_LINE_MEMBER(pc_state::pc_com_interrupt_2)
{
	m_pic8259->ir3_w(state);
}

const ins8250_interface ibm5150_com_interface[4]=
{
	{
		DEVCB_DEVICE_LINE_MEMBER("serport0", rs232_port_device, write_txd),
		DEVCB_DEVICE_LINE_MEMBER("serport0", rs232_port_device, write_dtr),
		DEVCB_DEVICE_LINE_MEMBER("serport0", rs232_port_device, write_rts),
		DEVCB_DRIVER_LINE_MEMBER(pc_state,pc_com_interrupt_1),
		DEVCB_NULL,
		DEVCB_NULL
	},
	{
		DEVCB_DEVICE_LINE_MEMBER("serport1", rs232_port_device, write_txd),
		DEVCB_DEVICE_LINE_MEMBER("serport1", rs232_port_device, write_dtr),
		DEVCB_DEVICE_LINE_MEMBER("serport1", rs232_port_device, write_rts),
		DEVCB_DRIVER_LINE_MEMBER(pc_state,pc_com_interrupt_2),
		DEVCB_NULL,
		DEVCB_NULL
	},
	{
		DEVCB_DEVICE_LINE_MEMBER("serport2", rs232_port_device, write_txd),
		DEVCB_DEVICE_LINE_MEMBER("serport2", rs232_port_device, write_dtr),
		DEVCB_DEVICE_LINE_MEMBER("serport2", rs232_port_device, write_rts),
		DEVCB_DRIVER_LINE_MEMBER(pc_state,pc_com_interrupt_1),
		DEVCB_NULL,
		DEVCB_NULL
	},
	{
		DEVCB_DEVICE_LINE_MEMBER("serport3", rs232_port_device, write_txd),
		DEVCB_DEVICE_LINE_MEMBER("serport3", rs232_port_device, write_dtr),
		DEVCB_DEVICE_LINE_MEMBER("serport3", rs232_port_device, write_rts),
		DEVCB_DRIVER_LINE_MEMBER(pc_state,pc_com_interrupt_2),
		DEVCB_NULL,
		DEVCB_NULL
	}
};

/**********************************************************
 *
 * NMI handling
 *
 **********************************************************/

static UINT8    nmi_enabled;

WRITE8_MEMBER(pc_state::pc_nmi_enable_w)
{
	//logerror( "%08X: changing NMI state to %s\n", space.device().safe_pc(), data & 0x80 ? "enabled" : "disabled" ); // this is clogging up the log

	nmi_enabled = data & 0x80;
}

/**********************************************************
 *
 * PPI8255 interface
 *
 *
 * PORT A (input)
 *
 * Directly attached to shift register which stores data
 * received from the keyboard.
 *
 * PORT B (output)
 * 0 - PB0 - TIM2GATESPK - Enable/disable counting on timer 2 of the 8253
 * 1 - PB1 - SPKRDATA    - Speaker data
 * 2 - PB2 -             - Enable receiving data from the keyboard when keyboard is not locked.
 * 3 - PB3 -             - Dipsswitch set selector
 * 4 - PB4 - ENBRAMPCK   - Enable ram parity check
 * 5 - PB5 - ENABLEI/OCK - Enable expansion I/O check
 * 6 - PB6 -             - Connected to keyboard clock signal
 *                         0 = ignore keyboard signals
 *                         1 = accept keyboard signals
 * 7 - PB7 -             - Clear/disable shift register and IRQ1 line
 *                         0 = normal operation
 *                         1 = clear and disable shift register and clear IRQ1 flip flop
 *
 * PORT C
 * 0 - PC0 -         - Dipswitch 0/4 SW1
 * 1 - PC1 -         - Dipswitch 1/5 SW1
 * 2 - PC2 -         - Dipswitch 2/6 SW1
 * 3 - PC3 -         - Dipswitch 3/7 SW1
 * 4 - PC4 - SPK     - Speaker/cassette data
 * 5 - PC5 - I/OCHCK - Expansion I/O check result
 * 6 - PC6 - T/C2OUT - Output of 8253 timer 2
 * 7 - PC7 - PCK     - Parity check result
 *
 * IBM5150 SW1:
 * 0   - OFF - One or more floppy drives
 *       ON  - Diskless operation
 * 1   - OFF - 8087 present
 *       ON  - No 8087 present
 * 2+3 - Used to determine on board memory configuration
 *       OFF OFF - 64KB
 *       ON  OFF - 48KB
 *       OFF ON  - 32KB
 *       ON  ON  - 16KB
 * 4+5 - Used to select display
 *       OFF OFF - Monochrome
 *       ON  OFF - CGA, 80 column
 *       OFF ON  - CGA, 40 column
 *       ON  ON  - EGA/VGA display
 * 6+7 - Used to select number of disk drives
 *       OFF OFF - four disk drives
 *       ON  OFF - three disk drives
 *       OFF ON  - two disk drives
 *       ON  ON  - one disk drive
 *
 **********************************************************/

WRITE_LINE_MEMBER( pc_state::keyboard_clock_w )
{
	if ( m_ppi_clock_signal != state )
	{
		if ( m_ppi_keyb_clock && m_ppi_shift_enable )
		{
			m_ppi_clock_signal = state;
			if ( ! m_ppi_keyboard_clear )
			{
				/* Data is clocked in on a high->low transition */
				if ( ! state )
				{
					UINT8   trigger_irq = m_ppi_shift_register & 0x01;

					m_ppi_shift_register = ( m_ppi_shift_register >> 1 ) | ( m_ppi_data_signal << 7 );
					if ( trigger_irq )
					{
						m_pic8259->ir1_w(1);
						m_ppi_shift_enable = 0;
						m_ppi_clock_signal = 0;
						m_pc_kbdc->clock_write_from_mb(m_ppi_clock_signal);
					}
				}
			}
		}
	}
}


WRITE_LINE_MEMBER( pc_state::keyboard_data_w )
{
	m_ppi_data_signal = state;
}


READ8_MEMBER(pc_state::ibm5160_ppi_porta_r)
{
	int data = 0xFF;
	/* KB port A */
	if (m_ppi_keyboard_clear)
	{
		/*   0  0 - no floppy drives
		 *   1  Not used
		 * 2-3  The number of memory banks on the system board
		 * 4-5  Display mode
		 *      11 = monochrome
		 *      10 - color 80x25
		 *      01 - color 40x25
		 * 6-7  The number of floppy disk drives
		 */
		data = ioport("DSW0")->read();
	}
	else
	{
		data = m_ppi_shift_register;
	}
	PIO_LOG(1,"PIO_A_r",("$%02x\n", data));
	return data;
}


READ8_MEMBER(pc_state::ibm5160_ppi_portc_r)
{
	int data=0xff;

	data&=~0x80; // no parity error
	data&=~0x40; // no error on expansion board
	/* KB port C: equipment flags */
//  if (pc_port[0x61] & 0x08)
	if (m_ppi_portc_switch_high)
	{
		/* read hi nibble of S2 */
		data = (data & 0xf0) | ((ioport("DSW0")->read() >> 4) & 0x0f);
		PIO_LOG(1,"PIO_C_r (hi)",("$%02x\n", data));
	}
	else
	{
		/* read lo nibble of S2 */
		data = (data & 0xf0) | (ioport("DSW0")->read() & 0x0f);
		PIO_LOG(1,"PIO_C_r (lo)",("$%02x\n", data));
	}

	if ( m_ppi_portb & 0x01 )
	{
		data = ( data & ~0x10 ) | ( m_pit_out2 ? 0x10 : 0x00 );
	}
	data = ( data & ~0x20 ) | ( m_pit_out2 ? 0x20 : 0x00 );

	return data;
}


WRITE8_MEMBER(pc_state::ibm5160_ppi_portb_w)
{
	/* PPI controller port B*/
	m_ppi_portb = data;
	m_ppi_portc_switch_high = data & 0x08;
	m_ppi_keyboard_clear = data & 0x80;
	m_ppi_keyb_clock = data & 0x40;
	m_pit8253->write_gate2(BIT(data, 0));
	pc_speaker_set_spkrdata( data & 0x02 );

	m_ppi_clock_signal = ( m_ppi_keyb_clock ) ? 1 : 0;
	m_pc_kbdc->clock_write_from_mb(m_ppi_clock_signal);

	/* If PB7 is set clear the shift register and reset the IRQ line */
	if ( m_ppi_keyboard_clear )
	{
		m_pic8259->ir1_w(0);
		m_ppi_shift_register = 0;
		m_ppi_shift_enable = 1;
	}
}


I8255_INTERFACE( ibm5160_ppi8255_interface )
{
	DEVCB_DRIVER_MEMBER(pc_state,ibm5160_ppi_porta_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(pc_state,ibm5160_ppi_portb_w),
	DEVCB_DRIVER_MEMBER(pc_state,ibm5160_ppi_portc_r),
	DEVCB_NULL
};


READ8_MEMBER(pc_state::pc_ppi_porta_r)
{
	int data = 0xFF;

	/* KB port A */
	if (m_ppi_keyboard_clear)
	{
		/*   0  0 - no floppy drives
		 *   1  Not used
		 * 2-3  The number of memory banks on the system board
		 * 4-5  Display mode
		 *      11 = monochrome
		 *      10 - color 80x25
		 *      01 - color 40x25
		 * 6-7  The number of floppy disk drives
		 */
		data = ioport("DSW0")->read();
	}
	else
	{
		data = pc_keyb_read();
	}
	PIO_LOG(1,"PIO_A_r",("$%02x\n", data));
	return data;
}


WRITE8_MEMBER(pc_state::pc_ppi_portb_w)
{
	/* PPI controller port B*/
	m_ppi_portb = data;
	m_ppi_portc_switch_high = data & 0x08;
	m_ppi_keyboard_clear = data & 0x80;
	m_ppi_keyb_clock = data & 0x40;
	m_pit8253->write_gate2(BIT(data, 0));
	pc_speaker_set_spkrdata( data & 0x02 );
	pc_keyb_set_clock( m_ppi_keyb_clock );

	if ( m_ppi_keyboard_clear )
		pc_keyb_clear();
}


I8255_INTERFACE( pc_ppi8255_interface )
{
	DEVCB_DRIVER_MEMBER(pc_state,pc_ppi_porta_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(pc_state,pc_ppi_portb_w),
	DEVCB_DRIVER_MEMBER(pc_state,ibm5160_ppi_portc_r),
	DEVCB_NULL
};




/**********************************************************
 *
 * NEC uPD765 floppy interface
 *
 **********************************************************/

WRITE_LINE_MEMBER( pc_state::fdc_interrupt )
{
	if (m_pic8259)
	{
		m_pic8259->ir6_w(state);
	}
}

WRITE_LINE_MEMBER( pc_state::pc_set_keyb_int)
{
	m_pic8259->ir1_w(state);
}

WRITE8_MEMBER(pc_state::asst128_fdc_dor_w)
{
	pc_fdc_xt_device *fdc = machine().device<pc_fdc_xt_device>("fdc");

	fdc->tc_w((data & 0x80) == 0x80);
	fdc->dor_w(space, offset, data, mem_mask);
}

/**********************************************************
 *
 * Initialization code
 *
 **********************************************************/
/*
   keyboard seams to permanently sent data clocked by the mainboard
   clock line low for longer means "resync", keyboard sends 0xaa as answer
   will become automatically 0x00 after a while
*/

UINT8 pc_state::pc_keyb_read()
{
	return m_pc_keyb_data;
}

TIMER_CALLBACK_MEMBER( pc_state::pc_keyb_timer )
{
	if ( m_pc_keyb_on ) {
		pc_keyboard();
	} else {
		/* Clock has been low for more than 5 msec, start diagnostic test */
		at_keyboard_reset(machine());
		m_pc_keyb_self_test = 1;
	}
}

void pc_state::pc_keyb_set_clock(int on)
{
	on = on ? 1 : 0;

	if (m_pc_keyb_on != on)
	{
		if (!on)
			m_pc_keyb_timer->adjust(attotime::from_msec(5));
		else {
			if ( m_pc_keyb_self_test ) {
				/* The self test of the keyboard takes some time. 2 msec seems to work. */
				/* This still needs to verified against a real keyboard. */
				m_pc_keyb_timer->adjust(attotime::from_msec( 2 ));
			} else {
				m_pc_keyb_timer->reset();
				m_pc_keyb_self_test = 0;
			}
		}

		m_pc_keyb_on = on;
	}
}

void pc_state::pc_keyb_clear(void)
{
	m_pc_keyb_data = 0;
	if (!m_pc_keyb_int_cb.isnull()) {
		m_pc_keyb_int_cb(0);
	}
}

void pc_state::pc_keyboard(void)
{
	int data;

	at_keyboard_polling();

	if (m_pc_keyb_on)
	{
		if ( (data=at_keyboard_read())!=-1) {
			m_pc_keyb_data = data;
			//DBG_LOG(1,"KB_scancode",("$%02x\n", m_pc_keyb_data));
			if ( !m_pc_keyb_int_cb.isnull() ) {
				m_pc_keyb_int_cb(1);
			}
			m_pc_keyb_self_test = 0;
		}
	}
}

void pc_state::init_pc_common(write_line_delegate set_keyb_int_func)
{
	m_pc_keyb_int_cb = set_keyb_int_func;
	if (!set_keyb_int_func.isnull()) {
		at_keyboard_init(machine(), AT_KEYBOARD_TYPE_PC);
		at_keyboard_set_scan_code_set(1);
		m_pc_keyb_int_cb = set_keyb_int_func;
		m_pc_keyb_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pc_state::pc_keyb_timer),this));
	}

	/* MESS managed RAM */
	if ( m_ram->pointer() )
		membank( "bank10" )->set_base( m_ram->pointer() );
}


DRIVER_INIT_MEMBER(pc_state,ibm5150)
{
	init_pc_common(write_line_delegate());
	pc_rtc_init();
}


DRIVER_INIT_MEMBER(pc_state,pccga)
{
	init_pc_common(write_line_delegate());
	pc_rtc_init();
}


DRIVER_INIT_MEMBER(pc_state,bondwell)
{
	init_pc_common(write_line_delegate());
	pc_turbo_setup(4.77/12, 1);
}

DRIVER_INIT_MEMBER(pc_state,pcmda)
{
	init_pc_common(write_line_delegate(FUNC(pc_state::pc_set_keyb_int),this));
}

IRQ_CALLBACK_MEMBER(pc_state::pc_irq_callback)
{
	return m_pic8259->acknowledge();
}


MACHINE_START_MEMBER(pc_state,pc)
{
	m_maincpu->set_irq_acknowledge_callback(device_irq_acknowledge_delegate(FUNC(pc_state::pc_irq_callback),this));
}


MACHINE_RESET_MEMBER(pc_state,pc)
{
	m_u73_q2 = 0;
	m_out1 = 0;
	m_pc_spkrdata = 0;
	m_pit_out2 = 1;
	m_dma_channel = -1;
	m_cur_eop = 0;
	memset(m_dma_offset,0,sizeof(m_dma_offset));
	m_ppi_portc_switch_high = 0;
	m_ppi_speaker = 0;
	m_ppi_keyboard_clear = 0;
	m_ppi_keyb_clock = 0;
	m_ppi_portb = 0;
	m_ppi_clock_signal = 0;
	m_ppi_data_signal = 0;
	m_ppi_shift_register = 0;
	m_ppi_shift_enable = 0;

	m_speaker->level_w(0);
}


/**************************************************************************
 *
 *      Interrupt handlers.
 *
 **************************************************************************/

TIMER_DEVICE_CALLBACK_MEMBER(pc_state::pc_frame_interrupt)
{
	int scanline = param;

	if((scanline % 64) == 0)
		pc_keyboard();
}

/*
ibm xt bios
-----------

fe0ac
fe10c
fe12b: hangs after reset
fe15e
fe19c
fe2c6
 graphics adapter sync signals
fe332
 search roms
fe35d
 pic test
fe38f
fe3c6
fe3e6
 301 written
 expect 0xaa after reset, send when lines activated in short time
 (keyboard polling in frame interrupt is not quick enough now)
fe424
fe448
 i/o expansion test
fe49c
 memory test
fe500
 harddisk bios used
fe55c
 disk booting

 f0bef
  f0b85
   f096d
    disk related
    feca0
     prueft kanal 0 address register (memory refresh!?)

ibm pc bios
-----------
fe104
fe165
fe1b4
fe205
fe23f
fe256
fe363
fe382 expansion test
fe3c4
 memory test
fe43b
fe443 call fe643 keyboard test
fe4a1 call ff979 tape!!! test
*/


// damned old checkit doesn't test at standard addresses
// will do more when I have a program supporting it
static struct {
	int data[0x18];
	emu_timer *timer;
} pc_rtc;

TIMER_CALLBACK_MEMBER(pc_state::pc_rtc_timer)
{
	int year;
	if (++pc_rtc.data[2]>=60) {
		pc_rtc.data[2]=0;
		if (++pc_rtc.data[3]>=60) {
			pc_rtc.data[3]=0;
			if (++pc_rtc.data[4]>=24) {
				pc_rtc.data[4]=0;
				pc_rtc.data[5]=(pc_rtc.data[5]%7)+1;
				year=pc_rtc.data[9]+2000;
				if (++pc_rtc.data[6]>=gregorian_days_in_month(pc_rtc.data[7], year)) {
					pc_rtc.data[6]=1;
					if (++pc_rtc.data[7]>12) {
						pc_rtc.data[7]=1;
						pc_rtc.data[9]=(pc_rtc.data[9]+1)%100;
					}
				}
			}
		}
	}
}

void pc_state::pc_rtc_init()
{
	memset(&pc_rtc,0,sizeof(pc_rtc));
	pc_rtc.timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(pc_state::pc_rtc_timer),this));
	pc_rtc.timer->adjust(attotime::zero, 0, attotime(1,0));
}

READ8_MEMBER(pc_state::pc_rtc_r)
{
	int data;
	switch (offset) {
	default:
		data=pc_rtc.data[offset];
	}
	logerror( "rtc read %.2x %.2x\n", offset, data);
	return data;
}

WRITE8_MEMBER(pc_state::pc_rtc_w)
{
	logerror( "rtc write %.2x %.2x\n", offset, data);
	switch(offset) {
	default:
		pc_rtc.data[offset]=data;
	}
}

/*************************************************************************
 *
 *      EXP
 *      expansion port
 *
 *************************************************************************/

// I even don't know what it is!
static struct {
	/*
	  reg 0 ram behaviour if in
	  reg 3 write 1 to enable it
	  reg 4 ram behaviour ???
	  reg 5,6 (5 hi, 6 lowbyte) ???
	*/
	/* selftest in ibmpc, ibmxt */
	UINT8 reg[8];
} pc_expansion={ { 0,0,0,0,0,0,1 } };

WRITE8_MEMBER(pc_state::pc_EXP_w)
{
	//DBG_LOG(1,"EXP_unit_w",("%.2x $%02x\n", offset, data));
	switch (offset) {
	case 4:
		pc_expansion.reg[4]=pc_expansion.reg[5]=pc_expansion.reg[6]=data;
		break;
	default:
		pc_expansion.reg[offset] = data;
	}
}

READ8_MEMBER(pc_state::pc_EXP_r)
{
	int data;
	UINT16 a;
	switch (offset) {
	case 6:
		data = pc_expansion.reg[offset];
		a=(pc_expansion.reg[5]<<8)|pc_expansion.reg[6];
		a<<=1;
		pc_expansion.reg[5]=a>>8;
		pc_expansion.reg[6]=a&0xff;
		break;
	default:
		data = pc_expansion.reg[offset];
	}
	//DBG_LOG(1,"EXP_unit_r",("%.2x $%02x\n", offset, data));
	return data;
}


TIMER_CALLBACK_MEMBER(pc_state::pc_turbo_callback)
{
	int val;

	val = ioport("DSW2")->read() & 0x02;

	if (val != m_turbo_cur_val)
	{
		m_turbo_cur_val = val;
		m_maincpu->set_clock_scale(val ? m_turbo_on_speed : m_turbo_off_speed);
	}
}

void pc_state::pc_turbo_setup(double off_speed, double on_speed)
{
	m_turbo_cur_val = -1;
	m_turbo_off_speed = off_speed;
	m_turbo_on_speed = on_speed;
	machine().scheduler().timer_pulse(attotime::from_msec(100), timer_expired_delegate(FUNC(pc_state::pc_turbo_callback),this));
}
