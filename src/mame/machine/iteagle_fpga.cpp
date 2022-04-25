// license:BSD-3-Clause
// copyright-holders:Ted Green
#include "emu.h"
#include "iteagle_fpga.h"
#include "coreutil.h"

#define LOG_FPGA            (0)
#define LOG_SERIAL          (0)
#define LOG_RTC             (0)
#define LOG_RAM             (0)
#define LOG_EEPROM          (0)
#define LOG_PERIPH          (0)

#define AM85C30_TAG "am85c30_0"
#define COM1_TAG "com1"
#define COM2_TAG "com2"

DEFINE_DEVICE_TYPE(ITEAGLE_FPGA, iteagle_fpga_device, "iteagle_fpga", "ITEagle FPGA")

void iteagle_fpga_device::fpga_map(address_map &map)
{
	map(0x000, 0x01f).rw(FUNC(iteagle_fpga_device::fpga_r), FUNC(iteagle_fpga_device::fpga_w));
}

void iteagle_fpga_device::rtc_map(address_map &map)
{
	map(0x000, 0x7ff).rw(FUNC(iteagle_fpga_device::rtc_r), FUNC(iteagle_fpga_device::rtc_w));
}

void iteagle_fpga_device::ram_map(address_map &map)
{
	map(0x00000, 0x3f).rw(FUNC(iteagle_fpga_device::e1_nvram_r), FUNC(iteagle_fpga_device::e1_nvram_w));
	map(0x10000, 0x1ffff).rw(FUNC(iteagle_fpga_device::e1_ram_r), FUNC(iteagle_fpga_device::e1_ram_w));
}

iteagle_fpga_device::iteagle_fpga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	pci_device(mconfig, ITEAGLE_FPGA, tag, owner, clock),
	m_rtc(*this, "eagle2_rtc"),
	m_e1_nvram(*this, "eagle1_bram"),
	m_scc1(*this, AM85C30_TAG),
	m_screen(*this, finder_base::DUMMY_TAG),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_in_cb(*this),
	m_trackx_cb(*this),
	m_tracky_cb(*this),
	m_gunx_cb(*this),
	m_guny_cb(*this),
	m_version(0),
	m_seq_init(0)
{
	set_ids(0x55cc33aa, 0xaa, 0xaaaaaa, 0x00);
}

void iteagle_fpga_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, "eagle2_rtc", nvram_device::DEFAULT_ALL_0);
	NVRAM(config, "eagle1_bram", nvram_device::DEFAULT_ALL_1);

	// RS232 serial ports
	// The console terminal (com1) operates at 38400 baud
	SCC85C30(config, m_scc1, 7.3728_MHz_XTAL);
	m_scc1->configure_channels((7.3728_MHz_XTAL).value(), 0, (7.3728_MHz_XTAL).value(), 0);
	m_scc1->out_int_callback().set(FUNC(iteagle_fpga_device::serial_interrupt));
	m_scc1->out_txda_callback().set(COM2_TAG, FUNC(rs232_port_device::write_txd));
	m_scc1->out_txdb_callback().set(COM1_TAG, FUNC(rs232_port_device::write_txd));

	rs232_port_device &com1(RS232_PORT(config, COM1_TAG, default_rs232_devices, nullptr));
	com1.rxd_handler().set(m_scc1, FUNC(scc85c30_device::rxb_w));
	com1.dcd_handler().set(m_scc1, FUNC(scc85c30_device::dcdb_w));
	com1.cts_handler().set(m_scc1, FUNC(scc85c30_device::ctsb_w));

	rs232_port_device &com2(RS232_PORT(config, COM2_TAG, default_rs232_devices, nullptr));
	com2.rxd_handler().set(m_scc1, FUNC(scc85c30_device::rxa_w));
	com2.dcd_handler().set(m_scc1, FUNC(scc85c30_device::dcda_w));
	com2.cts_handler().set(m_scc1, FUNC(scc85c30_device::ctsa_w));
}

void iteagle_fpga_device::device_start()
{
	// RTC M48T02
	m_rtc->set_base(m_rtc_regs, sizeof(m_rtc_regs));

	// Eagle 1 nvram
	m_e1_nvram->set_base(m_e1_nv_data, sizeof(m_e1_nv_data));

	pci_device::device_start();
	status = 0x5555;
	command = 0x5555;

	// always keep this device memory ranges active
	command |= 3;
	command_mask &= ~3;

	add_map(sizeof(m_fpga_regs), M_IO, FUNC(iteagle_fpga_device::fpga_map));
	// fpga defaults to base address 0x00000300
	bank_infos[0].adr = 0x00000300 & (~(bank_infos[0].size - 1));

	add_map(sizeof(m_rtc_regs), M_MEM, FUNC(iteagle_fpga_device::rtc_map));
	// RTC defaults to base address 0x000c0000
	bank_infos[1].adr = 0x000c0000 & (~(bank_infos[1].size - 1));

	add_map(0x20000, M_MEM, FUNC(iteagle_fpga_device::ram_map));
	// RAM defaults to base address 0x000e0000
	bank_infos[2].adr = 0x000e0000 & (~(bank_infos[2].size - 1));

	m_timer = timer_alloc(0);

	// Switch IO
	m_in_cb.resolve_all_safe(0xffff);
	// Track IO
	m_trackx_cb.resolve_safe(0xff);
	m_tracky_cb.resolve_safe(0xff);
	// Gun IO
	m_gunx_cb.resolve_safe(0xffff);
	m_guny_cb.resolve_safe(0xffff);

	// Save states
	save_item(NAME(m_fpga_regs));
	save_item(NAME(m_rtc_regs));
	save_item(NAME(m_e1_ram));
	save_item(NAME(m_prev_reg));
	// m_version
	save_item(NAME(m_seq_init));
	save_item(NAME(m_seq));
	save_item(NAME(m_seq_rem1));
	save_item(NAME(m_seq_rem2));
}

