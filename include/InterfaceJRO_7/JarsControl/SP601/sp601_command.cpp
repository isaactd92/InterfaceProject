
#include "sp601_command.h"

sp601_config cfg;// objeto de sp601_config

//Array of structures: {const char *name,char val,char size,bool ack}
struct jars_cmd JARS_CMD[] = {
	[NOOP]			=	 	{"NOOP",0,0,false},			// No effect or ACK
	[SET_SP601_IP]		=		{"SET_SP601_IP",1,4,true},		// Set IP address for SP601. No ACK since we might not be able to see it if 												the COMMS_IP has not been setup
	[SET_SP601_PORT]	=		{"SET_SP601_PORT",2,2,true},		// Set Port address for SP601
	[GET_SP601_MAC]		=		{"GET_SP601_MAC",3,6,true},		// Get MAC address for SP601. Note that we don't get to SET this. It's done 												in HW through DIP switches
	[SET_COMMS_IP]		=		{"SET_COMMS_IP",4,4,true},		// Set IP address for communication PC
	[SET_COMMS_PORT]	=		{"SET_COMMS_PORT",5,2,true}, 		// Set Port address for communication PC
	[SET_COMMS_MAC]		= 		{"SET_COMMS_MAC",6,6,true}, 		// Set MAC address for communication PC
	[GET_VALUE]		=	 	{"GET_VALUE",8,0,false}, 		// Get different values stored in FPGA
	[DATA_LOOPS]		= 		{"DATA_LOOPS",63,8,false}, 		// Up to 16-bits to loop packet sending. 16-bit (effective 14 bits in HW) 												for packet size. This is used to test for bandwidth in the Gigabit Connection
	[SEND_COMMAND]	  	= 		{"SEND_COMMAND",64,3,false}, 		// Up to 16-bits (effective 14 bits in HW) to request #32-bit words to start 												burst data transfer. Note, due to IP packet packaging, should be no more 												than 367 words
	[CHECK_ACK]		= 		{"CHECK_ACK",65,1,false}, 		// ack-command
	[SEND_FILTER]		=	 	{"SEND_FILTER",66,0,false} 		// Send filter
};


sp601_command::sp601_command()
{}

sp601_command::~sp601_command()
{}

//************************************* Funciones usadas en el método send **********************************************

//******************** Métodos usados por swich en send **************************************************
//*************** Guarda cada numero de la IP dentro del arreglo parts en formato char *******************
void IPstr_to_bytearray(char *IPstr, char *parts){
  char * pch;
  uint8_t ii=0;
  pch = strtok (IPstr,".");//divide el IPstr , dilimitado por .
  while (pch != NULL)
  {
    parts[ii] = (char)atoi(pch);//convierte string a int y luego lo guarda como char
    pch = strtok (NULL, ".");
    ii=ii+1;
  }
}

//************************** Nos da la IP en forma binaria y string ****************************************
void bytearray_to_IPstr(const char *bytes, char *IPstr){
  sprintf(IPstr,"%i.%i.%i.%i",(uint8_t)bytes[0],(uint8_t)bytes[1],(uint8_t)bytes[2],(uint8_t)bytes[3]);
  //printf("%s\n", IPstr);
}

//************************** Convierte de bytes a enteros **************************************************
void bytearray_to_int(const char *byte_array, int num){
  for (size_t i = 0; i < 2; i++) {
    num += (uint16_t)((uint8_t)byte_array[i] << (i*8));
  }
}

//*************** Guarda cada numero hexadecimal de la máscara dentro del arreglo parts en formato char *******************
void MACstr_to_bytearray(char *MACstr, char *parts){
  char *pch;
  uint8_t ii=0;
  pch = strtok (MACstr,":");
  while (pch != NULL)
  {//long int strtol(const char *str, char **endptr, int base)
    parts[ii] = (char)strtol(pch,NULL,16);//cambia de string a long int, tomando el valor hexadecimal 16, y lo guarda como tipo char
    //printf("p[%i]: %i\n", ii,(uint8_t)parts[ii]);
    pch = strtok (NULL, ":");
    ii=ii+1;
  }
}

