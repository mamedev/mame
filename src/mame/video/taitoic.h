/*************************************************************************

    taitoic.h

    Implementation of various Taito custom video & input ICs

**************************************************************************/

#include "devlegcy.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct pc080sn_interface
{
	int                gfxnum;

	int                x_offset, y_offset;
	int                y_invert;
	int                dblwidth;
};


struct pc090oj_interface
{
	int                gfxnum;

	int                x_offset, y_offset;
	int                use_buffer;
};


struct tc0080vco_interface
{
	int                gfxnum;
	int                txnum;

	int                bg_xoffs, bg_yoffs;
	int                bg_flip_yoffs;

	int                has_fg0;	/* for debug */
};

struct tc0100scn_interface
{
	const char         *screen;

	int                gfxnum;
	int                txnum;

	int                x_offset, y_offset;
	int                flip_xoffs, flip_yoffs;
	int                flip_text_xoffs, flip_text_yoffs;

	int                multiscrn_xoffs;
	int                multiscrn_hack;
};


struct tc0280grd_interface
{
	int                gfxnum;
};


struct tc0480scp_interface
{
	int                gfxnum;
	int                txnum;

	int                pixels;

	int                x_offset, y_offset;
	int                text_xoffs, text_yoffs;
	int                flip_xoffs, flip_yoffs;

	int                col_base;
};


struct tc0150rod_interface
{
	const char      *gfx_region;	/* gfx region for the road */
};


struct tc0110pcr_interface
{
	int               pal_offs;
};

struct tc0180vcu_interface
{
	int            bg_color_base;
	int            fg_color_base;
	int            tx_color_base;
};

class pc080sn_device : public device_t
{
public:
	pc080sn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~pc080sn_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
private:
	// internal state
	void *m_token;

	TILE_GET_INFO_MEMBER(pc080sn_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(pc080sn_get_fg_tile_info);
};

extern const device_type PC080SN;

class pc090oj_device : public device_t
{
public:
	pc090oj_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~pc090oj_device() { global_free(m_token); }

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

extern const device_type PC090OJ;

class tc0080vco_device : public device_t
{
public:
	tc0080vco_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0080vco_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
private:
	// internal state
	void *m_token;

