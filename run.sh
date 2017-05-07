#!/bin/sh

LD_PRELOAD=$PWD/.libs/libunlucky.so ${@}
