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

#include <stdio.h>
#include <app.h>
#include <glib.h>
#include <stdio.h>
#include <efl_extension.h>
#include <E_DBus.h>
#include <appcore-efl.h>
#include <vconf.h>
#include <vconf-internal-location-keys.h>
#include <eventsystem.h>


#include "gps-syspopup.h"

#define LOCATION_MENU_GPS			0
#define LOCATION_MENU_AGREE			1
#define LOCATION_MENU_DISAGREE		2
#define LOCATION_MENU_USER_CONSENT	3
#define LOCATION_MDM_CONTENT		4
#define LOCATION_BUTTON_OK			5

#define GPS_ENABLED		1
#define GPS_DISABLED	0

#define QUICKPANEL_DBUS_PATH		"/Org/Tizen/Quickpanel"
#define QUICKPANEL_DBUS_INTERFACE	"org.tizen.quickpanel"
#define QUICKPANEL_DBUS_NAME		"ACTIVITY"

static int __gps_popupsend_signal_to_quickpanel(void *data, const char *command);
Evas_Object *popup_type_create(void *data, int popup_type);

int myterm(bundle *b, void *data)
{
	return 0;
}

int mytimeout(bundle *b, void *data)
{
	return 0;
}

syspopup_handler handler = {
	.def_term_fn = myterm,
	.def_timeout_fn = mytimeout
};

enum {
	POPUP_GPS,
	POPUP_WIRELESS,
	POPUP_GPS_QP, /*for quickpanel */
	POPUP_WIRELESS_QP
};

#ifdef TIZEN_FEATURE_WPS
static void _app_control_reply_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	if(result != APP_CONTROL_RESULT_SUCCEEDED) {
		GPS_POPUP_LOG("[Error:%d]Launch request fail", result);
		return;
	}

	GPS_POPUP_LOG("Success to launch request");
}

static gboolean _destroy_wireless_popup(gpointer data)
{
	GPS_POPUP_LOG("ENTER");
	struct gps_popup_appdata *ad = (struct gps_popup_appdata *)data;

	int enabled = 0;

	vconf_get_int(VCONFKEY_LOCATION_USE_MY_LOCATION, &enabled);
	if (enabled == 0) {
		__gps_popupsend_signal_to_quickpanel(ad, "OFF");
	} else {
		__gps_popupsend_signal_to_quickpanel(ad, "ON");
	}

	evas_object_del(ad->wireless_popup_setting);
	ad->wireless_popup_setting = NULL;
	elm_exit();

	return false;
}

static void _anchor_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	GPS_POPUP_LOG("ENTER");

	Elm_Entry_Anchor_Info *anchor_info = (Elm_Entry_Anchor_Info *)event_info;

	app_control_h app_control = NULL;
	int ret = APP_CONTROL_ERROR_NONE;
	do {
		ret = app_control_create(&app_control);
		if(ret != APP_CONTROL_ERROR_NONE) {
			GPS_POPUP_LOG("[Error:%d]Fail to create handler", ret);
			break;
		}

		ret = app_control_set_operation(app_control, APP_CONTROL_OPERATION_DEFAULT);
		if(ret != APP_CONTROL_ERROR_NONE) {
			GPS_POPUP_LOG("[Error:%d]Fail to set operation", ret);
			break;
		}

		ret = app_control_set_app_id(app_control, "org.tizen.browser");
		if(ret != APP_CONTROL_ERROR_NONE) {
			GPS_POPUP_LOG("[Error:%d]Fail to set app id", ret);
			break;
		}

		ret = app_control_set_uri(app_control, anchor_info->name);
		if(ret != APP_CONTROL_ERROR_NONE) {
			GPS_POPUP_LOG("[Error:%d]Fail to set url [%s]", ret, anchor_info->name);
			break;
		}

		ret = app_control_send_launch_request(app_control, _app_control_reply_cb, NULL);
		if(ret != APP_CONTROL_ERROR_NONE) {
			GPS_POPUP_LOG("[Error:%d]Fail to send launch request", ret);
			break;
		}
	} while(FALSE);

	if(app_control) {
		app_control_destroy(app_control);
		app_control = NULL;
	}
}

