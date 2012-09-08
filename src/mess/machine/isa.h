/***************************************************************************

        ISA bus device


    8 bit ISA bus connector:

    A1    O  I/O CH CHK    B1       GND
    A2   IO  D7            B2   I   RESET
    A3   IO  D6            B3       +5V
    A4   IO  D5            B4    O  IRQ2
    A5   IO  D4            B5       -5V
    A6   IO  D3            B6    O  DRQ2
    A7   IO  D2            B7       -12V
    A8   IO  D1            B8    O  /NOWS
    A9   IO  D0            B9       +12V
    A10   O  I/O CH RDY    B10      GND
    A11  I   AEN           B11  I   /SMEMW
    A12  IO  A19           B12  I   /SMEMR
    A13  IO  A18           B13  I   /IOW
    A14  IO  A17           B14  I   /IOR
    A15  IO  A16           B15  I   /DACK3
    A16  IO  A15           B16   O  DRQ3
    A17  IO  A14           B17  I   /DACK1
    A18  IO  A13           B18   O  DRQ1
    A19  IO  A12           B19  IO  /REFRESH
    A20  IO  A11           B20  I   CLOCK
    A21  IO  A10           B21   O  IRQ7
    A22  IO  A9            B22   O  IRQ6
    A23  IO  A8            B23   O  IRQ5
    A24  IO  A7            B24   O  IRQ4
    A25  IO  A6            B25   O  IRQ3
    A26  IO  A5            B26  I   /DACK2
    A27  IO  A4            B27  I   T/C
    A28  IO  A3            B28  I   ALE
    A29  IO  A2            B29      +5V
    A30  IO  A1            B30  I   OSC
    A31  IO  A0            B31      GND

    16 bit ISA bus extension

    C1   I   SBHE          D1   I   /MEM CS 16
    C2   IO  A23           D2   I   /I/O CS 16
    C3   IO  A22           D3    O  IRQ10
    C4   IO  A21           D4    O  IRQ11
    C5   IO  A20           D5    O  IRQ12
    C6   IO  A19           D6    O  IRQ15
    C7   IO  A18           D7    O  IRQ14
    C8   IO  A17           D8   I   /DACK0
    C9   I   /MEMR         D9    O  DRQ0
    C10  I   /MEMW         D10  I   /DACK5
    C11  IO  D8            D11   O  DRQ5
    C12  IO  D9            D12  I   /DACK6
    C13  IO  D10           D13   O  DRQ6
    C14  IO  D11           D14  I   /DACK7
    C15  IO  D12           D15   O  DRQ7
    C16  IO  D13           D16      +5V
    C17  IO  D14           D17  I   MASTER
    C18  IO  D15           D18      GND

***************************************************************************/

#pragma once

#ifndef __ISA_H__
#define __ISA_H__

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ISA8_BUS_ADD(_tag, _cputag, _config) \
    MCFG_DEVICE_ADD(_tag, ISA8, 0) \
    MCFG_DEVICE_CONFIG(_config) \
    isa8_device::static_set_cputag(*device, _cputag); \

#define MCFG_ISA8_SLOT_ADD(_isatag, _tag, _slot_intf, _def_slot, _def_inp, _fixed) \
    MCFG_DEVICE_ADD(_tag, ISA8_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, _fixed) \
	isa8_slot_device::static_set_isa8_slot(*device, owner, _isatag); \

#define MCFG_ISA16_BUS_ADD(_tag, _cputag, _config) \
    MCFG_DEVICE_ADD(_tag, ISA16, 0) \
    MCFG_DEVICE_CONFIG(_config) \
    isa8_device::static_set_cputag(*device, _cputag); \

