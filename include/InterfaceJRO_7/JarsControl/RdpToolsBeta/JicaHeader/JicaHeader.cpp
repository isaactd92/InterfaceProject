#include "JicaHeader.h"

#include <sys/timeb.h>

#include <sys/stat.h>
#include <dirent.h>
#include <vector>
#include <algorithm>

#include </usr/include/python2.7/Python.h>

char* concat(char *dest, const char *src);

//--------------------Jicaheder define las atributos de la cabeceras necesarios solo para exp. tipo raw data--------------------

JicaHeader::JicaHeader()
{	FileCount=0;
	DirectorySetCount=0;
	Flag=true;//01
	//Initialize the Basic Header
	MyBasicHeader.nHeaderTotallength=24+24+116+12+40+12;//tamaño en bytes
	MyBasicHeader.nHeaderBasiclength=24;//tamaño en bytes
	MyBasicHeader.nHeaderVer=1103;
	MyBasicHeader.nBlockCounter=0;
	MyBasicHeader.nError=0;
	CloseSystemTime=false;
	TimeHourBefore=0;

}
JicaHeader::~JicaHeader()
{}


//---------------------------------------------------------
//Define los tamaños de las cabeceras en bytes : Basic Header y Long Header
//Para guardar los atributos establecidos de long header en espacios de memoria del arreglo LongHeader
void JicaHeader::GetExpParameters(ExpParameters* MyExpParam)
{
	printf("*****************************************************************************************\n");        
	printf("Estoy al principio de GetExpParameters de JicaHeader........\n");
	//Toma valor definidos en ExpParameters y los guarda en los objetos referenciados en JicaHeader.h
	MyBasicHeader.nBlockCounter = 0; // 07/03/2018 para reiniciar cuando hay trying restart
	MySystemParam = &MyExpParam->MySystemParam;// Puntero al objeto MySystemParam.
	MySystemParam->m_nHeader_Sys_Length = 24; //Da valor a m_nHeader_Sys_Length, atributo de la struct SystemParam
	MyGPParam = &MyExpParam->MyGPParam;//Puntero al objeto MyGPParam dentro de ExpPameters
	MyGRCParam = &MyExpParam->MyGRCParam;//Puntero al objeto MyGRCParam dentro de ExpPameters
    //header version 1103
	MyGPParam->m_nHeaderVER = MyBasicHeader.nHeaderVer;//1103 definido en el contructor//MyGPParam nuevo objeto tipo GlobPParam definido en JicaHeader.h
	//----------------------------------------------------------------------------------------------------------------//
	//sizeof te da el numero en bytes
	//Tamaño del header de ProcessParameter y algunos atributos más
	MyGPParam->FixPP.m_nHeader_PP_Length=
					//tamanho de parametros de FixPP que van en el header
						sizeof(MyGPParam->FixPP)//FixPP-ProcessParameter
						-sizeof(MyGPParam->FixPP.m_BeginTime)
						-sizeof(MyGPParam->FixPP.m_EndTime)
						//tamanho del arreglo que contiene los pares a combinar
						+2*MyGPParam->FixPP.m_nTotalSpectra*sizeof(unsigned char)					
						//tamanho de Dyn_sfDataH0 + Dyn_sfDataDH + Dyn_snDataNSA de
						//todas las ventanas
						+12*MyGPParam->FixPP.m_nData_Windows//float=4bytes; como son 3 H0,NSA y DH. m_nData_Window=1
						//si esta defindo FLAG_DEFINE_PROCESS_CODE, suma el tamanho
						//necesario
						+( ((MyGPParam->FixPP.m_nProcessFlags
						& FLAG_DEFINE_PROCESS_CODE)==FLAG_DEFINE_PROCESS_CODE)?//NUMBER OF PROCESS CODES del .racp es igual a m_nProcessFlags
						2*sizeof(unsigned long):0 );
						//tamanho de codigos
						+MyGPParam->m_nProcessBauds //m_nProcessBauds pertenece a GlobPPParam
						*MyGPParam->m_nProcessCodes //m_nProcessCode pertenece a GlobPPParam
						*sizeof(float);
	//------------------------------------------------------------------------------------------------------------//
	//Define el tamaño de la cabecera en bytes del RadarControllerParam
	//Ocupa menos espacio de memoria con char
	unsigned char m_nNumLongs4Code=(unsigned char)ceil(((double)MyGRCParam->m_nBauds)/32.0); //Problema(¿Para qué lo transforma a char)
	MyGRCParam->FixRCP.m_nHeader_RC_length=
					//tamanho de parmetros de FixPP que van en el header
						sizeof(MyGRCParam->FixRCP)
					//tamanho de Dyn_sfAcqH0 + Dyn_sfAcqDH + Dyn_snAcqNSA de
					//todas las ventanas
						+12*MyGRCParam->FixRCP.m_nNum_Windows //12*1/3
					//tamanho del arreglo que contiene los taus (Dyn_sfTau)
						+MyGRCParam->FixRCP.m_nNum_Taus*sizeof(float)
					//tamanho de m_nNumCodes + m_nBauds + arreglo de codigos
						+(8+4*MyGRCParam->m_nNum_Codes*m_nNumLongs4Code)*((MyGRCParam->FixRCP.m_nCodeType!=CODE_NONE)?1:0)
					//tamanho de m_nFLIP1
						+((MyGRCParam->FixRCP.m_nLine5_Function==FLIP)?sizeof(unsigned long):0)
					//tamanho de m_nFLIP2
						+((MyGRCParam->FixRCP.m_nLine6_Function==FLIP)?sizeof(unsigned long):0);
	//--------------------------------------------------------
	//short header
	//Da valor al tamaño de cabecera del basic header en bytes)
	MyBasicHeader.nHeaderBasiclength =
					//tamanho de la variable con la longitud del shortheader (nHeaderBasiclength)
						sizeof(MyBasicHeader.nHeaderBasiclength)
					//tamanho de varible con la version del header
						+sizeof(MyBasicHeader.nHeaderVer)
					//tamanho de la variable con el numero del bloque actual
						+sizeof(MyBasicHeader.nBlockCounter)
					//tamanho de la estructura de tiempo
						+10//time //10 bytes
					//tamanho del contador de error
						+sizeof(MyBasicHeader.nError);
	//--------------------------------------------------------
	//long header
	//Define el tamaño total de la cabecera:long header + Basic header
	MyBasicHeader.nHeaderTotallength=
					//tamanho del short header
						MyBasicHeader.nHeaderBasiclength //basic header tamaño requerido en bytes
					//Parece que es para el Long Header
					//tamanho de los parametros del sistema
						+MySystemParam->m_nHeader_Sys_Length //atributo de la struct SystemParam (24)
					//tamanho los parametros del RC
						+MyGRCParam->FixRCP.m_nHeader_RC_length//Define el tamaño de la cabecera en bytes del RadarControllerParam
					//tamanho de los parametros globales
						+MyGPParam->FixPP.m_nHeader_PP_Length;///Tamaño del header de  ProcessParameter y algunos atributos más en bytes
	//--------------------------------------------------------
	printf("El valor de MyBasicHeader.nHeaderTotallength es: %d\n",MyBasicHeader.nHeaderTotallength);
	//BYTE * ActualPosition;
	unsigned char * ActualPosition;
	size_t size;
	LongHeader = malloc(MyBasicHeader.nHeaderTotallength-MyBasicHeader.nHeaderBasiclength);
	ActualPosition = (unsigned char*)LongHeader;//char = 1 byte
   	//Los "size" atributos de MySystemParam son guardados en ActualPosition de 1 byte en un byte
	size = MySystemParam->m_nHeader_Sys_Length; //size=24
	memcpy(ActualPosition,(void*)MySystemParam,size);//¿Por qué usa tipo void*? ¿Será porq es const?
	ActualPosition += size;
	//--------------------------------------------------------
	//Los atributos de FixRCP los guarda en Actual Postion
	size = sizeof(MyGRCParam->FixRCP);
	memcpy(ActualPosition,(void*)(&MyGRCParam->FixRCP),size);// Usa & ya q necesita de la dirección como parámetro
	ActualPosition += size;
	//--------------------------------------------------------
	printf("Estoy antes de los for........\n");
	//printf("El valor de MyGRCParam->FixRCP.m_nNum_Windows es: %d \n",MyGRCParam->FixRCP.m_nNum_Windows);
	//Los valores de los atributos de la resolucion, la altura y el numero de muestras son guardados
	//printf("El valor de MyGRCParam->Dyn_sfAcqH0[0] es: %f \n",MyGRCParam->Dyn_sfAcqH0[0]);	
	for(size_t i=0;i<MyGRCParam->FixRCP.m_nNum_Windows;i++)
	{
		size = sizeof(MyGRCParam->Dyn_sfAcqH0[i]);
		memcpy(ActualPosition,(void*)(&MyGRCParam->Dyn_sfAcqH0[i]),size);//Guarda el valor de Dyn_sfAcqH0 en memoria
		ActualPosition+=size;
		printf("El valor de H0 para RadarControllerParameters es: %f\n",MyGRCParam->Dyn_sfAcqH0[i]);

		size = sizeof(MyGRCParam->Dyn_sfAcqDH[i]);
		memcpy(ActualPosition,(void*)(&MyGRCParam->Dyn_sfAcqDH[i]),size);//Guarda el valor de Dyn_sfAcqDH en memoria
		ActualPosition+=size;
		printf("El valor de DH para RadarControllerParameters es: %f\n",MyGRCParam->Dyn_sfAcqDH[i]);

		size = sizeof(MyGRCParam->Dyn_snAcqNSA[i]);
		memcpy(ActualPosition,(void*)(&MyGRCParam->Dyn_snAcqNSA[i]),size);//Guarda el valor de Dyn_snAcqNSA en memoria
		ActualPosition+=size;
		printf("El valor de NSA para RadarControllerParameters es: %d\n",MyGRCParam->Dyn_snAcqNSA[i]);
	}
	printf("Estoy despues del primer for........\n");
	//---------------------------------------------------------
	//Guarda el valor de Tau de RadarControllerParam en la memoria asignada con ActualPosition
	for(size_t i=0;i<MyGRCParam->FixRCP.m_nNum_Taus;i++)
	{
		size = sizeof(MyGRCParam->Dyn_sfTau[i]);
		memcpy(ActualPosition,(void*)&MyGRCParam->Dyn_sfTau[i],size);
		ActualPosition+=size;
	}
	//----------------------------------------------------------
	//Si esta se
	if(MyGRCParam->FixRCP.m_nCodeType!=CODE_NONE)//Si tiene algun valor de Code Type
	{	//Guarda el valor de m_nNum_Codes en la posición que sigue del arreglo
		size = sizeof(MyGRCParam->m_nNum_Codes);
		memcpy(ActualPosition,(void*)&MyGRCParam->m_nNum_Codes,size);
		ActualPosition+=size;
		//Guarda el valor de m_nBauds en la posicion que sigue del arreglo
		size = sizeof(MyGRCParam->m_nBauds);
		memcpy(ActualPosition,(void*)&MyGRCParam->m_nBauds,size);
		ActualPosition+=size;
		//guarda los codigos (parece q los codigos de cabecera) averiguar Dyn_snCode
		unsigned long pot;
		unsigned int nPortion;
		//unsigned long** m_sHeaderCodeArray = new unsigned long*[MyGRCParam->m_nNum_Codes];
		unsigned int** m_sHeaderCodeArray = new unsigned int*[MyGRCParam->m_nNum_Codes];//crea un arreglo de num_codes elementos de arreglos de 32 elementos

		for(size_t i=0;i<MyGRCParam->m_nNum_Codes;i++)
		{
			nPortion=m_nNumLongs4Code-1;pot=1;//nPortion= 0 (si MyGRCParam->m_nBauds es 32)
			//m_sHeaderCodeArray[i] = new unsigned long[MyGRCParam->m_nBauds];
			m_sHeaderCodeArray[i] = new unsigned int[MyGRCParam->m_nBauds];
			//Da valor al primer elemento del cada elemento de m_sHeaderCodeArray
			for(size_t baud=0;baud<MyGRCParam->m_nBauds;baud++)//ancho de codigo
			{
				if(pot==1)//Siempre será 1
					m_sHeaderCodeArray[i][nPortion]=0;
				if(MyGRCParam->Dyn_snCode[i][MyGRCParam->m_nBauds-(baud+1)]>0)//Dyn_snCode no tiene valores?¿?(problema)
					m_sHeaderCodeArray[i][nPortion]+=pot;
				if(pot&0x80000000)
				{
					nPortion--;
					pot=1;
				}
				else pot*=2;
			}
		}
		size = sizeof(m_sHeaderCodeArray[0][0]);//da el numero de bytes a size

		for(size_t i=0;i<MyGRCParam->m_nNum_Codes;i++)
		{
			for(size_t j=0;j<m_nNumLongs4Code;j++)
			{
				memcpy(ActualPosition,(void*)&m_sHeaderCodeArray[i][j],size);//Guarda el primer elemento de cada arreglo de m_sHeaderCodeArray
				ActualPosition+=size;
			}
		}
	}	
	//--------------------------------------------------------
	//guarda el valor de m_nFLIP1 en ActualPosition
	if(MyGRCParam->FixRCP.m_nLine5_Function == FLIP)//m_nLine5_Function es 3 y flip es 1
	{
		size = sizeof(MyGRCParam->m_nFLIP1);//m_nFLIP1 no está definido (Problema)
		memcpy(ActualPosition,(void*)&MyGRCParam->m_nFLIP1,size);
		ActualPosition+=size;
	}
	//--------------------------------------------------------
	//Guarda el valor de m_nFLIP2 en ActualPosition
	if(MyGRCParam->FixRCP.m_nLine6_Function == FLIP)//m_nLine6_Function es 3 y flip es 1
	{
		size = sizeof(MyGRCParam->m_nFLIP2);//m_nFLIP2 no está definido (Problema)
		memcpy(ActualPosition,(void*)&MyGRCParam->m_nFLIP2,size);
		ActualPosition+=size;
	}
	//--------------------------------------------------------
	//Guarda los valores de los atributos establecidos en FixPP
	printf("El valor de MyGPParam->FixPP.m_nProfilesperBlock en Jicaheader es: %d\n",MyGPParam->m_nProfilesperBlock);
	printf("El valor de MyGPParam->m_nProf2Join en Jicaheader es: %d\n",MyGPParam->m_nProf2Join);
	printf("El valor de MyGPParam->FixPP.m_nFFTorProfilesperBlock en Jicaheader es: %d\n",MyGPParam->FixPP.m_nFFTorProfilesperBlock);
	size = sizeof(MyGPParam->FixPP)
		  -sizeof(MyGPParam->FixPP.m_BeginTime)
		  -sizeof(MyGPParam->FixPP.m_EndTime);
	if(MyGPParam->m_nProf2Join > 1)//m_nProf2Join No está definido (problema)
	{
		MyGPParam->FixPP.m_nFFTorProfilesperBlock/=MyGPParam->m_nProf2Join;//m_nFFTorProfilesperBlock es el valor de PROFILES PER BLOCK y FFTPOINTS
		memcpy(ActualPosition,(void*)&MyGPParam->FixPP,size);
		MyGPParam->FixPP.m_nFFTorProfilesperBlock*=MyGPParam->m_nProf2Join;
	}
	else
	{//Guarda los valores de los atributos establecidos en FixPP sin alterar
		memcpy(ActualPosition,(void*)&MyGPParam->FixPP,size);
	}
	ActualPosition+=size;
	//--------------------------------------------------------
	//Guarda los valores de Dyn_sfDataH0, Dyn_sfDataDH y Dyn_snDataNSA dependiendo de la cantidad de ventanas
	for(size_t i=0;i<MyGPParam->FixPP.m_nData_Windows;i++)//m_nData_Windows inicializado con 1 pero no adquiere valor
	{
		size = sizeof(MyGPParam->Dyn_sfDataH0[i]);//
		memcpy(ActualPosition,(void*)&MyGPParam->Dyn_sfDataH0[i],size);
		ActualPosition+=size;
		printf("El valor de H0 para ProcessParameters es: %f\n",MyGPParam->Dyn_sfDataH0[i]);


		size = sizeof(MyGPParam->Dyn_sfDataDH[i]);//
		memcpy(ActualPosition,(void*)&MyGPParam->Dyn_sfDataDH[i],size);
		ActualPosition+=size;
		printf("El valor de DH para ProcessParameters es: %f\n",MyGPParam->Dyn_sfDataDH[i]);

		size = sizeof(MyGPParam->Dyn_snDataNSA[i]);//
		if(MyGPParam->m_nProf2Join > 1)//m_nProf2Join aun no tiene valor//m_nProf2Join no adquiere valor. NO Encontrado, solo declarado en ExpConstant y 							implicitamente definido como 0
		{
			MyGPParam->Dyn_snDataNSA[i]*=MyGPParam->m_nProf2Join;
			memcpy(ActualPosition,(void*)&MyGPParam->Dyn_snDataNSA[i],size);
			MyGPParam->Dyn_snDataNSA[i]/=MyGPParam->m_nProf2Join;
		}
		else
		{
			memcpy(ActualPosition,(void*)&MyGPParam->Dyn_snDataNSA[i],size);
		}
		printf("El valor de NSA para ProcessParameters es: %d\n",MyGPParam->Dyn_snDataNSA[i]);
		ActualPosition+=size;
	}
	//------------------------------------------------------------------------
	//Guarda los valores de combinaciones espectrales
	size = sizeof(MyGPParam->Dyn_nSpectraComb[0]);
	for(size_t i=0;i<MyGPParam->FixPP.m_nTotalSpectra;i++)
	{
		memcpy(ActualPosition,(void*)&MyGPParam->Dyn_nSpectraComb[2*i],size);
		ActualPosition+=size;
		memcpy(ActualPosition,(void*)&MyGPParam->Dyn_nSpectraComb[2*i+1],size);
		ActualPosition+=size;
	}
	//--------------------------------------------------------
	//
	if(MyGPParam->FixPP.m_nProcessFlags&FLAG_DEFINE_PROCESS_CODE)
	{//m_nProcessCodes aduiere el valor de NUMBER OF PROCESS CODES
		size = sizeof(MyGPParam->m_nProcessCodes);
		memcpy(ActualPosition,(void*)&MyGPParam->m_nProcessCodes,size);
		ActualPosition+=size;
	 //	m_nProcessBauds adquiere el valor de NUMBER OF PROCESS BAUDS
		size = sizeof(MyGPParam->m_nProcessBauds);
		memcpy(ActualPosition,(void*)&MyGPParam->m_nProcessBauds,size);
		ActualPosition+=size;
	 //Guarda los valore de NUMBER OF PROCESS BAUDS del .racp
		size = sizeof(m_sCodeFloat[0][0]);
		for(size_t i=0;i<MyGPParam->m_nProcessCodes;i++)
		{
			for(size_t j=0;j<MyGPParam->m_nProcessBauds;j++)
			{
				memcpy(ActualPosition,(void*)&m_sCodeFloat[i][j],size);
				ActualPosition+=size;
			}
		}
	}
	//Establece la cantidad de segundos que faltan para acabar el experimento en TimeToFinish
	GetEndTime();
	printf("***************Estoy al final de GetExpParameters de JicaHeader*************\n");
	return;
}

