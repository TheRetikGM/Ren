#pragma once
#include <Ren/Renderer/Renderer.h>
#include <Ren/InputInterface.hpp>
#include <vector>             // std::vector
#include <unordered_map>
#include "Components.hpp"
#include "SceneView.hpp"    // Scene, SceneView


/*
*  This file defines some basic commonly used components.
*/

namespace Ren::ecs
{
    using Transform2D = components::Transform2D;
    using SpriteRenderer = components::SpriteRenderer;
    using Script = components::Script;

    // Parent class for all systems. You can also create your own systems.
    class System
    {
    public:
        virtual ~System() {}
        virtual void Init() {};
        virtual void Destroy() {};
        virtual void ProcessInput(InputInterface* input) {};
        virtual void Update(float dt) {};
        virtual void Render() {};

        virtual void SetScene(Scene* p_scene) { mpActiveScene = p_scene; }
    protected:
        Scene* mpActiveScene{ nullptr };
    };

    class RenderSystem : public System
    {
    public:
        void Init()
        {
            // Get renderer.
            renderer_2d = Renderer2D::GetInstance();

            // Load and preapre all textures.
            for (const auto&& ent : SceneView<Transform2D, SpriteRenderer>(*mpActiveScene))
            {
                auto p_sprite_renderer = mpActiveScene->Get<SpriteRenderer>(ent);
                if (p_sprite_renderer && p_sprite_renderer->image_path != SpriteRenderer::IMAGE_NONE)
                {
                    auto tex = RawTexture::Load(p_sprite_renderer->image_path.c_str());
                    p_sprite_renderer->tex_id = renderer_2d->PrepareTexture(tex);
                    tex.Delete();
                }
            }
        }
        void Destroy()
        {
            return; // For now dont destroy anything. As resources will be freed in next Renderer2D::BeginPrepare() statement;

            for (const auto&& ent : SceneView<Transform2D, SpriteRenderer>(*mpActiveScene))
            {
                auto p_sprite_renderer = mpActiveScene->Get<SpriteRenderer>(ent);
                if (p_sprite_renderer->tex_id != TEXTURE_NONE)
                    renderer_2d->RemoveTexture(p_sprite_renderer->tex_id);
            }
        }
        void Update(float dt)
        {
            // TODO: Maybe update some animations and stuff
        }
        void Render()
        {
            for (const auto&& ent : SceneView<Transform2D, SpriteRenderer>(*mpActiveScene))
            {
                auto p_trans = mpActiveScene->Get<Transform2D>(ent);
                auto p_sprite = mpActiveScene->Get<SpriteRenderer>(ent);

                renderer_2d->SubmitQuad({ p_trans->position, p_trans->scale, p_trans->rotation }, { p_sprite->color, p_sprite->tex_id }, p_trans->layer);
            }
        }
    protected:
        Renderer2D* renderer_2d{ nullptr };
    };

    class ScriptSystem : public System
    {
    public:
        void Init() override
        {
            for (auto&& ent : SceneView<Script>(*mpActiveScene))
            {
                Script* s = mpActiveScene->Get<Script>(ent);
                for (auto&& script : s->scripts)
                    if (script)
                        script->Init();
            }
        };
        void Destroy() override
        {
            for (auto&& ent : SceneView<Script>(*mpActiveScene))
            {
                Script* s = mpActiveScene->Get<Script>(ent);
                for (auto&& script : s->scripts)
                    if (script)
                        script->Destroy();
            }
        };
        void ProcessInput(InputInterface* input) override
        {
            for (auto&& ent : SceneView<Script>(*mpActiveScene))
            {
                Script* s = mpActiveScene->Get<Script>(ent);
                for (auto&& script : s->scripts)
                    if (script)
                        script->ProcessInput(input);
            }
        };
        void Update(float dt) override
        {
            for (auto&& ent : SceneView<Script>(*mpActiveScene))
            {
                Script* s = mpActiveScene->Get<Script>(ent);
                for (auto&& script : s->scripts)
                    if (script)
                        script->Update(dt);
            }
        };
        void SetScene(Scene* p_scene) override
        {
            if (!p_scene)
                return;

            mpActiveScene = p_scene;
            for (auto&& ent : SceneView<Script>(*mpActiveScene))
            {
                Script* s = mpActiveScene->Get<Script>(ent);
                for (auto&& script : s->scripts) {
                    if (script) {
                        script->mpActiveScene = mpActiveScene;
                        script->mEntityID = ent;
                    }
                }
            }
        };
    };

    // Manages all systems --> Is calling required methods etc.
    // TODO: multithreading for certain systems.
    class SystemsManager
    {
        typedef int32_t SystemID;

    public:
        SystemsManager() = default;
        ~SystemsManager() {};

        template<typename TSystem>
        TSystem* AddSystem()
        {
            TSystem* instance = new TSystem();
            instance->SetScene(mpManagedScene);

            if (!mAvailableSystemIDs.empty()) {
                mSystems[mAvailableSystemIDs.back()] = Ref<System>(instance);
                mAvailableSystemIDs.pop_back();
            }
            else
                mSystems.emplace(getSystemID<TSystem>(), Ref<System>(instance));

            return instance;
        }
        template<typename TSystem>
        void RemoveSystem()
        {
            SystemID id = getSystemID<TSystem>();
            REN_ASSERT(mSystems.count(id) > 0, "Trying to remove system, that was not added.");
            mSystems[id].reset();
            mAvailableSystemIDs.push_back(id);
        }
        template<typename TSystem>
        TSystem* GetSystem()
        {
            SystemID id = getSystemID<TSystem>();
            if (mSystems.count(id) == 0)
                return nullptr;
            return dynamic_cast<TSystem>(mSystems[id].get());
        }
        void SetScene(Scene* p_scene) 
        {   
            mpManagedScene = p_scene; 

            // Update active scene for all managed systems.
            for (auto&& [id, sys] : mSystems)
                sys->SetScene(mpManagedScene);
        }
        inline Scene* GetScene() { return mpManagedScene; }

        // System functions
        void Init()
        {
            REN_ASSERT(mpManagedScene, "No scene is set. To set scene use SystemsManager::SetScene() method.");
            for (auto&& [id, sys] : mSystems)
                sys->Init();
        };
        void Destroy()
        {
            for (auto&& [id, sys] : mSystems)
                sys->Destroy();
        };
        void ProcessInput(InputInterface* input)
        {
            for (auto&& [id, sys] : mSystems)
                sys->ProcessInput(input);
        };
        void Update(float dt)
        {
            for (auto&& [id, sys] : mSystems)
                sys->Update(dt);
        };
        void Render()
        {
            for (auto&& [id, sys] : mSystems)
                sys->Render();
        };
    
    protected:
        Scene* mpManagedScene{ nullptr };
        std::unordered_map<SystemID, Ref<System>> mSystems;
        std::vector<SystemID> mAvailableSystemIDs;

        // Get unique system ID for each system type.
        inline static SystemID mLastSystemID = 0;
        template<typename T>
        static SystemID getSystemID()
        {
            static SystemID id = mLastSystemID++;
            return id;
        }
    };
};