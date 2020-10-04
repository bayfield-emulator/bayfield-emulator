#include <time.h>
#include <stdint.h>
#include <stdio.h>

#if !defined(CLOCK_MONOTONIC) && __APPLE__
#warning Using Mach clock instead of POSIX - this code is untested

#include <mach/clock.h>
#include <mach/mach.h>

static clock_serv_t clock_port = MACH_PORT_NULL;

uint32_t usec_since(uint32_t usec) {
    if (!clock_port) {
        host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &clock_port);
    }

    mach_timespec_t ts;
    if (clock_get_time(clock_port, &ts) != KERN_SUCCESS) {
        mach_port_deallocate(mach_task_self(), clock_port);
        host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &clock_port);
    }

    uint32_t us = (ts.tv_sec / 1000000) + (ts.tv_nsec * 1000);

    if (us < usec) {
        // it overflowed
        return (UINT32_MAX - usec) + us;
    } else {
        return us - usec;
    }
}

#else

// Then we can do it the normal way.
uint32_t usec_since(uint32_t usec) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    uint32_t us = (ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);

    if (us < usec) {
        // it overflowed
        return (UINT32_MAX - usec) + us;
    } else {
        return us - usec;
    }
}

#endif
