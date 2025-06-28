/*
                            __  __            _
                         ___\ \/ /_ __   __ _| |_
                        / _ \\  /| '_ \ / _` | __|
                       |  __//  \| |_) | (_| | |_
                        \___/_/\_\ .__/ \__,_|\__|
                                 |_| XML parser

   Copyright (c) 1997-2000 Thai Open Source Software Center Ltd
   Copyright (c) 2000      Clark Cooper <coopercc@users.sourceforge.net>
   Copyright (c) 2001-2003 Fred L. Drake, Jr. <fdrake@users.sourceforge.net>
   Copyright (c) 2004-2009 Karl Waclawek <karl@waclawek.net>
   Copyright (c) 2005-2007 Steven Solie <steven@solie.ca>
   Copyright (c) 2016-2023 Sebastian Pipping <sebastian@pipping.org>
   Copyright (c) 2017      Rhodri James <rhodri@wildebeest.org.uk>
   Copyright (c) 2019      David Loffredo <loffredo@steptools.com>
   Copyright (c) 2020      Joe Orton <jorton@redhat.com>
   Copyright (c) 2020      Kleber Tarcísio <klebertarcisio@yahoo.com.br>
   Copyright (c) 2021      Tim Bray <tbray@textuality.com>
   Copyright (c) 2022      Martin Ettl <ettl.martin78@googlemail.com>
   Copyright (c) 2022      Sean McBride <sean@rogue-research.com>
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

#include "expat_config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h> /* for isnan */
#include <errno.h>

#include "expat.h"
#include "codepage.h"
#include "internal.h" /* for UNUSED_P only */
#include "xmlfile.h"
#include "xmltchar.h"

#ifdef _MSC_VER
#  include <crtdbg.h>
#endif

#ifdef XML_UNICODE
#  include <wchar.h>
#endif

enum ExitCode {
  XMLWF_EXIT_SUCCESS = 0,
  XMLWF_EXIT_INTERNAL_ERROR = 1,
  XMLWF_EXIT_NOT_WELLFORMED = 2,
  XMLWF_EXIT_OUTPUT_ERROR = 3,
  XMLWF_EXIT_USAGE_ERROR = 4,
};

/* Structures for handler user data */
typedef struct NotationList {
  struct NotationList *next;
  const XML_Char *notationName;
  const XML_Char *systemId;
  const XML_Char *publicId;
} NotationList;

typedef struct xmlwfUserData {
  FILE *fp;
  NotationList *notationListHead;
  const XML_Char *currentDoctypeName;
} XmlwfUserData;

/* This ensures proper sorting. */

#define NSSEP T('\001')

static void XMLCALL
characterData(void *userData, const XML_Char *s, int len) {
  FILE *fp = ((XmlwfUserData *)userData)->fp;
  for (; len > 0; --len, ++s) {
    switch (*s) {
    case T('&'):
      fputts(T("&amp;"), fp);
      break;
    case T('<'):
      fputts(T("&lt;"), fp);
      break;
    case T('>'):
      fputts(T("&gt;"), fp);
      break;
#ifdef W3C14N
    case 13:
      fputts(T("&#xD;"), fp);
      break;
#else
    case T('"'):
      fputts(T("&quot;"), fp);
      break;
    case 9:
    case 10:
    case 13:
      ftprintf(fp, T("&#%d;"), *s);
      break;
#endif
    default:
      puttc(*s, fp);
      break;
    }
  }
}

static void
attributeValue(FILE *fp, const XML_Char *s) {
  puttc(T('='), fp);
  puttc(T('"'), fp);
  assert(s);
  for (;;) {
    switch (*s) {
    case 0:
    case NSSEP:
      puttc(T('"'), fp);
      return;
    case T('&'):
      fputts(T("&amp;"), fp);
      break;
    case T('<'):
      fputts(T("&lt;"), fp);
      break;
    case T('"'):
      fputts(T("&quot;"), fp);
      break;
#ifdef W3C14N
    case 9:
      fputts(T("&#x9;"), fp);
      break;
    case 10:
      fputts(T("&#xA;"), fp);
      break;
    case 13:
      fputts(T("&#xD;"), fp);
      break;
#else
    case T('>'):
      fputts(T("&gt;"), fp);
      break;
    case 9:
    case 10:
    case 13:
      ftprintf(fp, T("&#%d;"), *s);
      break;
#endif
    default:
      puttc(*s, fp);
      break;
    }
    s++;
  }
}

/* Lexicographically comparing UTF-8 encoded attribute values,
is equivalent to lexicographically comparing based on the character number. */

static int
attcmp(const void *att1, const void *att2) {
  return tcscmp(*(const XML_Char *const *)att1, *(const XML_Char *const *)att2);
}

static void XMLCALL
startElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  int nAtts;
  const XML_Char **p;
  FILE *fp = ((XmlwfUserData *)userData)->fp;
  puttc(T('<'), fp);
  fputts(name, fp);

  p = atts;
  while (*p)
    ++p;
  nAtts = (int)((p - atts) >> 1);
  if (nAtts > 1)
    qsort((void *)atts, nAtts, sizeof(XML_Char *) * 2, attcmp);
  while (*atts) {
    puttc(T(' '), fp);
    fputts(*atts++, fp);
    attributeValue(fp, *atts);
    atts++;
  }
  puttc(T('>'), fp);
}

static void XMLCALL
endElement(void *userData, const XML_Char *name) {
  FILE *fp = ((XmlwfUserData *)userData)->fp;
  puttc(T('<'), fp);
  puttc(T('/'), fp);
  fputts(name, fp);
  puttc(T('>'), fp);
}

