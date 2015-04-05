#include "es1373.h"

#define LOG_ES            (1)
#define LOG_ES_REG        (0)

static MACHINE_CONFIG_FRAGMENT( es1373 )
	MCFG_TIMER_DRIVER_ADD_PERIODIC("sound_timer", es1373_device, es_timer_callback,  attotime::from_hz(44100/16384))
MACHINE_CONFIG_END

machine_config_constructor es1373_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( es1373 );
}

const device_type ES1373 = &device_creator<es1373_device>;

DEVICE_ADDRESS_MAP_START(map, 32, es1373_device)
	AM_RANGE(0x00, 0x3f) AM_READWRITE  (reg_r,  reg_w)
ADDRESS_MAP_END

es1373_device::es1373_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, ES1373, "Creative Labs Ensoniq AudioPCI97 ES1373", tag, owner, clock, "es1373", __FILE__),
		m_irq_num(-1)
{
}

void es1373_device::set_irq_info(const char *tag, const int irq_num)
{
	m_cpu_tag = tag;
	m_irq_num = irq_num;
}

void es1373_device::device_start()
{
	//m_cpu = machine().device<cpu_device>(":maincpu");
	m_cpu = machine().device<cpu_device>(m_cpu_tag);
	pci_device::device_start();
	add_map(0x40, M_IO, FUNC(es1373_device::map));
}

void es1373_device::device_reset()
{
	pci_device::device_reset();
	memset(m_es_regs, 0, sizeof(m_es_regs));
	memset(m_ac97_regs, 0, sizeof(m_ac97_regs));
	m_ac97_regs[0] = 0x0800;
	// Reset ADC channel info
	m_adc.enable = false;
	m_adc.int_en = false;
	m_adc.loop_en = false;
	m_adc.initialized = false;
	m_adc.buf_count = 0;
	m_adc.buf_size = 0;
	m_adc.buf_rptr = 0x20;
	m_adc.buf_wptr = 0x20;
	// Reset DAC1 channel info
	m_dac1.enable = false;
	m_dac1.int_en = false;
	m_dac1.loop_en = false;
	m_dac1.initialized = false;
	m_dac1.buf_count = 0;
	m_dac1.buf_size = 0;
	m_dac1.buf_rptr = 0x0;
	m_dac1.buf_wptr = 0x0;
	// Reset DAC2 channel info
	m_dac2.enable = false;
	m_dac2.int_en = false;
	m_dac2.loop_en = false;
	m_dac2.initialized = false;
	m_dac2.buf_count = 0;
	m_dac2.buf_size = 0;
	m_dac2.buf_rptr = 0x10;
	m_dac2.buf_wptr = 0x10;
}

void es1373_device::map_extra(UINT64 memory_window_start, UINT64 memory_window_end, UINT64 memory_offset, address_space *memory_space,
							UINT64 io_window_start, UINT64 io_window_end, UINT64 io_offset, address_space *io_space)
{
	m_memory_space = memory_space;
}

