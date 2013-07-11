/*************************************************************************

    konicdev.h

    Implementation of various Konami custom video ICs

**************************************************************************/

#pragma once
#ifndef __KONICDEV_H__
#define __KONICDEV_H__

#include "konami_helper.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct k001006_interface
{
	const char     *m_gfx_region;
};

struct k001005_interface
{
	const char     *screen;
	const char     *cpu;
	const char     *dsp;
	const char     *k001006_1;
	const char     *k001006_2;

	const char     *gfx_memory_region;
	int            gfx_index;
};

struct k001604_interface
{
	int            gfx_index_1;
	int            gfx_index_2;
	int            layer_size;
	int            roz_size;
	int            txt_mem_offset;
	int            roz_mem_offset;
};

class k001006_device : public device_t,
										public k001006_interface
{
public:
	k001006_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k001006_device() {}

	UINT32 get_palette(int index);

	DECLARE_READ32_MEMBER( read );
	DECLARE_WRITE32_MEMBER( write );
	
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	//screen_device *m_screen;

	UINT16 *     m_pal_ram;
	UINT16 *     m_unknown_ram;
	UINT32       m_addr;
	int          m_device_sel;

	UINT32 *     m_palette;
};

extern const device_type K001006;

class k001005_device : public device_t
{
public:
	k001005_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k001005_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type K001005;

class k001604_device : public device_t
{
public:
	k001604_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k001604_device() { global_free(m_token); }

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

	TILEMAP_MAPPER_MEMBER(k001604_scan_layer_8x8_0_size0);
	TILEMAP_MAPPER_MEMBER(k001604_scan_layer_8x8_0_size1);
	TILEMAP_MAPPER_MEMBER(k001604_scan_layer_8x8_1_size0);
	TILEMAP_MAPPER_MEMBER(k001604_scan_layer_8x8_1_size1);
	TILEMAP_MAPPER_MEMBER(k001604_scan_layer_roz_256);
	TILEMAP_MAPPER_MEMBER(k001604_scan_layer_roz_128);
	TILE_GET_INFO_MEMBER(k001604_tile_info_layer_8x8);
	TILE_GET_INFO_MEMBER(k001604_tile_info_layer_roz);
};

extern const device_type K001604;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_K001006_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K001006, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K001005_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K001005, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_K001604_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K001604, 0) \
	MCFG_DEVICE_CONFIG(_interface)


/***************************************************************************
    HELPERS FOR DRIVERS
***************************************************************************/




/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/


/**  Konami 001005  **/
void k001005_draw(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect);
void k001005_swap_buffers(device_t *device);
void k001005_preprocess_texture_data(UINT8 *rom, int length, int gticlub);

DECLARE_READ32_DEVICE_HANDLER( k001005_r );
DECLARE_WRITE32_DEVICE_HANDLER( k001005_w );


/**  Konami 001604  **/
void k001604_draw_back_layer( device_t *device, bitmap_rgb32 &bitmap, const rectangle &cliprect );
void k001604_draw_front_layer( device_t *device, bitmap_rgb32 &bitmap, const rectangle &cliprect );
DECLARE_WRITE32_DEVICE_HANDLER( k001604_tile_w );
DECLARE_READ32_DEVICE_HANDLER( k001604_tile_r );
DECLARE_WRITE32_DEVICE_HANDLER( k001604_char_w );
DECLARE_READ32_DEVICE_HANDLER( k001604_char_r );
DECLARE_WRITE32_DEVICE_HANDLER( k001604_reg_w );
DECLARE_READ32_DEVICE_HANDLER( k001604_reg_r );



#endif
