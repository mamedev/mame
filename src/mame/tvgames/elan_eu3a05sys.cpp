// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "elan_eu3a05sys.h"

// DMA size and destination are 16-bit here, they're 24-bit on EU3A14

DEFINE_DEVICE_TYPE(ELAN_EU3A05_SYS, elan_eu3a05sys_device, "elan_eu3a05sys", "Elan EU3A05 System")
DEFINE_DEVICE_TYPE(ELAN_EU3A13_SYS, elan_eu3a13sys_device, "elan_eu3a13sys", "Elan EU3A13 System")

elan_eu3a05sys_device::elan_eu3a05sys_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	elan_eu3a05commonsys_device(mconfig, type, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_space_config("regs", ENDIANNESS_NATIVE, 8, 5, 0, address_map_constructor(FUNC(elan_eu3a05sys_device::map), this))
{
}

elan_eu3a05sys_device::elan_eu3a05sys_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	elan_eu3a05sys_device(mconfig, ELAN_EU3A05_SYS, tag, owner, clock)
{
}

elan_eu3a13sys_device::elan_eu3a13sys_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	elan_eu3a05sys_device(mconfig, ELAN_EU3A13_SYS, tag, owner, clock)
{
	// might be a difference on this hardware type, as rad_ftet (EU3A13) needs it, but rad_sinv (EU3A05) does not
	m_bank_on_low_bank_writes = true;
}


device_memory_interface::space_config_vector elan_eu3a05sys_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

void elan_eu3a05sys_device::map(address_map& map)
{
	elan_eu3a05commonsys_device::map(map); // 00 - 0e
	map(0x0f, 0x15).rw(FUNC(elan_eu3a05sys_device::dma_param_r), FUNC(elan_eu3a05sys_device::dma_param_w));
	map(0x16, 0x16).rw(FUNC(elan_eu3a05sys_device::elan_eu3a05_dmatrg_r), FUNC(elan_eu3a05sys_device::elan_eu3a05_dmatrg_w));
}

void elan_eu3a05sys_device::device_start()
{
	elan_eu3a05commonsys_device::device_start();

	save_item(NAME(m_dmaparams));
}

void elan_eu3a05sys_device::device_reset()
{
	elan_eu3a05commonsys_device::device_reset();

	for (int i = 0; i < 7; i++)
		m_dmaparams[i] = 0x00;
}

uint8_t elan_eu3a05sys_device::dma_param_r(offs_t offset)
{
	return m_dmaparams[offset];
}

void elan_eu3a05sys_device::dma_param_w(offs_t offset, uint8_t data)
{
	m_dmaparams[offset] = data;
}


uint8_t elan_eu3a05sys_device::elan_eu3a05_dmatrg_r()
{
	logerror("%s: elan_eu3a05_dmatrg_r (DMA operation state?)\n", machine().describe_context());
	return 0x00;//m_dmatrg_data;
}


void elan_eu3a05sys_device::elan_eu3a05_dmatrg_w(uint8_t data)
{
	logerror("%s: elan_eu3a05_dmatrg_w (trigger DMA operation) %02x\n", machine().describe_context(), data);
	//m_dmatrg_data = data;

	address_space& fullbankspace = m_bank->space(AS_PROGRAM);
	address_space& destspace = m_cpu->space(AS_PROGRAM);

	if (data)
	{
		int src = (m_dmaparams[0]) | (m_dmaparams[1] << 8) | (m_dmaparams[2] << 16);
		uint16_t dest = m_dmaparams[3] | (m_dmaparams[4] << 8);
		uint16_t size = m_dmaparams[5] | (m_dmaparams[6] << 8);

		logerror(" Doing DMA %06x to %04x size %04x\n", src, dest, size);

		for (int i = 0; i < size; i++)
		{
			uint8_t dat = fullbankspace.read_byte(src + i);
			destspace.write_byte(dest + i, dat);
		}
	}
}



