/***************************************************************************

  Southbridge implementation

***************************************************************************/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "southbridge.h"
#include "bus/pc_kbd/keyboards.h"


I8237_INTERFACE( at_dma8237_1_config )
{
	DEVCB_DEVICE_LINE_MEMBER("dma8237_2",am9517a_device,dreq0_w),
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

static const at_keyboard_controller_interface keyboard_controller_intf =
{
	DEVCB_CPU_INPUT_LINE(":maincpu", INPUT_LINE_RESET),
	DEVCB_CPU_INPUT_LINE(":maincpu", INPUT_LINE_A20),
	DEVCB_DEVICE_LINE_MEMBER("pic8259_master", pic8259_device, ir1_w),
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
	DEVCB_DEVICE_LINE_MEMBER("pic8259_slave",  pic8259_device, ir2_w), // in place of irq 2 on at irq 9 is used
	DEVCB_DEVICE_LINE_MEMBER("pic8259_master", pic8259_device, ir3_w),
	DEVCB_DEVICE_LINE_MEMBER("pic8259_master", pic8259_device, ir4_w),
	DEVCB_DEVICE_LINE_MEMBER("pic8259_master", pic8259_device, ir5_w),
	DEVCB_DEVICE_LINE_MEMBER("pic8259_master", pic8259_device, ir6_w),
	DEVCB_DEVICE_LINE_MEMBER("pic8259_master", pic8259_device, ir7_w),

	DEVCB_DEVICE_LINE_MEMBER("pic8259_slave", pic8259_device, ir3_w),
	DEVCB_DEVICE_LINE_MEMBER("pic8259_slave", pic8259_device, ir4_w),
	DEVCB_DEVICE_LINE_MEMBER("pic8259_slave", pic8259_device, ir5_w),
	DEVCB_DEVICE_LINE_MEMBER("pic8259_slave", pic8259_device, ir6_w),
	DEVCB_DEVICE_LINE_MEMBER("pic8259_slave", pic8259_device, ir7_w),

	// dma request
	DEVCB_DEVICE_LINE_MEMBER("dma8237_1", am9517a_device, dreq0_w),
	DEVCB_DEVICE_LINE_MEMBER("dma8237_1", am9517a_device, dreq1_w),
	DEVCB_DEVICE_LINE_MEMBER("dma8237_1", am9517a_device, dreq2_w),
	DEVCB_DEVICE_LINE_MEMBER("dma8237_1", am9517a_device, dreq3_w),

	DEVCB_DEVICE_LINE_MEMBER("dma8237_2", am9517a_device, dreq1_w),
	DEVCB_DEVICE_LINE_MEMBER("dma8237_2", am9517a_device, dreq2_w),
	DEVCB_DEVICE_LINE_MEMBER("dma8237_2", am9517a_device, dreq3_w),
};

static SLOT_INTERFACE_START(pc_isa_onboard)
	SLOT_INTERFACE("comat", ISA8_COM_AT)
	SLOT_INTERFACE("lpt", ISA8_LPT)
	SLOT_INTERFACE("fdcsmc", ISA8_FDC_SMC)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT( southbridge )
	MCFG_DEVICE_ADD("pit8254", PIT8254, 0)
	MCFG_PIT8253_CLK0(4772720/4) /* heartbeat IRQ */
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(southbridge_device, at_pit8254_out0_changed))
	MCFG_PIT8253_CLK1(4772720/4) /* dram refresh */
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(southbridge_device, at_pit8254_out1_changed))
	MCFG_PIT8253_CLK2(4772720/4) /* pio port c pin 4, and speaker polling enough */
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(southbridge_device, at_pit8254_out2_changed))

	MCFG_I8237_ADD( "dma8237_1", XTAL_14_31818MHz/3, at_dma8237_1_config )
	MCFG_I8237_ADD( "dma8237_2", XTAL_14_31818MHz/3, at_dma8237_2_config )

	MCFG_PIC8259_ADD( "pic8259_master", INPUTLINE(":maincpu", 0), VCC, READ8(southbridge_device, get_slave_ack) )
	MCFG_PIC8259_ADD( "pic8259_slave", DEVWRITELINE("pic8259_master", pic8259_device, ir2_w), GND, NULL )

	MCFG_AT_KEYBOARD_CONTROLLER_ADD("keybc", XTAL_12MHz, keyboard_controller_intf)
	MCFG_PC_KBDC_ADD("pc_kbdc", pc_kbdc_intf)
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL)

	MCFG_DS12885_ADD("rtc")
	MCFG_MC146818_IRQ_HANDLER(DEVWRITELINE("pic8259_slave", pic8259_device, ir0_w))
	MCFG_MC146818_CENTURY_INDEX(0x32)

	MCFG_BUS_MASTER_IDE_CONTROLLER_ADD("ide", ata_devices, "hdd", NULL, false)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(DEVWRITELINE("pic8259_slave", pic8259_device, ir6_w))
	MCFG_BUS_MASTER_IDE_CONTROLLER_SPACE(":maincpu", AS_PROGRAM)

	MCFG_BUS_MASTER_IDE_CONTROLLER_ADD("ide2", ata_devices, "cdrom", NULL, false)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(DEVWRITELINE("pic8259_slave", pic8259_device, ir7_w))
	MCFG_BUS_MASTER_IDE_CONTROLLER_SPACE(":maincpu", AS_PROGRAM)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_ISA16_BUS_ADD("isabus", ":maincpu", isabus_intf)
	// on board devices
	MCFG_ISA16_SLOT_ADD("isabus","board1", pc_isa_onboard, "fdcsmc", true)
	MCFG_ISA16_SLOT_ADD("isabus","board2", pc_isa_onboard, "comat", true)
	MCFG_ISA16_SLOT_ADD("isabus","board3", pc_isa_onboard, "lpt", true)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor southbridge_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( southbridge );
}

