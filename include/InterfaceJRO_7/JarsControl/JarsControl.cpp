
#include "JarsControl.h"
#include <string.h>
#include <zmq.hpp>

_JARSControl  JARSControl;

zmq::context_t context(1);

//******************************************* Define la clase JARSControl *********************************************
_JARSControl::_JARSControl()
{
	m_NumMarks=2;                  //2 elementos de la cabecera por eliminar
	m_MarkRe=-21846;	       //marca para comprobar la cabecera del JARS 2.0
	m_SizeofBuffer=0;              // se igualará a m_iSizeNIBuffer
	m_AcqNIBuffer=NULL;            //Doble Buffer
	m_AcqNIDoubleBuffer=NULL;      //Doble Buffer
	m_AcqBuffer=NULL;              //Buffer de adquisición
	m_nChannelsMask=0;             //Define los canales a energizar y adquirir
	m_nCardsMask=0;                //Se envía al JARS pero no definen canales físicos.Se usó para simular canales
	VarSetSystem=0;                //valor tomado para ver el estado de seteado
	MySP601 = new sp601_command(); //Crea un objeto de la clase sp601_command.

}

_JARSControl::~_JARSControl()
{
  /*printf("Terminando el programa en el destructor de JARSControl...\n");
  JARSControl.jro_StopAcquisition();
  sleep(1);
  exit(1);*///reporta el término del programa cuando algún error o interrupción ocurre.
}

//Define los parámetros del experimento (MyExpParam) con el parámetro ExpParam introducido en jro_MatchParameters
//Define MyExpParam para un objeto de la clase _JARSControl. Este objeto se usará para sincronizar la data en tiempo real
//entre threads.

void _JARSControl::jro_MatchParameters(ExpParameters* ExpParam)
{	
	printf("****************** Estoy dentro de jro_MatchParameters*******************************\n");	
	MyExpParam=ExpParam;
	//Usado en ConfigDataTransfer
	JARSControl.MyExpParam=ExpParam; // agregado para q actualice campo MyExpParam de JARSControl
	//printf("****************** Llegué al final de jro_MatchParameters****************************\n");	
	return;
}

//Llamará a los buffers de adquisición, doble buffers , al puntero de la función que comenzará el almacenamiento en disco
//y establecerá el sistema con SetSystem
void _JARSControl::jro_AssignMemory(MemoryManager* MemManager,void (*ptrClientFunc)(bool sucess),address host_address)
{	//Asigna los buffer de adquisición y memoria
	printf("-----------------------------------------------------------------------------------");
	printf("****************** Estoy dentro de jro_AssignMemory*****************************\n");
	m_SizeofBuffer=MemManager->m_iSizeNIBuffer;//tamaño del buffer más grande (sin integraciones)
	m_AcqBuffer=MemManager->m_mAcqBuffer[0];//buffer de adquisición
	m_AcqNIBuffer=MemManager->m_nNIBuffer;//buffer más grande separado en memoria //buffer para threads
	m_AcqNIDoubleBuffer=MemManager->m_nNIDoubleBuffer;//doble buffer más grande separado en memoria//buffer para threads
	ptrFunc=ptrClientFunc;
	if (!SetSystem(host_address))//si es seteado correctamente, regresa true//Se puede llamar a métodos que pertenecen a la misma clase dentro de otros métodos
	   VarSetSystem=1;
	usleep(100000); //microsegundos
	return;
}

//****** Define el socket y establece las direcciones de puertos ********************************
//****** Establece las máscaras (tarjeta y canales) y el filtro***************************************************************
//****** Envía los comandos de configuración del JARS:RESET,MODE_OPER,SELECT_RXD,PW_RXD,PROG_RXD,WRT_FILT*******
//**** Nos dice si el seteado del sistema se realizó correctamente devolviendo un true ***********
bool _JARSControl::SetSystem(address host_address)
{
	printf("-----------------------------------------------------------------------------------");
	printf("****************** Estoy dentro de SetSystem*****************************\n");
	//-------------- Define el socket y Establece las direcciones de puertos -------------------------------
	//-------------- Definiremos sockets UDP de GNU Radio --------------------------------------------------
	MySP601->init(host_address);// Inicializar parametros para comunicacion con tarjeta SP601 (tarjeta de bus)(nvia los puetos de la Pc y la SP601 a la SP601)
	//---------Detecta si las máscaras utilizadas por el objeto JARSControl fueron definidas----------------------------
	//---------Define parte del paquete (máscara) que será enviado para encender y seleccionar los canales de adquisición.
	if(!JARSControl.LoadMask()) // Lee la máscara de programacion del *.racp
		return false;
	usleep(50000);//suspende la ejecución por 50 seg.
	//----------Utiliza el .jars para establecer las direcciones del filtro---------------------------------------------
	if(!JARSControl.LoadFilter()) // Carga el archivo del filtro y define los valores para el AD6620
	    return false;
	usleep(50000);
	//----------- Envía un comando para resetear el JARS ------------------------------------------
	if(!JARSControl.RESET_CTRL_COMMAND()) // Resetea el sistema, reinicia todos los FPGA's
		return false;
	//---------- if GENERATE OWN SAMPLING WINDOW=YES del .racp----------------------------------
	printf("*********Estoy antes de OwnWindow\n");	
	if(MyExpParam->OwnWindow)
	////----------  Envía el comando MODE_OPER al JARS ---------------------------------------------
	  if(!JARSControl.MODE_OPER_COMMAND())
	    return false;
	usleep(50000);
	printf("*********Estoy después de OwnWindow\n");	
	//------------- Envía el comando ELECTION_RXD al JARS -----------------------------------------
	/*if(!JARSControl.SELECT_RXD_COMMAND())//selecciona las tarjetas de adquisición//usado para simular , no procesado
		return false;
	usleep(50000);*/
	//------------- Envía el comando POWER_RXD al JARS -----------------------------------------
	//Enciende las tarjetas de adquisición y envia las máscaras de los canales
	if(!JARSControl.PW_RXD_COMMAND()) // Alimenta los receptores digitales y envia la mascara de los canales seleccionados
		return false;
	usleep(50000);
	//------------- Envía el comando PROG_RXD al JARS -----------------------------------------
	if(!JARSControl.PROG_RXD_COMMAND()) // Envía el comando que permitirá programar el DSP AD6620
		return false;
	usleep(50000);
	//------------- Relacionado con filter -----------------------------------------
	if(!JARSControl.WriteFilter()) // Envía a través del puerto de datos los coeficientes del filtro
	     return false;
	usleep(50000);
	return true;
}

