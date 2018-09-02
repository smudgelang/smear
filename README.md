[![Build Status](https://travis-ci.com/smudgelang/smear.svg?branch=master)](https://travis-ci.com/smudgelang/smear)

Smear is a runtime environment for the Smudge state machine
programming language.

To use this package in your own Smudge projects, build "libsmear.a"
and put it in your library include path. Then link your code with
"-lsmear -pthread". Also put "smear.h" somewhere in your include path.

Smear implements all the functions whose names start with "SMUDGE_",
and has handy macros for generating your event handlers. Put this in
one of your .c files:

    #include <smear.h>

    // Call this macro once per Smudge state machine. This is how you
    // would call it for a machine called "machine".
    SRT_HANDLERS(machine)

    // And this is how you'd call it for another machine called
    // "traffic_light".
    SRT_HANDLERS(traffic_light)

Then, before you send any events in your program, call
"SRT_init()". Once your program is initialized, you can call
"SRT_run()" to start processing events. If you ever want to stop the
state machines, you can call "SRT_stop()" and if you want to run
forever in a happy little Smudge world, you can call "SRT_join()".