void iteagle_fpga_device::device_reset()
{
	remap_cb();
	memset(m_fpga_regs, 0, sizeof(m_fpga_regs));
	m_seq = m_seq_init;
	m_seq_rem1 = 0;
	m_seq_rem2 = 0;

	// Nibble starting at bit 20 is resolution, byte 0 is atmel response
	// Bit 16 is Eagle 1 battery ok
	// 0x00080000 and interrupt starts reading from 0x14
	// 0x02000000 and interrupt starts reading from 0x18
	// Write 0x01000000 is a global interrupt clear
	m_fpga_regs[0x04/4] =  0x00010000;
	m_prev_reg = 0;

	m_serial0_1.reset();
	m_serial2_3.reset();
}

void iteagle_fpga_device::update_sequence(uint32_t data)
{
	uint32_t offset = 0x04/4;
	if (data & 0x80) {
		m_fpga_regs[offset] = (m_fpga_regs[offset]&0xFFFFFF00) | ((m_version>>(8*(data&3)))&0xff);
	} else {
		uint32_t val1, feed;
		feed = ((m_seq<<4) ^ m_seq)>>7;
		if (data & 0x1) {
			val1 = ((m_seq & 0x2)<<1) | ((m_seq & 0x4)>>1) | ((m_seq & 0x8)>>3);
			m_seq_rem1 = ((m_seq & 0x10)) | ((m_seq & 0x20)>>2) | ((m_seq & 0x40)>>4);
			m_seq_rem2 = ((m_seq & 0x80)>>1) | ((m_seq & 0x100)>>3) | ((m_seq & 0x200)>>5);
			m_seq = (m_seq>>9) | ((feed&0x1ff)<<15);
			m_fpga_regs[offset] = (m_fpga_regs[offset]&0xFFFFFF00) | ((val1 + m_seq_rem1 + m_seq_rem2)&0xFF);
		} else if (data & 0x2) {
			val1 = ((m_seq & 0x2)<<1) | ((m_seq & 0x4)>>1) | ((m_seq & 0x8)>>3);
			m_seq_rem1 = ((m_seq & 0x10)) | ((m_seq & 0x20)>>2) | ((m_seq & 0x40)>>4);
			m_seq = (m_seq>>6) | ((feed&0x3f)<<18);
			m_fpga_regs[offset] = (m_fpga_regs[offset]&0xFFFFFF00) | ((val1 + m_seq_rem1 + m_seq_rem2)&0xFF);
		} else {
			val1 = ((m_seq & 0x2)<<6) | ((m_seq & 0x4)<<4) | ((m_seq & 0x8)<<2) | ((m_seq & 0x10)<<0)
					| ((m_seq & 0x20)>>2) | ((m_seq & 0x40)>>4) | ((m_seq & 0x80)>>6) | ((m_seq & 0x100)>>8);
			m_seq = (m_seq>>8) | ((feed&0xff)<<16);
			m_fpga_regs[offset] = (m_fpga_regs[offset]&0xFFFFFF00) | ((val1 + m_seq_rem1 + m_seq_rem2) & 0xff);
		}
		if (0 && LOG_FPGA)
			logerror("%s:fpga update_sequence In: %02X Seq: %06X Out: %02X\n", machine().describe_context(), data, m_seq, m_fpga_regs[offset]&0xff);
	}
}

