// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    SED1200

    A LCD controller.

    The D/F variants are a packaging difference (QFP80 vs. bare chip).
    The A/B variants are an internal CGROM difference (jis
    vs. european characters)

***************************************************************************/

#ifndef __SED1200_H__
#define __SED1200_H__

#define MCFG_SED1200D0A_ADD( _tag ) \
	MCFG_DEVICE_ADD( _tag, SED1200D0A, 0 )

#define MCFG_SED1200F0A_ADD( _tag ) \
	MCFG_DEVICE_ADD( _tag, SED1200F0A, 0 )

#define MCFG_SED1200D0B_ADD( _tag ) \
	MCFG_DEVICE_ADD( _tag, SED1200D0B, 0 )

#define MCFG_SED1200F0B_ADD( _tag ) \
	MCFG_DEVICE_ADD( _tag, SED1200F0B, 0 )

class sed1200_device : public device_t {
public:
	sed1200_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	void control_w(UINT8 data);
	UINT8 control_r();
	void data_w(UINT8 data);

	const UINT8 *render();

protected:
	virtual void device_start();

private:
	UINT8 cgram[4*8];
	UINT8 ddram[10*2];
	UINT8 render_buf[20*8];
	bool cursor_direction, cursor_blinking, cursor_full, cursor_on, display_on;
	UINT8 cursor_address, cgram_address;
	const UINT8 *cgrom;

	void soft_reset();
	void cursor_step();
};

class sed1200d0a_device : public sed1200_device {
public:
	sed1200d0a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual const rom_entry *device_rom_region() const;
};

class sed1200f0a_device : public sed1200_device {
public:
	sed1200f0a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual const rom_entry *device_rom_region() const;
};

class sed1200d0b_device : public sed1200_device {
public:
	sed1200d0b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual const rom_entry *device_rom_region() const;
};

class sed1200f0b_device : public sed1200_device {
public:
	sed1200f0b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual const rom_entry *device_rom_region() const;
};

extern const device_type SED1200D0A;
extern const device_type SED1200F0A;
extern const device_type SED1200D0B;
extern const device_type SED1200F0B;

#endif
