#include <Elementary.h>
#include "common.h"

#define COL_NUM 6

typedef struct color_tuple
{
   Eina_Stringshare *key;
   Eina_Stringshare *col;
} color_tuple;

typedef struct color
{
   char *val;
   Eina_List *keys;
} color;

typedef struct syntax_color_group
{
   char *comment;
   char *define;
   color colors[COL_NUM];
} syntax_color_group;

struct syntax_color_s
{
   Eina_Strbuf *strbuf;
   Eina_Strbuf *cachebuf;
   Eina_Hash *color_hash;
   Eina_Stringshare *col_comment;
   Eina_Stringshare *col_define;
   Eina_Stringshare *cols[COL_NUM];
   syntax_color_group *scg;
   Ecore_Thread *thread;

   Eina_Bool ready: 1;
};

static Eet_Data_Descriptor *edd_scg = NULL;
static Eet_Data_Descriptor *edd_color = NULL;

static void
hash_free_cb(void *data)
{
   Eina_Inarray *inarray = data;
   color_tuple *tuple;
   EINA_INARRAY_FOREACH(inarray, tuple)
     eina_stringshare_del(tuple->key);
   eina_inarray_free(inarray);
}

static void
eddc_init()
{
   Eet_Data_Descriptor_Class eddc;
   eet_eina_stream_data_descriptor_class_set(&eddc, sizeof(eddc),
                                             "syntax_color_group",
                                             sizeof(syntax_color_group));
   edd_scg = eet_data_descriptor_stream_new(&eddc);

   eet_eina_stream_data_descriptor_class_set(&eddc, sizeof(eddc), "color",
                                             sizeof(color));
   edd_color = eet_data_descriptor_stream_new(&eddc);

   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_scg, syntax_color_group, "comment",
                                 comment, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_scg, syntax_color_group, "define",
                                 define, EET_T_STRING);

   EET_DATA_DESCRIPTOR_ADD_BASIC(edd_color, color, "val", val, EET_T_STRING);
   EET_DATA_DESCRIPTOR_ADD_LIST_STRING(edd_color, color, "keys", keys);

   EET_DATA_DESCRIPTOR_ADD_ARRAY(edd_scg, syntax_color_group, "colors",
                                 colors, edd_color);
}

static void
eddc_term()
{
   eet_data_descriptor_free(edd_scg);
   eet_data_descriptor_free(edd_color);
}

static void
color_load(color_data *cd)
{
   char buf[PATH_MAX];
   snprintf(buf, sizeof(buf), "%s/color/color.eet", elm_app_data_dir_get());

   Eet_File *ef = eet_open(buf, EET_FILE_MODE_READ);
   if (ef)
     {
        cd->scg = eet_data_read(ef, edd_scg, "color");
        if (!cd->scg) EINA_LOG_ERR("Failed to read syntax color group.");
        eet_close(ef);
     }
   else EINA_LOG_ERR("Failed to open color data file \"%s\"", buf);
}

static void
color_table_init(color_data *cd)
{
   color_tuple *tuple;
   int i;
   Eina_List *l;
   char *key;
   char tmp[2];
   Eina_Inarray *inarray;
   syntax_color_group *scg = cd->scg;

   if (!scg) return;

   cd->col_comment = eina_stringshare_add(scg->comment);
   //free(scg->comment);
   cd->col_define = eina_stringshare_add(scg->define);
   //free(scg->define);

   cd->color_hash = eina_hash_string_small_new(hash_free_cb);

   for (i = 0; i < COL_NUM; i++)
     {
        cd->cols[i] = eina_stringshare_add(scg->colors[i].val);
        //free(scg->colors[i].val);

        EINA_LIST_FOREACH(scg->colors[i].keys, l, key)
          {
             tmp[0] = key[0];
             tmp[1] = '\0';

             inarray = eina_hash_find(cd->color_hash, tmp);
             if (!inarray)
               {
                  inarray = eina_inarray_new(sizeof(color_tuple), 0);
                  eina_hash_add(cd->color_hash, tmp, inarray);
               }

             tuple = malloc(sizeof(color_tuple));
             tuple->col = cd->cols[i];
             tuple->key = eina_stringshare_add(key);
             //free(key);
             eina_inarray_push(inarray, tuple);
          }
        eina_list_free(scg->colors[i].keys);
     }

   free(scg);
   cd->scg = NULL;
}

