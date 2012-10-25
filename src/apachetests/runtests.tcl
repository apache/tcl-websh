#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# set host and port apache should bind to for the tests
set host localhost
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
	# fix commandline for startserver
	set argv [lreplace $argv 0 0]
	incr argc -1
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

# do we only need to start the server?
if {[lindex $argv 0] == "startserver"} {
    apachetest::startserver
    exit
}

# we run the full suite

# we do this to keep tcltest happy - it reads argv...
set argv {}

# needed by tests
set urlbase "http://$host:$port/"

package require tcltest
package require http

# Test stanzas are created by giving the test a name and a
# description.  The code is then executed, and the results
# compared with the desired result, which is placed after the
# block of code.  See man tcltest for more information.

# start server only once for all tests (put in foreach to start a 
# server per test file)

apachetest::start {} {
    foreach file [glob -nocomplain test/*.test] {
	puts "Running test file $file"
	if {[catch {source $file} msg]} {
	    puts "Running test file $file failed: $msg"
	}
    }
}

::tcltest::cleanupTests
