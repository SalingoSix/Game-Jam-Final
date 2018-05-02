#include "cShadowMap.h"
#include <iostream>

cShadowMap::cShadowMap(GLuint width, GLuint height)
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

	fboInit();
	return;
}

void cShadowMap::BindFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
}
void cShadowMap::UnbindFBO()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void cShadowMap::ClearBuffers(bool clearColor, bool clearDepth)
{
	glViewport(0, 0, this->sWidth, this->sHeight);
	GLfloat zero = 0.0f;
	GLfloat one = 1.0f;
	if (clearColor)
		glClearBufferfv(GL_COLOR, 0, &zero);
	if (clearDepth)
		glClearBufferfv(GL_DEPTH, 0, &one);
}

bool cShadowMap::fboInit()
{
	bool ret = false;
	glGenFramebuffers(1, &this->fbo);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	//GLfloat border[] = { 1.0f, 0.0f, 0.0f, 0.0f };
	// Depth Buffer
	glGenTextures(1, &this->texDepthBuffer);
	glBindTexture(GL_TEXTURE_2D, this->texDepthBuffer);
	//glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, this->sWidth, this->sHeight);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, this->sWidth, this->sHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);




	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->texDepthBuffer, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Shadow MAP created successfully." << std::endl;
		ret = true;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return ret;
}
void cShadowMap::Reset(GLuint width, GLuint height)
{
	glDeleteTextures(1, &(this->texDepthBuffer));

	glDeleteFramebuffers(1, &(this->fbo));

	this->sWidth = width;
	this->sHeight = height;
	this->fboInit();
}

void cShadowMap::Draw()
{
	glBindVertexArray(VAO);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}