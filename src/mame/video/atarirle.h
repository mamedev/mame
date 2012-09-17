/***************************************************************************

    atarirle.h

    Common RLE-based motion object management functions for early 90's
    Atari raster games.

***************************************************************************/

#ifndef __ATARIRLE__
#define __ATARIRLE__


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define ATARIRLE_PRIORITY_SHIFT		12
#define ATARIRLE_BANK_SHIFT			15
#define ATARIRLE_PRIORITY_MASK		((~0 << ATARIRLE_PRIORITY_SHIFT) & 0xffff)
#define ATARIRLE_DATA_MASK			(ATARIRLE_PRIORITY_MASK ^ 0xffff)

#define ATARIRLE_CONTROL_MOGO		1
#define ATARIRLE_CONTROL_ERASE		2
#define ATARIRLE_CONTROL_FRAME		4

#define ATARIRLE_COMMAND_NOP		0
#define ATARIRLE_COMMAND_DRAW		1
#define ATARIRLE_COMMAND_CHECKSUM	2



/***************************************************************************
    TYPES & STRUCTURES
***************************************************************************/

/* description for an eight-word mask */
struct atarirle_entry
{
	UINT16			data[8];
};

/* description of the motion objects */
struct atarirle_desc
{
	const char *	region;				/* region where the GFX data lives */
	UINT16			spriteramentries;	/* number of entries in sprite RAM */
	UINT16			leftclip;			/* left clip coordinate */
	UINT16			rightclip;			/* right clip coordinate */

	UINT16			palettebase;		/* base palette entry */
	UINT16			maxcolors;			/* maximum number of colors */

	atarirle_entry	codemask;			/* mask for the code index */
	atarirle_entry	colormask;			/* mask for the color */
	atarirle_entry	xposmask;			/* mask for the X position */
	atarirle_entry	yposmask;			/* mask for the Y position */
	atarirle_entry	scalemask;			/* mask for the scale factor */
	atarirle_entry	hflipmask;			/* mask for the horizontal flip */
	atarirle_entry	ordermask;			/* mask for the order */
	atarirle_entry	prioritymask;		/* mask for the priority */
	atarirle_entry	vrammask;			/* mask for the VRAM target */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

class atarirle_device : public device_t
{
public:
	atarirle_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~atarirle_device() { global_free(m_token); }

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

extern const device_type ATARIRLE;

#define MCFG_ATARIRLE_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, ATARIRLE, 0) \
	MCFG_DEVICE_CONFIG(_interface)


/* control handlers */
void atarirle_control_w(device_t *device, UINT8 bits);
void atarirle_command_w(device_t *device, UINT8 command);

/* read/write handlers */
DECLARE_READ16_DEVICE_HANDLER( atarirle_spriteram_r );
DECLARE_READ32_DEVICE_HANDLER( atarirle_spriteram32_r );
DECLARE_WRITE16_DEVICE_HANDLER( atarirle_spriteram_w );
DECLARE_WRITE32_DEVICE_HANDLER( atarirle_spriteram32_w );

/* render helpers */
void atarirle_eof(device_t *device);
bitmap_ind16 *atarirle_get_vram(device_t *device, int idx);

#endif
