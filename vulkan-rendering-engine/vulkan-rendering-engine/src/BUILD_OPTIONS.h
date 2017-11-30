/*
*  Build-Options header file
*  Contains Information about the program and includes header used throughout the whole engine.
*
*  Date:    23.04.2016
*  Creator: Silvan Hau
*/

#ifndef BUILD_OPTIONS_H_
#define BUILD_OPTIONS_H_

#include "platform.hpp" /* includes platform specific code and vulkan.h */
#include "structs.hpp"
#include "logger/logger.h"
#include "utils/utils.h"

#include <assert.h>
#include <vector>
#include <memory>
#include <string>
#include <map>

//---------------------------------------------------------------------------
//  Defines
//---------------------------------------------------------------------------

#define VERTEX_BUFFER_BIND_ID               0
#define WHOLE_BUFFER_SIZE                   0
#define TS(v)                               std::to_string(v)


#endif // !BUILD_OPTIONS_H_
