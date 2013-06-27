/*----------- defined in machine/decoprot.c -----------*/

DECLARE_READ16_HANDLER( deco16_60_prot_r );
DECLARE_READ16_HANDLER( deco16_66_prot_r );
DECLARE_READ16_HANDLER( deco16_104_prot_r );
DECLARE_READ16_HANDLER( deco16_104_cninja_prot_r );
DECLARE_READ16_HANDLER( deco16_104_rohga_prot_r );
DECLARE_READ16_HANDLER( deco16_146_funkyjet_prot_r );
DECLARE_READ16_HANDLER( deco16_146_nitroball_prot_r );
DECLARE_READ16_HANDLER( deco16_104_pktgaldx_prot_r );
DECLARE_READ32_HANDLER( deco16_146_fghthist_prot_r );

DECLARE_WRITE16_HANDLER( deco16_60_prot_w );
DECLARE_WRITE16_HANDLER( deco16_66_prot_w );
DECLARE_WRITE16_HANDLER( deco16_104_prot_w );
DECLARE_WRITE16_HANDLER( deco16_104_cninja_prot_w );
DECLARE_WRITE16_HANDLER( deco16_104_rohga_prot_w );
DECLARE_WRITE16_HANDLER( deco16_146_funkyjet_prot_w );
DECLARE_WRITE16_HANDLER( deco16_146_nitroball_prot_w );
DECLARE_WRITE16_HANDLER( deco16_104_pktgaldx_prot_w );
DECLARE_WRITE32_HANDLER( deco16_146_fghthist_prot_w );

void decoprot_reset(running_machine &machine);

DECLARE_READ16_HANDLER( dietgo_104_prot_r );
DECLARE_WRITE16_HANDLER( dietgo_104_prot_w );
