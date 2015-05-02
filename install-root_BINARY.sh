#!/bin/bash
set -exv
echo $PATH
wget https://docs.google.com/uc?export=download&confirm=gQWw&id=0Bxmj9rGnkOZqZzRPUzZ2VEFQMk0 #ftp://root.cern.ch/root/root_v5.32.04.Linux-slc5_amd64-gcc4.3.tar.gz
tar -xzf root.tar.gz