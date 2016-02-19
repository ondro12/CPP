struct params
{
    in_addr destination;
	string coll;
    unsigned int intervl;
	unsigned int max_f;
	unsigned int t_timeout;    
    string input_file;
    bool input_from_stdin;
};


struct nf5_head
{
    u_int16_t version; // = 5
    u_int16_t count; // pocet zaznamu v paketu
    u_int32_t uptime_ms; // uptime v dobe odoalni
    // unixovy cas - vteriny a rezidualni nanosekundy
    u_int32_t time_sec;
    u_int32_t time_nanosec;
    u_int32_t flows_seen;// pocet zaznamenanych flowsod startu zariadenia
    u_int8_t engine_type; // = 0
    u_int8_t engine_id; // = 0
    // filler
    u_int16_t sampling; // = 0
};

// struktura flow
struct nflowv5_body {
	struct in_addr srcaddr;		// zdrojova ip adresa
	struct in_addr dstaddr;		// cielova ip adresa
	uint32_t nexthop;
	uint16_t input;
	uint16_t output;
	uint32_t dPkts ;	// pocet packetov vo flow
	uint32_t dOctets;	// celkova velkost paketu	 
	uint32_t First;		//sysuptime pri starte flow
	uint32_t Last;		//sysuptime pri prijati posledneho paketu
	uint16_t srcport;	//zdrojovy udp/tck port
	uint16_t dstport;	//cielovy udp/tcp port
	uint8_t pad1;
	uint8_t tcp_flags;
	uint8_t prot;
	uint8_t tos;		// type of service
	uint16_t src_as;
	uint16_t dst_as;
	uint8_t src_mask;
	uint8_t dst_mask;
	uint16_t pad2;
};

// struktura pre netflow paket, pouzivana pri odosielani
struct nf5_packet
{
    nf5_head h;
    nflowv5_body f[30];
    int getSize()
    {
        return sizeof(nf5_head) + h.count * sizeof(nflowv5_body);
    }
};
