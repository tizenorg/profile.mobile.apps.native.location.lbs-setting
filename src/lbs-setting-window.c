/*
 * lbs-setting
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

#include <app.h>
#include <ui-gadget.h>
#include <glib.h>
#include <vconf.h>
#include <Elementary.h>
#include <lbs-setting-common.h>
#include <libintl.h>
#include <locations.h>
#include <dlog.h>
#include <efl_extension.h>
#include <bundle_internal.h>
#include <system_info.h>
#include "lbs-setting-common.h"
#include "lbs-setting-window.h"
#include "lbs-setting-help.h"
#include <context_places_internal.h>

static void _anchor_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void __setting_reply_gps_wifi_status(void *data);
void location_wifi_popup(void *data);

Evas_Object *create_indicator_bg(Evas_Object *parent)
{
	LS_FUNC_ENTER
	Evas_Object *indicator_bg = NULL;
	indicator_bg = elm_bg_add(parent);
	elm_object_style_set(indicator_bg, "indicator/headerbg");
	elm_object_part_content_set(parent, "elm.swallow.indicator_bg", indicator_bg);
	evas_object_show(indicator_bg);
	LS_FUNC_EXIT
	return indicator_bg;
}

Evas_Object *create_bg(Evas_Object *parent)
{
	LS_FUNC_ENTER
	Evas_Object *bg = elm_bg_add(parent);
	LS_RETURN_VAL_IF_FAILED(bg, NULL);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, bg);
	evas_object_show(bg);
	LS_FUNC_EXIT
	return bg;
}

Evas_Object *create_conformant(Evas_Object *parent)
{
	LS_FUNC_ENTER
	Evas_Object *conformant = NULL;

	conformant = elm_conformant_add(parent);
	evas_object_size_hint_weight_set(conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, conformant);
	evas_object_show(conformant);

	LS_FUNC_EXIT
	return conformant;
}

Evas_Object *create_layout(Evas_Object *parent)
{
	LS_FUNC_ENTER
	Evas_Object *layout = NULL;

	if (parent != NULL) {
		layout = elm_layout_add(parent);
		evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_layout_theme_set(layout, "layout", "application", "default");
		elm_object_content_set(parent, layout);
		evas_object_show(layout);
	}

	LS_FUNC_EXIT
	return layout;
}

void profile_changed_cb(void *data, Evas_Object *obj, void *event)
{
	LS_FUNC_ENTER
	const char *profile = elm_config_profile_get();

	if (strcmp(profile, "desktop") == 0)
		elm_win_indicator_mode_set(obj, ELM_WIN_INDICATOR_HIDE);
	else
		elm_win_indicator_mode_set(obj, ELM_WIN_INDICATOR_SHOW);

	LS_FUNC_EXIT
}

void win_del(void *data, Evas_Object *obj, void *event)
{
	LS_FUNC_ENTER
	elm_exit();
}

Evas_Object *create_win(const char *name)
{
	LS_FUNC_ENTER
	Evas_Object *eo;
	eo = elm_win_util_standard_add(name, name);

	if (eo) {
		elm_win_title_set(eo, name);
		elm_win_indicator_mode_set(eo, ELM_WIN_INDICATOR_SHOW); /* indicator allow */
		elm_win_indicator_opacity_set(eo, ELM_WIN_INDICATOR_OPAQUE);
		elm_win_conformant_set(eo, EINA_TRUE);
		evas_object_smart_callback_add(eo, "delete,request", win_del, NULL);
		elm_win_autodel_set(eo, EINA_TRUE);
	}
	evas_object_show(eo);

	LS_FUNC_EXIT
	return eo;
}

static int __setting_location_set_int(const char *path, int val)
{
	int enabled = 0;
	int ret = 0;
	ret = vconf_get_int(path, &enabled);
	if (ret != VCONF_OK) {
		LS_LOGD("Fail to get vconf value");
		return -1;
	}

	if (enabled != val) {
		ret = vconf_set_int(path, val);
		if (ret != VCONF_OK) {
			LS_LOGD("Fail to set vconf value");
			return -1;
		}
	}

	return 0;
}

static void __setting_location_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;
	elm_naviframe_item_pop(ad->nf);
}

static void __setting_location_ea_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	if (ad->gps_popup) {
		evas_object_del(ad->gps_popup);
		ad->gps_popup = NULL;
	}

#ifdef TIZEN_FEATURE_WPS
	if (ad->wifi_popup) {
		evas_object_del(ad->wifi_popup);
		ad->wifi_popup = NULL;
	}
#endif
}

static Eina_Bool __setting_location_pop_cb(void *data, Elm_Object_Item *item)
{
	LS_FUNC_ENTER
	__setting_reply_gps_wifi_status(data);
	elm_exit();
	return EINA_FALSE;
}

static void __setting_location_help_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	evas_object_del(ad->ctx_popup);
	ad->ctx_popup = NULL;

	_setting_location_help_view(data);
}

void _mouseup_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	LS_RETURN_IF_FAILED(event_info);
	Evas_Event_Mouse_Up *ev = event_info;
	if (ev->button == 3) {/* if mouse right button is up */
		LS_RETURN_IF_FAILED(obj);
		evas_object_del(obj);
	}
}

static void __setting_location_item_disabled_update(void *data)
{
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	Eina_Bool state = EINA_FALSE;

	if (ad->is_myloc) {
		state = elm_object_item_disabled_get(ad->gi_gps);
		if (state == EINA_TRUE) {
			elm_object_item_disabled_set(ad->gi_gps, EINA_FALSE);
			elm_genlist_item_update(ad->gi_gps);
		}
#ifdef TIZEN_FEATURE_WPS
		state = elm_object_item_disabled_get(ad->gi_wifi);
		if (state == EINA_TRUE) {
			elm_object_item_disabled_set(ad->gi_wifi, EINA_FALSE);
			elm_genlist_item_update(ad->gi_wifi);
		}
#endif

	} else {
		state = elm_object_item_disabled_get(ad->gi_gps);
		if (state == EINA_FALSE) {
			elm_object_item_disabled_set(ad->gi_gps, EINA_TRUE);
			elm_genlist_item_update(ad->gi_gps);
		}
#ifdef TIZEN_FEATURE_WPS
		state = elm_object_item_disabled_get(ad->gi_wifi);
		if (state == EINA_FALSE) {
			elm_object_item_disabled_set(ad->gi_wifi, EINA_TRUE);
			elm_genlist_item_update(ad->gi_wifi);
		}
#endif
	}

	int restriction = 0;
	int ret = vconf_get_int(VCONFKEY_LOCATION_RESTRICT, &restriction);
	if (ret != VCONF_OK)
		LS_LOGE("fail to get vconf key!");

	if (restriction > 0)
		elm_object_item_disabled_set(ad->gi_loc, EINA_TRUE);
}

static void __setting_location_loc_set_key(void *data)
{
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	__setting_location_set_int(VCONFKEY_LOCATION_USE_MY_LOCATION, ad->is_myloc);
	elm_check_state_set(ad->gi_loc_check, ad->is_myloc);
	__setting_location_item_disabled_update(ad);
	elm_genlist_item_update(ad->gi_loc);
}

static void __setting_location_gps_set_key(void *data)
{
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	__setting_location_set_int(VCONFKEY_LOCATION_ENABLED, ad->is_gps);
	elm_check_state_set(ad->gi_gps_check, ad->is_gps);

	if (ad->is_gps) {
		if (ad->is_myloc == false) {
			ad->is_myloc = true;
			__setting_location_loc_set_key(ad);
		}
	} else {
#ifdef TIZEN_FEATURE_WPS
		if (!ad->is_wifi) {
			ad->is_myloc = false;
			__setting_location_loc_set_key(ad);
		}
#else
		ad->is_myloc = false;
		__setting_location_loc_set_key(ad);
#endif
	}

	__setting_location_item_disabled_update(ad);
	elm_genlist_item_update(ad->gi_gps);
}

static void __setting_location_wifi_set_key(void *data)
{
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	__setting_location_set_int(VCONFKEY_LOCATION_NETWORK_ENABLED, ad->is_wifi);
	elm_check_state_set(ad->gi_wifi_check, ad->is_wifi);

	if (ad->is_wifi) {
		if (ad->is_myloc == false) {
			ad->is_myloc = true;
			__setting_location_loc_set_key(ad);
		}
	} else {
		if (!ad->is_gps) {
			ad->is_myloc = false;
			__setting_location_loc_set_key(ad);
		}
	}

	__setting_location_item_disabled_update(ad);
	elm_genlist_item_update(ad->gi_wifi);
}

