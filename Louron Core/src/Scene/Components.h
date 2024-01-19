#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <map>

#include "Light.h"
#include "Camera.h"
#include "Mesh.h"
#include "../OpenGL/Material.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Louron {

	struct TagComponent {

		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& name) : Tag(name) { }
	};

	struct TransformComponent {

		glm::vec3 position = glm::vec3(0.0f);
		glm::vec3 rotation = glm::vec3(0.0f);
		glm::vec3 scale = glm::vec3(1.0f);

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation) : position(translation) { }

		void Translate(const glm::vec3& vector) { position += vector; }
		void Rotate(const glm::vec3& vector) { rotation += vector; }
		void Scale(const glm::vec3& vector) { scale += vector; }

		operator glm::mat4() { return this->GetTransform(); }
		glm::mat4 GetTransform() const {
			return glm::translate(glm::mat4(1.0f), position)
				* glm::toMat4(glm::quat(glm::radians(rotation)))
				* glm::scale(glm::mat4(1.0f), scale);
		}
	};

	struct CameraComponent {

		Camera* Camera = nullptr;
		bool Primary = true;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;

	};

	struct MaterialComponent {

		std::vector<std::shared_ptr<Material>> Materials;
		std::string ShaderName = "Default Shader";

		MaterialComponent() = default;
		MaterialComponent(const std::string& shaderName) : ShaderName(shaderName) { }
		MaterialComponent(const MaterialComponent&) = default;

		void LinkMeshMaterials(std::vector<std::pair<std::shared_ptr<MeshFilter>, std::shared_ptr<Material>>> meshMaterialPair) {
			for (const auto& material : meshMaterialPair)
				Materials.push_back(material.second);
		}

		void SetShader(const std::string& shaderName) {
			ShaderName = shaderName;
			for (const auto& mat : Materials) {
				mat->SetShader(Engine::Get().GetShaderLibrary().GetShader(ShaderName));
			}
		}
	};

	struct MeshComponent {

	public:
		bool active = true;

		MeshComponent() {}
		MeshComponent(std::vector<std::pair<std::shared_ptr<MeshFilter>, std::shared_ptr<Material>>> meshes) {
			for (const auto& mesh : meshes) {
				
				m_Meshes.push_back(mesh.first);
				
				m_MeshCount++;
			}
		}
		MeshComponent(const MeshComponent&) = default;
		~MeshComponent() = default;

		int LoadModel(const char* filePath, MaterialComponent& materialComponent);

		std::vector<std::shared_ptr<MeshFilter>>& GetMeshes() { return m_Meshes; }

	private:

		GLuint m_MeshCount = 0;
		std::vector<std::shared_ptr<MeshFilter>> m_Meshes;

		std::string m_Directory;

		void ProcessNode(aiNode* node, const aiScene* scene, MaterialComponent& materialComponent);
		std::shared_ptr<MeshFilter> ProcessMesh(aiMesh* mesh, const aiScene* scene, MaterialComponent& materialComponent) const;
	};
}