static int
nsattcmp(const void *p1, const void *p2) {
  const XML_Char *att1 = *(const XML_Char *const *)p1;
  const XML_Char *att2 = *(const XML_Char *const *)p2;
  int sep1 = (tcsrchr(att1, NSSEP) != 0);
  int sep2 = (tcsrchr(att2, NSSEP) != 0);
  if (sep1 != sep2)
    return sep1 - sep2;
  return tcscmp(att1, att2);
}

static void XMLCALL
startElementNS(void *userData, const XML_Char *name, const XML_Char **atts) {
  int nAtts;
  int nsi;
  const XML_Char **p;
  FILE *fp = ((XmlwfUserData *)userData)->fp;
  const XML_Char *sep;
  puttc(T('<'), fp);

  sep = tcsrchr(name, NSSEP);
  if (sep) {
    fputts(T("n1:"), fp);
    fputts(sep + 1, fp);
    fputts(T(" xmlns:n1"), fp);
    attributeValue(fp, name);
    nsi = 2;
  } else {
    fputts(name, fp);
    nsi = 1;
  }

  p = atts;
  while (*p)
    ++p;
  nAtts = (int)((p - atts) >> 1);
  if (nAtts > 1)
    qsort((void *)atts, nAtts, sizeof(XML_Char *) * 2, nsattcmp);
  while (*atts) {
    name = *atts++;
    sep = tcsrchr(name, NSSEP);
    puttc(T(' '), fp);
    if (sep) {
      ftprintf(fp, T("n%d:"), nsi);
      fputts(sep + 1, fp);
    } else
      fputts(name, fp);
    attributeValue(fp, *atts);
    if (sep) {
      ftprintf(fp, T(" xmlns:n%d"), nsi++);
      attributeValue(fp, name);
    }
    atts++;
  }
  puttc(T('>'), fp);
}

static void XMLCALL
endElementNS(void *userData, const XML_Char *name) {
  FILE *fp = ((XmlwfUserData *)userData)->fp;
  const XML_Char *sep;
  puttc(T('<'), fp);
  puttc(T('/'), fp);
  sep = tcsrchr(name, NSSEP);
  if (sep) {
    fputts(T("n1:"), fp);
    fputts(sep + 1, fp);
  } else
    fputts(name, fp);
  puttc(T('>'), fp);
}

#ifndef W3C14N

static void XMLCALL
processingInstruction(void *userData, const XML_Char *target,
                      const XML_Char *data) {
  FILE *fp = ((XmlwfUserData *)userData)->fp;
  puttc(T('<'), fp);
  puttc(T('?'), fp);
  fputts(target, fp);
  puttc(T(' '), fp);
  fputts(data, fp);
  puttc(T('?'), fp);
  puttc(T('>'), fp);
}

static XML_Char *
xcsdup(const XML_Char *s) {
  XML_Char *result;
  int count = 0;
  int numBytes;

  /* Get the length of the string, including terminator */
  while (s[count++] != 0) {
    /* Do nothing */
  }
  numBytes = count * sizeof(XML_Char);
  result = malloc(numBytes);
  if (result == NULL)
    return NULL;
  memcpy(result, s, numBytes);
  return result;
}

static void XMLCALL
startDoctypeDecl(void *userData, const XML_Char *doctypeName,
                 const XML_Char *sysid, const XML_Char *publid,
                 int has_internal_subset) {
  XmlwfUserData *data = (XmlwfUserData *)userData;
  UNUSED_P(sysid);
  UNUSED_P(publid);
  UNUSED_P(has_internal_subset);
  data->currentDoctypeName = xcsdup(doctypeName);
}

static void
freeNotations(XmlwfUserData *data) {
  NotationList *notationListHead = data->notationListHead;

  while (notationListHead != NULL) {
    NotationList *next = notationListHead->next;
    free((void *)notationListHead->notationName);
    free((void *)notationListHead->systemId);
    free((void *)notationListHead->publicId);
    free(notationListHead);
    notationListHead = next;
  }
  data->notationListHead = NULL;
}

static void
cleanupUserData(XmlwfUserData *userData) {
  free((void *)userData->currentDoctypeName);
  userData->currentDoctypeName = NULL;
  freeNotations(userData);
}

static int
xcscmp(const XML_Char *xs, const XML_Char *xt) {
  while (*xs != 0 && *xt != 0) {
    if (*xs < *xt)
      return -1;
    if (*xs > *xt)
      return 1;
    xs++;
    xt++;
  }
  if (*xs < *xt)
    return -1;
  if (*xs > *xt)
    return 1;
  return 0;
}

static int
notationCmp(const void *a, const void *b) {
  const NotationList *const n1 = *(const NotationList *const *)a;
  const NotationList *const n2 = *(const NotationList *const *)b;

  return xcscmp(n1->notationName, n2->notationName);
}