southbridge_device::southbridge_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	m_maincpu(*this, ":maincpu"),
	m_pic8259_master(*this, "pic8259_master"),
	m_pic8259_slave(*this, "pic8259_slave"),
	m_dma8237_1(*this, "dma8237_1"),
	m_dma8237_2(*this, "dma8237_2"),
	m_pit8254(*this, "pit8254"),
	m_keybc(*this, "keybc"),
	m_isabus(*this, "isabus"),
	m_speaker(*this, "speaker"),
	m_ds12885(*this, "rtc"),
	m_pc_kbdc(*this, "pc_kbdc"),
	m_ide(*this, "ide"),
	m_ide2(*this, "ide2")
{
}
/**********************************************************
 *
 * Init functions
 *
 **********************************************************/

IRQ_CALLBACK_MEMBER(southbridge_device::at_irq_callback)
{
	return m_pic8259_master->inta_r();
}

/// HACK: the memory system cannot cope with mixing the  8 bit device map from the fdc with a 32 bit handler
READ8_MEMBER(southbridge_device::ide_read_cs1_r)
{
	return m_ide->read_cs1(space, 1, (UINT32) 0xff0000) >> 16;
}

WRITE8_MEMBER(southbridge_device::ide_write_cs1_w)
{
	m_ide->write_cs1(space, 1, (UINT32) data << 16, (UINT32) 0xff0000);
}

READ8_MEMBER(southbridge_device::ide2_read_cs1_r)
{
	return m_ide2->read_cs1(space, 1, (UINT32) 0xff0000) >> 16;
}

