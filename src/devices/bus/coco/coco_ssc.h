// license:BSD-3-Clause
// copyright-holders:tim lindner
#pragma once

#ifndef __COCO_SSC_H__
#define __COCO_SSC_H__

#include "machine/ram.h"
#include "sound/ay8910.h"
#include "sound/sp0256.h"
#include "machine/netlist.h"
#include "netlist/devices/net_lib.h"
#include "cococart.h"

// #define SAC_ON

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> coco_ssc_device

class coco_ssc_device :
		public device_t,
		public device_cococart_interface
{
public:
		// construction/destruction
		coco_ssc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

		// optional information overrides
		virtual const tiny_rom_entry *device_rom_region() const override;
		virtual machine_config_constructor device_mconfig_additions() const override;
		void cart_set_line(cococart_slot_device::line which, cococart_slot_device::line_value value);

		virtual void device_reset() override;

		DECLARE_READ8_MEMBER(ssc_port_a_r);
		DECLARE_WRITE8_MEMBER(ssc_port_b_w);
		DECLARE_READ8_MEMBER(ssc_port_c_r);
		DECLARE_WRITE8_MEMBER(ssc_port_c_w);
		DECLARE_READ8_MEMBER(ssc_port_d_r);
		DECLARE_WRITE8_MEMBER(ssc_port_d_w);

#ifdef SAC_ON
		NETDEV_LOGIC_CALLBACK_MEMBER(sac_cb);
#endif
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual DECLARE_READ8_MEMBER(ff7d_read);
		virtual DECLARE_WRITE8_MEMBER(ff7d_write);
private:
		uint8_t reset_line;
		uint8_t tms7000_porta;
		uint8_t tms7000_portb;
		uint8_t tms7000_portc;
		uint8_t tms7000_portd;
		required_device<cpu_device> m_tms7040;
		required_device<ram_device> m_staticram;
		required_device<ay8910_device> m_ay;
		required_device<sp0256_device> m_spo;
};


// device type definition
extern const device_type COCO_SSC;

#endif  /* __COCO_SSC_H__ */
