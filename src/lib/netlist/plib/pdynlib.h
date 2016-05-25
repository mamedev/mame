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

PLIB_NAMESPACE_START()

// ----------------------------------------------------------------------------------------
// pdynlib: dynamic loading of libraries  ...
// ----------------------------------------------------------------------------------------

class dynlib
{
public:
	dynlib(const pstring libname);
	dynlib(const pstring path, const pstring libname);
	~dynlib();

	bool isLoaded() const;

	template <typename T>
	T getsym(const pstring name)
	{
		return reinterpret_cast<T>(getsym_p(name));
	}
private:
	void *getsym_p(const pstring name);

	bool m_isLoaded;
	void *m_lib;
};

PLIB_NAMESPACE_END()

#endif /* _PSTRING_H_ */
