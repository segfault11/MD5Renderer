/*
 * Auxiliaries for managing OpenGL representations of a MD5 mesh
 * Copyright (C) 2014 Arno in Wolde Luebke
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MD5OPENGLMESHMANAGER_H
#define MD5OPENGLMESHMANAGER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <Fxs/Math/Vector3.h>
#include <Fxs/MD5/MD5Mesh.h>
#define GL_GLEXT_PROTOTYPES 1
#include <Fxs/Opengl/glcorearb.h>

/*
** Submesh that actually stores all the opengl data
*/ 
typedef struct
{
	GLuint vao;
	GLuint positions; 			/* opengl positions buffer */
	FxsVector3* positionsHost; 	/* positions in host memory */
	int numPositions; 			/* # of positions */
	
	/* bounding box for the submesh */
    FxsVector3 min;
	FxsVector3 max;
}
MD5OpenGLSubMesh; 

/*
** Struct for storing OpenGL data for a MD5 mesh.
**
** A mesh consits of submeshes that store all the opengl data
*/
typedef struct
{
	FxsMD5Mesh* md5mesh; 			/* reference to the associated md5mesh */
	int numSubMeshes; 				/* # of submeshes */
	MD5OpenGLSubMesh* subMeshes;

	/* bounding box for the mesh */
    FxsVector3 min;
	FxsVector3 max;
}
MD5OpenGLMesh;

/*
** Creates the mesh manager with a config file.
*/ 
int MD5OpenGLMeshManagerCreate(const char* filename);

/*
** Gets the OpenGLMesh for an id. Returns NULL of the mesh does not exist.
*/
const MD5OpenGLMesh* MD5OpenGLMeshManagerGetMeshWithId(int id);

/*
** Updates the mesh pose with the frame of an animation
*/
const int MD5OpenGLMeshManagerUpdateMeshPoseWithAnimationFrame(
    int meshId,
    int animationId,
    int frame
);

/*
** Destroys the mesh manager. and releases all meshes it contains.
*/ 
void MD5OpenGLMeshManagerDestroy();

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: MD5OPENGLMESHMANAGER_H */
