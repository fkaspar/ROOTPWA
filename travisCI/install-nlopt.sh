#!/bin/bash
set -exv
export PARENT_DIR=${PWD}
mkdir -p ${PARENT_DIR}/nlopt-build
mkdir -p ${PARENT_DIR}/nlopt-install
wget http://ab-initio.mit.edu/nlopt/nlopt-2.4.2.tar.gz
tar -xzf nlopt-2.4.2.tar.gz -C ${PARENT_DIR}/nlopt-build --strip-components=1
cd ${PARENT_DIR}/nlopt-build
./configure --prefix=${PARENT_DIR}/nlopt-install --with-cxx && make && make install