//******* Define las máscaras de las tarjetas conectadas y los canales seleccionados ********************************
//******* Define parte del paquete que será enviado para encender y seleccionar los canales de adquisición.
//JARSControl.m_nChannelsMask: define los canales a energizar y adquirir
//JARSControl.m_nCardsMask: se envía al JARS pero no definen canales físicos.Se usó para simular canales
bool _JARSControl::LoadMask()
{	
	printf("********************** Estoy dentro de LoadMask******************************* \n");
	//----------------(Carga de máscara de tarjetas conectadas) Para simular   --------------------------------
	//****************m_nCardsMask= 11 elementos : ceros = (# tarjetas-1), unos = 11- # ceros *******************
	for(uint8_t i=0; i<MyExpParam->MyGPParam.m_nCards; i++)//uint8_t = unsigned char
	{
		if (MyExpParam->MyGPParam.Dyn_nCardSequence[i]==0)
		{	continue;}
		else//Dyn_nCardSequence = 8bits. Corren un bit a la izquierda <<
			//Adquiere el valor de la máscara de cada tarjeta (m_nCardsMask inicializado en 0 en el constructor)
			JARSControl.m_nCardsMask=JARSControl.m_nCardsMask | (1<<(MyExpParam->MyGPParam.Dyn_nCardSequence[i]-1));// | es un bitwise OR
	}
	JARSControl.m_nCardsMask^=1023; //^ es un XOR .Se realiza un XOR entre: m_nCardsMask y 255 y nos da m_nCardsMask
	JARSControl.m_nCardsMask=JARSControl.m_nCardsMask | 1024;
	printf("Cards mask was loaded\n");
	//------------------ Carga de mascara de canales seleccionados --------------------------------
	//******************m_nChannelsMask= 11 elementos : ceros = # canales, unos = 11- # canales *******************
	for(int i=(MyExpParam->MySystemParam.m_nChannels-1); i>=0; i--)
	{
		if (MyExpParam->MySystemParam.m_nNumberOfChannel[i]==0)
		{	continue;}
		else
			JARSControl.m_nChannelsMask=JARSControl.m_nChannelsMask | (1<<(MyExpParam->MySystemParam.m_nNumberOfChannel[i]-1));
	}
	JARSControl.m_nChannelsMask^=1023;
	JARSControl.m_nChannelsMask=JARSControl.m_nChannelsMask | 1024;
	printf("Channels mask was loaded\n");
	return true;
}

//Lee el archivo .jars que contiene la configuracion de los filtros del DSP.
//*********Define el arreglo m_nFilterCoeff donde sus elementos son valores hexadecimales que definen los registros del DSP y establecen los parámetros del filtros (decimació)
//Solo define , no  envía.
//Pd: Exsite un protocolo entre el FPGA de Programacion y el DSP antes de establecer los valores
bool _JARSControl::LoadFilter()
{
	printf("********************** Estoy dentro de LoadFilter **********************\n");	
	int Adress300=0x300, Adress0=0x0, AdressTemp;
	int16_t CoeffTemp;

	FILE* MyRacpFile;
	char StrTemp[100];
	char* StrPtr;
	
	printf("%s\n", MyExpParam->MySystemParam.m_sFilterPath);
	printf("El valor de strlen del filtro es: %d\n", strlen(MyExpParam->MySystemParam.m_sFilterPath));
	//printf("El valor de sizeof del filtro es: %d", sizeof(MyExpParam->MySystemParam.m_sFilterPath));
	MyRacpFile=fopen(MyExpParam->MySystemParam.m_sFilterPath,"r");//abre la direccion solo lectura
//	MyRacpFile=fopen("/home/itupac/RACP/F1MHZ_8_MATCH.jars","r");//abre la direccion solo lectura	
		
	if((MyRacpFile)==0)
	{
		printf("No filtro ingresado, tomando filtro por defecto\n");
	}

	for(;;)
	{	if(feof(MyRacpFile))//si llega al final
		{	fclose(MyRacpFile);
			break;
		}
		fgets(StrTemp,100,MyRacpFile);//lee las 100 primeras variables de MyRacpFile , incluyendo el null-character
		if (StrTemp[0]=='3' || StrTemp[0]==' ')
		{	if (StrTemp[0]=='3')
				AdressTemp=Adress300++; // primer  bloque de direcciones 300h -> 30Dh

			if (StrTemp[0]==' ')
				AdressTemp=Adress0++;// segundo bloque de direcciones 000h -> ?
			JARSControl.m_nFilterCoeff[JARSControl.m_nCountCoeff++]=(((AdressTemp>>8) & 0x000000FF ) | 0x00000700);
			JARSControl.m_nFilterCoeff[JARSControl.m_nCountCoeff++]= ((AdressTemp & 0x000000FF) | 0x00000600);

			StrPtr=strstr(StrTemp, "->");
			StrPtr+=2;
			if (AdressTemp==0x302)
			{	for(int j=0; j<4; j++)
				JARSControl.m_nFilterCoeff[JARSControl.m_nCountCoeff++]=(0xFF | (3-j)<<8);
			}
			else
			{
				for(int j=0;j<4;j++)
				{	if ((((CoeffTemp=(atoi(StrPtr)>>((3-j)<<3))& 0x00FF)==0) & (j!=3)) && AdressTemp>=0x300)
						continue;
					else
					{	JARSControl.m_nFilterCoeff[JARSControl.m_nCountCoeff]=(CoeffTemp |(3-j)<<8);
						JARSControl.m_nCountCoeff++;}
				}
			}
		}

		if (StrTemp[0]=='R') // Se terminan de leer los coefficientes, creo los ultimos caracteres finales
		{	JARSControl.m_nFilterCoeff[JARSControl.m_nCountCoeff++]= 0x03 | 0x00000700;
			JARSControl.m_nFilterCoeff[JARSControl.m_nCountCoeff++]= 0x00 | 0x00000600; // 300h
			JARSControl.m_nFilterCoeff[JARSControl.m_nCountCoeff++]= 0x01 | 0x00000000; // 0x01
			JARSControl.m_nFilterCoeff[JARSControl.m_nCountCoeff++]= 0x03 | 0x00000700;
			JARSControl.m_nFilterCoeff[JARSControl.m_nCountCoeff++]= 0x00 | 0x00000600; // 301h
			JARSControl.m_nFilterCoeff[JARSControl.m_nCountCoeff++]= 0x00 | 0x00000000; // 0x00
			fclose(MyRacpFile);
			printf("Filter was loaded\n");
			return true;
		}
	}
  return true;
}

