#include "es1373.h"

#define LOG_ES            (1)

const device_type ES1373 = &device_creator<es1373_device>;

DEVICE_ADDRESS_MAP_START(map, 32, es1373_device)
	AM_RANGE(0x00, 0x3f) AM_READWRITE  (reg_r,  reg_w)
ADDRESS_MAP_END

es1373_device::es1373_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: pci_device(mconfig, ES1373, "Creative Labs Ensoniq AudioPCI97 ES1373", tag, owner, clock, "es1373", __FILE__)
{
}

void es1373_device::device_start()
{
	pci_device::device_start();
	add_map(0x40, M_IO, FUNC(es1373_device::map));
}

void es1373_device::device_reset()
{
	pci_device::device_reset();
	memset(m_es_regs, 0, sizeof(m_es_regs));
	memset(m_ac97_regs, 0, sizeof(m_ac97_regs));
	m_ac97_regs[0] = 0x0800;
}

READ32_MEMBER (es1373_device::reg_r)
{
	UINT32 result = m_es_regs[offset];
	switch (offset) {
		case ES_CODEC:
			break;
		case ES_HOST_IF0: // 0x30
			switch (m_es_regs[ES_MEM_PAGE]&0xf) {
				case 0xc:
					result = m_dac1_fr.pci_addr;
					break;
				case 0xd:
					result = m_adc_fr.pci_addr;
					break;
				case 0xe:
				case 0xf:
					logerror("%06X:ES1373 Read UART offset %02X & %08X\n", space.device().safe_pc(), offset*4, mem_mask);
				default:
					break;
			}
			break;
		case ES_HOST_IF1: // 0x34
			switch (m_es_regs[ES_MEM_PAGE]&0xf) {
				case 0xc:
					result = (m_dac1_fr.curr_count<<16) | m_dac1_fr.buff_size;
					break;
				case 0xd:
					result = (m_adc_fr.curr_count<<16) | m_adc_fr.buff_size;
					break;
				case 0xe:
				case 0xf:
					logerror("%06X:ES1373 write UART offset %02X & %08X\n", space.device().safe_pc(), offset*4, mem_mask);
				default:
					break;
			}
			break;
		case ES_HOST_IF2: // 0x38
			switch (m_es_regs[ES_MEM_PAGE]&0xf) {
				case 0xc:
					result = m_dac2_fr.pci_addr;
					break;
				case 0xd:
					logerror("%06X:ES1373 read Unknown place offset %02X & %08X\n", space.device().safe_pc(), offset*4, mem_mask);
					break;
				case 0xe:
				case 0xf:
					logerror("%06X:ES1373 read UART offset %02X  & %08X\n", space.device().safe_pc(), offset*4, mem_mask);
				default:
					break;
			}
			break;
		case ES_HOST_IF3: // 0x3C
			switch (m_es_regs[ES_MEM_PAGE]&0xf) {
				case 0xc:
					result = (m_dac2_fr.curr_count<<16) | m_dac2_fr.buff_size;
					break;
				case 0xd:
					logerror("%06X:ES1373 read Unknown place offset %02X & %08X\n", space.device().safe_pc(), offset*4, mem_mask);
					break;
				case 0xe:
				case 0xf:
					logerror("%06X:ES1373 read UART offset %02X & %08X\n", space.device().safe_pc(), offset*4, mem_mask);
				default:
					break;
			}
			break;
		default:
			break;
	}
	if (LOG_ES)
		logerror("%06X:ES1373 read from offset %02X = %08X & %08X\n", space.device().safe_pc(), offset*4, result, mem_mask);
	return result;
}

WRITE32_MEMBER(es1373_device::reg_w)
{
	COMBINE_DATA(&m_es_regs[offset]);
	switch (offset) {
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
		case ES_HOST_IF0: // 0x30
			switch (m_es_regs[ES_MEM_PAGE]&0xf) {
				case 0xc:
					m_dac1_fr.pci_addr = data;
					break;
				case 0xd:
					m_adc_fr.pci_addr = data;
					break;
				case 0xe:
				case 0xf:
					logerror("%06X:ES1373 write UART offset %02X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);
				default:
					break;
			}
			break;
		case ES_HOST_IF1: // 0x34
			switch (m_es_regs[ES_MEM_PAGE]&0xf) {
				case 0xc:
					m_dac1_fr.curr_count = (data>>16)&0xffff;
					m_dac1_fr.buff_size = data&0xffff;
					break;
				case 0xd:
					m_adc_fr.curr_count = (data>>16)&0xffff;
					m_adc_fr.buff_size = data&0xffff;
					break;
				case 0xe:
				case 0xf:
					logerror("%06X:ES1373 write UART offset %02X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);
				default:
					break;
			}
			break;
		case ES_HOST_IF2: // 0x38
			switch (m_es_regs[ES_MEM_PAGE]&0xf) {
				case 0xc:
					m_dac2_fr.pci_addr = data;
					break;
				case 0xd:
					logerror("%06X:ES1373 write Unknown place offset %02X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);
					break;
				case 0xe:
				case 0xf:
					logerror("%06X:ES1373 write UART offset %02X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);
				default:
					break;
			}
			break;
		case ES_HOST_IF3: // 0x3C
			switch (m_es_regs[ES_MEM_PAGE]&0xf) {
				case 0xc:
					m_dac2_fr.curr_count = (data>>16)&0xffff;
					m_dac2_fr.buff_size = data&0xffff;
					break;
				case 0xd:
					logerror("%06X:ES1373 write Unknown place offset %02X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);
					break;
				case 0xe:
				case 0xf:
					logerror("%06X:ES1373 write UART offset %02X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);
				default:
					break;
			}
			break;
		default:
			break;
	}

	if (LOG_ES)
		logerror("%06X:ES1373 write to offset %02X = %08X & %08X\n", space.device().safe_pc(), offset*4, data, mem_mask);

}
