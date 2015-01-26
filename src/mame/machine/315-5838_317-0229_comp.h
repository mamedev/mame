
#pragma once

#ifndef __SEGA315_5838_COMP__
#define __SEGA315_5838_COMP__

typedef device_delegate<UINT16 (UINT32)> sega_m2_read_delegate;

extern const device_type SEGA315_5838_COMP;

#define MCFG_SET_5838_READ_CALLBACK( _class, _method) \
	sega_315_5838_comp_device::set_read_cb(*device, sega_m2_read_delegate(&_class::_method, #_class "::" #_method, NULL, (_class *)0));


class sega_315_5838_comp_device :  public device_t
{
public:
	// construction/destruction
	sega_315_5838_comp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	sega_m2_read_delegate m_read;

	static void set_read_cb(device_t &device,sega_m2_read_delegate readcb)
	{
		sega_315_5838_comp_device &dev = downcast<sega_315_5838_comp_device &>(device);
		dev.m_read = readcb;
	}

	DECLARE_READ32_MEMBER(decathlt_prot_r);
	void write_prot_data(UINT32 data, UINT32 mem_mask, int offset, int which);
	DECLARE_WRITE32_MEMBER(decathlt_prot1_w);
	DECLARE_WRITE32_MEMBER(decathlt_prot2_w);
	void install_decathlt_protection();
	void install_doa_protection();

	DECLARE_READ32_MEMBER(doa_prot_r);
	DECLARE_WRITE32_MEMBER(doa_prot_w);

protected:
	virtual void device_start();
	virtual void device_reset();

private:

	// Decathlete specific variables and functions (see machine/decathlt.c)
	UINT32 m_decathlt_protregs[4];
	UINT32 m_decathlt_lastcount;
	UINT32 m_decathlt_part;
	UINT32 m_decathlt_prot_uploadmode;
	UINT32 m_decathlt_prot_uploadoffset;
	UINT16 m_decathlt_prottable1[24];
	UINT16 m_decathlt_prottable2[128];

	// Doa
	int m_protstate;
	int m_prot_a;
	UINT8 m_protram[256];
};

#endif
