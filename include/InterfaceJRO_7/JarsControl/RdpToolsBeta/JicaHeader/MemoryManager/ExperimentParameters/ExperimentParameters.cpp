#include "ExperimentParameters.h"

#include <string.h> //usr/include
#include <string>  //usr/include/c++

ExpParameters::ExpParameters()
{
	Initialize();
}

//---------------------------------------------------------
ExpParameters::~ExpParameters()
{

}
//---------------------------------------------------------
void ExpParameters::Initialize()
{
		SaveData=false;
	//Initialize the System Params
		MySystemParam.m_nSamples=0;                 //MySystemParam es objeto de la clase SystemParam
		MySystemParam.m_nProfiles=0;
		MySystemParam.m_nChannels=1;
		MySystemParam.m_nADCResolution=12;
		MySystemParam.m_nPCIDIOBusWidth=32;
	//Initialize the RCParam into GlobalRCParam //Inicializa los atributos de la estructura RadarControllerParam
		MyGRCParam.FixRCP.m_nEspType=0;   //MyGRCParam objeto de GlobRCParam/FixRCP objeto de RadarControllerParam, definido dentro de GlobRCParam
		MyGRCParam.FixRCP.m_nNTX=1;
		MyGRCParam.FixRCP.m_fIPP=0.0f;
		MyGRCParam.FixRCP.m_fTXA=0.0f;
		MyGRCParam.FixRCP.m_fTXB=0.0f;
		MyGRCParam.FixRCP.m_nNum_Windows=1;
		MyGRCParam.FixRCP.m_nNum_Taus=0;

		MyGRCParam.FixRCP.m_nLine6_Function=NONE; //NONE definido como 0 en ExperimentConstant.h
		MyGRCParam.FixRCP.m_nLine5_Function=NONE; //NONE definido como 0 en ExperimentConstant.h
		MyGRCParam.FixRCP.m_fCLOCK=8.00f;
		MyGRCParam.FixRCP.m_nPrePulseBefore=0;
		MyGRCParam.FixRCP.m_nPrePulseAfter=0;
		MyGRCParam.FixRCP.m_sRango_IPP[0]='\0'; //último valor de un string literal
		MyGRCParam.FixRCP.m_sRango_TXA[0]='\0';
		MyGRCParam.FixRCP.m_sRango_TXB[0]='\0';
	//Initialize the rest of GlobalRCParam
		MyGRCParam.Dyn_sfAcqH0=NULL;   //MyGRCParam objeto de GlobRCParam/inicializa los atributos de GlobRCParam
		MyGRCParam.Dyn_sfAcqDH=NULL;
		MyGRCParam.Dyn_snAcqNSA=NULL;
	//Initilize the ProcessParam into GlobalPParam
		MyGPParam.FixPP.m_nExperimentType=EXP_RAWDATA;
		MyGPParam.FixPP.m_nSizeOfDataBlock=0;
		MyGPParam.FixPP.m_nDataBlocksperFile=1;
		MyGPParam.FixPP.m_nData_Windows=1;
		MyGPParam.FixPP.m_nProcessFlags=FLAG_DATATYPE_SHORT;//0x00000080
	//Initilize the rest of GlobalPParam
		MyGPParam.Dyn_sfDataH0=NULL;
		MyGPParam.Dyn_sfDataDH=NULL;
		MyGPParam.Dyn_snDataNSA=NULL;//arreglo vacio
/*-> ------ Agregado el 24 - 11 - 2009 ------ <-*/
		MyGPParam.bCreateDayDir=false;
		MyGPParam.bNameInDirectory=false;
		MyGPParam.m_bBeginOnStart=false;
/*->--------Agregado el 22 - 02 - 2010 -------<-*/
		MyGPParam.m_nProcessWindows = 0;

}