//--------- Define el payload de RST_CTRL para ser enviado en formato de bytes y lo envía----------
//********* Imprime: Nombre de comando,tipo de operación,paquete de dato recibido*******
bool _JARSControl::RESET_CTRL_COMMAND()
{
	printf("********************** Estoy dentro de RESET_CTRL_COMMAND **********************\n");	
	int cmd_jars=0;//4 bytes
	char payload[16]={};
	size_t size_payload=3;//24 bits
	//unsigned long int size_payload=3;
	printf("Reset JARS\n");
	cmd_jars = SYNCHRO;
	cmd_jars = (cmd_jars<<2)^CHIP_ALL;
	cmd_jars = (cmd_jars<<4)^RST_CTRL;//**define tipo de operación
	cmd_jars = (cmd_jars<<1)^BY_CMD;
	cmd_jars = (cmd_jars<<11)^0x00;
	cmd_jars = (int)cmd_jars;
	//Define el payload como 3 bytes de cmd_jars
	intToBytes(cmd_jars,payload,size_payload);
	//Envía el cmd_array con los bytes configurados para el comando RST_CTRL
	MySP601->send(SEND_COMMAND,0,payload,size_payload);
	//da valor a los elementos del buffer payload igual a cero
	memset(payload,0,strlen(payload));
  return true;
}

//------- Define el payload de MODE_OPER para ser enviado en formato de bytes. Esto hace que JARS 2.0 genere su propio win sin el CR------
//******************* Imprime: Nombre de comando,tipo de operación,paquete de dato recibido*******
bool _JARSControl::MODE_OPER_COMMAND()
{
	printf("********************* Estoy dentro de MODE_OPER_COMMAND ************************\n");
	int cmd_jars=0;
	char payload[16]={};
	size_t size_payload=3;
	printf("Operation Mode JARS\n");
	cmd_jars = SYNCHRO;
	cmd_jars = (cmd_jars<<2)^CHIP_ALL;
	cmd_jars = (cmd_jars<<4)^MODE_OPER;//**define tipo de operación
	cmd_jars = (cmd_jars<<1)^BY_CMD;
	cmd_jars = (cmd_jars<<11)^0x00;
	cmd_jars = (int)cmd_jars;
	//Define el payload como 3 bytes de cmd_jars
	intToBytes(cmd_jars,payload,size_payload);
	//Envía el cmd_array con los bytes configurados para el comando MODE_OPER.Imprime los arreglos seleccionados
	MySP601->send(SEND_COMMAND,0,payload,size_payload);
	//da valor a los elementos del buffer payload igual a cero
	memset(payload,0,strlen(payload));
  return true;
}

//------------------- Define el payload de ELECTION_RXD para ser enviado en formato de bytes// Simulacion de canales------
//******************* Imprime: Nombre de comando,tipo de operación,paquete de dato recibido*******
bool _JARSControl::SELECT_RXD_COMMAND()
{
	printf("********************* Estoy dentro de SELECT_RXD_COMMAND ************************\n");	
	int cmd_jars=0;
	char payload[16]={};
	size_t size_payload=3;
	printf("Select receivers\n");
	cmd_jars = SYNCHRO;
	cmd_jars = (cmd_jars<<2)^CHIP_ALL;
	cmd_jars = (cmd_jars<<4)^ELECTION_RXD;//**define tipo de operación
	cmd_jars = (cmd_jars<<1)^BY_CMD;
	cmd_jars = (cmd_jars<<11)^JARSControl.m_nCardsMask;
	cmd_jars = (int)cmd_jars;
	//Define el payload como 3 bytes de cmd_jars
	intToBytes(cmd_jars,payload,size_payload);
	//Envía el cmd_array con los bytes configurados para el comando ELECTION_RXD.Imprime los arreglos seleccionados
	MySP601->send(SEND_COMMAND,0,payload,size_payload);
	//da valor a los elementos del buffer payload igual a cero
	memset(payload,0,strlen(payload));
  return true;
}

