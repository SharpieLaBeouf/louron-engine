#include "Entity.h"

namespace Louron {
    Entity::Entity(entt::entity regHandle, Scene* scene) : 
        m_EntityHandle(regHandle), 
        m_Scene(scene)
    {
    }
}