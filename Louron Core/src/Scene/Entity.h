#pragma once

#include <iostream>

#include "Components.h"

//Base Entity Class
class Entity {

public:
	Entity() = default;
	Entity(const Entity&) = default;
	~Entity() = default;

private:
	GLuint* m_ParentID;
};



