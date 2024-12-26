
#include "fpga/fpga_com.h"
#include "sp601_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

//#include <boost/asio.hpp>

#define RCV_BUFSIZE 1472

enum JARS_CMD {	NOOP, SET_SP601_IP, SET_SP601_PORT, GET_SP601_MAC, SET_COMMS_IP, SET_COMMS_PORT, SET_COMMS_MAC, GET_VALUE, DATA_LOOPS, SEND_COMMAND, CHECK_ACK, SEND_FILTER};

// Se utilizan las definiciones de comandos del archivo ExperimentConstants.h
#define		RST_CTRL				0x05
#define		MODE_OPER				0x03
#define		ELECTION_RXD				0x04
#define		POWER_RXD				0x06
#define		PROG_RXD				0x09
#define		ENABLE_TRANSF				0x0A
#define		DISABLE_TRANSF				0x0B

struct address{
   const char* d_path_file;
   std::string d_mac_block;
   int d_port_block;
   std::string d_ip_host;
};


//Declara la estructura jars_cmd: {"NOOP",0,0,false}
struct jars_cmd{
	const char *name;
	char val;
	char size;
	bool ack;
};


class sp601_command
{
public:
	sp601_command();
	~sp601_command();
	void init(address host_address);
	ssize_t receive(char *buffer_rcv);
	void send(uint8_t cmd, char code, const char *payload, size_t size_payload);
};

