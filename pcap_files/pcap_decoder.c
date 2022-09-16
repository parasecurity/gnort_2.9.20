#include <stdio.h>
#include <pcap.h>

int main(int argc, char *argv[])
{

	char *dev = argv[1];
	char * errbuf[PCAP_ERRBUF_SIZE];

	printf("Device: %s\n", dev);
	
	pcap_t *handle;
	handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
		return(2);
	}
	if (pcap_datalink(handle) != DLT_EN10MB) {
		fprintf(stderr, "Device %s doesn't provide Ethernet headers - not supported\n", dev);
		return(2);
	}
	return(0);
}

// gcc pcap_decoder -o pcap_decoder -lpcap
