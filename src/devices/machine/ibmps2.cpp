// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic, Carl
/***************************************************************************

    IBM PS2 Compatibles

***************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/hlemouse.h"
#include "bus/rs232/null_modem.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/sun_kbd.h"
#include "bus/rs232/terminal.h"
#include "bus/mca/mca.h"
#include "bus/mca/mca_cards.h"
#include "bus/mca/planar_uart.h"
#include "bus/mca/planar_lpt.h"
#include "bus/mca/planar_fdc.h"
#include "bus/mca/planar_vga.h"
#include "imagedev/floppy.h"
#include "machine/ibmps2.h"
#include "cpu/i86/i286.h"
#include "cpu/i386/i386.h"
#include "formats/pc_dsk.h"
#include "speaker.h"
#include "xtal.h"

#define LOG_PORT80  0

#define LOG_SYSPORTS    (1U <<  2)
#define LOG_NVRAM       (1U <<  3)
#define LOG_TIMERS      (1U <<  4)
#define LOG_POST        (1U <<  5)
#define LOG_POS         (1U <<  6)
#define LOG_IRQ			(1U <<	7)

#define VERBOSE (LOG_SYSPORTS|LOG_NVRAM|LOG_TIMERS|LOG_POST|LOG_POS|LOG_IRQ)
#include "logmacro.h"

#define LOGSYSPORTS(...)    LOGMASKED(LOG_SYSPORTS, __VA_ARGS__)
#define LOGNVRAM(...)       LOGMASKED(LOG_NVRAM, __VA_ARGS__)
#define LOGTIMERS(...)      LOGMASKED(LOG_TIMERS, __VA_ARGS__)
#define LOGPOST(...)        LOGMASKED(LOG_POST, __VA_ARGS__)
#define LOGPOS(...)         LOGMASKED(LOG_POS, __VA_ARGS__)
#define LOGIRQ(...)			LOGMASKED(LOG_IRQ, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define XTAL_U153   25.175_MHz_XTAL
#define XTAL_U155   25.175_MHz_XTAL
#define XTAL_U156   28.322_MHz_XTAL
#define XTAL_U159   32_MHz_XTAL
#define XTAL_IO     14.318181_MHz_XTAL

DEFINE_DEVICE_TYPE(PS2_MB,          ps2_mb_device,          "ps2_mb",           "Generic PS/2 Planar")

ps2_mb_device::ps2_mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PS2_MB, tag, owner, clock),
	m_maincpu(*this, ":maincpu"),
	m_ram(*this, ":ram"),
	m_nvram(*this, "nvram"),
	m_mcabus(*this, "mcabus"),
	m_pic8259_master(*this, "pic8259_master"),
	m_pic8259_slave(*this, "pic8259_slave"),
	m_dmac(*this, "dmac"),
	m_speaker(*this, "speaker"),
	m_mc146818(*this, "rtc"),
	m_keybc(*this, "keybc"),
	m_ps2_con(*this, "ps2_con"),
	m_aux_con(*this, "aux_con"),
	m_timer_slow_refresh(*this, "timer_slow_refresh"),
	m_timer_fast_refresh(*this, "timer_fast_refresh"),
	m_io_controller(*this, "io_ctrlr"),
	m_mcaslot(*this, "mca%u", 0U),
	m_bios(*this, ":bios")
{
}

void ps2_mb_device::device_reset()
{
	m_at_spkrdata = 0;
	m_pit_out2 = 1;

	m_refresh_bit = 0;

	if(m_timer_fast_refresh != NULL) m_timer_fast_refresh->enable(0);
	m_timer_slow_refresh->enable(1);

	irq0_latch_reset();
}

void ps2_mb_device::device_start()
{
	m_nvram_index = 0;

	m_at_speaker = 0;
	m_at_spkrdata = 0;

	m_channel_check = 0;
	m_nmi_enabled = 0;
	m_parity_check_enabled = 0;
	m_channel_check_enabled = 0;
	m_memory_control = 0;
	m_gate_a20 = 0;
	m_nmi_flag = 0;
	m_alt_hot_reset = 0;

	m_pit_out2 = 1;

	m_refresh_bit = 0;
	m_refresh_state = 0;
}

void ps2_mb_device::device_config_complete()
{

}

void ps2_mb_device::ps2_32_map(address_map& map)
{
	map.unmap_value_high();
	map(0x00000000, 0x0009ffff).bankrw(":bank10");
	map(0x000e0000, 0x000fffff).rom().region(":bios", 0);
	map(0xfffe0000, 0xffffffff).rom().region(":bios", 0);
}

void ps2_mb_device::ps2_32_io(address_map& map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).m(*this, FUNC(ps2_mb_device::map));
}


void ps2_mb_device::ps2_16_map(address_map& map)
{
	map.unmap_value_high();
	map(0x000000, 0x09ffff).bankrw(":bank10");
	map(0x0e0000, 0x0fffff).rom().region(":bios", 0);
	map(0xfe0000, 0xffffff).rom().region(":bios", 0);
}

void ps2_mb_device::ps2_16_io(address_map& map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).m(*this, FUNC(ps2_mb_device::map));
}

void ps2_mb_device::add_mca_common(machine_config &config)
{
	m_mcabus->iochck_callback().set(FUNC(ps2_mb_device::iochck_w));

	// IRQs
	m_mcabus->irq_callback<2>().set(m_pic8259_slave, FUNC(pic8259_device::ir1_w)); 	// AT compatibility: IRQ 2 cascades to IRQ 9
	m_mcabus->irq_callback<3>().set(m_pic8259_master, FUNC(pic8259_device::ir3_w));
	m_mcabus->irq_callback<4>().set(m_pic8259_master, FUNC(pic8259_device::ir4_w));
	m_mcabus->irq_callback<5>().set(m_pic8259_master, FUNC(pic8259_device::ir5_w));
	m_mcabus->irq_callback<6>().set(m_pic8259_master, FUNC(pic8259_device::ir6_w));
	m_mcabus->irq_callback<7>().set(m_pic8259_master, FUNC(pic8259_device::ir7_w));
	// IRQ 8 doesn't use the 8259.
	// IRQ 9 is cascade.
	m_mcabus->irq_callback<10>().set(m_pic8259_slave, FUNC(pic8259_device::ir2_w));
	m_mcabus->irq_callback<11>().set(m_pic8259_slave, FUNC(pic8259_device::ir3_w));
	m_mcabus->irq_callback<12>().set(m_pic8259_slave, FUNC(pic8259_device::ir4_w));
	// IRQ 13 doesn't use the 8259.
	m_mcabus->irq_callback<14>().set(m_pic8259_slave, FUNC(pic8259_device::ir6_w));
	m_mcabus->irq_callback<15>().set(m_pic8259_slave, FUNC(pic8259_device::ir7_w));
}

void ps2_mb_device::add_mca16(machine_config &config)
{
	MCA16(config, m_mcabus, 0);
	m_mcabus->set_addrmap(0, &ps2_mb_device::ps2_16_map);
	m_mcabus->set_addrmap(1, &ps2_mb_device::ps2_16_io);

	m_mcabus->cs_feedback_callback().set(m_io_controller, FUNC(ibm72x8299_device::cd_sfdbk_w));

	add_mca_common(config);
}

void ps2_mb_device::add_mca32(machine_config &config)
{
	MCA32(config, m_mcabus, 0);
	m_mcabus->set_addrmap(0, &ps2_mb_device::ps2_32_map);
	m_mcabus->set_addrmap(1, &ps2_mb_device::ps2_32_io);

	m_mcabus->cs_feedback_callback().set(m_io_controller, FUNC(ibm72x8299_device::cd_sfdbk_w));

	add_mca_common(config);
}

void ps2_mb_device::device_add_mconfig(machine_config &config)
{
	// Common AT-class hardware.

	if(m_sram_size != 0) NVRAM(config, m_nvram);

	PIC8259(config, m_pic8259_master, 0);
	m_pic8259_master->out_int_callback().set_inputline(m_maincpu, 0);
	m_pic8259_master->in_sp_callback().set_constant(1);
	m_pic8259_master->read_slave_ack_callback().set(FUNC(ps2_mb_device::get_slave_ack));

	PIC8259(config, m_pic8259_slave, 0);
	m_pic8259_slave->out_int_callback().set(m_pic8259_master, FUNC(pic8259_device::ir2_w));
	m_pic8259_slave->in_sp_callback().set_constant(0);

	MC146818(config, m_mc146818, 32.768_kHz_XTAL);
	m_mc146818->irq().set(m_pic8259_slave, FUNC(pic8259_device::ir0_w));
	m_mc146818->set_century_index(0x37);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	PC_KBDC(config, m_ps2_con, pc_ps2_keyboards, STR_KBD_MICROSOFT_NATURAL);
	m_ps2_con->out_clock_cb().set(m_keybc, FUNC(ps2_keyboard_controller_device::kbd_clk_w));
	m_ps2_con->out_data_cb().set(m_keybc, FUNC(ps2_keyboard_controller_device::kbd_data_w));

	PC_KBDC(config, m_aux_con, ps2_mice, STR_HLE_PS2_MOUSE);
	m_aux_con->out_clock_cb().set(m_keybc, FUNC(ps2_keyboard_controller_device::aux_clk_w));
	m_aux_con->out_data_cb().set(m_keybc, FUNC(ps2_keyboard_controller_device::aux_data_w));

	PS2_KEYBOARD_CONTROLLER(config, m_keybc, 40_MHz_XTAL / 4); // operates at 10 MHz
	m_keybc->set_default_bios_tag("ibm");
	m_keybc->hot_res().set(FUNC(ps2_mb_device::hot_reset_w));
	m_keybc->gate_a20().set(FUNC(ps2_mb_device::gate_a20_w));
	m_keybc->kbd_irq().set(m_pic8259_master, FUNC(pic8259_device::ir1_w)); 	// IRQ 1
	m_keybc->kbd_clk().set(m_ps2_con, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_keybc->kbd_data().set(m_ps2_con, FUNC(pc_kbdc_device::data_write_from_mb));
	m_keybc->aux_irq().set(m_pic8259_slave, FUNC(pic8259_device::ir4_w));	// IRQ 12
	m_keybc->aux_clk().set(m_aux_con, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_keybc->aux_data().set(m_aux_con, FUNC(pc_kbdc_device::data_write_from_mb));

	// memory refresh timer
	TIMER(config, m_timer_slow_refresh).configure_periodic(FUNC(ps2_mb_device::refresh_cb), attotime::from_hz(66137));
	if(m_supports_fast_refresh)
	{
		timer_device &m_timer_fast_refresh(TIMER(config, "timer_fast_refresh"));
		m_timer_fast_refresh.configure_periodic(FUNC(ps2_mb_device::refresh_cb), attotime::from_hz(1250000));
	}	
}

/* Common PS/2 devices. */
void ps2_mb_device::add_southbridge_72x8299(machine_config &config)
{
	ibm72x8299_device &m_io_controller(IBM72X8299(config, "io_ctrlr", 0, this));
	
	m_io_controller.pit_ch_callback<0>().set(FUNC(ps2_mb_device::irq0_w));
	m_io_controller.pit_ch_callback<2>().set(FUNC(ps2_mb_device::pit8254_out2_changed));
	m_io_controller.pit_ch_callback<3>().set(FUNC(ps2_mb_device::watchdog_w));

	m_mcabus->cs_feedback_callback().set(m_io_controller, FUNC(ibm72x8299_device::cd_sfdbk_w));
}

