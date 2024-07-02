// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic
/***************************************************************************

    machine/genpc.cpp

***************************************************************************/

#include "emu.h"
#include "genpc.h"

#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"

#include "speaker.h"


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

void ibm5160_mb_device::pc_page_w(offs_t offset, uint8_t data)
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


void ibm5160_mb_device::pc_dma_hrq_changed(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237->hack_w(state);
}


uint8_t ibm5160_mb_device::pc_dma_read_byte(offs_t offset)
{
	if(m_dma_channel == -1)
		return 0xff;
	address_space &spaceio = m_maincpu->space(AS_PROGRAM);
	offs_t page_offset = (((offs_t) m_dma_offset[m_dma_channel]) << 16) & 0x0F0000;
	return spaceio.read_byte( page_offset + offset);
}


void ibm5160_mb_device::pc_dma_write_byte(offs_t offset, uint8_t data)
{
	if(m_dma_channel == -1)
		return;
	address_space &spaceio = m_maincpu->space(AS_PROGRAM);
	offs_t page_offset = (((offs_t) m_dma_offset[m_dma_channel]) << 16) & 0x0F0000;

	spaceio.write_byte( page_offset + offset, data);
}


uint8_t ibm5160_mb_device::pc_dma8237_1_dack_r()
{
	return m_isabus->dack_r(1);
}

uint8_t ibm5160_mb_device::pc_dma8237_2_dack_r()
{
	return m_isabus->dack_r(2);
}


uint8_t ibm5160_mb_device::pc_dma8237_3_dack_r()
{
	return m_isabus->dack_r(3);
}


void ibm5160_mb_device::pc_dma8237_1_dack_w(uint8_t data)
{
	m_isabus->dack_w(1,data);
}

void ibm5160_mb_device::pc_dma8237_2_dack_w(uint8_t data)
{
	m_isabus->dack_w(2,data);
}


void ibm5160_mb_device::pc_dma8237_3_dack_w(uint8_t data)
{
	m_isabus->dack_w(3,data);
}


void ibm5160_mb_device::pc_dma8237_0_dack_w(uint8_t data)
{
	m_u73_q2 = 0;
	m_dma8237->dreq0_w( m_u73_q2 );
}


void ibm5160_mb_device::pc_dma8237_out_eop(int state)
{
	m_cur_eop = state == ASSERT_LINE;
	if(m_dma_channel != -1)
		m_isabus->eop_w(m_dma_channel, m_cur_eop ? ASSERT_LINE : CLEAR_LINE );
}

