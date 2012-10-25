# \$Id\$
# Minimal config file for testing

# Parsed by makeconf.tcl

ServerRoot "$CWD"

PidFile "$CWD/logs/httpd.pid"
$LockFile
Timeout 300

MaxRequestsPerChild 0

$LOADMODULES

Listen $port

ServerName localhost

DocumentRoot "$CWD/docs"

<Directory "$CWD/docs">
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

ErrorLog $CWD/logs/error.log
TypesConfig $CWD/conf/mime.types

LogLevel debug

LogFormat "%h %l %u %t \\"%r\\" %>s %b \\"%{Referer}i\\" \\"%{User-Agent}i\\"" combined
CustomLog "$CWD/logs/access.log" combined
