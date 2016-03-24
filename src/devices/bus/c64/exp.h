// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore 64 Expansion Port emulation

**********************************************************************

                    GND       1      A       GND
                    +5V       2      B       _ROMH
                    +5V       3      C       _RESET
                   _IRQ       4      D       _NMI
                  _CR/W       5      E       Sphi2
                 DOTCLK       6      F       CA15
                  _I/O1       7      H       CA14
                  _GAME       8      J       CA13
                 _EXROM       9      K       CA12
                  _I/O2      10      L       CA11
                  _ROML      11      M       CA10
                     BA      12      N       CA9
                   _DMA      13      P       CA8
                    CD7      14      R       CA7
                    CD6      15      S       CA6
                    CD5      16      T       CA5
                    CD4      17      U       CA4
                    CD3      18      V       CA3
                    CD2      19      W       CA2
                    CD1      20      X       CA1
                    CD0      21      Y       CA0
                    GND      22      Z       GND

**********************************************************************/

#pragma once

#ifndef __C64_EXPANSION_SLOT__
#define __C64_EXPANSION_SLOT__

#include "emu.h"
#include "formats/cbm_crt.h"



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define C64_EXPANSION_SLOT_TAG      "exp"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_C64_EXPANSION_SLOT_ADD(_tag, _clock, _slot_intf, _def_slot) \
	MCFG_DEVICE_ADD(_tag, C64_EXPANSION_SLOT, _clock) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_C64_PASSTHRU_EXPANSION_SLOT_ADD() \
	MCFG_C64_EXPANSION_SLOT_ADD(C64_EXPANSION_SLOT_TAG, 0, c64_expansion_cards, NULL) \
	MCFG_C64_EXPANSION_SLOT_IRQ_CALLBACK(DEVWRITELINE(DEVICE_SELF_OWNER, c64_expansion_slot_device, irq_w)) \
	MCFG_C64_EXPANSION_SLOT_NMI_CALLBACK(DEVWRITELINE(DEVICE_SELF_OWNER, c64_expansion_slot_device, nmi_w)) \
	MCFG_C64_EXPANSION_SLOT_RESET_CALLBACK(DEVWRITELINE(DEVICE_SELF_OWNER, c64_expansion_slot_device, reset_w)) \
	MCFG_C64_EXPANSION_SLOT_CD_INPUT_CALLBACK(DEVREAD8(DEVICE_SELF_OWNER, c64_expansion_slot_device, dma_cd_r)) \
	MCFG_C64_EXPANSION_SLOT_CD_OUTPUT_CALLBACK(DEVWRITE8(DEVICE_SELF_OWNER, c64_expansion_slot_device, dma_cd_w)) \
	MCFG_C64_EXPANSION_SLOT_DMA_CALLBACK(DEVWRITELINE(DEVICE_SELF_OWNER, c64_expansion_slot_device, dma_w))


