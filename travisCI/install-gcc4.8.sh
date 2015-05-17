#!/bin/bash
set -e 

sudo apt-get install python-software-properties
sudo add-apt-repository ppa:ubuntu-toolchain-r/ppa -y
sudo apt-get update -qq
sudo apt-get install gcc-4.8
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 50
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 50
