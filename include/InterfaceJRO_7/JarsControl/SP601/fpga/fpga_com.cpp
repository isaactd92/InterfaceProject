
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>
#include <boost/asio.hpp>
#include <boost/format.hpp>

#include "fpga_com.h"

#include <iostream>
using namespace std;

boost::asio::ip::udp::socket* local_socket;
boost::asio::io_service local_io_service;
boost::asio::ip::udp::endpoint destino_endpoint_to;


//fpga_con_init(&fc, cfg.SP601_IP, cfg.COMMS_PORT, cfg.SP601_PORT)
//**************************** Define y establece el socket con las caracteísticas respectivas******************************
//void fpga_con_init( fpga_con_t *con, const char *daddr, int lport, int dport ) {
void fpga_con_init(char* sp601_ip_char, int Sp601_Port, int lport) { //(#HOST y # PORT llamados desde el bloque)

	printf("<<------------------------Estoy dentro de fpga_con_init--------------------------------->\n");
	printf("*****************************************************************************************\n");
	//printf("EL valor de sp601_ip es: %s \n",sp601_ip_char);		     	
	//************************************ Define un endpoint de destino:SP601 ************************************************************************	
	std::string valor_string = string(sp601_ip_char);
	//cout<<"El valor de valor_string es: \n"<< valor_string<<endl;
	boost::asio::ip::address ip_destino = boost::asio::ip::address::from_string(valor_string);
	//printf("El valor del puerto Sp601_Port es: %d\n",Sp601_Port);
	boost::asio::ip::udp::endpoint destino_endpoint(ip_destino, Sp601_Port);//int SP601_PORT
	destino_endpoint_to= destino_endpoint;
	//printf("<<------------------------Estoy después de destino_endpoint--------------------------------->\n");
	//******************************** Define un endpoint local , crea y abre un socket local *********************************************************
	//Creamos un endpoint:representing all IP addresses available on the host and the specified port number.
	boost::asio::ip::udp::endpoint local_endpoint(boost::asio::ip::address_v4::any(),lport);
        //Crea un puntero a un socket
        //printf("<<------------------------Estoy despues de local endoint--------------------------------->\n");
 	local_socket = new boost::asio::ip::udp::socket(local_io_service);
        local_socket->open(local_endpoint.protocol()); // abre el socket especificando el protocolo asociado al endpoint-udp protocolo
	//printf("<<------------------------Estoy despues de open--------------------------------->\n");
	//******************************* Setea el socket *************************************************************
	boost::asio::socket_base::reuse_address roption(true);//implementa la opción SOL_SOCKET/SO_REUSEADDR para el del socket
	local_socket->set_option(roption);//Establece la opción roption en el socket
	boost::asio::socket_base::receive_buffer_size option(2097152);//Máximo valor que obtendrá para guardar por si se quedan = 2097152
	local_socket->set_option(option);
	//printf("<<------------------------Estoy despues de setSocket--------------------------------->\n");	
	//***************************** Bind al endponit local ************************************************************
	local_socket->bind(local_endpoint);//enlaza el socket al endpoint establecido// Le da al socket dirección y puerto en la red
	//printf("<<------------------------Estoy despues de bind--------------------------------->\n");	
	
}

// ssize_t is the same as size_t, but is a signed type - read ssize_t as “signed size_t”.
//ssize_t is able to represent the number -1, which is returned by several system calls and library functions as a way to indicate error.
//***************Envía los paquetes de datos de buf a través del socket especificado en fpga_con_t********************************************************

ssize_t fpga_con_send(const char *buf, size_t len ) { //ssize_t , is read signed size_t , which can give us -1 as an error.*/
/*The sendto function transmits the data in the buffer through the socket socket to the destination
 address specified by the addr and length arguments. The size argument specifies the number of bytes to
 be transmitted. */
   printf("Estoy dentro de fpga_con_send\n");// send to -> 192.168.1.6 (donde se envía debe tener bind)
   ssize_t bytes_send = local_socket->send_to(boost::asio::buffer(buf, len), destino_endpoint_to);//(void*) buf//se puede quitar el boost::asio::buffer pero al ver GNURadio UDP sink y source lo usan sin problemas
  // printf("Estoy después de send_to \n");	
   if( bytes_send < 0 ) {
	     printf("Error en el envío. Revisar fpga_con_send\n");
    }
  //  printf("Estoy al final de fpga_con_send");	
    return bytes_send;   
}

// blocking receive: this call will block until a packet is received
//****************** Recibirá data y luego lo bloqueará hasta que haya alguna data por leer*********************
ssize_t fpga_con_recv(char *dbuf, size_t dsize ) {

	//printf("Estoy esperando recibir ...\n");
	ssize_t bytes_receive = local_socket->receive(boost::asio::buffer(dbuf, dsize));//no se especificó en endpoint de envío
	//printf("Estoy después de receive \n");
	if( bytes_receive < 0 ) {
		 printf("Error en la recepción. Revisar fpga_con_recv\n");
	}
	//printf("La cantidad de bytes recibidos son: %d \n", bytes_receive);
	//printf("Estoy al final de fpga_con_recv\n");
	return bytes_receive;

}


