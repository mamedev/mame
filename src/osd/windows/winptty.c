// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdlib.h>

#include "winfile.h"
#include "strconv.h"
#include "winutil.h"

const char *winfile_ptty_identifier = "\\\\.\\pipe\\";

file_error win_open_ptty(const char *path, UINT32 openflags, osd_file **file, UINT64 *filesize)
{
	TCHAR *t_name;
	HANDLE pipe;

	if((t_name = tstring_from_utf8(path)) == NULL)
		return FILERR_OUT_OF_MEMORY;

	pipe = CreateNamedPipe(t_name, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT, 1, 32, 32, 0, NULL);

	osd_free(t_name);

	if(pipe == INVALID_HANDLE_VALUE)
		return FILERR_ACCESS_DENIED;

	(*file)->handle = pipe;
	*filesize = 0;
	return FILERR_NONE;
}

file_error win_read_ptty(osd_file *file, void *buffer, UINT64 offset, UINT32 count, UINT32 *actual)
{
	BOOL res;
	DWORD bytes_read;

	res = ReadFile(file->handle, buffer, count, &bytes_read, NULL);
	if(res == FALSE)
		return win_error_to_file_error(GetLastError());

	if(actual != NULL)
		*actual = bytes_read;

	return FILERR_NONE;
}

file_error win_write_ptty(osd_file *file, const void *buffer, UINT64 offset, UINT32 count, UINT32 *actual)
{
	BOOL res;
	DWORD bytes_wrote;

	res = WriteFile(file->handle, buffer, count, &bytes_wrote, NULL);
	if(res == FALSE)
		return win_error_to_file_error(GetLastError());

	if(actual != NULL)
		*actual = bytes_wrote;

	return FILERR_NONE;
}

file_error win_close_ptty(osd_file *file)
{
	FlushFileBuffers(file->handle);
	DisconnectNamedPipe(file->handle);
	CloseHandle(file->handle);
	return FILERR_NONE;
}