void ps2_mb_device::add_dmac_72x7377(machine_config &config)
{
	ibm72x7377_device &m_dmac(IBM72X7377(config, "dmac", 14.318181_MHz_XTAL / 3));

	// DMA channels
	m_mcabus->drq_callback<0>().set(m_dmac, FUNC(ibm72x7377_device::dreq_w<0>));
	m_mcabus->drq_callback<1>().set(m_dmac, FUNC(ibm72x7377_device::dreq_w<1>));
	m_mcabus->drq_callback<2>().set(m_dmac, FUNC(ibm72x7377_device::dreq_w<2>));
	m_mcabus->drq_callback<3>().set(m_dmac, FUNC(ibm72x7377_device::dreq_w<3>));
	// DMA 4 doesn't exist
	m_mcabus->drq_callback<5>().set(m_dmac, FUNC(ibm72x7377_device::dreq_w<5>));
	m_mcabus->drq_callback<6>().set(m_dmac, FUNC(ibm72x7377_device::dreq_w<6>));
	m_mcabus->drq_callback<7>().set(m_dmac, FUNC(ibm72x7377_device::dreq_w<7>));
}

TIMER_DEVICE_CALLBACK_MEMBER(ps2_mb_device::refresh_cb)
{
	m_refresh_bit = !m_refresh_bit;
}

