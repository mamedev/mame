/**********************************************************************

    Hudson/NEC HuC6202 interface

**********************************************************************/

#ifndef __HUC6202_H_
#define __HUC6202_H_

#include "emu.h"
#include "machine/devhelpr.h"


#define MCFG_HUC6202_ADD( _tag, _intrf )	\
	MCFG_DEVICE_ADD( _tag, HUC6202, 0 )		\
	MCFG_DEVICE_CONFIG( _intrf )


struct huc6202_interface
{
	/* First gfx input device */
	devcb_read16				device_0_next_pixel;

	/* TODO: Choose proper types */
	/* Callback function to get time until next event */
	devcb_read16				get_time_til_next_event_0;

	devcb_write_line			vsync_0_changed;
	devcb_write_line			hsync_0_changed;
	devcb_read8					read_0;
	devcb_write8				write_0;


	/* Second gfx input device */
	devcb_read16				device_1_next_pixel;

	/* TODO: Choose proper types */
	/* Callback function to get time until next event */
	devcb_read16				get_time_til_next_event_1;

	devcb_write_line			vsync_1_changed;
	devcb_write_line			hsync_1_changed;
	devcb_read8					read_1;
	devcb_write8				write_1;
};


class huc6202_device : public device_t,
						public huc6202_interface
{
public:
	// construction/destruction
	huc6202_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( io_read );
	DECLARE_WRITE8_MEMBER( io_write );
	DECLARE_READ16_MEMBER( next_pixel );
	DECLARE_READ16_MEMBER( time_until_next_event );
	DECLARE_WRITE_LINE_MEMBER( vsync_changed );
	DECLARE_WRITE_LINE_MEMBER( hsync_changed );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	/* callbacks */
	devcb_resolved_read16		m_next_pixel_0;
	devcb_resolved_read16		m_get_time_til_next_event_0;
	devcb_resolved_write_line	m_hsync_changed_0;
	devcb_resolved_write_line	m_vsync_changed_0;
	devcb_resolved_read8		m_read_0;
	devcb_resolved_write8		m_write_0;
	devcb_resolved_read16		m_next_pixel_1;
	devcb_resolved_read16		m_get_time_til_next_event_1;
	devcb_resolved_write_line	m_hsync_changed_1;
	devcb_resolved_write_line	m_vsync_changed_1;
	devcb_resolved_read8		m_read_1;
	devcb_resolved_write8		m_write_1;

	struct {
		UINT8	prio_type;
		UINT8	dev0_enabled;
		UINT8	dev1_enabled;
	} m_prio[4];
	UINT16	m_window1;
	UINT16	m_window2;
	int		m_io_device;
	int		m_map_index;
	int		m_map_dirty;
	UINT8	m_prio_map[512];

};


extern const device_type HUC6202;


#endif