//Lee el archivo .racp y da valor a los atributos declarados con los valores de los parámetros del experimento
bool ExpParameters::ReadRACPFileParameters(const char* RACPFileName)
{
	FILE* MyRacpFile;
	char StrTemp[_MAX_NAME_PATH];//_MAX_NAME_PATH=260
	char* StrPtr; //puntero que nos da el valor encontrado del string

	PathExperiment = RACPFileName;

	if((MyRacpFile=fopen(RACPFileName,"r"))==NULL)//"r" :solo lectura. Lee si el .racp esta vacio
		return false;
	RacpFileName = new char[strlen(RACPFileName)];//crea un arreglo del tamaño del arreglo RACPFileName y apunta a la primera posicin con RacpFileName
	strcpy(RacpFileName,RACPFileName);//Copia el array de RACPFileName a RacpFileName
	for(;;)
	{	if(feof(MyRacpFile))//si llega al final del archivo .racp
		{	fclose(MyRacpFile);//cierra el archivo.
			break;
		}
	fgets(StrTemp,_MAX_NAME_PATH,MyRacpFile);//agarra los primeros _MAX_NAME_PATH (260), o hasta encontrar una nueva linea,  caracteres de MyRacpFile y lo guarda 							  en el arreglo StrTemp Ya que nuestro .racp tiene nuevas líneas,(enters), para en cada linea y guarda el puntero del 							  array de cada linea en StrTemp

//** Busca experiment Type en el .racp y establece si es EXP_PROCESS_SPECTRA o EXP_RAW_DATA, dependiendo de esto da el valor a
//m_nExperimentType de 1 o 0 y ?¿?¿ (flags)

		if((StrPtr=strstr(StrTemp,"EXPERIMENT TYPE="))==StrTemp)//Busca EXPERIMENT TYPE= en StrTemp y nos da el puntero
		{
			if((StrPtr=strstr(StrTemp,"EXP_PROCESS_SPECTRA"))!=NULL)
			{
				printf("No usaremos experimentos tipo espectra\n");
			}
			else if((StrPtr=strstr(StrTemp,"EXP_RAW_DATA"))!=NULL)
			{
				MyGPParam.FixPP.m_nExperimentType = EXP_RAWDATA; //EXP_RAWDATA= 0
				MyGPParam.FixPP.m_nProcessFlags &= ~FLAG_SPECTRA_CALC;//??? &= and binario
				printf("Raw data encontrado\n");
			}
			continue;
		}
// Busca experiment name en .racp (segundo bucle)(segunda lineas y nos da el puntero del primer elemento el cual es igual a
// StrTemp. Guarda el valor del nombre del experimento en letras dentro de un arreglo

		if((StrPtr=strstr(StrTemp,"EXPERIMENT NAME="))==StrTemp)
		{
			//strcpy_s(MyGPParam.m_sExpName,FindValue(StrPtr));  // In Windows
      strcpy(MyGPParam.m_sExpName,FindValue(StrPtr)); //Find value nos da el puntero a partir del string despues de igual("=")
      //strcpy copia el nombre del experimento despues de "=" en m_sExpName
			for(int i=0; ;i++)  // para borrar el ultimo caracter
				if(*(MyGPParam.m_sExpName+i)=='\0')
				{	*(MyGPParam.m_sExpName+i-1)=(char)(NULL);
					break;
				}
			continue;
		}
// FindValue nos da el puntero del valor despues de "=". Atoi transforma ese valor (integral number dentro de str) a int
//y lo guarda en m_nEspType.
//NUMBER OF EXPERIMENTS no encontrado en .racp
		if((StrPtr=strstr(StrTemp,"NUMBER OF EXPERIMENTS="))==StrTemp)
		{
			MyGRCParam.FixRCP.m_nEspType=(unsigned int)atoi(FindValue(StrPtr));
			continue;
		}

//FindValue nos da el puntero del valor despues de "=". Atoi transforma ese valor (integral number dentro de str) a int
//y lo guarda en m_nCards.
		if((StrPtr=strstr(StrTemp,"Number of Cards="))==StrTemp)
		{
			MyGPParam.m_nCards=(unsigned int)atoi(FindValue(StrPtr));
			if(MyGPParam.m_nCards>0 && MyGPParam.m_nCards < 10)
			{
				MyGPParam.Dyn_nCardSequence= new unsigned char[MyGPParam.m_nCards];
				//crea un arreglo con los valores (char) de las cartas dados en el .racp
				//---------------------------------------------------------------------
				//Read Cards Sequence
				//---------------------------------------------------------------------
				for(size_t q=0;q<MyGPParam.m_nCards;q++)
				{
					//Read and put the code string in valstr
					fgets(StrTemp,100,MyRacpFile);//Guardar la linea q sigue en StrTemp
					MyGPParam.Dyn_nCardSequence[q]=(unsigned char)atoi(FindValue(StrTemp));
					//Puesto por MArcos: MyGPParam.Dyn_nCardSequence[q]=(unsigned char)atoi(FindValue(StrPtr));
					//StrPtr sigue con el mismo valor que if()
				}
			}
			else if(MyGPParam.m_nCards>=10)
			{
				printf("SysParamScanFile: Error!\n");
			}
			continue;
		}

//FindValue nos da el puntero del valor despues de "=". Atoi transforma ese valor (integral number dentro de str) a int.
//Nos da el valor de m_nChannels del .racp

		if((StrPtr=strstr(StrTemp,"Number of Channels="))==StrTemp)
		{
			MySystemParam.m_nChannels = (unsigned int)atoi(FindValue(StrPtr));
//			MyAcqParam.nAcqChannels = MySystemParam.m_nChannels;//DEBUG1
			printf("El valor de number of channels del RACP es: %d \n",MySystemParam.m_nChannels);
			continue;
		}
//FindValue nos da el puntero del valor despues de "=".
//m_sBaseDirectory es un arreglo donde guarda el valor de RAW DATA DIRECTORY

		/*->  Agregado el 24-11-09  <-*/
		if((StrPtr=strstr(StrTemp, "RAW DATA DIRECTORY="))==StrTemp)
		{
			strcpy(MyGPParam.m_sBaseDirectory,FindValue(StrPtr));
			for(int i=0; ;i++)  // para borrar el ultimo caracter
				if(*(MyGPParam.m_sBaseDirectory+i)=='\0')
				{
					*(MyGPParam.m_sBaseDirectory+i-1)=(char)(NULL);//Borra el ùltimo "/" de RAW DATA DIRECTORY=/home/itupac/DataAdquirida/
					break;
				}
			continue;
		}

//FindValue nos da el puntero del valor despues de "=".
//m_sBaseDirectory es un arreglo donde guarda el valor de PROCESS DATA DIRECTORY
//No hay
		if((StrPtr=strstr(StrTemp, "PROCESS DATA DIRECTORY="))==StrTemp)
		{
			//strcpy_s(MyGPParam.m_sBaseDirectory,FindValue(StrPtr));  // In Windows
      strcpy(MyGPParam.m_sBaseDirectory,FindValue(StrPtr));
			for(int i=0; ;i++)  // para borrar el ultimo caracter
				if(*(MyGPParam.m_sBaseDirectory+i)=='\0')
				{
					*(MyGPParam.m_sBaseDirectory+i-1)=(char)(NULL);
					break;
				}
			continue;
		}
//Busca "Create Directory per day" en StrTemp. StrTemp guarda toda la linea de .racp dentro de un arreglo
		if((StrPtr=strstr(StrTemp,"CREATE DIRECTORY PER DAY="))==StrTemp)
		{
//StrTemp sigue siendo toda la linea. Asigna valores de verdadero o falso a los atributos bCreateDayDir
			if((StrPtr=strstr(StrTemp,"YES"))!=NULL)
			{
				MyGPParam.bCreateDayDir=true;
			}
			else
			{
				MyGPParam.bCreateDayDir=false;
			}
			continue;
		}

//Busca "INCLUDE EXPNAME IN DIRECTORY" en StrTemp. StrTemp guarda toda la linea de .racp dentro de un arreglo

		if((StrPtr=strstr(StrTemp,"INCLUDE EXPNAME IN DIRECTORY="))==StrTemp)
		{
//StrTemp sigue siendo toda la linea. Asigna valores de verdadero o falso a los atributos bNameInDirectory
			if((StrPtr=strstr(StrTemp,"YES"))!=NULL)
			{
				MyGPParam.bNameInDirectory=true;
			}
			else
			{
				MyGPParam.bNameInDirectory=false;
			}
			continue;
		}
//****	Atributos Declarados en experimentParameters.h********************************
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//Busca "INCLUDE EXPNAME IN DIRECTORY" en StrTemp. StrTemp guarda toda la linea de .racp dentro de un arreglo
		if((StrPtr=strstr(StrTemp,"GENERATE OWN SAMPLING WINDOW="))==StrTemp)
		{
			if((StrPtr=strstr(StrTemp,"YES"))!=NULL)
			{
				OwnWindow=true; //atributo declarado en experimentParameters.h
			}
			else
			{
				OwnWindow=false;
			}
			continue;
		}
//Busca "Digital RF" en StrTemp. StrTemp guarda toda la linea de .racp dentro de un arreglo
//Definido con errores.
		if((StrPtr=strstr(StrTemp,"DIGITAL RF="))==StrTemp)
		{
			//printf("No se definirá la extensión Digital RF. \n");
			continue;
		}
//Busca "ENABLE SIR" en StrTemp. StrTemp guarda toda la linea de .racp dentro de un arreglo
		if((StrPtr=strstr(StrTemp,"ENABLE SIR="))==StrTemp)
		{
			if((StrPtr=strstr(StrTemp,"YES"))!=NULL)
			{
				FlagSIR=true;// atributo declarado en experimentParameters.h
			}
			else
			{
				FlagSIR=false;
			}
			continue;
		}
//Busca "SAVE DATA" en StrTemp. StrTemp guarda toda la linea de .racp dentro de un arreglo
		if((StrPtr=strstr(StrTemp,"SAVE DATA="))==StrTemp)
		{
			if((StrPtr=strstr(StrTemp,"YES"))!=NULL)
			{
				SaveData=true;// atributo declarado en experimentParameters.h
			}
			else
			{
				SaveData=false;
			}
			break;
		}
//*******************Lee las demás lineas que no están incluidas arriba***************************
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		ReadRCParameters(MyRacpFile,StrTemp);   //Parámetros del Radar Controller que no están especificados arriba
		ReadSysParameters(MyRacpFile,StrTemp);
		ReadProcessParameters(MyRacpFile,StrTemp);
		ReadJARSParameters(MyRacpFile,StrTemp); /*Agregado x Marcos 20 mayo 2010*/
	}
return true;
}

