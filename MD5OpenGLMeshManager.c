#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <Fxs/Math/Vector4.h>
#include "MD5OpenGLMeshManager.h"
#include <Fxs/MD5/MD5Animation.h>
#include "../External/parson.h"

#define ERR_MSG(X) printf("In file: %s line: %d\n\t%s\n", __FILE__, __LINE__, X);
static char errMsg[1024];

/* forward decl. of a destructor fct. for a MD5OpenGLMesh */
static void MD5OpenGLMeshDestroy(MD5OpenGLMesh** glmesh);

/*
** Creates a MD5OpenGLMesh from an md5file
*/ 
static int MD5OpenGLMeshCreateWithFile(
	MD5OpenGLMesh** glmesh, 
	const char* filename
)
{
	FxsMD5Mesh* md5mesh = NULL;
	FxsMD5SubMesh* md5subMesh = NULL;
	FxsMD5Weight* md5weight = NULL;
	FxsMD5Vertex* md5vertex = NULL;
	FxsMD5Face* md5face = NULL;
	FxsMD5Joint* md5joint = NULL;
	FxsVector4 weightPosition; 				/* transformed weight position */
	FxsVector3* vertPosition;
	unsigned int count = 0; 				/* counts the vertices loaded */
	unsigned int* vertexIds = NULL; 		/* vertex indices of md5face */
	int i = 0, j = 0, k = 0, l = 0; 	 	/* loop variables */

	if (!FxsMD5MeshCreateWithFile(&md5mesh, filename))
	{
		sprintf(errMsg, "Warning: could not load md5mesh: %s", filename);
		ERR_MSG(errMsg);
		return 0;
	}

	*glmesh = (MD5OpenGLMesh*)malloc(sizeof(MD5OpenGLMesh));

	if (!glmesh) 
	{
		sprintf(
			errMsg,
			"Warning: malloc for MD5OpenGL mesh failed. Could not load md5mesh: %s", 
			filename
		);
	    ERR_MSG(errMsg)
		FxsMD5MeshDestroy(&md5mesh);
		return 0;
	}

	/* prepare gl mesh */
	memset(*glmesh, 0, sizeof(MD5OpenGLMesh));
	(*glmesh)->md5mesh = md5mesh;  
	(*glmesh)->numSubMeshes = md5mesh->numSubMeshes;
	(*glmesh)->subMeshes = (MD5OpenGLSubMesh*)malloc(
			md5mesh->numSubMeshes*sizeof(MD5OpenGLSubMesh)
		);

	if (!(*glmesh)->subMeshes)
	{
		sprintf(
			errMsg, 
			"Warning: malloc failed. Could not load md5mesh: %s", 
			filename
		);

		ERR_MSG(errMsg);	
		FxsMD5MeshDestroy(&md5mesh);
		MD5OpenGLMeshDestroy(glmesh);
		return 0;
	}		

	memset(
		(*glmesh)->subMeshes, 
		0, 
		(*glmesh)->numSubMeshes*sizeof(MD5OpenGLSubMesh)
	);

	(*glmesh)->min.x = FLT_MAX;
	(*glmesh)->min.y = FLT_MAX;
	(*glmesh)->min.z = FLT_MAX;
	(*glmesh)->max.x = FLT_MIN;
	(*glmesh)->max.y = FLT_MIN;
	(*glmesh)->max.z = FLT_MIN;
	
	/* load the submeshes */
	for (i = 0; i < md5mesh->numSubMeshes; i++)       /* for each md5submesh */
	{
	  	md5subMesh = &md5mesh->meshes[i];

		/* alloc host memory for positions of this gl submesh */
		(*glmesh)->subMeshes[i].numPositions = 3*md5subMesh->numFaces;
		(*glmesh)->subMeshes[i].positionsHost = (FxsVector3*)malloc(
				3*md5subMesh->numFaces*sizeof(FxsVector3)
			);
		
		if (!(*glmesh)->subMeshes[i].positionsHost) 
		{
		    sprintf(
				errMsg, 
				"Warning: malloc failed. Could not load md5mesh: %s", 
				filename
			);
			
			ERR_MSG(errMsg);	
			FxsMD5MeshDestroy(&md5mesh);
			MD5OpenGLMeshDestroy(glmesh);
		}

		memset(
			(*glmesh)->subMeshes[i].positionsHost,
			0,
			3*md5subMesh->numFaces*sizeof(FxsVector3)
		);

		/* init the bounding box for the sub mesh */
		(*glmesh)->subMeshes[i].min.x = FLT_MAX;
		(*glmesh)->subMeshes[i].min.y = FLT_MAX;
		(*glmesh)->subMeshes[i].min.z = FLT_MAX;
		(*glmesh)->subMeshes[i].max.x = FLT_MIN;
		(*glmesh)->subMeshes[i].max.y = FLT_MIN;
		(*glmesh)->subMeshes[i].max.z = FLT_MIN;
		
		count = 0;

		for (j = 0; j < md5subMesh->numFaces; j++)    /* for each face of the 
													  ** submesh */ 
		{
			md5face = &md5subMesh->faces[j];
			vertexIds = &md5face->v1;
	
			for (k = 0; k < 3; k++)           /* for each vertex of the face */
			{
				md5vertex = &md5subMesh->vertices[vertexIds[k]];
				
				/* make the current vertex position zero */
				vertPosition = &(*glmesh)->subMeshes[i].positionsHost[count];
				FxsVector3MakeZero(vertPosition);
			
				for (l = 0; l < md5vertex->numWeights; l++) /* for each weight 
															** of the vertex */
				{
					md5weight = &md5subMesh->weights[md5vertex->weightId + l];
					md5joint = &md5mesh->currentPose.joints[md5weight->jointId];
					
					FxsMatrix4MultiplyVector3(
						&weightPosition, 
						&md5joint->transform,
						&md5weight->position
					);

					/* add up all weight positions to compute the final
					** vertex position.
					*/
					vertPosition->x += md5weight->value*weightPosition.x;
					vertPosition->y += md5weight->value*weightPosition.y;
					vertPosition->z += md5weight->value*weightPosition.z;
				}

				/* update the bounding box */
				(*glmesh)->subMeshes[i].min.x = fminf(
						(*glmesh)->subMeshes[i].min.x, vertPosition->x
					);

				(*glmesh)->subMeshes[i].min.y = fminf(
						(*glmesh)->subMeshes[i].min.y, vertPosition->y
					);

				(*glmesh)->subMeshes[i].min.z = fminf(
						(*glmesh)->subMeshes[i].min.z, vertPosition->z
					);
		
				(*glmesh)->subMeshes[i].max.x = fmaxf(
						(*glmesh)->subMeshes[i].max.x, vertPosition->x
					);

				(*glmesh)->subMeshes[i].max.y = fmaxf(
						(*glmesh)->subMeshes[i].max.y, vertPosition->y
					);

				(*glmesh)->subMeshes[i].max.z = fmaxf(
						(*glmesh)->subMeshes[i].max.z, vertPosition->z
					);

				count++;
			}	
		}
        
		(*glmesh)->min.x = fminf((*glmesh)->subMeshes[i].min.x, (*glmesh)->min.x);
		(*glmesh)->min.y = fminf((*glmesh)->subMeshes[i].min.y, (*glmesh)->min.y);
		(*glmesh)->min.z = fminf((*glmesh)->subMeshes[i].min.z, (*glmesh)->min.z);

		(*glmesh)->max.x = fmaxf((*glmesh)->subMeshes[i].max.x, (*glmesh)->max.x);
		(*glmesh)->max.y = fmaxf((*glmesh)->subMeshes[i].max.y, (*glmesh)->max.y);
		(*glmesh)->max.z = fmaxf((*glmesh)->subMeshes[i].max.z, (*glmesh)->max.z);

		/* initialize the opengl data for the sub mesh */
		glGenBuffers(1, &(*glmesh)->subMeshes[i].positions);
		glBindBuffer(GL_ARRAY_BUFFER, (*glmesh)->subMeshes[i].positions);  
		
		glBufferData(
			GL_ARRAY_BUFFER,
		 	sizeof(FxsVector3)*(*glmesh)->subMeshes[i].numPositions,
			(*glmesh)->subMeshes[i].positionsHost,
			GL_DYNAMIC_DRAW
		);
		
		glGenVertexArrays(1, &(*glmesh)->subMeshes[i].vao);
		glBindVertexArray((*glmesh)->subMeshes[i].vao);
		glBindBuffer(GL_ARRAY_BUFFER, (*glmesh)->subMeshes[i].positions);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
		if (GL_NO_ERROR != glGetError()) 
		{
			sprintf(errMsg, "Warning: opengl failed. Could not load md5mesh: %s", filename);
			ERR_MSG(errMsg);	
			FxsMD5MeshDestroy(&md5mesh);
			MD5OpenGLMeshDestroy(glmesh);
			return 0;		    
		}
	}
	
	return 1;
}