static void location_consent_popup_disagree_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct gps_popup_appdata *ad = (struct gps_popup_appdata *)data;

	int enabled = 0;

	vconf_get_int(VCONFKEY_LOCATION_USE_MY_LOCATION, &enabled);
	if (enabled == 0) {
		__gps_popupsend_signal_to_quickpanel(ad, "OFF");
	} else {
		__gps_popupsend_signal_to_quickpanel(ad, "ON");
	}

	evas_object_del(ad->wireless_popup_setting);
	ad->wireless_popup_setting = NULL;
	elm_exit();
}

static void location_consent_popup_agree_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct gps_popup_appdata *ad = (struct gps_popup_appdata *)data;
	int enable = 0;

	vconf_get_int(VCONFKEY_LOCATION_USE_MY_LOCATION, &enable);
	if (enable == 0) {
		vconf_set_int(VCONFKEY_LOCATION_USE_MY_LOCATION, 1);
	}

	vconf_set_int(VCONFKEY_LOCATION_NETWORK_ENABLED, 1);

	__gps_popupsend_signal_to_quickpanel(ad, "ON");

	evas_object_del(ad->wireless_popup_setting);
	ad->wireless_popup_setting = NULL;
	elm_exit();
}
#endif


/* Callback function for the mouse up event */
static void __mouseup_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	GPS_POPUP_LOG("ENTER");
	GPS_POPUP_RETURN_IF_FAILED(event_info);
	GPS_POPUP_RETURN_IF_FAILED(obj);
	Evas_Event_Mouse_Up *ev = event_info;
	if (ev->button == 3) {
		/* if mouse right button is up */
		evas_object_del(obj);
	}
}

static int __gps_popup_init_edbus_event(struct gps_popup_appdata *ad)
{
	GPS_POPUP_LOG("ENTER");
	GPS_POPUP_RETURN_VAL_IF_FAILED(ad, -1);
	E_DBus_Connection *dbus_connection = NULL;

	e_dbus_init();

	dbus_connection = e_dbus_bus_get(DBUS_BUS_SYSTEM);
	if (dbus_connection == NULL) {
		GPS_POPUP_LOG("failed to get system bus");
		return -1;
	}

	ad->dbus_connection = dbus_connection;
	return 0;
}

static int __gps_popup_close_edbus_event(struct gps_popup_appdata *ad)
{
	GPS_POPUP_LOG("ENTER");
	GPS_POPUP_RETURN_VAL_IF_FAILED(ad, -1);
	GPS_POPUP_RETURN_VAL_IF_FAILED(ad->dbus_connection, -1);
	E_DBus_Connection *dbus_connection = ad->dbus_connection;

	if (dbus_connection != NULL) {
		e_dbus_connection_close(dbus_connection);
		dbus_connection = NULL;
	}

	e_dbus_shutdown();
	return 0;
}

static int __gps_popupsend_signal_to_quickpanel(void *data, const char *command)
{
	GPS_POPUP_LOG("ENTER");
	GPS_POPUP_RETURN_VAL_IF_FAILED(data, -1);
	GPS_POPUP_RETURN_VAL_IF_FAILED(command, -1);
	struct gps_popup_appdata *ad = (struct gps_popup_appdata *)data;
	GPS_POPUP_RETURN_VAL_IF_FAILED(ad->dbus_connection, -1);
	dbus_bool_t ret;
	DBusMessage *signal = NULL;
	char *module = "gps";
	E_DBus_Connection *dbus_connection = ad->dbus_connection;

	signal = dbus_message_new_signal(QUICKPANEL_DBUS_PATH,
									 QUICKPANEL_DBUS_INTERFACE,
									 QUICKPANEL_DBUS_NAME);
	if (signal == NULL) {
		GPS_POPUP_LOG("Failed to dbus_message_new_signal");
		return -1;
	}

	ret = dbus_message_append_args(signal,
								DBUS_TYPE_STRING, &module,
								DBUS_TYPE_STRING, &command,
								DBUS_TYPE_INVALID);

	if (!ret) {
		GPS_POPUP_LOG("Failed to dbus_message_append_args");
		return -1;
	}

	e_dbus_message_send(dbus_connection, signal, NULL, 0, NULL);

	dbus_message_unref(signal);

	return 0;
}

