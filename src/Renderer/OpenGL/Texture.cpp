#include "Ren/Renderer/OpenGL/Texture.h"
#include <exception>
#include <glad/glad.h>
#include <cstdio>
#include <cstring>
#include <stb_image.h>
#include <algorithm>

using namespace Ren;

Texture2D::Texture2D()
	: Width(0)
	, Height(0)
	, Internal_format(GL_RGB)
	, Image_format(GL_RGB)
	, Wrap_S(GL_REPEAT)
	, Wrap_T(GL_REPEAT)
	, Filter_min(GL_LINEAR)
	, Filter_mag(GL_LINEAR)
	, BindTarget(GL_TEXTURE_2D)
{
}
Texture2D::~Texture2D()
{	
}
unsigned int get_nearest_multiple_two(unsigned int n)
{
	n--;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n++;
	return n;
}

void Texture2D::Generate(unsigned int width, unsigned int height, unsigned char* data)
{
	glGenTextures(1, &this->ID);

	this->Width = width;//get_nearest_multiple_two(width);
	this->Height = height;//get_nearest_multiple_two(height);

	glBindTexture(GL_TEXTURE_2D, this->ID);
	glTexImage2D(GL_TEXTURE_2D, 0, Internal_format, width, height, 0, Image_format, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Wrap_S);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Wrap_T);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter_min);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter_mag);

	glBindTexture(GL_TEXTURE_2D, 0);
}
Texture2D& Texture2D::UpdateParameters()
{
	glBindTexture(GL_TEXTURE_2D, this->ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Wrap_S);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Wrap_T);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Filter_min);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Filter_mag);
	glBindTexture(GL_TEXTURE_2D, 0);
	return *this;
}

void Texture2D::Bind() const
{
	glBindTexture(GL_TEXTURE_2D, this->ID);
}

Texture2D& Texture2D::Resize(int new_width, int new_height)
{
	glBindTexture(GL_TEXTURE_2D, this->ID);
	glTexImage2D(GL_TEXTURE_2D, 0, Internal_format, new_width, new_height, 0, Image_format, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	return *this;
}
void Texture2D::Delete()
{
	glDeleteTextures(1, &ID);
}

// ===============================
// RawTextre -- Loading using STBI
// ===============================
RawTexture RawTexture::Load(const char* filename)
{
	RawTexture tex;
	tex.data = stbi_load(filename, (int*)&tex.width, (int*)&tex.height, (int*)&tex.channel_count, 0);

	if (!tex.data)
		LOG_E("Failed loading raw texture at path: " + std::string(filename));

	return tex;
}
void RawTexture::Delete(RawTexture& texture)
{
	stbi_image_free(texture.data);
	texture.data = nullptr;
}

// =============================
// TextureBatch
// =============================
TextureBatch::TextureBatch()
	: mLayers()
	, mTextureDescriptors()
{
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, (GLint*)&mMaxTextureWidth);
	mMaxTextureHeight = mMaxTextureWidth;
	Width = 0;
	Height = 0;
	Filter_mag = GL_NEAREST;
}

bool TextureBatch::insertToLayerAndSetOffset(uint32_t n_layer, const prebuf_elem& elem)
{
	bool new_layer = n_layer >= mLayers.size();
	TextureDescriptor& desc = mTextureDescriptors[elem.desc_i];

	// Create new layer if not exists.
	if (new_layer)
	{
		layer l;
		l.top_offset = mLayers.size() == 0 ? 0 : mLayers.back().top_offset + mLayers.back().height;
		l.current_width = 0;
		l.height = desc.size.y + 2 * mTextureMargin;	// Margin on the top and bottom.

		if (l.top_offset + l.height > mMaxTextureHeight)
			return false;

		n_layer = mLayers.size();
		mLayers.push_back(l);
	}
	layer& rLayer = mLayers[n_layer];

	// When height of the texture if bigger then the layer
	// or when the layer is too wide, try insertion to next layer.
	if ((uint32_t)desc.size.y > rLayer.height)
		return insertToLayerAndSetOffset(n_layer + 1, elem);
	if (rLayer.current_width + desc.size.x + 2 * mTextureMargin > mMaxTextureHeight)
		return insertToLayerAndSetOffset(n_layer + 1, elem);
	
	// Set offsets and expand the layer.
	desc.offset.x = rLayer.current_width + mTextureMargin;
	desc.offset.y = rLayer.top_offset + mTextureMargin;
	rLayer.current_width += desc.size.x + 2 * mTextureMargin;

	return true;
}

int32_t TextureBatch::AddTexture(const RawTexture& texture)
{
	REN_ASSERT(texture.channel_count != 0, "Cannot add texture to batch if the channel count isn't specified.");

	// Get texture descriptor
	TextureDescriptor desc;
	desc.offset = {0.0f, 0.0f};
	desc.size = {texture.width, texture.height};
	desc.pTexture = this;
	desc.descriptor_id = mTextureDescriptors.size();
	mTextureDescriptors.push_back(desc);

	// Store in prebuffer. To be later evaluated by the Build() method.
	prebuf_elem e;
	e.desc_i = desc.descriptor_id;
	e.size = desc.size.x * desc.size.y;
	e.channel_count = texture.channel_count;
	e.data_copy = new uint8_t[texture.width * texture.height * texture.channel_count];	// Gets deleted in TextureBatch::Build()
	std::memcpy(e.data_copy, texture.data, texture.width * texture.height * texture.channel_count);
	mPrebuffer.push_back(e);

	return desc.descriptor_id;
}