WRITE8_MEMBER(southbridge_device::ide2_write_cs1_w)
{
	m_ide2->write_cs1(space, 1, (UINT32) data << 16, (UINT32) 0xff0000);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void southbridge_device::device_start()
{
	address_space& spaceio = machine().device(":maincpu")->memory().space(AS_IO);

	spaceio.install_readwrite_handler(0x0000, 0x001f, read8_delegate(FUNC(am9517a_device::read),&(*m_dma8237_1)), write8_delegate(FUNC(am9517a_device::write),&(*m_dma8237_1)), 0xffffffff);
	spaceio.install_readwrite_handler(0x0020, 0x003f, read8_delegate(FUNC(pic8259_device::read),&(*m_pic8259_master)), write8_delegate(FUNC(pic8259_device::write),&(*m_pic8259_master)), 0xffffffff);
	spaceio.install_readwrite_handler(0x0040, 0x005f, read8_delegate(FUNC(pit8254_device::read),&(*m_pit8254)), write8_delegate(FUNC(pit8254_device::write),&(*m_pit8254)), 0xffffffff);
	spaceio.install_readwrite_handler(0x0060, 0x0063, read8_delegate(FUNC(southbridge_device::at_keybc_r),this), write8_delegate(FUNC(southbridge_device::at_keybc_w),this), 0xffffffff);
	spaceio.install_readwrite_handler(0x0064, 0x0067, read8_delegate(FUNC(at_keyboard_controller_device::status_r),&(*m_keybc)), write8_delegate(FUNC(at_keyboard_controller_device::command_w),&(*m_keybc)), 0xffffffff);
	spaceio.install_readwrite_handler(0x0070, 0x007f, read8_delegate(FUNC(ds12885_device::read),&(*m_ds12885)), write8_delegate(FUNC(ds12885_device::write),&(*m_ds12885)), 0xffffffff);
	spaceio.install_readwrite_handler(0x0080, 0x009f, read8_delegate(FUNC(southbridge_device::at_page8_r),this), write8_delegate(FUNC(southbridge_device::at_page8_w),this), 0xffffffff);
	spaceio.install_readwrite_handler(0x00a0, 0x00bf, read8_delegate(FUNC(pic8259_device::read),&(*m_pic8259_slave)), write8_delegate(FUNC(pic8259_device::write),&(*m_pic8259_slave)), 0xffffffff);
	spaceio.install_readwrite_handler(0x00c0, 0x00df, read8_delegate(FUNC(southbridge_device::at_dma8237_2_r),this), write8_delegate(FUNC(southbridge_device::at_dma8237_2_w),this), 0xffffffff);
	spaceio.install_readwrite_handler(0x0170, 0x0177, read32_delegate(FUNC(bus_master_ide_controller_device::read_cs0),&(*m_ide2)), write32_delegate(FUNC(bus_master_ide_controller_device::write_cs0), &(*m_ide2)),0xffffffff);
	spaceio.install_readwrite_handler(0x01f0, 0x01f7, read32_delegate(FUNC(bus_master_ide_controller_device::read_cs0),&(*m_ide)), write32_delegate(FUNC(bus_master_ide_controller_device::write_cs0), &(*m_ide)),0xffffffff);
//  HACK: this works if you take out the (non working) fdc
//  spaceio.install_readwrite_handler(0x0370, 0x0377, read32_delegate(FUNC(bus_master_ide_controller_device::read_cs1),&(*m_ide2)), write32_delegate(FUNC(bus_master_ide_controller_device::write_cs1), &(*m_ide2)),0xffffffff);
//  spaceio.install_readwrite_handler(0x03f0, 0x03f7, read32_delegate(FUNC(bus_master_ide_controller_device::read_cs1),&(*m_ide)), write32_delegate(FUNC(bus_master_ide_controller_device::write_cs1), &(*m_ide)),0xffffffff);
	spaceio.install_readwrite_handler(0x0374, 0x0377, read8_delegate(FUNC(southbridge_device::ide2_read_cs1_r),this), write8_delegate(FUNC(southbridge_device::ide2_write_cs1_w), this),0xff0000);
	spaceio.install_readwrite_handler(0x03f4, 0x03f7, read8_delegate(FUNC(southbridge_device::ide_read_cs1_r),this), write8_delegate(FUNC(southbridge_device::ide_write_cs1_w), this),0xff0000);
	spaceio.nop_readwrite(0x00e0, 0x00ef);


	machine().device(":maincpu")->execute().set_irq_acknowledge_callback(device_irq_acknowledge_delegate(FUNC(southbridge_device::at_irq_callback),this));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void southbridge_device::device_reset()
{
	m_at_spkrdata = 0;
	m_pit_out2 = 0;
	m_dma_channel = -1;
	m_cur_eop = false;
	m_nmi_enabled = 0;
	m_refresh = false;
}


/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/
READ8_MEMBER( southbridge_device::get_slave_ack )
{
	if (offset==2) // IRQ = 2
		return m_pic8259_slave->inta_r();

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
	m_speaker->level_w(m_at_spkrdata & m_pit_out2);
}



/*************************************************************
 *
 * pit8254 configuration
 *
 *************************************************************/

WRITE_LINE_MEMBER( southbridge_device::at_pit8254_out0_changed )
{
	if (m_pic8259_master)
		m_pic8259_master->ir0_w(state);
}

WRITE_LINE_MEMBER( southbridge_device::at_pit8254_out1_changed )
{
	if(state)
		m_refresh = !m_refresh;
}

WRITE_LINE_MEMBER( southbridge_device::at_pit8254_out2_changed )
{
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(m_at_spkrdata & m_pit_out2);
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
	m_dma8237_2->hack_w( state );
}

READ8_MEMBER(southbridge_device::pc_dma_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return 0xff;
	UINT8 result;
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16) & 0xFF0000;

	result = prog_space.read_byte(page_offset + offset);
	return result;
}


WRITE8_MEMBER(southbridge_device::pc_dma_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return;
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16) & 0xFF0000;

	prog_space.write_byte(page_offset + offset, data);
}