static void __gps_popup_win_del(void *data, Evas_Object *obj, void *event)
{
	GPS_POPUP_LOG("ENTER");
	elm_exit();
}

static Evas_Object *__gps_popup_create_win(const char *name)
{
	GPS_POPUP_LOG("ENTER");
	GPS_POPUP_RETURN_VAL_IF_FAILED(name, NULL);
	Evas_Object *eo;

	eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
	if (eo) {
		elm_win_title_set(eo, name);
		elm_win_borderless_set(eo, EINA_TRUE);
		evas_object_smart_callback_add(eo, "delete,request", __gps_popup_win_del, NULL);
		elm_win_alpha_set(eo, EINA_TRUE);
	}

	return eo;
}

static void _popup_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	GPS_POPUP_LOG("ENTER");
	GPS_POPUP_RETURN_IF_FAILED(data);
	struct gps_popup_appdata *ad = (struct gps_popup_appdata *)data;

	if (ad->gps_popup) {
		evas_object_del(ad->gps_popup);
		ad->gps_popup = NULL;
	}
	elm_exit();
}

static void __gps_popup_gps_agree_cb(void *data, Evas_Object *obj, void *event_info)
{
	GPS_POPUP_LOG("ENTER");
	GPS_POPUP_RETURN_IF_FAILED(data);
	struct gps_popup_appdata *ad = (struct gps_popup_appdata *)data;
	int enabled = 0;

	Eina_Bool ischoose = false;
	if (ad->ask_check) {
		ischoose = elm_check_state_get(ad->ask_check);
		if (ischoose) {
			vconf_set_int(VCONFKEY_LBS_SETTING_IS_SHOW_GPS_POPUP, 0);
		}
	}

	if (ad->gps_popup) {
		evas_object_del(ad->gps_popup);
	}

	vconf_get_int(VCONFKEY_LOCATION_USE_MY_LOCATION, &enabled);
	if (enabled == 0) {
		vconf_set_int(VCONFKEY_LOCATION_USE_MY_LOCATION, 1);
	}
	vconf_set_int(VCONFKEY_LOCATION_ENABLED, 1);
	__gps_popupsend_signal_to_quickpanel(ad, "ON");

	if (ad->gps_popup) {
		evas_object_del(ad->gps_popup);
		ad->gps_popup = NULL;
	}

#ifdef TIZEN_FEATURE_WPS
	ad->wireless_popup_setting = popup_type_create(data, POPUP_WIRELESS_QP);
#else
	elm_exit();
#endif
}

static void __gps_popup_gps_disagree_cb(void *data, Evas_Object *obj, void *event_info)
{
	GPS_POPUP_LOG("ENTER");
	GPS_POPUP_RETURN_IF_FAILED(data);
	struct gps_popup_appdata *ad = (struct gps_popup_appdata *)data;
	int enabled = 0;

	vconf_get_int(VCONFKEY_LOCATION_USE_MY_LOCATION, &enabled);
	if (enabled == 0) {
		__gps_popupsend_signal_to_quickpanel(ad, "OFF");
	} else {
		__gps_popupsend_signal_to_quickpanel(ad, "ON");
	}

	ad->wireless_popup_setting = popup_type_create(data, POPUP_WIRELESS_QP);

	if (ad->gps_popup) {
		evas_object_del(ad->gps_popup);
		ad->gps_popup = NULL;
	}
#ifndef TIZEN_FEATURE_WPS
	elm_exit();
#endif
}

static void _popup_gps_agree_cb_for_setting_search(void *data, Evas_Object *obj, void *event_info)
{
	GPS_POPUP_LOG("ENTER");
	GPS_POPUP_RETURN_IF_FAILED(data);
	struct gps_popup_appdata *ad = (struct gps_popup_appdata *)data;
	int enabled = 0;

	if (ad->gps_popup_setting) {
		evas_object_del(ad->gps_popup_setting);
		ad->gps_popup_setting = NULL;
	}

	if (ad->wireless_popup_setting == NULL) {
		elm_exit();
	}

	vconf_get_int(VCONFKEY_LOCATION_USE_MY_LOCATION, &enabled);
	if (enabled == 0) {
		vconf_set_int(VCONFKEY_LOCATION_USE_MY_LOCATION, 1);
	}
	vconf_set_int(VCONFKEY_LOCATION_ENABLED, 1);
}