static void XMLCALL
endDoctypeDecl(void *userData) {
  XmlwfUserData *data = (XmlwfUserData *)userData;
  NotationList **notations;
  int notationCount = 0;
  NotationList *p;
  int i;

  /* How many notations do we have? */
  for (p = data->notationListHead; p != NULL; p = p->next)
    notationCount++;
  if (notationCount == 0) {
    /* Nothing to report */
    free((void *)data->currentDoctypeName);
    data->currentDoctypeName = NULL;
    return;
  }

  notations = malloc(notationCount * sizeof(NotationList *));
  if (notations == NULL) {
    fprintf(stderr, "Unable to sort notations");
    freeNotations(data);
    return;
  }

  for (p = data->notationListHead, i = 0; i < notationCount; p = p->next, i++) {
    notations[i] = p;
  }
  qsort(notations, notationCount, sizeof(NotationList *), notationCmp);

  /* Output the DOCTYPE header */
  fputts(T("<!DOCTYPE "), data->fp);
  fputts(data->currentDoctypeName, data->fp);
  fputts(T(" [\n"), data->fp);

  /* Now the NOTATIONs */
  for (i = 0; i < notationCount; i++) {
    fputts(T("<!NOTATION "), data->fp);
    fputts(notations[i]->notationName, data->fp);
    if (notations[i]->publicId != NULL) {
      fputts(T(" PUBLIC '"), data->fp);
      fputts(notations[i]->publicId, data->fp);
      puttc(T('\''), data->fp);
      if (notations[i]->systemId != NULL) {
        puttc(T(' '), data->fp);
        puttc(T('\''), data->fp);
        fputts(notations[i]->systemId, data->fp);
        puttc(T('\''), data->fp);
      }
    } else if (notations[i]->systemId != NULL) {
      fputts(T(" SYSTEM '"), data->fp);
      fputts(notations[i]->systemId, data->fp);
      puttc(T('\''), data->fp);
    }
    puttc(T('>'), data->fp);
    puttc(T('\n'), data->fp);
  }

  /* Finally end the DOCTYPE */
  fputts(T("]>\n"), data->fp);

  free(notations);
  freeNotations(data);
  free((void *)data->currentDoctypeName);
  data->currentDoctypeName = NULL;
}

static void XMLCALL
notationDecl(void *userData, const XML_Char *notationName, const XML_Char *base,
             const XML_Char *systemId, const XML_Char *publicId) {
  XmlwfUserData *data = (XmlwfUserData *)userData;
  NotationList *entry = malloc(sizeof(NotationList));
  const char *errorMessage = "Unable to store NOTATION for output\n";

  UNUSED_P(base);
  if (entry == NULL) {
    fputs(errorMessage, stderr);
    return; /* Nothing we can really do about this */
  }
  entry->notationName = xcsdup(notationName);
  if (entry->notationName == NULL) {
    fputs(errorMessage, stderr);
    free(entry);
    return;
  }
  if (systemId != NULL) {
    entry->systemId = xcsdup(systemId);
    if (entry->systemId == NULL) {
      fputs(errorMessage, stderr);
      free((void *)entry->notationName);
      free(entry);
      return;
    }
  } else {
    entry->systemId = NULL;
  }
  if (publicId != NULL) {
    entry->publicId = xcsdup(publicId);
    if (entry->publicId == NULL) {
      fputs(errorMessage, stderr);
      free((void *)entry->systemId); /* Safe if it's NULL */
      free((void *)entry->notationName);
      free(entry);
      return;
    }
  } else {
    entry->publicId = NULL;
  }

  entry->next = data->notationListHead;
  data->notationListHead = entry;
}

#endif /* not W3C14N */

static void XMLCALL
defaultCharacterData(void *userData, const XML_Char *s, int len) {
  UNUSED_P(s);
  UNUSED_P(len);
  XML_DefaultCurrent((XML_Parser)userData);
}

static void XMLCALL
defaultStartElement(void *userData, const XML_Char *name,
                    const XML_Char **atts) {
  UNUSED_P(name);
  UNUSED_P(atts);
  XML_DefaultCurrent((XML_Parser)userData);
}

static void XMLCALL
defaultEndElement(void *userData, const XML_Char *name) {
  UNUSED_P(name);
  XML_DefaultCurrent((XML_Parser)userData);
}

static void XMLCALL
defaultProcessingInstruction(void *userData, const XML_Char *target,
                             const XML_Char *data) {
  UNUSED_P(target);
  UNUSED_P(data);
  XML_DefaultCurrent((XML_Parser)userData);
}

static void XMLCALL
nopCharacterData(void *userData, const XML_Char *s, int len) {
  UNUSED_P(userData);
  UNUSED_P(s);
  UNUSED_P(len);
}

static void XMLCALL
nopStartElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  UNUSED_P(userData);
  UNUSED_P(name);
  UNUSED_P(atts);
}

static void XMLCALL
nopEndElement(void *userData, const XML_Char *name) {
  UNUSED_P(userData);
  UNUSED_P(name);
}

static void XMLCALL
nopProcessingInstruction(void *userData, const XML_Char *target,
                         const XML_Char *data) {
  UNUSED_P(userData);
  UNUSED_P(target);
  UNUSED_P(data);
}

static void XMLCALL
markup(void *userData, const XML_Char *s, int len) {
  FILE *fp = ((XmlwfUserData *)XML_GetUserData((XML_Parser)userData))->fp;
  for (; len > 0; --len, ++s)
    puttc(*s, fp);
}

static void
metaLocation(XML_Parser parser) {
  const XML_Char *uri = XML_GetBase(parser);
  FILE *fp = ((XmlwfUserData *)XML_GetUserData(parser))->fp;
  if (uri)
    ftprintf(fp, T(" uri=\"%s\""), uri);
  ftprintf(fp,
           T(" byte=\"%") T(XML_FMT_INT_MOD) T("d\"") T(" nbytes=\"%d\"")
               T(" line=\"%") T(XML_FMT_INT_MOD) T("u\"") T(" col=\"%")
                   T(XML_FMT_INT_MOD) T("u\""),
           XML_GetCurrentByteIndex(parser), XML_GetCurrentByteCount(parser),
           XML_GetCurrentLineNumber(parser),
           XML_GetCurrentColumnNumber(parser));
}

