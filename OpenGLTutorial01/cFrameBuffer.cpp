#include "cFrameBuffer.h"

cFrameBuffer::cFrameBuffer(unsigned int SCR_HEIGHT, unsigned int SCR_WIDTH)
{
	resize(SCR_HEIGHT, SCR_WIDTH);
}

void cFrameBuffer::resize(unsigned int SCR_HEIGHT, unsigned int SCR_WIDTH)
{
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &colorTextureID);
	glBindTexture(GL_TEXTURE_2D, colorTextureID);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, SCR_WIDTH, SCR_HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &normalTextureID);
	glBindTexture(GL_TEXTURE_2D, normalTextureID);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, SCR_WIDTH, SCR_HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &vertexTextureID);
	glBindTexture(GL_TEXTURE_2D, vertexTextureID);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, SCR_WIDTH, SCR_HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenTextures(1, &depthTextureID);
	glBindTexture(GL_TEXTURE_2D, depthTextureID);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, SCR_WIDTH, SCR_HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorTextureID, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, normalTextureID, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, vertexTextureID, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTextureID, 0);

	static const GLenum drawBuffers[] =
	{
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2
	};
	glDrawBuffers(3, drawBuffers);


	//GLenum statis = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	//#define WHEN_FBO_STATUS(statis) case statis: std::cout << "glCheckFramebufferStatus failed: " << #statis << std::endl; break;
	//switch (statis)
	//{
	//	WHEN_FBO_STATUS(GL_FRAMEBUFFER_UNDEFINED);
	//	WHEN_FBO_STATUS(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
	//	WHEN_FBO_STATUS(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
	//	WHEN_FBO_STATUS(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
	//	WHEN_FBO_STATUS(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
	//	WHEN_FBO_STATUS(GL_FRAMEBUFFER_UNSUPPORTED);
	//	WHEN_FBO_STATUS(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE);
	//default:
	//	assert(false);
	//}


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Frame Buffer = BAD" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}