// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model II/12/16/6000 hard disk controller emulation

    WD1010 (u26) with a WD1100-11 (u27) managed sector buffer (u28),
    a Z80 CTC for interrupts on the host daisy chain, and two ST506
    drive ports.  Decoded at ports C0-CF.

*********************************************************************/

#ifndef MAME_BUS_TRS80M2_HDC_H
#define MAME_BUS_TRS80M2_HDC_H

#pragma once

#include "slot.h"

#include "imagedev/harddriv.h"
#include "machine/wd1010.h"
#include "machine/z80ctc.h"


class trs80m2_hdc_device : public device_t, public device_trs80m2_card_interface
{
public:
	trs80m2_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_trs80m2_card_interface implementation
	virtual int z80daisy_irq_state() override;
	virtual int z80daisy_irq_ack() override;
	virtual void z80daisy_irq_reti() override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	static constexpr uint16_t BUFFER_SIZE = 0x200;

	void map(address_map &map) ATTR_COLD;

	uint8_t wp_r();
	uint8_t control_r();
	void control_w(uint8_t data);
	uint8_t unused_r();
	uint8_t data_r();
	void data_w(uint8_t data);
	uint8_t hdc_r(offs_t offset);
	void hdc_w(offs_t offset, uint8_t data);

	uint8_t buf_in();
	void buf_out(uint8_t data);
	void intrq_w(int state);
	void bdrq_w(int state);
	void bcr_w(int state);

	void update_drdy();

	required_device<wd1010_device> m_hdc;
	required_device<z80ctc_device> m_ctc;
	required_device_array<harddisk_image_device, 2> m_hdd;

	uint8_t m_buf[BUFFER_SIZE];
	uint16_t m_ptr;
	uint8_t m_control;
	uint8_t m_sdh;
	bool m_intrq;
	bool m_bdrq;
};


DECLARE_DEVICE_TYPE(TRS80M2_HDC, trs80m2_hdc_device)

#endif // MAME_BUS_TRS80M2_HDC_H
