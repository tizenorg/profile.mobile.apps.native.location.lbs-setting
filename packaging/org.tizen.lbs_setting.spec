%bcond_with wayland

%define appdir /usr/apps/org.tizen.setting-location

Name:       org.tizen.setting-location
Summary:    location setting
Version:    1.0.31
Release:    1
Group:      Applications/Location
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(efl-extension)
BuildRequires: pkgconfig(ui-gadget-1)
BuildRequires: pkgconfig(elementary)
BuildRequires: pkgconfig(edje)
BuildRequires: pkgconfig(capi-location-manager)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(appcore-efl)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(edbus)
BuildRequires: pkgconfig(syspopup)
BuildRequires: pkgconfig(syspopup-caller)
BuildRequires: pkgconfig(feedback)
BuildRequires: pkgconfig(bundle)
BuildRequires: pkgconfig(eventsystem)
BuildRequires: pkgconfig(capi-system-info)
BuildRequires: pkgconfig(capi-appfw-package-manager)
BuildRequires: edje-bin
BuildRequires: cmake
BuildRequires: gettext-tools
Requires(post):   /sbin/ldconfig
Requires(post):   /usr/bin/vconftool
requires(postun): /sbin/ldconfig

%package -n org.tizen.gps-syspopup
Summary:    GPS popup
Requires:   %{name} = %{version}-%{release}

%description -n org.tizen.gps-syspopup
GPS popup.

%description
location setting.


%prep
%setup -q

%build
%define PREFIX		"/usr/apps/org.tizen.setting-location"
%define RW_PREFIX	"/opt/usr/apps/org.tizen.setting-location"

export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"

%if %{with wayland}
export WAYLAND_SUPPORT=On
%else
export WAYLAND_SUPPORT=Off
%endif

%if 0%{?model_build_feature_location_position_wps}
	-DENABLE_WPS=YES
%endif

cmake . -DCMAKE_INSTALL_PREFIX=%{PREFIX} -DCMAKE_INSTALL_RW_PREFIX=%{RW_PREFIX} -DWAYLAND_SUPPORT=${WAYLAND_SUPPORT} \

make %{?jobs:-j%jobs}


%install
rm -rf %{buildroot}

%make_install


%post
/sbin/ldconfig
mkdir -p /usr/ug/bin/
mkdir -p /usr/ug/res/locale/
ln -sf /usr/bin/ug-client /usr/ug/bin/setting-location-efl

%if 0%{?model_build_feature_location_position_wps}
	buxton2ctl -i -d set-int32 system db/location/setting/NetworkEnabled 1
%else
	buxton2ctl -i -d set-int32 system db/location/setting/NetworkEnabled 0
%endif

%if "%{profile}" == "wearable"
	buxton2ctl -i -d set-int32 system db/location/setting/GpsPopup 0
%else
	buxton2ctl -i -d set-int32 system db/location/setting/GpsPopup 1
%endif

%postun -p /sbin/ldconfig


%files
%manifest org.tizen.setting-location.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.setting-location/*
%defattr(-,root,root,757)
%{appdir}/data/
%{appdir}/res/locale/*/*/*
/usr/share/packages/org.tizen.setting-location.xml

%defattr(-,app,app,-)
#/opt/usr/apps/org.tizen.setting-location/data


%files -n org.tizen.gps-syspopup
%manifest org.tizen.gps-syspopup.manifest
%defattr(-,root,root,-)
/usr/apps/org.tizen.gps-syspopup/bin/*
/usr/apps/org.tizen.gps-syspopup/res/locale
/usr/apps/org.tizen.gps-syspopup/res/edje
/usr/share/packages/org.tizen.gps-syspopup.xml
