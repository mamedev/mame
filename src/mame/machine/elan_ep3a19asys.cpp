// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"
#include "elan_ep3a19asys.h"

// like EU3A05 but with the DMA at a lower address where the code bank register would usually be, and a single byte for bank register rather than 2 - any other changes?

DEFINE_DEVICE_TYPE(ELAN_EP3A19A_SYS, elan_ep3a19asys_device, "elan_ep3a19asys", "Elan EP3A19A System")

elan_ep3a19asys_device::elan_ep3a19asys_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	elan_eu3a05commonsys_device(mconfig, ELAN_EP3A19A_SYS, tag, owner, clock),
	device_memory_interface(mconfig, *this),
	m_space_config("regs", ENDIANNESS_NATIVE, 8, 5, 0, address_map_constructor(FUNC(elan_ep3a19asys_device::map), this))
{
}

device_memory_interface::space_config_vector elan_ep3a19asys_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}


void elan_ep3a19asys_device::rombank_w(offs_t offset, uint8_t data)
{
	m_rombank_lo = data;
	m_bank->set_bank(m_rombank_lo);
}

uint8_t elan_ep3a19asys_device::rombank_r(offs_t offset)
{
	return m_rombank_lo;
}


void elan_ep3a19asys_device::map(address_map& map)
{
//  elan_eu3a05commonsys_device::map(map); // 00 - 0e

	map(0x0c, 0x0c).rw(FUNC(elan_ep3a19asys_device::rombank_r), FUNC(elan_ep3a19asys_device::rombank_w));

	map(0x0d, 0x13).rw(FUNC(elan_ep3a19asys_device::dma_param_r), FUNC(elan_ep3a19asys_device::dma_param_w));
	map(0x14, 0x14).rw(FUNC(elan_ep3a19asys_device::elan_eu3a05_dmatrg_r), FUNC(elan_ep3a19asys_device::elan_eu3a05_dmatrg_w));
}

void elan_ep3a19asys_device::device_start()
{
	elan_eu3a05commonsys_device::device_start();

	save_item(NAME(m_dmaparams));
}

void elan_ep3a19asys_device::device_reset()
{
	elan_eu3a05commonsys_device::device_reset();

	for (int i = 0; i < 7; i++)
		m_dmaparams[i] = 0x00;
}

uint8_t elan_ep3a19asys_device::dma_param_r(offs_t offset)
{
	return m_dmaparams[offset];
}

void elan_ep3a19asys_device::dma_param_w(offs_t offset, uint8_t data)
{
	m_dmaparams[offset] = data;
}


uint8_t elan_ep3a19asys_device::elan_eu3a05_dmatrg_r()
{
	logerror("%s: elan_eu3a05_dmatrg_r (DMA operation state?)\n", machine().describe_context());
	return 0x00;//m_dmatrg_data;
}


void elan_ep3a19asys_device::elan_eu3a05_dmatrg_w(uint8_t data)
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

		logerror(" Doing %02x DMA %06x to %04x size %04x\n", data, src, dest, size);

		for (int i = 0; i < size; i++)
		{
			uint8_t dat = fullbankspace.read_byte(src);
			src++;
			destspace.write_byte(dest, dat);
			dest++;
		}

		m_dmaparams[0] = src & 0xff;
		m_dmaparams[1] = (src >> 8) & 0xff;
		m_dmaparams[2] = (src >> 16) & 0xff;
		m_dmaparams[3] = dest & 0xff;
		m_dmaparams[4] = (dest >> 8) & 0xff;

		m_dmaparams[5] = 0;
		m_dmaparams[6] = 0;
	}
}



