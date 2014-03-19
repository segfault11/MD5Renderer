/*
 * Renderes MD5 meshes.
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
#ifndef MD5OPENGLRENDERER_H
#define MD5OPENGLRENDERER_H

#ifdef __cplusplus
extern "C"
{
#endif

/*
** Creates the renderer. filename refers to the config file for the renderer.
** returns 0 if it fails.
** 
** The config file currently stores filenames for md5 meshes and animations
** and associates an index with each of them. The following is an example:
**
**      {
**
**          "meshes" :
**          [
**              {
**                  "id" : 0,
**                  "filename" : "hellknight.md5mesh"
**              }
**          ],
**
**          "animations" :
**          [
**              {
**                  "id" : 0,
**                  "filename" : "idle2.md5anim"
**              }
**          ]
**
**      }
*/ 
int FFMD5OpenGLRendererCreate(const char* filename);

/*
** Renders the mesh with id; uses the frame of animation with animation id.
*/ 
int FFMD5OpenGLRendererRender(int meshId, int animationId, int frame);

/*
** Sets the model matrix. Initially it is the identity.
** @param model a float array with 16 elements, representing and opengl 
**              model matrix (gl => column major)
*/
void FFMD5OpenGLRendererSetModelMatrix(const float* model);

/*
** Sets the view matrix. Initially it is the identity.
** @param view a float array with 16 elements, representing and opengl
**             view matrix (gl => column major)

*/
void FFMD5OpenGLRendererSetViewMatrix(const float* view);

/*
** Sets the projection matrix. Initially it is the identity.
** @param projection a float array with 16 elements, representing and opengl
**                   perspective matrix (gl => column major)
*/
void FFMD5OpenGLRendererSetProjectionMatrix(const float* projection);

/*
** Destroys the renderer.
*/ 
void FFMD5OpenGLRendererDestroy();

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: MD5OPENGLRENDERER_H */
