#pragma once
#ifndef __DECO104_H__
#define __DECO104_H__


/* Data East 104 protection chip */

class deco104_device : public device_t
{
public:
	deco104_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);


protected:
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();



private:


};

extern const device_type DECO104PROT;


#define MCFG_DECO104_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DECO104PROT, 0)

// old
void decoprot104_reset(running_machine &machine);


DECLARE_READ16_HANDLER( deco16_104_prot_r );
DECLARE_READ16_HANDLER( deco16_104_cninja_prot_r );
DECLARE_READ16_HANDLER( deco16_104_rohga_prot_r );
DECLARE_READ16_HANDLER( deco16_104_pktgaldx_prot_r );
DECLARE_WRITE16_HANDLER( deco16_104_prot_w );
DECLARE_WRITE16_HANDLER( deco16_104_cninja_prot_w );
DECLARE_WRITE16_HANDLER( deco16_104_rohga_prot_w );
DECLARE_WRITE16_HANDLER( deco16_104_pktgaldx_prot_w );
DECLARE_READ16_HANDLER( dietgo_104_prot_r );
DECLARE_WRITE16_HANDLER( dietgo_104_prot_w );


#endif
