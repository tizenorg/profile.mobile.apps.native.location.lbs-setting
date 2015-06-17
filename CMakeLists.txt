CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(setting-location C)
SET(PROJECT_NAME org.tizen.setting-location)

# Version control
SET(VERSION_MAJOR 0)
SET(VERSION "${VERSION_MAJOR}.0.1")

# Architecture
FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" STREQUAL "arm")
	ADD_DEFINITIONS("-D_SOUND_EQ_ARCH_ARM_")
ENDIF()

SET(COMMON_C_FLAGS "${CMAKE_C_FLAGS} -fPIE")
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(BINDIR "${PREFIX}/bin")
SET(LIBDIR "${PREFIX}/lib")
SET(RESDIR "${PREFIX}/res")
SET(DATADIR "${PREFIX}/data")
SET(LBS_SETTING_LOCALE_DIR "${RESDIR}/locale")
SET(LBS_SETTING_EDJ_DIR "${RESDIR}/edje")

SET(RW_PREFIX ${CMAKE_INSTALL_RW_PREFIX})

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE(FindPkgConfig)

# Check external libraries
INCLUDE(FindPkgConfig)
MACRO(PKG_CONFIG E_NAME A_PKG)
	SET(${E_NAME} ${A_PKG})
	pkg_check_modules(${E_NAME} "REQUIRED" ${A_PKG} efl-assist ui-gadget-1 sensor bundle eventsystem)
ENDMACRO(PKG_CONFIG)

PKG_CONFIG(E_APPFW				"capi-appfw-application")
PKG_CONFIG(E_EVAS				"evas")
PKG_CONFIG(E_EA					"efl-assist")
PKG_CONFIG(E_EA_EXTENSION		"efl-extension")

PKG_CONFIG(E_ELEMENTARY			"elementary")
PKG_CONFIG(E_EDJE				"edje")
PKG_CONFIG(E_MANAGER			"capi-location-manager")
PKG_CONFIG(E_DLOG				"dlog")
PKG_CONFIG(E_VCONF				"vconf")
PKG_CONFIG(E_APPCORE_EFL		"appcore-efl")
PKG_CONFIG(E_ECORE				"ecore")
PKG_CONFIG(E_ECORE_X			"ecore-x")
PKG_CONFIG(E_SYSPOPUP			"syspopup")
PKG_CONFIG(E_FEEDBACK			"feedback")
PKG_CONFIG(E_PKGMGR				"capi-appfw-package-manager")


SET(EXTRA_CFLAGS "")
FOREACH(flag ${ui_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${COMMON_C_FLAGS} ${EXTRA_CFLAGS}")

ADD_DEFINITIONS("-DEDJ_DIR=\"${LBS_SETTING_EDJ_DIR}\"")
ADD_DEFINITIONS("-DPROJECT_NAME=\"${PROJECT_NAME}\"")

# install application HOME directory
INSTALL(DIRECTORY DESTINATION ${DATADIR})

# install smack rule file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/org.tizen.setting-location.efl DESTINATION /etc/smack/accesses.d)

INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/data/xml/ DESTINATION /usr/apps/org.tizen.setting-location/res FILES_MATCHING PATTERN "*.xml")

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(resources/edc)
ADD_SUBDIRECTORY(resources/po)
ADD_SUBDIRECTORY(gps-syspopup)