//*************** Divide el numero por bytes y lo guarda en arrayOfByte *****************************************************
void intToBytes(int number, char *arrayOfByte, size_t size){
  for (size_t i = size; i > 0; i--) {
    arrayOfByte[size-i] = (char)(number >> ((i-1)*8));
  }
}

//*************** Guarda los valores del hexadecimal en un arreglo string literal ********************************************
void bytearray_to_MACstr(const char *bytes, char *MACstr){
  sprintf(MACstr,"%02X:%02X:%02X:%02X:%02X:%02X",(uint8_t)bytes[0],(uint8_t)bytes[1],(uint8_t)bytes[2],(uint8_t)bytes[3],(uint8_t)bytes[4],(uint8_t)bytes[5]);
}

// Implementation of itoa()
//itoa(cfg.COMMS_PORT,(char *)info_str,10);
//************** Guarda dentro de str los valores del num en caracteres char[] ***************
char* itoa(int num, char* str, int base){
    int i = 0;
    bool isNegative = false;

    // Handle 0 explicitely, otherwise empty string is printed for 0 
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    // In standard itoa(), negative numbers are handled only with
    // base 10. Otherwise numbers are considered unsigned.
    if (num < 0 && base == 10)
    {
        isNegative = true;
        num = -num;
    }

    // Process individual digits
    //guarda cada digito de num en str (el último dígito en el primer elemnto del str)/al reves
    while (num != 0)
    {
        int rem = num % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        num = num/base;
    }

    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';

    str[i] = '\0'; // Append string terminator

    // Reverse the string
    int n = strlen(str);

    // Swap character starting from two
    // corners
    for (int i=0; i<n/2; i++)
       swap(str[i], str[n-i-1]);

    return str;
}
//*****************************************************************************************************************************************

//***** Define el socket y establece las direcciones de los puertos, la MAC y la IP de la Pc y el SP601************************************
void sp601_command::init(address host_address)
{
	printf("-----------------------------------------------------------------------------------");
	printf("****************** Estoy dentro del init de sp601_command*************************\n");	
	//****************** Convierte de string a char* la ip y mac local	
	int mac_size= ((host_address.d_mac_block).size());
	char mac_array_char [mac_size+1]={};
	strcpy (mac_array_char,(host_address.d_mac_block).c_str());
	
	int ip_size= ((host_address.d_ip_host).size());
	char ip_array_char [ip_size+1]={};
	strcpy (ip_array_char,(host_address.d_ip_host).c_str());

	//******************* Empieza el envio de comandos 
	char payload[16]={};//lo define así ya que usa los subindices para contar
	printf("Connecting to SP601 IP: %s \n", cfg.SP601_IP);//muestra la IP del SP601
	//************** Define el socket : crea , direcciona y configura el socket UDP*****************************************************
	//fpga_con_init(cfg.sp601_ip_string,cfg.SP601_PORT,host_address.d_port_block);//Declar estos valores en el .h y definirlos en el constructor//char * IP// cfg
	fpga_con_init(cfg.sp601_ip,cfg.SP601_PORT,host_address.d_port_block);	
	printf("Se creó el socket UDP -any bind- con dirección de IP host igual a : %s\n",ip_array_char);
	//************** Envía y establece la dirección IP del SP601 ************************
	printf("************Estoy antes de SET_SP601_IP*********\n");	
	//el valor de solo los números de la dirección de la IP es guardado en payload de tipo char
	IPstr_to_bytearray(cfg.SP601_IP, payload);
	//printf("Estoy después de IPstr_to_bytearray\n");
	//Envia el valor de la IP del SP601 al SP601
	send(SET_SP601_IP,0,payload,4);//4 bytes del IP
	//setea a cero char los valores del payload
	memset(payload,0,strlen(payload));
	//printf("Estoy después del send SET_SP601_IP\n");
	//************** Envía y establece la dirección IP de la PC ************************
	printf("*************Estoy antes de SET_COMMS_IP**********\n");	
	//el valor de solo los números de la dirección de la IP es guardado en payload de tipo char
	IPstr_to_bytearray(ip_array_char, payload); //valor de IP-host en char* dado por el GNURadio block
	//Envia el valor de la IP de la PC al SP601
 	send(SET_COMMS_IP,0,payload,4);
 	//setea a cero char los valores del payload
	memset(payload,0,strlen(payload));
	//printf("Estoy después del send SET_COMMS_IP\n");
	//************** Envía y establece la MAC de la PC  ************************************
	printf("***************Estoy antes de SET_COMMS_MAC*********\n");	
	//Los valores hexadecimales de la mascará de la PC son guardados en payload en formato tipo char
	MACstr_to_bytearray(mac_array_char, payload); // valor del MAC -host en char* dado por el GNURadio block
	//Envia el valor de la MAC de la PC al SP601
	send(SET_COMMS_MAC,0,payload,6);
	//setea a cero char los valores del payload
	memset(payload,0,strlen(payload));
	//printf("Estoy después del send SET_COMMS_MAC\n");	
	//************** Envía y establece el puerto de la PC ************************************
	printf("****************Estoy antes de SET_COMMS_PORT********\n");		
	//Divide el puerto de la PC y lo guarda en el payload
	intToBytes(host_address.d_port_block, payload, 2);
	//Envía el valor del puerto de la PC al SP601
	send(SET_COMMS_PORT,0,payload,2);
	//setea a cero char los valores del payload
	memset(payload,0,strlen(payload));
	//printf("Estoy después del send SET_COMMS_PORT\n");
	//************** Envía y establece el puerto del SP601 ************************************
	printf("*****************Estoy antes de SET_SP601_PORT********\n");	
	//Divide el puerto de la SP601 y lo guarda en el payload
	intToBytes(cfg.SP601_PORT,payload,2);
	//Envía el valor del puerto de la SP601 al SP601
	send(SET_SP601_PORT,0,payload,2);
	//setea a cero char los valores del payload
	memset(payload,0,strlen(payload));
	//We can check the current values for MAC, IP and PORT
	//printf("Estoy después del send SET_SP601_PORT\n");
}