void TextureBatch::Build()
{
	REN_ASSERT(mTextureDescriptors.size() != 0, "0 textures provided");

	if (SortBySize) {
		// Sort the prebuffed textures by size
		std::sort(mPrebuffer.begin(), mPrebuffer.end(), [](const prebuf_elem& a, const prebuf_elem& b){ 
			return a.size > b.size; 
		});
	}
	
	// Create layered structure.
	for (auto&& elem : mPrebuffer) 
	{
		bool success = insertToLayerAndSetOffset(0, elem);
		if (!success)
			LOG_E("Could not add texture to batch!");
	}

	// Get batch texture dimensions.
	Height = mLayers.back().top_offset + mLayers.back().height;
	for (auto&& i : mLayers)
		Width = std::max(Width, i.current_width);

	// Allocate space for batch texture, which will later be copied to GPU memory.
	mBuffer = new uint8_t[Width * Height * ChannelCount];
	std::memset(mBuffer, 0, Width * Height * ChannelCount);

	// Copy each prebuffered texture pixel into its correct position in buffer.
	for (auto&& e : mPrebuffer)
	{
		TextureDescriptor& desc = mTextureDescriptors[e.desc_i];
		uint32_t local_width = desc.size.x;
		uint32_t local_height = desc.size.y;

		// Copy texture data into buffer
		for (uint32_t i = 0, local_x = 0, local_y = 0, buf_offset = 0; i < local_width * local_height; i++)
		{
			local_x = i % local_width;
			local_y = i / local_width;

			buf_offset = (desc.offset.y + local_y) * Width + (desc.offset.x + local_x);

			std::memset(&mBuffer[buf_offset * ChannelCount], 0, ChannelCount);
			if (ChannelCount == 4)
				std::memset(&mBuffer[buf_offset * 4 + 3], 255, 1);
			std::memcpy(&mBuffer[buf_offset * ChannelCount], &e.data_copy[i * e.channel_count], std::min(ChannelCount, e.channel_count));
		}
		
		// Create border from margin spacing.
		for (int i = 0; i < mTextureMargin; i++)
			createBorder(desc.offset - glm::ivec2(i), desc.size + 2 * glm::ivec2(i));

		delete[] e.data_copy;
	}
	mPrebuffer.clear();

	// TODO: Maybe implement support for some HDR internal formats (e.g. GL_RGBA16F) ?
	switch (ChannelCount)
	{
		case 1: Image_format = GL_RED; break;
		case 3: Image_format = GL_RGB; break;
		case 4: Image_format = GL_RGBA; break;
	}
	Internal_format = Image_format;
	Generate(Width, Height, mBuffer);
	delete[] mBuffer;
}

void TextureBatch::createBorder(glm::ivec2 offset, glm::ivec2 size)
{
	const auto& buf = [&](const glm::ivec2& pos) { return (pos.y * Width + pos.x) * ChannelCount; };

	// Copy top edge
	std::memcpy(&mBuffer[buf(offset - glm::ivec2(0, 1))], &mBuffer[buf(offset)], size.x * ChannelCount);
	// Copy bottom edge
	std::memcpy(&mBuffer[buf(offset + glm::ivec2(0, size.y))], &mBuffer[buf(offset + glm::ivec2(0, size.y - 1))], size.x * ChannelCount);
	// Copy left edge
	for (int i = 0; i < size.y; i++)
		std::memcpy(&mBuffer[buf(offset + glm::ivec2(-1, i))], &mBuffer[buf(offset + glm::ivec2(0, i))], ChannelCount);
	// Copy right edge
	for (int i = 0; i < size.y; i++)
		std::memcpy(&mBuffer[buf(offset + glm::ivec2(size.x, i))], &mBuffer[buf(offset + glm::ivec2(size.x - 1, i))], ChannelCount);
	// Copy left up corner
	std::memcpy(&mBuffer[buf(offset - glm::ivec2(1))], &mBuffer[buf(offset)], ChannelCount);
	// Copy right up corner
	std::memcpy(&mBuffer[buf(offset + glm::ivec2(size.x, -1))], &mBuffer[buf(offset + glm::ivec2(size.x - 1, 0))], ChannelCount);
	// Copy left down corner
	std::memcpy(&mBuffer[buf(offset + glm::ivec2(-1, size.y))], &mBuffer[buf(offset + glm::ivec2(0, size.y - 1))], ChannelCount);
	// copy right down corner
	std::memcpy(&mBuffer[buf(offset + size)], &mBuffer[buf(offset + size - glm::ivec2(1))], ChannelCount);
}