READ8_MEMBER(southbridge_device::pc_dma_read_word)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return 0xff;
	UINT16 result;
	offs_t page_offset = (((offs_t) m_dma_offset[1][m_dma_channel & 3]) << 16) & 0xFE0000;

	result = prog_space.read_word(page_offset + ( offset << 1 ) );
	m_dma_high_byte = result & 0xFF00;

	return result & 0xFF;
}


WRITE8_MEMBER(southbridge_device::pc_dma_write_word)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return;
	offs_t page_offset = (((offs_t) m_dma_offset[1][m_dma_channel & 3]) << 16) & 0xFE0000;

	prog_space.write_word(page_offset + ( offset << 1 ), m_dma_high_byte | data);
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
WRITE_LINE_MEMBER( southbridge_device::pc_dack4_w ) { m_dma8237_1->hack_w( state ? 0 : 1); } // it's inverted
WRITE_LINE_MEMBER( southbridge_device::pc_dack5_w ) { pc_select_dma_channel(5, state); }
WRITE_LINE_MEMBER( southbridge_device::pc_dack6_w ) { pc_select_dma_channel(6, state); }
WRITE_LINE_MEMBER( southbridge_device::pc_dack7_w ) { pc_select_dma_channel(7, state); }

READ8_MEMBER( southbridge_device::at_portb_r )
{
	UINT8 data = m_at_speaker;
	data &= ~0xd0; /* AT BIOS don't likes this being set */

	/* 0x10 is the dram refresh line bit on the 5170, just a timer here, 15.085us. */
	data |= m_refresh ? 0x10 : 0;

	if (m_pit_out2)
		data |= 0x20;
	else
		data &= ~0x20; /* ps2m30 wants this */

	return data;
}

WRITE8_MEMBER( southbridge_device::at_portb_w )
{
	m_at_speaker = data;
	m_pit8254->write_gate2(BIT(data, 0));
	at_speaker_set_spkrdata( BIT(data, 1));
	m_channel_check = BIT(data, 3);
	m_isabus->set_nmi_state((m_nmi_enabled==0) && (m_channel_check==0));
}

READ8_MEMBER( southbridge_device::at_dma8237_2_r )
{
	return m_dma8237_2->read( space, offset / 2);
}

WRITE8_MEMBER( southbridge_device::at_dma8237_2_w )
{
	m_dma8237_2->write( space, offset / 2, data);
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
		m_ds12885->write(space,0,data);
	}
	else {
		m_ds12885->write(space,offset,data);
	}
}
