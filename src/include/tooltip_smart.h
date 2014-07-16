#ifndef __TOOLTIP_SMART_H
#define __TOOLTIP_SMART_H
#include <Elementary.h>

EAPI Evas_Object*             tooltip_object_add(Evas_Object *parent);
EAPI void                     tooltip_object_text_set(Evas_Object *tooltip, Evas_Object *eventarea, const char *text);
#endif
