// license:BSD-3-Clause
// copyright-holders: Joakim Larsson Edstrom

/*
 * vme.h
 *
 * VME bus system
 *
 * Pinout: (from http://pinouts.ru/Slots/vmebus_pinout.shtml)

     P1/J1                                   P2/J2 (optional for 32 bit)
    +-A-B-C--+  A        B         C        +-A-B-C--+  A     B        C
 01 | [][][] | D00      BBSY*     D08       | [][][] | n/a   +5v      n/a
 02 | [][][] | D01      BCLR*     D09       | [][][] | n/a   GROUND   n/a
 03 | [][][] | D02      ACFAIL*   D10       | [][][] | n/a   RESERVED n/a
 04 | [][][] | D03      BG0IN*    D11       | [][][] | n/a   A24      n/a
 05 | [][][] | D04      BG0OUT*   D12       | [][][] | n/a   A25      n/a
 06 | [][][] | D05      BG1IN*    D13       | [][][] | n/a   A26      n/a
 07 | [][][] | D06      BG1OUT*   D14       | [][][] | n/a   A27      n/a
 08 | [][][] | D07      BG2IN*    D15       | [][][] | n/a   A28      n/a
 09 | [][][] | GROUND   BG2OUT*   GROUND    | [][][] | n/a   A29      n/a
 10 | [][][] | SYSCLK   BG3IN*    SYSFAIL*  | [][][] | n/a   A30      n/a
 11 | [][][] | GROUND   BG3OUT*   BERR*     | [][][] | n/a   A31      n/a
 12 | [][][] | DS1*     BR0*      SYSRESET* | [][][] | n/a   GROUND   n/a
 13 | [][][] | DS0*     BR1*      LWORD*    | [][][] | n/a   +5v      n/a
 14 | [][][] | WRITE*   BR2*      AM5       | [][][] | n/a   D16      n/a
 15 | [][][] | GROUND   BR3*      A23       | [][][] | n/a   D17      n/a
 16 | [][][] | DTACK*   AM0       A22       | [][][] | n/a   D18      n/a
 17 | [][][] | GROUND   AM1       A21       | [][][] | n/a   D19      n/a
 18 | [][][] | AS*      AM2       A20       | [][][] | n/a   D20      n/a
 19 | [][][] | GROUND   AM3       A19       | [][][] | n/a   D21      n/a
 20 | [][][] | IACK*    GROUND    A18       | [][][] | n/a   D22      n/a
 21 | [][][] | IACKIN*  SERCLK*   A17       | [][][] | n/a   D23      n/a
 22 | [][][] | IACKOUT* SERDAT*   A16       | [][][] | n/a   GROUND   n/a
 23 | [][][] | AM4      GROUND    A15       | [][][] | n/a   D24      n/a
 24 | [][][] | A07      IRQ7*     A14       | [][][] | n/a   D25      n/a
 25 | [][][] | A06      IRQ6*     A13       | [][][] | n/a   D26      n/a
 26 | [][][] | A05      IRQ5*     A12       | [][][] | n/a   D27      n/a
 27 | [][][] | A04      IRQ4*     A11       | [][][] | n/a   D28      n/a
 28 | [][][] | A03      IRQ3*     A10       | [][][] | n/a   D29      n/a
 29 | [][][] | A02      IRQ2*     A09       | [][][] | n/a   D30      n/a
 30 | [][][] | A01      IRQ1*     A08       | [][][] | n/a   D31      n/a
 31 | [][][] | -12v     +5v STDBY +12v      | [][][] | n/a   GROUND   n/a
 32 | [][][] | +5v      +5v       +5v       | [][][] | n/a   +5v      n/a

 */

#ifndef VME_H_
#define VME_H_

#pragma once

#include "emu.h"

//**************************************************************************
//	CONSTANTS
//**************************************************************************

#define VME_BUS_TAG        "vme"