//------------------- Define el payload de POWER_RXD para ser enviado en formato de bytes --------
//******************* Imprime: Nombre de comando,tipo de operación,paquete de dato recibido ******
bool _JARSControl::PW_RXD_COMMAND()
{
	printf("********************* Estoy dentro de PW_RXD_COMMAND ************************\n");	
	int	cmd_jars=0;
	char payload[16]={};
	size_t size_payload=3;
	printf("Power the selected receivers\n");
	cmd_jars = SYNCHRO;
	cmd_jars = (cmd_jars<<2)^CHIP_COMAND;
	cmd_jars = (cmd_jars<<4)^POWER_RXD;//**define tipo de operación
	cmd_jars = (cmd_jars<<1)^BY_CMD;
	cmd_jars = (cmd_jars<<11)^JARSControl.m_nChannelsMask;
	cmd_jars = (int)cmd_jars;
	//Define el payload como 3 bytes de cmd_jars
	intToBytes(cmd_jars,payload,size_payload);
	//Envía el cmd_array con los bytes configurados para el comando POWER_RXD.Imprime los arreglos seleccionados
	MySP601->send(SEND_COMMAND,0,payload,size_payload);
	//da valor a los elementos del buffer payload igual a cero
	memset(payload,0,strlen(payload));
  return true;
}

//--------------Define el payload de PROG_RXD para ser enviado en formato de bytes--------------
//**************Imprime: Nombre de comando,tipo de operación,paquete de dato recibido***********
bool _JARSControl::PROG_RXD_COMMAND()
{
	printf("********************* Estoy dentro de PROG_RXD_COMMAND ***********************\n");
	int	cmd_jars=0;
	char payload[16]={};
	size_t size_payload=3;
	printf("Program the selected receivers\n");
	cmd_jars = SYNCHRO;
	cmd_jars = (cmd_jars<<2)^CHIP_COMAND;
	cmd_jars = (cmd_jars<<4)^PROG_RXD;//**define tipo de operación
	cmd_jars = (cmd_jars<<1)^BY_DATA;
	cmd_jars = (cmd_jars<<11)^JARSControl.m_nCountCoeff;
	cmd_jars = (int)cmd_jars;
	//Define el payload como 3 bytes de cmd_jars
	intToBytes(cmd_jars,payload,size_payload);
	//Envía el cmd_array con los bytes configurados para el comando PROG_RXD.Imprime los arreglos seleccionados
    MySP601->send(SEND_COMMAND,0,payload,size_payload);
    //da valor a los elementos del buffer payload igual a cero
	memset(payload,0,strlen(payload));
  return true;
}

//***************** Envía los coeficientes de configuracion establecidos en el LoadFilter al SP601 *******************
//***************** Recibe el buffer_rcv de respuesta y lo muestra en formato hexadecimal ****************************
bool _JARSControl::WriteFilter()
{
    printf("********************* Estoy dentro de WriteFilter ***********************\n");	
    char vector_coeff[256];
    char payload[4]={};
    size_t size_vector_coeff=2*(JARSControl.m_nCountCoeff+1);//envia en char, considera en peso el '0'
    // Write Filter
    printf("Write Filter\n");
    printf("%lu coefficients\n",size_vector_coeff);
    vector_coeff[0]=0; vector_coeff[1]=0;
    //printf("%i,%i\n",vector_coeff[0],vector_coeff[1]);
    //***Define el vector_coeff el cual será enviado como payload en send***
    printf("Estoy antes del for....\n");
    for(unsigned int i=0;i<JARSControl.m_nCountCoeff;i++){
        intToBytes(JARSControl.m_nFilterCoeff[i],payload,2);//los coeficientes los pasa a bytes (2 partes)
        vector_coeff[2*i+2]=payload[0];//guarda la primera parte
        vector_coeff[2*i+3]=payload[1];//guarda la segunda parte
    }
    printf("Estoy después del for....\n");	
    MySP601->send(SEND_FILTER,0,vector_coeff,size_vector_coeff);//ENVIA EN BYTES
  return true;
}

//** Verifica si se habilitó la transferencia de comandos y el configDataTransfer************
bool _JARSControl::jro_InitAcquisition()
{

  printf("************** Estoy dentro de jro_InitAcquisition() *******************\n");
  if(VarSetSystem)//VarSetSystem=0 está bien
    return false;
  //Habilitado la transferencia de datos , nos da true al final
  if(!JARSControl.HAB_TRANS_COMMAND())  // Activa los receptores digitales
    return false;
  // Inicializa la adquisición, la generación de threads zmq y define el parámetro del puntero de función
  if(!JARSControl.ConfigDataTransfer())
  	return false;
  usleep(50000);
  return true;
}

//----- Define el payload de ENABLE_TRANSF q habilita la tranf de comandos y lo envía al SP601-----------
//***** Imprime: Nombre de comando,tipo de operación,paquete de dato recibido*************
bool _JARSControl::HAB_TRANS_COMMAND()
{
  printf("************** Estoy dentro de HAB_TRANS_COMMAND() *******************\n");	
  int  cmd_jars=0;
  char payload[16]={};
  size_t size_payload=3;
  printf("Habilitando la transferencia de datos...\n");
  cmd_jars = SYNCHRO;
  cmd_jars = (cmd_jars<<2)^CHIP_ALL;
  cmd_jars = (cmd_jars<<4)^ENABLE_TRANSF;//**define tipo de operación
  cmd_jars = (cmd_jars<<1)^BY_CMD;
  cmd_jars = (cmd_jars<<11)^0x00;
  cmd_jars = (int)cmd_jars;
  //Define el payload como 3 bytes de cmd_jars
  intToBytes(cmd_jars,payload,size_payload);
  //Envía el cmd_array con los bytes configurados para el comando ENABLE_TRANSF.Imprime los arreglos seleccionados
  MySP601->send(SEND_COMMAND,0,payload,size_payload);
  return true;
}

