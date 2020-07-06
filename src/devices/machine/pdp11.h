// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*****************************************************************************
 *
 * includes/pdp11.h
 *
 ****************************************************************************/

#ifndef PDP11_H_
#define PDP11_H_

// bit definitions for generic device CSR

enum {
	CSR_GO =	0000001,
	CSR_IE =	0000100,	// interrupt enable
	CSR_DONE =	0000200,
	CSR_BUSY =	0004000,
	CSR_ERR =	0100000
};


#define clear_virq(_callback, _csr, _ie, _intrq) \
	if ((_csr) & (_ie)) { (_intrq) = CLEAR_LINE; }

#define raise_virq(_callback, _csr, _ie, _intrq) \
	if ((_csr) & (_ie)) { (_intrq) = ASSERT_LINE; _callback (ASSERT_LINE); }


#define UPDATE_16BIT(_storage, _data, _mask) \
	do { *_storage = ((*_storage & ~_mask) | (_data & _mask)); } while (0)


#endif /* PDP11_H_ */