static void
metaStartDocument(void *userData) {
  fputts(T("<document>\n"),
         ((XmlwfUserData *)XML_GetUserData((XML_Parser)userData))->fp);
}

static void
metaEndDocument(void *userData) {
  fputts(T("</document>\n"),
         ((XmlwfUserData *)XML_GetUserData((XML_Parser)userData))->fp);
}

static void XMLCALL
metaStartElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  XML_Parser parser = (XML_Parser)userData;
  XmlwfUserData *data = (XmlwfUserData *)XML_GetUserData(parser);
  FILE *fp = data->fp;
  const XML_Char **specifiedAttsEnd
      = atts + XML_GetSpecifiedAttributeCount(parser);
  const XML_Char **idAttPtr;
  int idAttIndex = XML_GetIdAttributeIndex(parser);
  if (idAttIndex < 0)
    idAttPtr = 0;
  else
    idAttPtr = atts + idAttIndex;

  ftprintf(fp, T("<starttag name=\"%s\""), name);
  metaLocation(parser);
  if (*atts) {
    fputts(T(">\n"), fp);
    do {
      ftprintf(fp, T("<attribute name=\"%s\" value=\""), atts[0]);
      characterData(data, atts[1], (int)tcslen(atts[1]));
      if (atts >= specifiedAttsEnd)
        fputts(T("\" defaulted=\"yes\"/>\n"), fp);
      else if (atts == idAttPtr)
        fputts(T("\" id=\"yes\"/>\n"), fp);
      else
        fputts(T("\"/>\n"), fp);
    } while (*(atts += 2));
    fputts(T("</starttag>\n"), fp);
  } else
    fputts(T("/>\n"), fp);
}

static void XMLCALL
metaEndElement(void *userData, const XML_Char *name) {
  XML_Parser parser = (XML_Parser)userData;
  XmlwfUserData *data = (XmlwfUserData *)XML_GetUserData(parser);
  FILE *fp = data->fp;
  ftprintf(fp, T("<endtag name=\"%s\""), name);
  metaLocation(parser);
  fputts(T("/>\n"), fp);
}

static void XMLCALL
metaProcessingInstruction(void *userData, const XML_Char *target,
                          const XML_Char *data) {
  XML_Parser parser = (XML_Parser)userData;
  XmlwfUserData *usrData = (XmlwfUserData *)XML_GetUserData(parser);
  FILE *fp = usrData->fp;
  ftprintf(fp, T("<pi target=\"%s\" data=\""), target);
  characterData(usrData, data, (int)tcslen(data));
  puttc(T('"'), fp);
  metaLocation(parser);
  fputts(T("/>\n"), fp);
}

static void XMLCALL
metaComment(void *userData, const XML_Char *data) {
  XML_Parser parser = (XML_Parser)userData;
  XmlwfUserData *usrData = (XmlwfUserData *)XML_GetUserData(parser);
  FILE *fp = usrData->fp;
  fputts(T("<comment data=\""), fp);
  characterData(usrData, data, (int)tcslen(data));
  puttc(T('"'), fp);
  metaLocation(parser);
  fputts(T("/>\n"), fp);
}

static void XMLCALL
metaStartCdataSection(void *userData) {
  XML_Parser parser = (XML_Parser)userData;
  XmlwfUserData *data = (XmlwfUserData *)XML_GetUserData(parser);
  FILE *fp = data->fp;
  fputts(T("<startcdata"), fp);
  metaLocation(parser);
  fputts(T("/>\n"), fp);
}

static void XMLCALL
metaEndCdataSection(void *userData) {
  XML_Parser parser = (XML_Parser)userData;
  XmlwfUserData *data = (XmlwfUserData *)XML_GetUserData(parser);
  FILE *fp = data->fp;
  fputts(T("<endcdata"), fp);
  metaLocation(parser);
  fputts(T("/>\n"), fp);
}

static void XMLCALL
metaCharacterData(void *userData, const XML_Char *s, int len) {
  XML_Parser parser = (XML_Parser)userData;
  XmlwfUserData *data = (XmlwfUserData *)XML_GetUserData(parser);
  FILE *fp = data->fp;
  fputts(T("<chars str=\""), fp);
  characterData(data, s, len);
  puttc(T('"'), fp);
  metaLocation(parser);
  fputts(T("/>\n"), fp);
}

static void XMLCALL
metaStartDoctypeDecl(void *userData, const XML_Char *doctypeName,
                     const XML_Char *sysid, const XML_Char *pubid,
                     int has_internal_subset) {
  XML_Parser parser = (XML_Parser)userData;
  XmlwfUserData *data = (XmlwfUserData *)XML_GetUserData(parser);
  FILE *fp = data->fp;
  UNUSED_P(sysid);
  UNUSED_P(pubid);
  UNUSED_P(has_internal_subset);
  ftprintf(fp, T("<startdoctype name=\"%s\""), doctypeName);
  metaLocation(parser);
  fputts(T("/>\n"), fp);
}

static void XMLCALL
metaEndDoctypeDecl(void *userData) {
  XML_Parser parser = (XML_Parser)userData;
  XmlwfUserData *data = (XmlwfUserData *)XML_GetUserData(parser);
  FILE *fp = data->fp;
  fputts(T("<enddoctype"), fp);
  metaLocation(parser);
  fputts(T("/>\n"), fp);
}

