


#include "error.h"
#include "context.h"


namespace CU
{
	unique_context createContext(unsigned int flags, CUdevice device)
	{
		CUcontext context;
		checkError(cuCtxCreate(&context, flags, device));
		return unique_context(context);
	}
}
