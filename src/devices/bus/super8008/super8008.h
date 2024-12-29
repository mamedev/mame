// license:BSD-3-Clause
// copyright-holders:Jeremy English
/***************************************************************************
    
	8008-Super Bus Device aka ThunderDome


	Scott's 8008 Supercomputer a.k.a. Master Blaster
	by Dr. Scott M. Baker
    
	Schematics:
	https://github.com/sbelectronics/8008-super 
    
	ROM Source Code:
	https://github.com/sbelectronics/h8/tree/master/h8-8008
    
	Demo:
	https://youtu.be/wurKTPdPhrI?si=aerTbgHIFm_8YwU2
    
	Write up:
	https://www.smbaker.com/master-blaster-an-8008-supercomputer
    
	MAME driver for Jim Loos 8008-SBC
	src/mame/homebrew/sbc8008.cpp
        
	This computer is based on Jim Loos 8008-SBC:
	https://github.com/jim11662418/8008-SBC    

============================================================================

	 1 GND             2 NCO
	 3 EXT_A0          4 EXT_TAKE0
	 5 EXT_A1          6 EXT_TAKE1
	 7 EXT_A2          8 EXT_TAKE2
	 9 EXT_A3         10 EXT_TAKE3
	11 EXT_A4         12 EXT_TAKE4
	13 EXT_A5         14 EXT_TAKE5
	15 EXT_A6         16 EXT_TAKE6
	17 EXT_A7         18 EXT_TAKE7
	19 EXT_A8         20 EXT_RUN0
	21 EXT_A9         22 EXT_RUN1
	23 EXT_A10        24 EXT_RUN2
	25 EXT_A11        26 EXT_RUN3
	27 EXT_A12        28 EXT_RUN4
	29 EXT_A13        30 EXT_RUN5
	31 GND            32 EXT_RUN6
	33 EXT_D0         34 EXT_RUN7
	35 EXT_D1         36 EXT_REQ0
	37 EXT_D2         38 EXT_REQ1
	39 EXT_D3         40 EXT_REQ2
	41 EXT_D4         42 EXT_REQ3
	43 EXT_D5         44 EXT_REQ4
	45 EXT_D6         46 EXT_REQ5
	47 EXT_D7         48 EXT_REQ6
	49 GND            50 EXT_REQ7
	51 EXT_CS         52 EXT_INT0
	53 EXT_RD         54 EXT_INT1
	55 EXT_WR         56 EXT_INT2
	57 EXT_RESET      58 EXT_INT3
	59 EXT_BUSDIR     60 EXT_INT4
	61 EXT_WRP        62 EXT_INT5
	63 EXT_TAKEW      64 EXT_INT6
	65 NC             66 EXT_INT7
	67 +8V            68 NC


***************************************************************************/

#ifndef MAME_BUS_SUPER8008_SUPER8008_H
#define MAME_BUS_SUPER8008_SUPER8008_H

#pragma once

#include <functional>
#include <vector>



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class super8008_bus_device;

// ======================> device_super8008_card_interface

class device_super8008_card_interface : public device_interface
{
	friend class super8008_bus_device;

public:

	virtual void ext_write( offs_t offset, uint8_t data) {}
	virtual uint8_t ext_read(offs_t offset) {return 0;}
	virtual void ext_int(){}
	virtual void ext_reset(){}
	virtual void ext_req(){}
	virtual void ext_take(int state) { }
	virtual uint8_t ext_run() {return 0;}
	

protected:
	// construction/destruction
	device_super8008_card_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;

	super8008_bus_device  *m_bus;

};



// ======================> super8008_bus_device

class super8008_bus_device : public device_t
{
public:
	// construction/destruction
	super8008_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~super8008_bus_device();

	void add_card(device_super8008_card_interface &card);
	
	virtual void ext_write( offs_t offset, uint8_t data);
	virtual uint8_t ext_read(offs_t offset);
	void ext_take(int state);
	void ext_cs(uint8_t state){ m_ext_cs = state;}
	void ext_int();
	void ext_reset();
	void ext_req();
	uint8_t ext_run();

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override;
	

private:
	using card_vector = std::vector<std::reference_wrapper<device_super8008_card_interface> >;

	card_vector m_device_list;

	uint8_t m_ext_take;
	uint8_t m_ext_cs;
};


// ======================> super8008_slot_device

class super8008_slot_device : public device_t, public device_single_card_slot_interface<device_super8008_card_interface>
{
public:
	// construction/destruction
	template <typename T>
	super8008_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt)
		: super8008_slot_device(mconfig, tag, owner, DERIVED_CLOCK(1, 1))
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	super8008_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_bus(T &&tag) { m_bus.set_tag(std::forward<T>(tag)); }

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	required_device<super8008_bus_device> m_bus;
};



// device type definition
DECLARE_DEVICE_TYPE(SUPER8008_BUS,  super8008_bus_device)
DECLARE_DEVICE_TYPE(SUPER8008_SLOT, super8008_slot_device)

void super8008_bus_devices(device_slot_interface &device);


#endif // MAME_BUS_super8008_super8008_H