/*
** updates the md5mesh of mesh according to the passed animation and the frame.
** updates geometry on host and opengl side according to the updated pose.
*/ 
static int MD5OpenGLMeshUpdatePoseWithAnimationFrame(
	MD5OpenGLMesh* mesh,
	const FxsMD5Animation* animation, 
	unsigned int frame
)
{
	int i = 0, j = 0, k = 0, l = 0;
	FxsMD5Mesh* md5mesh = mesh->md5mesh;
	FxsMD5SubMesh* md5submesh = NULL;
	FxsMD5Face* face = NULL;
	FxsMD5Vertex* vertex = NULL;
	FxsMD5Weight* weight = NULL;	
	FxsMD5Joint* joint = NULL;
	FxsVector3* vertPosition = NULL;
	FxsVector4 weightPosition;
	unsigned int* vertexIds = NULL;
	MD5OpenGLSubMesh* glsubmesh = NULL;
	int count = 0;	

	if (!FxsMD5MeshUpdatePoseWithAnimationFrame(mesh->md5mesh, animation, frame))
	{
		return 0;
	}

	mesh->min.x = FLT_MAX;
	mesh->min.y = FLT_MAX;
	mesh->min.z = FLT_MAX;
	mesh->max.x = FLT_MIN;
	mesh->max.y = FLT_MIN;
	mesh->max.z = FLT_MIN;

	/* for all submeshes of the md5mesh, let's update the host and opengl data
	** of the opengl submeshes geometry (positions ...)
	*/ 
	for (i = 0; i < md5mesh->numSubMeshes; i++) 
	{
		glsubmesh = &mesh->subMeshes[i]; 		
		md5submesh = &md5mesh->meshes[i];		
		count = 0;

		mesh->subMeshes[i].min.x = FLT_MAX;
		mesh->subMeshes[i].min.y = FLT_MAX;
		mesh->subMeshes[i].min.z = FLT_MAX;
		mesh->subMeshes[i].max.x = FLT_MIN;
		mesh->subMeshes[i].max.y = FLT_MIN;
		mesh->subMeshes[i].max.z = FLT_MIN;

		/* for each face in the md5submesh ... */
		for (j = 0; j < md5submesh->numFaces; j++) 
		{
			face = &md5submesh->faces[j];		
			vertexIds = &face->v1;
		
			/* ... update each vertex */
			for (k = 0; k < 3; k++)
			{
				vertex = &md5submesh->vertices[vertexIds[k]];

				/* make the current vertex position zero */
				vertPosition = &mesh->subMeshes[i].positionsHost[count];
				FxsVector3MakeZero(vertPosition);

				/* update the position of each vertex by interation over
				** each of its weights
				*/
				for (l = 0; l < vertex->numWeights; l++)
				{
					weight = &md5submesh->weights[vertex->weightId + l];	
					joint = &md5mesh->currentPose.joints[weight->jointId];		
					
					FxsMatrix4MultiplyVector3(
						&weightPosition, 
						&joint->transform,
						&weight->position
					);

					/* add up all weight positions to compute the final
					** vertex position.
					*/
					vertPosition->x += weight->value*weightPosition.x;
					vertPosition->y += weight->value*weightPosition.y;
					vertPosition->z += weight->value*weightPosition.z;					
				}	

				/* update the bounding box */
				mesh->subMeshes[i].min.x = fminf(
						mesh->subMeshes[i].min.x, vertPosition->x
					);

				mesh->subMeshes[i].min.y = fminf(
						mesh->subMeshes[i].min.y, vertPosition->y
					);

				mesh->subMeshes[i].min.z = fminf(
						mesh->subMeshes[i].min.z, vertPosition->z
					);
		
				mesh->subMeshes[i].max.x = fmaxf(
						mesh->subMeshes[i].max.x, vertPosition->x
					);

				mesh->subMeshes[i].max.y = fmaxf(
						mesh->subMeshes[i].max.y, vertPosition->y
					);

				mesh->subMeshes[i].max.z = fmaxf(
						mesh->subMeshes[i].max.z, vertPosition->z
					);

				count++;
			}
		}
		
		mesh->min.x = fminf(mesh->subMeshes[i].min.x, mesh->min.x);
		mesh->min.y = fminf(mesh->subMeshes[i].min.y, mesh->min.y);
		mesh->min.z = fminf(mesh->subMeshes[i].min.z, mesh->min.z);

		mesh->max.x = fmaxf(mesh->subMeshes[i].max.x, mesh->max.x);
		mesh->max.y = fmaxf(mesh->subMeshes[i].max.y, mesh->max.y);
		mesh->max.z = fmaxf(mesh->subMeshes[i].max.z, mesh->max.z);

		/* update the opengl data for the sub mesh */
		glBindBuffer(GL_ARRAY_BUFFER, mesh->subMeshes[i].positions);  
	
		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
		 	sizeof(FxsVector3)*mesh->subMeshes[i].numPositions,
			mesh->subMeshes[i].positionsHost
		);
		
//		glGenVertexArrays(1, &(*glmesh)->subMeshes[i].vao);
//		glBindVertexArray((*glmesh)->subMeshes[i].vao);
//		glBindBuffer(GL_ARRAY_BUFFER, (*glmesh)->subMeshes[i].positions);
//		glEnableVertexAttribArray(0);
//		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	
		if (GL_NO_ERROR != glGetError()) 
		{
			sprintf(errMsg, "Warning: opengl failed. Could not update md5mesh");
			ERR_MSG(errMsg);	
			return 0;		    
		}	
	}
	
	return 1;
}

