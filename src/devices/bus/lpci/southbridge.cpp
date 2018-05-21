// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

  Southbridge implementation

***************************************************************************/

#include "emu.h"
#include "southbridge.h"

#include "bus/isa/com.h"
#include "bus/isa/fdc.h"
#include "bus/isa/lpt.h"
#include "bus/pc_kbd/keyboards.h"
#include "cpu/i386/i386.h"
#include "speaker.h"


/***************************************************************************
  Southbridge Device
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(southbridge_device::device_add_mconfig)
	MCFG_DEVICE_ADD("pit8254", PIT8254, 0)
	MCFG_PIT8253_CLK0(4772720/4) // heartbeat IRQ
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(*this, southbridge_device, at_pit8254_out0_changed))
	MCFG_PIT8253_CLK1(4772720/4) // dram refresh
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(*this, southbridge_device, at_pit8254_out1_changed))
	MCFG_PIT8253_CLK2(4772720/4) // pio port c pin 4, and speaker polling enough
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(*this, southbridge_device, at_pit8254_out2_changed))

	MCFG_DEVICE_ADD( "dma8237_1", AM9517A, XTAL(14'318'181)/3 )
	MCFG_I8237_OUT_HREQ_CB(WRITELINE("dma8237_2", am9517a_device, dreq0_w))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(*this, southbridge_device, at_dma8237_out_eop))
	MCFG_I8237_IN_MEMR_CB(READ8(*this, southbridge_device, pc_dma_read_byte))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(*this, southbridge_device, pc_dma_write_byte))
	MCFG_I8237_IN_IOR_0_CB(READ8(*this, southbridge_device, pc_dma8237_0_dack_r))
	MCFG_I8237_IN_IOR_1_CB(READ8(*this, southbridge_device, pc_dma8237_1_dack_r))
	MCFG_I8237_IN_IOR_2_CB(READ8(*this, southbridge_device, pc_dma8237_2_dack_r))
	MCFG_I8237_IN_IOR_3_CB(READ8(*this, southbridge_device, pc_dma8237_3_dack_r))
	MCFG_I8237_OUT_IOW_0_CB(WRITE8(*this, southbridge_device, pc_dma8237_0_dack_w))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(*this, southbridge_device, pc_dma8237_1_dack_w))
	MCFG_I8237_OUT_IOW_2_CB(WRITE8(*this, southbridge_device, pc_dma8237_2_dack_w))
	MCFG_I8237_OUT_IOW_3_CB(WRITE8(*this, southbridge_device, pc_dma8237_3_dack_w))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(*this, southbridge_device, pc_dack0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(*this, southbridge_device, pc_dack1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(*this, southbridge_device, pc_dack2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(*this, southbridge_device, pc_dack3_w))

	MCFG_DEVICE_ADD( "dma8237_2", AM9517A, XTAL(14'318'181)/3 )
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(*this, southbridge_device, pc_dma_hrq_changed))
	MCFG_I8237_IN_MEMR_CB(READ8(*this, southbridge_device, pc_dma_read_word))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(*this, southbridge_device, pc_dma_write_word))
	MCFG_I8237_IN_IOR_1_CB(READ8(*this, southbridge_device, pc_dma8237_5_dack_r))
	MCFG_I8237_IN_IOR_2_CB(READ8(*this, southbridge_device, pc_dma8237_6_dack_r))
	MCFG_I8237_IN_IOR_3_CB(READ8(*this, southbridge_device, pc_dma8237_7_dack_r))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(*this, southbridge_device, pc_dma8237_5_dack_w))
	MCFG_I8237_OUT_IOW_2_CB(WRITE8(*this, southbridge_device, pc_dma8237_6_dack_w))
	MCFG_I8237_OUT_IOW_3_CB(WRITE8(*this, southbridge_device, pc_dma8237_7_dack_w))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(*this, southbridge_device, pc_dack4_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(*this, southbridge_device, pc_dack5_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(*this, southbridge_device, pc_dack6_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(*this, southbridge_device, pc_dack7_w))

	MCFG_DEVICE_ADD("pic8259_master", PIC8259, 0)
	MCFG_PIC8259_OUT_INT_CB(INPUTLINE(":maincpu", 0))
	MCFG_PIC8259_IN_SP_CB(VCC)
	MCFG_PIC8259_CASCADE_ACK_CB(READ8(*this, southbridge_device, get_slave_ack))

	MCFG_DEVICE_ADD("pic8259_slave", PIC8259, 0)
	MCFG_PIC8259_OUT_INT_CB(WRITELINE("pic8259_master", pic8259_device, ir2_w))
	MCFG_PIC8259_IN_SP_CB(GND)

	MCFG_BUS_MASTER_IDE_CONTROLLER_ADD("ide", ata_devices, "hdd", nullptr, false)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE("pic8259_slave", pic8259_device, ir6_w))
	MCFG_BUS_MASTER_IDE_CONTROLLER_SPACE(":maincpu", AS_PROGRAM)

	MCFG_BUS_MASTER_IDE_CONTROLLER_ADD("ide2", ata_devices, "cdrom", nullptr, false)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE("pic8259_slave", pic8259_device, ir7_w))
	MCFG_BUS_MASTER_IDE_CONTROLLER_SPACE(":maincpu", AS_PROGRAM)

	// sound hardware
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DEVICE_ADD("isabus", ISA16, 0)
	MCFG_ISA16_CPU(":maincpu")
	MCFG_ISA_OUT_IRQ2_CB(WRITELINE("pic8259_slave",  pic8259_device, ir2_w)) // in place of irq 2 on at irq 9 is used
	MCFG_ISA_OUT_IRQ3_CB(WRITELINE("pic8259_master", pic8259_device, ir3_w))
	MCFG_ISA_OUT_IRQ4_CB(WRITELINE("pic8259_master", pic8259_device, ir4_w))
	MCFG_ISA_OUT_IRQ5_CB(WRITELINE("pic8259_master", pic8259_device, ir5_w))
	MCFG_ISA_OUT_IRQ6_CB(WRITELINE("pic8259_master", pic8259_device, ir6_w))
	MCFG_ISA_OUT_IRQ7_CB(WRITELINE("pic8259_master", pic8259_device, ir7_w))
	MCFG_ISA_OUT_IRQ10_CB(WRITELINE("pic8259_slave", pic8259_device, ir3_w))
	MCFG_ISA_OUT_IRQ11_CB(WRITELINE("pic8259_slave", pic8259_device, ir4_w))
	MCFG_ISA_OUT_IRQ12_CB(WRITELINE("pic8259_slave", pic8259_device, ir5_w))
	MCFG_ISA_OUT_IRQ14_CB(WRITELINE("pic8259_slave", pic8259_device, ir6_w))
	MCFG_ISA_OUT_IRQ15_CB(WRITELINE("pic8259_slave", pic8259_device, ir7_w))
	MCFG_ISA_OUT_DRQ0_CB(WRITELINE("dma8237_1", am9517a_device, dreq0_w))
	MCFG_ISA_OUT_DRQ1_CB(WRITELINE("dma8237_1", am9517a_device, dreq1_w))
	MCFG_ISA_OUT_DRQ2_CB(WRITELINE("dma8237_1", am9517a_device, dreq2_w))
	MCFG_ISA_OUT_DRQ3_CB(WRITELINE("dma8237_1", am9517a_device, dreq3_w))
	MCFG_ISA_OUT_DRQ5_CB(WRITELINE("dma8237_2", am9517a_device, dreq1_w))
	MCFG_ISA_OUT_DRQ6_CB(WRITELINE("dma8237_2", am9517a_device, dreq2_w))
	MCFG_ISA_OUT_DRQ7_CB(WRITELINE("dma8237_2", am9517a_device, dreq3_w))
MACHINE_CONFIG_END

southbridge_device::southbridge_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	m_maincpu(*this, ":maincpu"),
	m_pic8259_master(*this, "pic8259_master"),
	m_pic8259_slave(*this, "pic8259_slave"),
	m_dma8237_1(*this, "dma8237_1"),
	m_dma8237_2(*this, "dma8237_2"),
	m_pit8254(*this, "pit8254"),
	m_isabus(*this, "isabus"),
	m_speaker(*this, "speaker"),
	m_ide(*this, "ide"),
	m_ide2(*this, "ide2"),
	m_at_spkrdata(0), m_pit_out2(0), m_dma_channel(0), m_cur_eop(false), m_dma_high_byte(0), m_at_speaker(0), m_refresh(false), m_eisa_irq_mode(0), m_channel_check(0), m_nmi_enabled(0), m_ide_io_ports_enabled(true)
{
}

/**********************************************************
 *
 * Init functions
 *
 **********************************************************/

