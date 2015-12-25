// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore Plus/4 Expansion Port emulation

**********************************************************************

                    GND       1      A       GND
                    +5V       2      B       C1 LOW
                    +5V       3      C       _BRESET
                   _IRQ       4      D       _RAS
                   R/_W       5      E       phi0
                C1 HIGH       6      F       A15
                 C2 LOW       7      H       A14
                C2 HIGH       8      J       A13
                   _CS1       9      K       A12
                   _CS0      10      L       A11
                   _CAS      11      M       A10
                    MUX      12      N       A9
                     BA      13      P       A8
                     D7      14      R       A7
                     D6      15      S       A6
                     D5      16      T       A5
                     D4      17      U       A4
                     D3      18      V       A3
                     D2      19      W       A2
                     D1      20      X       A1
                     D0      21      Y       A0
                    AEC      22      Z       N.C. (RAMEN)
              EXT AUDIO      23      AA      N.C.
                   phi2      24      BB      N.C.
                    GND      25      CC      GND

**********************************************************************/

#pragma once

#ifndef __PLUS4_EXPANSION_SLOT__
#define __PLUS4_EXPANSION_SLOT__

#include "emu.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define PLUS4_EXPANSION_SLOT_TAG        "exp"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_PLUS4_EXPANSION_SLOT_ADD(_tag, _clock, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, PLUS4_EXPANSION_SLOT, _clock) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_PLUS4_PASSTHRU_EXPANSION_SLOT_ADD() \
	MCFG_PLUS4_EXPANSION_SLOT_ADD(PLUS4_EXPANSION_SLOT_TAG, 0, plus4_expansion_cards, NULL) \
	MCFG_PLUS4_EXPANSION_SLOT_IRQ_CALLBACK(DEVWRITELINE(DEVICE_SELF_OWNER, plus4_expansion_slot_device, irq_w)) \
	MCFG_PLUS4_EXPANSION_SLOT_CD_INPUT_CALLBACK(DEVREAD8(DEVICE_SELF_OWNER, plus4_expansion_slot_device, dma_cd_r)) \
	MCFG_PLUS4_EXPANSION_SLOT_CD_OUTPUT_CALLBACK(DEVWRITE8(DEVICE_SELF_OWNER, plus4_expansion_slot_device, dma_cd_w)) \
	MCFG_PLUS4_EXPANSION_SLOT_AEC_CALLBACK(DEVWRITELINE(DEVICE_SELF_OWNER, plus4_expansion_slot_device, aec_w))


#define MCFG_PLUS4_EXPANSION_SLOT_IRQ_CALLBACK(_write) \
	devcb = &plus4_expansion_slot_device::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_PLUS4_EXPANSION_SLOT_CD_INPUT_CALLBACK(_read) \
	devcb = &plus4_expansion_slot_device::set_cd_rd_callback(*device, DEVCB_##_read);

#define MCFG_PLUS4_EXPANSION_SLOT_CD_OUTPUT_CALLBACK(_write) \
	devcb = &plus4_expansion_slot_device::set_cd_wr_callback(*device, DEVCB_##_write);

#define MCFG_PLUS4_EXPANSION_SLOT_AEC_CALLBACK(_write) \
	devcb = &plus4_expansion_slot_device::set_aec_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> plus4_expansion_slot_device

class device_plus4_expansion_card_interface;

class plus4_expansion_slot_device : public device_t,
									public device_slot_interface,
									public device_image_interface
{
public:
	// construction/destruction
	plus4_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<plus4_expansion_slot_device &>(device).m_write_irq.set_callback(object); }
	template<class _Object> static devcb_base &set_cd_rd_callback(device_t &device, _Object object) { return downcast<plus4_expansion_slot_device &>(device).m_read_dma_cd.set_callback(object); }
	template<class _Object> static devcb_base &set_cd_wr_callback(device_t &device, _Object object) { return downcast<plus4_expansion_slot_device &>(device).m_write_dma_cd.set_callback(object); }
	template<class _Object> static devcb_base &set_aec_wr_callback(device_t &device, _Object object) { return downcast<plus4_expansion_slot_device &>(device).m_write_aec.set_callback(object); }

	// computer interface
	UINT8 cd_r(address_space &space, offs_t offset, UINT8 data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h);
	void cd_w(address_space &space, offs_t offset, UINT8 data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h);

	// cartridge interface
	DECLARE_READ8_MEMBER( dma_cd_r ) { return m_read_dma_cd(offset); }
	DECLARE_WRITE8_MEMBER( dma_cd_w ) { m_write_dma_cd(offset, data); }
	DECLARE_WRITE_LINE_MEMBER( irq_w ) { m_write_irq(state); }
	DECLARE_WRITE_LINE_MEMBER( aec_w ) { m_write_aec(state); }
	int phi2() { return clock(); }

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
	virtual bool must_be_loaded() const override { return 0; }
	virtual bool is_reset_on_load() const override { return 1; }
	virtual const char *image_interface() const override { return "plus4_cart"; }
	virtual const char *file_extensions() const override { return "rom,bin"; }
	virtual const option_guide *create_option_guide() const override { return nullptr; }

	// slot interface overrides
	virtual void get_default_card_software(std::string &result) override;

	devcb_write_line   m_write_irq;
	devcb_read8        m_read_dma_cd;
	devcb_write8       m_write_dma_cd;
	devcb_write_line   m_write_aec;

	device_plus4_expansion_card_interface *m_card;
};


// ======================> device_plus4_expansion_card_interface

class device_plus4_expansion_card_interface : public device_slot_card_interface
{
	friend class plus4_expansion_slot_device;

public:
	// construction/destruction
	device_plus4_expansion_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_plus4_expansion_card_interface();

	// runtime
	virtual UINT8 plus4_cd_r(address_space &space, offs_t offset, UINT8 data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h) { return data; };
	virtual void plus4_cd_w(address_space &space, offs_t offset, UINT8 data, int ba, int cs0, int c1l, int c2l, int cs1, int c1h, int c2h) { };

protected:
	optional_shared_ptr<UINT8> m_c1l;
	optional_shared_ptr<UINT8> m_c1h;
	optional_shared_ptr<UINT8> m_c2l;
	optional_shared_ptr<UINT8> m_c2h;

	size_t m_c1l_mask;
	size_t m_c1h_mask;
	size_t m_c2l_mask;
	size_t m_c2h_mask;

	plus4_expansion_slot_device *m_slot;
};


// device type definition
extern const device_type PLUS4_EXPANSION_SLOT;


SLOT_INTERFACE_EXTERN( plus4_expansion_cards );



#endif
