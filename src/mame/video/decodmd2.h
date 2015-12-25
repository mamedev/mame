// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *  Data East Pinball DMD Type 2 Display
 */

#ifndef DECODMD_H_
#define DECODMD_H_

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "video/mc6845.h"
#include "machine/ram.h"

#define MCFG_DECODMD_TYPE2_ADD(_tag, _region) \
	MCFG_DEVICE_ADD(_tag, DECODMD2, 0) \
	decodmd_type2_device::static_set_gfxregion(*device, _region);

#define START_ADDRESS       (((m_crtc_reg[0x0c]<<8) & 0x3f00) | (m_crtc_reg[0x0d] & 0xff))

class decodmd_type2_device : public device_t
{
public:
	decodmd_type2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	required_device<cpu_device> m_cpu;
	required_device<mc6845_device> m_mc6845;
	required_memory_bank m_rombank1;
	required_memory_bank m_rombank2;
	required_memory_bank m_rambank;
	required_device<ram_device> m_ram;
	memory_region* m_rom;

	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_WRITE8_MEMBER(crtc_address_w);
	DECLARE_WRITE8_MEMBER(crtc_register_w);
	DECLARE_READ8_MEMBER(crtc_status_r);
	DECLARE_READ8_MEMBER(latch_r);
	DECLARE_WRITE8_MEMBER(data_w);
	DECLARE_READ8_MEMBER(busy_r);
	DECLARE_WRITE8_MEMBER(ctrl_w);
	DECLARE_READ8_MEMBER(ctrl_r);
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_WRITE8_MEMBER(status_w);
	TIMER_DEVICE_CALLBACK_MEMBER(dmd_firq);
	MC6845_UPDATE_ROW(crtc_update_row);

	static void static_set_gfxregion(device_t &device, const char *tag);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	UINT8 m_crtc_index;
	UINT8 m_crtc_reg[0x100];
	UINT8 m_latch;
	UINT8 m_status;
	UINT8 m_ctrl;
	UINT8 m_busy;
	UINT8 m_command;
	const char* m_gfxtag;
};

extern const device_type DECODMD2;

#endif /* DECODMD_H_ */
