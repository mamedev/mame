ADDRESS_MAP_EXTERN( buggychl_mcu_map, 8 );

DECLARE_WRITE8_DEVICE_HANDLER( buggychl_mcu_w );
DECLARE_READ8_DEVICE_HANDLER( buggychl_mcu_r );
DECLARE_READ8_DEVICE_HANDLER( buggychl_mcu_status_r );

class buggychl_mcu_device : public device_t
{
public:
	buggychl_mcu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~buggychl_mcu_device() { global_free(m_token); }

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

extern const device_type BUGGYCHL_MCU;
