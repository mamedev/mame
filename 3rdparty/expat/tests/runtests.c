/* Run the Expat test suite
                            __  __            _
                         ___\ \/ /_ __   __ _| |_
                        / _ \\  /| '_ \ / _` | __|
                       |  __//  \| |_) | (_| | |_
                        \___/_/\_\ .__/ \__,_|\__|
                                 |_| XML parser

   Copyright (c) 2001-2006 Fred L. Drake, Jr. <fdrake@users.sourceforge.net>
   Copyright (c) 2003      Greg Stein <gstein@users.sourceforge.net>
   Copyright (c) 2005-2007 Steven Solie <steven@solie.ca>
   Copyright (c) 2005-2012 Karl Waclawek <karl@waclawek.net>
   Copyright (c) 2016-2022 Sebastian Pipping <sebastian@pipping.org>
   Copyright (c) 2017-2022 Rhodri James <rhodri@wildebeest.org.uk>
   Copyright (c) 2017      Joe Orton <jorton@redhat.com>
   Copyright (c) 2017      José Gutiérrez de la Concha <jose@zeroc.com>
   Copyright (c) 2018      Marco Maggi <marco.maggi-ipsu@poste.it>
   Copyright (c) 2019      David Loffredo <loffredo@steptools.com>
   Copyright (c) 2020      Tim Gates <tim.gates@iress.com>
   Copyright (c) 2021      Dong-hee Na <donghee.na@python.org>
   Licensed under the MIT license:

   Permission is  hereby granted,  free of charge,  to any  person obtaining
   a  copy  of  this  software   and  associated  documentation  files  (the
   "Software"),  to  deal in  the  Software  without restriction,  including
   without  limitation the  rights  to use,  copy,  modify, merge,  publish,
   distribute, sublicense, and/or sell copies of the Software, and to permit
   persons  to whom  the Software  is  furnished to  do so,  subject to  the
   following conditions:

   The above copyright  notice and this permission notice  shall be included
   in all copies or substantial portions of the Software.

   THE  SOFTWARE  IS  PROVIDED  "AS  IS",  WITHOUT  WARRANTY  OF  ANY  KIND,
   EXPRESS  OR IMPLIED,  INCLUDING  BUT  NOT LIMITED  TO  THE WARRANTIES  OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
   NO EVENT SHALL THE AUTHORS OR  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
   DAMAGES OR  OTHER LIABILITY, WHETHER  IN AN  ACTION OF CONTRACT,  TORT OR
   OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
   USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <expat_config.h>

#if defined(NDEBUG)
#  undef NDEBUG /* because test suite relies on assert(...) at the moment */
#endif

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h> /* ptrdiff_t */
#include <ctype.h>
#include <limits.h>
#include <stdint.h> /* intptr_t uint64_t */

#if ! defined(__cplusplus)
#  include <stdbool.h>
#endif

#include "expat.h"
#include "chardata.h"
#include "structdata.h"
#include "internal.h"
#include "minicheck.h"
#include "memcheck.h"
#include "siphash.h"
#include "ascii.h" /* for ASCII_xxx */

#ifdef XML_LARGE_SIZE
#  define XML_FMT_INT_MOD "ll"
#else
#  define XML_FMT_INT_MOD "l"
#endif

#ifdef XML_UNICODE_WCHAR_T
#  define XML_FMT_CHAR "lc"
#  define XML_FMT_STR "ls"
#  include <wchar.h>
#  define xcstrlen(s) wcslen(s)
#  define xcstrcmp(s, t) wcscmp((s), (t))
#  define xcstrncmp(s, t, n) wcsncmp((s), (t), (n))
#  define XCS(s) _XCS(s)
#  define _XCS(s) L##s
#else
#  ifdef XML_UNICODE
#    error "No support for UTF-16 character without wchar_t in tests"
#  else
#    define XML_FMT_CHAR "c"
#    define XML_FMT_STR "s"
#    define xcstrlen(s) strlen(s)
#    define xcstrcmp(s, t) strcmp((s), (t))
#    define xcstrncmp(s, t, n) strncmp((s), (t), (n))
#    define XCS(s) s
#  endif /* XML_UNICODE */
#endif   /* XML_UNICODE_WCHAR_T */

static XML_Parser g_parser = NULL;

static void
tcase_add_test__ifdef_xml_dtd(TCase *tc, tcase_test_function test) {
#ifdef XML_DTD
  tcase_add_test(tc, test);
#else
  UNUSED_P(tc);
  UNUSED_P(test);
#endif
}

static void
basic_setup(void) {
  g_parser = XML_ParserCreate(NULL);
  if (g_parser == NULL)
    fail("Parser not created.");
}

static void
basic_teardown(void) {
  if (g_parser != NULL) {
    XML_ParserFree(g_parser);
    g_parser = NULL;
  }
}

/* Generate a failure using the parser state to create an error message;
   this should be used when the parser reports an error we weren't
   expecting.
*/
static void
_xml_failure(XML_Parser parser, const char *file, int line) {
  char buffer[1024];
  enum XML_Error err = XML_GetErrorCode(parser);
  sprintf(buffer,
          "    %d: %" XML_FMT_STR " (line %" XML_FMT_INT_MOD
          "u, offset %" XML_FMT_INT_MOD "u)\n    reported from %s, line %d\n",
          err, XML_ErrorString(err), XML_GetCurrentLineNumber(parser),
          XML_GetCurrentColumnNumber(parser), file, line);
  _fail_unless(0, file, line, buffer);
}

static enum XML_Status
_XML_Parse_SINGLE_BYTES(XML_Parser parser, const char *s, int len,
                        int isFinal) {
  enum XML_Status res = XML_STATUS_ERROR;
  int offset = 0;

  if (len == 0) {
    return XML_Parse(parser, s, len, isFinal);
  }

  for (; offset < len; offset++) {
    const int innerIsFinal = (offset == len - 1) && isFinal;
    const char c = s[offset]; /* to help out-of-bounds detection */
    res = XML_Parse(parser, &c, sizeof(char), innerIsFinal);
    if (res != XML_STATUS_OK) {
      return res;
    }
  }
  return res;
}

#define xml_failure(parser) _xml_failure((parser), __FILE__, __LINE__)

static void
_expect_failure(const char *text, enum XML_Error errorCode,
                const char *errorMessage, const char *file, int lineno) {
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_OK)
    /* Hackish use of _fail_unless() macro, but let's us report
       the right filename and line number. */
    _fail_unless(0, file, lineno, errorMessage);
  if (XML_GetErrorCode(g_parser) != errorCode)
    _xml_failure(g_parser, file, lineno);
}

#define expect_failure(text, errorCode, errorMessage)                          \
  _expect_failure((text), (errorCode), (errorMessage), __FILE__, __LINE__)

/* Dummy handlers for when we need to set a handler to tickle a bug,
   but it doesn't need to do anything.
*/
static unsigned long dummy_handler_flags = 0;

#define DUMMY_START_DOCTYPE_HANDLER_FLAG (1UL << 0)
#define DUMMY_END_DOCTYPE_HANDLER_FLAG (1UL << 1)
#define DUMMY_ENTITY_DECL_HANDLER_FLAG (1UL << 2)
#define DUMMY_NOTATION_DECL_HANDLER_FLAG (1UL << 3)
#define DUMMY_ELEMENT_DECL_HANDLER_FLAG (1UL << 4)
#define DUMMY_ATTLIST_DECL_HANDLER_FLAG (1UL << 5)
#define DUMMY_COMMENT_HANDLER_FLAG (1UL << 6)
#define DUMMY_PI_HANDLER_FLAG (1UL << 7)
#define DUMMY_START_ELEMENT_HANDLER_FLAG (1UL << 8)
#define DUMMY_START_CDATA_HANDLER_FLAG (1UL << 9)
#define DUMMY_END_CDATA_HANDLER_FLAG (1UL << 10)
#define DUMMY_UNPARSED_ENTITY_DECL_HANDLER_FLAG (1UL << 11)
#define DUMMY_START_NS_DECL_HANDLER_FLAG (1UL << 12)
#define DUMMY_END_NS_DECL_HANDLER_FLAG (1UL << 13)
#define DUMMY_START_DOCTYPE_DECL_HANDLER_FLAG (1UL << 14)
#define DUMMY_END_DOCTYPE_DECL_HANDLER_FLAG (1UL << 15)
#define DUMMY_SKIP_HANDLER_FLAG (1UL << 16)
#define DUMMY_DEFAULT_HANDLER_FLAG (1UL << 17)

static void XMLCALL
dummy_xdecl_handler(void *userData, const XML_Char *version,
                    const XML_Char *encoding, int standalone) {
  UNUSED_P(userData);
  UNUSED_P(version);
  UNUSED_P(encoding);
  UNUSED_P(standalone);
}

static void XMLCALL
dummy_start_doctype_handler(void *userData, const XML_Char *doctypeName,
                            const XML_Char *sysid, const XML_Char *pubid,
                            int has_internal_subset) {
  UNUSED_P(userData);
  UNUSED_P(doctypeName);
  UNUSED_P(sysid);
  UNUSED_P(pubid);
  UNUSED_P(has_internal_subset);
  dummy_handler_flags |= DUMMY_START_DOCTYPE_HANDLER_FLAG;
}

static void XMLCALL
dummy_end_doctype_handler(void *userData) {
  UNUSED_P(userData);
  dummy_handler_flags |= DUMMY_END_DOCTYPE_HANDLER_FLAG;
}

static void XMLCALL
dummy_entity_decl_handler(void *userData, const XML_Char *entityName,
                          int is_parameter_entity, const XML_Char *value,
                          int value_length, const XML_Char *base,
                          const XML_Char *systemId, const XML_Char *publicId,
                          const XML_Char *notationName) {
  UNUSED_P(userData);
  UNUSED_P(entityName);
  UNUSED_P(is_parameter_entity);
  UNUSED_P(value);
  UNUSED_P(value_length);
  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  UNUSED_P(notationName);
  dummy_handler_flags |= DUMMY_ENTITY_DECL_HANDLER_FLAG;
}

static void XMLCALL
dummy_notation_decl_handler(void *userData, const XML_Char *notationName,
                            const XML_Char *base, const XML_Char *systemId,
                            const XML_Char *publicId) {
  UNUSED_P(userData);
  UNUSED_P(notationName);
  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  dummy_handler_flags |= DUMMY_NOTATION_DECL_HANDLER_FLAG;
}

static void XMLCALL
dummy_element_decl_handler(void *userData, const XML_Char *name,
                           XML_Content *model) {
  UNUSED_P(userData);
  UNUSED_P(name);
  /* The content model must be freed by the handler.  Unfortunately
   * we cannot pass the parser as the userData because this is used
   * with other handlers that require other userData.
   */
  XML_FreeContentModel(g_parser, model);
  dummy_handler_flags |= DUMMY_ELEMENT_DECL_HANDLER_FLAG;
}

static void XMLCALL
dummy_attlist_decl_handler(void *userData, const XML_Char *elname,
                           const XML_Char *attname, const XML_Char *att_type,
                           const XML_Char *dflt, int isrequired) {
  UNUSED_P(userData);
  UNUSED_P(elname);
  UNUSED_P(attname);
  UNUSED_P(att_type);
  UNUSED_P(dflt);
  UNUSED_P(isrequired);
  dummy_handler_flags |= DUMMY_ATTLIST_DECL_HANDLER_FLAG;
}

static void XMLCALL
dummy_comment_handler(void *userData, const XML_Char *data) {
  UNUSED_P(userData);
  UNUSED_P(data);
  dummy_handler_flags |= DUMMY_COMMENT_HANDLER_FLAG;
}

static void XMLCALL
dummy_pi_handler(void *userData, const XML_Char *target, const XML_Char *data) {
  UNUSED_P(userData);
  UNUSED_P(target);
  UNUSED_P(data);
  dummy_handler_flags |= DUMMY_PI_HANDLER_FLAG;
}

static void XMLCALL
dummy_start_element(void *userData, const XML_Char *name,
                    const XML_Char **atts) {
  UNUSED_P(userData);
  UNUSED_P(name);
  UNUSED_P(atts);
  dummy_handler_flags |= DUMMY_START_ELEMENT_HANDLER_FLAG;
}

static void XMLCALL
dummy_end_element(void *userData, const XML_Char *name) {
  UNUSED_P(userData);
  UNUSED_P(name);
}

static void XMLCALL
dummy_start_cdata_handler(void *userData) {
  UNUSED_P(userData);
  dummy_handler_flags |= DUMMY_START_CDATA_HANDLER_FLAG;
}

static void XMLCALL
dummy_end_cdata_handler(void *userData) {
  UNUSED_P(userData);
  dummy_handler_flags |= DUMMY_END_CDATA_HANDLER_FLAG;
}

static void XMLCALL
dummy_cdata_handler(void *userData, const XML_Char *s, int len) {
  UNUSED_P(userData);
  UNUSED_P(s);
  UNUSED_P(len);
}

static void XMLCALL
dummy_start_namespace_decl_handler(void *userData, const XML_Char *prefix,
                                   const XML_Char *uri) {
  UNUSED_P(userData);
  UNUSED_P(prefix);
  UNUSED_P(uri);
  dummy_handler_flags |= DUMMY_START_NS_DECL_HANDLER_FLAG;
}

static void XMLCALL
dummy_end_namespace_decl_handler(void *userData, const XML_Char *prefix) {
  UNUSED_P(userData);
  UNUSED_P(prefix);
  dummy_handler_flags |= DUMMY_END_NS_DECL_HANDLER_FLAG;
}

/* This handler is obsolete, but while the code exists we should
 * ensure that dealing with the handler is covered by tests.
 */
static void XMLCALL
dummy_unparsed_entity_decl_handler(void *userData, const XML_Char *entityName,
                                   const XML_Char *base,
                                   const XML_Char *systemId,
                                   const XML_Char *publicId,
                                   const XML_Char *notationName) {
  UNUSED_P(userData);
  UNUSED_P(entityName);
  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  UNUSED_P(notationName);
  dummy_handler_flags |= DUMMY_UNPARSED_ENTITY_DECL_HANDLER_FLAG;
}

static void XMLCALL
dummy_default_handler(void *userData, const XML_Char *s, int len) {
  UNUSED_P(userData);
  UNUSED_P(s);
  UNUSED_P(len);
}

static void XMLCALL
dummy_start_doctype_decl_handler(void *userData, const XML_Char *doctypeName,
                                 const XML_Char *sysid, const XML_Char *pubid,
                                 int has_internal_subset) {
  UNUSED_P(userData);
  UNUSED_P(doctypeName);
  UNUSED_P(sysid);
  UNUSED_P(pubid);
  UNUSED_P(has_internal_subset);
  dummy_handler_flags |= DUMMY_START_DOCTYPE_DECL_HANDLER_FLAG;
}

static void XMLCALL
dummy_end_doctype_decl_handler(void *userData) {
  UNUSED_P(userData);
  dummy_handler_flags |= DUMMY_END_DOCTYPE_DECL_HANDLER_FLAG;
}

static void XMLCALL
dummy_skip_handler(void *userData, const XML_Char *entityName,
                   int is_parameter_entity) {
  UNUSED_P(userData);
  UNUSED_P(entityName);
  UNUSED_P(is_parameter_entity);
  dummy_handler_flags |= DUMMY_SKIP_HANDLER_FLAG;
}

/* Useful external entity handler */
typedef struct ExtOption {
  const XML_Char *system_id;
  const char *parse_text;
} ExtOption;

static int XMLCALL
external_entity_optioner(XML_Parser parser, const XML_Char *context,
                         const XML_Char *base, const XML_Char *systemId,
                         const XML_Char *publicId) {
  ExtOption *options = (ExtOption *)XML_GetUserData(parser);
  XML_Parser ext_parser;

  UNUSED_P(base);
  UNUSED_P(publicId);
  while (options->parse_text != NULL) {
    if (! xcstrcmp(systemId, options->system_id)) {
      enum XML_Status rc;
      ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
      if (ext_parser == NULL)
        return XML_STATUS_ERROR;
      rc = _XML_Parse_SINGLE_BYTES(ext_parser, options->parse_text,
                                   (int)strlen(options->parse_text), XML_TRUE);
      XML_ParserFree(ext_parser);
      return rc;
    }
    options++;
  }
  fail("No suitable option found");
  return XML_STATUS_ERROR;
}

/*
 * Parameter entity evaluation support.
 */
#define ENTITY_MATCH_FAIL (-1)
#define ENTITY_MATCH_NOT_FOUND (0)
#define ENTITY_MATCH_SUCCESS (1)
static const XML_Char *entity_name_to_match = NULL;
static const XML_Char *entity_value_to_match = NULL;
static int entity_match_flag = ENTITY_MATCH_NOT_FOUND;

static void XMLCALL
param_entity_match_handler(void *userData, const XML_Char *entityName,
                           int is_parameter_entity, const XML_Char *value,
                           int value_length, const XML_Char *base,
                           const XML_Char *systemId, const XML_Char *publicId,
                           const XML_Char *notationName) {
  UNUSED_P(userData);
  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  UNUSED_P(notationName);
  if (! is_parameter_entity || entity_name_to_match == NULL
      || entity_value_to_match == NULL) {
    return;
  }
  if (! xcstrcmp(entityName, entity_name_to_match)) {
    /* The cast here is safe because we control the horizontal and
     * the vertical, and we therefore know our strings are never
     * going to overflow an int.
     */
    if (value_length != (int)xcstrlen(entity_value_to_match)
        || xcstrncmp(value, entity_value_to_match, value_length)) {
      entity_match_flag = ENTITY_MATCH_FAIL;
    } else {
      entity_match_flag = ENTITY_MATCH_SUCCESS;
    }
  }
  /* Else leave the match flag alone */
}

/*
 * Character & encoding tests.
 */

START_TEST(test_nul_byte) {
  char text[] = "<doc>\0</doc>";

  /* test that a NUL byte (in US-ASCII data) is an error */
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_OK)
    fail("Parser did not report error on NUL-byte.");
  if (XML_GetErrorCode(g_parser) != XML_ERROR_INVALID_TOKEN)
    xml_failure(g_parser);
}
END_TEST

START_TEST(test_u0000_char) {
  /* test that a NUL byte (in US-ASCII data) is an error */
  expect_failure("<doc>&#0;</doc>", XML_ERROR_BAD_CHAR_REF,
                 "Parser did not report error on NUL-byte.");
}
END_TEST

START_TEST(test_siphash_self) {
  if (! sip24_valid())
    fail("SipHash self-test failed");
}
END_TEST

START_TEST(test_siphash_spec) {
  /* https://131002.net/siphash/siphash.pdf (page 19, "Test values") */
  const char message[] = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09"
                         "\x0a\x0b\x0c\x0d\x0e";
  const size_t len = sizeof(message) - 1;
  const uint64_t expected = _SIP_ULL(0xa129ca61U, 0x49be45e5U);
  struct siphash state;
  struct sipkey key;

  sip_tokey(&key, "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09"
                  "\x0a\x0b\x0c\x0d\x0e\x0f");
  sip24_init(&state, &key);

  /* Cover spread across calls */
  sip24_update(&state, message, 4);
  sip24_update(&state, message + 4, len - 4);

  /* Cover null length */
  sip24_update(&state, message, 0);

  if (sip24_final(&state) != expected)
    fail("sip24_final failed spec test\n");

  /* Cover wrapper */
  if (siphash24(message, len, &key) != expected)
    fail("siphash24 failed spec test\n");
}
END_TEST

START_TEST(test_bom_utf8) {
  /* This test is really just making sure we don't core on a UTF-8 BOM. */
  const char *text = "\357\273\277<e/>";

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

START_TEST(test_bom_utf16_be) {
  char text[] = "\376\377\0<\0e\0/\0>";

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

START_TEST(test_bom_utf16_le) {
  char text[] = "\377\376<\0e\0/\0>\0";

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Parse whole buffer at once to exercise a different code path */
START_TEST(test_nobom_utf16_le) {
  char text[] = " \0<\0e\0/\0>\0";

  if (XML_Parse(g_parser, text, sizeof(text) - 1, XML_TRUE) == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

static void XMLCALL
accumulate_characters(void *userData, const XML_Char *s, int len) {
  CharData_AppendXMLChars((CharData *)userData, s, len);
}

static void XMLCALL
accumulate_attribute(void *userData, const XML_Char *name,
                     const XML_Char **atts) {
  CharData *storage = (CharData *)userData;
  UNUSED_P(name);
  /* Check there are attributes to deal with */
  if (atts == NULL)
    return;

  while (storage->count < 0 && atts[0] != NULL) {
    /* "accumulate" the value of the first attribute we see */
    CharData_AppendXMLChars(storage, atts[1], -1);
    atts += 2;
  }
}

static void
_run_character_check(const char *text, const XML_Char *expected,
                     const char *file, int line) {
  CharData storage;

  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  XML_SetCharacterDataHandler(g_parser, accumulate_characters);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    _xml_failure(g_parser, file, line);
  CharData_CheckXMLChars(&storage, expected);
}

#define run_character_check(text, expected)                                    \
  _run_character_check(text, expected, __FILE__, __LINE__)

static void
_run_attribute_check(const char *text, const XML_Char *expected,
                     const char *file, int line) {
  CharData storage;

  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  XML_SetStartElementHandler(g_parser, accumulate_attribute);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    _xml_failure(g_parser, file, line);
  CharData_CheckXMLChars(&storage, expected);
}

#define run_attribute_check(text, expected)                                    \
  _run_attribute_check(text, expected, __FILE__, __LINE__)

typedef struct ExtTest {
  const char *parse_text;
  const XML_Char *encoding;
  CharData *storage;
} ExtTest;

static void XMLCALL
ext_accumulate_characters(void *userData, const XML_Char *s, int len) {
  ExtTest *test_data = (ExtTest *)userData;
  accumulate_characters(test_data->storage, s, len);
}

static void
_run_ext_character_check(const char *text, ExtTest *test_data,
                         const XML_Char *expected, const char *file, int line) {
  CharData *const storage = (CharData *)malloc(sizeof(CharData));

  CharData_Init(storage);
  test_data->storage = storage;
  XML_SetUserData(g_parser, test_data);
  XML_SetCharacterDataHandler(g_parser, ext_accumulate_characters);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    _xml_failure(g_parser, file, line);
  CharData_CheckXMLChars(storage, expected);

  free(storage);
}

#define run_ext_character_check(text, test_data, expected)                     \
  _run_ext_character_check(text, test_data, expected, __FILE__, __LINE__)

/* Regression test for SF bug #491986. */
START_TEST(test_danish_latin1) {
  const char *text = "<?xml version='1.0' encoding='iso-8859-1'?>\n"
                     "<e>J\xF8rgen \xE6\xF8\xE5\xC6\xD8\xC5</e>";
#ifdef XML_UNICODE
  const XML_Char *expected
      = XCS("J\x00f8rgen \x00e6\x00f8\x00e5\x00c6\x00d8\x00c5");
#else
  const XML_Char *expected
      = XCS("J\xC3\xB8rgen \xC3\xA6\xC3\xB8\xC3\xA5\xC3\x86\xC3\x98\xC3\x85");
#endif
  run_character_check(text, expected);
}
END_TEST

/* Regression test for SF bug #514281. */
START_TEST(test_french_charref_hexidecimal) {
  const char *text = "<?xml version='1.0' encoding='iso-8859-1'?>\n"
                     "<doc>&#xE9;&#xE8;&#xE0;&#xE7;&#xEA;&#xC8;</doc>";
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\x00e9\x00e8\x00e0\x00e7\x00ea\x00c8");
#else
  const XML_Char *expected
      = XCS("\xC3\xA9\xC3\xA8\xC3\xA0\xC3\xA7\xC3\xAA\xC3\x88");
#endif
  run_character_check(text, expected);
}
END_TEST

START_TEST(test_french_charref_decimal) {
  const char *text = "<?xml version='1.0' encoding='iso-8859-1'?>\n"
                     "<doc>&#233;&#232;&#224;&#231;&#234;&#200;</doc>";
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\x00e9\x00e8\x00e0\x00e7\x00ea\x00c8");
#else
  const XML_Char *expected
      = XCS("\xC3\xA9\xC3\xA8\xC3\xA0\xC3\xA7\xC3\xAA\xC3\x88");
#endif
  run_character_check(text, expected);
}
END_TEST

START_TEST(test_french_latin1) {
  const char *text = "<?xml version='1.0' encoding='iso-8859-1'?>\n"
                     "<doc>\xE9\xE8\xE0\xE7\xEa\xC8</doc>";
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\x00e9\x00e8\x00e0\x00e7\x00ea\x00c8");
#else
  const XML_Char *expected
      = XCS("\xC3\xA9\xC3\xA8\xC3\xA0\xC3\xA7\xC3\xAA\xC3\x88");
#endif
  run_character_check(text, expected);
}
END_TEST

START_TEST(test_french_utf8) {
  const char *text = "<?xml version='1.0' encoding='utf-8'?>\n"
                     "<doc>\xC3\xA9</doc>";
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\x00e9");
#else
  const XML_Char *expected = XCS("\xC3\xA9");
#endif
  run_character_check(text, expected);
}
END_TEST

/* Regression test for SF bug #600479.
   XXX There should be a test that exercises all legal XML Unicode
   characters as PCDATA and attribute value content, and XML Name
   characters as part of element and attribute names.
*/
START_TEST(test_utf8_false_rejection) {
  const char *text = "<doc>\xEF\xBA\xBF</doc>";
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\xfebf");
#else
  const XML_Char *expected = XCS("\xEF\xBA\xBF");
#endif
  run_character_check(text, expected);
}
END_TEST

/* Regression test for SF bug #477667.
   This test assures that any 8-bit character followed by a 7-bit
   character will not be mistakenly interpreted as a valid UTF-8
   sequence.
*/
START_TEST(test_illegal_utf8) {
  char text[100];
  int i;

  for (i = 128; i <= 255; ++i) {
    sprintf(text, "<e>%ccd</e>", i);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        == XML_STATUS_OK) {
      sprintf(text, "expected token error for '%c' (ordinal %d) in UTF-8 text",
              i, i);
      fail(text);
    } else if (XML_GetErrorCode(g_parser) != XML_ERROR_INVALID_TOKEN)
      xml_failure(g_parser);
    /* Reset the parser since we use the same parser repeatedly. */
    XML_ParserReset(g_parser, NULL);
  }
}
END_TEST

/* Examples, not masks: */
#define UTF8_LEAD_1 "\x7f" /* 0b01111111 */
#define UTF8_LEAD_2 "\xdf" /* 0b11011111 */
#define UTF8_LEAD_3 "\xef" /* 0b11101111 */
#define UTF8_LEAD_4 "\xf7" /* 0b11110111 */
#define UTF8_FOLLOW "\xbf" /* 0b10111111 */

START_TEST(test_utf8_auto_align) {
  struct TestCase {
    ptrdiff_t expectedMovementInChars;
    const char *input;
  };

  struct TestCase cases[] = {
      {00, ""},

      {00, UTF8_LEAD_1},

      {-1, UTF8_LEAD_2},
      {00, UTF8_LEAD_2 UTF8_FOLLOW},

      {-1, UTF8_LEAD_3},
      {-2, UTF8_LEAD_3 UTF8_FOLLOW},
      {00, UTF8_LEAD_3 UTF8_FOLLOW UTF8_FOLLOW},

      {-1, UTF8_LEAD_4},
      {-2, UTF8_LEAD_4 UTF8_FOLLOW},
      {-3, UTF8_LEAD_4 UTF8_FOLLOW UTF8_FOLLOW},
      {00, UTF8_LEAD_4 UTF8_FOLLOW UTF8_FOLLOW UTF8_FOLLOW},
  };

  size_t i = 0;
  bool success = true;
  for (; i < sizeof(cases) / sizeof(*cases); i++) {
    const char *fromLim = cases[i].input + strlen(cases[i].input);
    const char *const fromLimInitially = fromLim;
    ptrdiff_t actualMovementInChars;

    _INTERNAL_trim_to_complete_utf8_characters(cases[i].input, &fromLim);

    actualMovementInChars = (fromLim - fromLimInitially);
    if (actualMovementInChars != cases[i].expectedMovementInChars) {
      size_t j = 0;
      success = false;
      printf("[-] UTF-8 case %2u: Expected movement by %2d chars"
             ", actually moved by %2d chars: \"",
             (unsigned)(i + 1), (int)cases[i].expectedMovementInChars,
             (int)actualMovementInChars);
      for (; j < strlen(cases[i].input); j++) {
        printf("\\x%02x", (unsigned char)cases[i].input[j]);
      }
      printf("\"\n");
    }
  }

  if (! success) {
    fail("UTF-8 auto-alignment is not bullet-proof\n");
  }
}
END_TEST

START_TEST(test_utf16) {
  /* <?xml version="1.0" encoding="UTF-16"?>
   *  <doc a='123'>some {A} text</doc>
   *
   * where {A} is U+FF21, FULLWIDTH LATIN CAPITAL LETTER A
   */
  char text[]
      = "\000<\000?\000x\000m\000\154\000 \000v\000e\000r\000s\000i\000o"
        "\000n\000=\000'\0001\000.\000\060\000'\000 \000e\000n\000c\000o"
        "\000d\000i\000n\000g\000=\000'\000U\000T\000F\000-\0001\000\066"
        "\000'\000?\000>\000\n"
        "\000<\000d\000o\000c\000 \000a\000=\000'\0001\0002\0003\000'\000>"
        "\000s\000o\000m\000e\000 \xff\x21\000 \000t\000e\000x\000t\000"
        "<\000/\000d\000o\000c\000>";
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("some \xff21 text");
#else
  const XML_Char *expected = XCS("some \357\274\241 text");
#endif
  CharData storage;

  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  XML_SetCharacterDataHandler(g_parser, accumulate_characters);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

START_TEST(test_utf16_le_epilog_newline) {
  unsigned int first_chunk_bytes = 17;
  char text[] = "\xFF\xFE"                  /* BOM */
                "<\000e\000/\000>\000"      /* document element */
                "\r\000\n\000\r\000\n\000"; /* epilog */

  if (first_chunk_bytes >= sizeof(text) - 1)
    fail("bad value of first_chunk_bytes");
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, first_chunk_bytes, XML_FALSE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  else {
    enum XML_Status rc;
    rc = _XML_Parse_SINGLE_BYTES(g_parser, text + first_chunk_bytes,
                                 sizeof(text) - first_chunk_bytes - 1,
                                 XML_TRUE);
    if (rc == XML_STATUS_ERROR)
      xml_failure(g_parser);
  }
}
END_TEST

/* Test that an outright lie in the encoding is faulted */
START_TEST(test_not_utf16) {
  const char *text = "<?xml version='1.0' encoding='utf-16'?>"
                     "<doc>Hi</doc>";

  /* Use a handler to provoke the appropriate code paths */
  XML_SetXmlDeclHandler(g_parser, dummy_xdecl_handler);
  expect_failure(text, XML_ERROR_INCORRECT_ENCODING,
                 "UTF-16 declared in UTF-8 not faulted");
}
END_TEST

/* Test that an unknown encoding is rejected */
START_TEST(test_bad_encoding) {
  const char *text = "<doc>Hi</doc>";

  if (! XML_SetEncoding(g_parser, XCS("unknown-encoding")))
    fail("XML_SetEncoding failed");
  expect_failure(text, XML_ERROR_UNKNOWN_ENCODING,
                 "Unknown encoding not faulted");
}
END_TEST

/* Regression test for SF bug #481609, #774028. */
START_TEST(test_latin1_umlauts) {
  const char *text
      = "<?xml version='1.0' encoding='iso-8859-1'?>\n"
        "<e a='\xE4 \xF6 \xFC &#228; &#246; &#252; &#x00E4; &#x0F6; &#xFC; >'\n"
        "  >\xE4 \xF6 \xFC &#228; &#246; &#252; &#x00E4; &#x0F6; &#xFC; ></e>";
#ifdef XML_UNICODE
  /* Expected results in UTF-16 */
  const XML_Char *expected = XCS("\x00e4 \x00f6 \x00fc ")
      XCS("\x00e4 \x00f6 \x00fc ") XCS("\x00e4 \x00f6 \x00fc >");
#else
  /* Expected results in UTF-8 */
  const XML_Char *expected = XCS("\xC3\xA4 \xC3\xB6 \xC3\xBC ")
      XCS("\xC3\xA4 \xC3\xB6 \xC3\xBC ") XCS("\xC3\xA4 \xC3\xB6 \xC3\xBC >");
#endif

  run_character_check(text, expected);
  XML_ParserReset(g_parser, NULL);
  run_attribute_check(text, expected);
  /* Repeat with a default handler */
  XML_ParserReset(g_parser, NULL);
  XML_SetDefaultHandler(g_parser, dummy_default_handler);
  run_character_check(text, expected);
  XML_ParserReset(g_parser, NULL);
  XML_SetDefaultHandler(g_parser, dummy_default_handler);
  run_attribute_check(text, expected);
}
END_TEST

/* Test that an element name with a 4-byte UTF-8 character is rejected */
START_TEST(test_long_utf8_character) {
  const char *text
      = "<?xml version='1.0' encoding='utf-8'?>\n"
        /* 0xf0 0x90 0x80 0x80 = U+10000, the first Linear B character */
        "<do\xf0\x90\x80\x80/>";
  expect_failure(text, XML_ERROR_INVALID_TOKEN,
                 "4-byte UTF-8 character in element name not faulted");
}
END_TEST

/* Test that a long latin-1 attribute (too long to convert in one go)
 * is correctly converted
 */
START_TEST(test_long_latin1_attribute) {
  const char *text
      = "<?xml version='1.0' encoding='iso-8859-1'?>\n"
        "<doc att='"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        /* Last character splits across a buffer boundary */
        "\xe4'>\n</doc>";

  const XML_Char *expected =
      /* 64 characters per line */
      /* clang-format off */
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO")
  /* clang-format on */
#ifdef XML_UNICODE
                                                  XCS("\x00e4");
#else
                                                  XCS("\xc3\xa4");
#endif

  run_attribute_check(text, expected);
}
END_TEST

/* Test that a long ASCII attribute (too long to convert in one go)
 * is correctly converted
 */
START_TEST(test_long_ascii_attribute) {
  const char *text
      = "<?xml version='1.0' encoding='us-ascii'?>\n"
        "<doc att='"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "01234'>\n</doc>";
  const XML_Char *expected =
      /* 64 characters per line */
      /* clang-format off */
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("01234");
  /* clang-format on */

  run_attribute_check(text, expected);
}
END_TEST

/* Regression test #1 for SF bug #653180. */
START_TEST(test_line_number_after_parse) {
  const char *text = "<tag>\n"
                     "\n"
                     "\n</tag>";
  XML_Size lineno;

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_FALSE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  lineno = XML_GetCurrentLineNumber(g_parser);
  if (lineno != 4) {
    char buffer[100];
    sprintf(buffer, "expected 4 lines, saw %" XML_FMT_INT_MOD "u", lineno);
    fail(buffer);
  }
}
END_TEST

/* Regression test #2 for SF bug #653180. */
START_TEST(test_column_number_after_parse) {
  const char *text = "<tag></tag>";
  XML_Size colno;

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_FALSE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  colno = XML_GetCurrentColumnNumber(g_parser);
  if (colno != 11) {
    char buffer[100];
    sprintf(buffer, "expected 11 columns, saw %" XML_FMT_INT_MOD "u", colno);
    fail(buffer);
  }
}
END_TEST

#define STRUCT_START_TAG 0
#define STRUCT_END_TAG 1
static void XMLCALL
start_element_event_handler2(void *userData, const XML_Char *name,
                             const XML_Char **attr) {
  StructData *storage = (StructData *)userData;
  UNUSED_P(attr);
  StructData_AddItem(storage, name, XML_GetCurrentColumnNumber(g_parser),
                     XML_GetCurrentLineNumber(g_parser), STRUCT_START_TAG);
}

static void XMLCALL
end_element_event_handler2(void *userData, const XML_Char *name) {
  StructData *storage = (StructData *)userData;
  StructData_AddItem(storage, name, XML_GetCurrentColumnNumber(g_parser),
                     XML_GetCurrentLineNumber(g_parser), STRUCT_END_TAG);
}

/* Regression test #3 for SF bug #653180. */
START_TEST(test_line_and_column_numbers_inside_handlers) {
  const char *text = "<a>\n"      /* Unix end-of-line */
                     "  <b>\r\n"  /* Windows end-of-line */
                     "    <c/>\r" /* Mac OS end-of-line */
                     "  </b>\n"
                     "  <d>\n"
                     "    <f/>\n"
                     "  </d>\n"
                     "</a>";
  const StructDataEntry expected[]
      = {{XCS("a"), 0, 1, STRUCT_START_TAG}, {XCS("b"), 2, 2, STRUCT_START_TAG},
         {XCS("c"), 4, 3, STRUCT_START_TAG}, {XCS("c"), 8, 3, STRUCT_END_TAG},
         {XCS("b"), 2, 4, STRUCT_END_TAG},   {XCS("d"), 2, 5, STRUCT_START_TAG},
         {XCS("f"), 4, 6, STRUCT_START_TAG}, {XCS("f"), 8, 6, STRUCT_END_TAG},
         {XCS("d"), 2, 7, STRUCT_END_TAG},   {XCS("a"), 0, 8, STRUCT_END_TAG}};
  const int expected_count = sizeof(expected) / sizeof(StructDataEntry);
  StructData storage;

  StructData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  XML_SetStartElementHandler(g_parser, start_element_event_handler2);
  XML_SetEndElementHandler(g_parser, end_element_event_handler2);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);

  StructData_CheckItems(&storage, expected, expected_count);
  StructData_Dispose(&storage);
}
END_TEST

/* Regression test #4 for SF bug #653180. */
START_TEST(test_line_number_after_error) {
  const char *text = "<a>\n"
                     "  <b>\n"
                     "  </a>"; /* missing </b> */
  XML_Size lineno;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_FALSE)
      != XML_STATUS_ERROR)
    fail("Expected a parse error");

  lineno = XML_GetCurrentLineNumber(g_parser);
  if (lineno != 3) {
    char buffer[100];
    sprintf(buffer, "expected 3 lines, saw %" XML_FMT_INT_MOD "u", lineno);
    fail(buffer);
  }
}
END_TEST

/* Regression test #5 for SF bug #653180. */
START_TEST(test_column_number_after_error) {
  const char *text = "<a>\n"
                     "  <b>\n"
                     "  </a>"; /* missing </b> */
  XML_Size colno;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_FALSE)
      != XML_STATUS_ERROR)
    fail("Expected a parse error");

  colno = XML_GetCurrentColumnNumber(g_parser);
  if (colno != 4) {
    char buffer[100];
    sprintf(buffer, "expected 4 columns, saw %" XML_FMT_INT_MOD "u", colno);
    fail(buffer);
  }
}
END_TEST

/* Regression test for SF bug #478332. */
START_TEST(test_really_long_lines) {
  /* This parses an input line longer than INIT_DATA_BUF_SIZE
     characters long (defined to be 1024 in xmlparse.c).  We take a
     really cheesy approach to building the input buffer, because
     this avoids writing bugs in buffer-filling code.
  */
  const char *text
      = "<e>"
        /* 64 chars */
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        /* until we have at least 1024 characters on the line: */
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "</e>";
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Test cdata processing across a buffer boundary */
START_TEST(test_really_long_encoded_lines) {
  /* As above, except that we want to provoke an output buffer
   * overflow with a non-trivial encoding.  For this we need to pass
   * the whole cdata in one go, not byte-by-byte.
   */
  void *buffer;
  const char *text
      = "<?xml version='1.0' encoding='iso-8859-1'?>"
        "<e>"
        /* 64 chars */
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        /* until we have at least 1024 characters on the line: */
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-+"
        "</e>";
  int parse_len = (int)strlen(text);

  /* Need a cdata handler to provoke the code path we want to test */
  XML_SetCharacterDataHandler(g_parser, dummy_cdata_handler);
  buffer = XML_GetBuffer(g_parser, parse_len);
  if (buffer == NULL)
    fail("Could not allocate parse buffer");
  assert(buffer != NULL);
  memcpy(buffer, text, parse_len);
  if (XML_ParseBuffer(g_parser, parse_len, XML_TRUE) == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/*
 * Element event tests.
 */

static void XMLCALL
start_element_event_handler(void *userData, const XML_Char *name,
                            const XML_Char **atts) {
  UNUSED_P(atts);
  CharData_AppendXMLChars((CharData *)userData, name, -1);
}

static void XMLCALL
end_element_event_handler(void *userData, const XML_Char *name) {
  CharData *storage = (CharData *)userData;
  CharData_AppendXMLChars(storage, XCS("/"), 1);
  CharData_AppendXMLChars(storage, name, -1);
}

START_TEST(test_end_element_events) {
  const char *text = "<a><b><c/></b><d><f/></d></a>";
  const XML_Char *expected = XCS("/c/b/f/d/a");
  CharData storage;

  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  XML_SetEndElementHandler(g_parser, end_element_event_handler);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

/*
 * Attribute tests.
 */

/* Helpers used by the following test; this checks any "attr" and "refs"
   attributes to make sure whitespace has been normalized.

   Return true if whitespace has been normalized in a string, using
   the rules for attribute value normalization.  The 'is_cdata' flag
   is needed since CDATA attributes don't need to have multiple
   whitespace characters collapsed to a single space, while other
   attribute data types do.  (Section 3.3.3 of the recommendation.)
*/
static int
is_whitespace_normalized(const XML_Char *s, int is_cdata) {
  int blanks = 0;
  int at_start = 1;
  while (*s) {
    if (*s == XCS(' '))
      ++blanks;
    else if (*s == XCS('\t') || *s == XCS('\n') || *s == XCS('\r'))
      return 0;
    else {
      if (at_start) {
        at_start = 0;
        if (blanks && ! is_cdata)
          /* illegal leading blanks */
          return 0;
      } else if (blanks > 1 && ! is_cdata)
        return 0;
      blanks = 0;
    }
    ++s;
  }
  if (blanks && ! is_cdata)
    return 0;
  return 1;
}

/* Check the attribute whitespace checker: */
static void
testhelper_is_whitespace_normalized(void) {
  assert(is_whitespace_normalized(XCS("abc"), 0));
  assert(is_whitespace_normalized(XCS("abc"), 1));
  assert(is_whitespace_normalized(XCS("abc def ghi"), 0));
  assert(is_whitespace_normalized(XCS("abc def ghi"), 1));
  assert(! is_whitespace_normalized(XCS(" abc def ghi"), 0));
  assert(is_whitespace_normalized(XCS(" abc def ghi"), 1));
  assert(! is_whitespace_normalized(XCS("abc  def ghi"), 0));
  assert(is_whitespace_normalized(XCS("abc  def ghi"), 1));
  assert(! is_whitespace_normalized(XCS("abc def ghi "), 0));
  assert(is_whitespace_normalized(XCS("abc def ghi "), 1));
  assert(! is_whitespace_normalized(XCS(" "), 0));
  assert(is_whitespace_normalized(XCS(" "), 1));
  assert(! is_whitespace_normalized(XCS("\t"), 0));
  assert(! is_whitespace_normalized(XCS("\t"), 1));
  assert(! is_whitespace_normalized(XCS("\n"), 0));
  assert(! is_whitespace_normalized(XCS("\n"), 1));
  assert(! is_whitespace_normalized(XCS("\r"), 0));
  assert(! is_whitespace_normalized(XCS("\r"), 1));
  assert(! is_whitespace_normalized(XCS("abc\t def"), 1));
}

static void XMLCALL
check_attr_contains_normalized_whitespace(void *userData, const XML_Char *name,
                                          const XML_Char **atts) {
  int i;
  UNUSED_P(userData);
  UNUSED_P(name);
  for (i = 0; atts[i] != NULL; i += 2) {
    const XML_Char *attrname = atts[i];
    const XML_Char *value = atts[i + 1];
    if (xcstrcmp(XCS("attr"), attrname) == 0
        || xcstrcmp(XCS("ents"), attrname) == 0
        || xcstrcmp(XCS("refs"), attrname) == 0) {
      if (! is_whitespace_normalized(value, 0)) {
        char buffer[256];
        sprintf(buffer,
                "attribute value not normalized: %" XML_FMT_STR
                "='%" XML_FMT_STR "'",
                attrname, value);
        fail(buffer);
      }
    }
  }
}

START_TEST(test_attr_whitespace_normalization) {
  const char *text
      = "<!DOCTYPE doc [\n"
        "  <!ATTLIST doc\n"
        "            attr NMTOKENS #REQUIRED\n"
        "            ents ENTITIES #REQUIRED\n"
        "            refs IDREFS   #REQUIRED>\n"
        "]>\n"
        "<doc attr='    a  b c\t\td\te\t' refs=' id-1   \t  id-2\t\t'  \n"
        "     ents=' ent-1   \t\r\n"
        "            ent-2  ' >\n"
        "  <e id='id-1'/>\n"
        "  <e id='id-2'/>\n"
        "</doc>";

  XML_SetStartElementHandler(g_parser,
                             check_attr_contains_normalized_whitespace);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/*
 * XML declaration tests.
 */

START_TEST(test_xmldecl_misplaced) {
  expect_failure("\n"
                 "<?xml version='1.0'?>\n"
                 "<a/>",
                 XML_ERROR_MISPLACED_XML_PI,
                 "failed to report misplaced XML declaration");
}
END_TEST

START_TEST(test_xmldecl_invalid) {
  expect_failure("<?xml version='1.0' \xc3\xa7?>\n<doc/>", XML_ERROR_XML_DECL,
                 "Failed to report invalid XML declaration");
}
END_TEST

START_TEST(test_xmldecl_missing_attr) {
  expect_failure("<?xml ='1.0'?>\n<doc/>\n", XML_ERROR_XML_DECL,
                 "Failed to report missing XML declaration attribute");
}
END_TEST

START_TEST(test_xmldecl_missing_value) {
  expect_failure("<?xml version='1.0' encoding='us-ascii' standalone?>\n"
                 "<doc/>",
                 XML_ERROR_XML_DECL,
                 "Failed to report missing attribute value");
}
END_TEST

/* Regression test for SF bug #584832. */
static int XMLCALL
UnknownEncodingHandler(void *data, const XML_Char *encoding,
                       XML_Encoding *info) {
  UNUSED_P(data);
  if (xcstrcmp(encoding, XCS("unsupported-encoding")) == 0) {
    int i;
    for (i = 0; i < 256; ++i)
      info->map[i] = i;
    info->data = NULL;
    info->convert = NULL;
    info->release = NULL;
    return XML_STATUS_OK;
  }
  return XML_STATUS_ERROR;
}

START_TEST(test_unknown_encoding_internal_entity) {
  const char *text = "<?xml version='1.0' encoding='unsupported-encoding'?>\n"
                     "<!DOCTYPE test [<!ENTITY foo 'bar'>]>\n"
                     "<test a='&foo;'/>";

  XML_SetUnknownEncodingHandler(g_parser, UnknownEncodingHandler, NULL);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Test unrecognised encoding handler */
static void
dummy_release(void *data) {
  UNUSED_P(data);
}

static int XMLCALL
UnrecognisedEncodingHandler(void *data, const XML_Char *encoding,
                            XML_Encoding *info) {
  UNUSED_P(data);
  UNUSED_P(encoding);
  info->data = NULL;
  info->convert = NULL;
  info->release = dummy_release;
  return XML_STATUS_ERROR;
}

START_TEST(test_unrecognised_encoding_internal_entity) {
  const char *text = "<?xml version='1.0' encoding='unsupported-encoding'?>\n"
                     "<!DOCTYPE test [<!ENTITY foo 'bar'>]>\n"
                     "<test a='&foo;'/>";

  XML_SetUnknownEncodingHandler(g_parser, UnrecognisedEncodingHandler, NULL);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_ERROR)
    fail("Unrecognised encoding not rejected");
}
END_TEST

/* Regression test for SF bug #620106. */
static int XMLCALL
external_entity_loader(XML_Parser parser, const XML_Char *context,
                       const XML_Char *base, const XML_Char *systemId,
                       const XML_Char *publicId) {
  ExtTest *test_data = (ExtTest *)XML_GetUserData(parser);
  XML_Parser extparser;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  extparser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (extparser == NULL)
    fail("Could not create external entity parser.");
  if (test_data->encoding != NULL) {
    if (! XML_SetEncoding(extparser, test_data->encoding))
      fail("XML_SetEncoding() ignored for external entity");
  }
  if (_XML_Parse_SINGLE_BYTES(extparser, test_data->parse_text,
                              (int)strlen(test_data->parse_text), XML_TRUE)
      == XML_STATUS_ERROR) {
    xml_failure(extparser);
    return XML_STATUS_ERROR;
  }
  XML_ParserFree(extparser);
  return XML_STATUS_OK;
}

START_TEST(test_ext_entity_set_encoding) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/dummy.ent'>\n"
                     "]>\n"
                     "<doc>&en;</doc>";
  ExtTest test_data
      = {/* This text says it's an unsupported encoding, but it's really
            UTF-8, which we tell Expat using XML_SetEncoding().
         */
         "<?xml encoding='iso-8859-3'?>\xC3\xA9", XCS("utf-8"), NULL};
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\x00e9");
#else
  const XML_Char *expected = XCS("\xc3\xa9");
#endif

  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader);
  run_ext_character_check(text, &test_data, expected);
}
END_TEST

/* Test external entities with no handler */
START_TEST(test_ext_entity_no_handler) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/dummy.ent'>\n"
                     "]>\n"
                     "<doc>&en;</doc>";

  XML_SetDefaultHandler(g_parser, dummy_default_handler);
  run_character_check(text, XCS(""));
}
END_TEST

/* Test UTF-8 BOM is accepted */
START_TEST(test_ext_entity_set_bom) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/dummy.ent'>\n"
                     "]>\n"
                     "<doc>&en;</doc>";
  ExtTest test_data = {"\xEF\xBB\xBF" /* BOM */
                       "<?xml encoding='iso-8859-3'?>"
                       "\xC3\xA9",
                       XCS("utf-8"), NULL};
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\x00e9");
#else
  const XML_Char *expected = XCS("\xc3\xa9");
#endif

  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader);
  run_ext_character_check(text, &test_data, expected);
}
END_TEST

/* Test that bad encodings are faulted */
typedef struct ext_faults {
  const char *parse_text;
  const char *fail_text;
  const XML_Char *encoding;
  enum XML_Error error;
} ExtFaults;

static int XMLCALL
external_entity_faulter(XML_Parser parser, const XML_Char *context,
                        const XML_Char *base, const XML_Char *systemId,
                        const XML_Char *publicId) {
  XML_Parser ext_parser;
  ExtFaults *fault = (ExtFaults *)XML_GetUserData(parser);

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser");
  if (fault->encoding != NULL) {
    if (! XML_SetEncoding(ext_parser, fault->encoding))
      fail("XML_SetEncoding failed");
  }
  if (_XML_Parse_SINGLE_BYTES(ext_parser, fault->parse_text,
                              (int)strlen(fault->parse_text), XML_TRUE)
      != XML_STATUS_ERROR)
    fail(fault->fail_text);
  if (XML_GetErrorCode(ext_parser) != fault->error)
    xml_failure(ext_parser);

  XML_ParserFree(ext_parser);
  return XML_STATUS_ERROR;
}

START_TEST(test_ext_entity_bad_encoding) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/dummy.ent'>\n"
                     "]>\n"
                     "<doc>&en;</doc>";
  ExtFaults fault
      = {"<?xml encoding='iso-8859-3'?>u", "Unsupported encoding not faulted",
         XCS("unknown"), XML_ERROR_UNKNOWN_ENCODING};

  XML_SetExternalEntityRefHandler(g_parser, external_entity_faulter);
  XML_SetUserData(g_parser, &fault);
  expect_failure(text, XML_ERROR_EXTERNAL_ENTITY_HANDLING,
                 "Bad encoding should not have been accepted");
}
END_TEST

/* Try handing an invalid encoding to an external entity parser */
START_TEST(test_ext_entity_bad_encoding_2) {
  const char *text = "<?xml version='1.0' encoding='us-ascii'?>\n"
                     "<!DOCTYPE doc SYSTEM 'foo'>\n"
                     "<doc>&entity;</doc>";
  ExtFaults fault
      = {"<!ELEMENT doc (#PCDATA)*>", "Unknown encoding not faulted",
         XCS("unknown-encoding"), XML_ERROR_UNKNOWN_ENCODING};

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_faulter);
  XML_SetUserData(g_parser, &fault);
  expect_failure(text, XML_ERROR_EXTERNAL_ENTITY_HANDLING,
                 "Bad encoding not faulted in external entity handler");
}
END_TEST

/* Test that no error is reported for unknown entities if we don't
   read an external subset.  This was fixed in Expat 1.95.5.
*/
START_TEST(test_wfc_undeclared_entity_unread_external_subset) {
  const char *text = "<!DOCTYPE doc SYSTEM 'foo'>\n"
                     "<doc>&entity;</doc>";

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Test that an error is reported for unknown entities if we don't
   have an external subset.
*/
START_TEST(test_wfc_undeclared_entity_no_external_subset) {
  expect_failure("<doc>&entity;</doc>", XML_ERROR_UNDEFINED_ENTITY,
                 "Parser did not report undefined entity w/out a DTD.");
}
END_TEST

/* Test that an error is reported for unknown entities if we don't
   read an external subset, but have been declared standalone.
*/
START_TEST(test_wfc_undeclared_entity_standalone) {
  const char *text
      = "<?xml version='1.0' encoding='us-ascii' standalone='yes'?>\n"
        "<!DOCTYPE doc SYSTEM 'foo'>\n"
        "<doc>&entity;</doc>";

  expect_failure(text, XML_ERROR_UNDEFINED_ENTITY,
                 "Parser did not report undefined entity (standalone).");
}
END_TEST

/* Test that an error is reported for unknown entities if we have read
   an external subset, and standalone is true.
*/
START_TEST(test_wfc_undeclared_entity_with_external_subset_standalone) {
  const char *text
      = "<?xml version='1.0' encoding='us-ascii' standalone='yes'?>\n"
        "<!DOCTYPE doc SYSTEM 'foo'>\n"
        "<doc>&entity;</doc>";
  ExtTest test_data = {"<!ELEMENT doc (#PCDATA)*>", NULL, NULL};

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetUserData(g_parser, &test_data);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader);
  expect_failure(text, XML_ERROR_UNDEFINED_ENTITY,
                 "Parser did not report undefined entity (external DTD).");
}
END_TEST

/* Test that external entity handling is not done if the parsing flag
 * is set to UNLESS_STANDALONE
 */
START_TEST(test_entity_with_external_subset_unless_standalone) {
  const char *text
      = "<?xml version='1.0' encoding='us-ascii' standalone='yes'?>\n"
        "<!DOCTYPE doc SYSTEM 'foo'>\n"
        "<doc>&entity;</doc>";
  ExtTest test_data = {"<!ENTITY entity 'bar'>", NULL, NULL};

  XML_SetParamEntityParsing(g_parser,
                            XML_PARAM_ENTITY_PARSING_UNLESS_STANDALONE);
  XML_SetUserData(g_parser, &test_data);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader);
  expect_failure(text, XML_ERROR_UNDEFINED_ENTITY,
                 "Parser did not report undefined entity");
}
END_TEST

/* Test that no error is reported for unknown entities if we have read
   an external subset, and standalone is false.
*/
START_TEST(test_wfc_undeclared_entity_with_external_subset) {
  const char *text = "<?xml version='1.0' encoding='us-ascii'?>\n"
                     "<!DOCTYPE doc SYSTEM 'foo'>\n"
                     "<doc>&entity;</doc>";
  ExtTest test_data = {"<!ELEMENT doc (#PCDATA)*>", NULL, NULL};

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader);
  run_ext_character_check(text, &test_data, XCS(""));
}
END_TEST

/* Test that an error is reported if our NotStandalone handler fails */
static int XMLCALL
reject_not_standalone_handler(void *userData) {
  UNUSED_P(userData);
  return XML_STATUS_ERROR;
}

START_TEST(test_not_standalone_handler_reject) {
  const char *text = "<?xml version='1.0' encoding='us-ascii'?>\n"
                     "<!DOCTYPE doc SYSTEM 'foo'>\n"
                     "<doc>&entity;</doc>";
  ExtTest test_data = {"<!ELEMENT doc (#PCDATA)*>", NULL, NULL};

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetUserData(g_parser, &test_data);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader);
  XML_SetNotStandaloneHandler(g_parser, reject_not_standalone_handler);
  expect_failure(text, XML_ERROR_NOT_STANDALONE,
                 "NotStandalone handler failed to reject");

  /* Try again but without external entity handling */
  XML_ParserReset(g_parser, NULL);
  XML_SetNotStandaloneHandler(g_parser, reject_not_standalone_handler);
  expect_failure(text, XML_ERROR_NOT_STANDALONE,
                 "NotStandalone handler failed to reject");
}
END_TEST

/* Test that no error is reported if our NotStandalone handler succeeds */
static int XMLCALL
accept_not_standalone_handler(void *userData) {
  UNUSED_P(userData);
  return XML_STATUS_OK;
}

START_TEST(test_not_standalone_handler_accept) {
  const char *text = "<?xml version='1.0' encoding='us-ascii'?>\n"
                     "<!DOCTYPE doc SYSTEM 'foo'>\n"
                     "<doc>&entity;</doc>";
  ExtTest test_data = {"<!ELEMENT doc (#PCDATA)*>", NULL, NULL};

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader);
  XML_SetNotStandaloneHandler(g_parser, accept_not_standalone_handler);
  run_ext_character_check(text, &test_data, XCS(""));

  /* Repeat without the external entity handler */
  XML_ParserReset(g_parser, NULL);
  XML_SetNotStandaloneHandler(g_parser, accept_not_standalone_handler);
  run_character_check(text, XCS(""));
}
END_TEST

START_TEST(test_wfc_no_recursive_entity_refs) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY entity '&#38;entity;'>\n"
                     "]>\n"
                     "<doc>&entity;</doc>";

  expect_failure(text, XML_ERROR_RECURSIVE_ENTITY_REF,
                 "Parser did not report recursive entity reference.");
}
END_TEST

/* Test incomplete external entities are faulted */
START_TEST(test_ext_entity_invalid_parse) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/dummy.ent'>\n"
                     "]>\n"
                     "<doc>&en;</doc>";
  const ExtFaults faults[]
      = {{"<", "Incomplete element declaration not faulted", NULL,
          XML_ERROR_UNCLOSED_TOKEN},
         {"<\xe2\x82", /* First two bytes of a three-byte char */
          "Incomplete character not faulted", NULL, XML_ERROR_PARTIAL_CHAR},
         {"<tag>\xe2\x82", "Incomplete character in CDATA not faulted", NULL,
          XML_ERROR_PARTIAL_CHAR},
         {NULL, NULL, NULL, XML_ERROR_NONE}};
  const ExtFaults *fault = faults;

  for (; fault->parse_text != NULL; fault++) {
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_faulter);
    XML_SetUserData(g_parser, (void *)fault);
    expect_failure(text, XML_ERROR_EXTERNAL_ENTITY_HANDLING,
                   "Parser did not report external entity error");
    XML_ParserReset(g_parser, NULL);
  }
}
END_TEST

/* Regression test for SF bug #483514. */
START_TEST(test_dtd_default_handling) {
  const char *text = "<!DOCTYPE doc [\n"
                     "<!ENTITY e SYSTEM 'http://example.org/e'>\n"
                     "<!NOTATION n SYSTEM 'http://example.org/n'>\n"
                     "<!ELEMENT doc EMPTY>\n"
                     "<!ATTLIST doc a CDATA #IMPLIED>\n"
                     "<?pi in dtd?>\n"
                     "<!--comment in dtd-->\n"
                     "]><doc/>";

  XML_SetDefaultHandler(g_parser, accumulate_characters);
  XML_SetStartDoctypeDeclHandler(g_parser, dummy_start_doctype_handler);
  XML_SetEndDoctypeDeclHandler(g_parser, dummy_end_doctype_handler);
  XML_SetEntityDeclHandler(g_parser, dummy_entity_decl_handler);
  XML_SetNotationDeclHandler(g_parser, dummy_notation_decl_handler);
  XML_SetElementDeclHandler(g_parser, dummy_element_decl_handler);
  XML_SetAttlistDeclHandler(g_parser, dummy_attlist_decl_handler);
  XML_SetProcessingInstructionHandler(g_parser, dummy_pi_handler);
  XML_SetCommentHandler(g_parser, dummy_comment_handler);
  XML_SetStartCdataSectionHandler(g_parser, dummy_start_cdata_handler);
  XML_SetEndCdataSectionHandler(g_parser, dummy_end_cdata_handler);
  run_character_check(text, XCS("\n\n\n\n\n\n\n<doc/>"));
}
END_TEST

/* Test handling of attribute declarations */
typedef struct AttTest {
  const char *definition;
  const XML_Char *element_name;
  const XML_Char *attr_name;
  const XML_Char *attr_type;
  const XML_Char *default_value;
  int is_required;
} AttTest;

static void XMLCALL
verify_attlist_decl_handler(void *userData, const XML_Char *element_name,
                            const XML_Char *attr_name,
                            const XML_Char *attr_type,
                            const XML_Char *default_value, int is_required) {
  AttTest *at = (AttTest *)userData;

  if (xcstrcmp(element_name, at->element_name))
    fail("Unexpected element name in attribute declaration");
  if (xcstrcmp(attr_name, at->attr_name))
    fail("Unexpected attribute name in attribute declaration");
  if (xcstrcmp(attr_type, at->attr_type))
    fail("Unexpected attribute type in attribute declaration");
  if ((default_value == NULL && at->default_value != NULL)
      || (default_value != NULL && at->default_value == NULL)
      || (default_value != NULL && xcstrcmp(default_value, at->default_value)))
    fail("Unexpected default value in attribute declaration");
  if (is_required != at->is_required)
    fail("Requirement mismatch in attribute declaration");
}

START_TEST(test_dtd_attr_handling) {
  const char *prolog = "<!DOCTYPE doc [\n"
                       "<!ELEMENT doc EMPTY>\n";
  AttTest attr_data[]
      = {{"<!ATTLIST doc a ( one | two | three ) #REQUIRED>\n"
          "]>"
          "<doc a='two'/>",
          XCS("doc"), XCS("a"),
          XCS("(one|two|three)"), /* Extraneous spaces will be removed */
          NULL, XML_TRUE},
         {"<!NOTATION foo SYSTEM 'http://example.org/foo'>\n"
          "<!ATTLIST doc a NOTATION (foo) #IMPLIED>\n"
          "]>"
          "<doc/>",
          XCS("doc"), XCS("a"), XCS("NOTATION(foo)"), NULL, XML_FALSE},
         {"<!ATTLIST doc a NOTATION (foo) 'bar'>\n"
          "]>"
          "<doc/>",
          XCS("doc"), XCS("a"), XCS("NOTATION(foo)"), XCS("bar"), XML_FALSE},
         {"<!ATTLIST doc a CDATA '\xdb\xb2'>\n"
          "]>"
          "<doc/>",
          XCS("doc"), XCS("a"), XCS("CDATA"),
#ifdef XML_UNICODE
          XCS("\x06f2"),
#else
          XCS("\xdb\xb2"),
#endif
          XML_FALSE},
         {NULL, NULL, NULL, NULL, NULL, XML_FALSE}};
  AttTest *test;

  for (test = attr_data; test->definition != NULL; test++) {
    XML_SetAttlistDeclHandler(g_parser, verify_attlist_decl_handler);
    XML_SetUserData(g_parser, test);
    if (_XML_Parse_SINGLE_BYTES(g_parser, prolog, (int)strlen(prolog),
                                XML_FALSE)
        == XML_STATUS_ERROR)
      xml_failure(g_parser);
    if (_XML_Parse_SINGLE_BYTES(g_parser, test->definition,
                                (int)strlen(test->definition), XML_TRUE)
        == XML_STATUS_ERROR)
      xml_failure(g_parser);
    XML_ParserReset(g_parser, NULL);
  }
}
END_TEST

/* See related SF bug #673791.
   When namespace processing is enabled, setting the namespace URI for
   a prefix is not allowed; this test ensures that it *is* allowed
   when namespace processing is not enabled.
   (See Namespaces in XML, section 2.)
*/
START_TEST(test_empty_ns_without_namespaces) {
  const char *text = "<doc xmlns:prefix='http://example.org/'>\n"
                     "  <e xmlns:prefix=''/>\n"
                     "</doc>";

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Regression test for SF bug #824420.
   Checks that an xmlns:prefix attribute set in an attribute's default
   value isn't misinterpreted.
*/
START_TEST(test_ns_in_attribute_default_without_namespaces) {
  const char *text = "<!DOCTYPE e:element [\n"
                     "  <!ATTLIST e:element\n"
                     "    xmlns:e CDATA 'http://example.org/'>\n"
                     "      ]>\n"
                     "<e:element/>";

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

static const char *long_character_data_text
    = "<?xml version='1.0' encoding='iso-8859-1'?><s>"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "</s>";

static XML_Bool resumable = XML_FALSE;

static void
clearing_aborting_character_handler(void *userData, const XML_Char *s,
                                    int len) {
  UNUSED_P(userData);
  UNUSED_P(s);
  UNUSED_P(len);
  XML_StopParser(g_parser, resumable);
  XML_SetCharacterDataHandler(g_parser, NULL);
}

/* Regression test for SF bug #1515266: missing check of stopped
   parser in doContext() 'for' loop. */
START_TEST(test_stop_parser_between_char_data_calls) {
  /* The sample data must be big enough that there are two calls to
     the character data handler from within the inner "for" loop of
     the XML_TOK_DATA_CHARS case in doContent(), and the character
     handler must stop the parser and clear the character data
     handler.
  */
  const char *text = long_character_data_text;

  XML_SetCharacterDataHandler(g_parser, clearing_aborting_character_handler);
  resumable = XML_FALSE;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_ERROR)
    xml_failure(g_parser);
  if (XML_GetErrorCode(g_parser) != XML_ERROR_ABORTED)
    xml_failure(g_parser);
}
END_TEST

/* Regression test for SF bug #1515266: missing check of stopped
   parser in doContext() 'for' loop. */
START_TEST(test_suspend_parser_between_char_data_calls) {
  /* The sample data must be big enough that there are two calls to
     the character data handler from within the inner "for" loop of
     the XML_TOK_DATA_CHARS case in doContent(), and the character
     handler must stop the parser and clear the character data
     handler.
  */
  const char *text = long_character_data_text;

  XML_SetCharacterDataHandler(g_parser, clearing_aborting_character_handler);
  resumable = XML_TRUE;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_SUSPENDED)
    xml_failure(g_parser);
  if (XML_GetErrorCode(g_parser) != XML_ERROR_NONE)
    xml_failure(g_parser);
  /* Try parsing directly */
  if (XML_Parse(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_ERROR)
    fail("Attempt to continue parse while suspended not faulted");
  if (XML_GetErrorCode(g_parser) != XML_ERROR_SUSPENDED)
    fail("Suspended parse not faulted with correct error");
}
END_TEST

static XML_Bool abortable = XML_FALSE;

static void
parser_stop_character_handler(void *userData, const XML_Char *s, int len) {
  UNUSED_P(userData);
  UNUSED_P(s);
  UNUSED_P(len);
  XML_StopParser(g_parser, resumable);
  XML_SetCharacterDataHandler(g_parser, NULL);
  if (! resumable) {
    /* Check that aborting an aborted parser is faulted */
    if (XML_StopParser(g_parser, XML_FALSE) != XML_STATUS_ERROR)
      fail("Aborting aborted parser not faulted");
    if (XML_GetErrorCode(g_parser) != XML_ERROR_FINISHED)
      xml_failure(g_parser);
  } else if (abortable) {
    /* Check that aborting a suspended parser works */
    if (XML_StopParser(g_parser, XML_FALSE) == XML_STATUS_ERROR)
      xml_failure(g_parser);
  } else {
    /* Check that suspending a suspended parser works */
    if (XML_StopParser(g_parser, XML_TRUE) != XML_STATUS_ERROR)
      fail("Suspending suspended parser not faulted");
    if (XML_GetErrorCode(g_parser) != XML_ERROR_SUSPENDED)
      xml_failure(g_parser);
  }
}

/* Test repeated calls to XML_StopParser are handled correctly */
START_TEST(test_repeated_stop_parser_between_char_data_calls) {
  const char *text = long_character_data_text;

  XML_SetCharacterDataHandler(g_parser, parser_stop_character_handler);
  resumable = XML_FALSE;
  abortable = XML_FALSE;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_ERROR)
    fail("Failed to double-stop parser");

  XML_ParserReset(g_parser, NULL);
  XML_SetCharacterDataHandler(g_parser, parser_stop_character_handler);
  resumable = XML_TRUE;
  abortable = XML_FALSE;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_SUSPENDED)
    fail("Failed to double-suspend parser");

  XML_ParserReset(g_parser, NULL);
  XML_SetCharacterDataHandler(g_parser, parser_stop_character_handler);
  resumable = XML_TRUE;
  abortable = XML_TRUE;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_ERROR)
    fail("Failed to suspend-abort parser");
}
END_TEST

START_TEST(test_good_cdata_ascii) {
  const char *text = "<a><![CDATA[<greeting>Hello, world!</greeting>]]></a>";
  const XML_Char *expected = XCS("<greeting>Hello, world!</greeting>");

  CharData storage;
  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  XML_SetCharacterDataHandler(g_parser, accumulate_characters);
  /* Add start and end handlers for coverage */
  XML_SetStartCdataSectionHandler(g_parser, dummy_start_cdata_handler);
  XML_SetEndCdataSectionHandler(g_parser, dummy_end_cdata_handler);

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);

  /* Try again, this time with a default handler */
  XML_ParserReset(g_parser, NULL);
  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  XML_SetCharacterDataHandler(g_parser, accumulate_characters);
  XML_SetDefaultHandler(g_parser, dummy_default_handler);

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

START_TEST(test_good_cdata_utf16) {
  /* Test data is:
   *   <?xml version='1.0' encoding='utf-16'?>
   *   <a><![CDATA[hello]]></a>
   */
  const char text[]
      = "\0<\0?\0x\0m\0l\0"
        " \0v\0e\0r\0s\0i\0o\0n\0=\0'\0\x31\0.\0\x30\0'\0"
        " \0e\0n\0c\0o\0d\0i\0n\0g\0=\0'\0u\0t\0f\0-\0"
        "1\0"
        "6\0'"
        "\0?\0>\0\n"
        "\0<\0a\0>\0<\0!\0[\0C\0D\0A\0T\0A\0[\0h\0e\0l\0l\0o\0]\0]\0>\0<\0/\0a\0>";
  const XML_Char *expected = XCS("hello");

  CharData storage;
  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  XML_SetCharacterDataHandler(g_parser, accumulate_characters);

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

START_TEST(test_good_cdata_utf16_le) {
  /* Test data is:
   *   <?xml version='1.0' encoding='utf-16'?>
   *   <a><![CDATA[hello]]></a>
   */
  const char text[]
      = "<\0?\0x\0m\0l\0"
        " \0v\0e\0r\0s\0i\0o\0n\0=\0'\0\x31\0.\0\x30\0'\0"
        " \0e\0n\0c\0o\0d\0i\0n\0g\0=\0'\0u\0t\0f\0-\0"
        "1\0"
        "6\0'"
        "\0?\0>\0\n"
        "\0<\0a\0>\0<\0!\0[\0C\0D\0A\0T\0A\0[\0h\0e\0l\0l\0o\0]\0]\0>\0<\0/\0a\0>\0";
  const XML_Char *expected = XCS("hello");

  CharData storage;
  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  XML_SetCharacterDataHandler(g_parser, accumulate_characters);

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

/* Test UTF16 conversion of a long cdata string */

/* 16 characters: handy macro to reduce visual clutter */
#define A_TO_P_IN_UTF16 "\0A\0B\0C\0D\0E\0F\0G\0H\0I\0J\0K\0L\0M\0N\0O\0P"

START_TEST(test_long_cdata_utf16) {
  /* Test data is:
   * <?xlm version='1.0' encoding='utf-16'?>
   * <a><![CDATA[
   * ABCDEFGHIJKLMNOP
   * ]]></a>
   */
  const char text[]
      = "\0<\0?\0x\0m\0l\0 "
        "\0v\0e\0r\0s\0i\0o\0n\0=\0'\0\x31\0.\0\x30\0'\0 "
        "\0e\0n\0c\0o\0d\0i\0n\0g\0=\0'\0u\0t\0f\0-\0\x31\0\x36\0'\0?\0>"
        "\0<\0a\0>\0<\0!\0[\0C\0D\0A\0T\0A\0["
      /* 64 characters per line */
      /* clang-format off */
        A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16
        A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16
        A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16
        A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16
        A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16
        A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16
        A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16
        A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16
        A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16
        A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16
        A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16
        A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16
        A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16
        A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16
        A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16
        A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16  A_TO_P_IN_UTF16
        A_TO_P_IN_UTF16
        /* clang-format on */
        "\0]\0]\0>\0<\0/\0a\0>";
  const XML_Char *expected =
      /* clang-format off */
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP")
        XCS("ABCDEFGHIJKLMNOP");
  /* clang-format on */
  CharData storage;
  void *buffer;

  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  XML_SetCharacterDataHandler(g_parser, accumulate_characters);
  buffer = XML_GetBuffer(g_parser, sizeof(text) - 1);
  if (buffer == NULL)
    fail("Could not allocate parse buffer");
  assert(buffer != NULL);
  memcpy(buffer, text, sizeof(text) - 1);
  if (XML_ParseBuffer(g_parser, sizeof(text) - 1, XML_TRUE) == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

/* Test handling of multiple unit UTF-16 characters */
START_TEST(test_multichar_cdata_utf16) {
  /* Test data is:
   *   <?xml version='1.0' encoding='utf-16'?>
   *   <a><![CDATA[{MINIM}{CROTCHET}]]></a>
   *
   * where {MINIM} is U+1d15e (a minim or half-note)
   *   UTF-16: 0xd834 0xdd5e
   *   UTF-8:  0xf0 0x9d 0x85 0x9e
   * and {CROTCHET} is U+1d15f (a crotchet or quarter-note)
   *   UTF-16: 0xd834 0xdd5f
   *   UTF-8:  0xf0 0x9d 0x85 0x9f
   */
  const char text[] = "\0<\0?\0x\0m\0l\0"
                      " \0v\0e\0r\0s\0i\0o\0n\0=\0'\0\x31\0.\0\x30\0'\0"
                      " \0e\0n\0c\0o\0d\0i\0n\0g\0=\0'\0u\0t\0f\0-\0"
                      "1\0"
                      "6\0'"
                      "\0?\0>\0\n"
                      "\0<\0a\0>\0<\0!\0[\0C\0D\0A\0T\0A\0["
                      "\xd8\x34\xdd\x5e\xd8\x34\xdd\x5f"
                      "\0]\0]\0>\0<\0/\0a\0>";
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\xd834\xdd5e\xd834\xdd5f");
#else
  const XML_Char *expected = XCS("\xf0\x9d\x85\x9e\xf0\x9d\x85\x9f");
#endif
  CharData storage;

  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  XML_SetCharacterDataHandler(g_parser, accumulate_characters);

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

/* Test that an element name with a UTF-16 surrogate pair is rejected */
START_TEST(test_utf16_bad_surrogate_pair) {
  /* Test data is:
   *   <?xml version='1.0' encoding='utf-16'?>
   *   <a><![CDATA[{BADLINB}]]></a>
   *
   * where {BADLINB} is U+10000 (the first Linear B character)
   * with the UTF-16 surrogate pair in the wrong order, i.e.
   *   0xdc00 0xd800
   */
  const char text[] = "\0<\0?\0x\0m\0l\0"
                      " \0v\0e\0r\0s\0i\0o\0n\0=\0'\0\x31\0.\0\x30\0'\0"
                      " \0e\0n\0c\0o\0d\0i\0n\0g\0=\0'\0u\0t\0f\0-\0"
                      "1\0"
                      "6\0'"
                      "\0?\0>\0\n"
                      "\0<\0a\0>\0<\0!\0[\0C\0D\0A\0T\0A\0["
                      "\xdc\x00\xd8\x00"
                      "\0]\0]\0>\0<\0/\0a\0>";

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      != XML_STATUS_ERROR)
    fail("Reversed UTF-16 surrogate pair not faulted");
  if (XML_GetErrorCode(g_parser) != XML_ERROR_INVALID_TOKEN)
    xml_failure(g_parser);
}
END_TEST

START_TEST(test_bad_cdata) {
  struct CaseData {
    const char *text;
    enum XML_Error expectedError;
  };

  struct CaseData cases[]
      = {{"<a><", XML_ERROR_UNCLOSED_TOKEN},
         {"<a><!", XML_ERROR_UNCLOSED_TOKEN},
         {"<a><![", XML_ERROR_UNCLOSED_TOKEN},
         {"<a><![C", XML_ERROR_UNCLOSED_TOKEN},
         {"<a><![CD", XML_ERROR_UNCLOSED_TOKEN},
         {"<a><![CDA", XML_ERROR_UNCLOSED_TOKEN},
         {"<a><![CDAT", XML_ERROR_UNCLOSED_TOKEN},
         {"<a><![CDATA", XML_ERROR_UNCLOSED_TOKEN},

         {"<a><![CDATA[", XML_ERROR_UNCLOSED_CDATA_SECTION},
         {"<a><![CDATA[]", XML_ERROR_UNCLOSED_CDATA_SECTION},
         {"<a><![CDATA[]]", XML_ERROR_UNCLOSED_CDATA_SECTION},

         {"<a><!<a/>", XML_ERROR_INVALID_TOKEN},
         {"<a><![<a/>", XML_ERROR_UNCLOSED_TOKEN},  /* ?! */
         {"<a><![C<a/>", XML_ERROR_UNCLOSED_TOKEN}, /* ?! */
         {"<a><![CD<a/>", XML_ERROR_INVALID_TOKEN},
         {"<a><![CDA<a/>", XML_ERROR_INVALID_TOKEN},
         {"<a><![CDAT<a/>", XML_ERROR_INVALID_TOKEN},
         {"<a><![CDATA<a/>", XML_ERROR_INVALID_TOKEN},

         {"<a><![CDATA[<a/>", XML_ERROR_UNCLOSED_CDATA_SECTION},
         {"<a><![CDATA[]<a/>", XML_ERROR_UNCLOSED_CDATA_SECTION},
         {"<a><![CDATA[]]<a/>", XML_ERROR_UNCLOSED_CDATA_SECTION}};

  size_t i = 0;
  for (; i < sizeof(cases) / sizeof(struct CaseData); i++) {
    const enum XML_Status actualStatus = _XML_Parse_SINGLE_BYTES(
        g_parser, cases[i].text, (int)strlen(cases[i].text), XML_TRUE);
    const enum XML_Error actualError = XML_GetErrorCode(g_parser);

    assert(actualStatus == XML_STATUS_ERROR);

    if (actualError != cases[i].expectedError) {
      char message[100];
      sprintf(message,
              "Expected error %d but got error %d for case %u: \"%s\"\n",
              cases[i].expectedError, actualError, (unsigned int)i + 1,
              cases[i].text);
      fail(message);
    }

    XML_ParserReset(g_parser, NULL);
  }
}
END_TEST

/* Test failures in UTF-16 CDATA */
START_TEST(test_bad_cdata_utf16) {
  struct CaseData {
    size_t text_bytes;
    const char *text;
    enum XML_Error expected_error;
  };

  const char prolog[] = "\0<\0?\0x\0m\0l\0"
                        " \0v\0e\0r\0s\0i\0o\0n\0=\0'\0\x31\0.\0\x30\0'\0"
                        " \0e\0n\0c\0o\0d\0i\0n\0g\0=\0'\0u\0t\0f\0-\0"
                        "1\0"
                        "6\0'"
                        "\0?\0>\0\n"
                        "\0<\0a\0>";
  struct CaseData cases[] = {
      {1, "\0", XML_ERROR_UNCLOSED_TOKEN},
      {2, "\0<", XML_ERROR_UNCLOSED_TOKEN},
      {3, "\0<\0", XML_ERROR_UNCLOSED_TOKEN},
      {4, "\0<\0!", XML_ERROR_UNCLOSED_TOKEN},
      {5, "\0<\0!\0", XML_ERROR_UNCLOSED_TOKEN},
      {6, "\0<\0!\0[", XML_ERROR_UNCLOSED_TOKEN},
      {7, "\0<\0!\0[\0", XML_ERROR_UNCLOSED_TOKEN},
      {8, "\0<\0!\0[\0C", XML_ERROR_UNCLOSED_TOKEN},
      {9, "\0<\0!\0[\0C\0", XML_ERROR_UNCLOSED_TOKEN},
      {10, "\0<\0!\0[\0C\0D", XML_ERROR_UNCLOSED_TOKEN},
      {11, "\0<\0!\0[\0C\0D\0", XML_ERROR_UNCLOSED_TOKEN},
      {12, "\0<\0!\0[\0C\0D\0A", XML_ERROR_UNCLOSED_TOKEN},
      {13, "\0<\0!\0[\0C\0D\0A\0", XML_ERROR_UNCLOSED_TOKEN},
      {14, "\0<\0!\0[\0C\0D\0A\0T", XML_ERROR_UNCLOSED_TOKEN},
      {15, "\0<\0!\0[\0C\0D\0A\0T\0", XML_ERROR_UNCLOSED_TOKEN},
      {16, "\0<\0!\0[\0C\0D\0A\0T\0A", XML_ERROR_UNCLOSED_TOKEN},
      {17, "\0<\0!\0[\0C\0D\0A\0T\0A\0", XML_ERROR_UNCLOSED_TOKEN},
      {18, "\0<\0!\0[\0C\0D\0A\0T\0A\0[", XML_ERROR_UNCLOSED_CDATA_SECTION},
      {19, "\0<\0!\0[\0C\0D\0A\0T\0A\0[\0", XML_ERROR_UNCLOSED_CDATA_SECTION},
      {20, "\0<\0!\0[\0C\0D\0A\0T\0A\0[\0Z", XML_ERROR_UNCLOSED_CDATA_SECTION},
      /* Now add a four-byte UTF-16 character */
      {21, "\0<\0!\0[\0C\0D\0A\0T\0A\0[\0Z\xd8",
       XML_ERROR_UNCLOSED_CDATA_SECTION},
      {22, "\0<\0!\0[\0C\0D\0A\0T\0A\0[\0Z\xd8\x34", XML_ERROR_PARTIAL_CHAR},
      {23, "\0<\0!\0[\0C\0D\0A\0T\0A\0[\0Z\xd8\x34\xdd",
       XML_ERROR_PARTIAL_CHAR},
      {24, "\0<\0!\0[\0C\0D\0A\0T\0A\0[\0Z\xd8\x34\xdd\x5e",
       XML_ERROR_UNCLOSED_CDATA_SECTION}};
  size_t i;

  for (i = 0; i < sizeof(cases) / sizeof(struct CaseData); i++) {
    enum XML_Status actual_status;
    enum XML_Error actual_error;

    if (_XML_Parse_SINGLE_BYTES(g_parser, prolog, (int)sizeof(prolog) - 1,
                                XML_FALSE)
        == XML_STATUS_ERROR)
      xml_failure(g_parser);
    actual_status = _XML_Parse_SINGLE_BYTES(g_parser, cases[i].text,
                                            (int)cases[i].text_bytes, XML_TRUE);
    assert(actual_status == XML_STATUS_ERROR);
    actual_error = XML_GetErrorCode(g_parser);
    if (actual_error != cases[i].expected_error) {
      char message[1024];

      sprintf(message,
              "Expected error %d (%" XML_FMT_STR "), got %d (%" XML_FMT_STR
              ") for case %lu\n",
              cases[i].expected_error, XML_ErrorString(cases[i].expected_error),
              actual_error, XML_ErrorString(actual_error),
              (long unsigned)(i + 1));
      fail(message);
    }
    XML_ParserReset(g_parser, NULL);
  }
}
END_TEST

static const char *long_cdata_text
    = "<s><![CDATA["
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "012345678901234567890123456789012345678901234567890123456789"
      "]]></s>";

/* Test stopping the parser in cdata handler */
START_TEST(test_stop_parser_between_cdata_calls) {
  const char *text = long_cdata_text;

  XML_SetCharacterDataHandler(g_parser, clearing_aborting_character_handler);
  resumable = XML_FALSE;
  expect_failure(text, XML_ERROR_ABORTED, "Parse not aborted in CDATA handler");
}
END_TEST

/* Test suspending the parser in cdata handler */
START_TEST(test_suspend_parser_between_cdata_calls) {
  const char *text = long_cdata_text;
  enum XML_Status result;

  XML_SetCharacterDataHandler(g_parser, clearing_aborting_character_handler);
  resumable = XML_TRUE;
  result = _XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE);
  if (result != XML_STATUS_SUSPENDED) {
    if (result == XML_STATUS_ERROR)
      xml_failure(g_parser);
    fail("Parse not suspended in CDATA handler");
  }
  if (XML_GetErrorCode(g_parser) != XML_ERROR_NONE)
    xml_failure(g_parser);
}
END_TEST

/* Test memory allocation functions */
START_TEST(test_memory_allocation) {
  char *buffer = (char *)XML_MemMalloc(g_parser, 256);
  char *p;

  if (buffer == NULL) {
    fail("Allocation failed");
  } else {
    /* Try writing to memory; some OSes try to cheat! */
    buffer[0] = 'T';
    buffer[1] = 'E';
    buffer[2] = 'S';
    buffer[3] = 'T';
    buffer[4] = '\0';
    if (strcmp(buffer, "TEST") != 0) {
      fail("Memory not writable");
    } else {
      p = (char *)XML_MemRealloc(g_parser, buffer, 512);
      if (p == NULL) {
        fail("Reallocation failed");
      } else {
        /* Write again, just to be sure */
        buffer = p;
        buffer[0] = 'V';
        if (strcmp(buffer, "VEST") != 0) {
          fail("Reallocated memory not writable");
        }
      }
    }
    XML_MemFree(g_parser, buffer);
  }
}
END_TEST

static void XMLCALL
record_default_handler(void *userData, const XML_Char *s, int len) {
  UNUSED_P(s);
  UNUSED_P(len);
  CharData_AppendXMLChars((CharData *)userData, XCS("D"), 1);
}

static void XMLCALL
record_cdata_handler(void *userData, const XML_Char *s, int len) {
  UNUSED_P(s);
  UNUSED_P(len);
  CharData_AppendXMLChars((CharData *)userData, XCS("C"), 1);
  XML_DefaultCurrent(g_parser);
}

static void XMLCALL
record_cdata_nodefault_handler(void *userData, const XML_Char *s, int len) {
  UNUSED_P(s);
  UNUSED_P(len);
  CharData_AppendXMLChars((CharData *)userData, XCS("c"), 1);
}

static void XMLCALL
record_skip_handler(void *userData, const XML_Char *entityName,
                    int is_parameter_entity) {
  UNUSED_P(entityName);
  CharData_AppendXMLChars((CharData *)userData,
                          is_parameter_entity ? XCS("E") : XCS("e"), 1);
}

/* Test XML_DefaultCurrent() passes handling on correctly */
START_TEST(test_default_current) {
  const char *text = "<doc>hell]</doc>";
  const char *entity_text = "<!DOCTYPE doc [\n"
                            "<!ENTITY entity '&#37;'>\n"
                            "]>\n"
                            "<doc>&entity;</doc>";
  CharData storage;

  XML_SetDefaultHandler(g_parser, record_default_handler);
  XML_SetCharacterDataHandler(g_parser, record_cdata_handler);
  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, XCS("DCDCDCDCDCDD"));

  /* Again, without the defaulting */
  XML_ParserReset(g_parser, NULL);
  XML_SetDefaultHandler(g_parser, record_default_handler);
  XML_SetCharacterDataHandler(g_parser, record_cdata_nodefault_handler);
  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, XCS("DcccccD"));

  /* Now with an internal entity to complicate matters */
  XML_ParserReset(g_parser, NULL);
  XML_SetDefaultHandler(g_parser, record_default_handler);
  XML_SetCharacterDataHandler(g_parser, record_cdata_handler);
  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, entity_text, (int)strlen(entity_text),
                              XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  /* The default handler suppresses the entity */
  CharData_CheckXMLChars(&storage, XCS("DDDDDDDDDDDDDDDDDDD"));

  /* Again, with a skip handler */
  XML_ParserReset(g_parser, NULL);
  XML_SetDefaultHandler(g_parser, record_default_handler);
  XML_SetCharacterDataHandler(g_parser, record_cdata_handler);
  XML_SetSkippedEntityHandler(g_parser, record_skip_handler);
  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, entity_text, (int)strlen(entity_text),
                              XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  /* The default handler suppresses the entity */
  CharData_CheckXMLChars(&storage, XCS("DDDDDDDDDDDDDDDDDeD"));

  /* This time, allow the entity through */
  XML_ParserReset(g_parser, NULL);
  XML_SetDefaultHandlerExpand(g_parser, record_default_handler);
  XML_SetCharacterDataHandler(g_parser, record_cdata_handler);
  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, entity_text, (int)strlen(entity_text),
                              XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, XCS("DDDDDDDDDDDDDDDDDCDD"));

  /* Finally, without passing the cdata to the default handler */
  XML_ParserReset(g_parser, NULL);
  XML_SetDefaultHandlerExpand(g_parser, record_default_handler);
  XML_SetCharacterDataHandler(g_parser, record_cdata_nodefault_handler);
  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, entity_text, (int)strlen(entity_text),
                              XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, XCS("DDDDDDDDDDDDDDDDDcD"));
}
END_TEST

/* Test DTD element parsing code paths */
START_TEST(test_dtd_elements) {
  const char *text = "<!DOCTYPE doc [\n"
                     "<!ELEMENT doc (chapter)>\n"
                     "<!ELEMENT chapter (#PCDATA)>\n"
                     "]>\n"
                     "<doc><chapter>Wombats are go</chapter></doc>";

  XML_SetElementDeclHandler(g_parser, dummy_element_decl_handler);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

static void XMLCALL
element_decl_check_model(void *userData, const XML_Char *name,
                         XML_Content *model) {
  UNUSED_P(userData);
  uint32_t errorFlags = 0;

  /* Expected model array structure is this:
   * [0] (type 6, quant 0)
   *   [1] (type 5, quant 0)
   *     [3] (type 4, quant 0, name "bar")
   *     [4] (type 4, quant 0, name "foo")
   *     [5] (type 4, quant 3, name "xyz")
   *   [2] (type 4, quant 2, name "zebra")
   */
  errorFlags |= ((xcstrcmp(name, XCS("junk")) == 0) ? 0 : (1u << 0));
  errorFlags |= ((model != NULL) ? 0 : (1u << 1));

  errorFlags |= ((model[0].type == XML_CTYPE_SEQ) ? 0 : (1u << 2));
  errorFlags |= ((model[0].quant == XML_CQUANT_NONE) ? 0 : (1u << 3));
  errorFlags |= ((model[0].numchildren == 2) ? 0 : (1u << 4));
  errorFlags |= ((model[0].children == &model[1]) ? 0 : (1u << 5));
  errorFlags |= ((model[0].name == NULL) ? 0 : (1u << 6));

  errorFlags |= ((model[1].type == XML_CTYPE_CHOICE) ? 0 : (1u << 7));
  errorFlags |= ((model[1].quant == XML_CQUANT_NONE) ? 0 : (1u << 8));
  errorFlags |= ((model[1].numchildren == 3) ? 0 : (1u << 9));
  errorFlags |= ((model[1].children == &model[3]) ? 0 : (1u << 10));
  errorFlags |= ((model[1].name == NULL) ? 0 : (1u << 11));

  errorFlags |= ((model[2].type == XML_CTYPE_NAME) ? 0 : (1u << 12));
  errorFlags |= ((model[2].quant == XML_CQUANT_REP) ? 0 : (1u << 13));
  errorFlags |= ((model[2].numchildren == 0) ? 0 : (1u << 14));
  errorFlags |= ((model[2].children == NULL) ? 0 : (1u << 15));
  errorFlags |= ((xcstrcmp(model[2].name, XCS("zebra")) == 0) ? 0 : (1u << 16));

  errorFlags |= ((model[3].type == XML_CTYPE_NAME) ? 0 : (1u << 17));
  errorFlags |= ((model[3].quant == XML_CQUANT_NONE) ? 0 : (1u << 18));
  errorFlags |= ((model[3].numchildren == 0) ? 0 : (1u << 19));
  errorFlags |= ((model[3].children == NULL) ? 0 : (1u << 20));
  errorFlags |= ((xcstrcmp(model[3].name, XCS("bar")) == 0) ? 0 : (1u << 21));

  errorFlags |= ((model[4].type == XML_CTYPE_NAME) ? 0 : (1u << 22));
  errorFlags |= ((model[4].quant == XML_CQUANT_NONE) ? 0 : (1u << 23));
  errorFlags |= ((model[4].numchildren == 0) ? 0 : (1u << 24));
  errorFlags |= ((model[4].children == NULL) ? 0 : (1u << 25));
  errorFlags |= ((xcstrcmp(model[4].name, XCS("foo")) == 0) ? 0 : (1u << 26));

  errorFlags |= ((model[5].type == XML_CTYPE_NAME) ? 0 : (1u << 27));
  errorFlags |= ((model[5].quant == XML_CQUANT_PLUS) ? 0 : (1u << 28));
  errorFlags |= ((model[5].numchildren == 0) ? 0 : (1u << 29));
  errorFlags |= ((model[5].children == NULL) ? 0 : (1u << 30));
  errorFlags |= ((xcstrcmp(model[5].name, XCS("xyz")) == 0) ? 0 : (1u << 31));

  XML_SetUserData(g_parser, (void *)(uintptr_t)errorFlags);
  XML_FreeContentModel(g_parser, model);
}

START_TEST(test_dtd_elements_nesting) {
  // Payload inspired by a test in Perl's XML::Parser
  const char *text = "<!DOCTYPE foo [\n"
                     "<!ELEMENT junk ((bar|foo|xyz+), zebra*)>\n"
                     "]>\n"
                     "<foo/>";

  XML_SetUserData(g_parser, (void *)(uintptr_t)-1);

  XML_SetElementDeclHandler(g_parser, element_decl_check_model);
  if (XML_Parse(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);

  if ((uint32_t)(uintptr_t)XML_GetUserData(g_parser) != 0)
    fail("Element declaration model regression detected");
}
END_TEST

/* Test foreign DTD handling */
START_TEST(test_set_foreign_dtd) {
  const char *text1 = "<?xml version='1.0' encoding='us-ascii'?>\n";
  const char *text2 = "<doc>&entity;</doc>";
  ExtTest test_data = {"<!ELEMENT doc (#PCDATA)*>", NULL, NULL};

  /* Check hash salt is passed through too */
  XML_SetHashSalt(g_parser, 0x12345678);
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetUserData(g_parser, &test_data);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader);
  /* Add a default handler to exercise more code paths */
  XML_SetDefaultHandler(g_parser, dummy_default_handler);
  if (XML_UseForeignDTD(g_parser, XML_TRUE) != XML_ERROR_NONE)
    fail("Could not set foreign DTD");
  if (_XML_Parse_SINGLE_BYTES(g_parser, text1, (int)strlen(text1), XML_FALSE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);

  /* Ensure that trying to set the DTD after parsing has started
   * is faulted, even if it's the same setting.
   */
  if (XML_UseForeignDTD(g_parser, XML_TRUE)
      != XML_ERROR_CANT_CHANGE_FEATURE_ONCE_PARSING)
    fail("Failed to reject late foreign DTD setting");
  /* Ditto for the hash salt */
  if (XML_SetHashSalt(g_parser, 0x23456789))
    fail("Failed to reject late hash salt change");

  /* Now finish the parse */
  if (_XML_Parse_SINGLE_BYTES(g_parser, text2, (int)strlen(text2), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Test foreign DTD handling with a failing NotStandalone handler */
START_TEST(test_foreign_dtd_not_standalone) {
  const char *text = "<?xml version='1.0' encoding='us-ascii'?>\n"
                     "<doc>&entity;</doc>";
  ExtTest test_data = {"<!ELEMENT doc (#PCDATA)*>", NULL, NULL};

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetUserData(g_parser, &test_data);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader);
  XML_SetNotStandaloneHandler(g_parser, reject_not_standalone_handler);
  if (XML_UseForeignDTD(g_parser, XML_TRUE) != XML_ERROR_NONE)
    fail("Could not set foreign DTD");
  expect_failure(text, XML_ERROR_NOT_STANDALONE,
                 "NotStandalonehandler failed to reject");
}
END_TEST

/* Test invalid character in a foreign DTD is faulted */
START_TEST(test_invalid_foreign_dtd) {
  const char *text = "<?xml version='1.0' encoding='us-ascii'?>\n"
                     "<doc>&entity;</doc>";
  ExtFaults test_data
      = {"$", "Dollar not faulted", NULL, XML_ERROR_INVALID_TOKEN};

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetUserData(g_parser, &test_data);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_faulter);
  XML_UseForeignDTD(g_parser, XML_TRUE);
  expect_failure(text, XML_ERROR_EXTERNAL_ENTITY_HANDLING,
                 "Bad DTD should not have been accepted");
}
END_TEST

/* Test foreign DTD use with a doctype */
START_TEST(test_foreign_dtd_with_doctype) {
  const char *text1 = "<?xml version='1.0' encoding='us-ascii'?>\n"
                      "<!DOCTYPE doc [<!ENTITY entity 'hello world'>]>\n";
  const char *text2 = "<doc>&entity;</doc>";
  ExtTest test_data = {"<!ELEMENT doc (#PCDATA)*>", NULL, NULL};

  /* Check hash salt is passed through too */
  XML_SetHashSalt(g_parser, 0x12345678);
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetUserData(g_parser, &test_data);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader);
  /* Add a default handler to exercise more code paths */
  XML_SetDefaultHandler(g_parser, dummy_default_handler);
  if (XML_UseForeignDTD(g_parser, XML_TRUE) != XML_ERROR_NONE)
    fail("Could not set foreign DTD");
  if (_XML_Parse_SINGLE_BYTES(g_parser, text1, (int)strlen(text1), XML_FALSE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);

  /* Ensure that trying to set the DTD after parsing has started
   * is faulted, even if it's the same setting.
   */
  if (XML_UseForeignDTD(g_parser, XML_TRUE)
      != XML_ERROR_CANT_CHANGE_FEATURE_ONCE_PARSING)
    fail("Failed to reject late foreign DTD setting");
  /* Ditto for the hash salt */
  if (XML_SetHashSalt(g_parser, 0x23456789))
    fail("Failed to reject late hash salt change");

  /* Now finish the parse */
  if (_XML_Parse_SINGLE_BYTES(g_parser, text2, (int)strlen(text2), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Test XML_UseForeignDTD with no external subset present */
static int XMLCALL
external_entity_null_loader(XML_Parser parser, const XML_Char *context,
                            const XML_Char *base, const XML_Char *systemId,
                            const XML_Char *publicId) {
  UNUSED_P(parser);
  UNUSED_P(context);
  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  return XML_STATUS_OK;
}

START_TEST(test_foreign_dtd_without_external_subset) {
  const char *text = "<!DOCTYPE doc [<!ENTITY foo 'bar'>]>\n"
                     "<doc>&foo;</doc>";

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetUserData(g_parser, NULL);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_null_loader);
  XML_UseForeignDTD(g_parser, XML_TRUE);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

START_TEST(test_empty_foreign_dtd) {
  const char *text = "<?xml version='1.0' encoding='us-ascii'?>\n"
                     "<doc>&entity;</doc>";

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_null_loader);
  XML_UseForeignDTD(g_parser, XML_TRUE);
  expect_failure(text, XML_ERROR_UNDEFINED_ENTITY,
                 "Undefined entity not faulted");
}
END_TEST

/* Test XML Base is set and unset appropriately */
START_TEST(test_set_base) {
  const XML_Char *old_base;
  const XML_Char *new_base = XCS("/local/file/name.xml");

  old_base = XML_GetBase(g_parser);
  if (XML_SetBase(g_parser, new_base) != XML_STATUS_OK)
    fail("Unable to set base");
  if (xcstrcmp(XML_GetBase(g_parser), new_base) != 0)
    fail("Base setting not correct");
  if (XML_SetBase(g_parser, NULL) != XML_STATUS_OK)
    fail("Unable to NULL base");
  if (XML_GetBase(g_parser) != NULL)
    fail("Base setting not nulled");
  XML_SetBase(g_parser, old_base);
}
END_TEST

/* Test attribute counts, indexing, etc */
typedef struct attrInfo {
  const XML_Char *name;
  const XML_Char *value;
} AttrInfo;

typedef struct elementInfo {
  const XML_Char *name;
  int attr_count;
  const XML_Char *id_name;
  AttrInfo *attributes;
} ElementInfo;

static void XMLCALL
counting_start_element_handler(void *userData, const XML_Char *name,
                               const XML_Char **atts) {
  ElementInfo *info = (ElementInfo *)userData;
  AttrInfo *attr;
  int count, id, i;

  while (info->name != NULL) {
    if (! xcstrcmp(name, info->name))
      break;
    info++;
  }
  if (info->name == NULL)
    fail("Element not recognised");
  /* The attribute count is twice what you might expect.  It is a
   * count of items in atts, an array which contains alternating
   * attribute names and attribute values.  For the naive user this
   * is possibly a little unexpected, but it is what the
   * documentation in expat.h tells us to expect.
   */
  count = XML_GetSpecifiedAttributeCount(g_parser);
  if (info->attr_count * 2 != count) {
    fail("Not got expected attribute count");
    return;
  }
  id = XML_GetIdAttributeIndex(g_parser);
  if (id == -1 && info->id_name != NULL) {
    fail("ID not present");
    return;
  }
  if (id != -1 && xcstrcmp(atts[id], info->id_name)) {
    fail("ID does not have the correct name");
    return;
  }
  for (i = 0; i < info->attr_count; i++) {
    attr = info->attributes;
    while (attr->name != NULL) {
      if (! xcstrcmp(atts[0], attr->name))
        break;
      attr++;
    }
    if (attr->name == NULL) {
      fail("Attribute not recognised");
      return;
    }
    if (xcstrcmp(atts[1], attr->value)) {
      fail("Attribute has wrong value");
      return;
    }
    /* Remember, two entries in atts per attribute (see above) */
    atts += 2;
  }
}

START_TEST(test_attributes) {
  const char *text = "<!DOCTYPE doc [\n"
                     "<!ELEMENT doc (tag)>\n"
                     "<!ATTLIST doc id ID #REQUIRED>\n"
                     "]>"
                     "<doc a='1' id='one' b='2'>"
                     "<tag c='3'/>"
                     "</doc>";
  AttrInfo doc_info[] = {{XCS("a"), XCS("1")},
                         {XCS("b"), XCS("2")},
                         {XCS("id"), XCS("one")},
                         {NULL, NULL}};
  AttrInfo tag_info[] = {{XCS("c"), XCS("3")}, {NULL, NULL}};
  ElementInfo info[] = {{XCS("doc"), 3, XCS("id"), NULL},
                        {XCS("tag"), 1, NULL, NULL},
                        {NULL, 0, NULL, NULL}};
  info[0].attributes = doc_info;
  info[1].attributes = tag_info;

  XML_SetStartElementHandler(g_parser, counting_start_element_handler);
  XML_SetUserData(g_parser, info);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Test reset works correctly in the middle of processing an internal
 * entity.  Exercises some obscure code in XML_ParserReset().
 */
START_TEST(test_reset_in_entity) {
  const char *text = "<!DOCTYPE doc [\n"
                     "<!ENTITY wombat 'wom'>\n"
                     "<!ENTITY entity 'hi &wom; there'>\n"
                     "]>\n"
                     "<doc>&entity;</doc>";
  XML_ParsingStatus status;

  resumable = XML_TRUE;
  XML_SetCharacterDataHandler(g_parser, clearing_aborting_character_handler);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_FALSE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  XML_GetParsingStatus(g_parser, &status);
  if (status.parsing != XML_SUSPENDED)
    fail("Parsing status not SUSPENDED");
  XML_ParserReset(g_parser, NULL);
  XML_GetParsingStatus(g_parser, &status);
  if (status.parsing != XML_INITIALIZED)
    fail("Parsing status doesn't reset to INITIALIZED");
}
END_TEST

/* Test that resume correctly passes through parse errors */
START_TEST(test_resume_invalid_parse) {
  const char *text = "<doc>Hello</doc"; /* Missing closing wedge */

  resumable = XML_TRUE;
  XML_SetCharacterDataHandler(g_parser, clearing_aborting_character_handler);
  if (XML_Parse(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  if (XML_ResumeParser(g_parser) == XML_STATUS_OK)
    fail("Resumed invalid parse not faulted");
  if (XML_GetErrorCode(g_parser) != XML_ERROR_UNCLOSED_TOKEN)
    fail("Invalid parse not correctly faulted");
}
END_TEST

/* Test that re-suspended parses are correctly passed through */
START_TEST(test_resume_resuspended) {
  const char *text = "<doc>Hello<meep/>world</doc>";

  resumable = XML_TRUE;
  XML_SetCharacterDataHandler(g_parser, clearing_aborting_character_handler);
  if (XML_Parse(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  resumable = XML_TRUE;
  XML_SetCharacterDataHandler(g_parser, clearing_aborting_character_handler);
  if (XML_ResumeParser(g_parser) != XML_STATUS_SUSPENDED)
    fail("Resumption not suspended");
  /* This one should succeed and finish up */
  if (XML_ResumeParser(g_parser) != XML_STATUS_OK)
    xml_failure(g_parser);
}
END_TEST

/* Test that CDATA shows up correctly through a default handler */
START_TEST(test_cdata_default) {
  const char *text = "<doc><![CDATA[Hello\nworld]]></doc>";
  const XML_Char *expected = XCS("<doc><![CDATA[Hello\nworld]]></doc>");
  CharData storage;

  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  XML_SetDefaultHandler(g_parser, accumulate_characters);

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

/* Test resetting a subordinate parser does exactly nothing */
static int XMLCALL
external_entity_resetter(XML_Parser parser, const XML_Char *context,
                         const XML_Char *base, const XML_Char *systemId,
                         const XML_Char *publicId) {
  const char *text = "<!ELEMENT doc (#PCDATA)*>";
  XML_Parser ext_parser;
  XML_ParsingStatus status;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser");
  XML_GetParsingStatus(ext_parser, &status);
  if (status.parsing != XML_INITIALIZED) {
    fail("Parsing status is not INITIALIZED");
    return XML_STATUS_ERROR;
  }
  if (_XML_Parse_SINGLE_BYTES(ext_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR) {
    xml_failure(parser);
    return XML_STATUS_ERROR;
  }
  XML_GetParsingStatus(ext_parser, &status);
  if (status.parsing != XML_FINISHED) {
    fail("Parsing status is not FINISHED");
    return XML_STATUS_ERROR;
  }
  /* Check we can't parse here */
  if (XML_Parse(ext_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_ERROR)
    fail("Parsing when finished not faulted");
  if (XML_GetErrorCode(ext_parser) != XML_ERROR_FINISHED)
    fail("Parsing when finished faulted with wrong code");
  XML_ParserReset(ext_parser, NULL);
  XML_GetParsingStatus(ext_parser, &status);
  if (status.parsing != XML_FINISHED) {
    fail("Parsing status not still FINISHED");
    return XML_STATUS_ERROR;
  }
  XML_ParserFree(ext_parser);
  return XML_STATUS_OK;
}

START_TEST(test_subordinate_reset) {
  const char *text = "<?xml version='1.0' encoding='us-ascii'?>\n"
                     "<!DOCTYPE doc SYSTEM 'foo'>\n"
                     "<doc>&entity;</doc>";

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_resetter);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Test suspending a subordinate parser */

static void XMLCALL
entity_suspending_decl_handler(void *userData, const XML_Char *name,
                               XML_Content *model) {
  XML_Parser ext_parser = (XML_Parser)userData;

  UNUSED_P(name);
  if (XML_StopParser(ext_parser, XML_TRUE) != XML_STATUS_ERROR)
    fail("Attempting to suspend a subordinate parser not faulted");
  if (XML_GetErrorCode(ext_parser) != XML_ERROR_SUSPEND_PE)
    fail("Suspending subordinate parser get wrong code");
  XML_SetElementDeclHandler(ext_parser, NULL);
  XML_FreeContentModel(g_parser, model);
}

static int XMLCALL
external_entity_suspender(XML_Parser parser, const XML_Char *context,
                          const XML_Char *base, const XML_Char *systemId,
                          const XML_Char *publicId) {
  const char *text = "<!ELEMENT doc (#PCDATA)*>";
  XML_Parser ext_parser;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser");
  XML_SetElementDeclHandler(ext_parser, entity_suspending_decl_handler);
  XML_SetUserData(ext_parser, ext_parser);
  if (_XML_Parse_SINGLE_BYTES(ext_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR) {
    xml_failure(ext_parser);
    return XML_STATUS_ERROR;
  }
  XML_ParserFree(ext_parser);
  return XML_STATUS_OK;
}

START_TEST(test_subordinate_suspend) {
  const char *text = "<?xml version='1.0' encoding='us-ascii'?>\n"
                     "<!DOCTYPE doc SYSTEM 'foo'>\n"
                     "<doc>&entity;</doc>";

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_suspender);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Test suspending a subordinate parser from an XML declaration */
/* Increases code coverage of the tests */
static void XMLCALL
entity_suspending_xdecl_handler(void *userData, const XML_Char *version,
                                const XML_Char *encoding, int standalone) {
  XML_Parser ext_parser = (XML_Parser)userData;

  UNUSED_P(version);
  UNUSED_P(encoding);
  UNUSED_P(standalone);
  XML_StopParser(ext_parser, resumable);
  XML_SetXmlDeclHandler(ext_parser, NULL);
}

static int XMLCALL
external_entity_suspend_xmldecl(XML_Parser parser, const XML_Char *context,
                                const XML_Char *base, const XML_Char *systemId,
                                const XML_Char *publicId) {
  const char *text = "<?xml version='1.0' encoding='us-ascii'?>";
  XML_Parser ext_parser;
  XML_ParsingStatus status;
  enum XML_Status rc;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser");
  XML_SetXmlDeclHandler(ext_parser, entity_suspending_xdecl_handler);
  XML_SetUserData(ext_parser, ext_parser);
  rc = _XML_Parse_SINGLE_BYTES(ext_parser, text, (int)strlen(text), XML_TRUE);
  XML_GetParsingStatus(ext_parser, &status);
  if (resumable) {
    if (rc == XML_STATUS_ERROR)
      xml_failure(ext_parser);
    if (status.parsing != XML_SUSPENDED)
      fail("Ext Parsing status not SUSPENDED");
  } else {
    if (rc != XML_STATUS_ERROR)
      fail("Ext parsing not aborted");
    if (XML_GetErrorCode(ext_parser) != XML_ERROR_ABORTED)
      xml_failure(ext_parser);
    if (status.parsing != XML_FINISHED)
      fail("Ext Parsing status not FINISHED");
  }

  XML_ParserFree(ext_parser);
  return XML_STATUS_OK;
}

START_TEST(test_subordinate_xdecl_suspend) {
  const char *text
      = "<!DOCTYPE doc [\n"
        "  <!ENTITY entity SYSTEM 'http://example.org/dummy.ent'>\n"
        "]>\n"
        "<doc>&entity;</doc>";

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_suspend_xmldecl);
  resumable = XML_TRUE;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

START_TEST(test_subordinate_xdecl_abort) {
  const char *text
      = "<!DOCTYPE doc [\n"
        "  <!ENTITY entity SYSTEM 'http://example.org/dummy.ent'>\n"
        "]>\n"
        "<doc>&entity;</doc>";

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_suspend_xmldecl);
  resumable = XML_FALSE;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Test external entity fault handling with suspension */
static int XMLCALL
external_entity_suspending_faulter(XML_Parser parser, const XML_Char *context,
                                   const XML_Char *base,
                                   const XML_Char *systemId,
                                   const XML_Char *publicId) {
  XML_Parser ext_parser;
  ExtFaults *fault = (ExtFaults *)XML_GetUserData(parser);
  void *buffer;
  int parse_len = (int)strlen(fault->parse_text);

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser");
  XML_SetXmlDeclHandler(ext_parser, entity_suspending_xdecl_handler);
  XML_SetUserData(ext_parser, ext_parser);
  resumable = XML_TRUE;
  buffer = XML_GetBuffer(ext_parser, parse_len);
  if (buffer == NULL)
    fail("Could not allocate parse buffer");
  assert(buffer != NULL);
  memcpy(buffer, fault->parse_text, parse_len);
  if (XML_ParseBuffer(ext_parser, parse_len, XML_FALSE) != XML_STATUS_SUSPENDED)
    fail("XML declaration did not suspend");
  if (XML_ResumeParser(ext_parser) != XML_STATUS_OK)
    xml_failure(ext_parser);
  if (XML_ParseBuffer(ext_parser, 0, XML_TRUE) != XML_STATUS_ERROR)
    fail(fault->fail_text);
  if (XML_GetErrorCode(ext_parser) != fault->error)
    xml_failure(ext_parser);

  XML_ParserFree(ext_parser);
  return XML_STATUS_ERROR;
}

START_TEST(test_ext_entity_invalid_suspended_parse) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/dummy.ent'>\n"
                     "]>\n"
                     "<doc>&en;</doc>";
  ExtFaults faults[]
      = {{"<?xml version='1.0' encoding='us-ascii'?><",
          "Incomplete element declaration not faulted", NULL,
          XML_ERROR_UNCLOSED_TOKEN},
         {/* First two bytes of a three-byte char */
          "<?xml version='1.0' encoding='utf-8'?>\xe2\x82",
          "Incomplete character not faulted", NULL, XML_ERROR_PARTIAL_CHAR},
         {NULL, NULL, NULL, XML_ERROR_NONE}};
  ExtFaults *fault;

  for (fault = &faults[0]; fault->parse_text != NULL; fault++) {
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetExternalEntityRefHandler(g_parser,
                                    external_entity_suspending_faulter);
    XML_SetUserData(g_parser, fault);
    expect_failure(text, XML_ERROR_EXTERNAL_ENTITY_HANDLING,
                   "Parser did not report external entity error");
    XML_ParserReset(g_parser, NULL);
  }
}
END_TEST

/* Test setting an explicit encoding */
START_TEST(test_explicit_encoding) {
  const char *text1 = "<doc>Hello ";
  const char *text2 = " World</doc>";

  /* Just check that we can set the encoding to NULL before starting */
  if (XML_SetEncoding(g_parser, NULL) != XML_STATUS_OK)
    fail("Failed to initialise encoding to NULL");
  /* Say we are UTF-8 */
  if (XML_SetEncoding(g_parser, XCS("utf-8")) != XML_STATUS_OK)
    fail("Failed to set explicit encoding");
  if (_XML_Parse_SINGLE_BYTES(g_parser, text1, (int)strlen(text1), XML_FALSE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  /* Try to switch encodings mid-parse */
  if (XML_SetEncoding(g_parser, XCS("us-ascii")) != XML_STATUS_ERROR)
    fail("Allowed encoding change");
  if (_XML_Parse_SINGLE_BYTES(g_parser, text2, (int)strlen(text2), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  /* Try now the parse is over */
  if (XML_SetEncoding(g_parser, NULL) != XML_STATUS_OK)
    fail("Failed to unset encoding");
}
END_TEST

/* Test handling of trailing CR (rather than newline) */
static void XMLCALL
cr_cdata_handler(void *userData, const XML_Char *s, int len) {
  int *pfound = (int *)userData;

  /* Internal processing turns the CR into a newline for the
   * character data handler, but not for the default handler
   */
  if (len == 1 && (*s == XCS('\n') || *s == XCS('\r')))
    *pfound = 1;
}

START_TEST(test_trailing_cr) {
  const char *text = "<doc>\r";
  int found_cr;

  /* Try with a character handler, for code coverage */
  XML_SetCharacterDataHandler(g_parser, cr_cdata_handler);
  XML_SetUserData(g_parser, &found_cr);
  found_cr = 0;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_OK)
    fail("Failed to fault unclosed doc");
  if (found_cr == 0)
    fail("Did not catch the carriage return");
  XML_ParserReset(g_parser, NULL);

  /* Now with a default handler instead */
  XML_SetDefaultHandler(g_parser, cr_cdata_handler);
  XML_SetUserData(g_parser, &found_cr);
  found_cr = 0;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_OK)
    fail("Failed to fault unclosed doc");
  if (found_cr == 0)
    fail("Did not catch default carriage return");
}
END_TEST

/* Test trailing CR in an external entity parse */
static int XMLCALL
external_entity_cr_catcher(XML_Parser parser, const XML_Char *context,
                           const XML_Char *base, const XML_Char *systemId,
                           const XML_Char *publicId) {
  const char *text = "\r";
  XML_Parser ext_parser;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser");
  XML_SetCharacterDataHandler(ext_parser, cr_cdata_handler);
  if (_XML_Parse_SINGLE_BYTES(ext_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(ext_parser);
  XML_ParserFree(ext_parser);
  return XML_STATUS_OK;
}

static int XMLCALL
external_entity_bad_cr_catcher(XML_Parser parser, const XML_Char *context,
                               const XML_Char *base, const XML_Char *systemId,
                               const XML_Char *publicId) {
  const char *text = "<tag>\r";
  XML_Parser ext_parser;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser");
  XML_SetCharacterDataHandler(ext_parser, cr_cdata_handler);
  if (_XML_Parse_SINGLE_BYTES(ext_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_OK)
    fail("Async entity error not caught");
  if (XML_GetErrorCode(ext_parser) != XML_ERROR_ASYNC_ENTITY)
    xml_failure(ext_parser);
  XML_ParserFree(ext_parser);
  return XML_STATUS_OK;
}

START_TEST(test_ext_entity_trailing_cr) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/dummy.ent'>\n"
                     "]>\n"
                     "<doc>&en;</doc>";
  int found_cr;

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_cr_catcher);
  XML_SetUserData(g_parser, &found_cr);
  found_cr = 0;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_OK)
    xml_failure(g_parser);
  if (found_cr == 0)
    fail("No carriage return found");
  XML_ParserReset(g_parser, NULL);

  /* Try again with a different trailing CR */
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_bad_cr_catcher);
  XML_SetUserData(g_parser, &found_cr);
  found_cr = 0;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_OK)
    xml_failure(g_parser);
  if (found_cr == 0)
    fail("No carriage return found");
}
END_TEST

/* Test handling of trailing square bracket */
static void XMLCALL
rsqb_handler(void *userData, const XML_Char *s, int len) {
  int *pfound = (int *)userData;

  if (len == 1 && *s == XCS(']'))
    *pfound = 1;
}

START_TEST(test_trailing_rsqb) {
  const char *text8 = "<doc>]";
  const char text16[] = "\xFF\xFE<\000d\000o\000c\000>\000]\000";
  int found_rsqb;
  int text8_len = (int)strlen(text8);

  XML_SetCharacterDataHandler(g_parser, rsqb_handler);
  XML_SetUserData(g_parser, &found_rsqb);
  found_rsqb = 0;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text8, text8_len, XML_TRUE)
      == XML_STATUS_OK)
    fail("Failed to fault unclosed doc");
  if (found_rsqb == 0)
    fail("Did not catch the right square bracket");

  /* Try again with a different encoding */
  XML_ParserReset(g_parser, NULL);
  XML_SetCharacterDataHandler(g_parser, rsqb_handler);
  XML_SetUserData(g_parser, &found_rsqb);
  found_rsqb = 0;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text16, (int)sizeof(text16) - 1,
                              XML_TRUE)
      == XML_STATUS_OK)
    fail("Failed to fault unclosed doc");
  if (found_rsqb == 0)
    fail("Did not catch the right square bracket");

  /* And finally with a default handler */
  XML_ParserReset(g_parser, NULL);
  XML_SetDefaultHandler(g_parser, rsqb_handler);
  XML_SetUserData(g_parser, &found_rsqb);
  found_rsqb = 0;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text16, (int)sizeof(text16) - 1,
                              XML_TRUE)
      == XML_STATUS_OK)
    fail("Failed to fault unclosed doc");
  if (found_rsqb == 0)
    fail("Did not catch the right square bracket");
}
END_TEST

/* Test trailing right square bracket in an external entity parse */
static int XMLCALL
external_entity_rsqb_catcher(XML_Parser parser, const XML_Char *context,
                             const XML_Char *base, const XML_Char *systemId,
                             const XML_Char *publicId) {
  const char *text = "<tag>]";
  XML_Parser ext_parser;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser");
  XML_SetCharacterDataHandler(ext_parser, rsqb_handler);
  if (_XML_Parse_SINGLE_BYTES(ext_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_ERROR)
    fail("Async entity error not caught");
  if (XML_GetErrorCode(ext_parser) != XML_ERROR_ASYNC_ENTITY)
    xml_failure(ext_parser);
  XML_ParserFree(ext_parser);
  return XML_STATUS_OK;
}

START_TEST(test_ext_entity_trailing_rsqb) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/dummy.ent'>\n"
                     "]>\n"
                     "<doc>&en;</doc>";
  int found_rsqb;

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_rsqb_catcher);
  XML_SetUserData(g_parser, &found_rsqb);
  found_rsqb = 0;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_OK)
    xml_failure(g_parser);
  if (found_rsqb == 0)
    fail("No right square bracket found");
}
END_TEST

/* Test CDATA handling in an external entity */
static int XMLCALL
external_entity_good_cdata_ascii(XML_Parser parser, const XML_Char *context,
                                 const XML_Char *base, const XML_Char *systemId,
                                 const XML_Char *publicId) {
  const char *text = "<a><![CDATA[<greeting>Hello, world!</greeting>]]></a>";
  const XML_Char *expected = XCS("<greeting>Hello, world!</greeting>");
  CharData storage;
  XML_Parser ext_parser;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  CharData_Init(&storage);
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser");
  XML_SetUserData(ext_parser, &storage);
  XML_SetCharacterDataHandler(ext_parser, accumulate_characters);

  if (_XML_Parse_SINGLE_BYTES(ext_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(ext_parser);
  CharData_CheckXMLChars(&storage, expected);

  XML_ParserFree(ext_parser);
  return XML_STATUS_OK;
}

START_TEST(test_ext_entity_good_cdata) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/dummy.ent'>\n"
                     "]>\n"
                     "<doc>&en;</doc>";

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_good_cdata_ascii);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_OK)
    xml_failure(g_parser);
}
END_TEST

/* Test user parameter settings */
/* Variable holding the expected handler userData */
static void *handler_data = NULL;
/* Count of the number of times the comment handler has been invoked */
static int comment_count = 0;
/* Count of the number of skipped entities */
static int skip_count = 0;
/* Count of the number of times the XML declaration handler is invoked */
static int xdecl_count = 0;

static void XMLCALL
xml_decl_handler(void *userData, const XML_Char *version,
                 const XML_Char *encoding, int standalone) {
  UNUSED_P(version);
  UNUSED_P(encoding);
  if (userData != handler_data)
    fail("User data (xml decl) not correctly set");
  if (standalone != -1)
    fail("Standalone not flagged as not present in XML decl");
  xdecl_count++;
}

static void XMLCALL
param_check_skip_handler(void *userData, const XML_Char *entityName,
                         int is_parameter_entity) {
  UNUSED_P(entityName);
  UNUSED_P(is_parameter_entity);
  if (userData != handler_data)
    fail("User data (skip) not correctly set");
  skip_count++;
}

static void XMLCALL
data_check_comment_handler(void *userData, const XML_Char *data) {
  UNUSED_P(data);
  /* Check that the userData passed through is what we expect */
  if (userData != handler_data)
    fail("User data (parser) not correctly set");
  /* Check that the user data in the parser is appropriate */
  if (XML_GetUserData(userData) != (void *)1)
    fail("User data in parser not correctly set");
  comment_count++;
}

static int XMLCALL
external_entity_param_checker(XML_Parser parser, const XML_Char *context,
                              const XML_Char *base, const XML_Char *systemId,
                              const XML_Char *publicId) {
  const char *text = "<!-- Subordinate parser -->\n"
                     "<!ELEMENT doc (#PCDATA)*>";
  XML_Parser ext_parser;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser");
  handler_data = ext_parser;
  if (_XML_Parse_SINGLE_BYTES(ext_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR) {
    xml_failure(parser);
    return XML_STATUS_ERROR;
  }
  handler_data = parser;
  XML_ParserFree(ext_parser);
  return XML_STATUS_OK;
}

START_TEST(test_user_parameters) {
  const char *text = "<?xml version='1.0' encoding='us-ascii'?>\n"
                     "<!-- Primary parse -->\n"
                     "<!DOCTYPE doc SYSTEM 'foo'>\n"
                     "<doc>&entity;";
  const char *epilog = "<!-- Back to primary parser -->\n"
                       "</doc>";

  comment_count = 0;
  skip_count = 0;
  xdecl_count = 0;
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetXmlDeclHandler(g_parser, xml_decl_handler);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_param_checker);
  XML_SetCommentHandler(g_parser, data_check_comment_handler);
  XML_SetSkippedEntityHandler(g_parser, param_check_skip_handler);
  XML_UseParserAsHandlerArg(g_parser);
  XML_SetUserData(g_parser, (void *)1);
  handler_data = g_parser;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_FALSE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  if (comment_count != 2)
    fail("Comment handler not invoked enough times");
  /* Ensure we can't change policy mid-parse */
  if (XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_NEVER))
    fail("Changed param entity parsing policy while parsing");
  if (_XML_Parse_SINGLE_BYTES(g_parser, epilog, (int)strlen(epilog), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  if (comment_count != 3)
    fail("Comment handler not invoked enough times");
  if (skip_count != 1)
    fail("Skip handler not invoked enough times");
  if (xdecl_count != 1)
    fail("XML declaration handler not invoked");
}
END_TEST

/* Test that an explicit external entity handler argument replaces
 * the parser as the first argument.
 *
 * We do not call the first parameter to the external entity handler
 * 'parser' for once, since the first time the handler is called it
 * will actually be a text string.  We need to be able to access the
 * global 'parser' variable to create our external entity parser from,
 * since there are code paths we need to ensure get executed.
 */
static int XMLCALL
external_entity_ref_param_checker(XML_Parser parameter, const XML_Char *context,
                                  const XML_Char *base,
                                  const XML_Char *systemId,
                                  const XML_Char *publicId) {
  const char *text = "<!ELEMENT doc (#PCDATA)*>";
  XML_Parser ext_parser;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  if ((void *)parameter != handler_data)
    fail("External entity ref handler parameter not correct");

  /* Here we use the global 'parser' variable */
  ext_parser = XML_ExternalEntityParserCreate(g_parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser");
  if (_XML_Parse_SINGLE_BYTES(ext_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(ext_parser);

  XML_ParserFree(ext_parser);
  return XML_STATUS_OK;
}

START_TEST(test_ext_entity_ref_parameter) {
  const char *text = "<?xml version='1.0' encoding='us-ascii'?>\n"
                     "<!DOCTYPE doc SYSTEM 'foo'>\n"
                     "<doc>&entity;</doc>";

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_ref_param_checker);
  /* Set a handler arg that is not NULL and not parser (which is
   * what NULL would cause to be passed.
   */
  XML_SetExternalEntityRefHandlerArg(g_parser, (void *)text);
  handler_data = (void *)text;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);

  /* Now try again with unset args */
  XML_ParserReset(g_parser, NULL);
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_ref_param_checker);
  XML_SetExternalEntityRefHandlerArg(g_parser, NULL);
  handler_data = (void *)g_parser;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Test the parsing of an empty string */
START_TEST(test_empty_parse) {
  const char *text = "<doc></doc>";
  const char *partial = "<doc>";

  if (XML_Parse(g_parser, NULL, 0, XML_FALSE) == XML_STATUS_ERROR)
    fail("Parsing empty string faulted");
  if (XML_Parse(g_parser, NULL, 0, XML_TRUE) != XML_STATUS_ERROR)
    fail("Parsing final empty string not faulted");
  if (XML_GetErrorCode(g_parser) != XML_ERROR_NO_ELEMENTS)
    fail("Parsing final empty string faulted for wrong reason");

  /* Now try with valid text before the empty end */
  XML_ParserReset(g_parser, NULL);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_FALSE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  if (XML_Parse(g_parser, NULL, 0, XML_TRUE) == XML_STATUS_ERROR)
    fail("Parsing final empty string faulted");

  /* Now try with invalid text before the empty end */
  XML_ParserReset(g_parser, NULL);
  if (_XML_Parse_SINGLE_BYTES(g_parser, partial, (int)strlen(partial),
                              XML_FALSE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  if (XML_Parse(g_parser, NULL, 0, XML_TRUE) != XML_STATUS_ERROR)
    fail("Parsing final incomplete empty string not faulted");
}
END_TEST

/* Test odd corners of the XML_GetBuffer interface */
static enum XML_Status
get_feature(enum XML_FeatureEnum feature_id, long *presult) {
  const XML_Feature *feature = XML_GetFeatureList();

  if (feature == NULL)
    return XML_STATUS_ERROR;
  for (; feature->feature != XML_FEATURE_END; feature++) {
    if (feature->feature == feature_id) {
      *presult = feature->value;
      return XML_STATUS_OK;
    }
  }
  return XML_STATUS_ERROR;
}

/* Having an element name longer than 1024 characters exercises some
 * of the pool allocation code in the parser that otherwise does not
 * get executed.  The count at the end of the line is the number of
 * characters (bytes) in the element name by that point.x
 */
static const char *get_buffer_test_text
    = "<documentwitharidiculouslylongelementnametotease"  /* 0x030 */
      "aparticularcorneroftheallocationinXML_GetBuffers"  /* 0x060 */
      "othatwecanimprovethecoverageyetagain012345678901"  /* 0x090 */
      "123456789abcdef0123456789abcdef0123456789abcdef0"  /* 0x0c0 */
      "123456789abcdef0123456789abcdef0123456789abcdef0"  /* 0x0f0 */
      "123456789abcdef0123456789abcdef0123456789abcdef0"  /* 0x120 */
      "123456789abcdef0123456789abcdef0123456789abcdef0"  /* 0x150 */
      "123456789abcdef0123456789abcdef0123456789abcdef0"  /* 0x180 */
      "123456789abcdef0123456789abcdef0123456789abcdef0"  /* 0x1b0 */
      "123456789abcdef0123456789abcdef0123456789abcdef0"  /* 0x1e0 */
      "123456789abcdef0123456789abcdef0123456789abcdef0"  /* 0x210 */
      "123456789abcdef0123456789abcdef0123456789abcdef0"  /* 0x240 */
      "123456789abcdef0123456789abcdef0123456789abcdef0"  /* 0x270 */
      "123456789abcdef0123456789abcdef0123456789abcdef0"  /* 0x2a0 */
      "123456789abcdef0123456789abcdef0123456789abcdef0"  /* 0x2d0 */
      "123456789abcdef0123456789abcdef0123456789abcdef0"  /* 0x300 */
      "123456789abcdef0123456789abcdef0123456789abcdef0"  /* 0x330 */
      "123456789abcdef0123456789abcdef0123456789abcdef0"  /* 0x360 */
      "123456789abcdef0123456789abcdef0123456789abcdef0"  /* 0x390 */
      "123456789abcdef0123456789abcdef0123456789abcdef0"  /* 0x3c0 */
      "123456789abcdef0123456789abcdef0123456789abcdef0"  /* 0x3f0 */
      "123456789abcdef0123456789abcdef0123456789>\n<ef0"; /* 0x420 */

/* Test odd corners of the XML_GetBuffer interface */
START_TEST(test_get_buffer_1) {
  const char *text = get_buffer_test_text;
  void *buffer;
  long context_bytes;

  /* Attempt to allocate a negative length buffer */
  if (XML_GetBuffer(g_parser, -12) != NULL)
    fail("Negative length buffer not failed");

  /* Now get a small buffer and extend it past valid length */
  buffer = XML_GetBuffer(g_parser, 1536);
  if (buffer == NULL)
    fail("1.5K buffer failed");
  assert(buffer != NULL);
  memcpy(buffer, text, strlen(text));
  if (XML_ParseBuffer(g_parser, (int)strlen(text), XML_FALSE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  if (XML_GetBuffer(g_parser, INT_MAX) != NULL)
    fail("INT_MAX buffer not failed");

  /* Now try extending it a more reasonable but still too large
   * amount.  The allocator in XML_GetBuffer() doubles the buffer
   * size until it exceeds the requested amount or INT_MAX.  If it
   * exceeds INT_MAX, it rejects the request, so we want a request
   * between INT_MAX and INT_MAX/2.  A gap of 1K seems comfortable,
   * with an extra byte just to ensure that the request is off any
   * boundary.  The request will be inflated internally by
   * XML_CONTEXT_BYTES (if defined), so we subtract that from our
   * request.
   */
  if (get_feature(XML_FEATURE_CONTEXT_BYTES, &context_bytes) != XML_STATUS_OK)
    context_bytes = 0;
  if (XML_GetBuffer(g_parser, INT_MAX - (context_bytes + 1025)) != NULL)
    fail("INT_MAX- buffer not failed");

  /* Now try extending it a carefully crafted amount */
  if (XML_GetBuffer(g_parser, 1000) == NULL)
    fail("1000 buffer failed");
}
END_TEST

/* Test more corners of the XML_GetBuffer interface */
START_TEST(test_get_buffer_2) {
  const char *text = get_buffer_test_text;
  void *buffer;

  /* Now get a decent buffer */
  buffer = XML_GetBuffer(g_parser, 1536);
  if (buffer == NULL)
    fail("1.5K buffer failed");
  assert(buffer != NULL);
  memcpy(buffer, text, strlen(text));
  if (XML_ParseBuffer(g_parser, (int)strlen(text), XML_FALSE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);

  /* Extend it, to catch a different code path */
  if (XML_GetBuffer(g_parser, 1024) == NULL)
    fail("1024 buffer failed");
}
END_TEST

/* Test for signed integer overflow CVE-2022-23852 */
#if defined(XML_CONTEXT_BYTES)
START_TEST(test_get_buffer_3_overflow) {
  XML_Parser parser = XML_ParserCreate(NULL);
  assert(parser != NULL);

  const char *const text = "\n";
  const int expectedKeepValue = (int)strlen(text);

  // After this call, variable "keep" in XML_GetBuffer will
  // have value expectedKeepValue
  if (XML_Parse(parser, text, (int)strlen(text), XML_FALSE /* isFinal */)
      == XML_STATUS_ERROR)
    xml_failure(parser);

  assert(expectedKeepValue > 0);
  if (XML_GetBuffer(parser, INT_MAX - expectedKeepValue + 1) != NULL)
    fail("enlarging buffer not failed");

  XML_ParserFree(parser);
}
END_TEST
#endif // defined(XML_CONTEXT_BYTES)

/* Test position information macros */
START_TEST(test_byte_info_at_end) {
  const char *text = "<doc></doc>";

  if (XML_GetCurrentByteIndex(g_parser) != -1
      || XML_GetCurrentByteCount(g_parser) != 0)
    fail("Byte index/count incorrect at start of parse");
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  /* At end, the count will be zero and the index the end of string */
  if (XML_GetCurrentByteCount(g_parser) != 0)
    fail("Terminal byte count incorrect");
  if (XML_GetCurrentByteIndex(g_parser) != (XML_Index)strlen(text))
    fail("Terminal byte index incorrect");
}
END_TEST

/* Test position information from errors */
#define PRE_ERROR_STR "<doc></"
#define POST_ERROR_STR "wombat></doc>"
START_TEST(test_byte_info_at_error) {
  const char *text = PRE_ERROR_STR POST_ERROR_STR;

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_OK)
    fail("Syntax error not faulted");
  if (XML_GetCurrentByteCount(g_parser) != 0)
    fail("Error byte count incorrect");
  if (XML_GetCurrentByteIndex(g_parser) != strlen(PRE_ERROR_STR))
    fail("Error byte index incorrect");
}
END_TEST
#undef PRE_ERROR_STR
#undef POST_ERROR_STR

/* Test position information in handler */
typedef struct ByteTestData {
  int start_element_len;
  int cdata_len;
  int total_string_len;
} ByteTestData;

static void
byte_character_handler(void *userData, const XML_Char *s, int len) {
#ifdef XML_CONTEXT_BYTES
  int offset, size;
  const char *buffer;
  ByteTestData *data = (ByteTestData *)userData;

  UNUSED_P(s);
  buffer = XML_GetInputContext(g_parser, &offset, &size);
  if (buffer == NULL)
    fail("Failed to get context buffer");
  if (offset != data->start_element_len)
    fail("Context offset in unexpected position");
  if (len != data->cdata_len)
    fail("CDATA length reported incorrectly");
  if (size != data->total_string_len)
    fail("Context size is not full buffer");
  if (XML_GetCurrentByteIndex(g_parser) != offset)
    fail("Character byte index incorrect");
  if (XML_GetCurrentByteCount(g_parser) != len)
    fail("Character byte count incorrect");
#else
  UNUSED_P(s);
  UNUSED_P(userData);
  UNUSED_P(len);
#endif
}

#define START_ELEMENT "<e>"
#define CDATA_TEXT "Hello"
#define END_ELEMENT "</e>"
START_TEST(test_byte_info_at_cdata) {
  const char *text = START_ELEMENT CDATA_TEXT END_ELEMENT;
  int offset, size;
  ByteTestData data;

  /* Check initial context is empty */
  if (XML_GetInputContext(g_parser, &offset, &size) != NULL)
    fail("Unexpected context at start of parse");

  data.start_element_len = (int)strlen(START_ELEMENT);
  data.cdata_len = (int)strlen(CDATA_TEXT);
  data.total_string_len = (int)strlen(text);
  XML_SetCharacterDataHandler(g_parser, byte_character_handler);
  XML_SetUserData(g_parser, &data);
  if (XML_Parse(g_parser, text, (int)strlen(text), XML_TRUE) != XML_STATUS_OK)
    xml_failure(g_parser);
}
END_TEST
#undef START_ELEMENT
#undef CDATA_TEXT
#undef END_ELEMENT

/* Test predefined entities are correctly recognised */
START_TEST(test_predefined_entities) {
  const char *text = "<doc>&lt;&gt;&amp;&quot;&apos;</doc>";
  const XML_Char *expected = XCS("<doc>&lt;&gt;&amp;&quot;&apos;</doc>");
  const XML_Char *result = XCS("<>&\"'");
  CharData storage;

  XML_SetDefaultHandler(g_parser, accumulate_characters);
  /* run_character_check uses XML_SetCharacterDataHandler(), which
   * unfortunately heads off a code path that we need to exercise.
   */
  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  /* The default handler doesn't translate the entities */
  CharData_CheckXMLChars(&storage, expected);

  /* Now try again and check the translation */
  XML_ParserReset(g_parser, NULL);
  run_character_check(text, result);
}
END_TEST

/* Regression test that an invalid tag in an external parameter
 * reference in an external DTD is correctly faulted.
 *
 * Only a few specific tags are legal in DTDs ignoring comments and
 * processing instructions, all of which begin with an exclamation
 * mark.  "<el/>" is not one of them, so the parser should raise an
 * error on encountering it.
 */
static int XMLCALL
external_entity_param(XML_Parser parser, const XML_Char *context,
                      const XML_Char *base, const XML_Char *systemId,
                      const XML_Char *publicId) {
  const char *text1 = "<!ELEMENT doc EMPTY>\n"
                      "<!ENTITY % e1 SYSTEM '004-2.ent'>\n"
                      "<!ENTITY % e2 '%e1;'>\n"
                      "%e1;\n";
  const char *text2 = "<!ELEMENT el EMPTY>\n"
                      "<el/>\n";
  XML_Parser ext_parser;

  UNUSED_P(base);
  UNUSED_P(publicId);
  if (systemId == NULL)
    return XML_STATUS_OK;

  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser");

  if (! xcstrcmp(systemId, XCS("004-1.ent"))) {
    if (_XML_Parse_SINGLE_BYTES(ext_parser, text1, (int)strlen(text1), XML_TRUE)
        != XML_STATUS_ERROR)
      fail("Inner DTD with invalid tag not rejected");
    if (XML_GetErrorCode(ext_parser) != XML_ERROR_EXTERNAL_ENTITY_HANDLING)
      xml_failure(ext_parser);
  } else if (! xcstrcmp(systemId, XCS("004-2.ent"))) {
    if (_XML_Parse_SINGLE_BYTES(ext_parser, text2, (int)strlen(text2), XML_TRUE)
        != XML_STATUS_ERROR)
      fail("Invalid tag in external param not rejected");
    if (XML_GetErrorCode(ext_parser) != XML_ERROR_SYNTAX)
      xml_failure(ext_parser);
  } else {
    fail("Unknown system ID");
  }

  XML_ParserFree(ext_parser);
  return XML_STATUS_ERROR;
}

START_TEST(test_invalid_tag_in_dtd) {
  const char *text = "<!DOCTYPE doc SYSTEM '004-1.ent'>\n"
                     "<doc></doc>\n";

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_param);
  expect_failure(text, XML_ERROR_EXTERNAL_ENTITY_HANDLING,
                 "Invalid tag IN DTD external param not rejected");
}
END_TEST

/* Test entities not quite the predefined ones are not mis-recognised */
START_TEST(test_not_predefined_entities) {
  const char *text[] = {"<doc>&pt;</doc>", "<doc>&amo;</doc>",
                        "<doc>&quid;</doc>", "<doc>&apod;</doc>", NULL};
  int i = 0;

  while (text[i] != NULL) {
    expect_failure(text[i], XML_ERROR_UNDEFINED_ENTITY,
                   "Undefined entity not rejected");
    XML_ParserReset(g_parser, NULL);
    i++;
  }
}
END_TEST

/* Test conditional inclusion (IGNORE) */
static int XMLCALL
external_entity_load_ignore(XML_Parser parser, const XML_Char *context,
                            const XML_Char *base, const XML_Char *systemId,
                            const XML_Char *publicId) {
  const char *text = "<![IGNORE[<!ELEMENT e (#PCDATA)*>]]>";
  XML_Parser ext_parser;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser");
  if (_XML_Parse_SINGLE_BYTES(ext_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(parser);

  XML_ParserFree(ext_parser);
  return XML_STATUS_OK;
}

START_TEST(test_ignore_section) {
  const char *text = "<!DOCTYPE doc SYSTEM 'foo'>\n"
                     "<doc><e>&entity;</e></doc>";
  const XML_Char *expected
      = XCS("<![IGNORE[<!ELEMENT e (#PCDATA)*>]]>\n&entity;");
  CharData storage;

  CharData_Init(&storage);
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetUserData(g_parser, &storage);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_load_ignore);
  XML_SetDefaultHandler(g_parser, accumulate_characters);
  XML_SetStartDoctypeDeclHandler(g_parser, dummy_start_doctype_handler);
  XML_SetEndDoctypeDeclHandler(g_parser, dummy_end_doctype_handler);
  XML_SetElementDeclHandler(g_parser, dummy_element_decl_handler);
  XML_SetStartElementHandler(g_parser, dummy_start_element);
  XML_SetEndElementHandler(g_parser, dummy_end_element);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

static int XMLCALL
external_entity_load_ignore_utf16(XML_Parser parser, const XML_Char *context,
                                  const XML_Char *base,
                                  const XML_Char *systemId,
                                  const XML_Char *publicId) {
  const char text[] =
      /* <![IGNORE[<!ELEMENT e (#PCDATA)*>]]> */
      "<\0!\0[\0I\0G\0N\0O\0R\0E\0[\0"
      "<\0!\0E\0L\0E\0M\0E\0N\0T\0 \0e\0 \0"
      "(\0#\0P\0C\0D\0A\0T\0A\0)\0*\0>\0]\0]\0>\0";
  XML_Parser ext_parser;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser");
  if (_XML_Parse_SINGLE_BYTES(ext_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(parser);

  XML_ParserFree(ext_parser);
  return XML_STATUS_OK;
}

START_TEST(test_ignore_section_utf16) {
  const char text[] =
      /* <!DOCTYPE d SYSTEM 's'> */
      "<\0!\0D\0O\0C\0T\0Y\0P\0E\0 \0d\0 "
      "\0S\0Y\0S\0T\0E\0M\0 \0'\0s\0'\0>\0\n\0"
      /* <d><e>&en;</e></d> */
      "<\0d\0>\0<\0e\0>\0&\0e\0n\0;\0<\0/\0e\0>\0<\0/\0d\0>\0";
  const XML_Char *expected = XCS("<![IGNORE[<!ELEMENT e (#PCDATA)*>]]>\n&en;");
  CharData storage;

  CharData_Init(&storage);
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetUserData(g_parser, &storage);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_load_ignore_utf16);
  XML_SetDefaultHandler(g_parser, accumulate_characters);
  XML_SetStartDoctypeDeclHandler(g_parser, dummy_start_doctype_handler);
  XML_SetEndDoctypeDeclHandler(g_parser, dummy_end_doctype_handler);
  XML_SetElementDeclHandler(g_parser, dummy_element_decl_handler);
  XML_SetStartElementHandler(g_parser, dummy_start_element);
  XML_SetEndElementHandler(g_parser, dummy_end_element);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

static int XMLCALL
external_entity_load_ignore_utf16_be(XML_Parser parser, const XML_Char *context,
                                     const XML_Char *base,
                                     const XML_Char *systemId,
                                     const XML_Char *publicId) {
  const char text[] =
      /* <![IGNORE[<!ELEMENT e (#PCDATA)*>]]> */
      "\0<\0!\0[\0I\0G\0N\0O\0R\0E\0["
      "\0<\0!\0E\0L\0E\0M\0E\0N\0T\0 \0e\0 "
      "\0(\0#\0P\0C\0D\0A\0T\0A\0)\0*\0>\0]\0]\0>";
  XML_Parser ext_parser;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser");
  if (_XML_Parse_SINGLE_BYTES(ext_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(parser);

  XML_ParserFree(ext_parser);
  return XML_STATUS_OK;
}

START_TEST(test_ignore_section_utf16_be) {
  const char text[] =
      /* <!DOCTYPE d SYSTEM 's'> */
      "\0<\0!\0D\0O\0C\0T\0Y\0P\0E\0 \0d\0 "
      "\0S\0Y\0S\0T\0E\0M\0 \0'\0s\0'\0>\0\n"
      /* <d><e>&en;</e></d> */
      "\0<\0d\0>\0<\0e\0>\0&\0e\0n\0;\0<\0/\0e\0>\0<\0/\0d\0>";
  const XML_Char *expected = XCS("<![IGNORE[<!ELEMENT e (#PCDATA)*>]]>\n&en;");
  CharData storage;

  CharData_Init(&storage);
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetUserData(g_parser, &storage);
  XML_SetExternalEntityRefHandler(g_parser,
                                  external_entity_load_ignore_utf16_be);
  XML_SetDefaultHandler(g_parser, accumulate_characters);
  XML_SetStartDoctypeDeclHandler(g_parser, dummy_start_doctype_handler);
  XML_SetEndDoctypeDeclHandler(g_parser, dummy_end_doctype_handler);
  XML_SetElementDeclHandler(g_parser, dummy_element_decl_handler);
  XML_SetStartElementHandler(g_parser, dummy_start_element);
  XML_SetEndElementHandler(g_parser, dummy_end_element);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

/* Test mis-formatted conditional exclusion */
START_TEST(test_bad_ignore_section) {
  const char *text = "<!DOCTYPE doc SYSTEM 'foo'>\n"
                     "<doc><e>&entity;</e></doc>";
  ExtFaults faults[]
      = {{"<![IGNORE[<!ELEM", "Broken-off declaration not faulted", NULL,
          XML_ERROR_SYNTAX},
         {"<![IGNORE[\x01]]>", "Invalid XML character not faulted", NULL,
          XML_ERROR_INVALID_TOKEN},
         {/* FIrst two bytes of a three-byte char */
          "<![IGNORE[\xe2\x82", "Partial XML character not faulted", NULL,
          XML_ERROR_PARTIAL_CHAR},
         {NULL, NULL, NULL, XML_ERROR_NONE}};
  ExtFaults *fault;

  for (fault = &faults[0]; fault->parse_text != NULL; fault++) {
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_faulter);
    XML_SetUserData(g_parser, fault);
    expect_failure(text, XML_ERROR_EXTERNAL_ENTITY_HANDLING,
                   "Incomplete IGNORE section not failed");
    XML_ParserReset(g_parser, NULL);
  }
}
END_TEST

/* Test recursive parsing */
static int XMLCALL
external_entity_valuer(XML_Parser parser, const XML_Char *context,
                       const XML_Char *base, const XML_Char *systemId,
                       const XML_Char *publicId) {
  const char *text1 = "<!ELEMENT doc EMPTY>\n"
                      "<!ENTITY % e1 SYSTEM '004-2.ent'>\n"
                      "<!ENTITY % e2 '%e1;'>\n"
                      "%e1;\n";
  XML_Parser ext_parser;

  UNUSED_P(base);
  UNUSED_P(publicId);
  if (systemId == NULL)
    return XML_STATUS_OK;
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser");
  if (! xcstrcmp(systemId, XCS("004-1.ent"))) {
    if (_XML_Parse_SINGLE_BYTES(ext_parser, text1, (int)strlen(text1), XML_TRUE)
        == XML_STATUS_ERROR)
      xml_failure(ext_parser);
  } else if (! xcstrcmp(systemId, XCS("004-2.ent"))) {
    ExtFaults *fault = (ExtFaults *)XML_GetUserData(parser);
    enum XML_Status status;
    enum XML_Error error;

    status = _XML_Parse_SINGLE_BYTES(ext_parser, fault->parse_text,
                                     (int)strlen(fault->parse_text), XML_TRUE);
    if (fault->error == XML_ERROR_NONE) {
      if (status == XML_STATUS_ERROR)
        xml_failure(ext_parser);
    } else {
      if (status != XML_STATUS_ERROR)
        fail(fault->fail_text);
      error = XML_GetErrorCode(ext_parser);
      if (error != fault->error
          && (fault->error != XML_ERROR_XML_DECL
              || error != XML_ERROR_TEXT_DECL))
        xml_failure(ext_parser);
    }
  }

  XML_ParserFree(ext_parser);
  return XML_STATUS_OK;
}

START_TEST(test_external_entity_values) {
  const char *text = "<!DOCTYPE doc SYSTEM '004-1.ent'>\n"
                     "<doc></doc>\n";
  ExtFaults data_004_2[] = {
      {"<!ATTLIST doc a1 CDATA 'value'>", NULL, NULL, XML_ERROR_NONE},
      {"<!ATTLIST $doc a1 CDATA 'value'>", "Invalid token not faulted", NULL,
       XML_ERROR_INVALID_TOKEN},
      {"'wombat", "Unterminated string not faulted", NULL,
       XML_ERROR_UNCLOSED_TOKEN},
      {"\xe2\x82", "Partial UTF-8 character not faulted", NULL,
       XML_ERROR_PARTIAL_CHAR},
      {"<?xml version='1.0' encoding='utf-8'?>\n", NULL, NULL, XML_ERROR_NONE},
      {"<?xml?>", "Malformed XML declaration not faulted", NULL,
       XML_ERROR_XML_DECL},
      {/* UTF-8 BOM */
       "\xEF\xBB\xBF<!ATTLIST doc a1 CDATA 'value'>", NULL, NULL,
       XML_ERROR_NONE},
      {"<?xml version='1.0' encoding='utf-8'?>\n$",
       "Invalid token after text declaration not faulted", NULL,
       XML_ERROR_INVALID_TOKEN},
      {"<?xml version='1.0' encoding='utf-8'?>\n'wombat",
       "Unterminated string after text decl not faulted", NULL,
       XML_ERROR_UNCLOSED_TOKEN},
      {"<?xml version='1.0' encoding='utf-8'?>\n\xe2\x82",
       "Partial UTF-8 character after text decl not faulted", NULL,
       XML_ERROR_PARTIAL_CHAR},
      {"%e1;", "Recursive parameter entity not faulted", NULL,
       XML_ERROR_RECURSIVE_ENTITY_REF},
      {NULL, NULL, NULL, XML_ERROR_NONE}};
  int i;

  for (i = 0; data_004_2[i].parse_text != NULL; i++) {
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_valuer);
    XML_SetUserData(g_parser, &data_004_2[i]);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        == XML_STATUS_ERROR)
      xml_failure(g_parser);
    XML_ParserReset(g_parser, NULL);
  }
}
END_TEST

/* Test the recursive parse interacts with a not standalone handler */
static int XMLCALL
external_entity_not_standalone(XML_Parser parser, const XML_Char *context,
                               const XML_Char *base, const XML_Char *systemId,
                               const XML_Char *publicId) {
  const char *text1 = "<!ELEMENT doc EMPTY>\n"
                      "<!ENTITY % e1 SYSTEM 'bar'>\n"
                      "%e1;\n";
  const char *text2 = "<!ATTLIST doc a1 CDATA 'value'>";
  XML_Parser ext_parser;

  UNUSED_P(base);
  UNUSED_P(publicId);
  if (systemId == NULL)
    return XML_STATUS_OK;
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser");
  if (! xcstrcmp(systemId, XCS("foo"))) {
    XML_SetNotStandaloneHandler(ext_parser, reject_not_standalone_handler);
    if (_XML_Parse_SINGLE_BYTES(ext_parser, text1, (int)strlen(text1), XML_TRUE)
        != XML_STATUS_ERROR)
      fail("Expected not standalone rejection");
    if (XML_GetErrorCode(ext_parser) != XML_ERROR_NOT_STANDALONE)
      xml_failure(ext_parser);
    XML_SetNotStandaloneHandler(ext_parser, NULL);
    XML_ParserFree(ext_parser);
    return XML_STATUS_ERROR;
  } else if (! xcstrcmp(systemId, XCS("bar"))) {
    if (_XML_Parse_SINGLE_BYTES(ext_parser, text2, (int)strlen(text2), XML_TRUE)
        == XML_STATUS_ERROR)
      xml_failure(ext_parser);
  }

  XML_ParserFree(ext_parser);
  return XML_STATUS_OK;
}

START_TEST(test_ext_entity_not_standalone) {
  const char *text = "<!DOCTYPE doc SYSTEM 'foo'>\n"
                     "<doc></doc>";

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_not_standalone);
  expect_failure(text, XML_ERROR_EXTERNAL_ENTITY_HANDLING,
                 "Standalone rejection not caught");
}
END_TEST

static int XMLCALL
external_entity_value_aborter(XML_Parser parser, const XML_Char *context,
                              const XML_Char *base, const XML_Char *systemId,
                              const XML_Char *publicId) {
  const char *text1 = "<!ELEMENT doc EMPTY>\n"
                      "<!ENTITY % e1 SYSTEM '004-2.ent'>\n"
                      "<!ENTITY % e2 '%e1;'>\n"
                      "%e1;\n";
  const char *text2 = "<?xml version='1.0' encoding='utf-8'?>";
  XML_Parser ext_parser;

  UNUSED_P(base);
  UNUSED_P(publicId);
  if (systemId == NULL)
    return XML_STATUS_OK;
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser");
  if (! xcstrcmp(systemId, XCS("004-1.ent"))) {
    if (_XML_Parse_SINGLE_BYTES(ext_parser, text1, (int)strlen(text1), XML_TRUE)
        == XML_STATUS_ERROR)
      xml_failure(ext_parser);
  }
  if (! xcstrcmp(systemId, XCS("004-2.ent"))) {
    XML_SetXmlDeclHandler(ext_parser, entity_suspending_xdecl_handler);
    XML_SetUserData(ext_parser, ext_parser);
    if (_XML_Parse_SINGLE_BYTES(ext_parser, text2, (int)strlen(text2), XML_TRUE)
        != XML_STATUS_ERROR)
      fail("Aborted parse not faulted");
    if (XML_GetErrorCode(ext_parser) != XML_ERROR_ABORTED)
      xml_failure(ext_parser);
  }

  XML_ParserFree(ext_parser);
  return XML_STATUS_OK;
}

START_TEST(test_ext_entity_value_abort) {
  const char *text = "<!DOCTYPE doc SYSTEM '004-1.ent'>\n"
                     "<doc></doc>\n";

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_value_aborter);
  resumable = XML_FALSE;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

START_TEST(test_bad_public_doctype) {
  const char *text = "<?xml version='1.0' encoding='utf-8'?>\n"
                     "<!DOCTYPE doc PUBLIC '{BadName}' 'test'>\n"
                     "<doc></doc>";

  /* Setting a handler provokes a particular code path */
  XML_SetDoctypeDeclHandler(g_parser, dummy_start_doctype_handler,
                            dummy_end_doctype_handler);
  expect_failure(text, XML_ERROR_PUBLICID, "Bad Public ID not failed");
}
END_TEST

/* Test based on ibm/valid/P32/ibm32v04.xml */
START_TEST(test_attribute_enum_value) {
  const char *text = "<?xml version='1.0' standalone='no'?>\n"
                     "<!DOCTYPE animal SYSTEM 'test.dtd'>\n"
                     "<animal>This is a \n    <a/>  \n\nyellow tiger</animal>";
  ExtTest dtd_data
      = {"<!ELEMENT animal (#PCDATA|a)*>\n"
         "<!ELEMENT a EMPTY>\n"
         "<!ATTLIST animal xml:space (default|preserve) 'preserve'>",
         NULL, NULL};
  const XML_Char *expected = XCS("This is a \n      \n\nyellow tiger");

  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader);
  XML_SetUserData(g_parser, &dtd_data);
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  /* An attribute list handler provokes a different code path */
  XML_SetAttlistDeclHandler(g_parser, dummy_attlist_decl_handler);
  run_ext_character_check(text, &dtd_data, expected);
}
END_TEST

/* Slightly bizarrely, the library seems to silently ignore entity
 * definitions for predefined entities, even when they are wrong.  The
 * language of the XML 1.0 spec is somewhat unhelpful as to what ought
 * to happen, so this is currently treated as acceptable.
 */
START_TEST(test_predefined_entity_redefinition) {
  const char *text = "<!DOCTYPE doc [\n"
                     "<!ENTITY apos 'foo'>\n"
                     "]>\n"
                     "<doc>&apos;</doc>";
  run_character_check(text, XCS("'"));
}
END_TEST

/* Test that the parser stops processing the DTD after an unresolved
 * parameter entity is encountered.
 */
START_TEST(test_dtd_stop_processing) {
  const char *text = "<!DOCTYPE doc [\n"
                     "%foo;\n"
                     "<!ENTITY bar 'bas'>\n"
                     "]><doc/>";

  XML_SetEntityDeclHandler(g_parser, dummy_entity_decl_handler);
  dummy_handler_flags = 0;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  if (dummy_handler_flags != 0)
    fail("DTD processing still going after undefined PE");
}
END_TEST

/* Test public notations with no system ID */
START_TEST(test_public_notation_no_sysid) {
  const char *text = "<!DOCTYPE doc [\n"
                     "<!NOTATION note PUBLIC 'foo'>\n"
                     "<!ELEMENT doc EMPTY>\n"
                     "]>\n<doc/>";

  dummy_handler_flags = 0;
  XML_SetNotationDeclHandler(g_parser, dummy_notation_decl_handler);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  if (dummy_handler_flags != DUMMY_NOTATION_DECL_HANDLER_FLAG)
    fail("Notation declaration handler not called");
}
END_TEST

static void XMLCALL
record_element_start_handler(void *userData, const XML_Char *name,
                             const XML_Char **atts) {
  UNUSED_P(atts);
  CharData_AppendXMLChars((CharData *)userData, name, (int)xcstrlen(name));
}

START_TEST(test_nested_groups) {
  const char *text
      = "<!DOCTYPE doc [\n"
        "<!ELEMENT doc "
        /* Sixteen elements per line */
        "(e,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,"
        "(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?"
        "))))))))))))))))))))))))))))))))>\n"
        "<!ELEMENT e EMPTY>"
        "]>\n"
        "<doc><e/></doc>";
  CharData storage;

  CharData_Init(&storage);
  XML_SetElementDeclHandler(g_parser, dummy_element_decl_handler);
  XML_SetStartElementHandler(g_parser, record_element_start_handler);
  XML_SetUserData(g_parser, &storage);
  dummy_handler_flags = 0;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, XCS("doce"));
  if (dummy_handler_flags != DUMMY_ELEMENT_DECL_HANDLER_FLAG)
    fail("Element handler not fired");
}
END_TEST

START_TEST(test_group_choice) {
  const char *text = "<!DOCTYPE doc [\n"
                     "<!ELEMENT doc (a|b|c)+>\n"
                     "<!ELEMENT a EMPTY>\n"
                     "<!ELEMENT b (#PCDATA)>\n"
                     "<!ELEMENT c ANY>\n"
                     "]>\n"
                     "<doc>\n"
                     "<a/>\n"
                     "<b attr='foo'>This is a foo</b>\n"
                     "<c></c>\n"
                     "</doc>\n";

  XML_SetElementDeclHandler(g_parser, dummy_element_decl_handler);
  dummy_handler_flags = 0;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  if (dummy_handler_flags != DUMMY_ELEMENT_DECL_HANDLER_FLAG)
    fail("Element handler flag not raised");
}
END_TEST

static int XMLCALL
external_entity_public(XML_Parser parser, const XML_Char *context,
                       const XML_Char *base, const XML_Char *systemId,
                       const XML_Char *publicId) {
  const char *text1 = (const char *)XML_GetUserData(parser);
  const char *text2 = "<!ATTLIST doc a CDATA 'value'>";
  const char *text = NULL;
  XML_Parser ext_parser;
  int parse_res;

  UNUSED_P(base);
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    return XML_STATUS_ERROR;
  if (systemId != NULL && ! xcstrcmp(systemId, XCS("http://example.org/"))) {
    text = text1;
  } else if (publicId != NULL && ! xcstrcmp(publicId, XCS("foo"))) {
    text = text2;
  } else
    fail("Unexpected parameters to external entity parser");
  assert(text != NULL);
  parse_res
      = _XML_Parse_SINGLE_BYTES(ext_parser, text, (int)strlen(text), XML_TRUE);
  XML_ParserFree(ext_parser);
  return parse_res;
}

START_TEST(test_standalone_parameter_entity) {
  const char *text = "<?xml version='1.0' standalone='yes'?>\n"
                     "<!DOCTYPE doc SYSTEM 'http://example.org/' [\n"
                     "<!ENTITY % entity '<!ELEMENT doc (#PCDATA)>'>\n"
                     "%entity;\n"
                     "]>\n"
                     "<doc></doc>";
  char dtd_data[] = "<!ENTITY % e1 'foo'>\n";

  XML_SetUserData(g_parser, dtd_data);
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_public);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Test skipping of parameter entity in an external DTD */
/* Derived from ibm/invalid/P69/ibm69i01.xml */
START_TEST(test_skipped_parameter_entity) {
  const char *text = "<?xml version='1.0'?>\n"
                     "<!DOCTYPE root SYSTEM 'http://example.org/dtd.ent' [\n"
                     "<!ELEMENT root (#PCDATA|a)* >\n"
                     "]>\n"
                     "<root></root>";
  ExtTest dtd_data = {"%pe2;", NULL, NULL};

  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader);
  XML_SetUserData(g_parser, &dtd_data);
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetSkippedEntityHandler(g_parser, dummy_skip_handler);
  dummy_handler_flags = 0;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  if (dummy_handler_flags != DUMMY_SKIP_HANDLER_FLAG)
    fail("Skip handler not executed");
}
END_TEST

/* Test recursive parameter entity definition rejected in external DTD */
START_TEST(test_recursive_external_parameter_entity) {
  const char *text = "<?xml version='1.0'?>\n"
                     "<!DOCTYPE root SYSTEM 'http://example.org/dtd.ent' [\n"
                     "<!ELEMENT root (#PCDATA|a)* >\n"
                     "]>\n"
                     "<root></root>";
  ExtFaults dtd_data = {"<!ENTITY % pe2 '&#37;pe2;'>\n%pe2;",
                        "Recursive external parameter entity not faulted", NULL,
                        XML_ERROR_RECURSIVE_ENTITY_REF};

  XML_SetExternalEntityRefHandler(g_parser, external_entity_faulter);
  XML_SetUserData(g_parser, &dtd_data);
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  expect_failure(text, XML_ERROR_EXTERNAL_ENTITY_HANDLING,
                 "Recursive external parameter not spotted");
}
END_TEST

/* Test undefined parameter entity in external entity handler */
static int XMLCALL
external_entity_devaluer(XML_Parser parser, const XML_Char *context,
                         const XML_Char *base, const XML_Char *systemId,
                         const XML_Char *publicId) {
  const char *text = "<!ELEMENT doc EMPTY>\n"
                     "<!ENTITY % e1 SYSTEM 'bar'>\n"
                     "%e1;\n";
  XML_Parser ext_parser;
  intptr_t clear_handler = (intptr_t)XML_GetUserData(parser);

  UNUSED_P(base);
  UNUSED_P(publicId);
  if (systemId == NULL || ! xcstrcmp(systemId, XCS("bar")))
    return XML_STATUS_OK;
  if (xcstrcmp(systemId, XCS("foo")))
    fail("Unexpected system ID");
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could note create external entity parser");
  if (clear_handler)
    XML_SetExternalEntityRefHandler(ext_parser, NULL);
  if (_XML_Parse_SINGLE_BYTES(ext_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(ext_parser);

  XML_ParserFree(ext_parser);
  return XML_STATUS_OK;
}

START_TEST(test_undefined_ext_entity_in_external_dtd) {
  const char *text = "<!DOCTYPE doc SYSTEM 'foo'>\n"
                     "<doc></doc>\n";

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_devaluer);
  XML_SetUserData(g_parser, (void *)(intptr_t)XML_FALSE);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);

  /* Now repeat without the external entity ref handler invoking
   * another copy of itself.
   */
  XML_ParserReset(g_parser, NULL);
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_devaluer);
  XML_SetUserData(g_parser, (void *)(intptr_t)XML_TRUE);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

static void XMLCALL
aborting_xdecl_handler(void *userData, const XML_Char *version,
                       const XML_Char *encoding, int standalone) {
  UNUSED_P(userData);
  UNUSED_P(version);
  UNUSED_P(encoding);
  UNUSED_P(standalone);
  XML_StopParser(g_parser, resumable);
  XML_SetXmlDeclHandler(g_parser, NULL);
}

/* Test suspending the parse on receiving an XML declaration works */
START_TEST(test_suspend_xdecl) {
  const char *text = long_character_data_text;

  XML_SetXmlDeclHandler(g_parser, aborting_xdecl_handler);
  resumable = XML_TRUE;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_SUSPENDED)
    xml_failure(g_parser);
  if (XML_GetErrorCode(g_parser) != XML_ERROR_NONE)
    xml_failure(g_parser);
  /* Attempt to start a new parse while suspended */
  if (XML_Parse(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_ERROR)
    fail("Attempt to parse while suspended not faulted");
  if (XML_GetErrorCode(g_parser) != XML_ERROR_SUSPENDED)
    fail("Suspended parse not faulted with correct error");
}
END_TEST

/* Test aborting the parse in an epilog works */
static void XMLCALL
selective_aborting_default_handler(void *userData, const XML_Char *s, int len) {
  const XML_Char *match = (const XML_Char *)userData;

  if (match == NULL
      || (xcstrlen(match) == (unsigned)len && ! xcstrncmp(match, s, len))) {
    XML_StopParser(g_parser, resumable);
    XML_SetDefaultHandler(g_parser, NULL);
  }
}

START_TEST(test_abort_epilog) {
  const char *text = "<doc></doc>\n\r\n";
  XML_Char match[] = XCS("\r");

  XML_SetDefaultHandler(g_parser, selective_aborting_default_handler);
  XML_SetUserData(g_parser, match);
  resumable = XML_FALSE;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_ERROR)
    fail("Abort not triggered");
  if (XML_GetErrorCode(g_parser) != XML_ERROR_ABORTED)
    xml_failure(g_parser);
}
END_TEST

/* Test a different code path for abort in the epilog */
START_TEST(test_abort_epilog_2) {
  const char *text = "<doc></doc>\n";
  XML_Char match[] = XCS("\n");

  XML_SetDefaultHandler(g_parser, selective_aborting_default_handler);
  XML_SetUserData(g_parser, match);
  resumable = XML_FALSE;
  expect_failure(text, XML_ERROR_ABORTED, "Abort not triggered");
}
END_TEST

/* Test suspension from the epilog */
START_TEST(test_suspend_epilog) {
  const char *text = "<doc></doc>\n";
  XML_Char match[] = XCS("\n");

  XML_SetDefaultHandler(g_parser, selective_aborting_default_handler);
  XML_SetUserData(g_parser, match);
  resumable = XML_TRUE;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_SUSPENDED)
    xml_failure(g_parser);
}
END_TEST

static void XMLCALL
suspending_end_handler(void *userData, const XML_Char *s) {
  UNUSED_P(s);
  XML_StopParser((XML_Parser)userData, 1);
}

START_TEST(test_suspend_in_sole_empty_tag) {
  const char *text = "<doc/>";
  enum XML_Status rc;

  XML_SetEndElementHandler(g_parser, suspending_end_handler);
  XML_SetUserData(g_parser, g_parser);
  rc = _XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE);
  if (rc == XML_STATUS_ERROR)
    xml_failure(g_parser);
  else if (rc != XML_STATUS_SUSPENDED)
    fail("Suspend not triggered");
  rc = XML_ResumeParser(g_parser);
  if (rc == XML_STATUS_ERROR)
    xml_failure(g_parser);
  else if (rc != XML_STATUS_OK)
    fail("Resume failed");
}
END_TEST

START_TEST(test_unfinished_epilog) {
  const char *text = "<doc></doc><";

  expect_failure(text, XML_ERROR_UNCLOSED_TOKEN,
                 "Incomplete epilog entry not faulted");
}
END_TEST

START_TEST(test_partial_char_in_epilog) {
  const char *text = "<doc></doc>\xe2\x82";

  /* First check that no fault is raised if the parse is not finished */
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_FALSE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  /* Now check that it is faulted once we finish */
  if (XML_ParseBuffer(g_parser, 0, XML_TRUE) != XML_STATUS_ERROR)
    fail("Partial character in epilog not faulted");
  if (XML_GetErrorCode(g_parser) != XML_ERROR_PARTIAL_CHAR)
    xml_failure(g_parser);
}
END_TEST

START_TEST(test_hash_collision) {
  /* For full coverage of the lookup routine, we need to ensure a
   * hash collision even though we can only tell that we have one
   * through breakpoint debugging or coverage statistics.  The
   * following will cause a hash collision on machines with a 64-bit
   * long type; others will have to experiment.  The full coverage
   * tests invoked from qa.sh usually provide a hash collision, but
   * not always.  This is an attempt to provide insurance.
   */
#define COLLIDING_HASH_SALT (unsigned long)_SIP_ULL(0xffffffffU, 0xff99fc90U)
  const char *text
      = "<doc>\n"
        "<a1/><a2/><a3/><a4/><a5/><a6/><a7/><a8/>\n"
        "<b1></b1><b2 attr='foo'>This is a foo</b2><b3></b3><b4></b4>\n"
        "<b5></b5><b6></b6><b7></b7><b8></b8>\n"
        "<c1/><c2/><c3/><c4/><c5/><c6/><c7/><c8/>\n"
        "<d1/><d2/><d3/><d4/><d5/><d6/><d7/>\n"
        "<d8>This triggers the table growth and collides with b2</d8>\n"
        "</doc>\n";

  XML_SetHashSalt(g_parser, COLLIDING_HASH_SALT);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST
#undef COLLIDING_HASH_SALT

/* Test resuming a parse suspended in entity substitution */
static void XMLCALL
start_element_suspender(void *userData, const XML_Char *name,
                        const XML_Char **atts) {
  UNUSED_P(userData);
  UNUSED_P(atts);
  if (! xcstrcmp(name, XCS("suspend")))
    XML_StopParser(g_parser, XML_TRUE);
  if (! xcstrcmp(name, XCS("abort")))
    XML_StopParser(g_parser, XML_FALSE);
}

START_TEST(test_suspend_resume_internal_entity) {
  const char *text
      = "<!DOCTYPE doc [\n"
        "<!ENTITY foo '<suspend>Hi<suspend>Ho</suspend></suspend>'>\n"
        "]>\n"
        "<doc>&foo;</doc>\n";
  const XML_Char *expected1 = XCS("Hi");
  const XML_Char *expected2 = XCS("HiHo");
  CharData storage;

  CharData_Init(&storage);
  XML_SetStartElementHandler(g_parser, start_element_suspender);
  XML_SetCharacterDataHandler(g_parser, accumulate_characters);
  XML_SetUserData(g_parser, &storage);
  if (XML_Parse(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_SUSPENDED)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, XCS(""));
  if (XML_ResumeParser(g_parser) != XML_STATUS_SUSPENDED)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected1);
  if (XML_ResumeParser(g_parser) != XML_STATUS_OK)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected2);
}
END_TEST

static void XMLCALL
suspending_comment_handler(void *userData, const XML_Char *data) {
  UNUSED_P(data);
  XML_Parser parser = (XML_Parser)userData;
  XML_StopParser(parser, XML_TRUE);
}

START_TEST(test_suspend_resume_internal_entity_issue_629) {
  const char *const text
      = "<!DOCTYPE a [<!ENTITY e '<!--COMMENT-->a'>]><a>&e;<b>\n"
        "<"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "/>"
        "</b></a>";
  const size_t firstChunkSizeBytes = 54;

  XML_Parser parser = XML_ParserCreate(NULL);
  XML_SetUserData(parser, parser);
  XML_SetCommentHandler(parser, suspending_comment_handler);

  if (XML_Parse(parser, text, (int)firstChunkSizeBytes, XML_FALSE)
      != XML_STATUS_SUSPENDED)
    xml_failure(parser);
  if (XML_ResumeParser(parser) != XML_STATUS_OK)
    xml_failure(parser);
  if (XML_Parse(parser, text + firstChunkSizeBytes,
                (int)(strlen(text) - firstChunkSizeBytes), XML_TRUE)
      != XML_STATUS_OK)
    xml_failure(parser);
  XML_ParserFree(parser);
}
END_TEST

/* Test syntax error is caught at parse resumption */
START_TEST(test_resume_entity_with_syntax_error) {
  const char *text = "<!DOCTYPE doc [\n"
                     "<!ENTITY foo '<suspend>Hi</wombat>'>\n"
                     "]>\n"
                     "<doc>&foo;</doc>\n";

  XML_SetStartElementHandler(g_parser, start_element_suspender);
  if (XML_Parse(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_SUSPENDED)
    xml_failure(g_parser);
  if (XML_ResumeParser(g_parser) != XML_STATUS_ERROR)
    fail("Syntax error in entity not faulted");
  if (XML_GetErrorCode(g_parser) != XML_ERROR_TAG_MISMATCH)
    xml_failure(g_parser);
}
END_TEST

/* Test suspending and resuming in a parameter entity substitution */
static void XMLCALL
element_decl_suspender(void *userData, const XML_Char *name,
                       XML_Content *model) {
  UNUSED_P(userData);
  UNUSED_P(name);
  XML_StopParser(g_parser, XML_TRUE);
  XML_FreeContentModel(g_parser, model);
}

START_TEST(test_suspend_resume_parameter_entity) {
  const char *text = "<!DOCTYPE doc [\n"
                     "<!ENTITY % foo '<!ELEMENT doc (#PCDATA)*>'>\n"
                     "%foo;\n"
                     "]>\n"
                     "<doc>Hello, world</doc>";
  const XML_Char *expected = XCS("Hello, world");
  CharData storage;

  CharData_Init(&storage);
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetElementDeclHandler(g_parser, element_decl_suspender);
  XML_SetCharacterDataHandler(g_parser, accumulate_characters);
  XML_SetUserData(g_parser, &storage);
  if (XML_Parse(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_SUSPENDED)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, XCS(""));
  if (XML_ResumeParser(g_parser) != XML_STATUS_OK)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

/* Test attempting to use parser after an error is faulted */
START_TEST(test_restart_on_error) {
  const char *text = "<$doc><doc></doc>";

  if (XML_Parse(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_ERROR)
    fail("Invalid tag name not faulted");
  if (XML_GetErrorCode(g_parser) != XML_ERROR_INVALID_TOKEN)
    xml_failure(g_parser);
  if (XML_Parse(g_parser, NULL, 0, XML_TRUE) != XML_STATUS_ERROR)
    fail("Restarting invalid parse not faulted");
  if (XML_GetErrorCode(g_parser) != XML_ERROR_INVALID_TOKEN)
    xml_failure(g_parser);
}
END_TEST

/* Test that angle brackets in an attribute default value are faulted */
START_TEST(test_reject_lt_in_attribute_value) {
  const char *text = "<!DOCTYPE doc [<!ATTLIST doc a CDATA '<bar>'>]>\n"
                     "<doc></doc>";

  expect_failure(text, XML_ERROR_INVALID_TOKEN,
                 "Bad attribute default not faulted");
}
END_TEST

START_TEST(test_reject_unfinished_param_in_att_value) {
  const char *text = "<!DOCTYPE doc [<!ATTLIST doc a CDATA '&foo'>]>\n"
                     "<doc></doc>";

  expect_failure(text, XML_ERROR_INVALID_TOKEN,
                 "Bad attribute default not faulted");
}
END_TEST

START_TEST(test_trailing_cr_in_att_value) {
  const char *text = "<doc a='value\r'/>";

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Try parsing a general entity within a parameter entity in a
 * standalone internal DTD.  Covers a corner case in the parser.
 */
START_TEST(test_standalone_internal_entity) {
  const char *text = "<?xml version='1.0' standalone='yes' ?>\n"
                     "<!DOCTYPE doc [\n"
                     "  <!ELEMENT doc (#PCDATA)>\n"
                     "  <!ENTITY % pe '<!ATTLIST doc att2 CDATA \"&ge;\">'>\n"
                     "  <!ENTITY ge 'AttDefaultValue'>\n"
                     "  %pe;\n"
                     "]>\n"
                     "<doc att2='any'/>";

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Test that a reference to an unknown external entity is skipped */
START_TEST(test_skipped_external_entity) {
  const char *text = "<!DOCTYPE doc SYSTEM 'http://example.org/'>\n"
                     "<doc></doc>\n";
  ExtTest test_data = {"<!ELEMENT doc EMPTY>\n"
                       "<!ENTITY % e2 '%e1;'>\n",
                       NULL, NULL};

  XML_SetUserData(g_parser, &test_data);
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Test a different form of unknown external entity */
typedef struct ext_hdlr_data {
  const char *parse_text;
  XML_ExternalEntityRefHandler handler;
} ExtHdlrData;

static int XMLCALL
external_entity_oneshot_loader(XML_Parser parser, const XML_Char *context,
                               const XML_Char *base, const XML_Char *systemId,
                               const XML_Char *publicId) {
  ExtHdlrData *test_data = (ExtHdlrData *)XML_GetUserData(parser);
  XML_Parser ext_parser;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser.");
  /* Use the requested entity parser for further externals */
  XML_SetExternalEntityRefHandler(ext_parser, test_data->handler);
  if (_XML_Parse_SINGLE_BYTES(ext_parser, test_data->parse_text,
                              (int)strlen(test_data->parse_text), XML_TRUE)
      == XML_STATUS_ERROR) {
    xml_failure(ext_parser);
  }

  XML_ParserFree(ext_parser);
  return XML_STATUS_OK;
}

START_TEST(test_skipped_null_loaded_ext_entity) {
  const char *text = "<!DOCTYPE doc SYSTEM 'http://example.org/one.ent'>\n"
                     "<doc />";
  ExtHdlrData test_data
      = {"<!ENTITY % pe1 SYSTEM 'http://example.org/two.ent'>\n"
         "<!ENTITY % pe2 '%pe1;'>\n"
         "%pe2;\n",
         external_entity_null_loader};

  XML_SetUserData(g_parser, &test_data);
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_oneshot_loader);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

START_TEST(test_skipped_unloaded_ext_entity) {
  const char *text = "<!DOCTYPE doc SYSTEM 'http://example.org/one.ent'>\n"
                     "<doc />";
  ExtHdlrData test_data
      = {"<!ENTITY % pe1 SYSTEM 'http://example.org/two.ent'>\n"
         "<!ENTITY % pe2 '%pe1;'>\n"
         "%pe2;\n",
         NULL};

  XML_SetUserData(g_parser, &test_data);
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_oneshot_loader);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Test that a parameter entity value ending with a carriage return
 * has it translated internally into a newline.
 */
START_TEST(test_param_entity_with_trailing_cr) {
#define PARAM_ENTITY_NAME "pe"
#define PARAM_ENTITY_CORE_VALUE "<!ATTLIST doc att CDATA \"default\">"
  const char *text = "<!DOCTYPE doc SYSTEM 'http://example.org/'>\n"
                     "<doc/>";
  ExtTest test_data
      = {"<!ENTITY % " PARAM_ENTITY_NAME " '" PARAM_ENTITY_CORE_VALUE "\r'>\n"
         "%" PARAM_ENTITY_NAME ";\n",
         NULL, NULL};

  XML_SetUserData(g_parser, &test_data);
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader);
  XML_SetEntityDeclHandler(g_parser, param_entity_match_handler);
  entity_name_to_match = XCS(PARAM_ENTITY_NAME);
  entity_value_to_match = XCS(PARAM_ENTITY_CORE_VALUE) XCS("\n");
  entity_match_flag = ENTITY_MATCH_NOT_FOUND;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  if (entity_match_flag == ENTITY_MATCH_FAIL)
    fail("Parameter entity CR->NEWLINE conversion failed");
  else if (entity_match_flag == ENTITY_MATCH_NOT_FOUND)
    fail("Parameter entity not parsed");
}
#undef PARAM_ENTITY_NAME
#undef PARAM_ENTITY_CORE_VALUE
END_TEST

START_TEST(test_invalid_character_entity) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY entity '&#x110000;'>\n"
                     "]>\n"
                     "<doc>&entity;</doc>";

  expect_failure(text, XML_ERROR_BAD_CHAR_REF,
                 "Out of range character reference not faulted");
}
END_TEST

START_TEST(test_invalid_character_entity_2) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY entity '&#xg0;'>\n"
                     "]>\n"
                     "<doc>&entity;</doc>";

  expect_failure(text, XML_ERROR_INVALID_TOKEN,
                 "Out of range character reference not faulted");
}
END_TEST

START_TEST(test_invalid_character_entity_3) {
  const char text[] =
      /* <!DOCTYPE doc [\n */
      "\0<\0!\0D\0O\0C\0T\0Y\0P\0E\0 \0d\0o\0c\0 \0[\0\n"
      /* U+0E04 = KHO KHWAI
       * U+0E08 = CHO CHAN */
      /* <!ENTITY entity '&\u0e04\u0e08;'>\n */
      "\0<\0!\0E\0N\0T\0I\0T\0Y\0 \0e\0n\0t\0i\0t\0y\0 "
      "\0'\0&\x0e\x04\x0e\x08\0;\0'\0>\0\n"
      /* ]>\n */
      "\0]\0>\0\n"
      /* <doc>&entity;</doc> */
      "\0<\0d\0o\0c\0>\0&\0e\0n\0t\0i\0t\0y\0;\0<\0/\0d\0o\0c\0>";

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      != XML_STATUS_ERROR)
    fail("Invalid start of entity name not faulted");
  if (XML_GetErrorCode(g_parser) != XML_ERROR_UNDEFINED_ENTITY)
    xml_failure(g_parser);
}
END_TEST

START_TEST(test_invalid_character_entity_4) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY entity '&#1114112;'>\n" /* = &#x110000 */
                     "]>\n"
                     "<doc>&entity;</doc>";

  expect_failure(text, XML_ERROR_BAD_CHAR_REF,
                 "Out of range character reference not faulted");
}
END_TEST

/* Test that processing instructions are picked up by a default handler */
START_TEST(test_pi_handled_in_default) {
  const char *text = "<?test processing instruction?>\n<doc/>";
  const XML_Char *expected = XCS("<?test processing instruction?>\n<doc/>");
  CharData storage;

  CharData_Init(&storage);
  XML_SetDefaultHandler(g_parser, accumulate_characters);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

/* Test that comments are picked up by a default handler */
START_TEST(test_comment_handled_in_default) {
  const char *text = "<!-- This is a comment -->\n<doc/>";
  const XML_Char *expected = XCS("<!-- This is a comment -->\n<doc/>");
  CharData storage;

  CharData_Init(&storage);
  XML_SetDefaultHandler(g_parser, accumulate_characters);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

/* Test PIs that look almost but not quite like XML declarations */
static void XMLCALL
accumulate_pi_characters(void *userData, const XML_Char *target,
                         const XML_Char *data) {
  CharData *storage = (CharData *)userData;

  CharData_AppendXMLChars(storage, target, -1);
  CharData_AppendXMLChars(storage, XCS(": "), 2);
  CharData_AppendXMLChars(storage, data, -1);
  CharData_AppendXMLChars(storage, XCS("\n"), 1);
}

START_TEST(test_pi_yml) {
  const char *text = "<?yml something like data?><doc/>";
  const XML_Char *expected = XCS("yml: something like data\n");
  CharData storage;

  CharData_Init(&storage);
  XML_SetProcessingInstructionHandler(g_parser, accumulate_pi_characters);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

START_TEST(test_pi_xnl) {
  const char *text = "<?xnl nothing like data?><doc/>";
  const XML_Char *expected = XCS("xnl: nothing like data\n");
  CharData storage;

  CharData_Init(&storage);
  XML_SetProcessingInstructionHandler(g_parser, accumulate_pi_characters);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

START_TEST(test_pi_xmm) {
  const char *text = "<?xmm everything like data?><doc/>";
  const XML_Char *expected = XCS("xmm: everything like data\n");
  CharData storage;

  CharData_Init(&storage);
  XML_SetProcessingInstructionHandler(g_parser, accumulate_pi_characters);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

START_TEST(test_utf16_pi) {
  const char text[] =
      /* <?{KHO KHWAI}{CHO CHAN}?>
       * where {KHO KHWAI} = U+0E04
       * and   {CHO CHAN}  = U+0E08
       */
      "<\0?\0\x04\x0e\x08\x0e?\0>\0"
      /* <q/> */
      "<\0q\0/\0>\0";
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\x0e04\x0e08: \n");
#else
  const XML_Char *expected = XCS("\xe0\xb8\x84\xe0\xb8\x88: \n");
#endif
  CharData storage;

  CharData_Init(&storage);
  XML_SetProcessingInstructionHandler(g_parser, accumulate_pi_characters);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

START_TEST(test_utf16_be_pi) {
  const char text[] =
      /* <?{KHO KHWAI}{CHO CHAN}?>
       * where {KHO KHWAI} = U+0E04
       * and   {CHO CHAN}  = U+0E08
       */
      "\0<\0?\x0e\x04\x0e\x08\0?\0>"
      /* <q/> */
      "\0<\0q\0/\0>";
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\x0e04\x0e08: \n");
#else
  const XML_Char *expected = XCS("\xe0\xb8\x84\xe0\xb8\x88: \n");
#endif
  CharData storage;

  CharData_Init(&storage);
  XML_SetProcessingInstructionHandler(g_parser, accumulate_pi_characters);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

/* Test that comments can be picked up and translated */
static void XMLCALL
accumulate_comment(void *userData, const XML_Char *data) {
  CharData *storage = (CharData *)userData;

  CharData_AppendXMLChars(storage, data, -1);
}

START_TEST(test_utf16_be_comment) {
  const char text[] =
      /* <!-- Comment A --> */
      "\0<\0!\0-\0-\0 \0C\0o\0m\0m\0e\0n\0t\0 \0A\0 \0-\0-\0>\0\n"
      /* <doc/> */
      "\0<\0d\0o\0c\0/\0>";
  const XML_Char *expected = XCS(" Comment A ");
  CharData storage;

  CharData_Init(&storage);
  XML_SetCommentHandler(g_parser, accumulate_comment);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

START_TEST(test_utf16_le_comment) {
  const char text[] =
      /* <!-- Comment B --> */
      "<\0!\0-\0-\0 \0C\0o\0m\0m\0e\0n\0t\0 \0B\0 \0-\0-\0>\0\n\0"
      /* <doc/> */
      "<\0d\0o\0c\0/\0>\0";
  const XML_Char *expected = XCS(" Comment B ");
  CharData storage;

  CharData_Init(&storage);
  XML_SetCommentHandler(g_parser, accumulate_comment);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

/* Test that the unknown encoding handler with map entries that expect
 * conversion but no conversion function is faulted
 */
static int XMLCALL
failing_converter(void *data, const char *s) {
  UNUSED_P(data);
  UNUSED_P(s);
  /* Always claim to have failed */
  return -1;
}

static int XMLCALL
prefix_converter(void *data, const char *s) {
  UNUSED_P(data);
  /* If the first byte is 0xff, raise an error */
  if (s[0] == (char)-1)
    return -1;
  /* Just add the low bits of the first byte to the second */
  return (s[1] + (s[0] & 0x7f)) & 0x01ff;
}

static int XMLCALL
MiscEncodingHandler(void *data, const XML_Char *encoding, XML_Encoding *info) {
  int i;
  int high_map = -2; /* Assume a 2-byte sequence */

  if (! xcstrcmp(encoding, XCS("invalid-9"))
      || ! xcstrcmp(encoding, XCS("ascii-like"))
      || ! xcstrcmp(encoding, XCS("invalid-len"))
      || ! xcstrcmp(encoding, XCS("invalid-a"))
      || ! xcstrcmp(encoding, XCS("invalid-surrogate"))
      || ! xcstrcmp(encoding, XCS("invalid-high")))
    high_map = -1;

  for (i = 0; i < 128; ++i)
    info->map[i] = i;
  for (; i < 256; ++i)
    info->map[i] = high_map;

  /* If required, put an invalid value in the ASCII entries */
  if (! xcstrcmp(encoding, XCS("invalid-9")))
    info->map[9] = 5;
  /* If required, have a top-bit set character starts a 5-byte sequence */
  if (! xcstrcmp(encoding, XCS("invalid-len")))
    info->map[0x81] = -5;
  /* If required, make a top-bit set character a valid ASCII character */
  if (! xcstrcmp(encoding, XCS("invalid-a")))
    info->map[0x82] = 'a';
  /* If required, give a top-bit set character a forbidden value,
   * what would otherwise be the first of a surrogate pair.
   */
  if (! xcstrcmp(encoding, XCS("invalid-surrogate")))
    info->map[0x83] = 0xd801;
  /* If required, give a top-bit set character too high a value */
  if (! xcstrcmp(encoding, XCS("invalid-high")))
    info->map[0x84] = 0x010101;

  info->data = data;
  info->release = NULL;
  if (! xcstrcmp(encoding, XCS("failing-conv")))
    info->convert = failing_converter;
  else if (! xcstrcmp(encoding, XCS("prefix-conv")))
    info->convert = prefix_converter;
  else
    info->convert = NULL;
  return XML_STATUS_OK;
}

START_TEST(test_missing_encoding_conversion_fn) {
  const char *text = "<?xml version='1.0' encoding='no-conv'?>\n"
                     "<doc>\x81</doc>";

  XML_SetUnknownEncodingHandler(g_parser, MiscEncodingHandler, NULL);
  /* MiscEncodingHandler sets up an encoding with every top-bit-set
   * character introducing a two-byte sequence.  For this, it
   * requires a convert function.  The above function call doesn't
   * pass one through, so when BadEncodingHandler actually gets
   * called it should supply an invalid encoding.
   */
  expect_failure(text, XML_ERROR_UNKNOWN_ENCODING,
                 "Encoding with missing convert() not faulted");
}
END_TEST

START_TEST(test_failing_encoding_conversion_fn) {
  const char *text = "<?xml version='1.0' encoding='failing-conv'?>\n"
                     "<doc>\x81</doc>";

  XML_SetUnknownEncodingHandler(g_parser, MiscEncodingHandler, NULL);
  /* BadEncodingHandler sets up an encoding with every top-bit-set
   * character introducing a two-byte sequence.  For this, it
   * requires a convert function.  The above function call passes
   * one that insists all possible sequences are invalid anyway.
   */
  expect_failure(text, XML_ERROR_INVALID_TOKEN,
                 "Encoding with failing convert() not faulted");
}
END_TEST

/* Test unknown encoding conversions */
START_TEST(test_unknown_encoding_success) {
  const char *text = "<?xml version='1.0' encoding='prefix-conv'?>\n"
                     /* Equivalent to <eoc>Hello, world</eoc> */
                     "<\x81\x64\x80oc>Hello, world</\x81\x64\x80oc>";

  XML_SetUnknownEncodingHandler(g_parser, MiscEncodingHandler, NULL);
  run_character_check(text, XCS("Hello, world"));
}
END_TEST

/* Test bad name character in unknown encoding */
START_TEST(test_unknown_encoding_bad_name) {
  const char *text = "<?xml version='1.0' encoding='prefix-conv'?>\n"
                     "<\xff\x64oc>Hello, world</\xff\x64oc>";

  XML_SetUnknownEncodingHandler(g_parser, MiscEncodingHandler, NULL);
  expect_failure(text, XML_ERROR_INVALID_TOKEN,
                 "Bad name start in unknown encoding not faulted");
}
END_TEST

/* Test bad mid-name character in unknown encoding */
START_TEST(test_unknown_encoding_bad_name_2) {
  const char *text = "<?xml version='1.0' encoding='prefix-conv'?>\n"
                     "<d\xffoc>Hello, world</d\xffoc>";

  XML_SetUnknownEncodingHandler(g_parser, MiscEncodingHandler, NULL);
  expect_failure(text, XML_ERROR_INVALID_TOKEN,
                 "Bad name in unknown encoding not faulted");
}
END_TEST

/* Test element name that is long enough to fill the conversion buffer
 * in an unknown encoding, finishing with an encoded character.
 */
START_TEST(test_unknown_encoding_long_name_1) {
  const char *text = "<?xml version='1.0' encoding='prefix-conv'?>\n"
                     "<abcdefghabcdefghabcdefghijkl\x80m\x80n\x80o\x80p>"
                     "Hi"
                     "</abcdefghabcdefghabcdefghijkl\x80m\x80n\x80o\x80p>";
  const XML_Char *expected = XCS("abcdefghabcdefghabcdefghijklmnop");
  CharData storage;

  CharData_Init(&storage);
  XML_SetUnknownEncodingHandler(g_parser, MiscEncodingHandler, NULL);
  XML_SetStartElementHandler(g_parser, record_element_start_handler);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

/* Test element name that is long enough to fill the conversion buffer
 * in an unknown encoding, finishing with an simple character.
 */
START_TEST(test_unknown_encoding_long_name_2) {
  const char *text = "<?xml version='1.0' encoding='prefix-conv'?>\n"
                     "<abcdefghabcdefghabcdefghijklmnop>"
                     "Hi"
                     "</abcdefghabcdefghabcdefghijklmnop>";
  const XML_Char *expected = XCS("abcdefghabcdefghabcdefghijklmnop");
  CharData storage;

  CharData_Init(&storage);
  XML_SetUnknownEncodingHandler(g_parser, MiscEncodingHandler, NULL);
  XML_SetStartElementHandler(g_parser, record_element_start_handler);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

START_TEST(test_invalid_unknown_encoding) {
  const char *text = "<?xml version='1.0' encoding='invalid-9'?>\n"
                     "<doc>Hello world</doc>";

  XML_SetUnknownEncodingHandler(g_parser, MiscEncodingHandler, NULL);
  expect_failure(text, XML_ERROR_UNKNOWN_ENCODING,
                 "Invalid unknown encoding not faulted");
}
END_TEST

START_TEST(test_unknown_ascii_encoding_ok) {
  const char *text = "<?xml version='1.0' encoding='ascii-like'?>\n"
                     "<doc>Hello, world</doc>";

  XML_SetUnknownEncodingHandler(g_parser, MiscEncodingHandler, NULL);
  run_character_check(text, XCS("Hello, world"));
}
END_TEST

START_TEST(test_unknown_ascii_encoding_fail) {
  const char *text = "<?xml version='1.0' encoding='ascii-like'?>\n"
                     "<doc>Hello, \x80 world</doc>";

  XML_SetUnknownEncodingHandler(g_parser, MiscEncodingHandler, NULL);
  expect_failure(text, XML_ERROR_INVALID_TOKEN,
                 "Invalid character not faulted");
}
END_TEST

START_TEST(test_unknown_encoding_invalid_length) {
  const char *text = "<?xml version='1.0' encoding='invalid-len'?>\n"
                     "<doc>Hello, world</doc>";

  XML_SetUnknownEncodingHandler(g_parser, MiscEncodingHandler, NULL);
  expect_failure(text, XML_ERROR_UNKNOWN_ENCODING,
                 "Invalid unknown encoding not faulted");
}
END_TEST

START_TEST(test_unknown_encoding_invalid_topbit) {
  const char *text = "<?xml version='1.0' encoding='invalid-a'?>\n"
                     "<doc>Hello, world</doc>";

  XML_SetUnknownEncodingHandler(g_parser, MiscEncodingHandler, NULL);
  expect_failure(text, XML_ERROR_UNKNOWN_ENCODING,
                 "Invalid unknown encoding not faulted");
}
END_TEST

START_TEST(test_unknown_encoding_invalid_surrogate) {
  const char *text = "<?xml version='1.0' encoding='invalid-surrogate'?>\n"
                     "<doc>Hello, \x82 world</doc>";

  XML_SetUnknownEncodingHandler(g_parser, MiscEncodingHandler, NULL);
  expect_failure(text, XML_ERROR_INVALID_TOKEN,
                 "Invalid unknown encoding not faulted");
}
END_TEST

START_TEST(test_unknown_encoding_invalid_high) {
  const char *text = "<?xml version='1.0' encoding='invalid-high'?>\n"
                     "<doc>Hello, world</doc>";

  XML_SetUnknownEncodingHandler(g_parser, MiscEncodingHandler, NULL);
  expect_failure(text, XML_ERROR_UNKNOWN_ENCODING,
                 "Invalid unknown encoding not faulted");
}
END_TEST

START_TEST(test_unknown_encoding_invalid_attr_value) {
  const char *text = "<?xml version='1.0' encoding='prefix-conv'?>\n"
                     "<doc attr='\xff\x30'/>";

  XML_SetUnknownEncodingHandler(g_parser, MiscEncodingHandler, NULL);
  expect_failure(text, XML_ERROR_INVALID_TOKEN,
                 "Invalid attribute valid not faulted");
}
END_TEST

/* Test an external entity parser set to use latin-1 detects UTF-16
 * BOMs correctly.
 */
enum ee_parse_flags { EE_PARSE_NONE = 0x00, EE_PARSE_FULL_BUFFER = 0x01 };

typedef struct ExtTest2 {
  const char *parse_text;
  int parse_len;
  const XML_Char *encoding;
  CharData *storage;
  enum ee_parse_flags flags;
} ExtTest2;

static int XMLCALL
external_entity_loader2(XML_Parser parser, const XML_Char *context,
                        const XML_Char *base, const XML_Char *systemId,
                        const XML_Char *publicId) {
  ExtTest2 *test_data = (ExtTest2 *)XML_GetUserData(parser);
  XML_Parser extparser;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  extparser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (extparser == NULL)
    fail("Coulr not create external entity parser");
  if (test_data->encoding != NULL) {
    if (! XML_SetEncoding(extparser, test_data->encoding))
      fail("XML_SetEncoding() ignored for external entity");
  }
  if (test_data->flags & EE_PARSE_FULL_BUFFER) {
    if (XML_Parse(extparser, test_data->parse_text, test_data->parse_len,
                  XML_TRUE)
        == XML_STATUS_ERROR) {
      xml_failure(extparser);
    }
  } else if (_XML_Parse_SINGLE_BYTES(extparser, test_data->parse_text,
                                     test_data->parse_len, XML_TRUE)
             == XML_STATUS_ERROR) {
    xml_failure(extparser);
  }

  XML_ParserFree(extparser);
  return XML_STATUS_OK;
}

/* Test that UTF-16 BOM does not select UTF-16 given explicit encoding */
static void XMLCALL
ext2_accumulate_characters(void *userData, const XML_Char *s, int len) {
  ExtTest2 *test_data = (ExtTest2 *)userData;
  accumulate_characters(test_data->storage, s, len);
}

START_TEST(test_ext_entity_latin1_utf16le_bom) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/dummy.ent'>\n"
                     "]>\n"
                     "<doc>&en;</doc>";
  ExtTest2 test_data
      = {/* If UTF-16, 0xfeff is the BOM and 0x204c is black left bullet */
         /* If Latin-1, 0xff = Y-diaeresis, 0xfe = lowercase thorn,
          *   0x4c = L and 0x20 is a space
          */
         "\xff\xfe\x4c\x20", 4, XCS("iso-8859-1"), NULL, EE_PARSE_NONE};
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\x00ff\x00feL ");
#else
  /* In UTF-8, y-diaeresis is 0xc3 0xbf, lowercase thorn is 0xc3 0xbe */
  const XML_Char *expected = XCS("\xc3\xbf\xc3\xbeL ");
#endif
  CharData storage;

  CharData_Init(&storage);
  test_data.storage = &storage;
  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader2);
  XML_SetUserData(g_parser, &test_data);
  XML_SetCharacterDataHandler(g_parser, ext2_accumulate_characters);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

START_TEST(test_ext_entity_latin1_utf16be_bom) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/dummy.ent'>\n"
                     "]>\n"
                     "<doc>&en;</doc>";
  ExtTest2 test_data
      = {/* If UTF-16, 0xfeff is the BOM and 0x204c is black left bullet */
         /* If Latin-1, 0xff = Y-diaeresis, 0xfe = lowercase thorn,
          *   0x4c = L and 0x20 is a space
          */
         "\xfe\xff\x20\x4c", 4, XCS("iso-8859-1"), NULL, EE_PARSE_NONE};
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\x00fe\x00ff L");
#else
  /* In UTF-8, y-diaeresis is 0xc3 0xbf, lowercase thorn is 0xc3 0xbe */
  const XML_Char *expected = XCS("\xc3\xbe\xc3\xbf L");
#endif
  CharData storage;

  CharData_Init(&storage);
  test_data.storage = &storage;
  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader2);
  XML_SetUserData(g_parser, &test_data);
  XML_SetCharacterDataHandler(g_parser, ext2_accumulate_characters);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

/* Parsing the full buffer rather than a byte at a time makes a
 * difference to the encoding scanning code, so repeat the above tests
 * without breaking them down by byte.
 */
START_TEST(test_ext_entity_latin1_utf16le_bom2) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/dummy.ent'>\n"
                     "]>\n"
                     "<doc>&en;</doc>";
  ExtTest2 test_data
      = {/* If UTF-16, 0xfeff is the BOM and 0x204c is black left bullet */
         /* If Latin-1, 0xff = Y-diaeresis, 0xfe = lowercase thorn,
          *   0x4c = L and 0x20 is a space
          */
         "\xff\xfe\x4c\x20", 4, XCS("iso-8859-1"), NULL, EE_PARSE_FULL_BUFFER};
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\x00ff\x00feL ");
#else
  /* In UTF-8, y-diaeresis is 0xc3 0xbf, lowercase thorn is 0xc3 0xbe */
  const XML_Char *expected = XCS("\xc3\xbf\xc3\xbeL ");
#endif
  CharData storage;

  CharData_Init(&storage);
  test_data.storage = &storage;
  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader2);
  XML_SetUserData(g_parser, &test_data);
  XML_SetCharacterDataHandler(g_parser, ext2_accumulate_characters);
  if (XML_Parse(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

START_TEST(test_ext_entity_latin1_utf16be_bom2) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/dummy.ent'>\n"
                     "]>\n"
                     "<doc>&en;</doc>";
  ExtTest2 test_data
      = {/* If UTF-16, 0xfeff is the BOM and 0x204c is black left bullet */
         /* If Latin-1, 0xff = Y-diaeresis, 0xfe = lowercase thorn,
          *   0x4c = L and 0x20 is a space
          */
         "\xfe\xff\x20\x4c", 4, XCS("iso-8859-1"), NULL, EE_PARSE_FULL_BUFFER};
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\x00fe\x00ff L");
#else
  /* In UTF-8, y-diaeresis is 0xc3 0xbf, lowercase thorn is 0xc3 0xbe */
  const XML_Char *expected = "\xc3\xbe\xc3\xbf L";
#endif
  CharData storage;

  CharData_Init(&storage);
  test_data.storage = &storage;
  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader2);
  XML_SetUserData(g_parser, &test_data);
  XML_SetCharacterDataHandler(g_parser, ext2_accumulate_characters);
  if (XML_Parse(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

/* Test little-endian UTF-16 given an explicit big-endian encoding */
START_TEST(test_ext_entity_utf16_be) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/dummy.ent'>\n"
                     "]>\n"
                     "<doc>&en;</doc>";
  ExtTest2 test_data
      = {"<\0e\0/\0>\0", 8, XCS("utf-16be"), NULL, EE_PARSE_NONE};
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\x3c00\x6500\x2f00\x3e00");
#else
  const XML_Char *expected = XCS("\xe3\xb0\x80"   /* U+3C00 */
                                 "\xe6\x94\x80"   /* U+6500 */
                                 "\xe2\xbc\x80"   /* U+2F00 */
                                 "\xe3\xb8\x80"); /* U+3E00 */
#endif
  CharData storage;

  CharData_Init(&storage);
  test_data.storage = &storage;
  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader2);
  XML_SetUserData(g_parser, &test_data);
  XML_SetCharacterDataHandler(g_parser, ext2_accumulate_characters);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

/* Test big-endian UTF-16 given an explicit little-endian encoding */
START_TEST(test_ext_entity_utf16_le) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/dummy.ent'>\n"
                     "]>\n"
                     "<doc>&en;</doc>";
  ExtTest2 test_data
      = {"\0<\0e\0/\0>", 8, XCS("utf-16le"), NULL, EE_PARSE_NONE};
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\x3c00\x6500\x2f00\x3e00");
#else
  const XML_Char *expected = XCS("\xe3\xb0\x80"   /* U+3C00 */
                                 "\xe6\x94\x80"   /* U+6500 */
                                 "\xe2\xbc\x80"   /* U+2F00 */
                                 "\xe3\xb8\x80"); /* U+3E00 */
#endif
  CharData storage;

  CharData_Init(&storage);
  test_data.storage = &storage;
  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader2);
  XML_SetUserData(g_parser, &test_data);
  XML_SetCharacterDataHandler(g_parser, ext2_accumulate_characters);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

/* Test little-endian UTF-16 given no explicit encoding.
 * The existing default encoding (UTF-8) is assumed to hold without a
 * BOM to contradict it, so the entity value will in fact provoke an
 * error because 0x00 is not a valid XML character.  We parse the
 * whole buffer in one go rather than feeding it in byte by byte to
 * exercise different code paths in the initial scanning routines.
 */
typedef struct ExtFaults2 {
  const char *parse_text;
  int parse_len;
  const char *fail_text;
  const XML_Char *encoding;
  enum XML_Error error;
} ExtFaults2;

static int XMLCALL
external_entity_faulter2(XML_Parser parser, const XML_Char *context,
                         const XML_Char *base, const XML_Char *systemId,
                         const XML_Char *publicId) {
  ExtFaults2 *test_data = (ExtFaults2 *)XML_GetUserData(parser);
  XML_Parser extparser;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  extparser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (extparser == NULL)
    fail("Could not create external entity parser");
  if (test_data->encoding != NULL) {
    if (! XML_SetEncoding(extparser, test_data->encoding))
      fail("XML_SetEncoding() ignored for external entity");
  }
  if (XML_Parse(extparser, test_data->parse_text, test_data->parse_len,
                XML_TRUE)
      != XML_STATUS_ERROR)
    fail(test_data->fail_text);
  if (XML_GetErrorCode(extparser) != test_data->error)
    xml_failure(extparser);

  XML_ParserFree(extparser);
  return XML_STATUS_ERROR;
}

START_TEST(test_ext_entity_utf16_unknown) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/dummy.ent'>\n"
                     "]>\n"
                     "<doc>&en;</doc>";
  ExtFaults2 test_data
      = {"a\0b\0c\0", 6, "Invalid character in entity not faulted", NULL,
         XML_ERROR_INVALID_TOKEN};

  XML_SetExternalEntityRefHandler(g_parser, external_entity_faulter2);
  XML_SetUserData(g_parser, &test_data);
  expect_failure(text, XML_ERROR_EXTERNAL_ENTITY_HANDLING,
                 "Invalid character should not have been accepted");
}
END_TEST

/* Test not-quite-UTF-8 BOM (0xEF 0xBB 0xBF) */
START_TEST(test_ext_entity_utf8_non_bom) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/dummy.ent'>\n"
                     "]>\n"
                     "<doc>&en;</doc>";
  ExtTest2 test_data
      = {"\xef\xbb\x80", /* Arabic letter DAD medial form, U+FEC0 */
         3, NULL, NULL, EE_PARSE_NONE};
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\xfec0");
#else
  const XML_Char *expected = XCS("\xef\xbb\x80");
#endif
  CharData storage;

  CharData_Init(&storage);
  test_data.storage = &storage;
  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader2);
  XML_SetUserData(g_parser, &test_data);
  XML_SetCharacterDataHandler(g_parser, ext2_accumulate_characters);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

/* Test that UTF-8 in a CDATA section is correctly passed through */
START_TEST(test_utf8_in_cdata_section) {
  const char *text = "<doc><![CDATA[one \xc3\xa9 two]]></doc>";
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("one \x00e9 two");
#else
  const XML_Char *expected = XCS("one \xc3\xa9 two");
#endif

  run_character_check(text, expected);
}
END_TEST

/* Test that little-endian UTF-16 in a CDATA section is handled */
START_TEST(test_utf8_in_cdata_section_2) {
  const char *text = "<doc><![CDATA[\xc3\xa9]\xc3\xa9two]]></doc>";
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\x00e9]\x00e9two");
#else
  const XML_Char *expected = XCS("\xc3\xa9]\xc3\xa9two");
#endif

  run_character_check(text, expected);
}
END_TEST

START_TEST(test_utf8_in_start_tags) {
  struct test_case {
    bool goodName;
    bool goodNameStart;
    const char *tagName;
  };

  // The idea with the tests below is this:
  // We want to cover 1-, 2- and 3-byte sequences, 4-byte sequences
  // go to isNever and are hence not a concern.
  //
  // We start with a character that is a valid name character
  // (or even name-start character, see XML 1.0r4 spec) and then we flip
  // single bits at places where (1) the result leaves the UTF-8 encoding space
  // and (2) we stay in the same n-byte sequence family.
  //
  // The flipped bits are highlighted in angle brackets in comments,
  // e.g. "[<1>011 1001]" means we had [0011 1001] but we now flipped
  // the most significant bit to 1 to leave UTF-8 encoding space.
  struct test_case cases[] = {
      // 1-byte UTF-8: [0xxx xxxx]
      {true, true, "\x3A"},   // [0011 1010] = ASCII colon ':'
      {false, false, "\xBA"}, // [<1>011 1010]
      {true, false, "\x39"},  // [0011 1001] = ASCII nine '9'
      {false, false, "\xB9"}, // [<1>011 1001]

      // 2-byte UTF-8: [110x xxxx] [10xx xxxx]
      {true, true, "\xDB\xA5"},   // [1101 1011] [1010 0101] =
                                  // Arabic small waw U+06E5
      {false, false, "\x9B\xA5"}, // [1<0>01 1011] [1010 0101]
      {false, false, "\xDB\x25"}, // [1101 1011] [<0>010 0101]
      {false, false, "\xDB\xE5"}, // [1101 1011] [1<1>10 0101]
      {true, false, "\xCC\x81"},  // [1100 1100] [1000 0001] =
                                  // combining char U+0301
      {false, false, "\x8C\x81"}, // [1<0>00 1100] [1000 0001]
      {false, false, "\xCC\x01"}, // [1100 1100] [<0>000 0001]
      {false, false, "\xCC\xC1"}, // [1100 1100] [1<1>00 0001]

      // 3-byte UTF-8: [1110 xxxx] [10xx xxxx] [10xxxxxx]
      {true, true, "\xE0\xA4\x85"},   // [1110 0000] [1010 0100] [1000 0101] =
                                      // Devanagari Letter A U+0905
      {false, false, "\xA0\xA4\x85"}, // [1<0>10 0000] [1010 0100] [1000 0101]
      {false, false, "\xE0\x24\x85"}, // [1110 0000] [<0>010 0100] [1000 0101]
      {false, false, "\xE0\xE4\x85"}, // [1110 0000] [1<1>10 0100] [1000 0101]
      {false, false, "\xE0\xA4\x05"}, // [1110 0000] [1010 0100] [<0>000 0101]
      {false, false, "\xE0\xA4\xC5"}, // [1110 0000] [1010 0100] [1<1>00 0101]
      {true, false, "\xE0\xA4\x81"},  // [1110 0000] [1010 0100] [1000 0001] =
                                      // combining char U+0901
      {false, false, "\xA0\xA4\x81"}, // [1<0>10 0000] [1010 0100] [1000 0001]
      {false, false, "\xE0\x24\x81"}, // [1110 0000] [<0>010 0100] [1000 0001]
      {false, false, "\xE0\xE4\x81"}, // [1110 0000] [1<1>10 0100] [1000 0001]
      {false, false, "\xE0\xA4\x01"}, // [1110 0000] [1010 0100] [<0>000 0001]
      {false, false, "\xE0\xA4\xC1"}, // [1110 0000] [1010 0100] [1<1>00 0001]
  };
  const bool atNameStart[] = {true, false};

  size_t i = 0;
  char doc[1024];
  size_t failCount = 0;

  for (; i < sizeof(cases) / sizeof(cases[0]); i++) {
    size_t j = 0;
    for (; j < sizeof(atNameStart) / sizeof(atNameStart[0]); j++) {
      const bool expectedSuccess
          = atNameStart[j] ? cases[i].goodNameStart : cases[i].goodName;
      sprintf(doc, "<%s%s><!--", atNameStart[j] ? "" : "a", cases[i].tagName);
      XML_Parser parser = XML_ParserCreate(NULL);

      const enum XML_Status status
          = XML_Parse(parser, doc, (int)strlen(doc), /*isFinal=*/XML_FALSE);

      bool success = true;
      if ((status == XML_STATUS_OK) != expectedSuccess) {
        success = false;
      }
      if ((status == XML_STATUS_ERROR)
          && (XML_GetErrorCode(parser) != XML_ERROR_INVALID_TOKEN)) {
        success = false;
      }

      if (! success) {
        fprintf(
            stderr,
            "FAIL case %2u (%sat name start, %u-byte sequence, error code %d)\n",
            (unsigned)i + 1u, atNameStart[j] ? "    " : "not ",
            (unsigned)strlen(cases[i].tagName), XML_GetErrorCode(parser));
        failCount++;
      }

      XML_ParserFree(parser);
    }
  }

  if (failCount > 0) {
    fail("UTF-8 regression detected");
  }
}
END_TEST

/* Test trailing spaces in elements are accepted */
static void XMLCALL
record_element_end_handler(void *userData, const XML_Char *name) {
  CharData *storage = (CharData *)userData;

  CharData_AppendXMLChars(storage, XCS("/"), 1);
  CharData_AppendXMLChars(storage, name, -1);
}

START_TEST(test_trailing_spaces_in_elements) {
  const char *text = "<doc   >Hi</doc >";
  const XML_Char *expected = XCS("doc/doc");
  CharData storage;

  CharData_Init(&storage);
  XML_SetElementHandler(g_parser, record_element_start_handler,
                        record_element_end_handler);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

START_TEST(test_utf16_attribute) {
  const char text[] =
      /* <d {KHO KHWAI}{CHO CHAN}='a'/>
       * where {KHO KHWAI} = U+0E04 = 0xe0 0xb8 0x84 in UTF-8
       * and   {CHO CHAN}  = U+0E08 = 0xe0 0xb8 0x88 in UTF-8
       */
      "<\0d\0 \0\x04\x0e\x08\x0e=\0'\0a\0'\0/\0>\0";
  const XML_Char *expected = XCS("a");
  CharData storage;

  CharData_Init(&storage);
  XML_SetStartElementHandler(g_parser, accumulate_attribute);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

START_TEST(test_utf16_second_attr) {
  /* <d a='1' {KHO KHWAI}{CHO CHAN}='2'/>
   * where {KHO KHWAI} = U+0E04 = 0xe0 0xb8 0x84 in UTF-8
   * and   {CHO CHAN}  = U+0E08 = 0xe0 0xb8 0x88 in UTF-8
   */
  const char text[] = "<\0d\0 \0a\0=\0'\0\x31\0'\0 \0"
                      "\x04\x0e\x08\x0e=\0'\0\x32\0'\0/\0>\0";
  const XML_Char *expected = XCS("1");
  CharData storage;

  CharData_Init(&storage);
  XML_SetStartElementHandler(g_parser, accumulate_attribute);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

START_TEST(test_attr_after_solidus) {
  const char *text = "<doc attr1='a' / attr2='b'>";

  expect_failure(text, XML_ERROR_INVALID_TOKEN, "Misplaced / not faulted");
}
END_TEST

static void XMLCALL
accumulate_entity_decl(void *userData, const XML_Char *entityName,
                       int is_parameter_entity, const XML_Char *value,
                       int value_length, const XML_Char *base,
                       const XML_Char *systemId, const XML_Char *publicId,
                       const XML_Char *notationName) {
  CharData *storage = (CharData *)userData;

  UNUSED_P(is_parameter_entity);
  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  UNUSED_P(notationName);
  CharData_AppendXMLChars(storage, entityName, -1);
  CharData_AppendXMLChars(storage, XCS("="), 1);
  CharData_AppendXMLChars(storage, value, value_length);
  CharData_AppendXMLChars(storage, XCS("\n"), 1);
}

START_TEST(test_utf16_pe) {
  /* <!DOCTYPE doc [
   * <!ENTITY % {KHO KHWAI}{CHO CHAN} '<!ELEMENT doc (#PCDATA)>'>
   * %{KHO KHWAI}{CHO CHAN};
   * ]>
   * <doc></doc>
   *
   * where {KHO KHWAI} = U+0E04 = 0xe0 0xb8 0x84 in UTF-8
   * and   {CHO CHAN}  = U+0E08 = 0xe0 0xb8 0x88 in UTF-8
   */
  const char text[] = "\0<\0!\0D\0O\0C\0T\0Y\0P\0E\0 \0d\0o\0c\0 \0[\0\n"
                      "\0<\0!\0E\0N\0T\0I\0T\0Y\0 \0%\0 \x0e\x04\x0e\x08\0 "
                      "\0'\0<\0!\0E\0L\0E\0M\0E\0N\0T\0 "
                      "\0d\0o\0c\0 \0(\0#\0P\0C\0D\0A\0T\0A\0)\0>\0'\0>\0\n"
                      "\0%\x0e\x04\x0e\x08\0;\0\n"
                      "\0]\0>\0\n"
                      "\0<\0d\0o\0c\0>\0<\0/\0d\0o\0c\0>";
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\x0e04\x0e08=<!ELEMENT doc (#PCDATA)>\n");
#else
  const XML_Char *expected
      = XCS("\xe0\xb8\x84\xe0\xb8\x88=<!ELEMENT doc (#PCDATA)>\n");
#endif
  CharData storage;

  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  XML_SetEntityDeclHandler(g_parser, accumulate_entity_decl);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

/* Test that duff attribute description keywords are rejected */
START_TEST(test_bad_attr_desc_keyword) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ATTLIST doc attr CDATA #!IMPLIED>\n"
                     "]>\n"
                     "<doc />";

  expect_failure(text, XML_ERROR_INVALID_TOKEN,
                 "Bad keyword !IMPLIED not faulted");
}
END_TEST

/* Test that an invalid attribute description keyword consisting of
 * UTF-16 characters with their top bytes non-zero are correctly
 * faulted
 */
START_TEST(test_bad_attr_desc_keyword_utf16) {
  /* <!DOCTYPE d [
   * <!ATTLIST d a CDATA #{KHO KHWAI}{CHO CHAN}>
   * ]><d/>
   *
   * where {KHO KHWAI} = U+0E04 = 0xe0 0xb8 0x84 in UTF-8
   * and   {CHO CHAN}  = U+0E08 = 0xe0 0xb8 0x88 in UTF-8
   */
  const char text[]
      = "\0<\0!\0D\0O\0C\0T\0Y\0P\0E\0 \0d\0 \0[\0\n"
        "\0<\0!\0A\0T\0T\0L\0I\0S\0T\0 \0d\0 \0a\0 \0C\0D\0A\0T\0A\0 "
        "\0#\x0e\x04\x0e\x08\0>\0\n"
        "\0]\0>\0<\0d\0/\0>";

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      != XML_STATUS_ERROR)
    fail("Invalid UTF16 attribute keyword not faulted");
  if (XML_GetErrorCode(g_parser) != XML_ERROR_SYNTAX)
    xml_failure(g_parser);
}
END_TEST

/* Test that invalid syntax in a <!DOCTYPE> is rejected.  Do this
 * using prefix-encoding (see above) to trigger specific code paths
 */
START_TEST(test_bad_doctype) {
  const char *text = "<?xml version='1.0' encoding='prefix-conv'?>\n"
                     "<!DOCTYPE doc [ \x80\x44 ]><doc/>";

  XML_SetUnknownEncodingHandler(g_parser, MiscEncodingHandler, NULL);
  expect_failure(text, XML_ERROR_SYNTAX,
                 "Invalid bytes in DOCTYPE not faulted");
}
END_TEST

START_TEST(test_bad_doctype_utf8) {
  const char *text = "<!DOCTYPE \xDB\x25"
                     "doc><doc/>"; // [1101 1011] [<0>010 0101]
  expect_failure(text, XML_ERROR_INVALID_TOKEN,
                 "Invalid UTF-8 in DOCTYPE not faulted");
}
END_TEST

START_TEST(test_bad_doctype_utf16) {
  const char text[] =
      /* <!DOCTYPE doc [ \x06f2 ]><doc/>
       *
       * U+06F2 = EXTENDED ARABIC-INDIC DIGIT TWO, a valid number
       * (name character) but not a valid letter (name start character)
       */
      "\0<\0!\0D\0O\0C\0T\0Y\0P\0E\0 \0d\0o\0c\0 \0[\0 "
      "\x06\xf2"
      "\0 \0]\0>\0<\0d\0o\0c\0/\0>";

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      != XML_STATUS_ERROR)
    fail("Invalid bytes in DOCTYPE not faulted");
  if (XML_GetErrorCode(g_parser) != XML_ERROR_SYNTAX)
    xml_failure(g_parser);
}
END_TEST

START_TEST(test_bad_doctype_plus) {
  const char *text = "<!DOCTYPE 1+ [ <!ENTITY foo 'bar'> ]>\n"
                     "<1+>&foo;</1+>";

  expect_failure(text, XML_ERROR_INVALID_TOKEN,
                 "'+' in document name not faulted");
}
END_TEST

START_TEST(test_bad_doctype_star) {
  const char *text = "<!DOCTYPE 1* [ <!ENTITY foo 'bar'> ]>\n"
                     "<1*>&foo;</1*>";

  expect_failure(text, XML_ERROR_INVALID_TOKEN,
                 "'*' in document name not faulted");
}
END_TEST

START_TEST(test_bad_doctype_query) {
  const char *text = "<!DOCTYPE 1? [ <!ENTITY foo 'bar'> ]>\n"
                     "<1?>&foo;</1?>";

  expect_failure(text, XML_ERROR_INVALID_TOKEN,
                 "'?' in document name not faulted");
}
END_TEST

START_TEST(test_unknown_encoding_bad_ignore) {
  const char *text = "<?xml version='1.0' encoding='prefix-conv'?>"
                     "<!DOCTYPE doc SYSTEM 'foo'>"
                     "<doc><e>&entity;</e></doc>";
  ExtFaults fault = {"<![IGNORE[<!ELEMENT \xffG (#PCDATA)*>]]>",
                     "Invalid character not faulted", XCS("prefix-conv"),
                     XML_ERROR_INVALID_TOKEN};

  XML_SetUnknownEncodingHandler(g_parser, MiscEncodingHandler, NULL);
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_faulter);
  XML_SetUserData(g_parser, &fault);
  expect_failure(text, XML_ERROR_EXTERNAL_ENTITY_HANDLING,
                 "Bad IGNORE section with unknown encoding not failed");
}
END_TEST

START_TEST(test_entity_in_utf16_be_attr) {
  const char text[] =
      /* <e a='&#228; &#x00E4;'></e> */
      "\0<\0e\0 \0a\0=\0'\0&\0#\0\x32\0\x32\0\x38\0;\0 "
      "\0&\0#\0x\0\x30\0\x30\0E\0\x34\0;\0'\0>\0<\0/\0e\0>";
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\x00e4 \x00e4");
#else
  const XML_Char *expected = XCS("\xc3\xa4 \xc3\xa4");
#endif
  CharData storage;

  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  XML_SetStartElementHandler(g_parser, accumulate_attribute);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

START_TEST(test_entity_in_utf16_le_attr) {
  const char text[] =
      /* <e a='&#228; &#x00E4;'></e> */
      "<\0e\0 \0a\0=\0'\0&\0#\0\x32\0\x32\0\x38\0;\0 \0"
      "&\0#\0x\0\x30\0\x30\0E\0\x34\0;\0'\0>\0<\0/\0e\0>\0";
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("\x00e4 \x00e4");
#else
  const XML_Char *expected = XCS("\xc3\xa4 \xc3\xa4");
#endif
  CharData storage;

  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  XML_SetStartElementHandler(g_parser, accumulate_attribute);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

START_TEST(test_entity_public_utf16_be) {
  const char text[] =
      /* <!DOCTYPE d [ */
      "\0<\0!\0D\0O\0C\0T\0Y\0P\0E\0 \0d\0 \0[\0\n"
      /* <!ENTITY % e PUBLIC 'foo' 'bar.ent'> */
      "\0<\0!\0E\0N\0T\0I\0T\0Y\0 \0%\0 \0e\0 \0P\0U\0B\0L\0I\0C\0 "
      "\0'\0f\0o\0o\0'\0 \0'\0b\0a\0r\0.\0e\0n\0t\0'\0>\0\n"
      /* %e; */
      "\0%\0e\0;\0\n"
      /* ]> */
      "\0]\0>\0\n"
      /* <d>&j;</d> */
      "\0<\0d\0>\0&\0j\0;\0<\0/\0d\0>";
  ExtTest2 test_data = {/* <!ENTITY j 'baz'> */
                        "\0<\0!\0E\0N\0T\0I\0T\0Y\0 \0j\0 \0'\0b\0a\0z\0'\0>",
                        34, NULL, NULL, EE_PARSE_NONE};
  const XML_Char *expected = XCS("baz");
  CharData storage;

  CharData_Init(&storage);
  test_data.storage = &storage;
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader2);
  XML_SetUserData(g_parser, &test_data);
  XML_SetCharacterDataHandler(g_parser, ext2_accumulate_characters);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

START_TEST(test_entity_public_utf16_le) {
  const char text[] =
      /* <!DOCTYPE d [ */
      "<\0!\0D\0O\0C\0T\0Y\0P\0E\0 \0d\0 \0[\0\n\0"
      /* <!ENTITY % e PUBLIC 'foo' 'bar.ent'> */
      "<\0!\0E\0N\0T\0I\0T\0Y\0 \0%\0 \0e\0 \0P\0U\0B\0L\0I\0C\0 \0"
      "'\0f\0o\0o\0'\0 \0'\0b\0a\0r\0.\0e\0n\0t\0'\0>\0\n\0"
      /* %e; */
      "%\0e\0;\0\n\0"
      /* ]> */
      "]\0>\0\n\0"
      /* <d>&j;</d> */
      "<\0d\0>\0&\0j\0;\0<\0/\0d\0>\0";
  ExtTest2 test_data = {/* <!ENTITY j 'baz'> */
                        "<\0!\0E\0N\0T\0I\0T\0Y\0 \0j\0 \0'\0b\0a\0z\0'\0>\0",
                        34, NULL, NULL, EE_PARSE_NONE};
  const XML_Char *expected = XCS("baz");
  CharData storage;

  CharData_Init(&storage);
  test_data.storage = &storage;
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_loader2);
  XML_SetUserData(g_parser, &test_data);
  XML_SetCharacterDataHandler(g_parser, ext2_accumulate_characters);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

/* Test that a doctype with neither an internal nor external subset is
 * faulted
 */
START_TEST(test_short_doctype) {
  const char *text = "<!DOCTYPE doc></doc>";
  expect_failure(text, XML_ERROR_INVALID_TOKEN,
                 "DOCTYPE without subset not rejected");
}
END_TEST

START_TEST(test_short_doctype_2) {
  const char *text = "<!DOCTYPE doc PUBLIC></doc>";
  expect_failure(text, XML_ERROR_SYNTAX,
                 "DOCTYPE without Public ID not rejected");
}
END_TEST

START_TEST(test_short_doctype_3) {
  const char *text = "<!DOCTYPE doc SYSTEM></doc>";
  expect_failure(text, XML_ERROR_SYNTAX,
                 "DOCTYPE without System ID not rejected");
}
END_TEST

START_TEST(test_long_doctype) {
  const char *text = "<!DOCTYPE doc PUBLIC 'foo' 'bar' 'baz'></doc>";
  expect_failure(text, XML_ERROR_SYNTAX, "DOCTYPE with extra ID not rejected");
}
END_TEST

START_TEST(test_bad_entity) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY foo PUBLIC>\n"
                     "]>\n"
                     "<doc/>";
  expect_failure(text, XML_ERROR_SYNTAX,
                 "ENTITY without Public ID is not rejected");
}
END_TEST

/* Test unquoted value is faulted */
START_TEST(test_bad_entity_2) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY % foo bar>\n"
                     "]>\n"
                     "<doc/>";
  expect_failure(text, XML_ERROR_SYNTAX,
                 "ENTITY without Public ID is not rejected");
}
END_TEST

START_TEST(test_bad_entity_3) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY % foo PUBLIC>\n"
                     "]>\n"
                     "<doc/>";
  expect_failure(text, XML_ERROR_SYNTAX,
                 "Parameter ENTITY without Public ID is not rejected");
}
END_TEST

START_TEST(test_bad_entity_4) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY % foo SYSTEM>\n"
                     "]>\n"
                     "<doc/>";
  expect_failure(text, XML_ERROR_SYNTAX,
                 "Parameter ENTITY without Public ID is not rejected");
}
END_TEST

START_TEST(test_bad_notation) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!NOTATION n SYSTEM>\n"
                     "]>\n"
                     "<doc/>";
  expect_failure(text, XML_ERROR_SYNTAX,
                 "Notation without System ID is not rejected");
}
END_TEST

/* Test for issue #11, wrongly suppressed default handler */
typedef struct default_check {
  const XML_Char *expected;
  const int expectedLen;
  XML_Bool seen;
} DefaultCheck;

static void XMLCALL
checking_default_handler(void *userData, const XML_Char *s, int len) {
  DefaultCheck *data = (DefaultCheck *)userData;
  int i;

  for (i = 0; data[i].expected != NULL; i++) {
    if (data[i].expectedLen == len
        && ! memcmp(data[i].expected, s, len * sizeof(XML_Char))) {
      data[i].seen = XML_TRUE;
      break;
    }
  }
}

START_TEST(test_default_doctype_handler) {
  const char *text = "<!DOCTYPE doc PUBLIC 'pubname' 'test.dtd' [\n"
                     "  <!ENTITY foo 'bar'>\n"
                     "]>\n"
                     "<doc>&foo;</doc>";
  DefaultCheck test_data[] = {{XCS("'pubname'"), 9, XML_FALSE},
                              {XCS("'test.dtd'"), 10, XML_FALSE},
                              {NULL, 0, XML_FALSE}};
  int i;

  XML_SetUserData(g_parser, &test_data);
  XML_SetDefaultHandler(g_parser, checking_default_handler);
  XML_SetEntityDeclHandler(g_parser, dummy_entity_decl_handler);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  for (i = 0; test_data[i].expected != NULL; i++)
    if (! test_data[i].seen)
      fail("Default handler not run for public !DOCTYPE");
}
END_TEST

START_TEST(test_empty_element_abort) {
  const char *text = "<abort/>";

  XML_SetStartElementHandler(g_parser, start_element_suspender);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_ERROR)
    fail("Expected to error on abort");
}
END_TEST

/* Regression test for GH issue #612: unfinished m_declAttributeType
 * allocation in ->m_tempPool can corrupt following allocation.
 */
static int XMLCALL
external_entity_unfinished_attlist(XML_Parser parser, const XML_Char *context,
                                   const XML_Char *base,
                                   const XML_Char *systemId,
                                   const XML_Char *publicId) {
  const char *text = "<!ELEMENT barf ANY>\n"
                     "<!ATTLIST barf my_attr (blah|%blah;a|foo) #REQUIRED>\n"
                     "<!--COMMENT-->\n";
  XML_Parser ext_parser;

  UNUSED_P(base);
  UNUSED_P(publicId);
  if (systemId == NULL)
    return XML_STATUS_OK;

  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser");

  if (_XML_Parse_SINGLE_BYTES(ext_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(ext_parser);

  XML_ParserFree(ext_parser);
  return XML_STATUS_OK;
}

START_TEST(test_pool_integrity_with_unfinished_attr) {
  const char *text = "<?xml version='1.0' encoding='UTF-8'?>\n"
                     "<!DOCTYPE foo [\n"
                     "<!ELEMENT foo ANY>\n"
                     "<!ENTITY % entp SYSTEM \"external.dtd\">\n"
                     "%entp;\n"
                     "]>\n"
                     "<a></a>\n";
  const XML_Char *expected = XCS("COMMENT");
  CharData storage;

  CharData_Init(&storage);
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_unfinished_attlist);
  XML_SetAttlistDeclHandler(g_parser, dummy_attlist_decl_handler);
  XML_SetCommentHandler(g_parser, accumulate_comment);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

typedef struct {
  XML_Parser parser;
  CharData *storage;
} ParserPlusStorage;

static void XMLCALL
accumulate_and_suspend_comment_handler(void *userData, const XML_Char *data) {
  ParserPlusStorage *const parserPlusStorage = (ParserPlusStorage *)userData;
  accumulate_comment(parserPlusStorage->storage, data);
  XML_StopParser(parserPlusStorage->parser, XML_TRUE);
}

START_TEST(test_nested_entity_suspend) {
  const char *const text = "<!DOCTYPE a [\n"
                           "  <!ENTITY e1 '<!--e1-->'>\n"
                           "  <!ENTITY e2 '<!--e2 head-->&e1;<!--e2 tail-->'>\n"
                           "  <!ENTITY e3 '<!--e3 head-->&e2;<!--e3 tail-->'>\n"
                           "]>\n"
                           "<a><!--start-->&e3;<!--end--></a>";
  const XML_Char *const expected = XCS("start") XCS("e3 head") XCS("e2 head")
      XCS("e1") XCS("e2 tail") XCS("e3 tail") XCS("end");
  CharData storage;
  XML_Parser parser = XML_ParserCreate(NULL);
  ParserPlusStorage parserPlusStorage = {parser, &storage};

  CharData_Init(&storage);
  XML_SetParamEntityParsing(parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetCommentHandler(parser, accumulate_and_suspend_comment_handler);
  XML_SetUserData(parser, &parserPlusStorage);

  enum XML_Status status = XML_Parse(parser, text, (int)strlen(text), XML_TRUE);
  while (status == XML_STATUS_SUSPENDED) {
    status = XML_ResumeParser(parser);
  }
  if (status != XML_STATUS_OK)
    xml_failure(parser);

  CharData_CheckXMLChars(&storage, expected);
  XML_ParserFree(parser);
}
END_TEST

/*
 * Namespaces tests.
 */

static void
namespace_setup(void) {
  g_parser = XML_ParserCreateNS(NULL, XCS(' '));
  if (g_parser == NULL)
    fail("Parser not created.");
}

static void
namespace_teardown(void) {
  basic_teardown();
}

/* Check that an element name and attribute name match the expected values.
   The expected values are passed as an array reference of string pointers
   provided as the userData argument; the first is the expected
   element name, and the second is the expected attribute name.
*/
static int triplet_start_flag = XML_FALSE;
static int triplet_end_flag = XML_FALSE;

static void XMLCALL
triplet_start_checker(void *userData, const XML_Char *name,
                      const XML_Char **atts) {
  XML_Char **elemstr = (XML_Char **)userData;
  char buffer[1024];
  if (xcstrcmp(elemstr[0], name) != 0) {
    sprintf(buffer, "unexpected start string: '%" XML_FMT_STR "'", name);
    fail(buffer);
  }
  if (xcstrcmp(elemstr[1], atts[0]) != 0) {
    sprintf(buffer, "unexpected attribute string: '%" XML_FMT_STR "'", atts[0]);
    fail(buffer);
  }
  triplet_start_flag = XML_TRUE;
}

/* Check that the element name passed to the end-element handler matches
   the expected value.  The expected value is passed as the first element
   in an array of strings passed as the userData argument.
*/
static void XMLCALL
triplet_end_checker(void *userData, const XML_Char *name) {
  XML_Char **elemstr = (XML_Char **)userData;
  if (xcstrcmp(elemstr[0], name) != 0) {
    char buffer[1024];
    sprintf(buffer, "unexpected end string: '%" XML_FMT_STR "'", name);
    fail(buffer);
  }
  triplet_end_flag = XML_TRUE;
}

START_TEST(test_return_ns_triplet) {
  const char *text = "<foo:e xmlns:foo='http://example.org/' bar:a='12'\n"
                     "       xmlns:bar='http://example.org/'>";
  const char *epilog = "</foo:e>";
  const XML_Char *elemstr[]
      = {XCS("http://example.org/ e foo"), XCS("http://example.org/ a bar")};
  XML_SetReturnNSTriplet(g_parser, XML_TRUE);
  XML_SetUserData(g_parser, (void *)elemstr);
  XML_SetElementHandler(g_parser, triplet_start_checker, triplet_end_checker);
  XML_SetNamespaceDeclHandler(g_parser, dummy_start_namespace_decl_handler,
                              dummy_end_namespace_decl_handler);
  triplet_start_flag = XML_FALSE;
  triplet_end_flag = XML_FALSE;
  dummy_handler_flags = 0;
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_FALSE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  if (! triplet_start_flag)
    fail("triplet_start_checker not invoked");
  /* Check that unsetting "return triplets" fails while still parsing */
  XML_SetReturnNSTriplet(g_parser, XML_FALSE);
  if (_XML_Parse_SINGLE_BYTES(g_parser, epilog, (int)strlen(epilog), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  if (! triplet_end_flag)
    fail("triplet_end_checker not invoked");
  if (dummy_handler_flags
      != (DUMMY_START_NS_DECL_HANDLER_FLAG | DUMMY_END_NS_DECL_HANDLER_FLAG))
    fail("Namespace handlers not called");
}
END_TEST

static void XMLCALL
overwrite_start_checker(void *userData, const XML_Char *name,
                        const XML_Char **atts) {
  CharData *storage = (CharData *)userData;
  CharData_AppendXMLChars(storage, XCS("start "), 6);
  CharData_AppendXMLChars(storage, name, -1);
  while (*atts != NULL) {
    CharData_AppendXMLChars(storage, XCS("\nattribute "), 11);
    CharData_AppendXMLChars(storage, *atts, -1);
    atts += 2;
  }
  CharData_AppendXMLChars(storage, XCS("\n"), 1);
}

static void XMLCALL
overwrite_end_checker(void *userData, const XML_Char *name) {
  CharData *storage = (CharData *)userData;
  CharData_AppendXMLChars(storage, XCS("end "), 4);
  CharData_AppendXMLChars(storage, name, -1);
  CharData_AppendXMLChars(storage, XCS("\n"), 1);
}

static void
run_ns_tagname_overwrite_test(const char *text, const XML_Char *result) {
  CharData storage;
  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  XML_SetElementHandler(g_parser, overwrite_start_checker,
                        overwrite_end_checker);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, result);
}

/* Regression test for SF bug #566334. */
START_TEST(test_ns_tagname_overwrite) {
  const char *text = "<n:e xmlns:n='http://example.org/'>\n"
                     "  <n:f n:attr='foo'/>\n"
                     "  <n:g n:attr2='bar'/>\n"
                     "</n:e>";
  const XML_Char *result = XCS("start http://example.org/ e\n")
      XCS("start http://example.org/ f\n")
          XCS("attribute http://example.org/ attr\n")
              XCS("end http://example.org/ f\n")
                  XCS("start http://example.org/ g\n")
                      XCS("attribute http://example.org/ attr2\n")
                          XCS("end http://example.org/ g\n")
                              XCS("end http://example.org/ e\n");
  run_ns_tagname_overwrite_test(text, result);
}
END_TEST

/* Regression test for SF bug #566334. */
START_TEST(test_ns_tagname_overwrite_triplet) {
  const char *text = "<n:e xmlns:n='http://example.org/'>\n"
                     "  <n:f n:attr='foo'/>\n"
                     "  <n:g n:attr2='bar'/>\n"
                     "</n:e>";
  const XML_Char *result = XCS("start http://example.org/ e n\n")
      XCS("start http://example.org/ f n\n")
          XCS("attribute http://example.org/ attr n\n")
              XCS("end http://example.org/ f n\n")
                  XCS("start http://example.org/ g n\n")
                      XCS("attribute http://example.org/ attr2 n\n")
                          XCS("end http://example.org/ g n\n")
                              XCS("end http://example.org/ e n\n");
  XML_SetReturnNSTriplet(g_parser, XML_TRUE);
  run_ns_tagname_overwrite_test(text, result);
}
END_TEST

/* Regression test for SF bug #620343. */
static void XMLCALL
start_element_fail(void *userData, const XML_Char *name,
                   const XML_Char **atts) {
  UNUSED_P(userData);
  UNUSED_P(name);
  UNUSED_P(atts);

  /* We should never get here. */
  fail("should never reach start_element_fail()");
}

static void XMLCALL
start_ns_clearing_start_element(void *userData, const XML_Char *prefix,
                                const XML_Char *uri) {
  UNUSED_P(prefix);
  UNUSED_P(uri);
  XML_SetStartElementHandler((XML_Parser)userData, NULL);
}

START_TEST(test_start_ns_clears_start_element) {
  /* This needs to use separate start/end tags; using the empty tag
     syntax doesn't cause the problematic path through Expat to be
     taken.
  */
  const char *text = "<e xmlns='http://example.org/'></e>";

  XML_SetStartElementHandler(g_parser, start_element_fail);
  XML_SetStartNamespaceDeclHandler(g_parser, start_ns_clearing_start_element);
  XML_SetEndNamespaceDeclHandler(g_parser, dummy_end_namespace_decl_handler);
  XML_UseParserAsHandlerArg(g_parser);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Regression test for SF bug #616863. */
static int XMLCALL
external_entity_handler(XML_Parser parser, const XML_Char *context,
                        const XML_Char *base, const XML_Char *systemId,
                        const XML_Char *publicId) {
  intptr_t callno = 1 + (intptr_t)XML_GetUserData(parser);
  const char *text;
  XML_Parser p2;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  if (callno == 1)
    text = ("<!ELEMENT doc (e+)>\n"
            "<!ATTLIST doc xmlns CDATA #IMPLIED>\n"
            "<!ELEMENT e EMPTY>\n");
  else
    text = ("<?xml version='1.0' encoding='us-ascii'?>"
            "<e/>");

  XML_SetUserData(parser, (void *)callno);
  p2 = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (_XML_Parse_SINGLE_BYTES(p2, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR) {
    xml_failure(p2);
    return XML_STATUS_ERROR;
  }
  XML_ParserFree(p2);
  return XML_STATUS_OK;
}

START_TEST(test_default_ns_from_ext_subset_and_ext_ge) {
  const char *text = "<?xml version='1.0'?>\n"
                     "<!DOCTYPE doc SYSTEM 'http://example.org/doc.dtd' [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/entity.ent'>\n"
                     "]>\n"
                     "<doc xmlns='http://example.org/ns1'>\n"
                     "&en;\n"
                     "</doc>";

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_handler);
  /* We actually need to set this handler to tickle this bug. */
  XML_SetStartElementHandler(g_parser, dummy_start_element);
  XML_SetUserData(g_parser, NULL);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Regression test #1 for SF bug #673791. */
START_TEST(test_ns_prefix_with_empty_uri_1) {
  const char *text = "<doc xmlns:prefix='http://example.org/'>\n"
                     "  <e xmlns:prefix=''/>\n"
                     "</doc>";

  expect_failure(text, XML_ERROR_UNDECLARING_PREFIX,
                 "Did not report re-setting namespace"
                 " URI with prefix to ''.");
}
END_TEST

/* Regression test #2 for SF bug #673791. */
START_TEST(test_ns_prefix_with_empty_uri_2) {
  const char *text = "<?xml version='1.0'?>\n"
                     "<docelem xmlns:pre=''/>";

  expect_failure(text, XML_ERROR_UNDECLARING_PREFIX,
                 "Did not report setting namespace URI with prefix to ''.");
}
END_TEST

/* Regression test #3 for SF bug #673791. */
START_TEST(test_ns_prefix_with_empty_uri_3) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ELEMENT doc EMPTY>\n"
                     "  <!ATTLIST doc\n"
                     "    xmlns:prefix CDATA ''>\n"
                     "]>\n"
                     "<doc/>";

  expect_failure(text, XML_ERROR_UNDECLARING_PREFIX,
                 "Didn't report attr default setting NS w/ prefix to ''.");
}
END_TEST

/* Regression test #4 for SF bug #673791. */
START_TEST(test_ns_prefix_with_empty_uri_4) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ELEMENT prefix:doc EMPTY>\n"
                     "  <!ATTLIST prefix:doc\n"
                     "    xmlns:prefix CDATA 'http://example.org/'>\n"
                     "]>\n"
                     "<prefix:doc/>";
  /* Packaged info expected by the end element handler;
     the weird structuring lets us re-use the triplet_end_checker()
     function also used for another test. */
  const XML_Char *elemstr[] = {XCS("http://example.org/ doc prefix")};
  XML_SetReturnNSTriplet(g_parser, XML_TRUE);
  XML_SetUserData(g_parser, (void *)elemstr);
  XML_SetEndElementHandler(g_parser, triplet_end_checker);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Test with non-xmlns prefix */
START_TEST(test_ns_unbound_prefix) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ELEMENT prefix:doc EMPTY>\n"
                     "  <!ATTLIST prefix:doc\n"
                     "    notxmlns:prefix CDATA 'http://example.org/'>\n"
                     "]>\n"
                     "<prefix:doc/>";

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_ERROR)
    fail("Unbound prefix incorrectly passed");
  if (XML_GetErrorCode(g_parser) != XML_ERROR_UNBOUND_PREFIX)
    xml_failure(g_parser);
}
END_TEST

START_TEST(test_ns_default_with_empty_uri) {
  const char *text = "<doc xmlns='http://example.org/'>\n"
                     "  <e xmlns=''/>\n"
                     "</doc>";
  /* Add some handlers to exercise extra code paths */
  XML_SetStartNamespaceDeclHandler(g_parser,
                                   dummy_start_namespace_decl_handler);
  XML_SetEndNamespaceDeclHandler(g_parser, dummy_end_namespace_decl_handler);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Regression test for SF bug #692964: two prefixes for one namespace. */
START_TEST(test_ns_duplicate_attrs_diff_prefixes) {
  const char *text = "<doc xmlns:a='http://example.org/a'\n"
                     "     xmlns:b='http://example.org/a'\n"
                     "     a:a='v' b:a='v' />";
  expect_failure(text, XML_ERROR_DUPLICATE_ATTRIBUTE,
                 "did not report multiple attributes with same URI+name");
}
END_TEST

START_TEST(test_ns_duplicate_hashes) {
  /* The hash of an attribute is calculated as the hash of its URI
   * concatenated with a space followed by its name (after the
   * colon).  We wish to generate attributes with the same hash
   * value modulo the attribute table size so that we can check that
   * the attribute hash table works correctly.  The attribute hash
   * table size will be the smallest power of two greater than the
   * number of attributes, but at least eight.  There is
   * unfortunately no programmatic way of getting the hash or the
   * table size at user level, but the test code coverage percentage
   * will drop if the hashes cease to point to the same row.
   *
   * The cunning plan is to have few enough attributes to have a
   * reliable table size of 8, and have the single letter attribute
   * names be 8 characters apart, producing a hash which will be the
   * same modulo 8.
   */
  const char *text = "<doc xmlns:a='http://example.org/a'\n"
                     "     a:a='v' a:i='w' />";
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Regression test for SF bug #695401: unbound prefix. */
START_TEST(test_ns_unbound_prefix_on_attribute) {
  const char *text = "<doc a:attr=''/>";
  expect_failure(text, XML_ERROR_UNBOUND_PREFIX,
                 "did not report unbound prefix on attribute");
}
END_TEST

/* Regression test for SF bug #695401: unbound prefix. */
START_TEST(test_ns_unbound_prefix_on_element) {
  const char *text = "<a:doc/>";
  expect_failure(text, XML_ERROR_UNBOUND_PREFIX,
                 "did not report unbound prefix on element");
}
END_TEST

/* Test that the parsing status is correctly reset by XML_ParserReset().
 * We usE test_return_ns_triplet() for our example parse to improve
 * coverage of tidying up code executed.
 */
START_TEST(test_ns_parser_reset) {
  XML_ParsingStatus status;

  XML_GetParsingStatus(g_parser, &status);
  if (status.parsing != XML_INITIALIZED)
    fail("parsing status doesn't start INITIALIZED");
  test_return_ns_triplet();
  XML_GetParsingStatus(g_parser, &status);
  if (status.parsing != XML_FINISHED)
    fail("parsing status doesn't end FINISHED");
  XML_ParserReset(g_parser, NULL);
  XML_GetParsingStatus(g_parser, &status);
  if (status.parsing != XML_INITIALIZED)
    fail("parsing status doesn't reset to INITIALIZED");
}
END_TEST

/* Test that long element names with namespaces are handled correctly */
START_TEST(test_ns_long_element) {
  const char *text
      = "<foo:thisisalongenoughelementnametotriggerareallocation\n"
        " xmlns:foo='http://example.org/' bar:a='12'\n"
        " xmlns:bar='http://example.org/'>"
        "</foo:thisisalongenoughelementnametotriggerareallocation>";
  const XML_Char *elemstr[]
      = {XCS("http://example.org/")
             XCS(" thisisalongenoughelementnametotriggerareallocation foo"),
         XCS("http://example.org/ a bar")};

  XML_SetReturnNSTriplet(g_parser, XML_TRUE);
  XML_SetUserData(g_parser, (void *)elemstr);
  XML_SetElementHandler(g_parser, triplet_start_checker, triplet_end_checker);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Test mixed population of prefixed and unprefixed attributes */
START_TEST(test_ns_mixed_prefix_atts) {
  const char *text = "<e a='12' bar:b='13'\n"
                     " xmlns:bar='http://example.org/'>"
                     "</e>";

  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Test having a long namespaced element name inside a short one.
 * This exercises some internal buffer reallocation that is shared
 * across elements with the same namespace URI.
 */
START_TEST(test_ns_extend_uri_buffer) {
  const char *text = "<foo:e xmlns:foo='http://example.org/'>"
                     " <foo:thisisalongenoughnametotriggerallocationaction"
                     "   foo:a='12' />"
                     "</foo:e>";
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Test that xmlns is correctly rejected as an attribute in the xmlns
 * namespace, but not in other namespaces
 */
START_TEST(test_ns_reserved_attributes) {
  const char *text1
      = "<foo:e xmlns:foo='http://example.org/' xmlns:xmlns='12' />";
  const char *text2
      = "<foo:e xmlns:foo='http://example.org/' foo:xmlns='12' />";
  expect_failure(text1, XML_ERROR_RESERVED_PREFIX_XMLNS,
                 "xmlns not rejected as an attribute");
  XML_ParserReset(g_parser, NULL);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text2, (int)strlen(text2), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Test more reserved attributes */
START_TEST(test_ns_reserved_attributes_2) {
  const char *text1 = "<foo:e xmlns:foo='http://example.org/'"
                      "  xmlns:xml='http://example.org/' />";
  const char *text2
      = "<foo:e xmlns:foo='http://www.w3.org/XML/1998/namespace' />";
  const char *text3 = "<foo:e xmlns:foo='http://www.w3.org/2000/xmlns/' />";

  expect_failure(text1, XML_ERROR_RESERVED_PREFIX_XML,
                 "xml not rejected as an attribute");
  XML_ParserReset(g_parser, NULL);
  expect_failure(text2, XML_ERROR_RESERVED_NAMESPACE_URI,
                 "Use of w3.org URL not faulted");
  XML_ParserReset(g_parser, NULL);
  expect_failure(text3, XML_ERROR_RESERVED_NAMESPACE_URI,
                 "Use of w3.org xmlns URL not faulted");
}
END_TEST

/* Test string pool handling of namespace names of 2048 characters */
/* Exercises a particular string pool growth path */
START_TEST(test_ns_extremely_long_prefix) {
  /* C99 compilers are only required to support 4095-character
   * strings, so the following needs to be split in two to be safe
   * for all compilers.
   */
  const char *text1
      = "<doc "
        /* 64 character on each line */
        /* ...gives a total length of 2048 */
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        ":a='12'";
  const char *text2
      = " xmlns:"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "='foo'\n>"
        "</doc>";

  if (_XML_Parse_SINGLE_BYTES(g_parser, text1, (int)strlen(text1), XML_FALSE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text2, (int)strlen(text2), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

/* Test unknown encoding handlers in namespace setup */
START_TEST(test_ns_unknown_encoding_success) {
  const char *text = "<?xml version='1.0' encoding='prefix-conv'?>\n"
                     "<foo:e xmlns:foo='http://example.org/'>Hi</foo:e>";

  XML_SetUnknownEncodingHandler(g_parser, MiscEncodingHandler, NULL);
  run_character_check(text, XCS("Hi"));
}
END_TEST

/* Test that too many colons are rejected */
START_TEST(test_ns_double_colon) {
  const char *text = "<foo:e xmlns:foo='http://example.org/' foo:a:b='bar' />";
  const enum XML_Status status
      = _XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE);
#ifdef XML_NS
  if ((status == XML_STATUS_OK)
      || (XML_GetErrorCode(g_parser) != XML_ERROR_INVALID_TOKEN)) {
    fail("Double colon in attribute name not faulted"
         " (despite active namespace support)");
  }
#else
  if (status != XML_STATUS_OK) {
    fail("Double colon in attribute name faulted"
         " (despite inactive namespace support");
  }
#endif
}
END_TEST

START_TEST(test_ns_double_colon_element) {
  const char *text = "<foo:bar:e xmlns:foo='http://example.org/' />";
  const enum XML_Status status
      = _XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE);
#ifdef XML_NS
  if ((status == XML_STATUS_OK)
      || (XML_GetErrorCode(g_parser) != XML_ERROR_INVALID_TOKEN)) {
    fail("Double colon in element name not faulted"
         " (despite active namespace support)");
  }
#else
  if (status != XML_STATUS_OK) {
    fail("Double colon in element name faulted"
         " (despite inactive namespace support");
  }
#endif
}
END_TEST

/* Test that non-name characters after a colon are rejected */
START_TEST(test_ns_bad_attr_leafname) {
  const char *text = "<foo:e xmlns:foo='http://example.org/' foo:?ar='baz' />";

  expect_failure(text, XML_ERROR_INVALID_TOKEN,
                 "Invalid character in leafname not faulted");
}
END_TEST

START_TEST(test_ns_bad_element_leafname) {
  const char *text = "<foo:?oc xmlns:foo='http://example.org/' />";

  expect_failure(text, XML_ERROR_INVALID_TOKEN,
                 "Invalid character in element leafname not faulted");
}
END_TEST

/* Test high-byte-set UTF-16 characters are valid in a leafname */
START_TEST(test_ns_utf16_leafname) {
  const char text[] =
      /* <n:e xmlns:n='URI' n:{KHO KHWAI}='a' />
       * where {KHO KHWAI} = U+0E04 = 0xe0 0xb8 0x84 in UTF-8
       */
      "<\0n\0:\0e\0 \0x\0m\0l\0n\0s\0:\0n\0=\0'\0U\0R\0I\0'\0 \0"
      "n\0:\0\x04\x0e=\0'\0a\0'\0 \0/\0>\0";
  const XML_Char *expected = XCS("a");
  CharData storage;

  CharData_Init(&storage);
  XML_SetStartElementHandler(g_parser, accumulate_attribute);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

START_TEST(test_ns_utf16_element_leafname) {
  const char text[] =
      /* <n:{KHO KHWAI} xmlns:n='URI'/>
       * where {KHO KHWAI} = U+0E04 = 0xe0 0xb8 0x84 in UTF-8
       */
      "\0<\0n\0:\x0e\x04\0 \0x\0m\0l\0n\0s\0:\0n\0=\0'\0U\0R\0I\0'\0/\0>";
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("URI \x0e04");
#else
  const XML_Char *expected = XCS("URI \xe0\xb8\x84");
#endif
  CharData storage;

  CharData_Init(&storage);
  XML_SetStartElementHandler(g_parser, start_element_event_handler);
  XML_SetUserData(g_parser, &storage);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

START_TEST(test_ns_utf16_doctype) {
  const char text[] =
      /* <!DOCTYPE foo:{KHO KHWAI} [ <!ENTITY bar 'baz'> ]>\n
       * where {KHO KHWAI} = U+0E04 = 0xe0 0xb8 0x84 in UTF-8
       */
      "\0<\0!\0D\0O\0C\0T\0Y\0P\0E\0 \0f\0o\0o\0:\x0e\x04\0 "
      "\0[\0 \0<\0!\0E\0N\0T\0I\0T\0Y\0 \0b\0a\0r\0 \0'\0b\0a\0z\0'\0>\0 "
      "\0]\0>\0\n"
      /* <foo:{KHO KHWAI} xmlns:foo='URI'>&bar;</foo:{KHO KHWAI}> */
      "\0<\0f\0o\0o\0:\x0e\x04\0 "
      "\0x\0m\0l\0n\0s\0:\0f\0o\0o\0=\0'\0U\0R\0I\0'\0>"
      "\0&\0b\0a\0r\0;"
      "\0<\0/\0f\0o\0o\0:\x0e\x04\0>";
#ifdef XML_UNICODE
  const XML_Char *expected = XCS("URI \x0e04");
#else
  const XML_Char *expected = XCS("URI \xe0\xb8\x84");
#endif
  CharData storage;

  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  XML_SetStartElementHandler(g_parser, start_element_event_handler);
  XML_SetUnknownEncodingHandler(g_parser, MiscEncodingHandler, NULL);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

START_TEST(test_ns_invalid_doctype) {
  const char *text = "<!DOCTYPE foo:!bad [ <!ENTITY bar 'baz' ]>\n"
                     "<foo:!bad>&bar;</foo:!bad>";

  expect_failure(text, XML_ERROR_INVALID_TOKEN,
                 "Invalid character in document local name not faulted");
}
END_TEST

START_TEST(test_ns_double_colon_doctype) {
  const char *text = "<!DOCTYPE foo:a:doc [ <!ENTITY bar 'baz' ]>\n"
                     "<foo:a:doc>&bar;</foo:a:doc>";

  expect_failure(text, XML_ERROR_SYNTAX,
                 "Double colon in document name not faulted");
}
END_TEST

START_TEST(test_ns_separator_in_uri) {
  struct test_case {
    enum XML_Status expectedStatus;
    const char *doc;
    XML_Char namesep;
  };
  struct test_case cases[] = {
      {XML_STATUS_OK, "<doc xmlns='one_two' />", XCS('\n')},
      {XML_STATUS_ERROR, "<doc xmlns='one&#x0A;two' />", XCS('\n')},
      {XML_STATUS_OK, "<doc xmlns='one:two' />", XCS(':')},
  };

  size_t i = 0;
  size_t failCount = 0;
  for (; i < sizeof(cases) / sizeof(cases[0]); i++) {
    XML_Parser parser = XML_ParserCreateNS(NULL, cases[i].namesep);
    XML_SetElementHandler(parser, dummy_start_element, dummy_end_element);
    if (XML_Parse(parser, cases[i].doc, (int)strlen(cases[i].doc),
                  /*isFinal*/ XML_TRUE)
        != cases[i].expectedStatus) {
      failCount++;
    }
    XML_ParserFree(parser);
  }

  if (failCount) {
    fail("Namespace separator handling is broken");
  }
}
END_TEST

/* Control variable; the number of times duff_allocator() will successfully
 * allocate */
#define ALLOC_ALWAYS_SUCCEED (-1)
#define REALLOC_ALWAYS_SUCCEED (-1)

static intptr_t allocation_count = ALLOC_ALWAYS_SUCCEED;
static intptr_t reallocation_count = REALLOC_ALWAYS_SUCCEED;

/* Crocked allocator for allocation failure tests */
static void *
duff_allocator(size_t size) {
  if (allocation_count == 0)
    return NULL;
  if (allocation_count != ALLOC_ALWAYS_SUCCEED)
    allocation_count--;
  return malloc(size);
}

/* Crocked reallocator for allocation failure tests */
static void *
duff_reallocator(void *ptr, size_t size) {
  if (reallocation_count == 0)
    return NULL;
  if (reallocation_count != REALLOC_ALWAYS_SUCCEED)
    reallocation_count--;
  return realloc(ptr, size);
}

/* Test that a failure to allocate the parser structure fails gracefully */
START_TEST(test_misc_alloc_create_parser) {
  XML_Memory_Handling_Suite memsuite = {duff_allocator, realloc, free};
  unsigned int i;
  const unsigned int max_alloc_count = 10;

  /* Something this simple shouldn't need more than 10 allocations */
  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    g_parser = XML_ParserCreate_MM(NULL, &memsuite, NULL);
    if (g_parser != NULL)
      break;
  }
  if (i == 0)
    fail("Parser unexpectedly ignored failing allocator");
  else if (i == max_alloc_count)
    fail("Parser not created with max allocation count");
}
END_TEST

/* Test memory allocation failures for a parser with an encoding */
START_TEST(test_misc_alloc_create_parser_with_encoding) {
  XML_Memory_Handling_Suite memsuite = {duff_allocator, realloc, free};
  unsigned int i;
  const unsigned int max_alloc_count = 10;

  /* Try several levels of allocation */
  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    g_parser = XML_ParserCreate_MM(XCS("us-ascii"), &memsuite, NULL);
    if (g_parser != NULL)
      break;
  }
  if (i == 0)
    fail("Parser ignored failing allocator");
  else if (i == max_alloc_count)
    fail("Parser not created with max allocation count");
}
END_TEST

/* Test that freeing a NULL parser doesn't cause an explosion.
 * (Not actually tested anywhere else)
 */
START_TEST(test_misc_null_parser) {
  XML_ParserFree(NULL);
}
END_TEST

/* Test that XML_ErrorString rejects out-of-range codes */
START_TEST(test_misc_error_string) {
  if (XML_ErrorString((enum XML_Error) - 1) != NULL)
    fail("Negative error code not rejected");
  if (XML_ErrorString((enum XML_Error)100) != NULL)
    fail("Large error code not rejected");
}
END_TEST

/* Test the version information is consistent */

/* Since we are working in XML_LChars (potentially 16-bits), we
 * can't use the standard C library functions for character
 * manipulation and have to roll our own.
 */
static int
parse_version(const XML_LChar *version_text,
              XML_Expat_Version *version_struct) {
  if (! version_text)
    return XML_FALSE;

  while (*version_text != 0x00) {
    if (*version_text >= ASCII_0 && *version_text <= ASCII_9)
      break;
    version_text++;
  }
  if (*version_text == 0x00)
    return XML_FALSE;

  /* version_struct->major = strtoul(version_text, 10, &version_text) */
  version_struct->major = 0;
  while (*version_text >= ASCII_0 && *version_text <= ASCII_9) {
    version_struct->major
        = 10 * version_struct->major + (*version_text++ - ASCII_0);
  }
  if (*version_text++ != ASCII_PERIOD)
    return XML_FALSE;

  /* Now for the minor version number */
  version_struct->minor = 0;
  while (*version_text >= ASCII_0 && *version_text <= ASCII_9) {
    version_struct->minor
        = 10 * version_struct->minor + (*version_text++ - ASCII_0);
  }
  if (*version_text++ != ASCII_PERIOD)
    return XML_FALSE;

  /* Finally the micro version number */
  version_struct->micro = 0;
  while (*version_text >= ASCII_0 && *version_text <= ASCII_9) {
    version_struct->micro
        = 10 * version_struct->micro + (*version_text++ - ASCII_0);
  }
  if (*version_text != 0x00)
    return XML_FALSE;
  return XML_TRUE;
}

static int
versions_equal(const XML_Expat_Version *first,
               const XML_Expat_Version *second) {
  return (first->major == second->major && first->minor == second->minor
          && first->micro == second->micro);
}

START_TEST(test_misc_version) {
  XML_Expat_Version read_version = XML_ExpatVersionInfo();
  /* Silence compiler warning with the following assignment */
  XML_Expat_Version parsed_version = {0, 0, 0};
  const XML_LChar *version_text = XML_ExpatVersion();

  if (version_text == NULL)
    fail("Could not obtain version text");
  assert(version_text != NULL);
  if (! parse_version(version_text, &parsed_version))
    fail("Unable to parse version text");
  if (! versions_equal(&read_version, &parsed_version))
    fail("Version mismatch");

#if ! defined(XML_UNICODE) || defined(XML_UNICODE_WCHAR_T)
  if (xcstrcmp(version_text, XCS("expat_2.5.0"))) /* needs bump on releases */
    fail("XML_*_VERSION in expat.h out of sync?\n");
#else
  /* If we have XML_UNICODE defined but not XML_UNICODE_WCHAR_T
   * then XML_LChar is defined as char, for some reason.
   */
  if (strcmp(version_text, "expat_2.2.5")) /* needs bump on releases */
    fail("XML_*_VERSION in expat.h out of sync?\n");
#endif /* ! defined(XML_UNICODE) || defined(XML_UNICODE_WCHAR_T) */
}
END_TEST

/* Test feature information */
START_TEST(test_misc_features) {
  const XML_Feature *features = XML_GetFeatureList();

  /* Prevent problems with double-freeing parsers */
  g_parser = NULL;
  if (features == NULL) {
    fail("Failed to get feature information");
  } else {
    /* Loop through the features checking what we can */
    while (features->feature != XML_FEATURE_END) {
      switch (features->feature) {
      case XML_FEATURE_SIZEOF_XML_CHAR:
        if (features->value != sizeof(XML_Char))
          fail("Incorrect size of XML_Char");
        break;
      case XML_FEATURE_SIZEOF_XML_LCHAR:
        if (features->value != sizeof(XML_LChar))
          fail("Incorrect size of XML_LChar");
        break;
      default:
        break;
      }
      features++;
    }
  }
}
END_TEST

/* Regression test for GitHub Issue #17: memory leak parsing attribute
 * values with mixed bound and unbound namespaces.
 */
START_TEST(test_misc_attribute_leak) {
  const char *text = "<D xmlns:L=\"D\" l:a='' L:a=''/>";
  XML_Memory_Handling_Suite memsuite
      = {tracking_malloc, tracking_realloc, tracking_free};

  g_parser = XML_ParserCreate_MM(XCS("UTF-8"), &memsuite, XCS("\n"));
  expect_failure(text, XML_ERROR_UNBOUND_PREFIX, "Unbound prefixes not found");
  XML_ParserFree(g_parser);
  /* Prevent the teardown trying to double free */
  g_parser = NULL;

  if (! tracking_report())
    fail("Memory leak found");
}
END_TEST

/* Test parser created for UTF-16LE is successful */
START_TEST(test_misc_utf16le) {
  const char text[] =
      /* <?xml version='1.0'?><q>Hi</q> */
      "<\0?\0x\0m\0l\0 \0"
      "v\0e\0r\0s\0i\0o\0n\0=\0'\0\x31\0.\0\x30\0'\0?\0>\0"
      "<\0q\0>\0H\0i\0<\0/\0q\0>\0";
  const XML_Char *expected = XCS("Hi");
  CharData storage;

  g_parser = XML_ParserCreate(XCS("UTF-16LE"));
  if (g_parser == NULL)
    fail("Parser not created");

  CharData_Init(&storage);
  XML_SetUserData(g_parser, &storage);
  XML_SetCharacterDataHandler(g_parser, accumulate_characters);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)sizeof(text) - 1, XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
  CharData_CheckXMLChars(&storage, expected);
}
END_TEST

typedef struct {
  XML_Parser parser;
  int deep;
} DataIssue240;

static void
start_element_issue_240(void *userData, const XML_Char *name,
                        const XML_Char **atts) {
  DataIssue240 *mydata = (DataIssue240 *)userData;
  UNUSED_P(name);
  UNUSED_P(atts);
  mydata->deep++;
}

static void
end_element_issue_240(void *userData, const XML_Char *name) {
  DataIssue240 *mydata = (DataIssue240 *)userData;

  UNUSED_P(name);
  mydata->deep--;
  if (mydata->deep == 0) {
    XML_StopParser(mydata->parser, 0);
  }
}

START_TEST(test_misc_stop_during_end_handler_issue_240_1) {
  XML_Parser parser;
  DataIssue240 *mydata;
  enum XML_Status result;
  const char *const doc1 = "<doc><e1/><e><foo/></e></doc>";

  parser = XML_ParserCreate(NULL);
  XML_SetElementHandler(parser, start_element_issue_240, end_element_issue_240);
  mydata = (DataIssue240 *)malloc(sizeof(DataIssue240));
  mydata->parser = parser;
  mydata->deep = 0;
  XML_SetUserData(parser, mydata);

  result = XML_Parse(parser, doc1, (int)strlen(doc1), 1);
  XML_ParserFree(parser);
  free(mydata);
  if (result != XML_STATUS_ERROR)
    fail("Stopping the parser did not work as expected");
}
END_TEST

START_TEST(test_misc_stop_during_end_handler_issue_240_2) {
  XML_Parser parser;
  DataIssue240 *mydata;
  enum XML_Status result;
  const char *const doc2 = "<doc><elem/></doc>";

  parser = XML_ParserCreate(NULL);
  XML_SetElementHandler(parser, start_element_issue_240, end_element_issue_240);
  mydata = (DataIssue240 *)malloc(sizeof(DataIssue240));
  mydata->parser = parser;
  mydata->deep = 0;
  XML_SetUserData(parser, mydata);

  result = XML_Parse(parser, doc2, (int)strlen(doc2), 1);
  XML_ParserFree(parser);
  free(mydata);
  if (result != XML_STATUS_ERROR)
    fail("Stopping the parser did not work as expected");
}
END_TEST

START_TEST(test_misc_deny_internal_entity_closing_doctype_issue_317) {
  const char *const inputOne = "<!DOCTYPE d [\n"
                               "<!ENTITY % e ']><d/>'>\n"
                               "\n"
                               "%e;";
  const char *const inputTwo = "<!DOCTYPE d [\n"
                               "<!ENTITY % e1 ']><d/>'><!ENTITY % e2 '&e1;'>\n"
                               "\n"
                               "%e2;";
  const char *const inputThree = "<!DOCTYPE d [\n"
                                 "<!ENTITY % e ']><d'>\n"
                                 "\n"
                                 "%e;";
  const char *const inputIssue317 = "<!DOCTYPE doc [\n"
                                    "<!ENTITY % foo ']>\n"
                                    "<doc>Hell<oc (#PCDATA)*>'>\n"
                                    "%foo;\n"
                                    "]>\n"
                                    "<doc>Hello, world</dVc>";

  const char *const inputs[] = {inputOne, inputTwo, inputThree, inputIssue317};
  size_t inputIndex = 0;

  for (; inputIndex < sizeof(inputs) / sizeof(inputs[0]); inputIndex++) {
    XML_Parser parser;
    enum XML_Status parseResult;
    int setParamEntityResult;
    XML_Size lineNumber;
    XML_Size columnNumber;
    const char *const input = inputs[inputIndex];

    parser = XML_ParserCreate(NULL);
    setParamEntityResult
        = XML_SetParamEntityParsing(parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    if (setParamEntityResult != 1)
      fail("Failed to set XML_PARAM_ENTITY_PARSING_ALWAYS.");

    parseResult = XML_Parse(parser, input, (int)strlen(input), 0);
    if (parseResult != XML_STATUS_ERROR) {
      parseResult = XML_Parse(parser, "", 0, 1);
      if (parseResult != XML_STATUS_ERROR) {
        fail("Parsing was expected to fail but succeeded.");
      }
    }

    if (XML_GetErrorCode(parser) != XML_ERROR_INVALID_TOKEN)
      fail("Error code does not match XML_ERROR_INVALID_TOKEN");

    lineNumber = XML_GetCurrentLineNumber(parser);
    if (lineNumber != 4)
      fail("XML_GetCurrentLineNumber does not work as expected.");

    columnNumber = XML_GetCurrentColumnNumber(parser);
    if (columnNumber != 0)
      fail("XML_GetCurrentColumnNumber does not work as expected.");

    XML_ParserFree(parser);
  }
}
END_TEST

START_TEST(test_misc_tag_mismatch_reset_leak) {
#ifdef XML_NS
  const char *const text = "<open xmlns='https://namespace1.test'></close>";
  XML_Parser parser = XML_ParserCreateNS(NULL, XCS('\n'));

  if (XML_Parse(parser, text, (int)strlen(text), XML_TRUE) != XML_STATUS_ERROR)
    fail("Call to parse was expected to fail");
  if (XML_GetErrorCode(parser) != XML_ERROR_TAG_MISMATCH)
    fail("Call to parse was expected to fail from a closing tag mismatch");

  XML_ParserReset(parser, NULL);

  if (XML_Parse(parser, text, (int)strlen(text), XML_TRUE) != XML_STATUS_ERROR)
    fail("Call to parse was expected to fail");
  if (XML_GetErrorCode(parser) != XML_ERROR_TAG_MISMATCH)
    fail("Call to parse was expected to fail from a closing tag mismatch");

  XML_ParserFree(parser);
#endif
}
END_TEST

static void
alloc_setup(void) {
  XML_Memory_Handling_Suite memsuite = {duff_allocator, duff_reallocator, free};

  /* Ensure the parser creation will go through */
  allocation_count = ALLOC_ALWAYS_SUCCEED;
  reallocation_count = REALLOC_ALWAYS_SUCCEED;
  g_parser = XML_ParserCreate_MM(NULL, &memsuite, NULL);
  if (g_parser == NULL)
    fail("Parser not created");
}

static void
alloc_teardown(void) {
  basic_teardown();
}

/* Test the effects of allocation failures on xml declaration processing */
START_TEST(test_alloc_parse_xdecl) {
  const char *text = "<?xml version='1.0' encoding='utf-8'?>\n"
                     "<doc>Hello, world</doc>";
  int i;
  const int max_alloc_count = 15;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetXmlDeclHandler(g_parser, dummy_xdecl_handler);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* Resetting the parser is insufficient, because some memory
     * allocations are cached within the parser.  Instead we use
     * the teardown and setup routines to ensure that we have the
     * right sort of parser back in our hands.
     */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing allocator");
  if (i == max_alloc_count)
    fail("Parse failed with max allocations");
}
END_TEST

/* As above, but with an encoding big enough to cause storing the
 * version information to expand the string pool being used.
 */
static int XMLCALL
long_encoding_handler(void *userData, const XML_Char *encoding,
                      XML_Encoding *info) {
  int i;

  UNUSED_P(userData);
  UNUSED_P(encoding);
  for (i = 0; i < 256; i++)
    info->map[i] = i;
  info->data = NULL;
  info->convert = NULL;
  info->release = NULL;
  return XML_STATUS_OK;
}

START_TEST(test_alloc_parse_xdecl_2) {
  const char *text
      = "<?xml version='1.0' encoding='"
        /* Each line is 64 characters */
        "ThisIsAStupidlyLongEncodingNameIntendedToTriggerPoolGrowth123456"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMN"
        "'?>"
        "<doc>Hello, world</doc>";
  int i;
  const int max_alloc_count = 20;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetXmlDeclHandler(g_parser, dummy_xdecl_handler);
    XML_SetUnknownEncodingHandler(g_parser, long_encoding_handler, NULL);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing allocator");
  if (i == max_alloc_count)
    fail("Parse failed with max allocations");
}
END_TEST

/* Test the effects of allocation failures on a straightforward parse */
START_TEST(test_alloc_parse_pi) {
  const char *text = "<?xml version='1.0' encoding='utf-8'?>\n"
                     "<?pi unknown?>\n"
                     "<doc>"
                     "Hello, world"
                     "</doc>";
  int i;
  const int max_alloc_count = 15;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetProcessingInstructionHandler(g_parser, dummy_pi_handler);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing allocator");
  if (i == max_alloc_count)
    fail("Parse failed with max allocations");
}
END_TEST

START_TEST(test_alloc_parse_pi_2) {
  const char *text = "<?xml version='1.0' encoding='utf-8'?>\n"
                     "<doc>"
                     "Hello, world"
                     "<?pi unknown?>\n"
                     "</doc>";
  int i;
  const int max_alloc_count = 15;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetProcessingInstructionHandler(g_parser, dummy_pi_handler);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing allocator");
  if (i == max_alloc_count)
    fail("Parse failed with max allocations");
}
END_TEST

START_TEST(test_alloc_parse_pi_3) {
  const char *text
      = "<?"
        /* 64 characters per line */
        "This processing instruction should be long enough to ensure that"
        "it triggers the growth of an internal string pool when the      "
        "allocator fails at a cruicial moment FGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "Q?><doc/>";
  int i;
  const int max_alloc_count = 20;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetProcessingInstructionHandler(g_parser, dummy_pi_handler);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing allocator");
  if (i == max_alloc_count)
    fail("Parse failed with max allocations");
}
END_TEST

START_TEST(test_alloc_parse_comment) {
  const char *text = "<?xml version='1.0' encoding='utf-8'?>\n"
                     "<!-- Test parsing this comment -->"
                     "<doc>Hi</doc>";
  int i;
  const int max_alloc_count = 15;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetCommentHandler(g_parser, dummy_comment_handler);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing allocator");
  if (i == max_alloc_count)
    fail("Parse failed with max allocations");
}
END_TEST

START_TEST(test_alloc_parse_comment_2) {
  const char *text = "<?xml version='1.0' encoding='utf-8'?>\n"
                     "<doc>"
                     "Hello, world"
                     "<!-- Parse this comment too -->"
                     "</doc>";
  int i;
  const int max_alloc_count = 15;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetCommentHandler(g_parser, dummy_comment_handler);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing allocator");
  if (i == max_alloc_count)
    fail("Parse failed with max allocations");
}
END_TEST

static int XMLCALL
external_entity_duff_loader(XML_Parser parser, const XML_Char *context,
                            const XML_Char *base, const XML_Char *systemId,
                            const XML_Char *publicId) {
  XML_Parser new_parser;
  unsigned int i;
  const unsigned int max_alloc_count = 10;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  /* Try a few different allocation levels */
  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    new_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
    if (new_parser != NULL) {
      XML_ParserFree(new_parser);
      break;
    }
  }
  if (i == 0)
    fail("External parser creation ignored failing allocator");
  else if (i == max_alloc_count)
    fail("Extern parser not created with max allocation count");

  /* Make sure other random allocation doesn't now fail */
  allocation_count = ALLOC_ALWAYS_SUCCEED;

  /* Make sure the failure code path is executed too */
  return XML_STATUS_ERROR;
}

/* Test that external parser creation running out of memory is
 * correctly reported.  Based on the external entity test cases.
 */
START_TEST(test_alloc_create_external_parser) {
  const char *text = "<?xml version='1.0' encoding='us-ascii'?>\n"
                     "<!DOCTYPE doc SYSTEM 'foo'>\n"
                     "<doc>&entity;</doc>";
  char foo_text[] = "<!ELEMENT doc (#PCDATA)*>";

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetUserData(g_parser, foo_text);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_duff_loader);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_ERROR) {
    fail("External parser allocator returned success incorrectly");
  }
}
END_TEST

/* More external parser memory allocation testing */
START_TEST(test_alloc_run_external_parser) {
  const char *text = "<?xml version='1.0' encoding='us-ascii'?>\n"
                     "<!DOCTYPE doc SYSTEM 'foo'>\n"
                     "<doc>&entity;</doc>";
  char foo_text[] = "<!ELEMENT doc (#PCDATA)*>";
  unsigned int i;
  const unsigned int max_alloc_count = 15;

  for (i = 0; i < max_alloc_count; i++) {
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetUserData(g_parser, foo_text);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_null_loader);
    allocation_count = i;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parsing ignored failing allocator");
  else if (i == max_alloc_count)
    fail("Parsing failed with allocation count 10");
}
END_TEST

static int XMLCALL
external_entity_dbl_handler(XML_Parser parser, const XML_Char *context,
                            const XML_Char *base, const XML_Char *systemId,
                            const XML_Char *publicId) {
  intptr_t callno = (intptr_t)XML_GetUserData(parser);
  const char *text;
  XML_Parser new_parser;
  int i;
  const int max_alloc_count = 20;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  if (callno == 0) {
    /* First time through, check how many calls to malloc occur */
    text = ("<!ELEMENT doc (e+)>\n"
            "<!ATTLIST doc xmlns CDATA #IMPLIED>\n"
            "<!ELEMENT e EMPTY>\n");
    allocation_count = 10000;
    new_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
    if (new_parser == NULL) {
      fail("Unable to allocate first external parser");
      return XML_STATUS_ERROR;
    }
    /* Stash the number of calls in the user data */
    XML_SetUserData(parser, (void *)(intptr_t)(10000 - allocation_count));
  } else {
    text = ("<?xml version='1.0' encoding='us-ascii'?>"
            "<e/>");
    /* Try at varying levels to exercise more code paths */
    for (i = 0; i < max_alloc_count; i++) {
      allocation_count = callno + i;
      new_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
      if (new_parser != NULL)
        break;
    }
    if (i == 0) {
      fail("Second external parser unexpectedly created");
      XML_ParserFree(new_parser);
      return XML_STATUS_ERROR;
    } else if (i == max_alloc_count) {
      fail("Second external parser not created");
      return XML_STATUS_ERROR;
    }
  }

  allocation_count = ALLOC_ALWAYS_SUCCEED;
  if (_XML_Parse_SINGLE_BYTES(new_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR) {
    xml_failure(new_parser);
    return XML_STATUS_ERROR;
  }
  XML_ParserFree(new_parser);
  return XML_STATUS_OK;
}

/* Test that running out of memory in dtdCopy is correctly reported.
 * Based on test_default_ns_from_ext_subset_and_ext_ge()
 */
START_TEST(test_alloc_dtd_copy_default_atts) {
  const char *text = "<?xml version='1.0'?>\n"
                     "<!DOCTYPE doc SYSTEM 'http://example.org/doc.dtd' [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/entity.ent'>\n"
                     "]>\n"
                     "<doc xmlns='http://example.org/ns1'>\n"
                     "&en;\n"
                     "</doc>";

  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_dbl_handler);
  XML_SetUserData(g_parser, NULL);
  if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);
}
END_TEST

static int XMLCALL
external_entity_dbl_handler_2(XML_Parser parser, const XML_Char *context,
                              const XML_Char *base, const XML_Char *systemId,
                              const XML_Char *publicId) {
  intptr_t callno = (intptr_t)XML_GetUserData(parser);
  const char *text;
  XML_Parser new_parser;
  enum XML_Status rv;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  if (callno == 0) {
    /* Try different allocation levels for whole exercise */
    text = ("<!ELEMENT doc (e+)>\n"
            "<!ATTLIST doc xmlns CDATA #IMPLIED>\n"
            "<!ELEMENT e EMPTY>\n");
    XML_SetUserData(parser, (void *)(intptr_t)1);
    new_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
    if (new_parser == NULL)
      return XML_STATUS_ERROR;
    rv = _XML_Parse_SINGLE_BYTES(new_parser, text, (int)strlen(text), XML_TRUE);
  } else {
    /* Just run through once */
    text = ("<?xml version='1.0' encoding='us-ascii'?>"
            "<e/>");
    new_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
    if (new_parser == NULL)
      return XML_STATUS_ERROR;
    rv = _XML_Parse_SINGLE_BYTES(new_parser, text, (int)strlen(text), XML_TRUE);
  }
  XML_ParserFree(new_parser);
  if (rv == XML_STATUS_ERROR)
    return XML_STATUS_ERROR;
  return XML_STATUS_OK;
}

/* Test more external entity allocation failure paths */
START_TEST(test_alloc_external_entity) {
  const char *text = "<?xml version='1.0'?>\n"
                     "<!DOCTYPE doc SYSTEM 'http://example.org/doc.dtd' [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/entity.ent'>\n"
                     "]>\n"
                     "<doc xmlns='http://example.org/ns1'>\n"
                     "&en;\n"
                     "</doc>";
  int i;
  const int alloc_test_max_repeats = 50;

  for (i = 0; i < alloc_test_max_repeats; i++) {
    allocation_count = -1;
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_dbl_handler_2);
    XML_SetUserData(g_parser, NULL);
    allocation_count = i;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        == XML_STATUS_OK)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  allocation_count = -1;
  if (i == 0)
    fail("External entity parsed despite duff allocator");
  if (i == alloc_test_max_repeats)
    fail("External entity not parsed at max allocation count");
}
END_TEST

/* Test more allocation failure paths */
static int XMLCALL
external_entity_alloc_set_encoding(XML_Parser parser, const XML_Char *context,
                                   const XML_Char *base,
                                   const XML_Char *systemId,
                                   const XML_Char *publicId) {
  /* As for external_entity_loader() */
  const char *text = "<?xml encoding='iso-8859-3'?>"
                     "\xC3\xA9";
  XML_Parser ext_parser;
  enum XML_Status status;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    return XML_STATUS_ERROR;
  if (! XML_SetEncoding(ext_parser, XCS("utf-8"))) {
    XML_ParserFree(ext_parser);
    return XML_STATUS_ERROR;
  }
  status
      = _XML_Parse_SINGLE_BYTES(ext_parser, text, (int)strlen(text), XML_TRUE);
  XML_ParserFree(ext_parser);
  if (status == XML_STATUS_ERROR)
    return XML_STATUS_ERROR;
  return XML_STATUS_OK;
}

START_TEST(test_alloc_ext_entity_set_encoding) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/dummy.ent'>\n"
                     "]>\n"
                     "<doc>&en;</doc>";
  int i;
  const int max_allocation_count = 30;

  for (i = 0; i < max_allocation_count; i++) {
    XML_SetExternalEntityRefHandler(g_parser,
                                    external_entity_alloc_set_encoding);
    allocation_count = i;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        == XML_STATUS_OK)
      break;
    allocation_count = -1;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Encoding check succeeded despite failing allocator");
  if (i == max_allocation_count)
    fail("Encoding failed at max allocation count");
}
END_TEST

static int XMLCALL
unknown_released_encoding_handler(void *data, const XML_Char *encoding,
                                  XML_Encoding *info) {
  UNUSED_P(data);
  if (! xcstrcmp(encoding, XCS("unsupported-encoding"))) {
    int i;

    for (i = 0; i < 256; i++)
      info->map[i] = i;
    info->data = NULL;
    info->convert = NULL;
    info->release = dummy_release;
    return XML_STATUS_OK;
  }
  return XML_STATUS_ERROR;
}

/* Test the effects of allocation failure in internal entities.
 * Based on test_unknown_encoding_internal_entity
 */
START_TEST(test_alloc_internal_entity) {
  const char *text = "<?xml version='1.0' encoding='unsupported-encoding'?>\n"
                     "<!DOCTYPE test [<!ENTITY foo 'bar'>]>\n"
                     "<test a='&foo;'/>";
  unsigned int i;
  const unsigned int max_alloc_count = 20;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetUnknownEncodingHandler(g_parser, unknown_released_encoding_handler,
                                  NULL);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Internal entity worked despite failing allocations");
  else if (i == max_alloc_count)
    fail("Internal entity failed at max allocation count");
}
END_TEST

/* Test the robustness against allocation failure of element handling
 * Based on test_dtd_default_handling().
 */
START_TEST(test_alloc_dtd_default_handling) {
  const char *text = "<!DOCTYPE doc [\n"
                     "<!ENTITY e SYSTEM 'http://example.org/e'>\n"
                     "<!NOTATION n SYSTEM 'http://example.org/n'>\n"
                     "<!ENTITY e1 SYSTEM 'http://example.org/e' NDATA n>\n"
                     "<!ELEMENT doc (#PCDATA)>\n"
                     "<!ATTLIST doc a CDATA #IMPLIED>\n"
                     "<?pi in dtd?>\n"
                     "<!--comment in dtd-->\n"
                     "]>\n"
                     "<doc><![CDATA[text in doc]]></doc>";
  const XML_Char *expected = XCS("\n\n\n\n\n\n\n\n\n<doc>text in doc</doc>");
  CharData storage;
  int i;
  const int max_alloc_count = 25;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    dummy_handler_flags = 0;
    XML_SetDefaultHandler(g_parser, accumulate_characters);
    XML_SetDoctypeDeclHandler(g_parser, dummy_start_doctype_handler,
                              dummy_end_doctype_handler);
    XML_SetEntityDeclHandler(g_parser, dummy_entity_decl_handler);
    XML_SetNotationDeclHandler(g_parser, dummy_notation_decl_handler);
    XML_SetElementDeclHandler(g_parser, dummy_element_decl_handler);
    XML_SetAttlistDeclHandler(g_parser, dummy_attlist_decl_handler);
    XML_SetProcessingInstructionHandler(g_parser, dummy_pi_handler);
    XML_SetCommentHandler(g_parser, dummy_comment_handler);
    XML_SetCdataSectionHandler(g_parser, dummy_start_cdata_handler,
                               dummy_end_cdata_handler);
    XML_SetUnparsedEntityDeclHandler(g_parser,
                                     dummy_unparsed_entity_decl_handler);
    CharData_Init(&storage);
    XML_SetUserData(g_parser, &storage);
    XML_SetCharacterDataHandler(g_parser, accumulate_characters);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Default DTD parsed despite allocation failures");
  if (i == max_alloc_count)
    fail("Default DTD not parsed with maximum alloc count");
  CharData_CheckXMLChars(&storage, expected);
  if (dummy_handler_flags
      != (DUMMY_START_DOCTYPE_HANDLER_FLAG | DUMMY_END_DOCTYPE_HANDLER_FLAG
          | DUMMY_ENTITY_DECL_HANDLER_FLAG | DUMMY_NOTATION_DECL_HANDLER_FLAG
          | DUMMY_ELEMENT_DECL_HANDLER_FLAG | DUMMY_ATTLIST_DECL_HANDLER_FLAG
          | DUMMY_COMMENT_HANDLER_FLAG | DUMMY_PI_HANDLER_FLAG
          | DUMMY_START_CDATA_HANDLER_FLAG | DUMMY_END_CDATA_HANDLER_FLAG
          | DUMMY_UNPARSED_ENTITY_DECL_HANDLER_FLAG))
    fail("Not all handlers were called");
}
END_TEST

/* Test robustness of XML_SetEncoding() with a failing allocator */
START_TEST(test_alloc_explicit_encoding) {
  int i;
  const int max_alloc_count = 5;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    if (XML_SetEncoding(g_parser, XCS("us-ascii")) == XML_STATUS_OK)
      break;
  }
  if (i == 0)
    fail("Encoding set despite failing allocator");
  else if (i == max_alloc_count)
    fail("Encoding not set at max allocation count");
}
END_TEST

/* Test robustness of XML_SetBase against a failing allocator */
START_TEST(test_alloc_set_base) {
  const XML_Char *new_base = XCS("/local/file/name.xml");
  int i;
  const int max_alloc_count = 5;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    if (XML_SetBase(g_parser, new_base) == XML_STATUS_OK)
      break;
  }
  if (i == 0)
    fail("Base set despite failing allocator");
  else if (i == max_alloc_count)
    fail("Base not set with max allocation count");
}
END_TEST

/* Test buffer extension in the face of a duff reallocator */
START_TEST(test_alloc_realloc_buffer) {
  const char *text = get_buffer_test_text;
  void *buffer;
  int i;
  const int max_realloc_count = 10;

  /* Get a smallish buffer */
  for (i = 0; i < max_realloc_count; i++) {
    reallocation_count = i;
    buffer = XML_GetBuffer(g_parser, 1536);
    if (buffer == NULL)
      fail("1.5K buffer reallocation failed");
    assert(buffer != NULL);
    memcpy(buffer, text, strlen(text));
    if (XML_ParseBuffer(g_parser, (int)strlen(text), XML_FALSE)
        == XML_STATUS_OK)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  reallocation_count = -1;
  if (i == 0)
    fail("Parse succeeded with no reallocation");
  else if (i == max_realloc_count)
    fail("Parse failed with max reallocation count");
}
END_TEST

/* Same test for external entity parsers */
static int XMLCALL
external_entity_reallocator(XML_Parser parser, const XML_Char *context,
                            const XML_Char *base, const XML_Char *systemId,
                            const XML_Char *publicId) {
  const char *text = get_buffer_test_text;
  XML_Parser ext_parser;
  void *buffer;
  enum XML_Status status;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    fail("Could not create external entity parser");

  reallocation_count = (intptr_t)XML_GetUserData(parser);
  buffer = XML_GetBuffer(ext_parser, 1536);
  if (buffer == NULL)
    fail("Buffer allocation failed");
  assert(buffer != NULL);
  memcpy(buffer, text, strlen(text));
  status = XML_ParseBuffer(ext_parser, (int)strlen(text), XML_FALSE);
  reallocation_count = -1;
  XML_ParserFree(ext_parser);
  return (status == XML_STATUS_OK) ? XML_STATUS_OK : XML_STATUS_ERROR;
}

START_TEST(test_alloc_ext_entity_realloc_buffer) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY en SYSTEM 'http://example.org/dummy.ent'>\n"
                     "]>\n"
                     "<doc>&en;</doc>";
  int i;
  const int max_realloc_count = 10;

  for (i = 0; i < max_realloc_count; i++) {
    XML_SetExternalEntityRefHandler(g_parser, external_entity_reallocator);
    XML_SetUserData(g_parser, (void *)(intptr_t)i);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        == XML_STATUS_OK)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Succeeded with no reallocations");
  if (i == max_realloc_count)
    fail("Failed with max reallocations");
}
END_TEST

/* Test elements with many attributes are handled correctly */
START_TEST(test_alloc_realloc_many_attributes) {
  const char *text = "<!DOCTYPE doc [\n"
                     "<!ATTLIST doc za CDATA 'default'>\n"
                     "<!ATTLIST doc zb CDATA 'def2'>\n"
                     "<!ATTLIST doc zc CDATA 'def3'>\n"
                     "]>\n"
                     "<doc a='1'"
                     "     b='2'"
                     "     c='3'"
                     "     d='4'"
                     "     e='5'"
                     "     f='6'"
                     "     g='7'"
                     "     h='8'"
                     "     i='9'"
                     "     j='10'"
                     "     k='11'"
                     "     l='12'"
                     "     m='13'"
                     "     n='14'"
                     "     p='15'"
                     "     q='16'"
                     "     r='17'"
                     "     s='18'>"
                     "</doc>";
  int i;
  const int max_realloc_count = 10;

  for (i = 0; i < max_realloc_count; i++) {
    reallocation_count = i;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite no reallocations");
  if (i == max_realloc_count)
    fail("Parse failed at max reallocations");
}
END_TEST

/* Test handling of a public entity with failing allocator */
START_TEST(test_alloc_public_entity_value) {
  const char *text = "<!DOCTYPE doc SYSTEM 'http://example.org/'>\n"
                     "<doc></doc>\n";
  char dtd_text[]
      = "<!ELEMENT doc EMPTY>\n"
        "<!ENTITY % e1 PUBLIC 'foo' 'bar.ent'>\n"
        "<!ENTITY % "
        /* Each line is 64 characters */
        "ThisIsAStupidlyLongParameterNameIntendedToTriggerPoolGrowth12345"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        " '%e1;'>\n"
        "%e1;\n";
  int i;
  const int max_alloc_count = 50;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    dummy_handler_flags = 0;
    XML_SetUserData(g_parser, dtd_text);
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_public);
    /* Provoke a particular code path */
    XML_SetEntityDeclHandler(g_parser, dummy_entity_decl_handler);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing allocation");
  if (i == max_alloc_count)
    fail("Parsing failed at max allocation count");
  if (dummy_handler_flags != DUMMY_ENTITY_DECL_HANDLER_FLAG)
    fail("Entity declaration handler not called");
}
END_TEST

START_TEST(test_alloc_realloc_subst_public_entity_value) {
  const char *text = "<!DOCTYPE doc SYSTEM 'http://example.org/'>\n"
                     "<doc></doc>\n";
  char dtd_text[]
      = "<!ELEMENT doc EMPTY>\n"
        "<!ENTITY % "
        /* Each line is 64 characters */
        "ThisIsAStupidlyLongParameterNameIntendedToTriggerPoolGrowth12345"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        " PUBLIC 'foo' 'bar.ent'>\n"
        "%ThisIsAStupidlyLongParameterNameIntendedToTriggerPoolGrowth12345"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP;";
  int i;
  const int max_realloc_count = 10;

  for (i = 0; i < max_realloc_count; i++) {
    reallocation_count = i;
    XML_SetUserData(g_parser, dtd_text);
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_public);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing reallocation");
  if (i == max_realloc_count)
    fail("Parsing failed at max reallocation count");
}
END_TEST

START_TEST(test_alloc_parse_public_doctype) {
  const char *text
      = "<?xml version='1.0' encoding='utf-8'?>\n"
        "<!DOCTYPE doc PUBLIC '"
        /* 64 characters per line */
        "http://example.com/a/long/enough/name/to/trigger/pool/growth/zz/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "' 'test'>\n"
        "<doc></doc>";
  int i;
  const int max_alloc_count = 25;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    dummy_handler_flags = 0;
    XML_SetDoctypeDeclHandler(g_parser, dummy_start_doctype_decl_handler,
                              dummy_end_doctype_decl_handler);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing allocator");
  if (i == max_alloc_count)
    fail("Parse failed at maximum allocation count");
  if (dummy_handler_flags
      != (DUMMY_START_DOCTYPE_DECL_HANDLER_FLAG
          | DUMMY_END_DOCTYPE_DECL_HANDLER_FLAG))
    fail("Doctype handler functions not called");
}
END_TEST

START_TEST(test_alloc_parse_public_doctype_long_name) {
  const char *text
      = "<?xml version='1.0' encoding='utf-8'?>\n"
        "<!DOCTYPE doc PUBLIC 'http://example.com/foo' '"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNOP"
        "'>\n"
        "<doc></doc>";
  int i;
  const int max_alloc_count = 25;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetDoctypeDeclHandler(g_parser, dummy_start_doctype_decl_handler,
                              dummy_end_doctype_decl_handler);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing allocator");
  if (i == max_alloc_count)
    fail("Parse failed at maximum allocation count");
}
END_TEST

static int XMLCALL
external_entity_alloc(XML_Parser parser, const XML_Char *context,
                      const XML_Char *base, const XML_Char *systemId,
                      const XML_Char *publicId) {
  const char *text = (const char *)XML_GetUserData(parser);
  XML_Parser ext_parser;
  int parse_res;

  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);
  ext_parser = XML_ExternalEntityParserCreate(parser, context, NULL);
  if (ext_parser == NULL)
    return XML_STATUS_ERROR;
  parse_res
      = _XML_Parse_SINGLE_BYTES(ext_parser, text, (int)strlen(text), XML_TRUE);
  XML_ParserFree(ext_parser);
  return parse_res;
}

/* Test foreign DTD handling */
START_TEST(test_alloc_set_foreign_dtd) {
  const char *text1 = "<?xml version='1.0' encoding='us-ascii'?>\n"
                      "<doc>&entity;</doc>";
  char text2[] = "<!ELEMENT doc (#PCDATA)*>";
  int i;
  const int max_alloc_count = 25;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetUserData(g_parser, &text2);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_alloc);
    if (XML_UseForeignDTD(g_parser, XML_TRUE) != XML_ERROR_NONE)
      fail("Could not set foreign DTD");
    if (_XML_Parse_SINGLE_BYTES(g_parser, text1, (int)strlen(text1), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing allocator");
  if (i == max_alloc_count)
    fail("Parse failed at maximum allocation count");
}
END_TEST

/* Test based on ibm/valid/P32/ibm32v04.xml */
START_TEST(test_alloc_attribute_enum_value) {
  const char *text = "<?xml version='1.0' standalone='no'?>\n"
                     "<!DOCTYPE animal SYSTEM 'test.dtd'>\n"
                     "<animal>This is a \n    <a/>  \n\nyellow tiger</animal>";
  char dtd_text[] = "<!ELEMENT animal (#PCDATA|a)*>\n"
                    "<!ELEMENT a EMPTY>\n"
                    "<!ATTLIST animal xml:space (default|preserve) 'preserve'>";
  int i;
  const int max_alloc_count = 30;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetExternalEntityRefHandler(g_parser, external_entity_alloc);
    XML_SetUserData(g_parser, dtd_text);
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    /* An attribute list handler provokes a different code path */
    XML_SetAttlistDeclHandler(g_parser, dummy_attlist_decl_handler);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing allocator");
  if (i == max_alloc_count)
    fail("Parse failed at maximum allocation count");
}
END_TEST

/* Test attribute enums sufficient to overflow the string pool */
START_TEST(test_alloc_realloc_attribute_enum_value) {
  const char *text = "<?xml version='1.0' standalone='no'?>\n"
                     "<!DOCTYPE animal SYSTEM 'test.dtd'>\n"
                     "<animal>This is a yellow tiger</animal>";
  /* We wish to define a collection of attribute enums that will
   * cause the string pool storing them to have to expand.  This
   * means more than 1024 bytes, including the parentheses and
   * separator bars.
   */
  char dtd_text[]
      = "<!ELEMENT animal (#PCDATA)*>\n"
        "<!ATTLIST animal thing "
        "(default"
        /* Each line is 64 characters */
        "|ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|BBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|CBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|DBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|EBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|FBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|GBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|HBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|IBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|JBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|KBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|LBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|MBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|NBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|OBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|PBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO)"
        " 'default'>";
  int i;
  const int max_realloc_count = 10;

  for (i = 0; i < max_realloc_count; i++) {
    reallocation_count = i;
    XML_SetExternalEntityRefHandler(g_parser, external_entity_alloc);
    XML_SetUserData(g_parser, dtd_text);
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    /* An attribute list handler provokes a different code path */
    XML_SetAttlistDeclHandler(g_parser, dummy_attlist_decl_handler);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing reallocator");
  if (i == max_realloc_count)
    fail("Parse failed at maximum reallocation count");
}
END_TEST

/* Test attribute enums in a #IMPLIED attribute forcing pool growth */
START_TEST(test_alloc_realloc_implied_attribute) {
  /* Forcing this particular code path is a balancing act.  The
   * addition of the closing parenthesis and terminal NUL must be
   * what pushes the string of enums over the 1024-byte limit,
   * otherwise a different code path will pick up the realloc.
   */
  const char *text
      = "<!DOCTYPE doc [\n"
        "<!ELEMENT doc EMPTY>\n"
        "<!ATTLIST doc a "
        /* Each line is 64 characters */
        "(ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|BBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|CBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|DBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|EBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|FBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|GBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|HBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|IBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|JBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|KBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|LBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|MBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|NBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|OBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|PBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMN)"
        " #IMPLIED>\n"
        "]><doc/>";
  int i;
  const int max_realloc_count = 10;

  for (i = 0; i < max_realloc_count; i++) {
    reallocation_count = i;
    XML_SetAttlistDeclHandler(g_parser, dummy_attlist_decl_handler);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing reallocator");
  if (i == max_realloc_count)
    fail("Parse failed at maximum reallocation count");
}
END_TEST

/* Test attribute enums in a defaulted attribute forcing pool growth */
START_TEST(test_alloc_realloc_default_attribute) {
  /* Forcing this particular code path is a balancing act.  The
   * addition of the closing parenthesis and terminal NUL must be
   * what pushes the string of enums over the 1024-byte limit,
   * otherwise a different code path will pick up the realloc.
   */
  const char *text
      = "<!DOCTYPE doc [\n"
        "<!ELEMENT doc EMPTY>\n"
        "<!ATTLIST doc a "
        /* Each line is 64 characters */
        "(ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|BBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|CBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|DBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|EBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|FBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|GBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|HBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|IBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|JBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|KBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|LBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|MBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|NBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|OBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO"
        "|PBCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMN)"
        " 'ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNO'"
        ">\n]><doc/>";
  int i;
  const int max_realloc_count = 10;

  for (i = 0; i < max_realloc_count; i++) {
    reallocation_count = i;
    XML_SetAttlistDeclHandler(g_parser, dummy_attlist_decl_handler);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing reallocator");
  if (i == max_realloc_count)
    fail("Parse failed at maximum reallocation count");
}
END_TEST

/* Test long notation name with dodgy allocator */
START_TEST(test_alloc_notation) {
  const char *text
      = "<!DOCTYPE doc [\n"
        "<!NOTATION "
        /* Each line is 64 characters */
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        " SYSTEM 'http://example.org/n'>\n"
        "<!ENTITY e SYSTEM 'http://example.org/e' NDATA "
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        ">\n"
        "<!ELEMENT doc EMPTY>\n"
        "]>\n<doc/>";
  int i;
  const int max_alloc_count = 20;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    dummy_handler_flags = 0;
    XML_SetNotationDeclHandler(g_parser, dummy_notation_decl_handler);
    XML_SetEntityDeclHandler(g_parser, dummy_entity_decl_handler);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite allocation failures");
  if (i == max_alloc_count)
    fail("Parse failed at maximum allocation count");
  if (dummy_handler_flags
      != (DUMMY_ENTITY_DECL_HANDLER_FLAG | DUMMY_NOTATION_DECL_HANDLER_FLAG))
    fail("Entity declaration handler not called");
}
END_TEST

/* Test public notation with dodgy allocator */
START_TEST(test_alloc_public_notation) {
  const char *text
      = "<!DOCTYPE doc [\n"
        "<!NOTATION note PUBLIC '"
        /* 64 characters per line */
        "http://example.com/a/long/enough/name/to/trigger/pool/growth/zz/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "' 'foo'>\n"
        "<!ENTITY e SYSTEM 'http://example.com/e' NDATA note>\n"
        "<!ELEMENT doc EMPTY>\n"
        "]>\n<doc/>";
  int i;
  const int max_alloc_count = 20;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    dummy_handler_flags = 0;
    XML_SetNotationDeclHandler(g_parser, dummy_notation_decl_handler);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite allocation failures");
  if (i == max_alloc_count)
    fail("Parse failed at maximum allocation count");
  if (dummy_handler_flags != DUMMY_NOTATION_DECL_HANDLER_FLAG)
    fail("Notation handler not called");
}
END_TEST

/* Test public notation with dodgy allocator */
START_TEST(test_alloc_system_notation) {
  const char *text
      = "<!DOCTYPE doc [\n"
        "<!NOTATION note SYSTEM '"
        /* 64 characters per line */
        "http://example.com/a/long/enough/name/to/trigger/pool/growth/zz/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "'>\n"
        "<!ENTITY e SYSTEM 'http://example.com/e' NDATA note>\n"
        "<!ELEMENT doc EMPTY>\n"
        "]>\n<doc/>";
  int i;
  const int max_alloc_count = 20;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    dummy_handler_flags = 0;
    XML_SetNotationDeclHandler(g_parser, dummy_notation_decl_handler);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite allocation failures");
  if (i == max_alloc_count)
    fail("Parse failed at maximum allocation count");
  if (dummy_handler_flags != DUMMY_NOTATION_DECL_HANDLER_FLAG)
    fail("Notation handler not called");
}
END_TEST

START_TEST(test_alloc_nested_groups) {
  const char *text
      = "<!DOCTYPE doc [\n"
        "<!ELEMENT doc "
        /* Sixteen elements per line */
        "(e,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,"
        "(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?"
        "))))))))))))))))))))))))))))))))>\n"
        "<!ELEMENT e EMPTY>"
        "]>\n"
        "<doc><e/></doc>";
  CharData storage;
  int i;
  const int max_alloc_count = 20;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    CharData_Init(&storage);
    XML_SetElementDeclHandler(g_parser, dummy_element_decl_handler);
    XML_SetStartElementHandler(g_parser, record_element_start_handler);
    XML_SetUserData(g_parser, &storage);
    dummy_handler_flags = 0;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }

  if (i == 0)
    fail("Parse succeeded despite failing reallocator");
  if (i == max_alloc_count)
    fail("Parse failed at maximum reallocation count");
  CharData_CheckXMLChars(&storage, XCS("doce"));
  if (dummy_handler_flags != DUMMY_ELEMENT_DECL_HANDLER_FLAG)
    fail("Element handler not fired");
}
END_TEST

START_TEST(test_alloc_realloc_nested_groups) {
  const char *text
      = "<!DOCTYPE doc [\n"
        "<!ELEMENT doc "
        /* Sixteen elements per line */
        "(e,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,"
        "(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?,(e?"
        "))))))))))))))))))))))))))))))))>\n"
        "<!ELEMENT e EMPTY>"
        "]>\n"
        "<doc><e/></doc>";
  CharData storage;
  int i;
  const int max_realloc_count = 10;

  for (i = 0; i < max_realloc_count; i++) {
    reallocation_count = i;
    CharData_Init(&storage);
    XML_SetElementDeclHandler(g_parser, dummy_element_decl_handler);
    XML_SetStartElementHandler(g_parser, record_element_start_handler);
    XML_SetUserData(g_parser, &storage);
    dummy_handler_flags = 0;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }

  if (i == 0)
    fail("Parse succeeded despite failing reallocator");
  if (i == max_realloc_count)
    fail("Parse failed at maximum reallocation count");
  CharData_CheckXMLChars(&storage, XCS("doce"));
  if (dummy_handler_flags != DUMMY_ELEMENT_DECL_HANDLER_FLAG)
    fail("Element handler not fired");
}
END_TEST

START_TEST(test_alloc_large_group) {
  const char *text = "<!DOCTYPE doc [\n"
                     "<!ELEMENT doc ("
                     "a1|a2|a3|a4|a5|a6|a7|a8|"
                     "b1|b2|b3|b4|b5|b6|b7|b8|"
                     "c1|c2|c3|c4|c5|c6|c7|c8|"
                     "d1|d2|d3|d4|d5|d6|d7|d8|"
                     "e1"
                     ")+>\n"
                     "]>\n"
                     "<doc>\n"
                     "<a1/>\n"
                     "</doc>\n";
  int i;
  const int max_alloc_count = 50;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetElementDeclHandler(g_parser, dummy_element_decl_handler);
    dummy_handler_flags = 0;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing allocator");
  if (i == max_alloc_count)
    fail("Parse failed at maximum allocation count");
  if (dummy_handler_flags != DUMMY_ELEMENT_DECL_HANDLER_FLAG)
    fail("Element handler flag not raised");
}
END_TEST

START_TEST(test_alloc_realloc_group_choice) {
  const char *text = "<!DOCTYPE doc [\n"
                     "<!ELEMENT doc ("
                     "a1|a2|a3|a4|a5|a6|a7|a8|"
                     "b1|b2|b3|b4|b5|b6|b7|b8|"
                     "c1|c2|c3|c4|c5|c6|c7|c8|"
                     "d1|d2|d3|d4|d5|d6|d7|d8|"
                     "e1"
                     ")+>\n"
                     "]>\n"
                     "<doc>\n"
                     "<a1/>\n"
                     "<b2 attr='foo'>This is a foo</b2>\n"
                     "<c3></c3>\n"
                     "</doc>\n";
  int i;
  const int max_realloc_count = 10;

  for (i = 0; i < max_realloc_count; i++) {
    reallocation_count = i;
    XML_SetElementDeclHandler(g_parser, dummy_element_decl_handler);
    dummy_handler_flags = 0;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing reallocator");
  if (i == max_realloc_count)
    fail("Parse failed at maximum reallocation count");
  if (dummy_handler_flags != DUMMY_ELEMENT_DECL_HANDLER_FLAG)
    fail("Element handler flag not raised");
}
END_TEST

START_TEST(test_alloc_pi_in_epilog) {
  const char *text = "<doc></doc>\n"
                     "<?pi in epilog?>";
  int i;
  const int max_alloc_count = 15;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetProcessingInstructionHandler(g_parser, dummy_pi_handler);
    dummy_handler_flags = 0;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse completed despite failing allocator");
  if (i == max_alloc_count)
    fail("Parse failed at maximum allocation count");
  if (dummy_handler_flags != DUMMY_PI_HANDLER_FLAG)
    fail("Processing instruction handler not invoked");
}
END_TEST

START_TEST(test_alloc_comment_in_epilog) {
  const char *text = "<doc></doc>\n"
                     "<!-- comment in epilog -->";
  int i;
  const int max_alloc_count = 15;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetCommentHandler(g_parser, dummy_comment_handler);
    dummy_handler_flags = 0;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse completed despite failing allocator");
  if (i == max_alloc_count)
    fail("Parse failed at maximum allocation count");
  if (dummy_handler_flags != DUMMY_COMMENT_HANDLER_FLAG)
    fail("Processing instruction handler not invoked");
}
END_TEST

START_TEST(test_alloc_realloc_long_attribute_value) {
  const char *text
      = "<!DOCTYPE doc [<!ENTITY foo '"
        /* Each line is 64 characters */
        "This entity will be substituted as an attribute value, and is   "
        "calculated to be exactly long enough that the terminating NUL   "
        "that the library adds internally will trigger the string pool to"
        "grow. GHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "'>]>\n"
        "<doc a='&foo;'></doc>";
  int i;
  const int max_realloc_count = 10;

  for (i = 0; i < max_realloc_count; i++) {
    reallocation_count = i;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing reallocator");
  if (i == max_realloc_count)
    fail("Parse failed at maximum reallocation count");
}
END_TEST

START_TEST(test_alloc_attribute_whitespace) {
  const char *text = "<doc a=' '></doc>";
  int i;
  const int max_alloc_count = 15;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing allocator");
  if (i == max_alloc_count)
    fail("Parse failed at maximum allocation count");
}
END_TEST

START_TEST(test_alloc_attribute_predefined_entity) {
  const char *text = "<doc a='&amp;'></doc>";
  int i;
  const int max_alloc_count = 15;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing allocator");
  if (i == max_alloc_count)
    fail("Parse failed at maximum allocation count");
}
END_TEST

/* Test that a character reference at the end of a suitably long
 * default value for an attribute can trigger pool growth, and recovers
 * if the allocator fails on it.
 */
START_TEST(test_alloc_long_attr_default_with_char_ref) {
  const char *text
      = "<!DOCTYPE doc [<!ATTLIST doc a CDATA '"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHI"
        "&#x31;'>]>\n"
        "<doc/>";
  int i;
  const int max_alloc_count = 20;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing allocator");
  if (i == max_alloc_count)
    fail("Parse failed at maximum allocation count");
}
END_TEST

/* Test that a long character reference substitution triggers a pool
 * expansion correctly for an attribute value.
 */
START_TEST(test_alloc_long_attr_value) {
  const char *text
      = "<!DOCTYPE test [<!ENTITY foo '\n"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "'>]>\n"
        "<test a='&foo;'/>";
  int i;
  const int max_alloc_count = 25;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing allocator");
  if (i == max_alloc_count)
    fail("Parse failed at maximum allocation count");
}
END_TEST

/* Test that an error in a nested parameter entity substitution is
 * handled correctly.  It seems unlikely that the code path being
 * exercised can be reached purely by carefully crafted XML, but an
 * allocation error in the right place will definitely do it.
 */
START_TEST(test_alloc_nested_entities) {
  const char *text = "<!DOCTYPE doc SYSTEM 'http://example.org/one.ent'>\n"
                     "<doc />";
  ExtFaults test_data
      = {"<!ENTITY % pe1 '"
         /* 64 characters per line */
         "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
         "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
         "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
         "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
         "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
         "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
         "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
         "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
         "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
         "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
         "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
         "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
         "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
         "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
         "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
         "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
         "'>\n"
         "<!ENTITY % pe2 '%pe1;'>\n"
         "%pe2;",
         "Memory Fail not faulted", NULL, XML_ERROR_NO_MEMORY};

  /* Causes an allocation error in a nested storeEntityValue() */
  allocation_count = 12;
  XML_SetUserData(g_parser, &test_data);
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
  XML_SetExternalEntityRefHandler(g_parser, external_entity_faulter);
  expect_failure(text, XML_ERROR_EXTERNAL_ENTITY_HANDLING,
                 "Entity allocation failure not noted");
}
END_TEST

START_TEST(test_alloc_realloc_param_entity_newline) {
  const char *text = "<!DOCTYPE doc SYSTEM 'http://example.org/'>\n"
                     "<doc/>";
  char dtd_text[]
      = "<!ENTITY % pe '<!ATTLIST doc att CDATA \""
        /* 64 characters per line */
        "This default value is carefully crafted so that the carriage    "
        "return right at the end of the entity string causes an internal "
        "string pool to have to grow.  This allows us to test the alloc  "
        "failure path from that point. OPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDE"
        "\">\n'>"
        "%pe;\n";
  int i;
  const int max_realloc_count = 5;

  for (i = 0; i < max_realloc_count; i++) {
    reallocation_count = i;
    XML_SetUserData(g_parser, dtd_text);
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_alloc);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing reallocator");
  if (i == max_realloc_count)
    fail("Parse failed at maximum reallocation count");
}
END_TEST

START_TEST(test_alloc_realloc_ce_extends_pe) {
  const char *text = "<!DOCTYPE doc SYSTEM 'http://example.org/'>\n"
                     "<doc/>";
  char dtd_text[]
      = "<!ENTITY % pe '<!ATTLIST doc att CDATA \""
        /* 64 characters per line */
        "This default value is carefully crafted so that the character   "
        "entity at the end causes an internal string pool to have to     "
        "grow.  This allows us to test the allocation failure path from  "
        "that point onwards. EFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFG&#x51;"
        "\">\n'>"
        "%pe;\n";
  int i;
  const int max_realloc_count = 5;

  for (i = 0; i < max_realloc_count; i++) {
    reallocation_count = i;
    XML_SetUserData(g_parser, dtd_text);
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_alloc);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parse succeeded despite failing reallocator");
  if (i == max_realloc_count)
    fail("Parse failed at maximum reallocation count");
}
END_TEST

START_TEST(test_alloc_realloc_attributes) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ATTLIST doc\n"
                     "    a1  (a|b|c)   'a'\n"
                     "    a2  (foo|bar) #IMPLIED\n"
                     "    a3  NMTOKEN   #IMPLIED\n"
                     "    a4  NMTOKENS  #IMPLIED\n"
                     "    a5  ID        #IMPLIED\n"
                     "    a6  IDREF     #IMPLIED\n"
                     "    a7  IDREFS    #IMPLIED\n"
                     "    a8  ENTITY    #IMPLIED\n"
                     "    a9  ENTITIES  #IMPLIED\n"
                     "    a10 CDATA     #IMPLIED\n"
                     "  >]>\n"
                     "<doc>wombat</doc>\n";
  int i;
  const int max_realloc_count = 5;

  for (i = 0; i < max_realloc_count; i++) {
    reallocation_count = i;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }

  if (i == 0)
    fail("Parse succeeded despite failing reallocator");
  if (i == max_realloc_count)
    fail("Parse failed at maximum reallocation count");
}
END_TEST

START_TEST(test_alloc_long_doc_name) {
  const char *text =
      /* 64 characters per line */
      "<LongRootElementNameThatWillCauseTheNextAllocationToExpandTheStr"
      "ingPoolForTheDTDQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
      " a='1'/>";
  int i;
  const int max_alloc_count = 20;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing reallocations");
  else if (i == max_alloc_count)
    fail("Parsing failed even at max reallocation count");
}
END_TEST

START_TEST(test_alloc_long_base) {
  const char *text = "<!DOCTYPE doc [\n"
                     "  <!ENTITY e SYSTEM 'foo'>\n"
                     "]>\n"
                     "<doc>&e;</doc>";
  char entity_text[] = "Hello world";
  const XML_Char *base =
      /* 64 characters per line */
      /* clang-format off */
        XCS("LongBaseURI/that/will/overflow/an/internal/buffer/and/cause/it/t")
        XCS("o/have/to/grow/PQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/");
  /* clang-format on */
  int i;
  const int max_alloc_count = 25;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetUserData(g_parser, entity_text);
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_alloc);
    if (XML_SetBase(g_parser, base) == XML_STATUS_ERROR) {
      XML_ParserReset(g_parser, NULL);
      continue;
    }
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing allocations");
  else if (i == max_alloc_count)
    fail("Parsing failed even at max allocation count");
}
END_TEST

START_TEST(test_alloc_long_public_id) {
  const char *text
      = "<!DOCTYPE doc [\n"
        "  <!ENTITY e PUBLIC '"
        /* 64 characters per line */
        "LongPublicIDThatShouldResultInAnInternalStringPoolGrowingAtASpec"
        "ificMomentKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "' 'bar'>\n"
        "]>\n"
        "<doc>&e;</doc>";
  char entity_text[] = "Hello world";
  int i;
  const int max_alloc_count = 40;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetUserData(g_parser, entity_text);
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_alloc);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing allocations");
  else if (i == max_alloc_count)
    fail("Parsing failed even at max allocation count");
}
END_TEST

START_TEST(test_alloc_long_entity_value) {
  const char *text
      = "<!DOCTYPE doc [\n"
        "  <!ENTITY e1 '"
        /* 64 characters per line */
        "Long entity value that should provoke a string pool to grow whil"
        "e setting up to parse the external entity below. xyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "'>\n"
        "  <!ENTITY e2 SYSTEM 'bar'>\n"
        "]>\n"
        "<doc>&e2;</doc>";
  char entity_text[] = "Hello world";
  int i;
  const int max_alloc_count = 40;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetUserData(g_parser, entity_text);
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_alloc);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing allocations");
  else if (i == max_alloc_count)
    fail("Parsing failed even at max allocation count");
}
END_TEST

START_TEST(test_alloc_long_notation) {
  const char *text
      = "<!DOCTYPE doc [\n"
        "  <!NOTATION note SYSTEM '"
        /* 64 characters per line */
        "ALongNotationNameThatShouldProvokeStringPoolGrowthWhileCallingAn"
        "ExternalEntityParserUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "'>\n"
        "  <!ENTITY e1 SYSTEM 'foo' NDATA "
        /* 64 characters per line */
        "ALongNotationNameThatShouldProvokeStringPoolGrowthWhileCallingAn"
        "ExternalEntityParserUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AB"
        ">\n"
        "  <!ENTITY e2 SYSTEM 'bar'>\n"
        "]>\n"
        "<doc>&e2;</doc>";
  ExtOption options[]
      = {{XCS("foo"), "Entity Foo"}, {XCS("bar"), "Entity Bar"}, {NULL, NULL}};
  int i;
  const int max_alloc_count = 40;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetUserData(g_parser, options);
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_optioner);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;

    /* See comment in test_alloc_parse_xdecl() */
    alloc_teardown();
    alloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing allocations");
  else if (i == max_alloc_count)
    fail("Parsing failed even at max allocation count");
}
END_TEST

static int XMLCALL
external_entity_parser_create_alloc_fail_handler(XML_Parser parser,
                                                 const XML_Char *context,
                                                 const XML_Char *base,
                                                 const XML_Char *systemId,
                                                 const XML_Char *publicId) {
  UNUSED_P(base);
  UNUSED_P(systemId);
  UNUSED_P(publicId);

  if (context != NULL)
    fail("Unexpected non-NULL context");

  // The following number intends to fail the upcoming allocation in line
  // "parser->m_protocolEncodingName = copyString(encodingName,
  // &(parser->m_mem));" in function parserInit.
  allocation_count = 3;

  const XML_Char *const encodingName = XCS("UTF-8"); // needs something non-NULL
  const XML_Parser ext_parser
      = XML_ExternalEntityParserCreate(parser, context, encodingName);
  if (ext_parser != NULL)
    fail(
        "Call to XML_ExternalEntityParserCreate was expected to fail out-of-memory");

  allocation_count = ALLOC_ALWAYS_SUCCEED;
  return XML_STATUS_ERROR;
}

START_TEST(test_alloc_reset_after_external_entity_parser_create_fail) {
  const char *const text = "<!DOCTYPE doc SYSTEM 'foo'><doc/>";

  XML_SetExternalEntityRefHandler(
      g_parser, external_entity_parser_create_alloc_fail_handler);
  XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);

  if (XML_Parse(g_parser, text, (int)strlen(text), XML_TRUE)
      != XML_STATUS_ERROR)
    fail("Call to parse was expected to fail");

  if (XML_GetErrorCode(g_parser) != XML_ERROR_EXTERNAL_ENTITY_HANDLING)
    fail("Call to parse was expected to fail from the external entity handler");

  XML_ParserReset(g_parser, NULL);
}
END_TEST

static void
nsalloc_setup(void) {
  XML_Memory_Handling_Suite memsuite = {duff_allocator, duff_reallocator, free};
  XML_Char ns_sep[2] = {' ', '\0'};

  /* Ensure the parser creation will go through */
  allocation_count = ALLOC_ALWAYS_SUCCEED;
  reallocation_count = REALLOC_ALWAYS_SUCCEED;
  g_parser = XML_ParserCreate_MM(NULL, &memsuite, ns_sep);
  if (g_parser == NULL)
    fail("Parser not created");
}

static void
nsalloc_teardown(void) {
  basic_teardown();
}

/* Test the effects of allocation failure in simple namespace parsing.
 * Based on test_ns_default_with_empty_uri()
 */
START_TEST(test_nsalloc_xmlns) {
  const char *text = "<doc xmlns='http://example.org/'>\n"
                     "  <e xmlns=''/>\n"
                     "</doc>";
  unsigned int i;
  const unsigned int max_alloc_count = 30;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    /* Exercise more code paths with a default handler */
    XML_SetDefaultHandler(g_parser, dummy_default_handler);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* Resetting the parser is insufficient, because some memory
     * allocations are cached within the parser.  Instead we use
     * the teardown and setup routines to ensure that we have the
     * right sort of parser back in our hands.
     */
    nsalloc_teardown();
    nsalloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing allocations");
  else if (i == max_alloc_count)
    fail("Parsing failed even at maximum allocation count");
}
END_TEST

/* Test XML_ParseBuffer interface with namespace and a dicky allocator */
START_TEST(test_nsalloc_parse_buffer) {
  const char *text = "<doc>Hello</doc>";
  void *buffer;

  /* Try a parse before the start of the world */
  /* (Exercises new code path) */
  if (XML_ParseBuffer(g_parser, 0, XML_FALSE) != XML_STATUS_ERROR)
    fail("Pre-init XML_ParseBuffer not faulted");
  if (XML_GetErrorCode(g_parser) != XML_ERROR_NO_BUFFER)
    fail("Pre-init XML_ParseBuffer faulted for wrong reason");

  buffer = XML_GetBuffer(g_parser, 1 /* any small number greater than 0 */);
  if (buffer == NULL)
    fail("Could not acquire parse buffer");

  allocation_count = 0;
  if (XML_ParseBuffer(g_parser, 0, XML_FALSE) != XML_STATUS_ERROR)
    fail("Pre-init XML_ParseBuffer not faulted");
  if (XML_GetErrorCode(g_parser) != XML_ERROR_NO_MEMORY)
    fail("Pre-init XML_ParseBuffer faulted for wrong reason");

  /* Now with actual memory allocation */
  allocation_count = ALLOC_ALWAYS_SUCCEED;
  if (XML_ParseBuffer(g_parser, 0, XML_FALSE) != XML_STATUS_OK)
    xml_failure(g_parser);

  /* Check that resuming an unsuspended parser is faulted */
  if (XML_ResumeParser(g_parser) != XML_STATUS_ERROR)
    fail("Resuming unsuspended parser not faulted");
  if (XML_GetErrorCode(g_parser) != XML_ERROR_NOT_SUSPENDED)
    xml_failure(g_parser);

  /* Get the parser into suspended state */
  XML_SetCharacterDataHandler(g_parser, clearing_aborting_character_handler);
  resumable = XML_TRUE;
  buffer = XML_GetBuffer(g_parser, (int)strlen(text));
  if (buffer == NULL)
    fail("Could not acquire parse buffer");
  assert(buffer != NULL);
  memcpy(buffer, text, strlen(text));
  if (XML_ParseBuffer(g_parser, (int)strlen(text), XML_TRUE)
      != XML_STATUS_SUSPENDED)
    xml_failure(g_parser);
  if (XML_GetErrorCode(g_parser) != XML_ERROR_NONE)
    xml_failure(g_parser);
  if (XML_ParseBuffer(g_parser, (int)strlen(text), XML_TRUE)
      != XML_STATUS_ERROR)
    fail("Suspended XML_ParseBuffer not faulted");
  if (XML_GetErrorCode(g_parser) != XML_ERROR_SUSPENDED)
    xml_failure(g_parser);
  if (XML_GetBuffer(g_parser, (int)strlen(text)) != NULL)
    fail("Suspended XML_GetBuffer not faulted");

  /* Get it going again and complete the world */
  XML_SetCharacterDataHandler(g_parser, NULL);
  if (XML_ResumeParser(g_parser) != XML_STATUS_OK)
    xml_failure(g_parser);
  if (XML_ParseBuffer(g_parser, (int)strlen(text), XML_TRUE)
      != XML_STATUS_ERROR)
    fail("Post-finishing XML_ParseBuffer not faulted");
  if (XML_GetErrorCode(g_parser) != XML_ERROR_FINISHED)
    xml_failure(g_parser);
  if (XML_GetBuffer(g_parser, (int)strlen(text)) != NULL)
    fail("Post-finishing XML_GetBuffer not faulted");
}
END_TEST

/* Check handling of long prefix names (pool growth) */
START_TEST(test_nsalloc_long_prefix) {
  const char *text
      = "<"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        ":foo xmlns:"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "='http://example.org/'>"
        "</"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        ":foo>";
  int i;
  const int max_alloc_count = 40;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_nsalloc_xmlns() */
    nsalloc_teardown();
    nsalloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing allocations");
  else if (i == max_alloc_count)
    fail("Parsing failed even at max allocation count");
}
END_TEST

/* Check handling of long uri names (pool growth) */
START_TEST(test_nsalloc_long_uri) {
  const char *text
      = "<foo:e xmlns:foo='http://example.org/"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "' bar:a='12'\n"
        "xmlns:bar='http://example.org/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A/"
        "'>"
        "</foo:e>";
  int i;
  const int max_alloc_count = 40;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_nsalloc_xmlns() */
    nsalloc_teardown();
    nsalloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing allocations");
  else if (i == max_alloc_count)
    fail("Parsing failed even at max allocation count");
}
END_TEST

/* Test handling of long attribute names with prefixes */
START_TEST(test_nsalloc_long_attr) {
  const char *text
      = "<foo:e xmlns:foo='http://example.org/' bar:"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "='12'\n"
        "xmlns:bar='http://example.org/'>"
        "</foo:e>";
  int i;
  const int max_alloc_count = 40;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_nsalloc_xmlns() */
    nsalloc_teardown();
    nsalloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing allocations");
  else if (i == max_alloc_count)
    fail("Parsing failed even at max allocation count");
}
END_TEST

/* Test handling of an attribute name with a long namespace prefix */
START_TEST(test_nsalloc_long_attr_prefix) {
  const char *text
      = "<foo:e xmlns:foo='http://example.org/' "
        /* 64 characters per line */
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        ":a='12'\n"
        "xmlns:"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "='http://example.org/'>"
        "</foo:e>";
  const XML_Char *elemstr[] = {
      /* clang-format off */
        XCS("http://example.org/ e foo"),
        XCS("http://example.org/ a ")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ")
        XCS("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ")
      /* clang-format on */
  };
  int i;
  const int max_alloc_count = 40;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetReturnNSTriplet(g_parser, XML_TRUE);
    XML_SetUserData(g_parser, (void *)elemstr);
    XML_SetElementHandler(g_parser, triplet_start_checker, triplet_end_checker);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_nsalloc_xmlns() */
    nsalloc_teardown();
    nsalloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing allocations");
  else if (i == max_alloc_count)
    fail("Parsing failed even at max allocation count");
}
END_TEST

/* Test attribute handling in the face of a dodgy reallocator */
START_TEST(test_nsalloc_realloc_attributes) {
  const char *text = "<foo:e xmlns:foo='http://example.org/' bar:a='12'\n"
                     "       xmlns:bar='http://example.org/'>"
                     "</foo:e>";
  int i;
  const int max_realloc_count = 10;

  for (i = 0; i < max_realloc_count; i++) {
    reallocation_count = i;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_nsalloc_xmlns() */
    nsalloc_teardown();
    nsalloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing reallocations");
  else if (i == max_realloc_count)
    fail("Parsing failed at max reallocation count");
}
END_TEST

/* Test long element names with namespaces under a failing allocator */
START_TEST(test_nsalloc_long_element) {
  const char *text
      = "<foo:thisisalongenoughelementnametotriggerareallocation\n"
        " xmlns:foo='http://example.org/' bar:a='12'\n"
        " xmlns:bar='http://example.org/'>"
        "</foo:thisisalongenoughelementnametotriggerareallocation>";
  const XML_Char *elemstr[]
      = {XCS("http://example.org/")
             XCS(" thisisalongenoughelementnametotriggerareallocation foo"),
         XCS("http://example.org/ a bar")};
  int i;
  const int max_alloc_count = 30;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetReturnNSTriplet(g_parser, XML_TRUE);
    XML_SetUserData(g_parser, (void *)elemstr);
    XML_SetElementHandler(g_parser, triplet_start_checker, triplet_end_checker);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_nsalloc_xmlns() */
    nsalloc_teardown();
    nsalloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing reallocations");
  else if (i == max_alloc_count)
    fail("Parsing failed at max reallocation count");
}
END_TEST

/* Test the effects of reallocation failure when reassigning a
 * binding.
 *
 * XML_ParserReset does not free the BINDING structures used by a
 * parser, but instead adds them to an internal free list to be reused
 * as necessary.  Likewise the URI buffers allocated for the binding
 * aren't freed, but kept attached to their existing binding.  If the
 * new binding has a longer URI, it will need reallocation.  This test
 * provokes that reallocation, and tests the control path if it fails.
 */
START_TEST(test_nsalloc_realloc_binding_uri) {
  const char *first = "<doc xmlns='http://example.org/'>\n"
                      "  <e xmlns='' />\n"
                      "</doc>";
  const char *second
      = "<doc xmlns='http://example.org/long/enough/URI/to/reallocate/'>\n"
        "  <e xmlns='' />\n"
        "</doc>";
  unsigned i;
  const unsigned max_realloc_count = 10;

  /* First, do a full parse that will leave bindings around */
  if (_XML_Parse_SINGLE_BYTES(g_parser, first, (int)strlen(first), XML_TRUE)
      == XML_STATUS_ERROR)
    xml_failure(g_parser);

  /* Now repeat with a longer URI and a duff reallocator */
  for (i = 0; i < max_realloc_count; i++) {
    XML_ParserReset(g_parser, NULL);
    reallocation_count = i;
    if (_XML_Parse_SINGLE_BYTES(g_parser, second, (int)strlen(second), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
  }
  if (i == 0)
    fail("Parsing worked despite failing reallocation");
  else if (i == max_realloc_count)
    fail("Parsing failed at max reallocation count");
}
END_TEST

/* Check handling of long prefix names (pool growth) */
START_TEST(test_nsalloc_realloc_long_prefix) {
  const char *text
      = "<"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        ":foo xmlns:"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "='http://example.org/'>"
        "</"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        ":foo>";
  int i;
  const int max_realloc_count = 12;

  for (i = 0; i < max_realloc_count; i++) {
    reallocation_count = i;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_nsalloc_xmlns() */
    nsalloc_teardown();
    nsalloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing reallocations");
  else if (i == max_realloc_count)
    fail("Parsing failed even at max reallocation count");
}
END_TEST

/* Check handling of even long prefix names (different code path) */
START_TEST(test_nsalloc_realloc_longer_prefix) {
  const char *text
      = "<"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "Q:foo xmlns:"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "Q='http://example.org/'>"
        "</"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "Q:foo>";
  int i;
  const int max_realloc_count = 12;

  for (i = 0; i < max_realloc_count; i++) {
    reallocation_count = i;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_nsalloc_xmlns() */
    nsalloc_teardown();
    nsalloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing reallocations");
  else if (i == max_realloc_count)
    fail("Parsing failed even at max reallocation count");
}
END_TEST

START_TEST(test_nsalloc_long_namespace) {
  const char *text1
      = "<"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        ":e xmlns:"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "='http://example.org/'>\n";
  const char *text2
      = "<"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        ":f "
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        ":attr='foo'/>\n"
        "</"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        ":e>";
  int i;
  const int max_alloc_count = 40;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text1, (int)strlen(text1), XML_FALSE)
            != XML_STATUS_ERROR
        && _XML_Parse_SINGLE_BYTES(g_parser, text2, (int)strlen(text2),
                                   XML_TRUE)
               != XML_STATUS_ERROR)
      break;
    /* See comment in test_nsalloc_xmlns() */
    nsalloc_teardown();
    nsalloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing allocations");
  else if (i == max_alloc_count)
    fail("Parsing failed even at max allocation count");
}
END_TEST

/* Using a slightly shorter namespace name provokes allocations in
 * slightly different places in the code.
 */
START_TEST(test_nsalloc_less_long_namespace) {
  const char *text
      = "<"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz012345678"
        ":e xmlns:"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz012345678"
        "='http://example.org/'>\n"
        "<"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz012345678"
        ":f "
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz012345678"
        ":att='foo'/>\n"
        "</"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789AZ"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz012345678"
        ":e>";
  int i;
  const int max_alloc_count = 40;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_nsalloc_xmlns() */
    nsalloc_teardown();
    nsalloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing allocations");
  else if (i == max_alloc_count)
    fail("Parsing failed even at max allocation count");
}
END_TEST

START_TEST(test_nsalloc_long_context) {
  const char *text
      = "<!DOCTYPE doc SYSTEM 'foo' [\n"
        "  <!ATTLIST doc baz ID #REQUIRED>\n"
        "  <!ENTITY en SYSTEM 'bar'>\n"
        "]>\n"
        "<doc xmlns='http://example.org/"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKL"
        "' baz='2'>\n"
        "&en;"
        "</doc>";
  ExtOption options[] = {
      {XCS("foo"), "<!ELEMENT e EMPTY>"}, {XCS("bar"), "<e/>"}, {NULL, NULL}};
  int i;
  const int max_alloc_count = 70;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetUserData(g_parser, options);
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_optioner);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;

    /* See comment in test_nsalloc_xmlns() */
    nsalloc_teardown();
    nsalloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing allocations");
  else if (i == max_alloc_count)
    fail("Parsing failed even at max allocation count");
}
END_TEST

/* This function is void; it will throw a fail() on error, so if it
 * returns normally it must have succeeded.
 */
static void
context_realloc_test(const char *text) {
  ExtOption options[] = {
      {XCS("foo"), "<!ELEMENT e EMPTY>"}, {XCS("bar"), "<e/>"}, {NULL, NULL}};
  int i;
  const int max_realloc_count = 6;

  for (i = 0; i < max_realloc_count; i++) {
    reallocation_count = i;
    XML_SetUserData(g_parser, options);
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_optioner);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_nsalloc_xmlns() */
    nsalloc_teardown();
    nsalloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing reallocations");
  else if (i == max_realloc_count)
    fail("Parsing failed even at max reallocation count");
}

START_TEST(test_nsalloc_realloc_long_context) {
  const char *text
      = "<!DOCTYPE doc SYSTEM 'foo' [\n"
        "  <!ENTITY en SYSTEM 'bar'>\n"
        "]>\n"
        "<doc xmlns='http://example.org/"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKL"
        "'>\n"
        "&en;"
        "</doc>";

  context_realloc_test(text);
}
END_TEST

START_TEST(test_nsalloc_realloc_long_context_2) {
  const char *text
      = "<!DOCTYPE doc SYSTEM 'foo' [\n"
        "  <!ENTITY en SYSTEM 'bar'>\n"
        "]>\n"
        "<doc xmlns='http://example.org/"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJK"
        "'>\n"
        "&en;"
        "</doc>";

  context_realloc_test(text);
}
END_TEST

START_TEST(test_nsalloc_realloc_long_context_3) {
  const char *text
      = "<!DOCTYPE doc SYSTEM 'foo' [\n"
        "  <!ENTITY en SYSTEM 'bar'>\n"
        "]>\n"
        "<doc xmlns='http://example.org/"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGH"
        "'>\n"
        "&en;"
        "</doc>";

  context_realloc_test(text);
}
END_TEST

START_TEST(test_nsalloc_realloc_long_context_4) {
  const char *text
      = "<!DOCTYPE doc SYSTEM 'foo' [\n"
        "  <!ENTITY en SYSTEM 'bar'>\n"
        "]>\n"
        "<doc xmlns='http://example.org/"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO"
        "'>\n"
        "&en;"
        "</doc>";

  context_realloc_test(text);
}
END_TEST

START_TEST(test_nsalloc_realloc_long_context_5) {
  const char *text
      = "<!DOCTYPE doc SYSTEM 'foo' [\n"
        "  <!ENTITY en SYSTEM 'bar'>\n"
        "]>\n"
        "<doc xmlns='http://example.org/"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABC"
        "'>\n"
        "&en;"
        "</doc>";

  context_realloc_test(text);
}
END_TEST

START_TEST(test_nsalloc_realloc_long_context_6) {
  const char *text
      = "<!DOCTYPE doc SYSTEM 'foo' [\n"
        "  <!ENTITY en SYSTEM 'bar'>\n"
        "]>\n"
        "<doc xmlns='http://example.org/"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNOP"
        "'>\n"
        "&en;"
        "</doc>";

  context_realloc_test(text);
}
END_TEST

START_TEST(test_nsalloc_realloc_long_context_7) {
  const char *text
      = "<!DOCTYPE doc SYSTEM 'foo' [\n"
        "  <!ENTITY en SYSTEM 'bar'>\n"
        "]>\n"
        "<doc xmlns='http://example.org/"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLM"
        "'>\n"
        "&en;"
        "</doc>";

  context_realloc_test(text);
}
END_TEST

START_TEST(test_nsalloc_realloc_long_ge_name) {
  const char *text
      = "<!DOCTYPE doc SYSTEM 'foo' [\n"
        "  <!ENTITY "
        /* 64 characters per line */
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        " SYSTEM 'bar'>\n"
        "]>\n"
        "<doc xmlns='http://example.org/baz'>\n"
        "&"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        ";"
        "</doc>";
  ExtOption options[] = {
      {XCS("foo"), "<!ELEMENT el EMPTY>"}, {XCS("bar"), "<el/>"}, {NULL, NULL}};
  int i;
  const int max_realloc_count = 10;

  for (i = 0; i < max_realloc_count; i++) {
    reallocation_count = i;
    XML_SetUserData(g_parser, options);
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_optioner);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;
    /* See comment in test_nsalloc_xmlns() */
    nsalloc_teardown();
    nsalloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing reallocations");
  else if (i == max_realloc_count)
    fail("Parsing failed even at max reallocation count");
}
END_TEST

/* Test that when a namespace is passed through the context mechanism
 * to an external entity parser, the parsers handle reallocation
 * failures correctly.  The prefix is exactly the right length to
 * provoke particular uncommon code paths.
 */
START_TEST(test_nsalloc_realloc_long_context_in_dtd) {
  const char *text1
      = "<!DOCTYPE "
        /* 64 characters per line */
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        ":doc [\n"
        "  <!ENTITY First SYSTEM 'foo/First'>\n"
        "]>\n"
        "<"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        ":doc xmlns:"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "='foo/Second'>&First;";
  const char *text2
      = "</"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        ":doc>";
  ExtOption options[] = {{XCS("foo/First"), "Hello world"}, {NULL, NULL}};
  int i;
  const int max_realloc_count = 20;

  for (i = 0; i < max_realloc_count; i++) {
    reallocation_count = i;
    XML_SetUserData(g_parser, options);
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_optioner);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text1, (int)strlen(text1), XML_FALSE)
            != XML_STATUS_ERROR
        && _XML_Parse_SINGLE_BYTES(g_parser, text2, (int)strlen(text2),
                                   XML_TRUE)
               != XML_STATUS_ERROR)
      break;
    /* See comment in test_nsalloc_xmlns() */
    nsalloc_teardown();
    nsalloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing reallocations");
  else if (i == max_realloc_count)
    fail("Parsing failed even at max reallocation count");
}
END_TEST

START_TEST(test_nsalloc_long_default_in_ext) {
  const char *text
      = "<!DOCTYPE doc [\n"
        "  <!ATTLIST e a1 CDATA '"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "ABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOPABCDEFGHIJKLMNOP"
        "'>\n"
        "  <!ENTITY x SYSTEM 'foo'>\n"
        "]>\n"
        "<doc>&x;</doc>";
  ExtOption options[] = {{XCS("foo"), "<e/>"}, {NULL, NULL}};
  int i;
  const int max_alloc_count = 50;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetUserData(g_parser, options);
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_optioner);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;

    /* See comment in test_nsalloc_xmlns() */
    nsalloc_teardown();
    nsalloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing allocations");
  else if (i == max_alloc_count)
    fail("Parsing failed even at max allocation count");
}
END_TEST

START_TEST(test_nsalloc_long_systemid_in_ext) {
  const char *text
      = "<!DOCTYPE doc SYSTEM 'foo' [\n"
        "  <!ENTITY en SYSTEM '"
        /* 64 characters per line */
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"
        "'>\n"
        "]>\n"
        "<doc>&en;</doc>";
  ExtOption options[] = {
      {XCS("foo"), "<!ELEMENT e EMPTY>"},
      {/* clang-format off */
            XCS("ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/")
            XCS("ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/")
            XCS("ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/")
            XCS("ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/")
            XCS("ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/")
            XCS("ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/")
            XCS("ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/")
            XCS("ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/")
            XCS("ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/")
            XCS("ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/")
            XCS("ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/")
            XCS("ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/")
            XCS("ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/")
            XCS("ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/")
            XCS("ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/")
            XCS("ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/ABCDEFGHIJKLMNO/"),
       /* clang-format on */
       "<e/>"},
      {NULL, NULL}};
  int i;
  const int max_alloc_count = 55;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetUserData(g_parser, options);
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_optioner);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;

    /* See comment in test_nsalloc_xmlns() */
    nsalloc_teardown();
    nsalloc_setup();
  }
  if (i == 0)
    fail("Parsing worked despite failing allocations");
  else if (i == max_alloc_count)
    fail("Parsing failed even at max allocation count");
}
END_TEST

/* Test the effects of allocation failure on parsing an element in a
 * namespace.  Based on test_nsalloc_long_context.
 */
START_TEST(test_nsalloc_prefixed_element) {
  const char *text = "<!DOCTYPE pfx:element SYSTEM 'foo' [\n"
                     "  <!ATTLIST pfx:element baz ID #REQUIRED>\n"
                     "  <!ENTITY en SYSTEM 'bar'>\n"
                     "]>\n"
                     "<pfx:element xmlns:pfx='http://example.org/' baz='2'>\n"
                     "&en;"
                     "</pfx:element>";
  ExtOption options[] = {
      {XCS("foo"), "<!ELEMENT e EMPTY>"}, {XCS("bar"), "<e/>"}, {NULL, NULL}};
  int i;
  const int max_alloc_count = 70;

  for (i = 0; i < max_alloc_count; i++) {
    allocation_count = i;
    XML_SetUserData(g_parser, options);
    XML_SetParamEntityParsing(g_parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
    XML_SetExternalEntityRefHandler(g_parser, external_entity_optioner);
    if (_XML_Parse_SINGLE_BYTES(g_parser, text, (int)strlen(text), XML_TRUE)
        != XML_STATUS_ERROR)
      break;

    /* See comment in test_nsalloc_xmlns() */
    nsalloc_teardown();
    nsalloc_setup();
  }
  if (i == 0)
    fail("Success despite failing allocator");
  else if (i == max_alloc_count)
    fail("Failed even at full allocation count");
}
END_TEST

#if defined(XML_DTD)
typedef enum XML_Status (*XmlParseFunction)(XML_Parser, const char *, int, int);

struct AccountingTestCase {
  const char *primaryText;
  const char *firstExternalText;  /* often NULL */
  const char *secondExternalText; /* often NULL */
  const unsigned long long expectedCountBytesIndirectExtra;
  XML_Bool singleBytesWanted;
};

static int
accounting_external_entity_ref_handler(XML_Parser parser,
                                       const XML_Char *context,
                                       const XML_Char *base,
                                       const XML_Char *systemId,
                                       const XML_Char *publicId) {
  UNUSED_P(context);
  UNUSED_P(base);
  UNUSED_P(publicId);

  const struct AccountingTestCase *const testCase
      = (const struct AccountingTestCase *)XML_GetUserData(parser);

  const char *externalText = NULL;
  if (xcstrcmp(systemId, XCS("first.ent")) == 0) {
    externalText = testCase->firstExternalText;
  } else if (xcstrcmp(systemId, XCS("second.ent")) == 0) {
    externalText = testCase->secondExternalText;
  } else {
    assert(! "systemId is neither \"first.ent\" nor \"second.ent\"");
  }
  assert(externalText);

  XML_Parser entParser = XML_ExternalEntityParserCreate(parser, context, 0);
  assert(entParser);

  const XmlParseFunction xmlParseFunction
      = testCase->singleBytesWanted ? _XML_Parse_SINGLE_BYTES : XML_Parse;

  const enum XML_Status status = xmlParseFunction(
      entParser, externalText, (int)strlen(externalText), XML_TRUE);

  XML_ParserFree(entParser);
  return status;
}

START_TEST(test_accounting_precision) {
  const XML_Bool filled_later = XML_TRUE; /* value is arbitrary */
  struct AccountingTestCase cases[] = {
      {"<e/>", NULL, NULL, 0, 0},
      {"<e></e>", NULL, NULL, 0, 0},

      /* Attributes */
      {"<e k1=\"v2\" k2=\"v2\"/>", NULL, NULL, 0, filled_later},
      {"<e k1=\"v2\" k2=\"v2\"></e>", NULL, NULL, 0, 0},
      {"<p:e xmlns:p=\"https://domain.invalid/\" />", NULL, NULL, 0,
       filled_later},
      {"<e k=\"&amp;&apos;&gt;&lt;&quot;\" />", NULL, NULL,
       sizeof(XML_Char) * 5 /* number of predefined entities */, filled_later},
      {"<e1 xmlns='https://example.org/'>\n"
       "  <e2 xmlns=''/>\n"
       "</e1>",
       NULL, NULL, 0, filled_later},

      /* Text */
      {"<e>text</e>", NULL, NULL, 0, filled_later},
      {"<e1><e2>text1<e3/>text2</e2></e1>", NULL, NULL, 0, filled_later},
      {"<e>&amp;&apos;&gt;&lt;&quot;</e>", NULL, NULL,
       sizeof(XML_Char) * 5 /* number of predefined entities */, filled_later},
      {"<e>&#65;&#41;</e>", NULL, NULL, 0, filled_later},

      /* Prolog */
      {"<?xml version=\"1.0\"?><root/>", NULL, NULL, 0, filled_later},

      /* Whitespace */
      {"  <e1>  <e2>  </e2>  </e1>  ", NULL, NULL, 0, filled_later},
      {"<e1  ><e2  /></e1  >", NULL, NULL, 0, filled_later},
      {"<e1><e2 k = \"v\"/><e3 k = 'v'/></e1>", NULL, NULL, 0, filled_later},

      /* Comments */
      {"<!-- Comment --><e><!-- Comment --></e>", NULL, NULL, 0, filled_later},

      /* Processing instructions */
      {"<?xml-stylesheet type=\"text/xsl\" href=\"https://domain.invalid/\" media=\"all\"?><e/>",
       NULL, NULL, 0, filled_later},
      {"<?pi0?><?pi1 ?><?pi2  ?><!DOCTYPE r SYSTEM 'first.ent'><r/>",
       "<?pi3?><!ENTITY % e1 SYSTEM 'second.ent'><?pi4?>%e1;<?pi5?>", "<?pi6?>",
       0, filled_later},

      /* CDATA */
      {"<e><![CDATA[one two three]]></e>", NULL, NULL, 0, filled_later},
      /* The following is the essence of this OSS-Fuzz finding:
         https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=34302
         https://oss-fuzz.com/testcase-detail/4860575394955264
      */
      {"<!DOCTYPE r [\n"
       "<!ENTITY e \"111<![CDATA[2 <= 2]]>333\">\n"
       "]>\n"
       "<r>&e;</r>\n",
       NULL, NULL, sizeof(XML_Char) * strlen("111<![CDATA[2 <= 2]]>333"),
       filled_later},

      /* Conditional sections */
      {"<!DOCTYPE r [\n"
       "<!ENTITY % draft 'INCLUDE'>\n"
       "<!ENTITY % final 'IGNORE'>\n"
       "<!ENTITY % import SYSTEM \"first.ent\">\n"
       "%import;\n"
       "]>\n"
       "<r/>\n",
       "<![%draft;[<!--1-->]]>\n"
       "<![%final;[<!--22-->]]>",
       NULL, sizeof(XML_Char) * (strlen("INCLUDE") + strlen("IGNORE")),
       filled_later},

      /* General entities */
      {"<!DOCTYPE root [\n"
       "<!ENTITY nine \"123456789\">\n"
       "]>\n"
       "<root>&nine;</root>",
       NULL, NULL, sizeof(XML_Char) * strlen("123456789"), filled_later},
      {"<!DOCTYPE root [\n"
       "<!ENTITY nine \"123456789\">\n"
       "]>\n"
       "<root k1=\"&nine;\"/>",
       NULL, NULL, sizeof(XML_Char) * strlen("123456789"), filled_later},
      {"<!DOCTYPE root [\n"
       "<!ENTITY nine \"123456789\">\n"
       "<!ENTITY nine2 \"&nine;&nine;\">\n"
       "]>\n"
       "<root>&nine2;&nine2;&nine2;</root>",
       NULL, NULL,
       sizeof(XML_Char) * 3 /* calls to &nine2; */ * 2 /* calls to &nine; */
           * (strlen("&nine;") + strlen("123456789")),
       filled_later},
      {"<!DOCTYPE r [\n"
       "  <!ENTITY five SYSTEM 'first.ent'>\n"
       "]>\n"
       "<r>&five;</r>",
       "12345", NULL, 0, filled_later},

      /* Parameter entities */
      {"<!DOCTYPE r [\n"
       "<!ENTITY % comment \"<!---->\">\n"
       "%comment;\n"
       "]>\n"
       "<r/>",
       NULL, NULL, sizeof(XML_Char) * strlen("<!---->"), filled_later},
      {"<!DOCTYPE r [\n"
       "<!ENTITY % ninedef \"&#60;!ENTITY nine &#34;123456789&#34;&#62;\">\n"
       "%ninedef;\n"
       "]>\n"
       "<r>&nine;</r>",
       NULL, NULL,
       sizeof(XML_Char)
           * (strlen("<!ENTITY nine \"123456789\">") + strlen("123456789")),
       filled_later},
      {"<!DOCTYPE r [\n"
       "<!ENTITY % comment \"<!--1-->\">\n"
       "<!ENTITY % comment2 \"&#37;comment;<!--22-->&#37;comment;\">\n"
       "%comment2;\n"
       "]>\n"
       "<r/>\n",
       NULL, NULL,
       sizeof(XML_Char)
           * (strlen("%comment;<!--22-->%comment;") + 2 * strlen("<!--1-->")),
       filled_later},
      {"<!DOCTYPE r [\n"
       "  <!ENTITY % five \"12345\">\n"
       "  <!ENTITY % five2def \"&#60;!ENTITY five2 &#34;[&#37;five;][&#37;five;]]]]&#34;&#62;\">\n"
       "  %five2def;\n"
       "]>\n"
       "<r>&five2;</r>",
       NULL, NULL, /* from "%five2def;": */
       sizeof(XML_Char)
           * (strlen("<!ENTITY five2 \"[%five;][%five;]]]]\">")
              + 2 /* calls to "%five;" */ * strlen("12345")
              + /* from "&five2;": */ strlen("[12345][12345]]]]")),
       filled_later},
      {"<!DOCTYPE r SYSTEM \"first.ent\">\n"
       "<r/>",
       "<!ENTITY % comment '<!--1-->'>\n"
       "<!ENTITY % comment2 '<!--22-->%comment;<!--22-->%comment;<!--22-->'>\n"
       "%comment2;",
       NULL,
       sizeof(XML_Char)
           * (strlen("<!--22-->%comment;<!--22-->%comment;<!--22-->")
              + 2 /* calls to "%comment;" */ * strlen("<!---->")),
       filled_later},
      {"<!DOCTYPE r SYSTEM 'first.ent'>\n"
       "<r/>",
       "<!ENTITY % e1 PUBLIC 'foo' 'second.ent'>\n"
       "<!ENTITY % e2 '<!--22-->%e1;<!--22-->'>\n"
       "%e2;\n",
       "<!--1-->", sizeof(XML_Char) * strlen("<!--22--><!--1--><!--22-->"),
       filled_later},
      {
          "<!DOCTYPE r SYSTEM 'first.ent'>\n"
          "<r/>",
          "<!ENTITY % e1 SYSTEM 'second.ent'>\n"
          "<!ENTITY % e2 '%e1;'>",
          "<?xml version='1.0' encoding='utf-8'?>\n"
          "hello\n"
          "xml" /* without trailing newline! */,
          0,
          filled_later,
      },
      {
          "<!DOCTYPE r SYSTEM 'first.ent'>\n"
          "<r/>",
          "<!ENTITY % e1 SYSTEM 'second.ent'>\n"
          "<!ENTITY % e2 '%e1;'>",
          "<?xml version='1.0' encoding='utf-8'?>\n"
          "hello\n"
          "xml\n" /* with trailing newline! */,
          0,
          filled_later,
      },
      {"<!DOCTYPE doc SYSTEM 'first.ent'>\n"
       "<doc></doc>\n",
       "<!ELEMENT doc EMPTY>\n"
       "<!ENTITY % e1 SYSTEM 'second.ent'>\n"
       "<!ENTITY % e2 '%e1;'>\n"
       "%e1;\n",
       "\xEF\xBB\xBF<!ATTLIST doc a1 CDATA 'value'>" /* UTF-8 BOM */,
       strlen("\xEF\xBB\xBF<!ATTLIST doc a1 CDATA 'value'>"), filled_later},
      {"<!DOCTYPE r [\n"
       "  <!ENTITY five SYSTEM 'first.ent'>\n"
       "]>\n"
       "<r>&five;</r>",
       "\xEF\xBB\xBF" /* UTF-8 BOM */, NULL, 0, filled_later},
  };

  const size_t countCases = sizeof(cases) / sizeof(cases[0]);
  size_t u = 0;
  for (; u < countCases; u++) {
    size_t v = 0;
    for (; v < 2; v++) {
      const XML_Bool singleBytesWanted = (v == 0) ? XML_FALSE : XML_TRUE;
      const unsigned long long expectedCountBytesDirect
          = strlen(cases[u].primaryText);
      const unsigned long long expectedCountBytesIndirect
          = (cases[u].firstExternalText ? strlen(cases[u].firstExternalText)
                                        : 0)
            + (cases[u].secondExternalText ? strlen(cases[u].secondExternalText)
                                           : 0)
            + cases[u].expectedCountBytesIndirectExtra;

      XML_Parser parser = XML_ParserCreate(NULL);
      XML_SetParamEntityParsing(parser, XML_PARAM_ENTITY_PARSING_ALWAYS);
      if (cases[u].firstExternalText) {
        XML_SetExternalEntityRefHandler(parser,
                                        accounting_external_entity_ref_handler);
        XML_SetUserData(parser, (void *)&cases[u]);
        cases[u].singleBytesWanted = singleBytesWanted;
      }

      const XmlParseFunction xmlParseFunction
          = singleBytesWanted ? _XML_Parse_SINGLE_BYTES : XML_Parse;

      enum XML_Status status
          = xmlParseFunction(parser, cases[u].primaryText,
                             (int)strlen(cases[u].primaryText), XML_TRUE);
      if (status != XML_STATUS_OK) {
        _xml_failure(parser, __FILE__, __LINE__);
      }

      const unsigned long long actualCountBytesDirect
          = testingAccountingGetCountBytesDirect(parser);
      const unsigned long long actualCountBytesIndirect
          = testingAccountingGetCountBytesIndirect(parser);

      XML_ParserFree(parser);

      if (actualCountBytesDirect != expectedCountBytesDirect) {
        fprintf(
            stderr,
            "Document " EXPAT_FMT_SIZE_T("") " of " EXPAT_FMT_SIZE_T("") ", %s: Expected " EXPAT_FMT_ULL(
                "") " count direct bytes, got " EXPAT_FMT_ULL("") " instead.\n",
            u + 1, countCases, singleBytesWanted ? "single bytes" : "chunks",
            expectedCountBytesDirect, actualCountBytesDirect);
        fail("Count of direct bytes is off");
      }

      if (actualCountBytesIndirect != expectedCountBytesIndirect) {
        fprintf(
            stderr,
            "Document " EXPAT_FMT_SIZE_T("") " of " EXPAT_FMT_SIZE_T("") ", %s: Expected " EXPAT_FMT_ULL(
                "") " count indirect bytes, got " EXPAT_FMT_ULL("") " instead.\n",
            u + 1, countCases, singleBytesWanted ? "single bytes" : "chunks",
            expectedCountBytesIndirect, actualCountBytesIndirect);
        fail("Count of indirect bytes is off");
      }
    }
  }
}
END_TEST

static float
portableNAN(void) {
  return strtof("nan", NULL);
}

static float
portableINFINITY(void) {
  return strtof("infinity", NULL);
}

START_TEST(test_billion_laughs_attack_protection_api) {
  XML_Parser parserWithoutParent = XML_ParserCreate(NULL);
  XML_Parser parserWithParent
      = XML_ExternalEntityParserCreate(parserWithoutParent, NULL, NULL);
  if (parserWithoutParent == NULL)
    fail("parserWithoutParent is NULL");
  if (parserWithParent == NULL)
    fail("parserWithParent is NULL");

  // XML_SetBillionLaughsAttackProtectionMaximumAmplification, error cases
  if (XML_SetBillionLaughsAttackProtectionMaximumAmplification(NULL, 123.0f)
      == XML_TRUE)
    fail("Call with NULL parser is NOT supposed to succeed");
  if (XML_SetBillionLaughsAttackProtectionMaximumAmplification(parserWithParent,
                                                               123.0f)
      == XML_TRUE)
    fail("Call with non-root parser is NOT supposed to succeed");
  if (XML_SetBillionLaughsAttackProtectionMaximumAmplification(
          parserWithoutParent, portableNAN())
      == XML_TRUE)
    fail("Call with NaN limit is NOT supposed to succeed");
  if (XML_SetBillionLaughsAttackProtectionMaximumAmplification(
          parserWithoutParent, -1.0f)
      == XML_TRUE)
    fail("Call with negative limit is NOT supposed to succeed");
  if (XML_SetBillionLaughsAttackProtectionMaximumAmplification(
          parserWithoutParent, 0.9f)
      == XML_TRUE)
    fail("Call with positive limit <1.0 is NOT supposed to succeed");

  // XML_SetBillionLaughsAttackProtectionMaximumAmplification, success cases
  if (XML_SetBillionLaughsAttackProtectionMaximumAmplification(
          parserWithoutParent, 1.0f)
      == XML_FALSE)
    fail("Call with positive limit >=1.0 is supposed to succeed");
  if (XML_SetBillionLaughsAttackProtectionMaximumAmplification(
          parserWithoutParent, 123456.789f)
      == XML_FALSE)
    fail("Call with positive limit >=1.0 is supposed to succeed");
  if (XML_SetBillionLaughsAttackProtectionMaximumAmplification(
          parserWithoutParent, portableINFINITY())
      == XML_FALSE)
    fail("Call with positive limit >=1.0 is supposed to succeed");

  // XML_SetBillionLaughsAttackProtectionActivationThreshold, error cases
  if (XML_SetBillionLaughsAttackProtectionActivationThreshold(NULL, 123)
      == XML_TRUE)
    fail("Call with NULL parser is NOT supposed to succeed");
  if (XML_SetBillionLaughsAttackProtectionActivationThreshold(parserWithParent,
                                                              123)
      == XML_TRUE)
    fail("Call with non-root parser is NOT supposed to succeed");

  // XML_SetBillionLaughsAttackProtectionActivationThreshold, success cases
  if (XML_SetBillionLaughsAttackProtectionActivationThreshold(
          parserWithoutParent, 123)
      == XML_FALSE)
    fail("Call with non-NULL parentless parser is supposed to succeed");

  XML_ParserFree(parserWithParent);
  XML_ParserFree(parserWithoutParent);
}
END_TEST

START_TEST(test_helper_unsigned_char_to_printable) {
  // Smoke test
  unsigned char uc = 0;
  for (; uc < (unsigned char)-1; uc++) {
    const char *const printable = unsignedCharToPrintable(uc);
    if (printable == NULL)
      fail("unsignedCharToPrintable returned NULL");
    if (strlen(printable) < (size_t)1)
      fail("unsignedCharToPrintable returned empty string");
  }

  // Two concrete samples
  if (strcmp(unsignedCharToPrintable('A'), "A") != 0)
    fail("unsignedCharToPrintable result mistaken");
  if (strcmp(unsignedCharToPrintable('\\'), "\\\\") != 0)
    fail("unsignedCharToPrintable result mistaken");
}
END_TEST
#endif // defined(XML_DTD)

static Suite *
make_suite(void) {
  Suite *s = suite_create("basic");
  TCase *tc_basic = tcase_create("basic tests");
  TCase *tc_namespace = tcase_create("XML namespaces");
  TCase *tc_misc = tcase_create("miscellaneous tests");
  TCase *tc_alloc = tcase_create("allocation tests");
  TCase *tc_nsalloc = tcase_create("namespace allocation tests");
#if defined(XML_DTD)
  TCase *tc_accounting = tcase_create("accounting tests");
#endif

  suite_add_tcase(s, tc_basic);
  tcase_add_checked_fixture(tc_basic, basic_setup, basic_teardown);
  tcase_add_test(tc_basic, test_nul_byte);
  tcase_add_test(tc_basic, test_u0000_char);
  tcase_add_test(tc_basic, test_siphash_self);
  tcase_add_test(tc_basic, test_siphash_spec);
  tcase_add_test(tc_basic, test_bom_utf8);
  tcase_add_test(tc_basic, test_bom_utf16_be);
  tcase_add_test(tc_basic, test_bom_utf16_le);
  tcase_add_test(tc_basic, test_nobom_utf16_le);
  tcase_add_test(tc_basic, test_illegal_utf8);
  tcase_add_test(tc_basic, test_utf8_auto_align);
  tcase_add_test(tc_basic, test_utf16);
  tcase_add_test(tc_basic, test_utf16_le_epilog_newline);
  tcase_add_test(tc_basic, test_not_utf16);
  tcase_add_test(tc_basic, test_bad_encoding);
  tcase_add_test(tc_basic, test_latin1_umlauts);
  tcase_add_test(tc_basic, test_long_utf8_character);
  tcase_add_test(tc_basic, test_long_latin1_attribute);
  tcase_add_test(tc_basic, test_long_ascii_attribute);
  /* Regression test for SF bug #491986. */
  tcase_add_test(tc_basic, test_danish_latin1);
  /* Regression test for SF bug #514281. */
  tcase_add_test(tc_basic, test_french_charref_hexidecimal);
  tcase_add_test(tc_basic, test_french_charref_decimal);
  tcase_add_test(tc_basic, test_french_latin1);
  tcase_add_test(tc_basic, test_french_utf8);
  tcase_add_test(tc_basic, test_utf8_false_rejection);
  tcase_add_test(tc_basic, test_line_number_after_parse);
  tcase_add_test(tc_basic, test_column_number_after_parse);
  tcase_add_test(tc_basic, test_line_and_column_numbers_inside_handlers);
  tcase_add_test(tc_basic, test_line_number_after_error);
  tcase_add_test(tc_basic, test_column_number_after_error);
  tcase_add_test(tc_basic, test_really_long_lines);
  tcase_add_test(tc_basic, test_really_long_encoded_lines);
  tcase_add_test(tc_basic, test_end_element_events);
  tcase_add_test(tc_basic, test_attr_whitespace_normalization);
  tcase_add_test(tc_basic, test_xmldecl_misplaced);
  tcase_add_test(tc_basic, test_xmldecl_invalid);
  tcase_add_test(tc_basic, test_xmldecl_missing_attr);
  tcase_add_test(tc_basic, test_xmldecl_missing_value);
  tcase_add_test(tc_basic, test_unknown_encoding_internal_entity);
  tcase_add_test(tc_basic, test_unrecognised_encoding_internal_entity);
  tcase_add_test(tc_basic, test_wfc_undeclared_entity_unread_external_subset);
  tcase_add_test(tc_basic, test_wfc_undeclared_entity_no_external_subset);
  tcase_add_test(tc_basic, test_wfc_undeclared_entity_standalone);
  tcase_add_test(tc_basic, test_wfc_undeclared_entity_with_external_subset);
  tcase_add_test(tc_basic, test_not_standalone_handler_reject);
  tcase_add_test(tc_basic, test_not_standalone_handler_accept);
  tcase_add_test(tc_basic,
                 test_wfc_undeclared_entity_with_external_subset_standalone);
  tcase_add_test(tc_basic, test_entity_with_external_subset_unless_standalone);
  tcase_add_test(tc_basic, test_wfc_no_recursive_entity_refs);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_ext_entity_set_encoding);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_ext_entity_no_handler);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_ext_entity_set_bom);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_ext_entity_bad_encoding);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_ext_entity_bad_encoding_2);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_ext_entity_invalid_parse);
  tcase_add_test__ifdef_xml_dtd(tc_basic,
                                test_ext_entity_invalid_suspended_parse);
  tcase_add_test(tc_basic, test_dtd_default_handling);
  tcase_add_test(tc_basic, test_dtd_attr_handling);
  tcase_add_test(tc_basic, test_empty_ns_without_namespaces);
  tcase_add_test(tc_basic, test_ns_in_attribute_default_without_namespaces);
  tcase_add_test(tc_basic, test_stop_parser_between_char_data_calls);
  tcase_add_test(tc_basic, test_suspend_parser_between_char_data_calls);
  tcase_add_test(tc_basic, test_repeated_stop_parser_between_char_data_calls);
  tcase_add_test(tc_basic, test_good_cdata_ascii);
  tcase_add_test(tc_basic, test_good_cdata_utf16);
  tcase_add_test(tc_basic, test_good_cdata_utf16_le);
  tcase_add_test(tc_basic, test_long_cdata_utf16);
  tcase_add_test(tc_basic, test_multichar_cdata_utf16);
  tcase_add_test(tc_basic, test_utf16_bad_surrogate_pair);
  tcase_add_test(tc_basic, test_bad_cdata);
  tcase_add_test(tc_basic, test_bad_cdata_utf16);
  tcase_add_test(tc_basic, test_stop_parser_between_cdata_calls);
  tcase_add_test(tc_basic, test_suspend_parser_between_cdata_calls);
  tcase_add_test(tc_basic, test_memory_allocation);
  tcase_add_test(tc_basic, test_default_current);
  tcase_add_test(tc_basic, test_dtd_elements);
  tcase_add_test(tc_basic, test_dtd_elements_nesting);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_set_foreign_dtd);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_foreign_dtd_not_standalone);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_invalid_foreign_dtd);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_foreign_dtd_with_doctype);
  tcase_add_test__ifdef_xml_dtd(tc_basic,
                                test_foreign_dtd_without_external_subset);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_empty_foreign_dtd);
  tcase_add_test(tc_basic, test_set_base);
  tcase_add_test(tc_basic, test_attributes);
  tcase_add_test(tc_basic, test_reset_in_entity);
  tcase_add_test(tc_basic, test_resume_invalid_parse);
  tcase_add_test(tc_basic, test_resume_resuspended);
  tcase_add_test(tc_basic, test_cdata_default);
  tcase_add_test(tc_basic, test_subordinate_reset);
  tcase_add_test(tc_basic, test_subordinate_suspend);
  tcase_add_test(tc_basic, test_subordinate_xdecl_suspend);
  tcase_add_test(tc_basic, test_subordinate_xdecl_abort);
  tcase_add_test(tc_basic, test_explicit_encoding);
  tcase_add_test(tc_basic, test_trailing_cr);
  tcase_add_test(tc_basic, test_ext_entity_trailing_cr);
  tcase_add_test(tc_basic, test_trailing_rsqb);
  tcase_add_test(tc_basic, test_ext_entity_trailing_rsqb);
  tcase_add_test(tc_basic, test_ext_entity_good_cdata);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_user_parameters);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_ext_entity_ref_parameter);
  tcase_add_test(tc_basic, test_empty_parse);
  tcase_add_test(tc_basic, test_get_buffer_1);
  tcase_add_test(tc_basic, test_get_buffer_2);
#if defined(XML_CONTEXT_BYTES)
  tcase_add_test(tc_basic, test_get_buffer_3_overflow);
#endif
  tcase_add_test(tc_basic, test_byte_info_at_end);
  tcase_add_test(tc_basic, test_byte_info_at_error);
  tcase_add_test(tc_basic, test_byte_info_at_cdata);
  tcase_add_test(tc_basic, test_predefined_entities);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_invalid_tag_in_dtd);
  tcase_add_test(tc_basic, test_not_predefined_entities);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_ignore_section);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_ignore_section_utf16);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_ignore_section_utf16_be);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_bad_ignore_section);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_external_entity_values);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_ext_entity_not_standalone);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_ext_entity_value_abort);
  tcase_add_test(tc_basic, test_bad_public_doctype);
  tcase_add_test(tc_basic, test_attribute_enum_value);
  tcase_add_test(tc_basic, test_predefined_entity_redefinition);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_dtd_stop_processing);
  tcase_add_test(tc_basic, test_public_notation_no_sysid);
  tcase_add_test(tc_basic, test_nested_groups);
  tcase_add_test(tc_basic, test_group_choice);
  tcase_add_test(tc_basic, test_standalone_parameter_entity);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_skipped_parameter_entity);
  tcase_add_test__ifdef_xml_dtd(tc_basic,
                                test_recursive_external_parameter_entity);
  tcase_add_test(tc_basic, test_undefined_ext_entity_in_external_dtd);
  tcase_add_test(tc_basic, test_suspend_xdecl);
  tcase_add_test(tc_basic, test_abort_epilog);
  tcase_add_test(tc_basic, test_abort_epilog_2);
  tcase_add_test(tc_basic, test_suspend_epilog);
  tcase_add_test(tc_basic, test_suspend_in_sole_empty_tag);
  tcase_add_test(tc_basic, test_unfinished_epilog);
  tcase_add_test(tc_basic, test_partial_char_in_epilog);
  tcase_add_test(tc_basic, test_hash_collision);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_suspend_resume_internal_entity);
  tcase_add_test__ifdef_xml_dtd(tc_basic,
                                test_suspend_resume_internal_entity_issue_629);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_resume_entity_with_syntax_error);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_suspend_resume_parameter_entity);
  tcase_add_test(tc_basic, test_restart_on_error);
  tcase_add_test(tc_basic, test_reject_lt_in_attribute_value);
  tcase_add_test(tc_basic, test_reject_unfinished_param_in_att_value);
  tcase_add_test(tc_basic, test_trailing_cr_in_att_value);
  tcase_add_test(tc_basic, test_standalone_internal_entity);
  tcase_add_test(tc_basic, test_skipped_external_entity);
  tcase_add_test(tc_basic, test_skipped_null_loaded_ext_entity);
  tcase_add_test(tc_basic, test_skipped_unloaded_ext_entity);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_param_entity_with_trailing_cr);
  tcase_add_test(tc_basic, test_invalid_character_entity);
  tcase_add_test(tc_basic, test_invalid_character_entity_2);
  tcase_add_test(tc_basic, test_invalid_character_entity_3);
  tcase_add_test(tc_basic, test_invalid_character_entity_4);
  tcase_add_test(tc_basic, test_pi_handled_in_default);
  tcase_add_test(tc_basic, test_comment_handled_in_default);
  tcase_add_test(tc_basic, test_pi_yml);
  tcase_add_test(tc_basic, test_pi_xnl);
  tcase_add_test(tc_basic, test_pi_xmm);
  tcase_add_test(tc_basic, test_utf16_pi);
  tcase_add_test(tc_basic, test_utf16_be_pi);
  tcase_add_test(tc_basic, test_utf16_be_comment);
  tcase_add_test(tc_basic, test_utf16_le_comment);
  tcase_add_test(tc_basic, test_missing_encoding_conversion_fn);
  tcase_add_test(tc_basic, test_failing_encoding_conversion_fn);
  tcase_add_test(tc_basic, test_unknown_encoding_success);
  tcase_add_test(tc_basic, test_unknown_encoding_bad_name);
  tcase_add_test(tc_basic, test_unknown_encoding_bad_name_2);
  tcase_add_test(tc_basic, test_unknown_encoding_long_name_1);
  tcase_add_test(tc_basic, test_unknown_encoding_long_name_2);
  tcase_add_test(tc_basic, test_invalid_unknown_encoding);
  tcase_add_test(tc_basic, test_unknown_ascii_encoding_ok);
  tcase_add_test(tc_basic, test_unknown_ascii_encoding_fail);
  tcase_add_test(tc_basic, test_unknown_encoding_invalid_length);
  tcase_add_test(tc_basic, test_unknown_encoding_invalid_topbit);
  tcase_add_test(tc_basic, test_unknown_encoding_invalid_surrogate);
  tcase_add_test(tc_basic, test_unknown_encoding_invalid_high);
  tcase_add_test(tc_basic, test_unknown_encoding_invalid_attr_value);
  tcase_add_test(tc_basic, test_ext_entity_latin1_utf16le_bom);
  tcase_add_test(tc_basic, test_ext_entity_latin1_utf16be_bom);
  tcase_add_test(tc_basic, test_ext_entity_latin1_utf16le_bom2);
  tcase_add_test(tc_basic, test_ext_entity_latin1_utf16be_bom2);
  tcase_add_test(tc_basic, test_ext_entity_utf16_be);
  tcase_add_test(tc_basic, test_ext_entity_utf16_le);
  tcase_add_test(tc_basic, test_ext_entity_utf16_unknown);
  tcase_add_test(tc_basic, test_ext_entity_utf8_non_bom);
  tcase_add_test(tc_basic, test_utf8_in_cdata_section);
  tcase_add_test(tc_basic, test_utf8_in_cdata_section_2);
  tcase_add_test(tc_basic, test_utf8_in_start_tags);
  tcase_add_test(tc_basic, test_trailing_spaces_in_elements);
  tcase_add_test(tc_basic, test_utf16_attribute);
  tcase_add_test(tc_basic, test_utf16_second_attr);
  tcase_add_test(tc_basic, test_attr_after_solidus);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_utf16_pe);
  tcase_add_test(tc_basic, test_bad_attr_desc_keyword);
  tcase_add_test(tc_basic, test_bad_attr_desc_keyword_utf16);
  tcase_add_test(tc_basic, test_bad_doctype);
  tcase_add_test(tc_basic, test_bad_doctype_utf8);
  tcase_add_test(tc_basic, test_bad_doctype_utf16);
  tcase_add_test(tc_basic, test_bad_doctype_plus);
  tcase_add_test(tc_basic, test_bad_doctype_star);
  tcase_add_test(tc_basic, test_bad_doctype_query);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_unknown_encoding_bad_ignore);
  tcase_add_test(tc_basic, test_entity_in_utf16_be_attr);
  tcase_add_test(tc_basic, test_entity_in_utf16_le_attr);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_entity_public_utf16_be);
  tcase_add_test__ifdef_xml_dtd(tc_basic, test_entity_public_utf16_le);
  tcase_add_test(tc_basic, test_short_doctype);
  tcase_add_test(tc_basic, test_short_doctype_2);
  tcase_add_test(tc_basic, test_short_doctype_3);
  tcase_add_test(tc_basic, test_long_doctype);
  tcase_add_test(tc_basic, test_bad_entity);
  tcase_add_test(tc_basic, test_bad_entity_2);
  tcase_add_test(tc_basic, test_bad_entity_3);
  tcase_add_test(tc_basic, test_bad_entity_4);
  tcase_add_test(tc_basic, test_bad_notation);
  tcase_add_test(tc_basic, test_default_doctype_handler);
  tcase_add_test(tc_basic, test_empty_element_abort);
  tcase_add_test__ifdef_xml_dtd(tc_basic,
                                test_pool_integrity_with_unfinished_attr);
  tcase_add_test(tc_basic, test_nested_entity_suspend);

  suite_add_tcase(s, tc_namespace);
  tcase_add_checked_fixture(tc_namespace, namespace_setup, namespace_teardown);
  tcase_add_test(tc_namespace, test_return_ns_triplet);
  tcase_add_test(tc_namespace, test_ns_tagname_overwrite);
  tcase_add_test(tc_namespace, test_ns_tagname_overwrite_triplet);
  tcase_add_test(tc_namespace, test_start_ns_clears_start_element);
  tcase_add_test__ifdef_xml_dtd(tc_namespace,
                                test_default_ns_from_ext_subset_and_ext_ge);
  tcase_add_test(tc_namespace, test_ns_prefix_with_empty_uri_1);
  tcase_add_test(tc_namespace, test_ns_prefix_with_empty_uri_2);
  tcase_add_test(tc_namespace, test_ns_prefix_with_empty_uri_3);
  tcase_add_test(tc_namespace, test_ns_prefix_with_empty_uri_4);
  tcase_add_test(tc_namespace, test_ns_unbound_prefix);
  tcase_add_test(tc_namespace, test_ns_default_with_empty_uri);
  tcase_add_test(tc_namespace, test_ns_duplicate_attrs_diff_prefixes);
  tcase_add_test(tc_namespace, test_ns_duplicate_hashes);
  tcase_add_test(tc_namespace, test_ns_unbound_prefix_on_attribute);
  tcase_add_test(tc_namespace, test_ns_unbound_prefix_on_element);
  tcase_add_test(tc_namespace, test_ns_parser_reset);
  tcase_add_test(tc_namespace, test_ns_long_element);
  tcase_add_test(tc_namespace, test_ns_mixed_prefix_atts);
  tcase_add_test(tc_namespace, test_ns_extend_uri_buffer);
  tcase_add_test(tc_namespace, test_ns_reserved_attributes);
  tcase_add_test(tc_namespace, test_ns_reserved_attributes_2);
  tcase_add_test(tc_namespace, test_ns_extremely_long_prefix);
  tcase_add_test(tc_namespace, test_ns_unknown_encoding_success);
  tcase_add_test(tc_namespace, test_ns_double_colon);
  tcase_add_test(tc_namespace, test_ns_double_colon_element);
  tcase_add_test(tc_namespace, test_ns_bad_attr_leafname);
  tcase_add_test(tc_namespace, test_ns_bad_element_leafname);
  tcase_add_test(tc_namespace, test_ns_utf16_leafname);
  tcase_add_test(tc_namespace, test_ns_utf16_element_leafname);
  tcase_add_test(tc_namespace, test_ns_utf16_doctype);
  tcase_add_test(tc_namespace, test_ns_invalid_doctype);
  tcase_add_test(tc_namespace, test_ns_double_colon_doctype);
  tcase_add_test(tc_namespace, test_ns_separator_in_uri);

  suite_add_tcase(s, tc_misc);
  tcase_add_checked_fixture(tc_misc, NULL, basic_teardown);
  tcase_add_test(tc_misc, test_misc_alloc_create_parser);
  tcase_add_test(tc_misc, test_misc_alloc_create_parser_with_encoding);
  tcase_add_test(tc_misc, test_misc_null_parser);
  tcase_add_test(tc_misc, test_misc_error_string);
  tcase_add_test(tc_misc, test_misc_version);
  tcase_add_test(tc_misc, test_misc_features);
  tcase_add_test(tc_misc, test_misc_attribute_leak);
  tcase_add_test(tc_misc, test_misc_utf16le);
  tcase_add_test(tc_misc, test_misc_stop_during_end_handler_issue_240_1);
  tcase_add_test(tc_misc, test_misc_stop_during_end_handler_issue_240_2);
  tcase_add_test__ifdef_xml_dtd(
      tc_misc, test_misc_deny_internal_entity_closing_doctype_issue_317);
  tcase_add_test(tc_misc, test_misc_tag_mismatch_reset_leak);

  suite_add_tcase(s, tc_alloc);
  tcase_add_checked_fixture(tc_alloc, alloc_setup, alloc_teardown);
  tcase_add_test(tc_alloc, test_alloc_parse_xdecl);
  tcase_add_test(tc_alloc, test_alloc_parse_xdecl_2);
  tcase_add_test(tc_alloc, test_alloc_parse_pi);
  tcase_add_test(tc_alloc, test_alloc_parse_pi_2);
  tcase_add_test(tc_alloc, test_alloc_parse_pi_3);
  tcase_add_test(tc_alloc, test_alloc_parse_comment);
  tcase_add_test(tc_alloc, test_alloc_parse_comment_2);
  tcase_add_test__ifdef_xml_dtd(tc_alloc, test_alloc_create_external_parser);
  tcase_add_test__ifdef_xml_dtd(tc_alloc, test_alloc_run_external_parser);
  tcase_add_test__ifdef_xml_dtd(tc_alloc, test_alloc_dtd_copy_default_atts);
  tcase_add_test__ifdef_xml_dtd(tc_alloc, test_alloc_external_entity);
  tcase_add_test__ifdef_xml_dtd(tc_alloc, test_alloc_ext_entity_set_encoding);
  tcase_add_test__ifdef_xml_dtd(tc_alloc, test_alloc_internal_entity);
  tcase_add_test__ifdef_xml_dtd(tc_alloc, test_alloc_dtd_default_handling);
  tcase_add_test(tc_alloc, test_alloc_explicit_encoding);
  tcase_add_test(tc_alloc, test_alloc_set_base);
  tcase_add_test(tc_alloc, test_alloc_realloc_buffer);
  tcase_add_test(tc_alloc, test_alloc_ext_entity_realloc_buffer);
  tcase_add_test(tc_alloc, test_alloc_realloc_many_attributes);
  tcase_add_test__ifdef_xml_dtd(tc_alloc, test_alloc_public_entity_value);
  tcase_add_test__ifdef_xml_dtd(tc_alloc,
                                test_alloc_realloc_subst_public_entity_value);
  tcase_add_test(tc_alloc, test_alloc_parse_public_doctype);
  tcase_add_test(tc_alloc, test_alloc_parse_public_doctype_long_name);
  tcase_add_test__ifdef_xml_dtd(tc_alloc, test_alloc_set_foreign_dtd);
  tcase_add_test__ifdef_xml_dtd(tc_alloc, test_alloc_attribute_enum_value);
  tcase_add_test__ifdef_xml_dtd(tc_alloc,
                                test_alloc_realloc_attribute_enum_value);
  tcase_add_test__ifdef_xml_dtd(tc_alloc, test_alloc_realloc_implied_attribute);
  tcase_add_test__ifdef_xml_dtd(tc_alloc, test_alloc_realloc_default_attribute);
  tcase_add_test(tc_alloc, test_alloc_notation);
  tcase_add_test(tc_alloc, test_alloc_public_notation);
  tcase_add_test(tc_alloc, test_alloc_system_notation);
  tcase_add_test__ifdef_xml_dtd(tc_alloc, test_alloc_nested_groups);
  tcase_add_test__ifdef_xml_dtd(tc_alloc, test_alloc_realloc_nested_groups);
  tcase_add_test(tc_alloc, test_alloc_large_group);
  tcase_add_test__ifdef_xml_dtd(tc_alloc, test_alloc_realloc_group_choice);
  tcase_add_test(tc_alloc, test_alloc_pi_in_epilog);
  tcase_add_test(tc_alloc, test_alloc_comment_in_epilog);
  tcase_add_test__ifdef_xml_dtd(tc_alloc,
                                test_alloc_realloc_long_attribute_value);
  tcase_add_test(tc_alloc, test_alloc_attribute_whitespace);
  tcase_add_test(tc_alloc, test_alloc_attribute_predefined_entity);
  tcase_add_test(tc_alloc, test_alloc_long_attr_default_with_char_ref);
  tcase_add_test(tc_alloc, test_alloc_long_attr_value);
  tcase_add_test__ifdef_xml_dtd(tc_alloc, test_alloc_nested_entities);
  tcase_add_test__ifdef_xml_dtd(tc_alloc,
                                test_alloc_realloc_param_entity_newline);
  tcase_add_test__ifdef_xml_dtd(tc_alloc, test_alloc_realloc_ce_extends_pe);
  tcase_add_test__ifdef_xml_dtd(tc_alloc, test_alloc_realloc_attributes);
  tcase_add_test(tc_alloc, test_alloc_long_doc_name);
  tcase_add_test(tc_alloc, test_alloc_long_base);
  tcase_add_test(tc_alloc, test_alloc_long_public_id);
  tcase_add_test(tc_alloc, test_alloc_long_entity_value);
  tcase_add_test(tc_alloc, test_alloc_long_notation);
  tcase_add_test__ifdef_xml_dtd(
      tc_alloc, test_alloc_reset_after_external_entity_parser_create_fail);

  suite_add_tcase(s, tc_nsalloc);
  tcase_add_checked_fixture(tc_nsalloc, nsalloc_setup, nsalloc_teardown);
  tcase_add_test(tc_nsalloc, test_nsalloc_xmlns);
  tcase_add_test(tc_nsalloc, test_nsalloc_parse_buffer);
  tcase_add_test(tc_nsalloc, test_nsalloc_long_prefix);
  tcase_add_test(tc_nsalloc, test_nsalloc_long_uri);
  tcase_add_test(tc_nsalloc, test_nsalloc_long_attr);
  tcase_add_test(tc_nsalloc, test_nsalloc_long_attr_prefix);
  tcase_add_test(tc_nsalloc, test_nsalloc_realloc_attributes);
  tcase_add_test(tc_nsalloc, test_nsalloc_long_element);
  tcase_add_test(tc_nsalloc, test_nsalloc_realloc_binding_uri);
  tcase_add_test(tc_nsalloc, test_nsalloc_realloc_long_prefix);
  tcase_add_test(tc_nsalloc, test_nsalloc_realloc_longer_prefix);
  tcase_add_test(tc_nsalloc, test_nsalloc_long_namespace);
  tcase_add_test(tc_nsalloc, test_nsalloc_less_long_namespace);
  tcase_add_test(tc_nsalloc, test_nsalloc_long_context);
  tcase_add_test(tc_nsalloc, test_nsalloc_realloc_long_context);
  tcase_add_test(tc_nsalloc, test_nsalloc_realloc_long_context_2);
  tcase_add_test(tc_nsalloc, test_nsalloc_realloc_long_context_3);
  tcase_add_test(tc_nsalloc, test_nsalloc_realloc_long_context_4);
  tcase_add_test(tc_nsalloc, test_nsalloc_realloc_long_context_5);
  tcase_add_test(tc_nsalloc, test_nsalloc_realloc_long_context_6);
  tcase_add_test(tc_nsalloc, test_nsalloc_realloc_long_context_7);
  tcase_add_test(tc_nsalloc, test_nsalloc_realloc_long_ge_name);
  tcase_add_test(tc_nsalloc, test_nsalloc_realloc_long_context_in_dtd);
  tcase_add_test(tc_nsalloc, test_nsalloc_long_default_in_ext);
  tcase_add_test(tc_nsalloc, test_nsalloc_long_systemid_in_ext);
  tcase_add_test(tc_nsalloc, test_nsalloc_prefixed_element);

#if defined(XML_DTD)
  suite_add_tcase(s, tc_accounting);
  tcase_add_test(tc_accounting, test_accounting_precision);
  tcase_add_test(tc_accounting, test_billion_laughs_attack_protection_api);
  tcase_add_test(tc_accounting, test_helper_unsigned_char_to_printable);
#endif

  return s;
}

int
main(int argc, char *argv[]) {
  int i, nf;
  int verbosity = CK_NORMAL;
  Suite *s = make_suite();
  SRunner *sr = srunner_create(s);

  /* run the tests for internal helper functions */
  testhelper_is_whitespace_normalized();

  for (i = 1; i < argc; ++i) {
    char *opt = argv[i];
    if (strcmp(opt, "-v") == 0 || strcmp(opt, "--verbose") == 0)
      verbosity = CK_VERBOSE;
    else if (strcmp(opt, "-q") == 0 || strcmp(opt, "--quiet") == 0)
      verbosity = CK_SILENT;
    else {
      fprintf(stderr, "runtests: unknown option '%s'\n", opt);
      return 2;
    }
  }
  if (verbosity != CK_SILENT)
    printf("Expat version: %" XML_FMT_STR "\n", XML_ExpatVersion());
  srunner_run_all(sr, verbosity);
  nf = srunner_ntests_failed(sr);
  srunner_free(sr);

  return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