static void __location_gps_popup_agree_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	if (ad->gps_popup) {
		evas_object_del(ad->gps_popup);
		ad->gps_popup = NULL;
	}

	if (ad->ask_check) {
		Eina_Bool ischoose = false;
		ischoose = elm_check_state_get(ad->ask_check);
		if (ischoose == EINA_TRUE) {
			int ret = vconf_set_int(VCONFKEY_LBS_SETTING_IS_SHOW_GPS_POPUP, KEY_DISABLED);
			LS_LOGE("ret:%d", ret);
		}
	}

	ad->is_gps = true;
	__setting_location_gps_set_key(ad);

#ifdef TIZEN_FEATURE_WPS
	if (!ad->is_wifi)
		location_wifi_popup(ad);
#endif
}

static void __location_gps_popup_disagree_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	if (ad->gps_popup) {
		evas_object_del(ad->gps_popup);
		ad->gps_popup = NULL;
	}

#ifdef TIZEN_FEATURE_WPS
	if (!ad->is_wifi)
		location_wifi_popup(ad);
#endif
}

static void __location_wifi_popup_agree_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	if (ad->wifi_popup) {
		evas_object_del(ad->wifi_popup);
		ad->wifi_popup = NULL;
	}

	if (!ad->is_myloc) {
		ad->is_myloc = true;
		__setting_location_loc_set_key(ad);
	}
	ad->is_wifi = true;
	__setting_location_wifi_set_key(ad);
}

static void __location_wifi_popup_disagree_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	if (ad->wifi_popup) {
		evas_object_del(ad->wifi_popup);
		ad->wifi_popup = NULL;
	}
}

static void location_gps_popup(void *data)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;
	Evas_Object *popup = NULL;
	Evas_Object *agree_btn = NULL;
	Evas_Object *disagree_btn = NULL;

	popup = elm_popup_add(ad->win_main);
	ad->gps_popup = popup;

	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP, _mouseup_cb, NULL);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __setting_location_ea_back_cb, ad);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_domain_translatable_part_text_set(popup, "title,text", LBS_SETTING_PKG, "IDS_ST_HEADER_CONSENT_TO_LOCATION_INFO_USAGE_ABB");
	Evas_Object *layout = elm_layout_add(popup);
	elm_layout_file_set(layout, LBS_SETTING_EDJ, "popup_gps_layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *check = elm_check_add(popup);
	elm_object_style_set(check, "popup");
	elm_object_text_set(check, P_("IDS_ST_BODY_DO_NOT_SHOW_AGAIN"));
	evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(layout, "elm.swallow.end", check);
	ad->ask_check = check;

	Evas_Object *scroller = elm_scroller_add(popup);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	elm_object_style_set(scroller, "effect");
	evas_object_show(scroller);

	Evas_Object *label = elm_label_add(scroller);
	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	elm_object_domain_translatable_text_set(label, LBS_SETTING_PKG, "IDS_ST_POP_YOUR_LOCATION_DATA_INCLUDING_GPS_DATA_WILL_BE_USED_BY_RELEVANT_APPLICATIONS");
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_EXPAND);
	elm_object_content_set(scroller, label);
	elm_object_part_content_set(layout, "elm.swallow.content", scroller);
	elm_object_content_set(popup, layout);

	disagree_btn = elm_button_add(popup);
	elm_object_style_set(disagree_btn, "popup");
	elm_object_domain_translatable_text_set(disagree_btn, LBS_SETTING_PKG, "IDS_ST_BUTTON_DISAGREE");
	elm_object_part_content_set(popup, "button1", disagree_btn);
	evas_object_smart_callback_add(disagree_btn, "clicked", __location_gps_popup_disagree_cb, ad);

	agree_btn = elm_button_add(popup);
	elm_object_style_set(agree_btn, "popup");
	elm_object_domain_translatable_text_set(agree_btn, LBS_SETTING_PKG, "IDS_ST_BUTTON_AGREE");
	elm_object_part_content_set(popup, "button2", agree_btn);
	evas_object_smart_callback_add(agree_btn, "clicked", __location_gps_popup_agree_cb, ad);
	evas_object_show(popup);
}

void location_wifi_popup(void *data)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;
	Evas_Object *popup = NULL;
	Evas_Object *disagree_btn = NULL;
	Evas_Object *agree_btn = NULL;

	popup = elm_popup_add(ad->win_main);
	ad->wifi_popup = popup;

	evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP, _mouseup_cb, NULL);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __location_wifi_popup_disagree_cb, ad);
	elm_object_domain_translatable_part_text_set(popup, "title,text", LBS_SETTING_PKG, "IDS_ST_HEADER_LOCATION_LEGAL_INFORMATION_ABB");

	char *buf = NULL;
	char text[4096] = {0};
	char *str1 = P_("IDS_POSITIONING_CONSENT_BODY");
	char *str2 = "</br><color=#006fd1ff underline=on underline_color=#006fd1ff><a href=http://here.com/terms/service-terms>http://here.com/terms/service-terms</a></color></br>";
	char *str3 = "</br><color=#006fd1ff underline=on underline_color=#006fd1ff><a href=http://here.com/privacy/privacy-policy>http://here.com/privacy/privacy-policy</a></color></br>";
	snprintf(text, 4096, str1, str2, str3);
	buf = g_strdup_printf("<color=#%s>%s</color>", "000000", text);

	Evas_Object *layout = elm_layout_add(popup);
	elm_layout_file_set(layout, LBS_SETTING_EDJ, "popup_checkview_layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *help_scroller = elm_scroller_add(popup);
	elm_scroller_bounce_set(help_scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(help_scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	elm_object_style_set(help_scroller, "effect");
	evas_object_show(help_scroller);

	Evas_Object *label = elm_entry_add(help_scroller);
	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	elm_object_text_set(label, buf);
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(label, "anchor,clicked", _anchor_clicked_cb, ad);
	elm_entry_editable_set(label, EINA_FALSE);
	elm_entry_input_panel_enabled_set(label, EINA_FALSE);
	elm_entry_context_menu_disabled_set(label, EINA_TRUE);

	elm_object_content_set(help_scroller, label);
	elm_object_part_content_set(layout, "elm.swallow.content", help_scroller);
	elm_object_content_set(popup, layout);

	disagree_btn = elm_button_add(popup);
	elm_object_style_set(disagree_btn, "popup");
	elm_object_domain_translatable_text_set(disagree_btn, LBS_SETTING_PKG, "IDS_ST_BUTTON_DISAGREE");
	elm_object_part_content_set(popup, "button1", disagree_btn);
	evas_object_smart_callback_add(disagree_btn, "clicked", __location_wifi_popup_disagree_cb, ad);

	agree_btn = elm_button_add(popup);
	elm_object_style_set(agree_btn, "popup");
	elm_object_domain_translatable_text_set(agree_btn, LBS_SETTING_PKG, "IDS_ST_BUTTON_AGREE");
	elm_object_part_content_set(popup, "button2", agree_btn);
	evas_object_smart_callback_add(agree_btn, "clicked", __location_wifi_popup_agree_cb, ad);

	evas_object_show(popup);

	LS_FUNC_EXIT
}

static char *__setting_location_title_text_get(void *data, Evas_Object *obj, const char *part)
{
	if (!g_strcmp0(part, "elm.text"))
		return strdup(P_("IDS_ST_BODY_LOCATION_SOURCES_ABB"));

	return NULL;
}

static char *__setting_location_loc_text_get(void *data, Evas_Object *obj, const char *part)
{
	if (!g_strcmp0(part, "elm.text"))
		return strdup(P_("IDS_ST_MBODY_USE_CURRENT_LOCATION"));

	return NULL;
}

static char *__setting_location_gps_text_get(void *data, Evas_Object *obj, const char *part)
{
	if (!g_strcmp0(part, "elm.text"))
		return strdup(P_("IDS_ST_BODY_GPS"));

	return NULL;
}

static char *__setting_location_wifi_text_get(void *data, Evas_Object *obj, const char *part)
{
	if (!g_strcmp0(part, "elm.text"))
		return strdup(P_("IDS_ST_BODY_WIRELESS_NETWORKS_ABB"));

	return NULL;
}

static void __setting_location_loc_check_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	if (ad->is_myloc) {
		ad->is_gps = false;
		__setting_location_gps_set_key(ad);
#ifdef TIZEN_FEATURE_WPS
		ad->is_wifi = false;
		__setting_location_wifi_set_key(ad);
#endif
	} else {
		int isShow = KEY_DISABLED;
		int ret = vconf_get_int(VCONFKEY_LBS_SETTING_IS_SHOW_GPS_POPUP, &isShow);
		if (ret != VCONF_OK)
			LS_LOGE("vconf_get_bool error:%d", ret);

		elm_check_state_set(ad->gi_loc_check, EINA_FALSE);
		elm_genlist_item_update(ad->gi_loc);

		if (isShow)
			location_gps_popup(ad);
		else {
			ad->is_gps = true;
			__setting_location_gps_set_key(ad);
#ifdef TIZEN_FEATURE_WPS
			location_wifi_popup(ad);
#endif
		}
	}
}

static void __setting_location_gps_check_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;
	ad->quick_panel_setting = true;

	if (ad->is_gps) {
		ad->is_gps = false;
		__setting_location_gps_set_key(ad);
	} else {
		int isShow = 0;
		int ret = vconf_get_int(VCONFKEY_LBS_SETTING_IS_SHOW_GPS_POPUP, &isShow);
		if (ret != VCONF_OK)
			LS_LOGE("vconf_get_bool error:%d", ret);

		elm_check_state_set(ad->gi_gps_check, EINA_FALSE);
		elm_genlist_item_update(ad->gi_gps);

		if (isShow)
			location_gps_popup(ad);
		else {
			ad->is_gps = true;
			__setting_location_gps_set_key(ad);
		}
	}
}

