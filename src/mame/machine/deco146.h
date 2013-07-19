#pragma once
#ifndef __DECO146_H__
#define __DECO146_H__


/* Data East 146 protection chip */

class deco146_device : public device_t
{
public:
	deco146_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void write_data(address_space &space, UINT16 address, UINT16 data, UINT16 mem_mask, UINT8 &csflags);
	UINT16 read_data(UINT16 address, UINT16 mem_mask, UINT8 &csflags);

protected:
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	UINT16 read_protport(UINT16 address, UINT16 mem_mask);
	void write_protport(address_space &space, UINT16 address, UINT16 data, UINT16 mem_mask);

	UINT16 m_rambank0[0x80];
	UINT16 m_rambank1[0x80];

	UINT16* m_current_rambank;

	// set these up as actual callbacks!
	UINT16 read_input_a_callback(void);
	UINT16 read_input_b_callback(void);
	UINT16 read_input_c_callback(void);
	void soundlatch_write_callback(address_space &space, UINT16 data, UINT16 mem_mask);

	UINT16 m_nand;
	UINT16 m_xor;
	UINT16 m_soundlatch;

	UINT16 m_latchaddr;
	UINT16 m_latchdata;
	int m_latchflag;


private:
	UINT8 region_selects[6];

};

extern const device_type DECO146PROT;


#define MCFG_DECO146_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DECO146PROT, 0)



#endif
