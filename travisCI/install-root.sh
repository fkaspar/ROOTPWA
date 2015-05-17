#!/bin/bash
set -exv
echo $PATH
mkdir root-$ROOT_VER
if [ "$ROOT_VER" = "5.34.30" ]; then wget https://www.dropbox.com/s/hfy0e4zch7oug0k/root-5.34.30.tar.gz; fi 
if [ "$ROOT_VER" = "6.02.08" ]; then wget https://www.dropbox.com/s/kzihg34enf2niqt/root-6.02.08.tar.gz; fi
tar -xzf root-$ROOT_VER.tar.gz -C ./root-$ROOT_VER --strip-components=1