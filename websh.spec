%define deftclver 8.3.1
%define tclver %(rpm -q tcl --queryformat '%%{version}' 2> /dev/null || echo %{deftclver})

Summary: Tcl scripting for the web, both a CGI and Apache module.
Name: websh
Version: 3.6.0b5
Release: 1
Copyright: Freely distributable and usable
Group: System Environment/Daemons
Source: http://www.apache.org/dyn/closer.cgi/tcl/websh/source/%{name}-%{version}-src.tar.gz
URL: http://tcl.apache.org/websh/
Packager: David N. Welton <davidw@dedasys.com>
BuildRoot: %{_tmppath}/%{name}-root
Requires: webserver, tcl = %{tclver}
BuildPrereq: tcl, tcl-devel, apr, apr-devel
Prereq: tcl

%description

Websh is a rapid development environment for building powerful,
fast, and reliable web applications. webshell is versatile and handles
everything from HTML generation to data-base driven one-to-one page
customization.

%prep
%setup -n %{name}-%{version}

%build
cd src/unix
/usr/bin/autoconf
./configure --with-tclinclude=/usr/include --with-tcl=/usr/lib --prefix=$RPM_BUILD_ROOT/usr --with-httpdinclude=/usr/include/httpd
make CFLAGS="-I/usr/include/apr-0"

%install
cd src/unix
make install DESTDIR=$RPM_BUILD_ROOT

%clean
cd src/unix
make clean

%files
%defattr(-,root,root)
%doc doc
/usr/bin/websh3.6.0b5
/usr/lib/httpd/modules/mod_websh3.6.0b5.so
/usr/lib/libwebsh3.6.0b5.so
/usr/share/websh3.6/conf/htmlhandler.ws3
/usr/share/websh3.6/conf/httpd.conf
/usr/share/websh3.6/conf/otherhandler.ws3
/usr/share/websh3.6/conf/websh.conf
/usr/share/websh3.6/htdocs/index.html
/usr/share/websh3.6/htdocs/myApp.ws3
/usr/share/websh3.6/htdocs/other.html
/usr/share/websh3.6/README

%changelog
* Fri Dec 15 2006 Peter Kohler <peter.kohler@netcetera.ch>
- fixed paths, dependencies, and installation
* Fri Oct 28 2005 Ronnie Brunner <ronnie.brunner@netcetera.ch>
- moved to version 3.6.0
- removed make mod_websh.so (now in default)
* Fri Aug 01 2002 Ronnie Brunner <ronnie.brunner@netcetera.ch>
- dynamic version number of mod_websh
- also make and install websh itself
* Fri Dec 14 2001 David N. Welton <davidw@dedasys.com>
- Initial packages.
