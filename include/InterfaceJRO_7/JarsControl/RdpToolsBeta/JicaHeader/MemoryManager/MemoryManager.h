#ifndef ARGPROCESSOR_JARSCONTROL_RDPTOOLSBETA_JICAHEADER_MEMORYMANAGER_MEMORYMANAGER_H_
#define ARGPROCESSOR_JARSCONTROL_RDPTOOLSBETA_JICAHEADER_MEMORYMANAGER_MEMORYMANAGER_H_

#include "ExperimentParameters/ExperimentParameters.h"

#include <complex>
using namespace std;

class MemoryManager
{
public:
//---------------------------------------------------------
	//Buffers para integracion coherente, decodificacion,
	//pos-integracion coherente,reordenamiento y spectra
	int *m_miSizeAcqBuffer;//guarda el tamaño del primer buffer el cual guarda variables de tipo float
	complex<short>** m_mAcqBuffer;//signed short complex//genera un arreglo donde su elemento es un arreglo del tamaño de m_miSizeAcqBuffer[0] y de tipo 16sc
//---------------------------------------------------------

	//NIDAQ buffers
	int m_iSizeNIDoubleBuffer;
	int m_iSizeNIBuffer;//adquiere el tamaño del buffer de memoria para el JARS definido en MemoryManager
	complex<short>* m_nNIBuffer;//separa en memoria un arreglo del tamaño de m_iSizeNIBuffer
	complex<short>* m_nNIDoubleBuffer;//separa en memoria un arreglo del tamaño de dos veces m_iSizeNIBuffer
/*
//---------------------------------------------------------
 */
	//buffers Array to send by zmq
	char buff_header[256];//No encontrado en Memory Manager//Usado en Jicaheader!!
//---------------------------------------------------------
	//objeto con los parametros del experimento estos se
	//establecen externamente (el propio objeto lo hace).
	//Este es solo un puntero a ese
	//objeto
	ExpParameters* MyParams;//Lo hace para obtener los parametros del experimento
	//variable para conteo de funciones que intervendran.Esto
	//se hace para poder inicializar los buffers necesarios
	int numrawbuffs;//posicion del buffer de data cruda
	//constructor y destructor
	MemoryManager();
	~MemoryManager();
//---------------------------------------------------------
	//Asigna la direccion de un objeto ExpParameters (debe
	//haber sido inicializado) a MyParams; y con los
	//parametros del experimento crea los buffers necesarios
	bool AllocateMemory(ExpParameters* MyParameters);
	//libera toda la memoria utilizada.
	void FreeMemory();
//---------------------------------------------------------
private:
	//devuleve el numero de buffers que se necesitaran para
	//realizar todos los procesos correspondientes a rawdata
	void GetBufferSizes();
	//flag para saber si los buffers han sido creados
	bool is_allocated;
};

#endif 
