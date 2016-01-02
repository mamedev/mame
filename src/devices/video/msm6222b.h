// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    MSM6222B

    A somewhat hd44780-compatible LCD controller.

    The -01 variant has a fixed cgrom, the other variants are mask-programmed.

***************************************************************************/

#ifndef __MSM6222B_H__
#define __MSM6222B_H__

#define MCFG_MSM6222B_ADD( _tag ) \
	MCFG_DEVICE_ADD( _tag, MSM6222B, 0 )

#define MCFG_MSM6222B_01_ADD( _tag ) \
	MCFG_DEVICE_ADD( _tag, MSM6222B_01, 0 )

class msm6222b_device : public device_t {
public:
	msm6222b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	msm6222b_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	void control_w(UINT8 data);
	UINT8 control_r();
	void data_w(UINT8 data);

	// Character n bits are at bytes n*16..n*16+7 when 8-high, +10 when 11-high.  Only the low 5 bits are used.
	// In one line mode n = 0..79.  In two line mode first line is 0..39 and second is 40..79.
	const UINT8 *render();

protected:
	virtual void device_start() override;

private:
	UINT8 cgram[8*8];
	UINT8 ddram[80];
	UINT8 render_buf[80*16];
	bool cursor_direction, cursor_blinking, two_line, shift_on_write, double_height, cursor_on, display_on;
	UINT8 adc, shift;
	const UINT8 *cgrom;

	void cursor_step(bool direction);
	void shift_step(bool direction);
	bool blink_on() const;
};

class msm6222b_01_device : public msm6222b_device {
public:
	msm6222b_01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual const rom_entry *device_rom_region() const override;
};

extern const device_type MSM6222B;
extern const device_type MSM6222B_01;

#endif
