// license:BSD-3-Clause
// copyright-holders:David Haywood, Charles MacDonald
#pragma once
#ifndef __DECO146_H__
#define __DECO146_H__

typedef device_delegate<UINT16 (int unused)> deco146_port_read_cb;
typedef device_delegate<void (address_space &space, UINT16 data, UINT16 mem_mask)> deco146_port_write_cb;


#define MCFG_DECO146_SET_PORTA_CALLBACK( _class, _method) \
	deco_146_base_device::set_port_a_cb(*device, deco146_port_read_cb(&_class::_method, #_class "::" #_method, NULL, (_class *)0));

#define MCFG_DECO146_SET_PORTB_CALLBACK( _class, _method) \
	deco_146_base_device::set_port_b_cb(*device, deco146_port_read_cb(&_class::_method, #_class "::" #_method, NULL, (_class *)0));

#define MCFG_DECO146_SET_PORTC_CALLBACK( _class, _method) \
	deco_146_base_device::set_port_c_cb(*device, deco146_port_read_cb(&_class::_method, #_class "::" #_method, NULL, (_class *)0));

#define MCFG_DECO146_SET_SOUNDLATCH_CALLBACK( _class, _method) \
	deco_146_base_device::set_soundlatch_cb(*device, deco146_port_write_cb(&_class::_method, #_class "::" #_method, NULL, (_class *)0));


// there are some standard ways the chip gets hooked up, so have them here ready to use
#define MCFG_DECO146_SET_INTERFACE_SCRAMBLE( a9,a8,a7,a6,a5,a4,a3,a2,a1,a0 ) \
	deco_146_base_device::set_interface_scramble(*device, a9,a8,a7,a6,a5,a4,a3,a2,a1,a0);

#define MCFG_DECO146_SET_INTERFACE_SCRAMBLE_REVERSE \
	deco_146_base_device::set_interface_scramble(*device, 0,1,2,3,4,5,6,7,8,9);

#define MCFG_DECO146_SET_INTERFACE_SCRAMBLE_INTERLEAVE \
	deco_146_base_device::set_interface_scramble(*device, 4,5,3,6,2,7,1,8,0,9 );

#define MCFG_DECO146_SET_USE_MAGIC_ADDRESS_XOR \
	deco_146_base_device::set_use_magic_read_address_xor(*device, 1 );




#define BLK (0xff)
#define INPUT_PORT_A (-1)
#define INPUT_PORT_B (-2)
#define INPUT_PORT_C (-3)


#define NIB3__ 0xc, 0xd, 0xe, 0xf
#define NIB3R1 0xd, 0xe, 0xf, 0xc
#define NIB3R2 0xe, 0xf, 0xc, 0xd
#define NIB3R3 0xf, 0xc, 0xd, 0xe

#define NIB2__ 0x8, 0x9, 0xa, 0xb
#define NIB2R1 0x9, 0xa, 0xb, 0x8
#define NIB2R2 0xa, 0xb, 0x8, 0x9
#define NIB2R3 0xb, 0x8, 0x9, 0xa

#define NIB1__ 0x4, 0x5, 0x6, 0x7
#define NIB1R1 0x5, 0x6, 0x7, 0x4
#define NIB1R2 0x6, 0x7, 0x4, 0x5
#define NIB1R3 0x7, 0x4, 0x5, 0x6

#define NIB0__ 0x0, 0x1, 0x2, 0x3
#define NIB0R1 0x1, 0x2, 0x3, 0x0
#define NIB0R2 0x2, 0x3, 0x0, 0x1
#define NIB0R3 0x3, 0x0, 0x1, 0x2

#define BLANK_ BLK, BLK, BLK, BLK

struct deco146port_xx
{
	int write_offset;
	UINT8 mapping[16];
	int use_xor;
	int use_nand;
};



/* Data East 146 protection chip */

class deco_146_base_device : public device_t
{
public:
	//deco_146_base_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	deco_146_base_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	void write_data(address_space &space, UINT16 address, UINT16 data, UINT16 mem_mask, UINT8 &csflags);
	UINT16 read_data(UINT16 address, UINT16 mem_mask, UINT8 &csflags);

	static void set_port_a_cb(device_t &device,deco146_port_read_cb port_cb);
	static void set_port_b_cb(device_t &device,deco146_port_read_cb port_cb);
	static void set_port_c_cb(device_t &device,deco146_port_read_cb port_cb);
	static void set_soundlatch_cb(device_t &device,deco146_port_write_cb port_cb);
	static void set_interface_scramble(device_t &device,UINT8 a9, UINT8 a8, UINT8 a7, UINT8 a6, UINT8 a5, UINT8 a4, UINT8 a3,UINT8 a2,UINT8 a1,UINT8 a0);
	static void set_use_magic_read_address_xor(device_t &device, int use_xor);





	deco146_port_read_cb m_port_a_r;
	deco146_port_read_cb m_port_b_r;
	deco146_port_read_cb m_port_c_r;
	deco146_port_write_cb m_soundlatch_w;

	UINT16 port_a_default(int unused);
	UINT16 port_b_default(int unused);
	UINT16 port_c_default(int unused);
	UINT16 port_dummy_cb(int unused);
	void soundlatch_default(address_space &space, UINT16 data, UINT16 mem_mask);
	void soundlatch_dummy(address_space &space, UINT16 data, UINT16 mem_mask);

	UINT8 m_bankswitch_swap_read_address;
	UINT16 m_magic_read_address_xor;
	int m_magic_read_address_xor_enabled;
	UINT8 m_xor_port;
	UINT8 m_mask_port;
	UINT8 m_soundlatch_port;


	UINT8 m_external_addrswap[10];

	deco146port_xx* m_lookup_table;



// for older handlers
#define DECO146__PORT(p) (prot_ram[p/2])





protected:
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	UINT16 read_protport(UINT16 address, UINT16 mem_mask);
	virtual void write_protport(address_space &space, UINT16 address, UINT16 data, UINT16 mem_mask);
	virtual UINT16 read_data_getloc(UINT16 address, int& location);

	UINT16 m_rambank0[0x80];
	UINT16 m_rambank1[0x80];

	int m_current_rambank;


	UINT16 m_nand;
	UINT16 m_xor;
	UINT16 m_soundlatch;

	UINT16 m_latchaddr;
	UINT16 m_latchdata;

	UINT8 m_configregion; // which value of upper 4 address lines accesses the config region
	int m_latchflag;
private:
	UINT8 region_selects[6];

};

extern const device_type DECO146BASE;

class deco146_device : public deco_146_base_device
{
public:
	deco146_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type DECO146PROT;

#define MCFG_DECO146_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DECO146PROT, 0)







#endif
