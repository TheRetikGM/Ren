#include "Ren/Renderer/OpenGL/Framebuffer.h"
#include "Ren/Logger.hpp"
#include "Ren/Renderer/OpenGL/RenderAPI.h"

using namespace Ren;

enum class AttachmentUsage: int {
    COLOR = GL_COLOR_ATTACHMENT0,
    DEPTH = GL_DEPTH_ATTACHMENT,
    STENCIL = GL_STENCIL_ATTACHMENT,
    DEPTH_STENCIL = GL_DEPTH_STENCIL_ATTACHMENT
};

AttachmentUsage getAttachmentUsage(AttachmentFormats format)
{
    switch (format)
    {
    case AttachmentFormats::RGB:
    case AttachmentFormats::RGBA:
    case AttachmentFormats::RED:
        return AttachmentUsage::COLOR;
    case AttachmentFormats::DEPTH24_STENCIL8:
        return AttachmentUsage::DEPTH_STENCIL;
    case AttachmentFormats::DEPTH_COMPONENT24:
        return AttachmentUsage::DEPTH;
    case AttachmentFormats::STENCIL_INDEX8:
        return AttachmentUsage::STENCIL;
    default:
        return AttachmentUsage::COLOR;
    }
}

void FramebufferAttachment::Attach(int nColorAttachment)
{
    this->nColorAttachment = nColorAttachment;

    AttachmentUsage usage = getAttachmentUsage(internalFormat);

    int attachment = (int)usage;
    if (usage == AttachmentUsage::COLOR)
        attachment += nColorAttachment;

    if (storageType == StorageType::TEXTURE)
    {
        Texture2D& texture = std::get<Texture2D>(storage);
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, texture.BindTarget, texture.ID, 0); 
    }
    else
    {
        Renderbuffer& buffer = std::get<Renderbuffer>(storage);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, buffer.ID);
    }
}

void FramebufferAttachment::Resize(unsigned int new_width, unsigned int new_height)
{
    if (storageType == StorageType::TEXTURE) {
        std::get<Texture2D>(storage).Resize(new_width, new_height);
        bufferID = std::get<Texture2D>(storage).ID;
    }
    else {
        std::get<Renderbuffer>(storage).Resize(new_width, new_height);
        bufferID = std::get<Renderbuffer>(storage).ID;
    }
}
void FramebufferAttachment::Delete()
{
    if (storageType == StorageType::TEXTURE)
        std::get<Texture2D>(storage).Delete();
    else
        std::get<Renderbuffer>(storage).Delete();
}
glm::uvec2 FramebufferAttachment::GetSize()
{
    if (storageType == StorageType::TEXTURE)
        return glm::uvec2(std::get<Texture2D>(storage).Width, std::get<Texture2D>(storage).Height);
    else
        return glm::uvec2(std::get<Renderbuffer>(storage).Width, std::get<Renderbuffer>(storage).Height);
}
void FramebufferAttachment::UpdateID()
{
    if (storageType == StorageType::TEXTURE)
        bufferID = std::get<Texture2D>(storage).ID;
    else
        bufferID = std::get<Renderbuffer>(storage).ID;
}
void FramebufferAttachment::Generate(unsigned int width, unsigned int height)
{
    if (storageType == StorageType::TEXTURE)
    {
        Texture2D tex;
        tex.Internal_format = (unsigned int)internalFormat;
        tex.Generate(width, height);
        storage = tex;
    }
    else
    {
        Renderbuffer buf;
        buf.InternalFormat = (unsigned int)internalFormat;
        buf.Generate(width, height);
        storage = buf;
    }
    UpdateID();
}


// ============================
// ======= Framebuffer ========
// ============================

Framebuffer::Framebuffer() : Attachments()
{
    nMaxAttachments = -1;
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, (GLint*)(&nMaxAttachments));
    nMaxAttachments += 2;   // Depth and Stencil.
}

Framebuffer& Framebuffer::AddAttachment(Ref<FramebufferAttachment> attachment)
{
    if (Attachments.size() == nMaxAttachments)
    {
        LOG_E("Cannot add attachment! Maximum number of attachments (" + std::to_string(nMaxAttachments) + ") reached.");
        return *this;
    }
    Attachments.push_back(attachment);

    return *this;
}

