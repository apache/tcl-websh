# \$Id\$
# Minimal config file for testing

# Parsed by makeconf.tcl

ServerType standalone

ServerRoot "$CWD"

PidFile "[file join $CWD httpd.pid]"

# ScoreBoardFile "$CWD/apache_runtime_status"

ResourceConfig [file join $CWD srm.conf]
AccessConfig [file join $CWD access.conf]

Timeout 300

MaxRequestsPerChild 100

$LOADMODULES

LoadModule websh_module [file join $CWD .. unix "mod_websh3.10[info sharedlibextension]"]

Port 8080

ServerName localhost

DocumentRoot "$CWD"

<Directory "$CWD">
Options All MultiViews 
AllowOverride All
Order allow,deny
Allow from all
</Directory>

<IfModule mod_dir.c>
DirectoryIndex index.html
</IfModule>

AccessFileName .htaccess

HostnameLookups Off

ErrorLog [file join $CWD error_log]

LogLevel debug

LogFormat "%h %l %u %t \\"%r\\" %>s %b \\"%{Referer}i\\" \\"%{User-Agent}i\\"" combined
CustomLog "$CWD/access_log" combined

WebshConfig [file join $CWD config.tcl]

<IfModule mod_mime.c>
AddLanguage en .en
AddLanguage it .it
AddLanguage es .es
AddHandler websh .ws3
</IfModule>
