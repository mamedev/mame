/***************************************************************************

    IBM AT Compatibles

***************************************************************************/

#include "includes/at.h"

#define LOG_PORT80	0

/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/
READ8_MEMBER( at_state::get_slave_ack )
{
	if (offset==2) // IRQ = 2
		return pic8259_acknowledge(m_pic8259_slave);

	return 0x00;
}

const struct pic8259_interface at_pic8259_master_config =
{
	DEVCB_CPU_INPUT_LINE("maincpu", 0),
	DEVCB_LINE_VCC,
	DEVCB_DRIVER_MEMBER(at_state, get_slave_ack)
};

const struct pic8259_interface at_pic8259_slave_config =
{
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir2_w),
	DEVCB_LINE_GND,
	DEVCB_NULL
};



/*************************************************************************
 *
 *      PC Speaker related
 *
 *************************************************************************/

void at_state::at_speaker_set_spkrdata(UINT8 data)
{
	m_at_spkrdata = data ? 1 : 0;
	speaker_level_w( m_speaker, m_at_spkrdata & m_at_speaker_input);
}

void at_state::at_speaker_set_input(UINT8 data)
{
	m_at_speaker_input = data ? 1 : 0;
	speaker_level_w( m_speaker, m_at_spkrdata & m_at_speaker_input);
}



/*************************************************************
 *
 * pit8254 configuration
 *
 *************************************************************/

WRITE_LINE_MEMBER( at_state::at_pit8254_out0_changed )
{
	if (m_pic8259_master)
		pic8259_ir0_w(m_pic8259_master, state);
}


WRITE_LINE_MEMBER( at_state::at_pit8254_out2_changed )
{
	at_speaker_set_input( state );
}


const struct pit8253_config at_pit8254_config =
{
	{
		{
			4772720/4,				/* heartbeat IRQ */
			DEVCB_NULL,
			DEVCB_DRIVER_LINE_MEMBER(at_state, at_pit8254_out0_changed)
		}, {
			4772720/4,				/* dram refresh */
			DEVCB_NULL,
			DEVCB_NULL
		}, {
			4772720/4,				/* pio port c pin 4, and speaker polling enough */
			DEVCB_NULL,
			DEVCB_DRIVER_LINE_MEMBER(at_state, at_pit8254_out2_changed)
		}
	}
};


/*************************************************************************
 *
 *      PC DMA stuff
 *
 *************************************************************************/

READ8_MEMBER( at_state::at_page8_r )
{
	UINT8 data = m_at_pages[offset % 0x10];

	switch(offset % 8)
	{
	case 1:
		data = m_dma_offset[BIT(offset, 3)][2];
		break;
	case 2:
		data = m_dma_offset[BIT(offset, 3)][3];
		break;
	case 3:
		data = m_dma_offset[BIT(offset, 3)][1];
		break;
	case 7:
		data = m_dma_offset[BIT(offset, 3)][0];
		break;
	}
	return data;
}


WRITE8_MEMBER( at_state::at_page8_w )
{
	m_at_pages[offset % 0x10] = data;

	if (LOG_PORT80 && (offset == 0))
	{
		logerror(" at_page8_w(): Port 80h <== 0x%02x (PC=0x%08x)\n", data,
							(unsigned) cpu_get_reg(m_maincpu, STATE_GENPC));
	}

	switch(offset % 8)
	{
	case 1:
		m_dma_offset[BIT(offset, 3)][2] = data;
		break;
	case 2:
		m_dma_offset[BIT(offset, 3)][3] = data;
		break;
	case 3:
		m_dma_offset[BIT(offset, 3)][1] = data;
		break;
	case 7:
		m_dma_offset[BIT(offset, 3)][0] = data;
		break;
	}
}


WRITE_LINE_MEMBER( at_state::pc_dma_hrq_changed )
{
	device_set_input_line(m_maincpu, INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237_2->hack_w(state);
}

READ8_MEMBER(at_state::pc_dma_read_byte)
{
	if(m_dma_channel == -1)
		return 0xff;
	UINT8 result;
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16) & 0xFF0000;

	result = space.read_byte(page_offset + offset);
	return result;
}


WRITE8_MEMBER(at_state::pc_dma_write_byte)
{
	if(m_dma_channel == -1)
		return;
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16) & 0xFF0000;

	space.write_byte(page_offset + offset, data);
}


READ8_MEMBER(at_state::pc_dma_read_word)
{
	if(m_dma_channel == -1)
		return 0xff;
	UINT16 result;
	offs_t page_offset = (((offs_t) m_dma_offset[1][m_dma_channel & 3]) << 16) & 0xFE0000;

	result = space.read_word(page_offset + ( offset << 1 ) );
	m_dma_high_byte = result & 0xFF00;

	return result & 0xFF;
}


