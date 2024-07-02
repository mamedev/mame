// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic, Carl
/***************************************************************************

    IBM AT Compatibles

***************************************************************************/

#include "emu.h"
#include "at.h"

#include "cpu/i86/i286.h"
#include "cpu/i386/i386.h"

#include "softlist_dev.h"
#include "speaker.h"

#define LOG_PORT80  (1U << 1)
#define VERBOSE (0)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(AT_MB, at_mb_device, "at_mb", "PC/AT Motherboard")

at_mb_device::at_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, AT_MB, tag, owner, clock),
	m_maincpu(*this, ":maincpu"),
	m_isabus(*this, "isabus"),
	m_pic8259_slave(*this, "pic8259_slave"),
	m_dma8237_1(*this, "dma8237_1"),
	m_dma8237_2(*this, "dma8237_2"),
	m_pit8254(*this, "pit8254"),
	m_speaker(*this, "speaker"),
	m_mc146818(*this, "rtc"),
	m_keybc(*this, "keybc")
{
}

void at_mb_device::device_reset()
{
	m_at_spkrdata = 0;
	m_pit_out2 = 1;
	m_dma_channel = -1;
	m_cur_eop = false;
}

void at_mb_device::device_start()
{
	// FIXME: this is gross and should be done in machine configuration
	if(!strncmp(m_maincpu->shortname(), "i80286", 6))
		downcast<i80286_cpu_device *>(m_maincpu.target())->set_a20_callback(*this, FUNC(at_mb_device::a20_286));
}

void at_mb_device::at_softlists(machine_config &config)
{
	/* software lists */
	SOFTWARE_LIST(config, "pc_disk_list").set_original("ibm5150");
	SOFTWARE_LIST(config, "at_disk_list").set_original("ibm5170");
	SOFTWARE_LIST(config, "at_cdrom_list").set_original("ibm5170_cdrom");
	SOFTWARE_LIST(config, "at_hdd_list").set_original("ibm5170_hdd");
	SOFTWARE_LIST(config, "midi_disk_list").set_compatible("midi_flop");
}