//---------------------------------------------------------
//Establece la cantidad de segundos que faltan para acabar el experimento en el atributo TimeToFinish
void JicaHeader::GetEndTime()
{ 
  printf("Estoy al principio de GetEndTime de JicaHeader........\n");
  //valor donde se guarda el tiempo en la estructura tm
  struct tm CurrentTimeBuffer;
  //__timeb32 EndTimeBuffer_32;
  //timeb EndTimeBuffer_32 encontrado
  struct timeb EndTimeBuffer_32;
  //EndTimeBuffer nos da el total de segundos desde January 1, 1970
  time_t EndTimeBuffer;
  time (&EndTimeBuffer);
  //ftime nos da: time,millitm,timezone,dstflag.formato en tipo timeb
  //_ftime32_s(&EndTimeBuffer_32);
  ftime(&EndTimeBuffer_32);
  //local time nos da el valor de EndTimeBuffer expresado en la estructura tm.parametro tipo time_t
  //localtime_s(&CurrentTimeBuffer,&EndTimeBuffer);
  CurrentTimeBuffer = *localtime(&EndTimeBuffer);
  //Tiempo en segundos restantes para que acabe
  //segundos from 1970 + segundos restantes en hora + segundos restantes en minutos
  TimeToFinish=EndTimeBuffer_32.time+(MyGPParam->FixPP.m_EndTime.tm_hour-CurrentTimeBuffer.tm_hour)*3600//tm_hour valor de la hora del .racp
		+(MyGPParam->FixPP.m_EndTime.tm_min-CurrentTimeBuffer.tm_min)*60;//
  printf("El valor de TimeToFinish es: %d\n",TimeToFinish);
  //Del dia siguiente
  if(MyGPParam->FixPP.m_EndTime.tm_hour<=CurrentTimeBuffer.tm_hour)
      if(MyGPParam->FixPP.m_EndTime.tm_min<=CurrentTimeBuffer.tm_min)
	 TimeToFinish+=24*60*60; // al d\EDa siguiente  cantidad de segundos en 01 d\EDa
  return;
}