WRITE8_MEMBER(at_state::pc_dma_write_word)
{
	if(m_dma_channel == -1)
		return;
	offs_t page_offset = (((offs_t) m_dma_offset[1][m_dma_channel & 3]) << 16) & 0xFE0000;

	space.write_word(page_offset + ( offset << 1 ), m_dma_high_byte | data);
}

READ8_MEMBER( at_state::pc_dma8237_0_dack_r ) { return m_isabus->dack_r(0); }
READ8_MEMBER( at_state::pc_dma8237_1_dack_r ) { return m_isabus->dack_r(1); }
READ8_MEMBER( at_state::pc_dma8237_2_dack_r ) { return m_isabus->dack_r(2); }
READ8_MEMBER( at_state::pc_dma8237_3_dack_r ) { return m_isabus->dack_r(3); }
READ8_MEMBER( at_state::pc_dma8237_5_dack_r ) { UINT16 ret = m_isabus->dack16_r(5); m_dma_high_byte = ret & 0xff00; return ret; }
READ8_MEMBER( at_state::pc_dma8237_6_dack_r ) { UINT16 ret = m_isabus->dack16_r(6); m_dma_high_byte = ret & 0xff00; return ret; }
READ8_MEMBER( at_state::pc_dma8237_7_dack_r ) { UINT16 ret = m_isabus->dack16_r(7); m_dma_high_byte = ret & 0xff00; return ret; }


WRITE8_MEMBER( at_state::pc_dma8237_0_dack_w ){ m_isabus->dack_w(0, data); }
WRITE8_MEMBER( at_state::pc_dma8237_1_dack_w ){ m_isabus->dack_w(1, data); }
WRITE8_MEMBER( at_state::pc_dma8237_2_dack_w ){ m_isabus->dack_w(2, data); }
WRITE8_MEMBER( at_state::pc_dma8237_3_dack_w ){ m_isabus->dack_w(3, data); }
WRITE8_MEMBER( at_state::pc_dma8237_5_dack_w ){ m_isabus->dack16_w(5, m_dma_high_byte | data); }
WRITE8_MEMBER( at_state::pc_dma8237_6_dack_w ){ m_isabus->dack16_w(6, m_dma_high_byte | data); }
WRITE8_MEMBER( at_state::pc_dma8237_7_dack_w ){ m_isabus->dack16_w(7, m_dma_high_byte | data); }

WRITE_LINE_MEMBER( at_state::at_dma8237_out_eop )
{
	m_cur_eop = state == ASSERT_LINE;
	if(m_dma_channel != -1)
		m_isabus->eop_w(m_dma_channel, m_cur_eop ? ASSERT_LINE : CLEAR_LINE );
}

void at_state::pc_set_dma_channel(int channel, int state)
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

WRITE_LINE_MEMBER( at_state::pc_dack0_w ) { pc_set_dma_channel(0, state); }
WRITE_LINE_MEMBER( at_state::pc_dack1_w ) { pc_set_dma_channel(1, state); }
WRITE_LINE_MEMBER( at_state::pc_dack2_w ) { pc_set_dma_channel(2, state); }
WRITE_LINE_MEMBER( at_state::pc_dack3_w ) { pc_set_dma_channel(3, state); }
WRITE_LINE_MEMBER( at_state::pc_dack4_w ) { m_dma8237_1->hack_w(state ? 0 : 1); } // it's inverted
WRITE_LINE_MEMBER( at_state::pc_dack5_w ) { pc_set_dma_channel(5, state); }
WRITE_LINE_MEMBER( at_state::pc_dack6_w ) { pc_set_dma_channel(6, state); }
WRITE_LINE_MEMBER( at_state::pc_dack7_w ) { pc_set_dma_channel(7, state); }

I8237_INTERFACE( at_dma8237_1_config )
{
	DEVCB_DEVICE_LINE_MEMBER("dma8237_2", am9517a_device, dreq0_w),
	DEVCB_DRIVER_LINE_MEMBER(at_state, at_dma8237_out_eop),
	DEVCB_DRIVER_MEMBER(at_state, pc_dma_read_byte),
	DEVCB_DRIVER_MEMBER(at_state, pc_dma_write_byte),
	{ DEVCB_DRIVER_MEMBER(at_state, pc_dma8237_0_dack_r), DEVCB_DRIVER_MEMBER(at_state, pc_dma8237_1_dack_r), DEVCB_DRIVER_MEMBER(at_state, pc_dma8237_2_dack_r), DEVCB_DRIVER_MEMBER(at_state, pc_dma8237_3_dack_r) },
	{ DEVCB_DRIVER_MEMBER(at_state, pc_dma8237_0_dack_w), DEVCB_DRIVER_MEMBER(at_state, pc_dma8237_1_dack_w), DEVCB_DRIVER_MEMBER(at_state, pc_dma8237_2_dack_w), DEVCB_DRIVER_MEMBER(at_state, pc_dma8237_3_dack_w) },
	{ DEVCB_DRIVER_LINE_MEMBER(at_state, pc_dack0_w), DEVCB_DRIVER_LINE_MEMBER(at_state, pc_dack1_w), DEVCB_DRIVER_LINE_MEMBER(at_state, pc_dack2_w), DEVCB_DRIVER_LINE_MEMBER(at_state, pc_dack3_w) }
};


