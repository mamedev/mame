// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi

#ifndef MAME_IGS_IGS025_H
#define MAME_IGS_IGS025_H

#pragma once

// used to connect the 022
typedef device_delegate<void (void)> igs025_execute_external;

class igs025_device : public device_t
{
public:
	igs025_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t killbld_igs025_prot_r(offs_t offset);
	// use setters instead of making public?
	const uint8_t (*m_kb_source_data)[0xec]{};
	uint32_t m_kb_game_id = 0;
	uint32_t m_kb_region = 0;

	template <typename... T> void set_external_cb(T &&... args) { m_execute_external.set(std::forward<T>(args)...); }

	void olds_w(offs_t offset, uint16_t data);
	void drgw2_d80000_protection_w(offs_t offset, uint16_t data);
	void killbld_igs025_prot_w(offs_t offset, uint16_t data);


protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	igs025_execute_external m_execute_external;

	uint16_t        m_kb_prot_hold = 0;
	uint16_t        m_kb_prot_hilo = 0;
	uint16_t        m_kb_prot_hilo_select = 0;

	int           m_kb_cmd = 0;
	int           m_kb_reg = 0;
	int           m_kb_ptr = 0;
	uint8_t         m_kb_swap = 0;

	void killbld_protection_calculate_hilo();
	void killbld_protection_calculate_hold(int y, int z);

	void no_callback_setup(void);


	uint16_t        m_olds_bs = 0;
	uint16_t        m_kb_cmd3 = 0;

};


DECLARE_DEVICE_TYPE(IGS025, igs025_device)

#endif // MAME_IGS_IGS025_H