// Callbacks to the board from the VME bus comes through here
#define MCFG_VME_J1_CB(_devcb) \
	devcb = &vme_p1_slot_device::static_set_vme_j1_callback(*device, DEVCB_##_devcb);

SLOT_INTERFACE_EXTERN(vme_p1_slot1);
SLOT_INTERFACE_EXTERN(vme_p1_slots);

class device_vme_p1_card_interface; // This interface is standardized

class vme_p1_slot_device : public device_t,
	public device_slot_interface
{
public:
	// VME BUS signals driven to or drived by the VME bus
	enum class control
	{
		AS,
		DS0,
		DS1,
		BERR,
	    DTACK,
		WRITE
	};

	enum class address
	{
		DS0,
		DS1,
		LWORD
	};

	// construction/destruction
	vme_p1_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	vme_p1_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	template<class _Object> static devcb_base &static_set_vme_j1_callback(device_t &device, _Object object)  { return downcast<vme_p1_slot_device &>(device).m_vme_j1_callback.set_callback(object); }

	// device-level overrides
	virtual void device_start() override;
	virtual void device_config_complete() override;

	static void static_set_vme_p1_slot(device_t &device, const char *tag, const char *slottag);
	// configuration
	const char *m_vme_p1_tag, *m_vme_p1_slottag;

	virtual DECLARE_READ8_MEMBER(read8);
	virtual DECLARE_WRITE8_MEMBER(write8);

	// callbacks
	devcb_write_line        m_vme_j1_callback;
	device_vme_p1_card_interface *m_card;
private:
};

extern const device_type VME_P1;

#define MCFG_VME_P1_DEVICE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, VME_P1, 0)

class vme_p1_card_interface;

class vme_p1_device : public device_t
{
public:
	vme_p1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	vme_p1_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	~vme_p1_device();

	void add_vme_p1_card(device_vme_p1_card_interface *card);
	void install_device(offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler, uint32_t mask);
	void install_device(offs_t start, offs_t end, read16_delegate rhandler, write16_delegate whandler, uint32_t mask);
	void install_device(offs_t start, offs_t end, read32_delegate rhandler, write32_delegate whandler, uint32_t mask);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	simple_list<device_vme_p1_card_interface> m_device_list;
};



// device type definition
extern const device_type VME_P1_SLOT;

class device_vme_p1_card_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_vme_p1_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_vme_p1_card_interface();
	void set_vme_p1_device();

	virtual DECLARE_READ8_MEMBER(read8);
	virtual DECLARE_WRITE8_MEMBER(write8);	
	device_t *m_device;

	// inline configuration
	static void static_set_vme_p1_tag(device_t &device, const char *tag, const char *slottag);
	vme_p1_device  *m_vme_p1;
	const char *m_vme_p1_tag, *m_vme_p1_slottag;
	int m_slot;
	device_vme_p1_card_interface *m_next;

	//
	// Address Modifiers
	//
	/* There are 6 address modifier lines. They allow the MASTER to pass additional binary
	   information to the SLAVE during data transfers. Table 2-3 lists all of the 64 possible
	   address modifier (AM) codes and classifies each into one of three categories:
	   - Defined
	   - Reserved
	   - User defined
	   The defined address modifier codes can be further classified into three categories:
	   Short addressing AM codes indicate that address lines A02-A15 are being used to select a BYTE(0-3) group.
	   Standard addressing AM codes ,indicate that address lines A02-A23 are being used to select a BYTE(0-3) group.
	   Extended addressing AM codes indicate that address lines A02-A31 are being used to select a BYTE(0-3) group.*/
	enum 
	{   // Defined and User Defined Address Modifier Values, The rest us Reserved between 0x00 and 0x3F
		AMOD_EXTENDED_NON_PRIV_DATA = 0x09,
		AMOD_EXTENDED_NON_PRIV_PRG  = 0x0A,
		AMOD_EXTENDED_NON_PRIV_BLK  = 0x0B,
		AMOD_EXTENDED_SUPERVIS_DATA = 0x0D,
		AMOD_EXTENDED_SUPERVIS_PRG  = 0x0E,
		AMOD_EXTENDED_SUPERVIS_BLK  = 0x0F,
		AMOD_USER_DEFINED_FIRST     = 0x10,
		AMOD_USER_DEFINED_LAST      = 0x1F,
		AMOD_SHORT_NON_PRIV_ACCESS  = 0x29,
		AMOD_SHORT_SUPERVIS_ACCESS  = 0x2D,
		AMOD_STANDARD_NON_PRIV_DATA = 0x39,
		AMOD_STANDARD_NON_PRIV_PRG  = 0x3A,
		AMOD_STANDARD_NON_PRIV_BLK  = 0x3B,
		AMOD_STANDARD_SUPERVIS_DATA = 0x3D,
		AMOD_STANDARD_SUPERVIS_PRG  = 0x3E,
		AMOD_STANDARD_SUPERVIS_BLK  = 0x3F
	};
};

SLOT_INTERFACE_EXTERN(vme_p1_slot1);

#define MCFG_VME_P1_SLOT_ADD(_tag, _slot_tag, _slot_intf,_def_slot)	\
	MCFG_DEVICE_ADD(_slot_tag, VME_P1_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false) \
	vme_p1_slot_device::static_set_vme_p1_slot(*device, _tag, _slot_tag);

#define MCFG_VME_P1_SLOT_REMOVE(_tag)        \
	MCFG_DEVICE_REMOVE(_tag)


#if 0
// Callbacks to the board from the VME bus comes through here
#define MCFG_VME_J2_CB(_devcb) \
	devcb = &vme_p2_slot_device::static_set_j2_callback(*device, DEVCB_##_devcb);

class device_vme_p2_interface; // This interface often has custom/propritary A and C rows

class vme_p2_slot_device : public device_t,
	public device_slot_interface
{
public:

	// construction/destruction
	vme_p2_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template<class _Object> static devcb_base &static_set_vme_j2_callback(device_t &device, _Object object)  { return downcast<vme_p2_slot_device &>(device).m_vme_j2_callback.set_callback(object); }

	// device-level overrides
	virtual void device_start() override;

	// callbacks
	devcb_write_line        m_vme_j2_callback;
private:
	device_vme_p2_interface *m_vme_p2;
};

// device type definition
extern const device_type VME_P2_SLOT;

class device_vme_p2_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_vme_p2_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_vme_p2_interface();

	virtual DECLARE_READ32_MEMBER(read32);
	virtual DECLARE_WRITE32_MEMBER(write32);
};