static void XMLCALL
metaNotationDecl(void *userData, const XML_Char *notationName,
                 const XML_Char *base, const XML_Char *systemId,
                 const XML_Char *publicId) {
  XML_Parser parser = (XML_Parser)userData;
  XmlwfUserData *data = (XmlwfUserData *)XML_GetUserData(parser);
  FILE *fp = data->fp;
  UNUSED_P(base);
  ftprintf(fp, T("<notation name=\"%s\""), notationName);
  if (publicId)
    ftprintf(fp, T(" public=\"%s\""), publicId);
  if (systemId) {
    fputts(T(" system=\""), fp);
    characterData(data, systemId, (int)tcslen(systemId));
    puttc(T('"'), fp);
  }
  metaLocation(parser);
  fputts(T("/>\n"), fp);
}

static void XMLCALL
metaEntityDecl(void *userData, const XML_Char *entityName, int is_param,
               const XML_Char *value, int value_length, const XML_Char *base,
               const XML_Char *systemId, const XML_Char *publicId,
               const XML_Char *notationName) {
  XML_Parser parser = (XML_Parser)userData;
  XmlwfUserData *data = (XmlwfUserData *)XML_GetUserData(parser);
  FILE *fp = data->fp;

  UNUSED_P(is_param);
  UNUSED_P(base);
  if (value) {
    ftprintf(fp, T("<entity name=\"%s\""), entityName);
    metaLocation(parser);
    puttc(T('>'), fp);
    characterData(data, value, value_length);
    fputts(T("</entity/>\n"), fp);
  } else if (notationName) {
    ftprintf(fp, T("<entity name=\"%s\""), entityName);
    if (publicId)
      ftprintf(fp, T(" public=\"%s\""), publicId);
    fputts(T(" system=\""), fp);
    characterData(data, systemId, (int)tcslen(systemId));
    puttc(T('"'), fp);
    ftprintf(fp, T(" notation=\"%s\""), notationName);
    metaLocation(parser);
    fputts(T("/>\n"), fp);
  } else {
    ftprintf(fp, T("<entity name=\"%s\""), entityName);
    if (publicId)
      ftprintf(fp, T(" public=\"%s\""), publicId);
    fputts(T(" system=\""), fp);
    characterData(data, systemId, (int)tcslen(systemId));
    puttc(T('"'), fp);
    metaLocation(parser);
    fputts(T("/>\n"), fp);
  }
}

static void XMLCALL
metaStartNamespaceDecl(void *userData, const XML_Char *prefix,
                       const XML_Char *uri) {
  XML_Parser parser = (XML_Parser)userData;
  XmlwfUserData *data = (XmlwfUserData *)XML_GetUserData(parser);
  FILE *fp = data->fp;
  fputts(T("<startns"), fp);
  if (prefix)
    ftprintf(fp, T(" prefix=\"%s\""), prefix);
  if (uri) {
    fputts(T(" ns=\""), fp);
    characterData(data, uri, (int)tcslen(uri));
    fputts(T("\"/>\n"), fp);
  } else
    fputts(T("/>\n"), fp);
}

static void XMLCALL
metaEndNamespaceDecl(void *userData, const XML_Char *prefix) {
  XML_Parser parser = (XML_Parser)userData;
  XmlwfUserData *data = (XmlwfUserData *)XML_GetUserData(parser);
  FILE *fp = data->fp;
  if (! prefix)
    fputts(T("<endns/>\n"), fp);
  else
    ftprintf(fp, T("<endns prefix=\"%s\"/>\n"), prefix);
}

static int XMLCALL
unknownEncodingConvert(void *data, const char *p) {
  return codepageConvert(*(int *)data, p);
}

static int XMLCALL
unknownEncoding(void *userData, const XML_Char *name, XML_Encoding *info) {
  int cp;
  static const XML_Char prefixL[] = T("windows-");
  static const XML_Char prefixU[] = T("WINDOWS-");
  int i;

  UNUSED_P(userData);
  for (i = 0; prefixU[i]; i++)
    if (name[i] != prefixU[i] && name[i] != prefixL[i])
      return 0;

  cp = 0;
  for (; name[i]; i++) {
    static const XML_Char digits[] = T("0123456789");
    const XML_Char *s = tcschr(digits, name[i]);
    if (! s)
      return 0;
    cp *= 10;
    cp += (int)(s - digits);
    if (cp >= 0x10000)
      return 0;
  }
  if (! codepageMap(cp, info->map))
    return 0;
  info->convert = unknownEncodingConvert;
  /* We could just cast the code page integer to a void *,
  and avoid the use of release. */
  info->release = free;
  info->data = malloc(sizeof(int));
  if (! info->data)
    return 0;
  *(int *)info->data = cp;
  return 1;
}

static int XMLCALL
notStandalone(void *userData) {
  UNUSED_P(userData);
  return 0;
}

static void
showVersion(XML_Char *prog) {
  XML_Char *s = prog;
  XML_Char ch;
  const XML_Feature *features = XML_GetFeatureList();
  while ((ch = *s) != 0) {
    if (ch == '/'
#if defined(_WIN32)
        || ch == '\\'
#endif
    )
      prog = s + 1;
    ++s;
  }
  ftprintf(stdout, T("%s using %s\n"), prog, XML_ExpatVersion());
  if (features != NULL && features[0].feature != XML_FEATURE_END) {
    int i = 1;
    ftprintf(stdout, T("%s"), features[0].name);
    if (features[0].value)
      ftprintf(stdout, T("=%ld"), features[0].value);
    while (features[i].feature != XML_FEATURE_END) {
      ftprintf(stdout, T(", %s"), features[i].name);
      if (features[i].value)
        ftprintf(stdout, T("=%ld"), features[i].value);
      ++i;
    }
    ftprintf(stdout, T("\n"));
  }
}

