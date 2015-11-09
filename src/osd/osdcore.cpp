// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "osdcore.h"

bool g_print_verbose = false;

static const int MAXSTACK = 10;
static osd_output *m_stack[MAXSTACK];
static int m_ptr = -1;

/*-------------------------------------------------
    osd_output
-------------------------------------------------*/

void osd_output::push(osd_output *delegate)
{
	if (m_ptr < MAXSTACK)
	{
		delegate->m_chain = (m_ptr >= 0 ? m_stack[m_ptr] : NULL);
		m_ptr++;
		m_stack[m_ptr] = delegate;
	}
}

void osd_output::pop(osd_output *delegate)
{
	int f = -1;
	for (int i=0; i<=m_ptr; i++)
		if (m_stack[i] == delegate)
		{
			f = i;
			break;
		}
	if (f >= 0)
	{
		if (f < m_ptr)
			m_stack[f+1]->m_chain = m_stack[f]->m_chain;
		m_ptr--;
		for (int i = f; i <= m_ptr; i++)
			m_stack[i] = m_stack[i+1];
	}
}


/***************************************************************************
    OUTPUT MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    osd_printf_error - output an error to the
    appropriate callback
-------------------------------------------------*/

void CLIB_DECL osd_printf_error(const char *format, ...)
{
	va_list argptr;

	/* do the output */
	va_start(argptr, format);
	if (m_ptr >= 0) m_stack[m_ptr]->output_callback(OSD_OUTPUT_CHANNEL_ERROR, format, argptr);
	va_end(argptr);
}


/*-------------------------------------------------
    osd_printf_warning - output a warning to the
    appropriate callback
-------------------------------------------------*/

void CLIB_DECL osd_printf_warning(const char *format, ...)
{
	va_list argptr;

	/* do the output */
	va_start(argptr, format);
	if (m_ptr >= 0) m_stack[m_ptr]->output_callback(OSD_OUTPUT_CHANNEL_WARNING, format, argptr);
	va_end(argptr);
}


/*-------------------------------------------------
    osd_printf_info - output info text to the
    appropriate callback
-------------------------------------------------*/

void CLIB_DECL osd_printf_info(const char *format, ...)
{
	va_list argptr;

	/* do the output */
	va_start(argptr, format);
	if (m_ptr >= 0) m_stack[m_ptr]->output_callback(OSD_OUTPUT_CHANNEL_INFO, format, argptr);
	va_end(argptr);
}


/*-------------------------------------------------
    osd_printf_verbose - output verbose text to
    the appropriate callback
-------------------------------------------------*/

void CLIB_DECL osd_printf_verbose(const char *format, ...)
{
	va_list argptr;

	/* if we're not verbose, skip it */
	if (!g_print_verbose)
		return;

	/* do the output */
	va_start(argptr, format);
	if (m_ptr >= 0) m_stack[m_ptr]->output_callback(OSD_OUTPUT_CHANNEL_VERBOSE, format, argptr);
	va_end(argptr);
}


/*-------------------------------------------------
    osd_printf_debug - output debug text to the
    appropriate callback
-------------------------------------------------*/

void CLIB_DECL osd_printf_debug(const char *format, ...)
{
	va_list argptr;

	/* do the output */
	va_start(argptr, format);
	if (m_ptr >= 0) m_stack[m_ptr]->output_callback(OSD_OUTPUT_CHANNEL_DEBUG, format, argptr);
	va_end(argptr);
}


/*-------------------------------------------------
    osd_printf_log - output log text to the
    appropriate callback
-------------------------------------------------*/

#ifdef UNUSED_FUNCTION
void CLIB_DECL osd_printf_log(const char *format, ...)
{
	va_list argptr;

	/* do the output */
	va_start(argptr, format);
	output_cb[OSD_OUTPUT_CHANNEL_LOG])(format, argptr);
	va_end(argptr);
}
#endif
