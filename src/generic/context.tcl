#
# context.tcl -- context manager
#
# Copyright (C) 2000 by Netcetera AG.
# Copyright (C) 2001 by Apache Software Foundation.
# All rights reserved.
#
# See the file "license.terms" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# @(#) $Id$
#

# setup config namespace
proc web::context {name} {

    # correct namespace (relative to caller)
    if {![string match ::* $name]} {
	set name [uplevel namespace current]::$name
    }

    namespace eval $name {

	namespace eval vars {
	    # initialize namespace for context
	    # we use a separate namespace to avoid name clashes
	}

	# set a config variable or array
	proc cset {key value} {
	    return [set vars::$key $value]
	}

	# append to a config variable or array
	proc cappend {key args} {
	    return [eval append [list vars::$key] $args]
	}

	# lappend to a config variable or array
	proc clappend {key args} {
	    return [eval lappend [list vars::$key] $args]
	}

	# unset config var(s) and/or array(s)
	# resets complete context if no args are specified
	proc cunset {args} {
	    if {![llength $args]} {
		namespace delete vars
		namespace eval vars {}
	    } else {
		foreach key $args {
		    if {[info exists vars::$key]} {
			unset vars::$key
		    }
		}
	    }
	}

	# get the content of a config var or array item
	# (return default value if item does not exist)
	proc cget {key {default ""}} {
	    if {[info exists vars::$key]} {
		return [set vars::$key]
	    } else {
		return $default
	    }
	}

	# does this config item exist
	proc cexists {key} {
	    return [info exists vars::$key]
	}
    
	# array procs on config array
	proc carray {option arrayName args} {
	    if {[llength $args]} {
		return [array $option vars::$arrayName [lindex $args 0]]
	    } else {
		return [array $option vars::$arrayName]
	    }
	}

	# get the names of all (matching) vars
	proc cnames {{pattern *}} {
	    set result ""
	    foreach var [info vars [namespace current]::vars::$pattern] {
		lappend result [namespace tail $var]
	    }
	    return $result
	}

	# dump content
	proc dump {} {
	    set result ""
	    # dump sorted variables (easier to compare)
	    foreach var [lsort [cnames]] {
		if {[array exists vars::$var]} {
		    # we have an array (and we also dump its sorted content)
		    set array {}
		    foreach arrayvar [lsort [array names vars::$var]] {
			lappend array $arrayvar [set vars::${var}($arrayvar)]
		    }
		    lappend result [list carray set $var $array]
		} else {
		    # we have a scalar
		    lappend result [list cset $var [set vars::$var]]
		}
	    }
	    return [join $result \n]
	}

	# destroy context
	proc delete {} {
	    namespace delete [namespace current]
	}
    }
}