void ps2_mb_device::hot_reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state);
	if(state) m_dmac->reset();
}

void ps2_mb_device::map(address_map &map)
{
	// TODO: Take into account the presence/absence of the IBM ASICs.

	map(0x0000, 0x001f).rw(m_dmac, FUNC(ibm72x7377_device::dmac1_r), FUNC(ibm72x7377_device::dmac1_w)).umask16(0xffff);
	map(0x0018, 0x0018).rw(m_dmac, FUNC(ibm72x7377_device::extended_function_register_r), FUNC(ibm72x7377_device::extended_function_register_w)).umask16(0x00ff);
	map(0x001a, 0x001a).rw(m_dmac, FUNC(ibm72x7377_device::extended_function_execute_r), FUNC(ibm72x7377_device::extended_function_execute_w)).umask16(0x00ff);
	map(0x0020, 0x0021).rw(m_pic8259_master, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0xffff);
	map(0x0040, 0x0047).rw(m_io_controller, FUNC(ibm72x8299_device::pit_r), FUNC(ibm72x8299_device::pit_w)).umask16(0xffff);
	map(0x0060, 0x0060).r(FUNC(ps2_mb_device::keybc_data_r)).w(m_keybc, FUNC(ps2_keyboard_controller_device::data_w));
	map(0x0061, 0x0061).rw(FUNC(ps2_mb_device::portb_r), FUNC(ps2_mb_device::portb_w));
	map(0x0064, 0x0064).rw(m_keybc, FUNC(ps2_keyboard_controller_device::status_r), FUNC(ps2_keyboard_controller_device::command_w));
	map(0x0070, 0x0071).r("rtc", FUNC(mc146818_device::read)).umask16(0xffff).w(FUNC(ps2_mb_device::write_rtc)).umask16(0xffff);
	if(m_sram_size != 0) map(0x0074, 0x0076).rw(FUNC(ps2_mb_device::nvram_r), FUNC(ps2_mb_device::nvram_w));
	map(0x0080, 0x008f).rw(m_dmac, FUNC(ibm72x7377_device::page8_r), FUNC(ibm72x7377_device::page8_w)).umask16(0xffff);
	map(0x0090, 0x0090).rw(m_dmac, FUNC(ibm72x7377_device::dma_arbiter_r), FUNC(ibm72x7377_device::dma_arbiter_w));
	map(0x0091, 0x0091).r(m_io_controller, FUNC(ibm72x8299_device::card_select_feedback_r)).w(m_dmac, FUNC(ibm72x7377_device::dma_feedback_w));
	map(0x0092, 0x0092).rw(FUNC(ps2_mb_device::porta_r), FUNC(ps2_mb_device::porta_w));
	map(0x0094, 0x0094).rw(m_io_controller, FUNC(ibm72x8299_device::system_board_pos_r), FUNC(ibm72x8299_device::system_board_pos_w));
	map(0x0096, 0x0096).rw(m_io_controller, FUNC(ibm72x8299_device::adapter_pos_r), FUNC(ibm72x8299_device::adapter_pos_w));
	map(0x00a0, 0x00a1).rw(m_pic8259_slave, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0xffff);
	map(0x00c0, 0x00df).rw(m_dmac, FUNC(ibm72x7377_device::dmac2_r), FUNC(ibm72x7377_device::dmac2_w)).umask16(0x00ff);
	map(0x0100, 0x0107).rw(m_io_controller, FUNC(ibm72x8299_device::pos_registers_r), FUNC(ibm72x8299_device::pos_registers_w));
	// 0x1F0 - Fixed Disk
	map(0x0680, 0x0681).w(FUNC(ps2_mb_device::post_debug_w));
}

