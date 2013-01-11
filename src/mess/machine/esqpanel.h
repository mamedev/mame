#pragma once

#ifndef __ESQPANEL_H__
#define __ESQPANEL_H__

#include "emu.h"
#include "machine/esqvfd.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ESQPANEL1x22_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, ESQPANEL1x22, 0) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_ESQPANEL1x22_REPLACE(_tag, _config) \
	MCFG_DEVICE_REPLACE(_tag, ESQPANEL1x22, 0) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_ESQPANEL1x22_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

#define MCFG_ESQPANEL2x40_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, ESQPANEL2x40, 0) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_ESQPANEL2x40_REPLACE(_tag, _config) \
	MCFG_DEVICE_REPLACE(_tag, ESQPANEL2x40, 0) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_ESQPANEL_2x40_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

#define MCFG_ESQPANEL2x40_SQ1_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, ESQPANEL2x40_SQ1, 0) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_ESQPANEL2x40_SQ1_REPLACE(_tag, _config) \
	MCFG_DEVICE_REPLACE(_tag, ESQPANEL2x40_SQ1, 0) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_ESQPANEL2x40_SQ1_REMOVE(_tag) \
	MCFG_DEVICE_REMOVE(_tag)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct esqpanel_interface
{
	devcb_write_line    m_out_tx_cb;
};

// ======================> esqpanel_device

class esqpanel_device :  public device_t, public device_serial_interface, public esqpanel_interface
{
public:
	// construction/destruction
	esqpanel_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER( rx_w ) { check_for_start(state); }

	virtual void send_to_display(UINT8 data) = 0;

	void xmit_char(UINT8 data);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

	// serial overrides
	virtual void rcv_complete();    // Rx completed receiving byte
	virtual void tra_complete();    // Tx completed sending byte
	virtual void tra_callback();    // Tx send bit
	void input_callback(UINT8 state);

	bool m_eps_mode;

private:
	static const int XMIT_RING_SIZE = 16;

	bool  m_bCalibSecondByte;

	devcb_resolved_write_line m_out_tx_func;
	UINT8 m_xmitring[XMIT_RING_SIZE];
	int m_xmit_read, m_xmit_write;
	bool m_tx_busy;
};

class esqpanel1x22_device : public esqpanel_device {
public:
	esqpanel1x22_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	required_device<esq1x22_t> m_vfd;

	virtual void send_to_display(UINT8 data) { m_vfd->write_char(data); }

protected:
	virtual machine_config_constructor device_mconfig_additions() const;

private:
};

class esqpanel2x40_device : public esqpanel_device {
public:
	esqpanel2x40_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	required_device<esq2x40_t> m_vfd;

	virtual void send_to_display(UINT8 data) { m_vfd->write_char(data); }

protected:
	virtual machine_config_constructor device_mconfig_additions() const;

private:
};

class esqpanel2x40_sq1_device : public esqpanel_device {
public:
	esqpanel2x40_sq1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	required_device<esq2x40_sq1_t> m_vfd;

	virtual void send_to_display(UINT8 data) { m_vfd->write_char(data); }

protected:
	virtual machine_config_constructor device_mconfig_additions() const;

private:
};

extern const device_type ESQPANEL1x22;
extern const device_type ESQPANEL2x40;
extern const device_type ESQPANEL2x40_SQ1;

#endif
