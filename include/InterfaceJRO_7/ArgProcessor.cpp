
#include "ArgProcessor.h"

//*** Define el atributo private de ArgProcessor igual a los parámetros del constructor
ArgProcessor::ArgProcessor(ExpParameters &params)
:MyParameters(params)//Parece que da valor al atributo private MyParameters igual a params//Para que no haya error de inicialización
{printf("*********** Estoy dentro del constructor de ArgProcessor **********\n");
}
ArgProcessor::~ArgProcessor()
{}

//*****Dependiendo de los parámetros que entran en el main, guarda las direcciones de los objetos de los modos de operación en ** AcqMan
void ArgProcessor::jro_arguments(const char* path)
{
	printf("***********Estoy dentro de jro_arguments**********\n");
	if(!MyParameters.ReadRACPFileParameters(path))//Obtenido de GNU Radio
	 {
	   printf("\nError en leer el RacpFile %s\n");

	 }
	printf("***********Terminó de leer el RACP**********\n");

}
