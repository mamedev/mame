// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model 16B/6000 hard disk controller emulation

    Internal hard disk controller 8000202 (Model 16B/6000, AX-9432)

*********************************************************************/

#ifndef MAME_BUS_TRS80M2_HDC_H
#define MAME_BUS_TRS80M2_HDC_H

#pragma once

#include "slot.h"

#include "imagedev/harddriv.h"
#include "machine/wd1010.h"
#include "machine/z80ctc.h"


class trs80m2_wd1010_card_device_base : public device_t, public device_trs80m2_card_interface
{
public:
	// device_trs80m2_card_interface implementation
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

protected:
	enum : uint8_t
	{
		CONTROL_DMA    = 0x01,
		CONTROL_INT    = 0x02,
		CONTROL_DEVEN  = 0x08,
		CONTROL_SFTRST = 0x10
	};

	trs80m2_wd1010_card_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void map(address_map &map) ATTR_COLD;
	virtual void update_drdy() = 0;
	virtual void intrq_w(int state) = 0;

	uint8_t control_r();
	void control_w(uint8_t data);

	int selected_drive() const { return BIT(m_sdh, 3, 2); }
	void update_int();
	void update_xferrq();

	required_device<wd1010_device> m_hdc;
	required_device<z80ctc_device> m_ctc;
	required_ioport m_config;

	uint8_t m_control;
	bool m_bdrq;

private:
	static constexpr uint16_t BUFFER_SIZE = 0x400;

	uint8_t data_r();
	void data_w(uint8_t data);
	uint8_t hdc_r(offs_t offset);
	void hdc_w(offs_t offset, uint8_t data);

	uint8_t buf_in();
	void buf_out(uint8_t data);
	void bcs_w(int state);
	void bdrq_w(int state);
	void bcr_w(int state);

	void ctc_int_w(int state);
	uint16_t sector_size() const;

	uint8_t m_buf[BUFFER_SIZE];
	uint16_t m_ptr;
	uint8_t m_sdh;
	bool m_bcs;
	bool m_ctc_int;
	bool m_installed;
};


class trs80m2_hdc_device : public trs80m2_wd1010_card_device_base
{
public:
	trs80m2_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void map(address_map &map) override ATTR_COLD;
	virtual void update_drdy() override;
	virtual void intrq_w(int state) override;

private:
	required_device_array<harddisk_image_device, 2> m_hdd;
};


DECLARE_DEVICE_TYPE(TRS80M2_HDC, trs80m2_hdc_device)

#endif // MAME_BUS_TRS80M2_HDC_H
