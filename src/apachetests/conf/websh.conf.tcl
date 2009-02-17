# WebshConfig file

# \$Id: websh.conf 383380 2006-03-05 19:41:49Z ronnie \$

proc web::interpmap {file} {

    # hello test
    if {\[string match "*/my_script_hello.ws3" \$file\]} {
	return \[file join \[file dirname \$file\] hello.ws3\]
    }

    # standard tests
    return \$file
}

web::interpclasscfg $CWD/docs/pool2.ws3 maxrequests 10