	TILE_GET_INFO_MEMBER(tc0080vco_get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(tc0080vco_get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(tc0080vco_get_tx_tile_info);
};

extern const device_type TC0080VCO;

class tc0100scn_device : public device_t
{
public:
	tc0100scn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0100scn_device() { global_free(m_token); }

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

	TILE_GET_INFO_MEMBER(tc0100scn_get_bg_tile_info);
	TILE_GET_INFO_MEMBER(tc0100scn_get_fg_tile_info);
	TILE_GET_INFO_MEMBER(tc0100scn_get_tx_tile_info);
};

extern const device_type TC0100SCN;

class tc0280grd_device : public device_t
{
public:
	tc0280grd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0280grd_device() { global_free(m_token); }

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

	TILE_GET_INFO_MEMBER(tc0280grd_get_tile_info);
};

extern const device_type TC0280GRD;

#define TC0430GRW TC0280GRD
class tc0360pri_device : public device_t
{
public:
	tc0360pri_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0360pri_device() { global_free(m_token); }

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

extern const device_type TC0360PRI;

class tc0480scp_device : public device_t
{
public:
	tc0480scp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0480scp_device() { global_free(m_token); }

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

	TILE_GET_INFO_MEMBER(tc0480scp_get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(tc0480scp_get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(tc0480scp_get_bg2_tile_info);
	TILE_GET_INFO_MEMBER(tc0480scp_get_bg3_tile_info);
	TILE_GET_INFO_MEMBER(tc0480scp_get_tx_tile_info);
};

extern const device_type TC0480SCP;

class tc0150rod_device : public device_t
{
public:
	tc0150rod_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0150rod_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
private:
	// internal state
	void *m_token;
};

extern const device_type TC0150ROD;

class tc0110pcr_device : public device_t
{
public:
	tc0110pcr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0110pcr_device() { global_free(m_token); }

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

extern const device_type TC0110PCR;

class tc0180vcu_device : public device_t
{
public:
	tc0180vcu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tc0180vcu_device() { global_free(m_token); }

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

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);
};

extern const device_type TC0180VCU;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_PC080SN_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, PC080SN, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_PC090OJ_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, PC090OJ, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0080VCO_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0080VCO, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0100SCN_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0100SCN, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0280GRD_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0280GRD, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0430GRW_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0430GRW, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0360PRI_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TC0360PRI, 0)

#define MCFG_TC0150ROD_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0150ROD, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0480SCP_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0480SCP, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0110PCR_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0110PCR, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#define MCFG_TC0180VCU_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TC0180VCU, 0) \
	MCFG_DEVICE_CONFIG(_interface)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

/**  PC080SN  **/
DECLARE_READ16_DEVICE_HANDLER( pc080sn_word_r );
DECLARE_WRITE16_DEVICE_HANDLER( pc080sn_word_w );
DECLARE_WRITE16_DEVICE_HANDLER( pc080sn_xscroll_word_w );
DECLARE_WRITE16_DEVICE_HANDLER( pc080sn_yscroll_word_w );
DECLARE_WRITE16_DEVICE_HANDLER( pc080sn_ctrl_word_w );

void pc080sn_set_scroll(device_t *device, int tilemap_num, int scrollx, int scrolly);
void pc080sn_set_trans_pen(device_t *device, int tilemap_num, int pen);
void pc080sn_tilemap_update(device_t *device);
void pc080sn_tilemap_draw(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority);
void pc080sn_tilemap_draw_offset(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority, int xoffs, int yoffs);

/* For Topspeed */
void pc080sn_tilemap_draw_special(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority, UINT16 *ram);


/**  PC090OJ  **/
DECLARE_READ16_DEVICE_HANDLER( pc090oj_word_r );
DECLARE_WRITE16_DEVICE_HANDLER( pc090oj_word_w );

void pc090oj_set_sprite_ctrl(device_t *device, UINT16 sprctrl);
void pc090oj_eof_callback(device_t *device);
void pc090oj_draw_sprites(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri_type);


/** TC0080VCO **/
DECLARE_READ16_DEVICE_HANDLER( tc0080vco_word_r );
DECLARE_WRITE16_DEVICE_HANDLER( tc0080vco_word_w );

void tc0080vco_tilemap_update(device_t *device);
void tc0080vco_tilemap_draw(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority);

DECLARE_READ16_DEVICE_HANDLER( tc0080vco_cram_0_r );
DECLARE_READ16_DEVICE_HANDLER( tc0080vco_cram_1_r );
DECLARE_READ16_DEVICE_HANDLER( tc0080vco_sprram_r );
DECLARE_READ16_DEVICE_HANDLER( tc0080vco_scrram_r );
READ_LINE_DEVICE_HANDLER( tc0080vco_flipscreen_r );


/** TC0100SCN **/
#define TC0100SCN_SINGLE_VDU    1024

/* Function to set separate color banks for the three tilemapped layers.
   To change from the default (0,0,0) use after calling TC0100SCN_vh_start */
void tc0100scn_set_colbanks(device_t *device, int bg0, int bg1, int tx);

/* Function to set separate color banks for each TC0100SCN.
   To change from the default (0,0,0) use after calling TC0100SCN_vh_start */
void tc0100scn_set_colbank(device_t *device, int colbank);

/* Function to set bg tilemask < 0xffff */
void tc0100scn_set_bg_tilemask(device_t *device, int mask);

/* Function to for Mjnquest to select gfx bank */
DECLARE_WRITE16_DEVICE_HANDLER( tc0100scn_gfxbank_w );

DECLARE_READ16_DEVICE_HANDLER( tc0100scn_word_r );
DECLARE_WRITE16_DEVICE_HANDLER( tc0100scn_word_w );
DECLARE_READ16_DEVICE_HANDLER( tc0100scn_ctrl_word_r );
DECLARE_WRITE16_DEVICE_HANDLER( tc0100scn_ctrl_word_w );

/* Functions for use with 68020 (Under Fire) */
DECLARE_READ32_DEVICE_HANDLER( tc0100scn_long_r );
DECLARE_WRITE32_DEVICE_HANDLER( tc0100scn_long_w );
DECLARE_READ32_DEVICE_HANDLER( tc0100scn_ctrl_long_r );
DECLARE_WRITE32_DEVICE_HANDLER( tc0100scn_ctrl_long_w );

void tc0100scn_tilemap_update(device_t *device);
int tc0100scn_tilemap_draw(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority);

/* returns 0 or 1 depending on the lowest priority tilemap set in the internal
   register. Use this function to draw tilemaps in the correct order. */
int tc0100scn_bottomlayer(device_t *device);


/** TC0280GRD & TC0430GRW **/
DECLARE_READ16_DEVICE_HANDLER( tc0280grd_word_r );
DECLARE_WRITE16_DEVICE_HANDLER( tc0280grd_word_w );
DECLARE_WRITE16_DEVICE_HANDLER( tc0280grd_ctrl_word_w );
void tc0280grd_tilemap_update(device_t *device, int base_color);
void tc0280grd_zoom_draw(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, UINT32 priority);

DECLARE_READ16_DEVICE_HANDLER( tc0430grw_word_r );
DECLARE_WRITE16_DEVICE_HANDLER( tc0430grw_word_w );
DECLARE_WRITE16_DEVICE_HANDLER( tc0430grw_ctrl_word_w );
void tc0430grw_tilemap_update(device_t *device, int base_color);
void tc0430grw_zoom_draw(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, UINT32 priority);


/** TC0360PRI **/
DECLARE_WRITE8_DEVICE_HANDLER( tc0360pri_w );
DECLARE_READ8_DEVICE_HANDLER( tc0360pri_r );


/** TC0480SCP **/
/* When writing a driver, pass zero for the text and flip offsets initially:
   then tweak them once you have the 4 bg layer positions correct. Col_base
   may be needed when tilemaps use a palette area from sprites. */

DECLARE_READ16_DEVICE_HANDLER( tc0480scp_word_r );
DECLARE_WRITE16_DEVICE_HANDLER( tc0480scp_word_w );
DECLARE_READ16_DEVICE_HANDLER( tc0480scp_ctrl_word_r );
DECLARE_WRITE16_DEVICE_HANDLER( tc0480scp_ctrl_word_w );

/* Functions for use with 68020 (Super-Z system) */
DECLARE_READ32_DEVICE_HANDLER( tc0480scp_long_r );
DECLARE_WRITE32_DEVICE_HANDLER( tc0480scp_long_w );
DECLARE_READ32_DEVICE_HANDLER( tc0480scp_ctrl_long_r );
DECLARE_WRITE32_DEVICE_HANDLER( tc0480scp_ctrl_long_w );

void tc0480scp_tilemap_update(device_t *device);
void tc0480scp_tilemap_draw(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority);

/* Returns the priority order of the bg tilemaps set in the internal
   register. The order in which the four layers should be drawn is
   returned in the lowest four nibbles  (msn = bottom layer; lsn = top) */
int tc0480scp_get_bg_priority(device_t *device);

/* Undrfire needs to read this for a sprite/tile priority hack */
DECLARE_READ8_DEVICE_HANDLER( tc0480scp_pri_reg_r );


/** TC0150ROD **/
DECLARE_READ16_DEVICE_HANDLER( tc0150rod_word_r );
DECLARE_WRITE16_DEVICE_HANDLER( tc0150rod_word_w );
void tc0150rod_draw(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs, int palette_offs, int type, int road_trans, UINT32 low_priority, UINT32 high_priority);


/** TC0110PCR **/
DECLARE_READ16_DEVICE_HANDLER( tc0110pcr_word_r );
DECLARE_WRITE16_DEVICE_HANDLER( tc0110pcr_word_w );	/* color index goes up in step of 2 */
DECLARE_WRITE16_DEVICE_HANDLER( tc0110pcr_step1_word_w );	/* color index goes up in step of 1 */
DECLARE_WRITE16_DEVICE_HANDLER( tc0110pcr_step1_rbswap_word_w );	/* swaps red and blue components */
DECLARE_WRITE16_DEVICE_HANDLER( tc0110pcr_step1_4bpg_word_w );	/* only 4 bits per color gun */


/** TC0180VCU **/
DECLARE_READ8_DEVICE_HANDLER( tc0180vcu_get_fb_page );
DECLARE_WRITE8_DEVICE_HANDLER( tc0180vcu_set_fb_page );
DECLARE_READ8_DEVICE_HANDLER( tc0180vcu_get_videoctrl );
DECLARE_READ16_DEVICE_HANDLER( tc0180vcu_ctrl_r );
DECLARE_WRITE16_DEVICE_HANDLER( tc0180vcu_ctrl_w );
DECLARE_READ16_DEVICE_HANDLER( tc0180vcu_scroll_r );
DECLARE_WRITE16_DEVICE_HANDLER( tc0180vcu_scroll_w );
DECLARE_READ16_DEVICE_HANDLER( tc0180vcu_word_r );
DECLARE_WRITE16_DEVICE_HANDLER( tc0180vcu_word_w );
void tc0180vcu_tilemap_draw(device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int tmap_num, int plane);
