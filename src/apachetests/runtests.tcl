#!/bin/sh
# the next line restarts using tclsh \
	exec tclsh "$0" "$@"

source [file join apachetest apachetest.tcl]

apachetest::getbinname $argv

apachetest::makeconf server.conf {
    LoadModule websh_module [file join $CWD .. unix "mod_websh3.10[info sharedlibextension]"]
    <IfModule mod_mime.c>
    AddLanguage en .en
    AddLanguage it .it
    AddLanguage es .es
    AddLanguage de .de
    AddHandler websh .ws3
    </IfModule>
}

# we do this to keep tcltest happy - it reads argv...
set commandline [lindex $argv 1]
set argv {}

switch -exact $commandline {
    startserver {
	apachetest::startserver
    }
    default {
	source [file join . mod_websh.test]
    }
}
