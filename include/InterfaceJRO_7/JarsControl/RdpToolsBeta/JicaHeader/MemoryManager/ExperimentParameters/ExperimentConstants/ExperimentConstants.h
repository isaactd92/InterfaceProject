/*
 * ExperimentConstants.h
 *
 *  Created on: Dec 13, 2019
 *      Author: itupac
 */

#ifndef ARGPROCESSOR_JARSCONTROL_RDPTOOLSBETA_JICAHEADER_MEMORYMANAGER_EXPERIMENTPARAMETER_EXPERIMENTCONSTANTS_EXPERIMENTCONSTANTS_H_
#define ARGPROCESSOR_JARSCONTROL_RDPTOOLSBETA_JICAHEADER_MEMORYMANAGER_EXPERIMENTPARAMETER_EXPERIMENTCONSTANTS_EXPERIMENTCONSTANTS_H_

/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
This file has the constants and structures used in Jicamarca Experiments
Taken from RdpTools of Pablo Reyes and Karim Kuyeng
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/


#include <stdlib.h>                     //usr/include
#include <sys/timeb.h>			// tampoco existe en el ubuntu de jhon ubuntu 16.04
#include <time.h>			//usr/include

#define _MAX_FNAME			260 //!!!Usado en JICAHEADER.h!!!!
#define _MAX_NAME_PATH                  260
#define _MAX_PATH		     	260

#define EXP_RAWDATA			0
#define EXP_SPECTRA			1

/********Constantes para las lineas 5 y 6************/
#define NONE				0
#define FLIP				1 //Usado en ¡¡JicaHeader!!
#define SAMPLING			3
#define SYNCHRO				5 //definido en JarsControl
/*****************************/

/********Constantes para Tipo de Código	************/
#define CODE_NONE			0
#define CODE_USERDEFINE			1
#define CODE_BARKER2			2
#define CODE_BARKER3			3
#define CODE_BARKER4			4
#define CODE_BARKER5			5
#define CODE_BARKER7			6
#define CODE_BARKER11			7
#define CODE_BARKER13			8
#define CODE_AC128			9
#define CODE_COMPLEMENTARYCODE2		10
#define CODE_COMPLEMENTARYCODE4		11
#define CODE_COMPLEMENTARYCODE8		12
#define CODE_COMPLEMENTARYCODE16	13
#define CODE_COMPLEMENTARYCODE32	14
#define CODE_COMPLEMENTARYCODE64	15
#define CODE_COMPLEMENTARYCODE128	16
#define CODE_BINARY28			17
/*****************************/

//Decoding
#define DECODING_TIME_DOMAIN		0x00000001
#define DECODING_FREQ_DOMAIN		0x00000002
#define DECODING_INV_FREQ_DOMAIN	0x00000004

/*****************************/
#define FLAG_COHERENT_INTEGRATION	0x00000001
#define FLAG_SPECTRA_CALC		0x00000004
#define FLAG_INCOHERENT_INTEGRATION	0x00000008
#define FLAG_POST_COHERENT_INTEGRATION	0x00000010
#define FLAG_DATATYPE_SHORT		0x00000080
#define FLAG_DATATYPE_FLOAT		0x00000400
#define FLAG_DATAARRANGE_CONTIGUOUS_CH	0x00001000
#define FLAG_DATAARRANGE_CONTIGUOUS_H	0x00002000
#define FLAG_DATAARRANGE_CONTIGUOUS_P	0x00004000
#define FLAG_SAVE_CHANNELS_DC		0x00008000
#define FLAG_DEFLIP_DATA		0x00010000
#define FLAG_DEFINE_PROCESS_CODE	0x00020000
#define FLAG_INVFFTDECODING		0x00040000

/*-: Data Mode :-*/
#define DEMO_16BIT_CMPLX_MODE		0
#define DEMO_20BIT_CMPLX_MODE		1
#define DEMO_16BIT_REAL_MODE		2
//#define DEMO_16BIT_DLY_CMPLX_MODE	4(no usado)

//RESYNCHRO METHODS
#define RESYNCHRO_RESET_RC		1
#define RESYNCHRO_WAIT_LASTSAMPLE	2

//EXTERNAL GATE STATE
#define EXTGATE_DISABLED		0
#define EXTGATE_ENABLED			1

//EXTERNAL GATE POLARIZATION
#define ACTIVE_HIGH			0
#define ACTIVE_LOW			1

/*******************************************************************************************/
//PAQUETE "CHIP"
/*******************************************************************************************/
#define		CHIP_ALL		0x00 //Usado en JARSControl
#define		CHIP_COMAND		0x02  //Usado en JARSControl

/*******************************************************************************************/
//PAQUETE "CMD"
/*******************************************************************************************/
#define		RST_CTRL		0x05  //Usado en JARSControl
#define		MODE_OPER		0x03
#define		ELECTION_RXD		0x04
#define		POWER_RXD		0x06
#define		PROG_RXD		0x09

/*******************************************************************************************/
//PAQUETE "CONTROL PORT & DATA PORT"
/*******************************************************************************************/
#define		BY_CMD			0x00   //Usado en JARSControl
#define		BY_DATA			0x01  //Usado en JARSControl