static void _popup_gps_disagree_cb_for_setting_search(void *data, Evas_Object *obj, void *event_info)
{
	GPS_POPUP_LOG("ENTER");
	GPS_POPUP_RETURN_IF_FAILED(data);
	struct gps_popup_appdata *ad = (struct gps_popup_appdata *)data;

	if (ad->gps_popup_setting) {
		evas_object_del(ad->gps_popup_setting);
		ad->gps_popup_setting = NULL;
	}

	if (ad->wireless_popup_setting == NULL) {
		elm_exit();
	}

	vconf_set_int(VCONFKEY_LOCATION_USE_MY_LOCATION, 0);
	vconf_set_int(VCONFKEY_LOCATION_ENABLED, 0);
}

Evas_Object *popup_type_create(void *data, int popup_type)
{
	GPS_POPUP_LOG("ENTER");
	GPS_POPUP_RETURN_VAL_IF_FAILED(data, NULL);
	struct gps_popup_appdata *ad = (struct gps_popup_appdata *)data;
	Evas_Object *popup = NULL;
	Evas_Object *agree_btn = NULL;
	Evas_Object *disagree_btn = NULL;

	popup = elm_popup_add(ad->layout);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, _popup_back_cb, ad);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP, __mouseup_cb, NULL);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	if ((popup_type == POPUP_GPS) || (popup_type == POPUP_GPS_QP)) {
		elm_object_domain_translatable_part_text_set(popup, "title,text", PACKAGE_NAME, "IDS_ST_HEADER_CONSENT_TO_LOCATION_INFO_USAGE_ABB");
//		elm_object_domain_translatable_text_set(popup, PACKAGE_NAME, "IDS_ST_POP_YOUR_LOCATION_DATA_INCLUDING_GPS_DATA_WILL_BE_USED_BY_RELEVANT_APPLICATIONS");

		/* layout */
		Evas_Object *layout = elm_layout_add(popup);
		elm_layout_file_set(layout, GPS_EDJ, "popup_gps_layout");
		evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		/* check */
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
		elm_object_domain_translatable_text_set(label, PACKAGE_NAME, "IDS_ST_POP_YOUR_LOCATION_DATA_INCLUDING_GPS_DATA_WILL_BE_USED_BY_RELEVANT_APPLICATIONS");
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_EXPAND);
		elm_object_content_set(scroller, label);
		elm_object_part_content_set(layout, "elm.swallow.content", scroller);
		elm_object_content_set(popup, layout);

