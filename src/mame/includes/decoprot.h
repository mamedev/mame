/*----------- defined in machine/decoprot.c -----------*/





DECLARE_READ16_HANDLER( deco16_104_prot_r );
DECLARE_READ16_HANDLER( deco16_104_cninja_prot_r );
DECLARE_READ16_HANDLER( deco16_104_rohga_prot_r );


DECLARE_READ16_HANDLER( deco16_104_pktgaldx_prot_r );



DECLARE_WRITE16_HANDLER( deco16_104_prot_w );
DECLARE_WRITE16_HANDLER( deco16_104_cninja_prot_w );
DECLARE_WRITE16_HANDLER( deco16_104_rohga_prot_w );

DECLARE_WRITE16_HANDLER( deco16_104_pktgaldx_prot_w );


void decoprot104_reset(running_machine &machine);

DECLARE_READ16_HANDLER( dietgo_104_prot_r );
DECLARE_WRITE16_HANDLER( dietgo_104_prot_w );
