# Snort open source installation guide

This is a short guide on how to install and run Snort IDS, including technical prerequisites.

## Technical prerequisites

OS: Linux (pref. 20.04/22.04)

Prerequisite libs:

  sudo apt install -y gcc libpcre3-dev zlib1g-dev libluajit-5.1-dev libpcap-dev openssl libssl-dev libnghttp2-dev libdumbnet-dev bison flex libdnet autoconf libtool
  
## Installation

Create directory.

  mkdir ~/snort_src && cd ~/snort_src
  
Data Aquisition library.
  
  wget https://www.snort.org/downloads/snort/daq-2.0.7.tar.gz
  
