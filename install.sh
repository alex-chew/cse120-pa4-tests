#!/bin/bash

cp -i Makefile acutest.h assimilate.sh runall.sh ~/pa4
rm -rf ~/pa4/tests
cp -r tests ~/pa4
cd ~/pa4
./assimilate.sh