//****************************** Nos da el número de bytes recibidos
ssize_t sp601_command::receive(char *buffer_rcv)
{
	ssize_t size_rcv_buf = fpga_con_recv(buffer_rcv,RCV_BUFSIZE);
	//printf("Se recibió el paquete en el método receive <---\n");
	return size_rcv_buf;
}


//void send(uint8_t cmd, char code, const char *payload,char size_payload)
//void send(uint8_t cmd, char code, const char *payload,size_t size_payload)
//void sp601_command::send(uint8_t cmd, char code, const char *payload, char size_payload)
//**********************Dependiendo del cmd y el payload genera el buffer que guardará el comando de envio: cmd_array**************************
//**********************Genera el buffer que guardará el paquete de data recibido: buffer_rcv**************************
//**********************Verifica los buffers creados imprimiéndolos (printf)******************************************
//********************** Envía el comando respectivo (PC->SP601) y recibe el comando de respuesta (SP601->PC) /Revisar Excel*******************
void sp601_command::send(uint8_t cmd, char code, const char *payload, size_t size_payload)
{
	//printf("---------> Estoy dentro de send <------\n");	
	//char cmd_array[16];
	char cmd_array[200];
	char buffer_rcv[RCV_BUFSIZE];//buffer donde se guardará el mensaje que recibe
	int size_rcv_buf=0;
	bool ack_received;
	//char size_payload;
	size_t size_cmd_array;
	uint8_t *info_str;
	//define VERBOSE_MODE para el objeto cfg
	cfg.VERBOSE_MODE = true;//********
	 //printf("%i\n",payload[0]);printf("%i\n",payload[1]);printf("%i\n",payload[2]);
	//printf("%02X,",(uint8_t)payload[0]);
	//printf("%02X,",(uint8_t)payload[1]);
	//printf("%02X,",(uint8_t)payload[2]);
	//printf("%02X\n",(uint8_t)payload[3]);	
	//size_payload = strlen(payload);
	//size_payload = payload.__size;
	//printf("%i",size_payload);
	
	size_cmd_array = 3+size_payload;//tamaño total después de agregar cmd_array[0],[1],[2]
	//printf("%i",size_cmd_array);
	//**************************** Crea el buffer de comando cmd_array ************************************
	cmd_array[0] = JARS_CMD[cmd].val;
	cmd_array[1] = code;//siempre será cero aparentemente
	cmd_array[2] = JARS_CMD[cmd].size;
	//guarda el valor de payload ,del tamaño de size_payload,a partir de cmd_array[3]
	memcpy(cmd_array+3,payload,size_payload);//Completa: val,code,size,direccion
	//**************envia los paquetes de datos del arreglo cmd_array a través del socket
	//printf("Estoy antes de fpga_con_send\n") ;	
	
	fpga_con_send(cmd_array,size_cmd_array);
	printf("Se envió el paquete -->\n") ;
	ack_received = false;
	
	if((JARS_CMD[cmd].val != 63) && (JARS_CMD[cmd].val != 0)){	
	size_rcv_buf = fpga_con_recv(buffer_rcv,RCV_BUFSIZE);//pone el valor en bytes del buffer receptor 1472	
	printf("La cantidad de bytes recibidos son: %d \n", size_rcv_buf );	
	printf("El comando recibido dentro de la interfaz es: ");
	if(size_rcv_buf>100) size_rcv_buf = 18; // se trunca en 18 bytes en caso hayan muchos datos recibidos
	 for(uint16_t i = 0 ; i<size_rcv_buf ; i++)//numero de bytes leidos en el buffer
	   {
	     printf("%02X ",(uint8_t)buffer_rcv[i]);//Se lee el char que llega en formato hexadecimal
	   }
	printf("\n");
        }
	
	//crea el buffer de adquisición si el ack es true
	/*if(JARS_CMD[cmd].ack)
	{		
		size_rcv_buf = fpga_con_recv(buffer_rcv,RCV_BUFSIZE);//pone el valor en bytes del buffer receptor 1472
		ack_received = true;
	}*/
	//************************* Da valor al arreglo info_str dependiendo del tipo de configuración ****************************************************
	//******PAsa de char [] a uint8_t
	/*switch (cmd) {
		case SET_SP601_IP:	//Guarda el valor de la IP del SP601 en formato literal string en info_str para mostrar en pantalla
					bytearray_to_IPstr(payload,cfg.SP601_IP);//pasa los números de char a int de la IP  y los guarda en formato ...
					info_str = (uint8_t *)malloc(strlen(cfg.SP601_IP)+1);
					memcpy(info_str,cfg.SP601_IP,strlen(cfg.SP601_IP)+1);
					printf("Estoy dentro del case SET_SP601_IP y el valor de info_str es: %s \n",info_str);
					break;
		case SET_SP601_PORT:	//Guarda el valor del puerto en info_str (char)
					bytearray_to_int(payload,cfg.SP601_PORT);
					info_str = (uint8_t *)malloc(10);
					itoa(cfg.SP601_PORT,(char *)info_str,10);
					break;
		case SET_COMMS_IP:	//Guarda el valor de la IP de la PC en formato literal string en info_str para mostrar en pantalla
					bytearray_to_IPstr(payload,cfg.IP_host);
					info_str = (uint8_t *)malloc(strlen(cfg.IP_host)+1);
					memcpy(info_str,cfg.IP_host,strlen(cfg.IP_host)+1);
					break;
		case SET_COMMS_PORT:	//Guarda el valor del puerto en info_str (char)
					bytearray_to_int(payload,cfg.PORT_host);
					//printf("%i\n", cfg.COMMS_PORT);
					info_str = (uint8_t *)malloc(10);
					//guarda el valor del puerto en info_str (char)
					//Lo cambia de int a char para mostrarlo como string
					itoa(cfg.PORT_host,(char *)info_str,10);
					break;
		case SET_COMMS_MAC:	//****Guarda el valor de la MAC de la PC en formato literal string en info_str para mostrar en pantalla
					bytearray_to_MACstr(payload,cfg.MAC_host);
					info_str = (uint8_t *)malloc(strlen(cfg.MAC_host)+1);
					memcpy(info_str,cfg.MAC_host,strlen(cfg.MAC_host)+1);
					break;
		case SEND_COMMAND:
					//******************* Creará un arreglo(info_str) donde se guardará el estado del 						JARS:RST_CTRL,RST_CTRL,SELECT_RXD,POWER_RXD,PROG_RXD,ENABLE_TRANSF,DISABLE_TRANSF************
					//********** Recibirá el primer paquete de datos mediante recv()*****************************
					//-----------Verifica mediante el payload[1] que se haya enviado el modo correcto de configuración------------------
					info_str = (uint8_t *)malloc(10);
					memset(info_str,0,10);//define el espacio de memoria info_str con 10 ceros.
					//Crea un indicador que define el arreglo de estados info_str
					char cmd_acq;
					cmd_acq = (payload[1]>>4)&15;//parece que el payload guarda el comando de estado
					if(cmd_acq==RST_CTRL) sprintf((char *)(info_str), "%s", "RST_CTRL");
					//else if(cmd_acq==MODE_OPER) sprintf((char *)(info_str), "%s", "RST_CTRL");
					else if(cmd_acq==MODE_OPER) sprintf((char *)(info_str), "%s", "MODE_OPER");
					else if(cmd_acq==ELECTION_RXD) sprintf((char *)(info_str), "%s", "SELECT_RXD");
					else if(cmd_acq==POWER_RXD) sprintf((char *)(info_str), "%s", "POWER_RXD");
					else if(cmd_acq==PROG_RXD) sprintf((char *)(info_str), "%s", "PROG_RXD");
					else if(cmd_acq==ENABLE_TRANSF) sprintf((char *)(info_str), "%s", "ENABLE_TRANSF");
					else if(cmd_acq==DISABLE_TRANSF) sprintf((char *)(info_str), "%s", "DISABLE_TRANSF");
					// Get the value returned from the UDP receiver thread
					// First packet
					size_rcv_buf = fpga_con_recv(buffer_rcv,RCV_BUFSIZE);//Recibe el paquete en el buffer de 1472 bytes de tamaño
					// Revisar los respectivos bits de buffer_rcv para colocar el info_str el comando que fue enviado
					break;
		case DATA_LOOPS:
					info_str = (uint8_t *)malloc(10);
					cfg.VERBOSE_MODE = false;
					break;
		case GET_VALUE:
					info_str = (uint8_t *)malloc(10);
					break;
		case SEND_FILTER:
					//Recibe el paquete de respuesta de envio del sp601
					info_str = (uint8_t *)malloc(12);
					memset(info_str,0,12);//establece el arreglo info_str con 12 ceros, pero tipo char
					sprintf((char *)(info_str), "%s", "WRITE_FILTER");
					//Get the value returned from the UDP receiver thread
					//First packet.
					size_rcv_buf = fpga_con_recv(buffer_rcv,RCV_BUFSIZE);
					printf("Filter received\n");
					break;
		case NOOP:		//No muestra nada
					info_str = (uint8_t *)malloc(10);
					cfg.VERBOSE_MODE = false;
					break;
		default:
					printf("Error: Command Not Found[%s]\n", JARS_CMD[cmd].name);
					break;
	}*/

	//printf("Estoy después de fpga_con_recv\n");
	//**************** Imprime el nombre del comando enviado y el estado del JARS y se lee el paquete recibido en formato hexadecimal
//	if((cfg.VERBOSE_MODE == true) && (cmd != GET_VALUE))
//	{
	 /*	printf("%s : %s\n", JARS_CMD[cmd].name,(uint8_t *)info_str);//muestra el name y el info_str en uint8_t(uint8_t* para asegurar)
		printf("Received: ");
		for(uint16_t i = 0 ; i<size_rcv_buf ; i++)//numero de bytes leidos en el buffer
		{
			printf("%02X,",(uint8_t)buffer_rcv[i]);//Se lee el char que llega en formato hexadecimal
		}
   		printf("\n");*/
		//size_t size_info_str = sizeof(info_str);
		//memset(info_str,0,size_info_str);

//	}

	//**************** Muestra todos los elementos del buffer: cmd_array en hexadecimal *********************
	/*if(JARS_CMD[cmd].ack && ack_received)
	{	if(cfg.VERBOSE_MODE)
		{
			printf("[OK] Validation for command [%s][%i][",JARS_CMD[cmd].name,code);//code sera siempre cero
			for(uint16_t i = 0 ; i<(size_cmd_array-1) ; i++)
			{
				printf("%02X,",(uint8_t)cmd_array[i]);
			}
			printf("%02X]",(uint8_t)cmd_array[size_cmd_array-1]);
			printf("\n");
		}
	}*/
	//printf("-------> Estoy al final de send <-------\n");
}

