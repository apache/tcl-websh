%define deftclver 8.3.1
%define tclver %(rpm -q tcl --queryformat '%%{version}' 2> /dev/null || echo %{deftclver})

Summary: Tcl scripting for the web, both a CGI and Apache module.
Name: websh
Version: 0.4.0
Release: 1
Copyright: Freely distributable and usable
Group: System Environment/Daemons
Source:	http://websh.com/
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
./src/configure
cd src
make

%install
cd src
make install

%clean
cd src
make clean

%files
%defattr(-,root,root)
%doc doc
%{_libdir}/apache/mod_websh.so

%changelog
* Fri Dec 14 2001 David N. Welton <davidw@dedasys.com>
- Initial packages.