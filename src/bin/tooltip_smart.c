#include "tooltip_smart.h"
#include <math.h>
#include <stdio.h>
#include "globals.h"

#define F printf("%s\n", __func__)
#define TOOLTIP_GROUP_NAME "edje/tooltip/base/default"

#define EVAS_OBJECT_SMART_TOOLTIPS_DATA_GET(o, sd) \
   Smart_Tooltip *sd = evas_object_smart_data_get(o);

static Evas_Smart *_smart = NULL;
static Evas_Smart_Class _parent_sc = EVAS_SMART_CLASS_INIT_NULL;
typedef Evas_Object * (*Tooltip_Content_Cb)(void *, Evas_Object *, Evas_Object *);

typedef struct _Smart_Tooltip
{
   Evas_Object_Smart_Clipped_Data __clipped_data;
   Evas_Object *rect;
   Evas_Object *parent;
   Evas_Object *eventarea;
   const void  *data;

   Tooltip_Content_Cb func;
   Evas_Smart_Cb del_cb;
   Ecore_Job *reconf_job;
} Smart_Tooltip;

static void
_smart_add(Evas_Object *obj)
{
   Evas *evas;
   Evas_Object *o;
   Smart_Tooltip *sd;

   sd = calloc(1, sizeof(Smart_Tooltip));
   EINA_SAFETY_ON_NULL_RETURN(sd);
   evas_object_smart_data_set(obj, sd);

   evas = evas_object_evas_get(obj);
   if (!evas)
     return;

   _parent_sc.add(obj);

   sd->rect = o = edje_object_add(evas);
   edje_object_file_set(o, EDJE_PATH,
                        TOOLTIP_GROUP_NAME);
   evas_object_smart_member_add(o, obj);
}

static void
_smart_del(Evas_Object *obj)
{
   EVAS_OBJECT_SMART_TOOLTIPS_DATA_GET(obj, sd);

   if (!sd) return;
   if (sd->rect) evas_object_del(sd->rect);
   _parent_sc.del(obj);
}

static void
_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   EVAS_OBJECT_SMART_TOOLTIPS_DATA_GET(obj, sd);
   evas_object_resize(sd->rect, w, h);
}

static void
_smart_calculate(Evas_Object *obj)
{
}

static void
_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   EVAS_OBJECT_SMART_TOOLTIPS_DATA_GET(obj, sd);
   if (!sd) return;
   if (sd->rect) evas_object_move(sd->rect, x, y);
}

static void
_smart_hide(Evas_Object *obj)
{
   EVAS_OBJECT_SMART_TOOLTIPS_DATA_GET(obj, sd);

   _parent_sc.hide(obj);
   evas_object_hide(sd->rect);
}

static void
_smart_show(Evas_Object *obj)
{
   EVAS_OBJECT_SMART_TOOLTIPS_DATA_GET(obj, sd);
   _parent_sc.show(obj);

   evas_object_show(sd->rect);
}

static void
_smart_init(void)
{
   static Evas_Smart_Class sc;

   evas_object_smart_clipped_smart_set(&_parent_sc);
   sc = _parent_sc;
   sc.name = "Tooltips";
   sc.version = EVAS_SMART_CLASS_VERSION;
   sc.add = _smart_add;
   sc.del = _smart_del;
   sc.resize = _smart_resize;
   sc.move = _smart_move;
   sc.calculate = _smart_calculate;
   sc.show = _smart_show;
   sc.hide = _smart_hide;
   _smart = evas_smart_class_new(&sc);
}

static void
_obj_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Smart_Tooltip *sd = data;
   F;
}

static void
_parent_mouse_in_cb(void *data, Evas *e,
                    Evas_Object *obj, void *event_info)
{
   Evas_Object *tooltip = data;
   F;

   Evas_Event_Mouse_In *mouse_in = event_info;
   evas_object_move(tooltip, mouse_in->canvas.x, mouse_in->canvas.y);
   evas_object_show(tooltip);
}

static void
_parent_mouse_out_cb(void *data, Evas *e,
                     Evas_Object *obj, void *event_info)
{
   evas_object_hide(data);
}

EAPI Evas_Object *
tooltip_object_add(Evas_Object *parent)
{
   Evas *e;
   Evas_Object *obj;
   Smart_Tooltip *sd;

   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);

   e = evas_object_evas_get(parent);
   if (!e) return NULL;
   if (!_smart) _smart_init();

   obj = evas_object_smart_add(e, _smart);
   sd = evas_object_smart_data_get(obj);

   evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL,
                                  _obj_del_cb, sd);
   sd->parent = parent;
   sd->eventarea = NULL;

   return obj;
}

