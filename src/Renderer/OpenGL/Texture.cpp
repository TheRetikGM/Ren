#include "Ren/Renderer/OpenGL/Texture.h"
#include <exception>
#include <glad/glad.h>
#include <cstdio>
#include <cstring>
#include <stb_image.h>
#include <algorithm>

using namespace Ren;

uint8_t formatToChannelCount(uint32_t format)
{
	switch (format)
	{
		case GL_RED:	 return 1; break;
		case GL_RG: 
		case GL_RG16F:	 return 2; break;
		case GL_RGB:
		case GL_RGB16F:  return 3; break;
		case GL_RGBA:
		case GL_RGBA16F: return 4; break;
		default: 
			return 3; 
			break;
	}
}

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

	REN_ASSERT(tex.data, "Failed loading raw texture at path: " + std::string(filename));

	tex.mStbiLoaded = true;
	return tex;
}
void RawTexture::Delete()
{
	if (data)
	{
		if (mStbiLoaded)
			stbi_image_free(data);
		else
			delete[] data;
	}
	mStbiLoaded = false;
	data = nullptr;
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
TextureBatch::~TextureBatch()
{
	Delete();
}
Ref<TextureBatch> TextureBatch::Create()
{
	return Ref<TextureBatch>(new TextureBatch());
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
	desc.descriptor_id = mAvailableID++;
	desc.ready_for_usage = false;
	mTextureDescriptors.emplace(desc.descriptor_id, desc);

	// Store in prebuffer. To be later evaluated by the Build() method.
	prebuf_elem e;
	e.desc_i = desc.descriptor_id;
	e.size = desc.size.x * desc.size.y;
	e.channel_count = texture.channel_count;
	e.data_copy = new uint8_t[texture.width * texture.height * texture.channel_count];	// Gets deleted in TextureBatch::Build()
	std::memcpy(e.data_copy, texture.data, texture.width * texture.height * texture.channel_count);
	mPrebuffer.push_back(e);


	// Perform insertion check.
	if (!SortBySize) 
	{
		if (insertToLayerAndSetOffset(0, mPrebuffer.back())) 
		{
			mDirty = true;
			return desc.descriptor_id;
		}
		else 
		{
			delete[] mPrebuffer.back().data_copy;
			mPrebuffer.pop_back();
			mTextureDescriptors.erase(mAvailableID--);
			return -1;
		}
	}

	mDirty = true;
	return desc.descriptor_id;
}
void TextureBatch::DeleteTexture(int32_t id)
{
	REN_ASSERT(mTextureDescriptors.count(id) != 0, "Invalid descriptor ID");
	mTextureDescriptors.erase(id);

	if (!mCreated)
	{
		// Delete from prebuffer.
		auto it = std::find_if(mPrebuffer.begin(), mPrebuffer.end(), [&id](const prebuf_elem& e) { return e.desc_i == id; });
		if (it != mPrebuffer.end())
		{
			if (it->data_copy)
				delete[] it->data_copy;
			mPrebuffer.erase(it);
		}
	}
	
	mDirty = true;
}
void TextureBatch::Build()
{
	REN_ASSERT(mTextureDescriptors.size() != 0, "0 textures provided");
	REN_ASSERT(!mCreated, "Batch is already built. For re-build use the Renew() method.");
	REN_ASSERT(mPrebuffer.size() != 0, "There are 0 textures in prebuffer.");

	if (SortBySize) {
		// Sort the prebuffed textures by size
		std::sort(mPrebuffer.begin(), mPrebuffer.end(), [](const prebuf_elem& a, const prebuf_elem& b){ 
			return a.size > b.size; 
		});
		// Create layered structure.
		for (auto&& elem : mPrebuffer) 
		{
			bool success = insertToLayerAndSetOffset(0, elem);
			if (!success)
				LOG_E("Could not add texture to batch!");
		}
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

		desc.ready_for_usage = true;
	}
	mPrebuffer.clear();

	// TODO: Maybe implement support for some HDR internal formats (e.g. GL_RGBA16F) ?
	switch (ChannelCount)
	{
		case 1: Image_format = GL_RED; break;
		case 2: Image_format = GL_RG; break;
		case 3: Image_format = GL_RGB; break;
		case 4: Image_format = GL_RGBA; break;
	}
	Internal_format = Image_format;
	// Disable byte alignment restriction.
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	Generate(Width, Height, mBuffer);
	delete[] mBuffer;

	mCreated = true;
	mDirty = false;
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
void TextureBatch::Renew()
{
	REN_ASSERT(mCreated, "Cannot renew batch if it was not built yet.");
	REN_ASSERT(mPrebuffer.size() == 0, "There already are elements in prebuffer. Cannot renew.");

	if (!mDirty)
		return;

	// Copy texture from GPU memory to RAM and delete the GL texture.
	RawTexture tex = Utils::GLToRawTexture(*(dynamic_cast<Texture2D*>(this)));
	this->Delete();		// Delete the texture in GPU memory.

	// Create prebuffer elements from descriptors.
	for (auto&& [desc_id, desc] : mTextureDescriptors)
	{
		prebuf_elem e;
		e.data_copy = new uint8_t[desc.size.x * desc.size.y * ChannelCount];	// To be deleted in Build() method.
		e.desc_i = desc_id;
		e.channel_count = ChannelCount;
		e.size = desc.size.x  * desc.size.y;

		// Copy each pixel from big texture, which is corresponding to 
		// this texture into its own prebuffer.
		for (uint32_t i = 0; i < e.size; i++)
		{
			uint32_t pixel_x = desc.offset.x + (i % desc.size.x);
			uint32_t pixel_y = desc.offset.y + (i / desc.size.x);
			std::memcpy(&e.data_copy[i * ChannelCount], &tex.data[(pixel_y * desc.size.x + pixel_x) * ChannelCount], ChannelCount);
		}
		mPrebuffer.push_back(e);
	}

	// Re-build the batch.
	delete tex.data;
	mCreated = false;
	Build();
}


/////////////////////////////////////////////////////////////////////////
///////////////////////////////// Utils /////////////////////////////////
/////////////////////////////////////////////////////////////////////////
#include "Ren/Renderer/OpenGL/Framebuffer.h"
#include <stb_image_write.h>

RawTexture Utils::GLToRawTexture(const Texture2D& gl_texture)
{
	// Create frambuffer color attachment, with this GL texture as a storage.
	auto a = std::make_shared<FramebufferAttachment>();
	a->internalFormat = (AttachmentFormats)gl_texture.Internal_format;
	a->storageType = StorageType::TEXTURE;
	a->bufferID = gl_texture.ID;
	a->storage = gl_texture;

	// Generate FBO.
	auto fbo = std::make_shared<Framebuffer>();
	fbo->AddAttachment(a);
	fbo->Generate(gl_texture.Width, gl_texture.Height);

	// Set initial params for raw texture.
	RawTexture tex;
	tex.channel_count = formatToChannelCount(gl_texture.Internal_format);
	tex.data = new uint8_t[gl_texture.Width * gl_texture.Height * tex.channel_count];
	tex.width = gl_texture.Width;
	tex.height = gl_texture.Height;

	// Backup binding.
	uint32_t fbo_binding;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&fbo_binding);

	// Copy pixels from color attachment to raw texture's data.
	fbo->Bind();
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, gl_texture.Width, gl_texture.Height, gl_texture.Internal_format, GL_UNSIGNED_BYTE, tex.data);

	// Restore OpenGL state and delete FBO.
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_binding);
	fbo.reset();
	a.reset();

	return tex;
}
void Utils::SaveTexturePNG(const char* filename, const RawTexture& texture, bool flip_vertically)
{
	stbi_flip_vertically_on_write(int(flip_vertically));
	stbi_write_png(filename, texture.width, texture.height, texture.channel_count, texture.data, texture.width * texture.channel_count);
}