static void __setting_location_wifi_check_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;
	ad->quick_panel_setting = true;

	if (ad->is_wifi) {
		ad->is_wifi = false;
		__setting_location_wifi_set_key(ad);
	} else {
		elm_check_state_set(ad->gi_wifi_check, EINA_FALSE);
		elm_genlist_item_update(ad->gi_wifi);
		location_wifi_popup(ad);
	}
}

static Evas_Object *__setting_location_loc_check_get(void *data, Evas_Object *obj, const char *part)
{
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	if (!strcmp(part, "elm.swallow.icon.1")) {
		ad->gi_loc_check = elm_check_add(obj);

		int value = -1;
		int ret = vconf_get_int(VCONFKEY_LOCATION_USE_MY_LOCATION, &value);
		if (ret != VCONF_OK)
			LS_LOGE("fail to get vconf key!");

		if (value)
			elm_check_state_set(ad->gi_loc_check, EINA_TRUE);
		else
			elm_check_state_set(ad->gi_loc_check, EINA_FALSE);

		elm_object_style_set(ad->gi_loc_check, "on&off");
		evas_object_propagate_events_set(ad->gi_loc_check, EINA_FALSE);
		evas_object_smart_callback_add(ad->gi_loc_check, "changed", __setting_location_loc_check_cb, ad);
		evas_object_show(ad->gi_loc_check);

		return ad->gi_loc_check;
	}
	return NULL;
}

static Evas_Object *__setting_location_gps_check_get(void *data, Evas_Object *obj, const char *part)
{
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	if (!strcmp(part, "elm.swallow.icon.1")) {
		Evas_Object *tg = elm_check_add(obj);

		int value = -1;
		int ret = vconf_get_int(VCONFKEY_LOCATION_ENABLED, &value);
		if (ret != VCONF_OK)
			LS_LOGE("fail to get vconf key!");

		if (value)
			elm_check_state_set(tg, EINA_TRUE);
		else
			elm_check_state_set(tg, EINA_FALSE);

		elm_object_style_set(tg, "on&off");
		evas_object_propagate_events_set(tg, EINA_FALSE);
		evas_object_smart_callback_add(tg, "changed", __setting_location_gps_check_cb, ad);
		evas_object_show(tg);

		ad->gi_gps_check = tg;
		elm_genlist_item_update(ad->gi_gps);

		return tg;
	}
	return NULL;
}

static Evas_Object *__setting_location_wifi_check_get(void *data, Evas_Object *obj, const char *part)
{
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	if (!strcmp(part, "elm.swallow.icon.1")) {
		Evas_Object *tg = elm_check_add(obj);

		int value = -1;
		int ret = vconf_get_int(VCONFKEY_LOCATION_NETWORK_ENABLED, &value);
		if (ret != VCONF_OK)
			LS_LOGE("fail to get vconf key!");

		if (value)
			elm_check_state_set(tg, EINA_TRUE);
		else
			elm_check_state_set(tg, EINA_FALSE);

		elm_object_style_set(tg, "on&off");
		evas_object_propagate_events_set(tg, EINA_FALSE);
		evas_object_smart_callback_add(tg, "changed", __setting_location_wifi_check_cb, ad);
		evas_object_show(tg);

		ad->gi_wifi_check = tg;

		return tg;
	}
	return NULL;
}

static void __setting_location_loc_sel(void *data, Evas_Object *obj, void *event_info)
{
	LS_RETURN_IF_FAILED(data);
	LS_RETURN_IF_FAILED(event_info);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	if (ad->is_myloc) {
		ad->is_gps = false;
		__setting_location_gps_set_key(ad);
#ifdef TIZEN_FEATURE_WPS
		ad->is_wifi = false;
		__setting_location_wifi_set_key(ad);
#endif
	} else {
		int isShow = KEY_DISABLED;
		int ret = vconf_get_int(VCONFKEY_LBS_SETTING_IS_SHOW_GPS_POPUP, &isShow);
		if (ret != VCONF_OK)
			LS_LOGE("vconf_get_bool error:%d", ret);

		if (isShow)
			location_gps_popup(ad);
		else {
			ad->is_gps = true;
			__setting_location_gps_set_key(ad);
#ifdef TIZEN_FEATURE_WPS
			location_wifi_popup(ad);
#endif
		}
	}
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
}

static void __setting_location_gps_sel(void *data, Evas_Object *obj, void *event_info)
{
	LS_RETURN_IF_FAILED(data);
	LS_RETURN_IF_FAILED(event_info);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;
	ad->quick_panel_setting = true;

	if (ad->is_gps) {
		ad->is_gps = false;
		__setting_location_gps_set_key(ad);

	} else {
		int isShow = 0;
		int ret = vconf_get_int(VCONFKEY_LBS_SETTING_IS_SHOW_GPS_POPUP, &isShow);
		if (ret != VCONF_OK)
			LS_LOGE("vconf_get_bool error:%d", ret);

		if (isShow)
			location_gps_popup(ad);
		else {
			ad->is_gps = true;
			__setting_location_gps_set_key(ad);
		}
	}

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
}

static void __setting_location_wifi_sel(void *data, Evas_Object *obj, void *event_info)
{
	LS_RETURN_IF_FAILED(data);
	LS_RETURN_IF_FAILED(event_info);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;
	ad->quick_panel_setting = true;

	if (ad->is_wifi) {
		ad->is_wifi = false;
		__setting_location_wifi_set_key(ad);

	} else
		location_wifi_popup(ad);

	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
}

static Evas_Object *__setting_location_create_navibar(Evas_Object *parent)
{
	Evas_Object *naviframe = NULL;

	naviframe = elm_naviframe_add(parent);
	elm_object_part_content_set(parent, "elm.swallow.content", naviframe);
	evas_object_show(naviframe);

	return naviframe;
}

static char *__setting_myplace_group_text_get(void *data, Evas_Object *obj, const char *part)
{
	if (!g_strcmp0(part, "elm.text"))
		return strdup(P_("IDS_MAPS_BODY_MY_PLACES"));

	return NULL;
}

static char *__setting_myplace_text_get(void *data, Evas_Object *obj, const char *part)
{
	if (!g_strcmp0(part, "elm.text"))
		return strdup(P_("IDS_MAPS_BODY_MY_PLACES"));

	return NULL;
}

static void __setting_myplace_sel(void *data, Evas_Object *obj, void *event_info)
{
	LS_RETURN_IF_FAILED(data);
	LS_RETURN_IF_FAILED(event_info);
	app_control_h app_control = NULL;
	app_control_create(&app_control);

	app_control_set_app_id(app_control, "org.tizen.myplace");
	app_control_add_extra_data(app_control, "caller", "lbs-setting");
	app_control_add_extra_data(app_control, "geofence", data);
	app_control_send_launch_request(app_control, NULL, NULL);

	app_control_destroy(app_control);
	elm_genlist_item_selected_set(event_info, EINA_FALSE);
}