//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//Estructuras para el manejo del experimento
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

	typedef struct _BasicHeader
	{	unsigned int nHeaderTotallength;  // para el primer bloque
		unsigned int nHeaderBasiclength; // para los bloques restantes
		unsigned short nHeaderVer; //Inicializado en !!JicaHeader!! como 1103
		unsigned int nBlockCounter;//Inicializado en !!JicaHeader!! como 0
		struct timeb FullTime; //USado en JicaHeader!!
		unsigned int nError; //Inicializado en !!JicaHeader!! como 0
	}BasicHeader;
//---------------------------------------------------------
	typedef struct _SystemParam
	{	unsigned int m_nHeader_Sys_Length; //Usado en !!JicaHeader!!
		unsigned int m_nSamples;//total de numero de muestras
		unsigned int m_nProfiles;//Asigna el valor de ACQUIRED PROFILES
		unsigned int m_nChannels;//Numero de canales del .racp
		unsigned int m_nADCResolution;//Valor del ADC Resolution
		unsigned int m_nPCIDIOBusWidth;//Valor del AnchoDeBus del PCI
		/*******************Agregado por Marcos el 03Nov2009 (2 de) ***********/
		//**************** No son guardados en el header-Preguntar si es necesario guardarlos ********************
		char m_sFilterPath[_MAX_PATH];//guarda el valor de JARS_FILTER del .racp//guarda la direccion donde se encuentra la configuracion del filtroç
		/*******Parametros de JARS*******/
		unsigned int m_nNumberOfChannel[8];//guarda los valores de los 8 canales
/********************************************/
	} SystemParam;
//---------------------------------------------------------
	typedef struct _RadarControllerParam
	{	unsigned int	m_nHeader_RC_length;//Definido en ¡¡JicaHeader!!
		unsigned int	m_nEspType;//0:normal,1:Join //NumeroDeExperimento
		unsigned int	m_nNTX; //Nos da el valor de TX ( el numero de IPPs)
		float		m_fIPP; //El valor del Ipp es definido por el .racp
		float		m_fTXA; //Nos da el ancho de pulso de transmision por A
		float		m_fTXB;//Nos da el ancho de pulso de transmision por B
		unsigned int	m_nNum_Windows; //numero de ventanas dado por el .racp
		unsigned int	m_nNum_Taus;//Asigna el valor de Number of Taus del .racp
		unsigned int	m_nCodeType; //Tipo de codigo del controlador de radar Ej:BARKER2 //Agregado 31/10		
		unsigned int	m_nLine6_Function;//Asigna el valor de 3 del .racp
		unsigned int	m_nLine5_Function;//Asigna el valor de 3 del .racp
		float		m_fCLOCK;//Obtiene el valor de la variable RELOJ del .racp
		unsigned int	m_nPrePulseBefore;//inicializados con 0
		unsigned int	m_nPrePulseAfter;//inicializados con 0
		char		m_sRango_IPP[20];//Asigna el valor del PULSE SELECTION_TXA= del .racp
		char		m_sRango_TXA[20];//Asigna el valor del PULSE SELECTION_TXA= del .racp
		char		m_sRango_TXB[20];//Asigna el valor del PULSE SELECTION_TXB== del .racp
	} RadarControllerParam;
//---------------------------------------------------------
	typedef struct _ProcessParam
	{	unsigned int	m_nHeader_PP_Length;//Definido en ¡¡Jicaheader!!
		unsigned int	m_nExperimentType;//Guarda el valor de EXPERIMENT TYPE.Puede ser EXP_SPECTRA(1) o EXP_RAWDATA (0)
		unsigned int	m_nSizeOfDataBlock;//Inicializado con 0
		unsigned int	m_nFFTorProfilesperBlock;//en Spectra = FFT,//Toma el valor de m_nProfilesperBlock (PROFILES PER BLOCK) del .racp.		
		unsigned int	m_nDataBlocksperFile;//Guarda el valor de raw data blocks o process data blocks en m_nDataBlocksperFile
		unsigned int	m_nData_Windows;//Inicializado con 1//igual al numero de ventanas en rdptools
		unsigned int	m_nProcessFlags;//Asigna valores a este flag dependiendo si el datatype es short o float o el data arrange es CONTIGUOUS_H,CONTIGUOUS_CH o CONTIGUOUS_P
		unsigned int	m_nCoherentIntegrations;//Asigna el valor de COHERENT INTEGRATIONS del .racp
		unsigned int	m_nIncoherentIntegrations;//Toma el valor de INCOHERENT INTEGRATIONS del .racp
		unsigned int	m_nTotalSpectra;//si 0.....no hay m_nSpectraCombinations ////Toma el valor de TOTAL SPECTRAL COMBINATIONS  del .racp		
		struct tm m_BeginTime; // estructura q almacena la hora y minutos del *.Racp para el inicio
		struct tm m_EndTime; // estructura q almacena la hora y minutos del *.Racp para el final
	} ProcessParam;
