Evas_Object *tools_init(Evas_Object *parent);
void tools_term(void);
Evas_Object *tools_live_edit_get(Evas_Object *tools);
void tools_highlight_update(Eina_Bool toggle);
void tools_lines_update(Eina_Bool toggle);
void tools_dummy_update(Eina_Bool toggle);
void tools_status_update(Eina_Bool toggle);
void tools_edc_navigator_update(Eina_Bool toggle);
void tools_goto_update(void);
void tools_search_update(void);
void tools_live_update(Eina_Bool on);
void tools_console_update(Eina_Bool on);
void tools_menu_update(Eina_Bool on);
