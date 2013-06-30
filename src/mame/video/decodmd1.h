/*
 * Data East Pinball DMD Type 1 display
 */

#ifndef DECODMD1_H_
#define DECODMD1_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"

#define MCFG_DECODMD_TYPE1_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, DECODMD1, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

struct decodmd_type1_intf
{
	const char* m_romregion;  // region containing display ROM
};

class decodmd_type1_device : public device_t,
							 public decodmd_type1_intf
{
public:
	decodmd_type1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	required_device<cpu_device> m_cpu;
	required_memory_bank m_rombank1;
	required_memory_bank m_rombank2;
	required_device<ram_device> m_ram;
	memory_region* m_rom;

	UINT32 screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect );

	DECLARE_WRITE8_MEMBER(bank_w);
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

protected:
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

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
	UINT32 m_pixels[0x100];
	bool m_busy_clr;
	bool m_busy_set;
	bool m_busy_clk;

	void output_data();
	void check_busy();
};

extern const device_type DECODMD1;


#endif /* DECODMD1_H_ */
