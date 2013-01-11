/***************************************************************************

  Southbridge implementation

***************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/southbridge.h"
#include "machine/pc_keyboards.h"
#include "machine/8237dma.h"

const struct pic8259_interface at_pic8259_master_config =
{
	DEVCB_CPU_INPUT_LINE(":maincpu", 0),
	DEVCB_LINE_VCC,
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, get_slave_ack)
};

const struct pic8259_interface at_pic8259_slave_config =
{
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir2_w),
	DEVCB_LINE_GND,
	DEVCB_NULL
};

const struct pit8253_config at_pit8254_config =
{
	{
		{
			4772720/4,              /* heartbeat IRQ */
			DEVCB_NULL,
			DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, at_pit8254_out0_changed)
		}, {
			4772720/4,              /* dram refresh */
			DEVCB_NULL,
			DEVCB_NULL
		}, {
			4772720/4,              /* pio port c pin 4, and speaker polling enough */
			DEVCB_NULL,
			DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, at_pit8254_out2_changed)
		}
	}
};

I8237_INTERFACE( at_dma8237_1_config )
{
	DEVCB_DEVICE_LINE("dma8237_2",i8237_dreq0_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, at_dma8237_out_eop),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dma_read_byte),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dma_write_byte),
	{ DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dma8237_0_dack_r), DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dma8237_1_dack_r), DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dma8237_2_dack_r), DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dma8237_3_dack_r) },
	{ DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dma8237_0_dack_w), DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dma8237_1_dack_w), DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dma8237_2_dack_w), DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dma8237_3_dack_w) },
	{ DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dack0_w), DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dack1_w), DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dack2_w), DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dack3_w) }
};


I8237_INTERFACE( at_dma8237_2_config )
{
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dma_hrq_changed),
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dma_read_word),
	DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dma_write_word),
	{ DEVCB_NULL, DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dma8237_5_dack_r), DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dma8237_6_dack_r), DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dma8237_7_dack_r) },
	{ DEVCB_NULL, DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dma8237_5_dack_w), DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dma8237_6_dack_w), DEVCB_DEVICE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dma8237_7_dack_w) },
	{ DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dack4_w), DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dack5_w), DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dack6_w), DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, pc_dack7_w) }
};

const struct mc146818_interface at_mc146818_config =
{
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, southbridge_device, at_mc146818_irq)
};

static const at_keyboard_controller_interface keyboard_controller_intf =
{
	DEVCB_CPU_INPUT_LINE(":maincpu", INPUT_LINE_RESET),
	DEVCB_CPU_INPUT_LINE(":maincpu", INPUT_LINE_A20),
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir1_w),
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER("pc_kbdc", pc_kbdc_device, clock_write_from_mb),
	DEVCB_DEVICE_LINE_MEMBER("pc_kbdc", pc_kbdc_device, data_write_from_mb)
};

static const pc_kbdc_interface pc_kbdc_intf =
{
	DEVCB_DEVICE_LINE_MEMBER("keybc", at_keyboard_controller_device, keyboard_clock_w),
	DEVCB_DEVICE_LINE_MEMBER("keybc", at_keyboard_controller_device, keyboard_data_w)
};

static const isa16bus_interface isabus_intf =
{
	// interrupts
	DEVCB_DEVICE_LINE("pic8259_slave",  pic8259_ir2_w), // in place of irq 2 on at irq 9 is used
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir3_w),
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir4_w),
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir5_w),
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir6_w),
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir7_w),

	DEVCB_DEVICE_LINE("pic8259_slave", pic8259_ir3_w),
	DEVCB_DEVICE_LINE("pic8259_slave", pic8259_ir4_w),
	DEVCB_DEVICE_LINE("pic8259_slave", pic8259_ir5_w),
	DEVCB_DEVICE_LINE("pic8259_slave", pic8259_ir6_w),
	DEVCB_DEVICE_LINE("pic8259_slave", pic8259_ir7_w),

	// dma request
	DEVCB_DEVICE_LINE("dma8237_1", i8237_dreq0_w),
	DEVCB_DEVICE_LINE("dma8237_1", i8237_dreq1_w),
	DEVCB_DEVICE_LINE("dma8237_1", i8237_dreq2_w),
	DEVCB_DEVICE_LINE("dma8237_1", i8237_dreq3_w),

	DEVCB_DEVICE_LINE("dma8237_2", i8237_dreq1_w),
	DEVCB_DEVICE_LINE("dma8237_2", i8237_dreq2_w),
	DEVCB_DEVICE_LINE("dma8237_2", i8237_dreq3_w),
};

