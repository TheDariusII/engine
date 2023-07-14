#include "Texture.h"

Texture::Texture(const std::string& path_in, unsigned int internalFormat, unsigned int format) : path(path_in) {
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
	OpenglWrapper::GenerateTextures(&texture);
	OpenglWrapper::BindTexture(static_cast<int>(texture));
	if (data) {
		OpenglWrapper::ImageTexture(internalFormat, width, height, data);
		OpenglWrapper::GenerateMipmap();
	}
	stbi_image_free(data);
}

void Texture::activate(GLenum unit, GLenum target) {
	OpenglWrapper::ActivateTexture(unit);
	OpenglWrapper::BindTexture(static_cast<int>(texture), target);
}

size_t Texture::get_texture() {
	return texture;
}

std::string Texture::get_type() {
	return type;
}

std::string Texture::get_path() {
	return path;
}

