#ifndef UTILITY_H_
#define UTILITY_H_

#include "Common.h"
#include <cstdlib>
#include <string>

namespace Utility 
{
	typedef struct 
    {
		GLType_uint vertex;
        GLType_uint fragment;
	} shaders_t;

    shaders_t createShaders(const std::string& vs_source, const std::string& fs_source);

    void attachAndLinkProgram(GLType_uint program, shaders_t shaders);

    char* loadFile(const char *fname, GLType_int &fSize);

    // printShaderInfoLog
    // From OpenGL Shading Language 3rd Edition, p215-216
    // Display (hopefully) useful error messages if shader fails to compile
    void printShaderInfoLog(GLType_int shader);

    void printLinkInfoLog(GLType_int prog);

    void LogOutput(const char* logMessage);
    void LogFile(const char* logMessage);
    void LogOutputAndEndLine(const char* logMessage);
    void LogFileAndEndLine(const char* logMessage);

    void LogMessage(const char* logMessage); 
    void LogMessageAndEndLine(const char* logMessage);

    uint32_t HashCString(const char* cString);
}
 
#endif
