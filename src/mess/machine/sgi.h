/*********************************************************************

    sgi.h

    Silicon Graphics MC (Memory Controller) code

*********************************************************************/

#ifndef _SGIMC_H
#define _SGIMC_H

void sgi_mc_init(running_machine &machine);

READ32_HANDLER(sgi_mc_r);
WRITE32_HANDLER(sgi_mc_w);


#endif /* _SGIMC_H */
