// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_MISC_VAMPHALF_PROT_H
#define MAME_MISC_VAMPHALF_PROT_H

#pragma once

class misncrft_fpga_prot_device : public device_t
{
public:
	misncrft_fpga_prot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

	u16 data_r(offs_t offset);
	template <int seed_size> void seed_w(offs_t offset, u16 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 prot_value_check(std::vector<std::vector<u8>> &prot_table, int seed_size);

	u8 m_seed[16];
	u8 m_retval;
	int m_idx;
	bool m_is_armed;
};

class wyvernwg_fpga_prot_device : public device_t
{
public:
	wyvernwg_fpga_prot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

	u16 data_r(offs_t offset);
	void seed_w(offs_t offset, u16 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u8 prot_value_check(std::vector<std::vector<u16>> &prot_table, int seed_size);

	u16 m_seed[16];
	u8 m_retval;
	int m_idx;
	bool m_is_armed;
};

class worldadv_fpga_prot_device : public device_t
{
public:
	worldadv_fpga_prot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock = 0);

	u16 data_r(offs_t offset);
	void seed_w(offs_t offset, u16 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	int m_read_idx;
	int m_write_idx;
	u64 m_seed;
	u8 m_retval;
	bool m_is_armed;
};


DECLARE_DEVICE_TYPE(MISNCRFT_FPGA_PROT, misncrft_fpga_prot_device)
DECLARE_DEVICE_TYPE(WYVERNWG_FPGA_PROT, wyvernwg_fpga_prot_device)
DECLARE_DEVICE_TYPE(WORLDADV_FPGA_PROT, worldadv_fpga_prot_device)


#endif // MAME_MISC_VAMPHALF_PROT_H
