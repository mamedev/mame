// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic
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


/*************************************************************************
 *
 *      PC DMA stuff
 *
 *************************************************************************/

READ8_MEMBER( ibm5160_mb_device::pc_page_r)
{
	return 0xff;
}


WRITE8_MEMBER( ibm5160_mb_device::pc_page_w)
{
	switch(offset % 4)
	{
	case 1:
		m_dma_offset[2] = data;
		break;
	case 2:
		m_dma_offset[3] = data;
		break;
	case 3:
		m_dma_offset[0] = m_dma_offset[1] = data;
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

/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/

WRITE_LINE_MEMBER(ibm5160_mb_device::pc_speaker_set_spkrdata)
{
	m_pc_spkrdata = state ? 1 : 0;
	m_speaker->level_w(m_pc_spkrdata & m_pit_out2);
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
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(m_pc_spkrdata & m_pit_out2);
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
WRITE_LINE_MEMBER( ibm5150_mb_device::keyboard_clock_w )
{
	if (!m_ppi_keyboard_clear && !state && !m_ppi_shift_enable)
	{
		m_ppi_shift_enable = m_ppi_shift_register & 0x01;

		m_ppi_shift_register >>= 1;
		m_ppi_shift_register |= m_ppi_data_signal << 7;

		m_pic8259->ir1_w(m_ppi_shift_enable);
		m_pc_kbdc->data_write_from_mb(!m_ppi_shift_enable);
	}
}

WRITE_LINE_MEMBER( ec1841_mb_device::keyboard_clock_w )
{
	if (!m_ppi_keyboard_clear && !state && !m_ppi_shift_enable)
	{
		m_ppi_shift_enable = m_ppi_shift_register & 0x01;

		m_ppi_shift_register >>= 1;
		m_ppi_shift_register |= m_ppi_data_signal << 7;

		m_pic8259->ir1_w(m_ppi_shift_enable);
		m_pc_kbdc->data_write_from_mb(!m_ppi_shift_enable);
	}
}

WRITE_LINE_MEMBER( ibm5160_mb_device::keyboard_clock_w )
{
	if (!m_ppi_keyboard_clear && !state && !m_ppi_shift_enable)
	{
		m_ppi_shift_enable = m_ppi_shift_register & 0x01;

		m_ppi_shift_register >>= 1;
		m_ppi_shift_register |= m_ppi_data_signal << 7;

		m_pic8259->ir1_w(m_ppi_shift_enable);
		m_pc_kbdc->data_write_from_mb(!BIT(m_ppi_portb, 2) && !m_ppi_shift_enable);
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
		data = ( data & ~0x10 ) | ( m_pit_out2 ? 0x10 : 0x00 );
	}
	data = ( data & ~0x20 ) | ( m_pit_out2 ? 0x20 : 0x00 );

	return data;
}


WRITE8_MEMBER( ibm5160_mb_device::pc_ppi_portb_w )
{
	/* PPI controller port B*/
	m_ppi_portb = data;
	m_ppi_portc_switch_high = data & 0x08;
	m_ppi_keyboard_clear = data & 0x80;
	m_ppi_keyb_clock = data & 0x40;
	m_pit8253->write_gate2(BIT(data, 0));
	pc_speaker_set_spkrdata( data & 0x02 );

	/* If PB7 is set clear the shift register and reset the IRQ line */
	if ( m_ppi_keyboard_clear )
	{
		m_ppi_shift_register = 0;
		m_ppi_shift_enable = 0;
		m_pic8259->ir1_w(m_ppi_shift_enable);
	}

	m_pc_kbdc->data_write_from_mb(!BIT(m_ppi_portb, 2) && !m_ppi_shift_enable);
	m_ppi_clock_signal = ( m_ppi_keyb_clock ) ? 1 : 0;
	m_pc_kbdc->clock_write_from_mb(m_ppi_clock_signal);
}


/**********************************************************
 *
 * NMI handling
 *
 **********************************************************/

WRITE8_MEMBER( ibm5160_mb_device::nmi_enable_w )
{
	m_nmi_enabled = BIT(data,7);
	m_isabus->set_nmi_state(m_nmi_enabled);
}
//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type IBM5160_MOTHERBOARD = &device_creator<ibm5160_mb_device>;

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

static MACHINE_CONFIG_FRAGMENT( ibm5160_mb_config )
	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL_14_31818MHz/12) /* heartbeat IRQ */
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir0_w))
	MCFG_PIT8253_CLK1(XTAL_14_31818MHz/12) /* dram refresh */
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(ibm5160_mb_device, pc_pit8253_out1_changed))
	MCFG_PIT8253_CLK2(XTAL_14_31818MHz/12) /* pio port c pin 4, and speaker polling enough */
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(ibm5160_mb_device, pc_pit8253_out2_changed))

	MCFG_DEVICE_ADD( "dma8237", AM9517A, XTAL_14_31818MHz/3 )
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(ibm5160_mb_device, pc_dma_hrq_changed))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(ibm5160_mb_device, pc_dma8237_out_eop))
	MCFG_I8237_IN_MEMR_CB(READ8(ibm5160_mb_device, pc_dma_read_byte))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(ibm5160_mb_device, pc_dma_write_byte))
	MCFG_I8237_IN_IOR_1_CB(READ8(ibm5160_mb_device, pc_dma8237_1_dack_r))
	MCFG_I8237_IN_IOR_2_CB(READ8(ibm5160_mb_device, pc_dma8237_2_dack_r))
	MCFG_I8237_IN_IOR_3_CB(READ8(ibm5160_mb_device, pc_dma8237_3_dack_r))
	MCFG_I8237_OUT_IOW_0_CB(WRITE8(ibm5160_mb_device, pc_dma8237_0_dack_w))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(ibm5160_mb_device, pc_dma8237_1_dack_w))
	MCFG_I8237_OUT_IOW_2_CB(WRITE8(ibm5160_mb_device, pc_dma8237_2_dack_w))
	MCFG_I8237_OUT_IOW_3_CB(WRITE8(ibm5160_mb_device, pc_dma8237_3_dack_w))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(ibm5160_mb_device, pc_dack0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(ibm5160_mb_device, pc_dack1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(ibm5160_mb_device, pc_dack2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(ibm5160_mb_device, pc_dack3_w))

	MCFG_PIC8259_ADD( "pic8259", INPUTLINE(":maincpu", 0), VCC, NULL )

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(ibm5160_mb_device, pc_ppi_porta_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(ibm5160_mb_device, pc_ppi_portb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(ibm5160_mb_device, pc_ppi_portc_r))

	MCFG_DEVICE_ADD("isa", ISA8, 0)
	MCFG_ISA8_CPU(":maincpu")
	MCFG_ISA_OUT_IRQ2_CB(DEVWRITELINE("pic8259", pic8259_device, ir2_w))
	MCFG_ISA_OUT_IRQ3_CB(DEVWRITELINE("pic8259", pic8259_device, ir3_w))
	MCFG_ISA_OUT_IRQ4_CB(DEVWRITELINE("pic8259", pic8259_device, ir4_w))
	MCFG_ISA_OUT_IRQ5_CB(DEVWRITELINE("pic8259", pic8259_device, ir5_w))
	MCFG_ISA_OUT_IRQ6_CB(DEVWRITELINE("pic8259", pic8259_device, ir6_w))
	MCFG_ISA_OUT_IRQ7_CB(DEVWRITELINE("pic8259", pic8259_device, ir7_w))
	MCFG_ISA_OUT_DRQ1_CB(DEVWRITELINE("dma8237", am9517a_device, dreq1_w))
	MCFG_ISA_OUT_DRQ2_CB(DEVWRITELINE("dma8237", am9517a_device, dreq2_w))
	MCFG_ISA_OUT_DRQ3_CB(DEVWRITELINE("dma8237", am9517a_device, dreq3_w))

	MCFG_DEVICE_ADD("pc_kbdc", PC_KBDC, 0)
	MCFG_PC_KBDC_OUT_CLOCK_CB(WRITELINE(ibm5160_mb_device, keyboard_clock_w))
	MCFG_PC_KBDC_OUT_DATA_CB(WRITELINE(ibm5160_mb_device, keyboard_data_w))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
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
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0xc0, "4" )
	PORT_DIPNAME( 0x30, 0x30, "Graphics adapter")
	PORT_DIPSETTING(    0x00, "EGA/VGA" )
	PORT_DIPSETTING(    0x10, "Color 40x25" )
	PORT_DIPSETTING(    0x20, "Color 80x25" )
	PORT_DIPSETTING(    0x30, "Monochrome" )
	PORT_DIPNAME( 0x0c, 0x0c, "RAM banks")
	PORT_DIPSETTING(    0x00, "1 - 16/ 64/256K" )
	PORT_DIPSETTING(    0x04, "2 - 32/128/512K" )
	PORT_DIPSETTING(    0x08, "3 - 48/192/576K" )
	PORT_DIPSETTING(    0x0c, "4 - 64/256/640K" )
	PORT_DIPNAME( 0x02, 0x00, "8087 installed")
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
	PORT_DIPSETTING(    0x02, DEF_STR(Yes) )
	PORT_DIPNAME( 0x01, 0x01, "Boot from floppy")
	PORT_DIPSETTING(    0x01, DEF_STR(Yes) )
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
INPUT_PORTS_END
//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor ibm5160_mb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ibm5160_mb );
}


void ibm5160_mb_device::static_set_cputag(device_t &device, std::string tag)
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

ibm5160_mb_device::ibm5160_mb_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, IBM5160_MOTHERBOARD, "IBM5160_MOTHERBOARD", tag, owner, clock, "ibm5160_mb", __FILE__),
		m_maincpu(*owner, "maincpu"),
		m_pic8259(*this, "pic8259"),
		m_dma8237(*this, "dma8237"),
		m_pit8253(*this, "pit8253"),
		m_ppi8255(*this, "ppi8255"),
		m_speaker(*this, "speaker"),
		m_isabus(*this, "isa"),
		m_pc_kbdc(*this, "pc_kbdc"),
		m_ram(*this, ":" RAM_TAG)
{
}

