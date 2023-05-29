// license:BSD-3-Clause
// copyright-holders:

#ifndef MAME_BUS_SNES_PROFIGHTER_H
#define MAME_BUS_SNES_PROFIGHTER_H

#pragma once

#include "snes_slot.h"


class sns_pro_fighter_q_device : public device_t,
						public device_sns_cart_interface
{
public:
	sns_pro_fighter_q_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

class sns_pro_fighter_qa_device : public device_t,
						public device_sns_cart_interface
{
public:
	sns_pro_fighter_qa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

class sns_pro_fighter_qb_device : public device_t,
						public device_sns_cart_interface
{
public:
	sns_pro_fighter_qb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};

class sns_pro_fighter_x_device : public device_t,
						public device_sns_cart_interface
{
public:
	sns_pro_fighter_x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
};


DECLARE_DEVICE_TYPE(SNS_PRO_FIGHTER_Q, sns_pro_fighter_q_device);
DECLARE_DEVICE_TYPE(SNS_PRO_FIGHTER_QA, sns_pro_fighter_qa_device);
DECLARE_DEVICE_TYPE(SNS_PRO_FIGHTER_QB, sns_pro_fighter_qb_device);
DECLARE_DEVICE_TYPE(SNS_PRO_FIGHTER_X, sns_pro_fighter_x_device);

#endif // MAME_BUS_SNES_PROFIGHTER_H
