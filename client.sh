#!/bin/sh

workloads/io-mixed-duration-short.sh
killall workloads/io-mixed-duration-short.sh
workloads/io-mixed-duration-mixed.sh
killall workloads/io-mixed-duration-mixed.sh
workloads/io-mixed-duration-mixed-burst.sh


