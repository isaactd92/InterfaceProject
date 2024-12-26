#include "MemoryManager.h"

MemoryManager::MemoryManager()
{
	is_allocated = false;		  //indicador de return
	m_mAcqBuffer = NULL;     //Guarda los tamaños de memoria en tipo Ipp dependiendo si es float or short
	m_nNIBuffer = NULL;          //NIDAQ buffers
	m_nNIDoubleBuffer = NULL;    //NIDAQ buffers
	MyParams = NULL;//puntero de tipo ExpParameters (puntero a un objeto tipo ExpParameters)
	m_miSizeAcqBuffer = NULL;//guarda los tamaños que usaremos para memoria(data tipo short)
	m_iSizeNIBuffer=0;   //NIDAQ buffers
	m_iSizeNIDoubleBuffer=0; //NIDAQ buffers
	numrawbuffs = 0;  //subindices del arreglo (short)
}

//-------------------------------------------------------------------
MemoryManager::~MemoryManager()
{
	if(!is_allocated)
	{
	  return;
	}
	FreeMemory();
}
//-------------------------------------------------------------------
//Cambia de tipo : m_mAcqBuffer y m_mAcqBufferFloat de tipo ipp
//Segun el tipo de dato :short or float, crea los arreglos de adquisicion m_mAcqBuffer o m_mAcqBufferFloat en formato ipp que separan memoria
//Separa en memoria el espacio donde se almacenará las muestras de adquisición: m_nNIBuffer
bool MemoryManager::AllocateMemory(ExpParameters* MyParameters)// MyParameters:Objeto de tipo ExpParameters
{
	printf("<************************Estoy dentro de AllocateMemory*****************************>\n");	
	//obtine un puntero a la estructura con los parametros para
	//iniciar los buffers  acceder a los atributos de ExpParameters
	MyParams = MyParameters;// guarda el objeto MyParameters
	//obtine numero y tamahno de buffers
	GetBufferSizes();//guarda los valores de espacio de memoria dependiendo si el dato es short o float en m_miSizeAcqBuffer o m_miSizeAcqBufferFloat
	//------------------------------------------------------------------------------------------------
	//Obtendremos buffers tipo short
	if((MyParams->MyGPParam.FixPP.m_nProcessFlags&FLAG_DATATYPE_SHORT)!=0)
	{
		printf("Estoy dentro de MyParams->MyGPParam.FixPP.m_nProcessFlags&FLAG_DATATYPE_SHORT\n");		
		m_mAcqBuffer = new complex<short>* [numrawbuffs];

		for(int indbuf=0;indbuf<numrawbuffs;indbuf++)
		{
		  m_mAcqBuffer[indbuf] = (complex<short>*) malloc(m_miSizeAcqBuffer[indbuf]*sizeof(complex<short>));
		}

		//El tamahno del bloque de datos es el tamahno del ultimo buffer
		//******Nos da el tamaño en bytes(debido al complex<short>) de la cantidad de elementos del buffer de adquisicion en raw data//No usado
		//****** Nos da el tamaño en bytes del bloque de datos.
		MyParams->MyGPParam.FixPP.m_nSizeOfDataBlock = m_miSizeAcqBuffer[numrawbuffs-1]*sizeof(complex<short>);//sizeof te da la cantidad de memoria que almacena

		////////////////////////////////////////////////////
		//Error en el guardado
		if(m_mAcqBuffer==NULL)//no hay suficiente memoria.No se creó el buffer
		{
			FreeMemory();
			return false;
		}
	}//fin (if else if)

	//---------Establece el buffer total de memoria donde almacenará las muestras-------------------------
	//No se usará EnableJARS ya que la interfaz funcionará directamente en modo JARS 
	
	 int Mark=1; 
	 //calcula tamahno de buffer de memoria
	 m_iSizeNIBuffer=(MyParams->MySystemParam.m_nSamples
						*MyParams->MySystemParam.m_nChannels
						*MyParams->MyGRCParam.FixRCP.m_nNTX+Mark)//espacio de mark (4bytes(1 int) = 0xaa 0xaa 0xam 0xaa)
						*MyParams->MyGPParam.m_nProfilesperBlock//si hubiera dos marcas por bloque
						/MyParams->MyGRCParam.FixRCP.m_nNTX;

	 printf("El valor de m_iSizeNIBuffer es: %d\n",m_iSizeNIBuffer);
	 m_iSizeNIDoubleBuffer=2*m_iSizeNIBuffer;
	 //crea buffers
		
	 m_nNIBuffer=(complex<short>*)malloc(m_iSizeNIBuffer*sizeof(complex<short>));
	 
	 m_nNIDoubleBuffer=(complex<short>*)malloc(m_iSizeNIDoubleBuffer*sizeof(complex<short>));

	 if(m_nNIBuffer==NULL||m_nNIDoubleBuffer==NULL||m_mAcqBuffer[0]==NULL)//No hay suficiente memoria.No se crearon los buffer
	   {	FreeMemory();
		return false;
	   }
	
	is_allocated = true; //Para borrar memoria en el destructor de MemoryManager
	
	printf("<************************Terminé AllocateMemory*****************************>\n");
return true;
}
//-------Borra debido a que se creo mal el buffer de adquisición debido a la falta de memoria----------------
void MemoryManager::FreeMemory()
{
	//libera memoria de buffers tipo short y reemplaza por null--------------------
	if((MyParams->MyGPParam.FixPP.m_nProcessFlags&FLAG_DATATYPE_SHORT)!=0)//si es tipo short
	{
		for(int i=0;i<numrawbuffs;i++)
		{
			if(m_mAcqBuffer[i]!=NULL)
			{
				free(m_mAcqBuffer[i]);//libera memoria de ippsMalloc_16sc(m_miSizeAcqBuffer[0])
				m_mAcqBuffer[i]=NULL;//cambia de valor a nulo
			}
		}
		delete m_miSizeAcqBuffer;//elimina m_miSizeAcqBuffer ya que hay error
	}

	return;
}