//------------------------------------------------------------------------------------------------
//Guarda los valores de los atributos de Basic Header en el arreglo buff_header de memory manager y nos devuelve la cantidad de bytes usados
int JicaHeader::GetBasicHeader(MemoryManager* MyMemManager)
{	size_t BytesWritten=0;
	size_t m_WByte;
	//_ftime32_s(&MyBasicHeader.FullTime);//Get the hour (long)
        ftime(&MyBasicHeader.FullTime);//Get the hour (long)
	//***************************************************
	//Save 24Bytes of Basic Header each variable
        //***************************************************
	int offset=0; m_WByte=4;//int= 4 bytes//m_WByte guarda el numero de bytes del tipo de palabra
	//if(MyBasicHeader.nBlockCounter==0)
	//Guarda el valor de Totallength (int) en buff_header (char) ya que char es de 1 byte y se puede contabilizar las posiciones con m_WByte
	memcpy(MyMemManager->buff_header+offset, &MyBasicHeader.nHeaderBasiclength,m_WByte);//Debería ser: nHeaderBasiclength, reemplazé 												    MyBasicHeader.nHeaderTotallength por MyBasicHeader.nHeaderBasiclength
	BytesWritten+=m_WByte;//Cantidad de bytes en uso

	offset+=m_WByte; m_WByte=2;//offset es para moverse en posicion
	memcpy(MyMemManager->buff_header+offset, &MyBasicHeader.nHeaderVer,m_WByte);
	BytesWritten+=m_WByte;

	offset+=m_WByte; m_WByte=4;
	memcpy(MyMemManager->buff_header+offset, &MyBasicHeader.nBlockCounter,m_WByte);
	BytesWritten+=m_WByte;

	//Saving Timeb Struct
	offset+=m_WByte; m_WByte=4;
	memcpy(MyMemManager->buff_header+offset, &MyBasicHeader.FullTime.time,m_WByte);
	BytesWritten+=m_WByte;

	offset+=m_WByte; m_WByte=2;
	memcpy(MyMemManager->buff_header+offset, &MyBasicHeader.FullTime.millitm,m_WByte);
	BytesWritten+=m_WByte;

	offset+=m_WByte; m_WByte=2;
	memcpy(MyMemManager->buff_header+offset, &MyBasicHeader.FullTime.timezone,m_WByte);
	BytesWritten+=m_WByte;

	offset+=m_WByte; m_WByte=2;
	memcpy(MyMemManager->buff_header+offset, &MyBasicHeader.FullTime.dstflag,m_WByte);
	BytesWritten+=m_WByte;

	offset+=m_WByte; m_WByte=4;
	memcpy(MyMemManager->buff_header+offset, &MyBasicHeader.nError,m_WByte);
	BytesWritten+=m_WByte;
	//*******************Pasado la hora, viendo timeb de FullTime**********************************
	if((TimeToFinish<MyBasicHeader.FullTime.time) && (MyGPParam->m_bBeginOnStart))  // CONDICION PARA FINALIZAR A UNA DETERMINADA HORA
		CloseSystemTime=true;//ya se paso la hora de fin
	return int(BytesWritten);
}