void at_mb_device::device_add_mconfig(machine_config &config)
{
	PIT8254(config, m_pit8254);
	m_pit8254->set_clk<0>(4772720/4); /* heartbeat IRQ */
	m_pit8254->out_handler<0>().set("pic8259_master", FUNC(pic8259_device::ir0_w));
	m_pit8254->set_clk<1>(4772720/4); /* dram refresh */
	m_pit8254->set_clk<2>(4772720/4); /* pio port c pin 4, and speaker polling enough */
	m_pit8254->out_handler<2>().set(FUNC(at_mb_device::pit8254_out2_changed));

	AM9517A(config, m_dma8237_1, 14.318181_MHz_XTAL / 3);
	m_dma8237_1->out_hreq_callback().set(m_dma8237_2, FUNC(am9517a_device::dreq0_w));
	m_dma8237_1->out_eop_callback().set(FUNC(at_mb_device::dma8237_out_eop));
	m_dma8237_1->in_memr_callback().set(FUNC(at_mb_device::dma_read_byte));
	m_dma8237_1->out_memw_callback().set(FUNC(at_mb_device::dma_write_byte));
	m_dma8237_1->in_ior_callback<0>().set(FUNC(at_mb_device::dma8237_0_dack_r));
	m_dma8237_1->in_ior_callback<1>().set(FUNC(at_mb_device::dma8237_1_dack_r));
	m_dma8237_1->in_ior_callback<2>().set(FUNC(at_mb_device::dma8237_2_dack_r));
	m_dma8237_1->in_ior_callback<3>().set(FUNC(at_mb_device::dma8237_3_dack_r));
	m_dma8237_1->out_iow_callback<0>().set(FUNC(at_mb_device::dma8237_0_dack_w));
	m_dma8237_1->out_iow_callback<1>().set(FUNC(at_mb_device::dma8237_1_dack_w));
	m_dma8237_1->out_iow_callback<2>().set(FUNC(at_mb_device::dma8237_2_dack_w));
	m_dma8237_1->out_iow_callback<3>().set(FUNC(at_mb_device::dma8237_3_dack_w));
	m_dma8237_1->out_dack_callback<0>().set(FUNC(at_mb_device::dack0_w));
	m_dma8237_1->out_dack_callback<1>().set(FUNC(at_mb_device::dack1_w));
	m_dma8237_1->out_dack_callback<2>().set(FUNC(at_mb_device::dack2_w));
	m_dma8237_1->out_dack_callback<3>().set(FUNC(at_mb_device::dack3_w));

	AM9517A(config, m_dma8237_2, 14.318181_MHz_XTAL / 3);
	m_dma8237_2->out_hreq_callback().set(FUNC(at_mb_device::dma_hrq_changed));
	m_dma8237_2->out_eop_callback().set(FUNC(at_mb_device::dma8237_2_out_eop));
	m_dma8237_2->in_memr_callback().set(FUNC(at_mb_device::dma_read_word));
	m_dma8237_2->out_memw_callback().set(FUNC(at_mb_device::dma_write_word));
	m_dma8237_2->in_ior_callback<1>().set(FUNC(at_mb_device::dma8237_5_dack_r));
	m_dma8237_2->in_ior_callback<2>().set(FUNC(at_mb_device::dma8237_6_dack_r));
	m_dma8237_2->in_ior_callback<3>().set(FUNC(at_mb_device::dma8237_7_dack_r));
	m_dma8237_2->out_iow_callback<1>().set(FUNC(at_mb_device::dma8237_5_dack_w));
	m_dma8237_2->out_iow_callback<2>().set(FUNC(at_mb_device::dma8237_6_dack_w));
	m_dma8237_2->out_iow_callback<3>().set(FUNC(at_mb_device::dma8237_7_dack_w));
	m_dma8237_2->out_dack_callback<0>().set(FUNC(at_mb_device::dack4_w));
	m_dma8237_2->out_dack_callback<1>().set(FUNC(at_mb_device::dack5_w));
	m_dma8237_2->out_dack_callback<2>().set(FUNC(at_mb_device::dack6_w));
	m_dma8237_2->out_dack_callback<3>().set(FUNC(at_mb_device::dack7_w));

	pic8259_device &pic8259_master(PIC8259(config, "pic8259_master"));
	pic8259_master.out_int_callback().set_inputline(":maincpu", 0);
	pic8259_master.in_sp_callback().set_constant(1);
	pic8259_master.read_slave_ack_callback().set(FUNC(at_mb_device::get_slave_ack));

	PIC8259(config, m_pic8259_slave);
	m_pic8259_slave->out_int_callback().set("pic8259_master", FUNC(pic8259_device::ir2_w));
	m_pic8259_slave->in_sp_callback().set_constant(0);

	ISA16(config, m_isabus, 0);
	m_isabus->set_memspace(":maincpu", AS_PROGRAM);
	m_isabus->set_iospace(":maincpu", AS_IO);
	m_isabus->irq2_callback().set(m_pic8259_slave, FUNC(pic8259_device::ir1_w)); // in place of irq 2 on at irq 9 is used
	m_isabus->irq3_callback().set("pic8259_master", FUNC(pic8259_device::ir3_w));
	m_isabus->irq4_callback().set("pic8259_master", FUNC(pic8259_device::ir4_w));
	m_isabus->irq5_callback().set("pic8259_master", FUNC(pic8259_device::ir5_w));
	m_isabus->irq6_callback().set("pic8259_master", FUNC(pic8259_device::ir6_w));
	m_isabus->irq7_callback().set("pic8259_master", FUNC(pic8259_device::ir7_w));
	m_isabus->irq10_callback().set(m_pic8259_slave, FUNC(pic8259_device::ir3_w));
	m_isabus->irq11_callback().set(m_pic8259_slave, FUNC(pic8259_device::ir4_w));
	m_isabus->irq12_callback().set(m_pic8259_slave, FUNC(pic8259_device::ir5_w));
	m_isabus->irq14_callback().set(m_pic8259_slave, FUNC(pic8259_device::ir6_w));
	m_isabus->irq15_callback().set(m_pic8259_slave, FUNC(pic8259_device::ir7_w));
	m_isabus->drq0_callback().set(m_dma8237_1, FUNC(am9517a_device::dreq0_w));
	m_isabus->drq1_callback().set(m_dma8237_1, FUNC(am9517a_device::dreq1_w));
	m_isabus->drq2_callback().set(m_dma8237_1, FUNC(am9517a_device::dreq2_w));
	m_isabus->drq3_callback().set(m_dma8237_1, FUNC(am9517a_device::dreq3_w));
	m_isabus->drq5_callback().set(m_dma8237_2, FUNC(am9517a_device::dreq1_w));
	m_isabus->drq6_callback().set(m_dma8237_2, FUNC(am9517a_device::dreq2_w));
	m_isabus->drq7_callback().set(m_dma8237_2, FUNC(am9517a_device::dreq3_w));
	m_isabus->iochck_callback().set(FUNC(at_mb_device::iochck_w));
	m_isabus->iochrdy_callback().set_inputline(":maincpu", INPUT_LINE_HALT);

	MC146818(config, m_mc146818, 32.768_kHz_XTAL);
	m_mc146818->irq().set(m_pic8259_slave, FUNC(pic8259_device::ir0_w));
	m_mc146818->set_century_index(0x32);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	at_keyboard_controller_device &keybc(AT_KEYBOARD_CONTROLLER(config, "keybc", 12_MHz_XTAL));
	keybc.hot_res().set_inputline(":maincpu", INPUT_LINE_RESET);
	keybc.gate_a20().set_inputline(":maincpu", INPUT_LINE_A20);
	keybc.kbd_irq().set("pic8259_master", FUNC(pic8259_device::ir1_w));
}