#define MCFG_VME_P2_ADD(_tag,_slot_intf,_def_slot) \
	MCFG_DEVICE_ADD(_tag, VME_P2_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false)

#define MCFG_VME_P2_REMOVE(_tag)        \
	MCFG_DEVICE_REMOVE(_tag)
#endif
#if 0
//**************************************************************************
//	INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VME16_SLOT_ADD(_vmetag, _tag, _slot_intf, _def_slot, _fixed) \
	MCFG_DEVICE_ADD(_tag, VME16_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _fixed) \
	vme16_slot_device::static_set_vme16_slot(*device, _vmetag, _tag);

//**************************************************************************
//	TYPE DEFINITIONS
//**************************************************************************

class device_vme16_card_interface;

// 
// The VME device
//
class vme_device : public device_t
,public device_memory_interface
{
public:
	// construction/destruction
	vme_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	vme_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	// inline configuration
	static void static_set_cputag(device_t &device, const char *tag);

	void install_device(offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_delegate rhandler, write8_delegate whandler);
	void install_device(offs_t start, offs_t end, offs_t mask, offs_t mirror, read16_delegate rhandler, write16_delegate whandler);
	void install_device(offs_t start, offs_t end, offs_t mask, offs_t mirror, read32_delegate rhandler, write32_delegate whandler);
	void install_bank(offs_t start, offs_t end, offs_t mask, offs_t mirror, const char *tag, uint8_t *data);
	void unmap_bank(offs_t start, offs_t end, offs_t mask, offs_t mirror);
	void install_rom(device_t *dev, offs_t start, offs_t end, offs_t mask, offs_t mirror, const char *tag, const char *region);
	void install_memory(offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_delegate rhandler, write8_delegate whandler);
	void set_irq_line(int slot, int state);
protected:
	void install_space(address_spacenum spacenum, offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_delegate rhandler, write8_delegate whandler);

	// address spaces
	address_space *m_prgspace;
	int  m_prgwidth;
	bool m_allocspaces;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// internal state
	cpu_device   *m_maincpu;
	const address_space_config m_program16_config;
	const address_space_config m_program24_config;
	const address_space_config m_program32_config;
};

class vme16_device : public vme_device
{
 public:
	vme16_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	vme16_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	virtual const address_space_config *memory_space_config(address_spacenum spacenum) const override;

	DECLARE_READ16_MEMBER ( vme_r );
	DECLARE_WRITE16_MEMBER( vme_w );

	void add_vme16_card(device_vme16_card_interface *card);
};

class vme24_device : public vme_device
{
 public:
	vme24_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	vme24_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	virtual const address_space_config *memory_space_config(address_spacenum spacenum) const override;

	DECLARE_READ16_MEMBER ( vme_r );
	DECLARE_WRITE16_MEMBER( vme_w );

	void add_vme24_card(device_vme16_card_interface *card);
};

class vme32_device : public vme_device
{
 public:
	vme32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	vme32_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	virtual const address_space_config *memory_space_config(address_spacenum spacenum) const override;

	DECLARE_READ32_MEMBER ( vme_r );
	DECLARE_WRITE32_MEMBER( vme_w );

	//	void add_vme32_card(device_vme16_card_interface *card);
};

//
// The SLOT device
//
class vme16_slot_device : public device_t,
	public device_slot_interface
{
public:
	// construction/destruction
	vme16_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	vme16_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	virtual ~vme16_slot_device();

	//template<class _Object> static devcb_base &set_out_irq2_callback(device_t &device, _Object object) { return downcast<vme16_slot_device &>(device).m_out_irq2_cb.set_callback(object); }

	//DECLARE_WRITE_LINE_MEMBER( irq2_w );

	// inline configuration
	static void static_set_vme16_slot(device_t &device, const char *tag, const char *slottag);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

protected:

	// configuration
	const char *m_vme16_tag, *m_vme16_slottag;

	device_vme16_card_interface *m_slot;
};


// device type definition
extern const device_type VME16_SLOT;
extern const device_type VME16;
extern const device_type VME24;
extern const device_type VME32;

//
// The CARD device
//
class device_vme16_card_interface : public device_slot_card_interface
{
	friend class vme16_device;
public:
	// construction/destruction
	device_vme16_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_vme16_card_interface();

	device_vme16_card_interface *next() const { return m_next; }

	void set_vme_device();

	// inline configuration
	static void static_set_vmebus(device_t &device, device_t *vme_device);
public:
	vme16_device  *m_vme;
	device_t     *m_vme_dev;
	device_vme16_card_interface *m_next;
};

#if 0
	// reset
	virtual void vme16_reset_w() { };

};
#endif
#endif


#endif /* VME_H_ */
