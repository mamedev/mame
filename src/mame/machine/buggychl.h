// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Nicola Salmoria
class buggychl_mcu_device : public device_t
{
public:
	buggychl_mcu_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~buggychl_mcu_device() {}

	DECLARE_WRITE8_MEMBER( buggychl_mcu_w );
	DECLARE_READ8_MEMBER( buggychl_mcu_r );
	DECLARE_READ8_MEMBER( buggychl_mcu_status_r );
	DECLARE_READ8_MEMBER( buggychl_68705_port_a_r );
	DECLARE_WRITE8_MEMBER( buggychl_68705_port_a_w );
	DECLARE_WRITE8_MEMBER( buggychl_68705_ddr_a_w );
	DECLARE_READ8_MEMBER( buggychl_68705_port_b_r );
	DECLARE_WRITE8_MEMBER( buggychl_68705_port_b_w );
	DECLARE_WRITE8_MEMBER( buggychl_68705_ddr_b_w );
	DECLARE_READ8_MEMBER( buggychl_68705_port_c_r );
	DECLARE_WRITE8_MEMBER( buggychl_68705_port_c_w );
	DECLARE_WRITE8_MEMBER( buggychl_68705_ddr_c_w );

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	UINT8       m_port_a_in;
	UINT8       m_port_a_out;
	UINT8       m_ddr_a;
	UINT8       m_port_b_in;
	UINT8       m_port_b_out;
	UINT8       m_ddr_b;
	UINT8       m_port_c_in;
	UINT8       m_port_c_out;
	UINT8       m_ddr_c;
	UINT8       m_from_main;
	UINT8       m_from_mcu;
	int         m_mcu_sent;
	int         m_main_sent;
	device_t *m_mcu;
};

ADDRESS_MAP_EXTERN( buggychl_mcu_map, 8 );

extern const device_type BUGGYCHL_MCU;