TIMER_DEVICE_CALLBACK_MEMBER(es1373_device::es_timer_callback)
{
	// Only transfer PCI data if bus mastering is enabled
	if (command & 0x4) {
		if (m_dac2.enable && (!(m_dac2.buf_rptr&0x7))) {
			transfer_pci_audio(m_dac2, ES_PCI_READ);
		}
		if (m_dac2.pci_count>8) {
			m_dac2.initialized = true;
		}
	}
	if (m_dac2.enable) {
		// The initalized is to signal that inital buffer has been written
		if (m_dac2.buf_count<=m_dac2.buf_size && m_dac2.initialized) {
			// Send data to sound???
			// sound = m_sound_cache[chan.buf_rptr] 
			if (0 && LOG_ES) 
				logerror("%X: DAC2 buf_count: %i buf_size: %X buf_rptr: %X buf_wptr: %X\n", machine().device("maincpu")->safe_pc(), 
					m_dac2.buf_count, m_dac2.buf_size, m_dac2.buf_rptr, m_dac2.buf_wptr);
			if (m_dac2.buf_count==m_dac2.buf_size) {
				if (m_dac2.int_en) {
					m_es_regs[ES_INT_CS_STATUS] |= ICSTATUS_DAC2_INT_MASK;
					if (LOG_ES)
						logerror("%X: es_timer_callback Setting DAC2 interrupt\n", machine().device("maincpu")->safe_pc());
				}
				if (m_dac2.loop_en) {
					// Keep playing
					m_dac2.buf_count = m_dac2.buf_count + 1 - 4;  // Should check SCTRL_P2_END_MASK
				} else {
					// Stop
					//m_dac2.enable = false;
				}
			} else {
				m_dac2.buf_count++;
			}
			m_dac2.buf_rptr++;
			if (!(m_dac2.buf_rptr&0xf)) {
				m_dac2.buf_rptr -= 0x10;
			}
		}
	}
	if (m_adc.enable) {
		if (m_adc.buf_count<=m_adc.buf_size) {
			if (LOG_ES) 
				logerror("%s: ADC buf_count: %i buf_size: %i buf_rptr: %i buf_wptr: %i\n", machine().describe_context(), 
					m_adc.buf_count, m_adc.buf_size, m_adc.buf_rptr, m_adc.buf_wptr);
			if (m_adc.int_en && m_adc.buf_count==m_adc.buf_size) {
				m_es_regs[ES_INT_CS_STATUS] |= ICSTATUS_ADC_INT_MASK;
				if (LOG_ES)
					logerror("%s: es_timer_callback Setting ADC interrupt\n", tag());
			}
			m_adc.buf_count++;
			m_adc.buf_wptr++;
			if (!(m_adc.buf_wptr&0xf)) {
				m_adc.buf_wptr -= 0x10;
			}
		}
	}
	// PCI Write Transfer
	if (command & 0x4) {
		if (m_adc.enable && (!(m_adc.buf_wptr&0x7))) {
			transfer_pci_audio(m_adc, ES_PCI_WRITE);
		}
	}
	if (m_es_regs[ES_INT_CS_STATUS]&(ICSTATUS_DAC1_INT_MASK|ICSTATUS_DAC2_INT_MASK|ICSTATUS_ADC_INT_MASK)) {
		m_es_regs[ES_INT_CS_STATUS] |= ICSTATUS_INTR_MASK;
		// Assert interrupt
		//m_cpu->set_input_line(ES_IRQ_NUM, ASSERT_LINE);
		if (m_irq_num!=-1) {
			m_cpu->set_input_line(m_irq_num, ASSERT_LINE);
		}
	}
}

void es1373_device::transfer_pci_audio(chan_info& chan, int type)
{
	UINT32 pci_addr, data;
	pci_addr = chan.pci_addr + (chan.pci_count<<2);
	if (type==ES_PCI_READ) {
		// Transfer from PCI to sound cache
		// Always transfer 8 longwords
		for (int i=0; i<8 && (chan.pci_count<=chan.pci_size); i++) {
			data = m_memory_space->read_dword(pci_addr, 0xffffffff);
			m_sound_cache[chan.buf_wptr++] = data;
			if (!(chan.buf_wptr&0xf)) {
				chan.buf_wptr -= 0x10;
			}
			chan.pci_count++;
			pci_addr += 4;
		}
	} else {
		// Transfer from sound cache to PCI
		// Always transfer 8 longwords
		for (int i=0; i<8 && chan.pci_count<=chan.pci_size; i++) {
			data = m_sound_cache[chan.buf_rptr++];
			m_memory_space->write_dword(pci_addr, data);
			if (!(chan.buf_rptr&0xf)) {
				chan.buf_rptr -= 0x10;
			}
			chan.pci_count++;
			pci_addr += 4;
		}
	}
}