#define MCFG_C64_EXPANSION_SLOT_IRQ_CALLBACK(_write) \
	devcb = &c64_expansion_slot_device::set_irq_wr_callback(*device, DEVCB_##_write);

#define MCFG_C64_EXPANSION_SLOT_NMI_CALLBACK(_write) \
	devcb = &c64_expansion_slot_device::set_nmi_wr_callback(*device, DEVCB_##_write);

#define MCFG_C64_EXPANSION_SLOT_RESET_CALLBACK(_write) \
	devcb = &c64_expansion_slot_device::set_reset_wr_callback(*device, DEVCB_##_write);

#define MCFG_C64_EXPANSION_SLOT_CD_INPUT_CALLBACK(_read) \
	devcb = &c64_expansion_slot_device::set_cd_rd_callback(*device, DEVCB_##_read);

#define MCFG_C64_EXPANSION_SLOT_CD_OUTPUT_CALLBACK(_write) \
	devcb = &c64_expansion_slot_device::set_cd_wr_callback(*device, DEVCB_##_write);

#define MCFG_C64_EXPANSION_SLOT_DMA_CALLBACK(_write) \
	devcb = &c64_expansion_slot_device::set_dma_wr_callback(*device, DEVCB_##_write);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_expansion_slot_device

class device_c64_expansion_card_interface;

class c64_expansion_slot_device : public device_t,
									public device_slot_interface,
									public device_image_interface
{
public:
	// construction/destruction
	c64_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_irq_wr_callback(device_t &device, _Object object) { return downcast<c64_expansion_slot_device &>(device).m_write_irq.set_callback(object); }
	template<class _Object> static devcb_base &set_nmi_wr_callback(device_t &device, _Object object) { return downcast<c64_expansion_slot_device &>(device).m_write_nmi.set_callback(object); }
	template<class _Object> static devcb_base &set_reset_wr_callback(device_t &device, _Object object) { return downcast<c64_expansion_slot_device &>(device).m_write_reset.set_callback(object); }
	template<class _Object> static devcb_base &set_cd_rd_callback(device_t &device, _Object object) { return downcast<c64_expansion_slot_device &>(device).m_read_dma_cd.set_callback(object); }
	template<class _Object> static devcb_base &set_cd_wr_callback(device_t &device, _Object object) { return downcast<c64_expansion_slot_device &>(device).m_write_dma_cd.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_wr_callback(device_t &device, _Object object) { return downcast<c64_expansion_slot_device &>(device).m_write_dma.set_callback(object); }

	// computer interface
	UINT8 cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2);
	void cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2);
	int game_r(offs_t offset, int sphi2, int ba, int rw, int hiram);
	int exrom_r(offs_t offset, int sphi2, int ba, int rw, int hiram);

	// cartridge interface
	DECLARE_READ8_MEMBER( dma_cd_r ) { return m_read_dma_cd(offset); }
	DECLARE_WRITE8_MEMBER( dma_cd_w ) { m_write_dma_cd(offset, data); }
	DECLARE_WRITE_LINE_MEMBER( irq_w ) { m_write_irq(state); }
	DECLARE_WRITE_LINE_MEMBER( nmi_w ) { m_write_nmi(state); }
	DECLARE_WRITE_LINE_MEMBER( dma_w ) { m_write_dma(state); }
	DECLARE_WRITE_LINE_MEMBER( reset_w ) { m_write_reset(state); }
	int phi2() { return clock(); }
	int dotclock() { return phi2() * 8; }
	int hiram() { return m_hiram; }

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
	virtual const char *image_interface() const override { return "c64_cart,vic10_cart"; }
	virtual const char *file_extensions() const override { return "80,a0,e0,crt"; }
	virtual const option_guide *create_option_guide() const override { return nullptr; }

	// slot interface overrides
	virtual std::string get_default_card_software() override;

	devcb_read8        m_read_dma_cd;
	devcb_write8       m_write_dma_cd;
	devcb_write_line   m_write_irq;
	devcb_write_line   m_write_nmi;
	devcb_write_line   m_write_dma;
	devcb_write_line   m_write_reset;

	device_c64_expansion_card_interface *m_card;

	int m_hiram;
};


// ======================> device_c64_expansion_card_interface

class device_c64_expansion_card_interface : public device_slot_card_interface
{
	friend class c64_expansion_slot_device;

public:
	// construction/destruction
	device_c64_expansion_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_c64_expansion_card_interface();

	virtual UINT8 c64_cd_r(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) { return data; };
	virtual void c64_cd_w(address_space &space, offs_t offset, UINT8 data, int sphi2, int ba, int roml, int romh, int io1, int io2) { };
	virtual int c64_game_r(offs_t offset, int sphi2, int ba, int rw) { return m_game; }
	virtual int c64_exrom_r(offs_t offset, int sphi2, int ba, int rw) { return m_exrom; }

protected:
	optional_shared_ptr<UINT8> m_roml;
	optional_shared_ptr<UINT8> m_romh;
	optional_shared_ptr<UINT8> m_nvram;

	int m_game;
	int m_exrom;

	c64_expansion_slot_device *m_slot;
};


// device type definition
extern const device_type C64_EXPANSION_SLOT;

SLOT_INTERFACE_EXTERN( c64_expansion_cards );



#endif
