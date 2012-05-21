/*********************************************************************

    coco_cas.h

    Format code for CoCo CAS (*.cas) files

*********************************************************************/

#ifndef COCO_CAS_H
#define COCO_CAS_H

#include "cassimg.h"

extern const struct CassetteFormat coco_cas_format;
extern const struct CassetteModulation coco_cas_modulation;

CASSETTE_FORMATLIST_EXTERN(coco_cassette_formats);
CASSETTE_FORMATLIST_EXTERN(alice32_cassette_formats);

#endif /* COCO_CAS_H */
