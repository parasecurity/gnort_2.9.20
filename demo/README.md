# GPU pattern matching demo in real (offline) packets/traces captured and dumped by libpcap. Patterns used are set manually and are not compiled via snort rules. (NOT READY - NEEDS CONFIRMATION)! Steps below:

TASKS:

- [x] Scripts run perfectly fine.
- [ ] Parallel running and not offline (possible?).
- [ ] Run same examples inside snort's pattern matching machine (need to identify snort's search call).

## Step 1) capture and log packets (TCP and UDP only) in a file "traces.dat".

Compile:

$ gcc pcap_decoder.c -o pcap_decoder -lpcap


Run:

$ sudo ./pcap_decoder -i &lt;interfacename &rt;
	

Notes: 
  
	-argument -lpcap mandatory to compile via libpcap.h
	-sudo mandatory to gain permission to iterate through interface traffic
 	-argument -i <interfacename> not mandatory, pcap_lookupdev() finds interface automatically
  	-find <interfacename> by executing "$ ifconfig -a" and identifying the interface of the current machine
  	-file generated called traces.dat contains up to 8000 packets (will never actually be 8000, since not all packets are captured successfully, and packets other than TCP/UDP are discarded). TODO: set an average number of packets captured.
  
Packets format:
	
(protocol, source_ip, destination_ip, source_port, destination_port, payload)
  
If file traces.dat is generated successfully, move to Step 2).
  
## Step 2) create a patterns.dat file.
  
For Step 3) it's mandatory to create a patterns.dat file in order to create the dfa. 
	
  
FOR THIS DEMO: This file is created manually and is not compiled via snort rules.
	
  
Contents of the patterns.dat file:
  
	skype
	facebook
	github
	spotify
	chrome
	google
	dummysite1337
	sitedummy1337
	IRvUWDOvIB
	bwr3Hl162H
	...

  
Some patterns are likely (but not surely) to appear and others (dummysite, random strings) will not appear while searching in Step 3).
  
## Step 3) compile and run gpuregex pattern matching scripts contained in src directory.
  
Identify device position:
  
$ clinfo -l
  
Example output:
  
Platform #0: NVIDIA CUDA
 `-- Device #0: NVIDIA GeForce RTX 3060
  
Compile the source code:
  
$ make
  
Run the executable:
  
$ ./bin/gpuregex -p patterns.dat -i traces.dat -d 0 -m 0 -v

Notes:
  
	-p: pattern file
	-i: input file
	-d: device position
	-v: verbose (prints matches -- default does not print)
	-m: 1 for cpu & shared gpu, 0 for discrete gpu
  
Example output:
  
	Match: udp 192.168.1.9 192.168.1.1 39688 53 .............region1.analytics.google.com.......)........
	Match: tcp 192.168.1.9 140.82.114.21 54038 443 ............d.'M.......G...=...{!......@.ET ...l.h.....g....3.P.....'....?... .........+./.,.0............./.5.................collector.github.com..............................#.........h2.http/1.1....................................3.+.)........ [t...G...Q.f..1.cM.N..=.p+mcO..L.-.....+................Di.....h2........t.....................................................................................................................).K.&. ...7E|$..;..l.@Cm(..D...../.I..O.....! gmd@..$._(.&r.D? .......:P.(..p}
	Match: tcp 192.168.1.9 34.242.155.96 43258 443 ............W..,..$.........r.2}.Kl.fn..... )....I?M...i=._..w....l.~..p.3>d. .........+./.,.0............./.5.................spotify.demdex.net..............................#.........h2.http/1.1....................................3.+.)........ dD.a.o........(.e.R1.^..}b$..>R'.-.....+...jj...........Di.....h2ZZ............................................................................................................................................................................................................
	Match: udp 192.168.1.1 192.168.1.9 53 42919 L............dart.l.doUBLeClICK.Net......l.douBLeclICk.NEt..........0.ns1.google.com..dns-admin.I.Q.................<..)........
	Match: udp 192.168.1.1 192.168.1.9 53 36136 To...........cm.g.doubleclick.net..............#.0.ns1.google.com..dns-admin.6.S.................<..)........

	Total matches: 23

	Execution time (usecs): 1051
	
	
It is noted that google, github and spotify patterns are matched with the input packets and the script prints in the console the header and contents of the packet. Detailed:
	
(protocol, source_ip, destination_ip, source_port, destination_port, payload)
 
  
  
  
  
  
  
  