void ibm5160_mb_device::pc_select_dma_channel(int channel, bool state)
{
	m_isabus->dack_line_w(channel, state);

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

void ibm5160_mb_device::pc_dack0_w(int state) { pc_select_dma_channel(0, state); }
void ibm5160_mb_device::pc_dack1_w(int state) { pc_select_dma_channel(1, state); }
void ibm5160_mb_device::pc_dack2_w(int state) { pc_select_dma_channel(2, state); }
void ibm5160_mb_device::pc_dack3_w(int state) { pc_select_dma_channel(3, state); }

/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/

void ibm5160_mb_device::pc_speaker_set_spkrdata(int state)
{
	m_pc_spkrdata = state ? 1 : 0;
	m_speaker->level_w(m_pc_spkrdata & m_pit_out2);
}

void ibm5160_mb_device::pic_int_w(int state)
{
	m_int_callback(state);
}


/*************************************************************
 *
 * pit8253 configuration
 *
 *************************************************************/

void ibm5160_mb_device::pc_pit8253_out1_changed(int state)
{
	/* Trigger DMA channel #0 */
	if ( m_out1 == 0 && state == 1 && m_u73_q2 == 0 )
	{
		m_u73_q2 = 1;
		m_dma8237->dreq0_w( m_u73_q2 );
	}
	m_out1 = state;
}


void ibm5160_mb_device::pc_pit8253_out2_changed(int state)
{
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(m_pc_spkrdata & m_pit_out2);
}

void ibm5150_mb_device::pc_pit8253_out2_changed(int state)
{
	ibm5160_mb_device::pc_pit8253_out2_changed(state);
	m_cassette->output(m_pit_out2 ? 1.0 : -1.0);
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
void ibm5150_mb_device::keyboard_clock_w(int state)
{
	if (!m_ppi_keyboard_clear && !state && !m_ppi_shift_enable)
	{
		m_ppi_shift_enable = m_ppi_shift_register & 0x01;

		m_ppi_shift_register >>= 1;
		m_ppi_shift_register |= m_ppi_data_signal << 7;

		m_pic8259->ir1_w(m_ppi_shift_enable);
		m_kbddata_callback(!m_ppi_shift_enable);
	}
}

void ec1841_mb_device::keyboard_clock_w(int state)
{
	if (!m_ppi_keyboard_clear && !state && !m_ppi_shift_enable)
	{
		m_ppi_shift_enable = m_ppi_shift_register & 0x01;

		m_ppi_shift_register >>= 1;
		m_ppi_shift_register |= m_ppi_data_signal << 7;

		m_pic8259->ir1_w(m_ppi_shift_enable);
		m_kbddata_callback(!m_ppi_shift_enable);
	}
}

void ibm5160_mb_device::keyboard_clock_w(int state)
{
	if (!m_ppi_keyboard_clear && !state && !m_ppi_shift_enable)
	{
		m_ppi_shift_enable = m_ppi_shift_register & 0x01;

		m_ppi_shift_register >>= 1;
		m_ppi_shift_register |= m_ppi_data_signal << 7;

		m_pic8259->ir1_w(m_ppi_shift_enable);
		m_kbddata_callback(!m_ppi_shift_enable);
	}
}


void ibm5160_mb_device::keyboard_data_w(int state)
{
	m_ppi_data_signal = state;
}

uint8_t ibm5160_mb_device::pc_ppi_porta_r()
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


uint8_t ibm5160_mb_device::pc_ppi_portc_r()
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


void ibm5160_mb_device::pc_ppi_portb_w(uint8_t data)
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

	m_kbddata_callback(!m_ppi_shift_enable);
	m_ppi_clock_signal = ( m_ppi_keyb_clock ) ? 1 : 0;
	m_kbdclk_callback(m_ppi_clock_signal);
}


/**********************************************************
 *
 * NMI handling
 *
 **********************************************************/

void ibm5160_mb_device::nmi_enable_w(uint8_t data)
{
	m_nmi_enabled = BIT(data,7);
	if (!m_nmi_enabled)
		m_nmi_callback(CLEAR_LINE);
}

void ibm5160_mb_device::iochck_w(int state)
{
	if (m_nmi_enabled && !state)
		m_nmi_callback(ASSERT_LINE);
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(IBM5160_MOTHERBOARD, ibm5160_mb_device, "ibm5160_mb", "IBM 5160 motherboard")

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void ibm5160_mb_device::device_add_mconfig(machine_config &config)
{
	PIT8253(config, m_pit8253);
	m_pit8253->set_clk<0>(XTAL(14'318'181)/12.0); // heartbeat IRQ
	m_pit8253->out_handler<0>().set(m_pic8259, FUNC(pic8259_device::ir0_w));
	m_pit8253->set_clk<1>(XTAL(14'318'181)/12.0); // DRAM refresh
	m_pit8253->out_handler<1>().set(FUNC(ibm5160_mb_device::pc_pit8253_out1_changed));
	m_pit8253->set_clk<2>(XTAL(14'318'181)/12.0); // PIO port C pin 4, and speaker polling enough
	m_pit8253->out_handler<2>().set(FUNC(ibm5160_mb_device::pc_pit8253_out2_changed));

	AM9517A(config, m_dma8237, XTAL(14'318'181)/3.0);
	m_dma8237->out_hreq_callback().set(FUNC(ibm5160_mb_device::pc_dma_hrq_changed));
	m_dma8237->out_eop_callback().set(FUNC(ibm5160_mb_device::pc_dma8237_out_eop));
	m_dma8237->in_memr_callback().set(FUNC(ibm5160_mb_device::pc_dma_read_byte));
	m_dma8237->out_memw_callback().set(FUNC(ibm5160_mb_device::pc_dma_write_byte));
	m_dma8237->in_ior_callback<1>().set(FUNC(ibm5160_mb_device::pc_dma8237_1_dack_r));
	m_dma8237->in_ior_callback<2>().set(FUNC(ibm5160_mb_device::pc_dma8237_2_dack_r));
	m_dma8237->in_ior_callback<3>().set(FUNC(ibm5160_mb_device::pc_dma8237_3_dack_r));
	m_dma8237->out_iow_callback<0>().set(FUNC(ibm5160_mb_device::pc_dma8237_0_dack_w));
	m_dma8237->out_iow_callback<1>().set(FUNC(ibm5160_mb_device::pc_dma8237_1_dack_w));
	m_dma8237->out_iow_callback<2>().set(FUNC(ibm5160_mb_device::pc_dma8237_2_dack_w));
	m_dma8237->out_iow_callback<3>().set(FUNC(ibm5160_mb_device::pc_dma8237_3_dack_w));
	m_dma8237->out_dack_callback<0>().set(FUNC(ibm5160_mb_device::pc_dack0_w));
	m_dma8237->out_dack_callback<1>().set(FUNC(ibm5160_mb_device::pc_dack1_w));
	m_dma8237->out_dack_callback<2>().set(FUNC(ibm5160_mb_device::pc_dack2_w));
	m_dma8237->out_dack_callback<3>().set(FUNC(ibm5160_mb_device::pc_dack3_w));

	PIC8259(config, m_pic8259);
	m_pic8259->out_int_callback().set(FUNC(ibm5160_mb_device::pic_int_w));

	I8255A(config, m_ppi8255);
	m_ppi8255->in_pa_callback().set(FUNC(ibm5160_mb_device::pc_ppi_porta_r));
	m_ppi8255->out_pb_callback().set(FUNC(ibm5160_mb_device::pc_ppi_portb_w));
	m_ppi8255->in_pc_callback().set(FUNC(ibm5160_mb_device::pc_ppi_portc_r));

	ISA8(config, m_isabus, 0);
	m_isabus->set_memspace(":maincpu", AS_PROGRAM);
	m_isabus->set_iospace(":maincpu", AS_IO);
	m_isabus->irq2_callback().set(m_pic8259, FUNC(pic8259_device::ir2_w));
	m_isabus->irq3_callback().set(m_pic8259, FUNC(pic8259_device::ir3_w));
	m_isabus->irq4_callback().set(m_pic8259, FUNC(pic8259_device::ir4_w));
	m_isabus->irq5_callback().set(m_pic8259, FUNC(pic8259_device::ir5_w));
	m_isabus->irq6_callback().set(m_pic8259, FUNC(pic8259_device::ir6_w));
	m_isabus->irq7_callback().set(m_pic8259, FUNC(pic8259_device::ir7_w));
	m_isabus->drq1_callback().set(m_dma8237, FUNC(am9517a_device::dreq1_w));
	m_isabus->drq2_callback().set(m_dma8237, FUNC(am9517a_device::dreq2_w));
	m_isabus->drq3_callback().set(m_dma8237, FUNC(am9517a_device::dreq3_w));
	m_isabus->iochck_callback().set(FUNC(ibm5160_mb_device::iochck_w));
	m_isabus->iochrdy_callback().set_inputline(":maincpu", INPUT_LINE_HALT);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);
}


static INPUT_PORTS_START( ibm5160_mb )
	PORT_START("DSW0") /* IN1 */
	PORT_DIPNAME( 0xc0, 0x40, "Number of floppy drives") PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0xc0, "4" )
	PORT_DIPNAME( 0x30, 0x30, "Graphics adapter") PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, "EGA/VGA" )
	PORT_DIPSETTING(    0x10, "Color 40x25" )
	PORT_DIPSETTING(    0x20, "Color 80x25" )
	PORT_DIPSETTING(    0x30, "Monochrome" )
	PORT_DIPNAME( 0x0c, 0x0c, "RAM banks") PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "1 - 16/ 64/256K" )
	PORT_DIPSETTING(    0x04, "2 - 32/128/512K" )
	PORT_DIPSETTING(    0x08, "3 - 48/192/576K" )
	PORT_DIPSETTING(    0x0c, "4 - 64/256/640K" )
	PORT_DIPNAME( 0x02, 0x00, "8087 installed") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
	PORT_DIPSETTING(    0x02, DEF_STR(Yes) )
	PORT_DIPNAME( 0x01, 0x01, "Boot from floppy") PORT_DIPLOCATION("SW1:1")
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


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ibm5160_mb_device - constructor
//-------------------------------------------------

