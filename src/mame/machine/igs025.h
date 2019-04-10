// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
/* Common device stuff for IGS025 / IGS022, should be split into devices for each chip once we know where what part does what */
#ifndef MAME_MACHINE_IGS025_H
#define MAME_MACHINE_IGS025_H

#pragma once

// used to connect the 022
typedef device_delegate<void (void)> igs025_execute_external;

class igs025_device : public device_t
{
public:
	igs025_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ16_MEMBER( killbld_igs025_prot_r );
	// use setters instead of making public?
	const uint8_t (*m_kb_source_data)[0xec];
	uint32_t m_kb_game_id;
	uint32_t m_kb_region;

	template <typename... T> void set_external_cb(T &&... args) { m_execute_external = igs025_execute_external(std::forward<T>(args)...); }

	DECLARE_WRITE16_MEMBER( olds_w );
	DECLARE_WRITE16_MEMBER( drgw2_d80000_protection_w );
	DECLARE_WRITE16_MEMBER( killbld_igs025_prot_w);


protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	igs025_execute_external m_execute_external;

	uint16_t        m_kb_prot_hold;
	uint16_t        m_kb_prot_hilo;
	uint16_t        m_kb_prot_hilo_select;

	int           m_kb_cmd;
	int           m_kb_reg;
	int           m_kb_ptr;
	uint8_t         m_kb_swap;

	void killbld_protection_calculate_hilo();
	void killbld_protection_calculate_hold(int y, int z);

	void no_callback_setup(void);


	uint16_t        m_olds_bs;
	uint16_t        m_kb_cmd3;

};


DECLARE_DEVICE_TYPE(IGS025, igs025_device)

#endif // MAME_MACHINE_IGS025_H
