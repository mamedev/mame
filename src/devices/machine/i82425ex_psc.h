// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_MACHINE_I82425EX_PSC_H
#define MAME_MACHINE_I82425EX_PSC_H

#pragma once

#include "pci.h"
#include "i82426ex_ib.h"

#include "bus/ata/ataintf.h"
#include "machine/idectrl.h"

class i82425ex_psc_device : public pci_host_device
{
public:
	template <typename T, typename U>
	i82425ex_psc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, U &&ib_tag, int ram_size)
		: i82425ex_psc_device(mconfig, tag, owner, clock)
	{
		// blanked class code
		set_ids(0x80860486, 0x00, 0x000000, 0x00);

		set_cpu_tag(std::forward<T>(cpu_tag));
		set_ib_tag(std::forward<U>(ib_tag));
		set_ram_size(ram_size);
	}
	i82425ex_psc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_cpu_tag(T &&tag) { m_host_cpu.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_ib_tag(T &&tag)  { m_ib.set_tag(std::forward<T>(tag)); }
	void set_ram_size(int ram_size) { m_ram_size = ram_size; }
	auto ide1_irq_w() { return m_ide1_irq.bind(); }
	auto ide2_irq_w() { return m_ide2_irq.bind(); }

//	void smi_act_w(int state);

protected:
	i82425ex_psc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void reset_all_mappings() override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override ATTR_COLD;

	virtual void config_address_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0) override;

private:
	required_device<cpu_device> m_host_cpu;
	required_device<i82426ex_ib_device> m_ib;
	required_device_array<ide_controller_32_device, 2> m_ide;
	devcb_write_line m_ide1_irq;
	devcb_write_line m_ide2_irq;
	std::vector<uint32_t> m_ram;

	u32 m_ram_size = 0;

	u8 m_pcicon;
	u8 m_xbcsa;
	u8 m_hostdev;
	u16 m_lbide;
	u8 m_iort;
	bool m_prev;
	u8 m_hostsel;
	u8 m_dfc;
	u16 m_scc;
	u16 m_dramc;
	u8 m_pam[7];
	u8 m_drb[5];
	u8 m_pirqrc[2];
	u8 m_dmh;
	u8 m_tom;
	u8 m_smramcon;
	u8 m_smicntl;
	u16 m_smien;
	u32 m_see;
	u8 m_ftmr;
	u16 m_smireq;
	u8 m_ctltmrl;
	u8 m_ctltmrh;

	void map_bios(address_space *memory_space, uint32_t start, uint32_t end);
	void map_shadowram(address_space *memory_space, offs_t start_offs, offs_t end_offs, u8 setting);
};

DECLARE_DEVICE_TYPE(I82425EX_PSC, i82425ex_psc_device)


#endif // MAME_MACHINE_I82425EX_PSC_H