ibm5160_mb_device::ibm5160_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ibm5160_mb_device(mconfig, IBM5160_MOTHERBOARD, tag, owner, clock)
{
}

ibm5160_mb_device::ibm5160_mb_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_maincpu(*this, finder_base::DUMMY_TAG)
	, m_pic8259(*this, "pic8259")
	, m_pit8253(*this, "pit8253")
	, m_dma8237(*this, "dma8237")
	, m_ppi8255(*this, "ppi8255")
	, m_speaker(*this, "speaker")
	, m_isabus(*this, "isa")
	, m_ram(*this, ":" RAM_TAG)
	, m_int_callback(*this)
	, m_nmi_callback(*this)
	, m_kbdclk_callback(*this)
	, m_kbddata_callback(*this)
{
}

void ibm5160_mb_device::map(address_map &map)
{
	map(0x0000, 0x000f).rw("dma8237", FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x002f).rw("pic8259", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0040, 0x004f).rw("pit8253", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x0060, 0x006f).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x0080, 0x008f).w(FUNC(ibm5160_mb_device::pc_page_w));
	map(0x00a0, 0x00a1).w(FUNC(ibm5160_mb_device::nmi_enable_w));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ibm5160_mb_device::device_start()
{
	if(!m_ram->started())
		throw device_missing_dependencies();
	m_maincpu->space(AS_PROGRAM).install_ram(0, m_ram->size() - 1, m_ram->pointer());
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

DEFINE_DEVICE_TYPE(IBM5150_MOTHERBOARD, ibm5150_mb_device, "ibm5150_mb", "IBM 5150 motherboard")

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void ibm5150_mb_device::device_add_mconfig(machine_config &config)
{
	ibm5160_mb_device::device_add_mconfig(config);

	m_ppi8255->out_pb_callback().set(FUNC(ibm5150_mb_device::pc_ppi_portb_w));
	m_ppi8255->in_pc_callback().set(FUNC(ibm5150_mb_device::pc_ppi_portc_r));

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_interface("ibm5150_cass");
}

static INPUT_PORTS_START( ibm5150_mb )
	PORT_START("DSW0")
	PORT_DIPNAME( 0xc0, 0x40, "Number of floppy drives") PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0xc0, "4" )
	PORT_DIPNAME( 0x30, 0x30, "Graphics adapter") PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x00, "EGA/VGA" )
	PORT_DIPSETTING(    0x10, "Color 40x25" )
	PORT_DIPSETTING(    0x20, "Color 80x25" )
	PORT_DIPSETTING(    0x30, "Monochrome" )
	PORT_DIPNAME( 0x0c, 0x0c, "Base RAM size") PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "16K" )
	PORT_DIPSETTING(    0x04, "32K" )
	PORT_DIPSETTING(    0x08, "48K" )
	PORT_DIPSETTING(    0x0c, "64K" )
	PORT_DIPNAME( 0x02, 0x00, "8087 installed") PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
	PORT_DIPSETTING(    0x02, DEF_STR(Yes) )
	PORT_DIPNAME( 0x01, 0x01, "Boot from floppy") PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR(Yes) )
	PORT_DIPSETTING(    0x00, DEF_STR(No) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x1f, 0x12, "Extra RAM size") PORT_DIPLOCATION("SW2:1,2,3,4,5")
	PORT_DIPSETTING(    0x00, "None" )
	PORT_DIPSETTING(    0x01, "32K" )
	PORT_DIPSETTING(    0x02, "64K" )
	PORT_DIPSETTING(    0x03, "96K" )
	PORT_DIPSETTING(    0x04, "128K" )
	PORT_DIPSETTING(    0x05, "160K" )
	PORT_DIPSETTING(    0x06, "192K" )
	PORT_DIPSETTING(    0x07, "224K" )
	PORT_DIPSETTING(    0x08, "256K" )
	PORT_DIPSETTING(    0x09, "288K" )
	PORT_DIPSETTING(    0x0a, "320K" )
	PORT_DIPSETTING(    0x0b, "352K" )
	PORT_DIPSETTING(    0x0c, "384K" )
	PORT_DIPSETTING(    0x0d, "416K" )
	PORT_DIPSETTING(    0x0e, "448K" )
	PORT_DIPSETTING(    0x0f, "480K" )
	PORT_DIPSETTING(    0x10, "512K" )
	PORT_DIPSETTING(    0x11, "544K" )
	PORT_DIPSETTING(    0x12, "576K" )
	PORT_DIPUNUSED_DIPLOC( 0xe0, 0x00, "SW2:6,7,8" )