//---------------------------------------------------------
//Nos da el puntero posicionado después del "="
//---------------------------------------------------------
char* ExpParameters::FindValue(char* MyCad)
{
  char* ptrStr;
	//printf("%s\n",(char *)MyCad);
	ptrStr=strstr(MyCad,"=");
	ptrStr++;
  //printf("%s\n",(char *)ptrStr);
	return ptrStr;

}
//-------------------------------------------------------------------
//Define los parámetros del RadarCOntroler en el Experiment Constant
//-------------------------------------------------------------------
bool ExpParameters::ReadRCParameters(FILE* RACPFile,char* StrTemp)
{
	char* StrPtr;

	if((StrPtr=strstr(StrTemp,"IPP="))==StrTemp)//periodo entre pulsos km
	{//Cambia el valor del ipp (str) del .racp a float
		MyGRCParam.FixRCP.m_fIPP=(float)atof(FindValue(StrPtr));//atof lo interpreta como float y nos da double.Pero (float) gana
		goto fin_RCP; //return true
	}
	//---------------------------------------------------------------
	if((StrPtr=strstr(StrTemp,"NTX="))==StrTemp)//numero de Ipps
	{//Asigna valor al parametro m_nNTX (número de Ipps)
		MyGRCParam.FixRCP.m_nNTX=(unsigned int)atoi(FindValue(StrPtr));//atoi nos da el numero expresado en int
		goto fin_RCP;//return true
	}
	//---------------------------------------------------------------
	if((StrPtr=strstr(StrTemp,"TXA="))==StrTemp)// Ancho de pulso de transmision por A
	{////asigna el valor a m_fTXA como float
		MyGRCParam.FixRCP.m_fTXA=(float)atof(FindValue(StrPtr));//atof lo interpreta como float y nos da double.Pero (float) gana
		goto fin_RCP;
	}
	//---------------------------------------------------------------
	if((StrPtr=strstr(StrTemp,"TXB="))==StrTemp)// Ancho de pulso de transmision por B
	{//asigna el valor a m_fTXB como float
		MyGRCParam.FixRCP.m_fTXB=(float)atof(FindValue(StrPtr));//atof lo interpreta como float y nos da double.Pero (float) gana
		goto fin_RCP;
	}
	//---------------------------------------------------------------
	//************Encontrado en JASMET
	//Asigna valor al atributo m_nCodeType
	if((StrPtr=strstr(StrTemp,"Code Type="))==StrTemp)
	{
		printf("No usaremos el tipo de código ya que no decodificaremos.\n");
		goto fin_RCP;//retun true
	}
	//---------------------------------------------------------------
	//Cantidad de codigos
	if((StrPtr=strstr(StrTemp,"Number of Codes="))==StrTemp)//Tipos de codigo de ipp
	{//Asigna valor al atributo m_nNum_Codes(no encontrado en .racp)
		printf("No decodificaremos ni utilizaremos el modo Cooperball, por lo que no necesitaremos el número de códigos.\n");
		goto fin_RCP;//return true
	}
	//---------------------------------------------------------------
	//Ancho de codigo
	if((StrPtr=strstr(StrTemp,"Code Width="))==StrTemp)//Code Width= mensaje //Cantidad de bits que contenía el código
	{
		printf("No decodificaremos ni utilizaremos el modo Cooperball, por lo que no necesitaremos el ancho de código.\n");
		goto fin_RCP;
	}
	//---------------------------------------------------------------
	//NO encontrado
	if((StrPtr=strstr(StrTemp,"PULSE SELECTION_TXA="))==StrTemp)
	{//Asigna valor a m_sRango_TXA como un arreglo de caracteres
		strcpy(MyGRCParam.FixRCP.m_sRango_TXA,FindValue(StrPtr)); //char [20]
		goto fin_RCP;
	}
	//---------------------------------------------------------------
	//NO encontrado
	if((StrPtr=strstr(StrTemp,"PULSE SELECTION_TXB="))==StrTemp)
	{//Asigna valor a m_sRango_TXB como un arreglo de caracteres
		strcpy(MyGRCParam.FixRCP.m_sRango_TXB,FindValue(StrPtr));
		goto fin_RCP;
	}
	//---------------------------------------------------------------
	//NO encontrado
	if((StrPtr=strstr(StrTemp,"PULSE SELECTION_TR="))==StrTemp)
	{//Asigna valor a m_sRango_IPP como un arreglo de caracteres
		strcpy(MyGRCParam.FixRCP.m_sRango_IPP,FindValue(StrPtr));
		goto fin_RCP;
	}
	//---------------------------------------------------------------
	//NO encontrado
	if((StrPtr=strstr(StrTemp,"SAMPLING WINDOWS (LINE 4)="))==StrTemp)
	{
		fgets(StrTemp,100,RACPFile);//DEBUG por que no se ha implementado
		fgets(StrTemp,100,RACPFile);
		fgets(StrTemp,100,RACPFile);
		goto fin_RCP;
	}
	//---------------------------------------------------------------
	//NO encontrado
	if((StrPtr=strstr(StrTemp,"SAMPLING WINDOWS (LINE 5)="))==StrTemp)
	{
		MyGRCParam.FixRCP.m_nLine5_Function=SAMPLING;//SAMPLING=3
		MyGRCParam.m_nL5_NumWindows =(unsigned int)atoi(FindValue(StrPtr));//Asigna el valor de SAMPLING WINDOWS (LINE 5) del .racp
		MyGRCParam.Dyn_sfL5_AcqH0 = new float[MyGRCParam.m_nL5_NumWindows];
		MyGRCParam.Dyn_snL5_AcqNSA = (unsigned int *)(new size_t[MyGRCParam.m_nL5_NumWindows]);
		MyGRCParam.Dyn_sfL5_AcqDH = new float[MyGRCParam.m_nL5_NumWindows];

		for(size_t i=0;i<MyGRCParam.m_nL5_NumWindows;i++)
		{
			fgets(StrTemp,100,RACPFile);//?¿?¿
			//Marcos MyGRCParam.Dyn_sfL5_AcqH0[i]=(float)atof(FindValue(StrPtr));
			MyGRCParam.Dyn_sfL5_AcqH0[i]=(float)atof(FindValue(StrTemp));

			fgets(StrTemp,100,RACPFile);
			//Añadido por Marcos: MyGRCParam.Dyn_snL5_AcqNSA[i]=(unsigned int)atoi(FindValue(StrPtr));
			MyGRCParam.Dyn_snL5_AcqNSA[i]=(unsigned int)atoi(FindValue(StrTemp));

			fgets(StrTemp,100,RACPFile);
			//	MyGRCParam.Dyn_sfL5_AcqDH[i]=(float)atof(FindValue(StrPtr));
			MyGRCParam.Dyn_sfL5_AcqDH[i]=(float)atof(FindValue(StrTemp));
		}
		goto fin_RCP;
	}
	//---------------------------------------------------------------
	//NO encontrado
	if((StrPtr=strstr(StrTemp,"SAMPLING WINDOWS (LINE 6)="))==StrTemp)
	{
		MyGRCParam.FixRCP.m_nLine6_Function=SAMPLING;//3
		MyGRCParam.m_nL6_NumWindows =(unsigned int)atoi(FindValue(StrPtr));//Asigna el valor despues del "="
		MyGRCParam.Dyn_sfL6_AcqH0 = new float[MyGRCParam.m_nL6_NumWindows];
		//MyGRCParam.Dyn_snL6_AcqNSA = new size_t[MyGRCParam.m_nL6_NumWindows]; // In Windows
		MyGRCParam.Dyn_snL6_AcqNSA = (unsigned int *)(new size_t[MyGRCParam.m_nL6_NumWindows]);
		MyGRCParam.Dyn_sfL6_AcqDH = new float[MyGRCParam.m_nL6_NumWindows];

		for(size_t i=0;i<MyGRCParam.m_nL6_NumWindows;i++)
		{
			fgets(StrTemp,100,RACPFile);//?¿? StrPtr no cambia de valor
			//Marcos: MyGRCParam.Dyn_sfL6_AcqH0[i]=(float)atof(FindValue(StrPtr));
			MyGRCParam.Dyn_sfL6_AcqH0[i]=(float)atof(FindValue(StrTemp));//¿el valor después del igual es asignado al arra?

			fgets(StrTemp,100,RACPFile);//?¿?
			//Marcos: MyGRCParam.Dyn_snL6_AcqNSA[i]=(unsigned int)atoi(FindValue(StrPtr));
			MyGRCParam.Dyn_snL6_AcqNSA[i]=(unsigned int)atoi(FindValue(StrTemp));
			//?¿?
			fgets(StrTemp,100,RACPFile);//?¿?
			//Marcos : MyGRCParam.Dyn_sfL6_AcqDH[i]=(float)atof(FindValue(StrPtr));
			MyGRCParam.Dyn_sfL6_AcqDH[i]=(float)atof(FindValue(StrTemp));
		}
		goto fin_RCP;
	}
	//---------------------------------------------------------------
	if((StrPtr=strstr(StrTemp,"Number of Taus="))==StrTemp)
	{
		MyGRCParam.FixRCP.m_nNum_Taus = (unsigned int)atoi(FindValue(StrPtr));
		MyGRCParam.Dyn_sfTau = new float[MyGRCParam.FixRCP.m_nNum_Taus];

		for(size_t i=0;i<MyGRCParam.FixRCP.m_nNum_Taus;i++)
		{
			fgets(StrTemp,100,RACPFile);//Obtiene el valor de tau de la siguiente línea
			//MyGRCParam.Dyn_sfTau[i]=(float)atof(FindValue(StrPtr));
			MyGRCParam.Dyn_sfTau[i]=(float)atof(FindValue(StrTemp));//Debería ser: StrTEmp en vez de StrPtr(Preguntar a Jhon)
		}
		goto fin_RCP;
	}
	//---------------------------------------------------------------
	if((StrPtr=strstr(StrTemp,"Sampling Windows="))==StrTemp)
	{
		MyGRCParam.FixRCP.m_nNum_Windows=(unsigned int)atoi(FindValue(StrPtr));
		MyGRCParam.Dyn_sfAcqH0 = new float[MyGRCParam.FixRCP.m_nNum_Windows];//array donde guardar los valores de H0
		//MyGRCParam.Dyn_snAcqNSA = new size_t[MyGRCParam.FixRCP.m_nNum_Windows]; // In Windows
		MyGRCParam.Dyn_snAcqNSA = (unsigned int *)(new size_t[MyGRCParam.FixRCP.m_nNum_Windows]);//array donde guardará los valores del numero de muestras
		MyGRCParam.Dyn_sfAcqDH = new float[MyGRCParam.FixRCP.m_nNum_Windows];//array donde guardará el valor de la resolucion
		for(size_t i=0;i<MyGRCParam.FixRCP.m_nNum_Windows;i++)
		{
			fgets(StrTemp,100,RACPFile);
			//Marcos: 	MyGRCParam.Dyn_sfAcqH0[i]=(float)atof(FindValue(StrPtr));
			MyGRCParam.Dyn_sfAcqH0[i]=(float)atof(FindValue(StrTemp));//Debería ser: StrTEmp en vez de StrPtr(Preguntar a Jhon)

			fgets(StrTemp,100,RACPFile);
			//Marcos:MyGRCParam.Dyn_snAcqNSA[i]=(unsigned int)atoi(FindValue(StrPtr));
			MyGRCParam.Dyn_snAcqNSA[i]=(unsigned int)atoi(FindValue(StrTemp));//Debería ser: StrTEmp en vez de StrPtr(Preguntar a Jhon)
			MySystemParam.m_nSamples+=MyGRCParam.Dyn_snAcqNSA[i];//total de numero de muestras
			printf("El valor de Dyn_snAcqNSA en ExpParameters es: %d\n",MyGRCParam.Dyn_snAcqNSA[i]);

			fgets(StrTemp,100,RACPFile);
			//Marcos: MyGRCParam.Dyn_sfAcqDH[i]=(float)atof(FindValue(StrPtr));
			MyGRCParam.Dyn_sfAcqDH[i]=(float)atof(FindValue(StrTemp));//Debería ser: StrTEmp en vez de StrPtr(Preguntar a Jhon)
		}
//		MyAcqParam.nAcqSamples = MySystemParam.m_nSamples;//DEBUG1//nAcqSamples=total de número de muestras
		goto fin_RCP;
	}
	
	//---------------------------------------------------------------
	if((StrPtr=strstr(StrTemp,"RELOJ="))==StrTemp)
	{
		MyGRCParam.FixRCP.m_fCLOCK = (float)atof(FindValue(StrPtr));//Obtiene el valor de la variable reloj del .racp
		goto fin_RCP;
	}
	//---------------------------------------------------------------
fin_RCP:
	return true;
}