static SLOT_INTERFACE_START(pc_isa_onboard)
	SLOT_INTERFACE("comat", ISA8_COM_AT)
	SLOT_INTERFACE("lpt", ISA8_LPT)
	SLOT_INTERFACE("fdcsmc", ISA8_FDC_SMC)
	SLOT_INTERFACE("ide", ISA16_IDE)
	SLOT_INTERFACE("ide_cd", ISA16_IDE_CD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( southbridge )
	MCFG_PIT8254_ADD( "pit8254", at_pit8254_config )

	MCFG_I8237_ADD( "dma8237_1", XTAL_14_31818MHz/3, at_dma8237_1_config )
	MCFG_I8237_ADD( "dma8237_2", XTAL_14_31818MHz/3, at_dma8237_2_config )

	MCFG_PIC8259_ADD( "pic8259_master", at_pic8259_master_config )
	MCFG_PIC8259_ADD( "pic8259_slave", at_pic8259_slave_config )

	MCFG_AT_KEYBOARD_CONTROLLER_ADD("keybc", XTAL_12MHz, keyboard_controller_intf)
	MCFG_PC_KBDC_ADD("pc_kbdc", pc_kbdc_intf)
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL, NULL)

	MCFG_MC146818_IRQ_ADD( "rtc", MC146818_STANDARD, at_mc146818_config )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_ISA16_BUS_ADD("isabus", ":maincpu", isabus_intf)
	// on board devices
	MCFG_ISA16_SLOT_ADD("isabus","board1", pc_isa_onboard, "fdcsmc", NULL, true)
	MCFG_ISA16_SLOT_ADD("isabus","board2", pc_isa_onboard, "comat", NULL, true)
	MCFG_ISA16_SLOT_ADD("isabus","board3", pc_isa_onboard, "ide", NULL, true)
	MCFG_ISA16_SLOT_ADD("isabus","board4", pc_isa_onboard, "lpt", NULL, true)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor southbridge_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( southbridge );
}

southbridge_device::southbridge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock),
	m_maincpu(*this, ":maincpu"),
	m_pic8259_master(*this, "pic8259_master"),
	m_pic8259_slave(*this, "pic8259_slave"),
	m_dma8237_1(*this, "dma8237_1"),
	m_dma8237_2(*this, "dma8237_2"),
	m_pit8254(*this, "pit8254"),
	m_keybc(*this, "keybc"),
	m_isabus(*this, "isabus"),
	m_speaker(*this, SPEAKER_TAG),
	m_mc146818(*this, "rtc"),
	m_pc_kbdc(*this, "pc_kbdc")
{
}
/**********************************************************
 *
 * Init functions
 *
 **********************************************************/