static void __myplace_automation_consent_set(bool consent, lbs_setting_app_data * ad)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(ad);

	elm_check_state_set(ad->gi_myplace_automation_check, consent);
	elm_genlist_item_update(ad->gi_myplace_automation);
	ad->is_myplace_automation_consent = consent;
	int ret = context_places_consent(consent);
	if (ret != CONTEXT_PLACES_ERROR_NONE) {
		LS_LOGE("context_places consent setting ERROR");
	}

	LS_FUNC_EXIT
}

static void __myplace_automation_popup_agree_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	/* Hide popup */
	if (ad->myplace_automation_popup) {
		evas_object_del(ad->myplace_automation_popup);
		ad->myplace_automation_popup = NULL;
	}

	__myplace_automation_consent_set(true, ad);
}

static void __myplace_automation_popup_disagree_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	/* Hide popup */
	if (ad->myplace_automation_popup) {
		evas_object_del(ad->myplace_automation_popup);
		ad->myplace_automation_popup = NULL;
	}

	__myplace_automation_consent_set(false, ad);
}

static void __myplace_automation_popup_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	/* Hide popup */
	if (ad->myplace_automation_popup) {
		evas_object_del(ad->myplace_automation_popup);
		ad->myplace_automation_popup = NULL;
	}
}

static void __myplace_automation_popup(void *data)
{
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	Evas_Object *popup = NULL;
	Evas_Object *agree_btn = NULL;
	Evas_Object *disagree_btn = NULL;

	popup = elm_popup_add(ad->win_main);
	ad->myplace_automation_popup = popup;
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP, _mouseup_cb, NULL);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, __myplace_automation_popup_back_cb, ad);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_text_set(popup, "title,text", "My places automation consent"); // TODO: Translate for other languages
	elm_object_text_set(popup, "Some fancy description of algorithm and its usage. "\
			"Some fancy description of algorithm and its usage. Some fancy description "\
			"of algorithm and its usage. Some fancy description of algorithm and its usage. "\
			"Some fancy description of algorithm and its usage. Some fancy description of "\
			"algorithm and its usage. Some fancy description of algorithm and its usage");	// TODO: Fill with proper description
																							// TODO: Translate for other languages

	/* Disagree button */
	disagree_btn = elm_button_add(popup);
	elm_object_style_set(disagree_btn, "popup");
	elm_object_domain_translatable_text_set(disagree_btn, LBS_SETTING_PKG, "IDS_ST_BUTTON_DISAGREE");
	elm_object_part_content_set(popup, "button1", disagree_btn);
	evas_object_smart_callback_add(disagree_btn, "clicked", __myplace_automation_popup_disagree_cb, ad);

	/* Agree button */
	agree_btn = elm_button_add(popup);
	elm_object_style_set(agree_btn, "popup");
	elm_object_domain_translatable_text_set(agree_btn, LBS_SETTING_PKG, "IDS_ST_BUTTON_AGREE");
	elm_object_part_content_set(popup, "button2", agree_btn);
	evas_object_smart_callback_add(agree_btn, "clicked", __myplace_automation_popup_agree_cb, ad);

	evas_object_show(popup);
}

static char *__setting_myplace_automation_text_get(void *data, Evas_Object *obj, const char *part)
{
	if (!g_strcmp0(part, "elm.text"))
		return strdup("Automatic suggestion");

	return NULL;
}

static void __setting_myplace_automation_check_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	elm_check_state_set(ad->gi_myplace_automation_check, ad->is_myplace_automation_consent);
	elm_genlist_item_update(ad->gi_myplace_automation);
	if (ad->is_myplace_automation_consent)
		__myplace_automation_consent_set(false, ad);
	else
		__myplace_automation_popup(ad);
}

static Evas_Object *__setting_myplace_automation_check_get(void *data, Evas_Object *obj, const char *part)
{
	LS_RETURN_VAL_IF_FAILED(data, NULL);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	if (!strcmp(part, "elm.swallow.icon.1")) {
		ad->gi_myplace_automation_check = elm_check_add(obj);

		elm_check_state_set(ad->gi_myplace_automation_check, ad->is_myplace_automation_consent);
		elm_object_style_set(ad->gi_myplace_automation_check, "on&off");
		evas_object_propagate_events_set(ad->gi_myplace_automation_check, EINA_FALSE);
		evas_object_smart_callback_add(ad->gi_myplace_automation_check, "changed", __setting_myplace_automation_check_cb, ad);
		evas_object_show(ad->gi_myplace_automation_check);

		return ad->gi_myplace_automation_check;
	}

	return NULL;
}

static void __setting_myplace_automation_sel(void *data, Evas_Object *obj, void *event_info)
{
	LS_RETURN_IF_FAILED(data);
	LS_RETURN_IF_FAILED(event_info);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	if (ad->is_myplace_automation_consent)
		__myplace_automation_consent_set(false, ad);
	else
		__myplace_automation_popup(ad);

	elm_genlist_item_selected_set(event_info, EINA_FALSE);
}

