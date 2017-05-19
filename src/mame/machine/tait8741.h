// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski
#ifndef MAME_MACHINE_TAITO8741_H
#define MAME_MACHINE_TAITO8741_H

#pragma once

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
public:
	taito8741_4pack_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> static devcb_base &set_port_handler_0_callback(device_t &device, Object &&cb) { return downcast<taito8741_4pack_device &>(device).m_port_handler_0_r.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port_handler_1_callback(device_t &device, Object &&cb) { return downcast<taito8741_4pack_device &>(device).m_port_handler_1_r.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port_handler_2_callback(device_t &device, Object &&cb) { return downcast<taito8741_4pack_device &>(device).m_port_handler_2_r.set_callback(std::forward<Object>(cb)); }
	template <class Object> static devcb_base &set_port_handler_3_callback(device_t &device, Object &&cb) { return downcast<taito8741_4pack_device &>(device).m_port_handler_3_r.set_callback(std::forward<Object>(cb)); }

	static void static_set_mode(device_t &device, int num, uint8_t mode) { downcast<taito8741_4pack_device &>(device).m_taito8741[num].mode = mode; }
	static void static_set_connect(device_t &device, int num, int conn) { downcast<taito8741_4pack_device &>(device).m_taito8741[num].connect = conn; }

	DECLARE_READ8_MEMBER( read_0 ) { if(offset&1) return status_r(0); else return data_r(0); }
	DECLARE_WRITE8_MEMBER( write_0 ) { if(offset&1) command_w(0,data); else data_w(0,data); }
	DECLARE_READ8_MEMBER( read_1 ) { if(offset&1) return status_r(1); else return data_r(1); }
	DECLARE_WRITE8_MEMBER( write_1 ) { if(offset&1) command_w(1,data); else data_w(1,data); }
	DECLARE_READ8_MEMBER( read_2 ) { if(offset&1) return status_r(2); else return data_r(2); }
	DECLARE_WRITE8_MEMBER( write_2 ) { if(offset&1) command_w(2,data); else data_w(2,data); }
	DECLARE_READ8_MEMBER( read_3 ) { if(offset&1) return status_r(3); else return data_r(3); }
	DECLARE_WRITE8_MEMBER( write_3 ) { if(offset&1) command_w(3,data); else data_w(3,data); }

	TIMER_CALLBACK_MEMBER( serial_tx );
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


DECLARE_DEVICE_TYPE(TAITO8741_4PACK, taito8741_4pack_device)

#endif // MAME_MACHINE_TAITO8741_H