READ32_MEMBER(southbridge_device::ide1_read32_cs0_r)
{
	if (!m_ide_io_ports_enabled)
		return 0xffffffff;
	return m_ide->read_cs0(space, offset, mem_mask);
}

WRITE32_MEMBER(southbridge_device::ide1_write32_cs0_w)
{
	if (!m_ide_io_ports_enabled)
		return;
	m_ide->write_cs0(space, offset, data, mem_mask);
}

READ32_MEMBER(southbridge_device::ide2_read32_cs0_r)
{
	if (!m_ide_io_ports_enabled)
		return 0xffffffff;
	return m_ide2->read_cs0(space, offset, mem_mask);
}

WRITE32_MEMBER(southbridge_device::ide2_write32_cs0_w)
{
	if (!m_ide_io_ports_enabled)
		return;
	m_ide2->write_cs0(space, offset, data, mem_mask);
}

READ8_MEMBER(southbridge_device::ide1_read_cs1_r)
{
	if (!m_ide_io_ports_enabled)
		return 0xff;
	return m_ide->read_cs1(1, 0xff0000) >> 16;
}

WRITE8_MEMBER(southbridge_device::ide1_write_cs1_w)
{
	if (!m_ide_io_ports_enabled)
		return;
	m_ide->write_cs1(1, data << 16, 0xff0000);
}