#if defined(__GNUC__)
__attribute__((noreturn))
#endif
static void
usage(const XML_Char *prog, int rc) {
  ftprintf(
      stderr,
      /* Generated with:
       * $ xmlwf/xmlwf_helpgen.sh
       * To update, change xmlwf/xmlwf_helpgen.py, then paste the output of
       * xmlwf/xmlwf_helpgen.sh in here.
       */
      /* clang-format off */
      T("usage:\n")
      T("  %s [OPTIONS] [FILE ...]\n")
      T("  %s -h|--help\n")
      T("  %s -v|--version\n")
      T("\n")
      T("xmlwf - Determines if an XML document is well-formed\n")
      T("\n")
      T("positional arguments:\n")
      T("  FILE           file to process (default: STDIN)\n")
      T("\n")
      T("input control arguments:\n")
      T("  -s             print an error if the document is not [s]tandalone\n")
      T("  -n             enable [n]amespace processing\n")
      T("  -p             enable processing of external DTDs and [p]arameter entities\n")
      T("  -x             enable processing of e[x]ternal entities\n")
      T("  -e ENCODING    override any in-document [e]ncoding declaration\n")
      T("  -w             enable support for [W]indows code pages\n")
      T("  -r             disable memory-mapping and use [r]ead calls instead\n")
      T("  -g BYTES       buffer size to request per call pair to XML_[G]etBuffer and read (default: 8 KiB)\n")
      T("  -k             when processing multiple files, [k]eep processing after first file with error\n")
      T("\n")
      T("output control arguments:\n")
      T("  -d DIRECTORY   output [d]estination directory\n")
      T("  -c             write a [c]opy of input XML, not canonical XML\n")
      T("  -m             write [m]eta XML, not canonical XML\n")
      T("  -t             write no XML output for [t]iming of plain parsing\n")
      T("  -N             enable adding doctype and [n]otation declarations\n")
      T("\n")
      T("billion laughs attack protection:\n")
      T("  NOTE: If you ever need to increase these values for non-attack payload, please file a bug report.\n")
      T("\n")
      T("  -a FACTOR      set maximum tolerated [a]mplification factor (default: 100.0)\n")
      T("  -b BYTES       set number of output [b]ytes needed to activate (default: 8 MiB)\n")
      T("\n")
      T("reparse deferral:\n")
      T("  -q             disable reparse deferral, and allow [q]uadratic parse runtime with large tokens\n")
      T("\n")
      T("info arguments:\n")
      T("  -h, --help     show this [h]elp message and exit\n")
      T("  -v, --version  show program's [v]ersion number and exit\n")
      T("\n")
      T("exit status:\n")
      T("  0              the input files are well-formed and the output (if requested) was written successfully\n")
      T("  1              could not allocate data structures, signals a serious problem with execution environment\n")
      T("  2              one or more input files were not well-formed\n")
      T("  3              could not create an output file\n")
      T("  4              command-line argument error\n")
      T("\n")
      T("xmlwf of libexpat is software libre, licensed under the MIT license.\n")
      T("Please report bugs at https://github.com/libexpat/libexpat/issues -- thank you!\n")
      , /* clang-format on */
      prog, prog, prog);
  exit(rc);
}

#if defined(__MINGW32__) && defined(XML_UNICODE)
/* Silence warning about missing prototype */
int wmain(int argc, XML_Char **argv);
#endif

#define XMLWF_SHIFT_ARG_INTO(constCharStarTarget, argc, argv, i, j)            \
  {                                                                            \
    if (argv[i][j + 1] == T('\0')) {                                           \
      if (++i == argc) {                                                       \
        usage(argv[0], XMLWF_EXIT_USAGE_ERROR);                                \
        /* usage called exit(..), never gets here */                           \
      }                                                                        \
      constCharStarTarget = argv[i];                                           \
    } else {                                                                   \
      constCharStarTarget = argv[i] + j + 1;                                   \
    }                                                                          \
    i++;                                                                       \
    j = 0;                                                                     \
  }

