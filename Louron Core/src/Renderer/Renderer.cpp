#include "Renderer.h"

namespace Louron {

	void Renderer::DrawMeshComponent(Entity* MeshEntity, Entity* CameraEntity, std::string optionalShader) {

		CameraComponent& camera = CameraEntity->GetComponent<CameraComponent>();
		Transform& transform = MeshEntity->GetComponent<Transform>();
		MeshRenderer& meshRenderer = MeshEntity->GetComponent<MeshRenderer>();
		MeshFilter& meshFilter = MeshEntity->GetComponent<MeshFilter>();

		// Get All Mesh Filters in Mesh Component
		for (auto& mesh : *meshFilter.Meshes) {

			mesh->VAO->Bind();

			if ((*meshRenderer.Materials)[mesh->MaterialIndex] != nullptr) {
				std::shared_ptr<Material> mat = (*meshRenderer.Materials)[mesh->MaterialIndex];
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

				shader->SetMat4("model", transform);
				shader->SetMat4("proj", camera.Camera->GetProjMatrix());
				shader->SetMat4("view", camera.Camera->GetViewMatrix());
				shader->SetVec3("u_CameraPos", camera.Camera->GetPosition());

				// Draw

				glDrawElements(GL_TRIANGLES, mesh->VAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);

				shader->UnBind();
			}
			else {
				std::cout << "[L20] Material Index Not In MaterialComponent" << std::endl;
			}

		}
	}

	void Renderer::DrawMesh(std::shared_ptr<Mesh> Mesh) {
		Mesh->VAO->Bind();
   		glDrawElements(GL_TRIANGLES, Mesh->VAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);
	}

	GLuint InstanceBuffer = -1;

	void Renderer::DrawInstancedMesh(std::shared_ptr<Mesh> Mesh, std::vector<Transform> Transforms) {

		// TODO: Instancing

		std::vector<glm::mat4> transformMatrices;
		for (int i = 0; i < Transforms.size(); i++)
			transformMatrices.push_back(Transforms[i]);

		if(InstanceBuffer == -1)
			glGenBuffers(1, &InstanceBuffer);

		glBindBuffer(GL_ARRAY_BUFFER, InstanceBuffer);
		glBufferData(GL_ARRAY_BUFFER, transformMatrices.size() * sizeof(glm::mat4), &transformMatrices[0], GL_STATIC_DRAW);

		Mesh->VAO->Bind();

		// vertex attributes
		std::size_t vec4Size = sizeof(glm::vec4);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, GLsizei(4 * vec4Size), (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, GLsizei(4 * vec4Size), (void*)(1 * vec4Size));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, GLsizei(4 * vec4Size), (void*)(2 * vec4Size));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, GLsizei(4 * vec4Size), (void*)(3 * vec4Size));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glDrawElementsInstanced(GL_TRIANGLES, Mesh->VAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0, (GLsizei)Transforms.size());

		// Reset state after drawing
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);
		glDisableVertexAttribArray(5);
		glDisableVertexAttribArray(6);

		// Optionally reset divisor values if needed
		glVertexAttribDivisor(3, 0);
		glVertexAttribDivisor(4, 0);
		glVertexAttribDivisor(5, 0);
		glVertexAttribDivisor(6, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &InstanceBuffer);

	}
}