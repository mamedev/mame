/***************************************************************************

    machine/genpc.c


***************************************************************************/

#include "emu.h"
#include "includes/genpc.h"

#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "sound/speaker.h"
#include "machine/ram.h"
#include "imagedev/cassette.h"

#define VERBOSE_PIO 0	/* PIO (keyboard controller) */

#define PIO_LOG(N,M,A) \
	do { \
		if(VERBOSE_PIO>=N) \
		{ \
			if( M ) \
				logerror("%11.6f: %-24s",machine().time().as_double(),(char*)M ); \
			logerror A; \
		} \
	} while (0)


/*************************************************************************
 *
 *      PC DMA stuff
 *
 *************************************************************************/

READ8_DEVICE_HANDLER(pc_page_r)
{
	return 0xFF;
}


WRITE8_DEVICE_HANDLER(pc_page_w)
{
	ibm5160_mb_device *board  = downcast<ibm5160_mb_device *>(device);
	switch(offset % 4)
	{
	case 1:
		board->m_dma_offset[2] = data;
		break;
	case 2:
		board->m_dma_offset[3] = data;
		break;
	case 3:
		board->m_dma_offset[0] = board->m_dma_offset[1] = data;
		break;
	}
}


WRITE_LINE_MEMBER( ibm5160_mb_device::pc_dma_hrq_changed )
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237->hack_w(state);
}


READ8_MEMBER( ibm5160_mb_device::pc_dma_read_byte )
{
	if(m_dma_channel == -1)
		return 0xff;
	address_space &spaceio = m_maincpu->space(AS_PROGRAM);
	offs_t page_offset = (((offs_t) m_dma_offset[m_dma_channel]) << 16) & 0x0F0000;
	return spaceio.read_byte( page_offset + offset);
}


WRITE8_MEMBER( ibm5160_mb_device::pc_dma_write_byte )
{
	if(m_dma_channel == -1)
		return;
	address_space &spaceio = m_maincpu->space(AS_PROGRAM);
	offs_t page_offset = (((offs_t) m_dma_offset[m_dma_channel]) << 16) & 0x0F0000;

	spaceio.write_byte( page_offset + offset, data);
}


READ8_MEMBER( ibm5160_mb_device::pc_dma8237_1_dack_r )
{
	return m_isabus->dack_r(1);
}

READ8_MEMBER( ibm5160_mb_device::pc_dma8237_2_dack_r )
{
	return m_isabus->dack_r(2);
}


READ8_MEMBER( ibm5160_mb_device::pc_dma8237_3_dack_r )
{
	return m_isabus->dack_r(3);
}


WRITE8_MEMBER( ibm5160_mb_device::pc_dma8237_1_dack_w )
{
	m_isabus->dack_w(1,data);
}

WRITE8_MEMBER( ibm5160_mb_device::pc_dma8237_2_dack_w )
{
	m_isabus->dack_w(2,data);
}


WRITE8_MEMBER( ibm5160_mb_device::pc_dma8237_3_dack_w )
{
	m_isabus->dack_w(3,data);
}


WRITE8_MEMBER( ibm5160_mb_device::pc_dma8237_0_dack_w )
{
	m_u73_q2 = 0;
	m_dma8237->dreq0_w( m_u73_q2 );
}


WRITE_LINE_MEMBER( ibm5160_mb_device::pc_dma8237_out_eop )
{
	m_cur_eop = state == ASSERT_LINE;
	if(m_dma_channel != -1 && m_cur_eop)
		m_isabus->eop_w(m_dma_channel, m_cur_eop ? ASSERT_LINE : CLEAR_LINE );
}

void ibm5160_mb_device::pc_select_dma_channel(int channel, bool state)
{
	if(!state) {
		m_dma_channel = channel;
		if(m_cur_eop)
			m_isabus->eop_w(channel, ASSERT_LINE );

	} else if(m_dma_channel == channel) {
		m_dma_channel = -1;
		if(m_cur_eop)
			m_isabus->eop_w(channel, CLEAR_LINE );
	}
}