//-------------------------------------------------------------------
//
//--------------------------------------------------------------------

bool ExpParameters::ReadSysParameters(FILE* RACPFile,char* StrTemp)
{
	char* StrPtr;
	//---------------------------------------------------------------
	if((StrPtr=strstr(StrTemp,"ADC RESOLUTION="))==StrTemp) //Busca ADC RESOLUTION en el .racp
	{
		MySystemParam.m_nADCResolution=(unsigned int)atoi(FindValue(StrPtr)); //El valor de la resolucion lo guarda en m_nADCResolution
		goto fin_SYSP;
	}
	//---------------------------------------------------------------
	if((StrPtr=strstr(StrTemp,"PCI DIO BUSWIDTH="))==StrTemp)//Busca PCI DIO BUSWIDTH= en el .racp
	{
		MySystemParam.m_nPCIDIOBusWidth=(unsigned int)atoi(FindValue(StrPtr));//Guarda el valor del ancho de bus del PCI en m_nPCIDIOBusWidth
		goto fin_SYSP;
	}
	//---------------------------------------------------------------
	//Debería ir en Process Parameter
	if((StrPtr=strstr(StrTemp,"RAW DATA BLOCKS="))==StrTemp)//Busca RAW DATA BLOCK en el .racp
	{
		MyGPParam.FixPP.m_nDataBlocksperFile=(unsigned int)atoi(FindValue(StrPtr));//Guarda el valor de raw data blocks en m_nDataBlocksperFile
		printf("El valor de RAW DATA BLOCKS es: %d\n",MyGPParam.FixPP.m_nDataBlocksperFile);
		goto fin_SYSP;
	}
	//---------------------------------------------------------------
	if((StrPtr=strstr(StrTemp,"PROCESS DATA BLOCKS="))==StrTemp)//Busca PROCESS DATA BLOCKS en el .racp
	{
		MyGPParam.FixPP.m_nDataBlocksperFile=(unsigned int)atoi(FindValue(StrPtr));//Guarda el valor de process data blocks en m_nDataBlocksperFile
		goto fin_SYSP;
	}
	//---------------------------------------------------------------
fin_SYSP:
return true;
}
//---------------------------------------------------------------
// Asigna valores a los parametros de procesamiento -------------
//---------------------------------------------------------------
bool ExpParameters::ReadProcessParameters(FILE* RACPFile,char* StrTemp)
{
	char* StrPtr;
	//---------------------------------------------------------------
	if((StrPtr=strstr(StrTemp,"DATATYPE="))==StrTemp)//Busca DATATYPE en el .racp
	{
		if((StrPtr=strstr(StrTemp,"FLOAT"))!=NULL)
		{   //ASigna valor al flag m_nProcessFlags
			printf("La interfaz no usará data de adqusición de tipo float, solo short complex\n");

		}
		else
		{
			MyGPParam.FixPP.m_nProcessFlags&=~FLAG_DATATYPE_FLOAT;
			MyGPParam.FixPP.m_nProcessFlags|=FLAG_DATATYPE_SHORT;
		}
		goto fin_PROP;
	}
	//---------------------------------------------------------------
	if((StrPtr=strstr(StrTemp,"DATA ARRANGE="))==StrTemp)//Busca DATA ARRANGE en el .racp
	{	//Asigna valor al flag m_nProcessFlags
		//clean the corresponding bits (INVERT MASK)
		MyGPParam.FixPP.m_nProcessFlags&=(~(FLAG_DATAARRANGE_CONTIGUOUS_H
						|FLAG_DATAARRANGE_CONTIGUOUS_CH|FLAG_DATAARRANGE_CONTIGUOUS_P));//?¿?¿?

		if((StrPtr=strstr(StrTemp,"CONTIGUOUS_H"))!=NULL)
		{	//Asigna valor al flag m_nProcessFlags
			MyGPParam.FixPP.m_nProcessFlags|=FLAG_DATAARRANGE_CONTIGUOUS_H;
		}
		else if((StrPtr=strstr(StrTemp,"CONTIGUOUS_CH"))!=NULL)
		{
			MyGPParam.FixPP.m_nProcessFlags|=FLAG_DATAARRANGE_CONTIGUOUS_CH;
		}
		else if((StrPtr=strstr(StrTemp,"CONTIGUOUS_P"))!=NULL)
		{
			MyGPParam.FixPP.m_nProcessFlags|=FLAG_DATAARRANGE_CONTIGUOUS_P;
		}
		goto fin_PROP;
	}
	//---------------------------------------------------------------
	if((StrPtr=strstr(StrTemp,"COHERENT INTEGRATIONS="))==StrTemp)//Busca COHERENT INTEGRATIONS en el .racp
	{
		printf("Integraciones coherentes no están definidas dentro de la interfaz.\n");
		goto fin_PROP;
	}
	//---------------------------------------------------------------
	if((StrPtr=strstr(StrTemp,"COHERENT INTEGRATION STRIDE="))==StrTemp)//Busca COHERENT INTEGRATION STRIDE en el .racp
	{
		printf("Stride integración coherente no está definida dentro de la interfaz.\n");
		goto fin_PROP;
	}
	//---------------------------------------------------------------
	if((StrPtr=strstr(StrTemp,"ACQUIRED PROFILES="))==StrTemp)//Busca ACQUIRED PROFILES en el .racp//perfiles usados para realizar las integr.cohere con respecto a PROFILES PER BLOCK
	{
		MySystemParam.m_nProfiles=(unsigned int)atoi(FindValue(StrPtr));//Asigna el valor de ACQUIRED PROFILES al m_nProfiles
		goto fin_PROP;
	}
	//---------------------------------------------------------------
	if((StrPtr=strstr(StrTemp,"PROFILES PER BLOCK="))==StrTemp)//Busca PROFILES PER BLOCK en el .racp
	{
		MyGPParam.m_nProfilesperBlock = (unsigned int)atoi(FindValue(StrPtr));//Asigna el valor de PROFILES PER BLOCK al m_nProfilesperBlock
		MyGPParam.FixPP.m_nFFTorProfilesperBlock = MyGPParam.m_nProfilesperBlock;		
		goto fin_PROP;
		printf("El valor de PROFILES PER BLOCK es = %d\n",MyGPParam.m_nProfilesperBlock);
	}
	//---------------------------------------------------------------
	if((StrPtr=strstr(StrTemp,"BEGIN ON START="))==StrTemp)//Busca BEGIN ON START en el .racp
	{
		if((StrPtr=strstr(StrTemp,"YES"))!=NULL)
			MyGPParam.m_bBeginOnStart = true; //Asigna valor a m_bBeginOnStart
		goto fin_PROP;
	}
	// No encontrado los valores de tm_min y tm_hour
	//---------------------------------------------------------------
	if((StrPtr=strstr(StrTemp,"BEGIN_TIME="))==StrTemp)//Busca BEGIN_TIME en el .racp
	{//sscanf: Busca lo que se quiere del primer operador.
		sscanf(FindValue(StrPtr),"%2d:%2d",&(MyGPParam.FixPP.m_BeginTime.tm_hour),&(MyGPParam.FixPP.m_BeginTime.tm_min));//Escoge los dos numeros del .racp y debería guardarlo en tm_hour y tm_min
		goto fin_PROP;
	}
	//---------------------------------------------------------------

	if((StrPtr=strstr(StrTemp,"END_TIME="))==StrTemp)
	{	sscanf(FindValue(StrPtr),"%2d:%2d",&(MyGPParam.FixPP.m_EndTime.tm_hour),&(MyGPParam.FixPP.m_EndTime.tm_min));//Escoge los dos numeros del .racp y debería guardarlo en tm_hour y tm_min
		goto fin_PROP;
	}
	//---------------------------------------------------------------
	if((StrPtr=strstr(StrTemp,"DECODE DATA="))==StrTemp)//Busca DECODE DATA en el .racp
	{
		printf("La interfaz no decodificará data\n");
		goto fin_PROP;
	}
	//---------------------------------------------------------------
	//No encontrado en el .racp
	if ((StrPtr=strstr(StrTemp,"DECODING TYPE="))==StrTemp)//Busca DECODING TYPE en el .racp
	{
		StrPtr=FindValue(StrPtr);
		MyGPParam.m_nDecodingFlags=0;
		if(strstr(StrPtr, "DECODING_INV_FREQ_DOMAIN") != NULL)
		{
			MyGPParam.FixPP.m_nProcessFlags|=FLAG_INVFFTDECODING; //0x00040000
			MyGPParam.m_nDecodingFlags|=DECODING_INV_FREQ_DOMAIN; //0x00000004
		}
		else if(strstr(StrPtr, "DECODING_TIME_DOMAIN") != NULL)
		{
			MyGPParam.m_nDecodingFlags|=DECODING_TIME_DOMAIN;//0x00000001
		}
		else if(strstr(StrPtr, "DECODING_FREQ_DOMAIN") != NULL)
		{
			MyGPParam.m_nDecodingFlags|=DECODING_FREQ_DOMAIN;//0x00000002
		}
		else
		{
			printf("\nError Reading *.rasp file : DECODING TYPE");
		}
		goto fin_PROP;
	}
	//-------------------------------------------------------------------
	//No encontrado en el .racp
	if((StrPtr=strstr(StrTemp,"NUMBER OF PROCESS CODES="))==StrTemp)//Busca NUMBER OF PROCESS CODES en el .racp
	{
		printf("No decodificaremos ni usaremos el modo cooperball, por lo que no usaremos número de códigos de procesamiento.\n");
		goto fin_PROP;
	}
	//---------------------------------------------------------------
	//No encontrado en el .racp
	if((StrPtr=strstr(StrTemp, "NUMBER OF PROCESS BAUDS="))==StrTemp)
	{ //Separa memoria
		printf("No decodificaremos ni usaremos el modo cooperball, por lo que no usaremos número de baudios de procesamiento.\n");
		goto fin_PROP;
	}
	//---------------------------------------------------------------
	//No encontrado en el .racp
	if((StrPtr=strstr(StrTemp,"POST COHERENT INTEGRATIONS="))==StrTemp)//Busca POST COHERENT INTEGRATIONS en el .racp
	{
		printf("La interfaz no realizará integraciones coherentes posteriores\n");
		goto fin_PROP;
	}
	//---------------------------------------------------------------
	//No encontrado en el .racp
	if((StrPtr=strstr(StrTemp,"DEFLIP DATA="))==StrTemp)//Busca DEFLIP DATA en el .racp
	{
		printf("No implementado\n");
		goto fin_PROP;
	}
	//---------------------------------------------------------------
	//No encontrado en el raw.racp/Encontrado en spectra.racp
	if((StrPtr=strstr(StrTemp,"FFTPOINTS="))==StrTemp) //Busca FFTPOINTS en el .racp
	{
		printf("No se realizará integracinoes incoherentes y no necesitaremos FFTPoints\n");
		goto fin_PROP;
	}
	//---------------------------------------------------------------
	//No encontrado en el raw.racp/Encontrado en spectra.racp
	if((StrPtr=strstr(StrTemp,"INCOHERENT INTEGRATIONS="))==StrTemp)//Busca INCOHERENT INTEGRATIONS en el .racp
	{
		printf("No se realizarán integraciones incoherentes\n");
		goto fin_PROP;
	}
	//---------------------------------------------------------------
	//No encontrado en el raw.racp/Encontrado en spectra.pdata
	if((StrPtr=strstr(StrTemp,"SAVE CHANNELS DC="))==StrTemp)//Busca SAVE CHANNELS DC en el .racp
	{
		//YES/NO
		printf("No se utilizará Save Channels DC ya que no habrán combinaciones espectrales.\n");
		goto fin_PROP;
	}
	//---------------------------------------------------------------
	//No encontrado en el raw.racp/Encontrado en spectral.racp
	if((StrPtr=strstr(StrTemp,"TOTAL SPECTRAL COMBINATIONS="))==StrTemp)//Busca TOTAL SPECTRAL COMBINATIONS en el .racp
	{
		printf("No se realizarán combinaciones espectrales\n");
		goto fin_PROP;
	}
	//---------------------------------------------------------------
	//No encontrado en el raw.racp/Encontrado en spectral.racp
	if((StrPtr=strstr(StrTemp,"SPEC_COMB"))==StrTemp)//Busca SPEC_COMB en el .racp (#,#)
	{
		printf("No se realizarán tomarán los SPEC_COMB\n");
		goto fin_PROP;
    }

fin_PROP:
		return true;
}
//-----------------------------------------------------------------------