/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/
IRQ_CALLBACK_MEMBER(ps2_mb_device::inta_cb)
{
	// Clear the latch on IRQ0, then pass the ack onto the PIC.
	//LOGIRQ("%s: acknowledging interrupt %02Xh\n", FUNCNAME, irqline);
	if(irqline == INPUT_LINE_IRQ0) { irq0_latch_reset(); }
	return m_pic8259_master->inta_cb(device, irqline);
}

uint8_t ps2_mb_device::get_slave_ack(offs_t offset)
{
	if (offset==2) // IRQ = 2
		return m_pic8259_slave->acknowledge();

	return 0x00;
}

WRITE_LINE_MEMBER( ps2_mb_device::keybc_irq_latch_w )
{
	// Latches on IRQ 1, reset by a read to 0x60.
	if(state == ASSERT_LINE) m_pic8259_master->ir1_w(state);
}

uint8_t ps2_mb_device::keybc_data_r()
{
	m_pic8259_master->ir1_w(CLEAR_LINE); // clears the latch on read
	return m_keybc->data_r();
}

/*************************************************************************
 *
 *      PC Speaker related
 *
 *************************************************************************/

void ps2_mb_device::speaker_set_spkrdata(uint8_t data)
{
	m_at_spkrdata = data ? 1 : 0;
	m_speaker->level_w(m_at_spkrdata & m_pit_out2);
}

