#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <Ren/Core.h>
#include <unordered_map>
#include <tuple>

namespace Ren
{
	class Texture2D
	{
	public:
		unsigned int ID;
		unsigned int Width, Height;
		unsigned int Internal_format;
		unsigned int Image_format;

		unsigned int Wrap_S, Wrap_T;
		unsigned int Filter_min, Filter_mag;

		bool FlipVertically = false;
		bool FlipHorizontally = false;

		unsigned int BindTarget;

		Texture2D();
		virtual ~Texture2D();

		void Generate(unsigned int width, unsigned int height, unsigned char* data = 0);
		Texture2D& UpdateParameters();

		Texture2D& SetMagFilter(int filter) { Filter_mag = filter; return *this; }
		Texture2D& SetMinFilter(int filter) { Filter_min = filter; return *this; }
		Texture2D& SetWrapS(int mode) { Wrap_S = mode; return *this; }
		Texture2D& SetWrapT(int mode) { Wrap_T = mode; return *this; }

		Texture2D& Resize(int new_width, int new_height);
		void Delete();

		void Bind() const;
	};

	struct TextureDescriptor
	{
		Texture2D* pTexture = nullptr;
		uint32_t descriptor_id = 0;
		glm::ivec2 offset = glm::vec2(0.0f);
		glm::ivec2 size = glm::vec2(0.0f);
	};
	struct RawTexture
	{
		uint8_t* data = nullptr;
		uint32_t width = 0;
		uint32_t height = 0;
		uint8_t channel_count = 0;

		// Load texture using STBI
		static RawTexture Load(const char* filename);
		// Free data pointed to by data using STBI
		void Delete();
	};

	class TextureBatch : public Texture2D
	{
	public:
		bool SortBySize = false;
		uint8_t ChannelCount = 4; // RGBA
		
		~TextureBatch();

		static Ref<TextureBatch> Create();

		// Check if texture fits into the batch. If it does not fit,
		// then AddTexture() method will throw an Exception.
		// bool CanAddTexture(const RawTexture& texture) const { return true; }

		// Add texture into the batch and return its id
		int32_t AddTexture(const RawTexture& texture);
		void DeleteTexture(uint32_t id) {}

		void Build();
		// Create the batch texture again. Should be used, if new
		// texture is added after the creation.
		void Renew() {}

		const TextureDescriptor& GetTextureDescriptor(uint32_t id) { return mTextureDescriptors[id]; }

	protected:
		struct prebuf_elem {
			uint32_t desc_i;
			uint32_t size;
			uint8_t* data_copy;
			uint8_t channel_count;
		};
		struct layer {
			uint32_t top_offset;
			uint32_t current_width;
			uint32_t height;
		};

		std::vector<layer> mLayers;
		std::vector<TextureDescriptor> mTextureDescriptors;
		// Buffer for temprarily storing textures, before they are moved to GPU memory.
		uint8_t* mBuffer = nullptr;
		// Store texture data, before actually copying it to mBuffer as a final step.
		std::vector<prebuf_elem> mPrebuffer;
		// Check if batch is already created --> textures are batched together. 
		bool mCreated = false;
		uint32_t mMaxTextureWidth;
		uint32_t mMaxTextureHeight;
		uint8_t mTextureMargin = 2;	// Two pixels margin.

		TextureBatch();

		// Recursively using layering structure, set element offset in the batch texture.
		bool insertToLayerAndSetOffset(uint32_t n_layer, const prebuf_elem& elem);
		// Create 1px border around specified region. Border acts as WRAP TO EDGE.
		void createBorder(glm::ivec2 offset, glm::ivec2 size);
	};

	namespace Utils
	{
		RawTexture GLToRawTexture(const Texture2D& gl_texture);
		void SaveTexturePNG(const char* filename, const RawTexture& texture, bool flip_vertically);
	};
}