#!/bin/bash
set -exv
mkdir ./libconfig && cd ./libconfig
wget http://www.hyperrealm.com/libconfig/libconfig-1.4.9.tar.gz
tar -xzf libconfig-1.4.9.tar.gz
mkdir libconfig_install
./libconfig-1.4.9/configure --prefix=$PWD/libconfig_install
make && make install