#pragma once

#include <iostream>

#include "Components.h"
#include "SceneManager.h"
#include "InstanceManager.h"

//Base Entity Class
class Entity {

public:
	Entity() = default;
	//Entity(entt::entity entity, Scene* scene) : m_Entity(entity), m_Scene(scene) { }
	Entity(const Entity&) = default;
	~Entity() = default;

	Entity(Entity* parentEntity) : m_ParentEntity(parentEntity->m_ParentEntity) { }


	/*template<typename T, typename... Args>
	T& AddComponent(Args&&... args) {
		if (HasComponent<T>()) 
			std::cout << "[L20] This Entity Already Contains a Component of This Type!" << std::endl;
		
		T& component = m_Scene->m_Registry.emplace<T>(m_Entity, std::forward<Args>(args)...);
		return component;
	}
	template<typename T, typename... Args>
	T& GetComponent(Args&&... args) {
		if (!HasComponent<T>()) 
			std::cout << "[L20] This Entity Does Not Contain This Component Type!" << std::endl;
		return m_Scene->m_Registry.get<T>(m_EntityHandle);
	}
	template<typename T, typename... Args>
	T& HasComponent(Args&&... args) {
		return m_Scene->m_Registry.has<T>(m_Entity);
	}
	template<typename T, typename... Args>
	T& RemoveComponent(Args&&... args) {
		if (!HasComponent<T>())
			std::cout << "[L20] This Entity Does Not Contain This Component Type!" << std::endl;

		m_Scene->m_Registry.remove<T>(m_EntityHandle);
	}

	operator entt::entity() const { return m_Entity; }
	operator bool() const { return m_Entity != entt::null; }
	bool operator==(const Entity& other) const { return m_Entity == other.m_Entity && m_Scene == other.m_Scene; }
	bool operator!=(const Entity& other) const { return !(*this == other); }*/

private:
	//entt::entity m_Entity{ entt::null };
	Entity* m_ParentEntity;

	Scene* m_Scene = nullptr;
};



