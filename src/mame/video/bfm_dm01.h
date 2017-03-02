// license:BSD-3-Clause
// copyright-holders:David Haywood
/************************************************************************************

    Bellfruit dotmatrix driver, (under heavy construction !!!)

*************************************************************************************/
#include "screen.h"

#ifndef BFM_DM01
#define BFM_DM01

#define DM_BYTESPERROW 9

#define MCFG_BF_DM01_BUSY_CB(_devcb) \
	devcb = &bfmdm01_device::set_busy_callback(*device, DEVCB_##_devcb);

class bfmdm01_device : public device_t
{
public:
	bfmdm01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~bfmdm01_device() {}

	template<class _Object> static devcb_base &set_busy_callback(device_t &device, _Object object) { return downcast<bfmdm01_device &>(device).m_busy_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( control_r );
	DECLARE_WRITE8_MEMBER( control_w );
	DECLARE_READ8_MEMBER( mux_r );
	DECLARE_WRITE8_MEMBER( mux_w );
	DECLARE_READ8_MEMBER( comm_r );
	DECLARE_WRITE8_MEMBER( comm_w );
	DECLARE_READ8_MEMBER( unknown_r );
	DECLARE_WRITE8_MEMBER( unknown_w );

	void writedata(uint8_t data);
	int busy(void);

	INTERRUPT_GEN_MEMBER(nmi_line_assert);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_device<cpu_device> m_matrixcpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	// internal state
	int m_data_avail;
	int m_control;
	int m_xcounter;
	int m_segbuffer[65];
	int m_busy;

	uint8_t m_scanline[DM_BYTESPERROW];
	uint8_t m_comdata;

	devcb_write_line m_busy_cb;

	int read_data(void);

	bitmap_ind16 m_tmpbitmap;

};

extern const device_type BF_DM01;


#endif