//**************   Método que inicializa la adquisición, la generación de threads zmq y el guardado de la data en disco
//**************   La única forma de salir de los bucles de los threads es con una interrupción externa Cnt+c
bool _JARSControl::ConfigDataTransfer()
{	
	printf("<-------------------------------------------------------------->\n");	
	printf("************* Estoy dentro de ConfigDataTransfer() *************\n");
		
	printf("************* Comienza la creación y recepción por Threads ***************\n");
	//************************Se crean threads paralelos en el proceso JARSControl que realizan las tareas de Internalx***********************
	//Ejecuta la parte del socket para la recepcion de paquetes
	if(pthread_create(&JARSControl._thread_rcv, NULL, JARSControl.InternalRcvFunc, this)==0)
		printf("Now the packets receiver thread starts\n");
	if(pthread_create(&JARSControl._thread_jars, NULL, JARSControl.InternalThreadEntryFunc, this)==0)
		printf("Now the block constructor thread starts\n");
	if(pthread_create(&JARSControl._thread_jars_rcv, NULL, JARSControl.InternalCallbackFunc, this)==0)//define el puntero
	        printf("Now the block save starts\n");

	pthread_join(JARSControl._thread_rcv,NULL);
	pthread_join(JARSControl._thread_jars,NULL);
	pthread_join(JARSControl._thread_jars_rcv,NULL);
       
	return true;
	  
}

//this pointer: puntero al OBJETO del método que lo llama
//**************this = Objeto de InternalRcvFunc**************************
//Ejecuta el método InternalRcv: Crea el socket zmq que almacena los paquetes de datos adquiridos,por marca,y los envía
void *_JARSControl::InternalRcvFunc(void * This) {
	((_JARSControl *)This)->InternalRcv(); return NULL;//This es el puntero del objeto de la clase JARSControl
}

//******** Crea un soquet ZMQ_PUSH, recibe bytes del socket fpga y los envía a través del socket ZMQ al cliente definido en InternalThreadEntry ******************
//zmq solo almacena la data adquirida del buffer de la tarjeta de red para que no se pierda en un espacio de memoria de la RAM
//********************* Comprueba que la marca del bloque es la correcta verificando así que el bloque de llegada es el correcto *****************

//Crea el socket zmq que almacena los paquetes de datos adquiridos, por marca, y los envía

void _JARSControl::InternalRcv(){
	
	printf("****************************** Estoy dentro de InternalRcv(): Primer Thread *************************************\n");
	
	float IPP,DH;
	unsigned int NTX, Nchannels, Nsamples_by_channel;
	unsigned int Nsamples;
	unsigned int Nprofilesperblock;
	unsigned int IPPu;

	char payload[4];
	//************ Establece parámetros de adquisición del radar para ser enviados **************************************************
	IPP=(float)(JARSControl.MyExpParam->MyGRCParam.FixRCP.m_fIPP);//valor de la Ipp del .racp
	NTX = (unsigned int)(JARSControl.MyExpParam->MyGRCParam.FixRCP.m_nNTX);//numero de ipp por synchr
	DH = (float)(JARSControl.MyExpParam->MyGRCParam.Dyn_sfAcqDH[0]);//resolución de altura
	IPPu = ceil(IPP/DH);//redondea para arriba //cantidad de DHs por Ipp
	Nchannels = (unsigned int)(JARSControl.MyExpParam->MySystemParam.m_nChannels);
	Nsamples_by_channel=(unsigned int)(JARSControl.MyExpParam->MyGRCParam.Dyn_snAcqNSA[0]);// número de muestras por canal para un perfil
	Nsamples = Nchannels*Nsamples_by_channel+1;//numero de muestra en todos los canales para un perfil
	Nprofilesperblock = MyExpParam->MyGPParam.m_nProfilesperBlock;

	if(IPPu>65536){
		printf("Units of IPP exceeded! %i",IPPu);
		exit(1);
	}else printf("IPP(Km/units): %.1f, %i\n",IPP,IPPu);

	if(NTX>65536){
		printf("Number of NTX exceeded! %i",NTX);
		exit(1);
	}else printf("Number of NTX: %i\n",NTX);

	printf("Resolution: %.4f\n",DH);

	if(Nsamples>4294967296) {
		printf("Number of samples per profile exceeded! %i\n",Nsamples);
		exit(1);
	}else printf("Number of Samples per profile: %i\n",Nsamples);

	printf("Number of Profiles per block: %i\n",Nprofilesperblock);
	printf("Number of Samples for all profiles: %i\n",Nsamples*Nprofilesperblock);

	//IPP=1000
	//payload[0]=0x03;payload[1]=0x20
	payload[0]=(char)(IPPu>>8); payload[1]=(char)(IPPu);	// Load IPPunits
	//payload[2]= 0x00 ; payload[3]=0x96
	payload[2]=(char)(NTX>>8); payload[3]=(char)(NTX);	// Load NTX
	//payload[4]=0x00 ; payload[5]= 0x00
	payload[4]=(char)(Nsamples>>24); payload[5]=(char)(Nsamples>>16);	// Load SamplesperProfile
	//payload[6]=0x03 ; payload[7]=0xE9
	payload[6]=(char)(Nsamples>>8); payload[7]=(char)(Nsamples);
	MySP601->send(DATA_LOOPS,0,payload,8); // 2 bytes for IPPunits, 2 bytes for NTX, 4 bytes for SamplesperProfile
	
	char buffer_rcv[1472];//buffer donde se guardará el mensaje que viene

	// Create server socket
	zmq::socket_t server (context, ZMQ_PUSH);
	server.bind("ipc://5559");//Revisar si está bien declarado y si es el correcto protocolo inproc.
	uint16_t prev_mark = 0, new_mark, dif_mark;

//	ssize_t size_rcv_buf=MySP601->receive(buffer_rcv);//Recive por el socket UDP//paquete de 1472 bytes pero recibe 1206 bytes
	printf("Acquisition threads begin...\n");
	for (;;) {
		//pthread_mutex_lock(&estado_mutex);
		// Recibir paquetes de datos en formato de bytes (0x41,0x42,0xaa,0xaa,0xaa,0xam,dat_0[])
		ssize_t size_rcv_buf=MySP601->receive(buffer_rcv);//Recive por el socket UDP//paquete de 1472 bytes pero recibe 1206 bytes
		//Por cada bloque UDP se genera una marca.Esta marca es de 32bits y el último es el contador.
		//printf("Se recibió dentro del for del primer Thread ...\n");		
		
	        new_mark = (int)(0x0F & buffer_rcv[5]);
		dif_mark = new_mark - prev_mark;
		// Review the correct sequence of the marks, acumulacion de subperfiles
		//(=1), siguiente marca;(=0),UDP en el mismo windows;(=0),recién comienza
		if((dif_mark == 1) || (dif_mark == 65521) || (dif_mark == 0) || (prev_mark==0)){
		}
		else{
			printf("Error..\n");
			printf("Lost packet: %i\n", dif_mark);
			printf("Mark_prev: %x\n", prev_mark);
			printf("Mark_new: %x\n", new_mark);
		}
		prev_mark = new_mark;
		//*****************Inicializa y envía un mensaje por el socket ZMQ *****************************************
		//inicializa un mensaje de size_rcv_buf bytes
		zmq::message_t reply(size_rcv_buf);
		//.data:retrieve pointer to message content
		//Copia el mensaje que llega en reply.data()
		memcpy((void *) reply.data(), buffer_rcv, size_rcv_buf);//void * memset ( void * ptr, int value, size_t num );
		server.send(reply);//envía el mensaje de llegada por el soquet ZMQ
		//printf("Se envio dentro del socket ZMQ hacia el segundo thread... \n");
       		}
}

