#!/bin/bash
set -exv
wget http://root.cern.ch/download/root_v5.34.30.source.tar.gz
tar -xzf root_v5.34.30.source.tar.gz
cd root && ./configure --enable-mathmore --enable-minuit2 && make 
. bin/thisroot.csh
