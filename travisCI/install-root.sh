#!/bin/bash
set -exv
echo $PATH
if[ "$ROOT_VER"="5.34.30" ]; then wget https://www.dropbox.com/s/vbpvign4f4sr13k/root.tar.gz; fi 
if[ "$ROOT_VER"="6.02.08" ]; then wget https://www.dropbox.com/s/kzihg34enf2niqt/root-6.02.08.tar.gz; fi
tar -xzf root-$ROOT_VER.tar.gz