//---------------------------------------------------------
//Guarda los valores de los atributos de long header junto con los atributos de basic header en buff_header
int JicaHeader::GetFirstHeader(MemoryManager* MyMemManager)
{
	size_t BytesWritten=0;
	size_t m_WByte;
	BytesWritten=(size_t)GetBasicHeader(MyMemManager);

	m_WByte=MyBasicHeader.nHeaderTotallength-MyBasicHeader.nHeaderBasiclength;
	memcpy(MyMemManager->buff_header+BytesWritten,LongHeader,m_WByte);//No usa ActualPosition por el tipo de valor (se requiere *void)
	BytesWritten+=m_WByte;

	return int(BytesWritten);
}

//---------------------------------------------------------
//Los valores de los atributos de LongHeader y basic header escritos en MyFile los guardará en MyFileTemp.
//También guardará los valores de MyMemManager->m_mAcqBuffer[MyMemManager->numrawbuffs-1] y MyMemManager->m_mAcqBufferFloat[MyMemManager->numrawbuffs+MyMemManager->numspecbuffs-2]
//dependiendo del tipo de dato ( Escribe la data adquirida en los files dependiendo si es tipo short o tipo float)
//También guardará el valor de MyMemManager->m_nChannelsDCBufferFloat si FLAG_SAVE_CHANNELS_DC está activo
//********* Crea el directorio, el file con su respectivo formato .r, escribirá bloques de datos : cabeceras y perfiles
//********* y seguirá hasta el CTRL-C lo detenga. Es llamado del main mediante InternalCallback()

