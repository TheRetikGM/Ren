#pragma once
#include <cstdint>  // uint32_t, uint64_t
#include <vector>   // std::vector
#include <bitset>   // std::bitset
#include <tuple>    // std::tuple, std::make_tuple
#include "Ren/Core.h"   // Ref

namespace Ren::ecs
{
    typedef uint64_t EntityID;
    typedef uint32_t EntityIndex;
    typedef uint32_t EntityVersion;
    const int MAX_COMPONENTS = 32;
    typedef std::bitset<MAX_COMPONENTS> ComponentMask;

    namespace Utils
    {
        inline EntityID CreateEntityID(EntityIndex index, EntityVersion version) { return (EntityID)index << 32 | (EntityID)version; }
        inline EntityIndex GetEntityIndex(EntityID id) { return EntityIndex(id >> 32); }
        inline EntityVersion GetEntityVersion(EntityID id) { return EntityVersion(id); }
        inline bool IsEntityValid(EntityID id) { return EntityIndex(id >> 32) != EntityIndex(-1); }

        // Return unique ID for each type. We don't need to know these types
        // as those will be defined by the user. Thus all this function does
        // is assigning new unique IDs to different types.
        int component_count = 0;
        template<typename T>
        int GetId()
        {
            static int s_component_id = component_count++;
            return s_component_id;
        }
    }
    #define INVALID_ENTITY Utils::CreateEntityID(EntityIndex(-1), 0)

    // Component pool, which will hold data for each entity.
    // For now, each component pool will allocate MAX_COMPONENTS worth of memory for each component type.
    // - TODO: It would be more efficient to use other means of storing them. For small games, this should be fine though.
    // - !! WARNING !! Component pool allocates memory when constructed and deletes it when destructed. Thus it is recommended to keep it in heap memory. Use with care!
    struct ComponentPool
    {
        // Size of component, which will be stored in this pool.
        size_t element_size{ 0 };
        // Array of bytes to store any type. Using this type allows for easier pointer arithmetic.
        uint8_t* p_data{ nullptr };

        ComponentPool(size_t element_size)
        {
            this->element_size = element_size;
            p_data = new uint8_t[element_size * MAX_COMPONENTS];
        }
        ~ComponentPool() { delete[] p_data; }

        // Return component at given index.
        inline void* get(size_t index) { return p_data + index * element_size; }
    };

    // Scene object, that stores all entities and manages assigning components to them.
    class Scene
    {
        // Entity description. 
        // - All information we need about each entity is its ID and which components are assigned with it.
        struct EntityDesc {
            EntityID id;
            ComponentMask mask;
        };
    public:
        static Ref<Scene> Create() { return Ref<Scene>(new Scene()); }
        ~Scene()
        {
            for (auto&& pool : mComponentPools)
                if (pool) delete pool;
        }

        EntityID NewEntity()
        {
            if (!mFreeEntities.empty())
            {
                EntityIndex new_index = mFreeEntities.back();
                mFreeEntities.pop_back();
                // Get new entity ID with the index and higher version (increased when destroying the previous entity).
                EntityID new_id = Utils::CreateEntityID(new_index, Utils::GetEntityVersion(mEntities[new_index].id));
                mEntities[new_index].id = new_id;
                return new_id;
            }
            // Create new entity at the end if entities array. Index corresponding to the size of array before push.
            mEntities.push_back({ Utils::CreateEntityID(EntityIndex(mEntities.size()), 0), ComponentMask() });
            return mEntities.back().id;
        }
        // Assign component to given entity. Returns pointer to the newly created component instance.
        template<typename TComponent>
        TComponent* Assign(EntityID id)
        {
            // Ensure we are not accessing entity that has been deleted.
            if (mEntities[Utils::GetEntityIndex(id)].id != id)
                return nullptr;

            int component_id = Utils::GetId<TComponent>();

            if ((int)mComponentPools.size() <= component_id)
                mComponentPools.resize(mComponentPools.size() + 1, nullptr);
            if (mComponentPools[component_id] == nullptr)
                mComponentPools[component_id] = new ComponentPool(sizeof(TComponent));

            // Create component in already allocated memory in component pool by using a **placement new**.
            TComponent* p_component = new(mComponentPools[component_id]->get(Utils::GetEntityIndex(id))) TComponent();

            // Set bit corresponding to this component ID, to indicate that it is in use.
            mEntities[Utils::GetEntityIndex(id)].mask.set(component_id);
            return p_component;
        }
        // Assign multiple components to given entity. Returns tuple of pointers to the newly created components.
        template<typename... TComponents>
        std::tuple<TComponents*...> AssignMultiple(EntityID id)
        {
            return std::make_tuple(Assign<TComponents>(id)...);
        }
        // Explicitly get pointer to the component instance for given entity. If component instance doesn't exist, nullptr is returned.
        template<typename TComponent>
        TComponent* Get(EntityID id)
        {
            if (mEntities[Utils::GetEntityIndex(id)].id != id)
                return nullptr;
            
            // Check if entity has given component.
            int component_id = Utils::GetId<TComponent>();
            if (!mEntities[Utils::GetEntityIndex(id)].mask.test(component_id))
                return nullptr;
            
            return static_cast<TComponent*>(mComponentPools[component_id]->get(Utils::GetEntityIndex(id)));
        }
        template<typename... TComponents>
        std::tuple<TComponents*...> GetMultiple(EntityID id)
        {
            return std::make_tuple(Get<TComponents>(id)...);
        }
        // Remove component from given entity.
        template<typename TComponent>
        void Remove(EntityID id)
        {
            if (mEntities[Utils::GetEntityIndex(id)].id != id)
                return;
            
            // Just set the bit, which corresponds to component, to low.
            int component_id = Utils::GetId<TComponent>();
            mEntities[Utils::GetEntityIndex(id)].mask.reset(component_id);
        }
        // Destroy the entity and remove all of its components. Note that the components itself will not be deleted
        // as they are stored in prealocated memory of the component pool.
        // - TODO: In the future, we could keep track of active components in component pool and if needed, delete the pool.
        void DestroyEntity(EntityID id)
        {
            // Invalidate this ID and increase the version.
            EntityID new_id = Utils::CreateEntityID(EntityIndex(-1), Utils::GetEntityVersion(id) + 1);
            mEntities[Utils::GetEntityIndex(id)].id = new_id;
            // Remove (deassign) all components.
            mEntities[Utils::GetEntityIndex(id)].mask.reset();
            // Register as free entity for future recyclation of the index.
            mFreeEntities.push_back(Utils::GetEntityIndex(id));
        }

    private:
        std::vector<EntityDesc>     mEntities;
        std::vector<ComponentPool*> mComponentPools;
        // Keep track of destroyed entities, so that we can use their indexes for new entities (thus not wasting memory).
        std::vector<EntityIndex>    mFreeEntities;

        Scene() = default;

        template<typename... TComponents> friend class SceneView;
    };
}