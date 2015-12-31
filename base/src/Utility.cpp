#include "Utility.h"

#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <Windows.h>
#include "gl/glew.h"

using namespace std;

namespace Utility 
{
    char* loadFile(const char *fname, GLType_int &fSize)
	{
		ifstream::pos_type size;
		char* memblock;

        std::ostringstream debugOutput;

        ifstream file (fname, ios::in|ios::binary|ios::ate);
		if (file.is_open())
		{
			size = file.tellg();
			fSize = (GLuint) size;
            unsigned int bufferSize = size;
			memblock = new char [bufferSize + 1];
            if (!memblock)
            {
                LogMessage("Not enough memory to load file!\n");
                assert(false);
            }
            
            file.seekg (0, ios::beg);
			file.read (memblock, size);
			file.close();
            memblock[bufferSize] = '\0';

			debugOutput << "File " << fname << " loaded." << endl;
            LogMessage(debugOutput.str().c_str());
		}
		else
		{
            debugOutput << "Unable to open file " << fname << endl;
            LogMessage(debugOutput.str().c_str());
			assert(false);
		}
		return memblock;
	}

	// printShaderInfoLog
	// From OpenGL Shading Language 3rd Edition, p215-216
	// Display (hopefully) useful error messages if shader fails to compile
    void printShaderInfoLog(GLType_int shader)
	{
		int infoLogLen = 0;
		int charsWritten = 0;
		GLchar *infoLog;

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);

		// should additionally check for OpenGL errors here
        std::ostringstream debugOutput;
		if (infoLogLen > 0)
		{
			infoLog = new GLchar[infoLogLen];
			// error check for fail to allocate memory omitted
			glGetShaderInfoLog(shader,infoLogLen, &charsWritten, infoLog);
            debugOutput << "InfoLog:" << endl << infoLog << endl;
            delete[] infoLog;
            LogMessage(debugOutput.str().c_str());
        }

		// should additionally check for OpenGL errors here
	}

    void printLinkInfoLog(GLType_int prog)
	{
		int infoLogLen = 0;
		int charsWritten = 0;
		GLchar *infoLog;

		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &infoLogLen);

		// should additionally check for OpenGL errors here
        std::ostringstream debugOutput;
		if (infoLogLen > 0)
		{
			infoLog = new GLchar[infoLogLen];
			// error check for fail to allocate memory omitted
			glGetProgramInfoLog(prog,infoLogLen, &charsWritten, infoLog);
            debugOutput << "InfoLog:" << endl << infoLog << endl;
			delete[] infoLog;
            LogMessage(debugOutput.str().c_str());
		}
	}

    shaders_t createShaders(const std::string& vs_source, const std::string& fs_source)
    {
		GLuint f, v;

		v = glCreateShader(GL_VERTEX_SHADER);
		f = glCreateShader(GL_FRAGMENT_SHADER);	

		// load shaders & get length of each
		GLint vlen = vs_source.length();
        GLint flen = fs_source.length();
        const char* vs = vs_source.c_str();
        const char* fs = fs_source.c_str();

		glShaderSource(v, 1, &vs, &vlen);
		glShaderSource(f, 1, &fs, &flen);

		GLint compiled;

		glCompileShader(v);
		glGetShaderiv(v, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
            LogMessage("Vertex shader not compiled.\n");
			printShaderInfoLog(v);
            assert(false);
		} 

		glCompileShader(f);
		glGetShaderiv(f, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
            LogMessage("Fragment shader not compiled.\n"); 
            printShaderInfoLog(f);
            assert(false);
		} 
		shaders_t out; out.vertex = v; out.fragment = f;

		return out;
	}

    void attachAndLinkProgram(GLType_uint program, shaders_t shaders)
    {
		glAttachShader(program, shaders.vertex);
		glAttachShader(program, shaders.fragment);

		glLinkProgram(program);
		GLint linked;
		glGetProgramiv(program,GL_LINK_STATUS, &linked);
		if (!linked) 
		{
            LogMessage("Program did not link.\n");
			printLinkInfoLog(program);
            assert(false);
		}
	}

    void LogHelper(const char* inMessage, const char* filename = nullptr, bool endLine = false)
    {
        std::string message(inMessage);
        if (endLine)
        {
            message.append("\n");
        }

        if (filename)
        {
            std::ofstream fileOutStream;
            fileOutStream.open(filename, ios::out);
            if (fileOutStream.is_open())
            {
                fileOutStream << message;
                fileOutStream.close();
            }
        }
        else
        {
            OutputDebugStringA(message.c_str());
        }
    }

    void LogFile(const char* message)
    {
        LogHelper(message, ".\\logfile.txt");
    }

    void LogOutput(const char* message)
    {
        LogHelper(message);
    }

    void LogFileAndEndLine(const char* message)
    {
        LogHelper(message, ".\\logfile.txt", true);
    }

    void LogOutputAndEndLine(const char* message)
    {
        LogHelper(message, nullptr, true);
    }

    void LogMessage(const char* message)
    {
        if (IsDebuggerPresent())
            LogOutput(message);
        else
            LogFile(message);
    }

    void LogMessageAndEndLine(const char* message)
    {
        if (IsDebuggerPresent())
            LogOutputAndEndLine(message);
        else
            LogFileAndEndLine(message);
    }

    uint32_t HashCString(const char* cString)
    {
        // SDBM method found here: http://www.cse.yorku.ca/~oz/hash.html
        uint32_t hash = 0;
        int32_t c;

        if (cString)
            while (c = *(cString++))
                hash = c + (hash << 6) + (hash << 16) - hash;

        return hash;
    }
}