static Evas_Object *__setting_location_create_gl(Evas_Object *parent, void *data)
{
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;
	Evas_Object *genlist = NULL;
	genlist = elm_genlist_add(parent);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	const char *geofence_feature = "http://tizen.org/feature/location.geofence";
	bool is_geofence_supported = false;

	/* Use my location */
	ad->itc_loc = elm_genlist_item_class_new();
	if (ad->itc_loc == NULL) {
		LS_LOGE("critical error : LS_RETURN_VAL_IS_FAILED");
		return NULL;
	}
	ad->itc_loc->item_style = "type1";
	ad->itc_loc->func.text_get = __setting_location_loc_text_get;
	ad->itc_loc->func.content_get = __setting_location_loc_check_get;
	ad->gi_loc = elm_genlist_item_append(genlist, ad->itc_loc, (void *)ad, NULL, ELM_GENLIST_ITEM_NONE, __setting_location_loc_sel, ad);

	/* Group title */
	ad->itc_title = elm_genlist_item_class_new();
	if (ad->itc_title == NULL) {
		g_free(ad->itc_loc);
		LS_LOGE("critical error : LS_RETURN_VAL_IS_FAILED");
		return NULL;
	}
	ad->itc_title->item_style = "group_index";
	ad->itc_title->func.text_get = __setting_location_title_text_get;
	ad->gi_group_title = elm_genlist_item_append(genlist, ad->itc_title, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(ad->gi_group_title, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	/* GPS satellite */
	ad->itc_gps = elm_genlist_item_class_new();
	if (ad->itc_gps == NULL) {
		g_free(ad->itc_loc);
		g_free(ad->itc_title);
		LS_LOGE("critical error : LS_RETURN_VAL_IS_FAILED");
		return NULL;
	}
	ad->itc_gps->item_style = "type1";
	ad->itc_gps->func.text_get = __setting_location_gps_text_get;
	ad->itc_gps->func.content_get = __setting_location_gps_check_get;
	ad->gi_gps = elm_genlist_item_append(genlist, ad->itc_gps, (void *)ad, NULL, ELM_GENLIST_ITEM_NONE, __setting_location_gps_sel, ad);

	/* Wi-fi & mobile network */
	ad->itc_wifi = elm_genlist_item_class_new();
	if (ad->itc_wifi == NULL) {
		g_free(ad->itc_loc);
		g_free(ad->itc_title);
		g_free(ad->itc_gps);
		LS_LOGE("critical error : LS_RETURN_VAL_IS_FAILED");
		return NULL;
	}

	ad->itc_wifi->item_style = "type1";
	ad->itc_wifi->func.text_get = __setting_location_wifi_text_get;
	ad->itc_wifi->func.content_get = __setting_location_wifi_check_get;
	ad->gi_wifi = elm_genlist_item_append(genlist, ad->itc_wifi, ad, NULL, ELM_GENLIST_ITEM_NONE, __setting_location_wifi_sel, ad);

#ifndef TIZEN_FEATURE_WPS
	elm_object_item_disabled_set(ad->gi_wifi, EINA_TRUE);
#endif

	__setting_location_item_disabled_update(ad);

	/* Myplace group */
	system_info_get_platform_bool(geofence_feature, &is_geofence_supported);
	if (is_geofence_supported) {

		/* Group title */
		ad->itc_myplace_title = elm_genlist_item_class_new();
		if (ad->itc_myplace_title == NULL) {
			g_free(ad->itc_loc);
			g_free(ad->itc_title);
			g_free(ad->itc_gps);
			g_free(ad->itc_wifi);
			LS_LOGE("critical error : LS_RETURN_VAL_IS_FAILED");
			return NULL;
		}
		ad->itc_myplace_title->item_style = "group_index";
		ad->itc_myplace_title->func.text_get = __setting_myplace_group_text_get;
		ad->gi_myplace_title = elm_genlist_item_append(genlist, ad->itc_myplace_title, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(ad->gi_myplace_title, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

		/* My places */
		ad->itc_myplace = elm_genlist_item_class_new();
		if (ad->itc_myplace == NULL) {
			g_free(ad->itc_loc);
			g_free(ad->itc_title);
			g_free(ad->itc_gps);
			g_free(ad->itc_wifi);
			g_free(ad->itc_myplace_title);
			LS_LOGE("critical error : LS_RETURN_VAL_IS_FAILED");
			return NULL;
		}
		ad->itc_myplace->item_style = "type1";
		ad->itc_myplace->func.text_get = __setting_myplace_text_get;
		ad->gi_myplace = elm_genlist_item_append(genlist, ad->itc_myplace, (void *)ad, NULL, ELM_GENLIST_ITEM_NONE, __setting_myplace_sel, ad);

		/* Automatic suggestion consent */
		if (ad->is_myplace_automation_supported) {
			ad->itc_myplace_automation = elm_genlist_item_class_new();
			if (ad->itc_myplace_automation == NULL) {
				LS_LOGE("critical error : LS_RETURN_VAL_IS_FAILED");
				return NULL;
			}
			ad->itc_myplace_automation->item_style = "type1";
			ad->itc_myplace_automation->func.text_get = __setting_myplace_automation_text_get;
			ad->itc_myplace_automation->func.content_get = __setting_myplace_automation_check_get;
			ad->gi_myplace_automation = elm_genlist_item_append(genlist, ad->itc_myplace_automation, (void *)ad, NULL, ELM_GENLIST_ITEM_NONE, __setting_myplace_automation_sel, ad);
		}
	}

	return genlist;
}

static void _ctx_popup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	if (!ad->ctx_popup) {
		LS_LOGE("Invalid parameters");
		return;
	}

	evas_object_del(ad->ctx_popup);
	ad->ctx_popup = NULL;
}

static void _move_more_ctxpopup(void *data)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(data);

	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	Evas_Coord w, h;
	int pos = -1;

	elm_win_screen_size_get(ad->win_main, NULL, NULL, &w, &h);
	pos = elm_win_rotation_get(ad->win_main);

	switch (pos) {
	case 0:
	case 180:
		evas_object_move(ad->ctx_popup, w / 2, h);
		break;
	case 90:
		evas_object_move(ad->ctx_popup, h / 2, w);
		break;
	case 270:
		evas_object_move(ad->ctx_popup, h / 2, w);
		break;
	}
}

static void _rotate_more_ctxpopup_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;
	_move_more_ctxpopup(ad);
}

static void _resize_more_ctxpopup_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;
	_move_more_ctxpopup(ad);
}

