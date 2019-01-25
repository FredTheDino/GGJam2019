#ifndef __BLOCK_TEXTURE__
#define __BLOCK_TEXTURE__

// TODO: Should I move this to the graphics part?

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

GLuint filter = GL_NEAREST;
Asset load_texture(const char *path)
{
	Texture texture = {};
	u8 *data = stbi_load(path, &texture.width, &texture.height, &texture.components, 0);
	if (!data)
	{
		print("|| Failed to load image \"%s\"\n", path);
		return {};
	}
	
	glGenTextures(1, &texture.id);
	glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	GLuint wrap = GL_CLAMP;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

	GLuint format = 0;
	switch(texture.components)
	{
		case 1:
			format = GL_RED;
			break;
		case 2:
			format = GL_RG;
			break;
		case 3:
			format = GL_RGB;
			break;
		case 4:
			format = GL_RGBA;
			break;
		default:
			print("|| Unsopported number of components for \"%s\" (%d)\n", path, texture.components);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
			texture.width, texture.height, 
			0, format, GL_UNSIGNED_BYTE, data);
	
	stbi_image_free(data);

	Asset asset;
	asset.texture = texture;

	return asset;
}


void reload_texture(Asset *asset)
{
	ASSERT(asset->type == AFT_TEXTURE);
	Texture texture = asset->texture;

	s32 width, height, comp;
	u8 *data = stbi_load(asset->path, &width, &height, &comp, 0);
	if (!data)
	{
		print("|| Failed to load image \"%s\"\n", asset->path);
		return;
	}

	GLuint format;
	switch(comp)
	{
		case 1:
			format = GL_RED;
			break;
		case 2:
			format = GL_RG;
			break;
		case 3:
			format = GL_RGB;
			break;
		case 4:
			format = GL_RGBA;
			break;
		default:
			print("|| Unsopported number of components for \"%s\" (%d)\n", asset->path, texture.components);
	}

	if (width != texture.width || height != texture.height || comp != texture.components)
	{
		// We nead to re create the texture since it's a different size.
		glDeleteTextures(1, &texture.id);

		texture.width = width;
		texture.height = height;
		glGenTextures(1, &texture.id);
		glBindTexture(GL_TEXTURE_2D, texture.id);
		GLuint filter = GL_LINEAR;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
		GLuint wrap = GL_CLAMP;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	}

	print("|| Reloaded texture \"%s\"\n", asset->path);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
			texture.width, texture.height, 
			0, format, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);
	asset->texture = texture;
}

void destroy_texture(Asset asset)
{
	ASSERT(asset.type == AFT_TEXTURE);
	Texture texture = asset.texture;
	glDeleteTextures(1, &texture.id);
}


void bind_texture(Texture texture, u32 slot=0)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, texture.id);
}

#endif
