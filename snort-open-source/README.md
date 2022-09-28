# Snort open source installation guide

This is a short guide on how to install and run Snort IDS, including technical prerequisites.

## Technical prerequisites

OS: Linux (pref. 20.04/22.04)

Prerequisite libs:

```
sudo apt install -y gcc libpcre3-dev zlib1g-dev libluajit-5.1-dev libpcap-dev openssl libssl-dev libnghttp2-dev libdumbnet-dev bison flex libdnet autoconf libtool
```
  
## Installation

Create directory.

```
  mkdir ~/snort_src && cd ~/snort_src
```
  
Data Aquisition library (DAQ), to make abstract calls to packet capture libraries.
  
```
wget https://www.snort.org/downloads/snort/daq-2.0.7.tar.gz
```

Extract.

```
tar -xvzf daq-2.0.7.tar.gz
cd daq-2.0.7
```

Install autoconf and libtool if needed.

```
sudo apt-get install -y autoconf libtool
```

Configure.

autoreconf -f -i

Install DAQ.

./configure && make && sudo make install

Go back to install Snort.

cd ~/snort_src

Download version (latest 2.9.20):

wget https://www.snort.org/downloads/snort/snort-2.9.20.tar.gz

or specify (if other version is chosen, adjust version tag in the future steps):

wget https://www.snort.org/downloads/snort/snort-<version>.tar.gz
  
Extract:
  
tar -xvzf snort-2.9.20.tar.gz
cd snort-2.9.20
  
Install Snort:
  
./configure --enable-sourcefire && make && sudo make install

Snort is successfully installed. **The following steps are mandatory!**
  
## Configure Snort
  
Update shared libraries.
  
sudo ldconfig

Link snort.  

sudo ln -s /usr/local/bin/snort /usr/sbin/snort
  
Setup username and folder structure:  

sudo groupadd snort
sudo useradd snort -r -s /sbin/nologin -c SNORT_IDS -g snort

sudo mkdir -p /etc/snort/rules
sudo mkdir /var/log/snort
sudo mkdir /usr/local/lib/snort_dynamicrules  
  
Set permissions:
  
sudo chmod -R 5775 /etc/snort
sudo chmod -R 5775 /var/log/snort
sudo chmod -R 5775 /usr/local/lib/snort_dynamicrules
sudo chown -R snort:snort /etc/snort
sudo chown -R snort:snort /var/log/snort
sudo chown -R snort:snort /usr/local/lib/snort_dynamicrules
  
Create files for white/black lists and rules.
  
sudo touch /etc/snort/rules/white_list.rules
sudo touch /etc/snort/rules/black_list.rules
sudo touch /etc/snort/rules/local.rules
  
Copy configuration files to the desired directory.  

sudo cp ~/snort_src/snort-2.9.16/etc/*.conf* /etc/snort
sudo cp ~/snort_src/snort-2.9.16/etc/*.map /etc/snort

## Community rules
  
wget https://www.snort.org/rules/community -O ~/community.tar.gz

sudo tar -xvf ~/community.tar.gz -C ~/

sudo cp ~/community-rules/* /etc/snort/rules

sudo sed -i 's/include $RULE_PATH/#include $RULE_PATH/' /etc/snort/snort.conf

## Configure the network and rule sets
  
sudo nano /etc/snort/snort.conf

Find public ip (from another terminal)
  
ifconfig -a
  
or
  
ip addr

Inside snort.conf edit server_public_ip from ifconfig -a:
  
# Setup the network addresses you are protecting
ipvar HOME_NET server_public_ip/32




