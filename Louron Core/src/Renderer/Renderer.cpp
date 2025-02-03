#include "Renderer.h"

#include "../OpenGL/Buffer.h"
#include "../OpenGL/Vertex Array.h"

namespace Louron {

	static std::unique_ptr<VertexArray> s_DebugCubeVAO;
	static std::unique_ptr<VertexArray> s_DebugSphereVAO; // TODO: Implement this

	RenderPassStats Renderer::s_RenderStats = {};

	static GLuint debug_VAO = -1;

	// Function to compute spherical interpolation between two points
	glm::vec3 sphericalInterp(glm::vec3 start, glm::vec3 end, float t) {
		// Normalize vectors to ensure they lie on the surface of the sphere
		glm::vec3 startDir = normalize(start);
		glm::vec3 endDir = normalize(end);

		// Spherical interpolation
		float dotProd = dot(startDir, endDir);
		float theta = acos(dotProd);
		if (abs(theta) < 0.001) {
			return normalize(mix(startDir, endDir, t)); // Normalize after mix
		}

		float sinTheta = sin(theta);
		float scaleStart = sin((1.0 - t) * theta) / sinTheta;
		float scaleEnd = sin(t * theta) / sinTheta;

		// Return the interpolated point, clamped to the unit sphere (radius = 1)
		glm::vec3 result = scaleStart * startDir + scaleEnd * endDir;
		return normalize(result); // Normalize to clamp to unit sphere
	}

	void generateOctahedronSphereData(std::vector<GLfloat>& vertices, std::vector<GLuint>& indices) {
		// Vertices of the octahedron (normalized to form a unit sphere)
		glm::vec3 octahedronVertices[] = {
			glm::vec3(1.0f, 0.0f, 0.0f),  // +X
			glm::vec3(-1.0f, 0.0f, 0.0f), // -X
			glm::vec3(0.0f, 1.0f, 0.0f),  // +Y
			glm::vec3(0.0f, -1.0f, 0.0f), // -Y
			glm::vec3(0.0f, 0.0f, 1.0f),  // +Z
			glm::vec3(0.0f, 0.0f, -1.0f)  // -Z
		};

		// Edges between the octahedron vertices (each pair of indices forms a line)
		GLuint octahedronEdges[] = {
			0, 2, 0, 3, 0, 4, 0, 5,  // +X connections
			1, 2, 1, 3, 1, 4, 1, 5,  // -X connections
			2, 4, 2, 5, 3, 4, 3, 5   // Top-bottom connections
		};

		// Step 1: Generate vertices using spherical interpolation
		int edgeCount = sizeof(octahedronEdges) / sizeof(GLuint);
		for (int i = 0; i < edgeCount; i += 2) {
			glm::vec3 start = octahedronVertices[octahedronEdges[i]];
			glm::vec3 end = octahedronVertices[octahedronEdges[i + 1]];

			// Interpolate between the start and end vertices
			for (int t = 0; t <= 10; ++t) {  // Divide each edge into 10 parts
				float interpFactor = float(t) / 10.0f;
				glm::vec3 point = sphericalInterp(start, end, interpFactor);

				// Add the spherical point to the vertices list
				vertices.push_back(point.x);
				vertices.push_back(point.y);
				vertices.push_back(point.z);
			}
		}

		// Step 2: Generate indices to connect the interpolated points (forming the sphere's edges)
		for (int i = 0; i < edgeCount; i += 2) {
			int startIdx = (i / 2) * 11;  // 11 points per edge
			int endIdx = startIdx + 10;

			// Connect the points of the current edge
			for (int j = startIdx; j < endIdx; ++j) {
				indices.push_back(j);
				indices.push_back(j + 1);
			}
		}
	}

