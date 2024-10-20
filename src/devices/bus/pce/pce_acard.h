// license:BSD-3-Clause
// copyright-holders:Fabio Priuli, Angelo Salese
#ifndef MAME_BUS_PCE_PCE_ACARD_H
#define MAME_BUS_PCE_PCE_ACARD_H

#pragma once

#include "pce_scdsys.h"
#include "pce_slot.h"


// ======================> pce_acard_duo_device

class pce_acard_duo_device : public device_t,
						public device_pce_cart_interface
{
public:
	// construction/destruction
	pce_acard_duo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual void install_memory_handlers(address_space &space) override;

protected:
	// construction/destruction
	pce_acard_duo_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);
	uint8_t peripheral_r(offs_t offset);
	void peripheral_w(offs_t offset, uint8_t data);

	/* Arcade Card specific */
	memory_share_creator<uint8_t> m_ram;

	struct dram_port
	{
		uint32_t ram_addr()
		{
			if (BIT(m_ctrl, 1))
				return (m_base_addr + m_addr_offset + (BIT(m_ctrl, 3) ? 0xff0000 : 0)) & 0x1fffff;
			else
				return m_base_addr & 0x1fffff;
		}

		void addr_increment()
		{
			if (BIT(m_ctrl, 0))
			{
				if (BIT(m_ctrl, 4))
				{
					m_base_addr += m_addr_inc;
					m_base_addr &= 0xffffff;
				}
				else
				{
					m_addr_offset += m_addr_inc;
				}
			}
		}

		void adjust_addr()
		{
			m_base_addr += m_addr_offset + (BIT(m_ctrl, 3) ? 0xff0000 : 0);
			m_base_addr &= 0xffffff;
		}

		uint8_t  m_ctrl;
		uint32_t m_base_addr;
		uint16_t m_addr_offset;
		uint16_t m_addr_inc;
	};

	dram_port m_port[4];
	uint32_t  m_shift;
	uint8_t   m_shift_reg;
	uint8_t   m_rotate_reg;
};


// ======================> pce_acard_duo_device

class pce_acard_pro_device : public pce_acard_duo_device

{
public:
	// construction/destruction
	pce_acard_pro_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// reading and writing
	virtual void install_memory_handlers(address_space &space) override;

protected:
	virtual void device_start() override ATTR_COLD;

private:
	pce_scdsys_shared m_scdsys;

	// reading
	uint8_t register_r(offs_t offset) { return m_scdsys.register_r(offset); }
};


// device type definition
DECLARE_DEVICE_TYPE(PCE_ROM_ACARD_DUO, pce_acard_duo_device)
DECLARE_DEVICE_TYPE(PCE_ROM_ACARD_PRO, pce_acard_pro_device)


#endif // MAME_BUS_PCE_PCE_ACARD_H