WRITE_LINE_MEMBER( ibm5160_mb_device::pc_dack0_w ) { pc_select_dma_channel(0, state); }
WRITE_LINE_MEMBER( ibm5160_mb_device::pc_dack1_w ) { pc_select_dma_channel(1, state); }
WRITE_LINE_MEMBER( ibm5160_mb_device::pc_dack2_w ) { pc_select_dma_channel(2, state); }
WRITE_LINE_MEMBER( ibm5160_mb_device::pc_dack3_w ) { pc_select_dma_channel(3, state); }

I8237_INTERFACE( pc_dma8237_config )
{
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_dma_hrq_changed),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_dma8237_out_eop),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_dma_read_byte),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_dma_write_byte),

	{ DEVCB_NULL,
	  DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_dma8237_1_dack_r),
	  DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_dma8237_2_dack_r),
	  DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_dma8237_3_dack_r) },


	{ DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_dma8237_0_dack_w),
	  DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_dma8237_1_dack_w),
	  DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_dma8237_2_dack_w),
	  DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_dma8237_3_dack_w) },

	// DACK's
	{ DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_dack0_w),
	  DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_dack1_w),
	  DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_dack2_w),
	  DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_dack3_w) }
};


/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/
WRITE_LINE_MEMBER(ibm5160_mb_device::pc_cpu_line)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state);
}

const struct pic8259_interface pc_pic8259_config =
{
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_cpu_line),
	DEVCB_LINE_VCC,
	DEVCB_NULL
};


WRITE_LINE_MEMBER(ibm5160_mb_device::pc_speaker_set_spkrdata)
{
	m_pc_spkrdata = state ? 1 : 0;
	speaker_level_w( m_speaker, m_pc_spkrdata & m_pc_input );
}


/*************************************************************
 *
 * pit8253 configuration
 *
 *************************************************************/

WRITE_LINE_MEMBER( ibm5160_mb_device::pc_pit8253_out1_changed )
{
	/* Trigger DMA channel #0 */
	if ( m_out1 == 0 && state == 1 && m_u73_q2 == 0 )
	{
		m_u73_q2 = 1;
		m_dma8237->dreq0_w( m_u73_q2 );
	}
	m_out1 = state;
}


WRITE_LINE_MEMBER( ibm5160_mb_device::pc_pit8253_out2_changed )
{
	m_pc_input = state ? 1 : 0;
	speaker_level_w( m_speaker, m_pc_spkrdata & m_pc_input );
}


const struct pit8253_config pc_pit8253_config =
{
	{
		{
			XTAL_14_31818MHz/12,				/* heartbeat IRQ */
			DEVCB_NULL,
			DEVCB_DEVICE_LINE("pic8259", pic8259_ir0_w)
		}, {
			XTAL_14_31818MHz/12,				/* dram refresh */
			DEVCB_NULL,
			DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_pit8253_out1_changed)
		}, {
			XTAL_14_31818MHz/12,				/* pio port c pin 4, and speaker polling enough */
			DEVCB_NULL,
			DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_pit8253_out2_changed)
		}
	}
};

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
WRITE_LINE_MEMBER( ibm5160_mb_device::keyboard_clock_w )
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
					UINT8	trigger_irq = m_ppi_shift_register & 0x01;

					m_ppi_shift_register = ( m_ppi_shift_register >> 1 ) | ( m_ppi_data_signal << 7 );
					if ( trigger_irq )
					{
						pic8259_ir1_w(m_pic8259, 1);
						m_ppi_shift_enable = 0;
						m_ppi_clock_signal = 0;
						m_pc_kbdc->clock_write_from_mb(m_ppi_clock_signal);
					}
				}
			}
		}
	}
}


WRITE_LINE_MEMBER( ibm5160_mb_device::keyboard_data_w )
{
	m_ppi_data_signal = state;
}

READ8_MEMBER (ibm5160_mb_device::pc_ppi_porta_r)
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