//**************this = Objeto de InternalThreadEntryFunc**************************
//Ejecuta el método InternalThreadEntry:almacena la cantidad de perfile por bloque*****************************
void *_JARSControl::InternalThreadEntryFunc(void * This) {
	((_JARSControl *)This)->InternalThreadEntry(); return NULL;
}

//*************** Tarea que adquiere los paquetes y convierte a perfiles hasta completar un bloque ************************
//**************Almacena la data por cada sync (NTX) en TempAcqNIDoubleBuffer hasta llegar al total de perfiles por bloque
//**** Al llegar a la cantidad de perfiles por bloque lo guarda en TempAcqNIBuffer y envía "Block received" por el socket.
//**** Nos da el tiempo de demora entre bloques
void _JARSControl::InternalThreadEntry(){

	printf("******************************** Estoy dentro de InternalThreadEntry(): Segundo Thread **********************************\n");	
	uint16_t prev_mark = 0, new_mark, dif_mark;
	long int num_prof=1;
	long int cnt_subprof=0;
	long int ind_buffer=0;

	uint16_t buffer_rcv_2[1472];
	ssize_t size_rcv_buf;
	int size_rcv_buf_2;
	long TmInit,TmFinal;

	// Create client socket
	zmq::socket_t client (context, ZMQ_PULL);
	client.connect("ipc://5559");//Revisar si está bien declarado y si es el correcto protocolo inproc.
	zmq::message_t msg;//inicializa el objeto mensaje msg de zmq

	std::string msg_recv;
	zmq::socket_t server (context, ZMQ_PUSH);
	server.bind("ipc://5557");
	uint16_t* TempAcqNIBuffer=(uint16_t*)JARSControl.m_AcqNIBuffer;//definido en jro_AssignMemory
	uint16_t* TempAcqNIDoubleBuffer=(uint16_t*)JARSControl.m_AcqNIDoubleBuffer;//definido en jro_AssignMemory

	for(;;){
			//printf("Estoy dentro del for del segundo thread...\n");			
			client.recv(&msg);
			msg_recv = string(static_cast<char*>(msg.data()), msg.size());//transforma data de tipo char a string
			size_rcv_buf = msg.size();//tamaño con cabecera
			// Reconstruir datos en formato de 16 bits (Real e Imaginario)
			size_rcv_buf_2 = (size_rcv_buf-2)/2;//le resta dos bytes de la cabecera (0x41 0x42) (/2) ya q lo almacena en 16bits//numero de dieciseis avos
			for(int i=0;i<size_rcv_buf_2;i++){
				buffer_rcv_2[i] = ((0x00FF & msg_recv[2*i+2])<<8) + (0x00FF & msg_recv[2*i+3]);			
			}
			new_mark = (int)(0x0F & msg_recv[5]);
			//printf("El valor de new mark es: %d \n", new_mark);
			dif_mark = new_mark - prev_mark;
			// Review the correct sequence of the marks, acumulacion de subperfiles
			if((dif_mark == 1) || (dif_mark == 65521) || (dif_mark == 0) || (prev_mark==0)){
			   //revisar si hay cambio de marca valida, para reiniciar el contador de subperfiles
			   if(dif_mark==1 || (dif_mark == 65521)){
				if(num_prof == (MyExpParam->MyGPParam.m_nProfilesperBlock/MyExpParam->MyGRCParam.FixRCP.m_nNTX)){//perfiles por bloque por cada ipp
				   //std::string msgToClient("Block compuesto por un perfil de 1470 bytes formado\n");//define msgToCliente
			           char mensaje[52] = "Block compuesto por un perfil de 1470 bytes formado";
				   memcpy((uint16_t *)TempAcqNIBuffer,(uint16_t *)TempAcqNIDoubleBuffer,ind_buffer*2);//copia los datagramas por bloque , con solo 					   la primera cabecera,guardados en TempAcqNIDoubleBuffer a TempAcqNIBuffer//ind_buffer*2 ya que se debe indicar la cantidad de bytes 
                                   //printf(".........--->>>>> Se guardó dentro de TempAcqNIBuffer <<<<<---........\n");
				//   printf("El valor de TempAcqNIBuffer [0] es: %02X y el valor de TempAcqNIBuffer [1] es: %02X\n",TempAcqNIBuffer [0],TempAcqNIBuffer [01]);	
				   zmq::message_t reply(sizeof(mensaje));//crea el mensaje 
				   //memcpy((void *) reply.data(), (msgToClient.c_str()), msgToClient.size());//c_string : string+'\0'//char *str = s; 
			   	   memcpy((void *) reply.data(), mensaje, sizeof(mensaje));
				   server.send(reply);//envía el mensaje 
				   num_prof=1;
				   ind_buffer=0;
				   }
				   else num_prof = num_prof + 1;//recibirá un bloque con dos marcas diferentes
				   cnt_subprof=0;
				}
				//**************Almacenar data recibida en TempAcqNIDobleBuffer con la cabecera aa aa aa am********************
				if(cnt_subprof==0){ // El primer perfil del bloque con cabecera pero sin 0x41 y 0x42
					memcpy((uint16_t *)(TempAcqNIDoubleBuffer+ind_buffer),buffer_rcv_2,size_rcv_buf-2);
					ind_buffer += size_rcv_buf_2;//300 muestras(NSA) + 1 marca
				}
				else{///a partir del SEGUNDO perfil guarda el dato adquirido sin cabecera  (quita toda la cabecera . incluso la marca)
					memcpy((uint16_t *)(TempAcqNIDoubleBuffer+ind_buffer),(uint16_t *)(buffer_rcv_2+2),size_rcv_buf-6);//buffer_rcv_2+2 le sacan los dos shorts de cabecera , size_rcv_buf-6 eliminida los 6 bytes de cabecera en el tamaño
					ind_buffer += size_rcv_buf_2-2;
				}
				cnt_subprof++;//contador de perfiles (de 300 en 300 muestras (NSA))
			}
			else{
				printf("Lost packet: %i\n", dif_mark);
				printf("Mark_prev: %x\n", prev_mark);
				printf("Mark_new: %x\n", new_mark);
			//	JARSControl.DESHAB_TRANS_COMMAND();
				printf("Lost packet:...?¿?¿?");
				exit(1);
				}
			prev_mark = new_mark;     // actualizar la marca anterior
			//printf ("Estoy al final del for del segundo thread...\n");
	}
};

