// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    imgterrs.h

    Imgtool errors

***************************************************************************/

#ifndef IMGTERRS_H
#define IMGTERRS_H

/* Error codes */
enum imgtoolerr_t
{
	IMGTOOLERR_SUCCESS,
	IMGTOOLERR_OUTOFMEMORY,
	IMGTOOLERR_UNEXPECTED,
	IMGTOOLERR_BUFFERTOOSMALL,
	IMGTOOLERR_READERROR,
	IMGTOOLERR_WRITEERROR,
	IMGTOOLERR_READONLY,
	IMGTOOLERR_CORRUPTIMAGE,
	IMGTOOLERR_CORRUPTFILE,
	IMGTOOLERR_CORRUPTDIR,
	IMGTOOLERR_FILENOTFOUND,
	IMGTOOLERR_MODULENOTFOUND,
	IMGTOOLERR_UNIMPLEMENTED,
	IMGTOOLERR_PARAMTOOSMALL,
	IMGTOOLERR_PARAMTOOLARGE,
	IMGTOOLERR_PARAMNEEDED,
	IMGTOOLERR_PARAMNOTNEEDED,
	IMGTOOLERR_PARAMCORRUPT,
	IMGTOOLERR_BADFILENAME,
	IMGTOOLERR_NOSPACE,
	IMGTOOLERR_INPUTPASTEND,
	IMGTOOLERR_CANNOTUSEPATH,
	IMGTOOLERR_INVALIDPATH,
	IMGTOOLERR_PATHNOTFOUND,
	IMGTOOLERR_DIRNOTEMPTY,
	IMGTOOLERR_SEEKERROR,
	IMGTOOLERR_NOFORKS,
	IMGTOOLERR_FORKNOTFOUND,
	IMGTOOLERR_INVALIDPARTITION
};



/* These error codes are actually modifiers that make it easier to distinguish
 * the cause of an error
 *
 * Note - drivers should not use these modifiers
 */
#define IMGTOOLERR_SRC_MODULE           0x1000
#define IMGTOOLERR_SRC_FUNCTIONALITY    0x2000
#define IMGTOOLERR_SRC_IMAGEFILE        0x3000
#define IMGTOOLERR_SRC_FILEONIMAGE      0x4000
#define IMGTOOLERR_SRC_NATIVEFILE       0x5000

#define ERRORCODE(err)      ((err) & 0x0fff)
#define ERRORSOURCE(err)    ((err) & 0xf000)
#define ERRORPARAM(err)     (((err) & 0xf0000) / 0x10000)

#define PARAM_TO_ERROR(errcode, param)  ((errcode) | ((param) * 0x10000))


const char *imgtool_error(imgtoolerr_t err);

#endif /* IMGTERRS_H */
