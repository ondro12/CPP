/*
* Projekt ISA 2015
* Varianta -> Offline NetFlow sonda
* autor: Matus Ondris -> xondri04
*/

#include <stdbool.h>
#include <pcap.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <pcap.h>
#include <string.h>
#include <getopt.h>
#include <vector>
#include <signal.h>
#include <sys/time.h>
#include <string>
#include <iostream>
#include <stdexcept>
using namespace std;
#include <arpa/inet.h>
#include "dalej.h"


// vychodzie hodnoty parametrov
const int inval = 300;
const string collector = "127.0.0.1:2055";
const int maxFlows = 50;
const int tcpTimeout = 300;
timespec started;
vector<struct nflowv5_body *> flow_arr;
vector<struct nflowv5_body *> expired;


// popis programu (help message)
void usage (void)
{
    cout
    << "Pouzitie: " << endl
    << "isa_exporter -h" << endl
    << "isa_exporter [-i <file>] [-c <netflow_collector>[:<port>]]" << endl
	<< "[-I <interval>] [-m <count>] [-t <seconds>]" << endl
    << "kde:" << endl
    << "-h                 vypise napovedu" << endl
    << "-i <file>          nazov spracovavaneho .pcap suboru " << endl
    << "default : stdin" << endl
    << "-c <netflow_collector>:<port> IP adresa a port netflow kolektoru" << endl
    << "default : " << collector << endl
    << "<-I <interval>  interval v sekundach po ktorom sa exportuju zaznamy na kolektor" << endl
    << "default : " << inval << endl
    << "-m <count>   maximalne mnozstvo flow po ktorom dosiahnuti dojde k exportu záznamov na kolektor" << endl
    << "default : " << maxFlows << endl
    << "-t <seconds>    interval po ktoreho vyprsani je mozne povazovat monitorovane TCP spojenie za ukoncené (a mozné ho oznacit ako expirované)" << endl
    << "default : " << tcpTimeout << endl;
}

// funkia na kontrolu ci uz nieje flow vo vektore zaznamenavajucom spracovavane flows
bool jeTam(vector<struct nflowv5_body *> &flow_arr, struct nflowv5_body *ptr,unsigned int t_timeout);
// funkia na kontrolu ci uz nieje flow vo vektore expirovanych
bool jeExp(vector<struct nflowv5_body *> &flow_arr, struct nflowv5_body *ptr);
// zikanie sytemoveho casu;
uint32_t getSysUptimeSeconds();

