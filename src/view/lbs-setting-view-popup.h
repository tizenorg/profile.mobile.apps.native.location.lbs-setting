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

#ifndef LBS_SETTING_VIEW_POPUP_H_
#define LBS_SETTING_VIEW_POPUP_H_

#include "lbs-setting-string.h"
#include "lbs-setting-view-location.h"

#ifdef __cplusplus
extern "C" {
#endif

gboolean lbs_setting_popup_title_2button(char *title, char *body, char *btn1, Evas_Smart_Cb btn1_func, char *btn2, Evas_Smart_Cb btn2_func, void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* LBS_SETTING_VIEW_POPUP_H_ */