static void __setting_location_create_more_button(void *data, Evas_Object *obj, void *event_info)
{
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	if (!ad || !ad->nf) {
		LS_LOGE("NULL parameters.\n");
		return;
	}

	if (ad->ctx_popup) {
		evas_object_del(ad->ctx_popup);
		ad->ctx_popup = NULL;
	}

	ad->ctx_popup = elm_ctxpopup_add(ad->win_main);
	elm_object_style_set(ad->ctx_popup, "more/default");
	eext_object_event_callback_add(ad->ctx_popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
	eext_object_event_callback_add(ad->ctx_popup, EEXT_CALLBACK_MORE, eext_popup_back_cb, NULL);
	evas_object_smart_callback_add(ad->ctx_popup, "dismissed", _ctx_popup_dismissed_cb, ad);
	elm_ctxpopup_auto_hide_disabled_set(ad->ctx_popup, EINA_TRUE);

	evas_object_event_callback_add(ad->nf, EVAS_CALLBACK_RESIZE, _resize_more_ctxpopup_cb, ad);
	evas_object_smart_callback_add(elm_object_top_widget_get(ad->ctx_popup), "rotation,changed",
									_rotate_more_ctxpopup_cb, ad);

	elm_ctxpopup_item_append(ad->ctx_popup, P_("IDS_ST_HEADER_HELP"), NULL, __setting_location_help_cb, ad);

	elm_ctxpopup_direction_priority_set(ad->ctx_popup, ELM_CTXPOPUP_DIRECTION_UP,
										ELM_CTXPOPUP_DIRECTION_LEFT,
										ELM_CTXPOPUP_DIRECTION_RIGHT,
										ELM_CTXPOPUP_DIRECTION_DOWN);

	_move_more_ctxpopup(ad);
	evas_object_show(ad->ctx_popup);
}

void __setting_location_create_view(lbs_setting_app_data *ad)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(ad);
	Elm_Object_Item *navi_it = NULL;
	Evas_Object *more_button = NULL;

	ad->view_id = LOCATION_MAIN_VIEW;
	ad->nf = __setting_location_create_navibar(ad->layout_main);
	elm_naviframe_prev_btn_auto_pushed_set(ad->nf, EINA_FALSE);
	eext_object_event_callback_add(ad->nf, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	eext_object_event_callback_add(ad->nf, EEXT_CALLBACK_MORE, eext_naviframe_more_cb, NULL);

	ad->genlist = __setting_location_create_gl(ad->nf, ad);
	LS_RETURN_IF_FAILED(ad->genlist);

	evas_object_show(ad->genlist);

	Evas_Object *back_btn = elm_button_add(ad->nf);
	elm_object_style_set(back_btn, "naviframe/back_btn/default");
	evas_object_smart_callback_add(back_btn, "clicked", __setting_location_back_cb, ad);
	navi_it = elm_naviframe_item_push(ad->nf, P_("IDS_ST_BUTTON2_LOCATION"), back_btn, NULL, ad->genlist, NULL);

	elm_naviframe_item_pop_cb_set(navi_it, __setting_location_pop_cb, ad);

	more_button = elm_button_add(ad->nf);
	elm_object_style_set(more_button, "naviframe/more/default");
	evas_object_smart_callback_add(more_button, "clicked", __setting_location_create_more_button, ad);
	elm_object_item_part_content_set(navi_it, "toolbar_more_btn", more_button);
	LS_FUNC_EXIT
}

void _location_key_changed_cb(keynode_t *key, void *data)
{
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;
	int enabled = 0;
	vconf_get_int(VCONFKEY_LOCATION_USE_MY_LOCATION, &enabled);
	if (ad->quick_panel_setting == false) {
		if (enabled == KEY_DISABLED) {
			ad->is_gps = false;
			__setting_location_gps_set_key(ad);
#ifdef TIZEN_FEATURE_WPS
			ad->is_wifi = false;
			__setting_location_wifi_set_key(ad);
#endif
		}
	}
	ad->quick_panel_setting = false;
}

void _gps_key_changed_cb(keynode_t *key, void *data)
{
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	int enabled = 0;
	int ret = vconf_get_int(VCONFKEY_LOCATION_ENABLED, &enabled);
	if (ret != VCONF_OK)
		LOGE("fail to get vconf key!");

	if (enabled)
		ad->is_gps = true;
	else
		ad->is_gps = false;

	if (ad->quick_panel_setting == false)
		__setting_location_gps_set_key(ad);
	else
		LS_LOGD("Notice: Current operation in Genlist callback function.");

	ad->quick_panel_setting = false;
}

void _wifi_key_changed_cb(keynode_t *key, void *data)
{
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	int enabled = 0;
	int ret = vconf_get_int(VCONFKEY_LOCATION_NETWORK_ENABLED, &enabled);
	if (ret != VCONF_OK)
		LS_LOGE("fail to get vconf key!");

	if (enabled)
		ad->is_wifi = true;
	else
		ad->is_wifi = false;

	if (ad->quick_panel_setting == false)
		__setting_location_wifi_set_key(ad);
	else
		LS_LOGD("Notice: Current operation in Genlist callback function.");

	ad->quick_panel_setting = false;
}

void _restriction_key_changed_cb(keynode_t *key, void *data)
{
	LS_LOGD("_restriction_key_changed_cb >> ENTER");
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	int value = -1;
	int restriction = 0;
	int ret = vconf_get_int(VCONFKEY_LOCATION_RESTRICT, &restriction);
	if (ret != VCONF_OK)
		LS_LOGE("fail to get vconf key!");
	else
		LS_LOGE("get_int restriction success [%d]", restriction);

	if (restriction > 0) {
		elm_object_item_disabled_set(ad->gi_gps, EINA_TRUE);
		elm_object_item_disabled_set(ad->gi_loc, EINA_TRUE);
#ifdef TIZEN_FEATURE_WPS
		elm_object_item_disabled_set(ad->gi_wps, EINA_TRUE);
#endif
	} else {
		ret = vconf_get_int(VCONFKEY_LOCATION_USE_MY_LOCATION, &value);
		if (ret != VCONF_OK)
			LS_LOGE("fail to get vconf key!");

		if (value == 1)
			elm_object_item_disabled_set(ad->gi_gps, EINA_FALSE);
		else
			elm_object_item_disabled_set(ad->gi_gps, EINA_TRUE);

		elm_object_item_disabled_set(ad->gi_loc, EINA_FALSE);

#ifdef TIZEN_FEATURE_WPS
		if (value == 1)
			elm_object_item_disabled_set(ad->gi_wps, EINA_FALSE);
		else
			elm_object_item_disabled_set(ad->gi_wps, EINA_TRUE);
#endif
	}

	elm_genlist_item_update(ad->gi_gps);
	elm_genlist_item_update(ad->gi_loc);
#ifdef TIZEN_FEATURE_WPS
	elm_genlist_item_update(ad->gi_wps);
#endif
}

int __setting_location_init(lbs_setting_app_data *ad)
{
	LS_RETURN_VAL_IF_FAILED(ad, -1);
	int ret = 0;
	int enabled = 0;
	ad->quick_panel_setting = false;

	ret &= vconf_get_int(VCONFKEY_LOCATION_USE_MY_LOCATION, &enabled);
	if (enabled) ad->is_myloc = true;
	else ad->is_myloc = false;
	ret &= vconf_get_int(VCONFKEY_LOCATION_ENABLED, &enabled);
	if (enabled) ad->is_gps = true;
	else ad->is_gps = false;
	ret &= vconf_notify_key_changed(VCONFKEY_LOCATION_USE_MY_LOCATION, _location_key_changed_cb, (void *)ad);
	ret &= vconf_notify_key_changed(VCONFKEY_LOCATION_ENABLED, _gps_key_changed_cb, (void *)ad);
#ifdef TIZEN_FEATURE_WPS
	ret &= vconf_get_int(VCONFKEY_LOCATION_NETWORK_ENABLED, &enabled);
	if (enabled) ad->is_wifi = true;
	else ad->is_wifi = false;
	ret &= vconf_notify_key_changed(VCONFKEY_LOCATION_NETWORK_ENABLED, _wifi_key_changed_cb, (void *)ad);
#endif

	ret &= vconf_notify_key_changed(VCONFKEY_LOCATION_RESTRICT, _restriction_key_changed_cb, (void *)ad);

	/* if myloc is off , and gps or wifi is on, we should set myloc on and its vconf value */
#ifdef TIZEN_FEATURE_WPS
	if (!ad->is_myloc && (ad->is_gps || ad->is_wifi)) {
#else
	if (!ad->is_myloc && ad->is_gps) {
#endif
		ad->is_myloc = true;
		__setting_location_set_int(VCONFKEY_LOCATION_USE_MY_LOCATION, KEY_ENABLED);
	}
	/* if gps and wifi both off, should set use my location off too. */
#ifdef TIZEN_FEATURE_WPS
	if (ad->is_myloc && (!ad->is_gps) && (!ad->is_wifi)) {
#else
	if (ad->is_myloc && (!ad->is_gps)) {
#endif
		ad->is_myloc = false;
		__setting_location_set_int(VCONFKEY_LOCATION_USE_MY_LOCATION, KEY_DISABLED);
	}

	return ret;
}

int _setting_location_deinit(lbs_setting_app_data *ad)
{
	int ret = 0;

	ret = vconf_ignore_key_changed(VCONFKEY_LOCATION_USE_MY_LOCATION, _location_key_changed_cb);
	ret = vconf_ignore_key_changed(VCONFKEY_LOCATION_ENABLED, _gps_key_changed_cb);
#ifdef TIZEN_FEATURE_WPS
	ret = vconf_ignore_key_changed(VCONFKEY_LOCATION_NETWORK_ENABLED, _wifi_key_changed_cb);
#endif
	ret = vconf_ignore_key_changed(VCONFKEY_LOCATION_RESTRICT, _restriction_key_changed_cb);

	return ret;
}


static void _app_control_reply_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	LS_FUNC_ENTER
	if (result != APP_CONTROL_RESULT_SUCCEEDED) {
		LS_LOGE("[Error:%d]Launch request fail", result);
		return;
	}

	LS_LOGI("Success to launch request");
}

static void _anchor_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(event_info);

	Elm_Entry_Anchor_Info *anchor_info = (Elm_Entry_Anchor_Info *)event_info;
	LS_LOGD("Anchor is clicked.... name [%s]", (anchor_info->name) ? (anchor_info->name) : "NULL");

	app_control_h app_control = NULL;
	int ret = APP_CONTROL_ERROR_NONE;
	do {
		ret = app_control_create(&app_control);
		if (ret != APP_CONTROL_ERROR_NONE) {
			LS_LOGE("[Error:%d]Fail to create handler", ret);
			break;
		}

		ret = app_control_set_operation(app_control, APP_CONTROL_OPERATION_DEFAULT);
		if (ret != APP_CONTROL_ERROR_NONE) {
			LS_LOGE("[Error:%d]Fail to set operation", ret);
			break;
		}

		ret = app_control_set_app_id(app_control, "org.tizen.browser");
		if (ret != APP_CONTROL_ERROR_NONE) {
			LS_LOGE("[Error:%d]Fail to set app id", ret);
			break;
		}

		ret = app_control_set_uri(app_control, anchor_info->name);
		if (ret != APP_CONTROL_ERROR_NONE) {
			LS_LOGE("[Error:%d]Fail to set url [%s]", ret, anchor_info->name);
			break;
		}

		ret = app_control_send_launch_request(app_control, _app_control_reply_cb, NULL);
		if (ret != APP_CONTROL_ERROR_NONE) {
			LS_LOGE("[Error:%d]Fail to send launch request", ret);
			break;
		}
	} while (FALSE);

	if (app_control) {
		app_control_destroy(app_control);
		app_control = NULL;
	}

	LS_FUNC_EXIT
}

static void __setting_reply_gps_wifi_status(void *data)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(data);

	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	if (ad->prev_handler == NULL) {
		LS_LOGE("[Error]service handler is NULL");
		return;
	}

	bool is_reply_requested = FALSE;
	int ret = app_control_is_reply_requested(ad->prev_handler, &is_reply_requested);
	if (ret != APP_CONTROL_ERROR_NONE) {
		LS_LOGE("[Error:%d]Fail to check reply request", ret);
		return;
	} else if (is_reply_requested == FALSE) {
		LS_LOGD("Reply is not requested");
		return;
	}

	app_control_h reply = NULL;
	app_control_result_e reply_err = APP_CONTROL_RESULT_FAILED;
	do {
		/* send reply */
		ret = app_control_create(&reply);
		if (ret != APP_CONTROL_ERROR_NONE) {
			LS_LOGE("[Error:%d]Fail to create control", ret);
			break;
		}

		/* Get Vconfkey value */
		int is_gps_on = -1;
		ret = vconf_get_int(VCONFKEY_LOCATION_ENABLED, &is_gps_on);
		if (ret != VCONF_OK) {
			LS_LOGE("[Error]Fail to get vconf value");
			break;
		}

		/* add key-value data to handler */
		ret = app_control_add_extra_data(reply, REPLY_KEY_GPS, is_gps_on ? REPLY_VALUE_ON : REPLY_VALUE_OFF);
		if (ret != VCONF_OK) {
			LS_LOGE("[Error]Fail to get gps vconf value");
			break;
		}

		/* Get Vconfkey value */
		int is_wifi_on = -1;
		ret = vconf_get_int(VCONFKEY_LOCATION_NETWORK_ENABLED, &is_wifi_on);
		if (ret != VCONF_OK) {
			LS_LOGE("[Error]Fail to get vconf value");
			break;
		}
		ret = app_control_add_extra_data(reply, REPLY_KEY_WIFI, is_wifi_on ? REPLY_VALUE_ON : REPLY_VALUE_OFF);
		if (ret != VCONF_OK) {
			LS_LOGE("[Error]Fail to get wifi vconf value");
			break;
		}

		reply_err = APP_CONTROL_RESULT_SUCCEEDED;
	} while (FALSE);

	if (reply) {
		ret = app_control_reply_to_launch_request(reply, ad->prev_handler, reply_err);
		if (ret != APP_CONTROL_ERROR_NONE)
			LS_LOGE("[Error:%d]Fail to send reply", ret);

		app_control_destroy(reply);
		reply = NULL;
		app_control_destroy(ad->prev_handler);
		ad->prev_handler = NULL;
	}
}

