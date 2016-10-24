// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski
#ifndef __TAITO8741__
#define __TAITO8741__


/****************************************************************************
  gladiatr and Great Swordsman set.
****************************************************************************/

/* NEC 8741 program mode */
#define TAITO8741_MASTER 0
#define TAITO8741_SLAVE  1
#define TAITO8741_PORT   2

#define MCFG_TAITO8741_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TAITO8741_4PACK, 0)

#define MCFG_TAITO8741_PORT_HANDLERS(_devcb0, _devcb1, _devcb2, _devcb3) \
	devcb = &taito8741_4pack_device::set_port_handler_0_callback(*device, DEVCB_##_devcb0); \
	devcb = &taito8741_4pack_device::set_port_handler_1_callback(*device, DEVCB_##_devcb1); \
	devcb = &taito8741_4pack_device::set_port_handler_2_callback(*device, DEVCB_##_devcb2); \
	devcb = &taito8741_4pack_device::set_port_handler_3_callback(*device, DEVCB_##_devcb3);

#define MCFG_TAITO8741_MODES(_mode0, _mode1, _mode2, _mode3) \
	taito8741_4pack_device::static_set_mode(*device, 0, _mode0);    \
	taito8741_4pack_device::static_set_mode(*device, 1, _mode1);    \
	taito8741_4pack_device::static_set_mode(*device, 2, _mode2);    \
	taito8741_4pack_device::static_set_mode(*device, 3, _mode3);


#define MCFG_TAITO8741_CONNECT(_con0, _con1, _con2, _con3) \
	taito8741_4pack_device::static_set_connect(*device, 0, _con0);  \
	taito8741_4pack_device::static_set_connect(*device, 1, _con1);  \
	taito8741_4pack_device::static_set_connect(*device, 2, _con2);  \
	taito8741_4pack_device::static_set_connect(*device, 3, _con3);


class taito8741_4pack_device : public device_t
{
	struct I8741 {
		int number;
		uint8_t toData;    /* to host data      */
		uint8_t fromData;  /* from host data    */
		uint8_t fromCmd;   /* from host command */
		uint8_t status;    /* b0 = rd ready,b1 = wd full,b2 = cmd ?? */
		uint8_t mode;
		uint8_t phase;
		uint8_t txd[8];
		uint8_t rxd[8];
		uint8_t parallelselect;
		uint8_t txpoint;
		int connect;
		uint8_t pending4a;
		int serial_out;
		int coins;
	};

public:
	taito8741_4pack_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~taito8741_4pack_device() {}

	template<class _Object> static devcb_base &set_port_handler_0_callback(device_t &device, _Object object) { return downcast<taito8741_4pack_device &>(device).m_port_handler_0_r.set_callback(object); }
	template<class _Object> static devcb_base &set_port_handler_1_callback(device_t &device, _Object object) { return downcast<taito8741_4pack_device &>(device).m_port_handler_1_r.set_callback(object); }
	template<class _Object> static devcb_base &set_port_handler_2_callback(device_t &device, _Object object) { return downcast<taito8741_4pack_device &>(device).m_port_handler_2_r.set_callback(object); }
	template<class _Object> static devcb_base &set_port_handler_3_callback(device_t &device, _Object object) { return downcast<taito8741_4pack_device &>(device).m_port_handler_3_r.set_callback(object); }

	static void static_set_mode(device_t &device, int num, uint8_t mode) { downcast<taito8741_4pack_device &>(device).m_taito8741[num].mode = mode; }
	static void static_set_connect(device_t &device, int num, int conn) { downcast<taito8741_4pack_device &>(device).m_taito8741[num].connect = conn; }

	uint8_t read_0(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { if(offset&1) return status_r(0); else return data_r(0); }
	void write_0(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { if(offset&1) command_w(0,data); else data_w(0,data); }
	uint8_t read_1(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { if(offset&1) return status_r(1); else return data_r(1); }
	void write_1(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { if(offset&1) command_w(1,data); else data_w(1,data); }
	uint8_t read_2(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { if(offset&1) return status_r(2); else return data_r(2); }
	void write_2(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { if(offset&1) command_w(2,data); else data_w(2,data); }
	uint8_t read_3(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { if(offset&1) return status_r(3); else return data_r(3); }
	void write_3(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { if(offset&1) command_w(3,data); else data_w(3,data); }

	void serial_tx(void *ptr, int32_t param);
	void update(int num);
	int status_r(int num);
	int data_r(int num);
	void data_w(int num, int data);
	void command_w(int num, int data);

	uint8_t port_read(int num, int offset);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void hostdata_w(I8741 *st,int data);
	int hostdata_r(I8741 *st);
	int hostcmd_r(I8741 *st);
	void serial_rx(I8741 *st,uint8_t *data);

	// internal state
	I8741       m_taito8741[4];

	devcb_read8 m_port_handler_0_r;
	devcb_read8 m_port_handler_1_r;
	devcb_read8 m_port_handler_2_r;
	devcb_read8 m_port_handler_3_r;
};


extern const device_type TAITO8741_4PACK;


/****************************************************************************
  joshi Volleyball set.
****************************************************************************/

#define MCFG_JOSVOLLY8741_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, JOSVOLLY8741_4PACK, 0)

#define MCFG_JOSVOLLY8741_PORT_HANDLERS(_devcb0, _devcb1, _devcb2, _devcb3) \
	devcb = &josvolly8741_4pack_device::set_port_handler_0_callback(*device, DEVCB_##_devcb0); \
	devcb = &josvolly8741_4pack_device::set_port_handler_1_callback(*device, DEVCB_##_devcb1); \
	devcb = &josvolly8741_4pack_device::set_port_handler_2_callback(*device, DEVCB_##_devcb2); \
	devcb = &josvolly8741_4pack_device::set_port_handler_3_callback(*device, DEVCB_##_devcb3);

#define MCFG_JOSVOLLY8741_CONNECT(_con0, _con1, _con2, _con3) \
	josvolly8741_4pack_device::static_set_connect(*device, 0, _con0);   \
	josvolly8741_4pack_device::static_set_connect(*device, 1, _con1);   \
	josvolly8741_4pack_device::static_set_connect(*device, 2, _con2);   \
	josvolly8741_4pack_device::static_set_connect(*device, 3, _con3);


class josvolly8741_4pack_device : public device_t
{
	struct JV8741  {
		uint8_t cmd;
		uint8_t sts;
		uint8_t txd;
		uint8_t outport;
		uint8_t rxd;
		uint8_t connect;
		uint8_t rst;
	};

public:
	josvolly8741_4pack_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~josvolly8741_4pack_device() {}

	template<class _Object> static devcb_base &set_port_handler_0_callback(device_t &device, _Object object) { return downcast<josvolly8741_4pack_device &>(device).m_port_handler_0_r.set_callback(object); }
	template<class _Object> static devcb_base &set_port_handler_1_callback(device_t &device, _Object object) { return downcast<josvolly8741_4pack_device &>(device).m_port_handler_1_r.set_callback(object); }
	template<class _Object> static devcb_base &set_port_handler_2_callback(device_t &device, _Object object) { return downcast<josvolly8741_4pack_device &>(device).m_port_handler_2_r.set_callback(object); }
	template<class _Object> static devcb_base &set_port_handler_3_callback(device_t &device, _Object object) { return downcast<josvolly8741_4pack_device &>(device).m_port_handler_3_r.set_callback(object); }

	static void static_set_connect(device_t &device, int num, int conn) { downcast<josvolly8741_4pack_device &>(device).m_i8741[num].connect = conn; }

	uint8_t read_0(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return read(0,offset); }
	void write_0(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { write(0,offset,data); }
	uint8_t read_1(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return read(1,offset); }
	void write_1(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { write(1,offset,data); }

	void nmi_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) { m_nmi_enable = 1; }

	void tx(void *ptr, int32_t param);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void update(int num);
	void write(int num, int offset, int data);
	uint8_t read(int num,int offset);
	uint8_t port_read(int num);

	// internal state
	JV8741 m_i8741[4];
	int m_nmi_enable;

	devcb_read8 m_port_handler_0_r;
	devcb_read8 m_port_handler_1_r;
	devcb_read8 m_port_handler_2_r;
	devcb_read8 m_port_handler_3_r;
};


extern const device_type JOSVOLLY8741_4PACK;


#endif