int
tmain(int argc, XML_Char **argv) {
  int i, j;
  const XML_Char *outputDir = NULL;
  const XML_Char *encoding = NULL;
  unsigned processFlags = XML_MAP_FILE;
  int windowsCodePages = 0;
  int outputType = 0;
  int useNamespaces = 0;
  int requireStandalone = 0;
  int requiresNotations = 0;
  int continueOnError = 0;

  float attackMaximumAmplification = -1.0f; /* signaling "not set" */
  unsigned long long attackThresholdBytes = 0;
  XML_Bool attackThresholdGiven = XML_FALSE;

  XML_Bool disableDeferral = XML_FALSE;

  int exitCode = XMLWF_EXIT_SUCCESS;
  enum XML_ParamEntityParsing paramEntityParsing
      = XML_PARAM_ENTITY_PARSING_NEVER;
  int useStdin = 0;
  XmlwfUserData userData = {NULL, NULL, NULL};

#ifdef _MSC_VER
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  i = 1;
  j = 0;
  while (i < argc) {
    if (j == 0) {
      if (argv[i][0] != T('-'))
        break;
      if (argv[i][1] == T('-')) {
        if (argv[i][2] == T('\0')) {
          i++;
          break;
        } else if (tcscmp(argv[i] + 2, T("help")) == 0) {
          usage(argv[0], XMLWF_EXIT_SUCCESS);
          // usage called exit(..), never gets here
        } else if (tcscmp(argv[i] + 2, T("version")) == 0) {
          showVersion(argv[0]);
          return XMLWF_EXIT_SUCCESS;
        }
      }
      j++;
    }
    switch (argv[i][j]) {
    case T('r'):
      processFlags &= ~XML_MAP_FILE;
      j++;
      break;
    case T('s'):
      requireStandalone = 1;
      j++;
      break;
    case T('n'):
      useNamespaces = 1;
      j++;
      break;
    case T('p'):
      paramEntityParsing = XML_PARAM_ENTITY_PARSING_ALWAYS;
      /* fall through */
    case T('x'):
      processFlags |= XML_EXTERNAL_ENTITIES;
      j++;
      break;
    case T('w'):
      windowsCodePages = 1;
      j++;
      break;
    case T('m'):
      outputType = 'm';
      j++;
      break;
    case T('c'):
      outputType = 'c';
      useNamespaces = 0;
      j++;
      break;
    case T('t'):
      outputType = 't';
      j++;
      break;
    case T('N'):
      requiresNotations = 1;
      j++;
      break;
    case T('d'):
      XMLWF_SHIFT_ARG_INTO(outputDir, argc, argv, i, j);
      break;
    case T('e'):
      XMLWF_SHIFT_ARG_INTO(encoding, argc, argv, i, j);
      break;
    case T('h'):
      usage(argv[0], XMLWF_EXIT_SUCCESS);
      // usage called exit(..), never gets here
    case T('v'):
      showVersion(argv[0]);
      return XMLWF_EXIT_SUCCESS;
    case T('g'): {
      const XML_Char *valueText = NULL;
      XMLWF_SHIFT_ARG_INTO(valueText, argc, argv, i, j);

      errno = 0;
      XML_Char *afterValueText = (XML_Char *)valueText;
      const long long read_size_bytes_candidate
          = tcstoull(valueText, &afterValueText, 10);
      if ((errno != 0) || (afterValueText[0] != T('\0'))
          || (read_size_bytes_candidate < 1)
          || (read_size_bytes_candidate > (INT_MAX / 2 + 1))) {
        // This prevents tperror(..) from reporting misleading "[..]: Success"
        errno = ERANGE;
        tperror(T("invalid buffer size") T(
            " (needs an integer from 1 to INT_MAX/2+1 i.e. 1,073,741,824 on most platforms)"));
        exit(XMLWF_EXIT_USAGE_ERROR);
      }
      g_read_size_bytes = (int)read_size_bytes_candidate;
      break;
    }
    case T('k'):
      continueOnError = 1;
      j++;
      break;
    case T('a'): {
      const XML_Char *valueText = NULL;
      XMLWF_SHIFT_ARG_INTO(valueText, argc, argv, i, j);

      errno = 0;
      XML_Char *afterValueText = NULL;
      attackMaximumAmplification = tcstof(valueText, &afterValueText);
      if ((errno != 0) || (afterValueText[0] != T('\0'))
          || isnan(attackMaximumAmplification)
          || (attackMaximumAmplification < 1.0f)) {
        // This prevents tperror(..) from reporting misleading "[..]: Success"
        errno = ERANGE;
        tperror(T("invalid amplification limit") T(
            " (needs a floating point number greater or equal than 1.0)"));
        exit(XMLWF_EXIT_USAGE_ERROR);
      }
#if XML_GE == 0
      ftprintf(stderr,
               T("Warning: Given amplification limit ignored")
                   T(", xmlwf has been compiled without DTD/GE support.\n"));
#endif
      break;
    }
    case T('b'): {
      const XML_Char *valueText = NULL;
      XMLWF_SHIFT_ARG_INTO(valueText, argc, argv, i, j);

      errno = 0;
      XML_Char *afterValueText = (XML_Char *)valueText;
      attackThresholdBytes = tcstoull(valueText, &afterValueText, 10);
      if ((errno != 0) || (afterValueText[0] != T('\0'))) {
        // This prevents tperror(..) from reporting misleading "[..]: Success"
        errno = ERANGE;
        tperror(T("invalid ignore threshold")
                    T(" (needs an integer from 0 to 2^64-1)"));
        exit(XMLWF_EXIT_USAGE_ERROR);
      }
      attackThresholdGiven = XML_TRUE;
#if XML_GE == 0
      ftprintf(stderr,
               T("Warning: Given attack threshold ignored")
                   T(", xmlwf has been compiled without DTD/GE support.\n"));
#endif
      break;
    }
    case T('q'): {
      disableDeferral = XML_TRUE;
      j++;
      break;
    }
    case T('\0'):
      if (j > 1) {
        i++;
        j = 0;
        break;
      }
      /* fall through */
    default:
      usage(argv[0], XMLWF_EXIT_USAGE_ERROR);
      // usage called exit(..), never gets here
    }
  }
  if (i == argc) {
    useStdin = 1;
    processFlags &= ~XML_MAP_FILE;
    i--;
  }
  for (; i < argc; i++) {
    XML_Char *outName = 0;
    int result;
    XML_Parser parser;
    if (useNamespaces)
      parser = XML_ParserCreateNS(encoding, NSSEP);
    else
      parser = XML_ParserCreate(encoding);

    if (! parser) {
      tperror(T("Could not instantiate parser"));
      exit(XMLWF_EXIT_INTERNAL_ERROR);
    }

    if (attackMaximumAmplification != -1.0f) {
#if XML_GE == 1
      XML_SetBillionLaughsAttackProtectionMaximumAmplification(
          parser, attackMaximumAmplification);
#endif
    }
    if (attackThresholdGiven) {
#if XML_GE == 1
      XML_SetBillionLaughsAttackProtectionActivationThreshold(
          parser, attackThresholdBytes);
#else
      (void)attackThresholdBytes; // silence -Wunused-but-set-variable
#endif
    }

    if (disableDeferral) {
      const XML_Bool success = XML_SetReparseDeferralEnabled(parser, XML_FALSE);
      if (! success) {
        // This prevents tperror(..) from reporting misleading "[..]: Success"
        errno = EINVAL;
        tperror(T("Failed to disable reparse deferral"));
        exit(XMLWF_EXIT_INTERNAL_ERROR);
      }
    }

    if (requireStandalone)
      XML_SetNotStandaloneHandler(parser, notStandalone);
    XML_SetParamEntityParsing(parser, paramEntityParsing);
    if (outputType == 't') {
      /* This is for doing timings; this gives a more realistic estimate of
         the parsing time. */
      outputDir = 0;
      XML_SetElementHandler(parser, nopStartElement, nopEndElement);
      XML_SetCharacterDataHandler(parser, nopCharacterData);
      XML_SetProcessingInstructionHandler(parser, nopProcessingInstruction);
    } else if (outputDir) {
      const XML_Char *delim = T("/");
      const XML_Char *file = useStdin ? T("STDIN") : argv[i];
      if (! useStdin) {
        /* Jump after last (back)slash */
        const XML_Char *lastDelim = tcsrchr(file, delim[0]);
        if (lastDelim)
          file = lastDelim + 1;
#if defined(_WIN32)
        else {
          const XML_Char *winDelim = T("\\");
          lastDelim = tcsrchr(file, winDelim[0]);
          if (lastDelim) {
            file = lastDelim + 1;
            delim = winDelim;
          }
        }
#endif
      }
      outName = (XML_Char *)malloc((tcslen(outputDir) + tcslen(file) + 2)
                                   * sizeof(XML_Char));
      if (! outName) {
        tperror(T("Could not allocate memory"));
        exit(XMLWF_EXIT_INTERNAL_ERROR);
      }
      tcscpy(outName, outputDir);
      tcscat(outName, delim);
      tcscat(outName, file);
      userData.fp = tfopen(outName, T("wb"));
      if (! userData.fp) {
        tperror(outName);
        exitCode = XMLWF_EXIT_OUTPUT_ERROR;
        free(outName);
        XML_ParserFree(parser);
        if (continueOnError) {
          continue;
        } else {
          break;
        }
      }
      setvbuf(userData.fp, NULL, _IOFBF, 16384);
#ifdef XML_UNICODE
      puttc(0xFEFF, userData.fp);
#endif
      XML_SetUserData(parser, &userData);
      switch (outputType) {
      case 'm':
        XML_UseParserAsHandlerArg(parser);
        XML_SetElementHandler(parser, metaStartElement, metaEndElement);
        XML_SetProcessingInstructionHandler(parser, metaProcessingInstruction);
        XML_SetCommentHandler(parser, metaComment);
        XML_SetCdataSectionHandler(parser, metaStartCdataSection,
                                   metaEndCdataSection);
        XML_SetCharacterDataHandler(parser, metaCharacterData);
        XML_SetDoctypeDeclHandler(parser, metaStartDoctypeDecl,
                                  metaEndDoctypeDecl);
        XML_SetEntityDeclHandler(parser, metaEntityDecl);
        XML_SetNotationDeclHandler(parser, metaNotationDecl);
        XML_SetNamespaceDeclHandler(parser, metaStartNamespaceDecl,
                                    metaEndNamespaceDecl);
        metaStartDocument(parser);
        break;
      case 'c':
        XML_UseParserAsHandlerArg(parser);
        XML_SetDefaultHandler(parser, markup);
        XML_SetElementHandler(parser, defaultStartElement, defaultEndElement);
        XML_SetCharacterDataHandler(parser, defaultCharacterData);
        XML_SetProcessingInstructionHandler(parser,
                                            defaultProcessingInstruction);
        break;
      default:
        if (useNamespaces)
          XML_SetElementHandler(parser, startElementNS, endElementNS);
        else
          XML_SetElementHandler(parser, startElement, endElement);
        XML_SetCharacterDataHandler(parser, characterData);
#ifndef W3C14N
        XML_SetProcessingInstructionHandler(parser, processingInstruction);
        if (requiresNotations) {
          XML_SetDoctypeDeclHandler(parser, startDoctypeDecl, endDoctypeDecl);
          XML_SetNotationDeclHandler(parser, notationDecl);
        }
#endif /* not W3C14N */
        break;
      }
    }
    if (windowsCodePages)
      XML_SetUnknownEncodingHandler(parser, unknownEncoding, 0);
    result = XML_ProcessFile(parser, useStdin ? NULL : argv[i], processFlags);
    if (outputDir) {
      if (outputType == 'm')
        metaEndDocument(parser);
      fclose(userData.fp);
      if (! result) {
        tremove(outName);
      }
      free(outName);
    }
    XML_ParserFree(parser);
    if (! result) {
      exitCode = XMLWF_EXIT_NOT_WELLFORMED;
      cleanupUserData(&userData);
      if (! continueOnError) {
        break;
      }
    }
  }
  return exitCode;
}