I8237_INTERFACE( at_dma8237_2_config )
{
	DEVCB_DRIVER_LINE_MEMBER(at_state, pc_dma_hrq_changed),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(at_state, pc_dma_read_word),
	DEVCB_DRIVER_MEMBER(at_state, pc_dma_write_word),
	{ DEVCB_NULL, DEVCB_DRIVER_MEMBER(at_state, pc_dma8237_5_dack_r), DEVCB_DRIVER_MEMBER(at_state, pc_dma8237_6_dack_r), DEVCB_DRIVER_MEMBER(at_state, pc_dma8237_7_dack_r) },
	{ DEVCB_NULL, DEVCB_DRIVER_MEMBER(at_state, pc_dma8237_5_dack_w), DEVCB_DRIVER_MEMBER(at_state, pc_dma8237_6_dack_w), DEVCB_DRIVER_MEMBER(at_state, pc_dma8237_7_dack_w) },
	{ DEVCB_DRIVER_LINE_MEMBER(at_state, pc_dack4_w), DEVCB_DRIVER_LINE_MEMBER(at_state, pc_dack5_w), DEVCB_DRIVER_LINE_MEMBER(at_state, pc_dack6_w), DEVCB_DRIVER_LINE_MEMBER(at_state, pc_dack7_w) }
};

READ8_MEMBER( at_state::at_portb_r )
{
	UINT8 data = m_at_speaker;
	data &= ~0xc0; /* AT BIOS don't likes this being set */

	/* This needs fixing/updating not sure what this is meant to fix */
	if ( --m_poll_delay < 0 )
	{
		m_poll_delay = 3;
		m_at_offset1 ^= 0x10;
	}
	data = (data & ~0x10) | ( m_at_offset1 & 0x10 );

	if ( pit8253_get_output(m_pit8254, 2 ) )
		data |= 0x20;
	else
		data &= ~0x20; /* ps2m30 wants this */

	return data;
}

WRITE8_MEMBER( at_state::at_portb_w )
{
	m_at_speaker = data;
	pit8253_gate2_w(m_pit8254, BIT(data, 0));
	at_speaker_set_spkrdata( BIT(data, 1));
	m_channel_check = BIT(data, 3);
	m_isabus->set_nmi_state((m_nmi_enabled==0) && (m_channel_check==0));
}


/**********************************************************
 *
 * Init functions
 *
 **********************************************************/

static void init_at_common(running_machine &machine)
{
	at_state *state = machine.driver_data<at_state>();
	address_space* space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	// The CS4031 chipset does this itself
	if (machine.device("cs4031") == NULL)
	{
		/* MESS managed RAM */
		state->membank("bank10")->set_base(machine.device<ram_device>(RAM_TAG)->pointer());

		if (machine.device<ram_device>(RAM_TAG)->size() > 0x0a0000)
		{
			offs_t ram_limit = 0x100000 + machine.device<ram_device>(RAM_TAG)->size() - 0x0a0000;
			space->install_read_bank(0x100000,  ram_limit - 1, "bank1");
			space->install_write_bank(0x100000,  ram_limit - 1, "bank1");
			state->membank("bank1")->set_base(machine.device<ram_device>(RAM_TAG)->pointer() + 0xa0000);
		}
	}

	state->m_at_offset1 = 0xff;
}

DRIVER_INIT_MEMBER(at_state,atcga)
{
	init_at_common(machine());
}

DRIVER_INIT_MEMBER(at_state,atvga)
{
	init_at_common(machine());
}

static IRQ_CALLBACK(at_irq_callback)
{
	at_state *st = device->machine().driver_data<at_state>();
	return pic8259_acknowledge(st->m_pic8259_master);
}

MACHINE_START( at )
{
	device_set_irq_callback(machine.device("maincpu"), at_irq_callback);
}

MACHINE_RESET( at )
{
	at_state *st = machine.driver_data<at_state>();
	st->m_poll_delay = 4;
	st->m_at_spkrdata = 0;
	st->m_at_speaker_input = 0;
	st->m_dma_channel = -1;
	st->m_cur_eop = false;
}