void at_mb_device::map(address_map &map)
{
	map(0x0000, 0x001f).rw("dma8237_1", FUNC(am9517a_device::read), FUNC(am9517a_device::write)).umask16(0xffff);
	map(0x0020, 0x003f).rw("pic8259_master", FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0xffff);
	map(0x0040, 0x005f).rw(m_pit8254, FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask16(0xffff);
	map(0x0061, 0x0061).rw(FUNC(at_mb_device::portb_r), FUNC(at_mb_device::portb_w));
	map(0x0060, 0x0060).rw("keybc", FUNC(at_keyboard_controller_device::data_r), FUNC(at_keyboard_controller_device::data_w));
	map(0x0064, 0x0064).rw("keybc", FUNC(at_keyboard_controller_device::status_r), FUNC(at_keyboard_controller_device::command_w));
	map(0x0070, 0x007f).w(FUNC(at_mb_device::rtcas_nmi_w)).umask16(0x00ff);
	map(0x0070, 0x007f).rw(m_mc146818, FUNC(mc146818_device::data_r), FUNC(mc146818_device::data_w)).umask16(0xff00);
	map(0x0080, 0x009f).rw(FUNC(at_mb_device::page8_r), FUNC(at_mb_device::page8_w)).umask16(0xffff);
	map(0x00a0, 0x00bf).rw("pic8259_slave", FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0xffff);
	map(0x00c0, 0x00df).rw("dma8237_2", FUNC(am9517a_device::read), FUNC(am9517a_device::write)).umask16(0x00ff);
}

/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/
uint8_t at_mb_device::get_slave_ack(offs_t offset)
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

void at_mb_device::speaker_set_spkrdata(uint8_t data)
{
	m_at_spkrdata = data ? 1 : 0;
	m_speaker->level_w(m_at_spkrdata & m_pit_out2);
}



/*************************************************************
 *
 * pit8254 configuration
 *
 *************************************************************/

void at_mb_device::pit8254_out2_changed(int state)
{
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(m_at_spkrdata & m_pit_out2);
}


/*************************************************************************
 *
 *      PC DMA stuff
 *
 *************************************************************************/

uint8_t at_mb_device::page8_r(offs_t offset)
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


void at_mb_device::page8_w(offs_t offset, uint8_t data)
{
	m_at_pages[offset % 0x10] = data;

	if (offset == 0)
	{
		LOGMASKED(LOG_PORT80, " at_page8_w(): Port 80h <== 0x%02x (PC=0x%08x)\n", data, m_maincpu->pc());
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


void at_mb_device::dma_hrq_changed(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237_2->hack_w(state);
}

uint8_t at_mb_device::dma_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return 0xff;
	uint8_t result;
	offs_t page_offset = ((offs_t) m_dma_offset[0][m_dma_channel]) << 16;

	result = prog_space.read_byte(page_offset + offset);
	return result;
}


void at_mb_device::dma_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t) m_dma_offset[0][m_dma_channel]) << 16;

	prog_space.write_byte(page_offset + offset, data);
}


uint8_t at_mb_device::dma_read_word(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return 0xff;
	uint16_t result;
	offs_t page_offset = ((offs_t) m_dma_offset[1][m_dma_channel & 3]) << 16;

	result = prog_space.read_word((page_offset & 0xfe0000) | (offset << 1));
	m_dma_high_byte = result & 0xff00;

	return result & 0xff;
}


void at_mb_device::dma_write_word(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	if(m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t) m_dma_offset[1][m_dma_channel & 3]) << 16;

	prog_space.write_word((page_offset & 0xfe0000) | (offset << 1), m_dma_high_byte | data);
}

