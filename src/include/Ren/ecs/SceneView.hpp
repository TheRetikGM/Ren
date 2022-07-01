#pragma once
#include "Scene.hpp"

namespace Ren
{
    // SceneView is an object, that is used for iterating through the scene.
    // For example we could iterate only through entities that have certain components attached etc.
    // Example:
    //     for (auto&& entity_id : SceneView<Transform, SpriteRenderer>(scene)) do_stuff...
    template<typename... TComponents>
    class SceneView
    {
    public:
        SceneView(Scene& scene)
            : mpScene(&scene)
        {
            // If no types are provided, then iterate through all of the components.
            if (sizeof...(TComponents) == 0)
                mAll = true;
            else
            {
                // Unpack the component types into array of theirs IDs. And prepare the corresponding mask.
                // Provide the 0, to avoid compilation error in case no types are provided.
                int component_ids[] = { 0, Utils::GetId<TComponents>()... };
                for (uint32_t i = 1; i < sizeof...(TComponents) + 1; i++)
                    mComponentMask.set(component_ids[i]);
            }
        }

        struct Iterator
        {   
            // Current entity index referenced by this iterator.
            EntityIndex index;
            Scene* pScene;
            // Described below. Has the same purpose, but iterator needs to store its own data.
            ComponentMask mask;
            bool all{ false };

            Iterator(Scene* scene, EntityIndex index, ComponentMask mask, bool all)
                : index(index)
                , pScene(scene)
                , mask(mask)
                , all(all) {}
            
            EntityID operator*() const
            {
                return pScene->mEntities[index].id;
            }
            bool operator==(const Iterator& rhs) const
            {   
                // If index is in the end, it is equal to anything, to avoid going further.
                return index == rhs.index || index == EntityIndex(pScene->mEntities.size());
            }
            bool operator!=(const Iterator& rhs) const
            {
                return index != rhs.index && index != EntityIndex(pScene->mEntities.size());
            }
            Iterator& operator++()
            {
                // Find next index, where the entity has all the components.
                do
                {
                    index++;
                } while (index < EntityIndex(pScene->mEntities.size()) && !ValidIndex());
                return *this;
                
            }
            // Helper function used to check validity of the index.
            bool ValidIndex()
            {
                return Utils::IsEntityValid(pScene->mEntities[index].id) && (all || mask == (mask & pScene->mEntities[index].mask));
            }
        };

        const Iterator begin()
        {
            // Find first entity that has required components.
            EntityIndex first_index = 0;
            while (first_index < EntityIndex(mpScene->mEntities.size())
                && (mComponentMask != (mComponentMask & mpScene->mEntities[first_index].mask) || !Utils::IsEntityValid(mpScene->mEntities[first_index].id)))
                first_index++;
            return Iterator(mpScene, first_index, mComponentMask, mAll);
        }
        const Iterator end()
        {
            return Iterator(mpScene, EntityIndex(mpScene->mEntities.size()), mComponentMask, mAll);
        }

    private:
        Scene* mpScene{ nullptr };
        // Flag for checking when we are iterating through all the components.
        bool mAll{ false };
        // When selecting only entities with some components, we use this mask to check for them.
        ComponentMask mComponentMask;
    };
}