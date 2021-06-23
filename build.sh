#!/bin/bash

MY_DIR=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd $MY_DIR
mkdir -p build
cd build
cmake -Wno-dev ..
make
