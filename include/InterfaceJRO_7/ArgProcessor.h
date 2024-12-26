#ifndef ARGPROCESSOR_ARGPROCESSOR_H_
#define ARGPROCESSOR_ARGPROCESSOR_H_

#include "JarsControl/JarsControl.h"//llama al objeto JARControl
#include <string>

class ArgProcessor
{
private:
	ExpParameters &MyParameters;

public:
	ArgProcessor(ExpParameters &MyParameters);
	~ArgProcessor();

	void jro_arguments(const char* path);
};

#endif /* ARGPROCESSOR_ARGPROCESSOR_H_ */
