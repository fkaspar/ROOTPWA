#!/bin/bash
set -exv
wget http://root.cern.ch/download/root_v5.34.30.source.tar.gz
tar -xzf root_v5.34.30.source.tar.gz
mkdir build_root
mkdir install_root
cd build_root && cmake -Dmathmore=ON -Dminuit2=ON -DCMAKE_INSTALL_PREFIX=../install_root ../root && make && make install
. bin/thisroot.sh
