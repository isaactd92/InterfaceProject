#include "RdpToolsBeta.h"

#include <cmath>

//Constructor
RdpToolsBeta::RdpToolsBeta()
{
	MyExpParam=NULL;
	MyMemManager=NULL;

}
//---------------------------------------------------------
RdpToolsBeta::~RdpToolsBeta()
{}
//---------------------------------------------------------
//Inicializa algunas variables (HO,NSA,DH) de Experiment Parameters
//Inicializa los punteros de ExpParameters y MemoryManager
void RdpToolsBeta::GetExtParameters(ExpParameters* ExpParam,MemoryManager* MemManger)throw() //la funcion no generará ninguna excepcion
{	
	printf("Estoy dentro de GetExtParameters.......\n");
	//almacena puntero a objeto con informacion del experimento
	MyExpParam=ExpParam;
	//almacena puntero a objeto con informacion de memoria
	MyMemManager=MemManger;
	//inicializa variables
	initVariables();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//funciones de inicicalizacion de procesos
//Da valor a los parámetros de ventana (H0,NSA,DH),genera excepciones RdpToolsExcepdefine,si las hay, y define Dyn_snCode
//Dyn_snCode=code_binaries=arreglo de arreglos donde cada elemento es un complemento a 2 dependiendo del code_binaries
bool RdpToolsBeta::initVariables()throw()
{
	printf("Estoy dentro de initVariables().......\n");	
	//numero de muestras que se van a restar debido a la decodificacion
	int restNSA=0;
	//ancho de estas muestras en km
	float restNSAkm=0;

//------------------------------------------------------------------------------------------
	//establece numero de ventanas a m_nData_Windows
	//m_nProcessWindows inicializado como 0 en ExperimentParameters y queda ese valor.
	if(MyExpParam->MyGPParam.m_nProcessWindows>0)
	{
		MyExpParam->MyGPParam.FixPP.m_nData_Windows = MyExpParam->MyGPParam.m_nProcessWindows;
	}
	else
	{
		MyExpParam->MyGPParam.FixPP.m_nData_Windows = MyExpParam->MyGRCParam.FixRCP.m_nNum_Windows;
	}
// Nos genera una excepcion de error
	if(MyExpParam->MyGPParam.FixPP.m_nData_Windows <= 0)//m_nData_Windows será mínimo 1
	{
		throw RdpToolsExcep(Data_Win_Err,MyExpParam->MyGPParam.m_sBaseDirectory);//genera la excepcion definida en RdpToolsExceptions
	}																			 //Excepcion de error
//--------------------------------------------------------------------------------------------------------------
	//crea arreglos para almacenar los valores de los parametros de las ventanas
	//Altura inicial,número de muestras,resolución(problema m_nData_Windows inicializado con 1 ¿Cuándo cambia?)

	MyExpParam->MyGPParam.Dyn_sfDataH0 = new float[MyExpParam->MyGPParam.FixPP.m_nData_Windows];
	MyExpParam->MyGPParam.Dyn_snDataNSA = new unsigned int [MyExpParam->MyGPParam.FixPP.m_nData_Windows];
	MyExpParam->MyGPParam.Dyn_sfDataDH = new float[MyExpParam->MyGPParam.FixPP.m_nData_Windows];

	//ingresa los valores de los parametros de ventana dependiendo de m_nData_Windows
	//Da valores de H0,NSAyDh a DataHo,NSA y DH
	printf("Estoy antes del for en initVariables de RdpToolsBeta.......\n");
	printf("El valor de MyExpParam->MyGPParam.FixPP.m_nData_Windows es: %d\n",MyExpParam->MyGPParam.FixPP.m_nData_Windows);	
	for(size_t i=0;i<MyExpParam->MyGPParam.FixPP.m_nData_Windows;i++)
	{
		//en muestras(¿Por qué se van a restar muestras en decodificación?)
		restNSA = -int(restNSAkm/MyExpParam->MyGRCParam.Dyn_sfAcqDH[i]);//numero de muestras adquiridas
		restNSA+= (restNSAkm==0)?0:1;//será cero.
		MyExpParam->MyGPParam.Dyn_sfDataH0[i]  = MyExpParam->MyGRCParam.Dyn_sfAcqH0[i];//el valor del .racp es guardado dentro de Dyn_sfDataH0
		MyExpParam->MyGPParam.Dyn_snDataNSA[i] = MyExpParam->MyGRCParam.Dyn_snAcqNSA[i] +restNSA;//Para hallar el valor de Dyn_snDataNSA se resta del .racp debido a la decodificacion?
		MyExpParam->MyGPParam.Dyn_sfDataDH[i]  = MyExpParam->MyGRCParam.Dyn_sfAcqDH[i];//el valor del .racp es guardado dentro de Dyn_sfDataDH
		printf("El valor de MyExpParam->MyGPParam.Dyn_sfDataH0 es: %f\n",MyExpParam->MyGPParam.Dyn_sfDataH0[i]);	
		printf("El valor de MyExpParam->MyGPParam.Dyn_snDataNSA es: %d\n",MyExpParam->MyGPParam.Dyn_snDataNSA[i]);	
		printf("El valor de MyExpParam->MyGPParam.Dyn_sfDataDH es: %f\n",MyExpParam->MyGPParam.Dyn_sfDataDH[i]);		
	}
	printf("Estoy después del for.......\n");	
	//---------------------------------------------------------------------
	//Initialization of m_sCodeArray_char
	//---------------------------------------------------------------------
	//Nos lleva a RdpToolsExcep si se cumple la condición
	if(MyExpParam->MyGRCParam.Dyn_sfAcqDH[0] <= 0)
	{
		throw RdpToolsExcep(Acq_DH_Err,MyExpParam->MyGPParam.m_sBaseDirectory);
	}

	return true;
}