void ibm5160_mb_device::install_device(offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_delegate rhandler, write8_delegate whandler)
{
	int buswidth = m_maincpu->space_config(AS_IO)->m_databus_width;
	switch(buswidth)
	{
		case 8:
			if(!rhandler.isnull()) m_maincpu->space(AS_IO).install_read_handler(start, end, mask, mirror, rhandler, 0);
			if(!whandler.isnull()) m_maincpu->space(AS_IO).install_write_handler(start, end, mask, mirror, whandler, 0);
			break;
		case 16:
			if(!rhandler.isnull()) m_maincpu->space(AS_IO).install_read_handler(start, end, mask, mirror, rhandler, 0xffff);
			if(!whandler.isnull()) m_maincpu->space(AS_IO).install_write_handler(start, end, mask, mirror, whandler, 0xffff);
			break;
		default:
			fatalerror("IBM5160_MOTHERBOARD: Bus width %d not supported\n", buswidth);
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ibm5160_mb_device::device_start()
{
	install_device(0x0000, 0x000f, 0, 0, read8_delegate(FUNC(am9517a_device::read), (am9517a_device*)m_dma8237), write8_delegate(FUNC(am9517a_device::write), (am9517a_device*)m_dma8237) );
	install_device(0x0020, 0x0021, 0, 0, read8_delegate(FUNC(pic8259_device::read), (pic8259_device*)m_pic8259), write8_delegate(FUNC(pic8259_device::write), (pic8259_device*)m_pic8259) );
	install_device(0x0040, 0x0043, 0, 0, read8_delegate(FUNC(pit8253_device::read), (pit8253_device*)m_pit8253), write8_delegate(FUNC(pit8253_device::write), (pit8253_device*)m_pit8253) );
	install_device(0x0060, 0x0063, 0, 0, read8_delegate(FUNC(i8255_device::read),   (i8255_device*)m_ppi8255),   write8_delegate(FUNC(i8255_device::write),   (i8255_device*)m_ppi8255)   );
	install_device(0x0080, 0x0087, 0, 0, read8_delegate(FUNC(ibm5160_mb_device::pc_page_r), this), write8_delegate(FUNC(ibm5160_mb_device::pc_page_w),this) );
	install_device(0x00a0, 0x00a1, 0, 0, read8_delegate(), write8_delegate(FUNC(ibm5160_mb_device::nmi_enable_w),this));
	/* MESS managed RAM */
	if ( m_ram->pointer() )
		membank( "bank10" )->set_base( m_ram->pointer() );
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ibm5160_mb_device::device_reset()
{
	m_u73_q2 = 0;
	m_out1 = 2; // initial state of pit output is undefined
	m_pc_spkrdata = 0;
	m_pit_out2 = 1;
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
	m_speaker->level_w(0);
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type IBM5150_MOTHERBOARD = &device_creator<ibm5150_mb_device>;

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************
static MACHINE_CONFIG_FRAGMENT( ibm5150_mb_config )
	MCFG_FRAGMENT_ADD(ibm5160_mb_config)

	MCFG_DEVICE_MODIFY("pc_kbdc")
	MCFG_PC_KBDC_OUT_CLOCK_CB(WRITELINE(ibm5150_mb_device, keyboard_clock_w))

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED)
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

ibm5150_mb_device::ibm5150_mb_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: ibm5160_mb_device(mconfig, tag, owner, clock),
		m_cassette(*this, "cassette")
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
		switch ( m_ram->size() )
		{
		case 16 * 1024:
			data |= 0x00;
			break;
		case 32 * 1024: /* Need to verify if this is correct */
			data |= 0x04;
			break;
		case 48 * 1024: /* Need to verify if this is correct */
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
	int data=0xff;

	data&=~0x80; // no parity error
	data&=~0x40; // no error on expansion board
	/* KB port C: equipment flags */
	if (m_ppi_portc_switch_high)
	{
		/* read hi nibble of SW2 */
		data = data & 0xf0;

		switch ( m_ram->size() - 64 * 1024 )
		{
		case 64 * 1024:     data |= 0x00; break;
		case 128 * 1024:    data |= 0x02; break;
		case 192 * 1024:    data |= 0x04; break;
		case 256 * 1024:    data |= 0x06; break;
		case 320 * 1024:    data |= 0x08; break;
		case 384 * 1024:    data |= 0x0A; break;
		case 448 * 1024:    data |= 0x0C; break;
		case 512 * 1024:    data |= 0x0E; break;
		case 576 * 1024:    data |= 0x01; break;
		case 640 * 1024:    data |= 0x03; break;
		case 704 * 1024:    data |= 0x05; break;
		case 768 * 1024:    data |= 0x07; break;
		case 832 * 1024:    data |= 0x09; break;
		case 896 * 1024:    data |= 0x0B; break;
		case 960 * 1024:    data |= 0x0D; break;
		}
		if ( m_ram->size() > 960 * 1024 )
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
			data = ( data & ~0x10 ) | ( m_pit_out2 ? 0x10 : 0x00 );
		}
	}
	data = ( data & ~0x20 ) | ( m_pit_out2 ? 0x20 : 0x00 );

	return data;
}


WRITE8_MEMBER( ibm5150_mb_device::pc_ppi_portb_w )
{
	/* KB controller port B */
	m_ppi_portb = data;
	m_ppi_portc_switch_high = data & 0x08;
	m_ppi_keyboard_clear = data & 0x80;
	m_ppi_keyb_clock = data & 0x40;
	m_pit8253->write_gate2(BIT(data, 0));
	pc_speaker_set_spkrdata( data & 0x02 );

	m_cassette->change_state(( data & 0x08 ) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);

	/* If PB7 is set clear the shift register and reset the IRQ line */
	if ( m_ppi_keyboard_clear )
	{
		m_ppi_shift_register = 0;
		m_ppi_shift_enable = 0;
		m_pic8259->ir1_w(m_ppi_shift_enable);
	}

	m_pc_kbdc->data_write_from_mb(!m_ppi_shift_enable);
	m_ppi_clock_signal = ( m_ppi_keyb_clock ) ? 1 : 0;
	m_pc_kbdc->clock_write_from_mb(m_ppi_clock_signal);
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type EC1841_MOTHERBOARD = &device_creator<ec1841_mb_device>;

static MACHINE_CONFIG_FRAGMENT( ec1841_mb_config )
	MCFG_FRAGMENT_ADD(ibm5160_mb_config)

	MCFG_DEVICE_MODIFY("pc_kbdc")
	MCFG_PC_KBDC_OUT_CLOCK_CB(WRITELINE(ec1841_mb_device, keyboard_clock_w))
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor ec1841_mb_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ec1841_mb_config );
}

static INPUT_PORTS_START( ec1841_mb )
	PORT_START("DSW0") /* SA1 */
	PORT_DIPNAME( 0xc0, 0x40, "Number of floppy drives")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0xc0, "4" )
	PORT_DIPNAME( 0x30, 0x20, "Graphics adapter")
	PORT_DIPSETTING(    0x00, "Reserved" )
	PORT_DIPSETTING(    0x10, "Color 40x25" )
	PORT_DIPSETTING(    0x20, "Color 80x25" )
	PORT_DIPSETTING(    0x30, "Monochrome" )
	PORT_BIT(     0x08, 0x08, IPT_UNUSED )
	/* BIOS does not support booting from QD floppies */
	PORT_DIPNAME( 0x04, 0x04, "Floppy type")
	PORT_DIPSETTING(    0x00, "80 tracks" )
	PORT_DIPSETTING(    0x04, "40 tracks" )
	PORT_DIPNAME( 0x02, 0x00, "8087 installed")
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
	PORT_DIPSETTING(    0x02, DEF_STR(Yes) )
	PORT_DIPNAME( 0x01, 0x01, "Boot from floppy")
	PORT_DIPSETTING(    0x01, DEF_STR(Yes) )
	PORT_DIPSETTING(    0x00, DEF_STR(No) )

	PORT_START("SA2")
	PORT_DIPNAME( 0x04, 0x04, "Speech synthesizer")
	PORT_DIPSETTING(    0x00, "Installed" )
	PORT_DIPSETTING(    0x04, "Not installed" )
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor ec1841_mb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ec1841_mb );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ec1841_mb_device - constructor
//-------------------------------------------------