bool JicaHeader::SaveData(MemoryManager* MyMemManager)
{ 
  //printf("**************************************** Estoy dentro de SaveData () **********************\n");
  time_t long_time;
  time( &long_time );
  //localtime_s(&MyLocalTime,&long_time);
  MyLocalTime=*localtime(&long_time);
  char timeline[26];

    	if(MyBasicHeader.nBlockCounter==0 )
	{	FILE *MyFile;
		GetFileName();//crea carpetas en formato de data cruda//se guarda el nombre del file en FileSaved copn formato .r o .pdata
		if((MyFile=fopen(FileSaved,"wb"))==NULL)//crea el file de filesaved y se pregunta si esta mal abierto
			return false; //FileSaved es ruta
		//Regresa la cantidad de bytes guardados debido a long header y basic header y escribe los valores de los atributos en las cabeceras
		SaveFirstHeader(MyFile);

		MyFileTemp=MyFile;//los valores de los atributos de LongHeader y basic header en MyFile los guardará en MyFileTemp.
	}
	else
		SaveBasicHeader(MyFileTemp);//ya que hay más bloques guardados y cada bloque solo tiene el basic header
	//MyGPParam->FixPP.m_nFFTorProfilesperBlock;
	switch (MyGPParam->FixPP.m_nProcessFlags & 0xF80)
	{   //Escribe la data adquirida en m_mAcqBuffer en MyFileTemp
		case FLAG_DATATYPE_SHORT: //escribe los elementos de m_mAcqBuffer[MyMemManager->numrawbuffs-1] en MyFileTemp
		    { /*printf("--------------> Estoy dentro de case FLAG_DATATYPE_SHORT <--------------- \n");
			printf("El valor de MyMemManager->m_mAcqBuffer[0][0] real en FLAG_DATATYPE_SHORT es: %d\n",MyMemManager->m_mAcqBuffer[0][0].real());
			printf("El valor de MyMemManager->m_mAcqBuffer[0][0] imag FLAG_DATATYPE_SHORT es: %d\n",MyMemManager->m_mAcqBuffer[0][0].imag());
			printf("El valor de MyMemManager->numrawbuffs-1 es: %d\n",MyMemManager->numrawbuffs-1); */
 			
			size_t bytes_written = fwrite(MyMemManager->m_mAcqBuffer[MyMemManager->numrawbuffs-1]//*****IN********
				  ,sizeof(complex<short>)
				  ,MyMemManager->m_miSizeAcqBuffer[MyMemManager->numrawbuffs-1]
				  ,MyFileTemp);
			FILE * pFile;
			/*pFile = fopen ("/home/itupac/AQUI_ESTOY_BLOQUE.txt", "wb");
			fwrite(MyMemManager->m_mAcqBuffer[MyMemManager->numrawbuffs-1]//*****IN********
				  ,sizeof(complex<short>)
				  ,MyMemManager->m_miSizeAcqBuffer[MyMemManager->numrawbuffs-1]
				  ,pFile);
			fclose (pFile);*/

			if (bytes_written<1)
			{
			   printf("Error en la escritura del bloque de dato (SaveData)...\n");	
			}else{
			   //printf("La cantidad de bytes escritos son: %lu",bytes_written);	
			}
		      //******** Opción 1
		      //******** Pone el bloque de datos adquiridos a la salida del bloque en lainterfaz  gnuradio companion ******************
		     // gnuradio_out=MyMemManager->m_mAcqBuffer[MyMemManager->numrawbuffs-1];			
                        
		      //******** Opción 2
		      //********* Pasar por un socket gnuradio_out al main para que no se produzca perdida de datos a la salida de la interfaz.
		      //zmq::socket_t server_gnuradio(context,ZMQ_PUSH);
		      //server_gnuradio.bind("ipc://5555");
		     // size_t tamano_envio = (sizeof(complex<short>))*(MyMemManager->m_miSizeAcqBuffer[MyMemManager->numrawbuffs-1]);
		      //********* Prepara la data para ser enviada
		      //char gnuradio_crudo[tamano_envio]={};
		   // memcpy(gnuradio_crudo, MyMemManager->m_mAcqBuffer[MyMemManager->numrawbuffs-1], tamano_envio);//pasa a char* para que sea enviado por el socket

		      //size_t tamano = sizeof(gnuradio_crudo);//
		      //zmq::message_t reply_gnuradio(tamano);
		      //memcpy((void *) reply_gnuradio.data(), gnuradio_crudo, tamano);//
		      //server_gnuradio.send(reply_gnuradio);
                    }
		      break;
		
                default:

		      break;
	}
	
	sprintf(timeline,"%s",ctime(&(MyBasicHeader.FullTime.time)));
	printf("\r Block number : %i || %s\n",MyBasicHeader.nBlockCounter,timeline);//Nos da la fecha hora año
	fflush(stdout);//limpia el buffer de salida (USar \n por si no se puede)
   	//termina la escritura de los atributos de basic y long header al terminar en el tiempo de experimento
	//printf("El valor de RAW DATA BLOCKS es: %d\n",MyGPParam->FixPP.m_nDataBlocksperFile);
	//******** Para saber la cantidad de bloques dentro de un file. Comienza otro file al terminar el conteo / Ver si se usa cuando se envía una señal *** 	
	if(MyBasicHeader.nBlockCounter++==MyGPParam->FixPP.m_nDataBlocksperFile-1) //aumenta el contador de bloques en 1 y contabiliza todos los bloques a guardar //nBlockCounter++ primero se reemplaza con el valor de nBlockCounter y luego se suma ++
	{	MyBasicHeader.nBlockCounter=0;
		//printf("Termina de escribir..\n");
		fclose(MyFileTemp);//TErmina de escribir al terminar todos los bloques RAW DATA BLOCKS= m_nDataBlocksperFile
	}

	//Resetea y crea nuevos directorios y files cuando empieza la media noche
	if(MyLocalTime.tm_hour-TimeHourBefore<0) //si ya se paso de la media noche
	{	MyBasicHeader.nBlockCounter=0;
		fclose(MyFileTemp);
		FileCount=0;
		DirectorySetCount=0;	// 17/08/18 para reiniciar contador de directorios
	}
	TimeHourBefore=MyLocalTime.tm_hour;
	//printf("El valor de RAW DATA BLOCKS al final es: %d\n",MyGPParam->FixPP.m_nDataBlocksperFile);
	//printf("************************************ Estoy al final de SaveData () **********************\n");
	return true;
}