static void _tooltip_unset(Evas_Object *);
static void
_parent_free_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
   F;
   Evas_Object *tooltip = data;
   EVAS_OBJECT_SMART_TOOLTIPS_DATA_GET(tooltip, sd);
   _tooltip_unset(tooltip);
}

static void
_tooltip_unset(Evas_Object *tooltip)
{
   EVAS_OBJECT_SMART_TOOLTIPS_DATA_GET(tooltip, sd);
   if (sd->eventarea)
     {
        evas_object_event_callback_del_full(sd->eventarea,
                                            EVAS_CALLBACK_MOUSE_IN,
                                            _parent_mouse_in_cb, tooltip);
        evas_object_event_callback_del_full(sd->eventarea, EVAS_CALLBACK_MOUSE_OUT,
                                            _parent_mouse_out_cb, tooltip);
        evas_object_event_callback_del_full(sd->eventarea, EVAS_CALLBACK_FREE,
                                            _parent_free_cb, tooltip);
     }
   if (sd->parent)
     {
        evas_object_event_callback_del_full(sd->parent, EVAS_CALLBACK_FREE,
                                            _parent_free_cb, tooltip);
     }

   sd->del_cb((void *)sd->data, tooltip, NULL);
   evas_object_del(tooltip);
}

static void
_tooltip_reconfigure_job(void *data)
{
   Evas_Object *tooltip = data;
   Evas_Coord lw, lh;
   EVAS_OBJECT_SMART_TOOLTIPS_DATA_GET(tooltip, sd);

   Evas_Object *label = sd->func((void *)sd->data, sd->eventarea, tooltip);
   evas_object_size_hint_min_get(label, &lw, &lh);
   evas_object_resize(tooltip, lw, lh);
   edje_object_part_swallow(sd->rect, "tooltip.swallow", label);
}

static void
_tooltip_reconfigure_job_start(Evas_Object *tooltip)
{
   EVAS_OBJECT_SMART_TOOLTIPS_DATA_GET(tooltip, sd);

   ecore_job_del(sd->reconf_job);
   sd->reconf_job = ecore_job_add(_tooltip_reconfigure_job, tooltip);
}

static void
_tooltip_content_cb_set(Evas_Object *tooltip, Evas_Object *eventarea,
                        Tooltip_Content_Cb func,
                        const void *data, Evas_Smart_Cb del_cb)
{
   EVAS_OBJECT_SMART_TOOLTIPS_DATA_GET(tooltip, sd);

   EINA_SAFETY_ON_NULL_GOTO(tooltip, error);
   EINA_SAFETY_ON_NULL_GOTO(eventarea, error);

   if (!func)
     {
        _tooltip_unset(tooltip);
        return;
     }

   if (sd->eventarea && (sd->eventarea == eventarea)) return;
   if (sd->eventarea != eventarea)
     {
        sd->eventarea = eventarea;
        evas_object_event_callback_add(sd->eventarea,
                                       EVAS_CALLBACK_MOUSE_IN,
                                       _parent_mouse_in_cb, tooltip);
        evas_object_event_callback_add(sd->eventarea,
                                       EVAS_CALLBACK_MOUSE_OUT,
                                       _parent_mouse_out_cb, tooltip);
        evas_object_event_callback_add(sd->eventarea,
                                       EVAS_CALLBACK_FREE,
                                       _parent_free_cb, tooltip);
        sd->func = func;
        sd->data = data;
        sd->del_cb = del_cb;

        _tooltip_reconfigure_job_start(tooltip);
     }

 error:
   if (del_cb) del_cb((void *)data, tooltip, NULL);
}

static Evas_Object *
_tooltip_label_create(void *data, Evas_Object *obj,
                      Evas_Object *tooltip)
{
   EVAS_OBJECT_SMART_TOOLTIPS_DATA_GET(tooltip, sd);
   F;

   Evas_Object *label = elm_label_add(tooltip);
   elm_object_style_set(label, "tooltip");
   elm_object_text_set(label, data);

   return label;
}

static void
_tooltip_label_del(void *data, Evas_Object *obj, void *event_info)
{
   F;
   eina_stringshare_del(data);
}

EAPI void
tooltip_object_text_set(Evas_Object *tooltip, Evas_Object *eventarea,
                        const char *text)
{
   text = eina_stringshare_add(text);

   _tooltip_content_cb_set(tooltip, eventarea, _tooltip_label_create,
                           text, _tooltip_label_del);
}

#undef TOOLTIP_GROUP_NAME