/*
IRQ_CALLBACK(southbridge_device::at_irq_callback)
{
    device_t *pic = device->machine().device(":pcibus:1:i82371ab:pic8259_master");
    //return pic8259_acknowledge(m_pic8259_master);
    return pic8259_acknowledge(pic);
}
*/
//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void southbridge_device::device_start()
{
	address_space& spaceio = machine().device(":maincpu")->memory().space(AS_IO);

	spaceio.install_legacy_readwrite_handler(*m_dma8237_1, 0x0000, 0x001f, FUNC(i8237_r), FUNC(i8237_w), 0xffffffff);
	spaceio.install_legacy_readwrite_handler(*m_pic8259_master, 0x0020, 0x003f, FUNC(pic8259_r), FUNC(pic8259_w), 0xffffffff);
	spaceio.install_legacy_readwrite_handler(*m_pit8254, 0x0040, 0x005f, FUNC(pit8253_r), FUNC(pit8253_w), 0xffffffff);
	spaceio.install_readwrite_handler(0x0060, 0x0063, read8_delegate(FUNC(southbridge_device::at_keybc_r),this), write8_delegate(FUNC(southbridge_device::at_keybc_w),this), 0xffffffff);
	spaceio.install_readwrite_handler(0x0064, 0x0067, read8_delegate(FUNC(at_keyboard_controller_device::status_r),&(*m_keybc)), write8_delegate(FUNC(at_keyboard_controller_device::command_w),&(*m_keybc)), 0xffffffff);
	spaceio.install_readwrite_handler(0x0070, 0x007f, read8_delegate(FUNC(mc146818_device::read),&(*m_mc146818)), write8_delegate(FUNC(mc146818_device::write),&(*m_mc146818)), 0xffffffff);
	spaceio.install_readwrite_handler(0x0080, 0x009f, read8_delegate(FUNC(southbridge_device::at_page8_r),this), write8_delegate(FUNC(southbridge_device::at_page8_w),this), 0xffffffff);
	spaceio.install_legacy_readwrite_handler(*m_pic8259_slave, 0x00a0, 0x00bf, FUNC(pic8259_r), FUNC(pic8259_w), 0xffffffff);
	spaceio.install_readwrite_handler(0x00c0, 0x00df, read8_delegate(FUNC(southbridge_device::at_dma8237_2_r),this), write8_delegate(FUNC(southbridge_device::at_dma8237_2_w),this), 0xffffffff);
	spaceio.nop_readwrite(0x00e0, 0x00ef);


	m_at_offset1 = 0xff;
	//machine().device(":maincpu")->execute().set_irq_acknowledge_callback(at_irq_callback);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void southbridge_device::device_reset()
{
	m_poll_delay = 4;
	m_at_spkrdata = 0;
	m_at_speaker_input = 0;
	m_dma_channel = -1;
	m_cur_eop = false;
}


/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/
READ8_MEMBER( southbridge_device::get_slave_ack )
{
	if (offset==2) // IRQ = 2
		return pic8259_acknowledge(m_pic8259_slave);

	return 0x00;
}

/*************************************************************************
 *
 *      PC Speaker related
 *
 *************************************************************************/

void southbridge_device::at_speaker_set_spkrdata(UINT8 data)
{
	m_at_spkrdata = data ? 1 : 0;
	speaker_level_w( m_speaker, m_at_spkrdata & m_at_speaker_input);
}

void southbridge_device::at_speaker_set_input(UINT8 data)
{
	m_at_speaker_input = data ? 1 : 0;
	speaker_level_w( m_speaker, m_at_spkrdata & m_at_speaker_input);
}



/*************************************************************
 *
 * pit8254 configuration
 *
 *************************************************************/

WRITE_LINE_MEMBER( southbridge_device::at_pit8254_out0_changed )
{
	if (m_pic8259_master)
		pic8259_ir0_w(m_pic8259_master, state);
}


WRITE_LINE_MEMBER( southbridge_device::at_pit8254_out2_changed )
{
	at_speaker_set_input( state );
}

/*************************************************************************
 *
 *      PC DMA stuff
 *
 *************************************************************************/

READ8_MEMBER( southbridge_device::at_page8_r )
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


WRITE8_MEMBER( southbridge_device::at_page8_w )
{
	m_at_pages[offset % 0x10] = data;

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


WRITE_LINE_MEMBER( southbridge_device::pc_dma_hrq_changed )
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	i8237_hlda_w( m_dma8237_2, state );
}

READ8_MEMBER(southbridge_device::pc_dma_read_byte)
{
	if(m_dma_channel == -1)
		return 0xff;
	UINT8 result;
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16) & 0xFF0000;

	result = space.read_byte(page_offset + offset);
	return result;
}


WRITE8_MEMBER(southbridge_device::pc_dma_write_byte)
{
	if(m_dma_channel == -1)
		return;
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16) & 0xFF0000;

	space.write_byte(page_offset + offset, data);
}


READ8_MEMBER(southbridge_device::pc_dma_read_word)
{
	if(m_dma_channel == -1)
		return 0xff;
	UINT16 result;
	offs_t page_offset = (((offs_t) m_dma_offset[1][m_dma_channel & 3]) << 16) & 0xFF0000;

	result = space.read_word(page_offset + ( offset << 1 ) );
	m_dma_high_byte = result & 0xFF00;

	return result & 0xFF;
}


WRITE8_MEMBER(southbridge_device::pc_dma_write_word)
{
	if(m_dma_channel == -1)
		return;
	offs_t page_offset = (((offs_t) m_dma_offset[1][m_dma_channel & 3]) << 16) & 0xFF0000;

	space.write_word(page_offset + ( offset << 1 ), m_dma_high_byte | data);
}


READ8_MEMBER( southbridge_device::pc_dma8237_0_dack_r ) { return m_isabus->dack_r(0); }
READ8_MEMBER( southbridge_device::pc_dma8237_1_dack_r ) { return m_isabus->dack_r(1); }
READ8_MEMBER( southbridge_device::pc_dma8237_2_dack_r ) { return m_isabus->dack_r(2); }
READ8_MEMBER( southbridge_device::pc_dma8237_3_dack_r ) { return m_isabus->dack_r(3); }
READ8_MEMBER( southbridge_device::pc_dma8237_5_dack_r ) { return m_isabus->dack_r(5); }
READ8_MEMBER( southbridge_device::pc_dma8237_6_dack_r ) { return m_isabus->dack_r(6); }
READ8_MEMBER( southbridge_device::pc_dma8237_7_dack_r ) { return m_isabus->dack_r(7); }