/*
** Destroys a MD5OpenGLMesh
*/ 
static void MD5OpenGLMeshDestroy(MD5OpenGLMesh** glmesh)
{
	int i = 0;

	if (!(*glmesh)) 
	{
	    return;
	}

	/* delete the md5 mesh */
	if ((*glmesh)->md5mesh)
	{
		FxsMD5MeshDestroy(&(*glmesh)->md5mesh);
	}

	/* delete the submeshes and their data */
	if ((*glmesh)->subMeshes)
	{
		for (i = 0; i < (*glmesh)->numSubMeshes; i++) 
		{
			if((*glmesh)->subMeshes[i].positions)
			{
				free((*glmesh)->subMeshes[i].positionsHost);
				glDeleteBuffers(1, &(*glmesh)->subMeshes[i].positions);
				glDeleteVertexArrays(1, &(*glmesh)->subMeshes[i].vao);
			}
		}
	}

	/* delete the gl mesh */
	free(*glmesh);
	
	*glmesh = NULL;
}

#define MAX_MESHES 32  /* max amount of meshes stored by the manager 
					   ** ids in the config file are allowed to be within
					   ** 0 .. MAX_MESHES - 1.	
					   */
#define MAX_ANIMATIONS 32 /* max amount of animations stored by the manager
					      ** ids in the config file are allowed to be within
					      ** 0 .. MAX_ANIMATIONS - 1.
					      */