READ32_MEMBER (es1373_device::reg_r)
{
	UINT32 result = m_es_regs[offset];
	switch (offset) {
		case ES_CODEC:
			break;
		case ES_DAC2_CNT:
				result = ((m_dac2.buf_size-m_dac2.buf_count)<<16) | m_dac2.buf_size;
			break;
		case ES_HOST_IF0: // 0x30
			result = m_sound_cache[(m_es_regs[ES_MEM_PAGE]<<2) | 0x0];
			switch (m_es_regs[ES_MEM_PAGE]&0xf) {
				case 0xc:
					result = m_dac1.pci_addr;
					break;
				case 0xd:
					result = m_adc.pci_addr;
					break;
				default:
					break;
			}
			break;
		case ES_HOST_IF1: // 0x34
			result = m_sound_cache[(m_es_regs[ES_MEM_PAGE]<<2) | 0x1];
			switch (m_es_regs[ES_MEM_PAGE]&0xf) {
				case 0xc:
					result = (m_dac1.pci_count<<16) | m_dac1.pci_size;
					break;
				case 0xd:
					result = (m_adc.pci_count<<16) | m_adc.pci_size;
					break;
				default:
					break;
			}
			break;
		case ES_HOST_IF2: // 0x38
			result = m_sound_cache[(m_es_regs[ES_MEM_PAGE]<<2) | 0x2];
			switch (m_es_regs[ES_MEM_PAGE]&0xf) {
				case 0xc:
					result = m_dac2.pci_addr;
					break;
				default:
					break;
			}
			break;
		case ES_HOST_IF3: // 0x3C
			result = m_sound_cache[(m_es_regs[ES_MEM_PAGE]<<2) | 0x3];
			switch (m_es_regs[ES_MEM_PAGE]&0xf) {
				case 0xc:
					result = ((m_dac2.pci_count)<<16) | m_dac2.pci_size;
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
	if (LOG_ES_REG)
		logerror("%08X:ES1373 read from offset %02X = %08X & %08X\n", machine().device("maincpu")->safe_pc(), offset*4, result, mem_mask);
	return result;
}

WRITE32_MEMBER(es1373_device::reg_w)
{
	COMBINE_DATA(&m_es_regs[offset]);
	switch (offset) {
		case ES_INT_CS_CTRL:
				m_dac1.enable = (m_es_regs[ES_INT_CS_CTRL] & ICCTRL_DAC1_EN_MASK);
				m_dac2.enable = (m_es_regs[ES_INT_CS_CTRL] & ICCTRL_DAC2_EN_MASK);
				m_adc.enable = (m_es_regs[ES_INT_CS_CTRL] & ICCTRL_ADC_EN_MASK);
			break;
		case ES_SRC_IF:
			if (data&(1<<24)) {
				// Write to Sample Rate Converter Ram
				m_src_ram[(data>>25)&0x7F] = data&0xFFFF;
			} else {
				// Read From Sample Rate Converter Ram
				m_es_regs[offset] = (data&0xFFFF0000) | m_src_ram[(data>>25)&0x7F];
			}
			break;
		case ES_CODEC:
			if (data&(1<<23)) {
				// Read from AC97 codec registers
				m_es_regs[offset] = (data&0xFFFF0000) | m_ac97_regs[(data>>16)&0x7f] | 0x80000000;
			} else {
				// Write to AC97 codec registers
				m_ac97_regs[(data>>16)&0x7f] = data&0xFFFF;
			}
			break;
		case ES_SERIAL_CTRL:
				m_adc.loop_en  = !(m_es_regs[ES_SERIAL_CTRL] & SCTRL_R1_LOOP_MASK);
				m_dac2.loop_en = !(m_es_regs[ES_SERIAL_CTRL] & SCTRL_P2_LOOP_MASK);
				m_dac1.loop_en = !(m_es_regs[ES_SERIAL_CTRL] & SCTRL_P1_LOOP_MASK);
				m_adc.int_en  = m_es_regs[ES_SERIAL_CTRL] & SCTRL_R1_INT_EN_MASK;
				m_dac2.int_en = m_es_regs[ES_SERIAL_CTRL] & SCTRL_P2_INT_EN_MASK;
				m_dac1.int_en = m_es_regs[ES_SERIAL_CTRL] & SCTRL_P1_INT_EN_MASK;
				if (!m_adc.int_en) m_es_regs[ES_INT_CS_STATUS]  &= ~ICSTATUS_ADC_INT_MASK;
				if (!m_dac1.int_en) m_es_regs[ES_INT_CS_STATUS] &= ~ICSTATUS_DAC1_INT_MASK;
				if (!m_dac2.int_en) m_es_regs[ES_INT_CS_STATUS] &= ~ICSTATUS_DAC2_INT_MASK;
				// Clear the summary interrupt and irq line
				if (!(m_es_regs[ES_INT_CS_STATUS]&(ICSTATUS_DAC1_INT_MASK|ICSTATUS_DAC2_INT_MASK|ICSTATUS_ADC_INT_MASK))) {
					// Deassert interrupt
					if (m_es_regs[ES_INT_CS_STATUS]&ICSTATUS_INTR_MASK && m_irq_num!=-1) {
						m_cpu->set_input_line(m_irq_num, CLEAR_LINE);
						m_es_regs[ES_INT_CS_STATUS] &= ~ICSTATUS_INTR_MASK;
						if (LOG_ES)
							logerror("%X: es1373_device::reg_w Clearing interrupt\n", machine().device("maincpu")->safe_pc());
					}
				}
				if (LOG_ES_REG)
					logerror("%s: es1373_device::reg_w adc_int_en: %i dac1_int_en: %i dac2_int_en: %i\n", tag(), m_adc.int_en, m_dac1.int_en, m_dac2.int_en);
			break;
		case ES_DAC2_CNT:
				m_dac2.buf_count = 0;
				m_dac2.buf_size = data&0xffff;
			break;
		case ES_HOST_IF0: // 0x30
			m_sound_cache[(m_es_regs[ES_MEM_PAGE]<<2) | 0x0] = data;
			switch (m_es_regs[ES_MEM_PAGE]&0xf) {
				case 0xc:
					m_dac1.pci_addr = data;
					break;
				case 0xd:
					m_adc.pci_addr = data;
					break;
				default:
					break;
			}
			break;
		case ES_HOST_IF1: // 0x34
			m_sound_cache[(m_es_regs[ES_MEM_PAGE]<<2) | 0x1] = data;
			switch (m_es_regs[ES_MEM_PAGE]&0xf) {
				case 0xc:
					m_dac1.pci_count = (data>>16)&0xffff;
					m_dac1.pci_size = data&0xffff;
					break;
				case 0xd:
					m_adc.pci_count = (data>>16)&0xffff;
					m_adc.pci_size = data&0xffff;
					break;
				default:
					break;
			}
			break;
		case ES_HOST_IF2: // 0x38
			m_sound_cache[(m_es_regs[ES_MEM_PAGE]<<2) | 0x2] = data;
			switch (m_es_regs[ES_MEM_PAGE]&0xf) {
				case 0xc:
					m_dac2.pci_addr = data;
					break;
				default:
					break;
			}
			break;
		case ES_HOST_IF3: // 0x3C
			m_sound_cache[(m_es_regs[ES_MEM_PAGE]<<2) | 0x3] = data;
			switch (m_es_regs[ES_MEM_PAGE]&0xf) {
				case 0xc:
					m_dac2.pci_count = (data>>16)&0xffff;
					m_dac2.pci_size = data&0xffff;
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}

	if (LOG_ES_REG)
		logerror("%08X:ES1373 write to offset %02X = %08X & %08X\n", machine().device("maincpu")->safe_pc(), offset*4, data, mem_mask);

}
