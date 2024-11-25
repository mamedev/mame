// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

  HP-HIL Keyboard connector interface

***************************************************************************/

#ifndef MAME_BUS_HP_HIL_HP_HIL_H
#define MAME_BUS_HP_HIL_HP_HIL_H

#pragma once


#define HPMLC_R1_OB     0x10

#define HPMLC_W1_C      0x0800

#define HPMLC_R2_PERR   0x01
#define HPMLC_R2_FERR   0x02
#define HPMLC_R2_FOF    0x04

#define HPMLC_W2_TEST   0x01
#define HPMLC_W2_IPF    0x04

#define HPMLC_R3_INT    0x01
#define HPMLC_R3_NMI    0x02
#define HPMLC_R3_LERR   0x04

#define HPMLC_W3_APE    0x02

// commands
#define HPHIL_IFC       0x00    // Interface Clear
#define HPHIL_EPT       0x01    // Enter Pass-Thru Mode
#define HPHIL_ELB       0x02    // Enter Loop-Back Mode
#define HPHIL_IDD       0x03    // Identify and Describe
#define HPHIL_DSR       0x04    // Device Soft Reset
#define HPHIL_PST       0x05    // Perform Self Test
#define HPHIL_RRG       0x06    // Read Register
#define HPHIL_WRG       0x07    // Write Register
#define HPHIL_ACF       0x08    // Auto Configure [08..0f]
#define HPHIL_POL       0x10    // Poll [10..1f]
#define HPHIL_RPL       0x20    // RePoll [20..2f]
#define HPHIL_RNM       0x30    // Report Name
#define HPHIL_RST       0x31    // Report Status
#define HPHIL_EXD       0x32    // Extended Describe
#define HPHIL_RSC       0x33    // Report Security Code
#define HPHIL_DKA       0x3D    // Disable Keyswitch AutoRepeat
#define HPHIL_EK1       0x3E    // Enable Keyswitch AutoRepeat 30cps
#define HPHIL_EK2       0x3F    // Enable Keyswitch AutoRepeat 60cps
#define HPHIL_PR1       0x40    // Prompt 1..7 [40..46]
#define HPHIL_PRM       0x47    // Prompt (General Purpose)
#define HPHIL_AK1       0x48    // Acknowledge 1..7 [40..46]
#define HPHIL_ACK       0x4F    // Acknowledge (General Purpose)
#define HPHIL_RIO       0xFA    // Register I/O Error
#define HPHIL_SHR       0xFB    // System Hard Reset
#define HPHIL_TER       0xFC    // Transmission Error
#define HPHIL_CAE       0xFD    // Configuration Address Error
#define HPHIL_DHR       0xFE    // Device Hard Reset

/*
 * init sequence (p. 4-13)
 *
 * DHR
 * IFC
 * ACF
 * IDD
 * EXD
 * EPT
 * ELB
 * RPL
 * POL
 * EPT
 * ELB
 *
 */

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_hp_hil_interface;
class hp_hil_mlc_device;


class hp_hil_slot_device : public device_t, public device_single_card_slot_interface<device_hp_hil_interface>
{
public:
	// construction/destruction
	template <typename T, typename U>
	hp_hil_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&mlc_tag, U &&opts, const char *dflt)
		: hp_hil_slot_device(mconfig, tag, owner, 0)
	{
		m_mlc.set_tag(std::forward<T>(mlc_tag));
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	hp_hil_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// configuration
	required_device<hp_hil_mlc_device> m_mlc;
};


// device type definition
DECLARE_DEVICE_TYPE(HP_HIL_SLOT, hp_hil_slot_device)


class hp_hil_mlc_device : public device_t
{
public:
	// construction/destruction
	hp_hil_mlc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~hp_hil_mlc_device() { m_device_list.detach_all(); }

	auto int_callback() { return int_cb.bind(); }
	auto nmi_callback() { return nmi_cb.bind(); }

	void add_hp_hil_device(device_hp_hil_interface *device);
	bool get_int(void) { return m_r3 & 1; }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	void ap_w(int state);

	void hil_write(uint16_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	simple_list<device_hp_hil_interface> m_device_list;

	util::fifo<uint16_t, 16> m_fifo;
	uint8_t m_r2, m_r3, m_w1, m_w2, m_w3, m_loop;

private:
	devcb_write_line int_cb, nmi_cb;
};


// device type definition
DECLARE_DEVICE_TYPE(HP_HIL_MLC, hp_hil_mlc_device)


// ======================> device_hp_hil_interface

class device_hp_hil_interface : public device_interface
{
	friend class hp_hil_mlc_device;
	template <class ElementType> friend class simple_list;

public:
	// construction/destruction
	virtual ~device_hp_hil_interface();

	device_hp_hil_interface *next() const { return m_next; }

	void set_hp_hil_mlc_device();

	// inline configuration
	void set_hp_hil_mlc(hp_hil_mlc_device &mlc_device) { m_hp_hil_mlc = &mlc_device; }

	virtual bool hil_write(uint16_t *data) { return true; }
	int device_id() { return m_device_id; }

protected:
	device_hp_hil_interface(const machine_config &mconfig, device_t &device);

	virtual void device_reset() { }

	hp_hil_mlc_device       *m_hp_hil_mlc;

	hp_hil_slot_device *m_slot;

	int                     m_device_id;
	uint16_t                m_device_id16;

private:
	device_hp_hil_interface *m_next;
};


#endif  // MAME_BUS_HP_HIL_HP_HIL_H
