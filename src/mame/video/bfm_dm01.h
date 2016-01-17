// license:BSD-3-Clause
// copyright-holders:David Haywood
/************************************************************************************

    Bellfruit dotmatrix driver, (under heavy construction !!!)

*************************************************************************************/
#ifndef BFM_DM01
#define BFM_DM01

#define DM_BYTESPERROW 9

#define MCFG_BF_DM01_BUSY_CB(_devcb) \
	devcb = &bfmdm01_device::set_busy_callback(*device, DEVCB_##_devcb);

class bfmdm01_device : public device_t
{
public:
	bfmdm01_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
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

	void writedata(UINT8 data);
	int busy(void);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	int m_data_avail;
	int m_control;
	int m_xcounter;
	int m_segbuffer[65];
	int m_busy;

	UINT8 m_scanline[DM_BYTESPERROW];
	UINT8 m_comdata;

	devcb_write_line m_busy_cb;

	int read_data(void);
};

extern const device_type BF_DM01;

ADDRESS_MAP_EXTERN( bfm_dm01_memmap,8 );

#endif
