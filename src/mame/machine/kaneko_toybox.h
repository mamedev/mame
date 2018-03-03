// license:BSD-3-Clause
// copyright-holders:David Haywood, Luca Elia, Sebastien Volpe
/* Kaneko Toybox */
#ifndef MAME_MACHINE_KANEKO_TOYBOX_H
#define MAME_MACHINE_KANEKO_TOYBOX_H

#pragma once

class kaneko_toybox_device : public device_t
{
public:
	static constexpr int GAME_NORMAL = 0;
	static constexpr int GAME_BONK = 1;

	static constexpr int TABLE_NORMAL = 0;
	static constexpr int TABLE_ALT = 1;

	kaneko_toybox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_table(int tabletype) { m_tabletype = tabletype; }
	void set_game_type(int gametype) { m_gametype = gametype; }

	DECLARE_WRITE16_MEMBER(mcu_com0_w);
	DECLARE_WRITE16_MEMBER(mcu_com1_w);
	DECLARE_WRITE16_MEMBER(mcu_com2_w);
	DECLARE_WRITE16_MEMBER(mcu_com3_w);
	DECLARE_READ16_MEMBER(mcu_status_r);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_shared_ptr<uint16_t> m_mcuram;
	uint16_t m_mcu_com[4];
	int m_gametype;
	int m_tabletype;

	void mcu_com_w(offs_t offset, uint16_t data, uint16_t mem_mask, int _n_);
	void decrypt_rom();
	void handle_04_subcommand(uint8_t mcu_subcmd, uint16_t *mcu_ram);
	void mcu_init();
	void mcu_run();
};


DECLARE_DEVICE_TYPE(KANEKO_TOYBOX, kaneko_toybox_device)

#define MCFG_TOYBOX_TABLE_TYPE(_type) \
	downcast<kaneko_toybox_device &>(*device).set_table(kaneko_toybox_device::TABLE_##_type);

#define MCFG_TOYBOX_GAME_TYPE(_type) \
	downcast<kaneko_toybox_device &>(*device).set_game_type(kaneko_toybox_device::GAME_##_type);

#endif // MAME_MACHINE_KANEKO_TOYBOX_H
