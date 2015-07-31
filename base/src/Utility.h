#ifndef UTILITY_H_
#define UTILITY_H_

#include "Common.h"
#include <cstdlib>
#include <string>

#include "gl/glew.h"

namespace Utility 
{
	typedef struct 
    {
		GLuint vertex;
		GLuint fragment;
	} shaders_t;

    shaders_t createShaders(const std::string& vs_source, const std::string& fs_source);

    void attachAndLinkProgram( GLuint program, shaders_t shaders);

    char* loadFile(const char *fname, GLint &fSize);

    // printShaderInfoLog
    // From OpenGL Shading Language 3rd Edition, p215-216
    // Display (hopefully) useful error messages if shader fails to compile
    void printShaderInfoLog(GLint shader);

    void printLinkInfoLog(GLint prog);

    void LogOutput(const char* logMessage);
    void LogFile(const char* logMessage);

    uint32_t HashCString(const char* cString);
}
 
#endif
