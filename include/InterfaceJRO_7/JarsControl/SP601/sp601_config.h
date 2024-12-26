#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include  <vector>

#include <string>
using namespace std;

class sp601_config
{
public:

    char *sp601_ip;
    char *SP601_MAC;   		        //  Must convert to hex if used//Espacio de memoria donde se guarda el valor de la MAC del SP601 + null
    char *SP601_IP;          		// Defined in Xilnx ISE ethernet_control.v//Espacio de memoria donde se guarda el valor de la IP del SP601 + null
    int SP601_PORT;                     // Defined in Xilnx ISE ethernet_control.v//Espacio de memoria donde se guarda el valor del puerto del SP601 + null
    string sp601_ip_string;		// IP del SP601 usado cuando definimos un endpoint tipo boost.
    char *IP_host;			// Se usará para mostrar info de la IP switch-send
    char *MAC_host;			// Se usará para mostrar info del MAC en switch-send
    int PORT_host;			// Se usará para mostrar info del puerto en switch-send

    char REPLY_START_DATA_PAYLOAD[2];   // AB, defined in DEFAULT_START_DATA_PAYLOAD (ethernet_control.v)
    //char UDP_timeout;

    bool VERBOSE_MODE;			// Atributo de sp601_config
    sp601_config();			// Declara un constructor
};
