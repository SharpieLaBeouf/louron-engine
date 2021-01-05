#include "../Headers/Abstracted GL/MeshRenderer.h"

MeshFilter::MeshFilter(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Material> materials)
{
	this->m_Vertices = vertices;
	this->m_Indices = indices;
	this->m_Materials = materials;

	setupMesh();

	m_Vertices.clear();
	m_Indices.clear();
}

