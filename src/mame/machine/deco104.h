#pragma once
#ifndef __DECO104_H__
#define __DECO104_H__


/* Data East 104 protection chip */

class deco104_device : public device_t
{
public:
	deco104_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ16_MEMBER(dblewing_prot_r);
	DECLARE_WRITE16_MEMBER(dblewing_prot_w);
protected:
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();


	/* for the buggy double wing sim */
	/* protection */
	UINT16 m_008_data;
	UINT16 m_104_data;
	UINT16 m_406_data;
	UINT16 m_608_data;
	UINT16 m_70c_data;
	UINT16 m_78a_data;
	UINT16 m_088_data;
	UINT16 m_58c_data;
	UINT16 m_408_data;
	UINT16 m_40e_data;
	UINT16 m_080_data;
	UINT16 m_788_data;
	UINT16 m_38e_data;
	UINT16 m_580_data;
	UINT16 m_60a_data;
	UINT16 m_200_data;
	UINT16 m_28c_data;
	UINT16 m_18a_data;
	UINT16 m_280_data;
	UINT16 m_384_data;

	UINT16 m_boss_move;
	UINT16 m_boss_shoot_type;
	UINT16 m_boss_3_data;
	UINT16 m_boss_4_data;
	UINT16 m_boss_5_data;
	UINT16 m_boss_5sx_data;
	UINT16 m_boss_6_data;
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