READ8_MEMBER ( ibm5160_mb_device::pc_ppi_portc_r )
{
	int timer2_output = pit8253_get_output( m_pit8253, 2 );
	int data=0xff;

	data&=~0x80; // no parity error
	data&=~0x40; // no error on expansion board
	/* KB port C: equipment flags */
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
		data = ( data & ~0x10 ) | ( timer2_output ? 0x10 : 0x00 );
	}
	data = ( data & ~0x20 ) | ( timer2_output ? 0x20 : 0x00 );

	return data;
}


WRITE8_MEMBER( ibm5160_mb_device::pc_ppi_portb_w )
{
	/* PPI controller port B*/
	m_ppi_portb = data;
	m_ppi_portc_switch_high = data & 0x08;
	m_ppi_keyboard_clear = data & 0x80;
	m_ppi_keyb_clock = data & 0x40;
	pit8253_gate2_w(m_pit8253, BIT(data, 0));
	pc_speaker_set_spkrdata( data & 0x02 );

	m_ppi_clock_signal = ( m_ppi_keyb_clock ) ? 1 : 0;
	m_pc_kbdc->clock_write_from_mb(m_ppi_clock_signal);

	/* If PB7 is set clear the shift register and reset the IRQ line */
	if ( m_ppi_keyboard_clear )
	{
		pic8259_ir1_w(m_pic8259, 0);
		m_ppi_shift_register = 0;
		m_ppi_shift_enable = 1;
	}
}


I8255A_INTERFACE( pc_ppi8255_interface )
{
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_ppi_porta_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_ppi_portb_w),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, pc_ppi_portc_r),
	DEVCB_NULL
};

static const isa8bus_interface isabus_intf =
{
	// interrupts
	DEVCB_DEVICE_LINE("pic8259", pic8259_ir2_w),
	DEVCB_DEVICE_LINE("pic8259", pic8259_ir3_w),
	DEVCB_DEVICE_LINE("pic8259", pic8259_ir4_w),
	DEVCB_DEVICE_LINE("pic8259", pic8259_ir5_w),
	DEVCB_DEVICE_LINE("pic8259", pic8259_ir6_w),
	DEVCB_DEVICE_LINE("pic8259", pic8259_ir7_w),

	// dma request
	DEVCB_DEVICE_LINE_MEMBER("dma8237", am9517a_device, dreq1_w),
	DEVCB_DEVICE_LINE_MEMBER("dma8237", am9517a_device, dreq2_w),
	DEVCB_DEVICE_LINE_MEMBER("dma8237", am9517a_device, dreq3_w)
};

static const pc_kbdc_interface pc_kbdc_intf =
{
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, keyboard_clock_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, ibm5160_mb_device, keyboard_data_w)
};

/**********************************************************
 *
 * NMI handling
 *
 **********************************************************/

static WRITE8_DEVICE_HANDLER( nmi_enable_w )
{
	ibm5160_mb_device *board  = downcast<ibm5160_mb_device *>(device);

	board->m_nmi_enabled = BIT(data,7);
	board->m_isabus->set_nmi_state(board->m_nmi_enabled);

}
//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type IBM5160_MOTHERBOARD = &device_creator<ibm5160_mb_device>;

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

static MACHINE_CONFIG_FRAGMENT( ibm5160_mb_config )
	MCFG_PIT8253_ADD( "pit8253", pc_pit8253_config )

	MCFG_I8237_ADD( "dma8237", XTAL_14_31818MHz/3, pc_dma8237_config )

	MCFG_PIC8259_ADD( "pic8259", pc_pic8259_config )

	MCFG_I8255A_ADD( "ppi8255", pc_ppi8255_interface )

	MCFG_ISA8_BUS_ADD("isa", "maincpu", isabus_intf)

	MCFG_PC_KBDC_ADD("pc_kbdc", pc_kbdc_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor ibm5160_mb_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ibm5160_mb_config );
}


