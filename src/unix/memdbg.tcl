set time [clock format [clock seconds] -format "%Y-%m-%d-%H-%M-%S"]
puts $time

set env(WEB_LIBRARY) ./libwebsh3.0b3.so

memory active ws3-$time-1.mem

interp create myinterp

# myinterp eval {
#     foreach item [array names env] {
#         puts $item
#     }
# }

myinterp eval source ../tests/webtest.ws3

interp delete myinterp

memory active ws3-$time-2.mem

# exec diff 

set cmd {diff ws3-$time-2.mem ws3-$time-1.mem | grep "@ ../"}

puts "done. gonna diff. cmd: $cmd"

exec $cmd