Framebuffer& Framebuffer::Generate(unsigned int width, unsigned int height)
{
    if (checkFramebufferValidity(glm::uvec2(width, height)) == false)
    {
        LOG_E("Framebuffer generation failed! Error: " + sError);
        return *this;
    }
    this->Size = glm::uvec2(width, height);

    glGenFramebuffers(1, &this->ID);
    this->Bind();

    // Attach all attachments to the OpenGL framebuffer object.
    for (int i = 0, nColorAttachmentsTotal = 0; i < int(Attachments.size()); i++)
    {
        int nColorAttachment = -1;  // Default no color attachment.
        if (getAttachmentUsage(Attachments[i]->internalFormat) == AttachmentUsage::COLOR) 
        {
            nColorAttachment = nColorAttachmentsTotal++;
            mColorBufferBindings[nColorAttachment] = &Attachments[i]->bufferID;
        }
        Attachments[i]->Attach(nColorAttachment);
    }

    // Final check for completness.
    unsigned int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOG_E("Framebuffer not complete! Status: " + std::to_string(status));
    
    this->Unbind();

    return *this;
}
Framebuffer& Framebuffer::Bind()
{
    if (PreserveFBOBinding)
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)(&nFbBackupID));
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
    if (AutoViewport)
    {
        RenderAPI::GetViewport(mViewportOffset_backup, mViewportSize_backup);
        RenderAPI::SetViewport({0, 0}, {Width, Height});
    }
    return *this;
}
Framebuffer& Framebuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, nFbBackupID);
    nFbBackupID = 0;

    if (AutoViewport)
        RenderAPI::SetViewport(mViewportOffset_backup, mViewportSize_backup);

    return *this;
}
Framebuffer& Framebuffer::Resize(unsigned int new_width, unsigned int new_height)
{
    Width = new_width;
    Height = new_height;
    Invalidate();

    return *this;
}
bool Framebuffer::checkFramebufferValidity(glm::uvec2 size)
{
    if (Attachments.size() > nMaxAttachments)
    {
        sError = "More attachments then max size (" + std::to_string(nMaxAttachments) + ").";
        return false;
    }
    int nColorAttachments = 0;
    for (auto& a : Attachments)
    {
        if (a->GetSize() != size)
        {
            sError = "One or more attachments have different sizes.";
            return false;
        }
        nColorAttachments += int(getAttachmentUsage(a->internalFormat) == AttachmentUsage::COLOR);
    }

    if (nColorAttachments == 0)
    {
        sError = "No color attachments found.";
        return false;
    }
    
    return true;
}
Framebuffer& Framebuffer::Clear(glm::vec4 vClearColor)
{
    RenderAPI::SetClearColor(vClearColor);
    RenderAPI::Clear();
    return *this;
}
void Framebuffer::Delete()
{
    glDeleteFramebuffers(1, &ID);
    for  (auto& i : Attachments)
        i->Delete();
    Attachments.clear();
    mColorBufferBindings.clear();
    ID = 0;
}
void Framebuffer::Invalidate()
{
    glDeleteFramebuffers(1, &ID);
    for  (auto& i : Attachments)
    {
        i->Delete();
        i->Generate(Width, Height);
        i->UpdateID();
    }
    Generate(Width, Height);
}

// ===============
// Factory pattern
// ===============

std::shared_ptr<Framebuffer> Framebuffer::CreateBasicFramebuffer(unsigned int width, unsigned int height)
{
    auto aColor = std::make_shared<FramebufferAttachment>();
    aColor->storageType = StorageType::TEXTURE;
    aColor->internalFormat = AttachmentFormats::RGBA;
    aColor->Generate(width, height);

    auto aDepthStencil = std::make_shared<FramebufferAttachment>();
    aDepthStencil->storageType = StorageType::RENDERBUFFER;
    aDepthStencil->internalFormat = AttachmentFormats::DEPTH24_STENCIL8;
    aDepthStencil->Generate(width, height);

    auto fbo = std::make_shared<Framebuffer>();
    fbo->AddAttachment(aColor).AddAttachment(aDepthStencil);
    fbo->Generate(width, height);

    return fbo;
}

Ref<Framebuffer> Framebuffer::Create2DBasicFramebuffer(unsigned int width, unsigned int height)
{
    auto aColor = std::make_shared<FramebufferAttachment>();
    aColor->storageType = StorageType::TEXTURE;
    aColor->internalFormat = AttachmentFormats::RGBA;
    aColor->Generate(width, height);

    auto fbo = std::make_shared<Framebuffer>();
    fbo->AddAttachment(aColor);
    fbo->Generate(width, height);

    return fbo;
}