/**
 ************************************************************************
 * create view for setup wizard
 ************************************************************************
 */

#define BG_PORTRAIT			"A01-9_image_dropbox_bg.png"
#define BG_LANDSCAPE		"A01-9_image_dropbox_bg_h.png"

static void _change_indicator_style(lbs_setting_app_data *ad, Eina_Bool isHeaderbg)
{
	LS_FUNC_ENTER
	if (isHeaderbg == EINA_TRUE) {
		elm_object_signal_emit(ad->conformant, "elm,state,indicator,nooverlap", "elm");
		Evas_Object *conform_bg = elm_object_part_content_get(ad->conformant, "elm.swallow.indicator_bg");
		elm_object_style_set(conform_bg, "indicator/headerbg");
	} else {
		elm_object_signal_emit(ad->conformant, "elm,state,indicator,overlap", "elm");
		Evas_Object *conform_bg = elm_object_part_content_get(ad->conformant, "elm.swallow.indicator_bg");
		elm_object_style_set(conform_bg, "indicator/transparent");
	}
	LS_FUNC_EXIT
}

#if 0
static void _launch_layout_ug_cb(ui_gadget_h ug, enum ug_mode mode, void *priv)
{
	Evas_Object *base;
	if (!priv)
		return;

	base = (Evas_Object *) ug_get_layout(ug);
	ug_disable_effect(ug);
	if (!base)
		return;

	switch (mode) {
	case UG_MODE_FULLVIEW:
		evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_show(base);
		break;
	default:
		break;
	}
}
static void _launch_destroy_ug_cb(ui_gadget_h ug, void *priv)
{
	if (!priv)
		return;

	if (ug)
		ug_destroy(ug);
}
#endif

static void _lbutton_click_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(data);
	app_control_h service;
	int ret;
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;
	app_control_result_e reply_err = APP_CONTROL_RESULT_FAILED;

	ret = app_control_create(&service);
	if (ret != APP_CONTROL_ERROR_NONE) {
		LS_LOGE("app_control_create failed: %d", ret);
		return;
	}

	app_control_add_extra_data(service, "result", "lbutton_click");
	app_control_reply_to_launch_request(service, ad->prev_handler, reply_err);

	app_control_destroy(service);
	LS_FUNC_EXIT
	elm_exit();
}

static void _rbutton_click_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(data);
	app_control_h service;
	int ret;
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;
	app_control_result_e reply_err = APP_CONTROL_RESULT_FAILED;

	ret = app_control_create(&service);
	if (ret != APP_CONTROL_ERROR_NONE) {
		LS_LOGE("app_control_create failed: %d", ret);
		return;
	}

	app_control_add_extra_data(service, "result", "rbutton_click");
	app_control_reply_to_launch_request(service, ad->prev_handler, reply_err);

	app_control_destroy(service);
	LS_FUNC_EXIT
	elm_exit();
}

static void _ea_setup_wizard_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(data);
	int *screen_type = evas_object_data_get(obj, CURRENT_SCREEN_TYPE_ID);
	if (*screen_type != LOCATION_WIZARD_VIEW) {
		eext_naviframe_back_cb(data, obj, event_info);
		return;
	}
	_lbutton_click_cb(data, obj, event_info);
	LS_FUNC_EXIT
}

static void __setting_location_wizard_location_privicy_ug(void *data, Evas_Object *obj, void *event_info)
{
	LS_FUNC_ENTER
#if 0
	LS_RETURN_IF_FAILED(data);
	app_control_h service;
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;
	struct ug_cbs *cbs = (struct ug_cbs *)calloc(1, sizeof(struct ug_cbs));
	int ret = -1;
	LS_RETURN_IF_FAILED(cbs);

	UG_INIT_EFL(ad->win_main, UG_OPT_INDICATOR_ENABLE);

	ret = app_control_create(&service);
	if (ret != APP_CONTROL_ERROR_NONE) {
		LS_LOGE("app_control_create failed: %d", ret);
		free(cbs);
		return;
	}
	app_control_add_extra_data(service, "service_type", "location");

	cbs->layout_cb = _launch_layout_ug_cb;
	cbs->destroy_cb = _launch_destroy_ug_cb;
	cbs->priv = (void *)ad;
	_change_indicator_style(ad, EINA_TRUE);

	ui_gadget_h loading = ug_create(NULL, "setting-privacy-efl", UG_OPT_INDICATOR_ENABLE, service, cbs);
	if (NULL == loading)
		LS_LOGE("launch setting-privacy-efl failed");

	app_control_destroy(service);
	free(cbs);
	LS_FUNC_EXIT
#endif
}

#if 0
void __set_window_top(lbs_setting_app_data *ad)
{
	LS_FUNC_ENTER
	Ecore_X_Window w = elm_win_xwindow_get(ad->win_main);
	ecore_x_netwm_window_type_set(w, ECORE_X_WINDOW_TYPE_NOTIFICATION);
	utilx_set_system_notification_level(ecore_x_display_get(), w, UTILX_NOTIFICATION_LEVEL_LOW);
	LS_FUNC_EXIT
}
#endif

static void _setting_wizard_check_cb(void *data, Evas_Object *obj, void *event_info)
{
	LS_RETURN_IF_FAILED(data);
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;

	LS_RETURN_IF_FAILED(ad->loc_check);
	Evas_Object *check = ad->loc_check;

	if (check) {
		Eina_Bool state = elm_check_state_get(check);
		LS_LOGD("state:%d", state);
		if (state == EINA_FALSE) {
			__setting_location_set_int(VCONFKEY_LOCATION_USE_MY_LOCATION, KEY_DISABLED);
			__setting_location_set_int(VCONFKEY_LOCATION_ENABLED, KEY_DISABLED);
			edje_object_signal_emit(_EDJ(ad->wizard_layout), "location,dimmed", "elm");
			elm_object_disabled_set(ad->allow_button, EINA_TRUE);
			evas_object_pass_events_set(ad->help_scroller, EINA_TRUE);
			evas_object_pass_events_set(ad->allow_scroller, EINA_TRUE);
		} else {
			__setting_location_set_int(VCONFKEY_LOCATION_USE_MY_LOCATION, KEY_ENABLED);
			__setting_location_set_int(VCONFKEY_LOCATION_ENABLED, KEY_ENABLED);
			edje_object_signal_emit(_EDJ(ad->wizard_layout), "location,default", "elm");
			elm_object_disabled_set(ad->allow_button, EINA_FALSE);
			evas_object_pass_events_set(ad->help_scroller, EINA_FALSE);
			evas_object_pass_events_set(ad->allow_scroller, EINA_FALSE);
		}
	}
}

static char *_setting_wizard_text_get(void *data, Evas_Object *obj, const char *part)
{
	LS_FUNC_ENTER

	if (!g_strcmp0(part, "elm.text"))
		return strdup(P_("IDS_ST_MBODY_USE_CURRENT_LOCATION"));

	return NULL;
}

