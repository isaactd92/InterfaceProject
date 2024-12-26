#ifndef ARGPROCESSOR_JARSCONTROL_RDPTOOLSBETA_JICAHEADER_JICAHEADER_H_
#define ARGPROCESSOR_JARSCONTROL_RDPTOOLSBETA_JICAHEADER_JICAHEADER_H_

#include <stdlib.h>
#include <time.h>

#include <stdio.h>
#include <string>
#include <string.h>
#include <iostream>

#include "MemoryManager/MemoryManager.h"

//#include <zmq.hpp> *****sockets ZMQ opción 2********

using namespace std;

//zmq::context_t context(1); *****sockets ZMQ opción 2********

class JicaHeader
{private:
	int FileCount;//contador de files
	bool Flag;//usado para saber si mi MyLocalTime.tm_hour es 00(false) o 01 (true)
	int TimeHourBefore; //Added 14 Feb 2011//iniciailzado con 0 y usado para saber si ya se pasó de la hora
	int DirectorySetCount; //incrementa el numero de Directorio cuando llega a 1000 files//contador de directorios
	char Patnhame[_MAX_PATH];//no usado en jicaheader
	char FileName[_MAX_FNAME];//guarda el nombre del file especificado
	int SaveBasicHeader(FILE* MyFile);//nos da el valor de bytes guardados debido a los atributos de MyBasicHeader
	int SaveFirstHeader(FILE* MyFile);////Nos da la cantidad de bytes guardados en BytesWritten debido a longHeader y basic header (Total)
	int GetBasicHeader(MemoryManager* MyMemManager);//Guarda los valores de los atributos de Basic Header en el arreglo buff_header de memory manager
	int GetFirstHeader(MemoryManager* MyMemManager);//Guarda los valores de los atributos de long header junto con los atributos de basic header en buff_header
	FILE *MyFileTemp; //usado en fwrite////los valores de los atributos de LongHeader y basic header en MyFile los guardará en MyFileTemp.
	void* LongHeader; //espacios de memoria del tamaño de long header
 public:
	bool GetFileName();
	bool CompareTime(void);
	unsigned int TimeToFinish;////Tiempo en segundos restantes para que acabe
	bool CloseSystemTime;//toma valor de true si ya se paso la hora
	JicaHeader();
	~JicaHeader();
	void GetEndTime(); // funcion q obtiene la cantidad de segundos q hay desde 1970 hasta la hora en q se debe apagar el sistema
	void GetExpParameters(ExpParameters* MyExpParam);//Guarda los atributos establecidos en long header en espacios de memoria del arreglo ActualPosition
	bool SaveData(MemoryManager* MyMemManager);
	complex<short>* gnuradio_out;// almacenará el bloque de datos para ser visto a la salida de la interfaz en gnuradio-companion
	struct tm MyLocalTime; // variable donde se almacena la fecha actual

	BasicHeader MyBasicHeader; //objeto de la clase BasicHeader para dar valor al atributo:
	SystemParam *MySystemParam;
	GlobPParam  *MyGPParam;
	GlobRCParam *MyGRCParam;

	float** m_sCodeFloat;//puntero al array que almacena los valores de NUMBER OF PROCESS BAUDS en un malloc tipo float	
	char FileSaved[_MAX_PATH];//Copia m_sCurrentDirectory aqui en formato .r o .pdata si no hay digital_rf
	void MakeDirectory(void); // Fx q retorna un puntero a la cadena de carpetas creadas
};

#endif
