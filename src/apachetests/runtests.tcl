#!/bin/sh
# the next line restarts using tclsh \
	exec tclsh "$0" "$@"

package require tcltest
package require http 2.1
package require Tclx

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

# we do this to keep tcltest happy
set commandline [lindex $argv 1]
set argv {}

switch -exact -- $commandline {
    startserver {
	set apachepid [exec $binname -X -f "[file join [pwd] server.conf]" &]
    }
    default {
	source [file join . mod_websh.test]
    }
}
