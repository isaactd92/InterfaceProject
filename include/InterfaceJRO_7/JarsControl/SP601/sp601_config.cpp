#include "sp601_config.h"

//Constructor de sp601_config
//**************************** Establece los parámetros de RED ******************************************
sp601_config::sp601_config()
{
  //************************ Define values for network parameters ********************************
  //IP,MAC,puerto del sp601
  sp601_ip = "10.10.30.123";//"Para el ROJ:10.10.30.123";//localizador de mi host en la red
  char sp601_mac[] = "00:0a:35:02:05:f2";//Para el ROJ: "00:0a:35:02:05:f2";//dirección única MAC, que identifica un elemento de la capa de enlace de datos(tarjeta   	de red)//Se establece en el main
  int sp601_port = 52428; //Para el ROJ: 52428;

  //IP y puerto del sp601 en string
  //string sp601_ip_string = string(sp601_ip);
  	 	
  //IP,MAC,puerto de la tarjeta de red serán dados por el bloque

  //****Guarda los valores de la red establecidos en memoria con su null (ya que estamos en size_t correctito)
  //Guarda el valor de la ip del sp601
  SP601_IP=(char *)malloc(strlen(sp601_ip)+1);//toma el término null caracter (string literal)
  memcpy(SP601_IP,sp601_ip,strlen(sp601_ip)+1);
  //Guarda el valor de la mac del sp601
  SP601_MAC=(char *)malloc(strlen(sp601_mac)+1);//
  memcpy(SP601_MAC,sp601_mac,strlen(sp601_mac)+1);
  //el valor del puerto de la sp601 es guardado en SP601_PORT
  SP601_PORT = sp601_port;
  //Establece los parámetros de UDP
  //UDP_timeout = 100;
  VERBOSE_MODE = true;
}
