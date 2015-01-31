#include "emucore.h"
#include "osdcore.h"

bool g_print_verbose = false;


/*-------------------------------------------------
    osd_file_output_callback - default callback
    for file output
-------------------------------------------------*/

void osd_file_output_callback(FILE *param, const char *format, va_list argptr)
{
	vfprintf(param, format, argptr);
}


/*-------------------------------------------------
    osd_null_output_callback - default callback
    for no output
-------------------------------------------------*/

void osd_null_output_callback(FILE *param, const char *format, va_list argptr)
{
}



/* output channels */
static output_delegate output_cb[OSD_OUTPUT_CHANNEL_COUNT] =
{
	output_delegate(FUNC(osd_file_output_callback), stderr),   // OSD_OUTPUT_CHANNEL_ERROR
	output_delegate(FUNC(osd_file_output_callback), stderr),   // OSD_OUTPUT_CHANNEL_WARNING
	output_delegate(FUNC(osd_file_output_callback), stdout),   // OSD_OUTPUT_CHANNEL_INFO
#ifdef MAME_DEBUG
	output_delegate(FUNC(osd_file_output_callback), stdout),   // OSD_OUTPUT_CHANNEL_DEBUG
#else
	output_delegate(FUNC(osd_null_output_callback), stdout),   // OSD_OUTPUT_CHANNEL_DEBUG
#endif
	output_delegate(FUNC(osd_file_output_callback), stdout),   // OSD_OUTPUT_CHANNEL_VERBOSE
	output_delegate(FUNC(osd_file_output_callback), stdout)    // OSD_OUTPUT_CHANNEL_LOG
};


/***************************************************************************
    OUTPUT MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    osd_set_output_channel - configure an output
    channel
-------------------------------------------------*/

output_delegate osd_set_output_channel(output_channel channel, output_delegate callback)
{
	if (!(channel < OSD_OUTPUT_CHANNEL_COUNT) || callback.isnull())
	{
		throw std::exception();
	}

	/* return the originals if requested */
	output_delegate prevcb = output_cb[channel];

	/* set the new ones */
	output_cb[channel] = callback;
	return prevcb;
}

/*-------------------------------------------------
    osd_printf_error - output an error to the
    appropriate callback
-------------------------------------------------*/

void CLIB_DECL osd_printf_error(const char *format, ...)
{
	va_list argptr;

	/* do the output */
	va_start(argptr, format);
	output_cb[OSD_OUTPUT_CHANNEL_ERROR](format, argptr);
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
	output_cb[OSD_OUTPUT_CHANNEL_WARNING](format, argptr);
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
	output_cb[OSD_OUTPUT_CHANNEL_INFO](format, argptr);
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
	output_cb[OSD_OUTPUT_CHANNEL_VERBOSE](format, argptr);
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
	output_cb[OSD_OUTPUT_CHANNEL_DEBUG](format, argptr);
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