#define MCFG_ISA16_SLOT_ADD(_isatag, _tag, _slot_intf, _def_slot, _def_inp, _fixed) \
    MCFG_DEVICE_ADD(_tag, ISA16_SLOT, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(_slot_intf, _def_slot, _def_inp, _fixed) \
	isa16_slot_device::static_set_isa16_slot(*device, owner, _isatag); \

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class isa8_device;

class isa8_slot_device : public device_t,
						 public device_slot_interface
{
public:
	// construction/destruction
	isa8_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	isa8_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start();

    // inline configuration
    static void static_set_isa8_slot(device_t &device, device_t *owner, const char *isa_tag);
protected:
	// configuration
	device_t *m_owner;
	const char *m_isa_tag;
};

// device type definition
extern const device_type ISA8_SLOT;

// ======================> isa8bus_interface

struct isa8bus_interface
{
    devcb_write_line	m_out_irq2_cb;
    devcb_write_line	m_out_irq3_cb;
    devcb_write_line	m_out_irq4_cb;
    devcb_write_line	m_out_irq5_cb;
    devcb_write_line	m_out_irq6_cb;
    devcb_write_line	m_out_irq7_cb;
    devcb_write_line	m_out_drq1_cb;
    devcb_write_line	m_out_drq2_cb;
    devcb_write_line	m_out_drq3_cb;
};

class device_isa8_card_interface;
// ======================> isa8_device
class isa8_device : public device_t,
                    public isa8bus_interface
{
public:
	// construction/destruction
	isa8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	isa8_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock);
	// inline configuration
	static void static_set_cputag(device_t &device, const char *tag);

	void install_device(device_t *dev, offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_device_func rhandler, const char* rhandler_name, write8_device_func whandler, const char *whandler_name);
	void install_device(offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_delegate rhandler, write8_delegate whandler);
	void install_device(offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_space_func rhandler, const char* rhandler_name, write8_space_func whandler, const char *whandler_name);
	void install_bank(offs_t start, offs_t end, offs_t mask, offs_t mirror, const char *tag, UINT8 *data);
	void install_rom(device_t *dev, offs_t start, offs_t end, offs_t mask, offs_t mirror, const char *tag, const char *region);
	void install_memory(device_t *dev, offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_device_func rhandler, const char* rhandler_name, write8_device_func whandler, const char *whandler_name);
	void install_memory(offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_delegate rhandler, write8_delegate whandler);
	void install_memory(offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_space_func rhandler, const char* rhandler_name, write8_space_func whandler, const char *whandler_name);

	void unmap_bank(offs_t start, offs_t end, offs_t mask, offs_t mirror);
	void unmap_rom(offs_t start, offs_t end, offs_t mask, offs_t mirror);
	bool is_option_rom_space_available(offs_t start, int size);

	DECLARE_WRITE_LINE_MEMBER( irq2_w );
	DECLARE_WRITE_LINE_MEMBER( irq3_w );
	DECLARE_WRITE_LINE_MEMBER( irq4_w );
	DECLARE_WRITE_LINE_MEMBER( irq5_w );
	DECLARE_WRITE_LINE_MEMBER( irq6_w );
	DECLARE_WRITE_LINE_MEMBER( irq7_w );

	DECLARE_WRITE_LINE_MEMBER( drq1_w );
	DECLARE_WRITE_LINE_MEMBER( drq2_w );
	DECLARE_WRITE_LINE_MEMBER( drq3_w );

	UINT8 dack_r(int line);
	void dack_w(int line,UINT8 data);
	void eop_w(int channels, int state);

	void nmi();
	void set_nmi_state(bool enabled) { m_nmi_enabled = enabled; }

	virtual void set_dma_channel(UINT8 channel, device_isa8_card_interface *dev, bool do_eop);
protected:
	void install_space(address_spacenum spacenum, device_t *dev, offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_device_func rhandler, const char* rhandler_name, write8_device_func whandler, const char *whandler_name);
	void install_space(address_spacenum spacenum, offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_space_func rhandler, const char* rhandler_name, write8_space_func whandler, const char *whandler_name);
	void install_space(address_spacenum spacenum, offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_delegate rhandler, write8_delegate whandler);

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();

	// internal state
	device_t   *m_maincpu;

	devcb_resolved_write_line	m_out_irq2_func;
	devcb_resolved_write_line	m_out_irq3_func;
	devcb_resolved_write_line	m_out_irq4_func;
	devcb_resolved_write_line	m_out_irq5_func;
	devcb_resolved_write_line	m_out_irq6_func;
	devcb_resolved_write_line	m_out_irq7_func;

	devcb_resolved_write_line	m_out_drq1_func;
	devcb_resolved_write_line	m_out_drq2_func;
	devcb_resolved_write_line	m_out_drq3_func;

	device_isa8_card_interface *m_dma_device[8];
	bool						m_dma_eop[8];
	const char				   *m_cputag;
	bool						m_nmi_enabled;
};


// device type definition
extern const device_type ISA8;

// ======================> device_isa8_card_interface

// class representing interface-specific live isa8 card
class device_isa8_card_interface : public device_slot_card_interface
{
	friend class isa8_device;
public:
	// construction/destruction
	device_isa8_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_isa8_card_interface();