	void Renderer::Init() 
	{

		if (!s_DebugCubeVAO) {

			// Define the vertices of the cube
			GLfloat cubeVertices[] = {
				// Vertices of a unit cube
				-0.5f, -0.5f, -0.5f,
				 0.5f, -0.5f, -0.5f,
				 0.5f,  0.5f, -0.5f,
				-0.5f,  0.5f, -0.5f,
				-0.5f, -0.5f,  0.5f,
				 0.5f, -0.5f,  0.5f,
				 0.5f,  0.5f,  0.5f,
				-0.5f,  0.5f,  0.5f
			};

			// Define the indices for the edges of the cube
			GLuint cubeIndices[] = {
				0, 1, 1, 2, 2, 3, 3, 0, // Bottom face
				4, 5, 5, 6, 6, 7, 7, 4, // Top face
				0, 4, 1, 5, 2, 6, 3, 7  // Side edges
			};

			s_DebugCubeVAO = std::make_unique<VertexArray>();
			s_DebugCubeVAO->Bind();

			VertexBuffer* vbo = new VertexBuffer(cubeVertices, sizeof(cubeVertices));
			BufferLayout layout = {
				{ ShaderDataType::Float3, "aPos" }
			};
			vbo->SetLayout(layout);

			IndexBuffer* ebo = new IndexBuffer(cubeIndices, sizeof(cubeIndices) / sizeof(GLuint));

			s_DebugCubeVAO->AddVertexBuffer(vbo);
			s_DebugCubeVAO->SetIndexBuffer(ebo);

			s_DebugCubeVAO->UnBind();
		}

		if (!s_DebugSphereVAO)
		{
			std::vector<GLfloat> vertices;
			std::vector<GLuint> indices;

			generateOctahedronSphereData(vertices, indices);

			s_DebugSphereVAO = std::make_unique<VertexArray>();
			s_DebugSphereVAO->Bind();

			// Create a Vertex Buffer Object (VBO)
			VertexBuffer* vbo = new VertexBuffer(vertices.data(), vertices.size() * sizeof(GLfloat));
			BufferLayout layout = {
				{ ShaderDataType::Float3, "aPos" }
			};
			vbo->SetLayout(layout);

			// Create an Index Buffer Object (IBO)
			IndexBuffer* ebo = new IndexBuffer(indices.data(), indices.size());

			// Add buffers to VAO
			s_DebugSphereVAO->AddVertexBuffer(vbo);
			s_DebugSphereVAO->SetIndexBuffer(ebo);

			// Unbind VAO (good practice)
			s_DebugSphereVAO->UnBind();

		}
	}

	void Renderer::ClearColour(const glm::vec4 colour) 
	{
		glClearColor(colour.r, colour.g, colour.b, colour.a);
	}

	void Renderer::ClearBuffer(GLbitfield mask) 
	{
		glClear(mask);
	}

	void Renderer::DrawSubMesh(std::shared_ptr<SubMesh> sub_mesh) 
	{
		sub_mesh->VAO->Bind();
		glDrawElements(GL_TRIANGLES, sub_mesh->VAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);

		s_RenderStats.DrawCalls++;

		s_RenderStats.SubMeshes_Rendered++;

		s_RenderStats.Primitives_TriangleCount += sub_mesh->VAO->GetIndexBuffer()->GetCount() / 3;
		s_RenderStats.Primitives_VerticeCount += sub_mesh->VAO->GetIndexBuffer()->GetCount();
	}

	void Renderer::DrawDebugCube() 
	{
		s_DebugCubeVAO->Bind();
		glDrawElements(GL_LINES, s_DebugCubeVAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);

		s_RenderStats.DrawCalls++;

		s_RenderStats.SubMeshes_Rendered++;

		s_RenderStats.Primitives_LineCount += s_DebugCubeVAO->GetIndexBuffer()->GetCount() / 2;
		s_RenderStats.Primitives_VerticeCount += s_DebugCubeVAO->GetIndexBuffer()->GetCount();
		
	}

	void Renderer::DrawDebugSphere()
	{
		s_DebugSphereVAO->Bind();

		glDrawElements(GL_LINES, s_DebugSphereVAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);
		
		s_RenderStats.DrawCalls++;
		s_RenderStats.SubMeshes_Rendered++;
		s_RenderStats.Primitives_LineCount += s_DebugSphereVAO->GetIndexBuffer()->GetCount() / 2;
		s_RenderStats.Primitives_VerticeCount += s_DebugSphereVAO->GetIndexBuffer()->GetCount();
	}

	static GLuint s_DebugCubeInstanceBuffers = -1;