READ8_MEMBER(southbridge_device::ide2_read_cs1_r)
{
	if (!m_ide_io_ports_enabled)
		return 0xff;
	return m_ide2->read_cs1(1, 0xff0000) >> 16;
}

WRITE8_MEMBER(southbridge_device::ide2_write_cs1_w)
{
	if (!m_ide_io_ports_enabled)
		return;
	m_ide2->write_cs1(1, data << 16, 0xff0000);
}

// With EISA it is possible to select whether each IRQ line is edge sensitive or level sensitive
// Each bit corresponds to an IRQ, 0 for edge triggered 1 for level sensitive
// IRQs 0 1 2 8 13 are always edge triggered
READ8_MEMBER(southbridge_device::eisa_irq_read)
{
	if (offset == 0)
		return m_eisa_irq_mode & 0xff;
	else
		return m_eisa_irq_mode >> 8;
}

WRITE8_MEMBER(southbridge_device::eisa_irq_write)
{
	if (offset == 0)
		m_eisa_irq_mode = (m_eisa_irq_mode & 0xff00) | data;
	else
		m_eisa_irq_mode = (m_eisa_irq_mode & 0x00ff) | (data << 8);
	// TODO: update m_pic8259_master and m_pic8259_slave with the new configuration
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void southbridge_device::device_start()
{
	spaceio = &machine().device(":maincpu")->memory().space(AS_IO);

	spaceio->install_readwrite_handler(0x0000, 0x001f, read8_delegate(FUNC(am9517a_device::read), &(*m_dma8237_1)), write8_delegate(FUNC(am9517a_device::write), &(*m_dma8237_1)), 0xffffffff);
	spaceio->install_readwrite_handler(0x0020, 0x003f, read8_delegate(FUNC(pic8259_device::read), &(*m_pic8259_master)), write8_delegate(FUNC(pic8259_device::write), &(*m_pic8259_master)), 0xffffffff);
	spaceio->install_readwrite_handler(0x0040, 0x005f, read8_delegate(FUNC(pit8254_device::read), &(*m_pit8254)), write8_delegate(FUNC(pit8254_device::write), &(*m_pit8254)), 0xffffffff);
	spaceio->install_readwrite_handler(0x0060, 0x0063, read8_delegate(FUNC(southbridge_device::at_portb_r), this), write8_delegate(FUNC(southbridge_device::at_portb_w), this), 0x0000ff00);
	spaceio->install_readwrite_handler(0x0080, 0x009f, read8_delegate(FUNC(southbridge_device::at_page8_r), this), write8_delegate(FUNC(southbridge_device::at_page8_w), this), 0xffffffff);
	spaceio->install_readwrite_handler(0x00a0, 0x00bf, read8_delegate(FUNC(pic8259_device::read), &(*m_pic8259_slave)), write8_delegate(FUNC(pic8259_device::write), &(*m_pic8259_slave)), 0xffffffff);
	spaceio->install_readwrite_handler(0x00c0, 0x00df, read8_delegate(FUNC(southbridge_device::at_dma8237_2_r), this), write8_delegate(FUNC(southbridge_device::at_dma8237_2_w), this), 0xffffffff);
	spaceio->install_readwrite_handler(0x0170, 0x0177, read32_delegate(FUNC(southbridge_device::ide2_read32_cs0_r), this), write32_delegate(FUNC(southbridge_device::ide2_write32_cs0_w), this), 0xffffffff);
	spaceio->install_readwrite_handler(0x01f0, 0x01f7, read32_delegate(FUNC(southbridge_device::ide1_read32_cs0_r), this), write32_delegate(FUNC(southbridge_device::ide1_write32_cs0_w), this), 0xffffffff);
	spaceio->install_readwrite_handler(0x0374, 0x0377, read8_delegate(FUNC(southbridge_device::ide2_read_cs1_r), this), write8_delegate(FUNC(southbridge_device::ide2_write_cs1_w), this), 0xff0000);
	spaceio->install_readwrite_handler(0x03f4, 0x03f7, read8_delegate(FUNC(southbridge_device::ide1_read_cs1_r), this), write8_delegate(FUNC(southbridge_device::ide1_write_cs1_w), this), 0xff0000);
	spaceio->install_readwrite_handler(0x04d0, 0x04d3, read8_delegate(FUNC(southbridge_device::eisa_irq_read), this), write8_delegate(FUNC(southbridge_device::eisa_irq_write), this), 0x0000ffff);
	spaceio->nop_readwrite(0x00e0, 0x00ef);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void southbridge_device::device_reset()
{
	m_at_spkrdata = 0;
	m_pit_out2 = 1;
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
		return m_pic8259_slave->acknowledge();

	return 0x00;
}

/*************************************************************************
 *
 *      PC Speaker related
 *
 *************************************************************************/

void southbridge_device::at_speaker_set_spkrdata(uint8_t data)
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
	uint8_t data = m_at_pages[offset % 0x10];

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

	if (offset == 0)
		port80_debug_write(data);
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

	// Assert HLDA
	m_dma8237_2->hack_w( state );
}

READ8_MEMBER(southbridge_device::pc_dma_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return 0xff;
	uint8_t result;
	offs_t page_offset = ((offs_t) m_dma_offset[0][m_dma_channel]) << 16;

	result = prog_space.read_byte(page_offset + offset);
	return result;
}


WRITE8_MEMBER(southbridge_device::pc_dma_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t) m_dma_offset[0][m_dma_channel]) << 16;

	prog_space.write_byte(page_offset + offset, data);
}


