#include <stdio.h>
#include <stdlib.h>
#include "MD5OpenGLRenderer.h"
#include "MD5OpenGLMeshManager.h"
#include <Fxs/OpenGL/Program.h>

#define ERR_MSG(X) printf("In file: %s line: %d\n\t%s\n", __FILE__, __LINE__, X);

/*
** Definition of our shaders.
*/ 
#define TO_STRING(X) #X

static char* vertexShader =
	"#version 150\n"
TO_STRING(
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

	in vec3 position;

	void main()
	{
		gl_Position = projection*view*model*vec4(position, 1.0);
	}
);

static char* fragmentShader =
	"#version 150\n"
TO_STRING(
	out vec4 fragOut;	

	void main()
	{
		fragOut = vec4(1.0, 0.0, 0.0, 1.0);	
	}
);

/* the opengl program we use to render */
static GLuint program; 
static int wasInitialized = 0;

int FFMD5OpenGLRendererCreate(const char* filename)
{
    float identity[16] = {
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0
        };

	if (wasInitialized)
	{
		ERR_MSG("FFMD5OpenGLRenderer was already initialized"); 
		return 0;
	}

	if (!MD5OpenGLMeshManagerCreate(filename))
	{
		return 0;
	}

	/* create our program */		
	program = glCreateProgram();
	
	FxsOpenGLProgramAttachShaderWithSource(
		program, 
		GL_VERTEX_SHADER, 
		vertexShader
	);
	
	FxsOpenGLProgramAttachShaderWithSource(
		program, 
		GL_FRAGMENT_SHADER, 
		fragmentShader
	);

	glBindAttribLocation(program, 0, "position");
	glBindFragDataLocation(program, 0, "fragOut"); 
	FxsOpenGLProgramLink(program);

	if (GL_NO_ERROR != glGetError())
	{
		ERR_MSG("Detected OpenGL error.")
		return 0;
	}
    
    /* initialize our program */
    FFMD5OpenGLRendererSetModelMatrix(identity);
    FFMD5OpenGLRendererSetViewMatrix(identity);
    FFMD5OpenGLRendererSetProjectionMatrix(identity);
    
	wasInitialized = 1;

	return 1;
}

void FFMD5OpenGLRendererDestroy()
{
	if (!wasInitialized)
	{
		ERR_MSG("FFMD5OpenGLRenderer is not initialized"); 
		return;
	}

	glDeleteProgram(program);

	MD5OpenGLMeshManagerDestroy();
}

int FFMD5OpenGLRendererRender(int meshId, int animationId, int frame)
{
	const MD5OpenGLMesh* mesh = NULL;
	int i = 0;

    if (!wasInitialized)
    {
        return 0;
    }
    
    MD5OpenGLMeshManagerUpdateMeshPoseWithAnimationFrame(
        meshId,
        animationId,
        frame
    );
    
    mesh = MD5OpenGLMeshManagerGetMeshWithId(meshId);

	if (!mesh)
	{
		return 0;
	}
    
	glUseProgram(program);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	for (i = 0; i < mesh->numSubMeshes; i++)
	{
		glBindVertexArray(mesh->subMeshes[i].vao);
		glDrawArrays(GL_TRIANGLES, 0, mesh->subMeshes[i].numPositions);
	}

	return 1;
}

void FFMD5OpenGLRendererSetModelMatrix(const float* model)
{
    FxsOpenGLProgramUniformMatrix4(program, "model", model, GL_FALSE);
}

void FFMD5OpenGLRendererSetViewMatrix(const float* view)
{
    FxsOpenGLProgramUniformMatrix4(program, "view", view, GL_FALSE);
}

void FFMD5OpenGLRendererSetProjectionMatrix(const float* projection)
{
    FxsOpenGLProgramUniformMatrix4(program, "projection", projection, GL_FALSE);
}