INPUT_PORTS_END

ioport_constructor ibm5150_mb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ibm5150_mb );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ibm5150_mb_device - constructor
//-------------------------------------------------

ibm5150_mb_device::ibm5150_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ibm5150_mb_device(mconfig, IBM5150_MOTHERBOARD, tag, owner, clock)
{
}

ibm5150_mb_device::ibm5150_mb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: ibm5160_mb_device(mconfig, type, tag, owner, clock)
	, m_cassette(*this, "cassette")
{
}

uint8_t ibm5150_mb_device::pc_ppi_porta_r()
{
	int data = 0xFF;
	/* KB port A */
	if (m_ppi_keyboard_clear)
	{
		data = ioport("DSW0")->read();
	}
	else
	{
		data = m_ppi_shift_register;
	}
	PIO_LOG(1,"PIO_A_r",("$%02x\n", data));
	return data;
}


uint8_t ibm5150_mb_device::pc_ppi_portc_r()
{
	int data=0xff;

	data&=~0x80; // no parity error
	data&=~0x40; // no error on expansion board
	/* KB port C: equipment flags */
	if (m_ppi_portc_switch_high)
	{
		data = (data & 0xf0) | ioport("DSW1")->read();
	}
	else
	{
		data = (data & 0xf0) | (ioport("DSW1")->read() >> 4);
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


void ibm5150_mb_device::pc_ppi_portb_w(uint8_t data)
{
	/* KB controller port B */
	m_ppi_portb = data;
	m_ppi_portc_switch_high = data & 0x04;
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

	m_kbddata_callback(!m_ppi_shift_enable);
	m_ppi_clock_signal = ( m_ppi_keyb_clock ) ? 1 : 0;
	m_kbdclk_callback(m_ppi_clock_signal);
}

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(EC1840_MOTHERBOARD, ec1840_mb_device, "ec1840_mb", "EC-1840 motherboard")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void ec1840_mb_device::device_add_mconfig(machine_config &config)
{
	ec1841_mb_device::device_add_mconfig(config);

	m_ppi8255->in_pc_callback().set(FUNC(ec1840_mb_device::pc_ppi_portc_r));
}

// via http://oldpc.su/pc/ec1840/ec1840rep.html
static INPUT_PORTS_START( ec1840_mb )
	PORT_START("DSW0") /* SA1 */
	PORT_DIPNAME( 0xc0, 0x40, "Number of floppy drives")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0xc0, "4" )
	PORT_DIPNAME( 0x30, 0x30, "Graphics adapter")
	PORT_DIPSETTING(    0x00, "Reserved" )
	PORT_DIPSETTING(    0x10, "Color 40x25" )
	PORT_DIPSETTING(    0x20, "Color 80x25" )
	PORT_DIPSETTING(    0x30, "Monochrome" )
	PORT_BIT(     0x0c, 0x0c, IPT_UNUSED )
	PORT_DIPNAME( 0x02, 0x02, "DMAC installed")
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
	PORT_DIPSETTING(    0x02, DEF_STR(Yes) )
	PORT_DIPNAME( 0x01, 0x01, "Boot from floppy")
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
	PORT_DIPSETTING(    0x01, DEF_STR(Yes) )

	PORT_START("SA2")
	PORT_BIT(     0xcf, 0x00, IPT_UNUSED )
	PORT_DIPNAME( 0x20, 0x20, "SA2.5")
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
	PORT_DIPSETTING(    0x20, DEF_STR(Yes) )
	PORT_DIPNAME( 0x10, 0x10, "SA2.4")
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
	PORT_DIPSETTING(    0x10, DEF_STR(Yes) )
INPUT_PORTS_END

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor ec1840_mb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( ec1840_mb );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  ec1840_mb_device - constructor
//-------------------------------------------------

ec1840_mb_device::ec1840_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ec1841_mb_device(mconfig, EC1840_MOTHERBOARD, tag, owner, clock)
{
}

void ec1840_mb_device::device_start()
{
}

uint8_t ec1840_mb_device::pc_ppi_portc_r()
{
	int data = 0xff;

	data &= ~0x80; // no parity error
	data &= ~0x40; // no error on expansion board

	if (m_ppi_portc_switch_high)
	{
		/* read hi nibble of SW2 */
		data = data & 0xf0;

		switch (m_ram->size())
		{
		case 128 * 1024:    data |= 0x00; break;
		case 256 * 1024:    data |= 0x01; break;
		case 384 * 1024:    data |= 0x02; break;
		case 512 * 1024:    data |= 0x03; break;
		case 640 * 1024:    data |= 0x04; break;
		}

		PIO_LOG(1,"PIO_C_r (hi)",("$%02x\n", data));
	}
	else
	{
		/* read lo nibble of S2 */
		data = (data & 0xf0) | (ioport("DSW0")->read() >> 4);
		PIO_LOG(1,"PIO_C_r (lo)",("$%02x\n", data));
	}

	data = ( data & ~0x20 ) | ( m_pit_out2 ? 0x20 : 0x00 );

	return data;
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(EC1841_MOTHERBOARD, ec1841_mb_device, "ec1841_mb", "EC-1841 motherboard")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void ec1841_mb_device::device_add_mconfig(machine_config &config)
{
	ibm5160_mb_device::device_add_mconfig(config);

	m_ppi8255->out_pb_callback().set(FUNC(ec1841_mb_device::pc_ppi_portb_w));
	m_ppi8255->in_pc_callback().set(FUNC(ec1841_mb_device::pc_ppi_portc_r));
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
	PORT_BIT(     0xcb, 0x00, IPT_UNUSED )
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

ec1841_mb_device::ec1841_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: ibm5160_mb_device(mconfig, EC1841_MOTHERBOARD, tag, owner, clock)
{
}

ec1841_mb_device::ec1841_mb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: ibm5160_mb_device(mconfig, type, tag, owner, clock)
{
}

void ec1841_mb_device::device_start()
{
}

// kbd interface is 5150-like but PB2 controls access to second bank of DIP switches (SA2).
void ec1841_mb_device::pc_ppi_portb_w(uint8_t data)
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

	m_kbddata_callback(!m_ppi_shift_enable);
	m_ppi_clock_signal = ( m_ppi_keyb_clock ) ? 1 : 0;
	m_kbdclk_callback(m_ppi_clock_signal);
}

uint8_t ec1841_mb_device::pc_ppi_portc_r()
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

pc_noppi_mb_device::pc_noppi_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc_noppi_mb_device(mconfig, PCNOPPI_MOTHERBOARD, tag, owner, clock)
{
}

pc_noppi_mb_device::pc_noppi_mb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: ibm5160_mb_device(mconfig, type, tag, owner, clock)
{
}

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void pc_noppi_mb_device::device_add_mconfig(machine_config &config)
{
	ibm5160_mb_device::device_add_mconfig(config);

	config.device_remove("ppi8255");
}

static INPUT_PORTS_START( pc_noppi_mb )
INPUT_PORTS_END

ioport_constructor pc_noppi_mb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc_noppi_mb );
}

uint8_t pc_noppi_mb_device::pc_ppi_porta_r()
{
	return m_ppi_shift_register;
}

uint8_t pc_noppi_mb_device::pc_ppi_portb_r()
{
	return m_ppi_portb;
}

void pc_noppi_mb_device::map(address_map &map)
{
	map(0x0000, 0x000f).rw("dma8237", FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x002f).rw("pic8259", FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0040, 0x004f).rw("pit8253", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x0060, 0x0060).r(FUNC(pc_noppi_mb_device::pc_ppi_porta_r));
	map(0x0061, 0x0061).rw(FUNC(pc_noppi_mb_device::pc_ppi_portb_r), FUNC(pc_noppi_mb_device::pc_ppi_portb_w));
	map(0x0080, 0x008f).w(FUNC(pc_noppi_mb_device::pc_page_w));
	map(0x00a0, 0x00a1).w(FUNC(pc_noppi_mb_device::nmi_enable_w));
}

DEFINE_DEVICE_TYPE(PCNOPPI_MOTHERBOARD, pc_noppi_mb_device, "pcnoppi_mb", "PCNOPPI_MOTHERBOARD")