/*************************************************************
 *
 * pit8254 configuration
 *
 *************************************************************/

WRITE_LINE_MEMBER( ps2_mb_device::pit8254_out2_changed )
{
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(m_at_spkrdata & m_pit_out2);
}

/*************************************************************************
 *
 * NVRAM and RTC
 *
 *************************************************************************/


void ps2_mb_device::write_rtc(offs_t offset, uint8_t data)
{
	if (offset==0) {
		m_nmi_enabled = BIT(data,7);
		if (!m_nmi_enabled)
			m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
		m_mc146818->write(0,data);
	}
	else {
		m_mc146818->write(offset,data);
	}
}

uint8_t ps2_mb_device::nvram_r(offs_t offset)
{
	switch(offset)
	{
		case 0:
			return (m_nvram_index & 0x00FF);
		case 1:
			return (m_nvram_index & 0x0700) >> 8;
		case 2:
			if(m_nvram_index > m_sram_size)
			{
				LOGNVRAM("NVRAM read out of range: %04X\n", m_nvram_index);
				return 0xFF;
			}
			if (!machine().side_effects_disabled())
				LOGNVRAM("reading NVRAM at %04X: %02X\n", m_nvram_index, m_sram[m_nvram_index]);
			return m_sram[m_nvram_index];
		default:
			fatalerror("nvram_r out of range");
	}
}

void ps2_mb_device::nvram_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0:
			m_nvram_index = (m_nvram_index & 0x0700) | data;
			break;
		case 1:
			m_nvram_index = (m_nvram_index & 0x00FF) | (data << 8);
			break;
		case 2:
			if(m_nvram_index > m_sram_size)
			{
				LOGNVRAM("NVRAM write out of range: %04X %02X\n", m_nvram_index, data);
				return;
			}
			if (!machine().side_effects_disabled())
				LOGNVRAM("Writing NVRAM at %04X: %02X\n", m_nvram_index, data);
			m_sram[m_nvram_index] = data;
			break;
		default:
			fatalerror("nvram_w out of range");
	}
}

WRITE_LINE_MEMBER( ps2_mb_device::shutdown )
{
	if(state)
		m_maincpu->reset();
}

WRITE_LINE_MEMBER( ps2_mb_device::irq0_w )
{
	// IRQ0 is latched, the PIT channel 3 lines are not.
	if(state) m_pic8259_master->ir0_w(state);
	m_io_controller->pit_ch3_gate_w(state);
	m_io_controller->pit_ch3_clk_w(!state);
}

void ps2_mb_device::irq0_latch_reset()
{
	m_pic8259_master->ir0_w(false);
}