	void Renderer::DrawInstancedDebugCube(std::vector<glm::mat4> transforms) 
	{
		if (transforms.empty())
			return;

		if (s_DebugCubeInstanceBuffers == -1) {

			glGenBuffers(1, &s_DebugCubeInstanceBuffers);
			glBindBuffer(GL_ARRAY_BUFFER, s_DebugCubeInstanceBuffers);
			glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4), transforms.data(), GL_DYNAMIC_DRAW);
		}
		else {

			glBindBuffer(GL_ARRAY_BUFFER, s_DebugCubeInstanceBuffers);

			GLint bufferSize = 0;
			glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);

			if (bufferSize < transforms.size() * sizeof(glm::mat4)) {
				// Reallocate buffer with the new size
				glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4), &transforms[0], GL_DYNAMIC_DRAW);
			}
			else {
				// Update existing buffer with new data
				glBufferSubData(GL_ARRAY_BUFFER, 0, transforms.size() * sizeof(glm::mat4), &transforms[0]);
			}
		}

		// Additional validation
		if (glIsBuffer(s_DebugCubeInstanceBuffers) == GL_FALSE) {
			L_CORE_ERROR("Buffer is not valid after binding or updating data.");
			return;
		}

		s_DebugCubeVAO->Bind();

		// Set vertex attributes
		std::size_t vec4Size = sizeof(glm::vec4);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, GLsizei(4 * vec4Size), (void*)0);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, GLsizei(4 * vec4Size), (void*)(1 * vec4Size));

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, GLsizei(4 * vec4Size), (void*)(2 * vec4Size));

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, GLsizei(4 * vec4Size), (void*)(3 * vec4Size));

		glVertexAttribDivisor(1, 1);
		glVertexAttribDivisor(2, 1);
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);

		// DRAW CALL
		glDrawElementsInstanced(GL_LINES, s_DebugCubeVAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0, static_cast<GLuint>(transforms.size()));

		// Reset state after drawing
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);

		// Optionally reset divisor values if needed
		glVertexAttribDivisor(1, 0);
		glVertexAttribDivisor(2, 0);
		glVertexAttribDivisor(3, 0);
		glVertexAttribDivisor(4, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		s_RenderStats.DrawCalls++;
		s_RenderStats.Primitives_LineCount += s_DebugCubeVAO->GetIndexBuffer()->GetCount() / 2 * static_cast<GLuint>(transforms.size());
		s_RenderStats.Primitives_VerticeCount += s_DebugCubeVAO->GetIndexBuffer()->GetCount() * static_cast<GLuint>(transforms.size());
	}

	void Renderer::DrawSkybox(SkyboxComponent& skybox) 
	{

		skybox.Bind();
		glDrawArrays(GL_TRIANGLES, 0, 36);

		s_RenderStats.DrawCalls++;

		s_RenderStats.Skybox_Rendered++;

		s_RenderStats.Primitives_TriangleCount += 36 / 3;
		s_RenderStats.Primitives_VerticeCount += 36;

	}

	static GLuint s_MeshInstanceBuffers = -1;

	void Renderer::DrawInstancedSubMesh(std::shared_ptr<SubMesh> mesh, std::vector<glm::mat4> transforms) 
	{

		if (transforms.empty())
			return;

		if (s_MeshInstanceBuffers == -1) {
			
			glGenBuffers(1, &s_MeshInstanceBuffers);
			glBindBuffer(GL_ARRAY_BUFFER, s_MeshInstanceBuffers);
			glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4), transforms.data(), GL_DYNAMIC_DRAW);
		}
		else {

			glBindBuffer(GL_ARRAY_BUFFER, s_MeshInstanceBuffers);

			GLint bufferSize = 0;
			glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);

			if (bufferSize < transforms.size() * sizeof(glm::mat4)) {
				// Reallocate buffer with the new size
				glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4), &transforms[0], GL_DYNAMIC_DRAW);
			}
			else {
				// Update existing buffer with new data
				glBufferSubData(GL_ARRAY_BUFFER, 0, transforms.size() * sizeof(glm::mat4), &transforms[0]);
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
		glDrawElementsInstanced(GL_TRIANGLES, mesh->VAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0, static_cast<GLuint>(transforms.size()));

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

		s_RenderStats.DrawCalls++;

		s_RenderStats.SubMeshes_Instanced += static_cast<GLuint>(transforms.size());

		s_RenderStats.Primitives_TriangleCount += mesh->VAO->GetIndexBuffer()->GetCount() / 3 * static_cast<GLuint>(transforms.size());
		s_RenderStats.Primitives_VerticeCount += mesh->VAO->GetIndexBuffer()->GetCount() * static_cast<GLuint>(transforms.size());
	}

	void Renderer::CleanupRenderData() 
	{

		if (s_MeshInstanceBuffers != -1) {
			glDeleteBuffers(1, &s_MeshInstanceBuffers);
			s_MeshInstanceBuffers = -1;
		}
	}

	void Renderer::ClearRenderStats() { s_RenderStats = {}; }
	const RenderPassStats& Renderer::GetFrameRenderStats() { return s_RenderStats; }
}