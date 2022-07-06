#pragma once
#include "Ren/Core.h"
#include "Ren/InputInterface.hpp"
#include <glm/glm.hpp>


namespace Ren
{
    class Camera2D
    {
    public:
        virtual ~Camera2D() {} 

        virtual void Init(const glm::vec2& screen_size) = 0;
        virtual void Update(InputInterface* input, float dt) = 0;
        virtual void OnResize(const glm::vec2& new_screen_size) = 0;
        virtual bool CanSee(glm::vec2 point) = 0;
        virtual glm::mat4 GetProjection() = 0;
        virtual glm::mat4 GetView() = 0;

        inline void SetPosition(const glm::vec2& pos) { m_cameraPosition = pos; }
        inline const glm::vec2& GetPosition() { return m_cameraPosition; }
        inline glm::mat4 GetPVMat() { return GetProjection() * GetView(); }
        inline float GetZoom() { return m_zoom; }
        inline void SetZoom(float zoom_value) { if (zoom_value > 0.0f) m_zoom = zoom_value; }
        inline void AddToZoom(float val) { if (m_zoom + val > 0.0f) m_zoom += val; }
    protected:
        glm::vec2 m_cameraPosition{ 0.0f, 0.0f };
        glm::vec2 m_screenSize{ 0.0f, 0.0f };
        glm::vec2 m_cameraSize{ 0.0f, 0.0f };
        float m_zoom = 1.0f;
    };

    class PixelCamera : public Camera2D
    {
    public:
        void Init(const glm::vec2& screen_size) override
        {
            m_screenSize = screen_size;
            m_cameraSize = m_screenSize * m_zoom;
        }
        void Update(InputInterface* input, float dt) override 
        {
            if (input->MousePressed(MouseButton::RIGHT))
                m_lastMousePos = input->GetMousePos();
            else if (input->MouseHeld(MouseButton::RIGHT))
            {
                glm::vec2 relative_mouse_pos = input->GetMousePos() - m_lastMousePos;
                m_lastMousePos = input->GetMousePos();

                m_cameraPosition -= relative_mouse_pos;
            }
        }
        void OnResize(const glm::vec2& new_screen_size) override
        {
            m_screenSize = new_screen_size;
            m_cameraSize = m_screenSize * m_zoom;
        }
        bool CanSee(glm::vec2 point) override
        {
            m_cameraSize = m_screenSize * m_zoom;
            return point.x >= m_cameraPosition.x && point.x < m_cameraPosition.x + m_cameraSize.x
                && point.y >= m_cameraPosition.y && point.y < m_cameraPosition.y + m_cameraSize.y;
        }
        glm::mat4 GetProjection() override
        {
            return glm::ortho(0.0f, m_screenSize.x, m_screenSize.y, 0.0f, -1.0f, 1.0f);
        }
        glm::mat4 GetView() override
        {
            glm::mat4 view = glm::mat4(1.0f);
            view = glm::scale(view, glm::vec3(m_zoom, m_zoom, 1.0f));
            view = glm::translate(view, glm::vec3(-m_cameraPosition, 0.0f));
            return view;
        }
    private:
        glm::vec2 m_lastMousePos{ 0.0f, 0.0f };
    };
}