	device_isa8_card_interface *next() const { return m_next; }

	void set_isa_device();
	// configuration access
	virtual UINT8 dack_r(int line);
	virtual void dack_w(int line,UINT8 data);
	virtual void eop_w(int state);

    // inline configuration
    static void static_set_isabus(device_t &device, device_t *isa_device);
public:
	isa8_device  *m_isa;
	device_t	 *m_isa_dev;
	device_isa8_card_interface *m_next;
};

// ======================> isa16bus_interface

struct isa16bus_interface
{
    devcb_write_line	m_out_irq2_cb; // this goes to IRQ 9 on pic
    devcb_write_line	m_out_irq3_cb;
    devcb_write_line	m_out_irq4_cb;
    devcb_write_line	m_out_irq5_cb;
    devcb_write_line	m_out_irq6_cb;
    devcb_write_line	m_out_irq7_cb;

	devcb_write_line	m_out_irq10_cb;
	devcb_write_line	m_out_irq11_cb;
	devcb_write_line	m_out_irq12_cb;
	devcb_write_line	m_out_irq14_cb;
	devcb_write_line	m_out_irq15_cb;

    devcb_write_line	m_out_drq0_cb;
	devcb_write_line	m_out_drq1_cb;
    devcb_write_line	m_out_drq2_cb;
    devcb_write_line	m_out_drq3_cb;
	devcb_write_line	m_out_drq5_cb;
	devcb_write_line	m_out_drq6_cb;
	devcb_write_line	m_out_drq7_cb;
};

class isa16_device;

class isa16_slot_device : public isa8_slot_device
{
public:
	// construction/destruction
	isa16_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
		// device-level overrides
	virtual void device_start();

    // inline configuration
    static void static_set_isa16_slot(device_t &device, device_t *owner, const char *isa_tag);
};


// device type definition
extern const device_type ISA16_SLOT;

// ======================> isa16_device
class isa16_device : public isa8_device,
                     public isa16bus_interface
{
public:
	// construction/destruction
	isa16_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void install16_device(device_t *dev, offs_t start, offs_t end, offs_t mask, offs_t mirror, read16_device_func rhandler, const char* rhandler_name, write16_device_func whandler, const char *whandler_name);
	void install16_device(offs_t start, offs_t end, offs_t mask, offs_t mirror, read16_delegate rhandler, write16_delegate whandler);
	void install16_device(offs_t start, offs_t end, offs_t mask, offs_t mirror, read16_space_func rhandler, const char* rhandler_name, write16_space_func whandler, const char *whandler_name);

	DECLARE_WRITE_LINE_MEMBER( irq10_w );
	DECLARE_WRITE_LINE_MEMBER( irq11_w );
	DECLARE_WRITE_LINE_MEMBER( irq12_w );
	DECLARE_WRITE_LINE_MEMBER( irq14_w );
	DECLARE_WRITE_LINE_MEMBER( irq15_w );

	DECLARE_WRITE_LINE_MEMBER( drq0_w );
	DECLARE_WRITE_LINE_MEMBER( drq5_w );
	DECLARE_WRITE_LINE_MEMBER( drq6_w );
	DECLARE_WRITE_LINE_MEMBER( drq7_w );

	UINT16 dack16_r(int line);
	void dack16_w(int line,UINT16 data);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_config_complete();

private:
	// internal state
	devcb_resolved_write_line	m_out_irq10_func;
	devcb_resolved_write_line	m_out_irq11_func;
	devcb_resolved_write_line	m_out_irq12_func;
	devcb_resolved_write_line	m_out_irq14_func;
	devcb_resolved_write_line	m_out_irq15_func;

	devcb_resolved_write_line	m_out_drq0_func;
	devcb_resolved_write_line	m_out_drq5_func;
	devcb_resolved_write_line	m_out_drq6_func;
	devcb_resolved_write_line	m_out_drq7_func;
};


// device type definition
extern const device_type ISA16;

// ======================> device_isa16_card_interface

// class representing interface-specific live isa16 card
class device_isa16_card_interface : public device_isa8_card_interface
{
	friend class isa16_device;
public:
	// construction/destruction
	device_isa16_card_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_isa16_card_interface();
	virtual UINT16 dack16_r(int line);
	virtual void dack16_w(int line,UINT16 data);

	void set_isa_device();
	isa16_device  *m_isa;
};

#endif  /* __ISA_H__ */
