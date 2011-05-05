#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# set the port apache should bind to for the tests
set port 8081

source apachetest.tcl

global env
if {![info exists env(HTTPD_BIN)] || ![string length $env(HTTPD_BIN)]} {
    if {[string length [lindex $argv 0]]} {
	# get from commandline
	set httpdbin [lindex $argv 0]
	if {[regexp -- {^-I"?(.*?)"?$} $httpdbin all include]} {
	    # special case for call by Makefile
	    set env(HTTPD_BIN) [file join [file dirname $include] bin httpd]
	} else {
	    # direct call (just binary on command line)
	    set env(HTTPD_BIN) [lindex $argv 0]
	}
    } elseif {[file exists httpd]} {
	set env(HTTPD_BIN) [file join [pwd] httpd]
    } else {
	# don't know what to do
	set env(HTTPD_BIN) "unknown"
    }
}

if {![info exists env(MOD_WEBSH)]} {
    global tcl_platform
    if {[string equal $tcl_platform(platform) "windows"]} {
	set dir win
    } else {
	set dir unix
    }
    set env(MOD_WEBSH) [lindex [glob [file join [pwd] .. $dir "mod_websh*.so"]] 0]
}

apachetest::setbinname $env(HTTPD_BIN)

apachetest::makeconf conf/server.conf {

LoadModule websh_module \"$env(MOD_WEBSH)\"
AddHandler websh .ws3

WebshConfig \"[file join [pwd] conf websh.conf]\"

}

apachetest::makewebshconf conf/websh.conf

# we do this to keep tcltest happy - it reads argv...
set commandline [lindex $argv 1]
set argv {}

switch -exact $commandline {
    startserver {
	apachetest::startserver
    }
    default {
	source [file join test mod_websh.test]
    }
}
