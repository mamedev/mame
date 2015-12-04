// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    voodoo.h

    3dfx Voodoo Graphics SST-1/2 emulator.

***************************************************************************/

#ifndef __VOODOO_H__
#define __VOODOO_H__

#pragma once



/***************************************************************************
    CONSTANTS
***************************************************************************/
/* enumeration specifying which model of Voodoo we are emulating */
enum
{
	TYPE_VOODOO_1,
	TYPE_VOODOO_2,
	TYPE_VOODOO_BANSHEE,
	TYPE_VOODOO_3
};

#define STD_VOODOO_1_CLOCK          50000000
#define STD_VOODOO_2_CLOCK          90000000
#define STD_VOODOO_BANSHEE_CLOCK    90000000
#define STD_VOODOO_3_CLOCK          132000000



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_VOODOO_FBMEM(_value) \
	voodoo_device::static_set_fbmem(*device, _value);

#define MCFG_VOODOO_TMUMEM(_value1, _value2) \
	voodoo_device::static_set_tmumem(*device, _value1, _value2);

#define MCFG_VOODOO_SCREEN_TAG(_tag) \
	voodoo_device::static_set_screen_tag(*device, _tag);

#define MCFG_VOODOO_CPU_TAG(_tag) \
	voodoo_device::static_set_cpu_tag(*device, _tag);

#define MCFG_VOODOO_VBLANK_CB(_devcb) \
	devcb = &voodoo_device::static_set_vblank_callback(*device, DEVCB_##_devcb);

#define MCFG_VOODOO_STALL_CB(_devcb) \
	devcb = &voodoo_device::static_set_stall_callback(*device, DEVCB_##_devcb);


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

int voodoo_update(device_t *device, bitmap_rgb32 &bitmap, const rectangle &cliprect);
int voodoo_get_type(device_t *device);
int voodoo_is_stalled(device_t *device);
void voodoo_set_init_enable(device_t *device, UINT32 newval);

/* ----- device interface ----- */

class voodoo_device : public device_t
{
public:
	voodoo_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	~voodoo_device();


	static void static_set_fbmem(device_t &device, int value) { downcast<voodoo_device &>(device).m_fbmem = value; }
	static void static_set_tmumem(device_t &device, int value1, int value2) { downcast<voodoo_device &>(device).m_tmumem0 = value1; downcast<voodoo_device &>(device).m_tmumem1 = value2; }
	static void static_set_screen_tag(device_t &device, const char *tag) { downcast<voodoo_device &>(device).m_screen = tag; }
	static void static_set_cpu_tag(device_t &device, const char *tag) { downcast<voodoo_device &>(device).m_cputag = tag; }
	template<class _Object> static devcb_base &static_set_vblank_callback(device_t &device, _Object object) { return downcast<voodoo_device &>(device).m_vblank.set_callback(object); }
	template<class _Object> static devcb_base &static_set_stall_callback(device_t &device, _Object object)  { return downcast<voodoo_device &>(device).m_stall.set_callback(object); }

	DECLARE_READ32_MEMBER( voodoo_r );
	DECLARE_WRITE32_MEMBER( voodoo_w );

	// access to legacy token
	struct voodoo_state *token() const { assert(m_token != nullptr); return m_token; }
	void common_start_voodoo(UINT8 type);

	UINT8               m_fbmem;
	UINT8               m_tmumem0;
	UINT8               m_tmumem1;
	const char *        m_screen;
	const char *        m_cputag;
	devcb_write_line   m_vblank;
	devcb_write_line   m_stall;

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_stop();
	virtual void device_reset();
private:
	// internal state
	struct voodoo_state *m_token;
};

class voodoo_1_device : public voodoo_device
{
public:
	voodoo_1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type VOODOO_1;

class voodoo_2_device : public voodoo_device
{
public:
	voodoo_2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type VOODOO_2;

class voodoo_banshee_device : public voodoo_device
{
public:
	voodoo_banshee_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	voodoo_banshee_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	DECLARE_READ32_MEMBER( banshee_r );
	DECLARE_WRITE32_MEMBER( banshee_w );
	DECLARE_READ32_MEMBER( banshee_fb_r );
	DECLARE_WRITE32_MEMBER( banshee_fb_w );
	DECLARE_READ32_MEMBER( banshee_io_r );
	DECLARE_WRITE32_MEMBER( banshee_io_w );
	DECLARE_READ32_MEMBER( banshee_rom_r );

protected:
	// device-level overrides
	virtual void device_start();
	DECLARE_READ32_MEMBER( banshee_agp_r );
	DECLARE_WRITE32_MEMBER( banshee_agp_w );
	DECLARE_READ8_MEMBER( banshee_vga_r );
	DECLARE_WRITE8_MEMBER( banshee_vga_w );
};

extern const device_type VOODOO_BANSHEE;

class voodoo_3_device : public voodoo_banshee_device
{
public:
	voodoo_3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
protected:
	// device-level overrides
	virtual void device_start();
};

extern const device_type VOODOO_3;

#endif