//-------------------------------------------------------------------
//guarda los valores de tamaño de memoria dependiendo si el dato es short o float en m_miSizeAcqBuffer o m_miSizeAcqBufferFloat
void MemoryManager::GetBufferSizes()
{
	printf("<************************Estoy dentro de GetBufferSizes*****************************>\n");	
	size_t sizeprocrawbuffs[2];//guarda los tamaños
	numrawbuffs = 0; // 07/03/2018 para reiniciar cuando hay trying restart


	//inicialmente debe haber un buffer, que se usa para que el sistema de adquisicion
	//volque la data cruda
	sizeprocrawbuffs[numrawbuffs] = MyParams->MySystemParam.m_nSamples//numero de muestras
								   *MyParams->MySystemParam.m_nChannels//numero de canales
								   *MyParams->MySystemParam.m_nProfiles;//numero de perfiles adquiridos
	printf("El valor de sizeprocrawbuffs[numrawbuffs] es: %d\n",sizeprocrawbuffs[numrawbuffs]);	
	numrawbuffs++;
	//------------------------------------------------------

	//Si es raw data, guarda los valores solo de raw en m_miSizeAcqBuffer
	if((MyParams->MyGPParam.FixPP.m_nProcessFlags&FLAG_DATATYPE_SHORT)!=0)//Si hay short data
	{
		m_miSizeAcqBuffer = new int[numrawbuffs];//crea un arreglo del tamaño de numrawbuffs

		for(int i=0;i<numrawbuffs;i++)
		{
			m_miSizeAcqBuffer[i] = sizeprocrawbuffs[i];//Da valores al nuevo arreglo con elementos del raw
		}//Copia los lementos del raw en un nuevo arreglo
	}

	//-----------------------------------------------------
}
//------------------------------------------------------------------