static void
init_thread_blocking(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   color_data *cd = data;

   eddc_init();
   color_load(cd);
   eddc_term();
   color_table_init(cd);

   cd->thread = NULL;
   cd->ready = EINA_TRUE;
}

color_data *
color_init(Eina_Strbuf *strbuf)
{
   color_data *cd = malloc(sizeof(color_data));
   cd->strbuf = strbuf;
   cd->cachebuf = eina_strbuf_new();
   cd->thread = ecore_thread_run(init_thread_blocking, NULL, NULL, cd);

   return cd;
}

void
color_term(color_data *cd)
{
   ecore_thread_cancel(cd->thread);

   eina_hash_free(cd->color_hash);
   eina_strbuf_free(cd->cachebuf);

   eina_stringshare_del(cd->col_comment);
   eina_stringshare_del(cd->col_define);

   int i;
   for(i = 0; i < COL_NUM; i++)
     eina_stringshare_del(cd->cols[i]);

   free(cd);
}

static Eina_Bool
color_markup_insert(Eina_Strbuf *strbuf, const char **src, int length,
                    char **cur, char **prev,  const char *cmp,
                    Eina_Stringshare *color)
{
   char buf[128];

   eina_strbuf_append_length(strbuf, *prev, *cur - *prev);
   snprintf(buf, sizeof(buf), "<color=#%s>%s</color>", color, cmp);
   eina_strbuf_append(strbuf, buf);
   *cur += strlen(cmp);
   if (*cur > (*src + length)) return EINA_FALSE;
   *prev = *cur;

   return EINA_TRUE;
}

static int
markup_skip(Eina_Strbuf *strbuf, const char **src, int length, char **cur,
            char **prev)
{
   if ((*cur)[0] != '<') return 0;

   eina_strbuf_append_length(strbuf, *prev, (*cur - *prev));
   (*cur)++;

   if (*cur > (*src + length)) return -1;
   *prev = *cur;

   *cur = strchr(*prev, '>');

   if (*cur)
     {
        (*cur)++;
        *prev = *cur;
        return 1;
     }
   else
     {
        *prev = *cur;
        return -1;
     }

   return 1;
}

static int
tab_skip(Eina_Strbuf *strbuf, const char **src, int length, char **cur,
        char **prev)
{
   int cmp_size = 6;    //strlen("<tab/>");
   if (strncmp(*cur, "<tab/>", cmp_size)) return 0;
   eina_strbuf_append_length(strbuf, *prev, (*cur - *prev + cmp_size));
   *cur += cmp_size;
   if (*cur > (*src + length)) return -1;
   *prev = *cur;

   return 1;
}

static int
br_skip(Eina_Strbuf *strbuf, const char **src, int length, char **cur,
        char **prev)
{
   if (strncmp(*cur, EOL, EOL_LEN)) return 0;
   eina_strbuf_append_length(strbuf, *prev, (*cur - *prev + EOL_LEN));
   *cur += EOL_LEN;
   if (*cur > (*src + length)) return -1;
   *prev = *cur;

   return 1;
}

static int
comment_apply(Eina_Strbuf *strbuf, const char **src, int length, char **cur,
              char **prev, const Eina_Stringshare *color,
              Eina_Bool *inside_comment)
{
   if (!(*inside_comment))
     {
        if ((*cur)[0] != '/') return 0;
        if ((*cur) + 1 > ((*src) + length)) return -1;
        if ((*cur)[1] != '*') return 0;

        eina_strbuf_append_length(strbuf, *prev, (*cur - *prev));

        char buf[128];
        snprintf(buf, sizeof(buf), "<color=#%s>/*", color);
        eina_strbuf_append(strbuf, buf);

        int cmp_size = 2;     //strlen("/*");

        *cur += cmp_size;

        if (*cur > (*src + length))
          {
             *inside_comment = EINA_TRUE;
             return -1;
          }

        *prev = *cur;

        *cur = strstr(*prev, "*/");

        if (*cur)
          {
             eina_strbuf_append_length(strbuf, *prev, (*cur - *prev));
             eina_strbuf_append(strbuf, "*/</color>");
             *cur += cmp_size;
             *prev = *cur;
             return 0;
          }

        eina_strbuf_append(strbuf, *prev);
        *prev = *cur;

        *inside_comment = EINA_TRUE;
        return -1;
     }
   else
     {
        if ((*cur)[0] != '*') return 0;
        if ((*cur) + 1 > ((*src) + length)) return -1;
        if ((*cur)[1] != '/') return 0;

        eina_strbuf_append_length(strbuf, *prev, (*cur - *prev));
        eina_strbuf_append(strbuf, "*/</color>");

        int cmp_size = 2;     //strlen("*/");

        *cur += cmp_size;
        *inside_comment = EINA_FALSE;

        if (*cur > (*src + length)) return -1;
        *prev = *cur;
        return 1;
     }

   return -1;
}