ec1841_mb_device::ec1841_mb_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: ibm5160_mb_device(mconfig, tag, owner, clock)
{
}

void ec1841_mb_device::device_start()
{
	ibm5160_mb_device::device_start();
}
void ec1841_mb_device::device_reset()
{
	ibm5160_mb_device::device_reset();
}

// kbd interface is 5150-like but PB2 controls access to second bank of DIP switches (SA2).
WRITE8_MEMBER( ec1841_mb_device::pc_ppi_portb_w )
{
	/* KB controller port B */
	m_ppi_portb = data;
	m_ppi_portc_switch_high = data & 0x04;
	m_ppi_keyboard_clear = data & 0x80;
	m_ppi_keyb_clock = data & 0x40;
	m_pit8253->write_gate2(BIT(data, 0));
	pc_speaker_set_spkrdata( data & 0x02 );

	/* If PB7 is set clear the shift register and reset the IRQ line */
	if ( m_ppi_keyboard_clear )
	{
		m_ppi_shift_register = 0;
		m_ppi_shift_enable = 0;
		m_pic8259->ir1_w(m_ppi_shift_enable);
	}

	m_pc_kbdc->data_write_from_mb(!m_ppi_shift_enable);
	m_ppi_clock_signal = ( m_ppi_keyb_clock ) ? 1 : 0;
	m_pc_kbdc->clock_write_from_mb(m_ppi_clock_signal);
}