MD5OpenGLMesh* meshes[MAX_MESHES];
FxsMD5Animation* animations[MAX_ANIMATIONS];

static int wasInitialized = 0;

int MD5OpenGLMeshManagerCreate(const char* filename)
{
	JSON_Value* root = NULL;
	JSON_Array* array = NULL;
    JSON_Object* rootObj = NULL;
	JSON_Object* object = NULL;
	size_t arraySize = 0;
	int i = 0; 
	const char* md5filename = NULL;
	int id = 0;
    MD5OpenGLMesh* mesh = NULL;
	FxsMD5Animation* animation = NULL;

    if (wasInitialized)
    {
        ERR_MSG("Warning: MD5OpenGLMeshManagerCreate was already initialized")
        return 0;
    }
    
	root = json_parse_file(filename);
		
	if (!root) 
	{
	    sprintf(errMsg, "Failed to parse file: %s", filename);
		ERR_MSG(errMsg)
		return 0;
	}
    
    rootObj = json_value_get_object(root);
    
	if (!rootObj)
	{
	    sprintf(errMsg, "Failed to parse file: %s", filename);
		ERR_MSG(errMsg)
		return 0;
	}
	
	/* load meshes */
	array = json_object_get_array(rootObj, "meshes");

	if (!array) 
	{
	    sprintf(errMsg, "Failed to parse file: %s", filename);
		ERR_MSG(errMsg)
 		json_value_free(root);
		return 0;
	}

    arraySize = json_array_get_count(array);

	for (i = 0; i < arraySize; i++) 
	{
	   	object = json_array_get_object(array, i);
	    id = json_object_get_number(object, "id");	
		md5filename = json_object_get_string(object, "filename");
	
		if (id < 0 || id > MAX_MESHES)
        {
            sprintf(errMsg, "Warning: Invalid id: %d. Id has to be inbetween 0 .. %d. Skipping mesh for file %s", id, MAX_MESHES - 1, md5filename);
            ERR_MSG(errMsg);
            continue;
        }
        
        if (meshes[id] != NULL)
        {
            sprintf(errMsg, "Warning: Mesh for id %d was already initialized. Skipping mesh for file %s", id, md5filename);
            ERR_MSG(errMsg);
            continue;
        }
        
        if (!MD5OpenGLMeshCreateWithFile(&mesh, md5filename))
        {
            sprintf(errMsg, "Warning: Failed to load mesh for: %s", md5filename);
            ERR_MSG(errMsg);
            continue;
        }
        
        meshes[id] = mesh;
	}
	
	/* load animations */		
	array = json_object_get_array(rootObj, "animations");

	if (!array) 
	{
	    sprintf(errMsg, "Failed to parse file: %s", filename);
		ERR_MSG(errMsg)
 		json_value_free(root);
		return 0;
	}

    arraySize = json_array_get_count(array);

	for (i = 0; i < arraySize; i++) 
	{
		object = json_array_get_object(array, i);
	    id = json_object_get_number(object, "id");	
		md5filename = json_object_get_string(object, "filename");

		if (id < 0 || id > MAX_ANIMATIONS)
        {
            sprintf(errMsg, "Warning: Invalid id: %d. Id has to be inbetween 0 .. %d. Skipping animation for file %s", id, MAX_ANIMATIONS - 1, md5filename);
            ERR_MSG(errMsg);
            continue;
        }
        
        if (animations[id] != NULL)
        {
            sprintf(errMsg, "Warning: Animation for id %d was already initialized. Skipping animation for file %s", id, md5filename);
            ERR_MSG(errMsg);
            continue;
        }
        
        if (!FxsMD5AnimationCreateWithFile(&animation, md5filename)) 
        {
            sprintf(errMsg, "Warning: Failed to load animation for: %s", md5filename);
            ERR_MSG(errMsg);
            continue;
        }

		animations[id] = animation;
	}

	/* clean up */
 	json_value_free(root);
    
    wasInitialized = 1;
    
    return 1;
}

