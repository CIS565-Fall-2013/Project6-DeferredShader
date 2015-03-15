#include "Utility.h"

#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <Windows.h>

using namespace std;

namespace Utility {

	char* loadFile(const char *fname, GLint &fSize)
	{
		ifstream::pos_type size;
		char* memblock;

        std::ostringstream debugOutput;

        ifstream file (fname, ios::in|ios::binary|ios::ate);
		if (file.is_open())
		{
			size = file.tellg();
			fSize = (GLuint) size;
			memblock = new char [size];
            if (!memblock)
            {
                LogOutput("Not enough memory to load file!\n");
                exit(EXIT_FAILURE);
            }
            
            file.seekg (0, ios::beg);
			file.read (memblock, size);
			file.close();

			debugOutput << "File " << fname << " loaded." << endl;
            LogOutput(debugOutput.str().c_str());
		}
		else
		{
            debugOutput << "Unable to open file " << fname << endl;
            LogOutput(debugOutput.str().c_str());
			exit(EXIT_FAILURE);
		}
		return memblock;
	}

	// printShaderInfoLog
	// From OpenGL Shading Language 3rd Edition, p215-216
	// Display (hopefully) useful error messages if shader fails to compile
	void printShaderInfoLog(GLint shader)
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
            LogOutput(debugOutput.str().c_str());
        }

		// should additionally check for OpenGL errors here
	}

	void printLinkInfoLog(GLint prog) 
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
            LogOutput(debugOutput.str().c_str());
		}
	}

	shaders_t loadShaders(const char * vert_path, const char * frag_path) 
    {
		GLuint f, v;

		char *vs,*fs;

		v = glCreateShader(GL_VERTEX_SHADER);
		f = glCreateShader(GL_FRAGMENT_SHADER);	

		// load shaders & get length of each
		GLint vlen;
		GLint flen;
		vs = loadFile(vert_path,vlen);
		fs = loadFile(frag_path,flen);

		const char * vv = vs;
		const char * ff = fs;

		glShaderSource(v, 1, &vv,&vlen);
		glShaderSource(f, 1, &ff,&flen);

		GLint compiled;

		glCompileShader(v);
		glGetShaderiv(v, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
            LogOutput("Vertex shader not compiled.\n");
			printShaderInfoLog(v);
		} 

		glCompileShader(f);
		glGetShaderiv(f, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
            LogOutput("Fragment shader not compiled.\n"); 
            printShaderInfoLog(f);
		} 
		shaders_t out; out.vertex = v; out.fragment = f;

		delete [] vs; // dont forget to free allocated memory
		delete [] fs; // we allocated this in the loadFile function...

		return out;
	}

	void attachAndLinkProgram( GLuint program, shaders_t shaders) 
    {
		glAttachShader(program, shaders.vertex);
		glAttachShader(program, shaders.fragment);

		glLinkProgram(program);
		GLint linked;
		glGetProgramiv(program,GL_LINK_STATUS, &linked);
		if (!linked) 
		{
			LogOutput("Program did not link.\n");
			printLinkInfoLog(program);
		}
	}

    void LogHelper(const char* message, const char* filename = nullptr)
    {
        if (filename)
        {
            std::ofstream fileOutStream;
            fileOutStream.open(filename, ios::app);
            if (fileOutStream.is_open())
            {
                fileOutStream << message;
                fileOutStream.close();
            }
        }
        else
        {
            OutputDebugStringA(message);
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
}
