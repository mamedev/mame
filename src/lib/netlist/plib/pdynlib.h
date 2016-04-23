// license:GPL-2.0+
// copyright-holders:Couriersud
/*
 * pstring.h
 */

#ifndef _PDYNLIB_H_
#define _PDYNLIB_H_

#include <cstdarg>
#include <cstddef>

#include "pconfig.h"
#include "pstring.h"

// ----------------------------------------------------------------------------------------
// pdynlib: dynamic loading of libraries  ...
// ----------------------------------------------------------------------------------------

class pdynlib
{
public:
	pdynlib(const pstring libname);
	pdynlib(const pstring path, const pstring libname);
	~pdynlib();

	bool isLoaded() const;

	template <typename T>
	T *getsym(const pstring name)
	{
		return reinterpret_cast<T *>(getsym_p(name));
	}
private:
	void *getsym_p(const pstring name);

	bool m_isLoaded;
	void *m_lib;
};

#endif /* _PSTRING_H_ */
