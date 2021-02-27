#pragma once

#include <iostream>

#include "Components.h"
#include "../InstanceManager.h"

//Base Entity Class
class Entity {

public:
	Entity() = default;
	Entity(const Entity&) = default;
	~Entity() = default;

	Entity(Entity* parentEntity) : m_ParentEntity(parentEntity->m_ParentEntity) { }

private:
	Entity* m_ParentEntity;
};