const MD5OpenGLMesh* MD5OpenGLMeshManagerGetMeshWithId(int id)
{
    if (!wasInitialized)
    {
        ERR_MSG("Warning: MD5OpenGLMeshManagerCreate is not initialized")
        return 0;
    }

    if (id < 0 || id > MAX_MESHES)
    {
        sprintf(errMsg, "Warning: Invalid id: %d. Id has to be inbetween 0 .. %d.", id, MAX_MESHES - 1);
        ERR_MSG(errMsg);
        return NULL;
    }
    
    if (!meshes[id])
    {
        sprintf(errMsg, "Warning: Mesh with id: %d not found.", id);
        ERR_MSG(errMsg);
        return NULL;
    }
    
    return (const MD5OpenGLMesh*)meshes[id];
}

void MD5OpenGLMeshManagerDestroy()
{
    int i = 0;

    if (!wasInitialized)
    {
        ERR_MSG("Warning: MD5OpenGLMeshManagerCreate is not initialized")
    }
    
    for (i = 0; i < MAX_MESHES; i++)
    {
        if (meshes[i])
        {
            MD5OpenGLMeshDestroy(&meshes[i]);
        }
    }

    for (i = 0; i < MAX_ANIMATIONS; i++)
    {
        if (animations[i])
        {
            FxsMD5AnimationDestroy(&animations[i]);
        }
    }
}

const int MD5OpenGLMeshManagerUpdateMeshPoseWithAnimationFrame(
    int meshId,
    int animationId,
    int frame
)
{
    int f = 0;

    if (!wasInitialized)
    {
        ERR_MSG("Warning: MD5OpenGLMeshManagerCreate is not initialized")
        return 0;
    }
    
    if (meshId < 0 || meshId > MAX_MESHES)
    {
        sprintf(errMsg, "Warning: Invalid id: %d. Id has to be inbetween 0 .. %d.", meshId, MAX_MESHES - 1);
        ERR_MSG(errMsg);
        return 0;
    }
    
    if (meshes[meshId] == NULL)
    {
        sprintf(errMsg, "Warning: Mesh with id %d not found", meshId);
        ERR_MSG(errMsg);
        return 0;
    }
    
    if (animationId < 0 || animationId > MAX_ANIMATIONS)
    {
        sprintf(errMsg, "Warning: Invalid id: %d. Id has to be inbetween 0 .. %d. ", animationId, MAX_ANIMATIONS - 1);
        return 0;
    }
        
    if (animations[animationId] == NULL)
    {
        sprintf(errMsg, "Warning: Animation with id %d not found", animationId);
        ERR_MSG(errMsg);
        return 0;
    }
    
    if (frame < 0)
    {
        ERR_MSG("Frame index cannot be negative");
    }
    
    /* keep the frame between 0 .. animations[animationId]->numFrames */
    f = frame % animations[animationId]->numFrames;

    if (!MD5OpenGLMeshUpdatePoseWithAnimationFrame(meshes[meshId], animations[animationId], f))
    {
        ERR_MSG("Failed to update the opengl mesh");
        return 0;
    }

    return 1;
}

