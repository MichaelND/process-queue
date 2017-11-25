#!/usr/bin/env python2.7

# worksim.py: Work Simulator

import os
import random
import signal
import sys
import time

# Logging ----------------------------------------------------------------------

PID = os.getpid()

def log(msg):
    if not os.environ.get('NDEBUG', None):
        print '[{:5d}] {}'.format(PID, msg)

# Do work ----------------------------------------------------------------------

class StopWork(Exception):
    pass

def timer_handler(signum, frame):
    raise StopWork

def do_compute():
    log('do_compute')
    try:
        x = 1
        while True:
            x = (x + 1) % 10000000
        return x
    except StopWork:
        pass

def do_io():
    log('do_io')
    try:
        time.sleep(0.1)
    except StopWork:
        pass

def do_work(io_ratio):
    workload = [do_io if random.random() < io_ratio else do_compute for _ in range(10)]
    signal.setitimer(signal.ITIMER_VIRTUAL, 0.1, 0.1)
    for work in workload:
        work()
    signal.setitimer(signal.ITIMER_VIRTUAL, 0, 0)

# Main execution ---------------------------------------------------------------

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print 'Usage: {} io_ratio duration'.format(sys.argv[0])
        sys.exit(1)

    io_ratio = float(sys.argv[1])
    duration = int(sys.argv[2])

    signal.signal(signal.SIGVTALRM, timer_handler)

    log('start_work')
    for _ in range(duration):
        do_work(io_ratio)
    log('end_work')

# vim: set sts=4 sw=4 ts=8 expandtab ft=python:
