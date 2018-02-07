#include "OpenGLRenderer.h"

OpenGLRenderer::OpenGLRenderer(int screenWidth, int screenHeight, VolumeDataset &volume, Camera &camera)
{
	currTexture3D = GenerateTexture(volume);

	raycaster = new GPURaycaster(screenWidth, screenHeight, volume);
	transferFunction.Init(" ", volume);
}


// Updates the texture by copying the block corresponding to the current timestep to GPU memory
void OpenGLRenderer::UpdateTexture(int currentTimestep, VolumeDataset &volume)
{
	glBindTexture(GL_TEXTURE_3D, currTexture3D);

	if (!volume.littleEndian)
		glPixelStoref(GL_UNPACK_SWAP_BYTES, true);

	if (volume.elementType == "MET_UCHAR")
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, volume.xRes, volume.yRes, volume.zRes, 0,  GL_RED, GL_UNSIGNED_BYTE, volume.memblock3D + (textureSize * currentTimestep));

	else if (volume.elementType == "SHORT")
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, volume.xRes, volume.yRes, volume.zRes, 0, GL_RED, GL_UNSIGNED_SHORT, volume.memblock3D + (textureSize * currentTimestep));

	else if (volume.elementType == "FLOAT")
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, volume.xRes, volume.yRes, volume.zRes, 0, GL_RED, GL_FLOAT, volume.memblock3D + (textureSize * currentTimestep));

	glPixelStoref(GL_UNPACK_SWAP_BYTES, false);

	glBindTexture(GL_TEXTURE_3D, 0);
}


// Generates the original 3D texture
GLuint OpenGLRenderer::GenerateTexture(VolumeDataset &volume)
{
	GLuint tex;
	textureSize = volume.xRes * volume.yRes * volume.zRes * volume.bytesPerElement;

	glEnable(GL_TEXTURE_3D);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_3D, tex);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


	// Reverses endianness in copy
	if (!volume.littleEndian)
		glPixelStoref(GL_UNPACK_SWAP_BYTES, true);

	if (volume.elementType == "MET_UCHAR")
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, volume.xRes, volume.yRes, volume.zRes, 0,  GL_RED, GL_UNSIGNED_BYTE, volume.memblock3D);

	else if (volume.elementType == "SHORT")
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, volume.xRes, volume.yRes, volume.zRes, 0, GL_RED, GL_UNSIGNED_SHORT, volume.memblock3D);

	else if (volume.elementType == "FLOAT")
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, volume.xRes, volume.yRes, volume.zRes, 0, GL_RED, GL_FLOAT, volume.memblock3D);

	glPixelStoref(GL_UNPACK_SWAP_BYTES, false);

	GLenum err = glGetError();
	
	glBindTexture(GL_TEXTURE_3D, 0);

	return tex;
}


void OpenGLRenderer::Draw(VolumeDataset &volume, Camera &camera)
{
	GLuint shaderProgramID = ShaderManager::UseShader(TFShader);
	raycaster->Raycast(currTexture3D, transferFunction, shaderProgramID, camera);
}

