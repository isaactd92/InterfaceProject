#ifndef ARGPROCESSOR_JARSCONTROL_JARSCONTROL_H_
#define ARGPROCESSOR_JARSCONTROL_JARSCONTROL_H_

#include "RdpToolsBeta/RdpToolsBeta.h"
#include "SP601/sp601_command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <pthread.h>

#include <unistd.h>
//#include <boost/asio.hpp>

//**************************************************************************************************************************
class _JARSControl
{
protected:

	void InternalRcv();
	void InternalThreadEntry();
	void InternalCallback();

public:

       //**********************************************************************
	_JARSControl();
	~_JARSControl();
    	//Parameters of Experiment (luego pasar a AcquisitionManager.h)
	ExpParameters* MyExpParam;
	int m_SizeofBuffer;//adquiere el valor del tamaño del buffer
	void* m_AcqBuffer;//arreglo del buffer que guardará la data obtenida dependiendo si es short or float en tipo 16sc
	//Pointer to client function in order to get advised when interrupt came
	 void (*ptrFunc)(bool success);//_AcquisitionManager//toma el valor de ptrClientFunc asignado en jro_AssignMemory
    	//Function that receives a pointer from ExperimentParameters	
	void jro_MatchParameters(ExpParameters* ExpParam);
	void jro_AssignMemory(MemoryManager* MemManager, void (*ptrClientFunc)(bool sucess),address host_address);
	bool jro_InitAcquisition();
	bool jro_StopAcquisition();
	static void * InternalRcvFunc(void * This);
	static void * InternalThreadEntryFunc(void * This);
	static void * InternalCallbackFunc(void * This);
	pthread_t _thread_rcv;		// thread for acquire UDP packets
	pthread_t _thread_jars;		// thread for acquire UDP packets
	pthread_t _thread_jars_rcv;		// thread for acquire UDP packets

private:
	//Instanciacion de sp601_command
	sp601_command* MySP601;
	/*pthread_t _thread_rcv;		// thread for acquire UDP packets
	pthread_t _thread_jars;		// thread for acquire UDP packets
	pthread_t _thread_jars_rcv;		// thread for acquire UDP packets
	pthread_t _thread_config;		// thread for acquire UDP packets
	*/int VarSetSystem;//valor tomado si está mal seteado
	int16_t m_nChannelsMask; //variable q contiene la mascara de canales adquirir
	int16_t m_nCardsMask; //variable q contiene la mascara de programacion de tarjetas conectadas
	uint16_t m_nFilterCoeff[2047]; // arreglo donde se guardan los coefficientes del filtro
	size_t    m_nCountCoeff; // contador de coeficientes
	
	//Number of Marks, el valor para el JARS es 2
	int m_NumMarks;
	//Mark in Data Real
	int m_MarkRe;
	//Pointer to Acquired Buffer
	void* m_AcqNIBuffer;//buffer más grande separado en memoria
	//Pointer to double memory used for Traditional NIDAQ
	void* m_AcqNIDoubleBuffer;//doble buffer más grande separado en memoria
	//Payload to send jars command
	char payload[16];
	//*************************************************************
	void RemoveMark(); //Fx to remove marks in data
  	//************************************************************
	bool RESET_CTRL_COMMAND();
	bool MODE_OPER_COMMAND();
	bool SELECT_RXD_COMMAND(); //enviado pero no procesado.
	bool PW_RXD_COMMAND();
	bool PROG_RXD_COMMAND();
	bool HAB_TRANS_COMMAND();
	bool DESHAB_TRANS_COMMAND();
	//************************************************************
	bool LoadMask(); // Fx q carga la máscara de programación
	bool ConfigDataTransfer(); // Fx q envia la cantidad de muestras del perfil y NTX que se debe adquirir
	bool SetSystem(address host_address); // Funcion encargada de dejar listo el sistema para adquisicion
	bool LoadFilter(); // Fx q carga la los coefcicientes del filtro
	bool WriteFilter(); // Fx q escribe la los coefcicientes del filtro

};

extern _JARSControl JARSControl;

#endif

