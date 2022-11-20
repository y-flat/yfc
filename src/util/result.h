#ifndef UTIL_RESULT_H
#define UTIL_RESULT_H

#include "platform.h"

typedef enum yf_result {
	/** Success */
	YF_OK,
	/** Generic error */
	YF_ERROR,
	/** Reached end of collection */
	YF_REACHED_END,
} yf_result;

#endif
