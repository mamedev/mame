/*
 * nl_util.h
 *
 */

#ifndef NL_UTIL_H_
#define NL_UTIL_H_

#include "pstring.h"
#include "plists.h"

class nl_util
{
// this is purely static
private:
	nl_util() {};

public:
	typedef plinearlist_t<pstring, 10> pstring_list;

	static pstring_list split(const pstring &str, const pstring &onstr)
	{
		pstring_list temp;

		int p = 0;
		int pn;

		pn = str.find(onstr, p);
		while (pn>=0)
		{
			temp.add(str.substr(p, pn - p));
			p = pn + onstr.len();
			pn = str.find(onstr, p);
		}
		if (p<str.len())
			temp.add(str.substr(p));
		return temp;
	}
};

#endif /* NL_UTIL_H_ */
