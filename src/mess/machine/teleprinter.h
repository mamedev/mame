#ifndef __TELEPRINTER_H__
#define __TELEPRINTER_H__

#include "machine/terminal.h"

#define TELEPRINTER_WIDTH 80
#define TELEPRINTER_HEIGHT 50

#define GENERIC_TELEPRINTER_INTERFACE GENERIC_TERMINAL_INTERFACE

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/
#define TELEPRINTER_TAG "teleprinter"
#define TELEPRINTER_SCREEN_TAG "tty_screen"

#define MCFG_GENERIC_TELEPRINTER_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, TELEPRINTER, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_GENERIC_TELEPRINTER_REMOVE(_tag)		\
	MCFG_DEVICE_REMOVE(_tag)

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

class teleprinter_device : public generic_terminal_device
{
public:
	teleprinter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	UINT32 tp_update(screen_device &device, bitmap_rgb32 &bitmap, const rectangle &cliprect);
protected:
	virtual void term_write(UINT8 data);
	virtual void device_reset();
	machine_config_constructor device_mconfig_additions() const;
private:
	void scroll_line();
	void write_char(UINT8 data);
	void clear();
};

extern const device_type TELEPRINTER;

#endif /* __TELEPRINTER_H__ */