//-----------------------------------------------------------------------------
// Nos da el valor de bytes guardados debido a los atributos de MyBasicHeader
// Escribe los atributos (espacios guardados en memoria) de BasicHeader en el file MyFile
int JicaHeader::SaveBasicHeader(FILE* MyFile)
{	size_t BytesWritten=0;
	size_t m_WByte;
	//_ftime32_s(&MyBasicHeader.FullTime);//Get the hour
	ftime(&MyBasicHeader.FullTime);//Get the hour
	//***************************************************
	//*********** Save 24Bytes of Basic Header each variable
	//Guarda los atributos (espacios guardados en memoria) de BasicHeader en el file MyFile
	m_WByte=4;
	if(MyBasicHeader.nBlockCounter==0)
		BytesWritten+=fwrite(&MyBasicHeader.nHeaderTotallength,1,m_WByte,MyFile);//escribe el nHeaderTotallength (int=4bytes) en MyFile
	else
		BytesWritten+=fwrite(&MyBasicHeader.nHeaderBasiclength,1,m_WByte,MyFile);//escribe el nHeaderBasiclength (int=4bytes) en MyFile

	m_WByte=2;
	BytesWritten+=fwrite(&MyBasicHeader.nHeaderVer,1,m_WByte,MyFile);
	m_WByte=4;
	BytesWritten+=fwrite(&MyBasicHeader.nBlockCounter,1,m_WByte,MyFile);
	//Saving Timeb Struct//Tiempo actual ftime
	m_WByte=4;
	BytesWritten+=fwrite(&MyBasicHeader.FullTime.time,1,m_WByte,MyFile);
	m_WByte=2;
	BytesWritten+=fwrite(&MyBasicHeader.FullTime.millitm,1,m_WByte,MyFile);
	m_WByte=2;
	BytesWritten+=fwrite(&MyBasicHeader.FullTime.timezone,1,m_WByte,MyFile);
	m_WByte=2;
	BytesWritten+=fwrite(&MyBasicHeader.FullTime.dstflag,1,m_WByte,MyFile);
	m_WByte=4;
	BytesWritten+=fwrite(&MyBasicHeader.nError,1,m_WByte,MyFile);
	//*****************************************************
	if((TimeToFinish<MyBasicHeader.FullTime.time) && (MyGPParam->m_bBeginOnStart))  // CONDICION PARA FINALIZAR A UNA DETERMINADA HORA
		CloseSystemTime=true;// //ya se paso la hora
	return int(BytesWritten);
}