static INPUT_PORTS_START( ibm5160_mb )
	PORT_START("DSW0") /* IN1 */
	PORT_DIPNAME( 0xc0, 0x40, "Number of floppy drives")
	PORT_DIPSETTING(	0x00, "1" )
	PORT_DIPSETTING(	0x40, "2" )
	PORT_DIPSETTING(	0x80, "3" )
	PORT_DIPSETTING(	0xc0, "4" )
	PORT_DIPNAME( 0x30, 0x30, "Graphics adapter")
	PORT_DIPSETTING(	0x00, "EGA/VGA" )
	PORT_DIPSETTING(	0x10, "Color 40x25" )
	PORT_DIPSETTING(	0x20, "Color 80x25" )
	PORT_DIPSETTING(	0x30, "Monochrome" )
	PORT_DIPNAME( 0x0c, 0x0c, "RAM banks")
	PORT_DIPSETTING(	0x00, "1 - 16/ 64/256K" )
	PORT_DIPSETTING(	0x04, "2 - 32/128/512K" )
	PORT_DIPSETTING(	0x08, "3 - 48/192/576K" )
	PORT_DIPSETTING(	0x0c, "4 - 64/256/640K" )
	PORT_DIPNAME( 0x02, 0x00, "8087 installed")
	PORT_DIPSETTING(	0x00, DEF_STR(No) )
	PORT_DIPSETTING(	0x02, DEF_STR(Yes) )
	PORT_DIPNAME( 0x01, 0x01, "Any floppy drive installed")
	PORT_DIPSETTING(	0x00, DEF_STR(No) )
	PORT_DIPSETTING(	0x01, DEF_STR(Yes) )
INPUT_PORTS_END
//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor ibm5160_mb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ibm5160_mb );
}


