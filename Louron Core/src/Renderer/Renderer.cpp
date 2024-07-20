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


	static GLuint s_MeshInstanceBuffers = -1;

	void Renderer::DrawInstancedMesh(std::shared_ptr<Mesh> mesh, std::vector<Transform> transforms) {
		
		std::vector<glm::mat4> transformMatrices;
		for (int i = 0; i < transforms.size(); i++) {
			transformMatrices.push_back(transforms[i]);
		}

		if (s_MeshInstanceBuffers == -1) {
			
			glGenBuffers(1, &s_MeshInstanceBuffers);
			glBindBuffer(GL_ARRAY_BUFFER, s_MeshInstanceBuffers);
			glBufferData(GL_ARRAY_BUFFER, transformMatrices.size() * sizeof(glm::mat4), &transformMatrices[0], GL_DYNAMIC_DRAW);
		}
		else {

			glBindBuffer(GL_ARRAY_BUFFER, s_MeshInstanceBuffers);

			GLint bufferSize = 0;
			glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);

			if (bufferSize < transformMatrices.size() * sizeof(glm::mat4)) {
				// Reallocate buffer with the new size
				glBufferData(GL_ARRAY_BUFFER, transformMatrices.size() * sizeof(glm::mat4), &transformMatrices[0], GL_DYNAMIC_DRAW);
			}
			else {
				// Update existing buffer with new data
				glBufferSubData(GL_ARRAY_BUFFER, 0, transformMatrices.size() * sizeof(glm::mat4), &transformMatrices[0]);
			}
		}

		// Additional validation
		if (glIsBuffer(s_MeshInstanceBuffers) == GL_FALSE) {
			L_CORE_ERROR("Buffer is not valid after binding or updating data.");
			return;
		}

		mesh->VAO->Bind();

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

		// DRAW CALL
		glDrawElementsInstanced(GL_TRIANGLES, mesh->VAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0, (GLsizei)transforms.size());

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
	void Renderer::CleanupRenderData() {

		if (s_MeshInstanceBuffers != -1) {
			glDeleteBuffers(1, &s_MeshInstanceBuffers);
			s_MeshInstanceBuffers = -1;
		}
	}
}