#ifdef TIZEN_FEATURE_WPS
	} else if(popup_type == POPUP_WIRELESS_QP){
		char text[4096] = {0};
		char *str1 = P_("IDS_POSITIONING_CONSENT_BODY");
		char *str2 = "</br><color=#006fd1ff underline=on underline_color=#006fd1ff><a href=http://here.com/terms/service-terms>http://here.com/terms/service-terms</a></color></br>";
		char *str3 = "</br><color=#006fd1ff underline=on underline_color=#006fd1ff><a href=http://here.com/privacy/privacy-policy>http://here.com/privacy/privacy-policy</a></color></br>";
		snprintf(text,4096, str1, str2, str3);

		elm_object_domain_translatable_part_text_set(popup, "title,text", PACKAGE_NAME, "IDS_ST_HEADER_LOCATION_LEGAL_INFORMATION_ABB");
//		elm_object_domain_translatable_text_set(popup, PACKAGE_NAME, text);

		Evas_Object *layout = elm_layout_add(popup);
		elm_layout_file_set(layout, GPS_EDJ, "popup_checkview_layout");
		evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		Evas_Object * help_scroller = elm_scroller_add(popup);
		elm_scroller_bounce_set(help_scroller, EINA_FALSE, EINA_TRUE);
		elm_scroller_policy_set(help_scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
		elm_object_style_set(help_scroller, "effect");
		evas_object_show(help_scroller);

		Evas_Object *label = elm_entry_add(help_scroller);
		elm_object_style_set(label, "popup/default");
		elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
		elm_object_text_set(label, text);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_EXPAND);
		evas_object_smart_callback_add(label, "anchor,clicked", _anchor_clicked_cb, ad);
		elm_entry_editable_set(label, EINA_FALSE);
		elm_entry_input_panel_enabled_set(label, EINA_FALSE);
		elm_entry_context_menu_disabled_set(label, EINA_TRUE);

		elm_object_content_set(help_scroller, label);
		elm_object_part_content_set(layout, "elm.swallow.content", help_scroller);
		elm_object_content_set(popup, layout);
#endif
	}

	disagree_btn = elm_button_add(popup);
	elm_object_style_set(disagree_btn, "popup");
	elm_object_domain_translatable_text_set(disagree_btn, PACKAGE_NAME, "IDS_ST_BUTTON_DISAGREE");
	elm_object_part_content_set(popup, "button1", disagree_btn);
	if (popup_type == POPUP_GPS) {
		evas_object_smart_callback_add(disagree_btn, "clicked", _popup_gps_disagree_cb_for_setting_search, ad);
	} else if (popup_type == POPUP_GPS_QP) {
		evas_object_smart_callback_add(disagree_btn, "clicked", __gps_popup_gps_disagree_cb, ad);
#ifdef TIZEN_FEATURE_WPS
	} else if(popup_type == POPUP_WIRELESS_QP){
		evas_object_smart_callback_add(disagree_btn, "clicked", location_consent_popup_disagree_cb, ad);
#endif
	}
	agree_btn = elm_button_add(popup);
	elm_object_style_set(agree_btn, "popup");
	elm_object_domain_translatable_text_set(agree_btn, PACKAGE_NAME, "IDS_ST_BUTTON_AGREE");
	elm_object_part_content_set(popup, "button2", agree_btn);
	if (popup_type == POPUP_GPS) {
		evas_object_smart_callback_add(agree_btn, "clicked", _popup_gps_agree_cb_for_setting_search, ad);
	} else if (popup_type == POPUP_GPS_QP) {
		evas_object_smart_callback_add(agree_btn, "clicked", __gps_popup_gps_agree_cb, ad);
#ifdef TIZEN_FEATURE_WPS
	} else if(popup_type == POPUP_WIRELESS_QP){
		evas_object_smart_callback_add(agree_btn, "clicked", location_consent_popup_agree_cb, ad);
#endif
	}
	evas_object_show(popup);
	evas_object_show(ad->win_main);
	elm_object_focus_next_object_set(agree_btn, disagree_btn, ELM_FOCUS_NEXT);

	return popup;
}

