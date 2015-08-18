/*
 *  lbs-setting
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Kyoungjun Sung <kj7.sung@samsung.com>, Young-Ae Kang <youngae.kang@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "lbs-setting-help.h"
#include "lbs-setting-common.h"

#define HELP_TITLE_STYLE "groupindex"
#define HELP_TEXT_STYLE "1line"
#define HELP_SEPERATOR_STYLE "dialogue/separator"
#define HELP_ITEM_NUM 2
#define HELP_SUBITEM_NUM 6

typedef struct _help_item_info_s {
	int text_index;
} help_item_info;

typedef struct _help_genlist_item_s {
	Elm_Gen_Item_Class *itc_separator[HELP_ITEM_NUM];
	Elm_Gen_Item_Class *itc_help_title[HELP_ITEM_NUM];
	Elm_Gen_Item_Class *itc_help_text[HELP_ITEM_NUM];
} help_genlist_item;

enum {
	HELP_GPS_TITLE,
	HELP_GPS_TEXT,
	HELP_WIRELESS_TIILE,
	HELP_WIRELESS_TEXT,
};
help_item_info itc_item[] = {
	{HELP_GPS_TITLE},
	{HELP_GPS_TEXT},
	{HELP_WIRELESS_TIILE},
	{HELP_WIRELESS_TEXT}
};

static char *help_info[] = {
	N_("IDS_ST_BODY_GPS"),
	N_("IDS_ST_BODY_ALLOW_APPLICATIONS_TO_USE_GPS_SATELLITES_TO_PINPOINT_YOUR_LOCATION"),
	N_("IDS_ST_BODY_WIRELESS_NETWORKS_ABB"),
	N_("IDS_ST_BODY_USE_WI_FI_OR_MOBILE_NETWORKS_TO_HELP_FIND_YOUR_LOCATION_EVEN_IF_YOU_ARE_INDOORS_USING_MOBILE_NETWORKS_MAY_RESULT_IN_ADDITIONAL_CHARGES_MSG")
};


static void _setting_location_free_itc(Elm_Genlist_Item_Class *itc)
{
	if (itc) {
		itc->item_style = NULL;
		itc->func.text_get = NULL;
		itc->func.content_get = NULL;
		itc->func.state_get = NULL;
		itc->func.del = NULL;
		elm_genlist_item_class_free(itc);
		itc = NULL;
	}
}

static void __setting_location_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;
	elm_naviframe_item_pop(ad->nf);
}

static Eina_Bool _pop_cb(void *data, Elm_Object_Item *it)
{
	LS_LOGD("_pop_cb,try to free the memory");
	int index;
	if (data != NULL) {
		help_genlist_item *genlist_item = (help_genlist_item *)data;
		for (index = 0; index < HELP_ITEM_NUM; index++) {
			if (genlist_item->itc_separator[index] != NULL) {
				_setting_location_free_itc(genlist_item->itc_separator[index]);
			}
			if (genlist_item->itc_help_title[index] != NULL) {
				_setting_location_free_itc(genlist_item->itc_help_title[index]);
			}
			if (genlist_item->itc_help_text[index] != NULL) {
				_setting_location_free_itc(genlist_item->itc_help_text[index]);
			}
		}
		free(genlist_item);
		genlist_item = NULL;
	}
	return EINA_TRUE;
}

void __setting_location_help_create_seperator(Evas_Object *genlist, Elm_Gen_Item_Class *itc_separator)
{
	LS_RETURN_IF_FAILED(genlist);
	LS_RETURN_IF_FAILED(itc_separator);
	Elm_Object_Item *item = NULL;

	itc_separator->item_style = HELP_SEPERATOR_STYLE;
	itc_separator->func.text_get = NULL;
	itc_separator->func.content_get = NULL;
	itc_separator->func.state_get = NULL;
	itc_separator->func.del = NULL;

	item = elm_genlist_item_append(genlist, itc_separator, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
}

static char *_gps_help_text_get(void *data, Evas_Object *obj, const char *part)
{
	if (!g_strcmp0(part, "elm.text.main")) {
		return strdup(P_(help_info[0]));
	} else if (!g_strcmp0(part, "elm.text.multiline")) {
		return strdup(P_(help_info[1]));
	}
	return NULL;
}

static char *_wps_help_text_get(void *data, Evas_Object *obj, const char *part)
{
	if (!g_strcmp0(part, "elm.text.main")) {
		return strdup(P_(help_info[2]));
	} else if (!g_strcmp0(part, "elm.text.multiline")) {
		return strdup(P_(help_info[3]));
	}
	return NULL;
}

void _setting_location_help_view(void *data)
{
	LS_RETURN_IF_FAILED(data);

	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	Elm_Object_Item *navi_it = NULL;
	Evas_Object *genlist;
	Elm_Gen_Item_Class *itc_help_gps;
	Elm_Gen_Item_Class *itc_help_wps;

	genlist = elm_genlist_add(ad->nf);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	help_genlist_item *genlist_item = calloc(sizeof(help_genlist_item), 1);

	itc_help_gps = elm_genlist_item_class_new();
	if (itc_help_gps == NULL) {
		g_free(genlist_item);
		LS_LOGE("critical error : LS_RETURN_VAL_IS_FAILED");
		return;
	}

	itc_help_gps->item_style = "multiline_sub.main";
	itc_help_gps->func.text_get = _gps_help_text_get;
	itc_help_gps->func.content_get = NULL;
	itc_help_gps->func.state_get = NULL;
	itc_help_gps->func.del = NULL;
	Elm_Object_Item *item_gps = elm_genlist_item_append(genlist, itc_help_gps, (void *)&itc_item[0], NULL, ELM_GENLIST_ITEM_NONE, NULL, ad);
	elm_genlist_item_select_mode_set(item_gps, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	itc_help_wps = elm_genlist_item_class_new();
	if (itc_help_wps == NULL) {
		g_free(itc_help_gps);
		g_free(genlist_item);
		LS_LOGE("critical error : LS_RETURN_VAL_IS_FAILED");
		return;
	}

	itc_help_wps->item_style = "multiline_sub.main";
	itc_help_wps->func.text_get = _wps_help_text_get;
	itc_help_wps->func.content_get = NULL;
	itc_help_wps->func.state_get = NULL;
	itc_help_wps->func.del = NULL;
	Elm_Object_Item *item_wps = elm_genlist_item_append(genlist, itc_help_wps, (void *)&itc_item[2], NULL, ELM_GENLIST_ITEM_NONE, NULL, ad);
	elm_genlist_item_select_mode_set(item_wps, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	evas_object_show(genlist);

	Evas_Object *back_btn = elm_button_add(ad->nf);
	elm_object_style_set(back_btn, "naviframe/back_btn/default");
	evas_object_smart_callback_add(back_btn, "clicked", __setting_location_back_cb, ad);

	navi_it = elm_naviframe_item_push(ad->nf, P_("IDS_ST_HEADER_HELP"), back_btn, NULL, genlist, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, _pop_cb, genlist_item);
}
