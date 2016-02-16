#/bin/bash

touch driver/test.cpp && make CFLAGS+=-DUSE_SIMULATOR