//********** Ejecuta el método InternalCallback mediante el objeto seleccionado por this.
//el cual :Guarda la data adquirida sin marca en m_AcqBuffer[0] y ordena el comienzo del procesamiento y guardado en disco
void *_JARSControl::InternalCallbackFunc(void * This) {
	((_JARSControl *)This)->InternalCallback(); return NULL;
}

//Guarda la data adquirida sin marca en m_AcqBuffer[0] y ordena el comienzo del procesamiento y guardado en disco
void _JARSControl::InternalCallback(){

	printf("********************************** Estoy dentro de InternalCallback(): Tercer Thread ************************\n");
	bool ClientValue=false;
	long TmInit,TmFinal;
	// Create client socket
	zmq::socket_t client (context, ZMQ_PULL);
	client.connect("ipc://5557");
	zmq::message_t msg;
	std::string msg_recv;

	uint16_t* TempAcqNIBuffer=(uint16_t*)JARSControl.m_AcqNIBuffer;//Puntero al buffer grande NI,JARSControl.m_AcqNIBuffer,con la data adquirida previamente por 		el thread InternalThreadEntry
	uint16_t prev_mark;
	//char mensaje[msg.size()];
	for(;;){
		//printf("Estoy dentro del for del tercer Thread...\n");
		client.recv(&msg);//recive el mensaje "Block received"
		//memcpy(mensaje,(char*)msg.data(),msg.size());
                //printf("El mensaje recibido en el tercer Thread por el socket ZeroMQ es:%s\n",mensaje);  		
	        //guarda la marca para su verificación ya que adquiere el thread anterior y guarda en este thread "simultaneamente"
         	prev_mark = TempAcqNIBuffer[1];//TempAcqNIBuffer[1]=0xaa 0xam (En la última version ya no se cambia de orden los bytes)
		printf("El valor de prev_mark del TERCER THREAD es: %02X \n",prev_mark);
       		if(TempAcqNIBuffer[0]==(0xFFFF&JARSControl.m_MarkRe)){//m_MarkRe=-21846//1010101010101010(0xaa 0xaa)
		/*	printf("Estoy dentro del if en el tercer thread..\n");
			//printf("strlen(Block compuesto por un perfil de 1470 bytes formado) es: %d\n",strlen("Block compuesto por un perfil de 1470 bytes formado"));
			//printf("strlen(mensaje) es: %d\n",strlen(mensaje));
			if (strlen("Block compuesto por un perfil de 1470 bytes formado") != strlen(mensaje))
			{
			   printf("Aún no se general el bloque de datos...\n ");
			   ClientValue = false;//no save data (atributo que se define para la función MyCallbackFunction
			   printf(ClientValue ? " El valor de ClientValue es true\n" : "El valor de ClientValue es false\n");	
			}*/			
			ClientValue = true;
			//printf(ClientValue ? " El valor de ClientValue es true\n" : "El valor de ClientValue es false\n");
			JARSControl.RemoveMark(); 
						
			//*******verifica si el bloque adquirido del anterior thread es correcto
			if(TempAcqNIBuffer[1]!=prev_mark){ // Error producido cuando los datos se actualizan al escribir en disco
				printf("Incorrect data:Tercer Thread\n");
	//			JARSControl.DESHAB_TRANS_COMMAND(); //*******FALTA
	//			bool flagError= true;
	//			exit(1);
                       }
										
		}
		else{ 
			ClientValue = false;//no save data (atributo que se define para la función MyCallbackFunction
			//printf(ClientValue ? " El valor de ClientValue es true\n" : "El valor de ClientValue es false\n");
		}			
		//*********ptrFunc: Arreglo de funciones definido en main que inicializa el procesamiento y lo guarda en disco con fwrite
	
		if(JARSControl.ptrFunc!=NULL)
		  JARSControl.ptrFunc(ClientValue);//ClientValue = true da inicio a la función MyCallbackFunction.
		
		//printf ("Estoy al final del for del tercer thread...\n");	
		
	}
	client.close();//se cierra o destruye el socket client para no recibir mas bloques
};