int main (int argc, char *argv[]){

	unsigned int size_farray = 0;
	clock_gettime(CLOCK_MONOTONIC, &started);
    params p;
	int f_count;

	p.intervl = inval;	
	p.max_f = maxFlows;
	p.coll = collector;
	p.t_timeout = tcpTimeout;
	p.input_from_stdin = 1;

    
    // sprcovanie argumentov prik.riadku
    char argument;
    while ( (argument = getopt (argc, argv, "hi:c:I:m:t:")) != -1)
    {
        switch (argument)
        {
                // napoveda
            case 'h':
                usage();
                return EXIT_SUCCESS;
                // vstupny spracovavany .pcap subor
            case 'i':
                p.input_file = optarg;
				p.input_from_stdin = 0;
                break;
                // IP adresa  port kolektoru
            case 'c':
				p.coll = optarg;
                break;
				// interval
            case 'I':
                p.intervl = atoi (optarg);
                break;
                // maximalny pocet flows
            case 'm':
                p.max_f = atoi (optarg);
                break;
                // tcp timeout
            case 't':
                p.t_timeout = atoi (optarg);
                break;
                // pre pripad chyby v spracovani parametrov
            default:
                usage();
                return EXIT_FAILURE;
        }
    }

	//pokus o otvorenie pcap suboru
	if(p.input_from_stdin == 0){

		struct pcap_pkthdr header;
		const u_char *cur_packet;
		pcap_t *handle;
		char errbuf[PCAP_ERRBUF_SIZE];
		handle = pcap_open_offline(p.input_file.c_str(), errbuf);


		//overenie , ci je zadany pcap subor mozne otvorit
		if (handle == NULL) {
			printf("Couldn't open pcap file %s: %s\n", p.input_file.c_str(), errbuf);
			return -1;
		}

		int count = 1; // pocitac paketov

		// cyklus nacitvania a spracovavania paketov v pcap subore
		while (cur_packet = pcap_next(handle, &header)) {
			// kontrola ci nevyprsal casovy interval pre odoslanie na kolektor
			if(getSysUptimeSeconds() >= started.tv_sec+p.intervl){
			// vyprsal casovy interval odoslanie na kolektor

			
			}

			//vytvorenie prvku struktury nfv5 body aliaz b4 tabuky
			nflowv5_body* f = new nflowv5_body();
			f->srcport = 0;
			f->srcport = 0;
			f->nexthop = 0;
			
			//  Standardna informacia o pakete,
			count = count + 1;
			
			// ziskanie ip hlavicky a vycitanie src a dest addr protokolu a tos
			cur_packet += 14;
			struct ip *ip_header = (struct ip*) cur_packet;
			f->srcaddr = ip_header->ip_src;
			f->dstaddr = ip_header->ip_dst;
			f->prot = ip_header->ip_p;
			f->tos = ip_header->ip_tos;				

			//  informacie o pakete resp. zistenie portov u protokolov tcp a udp 
			if (f->prot == 6) {
				cur_packet += 20;
				struct tcphdr *tcp_header = (struct tcphdr*) cur_packet;
				f->srcport = ntohs(tcp_header->th_sport);
				f->dstport = ntohs(tcp_header->th_sport);
				f->tcp_flags = tcp_header->th_flags;
			}
			else if (f->prot == 17) {
				cur_packet += 20;
				struct udphdr *udp_header = (struct udphdr*) cur_packet;
				f->srcport = ntohs(udp_header->uh_sport);
				f->dstport = ntohs(udp_header->uh_dport);
			}

			//ak sa flow uz nachadza v expirovanych tokoch , paket s zahodi
			if((f->prot == 6) && (jeExp(flow_arr,f) == true)){}

			// kontrola ci sa uz spracovavana flow nachadza vo vektore flows ak nie prida ju tam , ak ano prevedie prislusne operacie
			else if(jeTam(flow_arr,f,p.t_timeout) == true){
				f->dPkts = 0;
				size_farray++;
				f_count++;
				f->First = getSysUptimeSeconds();
				f->Last = getSysUptimeSeconds();
				flow_arr.push_back(f);

			}
			// kontrola , ci nebol dosiahnuty parameter -m rep. maxflows v pamati resp. vektore flow_arr , ak ano TODO export na 				kolektor
			if(size_farray == p.max_f){
					//TODO export na kolektor dosiahnutie max flow v pamati
			}

		}
			//TODO export na kolektor vsetky pakety co ostali
		pcap_close(handle);
		return 0;
	}
}

uint32_t getSysUptimeSeconds(){
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	uint32_t seconds = ts.tv_sec - started.tv_sec;
	return seconds;
}

// funkcia sluziaca na kontrolu , ci zadane flow uz nieje expirovane
bool jeExp(vector<struct nflowv5_body *> &expired, struct nflowv5_body *ptr) {
	for (vector<struct nflowv5_body *>::iterator it = expired.begin() ; it != expired.end(); ++it) {
		if ((ptr->srcaddr.s_addr == (*it)->srcaddr.s_addr) && (ptr->dstaddr.s_addr == (*it)->dstaddr.s_addr) && (ptr->prot == (*it)->prot) && (ptr->srcport == (*it)->srcport) && (ptr->dstport == (*it)->dstport)){ // spracovavane flow uz je vo vektore
			return true;
		}
	}
		return false;
}

// funkcia sluziaca na kontrolu , ci sa aktualne spracovavane flow nachadza vo vektore ak ano vrati true inak false
bool jeTam(vector<struct nflowv5_body *> &flow_arr, struct nflowv5_body *ptr,unsigned int t_timeout) {
	for (vector<struct nflowv5_body *>::iterator it = flow_arr.begin() ; it != flow_arr.end(); ++it) {
		if ((ptr->srcaddr.s_addr == (*it)->srcaddr.s_addr) && (ptr->dstaddr.s_addr == (*it)->dstaddr.s_addr) && (ptr->prot == (*it)->prot) && (ptr->srcport == (*it)->srcport) && (ptr->dstport == (*it)->dstport)){ // spracovavane flow uz je vo vektore
				// kontrola casu ci nedoslo k expiracii flow ak ano prida do vektoru expired
				if((ptr->prot == 6) && (ptr->Last >= (ptr->First+t_timeout))){
					expired.push_back(ptr);
			return true;
			}
			ptr->dPkts = ptr->dPkts+1;	
			ptr->Last = getSysUptimeSeconds();	
		return true;
		}
	}
	return false;
}