uint8_t at_mb_device::dma8237_0_dack_r() { return m_isabus->dack_r(0); }
uint8_t at_mb_device::dma8237_1_dack_r() { return m_isabus->dack_r(1); }
uint8_t at_mb_device::dma8237_2_dack_r() { return m_isabus->dack_r(2); }
uint8_t at_mb_device::dma8237_3_dack_r() { return m_isabus->dack_r(3); }
uint8_t at_mb_device::dma8237_5_dack_r() { uint16_t ret = m_isabus->dack16_r(5); m_dma_high_byte = ret & 0xff00; return ret; }
uint8_t at_mb_device::dma8237_6_dack_r() { uint16_t ret = m_isabus->dack16_r(6); m_dma_high_byte = ret & 0xff00; return ret; }
uint8_t at_mb_device::dma8237_7_dack_r() { uint16_t ret = m_isabus->dack16_r(7); m_dma_high_byte = ret & 0xff00; return ret; }


void at_mb_device::dma8237_0_dack_w(uint8_t data) { m_isabus->dack_w(0, data); }
void at_mb_device::dma8237_1_dack_w(uint8_t data) { m_isabus->dack_w(1, data); }
void at_mb_device::dma8237_2_dack_w(uint8_t data) { m_isabus->dack_w(2, data); }
void at_mb_device::dma8237_3_dack_w(uint8_t data) { m_isabus->dack_w(3, data); }
void at_mb_device::dma8237_5_dack_w(uint8_t data) { m_isabus->dack16_w(5, m_dma_high_byte | data); }
void at_mb_device::dma8237_6_dack_w(uint8_t data) { m_isabus->dack16_w(6, m_dma_high_byte | data); }
void at_mb_device::dma8237_7_dack_w(uint8_t data) { m_isabus->dack16_w(7, m_dma_high_byte | data); }

void at_mb_device::dma8237_out_eop(int state)
{
	m_cur_eop = state == ASSERT_LINE;
	if(m_dma_channel != -1)
		m_isabus->eop_w(m_dma_channel, m_cur_eop ? ASSERT_LINE : CLEAR_LINE );
}

void at_mb_device::dma8237_2_out_eop(int state)
{
	m_cur_eop2 = state == ASSERT_LINE;
	if(m_dma_channel != -1)
		m_isabus->eop_w(m_dma_channel, m_cur_eop2 ? ASSERT_LINE : CLEAR_LINE );
}

void at_mb_device::set_dma_channel(int channel, int state)
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

void at_mb_device::rtcas_nmi_w(uint8_t data)
{
	m_nmi_enabled = BIT(data,7);
	if (!m_nmi_enabled)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	m_mc146818->address_w(data);
}

uint32_t at_mb_device::a20_286(bool state)
{
	return (state ? 0xffffff : 0xefffff);
}

void at_mb_device::shutdown(int state)
{
	if(state)
		m_maincpu->reset();
}
void at_mb_device::dack0_w(int state) { set_dma_channel(0, state); }
void at_mb_device::dack1_w(int state) { set_dma_channel(1, state); }
void at_mb_device::dack2_w(int state) { set_dma_channel(2, state); }
void at_mb_device::dack3_w(int state) { set_dma_channel(3, state); }
void at_mb_device::dack4_w(int state) { m_dma8237_1->hack_w(state ? 0 : 1); } // it's inverted
void at_mb_device::dack5_w(int state) { set_dma_channel(5, state); }
void at_mb_device::dack6_w(int state) { set_dma_channel(6, state); }
void at_mb_device::dack7_w(int state) { set_dma_channel(7, state); }

void at_mb_device::kbd_clk_w(int state) { m_keybc->kbd_clk_w(state); }
void at_mb_device::kbd_data_w(int state) { m_keybc->kbd_data_w(state); }

uint8_t at_mb_device::portb_r()
{
	uint8_t data = m_at_speaker;
	data &= ~0xd0; /* AT BIOS don't likes this being set */

	/* 0x10 is the dram refresh line bit, 15.085us. */
	data |= (machine().time().as_ticks(110000) & 1) ? 0x10 : 0;

	if (m_pit_out2)
		data |= 0x20;
	else
		data &= ~0x20; /* ps2m30 wants this */

	return data;
}

void at_mb_device::portb_w(uint8_t data)
{
	m_at_speaker = data;
	m_pit8254->write_gate2(BIT(data, 0));
	speaker_set_spkrdata( BIT(data, 1));
	m_channel_check = BIT(data, 3);
	if (m_channel_check)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void at_mb_device::iochck_w(int state)
{
	if (!state && m_nmi_enabled && !m_channel_check)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}
