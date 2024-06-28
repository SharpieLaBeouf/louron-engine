#include "Renderer.h"

namespace Louron {

	void Renderer::ClearColour(const glm::vec4 colour) {
		glClearColor(colour.r, colour.g, colour.b, colour.a);
	}

	void Renderer::ClearBuffer(GLbitfield mask) {
		glClear(mask);
	}

	void Renderer::DrawMesh(std::shared_ptr<Mesh> Mesh) {
		Mesh->VAO->Bind();
   		glDrawElements(GL_TRIANGLES, Mesh->VAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);
	}

	void Renderer::DrawSkybox(SkyboxComponent& skybox) {
		skybox.Bind();
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	static GLuint s_InstanceBuffer = -1;

	void Renderer::DrawInstancedMesh(std::shared_ptr<Mesh> Mesh, std::vector<Transform> Transforms) {
		
		std::vector<glm::mat4> transformMatrices;
		for (int i = 0; i < Transforms.size(); i++) {
			transformMatrices.push_back(Transforms[i]);
		}

		if (s_InstanceBuffer == -1) {
			glGenBuffers(1, &s_InstanceBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, s_InstanceBuffer);
			glBufferData(GL_ARRAY_BUFFER, transformMatrices.size() * sizeof(glm::mat4), &transformMatrices[0], GL_DYNAMIC_DRAW);

		}
		else {
			glBindBuffer(GL_ARRAY_BUFFER, s_InstanceBuffer);
			glBufferSubData(GL_ARRAY_BUFFER, 0, transformMatrices.size() * sizeof(glm::mat4), &transformMatrices[0]);
		}


		Mesh->VAO->Bind();

		// Set vertex attributes
		std::size_t vec4Size = sizeof(glm::vec4);
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, GLsizei(4 * vec4Size), (void*)0);
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, GLsizei(4 * vec4Size), (void*)(1 * vec4Size));
		glEnableVertexAttribArray(7);
		glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, GLsizei(4 * vec4Size), (void*)(2 * vec4Size));
		glEnableVertexAttribArray(8);
		glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, GLsizei(4 * vec4Size), (void*)(3 * vec4Size));

		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
		glVertexAttribDivisor(7, 1);
		glVertexAttribDivisor(8, 1);

		glDrawElementsInstanced(GL_TRIANGLES, Mesh->VAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0, (GLsizei)Transforms.size());

		// Reset state after drawing
		glDisableVertexAttribArray(5);
		glDisableVertexAttribArray(6);
		glDisableVertexAttribArray(7);
		glDisableVertexAttribArray(8);

		// Optionally reset divisor values if needed
		glVertexAttribDivisor(5, 0);
		glVertexAttribDivisor(6, 0);
		glVertexAttribDivisor(7, 0);
		glVertexAttribDivisor(8, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	void Renderer::CleanupInstanceData() {
		// TODO: Implement cleaning up of buffer somewhere
		if (s_InstanceBuffer != -1) {
			glDeleteBuffers(1, &s_InstanceBuffer);
			s_InstanceBuffer = -1;
		}
	}
}