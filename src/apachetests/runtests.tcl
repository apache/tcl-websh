#!/bin/sh
# the next line restarts using tclsh \
	exec tclsh "$0" "$@"

proc getbinname { } {
    global argv
    set binname [lindex $argv 0]
    if { $binname == "" || ! [file exists $binname] } {
	puts stderr "Please supply the full name and path of the Apache executable on the command line!"
	exit 1
    }
    return $binname
}

set binname [ getbinname ]

source makeconf.tcl
makeconf $binname server.conf

puts "$binname -X -f [file join [pwd] server.conf]"
switch -exact [lindex $argv 1] {
    withconfigs {
	foreach {option val} {
	    {} {}
	    WebshConfig config.tcl
	} {
	    set apachepid [exec $binname -X -f "[file join [pwd] server.conf]" -c "$option $val" &]
	    set oput [exec [file join . mod_websh.test]]
	    puts $oput
	    exec kill $apachepid
	}
    } 
    startserver {
	set apachepid [exec $binname -X -f "[file join [pwd] server.conf]" &]
    }
    default {
	set apachepid [exec $binname -X -f "[file join [pwd] server.conf]" &]
	set oput [exec [file join . mod_websh.test]]
	puts $oput
	exec kill $apachepid
    }
}
