#ifndef _SHADOW_MAP_HG_
#define _SHADOW_MAP_HG_

#include <glad\glad.h>
#include <GLFW\glfw3.h>

class cShader;

class cShadowMap
{
public:
	cShadowMap(GLuint width, GLuint height);
	void BindFBO();
	void UnbindFBO();
	void ClearBuffers(bool clearColor = true, bool clearDepth = true);
	void Draw();
	void Reset(GLuint width, GLuint height);
	GLuint texDepthBuffer;
	GLuint sWidth;
	GLuint sHeight;
private:
	GLuint fbo;
	GLuint rbo;
	bool fboInit();

	GLuint VAO;
	GLuint VBO;
};
#endif // !_FBO_HG