READ8_MEMBER(southbridge_device::pc_dma_read_word)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return 0xff;
	uint16_t result;
	offs_t page_offset = ((offs_t) m_dma_offset[1][m_dma_channel & 3]) << 16;

	result = prog_space.read_word((page_offset & 0xfe0000) | (offset << 1));
	m_dma_high_byte = result & 0xFF00;

	return result & 0xFF;
}


WRITE8_MEMBER(southbridge_device::pc_dma_write_word)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t) m_dma_offset[1][m_dma_channel & 3]) << 16;

	prog_space.write_word((page_offset & 0xfe0000) | (offset << 1), m_dma_high_byte | data);
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
	uint8_t data = m_at_speaker;
	data &= ~0xd0; // AT BIOS don't likes this being set

	// 0x10 is the dram refresh line bit on the 5170, just a timer here, 15.085us.
	data |= m_refresh ? 0x10 : 0;

	if (m_pit_out2)
		data |= 0x20;
	else
		data &= ~0x20; // ps2m30 wants this

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

/***************************************************************************
  Extended Southbridge Device
***************************************************************************/

static void pc_isa_onboard(device_slot_interface &device)
{
	device.option_add("comat", ISA8_COM_AT);
	device.option_add("lpt", ISA8_LPT);
	device.option_add("fdcsmc", ISA8_FDC_SMC);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(southbridge_extended_device::device_add_mconfig)
	southbridge_device::device_add_mconfig(config);

	MCFG_DEVICE_ADD("keybc", AT_KEYBOARD_CONTROLLER, XTAL(12'000'000))
	MCFG_AT_KEYBOARD_CONTROLLER_SYSTEM_RESET_CB(INPUTLINE(":maincpu", INPUT_LINE_RESET))
	MCFG_AT_KEYBOARD_CONTROLLER_GATE_A20_CB(INPUTLINE(":maincpu", INPUT_LINE_A20))
	MCFG_AT_KEYBOARD_CONTROLLER_INPUT_BUFFER_FULL_CB(WRITELINE("pic8259_master", pic8259_device, ir1_w))
	MCFG_AT_KEYBOARD_CONTROLLER_KEYBOARD_CLOCK_CB(WRITELINE("pc_kbdc", pc_kbdc_device, clock_write_from_mb))
	MCFG_AT_KEYBOARD_CONTROLLER_KEYBOARD_DATA_CB(WRITELINE("pc_kbdc", pc_kbdc_device, data_write_from_mb))
	MCFG_DEVICE_ADD("pc_kbdc", PC_KBDC, 0)
	MCFG_PC_KBDC_OUT_CLOCK_CB(WRITELINE("keybc", at_keyboard_controller_device, keyboard_clock_w))
	MCFG_PC_KBDC_OUT_DATA_CB(WRITELINE("keybc", at_keyboard_controller_device, keyboard_data_w))
	MCFG_PC_KBDC_SLOT_ADD("pc_kbdc", "kbd", pc_at_keyboards, STR_KBD_MICROSOFT_NATURAL)

	MCFG_DS12885_ADD("rtc")
	MCFG_MC146818_IRQ_HANDLER(WRITELINE("pic8259_slave", pic8259_device, ir0_w))
	MCFG_MC146818_CENTURY_INDEX(0x32)

	// on board devices
	MCFG_DEVICE_ADD("board1", ISA16_SLOT, 0, "isabus", pc_isa_onboard, "fdcsmc", true) // FIXME: determine ISA bus clock
	MCFG_DEVICE_ADD("board2", ISA16_SLOT, 0, "isabus", pc_isa_onboard, "comat", true)
	MCFG_DEVICE_ADD("board3", ISA16_SLOT, 0, "isabus", pc_isa_onboard, "lpt", true)
MACHINE_CONFIG_END

southbridge_extended_device::southbridge_extended_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: southbridge_device(mconfig, type, tag, owner, clock),
	m_keybc(*this, "keybc"),
	m_ds12885(*this, "rtc"),
	m_pc_kbdc(*this, "pc_kbdc")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void southbridge_extended_device::device_start()
{
	address_space& spaceio = machine().device(":maincpu")->memory().space(AS_IO);

	southbridge_device::device_start();

	spaceio.install_readwrite_handler(0x0060, 0x0063, read8_delegate(FUNC(at_keyboard_controller_device::data_r), &(*m_keybc)), write8_delegate(FUNC(at_keyboard_controller_device::data_w), &(*m_keybc)), 0x000000ff);
	spaceio.install_readwrite_handler(0x0064, 0x0067, read8_delegate(FUNC(at_keyboard_controller_device::status_r), &(*m_keybc)), write8_delegate(FUNC(at_keyboard_controller_device::command_w), &(*m_keybc)), 0xffffffff);
	spaceio.install_readwrite_handler(0x0070, 0x007f, read8_delegate(FUNC(ds12885_device::read), &(*m_ds12885)), write8_delegate(FUNC(ds12885_device::write), &(*m_ds12885)), 0xffffffff);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void southbridge_extended_device::device_reset()
{
	southbridge_device::device_reset();
}

WRITE8_MEMBER( southbridge_extended_device::write_rtc )
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
