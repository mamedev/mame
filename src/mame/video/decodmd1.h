// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * Data East Pinball DMD Type 1 display
 */

#ifndef DECODMD1_H_
#define DECODMD1_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"

#define MCFG_DECODMD_TYPE1_ADD(_tag, _region) \
	MCFG_DEVICE_ADD(_tag, DECODMD1, 0) \
	decodmd_type1_device::static_set_gfxregion(*device, _region);

#define B_CLR 0x01
#define B_SET 0x02
#define B_CLK 0x04

class decodmd_type1_device : public device_t
{
public:
	decodmd_type1_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	required_device<cpu_device> m_cpu;
	required_memory_bank m_rombank1;
	required_memory_bank m_rombank2;
	required_device<ram_device> m_ram;
	memory_region* m_rom;

	UINT32 screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect );

	DECLARE_READ8_MEMBER(latch_r);
	DECLARE_WRITE8_MEMBER(data_w);
	DECLARE_READ8_MEMBER(busy_r);
	DECLARE_WRITE8_MEMBER(ctrl_w);
	DECLARE_READ8_MEMBER(ctrl_r);
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_WRITE8_MEMBER(status_w);
	DECLARE_READ8_MEMBER(dmd_port_r);
	DECLARE_WRITE8_MEMBER(dmd_port_w);
	TIMER_DEVICE_CALLBACK_MEMBER(dmd_nmi);

	static void static_set_gfxregion(device_t &device, std::string tag);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	UINT8 m_latch;
	UINT8 m_status;
	UINT8 m_ctrl;
	UINT8 m_busy;
	UINT8 m_command;
	UINT8 m_bank;
	UINT8 m_rowclock;
	UINT8 m_rowdata;
	UINT32 m_rowselect;
	UINT8 m_blank;
	UINT32 m_pxdata1;
	UINT32 m_pxdata2;
	UINT32 m_pxdata1_latched;
	UINT32 m_pxdata2_latched;
	bool m_frameswap;
	UINT32 m_pixels[0x200];
	UINT8 m_busy_lines;
	UINT32 m_prevrow;
	std::string m_gfxtag;

	void output_data();
	void set_busy(UINT8 input, UINT8 val);
};

extern const device_type DECODMD1;


#endif /* DECODMD1_H_ */
