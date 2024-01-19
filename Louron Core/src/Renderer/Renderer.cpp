#include "Renderer.h"

namespace Louron {

	void Renderer::DrawMeshComponent(Entity* MeshEntity, Entity* CameraEntity, std::string optionalShader) {

		CameraComponent& camera = CameraEntity->GetComponent<CameraComponent>();
		MaterialComponent& material = MeshEntity->GetComponent<MaterialComponent>();
		TransformComponent& transform = MeshEntity->GetComponent<TransformComponent>();
		std::vector<std::shared_ptr<MeshFilter>> meshes = MeshEntity->GetComponent<MeshComponent>().GetMeshes();

		// Get All Mesh Filters in Mesh Component
		for (auto& meshFilter : meshes) {

			meshFilter->VAO->Bind();

			if (material.Materials[meshFilter->MaterialIndex] != nullptr) {
				std::shared_ptr<Material>& mat = material.Materials[meshFilter->MaterialIndex];
				std::shared_ptr<Shader> shader;
				
				if (optionalShader == "")
					shader = mat->GetShader();
				else
					shader = Engine::Get().GetShaderLibrary().GetShader(optionalShader);

				shader->Bind();

				// mat->UpdateUniforms();
				
				if (shader->GetName() != "FP_Depth")
				{

					// Set Material Uniforms

					shader->SetFloat("u_Material.shine", mat->GetShine());
					shader->SetVec4("u_Material.diffuse", *mat->GetDiffuse());
					shader->SetVec4("u_Material.specular", *mat->GetSpecular());

					if (mat->GetTextureMap(L20_TEXTURE_DIFFUSE_MAP) != nullptr) {

						shader->SetInt(std::string("u_Material.diffuseMap").c_str(), L20_TEXTURE_DIFFUSE_MAP);
						glActiveTexture(GL_TEXTURE0 + L20_TEXTURE_DIFFUSE_MAP);
						glBindTexture(GL_TEXTURE_2D, mat->GetTextureMap(L20_TEXTURE_DIFFUSE_MAP)->getID());
					}
					if (mat->GetTextureMap(L20_TEXTURE_SPECULAR_MAP) != nullptr) {

						shader->SetInt(std::string("u_Material.specularMap").c_str(), L20_TEXTURE_SPECULAR_MAP);
						glActiveTexture(GL_TEXTURE0 + L20_TEXTURE_SPECULAR_MAP);
						glBindTexture(GL_TEXTURE_2D, mat->GetTextureMap(L20_TEXTURE_SPECULAR_MAP)->getID());
					}
					if (mat->GetTextureMap(L20_TEXTURE_NORMAL_MAP) != nullptr) {

						shader->SetInt(std::string("u_Material.normalMap").c_str(), L20_TEXTURE_NORMAL_MAP);
						glActiveTexture(GL_TEXTURE0 + L20_TEXTURE_NORMAL_MAP);
						glBindTexture(GL_TEXTURE_2D, mat->GetTextureMap(L20_TEXTURE_NORMAL_MAP)->getID());
					}
				}

				// Set General Shader Uniforms

				shader->SetMat4("model", transform.GetTransform());
				shader->SetMat4("proj", camera.Camera->GetProjMatrix());
				shader->SetMat4("view", camera.Camera->GetViewMatrix());
				shader->SetVec3("u_CameraPos", camera.Camera->GetPosition());

				// Draw

				glDrawElements(GL_TRIANGLES, meshFilter->VAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);

				shader->UnBind();
			}
			else {
				std::cout << "[L20] Material Index Not In MaterialComponent" << std::endl;
			}

		}
	}
	void Renderer::DrawMeshFilter(std::shared_ptr<MeshFilter> Mesh)	{

		Mesh->VAO->Bind();
		glDrawElements(GL_TRIANGLES, Mesh->VAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);

	}
}