/*----------- defined in machine/decoprot.c -----------*/

READ16_HANDLER( deco16_60_prot_r );
READ16_HANDLER( deco16_66_prot_r );
READ16_HANDLER( deco16_104_prot_r );
READ16_HANDLER( deco16_104_cninja_prot_r );
READ16_HANDLER( deco16_104_rohga_prot_r );
READ16_HANDLER( deco16_146_funkyjet_prot_r );
READ16_HANDLER( deco16_146_nitroball_prot_r );
READ16_HANDLER( deco16_104_pktgaldx_prot_r );
READ32_HANDLER( deco16_146_fghthist_prot_r );

WRITE16_HANDLER( deco16_60_prot_w );
WRITE16_HANDLER( deco16_66_prot_w );
WRITE16_HANDLER( deco16_104_prot_w );
WRITE16_HANDLER( deco16_104_cninja_prot_w );
WRITE16_HANDLER( deco16_104_rohga_prot_w );
WRITE16_HANDLER( deco16_146_funkyjet_prot_w );
WRITE16_HANDLER( deco16_146_nitroball_prot_w );
WRITE16_HANDLER( deco16_104_pktgaldx_prot_w );
WRITE32_HANDLER( deco16_146_fghthist_prot_w );

void decoprot_reset(void);

extern UINT16 *deco16_prot_ram;
extern UINT32 *deco32_prot_ram;