//*********** Remueve la marca y guarda la data adquirida por bloque en *TempAcqBuffer=*m_AcqBuffer[0] sin marca
void _JARSControl::RemoveMark()
{
	complex<short>* TempAcqNIBuffer=(complex<short>*)m_AcqNIBuffer;//buffer separado con marca
	complex<short>* TempAcqBuffer=(complex<short>*)m_AcqBuffer;//buffer separado sin marca
	int Samples=MyExpParam->MySystemParam.m_nSamples;//NSA de .racp
	int Channels=MyExpParam->MySystemParam.m_nChannels;//canales del .racp
	int NTX=MyExpParam->MyGRCParam.FixRCP.m_nNTX;//IPPs por synchr del .racp
	int Profiles=MyExpParam->MySystemParam.m_nProfiles;//ACQUIRED PROFILES del .racp
	int jump=Samples*Channels*NTX;// todo un bloque
	size_t jumpInBytes= jump*4;//1 muestra en formato complex<short> tiene 4 bytes
	//*******************Copia lo que hay en el buffer grande total m_nNIBuffer en en AcqBuffer[0]************
	//************Copia desde la posición 1 de TempAcqNIBuffer , que es de donde comienza la data adquirida****
	for(int i=0;i<Profiles/NTX;i++)//perfiles adquiridos en cada synchro
	  memcpy(&TempAcqBuffer[i*jump],&TempAcqNIBuffer[i*(jumpInBytes+m_NumMarks)+m_NumMarks],jumpInBytes);//m_NumMarks=2
	
	/*printf("Se removió la marca, se guardó en los buffers de memoria\n");
	printf("El valor de TempAcqNIBuffer[0] real es: %d\n",TempAcqNIBuffer[0].real());
	printf("El valor de TempAcqNIBuffer[0] imag es: %d\n",TempAcqNIBuffer[0].imag());
	printf("El valor de TempAcqBuffer[0] real es: %d\n",TempAcqBuffer[0].real());
	printf("El valor de TempAcqBuffer[0] imag es: %d\n",TempAcqBuffer[0].imag());*/

	return;//guardo los perfiles con tamaño jump (ChHP) hasta completar un bloque
}

//verifica si se detuvo la adquisicion mediante el envio del comando de deshabilitacion y reseteo del JARS
//Envia un request de cancelación al thread de origen de bloques
bool _JARSControl::jro_StopAcquisition()
{
	printf(".....Estoy en jro_StopAcquisition.......\n");	
	printf("Terminating acquisition thread...\n");
	pthread_cancel(JARSControl._thread_rcv);
	pthread_cancel(JARSControl._thread_jars);	// Sending cancellation request//cancela el thread por si queda data guardada
	pthread_cancel(JARSControl._thread_jars_rcv);
	sleep(1);
	if(!JARSControl.DESHAB_TRANS_COMMAND())//Envía el comando de deshabilitación
		return false;
	if(!JARSControl.RESET_CTRL_COMMAND()) // Resetea el sistema, reincia todos los FPGA's
		return false;
	// Terminating acquisition thread
	/*printf("Terminating acquisition thread...\n");
	pthread_cancel(JARSControl._thread_jars);	// Sending cancellation request//cancela el thread por si queda data guardada
	sleep(0.5);*/
	return true;
}

//--- Define un payload con comando 0x00 y el payload de DISABLE_TRANSF para ser enviado en formato de bytes-----------
//comando 0x00: crea el arreglo cmd_array.NO crea  info_Str,buffer_rcv
//comando DISABLE_TRANSF: genera el cmd_array,info_str, recive en el buffer_rcv,imprime el nombre de comando, estdo de comando de config y buffer_rcv.NO imprime el cmd_array
//***** Espera la finalizacion de la adquisición y envia el comando de deshabilitación.
bool _JARSControl::DESHAB_TRANS_COMMAND()
{
	printf(".....Estoy dentro de DESHAB_TRANS_COMMAND() .....\n");
	int  cmd_jars=0;
	char payload[16]={};
	size_t size_payload=3;

	// ********close all the threads
	// ********Espera a que el hilo _thread_rcv acabe (termina el hilo _thread_rcv y devuelve NULL a nuestro hilo de trabajo)
	//Al terminar ese thread, continuará esta parte.Los demás threads continuarán
	//pthread_join(_thread_rcv, NULL);

	//************ Previous command to exit JARS from acquisition loop
	//sp601_command('NOOP',0,int_to_bytearray(0,3))
	//************ Crea el payload antes del comando de deshabilitación ***********
	//************ Por qué envía 0x00
	cmd_jars = SYNCHRO;
	cmd_jars = (cmd_jars<<2)^CHIP_ALL;
	cmd_jars = (cmd_jars<<4)^0x00;
	cmd_jars = (cmd_jars<<1)^BY_CMD;
	cmd_jars = (cmd_jars<<11)^0x00;
	cmd_jars = (int)cmd_jars;
	//Define el payload como 3 bytes de cmd_jars
	intToBytes(cmd_jars,payload,size_payload);
	//Envía el cmd_array con los bytes configurados para el comando NOOP
	MySP601->send(NOOP,0,payload,size_payload); // No muestra nada
	usleep(50000);//suspende la ejecución por 0.5 segundos

	//********** Crea el payload oara el comando de deshabilitacion *****************
	printf("Disabling data transfer...\n");
	cmd_jars = SYNCHRO;
	cmd_jars = (cmd_jars<<2)^CHIP_ALL;
	cmd_jars = (cmd_jars<<4)^DISABLE_TRANSF;
	cmd_jars = (cmd_jars<<1)^BY_CMD;
	cmd_jars = (cmd_jars<<11)^0x00;
	cmd_jars = (int)cmd_jars;
	//Define el payload como 3 bytes de cmd_jars
	intToBytes(cmd_jars,payload,size_payload);
	//Envía el cmd_array con los bytes configurados para el comando DISABLE_TRANSF
	MySP601->send(SEND_COMMAND,0,payload,size_payload);

	return true;
}








