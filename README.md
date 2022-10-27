# === STILL UNDER DEVELOPMENT === ADD ALERTING AND LOGGING VIA SNORT

# Gnort (Pattern Matching on GPU) installation guide

This is a short guide on how to install and run GPU Accelerated Snort IDS, including technical prerequisites.
Base snort version: 2.9.20

[Technical Prerequisites](#possible-installation-problems) | [Installation](#installation) | [Configure Snort](#configure-snort) | [Community rules](#community-rules) | [Configure the network and rule sets](#configure-the-network-and-rule-sets) | [Validate settings](#validate-settings) | [Good rule writing sources](#good-rule-writing-sources) | [Possible installation problems](#possible-installation-problems)

## Technical prerequisites

OS: Linux (pref. 20.04/22.04)

OpenCL installed (via a local discrete GPU or Intel CPU graphics support).

Prerequisite libs:

```
sudo apt install -y gcc libpcre3-dev zlib1g-dev libluajit-5.1-dev libpcap-dev openssl libssl-dev libnghttp2-dev libdumbnet-dev bison flex libdnet autoconf libtool
```
  
## Installation
Clone repository.

```
git clone https://github.com/parasecurity/gnort_2.9.20
```
  
Either run the demo on captured offline packets/traces or setup and run GPU-snort.

Install GNORT:

```
cd daq-2.0.7
```

Install autoconf and libtool if needed.

```
sudo apt-get install -y autoconf libtool
```

Configure.

```
autoreconf -f -i
```

Install DAQ.

```
./configure && make && sudo make install
```

Go back to install Snort.

```
cd ../snort-2.9.20
```


Install Snort:

```
./configure --enable-sourcefire && make && sudo make install
```

If you run into installation problems, either look up the errors online or [click here](#possible-installation-problems) to solve common problems.

If Snort is successfully installed, follow the next steps. **mandatory!**
  
## Configure Snort
  
Update shared libraries.

```
sudo ldconfig
```

Link snort.  

```
sudo ln -s /usr/local/bin/snort /usr/sbin/snort
```

Setup username and folder structure:  

```
sudo groupadd snort
sudo useradd snort -r -s /sbin/nologin -c SNORT_IDS -g snort
```

```
sudo mkdir -p /etc/snort/rules
sudo mkdir /var/log/snort
sudo mkdir /usr/local/lib/snort_dynamicrules  
```  
Set permissions:
  
```
sudo chmod -R 5775 /etc/snort
sudo chmod -R 5775 /var/log/snort
sudo chmod -R 5775 /usr/local/lib/snort_dynamicrules
sudo chown -R snort:snort /etc/snort
sudo chown -R snort:snort /var/log/snort
sudo chown -R snort:snort /usr/local/lib/snort_dynamicrules
```

Create files for white/black lists and rules.

```
sudo touch /etc/snort/rules/white_list.rules
sudo touch /etc/snort/rules/black_list.rules
sudo touch /etc/snort/rules/local.rules
```

Copy configuration files to the desired directory.  

```
sudo cp ~/snort_src/snort-2.9.20/etc/*.conf* /etc/snort
sudo cp ~/snort_src/snort-2.9.20/etc/*.map /etc/snort
```

## Community rules

```
wget https://www.snort.org/rules/community -O ~/community.tar.gz
```

```
sudo tar -xvf ~/community.tar.gz -C ~/
```

```
sudo cp ~/community-rules/* /etc/snort/rules
```

```
sudo sed -i 's/include $RULE_PATH/#include $RULE_PATH/' /etc/snort/snort.conf
```

## Configure the network and rule sets

Prepare the conf file, comment inclusions.

```
sudo sed -i 's/include $RULE_PATH/#include $RULE_PATH/' /etc/snort/snort.conf
```

```
sudo nano /etc/snort/snort.conf
```

Find public ip (from another terminal)

```
ifconfig -a
```  
or

```  
ip addr
```

Inside snort.conf, replace any with the public ip:

```
# Setup the network addresses you are protecting
ipvar HOME_NET any
```

to

```
# Setup the network addresses you are protecting
ipvar HOME_NET XXX.XXX.XXX.XXX/XX
```

At the EXTERNAL_NET, replace any with !$HOME_NET:

```
# Set up the external network addresses. Leave as "any" in most situations
ipvar EXTERNAL_NET any
```

to

```
# Set up the external network addresses. Leave as "any" in most situations
ipvar EXTERNAL_NET !$HOME_NET
```


Do the following changes:

```
# Path to your rules files (this can be a relative path)
var RULE_PATH /etc/snort/rules
var SO_RULE_PATH /etc/snort/so_rules
var PREPROC_RULE_PATH /etc/snort/preproc_rules
```

```
# Set the absolute path appropriately
var WHITE_LIST_PATH /etc/snort/rules
var BLACK_LIST_PATH /etc/snort/rules
```

At section 6 set the output for unified2 to log under filename of snort.log like below.

```
# unified2
# Recommended for most installs
output unified2: filename snort.log, limit 128
```
  
Uncomment the line below at the bottom of the same file in order to allow snort to iterate through custom rules. 

```  
include $RULE_PATH/local.rules
```

Add community rules path, if you downloaded them.

```
include $RULE_PATH/community.rules
```

## Validate settings

```
sudo snort -T -c /etc/snort/snort.conf
```

  
## Test the configuration
  
Open the local rules file  

```
sudo nano /etc/snort/rules/local.rules
```

Copy paste testing rules  

```
alert tcp any any -> $HOME_NET any ( msg:"TCP test GITHUB"; content:"github"; fast_pattern:only; sid:10000004; rev:1; )
alert udp any any -> $HOME_NET any ( msg:"UDP test GITHUB"; content:"github"; fast_pattern:only;  sid:10000005; rev:1; )
```

The rule consists of the following parts:

* action for traffic matching the rule, alert in this case
* traffic protocol like TCP, UDP or ICMP like here
* the source address and port, simply marked as any to include all addresses and ports
* the destination address and port, $HOME_NET as declared in the configuration and any for port
* some additional bits
  * log message
  * unique rule identifier (sid) which for local rules needs to be 1000001 or higher
  * rule version number.
  
  
Run snort (edit the interfacename variable in order to sniff to the specific network interface, typically its eth0, en0 etc, this is checked by running the previous ip addr command on the terminal).

```
sudo snort -A console -i <interfacename> -u snort -g snort -c /etc/snort/snort.conf
```

You should now see alerts popping up! To stop sniffing interrupt with ctrl^c.
  
Read the logs (press TAB to find specific timestamp of sniffing)

```
snort -r /var/log/snort/snort.log.
```

## Good rule writing sources

* http://www.cyb3rs3c.net/, rule editor
* https://www.youtube.com/watch?v=8T8XVoNqMbc
* https://cyvatar.ai/write-configure-snort-rules/

## Possible installation problems

1) You don't have make installed.

```
sudo apt install make
```

2) During snort installation, header files are missing. For example:

```
fatal error: rpc/rpc.h: No such file or directory
   32 | #include <rpc/rpc.h>
```

You can either find online the source code of the specific headers or look through the already installed libraries and copy them to the desired directory.

```
sudo apt install apt-file
```

```
sudo apt-file update
```

-> Search for the missing file:

```
apt-file search rpc/rpc.h
```

Example output:

```
/snort_src/snort-2.9.20$ apt-file search rpc/rpc.h
dietlibc-dev: /usr/include/diet/rpc/rpc.h 
krb5-multidev: /usr/include/mit-krb5/gssrpc/rpc.h
libkrb5-dev: /usr/include/gssrpc/rpc.h
libntirpc-dev: /usr/include/ntirpc/rpc/rpc.h
libtirpc-dev: /usr/include/tirpc/rpc/rpc.h
```

Copy file to desired location from any source

```
cp /usr/include/ntirpc/rpc/rpc.h /usr/include/rpc
```

Try to install again

```
./configure --enable-sourcefire && make && sudo make install
```

Repeat searching and copying files if the error persists. If the installation succeeds continue configuration [here](#configure-snort).

## Running Snort in the background
  
Needed ?



  
  