//---------------------------------------------------------
	typedef struct _GlobPParam //Parametros extras que presenta el ProcessParam
	{	ProcessParam	FixPP;                 //
		bool 		bCreateDayDir; // nos da el valor true or false (yes or no) de CREATE DIRECTORY PER DAY del .racp
		bool 		bNameInDirectory;// nos da el valor de INCLUDE EXPNAME IN DIRECTORY del .racp true or false (yes or no)
		bool 		m_bBeginOnStart;//Asigna el valor de BEGIN ON START del .racp
		unsigned char	*Dyn_nSpectraComb; //SPEC_COMB		
		unsigned char	*Dyn_nCardSequence;//arreglo del tamaño del numero de cards.Guarda el valor de cada tarjeta en el arreglo
		char 		m_sExpName[_MAX_PATH];//Nos da el nombre del experimento obtenido de .racp.EJmp: CH1-4
		char 		m_sBaseDirectory[_MAX_PATH]; //Nos da la direccion del .racp /media/ROJ/JARS2_0/Data/TEST_49_92MHZ_1MHZ/
		char 		m_sCurrentDirectory[_MAX_PATH];//Usado en Jicaheader!
		float*		Dyn_sfDataH0;//inicializado como null//toma valor en RdpToolsBeta
		float*  	Dyn_sfDataDH;//inicializado como null//toma valor en RdpToolsBeta
		unsigned int	m_nFFTPoints;//aumentado 7/1/04		
		unsigned int	m_nProfilesperBlock;//aumentado 7/1/04  //Asigna el valor de PROFILES PER BLOCK del .racp
		unsigned int	m_nCards;//aumentado 12/1/04  //Numero de cartas
		unsigned int 	m_nHeaderVER; //Definido dentro de ¡¡JicaHeader!!
		unsigned int 	m_nCohIntStride;//Toma el valor de COHERENT INTEGRATION STRIDE del .racp
			//unsigned int m_nCollectionMode;//0 si ECHOTEK COLLECTION MODE= GATE_CONTINUOUS_MODE y 1 si ECHOTEK COLLECTION MODE=COUNTED_BURST_MODE(echotek)
		unsigned int	m_nProcessCodes;//Nos da el valor de NUMBER OF PROCESS CODES del .racp	
		unsigned int 	m_nProcessBauds;//Nos da el valor de NUMBER OF PROCESS BAUDS del .racp		
		unsigned int 	m_nProcessWindows;//inicilizado como 0//GlobPParam////m_nProcessWindows aún no definida hasta rdp
		unsigned int 	m_nDecodingFlags;//Toma valor dependiendo del  DECODING TYPE
		unsigned int 	m_nProf2Join;// Nombrado en JicaHeader
		unsigned int	m_nFileSetDigits;//NOmbrado en Jicaheader
		unsigned int 	*Dyn_snDataNSA;//Inicializado como null****//toma valor en RdpTools = new int [m_ndata_windows]

	}GlobPParam;
//---------------------------------------------------------
	typedef struct _GlobRCParam
	{	RadarControllerParam	FixRCP;//Asignamos valores a los atributos del controladores de radar
		char	**Dyn_snCode;//Definido en ¡¡RdpToolsBeta!!//guarda el valor del código		
		float*	Dyn_sfAcqH0; //array donde guarda el valor de H0 dado por el .racp (altura inicial)
		float*  Dyn_sfAcqDH;//array donde guarda el valor de DH dado por el .racp (resolucion)
		float *Dyn_sfTau; //arrray que guarda los valores de tau del archivo .racp
		float*	Dyn_sfL5_AcqH0; //SAMPLING WINDOWS (LINE 5)//array que guarda los valores de H0
		float*  Dyn_sfL5_AcqDH;//SAMPLING WINDOWS (LINE 5)//array que guarda los valores de DH
		float*	Dyn_sfL6_AcqH0;//SAMPLING WINDOWS (LINE 6)//array que guarda los valores de H0
		float*  Dyn_sfL6_AcqDH;//SAMPLING WINDOWS (LINE 6)//array que guarda los valores de DH
		unsigned int	m_nNum_Codes;//guarda el valor de numero de codigos del .racp
		unsigned int	m_nBauds;//Ancho de codigo (code width) del .racp //32		
		unsigned int *Dyn_snAcqNSA;//array donde guarda el valor de NSA dado por el .racp(número de muestras a adquirir)
		unsigned int	m_nL5_NumWindows;//Asigna el valor de SAMPLING WINDOWS (LINE 5) del .racp
		unsigned int *Dyn_snL5_AcqNSA;//SAMPLING WINDOWS (LINE 5)//array que guarda los valores de NSA
		unsigned int	m_nL6_NumWindows;//Asigna el valor de SAMPLING WINDOWS (LINE 6) del .racp
		unsigned int *Dyn_snL6_AcqNSA;//SAMPLING WINDOWS (LINE 6)
		unsigned int	m_nFLIP1;//Usado en Jicaheader!
		unsigned int	m_nFLIP2;//Usado en Jicaheader!
	}GlobRCParam;

#endif 

