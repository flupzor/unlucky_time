#!/bin/sh

LD_PRELOAD=$PWD/unlucky_time.so ${@}