// Eagle 1 sequence generator
void iteagle_fpga_device::update_sequence_eg1(uint32_t data)
{
	uint32_t offset = 0x04/4;
	uint32_t val1, feed;
	feed = ((m_seq<<4) ^ m_seq)>>7;
	if (data & 0x1) {
		val1 = ((m_seq & 0x2)<<6) | ((m_seq & 0x4)<<4) | ((m_seq & 0x8)<<2) | ((m_seq & 0x10)<<0)
				| ((m_seq & 0x20)>>2) | ((m_seq & 0x40)>>4) | ((m_seq & 0x80)>>6) | ((m_seq & 0x100)>>8);
		m_seq = (m_seq>>8) | ((feed&0xff)<<16);
		m_fpga_regs[offset] = (m_fpga_regs[offset]&0xFFFFFF00) | ((val1 + m_seq_rem1 + m_seq_rem2)&0xFF);
	} else if (data & 0x2) {
		val1 = ((m_seq & 0x2)<<1) | ((m_seq & 0x4)>>1) | ((m_seq & 0x8)>>3);
		m_seq_rem1 = ((m_seq & 0x10)) | ((m_seq & 0x20)>>2) | ((m_seq & 0x40)>>4);
		m_seq = (m_seq>>6) | ((feed&0x3f)<<18);
		m_fpga_regs[offset] = (m_fpga_regs[offset]&0xFFFFFF00) | ((val1 + m_seq_rem1 + m_seq_rem2)&0xFF);
	} else {
		val1 = ((m_seq & 0x2)<<1) | ((m_seq & 0x4)>>1) | ((m_seq & 0x8)>>3);
		m_seq_rem1 = ((m_seq & 0x10)) | ((m_seq & 0x20)>>2) | ((m_seq & 0x40)>>4);
		m_seq_rem2 = ((m_seq & 0x80)>>1) | ((m_seq & 0x100)>>3) | ((m_seq & 0x200)>>5);
		m_seq = (m_seq>>9) | ((feed&0x1ff)<<15);
		m_fpga_regs[offset] = (m_fpga_regs[offset]&0xFFFFFF00) | ((val1 + m_seq_rem1 + m_seq_rem2) & 0xff);
	}
	if (0 && LOG_FPGA)
		logerror("%s:fpga update_sequence In: %02X Seq: %06X Out: %02X other %02X%02X%02X\n", machine().describe_context(),
			data, m_seq, m_fpga_regs[offset]&0xff, m_seq_rem2, m_seq_rem1, val1);
}

//-------------------------------------------------
//  device_timer - called when our device timer expires
//-------------------------------------------------
void iteagle_fpga_device::device_timer(emu_timer &timer, device_timer_id tid, int param)
{
	//int beamy = m_screen->vpos();
	//const rectangle &visarea = m_screen->visible_area();
	//beamy++;
	//if (beamy <= m_screen->visible_area().max_y && beamy <= m_gun_y + BEAM_DY) {
	//  m_timer->adjust(m_screen->time_until_pos(beamy, std::max(0, m_gun_x - BEAM_DX)));
	//}
	//m_fpga_regs[0x04/4] |=  0x02080000;
	m_fpga_regs[0x04 / 4] |= 0x00080000;
	m_cpu->set_input_line(m_irq_num, ASSERT_LINE);
	if (LOG_FPGA)
		logerror("%s:fpga device_timer Setting interrupt(%i)\n", machine().describe_context(), m_irq_num);
}

WRITE_LINE_MEMBER(iteagle_fpga_device::vblank_update)
{
	m_vblank_state = state;
	if (state && m_fpga_regs[0x4 / 4] & 0x01000000) {
		//m_cpu->set_input_line(m_irq_num, ASSERT_LINE);
		if (1 || (m_fpga_regs[0x14 / 4] & 0x01)) {
			// Set the gun timer to first fire
			const rectangle &visarea = m_screen->visible_area();
			m_gun_x = m_gunx_cb(0) * (visarea.width() - 14) / 512;
			m_gun_y = m_guny_cb(0) * visarea.height() / 512;
			m_timer->adjust(attotime::zero);
			//m_timer->adjust(m_screen->time_until_pos(std::max(0, m_gun_y - BEAM_DY), std::max(0, m_gun_x - BEAM_DX)));
			//printf("w: %d h: %d x: %d y: %d\n", visarea.width(), visarea.height(), m_gun_x, m_gun_y);
		}
		if (LOG_FPGA)
			logerror("%s:fpga vblank_update Setting interrupt(%i)\n", machine().describe_context(), m_irq_num);
	}
}

WRITE_LINE_MEMBER(iteagle_fpga_device::serial_interrupt)
{
	if (LOG_SERIAL) {
		logerror("serial_interrupt: intr(%i) = %i\n", m_serial_irq_num, state);
	}
	m_cpu->set_input_line(m_serial_irq_num, state);
}

void iteagle_fpga_device::serial_rx_w(uint8_t data)
{
	std::string tmpStr;
	tmpStr += data;
	//osd_printf_info("serial_rx_w: %02x\n", data);
	m_serial0_1.write_rx_str(1, tmpStr);
	if (0 && m_serial0_1.check_interrupt()) {
		m_cpu->set_input_line(m_serial_irq_num, ASSERT_LINE);
	}
}