//---------------------------------------------------------
//Nos da la cantidad de bytes guardados en BytesWritten debido a longHeader y basic header (Total)
// Escribe los atributos (espacios guardados en memoria) de Long Header y BasicHeader en el file MyFile
int JicaHeader::SaveFirstHeader(FILE* MyFile)
{
	size_t BytesWritten=0;
//	size_t m_WByte;
	BytesWritten=(size_t)SaveBasicHeader(MyFile);//BytesWritten nos da la cantidad de bytes escritos del basicheader.
	//--------------------------------------------------------
	BytesWritten += fwrite(LongHeader,1//LongHeader a pointer of malloc ya definido para la parte long header de cabecera
						,MyBasicHeader.nHeaderTotallength-MyBasicHeader.nHeaderBasiclength//esta expresado en bytes
						,MyFile);//guarda los espacios de memoria de long header en MyFile
	//--------------------------------------------------------
	return int(BytesWritten);
}

//---------------------------------------------------------
//Obtiene el nombre (path) del file el .r o .pdata  donde se guardarán las muestras cuando no es hdf5 data.Este path será guardado en FileSaved
//MakeDirectory():Crea carpetas dependiendo si es por día y si quieren una carpeta con nombre del experimento dentro del directorio
//Si llega a los 1000 files creará otro carpeta
bool JicaHeader::GetFileName()
{
	time_t long_time;
	time(&long_time);
	//localtime_s(&MyLocalTime,&long_time);
    MyLocalTime = *localtime(&long_time);
    //Si son las 00 hrs
	if(Flag&&MyLocalTime.tm_hour==00)//Flag comienza como true
	{	Flag=false;
		FileCount=0;
	}
	//Se crea el archivo en formato Jicamarca, que puede ser .r o .pdata
	else{
	 if(FileCount==0)//No hay data
	    MakeDirectory(); //crea los directorios y carpetas donde se guardará
		//Crea el nombre del file
	 //printf("Se creó el directorio\n");	
	 sprintf(FileName,"D%.4d%.3d%.3d",MyLocalTime.tm_year+1900,MyLocalTime.tm_yday+1,FileCount);
	 //crea el nombre con la extensión y lo guarda el path en FileSaved
	 sprintf(FileSaved,"%s/%s",MyGPParam->m_sCurrentDirectory,FileName);//une el directorio con el nombre del archivo
	 strcat(FileSaved,".r");

	 FileCount++;// para el siguiente archivo
	 if(FileCount==1000)
	  { FileCount=0; // si llega a 1000 se crea un nuevo directorio
	    DirectorySetCount++; // contador de directorios
	  }

	 if(MyLocalTime.tm_hour==01)
	  Flag=true;

	 //PreDay = MyLocalTime.tm_yday;
	 printf("\r Saving data in file %s\n",FileSaved);
	 }

	return true;
}
//---------------------------------------------------------------------
//Nos da true si el número de caracteres de s es igual a suffix y si s es igual a suffix visto desde el final
bool has_suffix(const string& s, const string& suffix)
{
	return (s.size() >= suffix.size()) && equal(suffix.rbegin(), suffix.rend(), s.begin());
}			//numero de caracteres en el string sin considerar el null
			//revisa si los caracteres vistos desde el final del suffix son iguales a s