static Evas_Object *_setting_wizard_check_get(void *data, Evas_Object *obj, const char *part)
{
	lbs_setting_app_data *ad = (lbs_setting_app_data *)data;
	Evas_Object *ly = NULL;

	LS_LOGD("part:%s", part);

	if (!strcmp(part, "elm.swallow.icon.1")) {
		ly = elm_layout_add(obj);
		elm_layout_theme_set(ly, "layout", "list/C/type.3", "default");

		ad->loc_check = elm_check_add(ly);

		int value = -1;
		int ret = vconf_get_int(VCONFKEY_LOCATION_USE_MY_LOCATION, &value);
		if (ret != VCONF_OK)
			LS_LOGE("fail to get vconf key!");

		if (value)
			elm_check_state_set(ad->loc_check, EINA_TRUE);
		else
			elm_check_state_set(ad->loc_check, EINA_FALSE);

		elm_object_style_set(ad->loc_check, "on&off");
		evas_object_propagate_events_set(ad->loc_check, EINA_FALSE);
		evas_object_smart_callback_add(ad->loc_check, "changed", _setting_wizard_check_cb, ad);
		evas_object_show(ad->loc_check);
		elm_layout_content_set(ly, "elm.swallow.content", ad->loc_check);
	}

	return ly;
}

void set_dim_view(lbs_setting_app_data *ad)
{
	LS_FUNC_ENTER
	int value = -1;
	int ret = vconf_get_int(VCONFKEY_LOCATION_USE_MY_LOCATION, &value);
	if (ret != VCONF_OK)
		LS_LOGE("fail to get vconf key!");

	if (value == KEY_DISABLED) {
		edje_object_signal_emit(_EDJ(ad->wizard_layout), "location,dimmed", "elm");
		elm_object_disabled_set(ad->allow_button, EINA_TRUE);
		evas_object_pass_events_set(ad->help_scroller, EINA_TRUE);
		evas_object_pass_events_set(ad->allow_scroller, EINA_TRUE);
	} else {
		edje_object_signal_emit(_EDJ(ad->wizard_layout), "location,default", "elm");
		elm_object_disabled_set(ad->allow_button, EINA_FALSE);
		evas_object_pass_events_set(ad->help_scroller, EINA_FALSE);
		evas_object_pass_events_set(ad->allow_scroller, EINA_FALSE);
	}
}

Evas_Object *__setting_location_wizard_view(lbs_setting_app_data *ad)
{
	LS_FUNC_ENTER
	LS_RETURN_VAL_IF_FAILED(ad, NULL);

	/*	__set_window_top(ad); */
	_change_indicator_style(ad, EINA_TRUE);
	ad->view_id = LOCATION_WIZARD_VIEW;

	ad->nf = elm_naviframe_add(ad->layout_main);
	LS_RETURN_VAL_IF_FAILED(ad->nf, NULL);
	elm_object_part_content_set(ad->layout_main, "elm.swallow.content", ad->nf);

	Evas_Object *layout = elm_layout_add(ad->nf);
	LS_RETURN_VAL_IF_FAILED(layout, NULL);

	elm_layout_file_set(layout, LBS_SETTING_EDJ, "wizard");
	ad->wizard_layout = layout;
	evas_object_show(layout);

	Evas_Object *genlist = elm_genlist_add(ad->nf);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	elm_object_part_content_set(layout, "wizard.genlist", genlist);
	evas_object_show(genlist);

	Elm_Genlist_Item_Class *itc_wizard;

	itc_wizard = elm_genlist_item_class_new();
	if (itc_wizard == NULL) {
		LS_LOGE("itc_wizard is NULL");
		return layout;
	}

	itc_wizard->item_style = "type1";
	itc_wizard->func.text_get = _setting_wizard_text_get;
	itc_wizard->func.content_get = _setting_wizard_check_get;
	ad->itc_1 = elm_genlist_item_append(genlist, itc_wizard, (void *)ad, NULL, ELM_GENLIST_ITEM_NONE, NULL, ad);
	elm_genlist_item_select_mode_set(ad->itc_1, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	Evas_Object *help_scroller = elm_scroller_add(ad->nf);
	ad->help_scroller = help_scroller;
	elm_scroller_bounce_set(help_scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(help_scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(help_scroller);

	char *buf;
	char *text = P_("IDS_ST_BODY_ALLOW_APPLICATIONS_TO_USE_GPS_SATELLITES_TO_PINPOINT_YOUR_LOCATION");

	buf = g_strdup_printf("<font=%s><font_size=%d><color=#%s><color_class=%s>%s</color_class></color></font_size></font>",
						"Tizen:style=Regular", 25, "080808", "T0231", text);

	elm_theme_extension_add(NULL, LBS_SETTING_EDJ);

	Evas_Object *help_label = elm_label_add(help_scroller);
	elm_object_style_set(help_label, "popup/custom_default");
	evas_object_size_hint_align_set(help_label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(help_label, EVAS_HINT_EXPAND, 0);
	elm_label_line_wrap_set(help_label, ELM_WRAP_MIXED);
	elm_object_text_set(help_label, buf);
	evas_object_show(help_label);
	evas_object_smart_callback_add(help_label, "anchor,clicked", _anchor_clicked_cb, ad);
	elm_object_content_set(help_scroller, help_label);
	elm_object_part_content_set(layout, "sw.help.text", help_scroller);

	Evas_Object *allow_scroller = elm_scroller_add(layout);
	ad->allow_scroller = allow_scroller;
	elm_scroller_bounce_set(allow_scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(allow_scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(allow_scroller);

	Evas_Object *allow_label = elm_label_add(allow_scroller);
	elm_label_line_wrap_set(allow_label, ELM_WRAP_MIXED);
	elm_object_style_set(allow_label, "popup/custom_default");

	char *allow_buf;
	char *allow_str = LOCATION_SETUP_WIZARD_HELP_3;

	allow_buf = g_strdup_printf("<font=%s><font_size=%d><color=#%s><color_class=%s>%s</color_class></color></font_size></font>",
								"Tizen:style=Regular", 25, "080808", "T0231", allow_str);

	elm_object_text_set(allow_label, allow_buf);
	elm_object_focus_allow_set(allow_label, EINA_FALSE);
	evas_object_size_hint_weight_set(allow_label, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(allow_label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(allow_label);

	elm_object_content_set(allow_scroller, allow_label);
	elm_object_part_content_set(layout, "sw.allow.text", allow_scroller);

	ad->allow_button = elm_button_add(layout);
	elm_object_domain_translatable_text_set(ad->allow_button, LBS_SETTING_PKG, "IDS_STU_BUTTON_ALLOWED_APPLICATIONS_ABB");

	evas_object_smart_callback_add(ad->allow_button, "clicked", __setting_location_wizard_location_privicy_ug, ad);
	elm_object_part_content_set(layout, "allow.btn", ad->allow_button);

	Evas_Object *prev_btn = elm_button_add(ad->nf);
	elm_object_style_set(prev_btn, "bottom");
	elm_object_part_content_set(layout, "button.prev", prev_btn);
	elm_object_text_set(prev_btn, P_("IDS_STU_BUTTON_PREVIOUS"));
	evas_object_smart_callback_add(prev_btn, "clicked", _lbutton_click_cb, ad);
	ad->prev_button = prev_btn;

	Evas_Object *next_btn = elm_button_add(ad->nf);
	elm_object_style_set(next_btn, "bottom");
	elm_object_part_content_set(layout, "button.next", next_btn);
	elm_object_text_set(next_btn, P_("IDS_ST_BUTTON_NEXT"));
	evas_object_smart_callback_add(next_btn, "clicked", _rbutton_click_cb, ad);

	ad->next_button = next_btn;

	elm_naviframe_item_push(ad->nf, P_("IDS_ST_BUTTON2_LOCATION"), NULL, NULL, layout, NULL);
	eext_object_event_callback_add(ad->nf, EEXT_CALLBACK_BACK, _ea_setup_wizard_back_cb, ad);

	set_dim_view(ad);

	LS_FUNC_EXIT

	return layout;
}

void lbs_setting_win_show(lbs_setting_app_data *ad)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(ad);
	evas_object_show(ad->win_main);

	if (ad->layout_main)
		evas_object_show(ad->layout_main);

	if (ad->nf)
		evas_object_show(ad->nf);
}

void lbs_setting_win_activate(lbs_setting_app_data *ad)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(ad);
	elm_win_activate(ad->win_main);
}

void lbs_setting_win_deactivate(lbs_setting_app_data *ad)
{
	LS_FUNC_ENTER
	LS_RETURN_IF_FAILED(ad);
	elm_win_lower(ad->win_main);
}
