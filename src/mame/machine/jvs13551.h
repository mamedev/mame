// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef __JVS13551_H__
#define __JVS13551_H__

#include "emu.h"
#include "machine/jvsdev.h"

#define MCFG_SEGA_837_13551_DEVICE_ADD(_tag, _host, tilt, d0, d1, a0, a1, a2, a3, a4, a5, a6, a7, out) \
	MCFG_JVS_DEVICE_ADD(_tag, SEGA_837_13551, _host) \
	sega_837_13551::static_set_port_tag(*device, 0, tilt);  \
	sega_837_13551::static_set_port_tag(*device, 1, d0); \
	sega_837_13551::static_set_port_tag(*device, 2, d1); \
	sega_837_13551::static_set_port_tag(*device, 3, a0); \
	sega_837_13551::static_set_port_tag(*device, 4, a1); \
	sega_837_13551::static_set_port_tag(*device, 5, a2); \
	sega_837_13551::static_set_port_tag(*device, 6, a3); \
	sega_837_13551::static_set_port_tag(*device, 7, a4); \
	sega_837_13551::static_set_port_tag(*device, 8, a5); \
	sega_837_13551::static_set_port_tag(*device, 9, a6); \
	sega_837_13551::static_set_port_tag(*device, 10, a7); \
	sega_837_13551::static_set_port_tag(*device, 11, out);

class jvs_host;

class sega_837_13551 : public jvs_device
{
public:
	sega_837_13551(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	static void static_set_port_tag(device_t &device, int port, const char *tag);

	virtual const rom_entry *device_rom_region() const;

	DECLARE_WRITE_LINE_MEMBER(jvs13551_coin_1_w);
	DECLARE_WRITE_LINE_MEMBER(jvs13551_coin_2_w);
	void inc_coin(int coin);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual ioport_constructor device_input_ports() const;

	// JVS device overrides
	virtual const char *device_id();
	virtual UINT8 command_format_version();
	virtual UINT8 jvs_standard_version();
	virtual UINT8 comm_method_version();
	virtual void function_list(UINT8 *&buf);
	virtual bool switches(UINT8 *&buf, UINT8 count_players, UINT8 bytes_per_switch);
	virtual bool coin_counters(UINT8 *&buf, UINT8 count);
	virtual bool coin_add(UINT8 slot, INT32 count);
	virtual bool analogs(UINT8 *&buf, UINT8 count);
	virtual bool swoutputs(UINT8 count, const UINT8 *vals);
	virtual bool swoutputs(UINT8 id, UINT8 val);

private:
	const char *port_tag[12];
	UINT16 coin_counter[2];
};

extern const device_type SEGA_837_13551;

#endif
