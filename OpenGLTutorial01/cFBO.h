#ifndef _FBO_HG_
#define _FBO_HG_

#include <glad\glad.h>
#include <GLFW\glfw3.h>

class cShader;

class cFBO
{
public:
	cFBO(GLuint width, GLuint height, bool isMultisampled = true);
	void BindFBO();
	void BindRead();
	void BindDraw();
	void UnbindFBO();
	void ClearBuffers(bool clearColor = true, bool clearDepth = true);
	void Draw();
	void BlitFBO();
	GLuint texColorBuffer;
	GLuint texNormalBuffer;
	GLuint texVertexBuffer;
	GLuint texMotionBuffer;
	GLuint texDepthBuffer;
	GLint  numPass;
	GLuint sWidth;
	void Reset(GLuint width, GLuint height);
	GLuint sHeight;
	GLuint fbo;
private:
	GLuint rbo;
	bool fboInit();
	bool multisampledFboInit();
	bool rboInit();

	GLuint VAO;
	GLuint VBO;
};
#endif // !_FBO_HG
