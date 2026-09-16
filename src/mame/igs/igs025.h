// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi

#ifndef MAME_IGS_IGS025_H
#define MAME_IGS_IGS025_H

#pragma once

class igs025_device : public device_t
{
public:
	igs025_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// configurations
	// used to connect the 022
	using igs025_execute_external = device_delegate<void ()>;
	template <typename... T> void set_external_cb(T &&... args) { m_execute_external.set(std::forward<T>(args)...); }

	using igs025_source_read_delegate = device_delegate<uint8_t (uint32_t region, uint8_t addr)>;
	template <typename... T> void set_source_cb(T &&... args) { m_source_cb.set(std::forward<T>(args)...); }

	void set_game_id(uint32_t id) { m_game_id = id; }
	void set_region(uint32_t region) { m_region = region; }

	uint16_t prot_r(offs_t offset);

	void olds_w(offs_t offset, uint16_t data);
	void drgw2_prot_w(offs_t offset, uint16_t data);
	void killbld_igs025_prot_w(offs_t offset, uint16_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void protection_calculate_hilo();
	void protection_calculate_hold(int y, int z);

	void no_callback_setup();

	igs025_execute_external m_execute_external;
	igs025_source_read_delegate m_source_cb;

	uint32_t m_game_id;
	uint32_t m_region;

	uint16_t m_prot_hold;
	uint16_t m_prot_hilo;
	uint16_t m_prot_hilo_select;

	int32_t m_prot_cmd;
	int32_t m_prot_reg;
	int32_t m_prot_ptr;
	uint8_t m_prot_swap;

	uint16_t m_prot_bs;
	uint16_t m_prot_cmd3;
};


DECLARE_DEVICE_TYPE(IGS025, igs025_device)

#endif // MAME_IGS_IGS025_H
