#pragma once
#ifndef __DECO146_H__
#define __DECO146_H__

typedef device_delegate<UINT16 (int unused)> deco146_port_read_cb;


#define MCFG_DECO146_SET_PORTA_CALLBACK( _class, _method) \
	deco146_device::set_port_a_cb(*device, deco146_port_read_cb(&_class::_method, #_class "::" #_method, NULL, (_class *)0));

#define MCFG_DECO146_SET_PORTB_CALLBACK( _class, _method) \
	deco146_device::set_port_b_cb(*device, deco146_port_read_cb(&_class::_method, #_class "::" #_method, NULL, (_class *)0));

#define MCFG_DECO146_SET_PORTC_CALLBACK( _class, _method) \
	deco146_device::set_port_c_cb(*device, deco146_port_read_cb(&_class::_method, #_class "::" #_method, NULL, (_class *)0));


/* Data East 146 protection chip */

class deco146_device : public device_t
{
public:
	deco146_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void write_data(address_space &space, UINT16 address, UINT16 data, UINT16 mem_mask, UINT8 &csflags);
	UINT16 read_data(UINT16 address, UINT16 mem_mask, UINT8 &csflags, int extra_read_address_xor);

	static void set_port_a_cb(device_t &device,deco146_port_read_cb port_cb);
	static void set_port_b_cb(device_t &device,deco146_port_read_cb port_cb);
	static void set_port_c_cb(device_t &device,deco146_port_read_cb port_cb);

	// legacy stuff
	DECLARE_READ32_MEMBER(dragngun_prot_r);
	DECLARE_READ32_MEMBER(captaven_prot_r);
	DECLARE_READ16_MEMBER(lemmings_prot_r);
	DECLARE_READ16_MEMBER(robocop2_prot_r);
	DECLARE_READ32_MEMBER(stadhr96_prot_146_r);
	DECLARE_WRITE32_MEMBER(stadhr96_prot_146_w);

	deco146_port_read_cb m_port_a_r;
	deco146_port_read_cb m_port_b_r;
	deco146_port_read_cb m_port_c_r;

	UINT16 port_a_default(int unused);
	UINT16 port_b_default(int unused);
	UINT16 port_c_default(int unused);


protected:
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	UINT16 read_protport(UINT16 address, UINT16 mem_mask, int extra_read_address_xor);
	void write_protport(address_space &space, UINT16 address, UINT16 data, UINT16 mem_mask);

	UINT16 m_rambank0[0x80];
	UINT16 m_rambank1[0x80];

	UINT16* m_current_rambank;

	void soundlatch_write_callback(address_space &space, UINT16 data, UINT16 mem_mask);

	UINT16 m_nand;
	UINT16 m_xor;
	UINT16 m_soundlatch;

	UINT16 m_latchaddr;
	UINT16 m_latchdata;
	int m_latchflag;


	int m_strobe;

private:
	UINT8 region_selects[6];

};

extern const device_type DECO146PROT;


#define MCFG_DECO146_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DECO146PROT, 0)

// old
void decoprot146_reset(running_machine &machine);





#endif
