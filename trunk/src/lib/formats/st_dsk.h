/*********************************************************************

    formats/st_dsk.h

    Atari ST generic 9/10/11 sector-per-track formats

*********************************************************************/

#ifndef ST_DSK_H_
#define ST_DSK_H_

#include "flopimg.h"

class st_gen_format : public floppy_image_format_t
{
public:
	st_gen_format(const char *name,const char *extensions,const char *description,const char *param_guidelines);

	static const desc_e desc_fcp_9[];

	static const desc_e desc_fcp_10_0[];
	static const desc_e desc_fcp_10_1[];
	static const desc_e desc_fcp_10_2[];
	static const desc_e desc_fcp_10_3[];
	static const desc_e desc_fcp_10_4[];
	static const desc_e desc_fcp_10_5[];
	static const desc_e desc_fcp_10_6[];
	static const desc_e desc_fcp_10_7[];
	static const desc_e desc_fcp_10_8[];
	static const desc_e desc_fcp_10_9[];
	static const desc_e *const desc_fcp_10[];

	static const desc_e desc_fcp_11_0[];
	static const desc_e desc_fcp_11_1[];
	static const desc_e desc_fcp_11_2[];
	static const desc_e desc_fcp_11_3[];
	static const desc_e desc_fcp_11_4[];
	static const desc_e desc_fcp_11_5[];
	static const desc_e desc_fcp_11_6[];
	static const desc_e desc_fcp_11_7[];
	static const desc_e desc_fcp_11_8[];
	static const desc_e desc_fcp_11_9[];
	static const desc_e desc_fcp_11_10[];
	static const desc_e *const desc_fcp_11[];
};

#endif /*ST_DSK_H_*/
