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

#ifndef LBS_SETTING_COMMON_H_
#define LBS_SETTING_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <app.h>
#include <dlog.h>
#include <glib.h>
#ifndef Eina_Bool
#include <stdbool.h>
#endif
#include <stdio.h>
#include <string.h>
#include <efl_extension.h>
#include <locations.h>

#if !defined(LBS_SETTING_PKG)
#define LBS_SETTING_PKG "org.tizen.setting-location"
#endif

#define DOMAIN_NAME LBS_SETTING_PKG

#define TAG_LBS_SETTING "LBS_SETTING"
#define LBS_SETTING_DLOG_DEBUG

#ifdef LBS_SETTING_DLOG_DEBUG		/**< if debug mode, show filename & line number */

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG TAG_LBS_SETTING
#endif

#define LS_LOGD(fmt,args...) LOGD(fmt, ##args)
#define LS_LOGW(fmt,args...) LOGW(fmt, ##args)
#define LS_LOGI(fmt,args...) LOGI(fmt, ##args)
#define LS_LOGE(fmt,args...) LOGE(fmt, ##args)

#elif LBS_SETTING_DLOG_RELEASE	/* if release mode */

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG TAG_LBS_SETTING
#endif

#define LS_LOGD(fmt,args...) LOGD(fmt, ##args)
#define LS_LOGW(fmt,args...) LOGW(fmt, ##args)
#define LS_LOGI(fmt,args...) LOGI(fmt, ##args)
#define LS_LOGE(fmt,args...) LOGE(fmt, ##args)
#else						/* if do not use dlog */
#define LS_LOGD(...) g_debug(__VA_ARGS__)
#define LS_LOGW(...) g_warning(__VA_ARGS__)
#define LS_LOGI(...) g_message(__VA_ARGS__)
#define LS_LOGE(...) g_error(__VA_ARGS__)
#endif

#define P_(s)			dgettext(LBS_SETTING_PKG, s)
#define S_(s)			dgettext("sys_string", s)
#define dgettext_noop(s)	(s)
#define N_(s)			dgettext_noop(s)

#define KEY_ENABLED	1
#define KEY_DISABLED 0

#define LS_FUNC_ENTER	LS_LOGD("(%s) ENTER", __FUNCTION__);
#define LS_FUNC_EXIT	LS_LOGD("(%s) EXIT", __FUNCTION__);

#define LBS_SETTING_EDJ EDJ_DIR"/setting.edj"

#define COLOR_TABLE "/usr/apps/org.tizen.setting-location/res/lbs_setting_ChangeableColorInfo.xml"
#define FONT_TABLE "/usr/apps/org.tizen.setting-location/res/lbs_setting_ChangeableFontInfo.xml"

#define _EDJ(o)	elm_layout_edje_get(o)
#define SAFE_STRDUP(src) (src) ? strdup(src) : NULL

#define LS_MEM_FREE(ptr)	\
	do { \
		if (ptr != NULL) {	\
			free((void *)ptr);	\
			ptr = NULL;	\
		}	\
	} while (0)


#define LS_MEM_NEW(ptr, num_elements, type)	 \
	do { \
		if ((int)(num_elements) <= 0) { \
			ptr = NULL; \
		} else { \
			ptr = (type *) calloc(num_elements, sizeof(type)); \
		} \
	} while (0)


#define LS_RETURN_IF_FAILED(point) do { \
		if (point == NULL) { \
			LS_LOGE("critical error : LS_RETURN_IF_FAILED"); \
			return; \
		} \
	} while (0)

#define LS_RETURN_VAL_IF_FAILED(point, val) do { \
		if (point == NULL) { \
			LS_LOGE("critical error : LS_RETURN_VAL_IS_FAILED"); \
			return val; \
		} \
	} while (0)

typedef enum {
	GENLIST_INDEX_SEP = 0,		/** separator index */
	GENLIST_INDEX_TOP,			/** top index */
	GENLIST_INDEX_CENTER,		/** center index */
	GENLIST_INDEX_BOTTOM,		/** bottom index */
	GENLIST_INDEX_INVALID,		/** invalid index */
}
loc_genlist_index_e;


typedef struct appdata {
	Evas_Object *ug;
	Evas_Object *wizard_layout;
	Evas_Object *prev_button;
	Evas_Object *next_button;
	Evas_Object *allow_button;
	Evas_Object *info_label;
	Evas_Object *title_label;
	Evas_Object *focus_title;
	Evas_Object *focus_help;
	Evas_Object *focus_help_sub;

	Evas_Object *win_main;
	Evas_Object *conformant;
	Evas_Object *bg;
	Evas_Object *layout_main;
	Evas_Object *nf;
	Evas_Object *app_popup;

	Evas_Object *help_scroller;
	Evas_Object *allow_scroller;

	Evas_Object *base;
	Evas_Object *genlist;
	Evas_Object *gps_popup;
	Evas_Object *wifi_popup;
	Evas_Object *ctx_popup;
	Evas_Object *disagree_btn[3];
	Evas_Object *agree_btn[3];

	Evas_Object *gi_loc_check;
	Evas_Object *gi_gps_check;
	Evas_Object *gi_wifi_check;
	Evas_Object *gi_hereuc_check;
	Evas_Object *wizard_check;

	Elm_Object_Item *gi_loc, *gi_gps, *gi_wifi, *gi_group_title, *gi_myplace_title, *gi_myplace, *gi_hereuc_title, *gi_hereuc;
	Elm_Genlist_Item_Class *itc_loc, *itc_gps, *itc_wifi,  *itc_title, *itc_myplace_title, *itc_myplace, *itc_hereuc_title, *itc_hereuc;
	bool is_myloc, is_gps, is_wifi, is_hereuc;
	bool quick_panel_setting;
	int view_id;
	unsigned int location_event_req_id, gps_event_req_id;;

	Evas_Object *elm_conform;
	app_control_h prev_handler;
	Evas_Object *loc_check;
	Elm_Object_Item *itc_1;
	Evas_Object *ask_check;

} lbs_setting_app_data;

lbs_setting_app_data *lbs_setting_common_get_app_data(void);
void lbs_setting_common_destroy_app_data(void);

#ifdef __cplusplus
}
#endif /*__cplusplus */


#endif /* LBS_SETTING_COMMON_H_ */
