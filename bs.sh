#!/bin/bash
cd Server/build
cmake ..
make -j8
./florr_incremental-server