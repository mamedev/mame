// license:GPL-2.0+
// copyright-holders:SIMH project
/*****************************************************************************
 *
 * includes/pdp11.h
 *
 ****************************************************************************/

#ifndef PDP11_H_
#define PDP11_H_

/*
 * taken from PDP11/pdp11_defs.h in SIMH
 */

/* Device CSRs */

#define CSR_V_GO        0                               /* go */
#define CSR_V_IE        6                               /* interrupt enable */
#define CSR_V_DONE      7                               /* done */
#define CSR_V_BUSY      11                              /* busy */
#define CSR_V_ERR       15                              /* error */
#define CSR_GO          (1u << CSR_V_GO)
#define CSR_IE          (1u << CSR_V_IE)
#define CSR_DONE        (1u << CSR_V_DONE)
#define CSR_BUSY        (1u << CSR_V_BUSY)
#define CSR_ERR         (1u << CSR_V_ERR)


#define clear_virq(_callback, _csr, _ie, _intrq) \
	if ((_csr) & (_ie)) { (_intrq) = CLEAR_LINE; }

#define raise_virq(_callback, _csr, _ie, _intrq) \
	if ((_csr) & (_ie)) { (_intrq) = ASSERT_LINE; _callback (ASSERT_LINE); }


#define UPDATE_16BIT(_storage, _data, _mask) \
	do { *_storage = ((*_storage & ~_mask) | (_data & _mask)); } while (0)


#endif /* PDP11_H_ */
