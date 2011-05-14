# Copyright (c) 2006-2009 Erdem U. Altinyurt
# Thanks for oc2pus
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# Please submit bugfixes or comments to spamjunkeater gmail.com

# norootforbuild

%define _prefix	/usr

Name:			meteorite
Summary:		A program to repair broken MKV file streams
Version:		0.11
Release:		1
License:		GPL
Group:			Productivity/Multimedia/Video/Editors and Convertors
URL:			http://meteorite.sourceforge.net/
Source0:		Meteorite-v%{version}-src.tar.bz2
BuildRoot:		%{_tmppath}/%{name}-%{version}-build
BuildRequires:	dos2unix
BuildRequires:	gcc-c++
BuildRequires:	pkgconfig
%if %{defined suse_version}
BuildRequires:	update-desktop-files
%endif
BuildRequires:	wxGTK-devel >= 2.8

%description
This program designed to repair broken MKV file streams. by
This is very useful when trying to preview movies on
currently downloading from ed2k or bittorent networks...

%prep
%setup -q -n Meteorite

dos2unix     docs/*
%__chmod 644 docs/*

%build
%if %{defined suse_version} && 0%{?suse_version} < 1030
		%__make %{?jobs:-j%{jobs}} \
			WXCONFIG=wx-config-2.8
%else
	%__make %{?jobs:-j%{jobs}} \
		WXCONFIG=wx-config
%endif

%install
%__install -dm 755 %{buildroot}%{_datadir}/pixmaps
%__install -dm 755 %{buildroot}%{_datadir}/applications
%__rm -f %{buildroot}%{_datadir}/applications/%{name}.desktop
%__install -D -s -m 755 %{name} %{buildroot}%{_bindir}/%{name}
%__install -D -m 644 resources/%{name}.ico %{buildroot}%{_datadir}/pixmaps/%{name}.ico
%__cat > %{name}.desktop << EOF
[Desktop Entry]
Comment=A program to repair broken MKV file streams.
Name=%{name}
GenericName=
Type=Application
Exec=%{name}
Icon=%{name}.png
Encoding=UTF-8
EOF
%if %{defined suse_version}
%suse_update_desktop_file -i %{name} AudioVideo AudioVideoEditing
%else
%__install -D -m 644 resources/%{name}.desktop %{buildroot}%{_datadir}/applications/%{name}.desktop
%endif

%clean
[ -d "%{buildroot}" -a "%{buildroot}" != "" ] && %__rm -rf "%{buildroot}"

%files
%defattr(-,root,root)
%doc docs/*
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/pixmaps/%{name}.ico

%changelog
* Sat May 14 2011 Erdem U. Altinyurt <spamjunkeater@gmail.com> - 0.1-0
- Initial Release