uint32_t iteagle_fpga_device::fpga_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = m_fpga_regs[offset];

	switch (offset) {
		case 0x00/4:
			result = (m_in_cb[IO_SYSTEM](0) << 16) | (m_in_cb[IO_IN1](0));
			if (LOG_FPGA && m_prev_reg!=offset)
				logerror("%s:fpga_r offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
		case 0x04/4:
			result = (result & 0xFF0FFFFF) | ((m_in_cb[IO_SW5](0) & 0xf) << 20);
			if (0 && LOG_FPGA && !ACCESSING_BITS_0_7)
				logerror("%s:fpga_r offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
		case 0x08/4:
			result = (m_tracky_cb(0) << 8) | m_trackx_cb(0);
			if (LOG_FPGA && m_prev_reg!=offset)
				logerror("%s:fpga_r offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
		case 0x14/4: // GUN1-- Interrupt & 0x4==0x00080000
			result = (m_gun_y << 16) | (m_gun_x << 0);
			if (LOG_FPGA)
				logerror("%s:fpga_r offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
		case 0x18/4: // Interrupt & 0x4==0x02000000
			result = 0;
			if (LOG_FPGA)
				logerror("%s:fpga_r offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
		case 0x0c/4: //
			result = 0;
			// Need to eat some CPU cycles otherwise the CPU times out waiting for tx uart buffer empty
			if (ACCESSING_BITS_0_15) {
				m_cpu->eat_cycles(40);
			}
			if (ACCESSING_BITS_0_7) {
				result |= m_scc1->cb_r(offset) << 0;
				if (LOG_SERIAL) m_serial0_1.read_control(1);
			}
			if (ACCESSING_BITS_8_15) {
				result |= m_scc1->ca_r(offset) << 8;
				if (LOG_SERIAL) m_serial0_1.read_control(0);
			}
			if (ACCESSING_BITS_16_23) {
				result |= m_scc1->db_r(offset) <<16;
				if (LOG_SERIAL) m_serial0_1.read_data(1);
			}
			if (ACCESSING_BITS_24_31) {
				result |= m_scc1->da_r(offset) << 24;
				if (LOG_SERIAL) m_serial0_1.read_data(0);
			}
			if (0 && LOG_FPGA && m_prev_reg != offset)
				logerror("%s:fpga_r offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
		case 0x1c/4:
			result = 0;
			if (ACCESSING_BITS_0_7) {
				result |= m_serial2_3.read_control(1) << 0;
			}
			if (ACCESSING_BITS_8_15) {
				result |= m_serial2_3.read_control(0) << 8;
			}
			if (ACCESSING_BITS_16_23) {
				result |= m_serial2_3.read_data(1) << 16;
				logerror("fpga_r: LEDSIGN read byte: %02X\n", uint8_t(result >> 16));
			}
			if (ACCESSING_BITS_24_31) {
				result |= m_serial2_3.read_data(0) << 24;
				logerror("fpga_r: MODEM read byte: %c\n", (result >> 24) & 0xff);
			}
			// Clear interrupts
			if (ACCESSING_BITS_16_31) {
				if (!m_serial2_3.check_interrupt()) {
					m_cpu->set_input_line(m_serial_irq_num, CLEAR_LINE);
				}
			}
			if (LOG_FPGA)
				logerror("%s:fpga_r offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
		default:
			if (LOG_FPGA)
				logerror("%s:fpga_r offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			osd_printf_debug("%s:fpga_r offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
	}
	if (offset!=0x4/4)
		m_prev_reg = offset;
	return result;
}

void iteagle_fpga_device::fpga_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_fpga_regs[offset]);
	switch (offset) {
		case 0x04/4:
			if (ACCESSING_BITS_0_7) {
				if ((m_version & 0xff00) == 0x0200)
					update_sequence_eg1(data & 0xff);
				else
					// ATMEL Chip access.  Returns version id's when bit 7 is set.
					update_sequence(data & 0xff);
				if (0 && LOG_FPGA)
						logerror("%s:fpga_w offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			} else if (ACCESSING_BITS_8_15) {
				// Interrupt enable?
				if (LOG_FPGA)
						logerror("%s:fpga_w offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			} else if (ACCESSING_BITS_24_31 && (data & 0x01000000)) {
			// Interrupt clear/enable
				m_cpu->set_input_line(m_irq_num, CLEAR_LINE);
				if (LOG_FPGA)
						logerror("%s:fpga_w offset %04X = %08X & %08X Clearing interrupt(%i)\n", machine().describe_context(), offset*4, data, mem_mask, m_irq_num);
			} else {
				if (LOG_FPGA)
						logerror("%s:fpga_w offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			}
			break;
		case 0x14/4:
			if (ACCESSING_BITS_0_7 && (data&0x1)) {
				m_fpga_regs[0x04/4] &=  ~0x00080000;
			}
			if (LOG_FPGA)
					logerror("%s:fpga_w offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			break;
		case 0x18/4:
			if (ACCESSING_BITS_0_7 && (data&0x1)) {
				m_fpga_regs[0x04/4] &=  ~0x02000000;
			}
			if (LOG_FPGA)
					logerror("%s:fpga_w offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			break;
		case 0x0c/4:
			if (ACCESSING_BITS_0_7) {
				m_scc1->cb_w(offset, (data >> 0) & 0xff);
				if (LOG_SERIAL) m_serial0_1.write_control((data >> 0) & 0xff, 1);
			}
			if (ACCESSING_BITS_8_15) {
				m_scc1->ca_w(offset, (data >> 8) & 0xff);
				if (LOG_SERIAL) m_serial0_1.write_control((data >> 8) & 0xff, 0);
			}
			if (ACCESSING_BITS_16_23) {
				// Convert 0xd to 0xa
				uint8_t byte = data >> 16;
				if (byte==0xd)
					m_scc1->db_w(offset, 0xa);
				else
					m_scc1->db_w(offset, byte);
				if (LOG_SERIAL) {
					m_serial0_1.write_data((data >> 16) & 0xff, 1);
					if (m_serial0_1.get_tx_str(1).back() == 0xd) {
						logerror("com0: %s", m_serial0_1.get_tx_str(1).c_str());
						osd_printf_info("com0: %s\n", m_serial0_1.get_tx_str(1));
						m_serial0_1.clear_tx_str(1);
					}
				}
			}
			if (ACCESSING_BITS_24_31) {
				m_scc1->da_w(offset, (data >> 24) & 0xff);
				if (LOG_SERIAL) m_serial0_1.write_data((data >> 24) & 0xff, 0);
			}
			if (1 && LOG_FPGA)
					logerror("%s:fpga_w offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			break;
		case 0x1c/4:
			if (ACCESSING_BITS_0_7) {
				m_serial2_3.write_control((data >> 0) & 0xff, 1);
			}
			if (ACCESSING_BITS_8_15) {
				m_serial2_3.write_control((data >> 8) & 0xff, 0);
			}
			if (ACCESSING_BITS_16_23) {
				int chan = 1;
				m_serial2_3.write_data((data >> 16) & 0xff, chan);
				std::string txString = m_serial2_3.get_tx_str(chan);
				if (txString.length() >= 8) {
					int length = (uint8_t(txString[4]) << 8) | uint8_t(txString[5]);
					if (txString.length() >= length) {
						osd_printf_debug("com2:");
						if (LOG_SERIAL) logerror("com2:\n");
						for (int i = 0; i < txString.length(); i++) {
							if (LOG_SERIAL) logerror(" %02x", uint8_t(txString[i]));
							osd_printf_debug(" %02x", uint8_t(txString[i]));
							if ((i + 1) % 16 == 0 || i==length-1) {
								osd_printf_debug("\n");
								if (LOG_SERIAL) logerror("\n");
							}
						}
						osd_printf_debug("\n");
						// Set Sign Response ACK
						//if (txString[0]==0x01 || txString[0] == 0x23)
						m_serial2_3.write_rx_str(chan, "\x10");
						// Clear string
						m_serial2_3.clear_tx_str(chan);
						txString.clear();
					}
				}
			}
			if (ACCESSING_BITS_24_31) {
				int chan = 0;
				m_serial2_3.write_data((data >> 24) & 0xff, chan);
				if (m_serial2_3.get_tx_str(chan).back() == 0xd) {
					if (LOG_SERIAL) logerror("com3: %s\n", m_serial2_3.get_tx_str(chan).c_str());
					osd_printf_debug("com3: %s\n", m_serial2_3.get_tx_str(chan));
					if (m_serial2_3.get_tx_str(chan).find("ATI5") != -1)
						m_serial2_3.write_rx_str(chan, "OK\r181\r");
					else if (m_serial2_3.get_tx_str(chan).find("ATS0?") != -1)
						m_serial2_3.write_rx_str(chan, "0\r");
					else
						m_serial2_3.write_rx_str(chan, "OK\r");
					m_serial2_3.clear_tx_str(chan);
				}
			}
			// Set interrupt
			if (ACCESSING_BITS_16_31) {
				if (m_serial2_3.check_interrupt()) {
					m_cpu->set_input_line(m_serial_irq_num, ASSERT_LINE);
				}

			}

			if (LOG_FPGA)
					logerror("%s:fpga_w offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			break;
		default:
			if (LOG_FPGA)
					logerror("%s:fpga_w offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			osd_printf_debug("%s:fpga_w offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			break;
	}
}
//*************************************
//*  AM85c30 serial controller
//*************************************
void iteagle_am85c30::reset()
{
	memset(m_rr_regs, 0, 0x10 * 2);
	memset(m_wr_regs, 0, 0x10 * 2);
	// Set DTS, DCD, and Tx Buf Empty
	m_rr_regs[0][0] = 0x2c;
	m_rr_regs[1][0] = 0x2c;
}

void iteagle_am85c30::write_control(uint8_t data, int channel)
{
	uint8_t addr = m_wr_regs[channel][0] & 0xf;
	m_wr_regs[channel][addr] = data;
	// Reset address pointer to 0
	if (addr != 0) {
		m_wr_regs[channel][0] = 0;
		// Mirror wr2 to rr2[chan0]
		m_rr_regs[0][2] = m_wr_regs[channel][2];
		if (addr == 12 || addr == 13)
			m_rr_regs[channel][addr] = data;
	}
}

uint8_t iteagle_am85c30::read_control(int channel)
{
	uint8_t retVal;
	uint8_t addr = m_wr_regs[channel][0] & 0xf;
	retVal = m_rr_regs[channel][addr];
	// Reset address pointer to 0
	m_wr_regs[channel][0] = 0;
	return retVal;
}

void iteagle_am85c30::write_data(uint8_t data, int channel)
{
	if (0 && LOG_SERIAL) printf("chan %i: TX 0x%2X\n", channel, data);
	m_serial_tx[channel] += data;
	m_rr_regs[channel][0] |= 0x4; // Tx Buffer Empty
	// Tx Interrupt
	if (0 && (m_wr_regs[channel][1] & 0x2)) {
		// RR3 is shared between A and B
		m_rr_regs[0][3] |= 0x10 >> (channel * 3);  // 0x10 = ChanA Tx
		m_rr_regs[1][3] = m_rr_regs[0][3];
	}
	// Limit length
	if (m_serial_tx[channel].size() >= 4000) {
		if (LOG_SERIAL) printf("%s\n", m_serial_tx[channel].c_str());
		osd_printf_debug("%s\n", m_serial_tx[channel]);
		m_serial_tx[channel].clear();
	}
}

uint8_t iteagle_am85c30::read_data(int channel)
{
	uint8_t retVal = 0;
	if (!m_serial_rx[channel].empty()) {
		//logerror("fpga_r: read byte: %c\n", m_serial_rx[channel].at(0));
		retVal = m_serial_rx[channel].at(0);
		m_serial_rx[channel].erase(m_serial_rx[channel].begin());
	}
	if (m_serial_rx[channel].empty()) {
		m_rr_regs[channel][0] &= ~0x1;
		if (m_wr_regs[channel][1] & 0x18) {
			// RR3 is shared between A and B
			m_rr_regs[0][3] &= ~(0x20 >> (channel * 3)); // 0x20 = ChanA Rx
			m_rr_regs[1][3] = m_rr_regs[0][3];
		}
	}
	return retVal;
}

void iteagle_am85c30::write_rx_str(int channel, std::string resp)
{
	m_serial_rx[channel] += resp;
	m_rr_regs[channel][0] |= 0x1;
	if (m_wr_regs[channel][1] & 0x18) {
		// RR3 is shared between A and B
		m_rr_regs[0][3] |= (0x20 >> (channel * 3)); // 0x20 = ChanA Rx
		m_rr_regs[1][3] = m_rr_regs[0][3];
	}
}

//*************************************
//*  RTC M48T02
//*************************************
uint32_t iteagle_fpga_device::rtc_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = m_rtc_regs[offset];

	switch (offset) {
		default:
			if (LOG_RTC)
				logerror("%s:RTC read from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
	}
	return result;
}

void iteagle_fpga_device::rtc_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	system_time systime;
	int raw[8];

	COMBINE_DATA(&m_rtc_regs[offset]);
	switch (offset) {
		case 0x7F8/4: // M48T02 time
			if (data & mem_mask & 0x40) {
				// get the current date/time from the core
				machine().current_datetime(systime);
				raw[0] = 0x40;
				raw[1] = dec_2_bcd(systime.local_time.second);
				raw[2] = dec_2_bcd(systime.local_time.minute);
				raw[3] = dec_2_bcd(systime.local_time.hour);

				raw[4] = dec_2_bcd((systime.local_time.weekday != 0) ? systime.local_time.weekday : 7);
				raw[5] = dec_2_bcd(systime.local_time.mday);
				raw[6] = dec_2_bcd(systime.local_time.month + 1);
				raw[7] = dec_2_bcd(systime.local_time.year - 1900); // Epoch is 1900
				m_rtc_regs[0x7F8/4] = (raw[3]<<24) | (raw[2]<<16) | (raw[1]<<8) | (raw[0] <<0);
				m_rtc_regs[0x7FC/4] = (raw[7]<<24) | (raw[6]<<16) | (raw[5]<<8) | (raw[4] <<0);
				//m_rtc_regs[0x7FC/4] = (0x95<<24) | (raw[6]<<16) | (raw[5]<<8) | (raw[4] <<0);
			}
			if (LOG_RTC)
				logerror("%s:RTC write to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);

			break;
		default:
			if (LOG_RTC)
				logerror("%s:RTC write to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			break;
	}

}

//*************************************
//*  FPGA NV RAM -- Eagle 1 only
//*************************************
uint32_t iteagle_fpga_device::e1_nvram_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = m_e1_nv_data[offset];
	if (LOG_RAM)
		logerror("FPGA e1_nvram_r from offset %04X = %08X & %08X\n", offset * 4, result, mem_mask);
	return result;
}

void iteagle_fpga_device::e1_nvram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_e1_nv_data[offset]);
	if (LOG_RAM)
		logerror("FPGA e1_ram_w to offset %04X = %08X & %08X\n", offset * 4, data, mem_mask);
}
//*************************************
//*  FPGA RAM -- Eagle 1 only
//*************************************
uint32_t iteagle_fpga_device::e1_ram_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = m_e1_ram[offset];
	if (LOG_RAM)
		logerror("%s:FPGA e1_ram_r from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
	return result;
}

void iteagle_fpga_device::e1_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_e1_ram[offset]);
	if (LOG_RAM)
		logerror("%s:FPGA e1_ram_w to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
}

//************************************
// Attached serial EEPROM
//************************************

DEFINE_DEVICE_TYPE(ITEAGLE_EEPROM, iteagle_eeprom_device, "iteagle_eeprom", "ITEagle EEPROM AT93C46")

void iteagle_eeprom_device::eeprom_map(address_map &map)
{
	map(0x0000, 0x000F).rw(FUNC(iteagle_eeprom_device::eeprom_r), FUNC(iteagle_eeprom_device::eeprom_w));
}

void iteagle_eeprom_device::device_add_mconfig(machine_config &config)
{
	EEPROM_93C46_16BIT(config, "eeprom");
}

iteagle_eeprom_device::iteagle_eeprom_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, ITEAGLE_EEPROM, tag, owner, clock)
	, m_sw_version(0), m_hw_version(0), m_eeprom(*this, "eeprom")
{
	set_ids(0x80861229, 0x02, 0x020000, 0x00);

	// When corrupt writes 0x3=2, 0x3e=2, 0xa=0, 0x30=0
	// 0x4 = HW Version - 6-8 is GREEN board PCB, 9 is RED board PCB
	// 0x5 = Serial Num + top byte of 0x4
	// 0x6 = OperID
	// 0xd = GT Fore Tournament Board
	// 0xe = SW Version
	// 0xf = 0x01 for extra courses
	// 0x3e = 0x0002 for good nvram
	// 0x3f = checksum
	m_iteagle_default_eeprom =
	{ {
		0xd000,0x0022,0x0000,0x0003,0x1209,0x1111,0x2222,0x1234,
		0x0000,0x0000,0x0000,0x0000,0xcd00,0x0001,0x0000,0x0001,
		0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
		0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
		0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
		0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
		0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
		0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0002,0x0000
	} };
}

void iteagle_eeprom_device::device_start()
{
	// EEPROM: Set software version and calc crc
	m_iteagle_default_eeprom[0xe] = m_sw_version;
	m_iteagle_default_eeprom[0x4] = (m_iteagle_default_eeprom[0x4] & 0xff00) | m_hw_version;
	uint16_t checkSum = 0;
	for (int i=0; i<0x3f; i++) {
		checkSum += m_iteagle_default_eeprom[i];
	//logerror("eeprom init i: %x data: %04x\n", i, iteagle_default_eeprom[i]);
	}
	m_iteagle_default_eeprom[0x3f] = checkSum;

	m_eeprom->default_data(m_iteagle_default_eeprom.data(), 0x80);

	pci_device::device_start();
	skip_map_regs(1);
	add_map(0x10, M_IO, FUNC(iteagle_eeprom_device::eeprom_map));

	// always keep this device memory ranges active
	command |= 3;
	command_mask &= ~3;
}

void iteagle_eeprom_device::device_reset()
{
	pci_device::device_reset();
}

void iteagle_eeprom_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
							uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	m_memory_space = memory_space;
}

uint32_t iteagle_eeprom_device::eeprom_r(offs_t offset, uint32_t mem_mask)
{
	uint32_t result = 0;

	switch (offset) {
		case 0xC/4: // I2C Handler
			if (ACCESSING_BITS_16_23) {
				result = m_eeprom->do_read()<<(16+3);
				if (LOG_EEPROM)
					logerror("%s:eeprom_r from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			}   else {
					logerror("%s:eeprom_r from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			}
			break;
		default:
				logerror("%s:eeprom read from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
	}
	return result;
}

void iteagle_eeprom_device::eeprom_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	switch (offset) {
		case 0x8/4: // 8255x PORT command
			if ((data&0xf)==0x1) {
				// Self test for ethernet controller
				m_memory_space->write_dword((data&0xfffffff0) | 0x4, 0x0);
				logerror("%s:eeprom_w to offset %04X = %08X & %08X Self Test\n", machine().describe_context(), offset*4, data, mem_mask);
			}
			break;
		case 0xC/4: // I2C Handler
			if (ACCESSING_BITS_16_23) {
				m_eeprom->di_write((data  & 0x040000) >> (16+2));
				m_eeprom->cs_write((data  & 0x020000) ? ASSERT_LINE : CLEAR_LINE);
				m_eeprom->clk_write((data & 0x010000) ? ASSERT_LINE : CLEAR_LINE);
				if (LOG_EEPROM)
					logerror("%s:eeprom_w to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			}   else {
				//if (LOG_EEPROM)
					logerror("%s:eeprom_w to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			}
			break;
		default:
			//if (LOG_EEPROM)
				logerror("%s:eeprom write to offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, data, mem_mask);
			break;
	}
}

//************************************
// Attached Peripheral Controller
//************************************

void iteagle_periph_device::device_add_mconfig(machine_config &config)
{
	NVRAM(config, "eagle1_rtc", nvram_device::DEFAULT_ALL_0);
}

DEFINE_DEVICE_TYPE(ITEAGLE_PERIPH, iteagle_periph_device, "iteagle_periph", "ITEagle Peripheral Controller")

void iteagle_periph_device::ctrl_map(address_map &map)
{
	map(0x000, 0x0cf).rw(FUNC(iteagle_periph_device::ctrl_r), FUNC(iteagle_periph_device::ctrl_w));
}

iteagle_periph_device::iteagle_periph_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pci_device(mconfig, ITEAGLE_PERIPH, tag, owner, clock)
	, m_rtc(*this, "eagle1_rtc")
{
	set_ids(0x1080c693, 0x00, 0x060100, 0x00);
}

void iteagle_periph_device::device_start()
{
	pci_device::device_start();
	pci_device::set_multifunction_device(true);
	add_map(sizeof(m_ctrl_regs), M_IO, FUNC(iteagle_periph_device::ctrl_map));
	// ctrl defaults to base address 0x00000000
	bank_infos[0].adr = 0x000;

	m_rtc_regs[0xa] = 0x20; // 32.768 MHz
	m_rtc_regs[0xb] = 0x02; // 24-hour format
	m_rtc->set_base(m_rtc_regs, sizeof(m_rtc_regs));

	// always keep this device memory ranges active
	command |= 3;
	command_mask &= ~3;

	// Save states
	save_item(NAME(m_ctrl_regs));
}

void iteagle_periph_device::device_reset()
{
	pci_device::device_reset();
	memset(m_ctrl_regs, 0, sizeof(m_ctrl_regs));
	// Bit 0: might be lan chip present.
	// Bit 1: 0 == 8 Meg Ram @ 0x0
	// Bit 1: 1 == 32 Meg Simm @ 0x0, 8 Meg Ram @ 0x02000000
	// Top 16 bits are compared to 0x3
	m_ctrl_regs[0x10/4] =  0x00040002;
}

uint32_t iteagle_periph_device::ctrl_r(offs_t offset, uint32_t mem_mask)
{
	system_time systime;
	uint32_t result = m_ctrl_regs[offset];
	switch (offset) {
		case 0x0/4:
			if (LOG_PERIPH)
				logerror("%s:fpga ctrl_r from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			osd_printf_debug("%s:fpga ctrl_r from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
		case 0x70/4:
			if (ACCESSING_BITS_8_15) {
				if (m_rtc_regs[m_ctrl_regs[0x70 / 4] & 0xff] < 0x10) {
					// get the current date/time from the core
					machine().current_datetime(systime);
					m_rtc_regs[0] = dec_2_bcd(systime.local_time.second);
					m_rtc_regs[1] = 0x00; // Seconds Alarm
					m_rtc_regs[2] = dec_2_bcd(systime.local_time.minute);
					m_rtc_regs[3] = 0x00; // Minutes Alarm
					m_rtc_regs[4] = dec_2_bcd(systime.local_time.hour);
					m_rtc_regs[5] = 0x00; // Hours Alarm

					m_rtc_regs[6] = dec_2_bcd((systime.local_time.weekday != 0) ? systime.local_time.weekday : 7);
					m_rtc_regs[7] = dec_2_bcd(systime.local_time.mday);
					m_rtc_regs[8] = dec_2_bcd(systime.local_time.month + 1);
					m_rtc_regs[9] = dec_2_bcd(systime.local_time.year - 1900); // Epoch is 1900
					//m_rtc_regs[9] = 0x99; // Use 1998
					//m_rtc_regs[0xa] &= ~0x10; // Reg A Status
					//m_ctrl_regs[0xb] &= 0x10; // Reg B Status
					//m_ctrl_regs[0xc] &= 0x10; // Reg C Interrupt Status
					m_rtc_regs[0xd] = 0x80; // Reg D Valid time/ram Status
				}
				result = (result & 0xffff00ff) | (m_rtc_regs[m_ctrl_regs[0x70/4]&0xff]<<8);
			}
			else if (ACCESSING_BITS_24_31) {
				// High 128 bytes of rtc ram
				result = (result & 0x00ffffff) | (m_rtc_regs[m_ctrl_regs[0x72 / 4] & 0xff] << 24);
			}
			if (LOG_PERIPH)
				logerror("%s:fpga ctrl_r from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
		default:
			if (LOG_PERIPH)
				logerror("%s:fpga ctrl_r from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			osd_printf_debug("%s:fpga ctrl_r from offset %04X = %08X & %08X\n", machine().describe_context(), offset*4, result, mem_mask);
			break;
	}
	return result;
}

void iteagle_periph_device::ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_ctrl_regs[offset]);
	switch (offset) {
		case 0x20/4: // IDE LED
			if (ACCESSING_BITS_16_23) {
				// Sets register index
			} else if (ACCESSING_BITS_24_31) {
				// Bit 25 is IDE LED
			} else {
			}
			break;
		case 0x70/4:
			if (ACCESSING_BITS_8_15) {
				m_rtc_regs[m_ctrl_regs[0x70/4]&0xff] = (data>>8)&0xff;
			}
			break;
		default:
			break;
	}
	if (LOG_PERIPH)
		logerror("%s:fpga ctrl_w to offset %04X = %08X & %08X\n", machine().describe_context(), offset * 4, data, mem_mask);
}
