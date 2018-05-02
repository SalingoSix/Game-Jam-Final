#include "cHDR.h"
#include <iostream>

cHDR::cHDR(GLuint width, GLuint height, bool isMultisampled)
{
	this->sWidth = width;
	this->sHeight = height;

	float quadVertices[] = {
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);

	if (isMultisampled)
		multisampledFboInit();
	else
		fboInit();
	return;
}

void cHDR::BindRead()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, this->fbo);
}
void cHDR::BindDraw()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, this->fbo);
}

void cHDR::BlitFBO()
{
	glBlitFramebuffer(0, 0, this->sWidth, this->sHeight, 0, 0, this->sWidth, this->sHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}
void cHDR::BindFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
}
void cHDR::UnbindFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void cHDR::ClearBuffers(bool clearColor, bool clearDepth)
{
	glViewport(0, 0, this->sWidth, this->sHeight);
	GLfloat zero = 0.0f;
	GLfloat one = 1.0f;
	if (clearColor)
		glClearBufferfv(GL_COLOR, 0, &zero);
	if (clearDepth)
		glClearBufferfv(GL_DEPTH, 0, &one);
}

bool cHDR::fboInit()
{
	bool ret = false;
	glGenFramebuffers(1, &this->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// Color Buffer
	glGenTextures(1, &this->texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, this->texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, sWidth, sHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &this->texBloomBuffer);
	glBindTexture(GL_TEXTURE_2D, this->texBloomBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, sWidth, sHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Depth Buffer
	glGenTextures(1, &this->texDepthBuffer);
	glBindTexture(GL_TEXTURE_2D, this->texDepthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, sWidth, sHeight, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	// Create the frame buffer textures
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, this->texColorBuffer, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, this->texBloomBuffer, 0);

	static const GLenum drawBuffers[] =
	{
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1
	};
	glDrawBuffers(2, drawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "HDR FBO created successfully." << std::endl;
		ret = true;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return ret;
}

bool cHDR::multisampledFboInit()
{
	bool ret = false;
	glGenFramebuffers(1, &this->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// Color Buffer
	glGenTextures(1, &this->texColorBuffer);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, this->texColorBuffer);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA32F, this->sWidth, this->sHeight, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	// Depth Buffer
	glGenTextures(1, &this->texDepthBuffer);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, this->texDepthBuffer);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_DEPTH_COMPONENT32F, this->sWidth, this->sHeight, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Create the frame buffer textures
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, this->texColorBuffer, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, this->texDepthBuffer, 0);

	static const GLenum drawBuffers[] =
	{
		GL_COLOR_ATTACHMENT0
	};
	glDrawBuffers(1, drawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "FBO created successfully." << std::endl;
		ret = true;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return ret;
}
void cHDR::Reset(GLuint width, GLuint height)
{
	glDeleteTextures(1, &(this->texColorBuffer));
	glDeleteTextures(1, &(this->texDepthBuffer));

	glDeleteFramebuffers(1, &(this->fbo));

	this->sWidth = width;
	this->sHeight = height;
	this->fboInit();
}

void cHDR::Draw()
{
	glBindVertexArray(VAO);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}