static int
comment2_apply(Eina_Strbuf *strbuf, const char **src, int length, char **cur,
               char **prev, const Eina_Stringshare *color,
               Eina_Bool *inside_comment)
{
   if (*inside_comment) return 0;
   if ((*cur)[0] != '/') return 0;
   if (((*cur) + 1) > ((*src) + length)) return -1;
   if ((*cur)[1] != '/') return 0;

   eina_strbuf_append_length(strbuf, *prev, (*cur - *prev));

   char buf[128];
   snprintf(buf, sizeof(buf), "<color=#%s>//", color);
   eina_strbuf_append(strbuf, buf);

   int cmp_size = 2;    //strlen("//");
   *cur += cmp_size;

   if (*cur > (*src + length))
     {
        eina_strbuf_append(strbuf, "</color>");
        return -1;
     }

   *prev = *cur;

   *cur = strstr(*prev, EOL);

   if (*cur)
     {
        eina_strbuf_append_length(strbuf, *prev, (*cur - *prev));
        eina_strbuf_append(strbuf, "</color><br/>");
        *cur += EOL_LEN;
        *prev = *cur;
        return 1;
     }

   eina_strbuf_append(strbuf, *prev);
   *prev = *cur;

   eina_strbuf_append(strbuf, "</color>");
   return -1;
}

static int
sharp_apply(Eina_Strbuf *strbuf, const char **src, int length, char **cur,
            char **prev, const Eina_Stringshare *color)
{
   if ((*cur)[0] != '#') return 0;

   eina_strbuf_append_length(strbuf, *prev, (*cur - *prev));

   char buf[128];
   snprintf(buf, sizeof(buf), "<color=#%s>#", color);
   eina_strbuf_append(strbuf, buf);

   int cmp_size = 1;    //strlen("#");
   *cur += cmp_size;

   if (*cur > (*src + length))
     {
        eina_strbuf_append(strbuf, "</color>");
        return -1;
     }

   *prev = *cur;

   char *space = strstr(*prev, " ");
   char *eol = strstr(*prev, EOL);

   if (space < eol)
     {
        *cur = space;
        cmp_size = 1; //strlen(" ");
     }
   else
     {
        *cur = eol;
        cmp_size = EOL_LEN;
     }

   if (*cur)
     {
        eina_strbuf_append_length(strbuf, *prev, (*cur - *prev));
        if (space < eol) eina_strbuf_append(strbuf, "</color> ");
        else eina_strbuf_append(strbuf, "</color><br/>");
        *cur += cmp_size;
        *prev = *cur;
        return 1;
     }

   eina_strbuf_append(strbuf, *prev);
   *prev = *cur;

   eina_strbuf_append(strbuf, "</color>");
   return -1;
}

const char *
color_cancel(color_data *cd, const char *src, int length)
{
   if (!src || length < 1) return NULL;
   Eina_Strbuf *strbuf = cd->strbuf;
   eina_strbuf_reset(strbuf);

   const char *str = NULL;
   char *prev = (char *) src;
   char *cur = (char *) src;

   while (cur && (cur <= (src + length)))
     {
        //escape EOL: <br/>
        if (br_skip(strbuf, &src, length, &cur, &prev) == 1)
          continue;

        //escape EOL: <tab/>
        if (tab_skip(strbuf, &src, length, &cur, &prev) == 1)
          continue;

        //escape markups: <..> ~ </..> 
        if (markup_skip(strbuf, &src, length, &cur, &prev) == 1)
          continue;

        cur++;
     }

   //Same with origin source.
   if (prev == src)
     str = src;
   //Some color syntax is applied.
   else
     {
        //append leftovers.
        if (prev + 1 < cur) eina_strbuf_append(strbuf, prev);
        str = eina_strbuf_string_get(strbuf);
     }
   return str;
}

