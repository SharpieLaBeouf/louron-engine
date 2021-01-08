#include "../Headers/Abstracted GL/MeshRenderer.h"

MeshFilter::MeshFilter(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Material* material)
{
	this->m_Vertices = vertices;
	this->m_Indices = indices;
	this->m_Material = material;

	setupMesh();

	m_Vertices.clear();
	m_Indices.clear();
}

