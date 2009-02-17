# $Id$

# Tcl based Apache test suite, by David N. Welton <davidw@dedasys.com>

# This test suite provides a means to create configuration files, and
# start apache with user-specified options.  All it needs to run is
# the name of the Apache executable, which must, however, be compiled
# with the right options.

package provide apachetest 0.1

namespace eval apachetest {

    # Associate module names with their internal names.
    array set module_assoc {
	mod_log_config	  config_log_module
	mod_mime		mime_module
	mod_negotiation	 negotiation_module
	mod_dir			 dir_module
	mod_access	      access_module
	mod_auth		auth_module
    }

    ## Apache 2.2 has different names for some modules
    array set module_assoc_22 {
	mod_log_config           config_log_module
	mod_mime                       mime_module
	mod_negotiation         negotiation_module
	mod_dir                         dir_module
	mod_authz_host           authz_host_module
	mod_auth_basic           auth_basic_module
	mod_authn_file           authn_file_module
	mod_authz_user           authz_user_module
	mod_authz_groupfile authz_groupfile_module
    }
    # name of the apache binary, such as /usr/sbin/httpd
    variable binname ""
    # this file should be in the same directory this script is.
    variable templatefile [file join conf server.conf.tcl]
    variable templatewebshconf [file join conf websh.conf.tcl]
}

# make sure we can connect to the server
proc apachetest::connect { } {

    global waiting
    set waiting 10

    set connect [after 5000 {set waiting 0}]

    while {$waiting > 9} {
	if { ! [catch {
	    set sk [socket localhost 8081]
	} err]} {
	    close $sk
	    after cancel $connect
	    return 1
	}
	after 10 {incr waiting}
	vwait waiting
    }
    return 0
}

# start - start the server in the background with 'options' and then
# run 'code'.

proc apachetest::start { options code } {
    variable serverpid 0
    variable binname

    # There has got to be a better way to do this, aside from waiting.
#    set serverpid [eval exec  $binname -X -f \
#		       [file join [pwd] server.conf] $options &]

    set serverhandle [open "|$binname -X -f [file join [pwd] conf server.conf]" r]
    set serverpid [pid $serverhandle]
    fconfigure $serverhandle -blocking 0

    puts "Apache started as PID $serverpid"

    if {[apachetest::connect]} {
	if { [catch {
	    uplevel $code
	} err] } {
	    puts $err
	}
    } else {
	error "Could not connect to Apache"
    }

    exec kill $serverpid
    set kill9 [after 2500 "
	puts stderr \"Can't kill process, trying with kill -9\";
	exec kill -9 $serverpid
    "]
    global waiting
    set waiting 1
    while {![eof $serverhandle]} {
	gets $serverhandle
	after 500 {incr waiting}
	vwait waiting
    }
    after cancel $kill9
    puts "Apache stopped"
    catch {file delete httpd.pid}
}

# startserver - start the server with 'options'.

proc apachetest::startserver { options } {
    variable binname
    if { [catch {
	eval exec $binname -X -f \
	    "[file join [pwd] server.conf]" $options
    } err] } {
	puts $err
    }
}

# setbinname - set the name of the apache binary

proc apachetest::setbinname { name } {
    variable binname
    global argv0
    if {![file exists $name]} {
	puts stderr "Please supply the full name and path of the Apache executable"
	puts stderr "on the command line (or in the HTTPD_BIN environment variable):"
	puts stderr "$argv0 /path/to/httpd"
	exit 1
    }

    set binname $name
    return $binname
}

# get the modules that are compiled into Apache directly, and return
# the XXX_module name.  Check also for the existence of mod_so, which
# we need to load the shared object in the directory above...

proc apachetest::getcompiledin { binname } {
    variable modules
    set bin [open [list | "$binname" -l] r]
    set compiledin [read $bin]
    close $bin
    set modlist [split $compiledin]
    set compiledin [list]
    set mod_so_present 0
    foreach entry $modlist {
	if { [regexp {(.*)\.c$} $entry match modname] } {
	    if { $modname == "mod_so" } {
		set mod_so_present 1
	    }
	    if { [info exists modules($modname)] } {
		lappend compiledin $modules($modname)
	    }
	}
    }
    if { $mod_so_present == 0 } {
	error "We need mod_so in Apache to run these tests"
    }
    return $compiledin
}

# find the httpd.conf file

proc apachetest::gethttpdconf { binname } {
    set bin [ open [list | "$binname" -V ] r ]
    set options [ read $bin ]
    close $bin
    regexp {SERVER_CONFIG_FILE="(.*?)"} "$options" match filename
    if { ! [file exists $filename] } {
	# see if we can find something by combining HTTP_ROOT + SERVER_CONFIG_FILE
	regexp {HTTPD_ROOT="(.*?)"} "$options" match httpdroot
	set completename [file join $httpdroot $filename]
	if { ! [file exists $completename] } {
	    error "neither '$filename' or '$completename' exists"
	}
	return $completename
    }
    return $filename
}

# if we need to load some modules, find out how to do it from the
# 'real' (the one installed on the system) conf file, with this proc

proc apachetest::getloadmodules { conffile needtoget } {
    set fl [open $conffile r]
    set confdata [read $fl]
    close $fl
    set loadline [list]
    foreach mod $needtoget {
	if { ! [regexp -line "^.*?(LoadModule\\s+$mod\\s+.+)\$"\
		    $confdata match line] } {
	    error "No LoadModule line for $mod!"
	} else {
	    lappend loadline $line
	}
    }
    return [join $loadline "\n"]
}

# compare what's compiled in with what we need

proc apachetest::determinemodules { binname } {
    variable module_assoc
    variable module_assoc_22
    variable modules

    set ver [exec $binname -v]
    if {[regexp {Apache/1\.3\.} $ver] || [regexp {Apache/2\.0\.} $ver]} {
	array set modules [array get module_assoc]
    } else {
	array set modules [array get module_assoc_22]
    }
    set compiledin [lsort [getcompiledin $binname]]
    set conffile [gethttpdconf $binname]

    foreach {n k} [array get modules] {
	lappend needed $k
    }
    set needed [lsort $needed]

    set needtoget [list]
    foreach mod $needed {
	if { [lsearch $compiledin $mod] == -1 } {
	    lappend needtoget $mod
	}
    }
    if { $needtoget == "" } {
	return ""
    } else {
	return [getloadmodules $conffile $needtoget]
    }
}

# dump out a config
# outfile is the file to write to.
# extra is for extra config things we want to tack on.

proc apachetest::makeconf { outfile {extra ""} } {
    global env
    variable binname
    variable templatefile
    set CWD [pwd]

    # replace with determinemodules
    set LOADMODULES [determinemodules $binname]

    set fl [open [file join . $templatefile] r]
    set template [read $fl]
    close $fl

    append template $extra

    set out [subst $template]

    set of [open $outfile w]
    puts $of "$out"
    close $of
}

proc apachetest::makewebshconf {outfile} {
    global env
    variable templatewebshconf
    set CWD [pwd]

    # replace with determinemodules
    set fl [open [file join . $templatewebshconf] r]
    set template [read $fl]
    close $fl

    set out [subst $template]

    set of [open $outfile w]
    puts $of "$out"
    close $of
}
