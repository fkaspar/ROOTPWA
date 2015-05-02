#!/bin/bash
set -exv
echo $PATH
wget https://drive.google.com/open?id=0Bxmj9rGnkOZqZzRPUzZ2VEFQMk0&authuser=0 #ftp://root.cern.ch/root/root_v5.32.04.Linux-slc5_amd64-gcc4.3.tar.gz
tar -xzf root.tar.gz