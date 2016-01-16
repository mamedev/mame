// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor ABC 890 bus expander emulation

*********************************************************************/

#pragma once

#ifndef __ABC890__
#define __ABC890__

#include "emu.h"
#include "abcbus.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> abc890_t

class abc890_t :  public device_t,
					public device_abcbus_card_interface
{
public:
	// construction/destruction
	abc890_t(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
	abc890_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_abcbus_interface overrides
	virtual void abcbus_cs(UINT8 data) override;
	virtual UINT8 abcbus_inp() override;
	virtual void abcbus_out(UINT8 data) override;
	virtual UINT8 abcbus_stat() override;
	virtual void abcbus_c1(UINT8 data) override;
	virtual void abcbus_c2(UINT8 data) override;
	virtual void abcbus_c3(UINT8 data) override;
	virtual void abcbus_c4(UINT8 data) override;
	virtual UINT8 abcbus_xmemfl(offs_t offset) override;
	virtual void abcbus_xmemw(offs_t offset, UINT8 data) override;
};


// ======================> abc_expansion_unit_t

class abc_expansion_unit_t :  public abc890_t
{
public:
	// construction/destruction
	abc_expansion_unit_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
};


// ======================> abc894_t

class abc894_t :  public abc890_t
{
public:
	// construction/destruction
	abc894_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
};


// ======================> abc850_t

class abc850_t :  public abc890_t
{
public:
	// construction/destruction
	abc850_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
};


// ======================> abc852_t

class abc852_t :  public abc890_t
{
public:
	// construction/destruction
	abc852_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
};


// ======================> abc856_t

class abc856_t :  public abc890_t
{
public:
	// construction/destruction
	abc856_t(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
};


// device type definition
extern const device_type ABC_EXPANSION_UNIT;
extern const device_type ABC890;
extern const device_type ABC894;
extern const device_type ABC850;
extern const device_type ABC852;
extern const device_type ABC856;



#endif
