%define deftclver 8.3.1
%define tclver %(rpm -q tcl --queryformat '%%{version}' 2> /dev/null || echo %{deftclver})

Summary: Tcl scripting for the web, both a CGI and Apache module.
Name: websh
Version: 4.0.0
Release: 1
Copyright: Freely distributable and usable
Group: System Environment/Daemons
Source:	http://tcl.apache.org/websh/download/%{name}-%{version}.tar.gz
URL: http://websh.com/
Packager: David N. Welton <davidw@dedasys.com>
BuildRoot: %{_tmppath}/%{name}-root
Requires: webserver, tcl = %{tclver}
BuildPrereq: apache-devel, tcl
Prereq: tcl

%description

Webshell is a rapid development environment for building powerful,
fast, and reliable web applications. webshell is versatile and handles
everything from HTML generation to data-base driven one-to-one page
customization.

%prep
%setup -n %{name}

%build
cd src/unix
./configure --with-tclinclude=/usr/include/tcl8.3 --with-tcl=/usr/lib/tcl8.3/ --prefix=/usr --with-httpdinclude=/usr/include/apache-1.3/
make mod_websh.so

%install
cd src/unix
make install DESTDIR=$RPM_BUILD_ROOT
mkdir $RPM_BUILD_ROOT/usr/lib/apache/
cp mod_websh4.00.so $RPM_BUILD_ROOT/usr/lib/apache/

%clean
cd src/unix
make clean

%files
%defattr(-,root,root)
%doc doc
%{_libdir}/apache/mod_websh4.00.so

%changelog
* Fri Dec 14 2001 David N. Welton <davidw@dedasys.com>
- Initial packages.