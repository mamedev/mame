// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore VIC-10 Expansion Port emulation

**********************************************************************

                    GND       1      A       GND
                    +5V       2      B       _UPROM
                    +5V       3      C       _RESET
                   _IRQ       4      D       _NMI
                  _CR/W       5      E       Sphi2
                     SP       6      F       CA15
                 _EXRAM       7      H       CA14
                    CNT       8      J       CA13
                   _CIA       9      K       CA12
               _CIA PLA      10      L       CA11
                 _LOROM      11      M       CA10
                     BA      12      N       CA9
               R/_W PLA      13      P       CA8
                    CD7      14      R       CA7
                    CD6      15      S       CA6
                    CD5      16      T       CA5
                    CD4      17      U       CA4
                    CD3      18      V       CA3
                    CD2      19      W       CA2
                    CD1      20      X       CA1
                    CD0      21      Y       CA0
                     P2      22      Z       GND

**********************************************************************/

#pragma once

#ifndef __VIC10_EXPANSION_SLOT__
#define __VIC10_EXPANSION_SLOT__

#include "emu.h"
#include "formats/cbm_crt.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define VIC10_EXPANSION_SLOT_TAG        "exp"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VIC10_EXPANSION_SLOT_ADD(_tag, _clock, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, VIC10_EXPANSION_SLOT, _clock) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)


#define MCFG_VIC10_EXPANSION_SLOT_IRQ_CALLBACK(_write) \
	devcb = &vic10_expansion_slot_device::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_VIC10_EXPANSION_SLOT_RES_CALLBACK(_write) \
	devcb = &vic10_expansion_slot_device::set_res_wr_callback(*device, DEVCB_##_write);

#define MCFG_VIC10_EXPANSION_SLOT_CNT_CALLBACK(_write) \
	devcb = &vic10_expansion_slot_device::set_cnt_wr_callback(*device, DEVCB_##_write);

#define MCFG_VIC10_EXPANSION_SLOT_SP_CALLBACK(_write) \
	devcb = &vic10_expansion_slot_device::set_sp_wr_callback(*device, DEVCB_##_write);


#define MCFG_VIC10_EXPANSION_SLOT_IRQ_CALLBACKS(_irq, _res) \
	downcast<vic10_expansion_slot_device *>(device)->set_irq_callbacks(DEVCB_##_irq, DEVCB_##_res);

#define MCFG_VIC10_EXPANSION_SLOT_SERIAL_CALLBACKS(_cnt, _sp) \
	downcast<vic10_expansion_slot_device *>(device)->set_serial_callbacks(DEVCB_##_cnt, DEVCB_##_sp);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> vic10_expansion_slot_device

class device_vic10_expansion_card_interface;

class vic10_expansion_slot_device : public device_t,
									public device_slot_interface,
									public device_image_interface
{
public:
	// construction/destruction
	vic10_expansion_slot_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<vic10_expansion_slot_device &>(device).m_write_irq.set_callback(object); }
	template<class _Object> static devcb_base &set_res_wr_callback(device_t &device, _Object object) { return downcast<vic10_expansion_slot_device &>(device).m_write_res.set_callback(object); }
	template<class _Object> static devcb_base &set_cnt_wr_callback(device_t &device, _Object object) { return downcast<vic10_expansion_slot_device &>(device).m_write_cnt.set_callback(object); }
	template<class _Object> static devcb_base &set_sp_wr_callback(device_t &device, _Object object) { return downcast<vic10_expansion_slot_device &>(device).m_write_sp.set_callback(object); }

	// computer interface
	UINT8 cd_r(address_space &space, offs_t offset, UINT8 data, int lorom, int uprom, int exram);
	void cd_w(address_space &space, offs_t offset, UINT8 data, int lorom, int uprom, int exram);
	DECLARE_READ_LINE_MEMBER( p0_r );
	DECLARE_WRITE_LINE_MEMBER( p0_w );

	// cartridge interface
	DECLARE_WRITE_LINE_MEMBER( irq_w ) { m_write_irq(state); }
	DECLARE_WRITE_LINE_MEMBER( res_w ) { m_write_res(state); }
	DECLARE_WRITE_LINE_MEMBER( cnt_w ) { m_write_cnt(state); }
	DECLARE_WRITE_LINE_MEMBER( sp_w ) { m_write_sp(state); }

protected:
	// device-level overrides
	virtual void device_config_complete() override { update_names(); }
	virtual void device_start() override;
	virtual void device_reset() override;

	// image-level overrides
	virtual bool call_load() override;
	virtual bool call_softlist_load(software_list_device &swlist, const char *swname, const rom_entry *start_entry) override;

	virtual iodevice_t image_type() const override { return IO_CARTSLOT; }

	virtual bool is_readable()  const override { return 1; }
	virtual bool is_writeable() const override { return 0; }
	virtual bool is_creatable() const override { return 0; }
	virtual bool must_be_loaded() const override { return 1; }
	virtual bool is_reset_on_load() const override { return 1; }
	virtual const char *image_interface() const override { return "vic10_cart"; }
	virtual const char *file_extensions() const override { return "80,e0"; }
	virtual const option_guide *create_option_guide() const override { return nullptr; }

	// slot interface overrides
	virtual std::string get_default_card_software() override;

	devcb_write_line   m_write_irq;
	devcb_write_line   m_write_res;
	devcb_write_line   m_write_cnt;
	devcb_write_line   m_write_sp;

	device_vic10_expansion_card_interface *m_card;
};


// ======================> device_vic10_expansion_card_interface

// class representing interface-specific live vic10_expansion card
class device_vic10_expansion_card_interface : public device_slot_card_interface
{
	friend class vic10_expansion_slot_device;

public:
	// construction/destruction
	device_vic10_expansion_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_vic10_expansion_card_interface();

	virtual UINT8 vic10_cd_r(address_space &space, offs_t offset, UINT8 data, int lorom, int uprom, int exram) { return data; };
	virtual void vic10_cd_w(address_space &space, offs_t offset, UINT8 data, int lorom, int uprom, int exram) { };
	virtual int vic10_p0_r() { return 0; };
	virtual void vic10_p0_w(int state) { };
	virtual void vic10_sp_w(int state) { };
	virtual void vic10_cnt_w(int state) { };

protected:
	optional_shared_ptr<UINT8> m_lorom;
	optional_shared_ptr<UINT8> m_exram;
	optional_shared_ptr<UINT8> m_uprom;

	vic10_expansion_slot_device *m_slot;
};


// device type definition
extern const device_type VIC10_EXPANSION_SLOT;


SLOT_INTERFACE_EXTERN( vic10_expansion_cards );



#endif
