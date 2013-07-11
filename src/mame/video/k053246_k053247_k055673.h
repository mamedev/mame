/* */

#pragma once
#ifndef __K05324x_H__
#define __K05324x_H__

#define NORMAL_PLANE_ORDER 0x0123
#define TASMAN_PLANE_ORDER 0x1616

typedef void (*k05324x_callback)(running_machine &machine, int *code, int *color, int *priority);


/**  Konami 053246 / 053247 / 055673  **/
#define K055673_LAYOUT_GX  0
#define K055673_LAYOUT_RNG 1
#define K055673_LAYOUT_LE2 2
#define K055673_LAYOUT_GX6 3

DECLARE_READ16_DEVICE_HANDLER( k055673_rom_word_r );
DECLARE_READ16_DEVICE_HANDLER( k055673_GX6bpp_rom_word_r );

/*
Callback procedures for non-standard shadows:

1) translate shadow code to the correct 2-bit form (0=off, 1-3=style)
2) shift shadow code left by K053247_SHDSHIFT and add the K053247_CUSTOMSHADOW flag
3) combine the result with sprite color
*/
#define K053247_CUSTOMSHADOW    0x20000000
#define K053247_SHDSHIFT        20

DECLARE_READ8_DEVICE_HANDLER( k053247_r );
DECLARE_WRITE8_DEVICE_HANDLER( k053247_w );
DECLARE_READ16_DEVICE_HANDLER( k053247_word_r );
DECLARE_WRITE16_DEVICE_HANDLER( k053247_word_w );
DECLARE_READ32_DEVICE_HANDLER( k053247_long_r );
DECLARE_WRITE32_DEVICE_HANDLER( k053247_long_w );
DECLARE_WRITE16_DEVICE_HANDLER( k053247_reg_word_w ); // "OBJSET2" registers
DECLARE_WRITE32_DEVICE_HANDLER( k053247_reg_long_w );

void k053247_sprites_draw(device_t *device, bitmap_ind16 &bitmap,const rectangle &cliprect);
void k053247_sprites_draw(device_t *device, bitmap_rgb32 &bitmap,const rectangle &cliprect);
int k053247_read_register(device_t *device, int regnum);
void k053247_set_sprite_offs(device_t *device, int offsx, int offsy);
void k053247_wraparound_enable(device_t *device, int status);
void k053247_set_z_rejection(device_t *device, int zcode); // common to k053246/7
void k053247_get_ram(device_t *device, UINT16 **ram);
int k053247_get_dx(device_t *device);
int k053247_get_dy(device_t *device);

DECLARE_READ8_DEVICE_HANDLER( k053246_r );
DECLARE_WRITE8_DEVICE_HANDLER( k053246_w );
DECLARE_READ16_DEVICE_HANDLER( k053246_word_r );
DECLARE_WRITE16_DEVICE_HANDLER( k053246_word_w );
DECLARE_READ32_DEVICE_HANDLER( k053246_long_r );
DECLARE_WRITE32_DEVICE_HANDLER( k053246_long_w );

void k053246_set_objcha_line(device_t *device, int state);
int k053246_is_irq_enabled(device_t *device);
int k053246_read_register(device_t *device, int regnum);




struct k053247_interface
{
	const char         *screen;
	const char         *gfx_memory_region;
	int                gfx_num;
	int                plane_order;
	int                dx, dy;
	int                deinterleave;
	k05324x_callback   callback;
};

class k053247_device : public device_t
{
public:
	k053247_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k053247_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type K053246;

#define K053247 K053246
class k055673_device : public device_t
{
public:
	k055673_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k055673_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type K055673;


DECLARE_READ16_DEVICE_HANDLER( k053246_reg_word_r );    // OBJSET1
DECLARE_READ16_DEVICE_HANDLER( k053247_reg_word_r );    // OBJSET2
DECLARE_READ32_DEVICE_HANDLER( k053247_reg_long_r );    // OBJSET2

/* old non-device stuff */

void K055673_vh_start(running_machine &machine, const char *gfx_memory_region, int alt_layout, int dx, int dy,
		void (*callback)(running_machine &machine, int *code,int *color,int *priority));
DECLARE_READ16_HANDLER( K055673_rom_word_r );
DECLARE_READ16_HANDLER( K055673_GX6bpp_rom_word_r );

/*
Callback procedures for non-standard shadows:

1) translate shadow code to the correct 2-bit form (0=off, 1-3=style)
2) shift shadow code left by K053247_SHDSHIFT and add the K053247_CUSTOMSHADOW flag
3) combine the result with sprite color
*/
#define K053247_CUSTOMSHADOW    0x20000000
#define K053247_SHDSHIFT        20

DECLARE_READ16_HANDLER( K053247_word_r );
DECLARE_WRITE16_HANDLER( K053247_word_w );
DECLARE_READ32_HANDLER( K053247_long_r );
DECLARE_WRITE32_HANDLER( K053247_long_w );
DECLARE_WRITE16_HANDLER( K053247_reg_word_w ); // "OBJSET2" registers
DECLARE_WRITE32_HANDLER( K053247_reg_long_w );

int K053247_read_register(int regnum);
void K053247_set_SpriteOffset(int offsx, int offsy);
void K053247_export_config(UINT16 **ram, gfx_element **gfx, void (**callback)(running_machine &, int *, int *, int *), int *dx, int *dy);

DECLARE_WRITE16_HANDLER( K053246_word_w );
DECLARE_WRITE32_HANDLER( K053246_long_w );
void K053246_set_OBJCHA_line(int state);
int K053246_is_IRQ_enabled(void);
int K053246_read_register(int regnum);




#define MCFG_K053246_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K053246, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K053247_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K053247, 0) \
	MCFG_DEVICE_CONFIG(_interface)
	
#define MCFG_K055673_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K055673, 0) \
	MCFG_DEVICE_CONFIG(_interface)


#endif