//---------------------------------------------------------------------------------------------------------
//Crea carpetas dependiendo si es por día y si quieren una carpeta con nombre del experimento dentro del directorio
//Guarda la direccion del directorio con las caracteristicas requeridas agregadas
void JicaHeader::MakeDirectory(void)
{
	printf("Estoy dentro de MakeDirectory....\n");
        time_t long_time;
	time( &long_time );
	//localtime_s(&MyLocalTime,&long_time);
  	MyLocalTime = *localtime(&long_time);

	char TempPatnhame[_MAX_PATH]; // arreglo temporal donde se grabara el directorio
	//TempPatnhame , lugar donde se guardara la direccion: el MyGPParam->m_sBaseDirectory,//Tipo %s: String of characters
	sprintf(TempPatnhame,"%s",MyGPParam->m_sBaseDirectory); // se copia el directorio base

//	printf("El valor del path es: %s\n",MyGPParam->m_sBaseDirectory);
//	printf("El strlen del path es: %d\n",strlen(MyGPParam->m_sBaseDirectory));
//	printf("El valor de MyGPParam->m_sBaseDirectory[strlen(MyGPParam->m_sBaseDirectory)-1] es: %c\n",MyGPParam->m_sBaseDirectory[strlen(MyGPParam->m_sBaseDirectory)-1]);

	//Si no está la última barra '/'(se borra en ExperimentParameter)
	//Revisar para cambiar el if
	if((MyGPParam->m_sBaseDirectory[strlen(MyGPParam->m_sBaseDirectory)-1]=='\\') ||
		(MyGPParam->m_sBaseDirectory[strlen(MyGPParam->m_sBaseDirectory)-1]!='/'))
			//se agrega // al final y tambien el '\0'
			concat(TempPatnhame,"/"); // se le agrega \\ para crear carpetas dentro
	//Crea una carpeta con el año y el día y dentro de la carpeta crea otras por nombre y por si tiene màs de 1000 archivos
	if(MyGPParam->bCreateDayDir) // si quieren tener un directorio por dia
	{	char sDaySubDir[_MAX_PATH]; //
		if(MyGPParam->bNameInDirectory) // si quieren el nombre del experimento dentro de la carpeta por dia y año
		{	//sprintf(sDaySubDir, "d%.4d%.3d\\",MyLocalTime.tm_year+1900,MyLocalTime.tm_yday+1); //creo la carpeta por dia
			//d:Signed decimal integer
			//***********crea carpeta con año y dia desde 1 de enero*********************************
			sprintf(sDaySubDir, "d%.4d%.3d/",MyLocalTime.tm_year+1900,MyLocalTime.tm_yday+1); //creo la carpeta por dia
			//strcat(TempPatnhame,sDaySubDir); //la a\F1ado a la base
			concat(TempPatnhame,sDaySubDir); //la a\F1ado a la base//agrega el año y el dia siguiente , lo cuales seran el nombre de la nueva carpeta
			//CreateDirectory(TempPatnhame,0); // creo carpeta
			mkdir(TempPatnhame,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); // creo carpeta con todos los permisos (read/write/search)

			char sExpName[_MAX_PATH];
			if(DirectorySetCount>0) // si es q hay m\E1s de una carpeta dentro le agrego el nombre del exp con un numero al costado
				//sprintf(sExpName, "%s_%.2d\\",MyGPParam->m_sExpName,DirectorySetCount);
				sprintf(sExpName, "%s_%.2d/",MyGPParam->m_sExpName,DirectorySetCount);
			else
				//sprintf(sExpName,"%s\\",MyGPParam->m_sExpName); // sino solo el nombre del exp
				sprintf(sExpName,"%s/",MyGPParam->m_sExpName); // sino solo el nombre del exp
			//strcat(TempPatnhame,sExpName);
			concat(TempPatnhame,sExpName);//
		}
		else // si no quieren el nombre del exp dentro del directorio
		{	if(DirectorySetCount>0)//hago lo mismo que en caso anterior pero ya no guardo el nombre del exp
				//sprintf(sDaySubDir,"d%.4d%.3d_%.2d\\",MyLocalTime.tm_year+1900,MyLocalTime.tm_yday+1,DirectorySetCount);
				sprintf(sDaySubDir,"d%.4d%.3d_%.2d/",MyLocalTime.tm_year+1900,MyLocalTime.tm_yday+1,DirectorySetCount);
			else
				//sprintf(sDaySubDir, "d%.4d%.3d\\",MyLocalTime.tm_year+1900,MyLocalTime.tm_yday+1);
				sprintf(sDaySubDir, "d%.4d%.3d/",MyLocalTime.tm_year+1900,MyLocalTime.tm_yday+1);//raw data
			//strcat(TempPatnhame,sDaySubDir);
			concat(TempPatnhame,sDaySubDir);
		}
		//CreateDirectory(TempPatnhame,0);
    	mkdir(TempPatnhame,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); // crea el directorio con todos los permisos (read/write/search)
	}//Si no quieren crear sus directorios por dìas.Tambièn denderá si quieren el nombre del experimento
	//
	else//si no quieren distribuir sus archivos de firectorio por d\EDa//Si no quieren el nombre del día en el directorio
	{	char sExpName[_MAX_PATH];
		if(MyGPParam->bNameInDirectory) // si quieren nombre del exp dentro de la carpeta
		{	if(DirectorySetCount>0)
				//sprintf(sExpName,"%s_%.2d\\",MyGPParam->m_sExpName,DirectorySetCount);
				sprintf(sExpName,"%s_%.2d/",MyGPParam->m_sExpName,DirectorySetCount);
			else
				//sprintf(sExpName, "%s\\",MyGPParam->m_sExpName);
				sprintf(sExpName, "%s/",MyGPParam->m_sExpName);
		}
		else // sino
		{	if(DirectorySetCount>0)
				//sprintf(sExpName, "_%.2d\\",DirectorySetCount);
				sprintf(sExpName, "_%.2d/",DirectorySetCount);
			else
				//sprintf(sExpName, "\\");
				sprintf(sExpName, "/");
		}
		//strcat(TempPatnhame,sExpName);
		concat(TempPatnhame,sExpName);
		//CreateDirectory(TempPatnhame,0);
    mkdir(TempPatnhame,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); // crea el directorio con todos los permisos (read/write/search)
	}
	strcpy(MyGPParam->m_sCurrentDirectory,TempPatnhame);
}
//adjunta al final de la dirrecciòn (path)
char* concat(char *dest, const char *src)
{
	size_t dest_len = strlen(dest);
	size_t i;
	for(i=0;src[i]!='\0';i++)//considerar i=1;
	{
		dest[dest_len + i] = src[i];
	}
	dest[dest_len + i] = '\0';

	return dest;
}

//---------------------------------------------------------
//Compara si la hora actual está dentro del rango de inicio y fin ( nos da Acquiring data right now! en pantalla)
bool JicaHeader::CompareTime()
{	

	printf("*****************************************************************************************************\n");	
	printf("************************************** Estoy dentro de CompareTime **********************************\n");
	if (!MyGPParam->m_bBeginOnStart)//m_bBeginOnStart = true or false (lo toma del .racp)
		return true;

	time_t Init_long_time;
    	time( &Init_long_time );
	//localtime_s(&MyLocalTime,&Init_long_time);
  	MyLocalTime = *localtime(&Init_long_time);
  	//system("cls");
	printf("The system will start at %2d:%2d hrs, currently it is %2d:%2d hrs\n",
		MyGPParam->FixPP.m_BeginTime.tm_hour,MyGPParam->FixPP.m_BeginTime.tm_min,//los valores del .racp de begin
		MyLocalTime.tm_hour,MyLocalTime.tm_min);//actual de local time


	if (MyGPParam->FixPP.m_BeginTime.tm_hour<=MyGPParam->FixPP.m_EndTime.tm_hour)//si la hora final del .racp es menor que la hora inicial del .racp //No 											       madrugada
	{	if( (MyGPParam->FixPP.m_BeginTime.tm_hour<=MyLocalTime.tm_hour) &&
			(MyGPParam->FixPP.m_EndTime.tm_hour>=MyLocalTime.tm_hour))// Estamos dentro del rango de la hora
		{	if (MyGPParam->FixPP.m_BeginTime.tm_min<=MyLocalTime.tm_min)//si es la misma hora
			{	printf("Acquiring data right now!\n");
				return true;
			}
		}
	}
	else
	{	if( (MyGPParam->FixPP.m_BeginTime.tm_hour<=MyLocalTime.tm_hour) || //(madrugada)
			(MyGPParam->FixPP.m_EndTime.tm_hour>=MyLocalTime.tm_hour))
		{	if (MyGPParam->FixPP.m_BeginTime.tm_min<=MyLocalTime.tm_min)
			{	printf("Acquiring data right now!\n");
				return true;
			}
		}
	}
	return false;
	printf("************************************** Estoy al final de CompareTime *********************************\n");
}


