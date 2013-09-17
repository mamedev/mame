/************************************************************************************

    Bellfruit dotmatrix driver, (under heavy construction !!!)

*************************************************************************************/
#ifndef BFM_DM01
#define BFM_DM01

#define DM_BYTESPERROW 9

struct bfmdm01_interface
{
	void (*m_busy_func)(running_machine &machine, int state);
};

class bfmdm01_device : public device_t,
								public bfmdm01_interface
{
public:
	bfmdm01_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~bfmdm01_device() {}

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
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	int m_data_avail;
	int m_control;
	int m_xcounter;
	int m_segbuffer[65];
	int m_busy;

	UINT8 m_scanline[DM_BYTESPERROW];
	UINT8 m_comdata;

	int read_data(void);

};

extern const device_type BF_DM01;

ADDRESS_MAP_EXTERN( bfm_dm01_memmap,8 );


#define MCFG_DM01_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, BF_DM01, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#endif
