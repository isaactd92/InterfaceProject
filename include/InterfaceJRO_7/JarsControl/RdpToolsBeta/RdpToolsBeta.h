#ifndef ARGPROCESSOR_JARSCONTROL_RDPTOOLSBETA_RDPTOOLBETA_H_
#define ARGPROCESSOR_JARSCONTROL_RDPTOOLSBETA_RDPTOOLBETA_H_

#include <exception>
#include <string>
#include <iostream>
using namespace std;

#include "JicaHeader/JicaHeader.h"

//-------------------------------------------------------------------
#define Data_Win_Err					0
#define Acq_DH_Err					1

class RdpToolsExcep : public exception //clase:superclase
{
public:
//genera 3 string: logfilename: guarda la direccion, info especifica el error y se adjunta a msg, msg nos da el error con tiempo.
//Creará un archivo .log con: la información del error, la fecha y hora del error.
	RdpToolsExcep(int err,char* logpath)//(Data_Win_Err,m_sBaseDirectory);directorio donde se guardará
	{
		time_t seconds;
		char *ptime;

		logfilename.append(logpath);
		logfilename.append("/ProcessErr.log");
		msg.append("Error:\n\t");
		seconds = time(NULL);//NULL=no toma en consideración el parámetro pero igual te da la respuesta en la variable seconds
		ptime =ctime(&seconds);//nos da la hora en formato humano
		switch(err)
		{
			case Data_Win_Err:
				info.append("MyExpParam->MyGPParam.FixPP.m_nData_Windows <= 0");
				msg.append(info);
				info.append("\n\t");
				break;
			case Acq_DH_Err:
				info.append("MyExpParam->MyGRCParam.Dyn_sfAcqDH[0] <= 0");
				msg.append(info);
				info.append("\n\t");
				break;

		}
		//agrega la hora  y fecha del error
		info+="Date: ";
		info+=ptime;info+="\n";
		//DEBUG implementar esto aqui
		msg.append("\n\nRead the ProcessErr.log for more information.");
	}
	~RdpToolsExcep() throw(){}
//-----------------------------------------------------------------------------------------------
	const char* what() const throw()
	{
		FILE* pRdpToolsLog;
		pRdpToolsLog = fopen(logfilename.c_str(),"a");
		//fprintf(pRdpToolsLog,info.c_str());
		fputs(info.c_str(),pRdpToolsLog);
		fclose(pRdpToolsLog);
		return msg.c_str();
	}

private:
	string msg;//se guardará el error
	string info;//informa el error
	string logfilename;//se anexará (append) la direccion del .racp
};

class RdpToolsBeta
{
public:
	RdpToolsBeta();
	~RdpToolsBeta();
//---------------------------------------------------------------------------------------
//Obtiene punteros a objetos ExpParameters y MemoryManager, para acceder a los parametros
//y a los buffers que se necesitaran para la adquisicion y procesamiento.
	void GetExtParameters(ExpParameters* ExpParam,MemoryManager* MemManger)throw();

/////////////////////////////////////////////////////////////////////////////////////////
private:
	//punteros para acceder a los parametros del experimento y a los buffers creados para
	//este.
	ExpParameters* MyExpParam;
	MemoryManager* MyMemManager;

//---------------------------------------------------------------------------------------
//funcion encargada de cargar los parametros en una estructura tipo params y de cargar
//las funciones a el arreglo process_functions(params&)
	bool initVariables()throw();
};

#endif
