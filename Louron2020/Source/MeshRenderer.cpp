#include "../Headers/Abstracted GL/MeshRenderer.h"

MeshFilter::MeshFilter(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices, State::InstanceManager* mgr) {
	m_InstanceManager = mgr;

	m_VAO = new VertexArray();
	VertexBuffer* vbo = new VertexBuffer(vertices, (GLuint)vertices.size());
	BufferLayout layout = { 
		{ ShaderDataType::Float3, "aPos" }, 
		{ ShaderDataType::Float3, "aNormal" }, 
		{ ShaderDataType::Float2, "aTexCoord" } 
	};
	vbo->SetLayout(layout);

	IndexBuffer* ebo = new IndexBuffer(indices, (GLuint)indices.size());

	m_VAO->AddVertexBuffer(vbo);
	m_VAO->SetIndexBuffer(ebo);
}

void MeshFilter::renderMeshFilter(Camera* mainCamera, Material* mat, Light* mainLight) {

	m_VAO->Bind();

	if (mat->Bind()) {
		mat->setUniforms();
		mat->getShader()->setMat4("model",	glm::mat4(1.0f));
		mat->getShader()->setMat4("proj",	glm::perspective(glm::radians(60.0f), m_InstanceManager->getWindowInstance()->getWidth() / m_InstanceManager->getWindowInstance()->getHeight(), 0.1f, 100.0f));
		mat->getShader()->setMat4("view",	mainCamera->getViewMatrix());
		mat->getShader()->setVec3("u_Light.position",	mainLight->position);
		mat->getShader()->setVec4("u_Light.ambient",	mainLight->ambient);
		mat->getShader()->setVec4("u_Light.diffuse",	mainLight->diffuse);
		mat->getShader()->setVec4("u_Light.specular",	mainLight->specular);
		mat->getShader()->setVec3("u_CameraPos",		mainCamera->getPosition());

		glDrawElements(GL_TRIANGLES, m_VAO->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, 0);

		mat->UnBind();
	}

	m_VAO->Unbind();
}

MeshRenderer::MeshRenderer(State::InstanceManager* instanceManager) : m_InstanceManager(instanceManager), m_ShaderName("material_shader_flat") {

}

int MeshRenderer::loadModel(const char* filePath, const char* shaderName) {

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_OptimizeMeshes);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "[L20] Error Reading File: " << importer.GetErrorString() << std::endl;
		return GL_FALSE;
	}
	m_Directory = filePath;
	m_Directory = m_Directory.substr(0, m_Directory.find_last_of('/'));
	m_ShaderName = shaderName;

	processNode(scene->mRootNode, scene);

	return GL_TRUE;
}

std::vector<MeshFilter*> MeshRenderer::getMeshes() { return m_Meshes; }

std::map<int, Material*>* MeshRenderer::getMaterials() { return &m_Materials; }
Material* MeshRenderer::getMaterial(int index) { return m_Materials[index]; }
void MeshRenderer::addMaterial(int index, Material* mat) { m_Materials[index] = mat; }

void MeshRenderer::renderEntireMesh(Camera* mainCamera, Light* mainLight) {
	for (GLuint i = 0; i < m_Meshes.size(); i++) {
		m_Meshes[i]->renderMeshFilter(mainCamera, m_Materials[m_Meshes[i]->getMaterialIndex()], mainLight);
	}
}

void MeshRenderer::processNode(aiNode* node, const aiScene* scene) {
	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		m_Meshes.push_back(processMesh(mesh, scene));
		m_MeshCount++;
	}
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}

}

MeshFilter* MeshRenderer::processMesh(aiMesh* mesh, const aiScene* scene) {


	aiString mesh_name;
	scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_NAME, mesh_name);
	std::cout << "[L20] Loading Mesh: " << mesh_name.C_Str() << std::endl;

	// Process Vertices
	std::vector<Vertex> mesh_vertices;
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
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

	// Create Material

	for (int i = 0; i < m_Materials.size(); i++) {
		if (m_Materials[i] != nullptr) {
			if (m_Materials[i]->getMaterialIndex() == mesh->mMaterialIndex) {

				MeshFilter* temp_mesh = new MeshFilter(mesh_vertices, mesh_indices, m_InstanceManager);
				temp_mesh->setMaterialIndex(mesh->mMaterialIndex);
				return temp_mesh;
			}

		}
	}

	Material* mesh_material = new Material(m_InstanceManager->getShaderLibInstance()->getShader(m_ShaderName),
		m_InstanceManager->getTextureLibInstance()->getTexture("blank_texture"));
	if (mesh->mMaterialIndex >= 0) {

		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		// Load any applicable texture files
		aiString texture_str;
		Texture* texture = nullptr;

		std::string temp;
		if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			material->GetTexture(aiTextureType_DIFFUSE, 0, &texture_str);
			temp = texture_str.C_Str(); if (temp.rfind("..\\", 0) == 0) temp.erase(0, 3);
			texture = m_InstanceManager->getTextureLibInstance()->loadTexture(m_Directory + "/" + temp);
			mesh_material->AddTextureMap(L20_TEXTURE_DIFFUSE_MAP, texture);
		}
		if (material->GetTextureCount(aiTextureType_SPECULAR) > 0) {
			material->GetTexture(aiTextureType_SPECULAR, 0, &texture_str);
			temp = texture_str.C_Str(); if (temp.rfind("..\\", 0) == 0) temp.erase(0, 3);
			texture = m_InstanceManager->getTextureLibInstance()->loadTexture(m_Directory + "/" + temp);
			mesh_material->AddTextureMap(L20_TEXTURE_SPECULAR_MAP, texture);
		}

		if (material->GetTextureCount(aiTextureType_NORMALS) > 0) {
			material->GetTexture(aiTextureType_NORMALS, 0, &texture_str);
			temp = texture_str.C_Str(); if (temp.rfind("..\\", 0) == 0) temp.erase(0, 3);
			texture = m_InstanceManager->getTextureLibInstance()->loadTexture(m_Directory + "/" + temp);
			mesh_material->AddTextureMap(L20_TEXTURE_NORMAL_MAP, texture);
		}

		// Load relevant material values
		float shine = 0;
		aiColor3D colour;
		material->Get(AI_MATKEY_COLOR_AMBIENT, colour);
		mesh_material->setAmbient(glm::vec4(colour.r, colour.g, colour.b, 1.0f));
		material->Get(AI_MATKEY_COLOR_DIFFUSE, colour);
		mesh_material->setDiffuse(glm::vec4(colour.r, colour.g, colour.b, 1.0f));
		material->Get(AI_MATKEY_COLOR_SPECULAR, colour);
		mesh_material->setSpecular(glm::vec4(colour.r, colour.g, colour.b, 1.0f));
		material->Get(AI_MATKEY_SHININESS, shine);
		mesh_material->setShine(32.0f);

		mesh_material->setMaterialIndex(mesh->mMaterialIndex);
	}
	m_Materials[mesh->mMaterialIndex] = mesh_material;

	MeshFilter* temp_mesh = new MeshFilter(mesh_vertices, mesh_indices, m_InstanceManager);
	temp_mesh->setMaterialIndex(mesh->mMaterialIndex);
	return temp_mesh;
}