READ8_MEMBER ( ec1841_mb_device::pc_ppi_portc_r )
{
	int data=0xff;

	data&=~0x80; // no parity error
	data&=~0x40; // no error on expansion board

	if (m_ppi_portc_switch_high)
	{
		data = (data & 0xf0) | (ioport("SA2")->read() & 0x0f);
	}

	data = ( data & ~0x20 ) | ( m_pit_out2 ? 0x20 : 0x00 );

	return data;
}

pc_noppi_mb_device::pc_noppi_mb_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
		: ibm5160_mb_device(mconfig, tag, owner, clock)
{
}

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

static MACHINE_CONFIG_FRAGMENT( pc_noppi_mb_config )
	MCFG_FRAGMENT_ADD(ibm5160_mb_config)

	MCFG_DEVICE_REMOVE("pc_kbdc")
	MCFG_DEVICE_REMOVE("ppi8255")
MACHINE_CONFIG_END

static INPUT_PORTS_START( pc_noppi_mb )
INPUT_PORTS_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor pc_noppi_mb_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pc_noppi_mb_config );
}

ioport_constructor pc_noppi_mb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc_noppi_mb );
}

void pc_noppi_mb_device::device_start()
{
	install_device(0x0000, 0x000f, 0, 0, read8_delegate(FUNC(am9517a_device::read), (am9517a_device*)m_dma8237), write8_delegate(FUNC(am9517a_device::write), (am9517a_device*)m_dma8237) );
	install_device(0x0020, 0x0021, 0, 0, read8_delegate(FUNC(pic8259_device::read), (pic8259_device*)m_pic8259), write8_delegate(FUNC(pic8259_device::write), (pic8259_device*)m_pic8259) );
	install_device(0x0040, 0x0043, 0, 0, read8_delegate(FUNC(pit8253_device::read), (pit8253_device*)m_pit8253), write8_delegate(FUNC(pit8253_device::write), (pit8253_device*)m_pit8253) );
	install_device(0x0080, 0x0087, 0, 0, read8_delegate(FUNC(ibm5160_mb_device::pc_page_r), this), write8_delegate(FUNC(ibm5160_mb_device::pc_page_w),this) );
	install_device(0x00a0, 0x00a1, 0, 0, read8_delegate(), write8_delegate(FUNC(ibm5160_mb_device::nmi_enable_w),this));
	/* MESS managed RAM */
	if ( m_ram->pointer() )
		membank( "bank10" )->set_base( m_ram->pointer() );
}

const device_type PCNOPPI_MOTHERBOARD = &device_creator<pc_noppi_mb_device>;
