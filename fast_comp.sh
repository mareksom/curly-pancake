#!/bin/bash

BIN=.bin

make --no-print-directory .${1}.cpp
make --no-print-directory ${BIN}/all/fast/${1}.e
cp ${BIN}/all/fast/${1}.e ${1}.e
