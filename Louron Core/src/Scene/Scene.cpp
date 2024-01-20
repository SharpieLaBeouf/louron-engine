#include "Scene.h"

#include "Mesh.h"
#include "Entity.h"
#include "Components.h"

#include "../Renderer/Renderer.h"

#include <glm/gtc/quaternion.hpp>

namespace Louron {
	Scene::Scene() {

		FP_Data.workGroupsX = (Engine::Get().GetWindow().GetWidth() + (Engine::Get().GetWindow().GetWidth() % 16)) / 16;
		FP_Data.workGroupsY = (Engine::Get().GetWindow().GetHeight() + (Engine::Get().GetWindow().GetHeight() % 16)) / 16;
		size_t numberOfTiles = static_cast<size_t>(FP_Data.workGroupsX * FP_Data.workGroupsY);

		// Setup Light Buffers

		glGenBuffers(1, &FP_Data.PL_Buffer);
		glGenBuffers(1, &FP_Data.PL_Indices_Buffer);

		glGenBuffers(1, &FP_Data.SL_Buffer);
		glGenBuffers(1, &FP_Data.SL_Indices_Buffer);

		glGenBuffers(1, &FP_Data.DL_Buffer);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.PL_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_POINT_LIGHTS * sizeof(PointLightComponent), 0, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.PL_Indices_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleLightIndex) * MAX_POINT_LIGHTS, 0, GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.SL_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_SPOT_LIGHTS * sizeof(SpotLightComponent), 0, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.SL_Indices_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, numberOfTiles * sizeof(VisibleLightIndex) * MAX_SPOT_LIGHTS, 0, GL_STATIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.DL_Buffer);
		glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_DIRECTIONAL_LIGHTS * sizeof(DirectionalLightComponent), 0, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		// Setup Depth Texture

		glGenFramebuffers(1, &FP_Data.DepthMap_FBO);
		glGenTextures(1, &FP_Data.DepthMap_Texture);

		glBindTexture(GL_TEXTURE_2D, FP_Data.DepthMap_Texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, Engine::Get().GetWindow().GetWidth(), Engine::Get().GetWindow().GetHeight(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glBindFramebuffer(GL_FRAMEBUFFER, FP_Data.DepthMap_FBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, FP_Data.DepthMap_Texture, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
	}
	
	// Creates Entity in Scene
	Entity Scene::CreateEntity(const std::string& name) {
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		return entity;
	}

	// Duplicates Entity in Scene
	Entity Scene::DuplicateEntity(Entity entity) {
		return Entity();
	}

	// Destroys Entity in Scene
	void Scene::DestroyEntity(Entity entity) {
		m_Registry.destroy(entity);
	}

	// Returns Entity within Scene on Tag Name
	Entity Scene::FindEntityByName(std::string_view name) {
		
		auto view = m_Registry.view<TagComponent>();
		for (auto entity : view) {
			const TagComponent& tag = view.get<TagComponent>(entity);
			if (tag.Tag == name)
				return Entity{ entity, this };
		}
		
		return {};
	}

	// Returns Primary Camera Entity
	Entity Scene::GetPrimaryCameraEntity() {

		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view) {
			const CameraComponent& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity{ entity, this };
		}

		return {};
	}

	void Scene::OnStart() {

		m_IsRunning = true;

	}
	
	// TODO: Forward+ Rendering
	void Scene::OnUpdate() {

		if (!m_IsPaused) {

			/*
				 
			Forward Plus Render Sequence
			
				1. Get Primary Camera Data
				2. Conduct Depth Pre-Pass
				3. Bind and Update Light SSBOs
				4. Conduct Light Cull
				5. Render All MeshComponents e.g., Renderer::Submit(Transform, MeshComponent, MaterialComponent)

			*/

			// 1. Get Primary Camera Data

			Entity cameraEntity;
			{
				auto view = m_Registry.view<TransformComponent, CameraComponent>();
				for (auto entity : view) {
					auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

					if (camera.Primary) {
						cameraEntity = Entity{ entity, this };
						break;
					}
				}
			}

			// 2. Conduct Depth Pre-Pass
			if (cameraEntity.GetComponent<CameraComponent>().Camera) {

				// Call Renderer for all Meshes
				{
					auto view = m_Registry.view<TransformComponent, MeshRenderer, MeshFilter>();

					if (view.begin() != view.end()) {
						for (auto entity : view) {
							auto [transform, meshRenderer, meshFilter] = view.get<TransformComponent, MeshRenderer, MeshFilter>(entity);

							Entity meshEntity = Entity{ entity, this };

							if (!meshRenderer.active)
								continue;

							glBindFramebuffer(GL_FRAMEBUFFER, FP_Data.DepthMap_FBO);
							glClear(GL_DEPTH_BUFFER_BIT);
							Renderer::DrawMeshComponent(&meshEntity, &cameraEntity, "FP_Depth");
							glBindFramebuffer(GL_FRAMEBUFFER, 0);
						}
					}
				}
			}
			

			// 3. Bind and Update Light SSBOs
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, FP_Data.PL_Buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, FP_Data.PL_Indices_Buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, FP_Data.SL_Buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, FP_Data.SL_Indices_Buffer);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, FP_Data.DL_Buffer);

			// Point Lights
			{
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.PL_Buffer);
				PointLightComponent* pointLights = (PointLightComponent*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

				if (pointLights) {
					auto view = m_Registry.view<TransformComponent, PointLightComponent>();

					int i = 0;
					for (auto entity : view) {
						auto [transform, point_light] = view.get<TransformComponent, PointLightComponent>(entity);

						point_light.position = glm::vec4(transform.position, 1.0f);

						pointLights[i] = point_light;
						pointLights[i].lastLight = false;

						i++;

						if (i >= MAX_POINT_LIGHTS)
							break;
					}
					pointLights[i].lastLight = true;
				}
				else {
					std::cout << "[L20] Point Light Buffer Not Mapped Successfully!" << std::endl;
				}

				glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
				}

			// Spot Lights
			{
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.SL_Buffer);
				SpotLightComponent* spotLights = (SpotLightComponent*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

				if (spotLights) {
					auto view = m_Registry.view<TransformComponent, SpotLightComponent>();

					int i = 0;
					for (auto entity : view) {
						auto [transform, spot_light] = view.get<TransformComponent, SpotLightComponent>(entity);

						spot_light.position = glm::vec4(transform.position, 1.0f);

						spotLights[i] = spot_light;
						spotLights[i].lastLight = false;

						i++;

						if (i >= MAX_SPOT_LIGHTS)
							break;
					}
					spotLights[i].lastLight = true;
				}
				else {
					std::cout << "[L20] Spot Light Buffer Not Mapped Successfully!" << std::endl;
				}

				glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}

			// Directional Lights
			{
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, FP_Data.DL_Buffer);
				DirectionalLightComponent* directionalLights = (DirectionalLightComponent*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);

				if (directionalLights) {
					auto view = m_Registry.view<TransformComponent, DirectionalLightComponent>();

					int i = 0;
					for (auto entity : view) {

						directionalLights[i] = view.get<DirectionalLightComponent>(entity);
						directionalLights[i].lastLight = false;
						directionalLights[i].direction = glm::normalize(
							glm::vec4(0.0f, 0.0f, -1.0f, 0.0f) * 
							glm::quat(glm::radians(view.get<TransformComponent>(entity).rotation)));

						i++;

						if (i >= MAX_DIRECTIONAL_LIGHTS)
							break;
					}
					directionalLights[i].lastLight = true;
				}
				else {
					std::cout << "[L20] Spot Light Buffer Not Mapped Successfully!" << std::endl;
				}

				glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}
			
			// 4. Conduct Light Cull
			std::shared_ptr<Shader>& lightCull = Engine::Get().GetShaderLibrary().GetShader("FP_Light_Culling");
			lightCull->Bind();
			glUniformMatrix4fv(glGetUniformLocation(lightCull->GetProgram(), "projection"), 1, GL_FALSE, glm::value_ptr(cameraEntity.GetComponent<CameraComponent>().Camera->GetProjMatrix()));
			glUniformMatrix4fv(glGetUniformLocation(lightCull->GetProgram(), "view"), 1, GL_FALSE, glm::value_ptr(cameraEntity.GetComponent<CameraComponent>().Camera->GetViewMatrix()));

			glActiveTexture(GL_TEXTURE4);
			glUniform1i(glGetUniformLocation(lightCull->GetProgram(), "depthMap"), 4);
			glBindTexture(GL_TEXTURE_2D, FP_Data.DepthMap_Texture);

			glDispatchCompute(FP_Data.workGroupsX, FP_Data.workGroupsY, 1);

			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, 0);
					
			// 5. Render All MeshComponents
			if (cameraEntity.GetComponent<CameraComponent>().Camera) {

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// Gather all entities within scene with appropriate components
				auto view = m_Registry.view<TransformComponent, MeshRenderer, MeshFilter>();
				if (view.begin() != view.end()) {
					std::unordered_map<std::shared_ptr<Material>, std::vector<std::pair<std::shared_ptr<Mesh>, TransformComponent>>> materialMeshMap;

					// Loop entities and access relevant components required for rendering
					for (const auto& entity : view) {
						auto [transform, meshRenderer, meshFilter] = view.get<TransformComponent, MeshRenderer, MeshFilter>(entity);

						Entity meshEntity = Entity{ entity, this };

						// Check if MeshRenderer is toggled for rendering, if not, continue to next entity
						if (!meshRenderer.active)
							continue;

						// Loop through each submesh within the meshfilter
						for (auto& subMesh : *meshFilter.Meshes)
							if ((*meshRenderer.Materials)[subMesh->MaterialIndex] != nullptr)
								materialMeshMap[(*meshRenderer.Materials)[subMesh->MaterialIndex]].push_back(std::make_pair(subMesh, transform));
							else
								std::cout << "[L20] Mesh with invalid material encountered!" << std::endl;

					}

					// FINAL RENDER!
					for (const auto& materialMeshPair : materialMeshMap) {
						const auto& material = materialMeshPair.first;
						const auto& meshes = materialMeshPair.second;

						// Update appropriate uniforms per Material
						if (material) {
							material->Bind();
							material->UpdateUniforms(cameraEntity.GetComponent<CameraComponent>());
						}
						else {
							std::cout << "[L20] Invalid material encountered during rendering!" << std::endl;
							continue;
						}

						// TODO: potential instancing for same meshes?

						// Render all Meshes per material
						for (const auto& meshTransform : meshes) {
							material->GetShader()->SetMat4("model", meshTransform.second.GetTransform());
							Renderer::DrawMesh(meshTransform.first);
						}
					}

					// Clean Up Scene Render Pass
					if (!materialMeshMap.empty())
						materialMeshMap.begin()->first->UnBind();
				}
			}
		}
	}

	void Scene::OnStop() {

		m_IsRunning = false;

	}

	// RESOURCE MANAGER FOR SCENES

	int ResourceManager::LoadMesh(const char* filePath, const std::shared_ptr<Shader>& shader) {

		std::string mesh_name = filePath;

		auto lastSlash = mesh_name.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = mesh_name.rfind('.');
		auto count = lastDot == std::string::npos ? mesh_name.size() - lastSlash : lastDot - lastSlash;
		mesh_name = mesh_name.substr(lastSlash, count);

		if (Meshes[mesh_name].first != nullptr) {
			std::cout << "[L20] INFO: Mesh Already Loaded!" << std::endl;
			return GL_FALSE;
		}

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filePath,
			aiProcess_CalcTangentSpace |
			aiProcess_GenSmoothNormals |
			aiProcess_Triangulate |
			aiProcess_OptimizeMeshes |
			aiProcess_SortByPType
		);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			std::cerr << "[L20] ERROR: " << importer.GetErrorString() << std::endl;
			assert(false);
		}

		std::string directory = filePath;
		directory = directory.substr(0, directory.find_last_of('/'));

		Meshes[mesh_name] = ProcessNode(scene->mRootNode, scene, directory, shader);

		std::cout << "[L20] Loaded Mesh: " << mesh_name.c_str() << std::endl;

		return GL_TRUE;
	}

	int ResourceManager::LoadShader(const char* filePath) {
		return 0;
	}

	int ResourceManager::LoadTexture(const char* filePath) {
		return 0;
	}

	void ResourceManager::LinkShader(std::shared_ptr<Shader> shader)
	{
		Shaders[shader->GetName()] = shader;
	}

	std::shared_ptr<MeshFilter> ResourceManager::GetMeshFilter(const std::string& name)
	{
		if (Meshes[name].first == nullptr) {
			std::cerr << "[L20] ERROR: Mesh Not Loaded to Scene Resource Manager!" << std::endl;
			assert(false);
		}

		return Meshes[name].first;
	}

	std::shared_ptr<MeshRenderer> ResourceManager::GetMeshRenderer(const std::string& name)
	{
		if (Meshes[name].second == nullptr) {
			std::cerr << "[L20] ERROR: Mesh Not Loaded to Scene Resource Manager!" << std::endl;
			assert(false);
		}

		return Meshes[name].second;
	}

	std::pair<std::shared_ptr<MeshFilter>, std::shared_ptr<MeshRenderer>> ResourceManager::ProcessNode(aiNode* node, const aiScene* scene, std::string directory, const std::shared_ptr<Shader>& shader)	{

		std::pair<std::shared_ptr<MeshFilter>, std::shared_ptr<MeshRenderer>> meshGroup = { std::make_shared<MeshFilter>(), std::make_shared<MeshRenderer>() };

		// process all the node's meshes (if any)
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			std::pair<std::shared_ptr<Mesh>, std::shared_ptr<Material>> meshMaterial = ProcessMesh(mesh, scene, directory, shader);

			meshGroup.first->Meshes->push_back(meshMaterial.first);
			meshGroup.second->Materials->push_back(meshMaterial.second);
		}
		// then do the same for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			std::pair<std::shared_ptr<MeshFilter>, std::shared_ptr<MeshRenderer>> childMeshGroup = ProcessNode(node->mChildren[i], scene, directory, shader);

			// Connect Meshes to MeshFilter
			meshGroup.first->Meshes->insert(meshGroup.first->Meshes->end(), childMeshGroup.first->Meshes->begin(), childMeshGroup.first->Meshes->end());
			meshGroup.second->Materials->insert(meshGroup.second->Materials->end(), childMeshGroup.second->Materials->begin(), childMeshGroup.second->Materials->end());
		}

		return meshGroup;
	}

	std::pair<std::shared_ptr<Mesh>, std::shared_ptr<Material>> ResourceManager::ProcessMesh(aiMesh* mesh, const aiScene* scene, std::string directory, const std::shared_ptr<Shader>& shader) {

		// Process Vertices
		std::vector<Vertex> mesh_vertices;
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex{ glm::vec3(0.0f), glm::vec3(0.0f), glm::vec2(0.0f) };
			vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
			if (mesh->mTextureCoords[0]) {
				vertex.texCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			}
			else vertex.texCoords = glm::vec2(0.0f, 0.0f);
			mesh_vertices.push_back(vertex);
		}

		// Process Indices
		std::vector<GLuint> mesh_indices;
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				mesh_indices.push_back(face.mIndices[j]);
		}

		std::shared_ptr<Material> temp_material = std::make_shared<Material>(shader, Engine::Get().GetTextureLibrary().GetTexture("blank_texture"));
		if (mesh->mMaterialIndex >= 0) {

			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			// Load any applicable texture files
			aiString texture_str;
			Texture* texture = nullptr;

			std::string temp;
			if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
				material->GetTexture(aiTextureType_DIFFUSE, 0, &texture_str);
				temp = texture_str.C_Str(); if (temp.rfind("..\\", 0) == 0) temp.erase(0, 3);
				texture = Engine::Get().GetTextureLibrary().loadTexture(directory + "/" + temp);
				temp_material->AddTextureMap(L20_TEXTURE_DIFFUSE_MAP, texture);
			}
			if (material->GetTextureCount(aiTextureType_SPECULAR) > 0) {
				material->GetTexture(aiTextureType_SPECULAR, 0, &texture_str);
				temp = texture_str.C_Str(); if (temp.rfind("..\\", 0) == 0) temp.erase(0, 3);
				texture = Engine::Get().GetTextureLibrary().loadTexture(directory + "/" + temp);
				temp_material->AddTextureMap(L20_TEXTURE_SPECULAR_MAP, texture);
			}

			if (material->GetTextureCount(aiTextureType_NORMALS) > 0) {
				material->GetTexture(aiTextureType_NORMALS, 0, &texture_str);
				temp = texture_str.C_Str(); if (temp.rfind("..\\", 0) == 0) temp.erase(0, 3);
				texture = Engine::Get().GetTextureLibrary().loadTexture(directory + "/" + temp);
				temp_material->AddTextureMap(L20_TEXTURE_NORMAL_MAP, texture);
			}

			// Load relevant material values
			float shine = 0;
			aiColor3D colour;
			material->Get(AI_MATKEY_COLOR_DIFFUSE, colour);
			temp_material->SetDiffuse(glm::vec4(colour.r, colour.g, colour.b, 1.0f));
			material->Get(AI_MATKEY_COLOR_SPECULAR, colour);
			temp_material->SetSpecular(glm::vec4(colour.r, colour.g, colour.b, 1.0f));
			material->Get(AI_MATKEY_SHININESS, shine);
			temp_material->SetShine(shine);

			aiString materialName;
			material->Get(AI_MATKEY_NAME, materialName);
			std::stringstream materialNameString;
			
			materialNameString << mesh->mName.C_Str() << "." << materialName.C_Str();
			
			temp_material->SetName(materialNameString.str());		
			Materials[temp_material->GetName()] = temp_material;
		}

		std::shared_ptr<Mesh> temp_mesh = std::make_shared<Mesh>(mesh_vertices, mesh_indices);
		temp_mesh->MaterialIndex = mesh->mMaterialIndex;

		return std::make_pair(temp_mesh, temp_material);
	}
}