#pragma once

namespace Ren
{
    class Renderbuffer 
    {
    public:
        unsigned int ID;
        unsigned int Width;
        unsigned int Height;
        // unsigned int Samples = 1; /// TODO

        unsigned int InternalFormat;

        Renderbuffer();
        ~Renderbuffer() {}

        void Generate(unsigned int width, unsigned int height);
        void Resize(unsigned int new_width, unsigned int new_height);

        void Bind() const;
        void Delete();
    };
}