static int
bracket_escape(Eina_Strbuf *strbuf, char **cur, char **prev)
{
   if ((*cur)[0] != '&') return 0;
   int cmp_size = 4;

   if (!strncmp(*cur, "&lt;", cmp_size))
     {
        eina_strbuf_append_length(strbuf, *prev, *cur - *prev);
        eina_strbuf_append(strbuf, "&lt;");
        *cur += cmp_size;
        *prev = *cur;
        return 1;
     }
   else if (!strncmp(*cur, "&gt;", cmp_size))
     {
        eina_strbuf_append_length(strbuf, *prev, *cur - *prev);
        eina_strbuf_append(strbuf, "&gt;");
        *cur += cmp_size;
        *prev = *cur;
        return 1;
     }
   return 0;
}

/* 
	OPTIMIZATION POINT 
	1. Apply Color only changed line.
*/
const char *
color_apply(color_data *cd, const char *src, int length)
{
   Eina_Bool inside_string = EINA_FALSE;
   Eina_Bool inside_comment = EINA_FALSE;

   if (!src || (length < 1)) return NULL;

   Eina_Strbuf *strbuf = cd->cachebuf;
   eina_strbuf_reset(strbuf);

   const char *str = NULL;
   char *prev = (char *) src;
   char *cur = (char *) src;
   int ret;
   Eina_Inarray *inarray;
   color_tuple *tuple;

   while (cur && (cur <= (src + length)))
     {
        //escape empty string
        if (cur[0] == ' ')
          {
             eina_strbuf_append_length(strbuf, prev, cur - prev);
             eina_strbuf_append_char(strbuf, ' ');
             ++cur;
             prev = cur;
             continue;
          }

        //handle comment: /* ~ */
        ret = comment_apply(strbuf, &src, length, &cur, &prev, cd->col_comment,
                            &inside_comment);
        if (ret == 1) continue;
        else if (ret == -1) goto finished;

        //handle comment: //
        ret = comment2_apply(strbuf, &src, length, &cur, &prev, cd->col_comment,
                             &inside_comment);
        if (ret == 1) continue;
        else if (ret == -1) goto finished;

        //escape string: " ~ "
        if (cur[0] == QUOT_C)
          {
             eina_strbuf_append_length(strbuf, prev, cur - prev);
             eina_strbuf_append_char(strbuf, QUOT_C);
             cur++;
             prev = cur;
             inside_string = !inside_string;
             continue;
          }
        if (inside_string || inside_comment)
          {
             cur++;
             continue;
          }

        //FIXME: This might be textblock problem. should be removed here.
        //escape <> bracket.
        ret = bracket_escape(strbuf, &cur, &prev);
        if (ret == 1) continue;

        //handle comment: #
        ret = sharp_apply(strbuf, &src, length, &cur, &prev, cd->col_define);
        if (ret == 1) continue;
        else if (ret == -1) goto finished;

        char key[2];
        key[0] = cur[0];
        key[1] = '\0';
        inarray = eina_hash_find(cd->color_hash, key);

        //Found tuple list. Search in detail.
        if (inarray)
          {
             Eina_Bool found = EINA_FALSE;

             EINA_INARRAY_FOREACH(inarray, tuple)
               {
                  if (!strncmp(cur, tuple->key, strlen(tuple->key)))
                    {
                       ret = color_markup_insert(strbuf, &src, length, &cur,
                                                 &prev, tuple->key, tuple->col);
                       if (ret)
                         {
                            found = EINA_TRUE;
                            break;
                         }
                       else goto finished;
                    }
               }
             if (found) continue;
          }
        cur++;
     }

   //Same with origin source.
   if (prev == src)
     str = src;
   //Some color syntax is applied.
   else
     {
finished:
        //append leftovers.
        if (prev + 1 < cur) eina_strbuf_append(strbuf, prev);
        str = eina_strbuf_string_get(strbuf);
     }

   return str;
}

Eina_Bool
color_ready(color_data *cd)
{
   return cd->ready;
}
