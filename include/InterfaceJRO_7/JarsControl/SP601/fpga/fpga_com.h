
#include <string>
using namespace std;

//#include <zmq.hpp>
//zmq::context_t context(1);// --------------------------> Usar en JicaHeader

void fpga_con_init(char* sp601_ip_char, int Sp601_Port, int lport);
ssize_t fpga_con_send( const char *buf, size_t len );
ssize_t fpga_con_recv(char *dbuf, size_t dsize );

void IPstr_to_bytearray(char *IPstr, char *parts);
void MACstr_to_bytearray(char *MACstr, char *parts);
void intToBytes(int number, char *arrayOfByte, size_t size);
void bytearray_to_IPstr(const char *bytes, char *IPstr);
void bytearray_to_int(const char *byte_array, int num);
void bytearray_to_MACstr(const char *bytes, char *MACstr);
char* itoa(int num, char* str, int base);
