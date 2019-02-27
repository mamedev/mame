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

#ifndef MAME_BUS_VME_VME_H
#define MAME_BUS_VME_VME_H

#pragma once


//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define VME_BUS_TAG        "vme"

// Callbacks to the board from the VME bus comes through here
#define MCFG_VME_J1_CB(_devcb) \
	downcast<vme_slot_device &>(*device).set_vme_j1_callback(DEVCB_##_devcb);

//void vme_slot1(device_slot_interface &device); // Disabled until we know how to combine a board driver and a slot device.
void vme_slots(device_slot_interface &device);

class device_vme_card_interface; // This interface is standardized

class vme_slot_device : public device_t,
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
	vme_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_vme_j1_callback(Object &&cb)  { return m_vme_j1_callback.set_callback(std::forward<Object>(cb)); }

	void set_vme_slot(const char *tag, const char *slottag);
	void update_vme_chains(uint32_t slot_nbr);

	virtual DECLARE_READ8_MEMBER(read8);
	virtual DECLARE_WRITE8_MEMBER(write8);

protected:
	vme_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_config_complete() override;

	// configuration
	const char *m_vme_tag, *m_vme_slottag;

	// callbacks
	devcb_write_line        m_vme_j1_callback;
	device_vme_card_interface *m_card;
};

DECLARE_DEVICE_TYPE(VME, vme_device)

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_VME_DEVICE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, VME, 0)

#define MCFG_VME_CPU(_cputag) \
	downcast<vme_device &>(*device).set_cputag(_cputag);

#define MCFG_VME_BUS_OWNER_SPACES() \
	downcast<vme_device &>(*device).use_owner_spaces();

class vme_card_interface;

class vme_device : public device_t,
	public device_memory_interface
{
public:
	vme_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~vme_device();

	// inline configuration
	void set_cputag(const char *tag) { m_cputag = tag; }
	void use_owner_spaces();

	virtual space_config_vector memory_space_config() const override;

	const address_space_config m_a32_config;

	void add_vme_card(device_vme_card_interface *card);

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

	enum vme_amod_t
	{   // Defined and User Defined Address Modifier Values (long bnames from VME standard text. please use short)
		AMOD_EXTENDED_NON_PRIV_DATA = 0x09, //A32 SC (Single Cycle)
		A32_SC                      = 0x09, //A32 SC (Single Cycle)
		AMOD_EXTENDED_NON_PRIV_PRG  = 0x0A,
		AMOD_EXTENDED_NON_PRIV_BLK  = 0x0B,
		AMOD_EXTENDED_SUPERVIS_DATA = 0x0D,
		AMOD_EXTENDED_SUPERVIS_PRG  = 0x0E,
		AMOD_EXTENDED_SUPERVIS_BLK  = 0x0F,
		AMOD_USER_DEFINED_FIRST     = 0x10,
		AMOD_USER_DEFINED_LAST      = 0x1F,
		AMOD_SHORT_NON_PRIV_ACCESS  = 0x29, //A16 SC
		A16_SC                      = 0x29, //A16 SC
		AMOD_SHORT_SUPERVIS_ACCESS  = 0x2D,
		AMOD_STANDARD_NON_PRIV_DATA = 0x39, //A24 SC
		A24_SC                      = 0x39, //A24 SC
		AMOD_STANDARD_NON_PRIV_PRG  = 0x3A,
		AMOD_STANDARD_NON_PRIV_BLK  = 0x3B, //A24 BLT
		AMOD_STANDARD_SUPERVIS_DATA = 0x3D,
		AMOD_STANDARD_SUPERVIS_PRG  = 0x3E,
		AMOD_STANDARD_SUPERVIS_BLK  = 0x3F
	};
	void install_device(vme_amod_t amod, offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler, uint32_t mask);
	//  void install_device(vme_amod_t amod, offs_t start, offs_t end, read8_delegate rhandler, write8_delegate whandler);
	void install_device(vme_amod_t amod, offs_t start, offs_t end, read16_delegate rhandler, write16_delegate whandler, uint32_t mask);
	void install_device(vme_amod_t amod, offs_t start, offs_t end, read32_delegate rhandler, write32_delegate whandler, uint32_t mask);

protected:
	vme_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	simple_list<device_vme_card_interface> m_device_list;

	// internal state
	cpu_device   *m_maincpu;

	// address spaces
	address_space *m_prgspace;
	int m_prgwidth;
	bool m_allocspaces;

	const char                 *m_cputag;

};



// device type definition
DECLARE_DEVICE_TYPE(VME_SLOT, vme_slot_device)

class device_vme_card_interface : public device_slot_card_interface
{
	template <class ElementType> friend class simple_list;
public:
	// inline configuration
	void set_vme_tag(const char *tag, const char *slottag);

	// construction/destruction
	virtual ~device_vme_card_interface();
	void set_vme_device();

	virtual DECLARE_READ8_MEMBER(read8);
	virtual DECLARE_WRITE8_MEMBER(write8);

	device_vme_card_interface(const machine_config &mconfig, device_t &device);

protected:
	device_t *m_device;

	vme_device  *m_vme;
	const char *m_vme_tag, *m_vme_slottag;
	int m_slot;

private:
	device_vme_card_interface *m_next;
};

#define MCFG_VME_SLOT_ADD(_tag, _slotnbr, _slot_intf,_def_slot)            \
	{   std::string stag = "slot" + std::to_string(_slotnbr);              \
		MCFG_DEVICE_ADD(stag.c_str(), VME_SLOT, 0);                        \
		MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, false);          \
		downcast<vme_slot_device &>(*device).set_vme_slot(_tag, stag.c_str()); \
		downcast<vme_slot_device &>(*device).update_vme_chains(_slotnbr);      \
	}

#endif // MAME_BUS_VME_VME_H