void ibm5160_mb_device::static_set_cputag(device_t &device, const char *tag)
{
	ibm5160_mb_device &board = downcast<ibm5160_mb_device &>(device);
	board.m_cputag = tag;
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ibm5160_mb_device - constructor
//-------------------------------------------------

ibm5160_mb_device::ibm5160_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
       : device_t(mconfig, IBM5160_MOTHERBOARD, "IBM5160_MOTHERBOARD", tag, owner, clock),
		m_maincpu(*owner, "maincpu"),
		m_pic8259(*this, "pic8259"),
		m_dma8237(*this, "dma8237"),
		m_pit8253(*this, "pit8253"),
		m_ppi8255(*this, "ppi8255"),
		m_speaker(*this, SPEAKER_TAG),
		m_isabus(*this, "isa"),
		m_pc_kbdc(*this, "pc_kbdc")
{
}

void ibm5160_mb_device::install_device(device_t *dev, offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_device_func rhandler, const char* rhandler_name, write8_device_func whandler, const char *whandler_name)
{
	int buswidth = machine().firstcpu->space_config(AS_IO)->m_databus_width;
	switch(buswidth)
	{
		case 8:
			m_maincpu->space(AS_IO).install_legacy_readwrite_handler(*dev, start, end, mask, mirror, rhandler, rhandler_name, whandler, whandler_name, 0);
			break;
		case 16:
			m_maincpu->space(AS_IO).install_legacy_readwrite_handler(*dev, start, end, mask, mirror, rhandler, rhandler_name, whandler, whandler_name,0xffff);
			break;
		default:
			fatalerror("IBM5160_MOTHERBOARD: Bus width %d not supported\n", buswidth);
			break;
	}
}

void ibm5160_mb_device::install_device_write(device_t *dev, offs_t start, offs_t end, offs_t mask, offs_t mirror, write8_device_func whandler, const char *whandler_name)
{
	int buswidth = machine().firstcpu->space_config(AS_IO)->m_databus_width;
	switch(buswidth)
	{
		case 8:
			m_maincpu->space(AS_IO).install_legacy_write_handler(*dev, start, end, mask, mirror, whandler, whandler_name,0);
			break;
		case 16:
			m_maincpu->space(AS_IO).install_legacy_write_handler(*dev, start, end, mask, mirror, whandler, whandler_name, 0xffff);
			break;
		default:
			fatalerror("IBM5160_MOTHERBOARD: Bus width %d not supported\n", buswidth);
			break;
	}
}

void ibm5160_mb_device::install_device(offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_delegate rhandler, write8_delegate whandler)
{
	int buswidth = m_maincpu->space_config(AS_IO)->m_databus_width;
	switch(buswidth)
	{
		case 8:
			m_maincpu->space(AS_IO).install_readwrite_handler(start, end, mask, mirror, rhandler, whandler, 0);
			break;
		case 16:
			m_maincpu->space(AS_IO).install_readwrite_handler(start, end, mask, mirror, rhandler, whandler, 0xffff);
			break;
		default:
			fatalerror("IBM5160_MOTHERBOARD: Bus width %d not supported\n", buswidth);
			break;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ibm5160_mb_device::device_start()
{
	install_device(0x0000, 0x000f, 0, 0, read8_delegate(FUNC(am9517a_device::read), (am9517a_device*)m_dma8237), write8_delegate(FUNC(am9517a_device::write), (am9517a_device*)m_dma8237) );
	install_device(m_pic8259, 0x0020, 0x0021, 0, 0, FUNC(pic8259_r), FUNC(pic8259_w) );
	install_device(m_pit8253, 0x0040, 0x0043, 0, 0, FUNC(pit8253_r), FUNC(pit8253_w) );

	//  install_device(m_ppi8255, 0x0060, 0x0063, 0, 0, FUNC(i8255a_r), FUNC(i8255a_w) );
	int buswidth = machine().firstcpu->space_config(AS_IO)->m_databus_width;
	switch(buswidth)
	{
		case 8:
			m_maincpu->space(AS_IO).install_readwrite_handler(0x0060, 0x0063, 0, 0, read8_delegate(FUNC(i8255_device::read), (i8255_device*)m_ppi8255), write8_delegate(FUNC(i8255_device::write), (i8255_device*)m_ppi8255), 0);
			break;
		case 16:
			m_maincpu->space(AS_IO).install_readwrite_handler(0x0060, 0x0063, 0, 0, read8_delegate(FUNC(i8255_device::read), (i8255_device*)m_ppi8255), write8_delegate(FUNC(i8255_device::write), (i8255_device*)m_ppi8255), 0xffff);
			break;
		default:
			fatalerror("IBM5160_MOTHERBOARD: Bus width %d not supported\n", buswidth);
			break;
	}

	install_device(this,    0x0080, 0x0087, 0, 0, FUNC(pc_page_r), FUNC(pc_page_w) );
	install_device_write(this,    0x00a0, 0x00a1, 0, 0, FUNC(nmi_enable_w));
	/* MESS managed RAM */
	if ( machine().device<ram_device>(RAM_TAG)->pointer() )
		membank( "bank10" )->set_base( machine().device<ram_device>(RAM_TAG)->pointer() );
}

IRQ_CALLBACK(ibm5160_mb_device::pc_irq_callback)
{
	device_t *pic = device->machine().device("mb:pic8259");
	return pic8259_acknowledge( pic );
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ibm5160_mb_device::device_reset()
{
	m_maincpu->set_irq_acknowledge_callback(pc_irq_callback);

	m_u73_q2 = 0;
	m_out1 = 2; // initial state of pit output is undefined
	m_pc_spkrdata = 0;
	m_pc_input = 0;
	m_dma_channel = -1;
	m_cur_eop = false;
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
	m_nmi_enabled = 0;
	speaker_level_w( m_speaker, 0 );
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type IBM5150_MOTHERBOARD = &device_creator<ibm5150_mb_device>;

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************
static const cassette_interface ibm5150_cassette_interface =
{
	cassette_default_formats,
	NULL,
	(cassette_state)(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED),
	NULL,
	NULL
};

static MACHINE_CONFIG_FRAGMENT( ibm5150_mb_config )
	MCFG_FRAGMENT_ADD(ibm5160_mb_config)

	MCFG_CASSETTE_ADD( CASSETTE_TAG, ibm5150_cassette_interface )
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor ibm5150_mb_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ibm5150_mb_config );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ibm5150_mb_device - constructor
//-------------------------------------------------

ibm5150_mb_device::ibm5150_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ibm5160_mb_device(mconfig, tag, owner, clock),
	  m_cassette(*this, CASSETTE_TAG)
{
}

void ibm5150_mb_device::device_start()
{
	ibm5160_mb_device::device_start();
}
void ibm5150_mb_device::device_reset()
{
	ibm5160_mb_device::device_reset();
}

READ8_MEMBER (ibm5150_mb_device::pc_ppi_porta_r)
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
		data = ioport("DSW0")->read() & 0xF3;
		switch ( machine().device<ram_device>(RAM_TAG)->size() )
		{
		case 16 * 1024:
			data |= 0x00;
			break;
		case 32 * 1024:	/* Need to verify if this is correct */
			data |= 0x04;
			break;
		case 48 * 1024:	/* Need to verify if this is correct */
			data |= 0x08;
			break;
		default:
			data |= 0x0C;
			break;
		}
	}
	else
	{
		data = m_ppi_shift_register;
	}
    PIO_LOG(1,"PIO_A_r",("$%02x\n", data));
    return data;
}


READ8_MEMBER ( ibm5150_mb_device::pc_ppi_portc_r )
{
	int timer2_output = pit8253_get_output( m_pit8253, 2 );
	int data=0xff;

	data&=~0x80; // no parity error
	data&=~0x40; // no error on expansion board
	/* KB port C: equipment flags */
	if (m_ppi_portc_switch_high)
	{
		/* read hi nibble of SW2 */
		data = data & 0xf0;

		switch ( machine().device<ram_device>(RAM_TAG)->size() - 64 * 1024 )
		{
		case 64 * 1024:		data |= 0x00; break;
		case 128 * 1024:	data |= 0x02; break;
		case 192 * 1024:	data |= 0x04; break;
		case 256 * 1024:	data |= 0x06; break;
		case 320 * 1024:	data |= 0x08; break;
		case 384 * 1024:	data |= 0x0A; break;
		case 448 * 1024:	data |= 0x0C; break;
		case 512 * 1024:	data |= 0x0E; break;
		case 576 * 1024:	data |= 0x01; break;
		case 640 * 1024:	data |= 0x03; break;
		case 704 * 1024:	data |= 0x05; break;
		case 768 * 1024:	data |= 0x07; break;
		case 832 * 1024:	data |= 0x09; break;
		case 896 * 1024:	data |= 0x0B; break;
		case 960 * 1024:	data |= 0x0D; break;
		}
		if ( machine().device<ram_device>(RAM_TAG)->size() > 960 * 1024 )
			data |= 0x0D;

		PIO_LOG(1,"PIO_C_r (hi)",("$%02x\n", data));
	}
	else
	{
		/* read lo nibble of S2 */
		data = (data & 0xf0) | (ioport("DSW0")->read() & 0x0f);
		PIO_LOG(1,"PIO_C_r (lo)",("$%02x\n", data));
	}

	if ( ! ( m_ppi_portb & 0x08 ) )
	{
		double tap_val = m_cassette->input();

		if ( tap_val < 0 )
		{
			data &= ~0x10;
		}
		else
		{
			data |= 0x10;
		}
	}
	else
	{
		if ( m_ppi_portb & 0x01 )
		{
			data = ( data & ~0x10 ) | ( timer2_output ? 0x10 : 0x00 );
		}
	}
	data = ( data & ~0x20 ) | ( timer2_output ? 0x20 : 0x00 );

	return data;
}


WRITE8_MEMBER( ibm5150_mb_device::pc_ppi_portb_w )
{
	/* KB controller port B */
	m_ppi_portb = data;
	m_ppi_portc_switch_high = data & 0x08;
	m_ppi_keyboard_clear = data & 0x80;
	m_ppi_keyb_clock = data & 0x40;
	pit8253_gate2_w(m_pit8253, BIT(data, 0));
	pc_speaker_set_spkrdata( data & 0x02 );

	m_cassette->change_state(( data & 0x08 ) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);

	m_ppi_clock_signal = ( m_ppi_keyb_clock ) ? 1 : 0;
	m_pc_kbdc->clock_write_from_mb(m_ppi_clock_signal);


	/* If PB7 is set clear the shift register and reset the IRQ line */
	if ( m_ppi_keyboard_clear )
	{
		pic8259_ir1_w(m_pic8259, 0);
		m_ppi_shift_register = 0;
		m_ppi_shift_enable = 1;
	}
}