bool ExpParameters::ReadJARSParameters(FILE* RACPFile,char* StrTemp)
{
	char* StrPtr;
	//---------------------------------------------------------------
	//Asigna valor a los m_nNumberOfChannel del .racp
	if ((StrPtr=strstr(StrTemp,"Channel(0)="))==StrTemp)
	{	MySystemParam.m_nNumberOfChannel[0]=(unsigned int)atoi(FindValue(StrPtr));
		goto fin_JARSP; }

	if ((StrPtr=strstr(StrTemp,"Channel(1)="))==StrTemp)
	{	MySystemParam.m_nNumberOfChannel[1]=(unsigned int)atoi(FindValue(StrPtr));
		goto fin_JARSP; }

	if ((StrPtr=strstr(StrTemp,"Channel(2)="))==StrTemp)
	{	MySystemParam.m_nNumberOfChannel[2]=(unsigned int)atoi(FindValue(StrPtr));
		goto fin_JARSP; }

	if ((StrPtr=strstr(StrTemp,"Channel(3)="))==StrTemp)
	{	MySystemParam.m_nNumberOfChannel[3]=(unsigned int)atoi(FindValue(StrPtr));
		goto fin_JARSP; }

	if ((StrPtr=strstr(StrTemp,"Channel(4)="))==StrTemp)
	{	MySystemParam.m_nNumberOfChannel[4]=(unsigned int)atoi(FindValue(StrPtr));
		goto fin_JARSP; }

	if ((StrPtr=strstr(StrTemp,"Channel(5)="))==StrTemp)
	{	MySystemParam.m_nNumberOfChannel[5]=(unsigned int)atoi(FindValue(StrPtr));
		goto fin_JARSP; }

		if ((StrPtr=strstr(StrTemp,"Channel(6)="))==StrTemp)
	{	MySystemParam.m_nNumberOfChannel[6]=(unsigned int)atoi(FindValue(StrPtr));
		goto fin_JARSP; }

	if ((StrPtr=strstr(StrTemp,"Channel(7)="))==StrTemp)
	{	MySystemParam.m_nNumberOfChannel[7]=(unsigned int)atoi(FindValue(StrPtr));
		goto fin_JARSP; }
//--------------------------------------------------------------------------------------------
	if ((StrPtr=strstr(StrTemp,"JARS_FILTER"))==StrTemp)
	{
		strcpy(MySystemParam.m_sFilterPath,FindValue(StrPtr));
		for(int i=0; ;i++)
			if(*(MySystemParam.m_sFilterPath+i)=='\0')//borra el salto de linea
			{	*(MySystemParam.m_sFilterPath+i-1)=(char)(NULL);
				break;}
		printf("El valor del path del filtro es: %s \n",MySystemParam.m_sFilterPath);
		goto fin_JARSP;}

fin_JARSP:
	return true;
}