WRITE_LINE_MEMBER( ps2_mb_device::watchdog_w )
{
	if(state)
	{
		LOGTIMERS("watchdog timeout, asserting NMI\n");
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
}

uint8_t ps2_mb_device::portb_r()
{
	// 0x80 = status of parity check latch
	// 0x40 = status of channel check latch
	// 0x20 = timer 2 output bit
	// 0x10 = DRAM refresh line
	// 0x08 = channel check enabled
	// 0x04 = parity check enabled
	// 0x02 = speaker gate enabled
	// 0x01 = timer 2 gate enabled

	uint8_t data = 0;

	// no parity check.
	data |= m_channel_check ? 0x40 : 0;
	data |= m_pit_out2 ? 0x20 : 0;

	/*
	 *  Bit 1 of MCR controls refresh rate.
	 *  1: rate is 0.8uS
	 *  0: rate is 15.12uS
	 */
	data |= m_refresh_bit ? 0x10 : 0;
	data |= m_channel_check_enabled ? 0x08 : 0;
	data |= m_parity_check_enabled ? 0x04 : 0;
	data |= m_at_speaker ? 0x02 : 0;
	data |= m_write_gate_2 ? 0x01 : 0;

	return data;
}

void ps2_mb_device::portb_w(uint8_t data)
{
	// 0x80 = reset IRQ 0
	// 0x08 = enable channel check
	// 0x04 = enable parity check
	// 0x02 = enable speaker gate
	// 0x01 = enable timer 2 gate

	m_at_speaker = data;
	if(BIT(data, 7)) { irq0_latch_reset(); }
	m_channel_check_enabled = BIT(data, 3);
	m_parity_check_enabled = BIT(data, 2);
	speaker_set_spkrdata(BIT(data, 1));
	m_write_gate_2 = BIT(data, 0);
	m_io_controller->pit_ch2_gate_w(m_write_gate_2);
}

WRITE_LINE_MEMBER( ps2_mb_device::iochck_w )
{
	if (!state && m_nmi_enabled && !m_channel_check)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

WRITE_LINE_MEMBER( ps2_mb_device::gate_a20_w )
{
	m_gate_a20 = state;
	m_maincpu->set_input_line(INPUT_LINE_A20, m_gate_a20);
}

uint8_t ps2_mb_device::porta_r()
{
	uint8_t data = 0;

	data |= m_nmi_flag ? 0x10 : 0;
	data |= m_gate_a20 ? 0x02 : 0;
	data |= m_alt_hot_reset ? 0x01 : 0;

	return data;
}

void ps2_mb_device::porta_w(uint8_t data)
{
	//LOGSYSPORTS("%s d:%02X\n", FUNCNAME, data);

	m_nmi_flag = BIT(data, 4);
	gate_a20_w(BIT(data, 1));

	// Bit 0 is latched so POST can read whether this is a cold boot or the system dropping to real mode.
	if(!m_alt_hot_reset && BIT(data, 0))
	{
		// 0 to 1 transition: Hot reset.
		m_alt_hot_reset = true;
		// page 4-196 of the Model 80 manual: pulse the reset line for 100-125ns
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::from_nsec(100));
	}
	else if(m_alt_hot_reset && !BIT(data,0))
	{
		// 1 to 0 transition: Clear the latch.
		m_alt_hot_reset = false;
	}
	// 1 to 1: Do nothing.
	// 0 to 0: Do nothing.
}

/************************************************************/
void ps2_mb_device::post_debug_w(offs_t offset, uint8_t data)
{
	if(offset == 0) LOGPOST("POST checkpoint: %02X\n", data);
	else LOGPOST("POST detail: %02X\n", data);
}

/* PS/2 POS registers */

uint8_t ps2_mb_device::planar_pos_r(offs_t offset)
{
	LOGPOS("%s: O:%02X\n", FUNCNAME, offset);

	switch(offset)
	{
		case MCABus::POS::ADAPTER_ID_LO: return m_planar_id & 0xFF;
		case MCABus::POS::ADAPTER_ID_HI: return (m_planar_id & 0xFF00) >> 8;
		case MCABus::POS::OPTION_SELECT_DATA_1: return m_io_controller->system_board_io_r();
		case MCABus::POS::OPTION_SELECT_DATA_2: return 0b00001010;
		default: return 0xFF;
	}
}

void ps2_mb_device::planar_pos_w(offs_t offset, uint8_t data)
{
	if(offset == 2)
	{
		// System Board I/O Byte.
		LOGPOS("PS/2 system board I/O enable bits now %02X\n", data);
		m_io_controller->system_board_io_w(data);
	}
}

uint8_t ps2_mb_device::memory_control_r()
{
	if(!machine().side_effects_disabled()) LOGPOS("%s\n", FUNCNAME);
	return m_memory_control;
}

void ps2_mb_device::memory_control_w(uint8_t data)
{
	LOGPOS("*** %s %02X\n", FUNCNAME, data);
	m_memory_control = data;
}

#define SYS_IOEN_BOARD  1
#define SYS_IOEN_FDC    2
#define SYS_IOEN_UART   4
#define SYS_IOEN_COM1   8
