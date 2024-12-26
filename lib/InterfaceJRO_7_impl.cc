/* -*- c++ -*- */
/* 
 * Copyright 2020 gr-InterfaceJRO_7 author.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "InterfaceJRO_7_impl.h"

namespace gr {
  namespace InterfaceJRO_7 {

    /*********************************/
    /*FILE REPORT RESTARTING*/
    /*********************************/
    //Por Remover
    bool FlagFunction=false;//Usado en un blucle para que no se core dumb-por remover
    int timmer;//Usado en un blucle para que no se core dumb-por remover.
    #define TimeLimit (10000)

    // FILE *MyReport;
    // char Report[_MAX_PATH];
    // char Report1[_MAX_PATH];
    //struct timeb FullTime;	
    void MyCallbackFunction(bool);
    	
    /*********************************/
    //Define los objetos que usarán los métodos
    ExpParameters MyParameters;//Objeto de experiment parameters con atributos de experiment parameters
    MemoryManager MyMemManager;
    JicaHeader MyHeader;//Objeto de JicaHeader
    RdpToolsBeta MyProcess;//Objeto de RdpToolsBeta

    InterfaceJRO_7::sptr
    InterfaceJRO_7::make(const char* path_file, const std::string mac_block, int port_block, const std::string ip_host)
    {
      return gnuradio::get_initial_sptr
        (new InterfaceJRO_7_impl(path_file, mac_block, port_block, ip_host));
    }

    /*
     * The private constructor
     */
    InterfaceJRO_7_impl::InterfaceJRO_7_impl(const char* path_file, const std::string mac_block, int port_block, const std::string ip_host)
      : gr::block("InterfaceJRO_7",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(1, 1, sizeof(float))),
    c_path_file(path_file),c_mac_block(mac_block),c_port_block(port_block),c_ip_host(ip_host)
    {}

    /*
     * Our virtual destructor.
     */
    InterfaceJRO_7_impl::~InterfaceJRO_7_impl()
    {
       //JARSControl.~_JARSControl();      
      //printf("...Terminating program en el destructor de InterfaceJRO_7_impl...\n");
      //JARSControl.jro_StopAcquisition(); 
			//sleep(1);
      //exit(1);//termina el proceso
    }

    void
    InterfaceJRO_7_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

		void terminate (int param)
		{
			JARSControl.jro_StopAcquisition(); 
			printf("Terminating program...\n");
			sleep(1);
			exit(1);
		}

    int
    InterfaceJRO_7_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
     // float *out = (float *) output_items[0];
       
      // Do <+signal processing+>
      //******************** Define un objeto de la estructura address *********************************      	
	address host;
        host.d_path_file = c_path_file;
	host.d_mac_block = c_mac_block;
	host.d_port_block = c_port_block;
	host.d_ip_host =  c_ip_host;	
	///////////////////////////////////////////////////////////////////////////////////////////////////

	//Inicializa la IP estática y la MAC

	//****** Estable el reconocimiento de la MAC y la IP de la SP601 para la comunicación directa**arp(protocolo de resolución de direcciones***
	system("sudo arp -s 10.10.30.123 00:0a:35:02:05:f2");//inicializa la IP estática y la mac de la SP601 en la PC para que pueda reconocerla.
	//**** Maximiza el tamaño del buffer de recepción o lectura del socket para minimizar los paquetes de datos perdidos
	system("sudo sysctl -w net.core.rmem_max=26214400");//máxima capacidad del buffer de la tarjeta de red para una mejor performance cuando se trabaje en 		gigabit ethernet
	printf ("-------------->> Inicio de la Interfaz Final 04/11/2020 <<-------------\n");
        //********************* Prueba lectura y reconocimieto de ExperimentParameters ******************************************************
	printf("****************Prueba lectura y reconocimieto de ExperimentParameters*******************\n");	
	printf("El valor de ADC inicializado es: %d\n",MyParameters.MySystemParam.m_nADCResolution);
        printf("El valor de m_nCards sin leer el RACP es: %d\n",MyParameters.MyGPParam.m_nCards);
	//printf("-----------------------------------------------------------------------------------------------------------------------\n");	
	
	//**********Define el objeto ArgProcess que establece el objeto MyParameters de ExpParameters para la lectura del .racp
	ArgProcessor ArgProcess(MyParameters);
	ArgProcess.jro_arguments(host.d_path_file);//ejecuta el método jro_arguments (lectura del .racp)

	signal(SIGINT,&terminate); // Se ejecuta terminate cuando hay Ctrl+C
	signal(SIGTERM,&terminate); // Se ejecuta terminate cuando se presiona Kill flowgraph

	//printf("El valor de ADC leido del RACP es: %d\n",MyParameters.MySystemParam.m_nADCResolution);
        //printf("El valor de m_nCards leido del RACP es: %d\n",MyParameters.MyGPParam.m_nCards);
	
	//***************Ingresa parametros y memoria a RdpToolsBeta
	//Ejecuta el método GetExtParameters , el cual ingresa los atributos leídos del .racp mediante un puntero al objeto MyParameters
	//y los atributos declarados de memory manager mediante un puntero al objeto MyMemManager.
	//Ejecuta initVariables():Da valor a los parámetros de ventana (H0,NSA,DH) y define Dyn_snCode
	//*****Utiliza los objetos de ExperimentParameters y MemoryManager para establecer sus atributos y métodos en el objeto de RdpToolsBeta
	MyProcess.GetExtParameters(&MyParameters,&MyMemManager);//-------------------------------------->>>>> (Check)

	//******Usando el objeto MyMemManager y el objetoMyParameters: crea todos los buffers necesarios para iniciar la adquisicion
	//Crea los arreglos de adquisicion m_mAcqBuffer en formato complex<short> separados en memoria donde se guardará la data adquirida
	//Separa en memoria el espacio donde se almacenará las muestras de adquisición: m_nNIBuffer
	if(!MyMemManager.AllocateMemory(&MyParameters))//---------------------------------<<(check)
	{
		printf("\nError allocating memory\n");
		return -1;
	}
	
	printf("****************Prueba lectura y reconocimiento de MemoryManager*************************\n");
	printf("El valor de m_iSizeNIBuffer es: %d\n",MyMemManager.m_iSizeNIBuffer);	
	printf("El valor de numrawbuffs es: %d\n",MyMemManager.numrawbuffs);	
	printf("-----------------------------------------------------------------------------------------------------------------------\n");   
		 
	//**Define en el objeto de JicaHeader los atributos del experimento de longHeader
	//ingresa los parametros para crear las cabeceras del formato Jicamarca.
	//Separa en memoria y establece los atributos de las cabecera long header
	
	MyHeader.GetExpParameters(&MyParameters);//-----------------------<<<<<<<<<< (check)

	//***Dependiendo si pAcquisitionManager apunta a un objeto tipo JARSControl o cooperball ejecuta el método jro_MatchParameters
	//JARSControl: Da los valores de los atributos de ExperimentParameters al obejto de JARSControl?¿
	JARSControl.jro_MatchParameters(&MyParameters);//-------------------------<<<<<(check)

	//***Dependiendo si pAcquisitionManager apunta a un objeto tipo JARSControl o cooperball:
	//JARSControl:Define los buffers de memoria de MemoryManager en el objeto JARSControl (RemoveMark),define el puntero ptrClientFunc igual al address 		MyCallbackFunction y setea el JARS por comandos
	JARSControl.jro_AssignMemory(&MyMemManager,&MyCallbackFunction,host);

	//Compara si la hora actual está dentro del rango de inicio y fin
	//Si tiene que empezar en una hora estipulada, estará en el while hasta que comienze
	while(!MyHeader.CompareTime())//empieza a la hora estipulada
		sleep(1);
	
	//inicia adquisicion
	//JARSControl:Habilita la transferencia de datos e inicializa la adquisición, la generación de threads zmq y define el parámetro del puntero de función 	(true) asignada en AssignMemory que guarda en disco
	if(!JARSControl.jro_InitAcquisition())
	 printf("Problemas en el inicio de la adquisición. Revisar jro_InitAcquisition() !!!\n");

	// ************* Ver si va un bucle for para que no salga ***********************	
	//for (int i = 0 ; i<noutput_items;i++)	 	      
	 // out[i]=2;
        // Tell runtime system how many input items we consumed on
        // each input stream.
	
	//printf(" *************xxxxxx Estoy antes de pthread_join en el main xxxxxxxx**********\n ");
	//pthread_join(JARSControl._thread_rcv,NULL);
	//pthread_join(JARSControl._thread_jars,NULL);
	//pthread_join(JARSControl._thread_jars_rcv,NULL);
	//printf(" ************* Estoy después de pthread_join en el main **********\n ");
       	
	/*Se deja esta parte del codigo para monitorear si hay perdida de comunicacion con JARS*/
	try
	{
	   FlagFunction = true;
	   while(FlagFunction==true)
		{	sleep(1);
			timmer+=1;
			if(FlagFunction)
			if (timmer==TimeLimit)
			{
				timmer=0;
				printf("Trying restart!\n");

			}
		}
	}
	catch(std::exception& excep)//¿Qué excepción tomará?// Parece que si ocurriese alguna excepción no especificada lo pone en pantalla
	{
		std::cout<<std::endl<<std::endl<<excep.what()<<std::endl;
		sleep(3);
	}	

	//consume_each (noutput_items);

        // Tell runtime system how many output items we produced.
        //return noutput_items;
				//printf("!!! Estoy al final del main !!!\n");
	return 0;
    }

  //-----Se guarda en disco la data por bloque y nos da la hora en que se guardó----------------
     // Si es false, se crea un file con la hora en q hubo error
     //JARSControl:flag=true cuando llega un bloque correcto(InternalCallback) y false cuando no
    void MyCallbackFunction(bool flag)
     {
	if(flag)
	{
		MyHeader.SaveData(&MyMemManager);//Escribe la data en memoria (fwrite en la carpeta de adquisición)
                
	}
	else //La cabecera no es (0xaa 0xaa)
	{	//Crea un file donde se guardará la hora (Www Mmm dd hh:mm:ss yyyy) cada vez que no se guarde en disco (llega mal el bloque debido a la marca)
		printf("Error... restarting!!!\n");
		//Crea el file con el nombre del directorio adjuntando Restarting Report.txt
	/*	MyHeader.MakeDirectory();
		sprintf(Report1,"%s",MyHeader.MyGPParam->m_sCurrentDirectory);//path donde se guardará la data(fecha)
		sprintf(Report,"%sRestarting Report.txt",Report1);

		MyReport=fopen(Report,"a");

		//Escribe en el .txt la hora en el que se produjo el error de adquisición,
		char timeline[26];
		ftime(&FullTime);//Get the hour
		sprintf(timeline,"%s",ctime(&FullTime.time));
		fprintf(MyReport,"System Restarted at %s",timeline);
		fclose(MyReport);*/
	}
	return;
     }

  } /* namespace InterfaceJRO_7 */
} /* namespace gr */