WRITE8_MEMBER( southbridge_device::pc_dma8237_0_dack_w ){ m_isabus->dack_w(0, data); }
WRITE8_MEMBER( southbridge_device::pc_dma8237_1_dack_w ){ m_isabus->dack_w(1, data); }
WRITE8_MEMBER( southbridge_device::pc_dma8237_2_dack_w ){ m_isabus->dack_w(2, data); }
WRITE8_MEMBER( southbridge_device::pc_dma8237_3_dack_w ){ m_isabus->dack_w(3, data); }
WRITE8_MEMBER( southbridge_device::pc_dma8237_5_dack_w ){ m_isabus->dack_w(5, data); }
WRITE8_MEMBER( southbridge_device::pc_dma8237_6_dack_w ){ m_isabus->dack_w(6, data); }
WRITE8_MEMBER( southbridge_device::pc_dma8237_7_dack_w ){ m_isabus->dack_w(7, data); }

WRITE_LINE_MEMBER( southbridge_device::at_dma8237_out_eop )
{
	m_cur_eop = state == ASSERT_LINE;
	if(m_dma_channel != -1)
		m_isabus->eop_w(m_dma_channel, m_cur_eop ? ASSERT_LINE : CLEAR_LINE );
}

void southbridge_device::pc_select_dma_channel(int channel, bool state)
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


WRITE_LINE_MEMBER( southbridge_device::pc_dack0_w ) { pc_select_dma_channel(0, state); }
WRITE_LINE_MEMBER( southbridge_device::pc_dack1_w ) { pc_select_dma_channel(1, state); }
WRITE_LINE_MEMBER( southbridge_device::pc_dack2_w ) { pc_select_dma_channel(2, state); }
WRITE_LINE_MEMBER( southbridge_device::pc_dack3_w ) { pc_select_dma_channel(3, state); }
WRITE_LINE_MEMBER( southbridge_device::pc_dack4_w ) { i8237_hlda_w( m_dma8237_1, state ? 0 : 1); } // it's inverted
WRITE_LINE_MEMBER( southbridge_device::pc_dack5_w ) { pc_select_dma_channel(5, state); }
WRITE_LINE_MEMBER( southbridge_device::pc_dack6_w ) { pc_select_dma_channel(6, state); }
WRITE_LINE_MEMBER( southbridge_device::pc_dack7_w ) { pc_select_dma_channel(7, state); }

READ8_MEMBER( southbridge_device::at_portb_r )
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

WRITE8_MEMBER( southbridge_device::at_portb_w )
{
	m_at_speaker = data;
	pit8253_gate2_w(m_pit8254, BIT(data, 0));
	at_speaker_set_spkrdata( BIT(data, 1));
	m_channel_check = BIT(data, 3);
	m_isabus->set_nmi_state((m_nmi_enabled==0) && (m_channel_check==0));
}

WRITE_LINE_MEMBER( southbridge_device::at_mc146818_irq )
{
	pic8259_ir0_w(m_pic8259_slave, (state) ? 0 : 1);
}

READ8_MEMBER( southbridge_device::at_dma8237_2_r )
{
	return i8237_r( m_dma8237_2, space, offset / 2);
}

WRITE8_MEMBER( southbridge_device::at_dma8237_2_w )
{
	i8237_w( m_dma8237_2, space, offset / 2, data);
}

READ8_MEMBER( southbridge_device::at_keybc_r )
{
	switch (offset)
	{
	case 0: return m_keybc->data_r(space, 0);
	case 1: return at_portb_r(space, 0);
	}

	return 0xff;
}

WRITE8_MEMBER( southbridge_device::at_keybc_w )
{
	switch (offset)
	{
	case 0: m_keybc->data_w(space, 0, data); break;
	case 1: at_portb_w(space, 0, data); break;
	}
}


WRITE8_MEMBER( southbridge_device::write_rtc )
{
	if (offset==0) {
		m_nmi_enabled = BIT(data,7);
		m_isabus->set_nmi_state((m_nmi_enabled==0) && (m_channel_check==0));
		m_mc146818->write(space,0,data);
	}
	else {
		m_mc146818->write(space,offset,data);
	}
}