int __gps_popup_create(void *data)
{
	GPS_POPUP_LOG("ENTER");
	GPS_POPUP_RETURN_VAL_IF_FAILED(data, -1);
	struct gps_popup_appdata *ad = data;
	Evas_Object *win = NULL;
	int ret = -1;

	elm_app_base_scale_set(1.8);
	win = __gps_popup_create_win(PACKAGE);
	GPS_POPUP_RETURN_VAL_IF_FAILED(win, -1);
	ad->win_main = win;

	Evas_Object *conformant = elm_conformant_add(ad->win_main);
	elm_win_conformant_set(ad->win_main, EINA_TRUE);
	elm_win_resize_object_add(ad->win_main, conformant);
	evas_object_size_hint_weight_set(conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(conformant, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(conformant);
	ad->conform = conformant;

	Evas_Object *layout = elm_layout_add(conformant);
	elm_object_content_set(conformant, layout);
	ad->layout = layout;
	evas_object_show(layout);


	ad->wireless_popup_setting = NULL;
	ad->gps_popup_setting = NULL;

	bindtextdomain(PACKAGE_NAME, GPSPOPUP_COMMON_RES);

	ret = __gps_popup_init_edbus_event(ad);
	if (ret < 0) {
		GPS_POPUP_LOG("ERROR: Fail to init edbus event !!!");
		return -1;
	}

	return 0;
}

int __gps_popup_terminate(void *data)
{
	GPS_POPUP_LOG("ENTER");
	GPS_POPUP_RETURN_VAL_IF_FAILED(data, -1);
	struct gps_popup_appdata *ad = data;
	int ret = -1;

	if (ad->gps_popup) {
		evas_object_del(ad->gps_popup);
	}

	if (ad->win_main) {
		evas_object_del(ad->win_main);
	}

	ret = __gps_popup_close_edbus_event(ad);
	if (ret < 0) {
		GPS_POPUP_LOG("ERROR: Fail to init edbus event !!!");
	}
	return 0;
}

int __gps_popup_pause(void *data)
{
	GPS_POPUP_LOG("ENTER");
	return 0;
}

int __gps_popup_resume(void *data)
{
	GPS_POPUP_LOG("ENTER");
	return 0;
}

int __gps_popup_reset(bundle *b, void *data)
{
	GPS_POPUP_LOG("ENTER");
	GPS_POPUP_RETURN_VAL_IF_FAILED(data, -1);
	struct gps_popup_appdata *ad = data;
	int enabled = 0;

	if (syspopup_has_popup(b)) {
		GPS_POPUP_LOG("syspopup_has_popup failed.");
		syspopup_reset(b);
	} else {
		syspopup_create(b, &handler, ad->win_main, ad);
		evas_object_show(ad->win_main);

		const char *val = bundle_get_val(b, "event-type");
		GPS_POPUP_LOG("event-type = %s", val);
		if (!g_strcmp0("gps", val)) {
			vconf_get_int(VCONFKEY_LOCATION_ENABLED, &enabled);
			if (enabled == 0) {
				ad->gps_popup_setting = popup_type_create(data, POPUP_GPS);
			}
		} else if (!g_strcmp0("both", val)) {
			vconf_get_int(VCONFKEY_LOCATION_USE_MY_LOCATION, &enabled);
			if (enabled == 0) {
				ad->gps_popup_setting = popup_type_create(data, POPUP_GPS);
			} else {
				__gps_popupsend_signal_to_quickpanel(ad, "OFF");
				elm_exit();
			}
		} else {
			vconf_get_int(VCONFKEY_LOCATION_USE_MY_LOCATION, &enabled);
			if (enabled == 0) {
				int is_shown = 0, gps_enable = 0, loc_enable = 0;

				vconf_get_int(VCONFKEY_LBS_SETTING_IS_SHOW_GPS_POPUP, &is_shown);
				vconf_get_int(VCONFKEY_LOCATION_ENABLED, &gps_enable);

				if (is_shown == 1 && gps_enable == 0) {
					GPS_POPUP_LOG("## popup shown ##");
					ad->gps_popup = popup_type_create(data, POPUP_GPS_QP);
				} else {
					GPS_POPUP_LOG("## popup already shown ##");
					if (gps_enable == 0) {
						vconf_set_int(VCONFKEY_LOCATION_ENABLED, 1);
					}
					vconf_get_int(VCONFKEY_LOCATION_USE_MY_LOCATION, &loc_enable);
					if (loc_enable == 0) {
						vconf_set_int(VCONFKEY_LOCATION_USE_MY_LOCATION, 1);
					}
					__gps_popupsend_signal_to_quickpanel(ad, "ON");

#ifdef TIZEN_FEATURE_WPS
					ad->wireless_popup_setting = popup_type_create(data,POPUP_WIRELESS_QP);
#else
					elm_exit();
#endif
				}
			} else {
				GPS_POPUP_LOG("Nothing to do");
				__gps_popupsend_signal_to_quickpanel(ad, "OFF");
				elm_exit();
			}
		}

	}

	return 0;
}

int main(int argc, char *argv[])
{
	GPS_POPUP_LOG("ENTER");
	struct gps_popup_appdata ad = {0};

	/* App life cycle management */
	struct appcore_ops ops = {
		.create = __gps_popup_create,
		.terminate = __gps_popup_terminate,
		.pause = __gps_popup_pause,
		.resume = __gps_popup_resume,
		.reset = __gps_popup_reset,
	};

	ops.data = &ad;

	return appcore_efl_main(PACKAGE, &argc, &argv, &ops);
}


