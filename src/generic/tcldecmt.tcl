if {$argc > 0 } {
    set fileName [lindex $argv 0]
} else {
    puts "hdb2pab.tcl --- usage: hdb2pab.tcl input.hdb."
    exit 1
}

# ----------------------------------------------------------------------------
# try to open file
# ----------------------------------------------------------------------------
if {[file exists $fileName] } {
    set fileId [open $fileName "r"]
} else {
    puts "hdb2pab.tcl --- cannot open $fileName."
    exit 1
}

# ----------------------------------------------------------------------------
# read line by line to the end of the file
# ----------------------------------------------------------------------------
while {[eof $fileId] == 0} {

    gets $fileId tLine
    if { ![regexp "^$" $tLine] } {
	if { ![regexp {^[^#]*#} $tLine] } {
          regsub -all {\s+} $tLine { } res
          set tLine $res
	  if { [regexp {^\s*(.*)} $tLine dum res] } {
	        puts $res
	  } else {
	      puts $tLine
	  }
	}
    }
}
