#ifndef ARGPROCESSOR_JARSCONTROL_RDPTOOLSBETA_JICAHEADER_MEMORYMANAGER_EXPERIMENTPARAMETER_EXPERIMENTPARAMETERS_H_
#define ARGPROCESSOR_JARSCONTROL_RDPTOOLSBETA_JICAHEADER_MEMORYMANAGER_EXPERIMENTPARAMETER_EXPERIMENTPARAMETERS_H_

#include <stdlib.h>                			 // usr/include
#include <stdio.h>					// usr/include
#include <string.h> //usr/include
#include <string>  //usr/include/c++

#include "ExperimentConstants/ExperimentConstants.h"

class ExpParameters
{private: //http://www.cplusplus.com/reference/cstdio/ (stdio , FILE)
	bool ReadRCParameters(FILE* RACPFile,char* StrTemp);
	bool ReadSysParameters(FILE* RACPFile,char* StrTemp);
	bool ReadProcessParameters(FILE* RACPFile,char* StrTemp);
	bool ReadJARSParameters(FILE* RACPFile,char* StrTemp); /*Agregado x Marcos 20 mayo 2010*/
	char* FindValue(char* MyCad);
 public:
	const char* PathExperiment;
	ExpParameters();
	void Initialize();
	bool ReadRACPFileParameters(const char* RACPFileName);
	~ExpParameters();
	SystemParam MySystemParam;
	GlobPParam  MyGPParam;
	GlobRCParam MyGRCParam;
//---------------------------------------------------------
// Added April 5th 2011
//---------------------------------------------------------
//	FtpParam MyFtpParam;
//---------------------------------------------------------
	bool SaveData;//atributo definido en experiment parameter .cpp.Nos da el valor de SAVE DATA del .racp (yes= true,no=false)
	bool FlagSIR;//atributo definido en experiment parameter .cpp.Nos da el valor de ENABLE SIR del .racp(yes= true,no=false)
	bool OwnWindow; //atributo definido en experiment parameter .cpp . Nos da el valor de GENERATE OWN SAMPLING WINDOW del.racp (yes= true,no=false)
/*************Agregado el 03Nov2009 por Marcos Inoñan***************************/
	bool EnableJARS;//usado para definir el tamaño total de memoria que guardará las muestras//Adquiere valor en ArgProcessor
	bool EnableCooperSimulation;//Adquiere valor en ArgProcessor
/*************Agregado el 09 de febrero del 2010 por Alan Aguilar***************/
	char* RacpFileName;//Guarda una copia del .racp
};

#endif 
