// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Nicola Salmoria
class buggychl_mcu_device : public device_t
{
public:
	buggychl_mcu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~buggychl_mcu_device() {}

	void buggychl_mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t buggychl_mcu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t buggychl_mcu_status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t buggychl_68705_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void buggychl_68705_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void buggychl_68705_ddr_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t buggychl_68705_port_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void buggychl_68705_port_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void buggychl_68705_ddr_b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t buggychl_68705_port_c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void buggychl_68705_port_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void buggychl_68705_ddr_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state
	uint8_t       m_port_a_in;
	uint8_t       m_port_a_out;
	uint8_t       m_ddr_a;
	uint8_t       m_port_b_in;
	uint8_t       m_port_b_out;
	uint8_t       m_ddr_b;
	uint8_t       m_port_c_in;
	uint8_t       m_port_c_out;
	uint8_t       m_ddr_c;
	uint8_t       m_from_main;
	uint8_t       m_from_mcu;
	int         m_mcu_sent;
	int         m_main_sent;
	device_t *m_mcu;
};

ADDRESS_MAP_EXTERN( buggychl_mcu_map, 8 );

extern const device_type BUGGYCHL_MCU;
