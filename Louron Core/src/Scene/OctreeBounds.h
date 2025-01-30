#pragma once

// This was a big headache - take a look at my GameDev Stack Exchange post 
// where I hear crickets from the crowd and eventually figure it out myself lol
// https://gamedev.stackexchange.com/questions/211647/octree-query-frustum-search-and-recursive-vector-inserts/211698#211698

#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/component_wise.hpp>

#include "../Core/Logging.h"
#include "../Debug/Assert.h"

#include "Bounds.h"
#include "Frustum.h"

namespace Louron {

	struct OctreeBoundsConfig {

		/// <summary>
		/// This is the minimum node size we can have. If the node attemps to split
		/// this configuration will prevent the Octree from splitting into smaller
		/// child nodes. This is set at 1.0f == 1 unit in worldspace.
		/// 
		/// If this is increased to 2.0f, a node cannot split if it's child nodes 
		/// will be smaller than 2 units in worldspace.
		/// </summary>
		float MinNodeSize = 1.0f;

		/// <summary>
		/// This is the preferred limit of Data Sources per node before the node
		/// splits into child nodes and attempts to reinsert these Data Sources 
		/// into its child nodes.
		/// 
		/// If we reach the the minimum node size, we cannot 
		/// split any further, which is why this is a preferred limit for 
		/// Data Sources per node.
		/// </summary>
		int PreferredDataSourceLimit = 4;

		/// <summary>
		/// Looseness of Octree
		/// 
		/// Min Value: 1.0f
		/// Max Value: 2.0f 
		/// (any other values will be clamped between min and max)
		/// 
		/// Standard Octree 1.0f: child boundaries are exactly 100% / 2.0 = 50% 
		/// of the parent node boundaries.
		/// 
		/// As you increase this to two, this increases how far the octree
		/// overlaps into neighbouring child nodes to reduce edge cases.
		/// 
		/// Loose Octree 1.5f: child boundaries are exactly 150% / 2.0 = 75% 
		/// of the parent node boundaries.
		/// 
		/// </summary>
		float Looseness = 1.0f;

		/// <summary>
		/// Initial Bounds of the Octree Root Node.
		/// </summary>
		Bounds_AABB InitialBounds{ glm::vec3 (-50.0f), glm::vec3(50.0f) };

		/// <summary>
		/// When a Node is Queried, and itself and it's children
		/// have no data sources, a life count will be incremented
		/// until 8 is reached. This means that if you remove a 
		/// data source, it will start the counter. If you query
		/// and the node is empty, we will increment again. If the
		/// life of the node exceeds the MaxLifeIfEmpty, the node
		/// and its children will be deleted. Each Node starts off
		/// with this amount, and doubles each time if it is 
		/// saved by something being added up to a max of 64 queries.
		/// </summary>
		uint8_t MaxLifeIfEmpty = 8;

	};

	template <typename DataType>
	struct OctreeDataSource {

		DataType Data;
		Bounds_AABB Bounds;

		OctreeDataSource() = delete;
		OctreeDataSource(DataType data, Bounds_AABB bounds) : Data(data), Bounds(bounds) { }

		OctreeDataSource(const OctreeDataSource& other) = default;
		OctreeDataSource& operator=(const OctreeDataSource& other) = default;

		OctreeDataSource(OctreeDataSource&& other) = default;
		OctreeDataSource& operator=(OctreeDataSource&& other) = default;
	};

	template <typename DataType>
	class OctreeBounds {

	private:

		template <typename DataType>
		class OctreeBoundsNode {

		private:

			using OctreeNode = std::shared_ptr<OctreeBoundsNode<DataType>>;
			using OctreeData = std::shared_ptr<OctreeDataSource<DataType>>;
			friend class OctreeBounds;

		public:

			// Constructor and Destructor
			OctreeBoundsNode() = default;
			OctreeBoundsNode(Bounds_AABB node_bounds, OctreeBounds<DataType>* octree) : m_NodeBounds(node_bounds), m_Octree(octree){

				m_LifeMax = m_Octree ? m_Octree->m_Config.MaxLifeIfEmpty : 8;

				m_NodeMat4 = m_NodeBounds.GetGlobalBoundsMat4();
				m_ChildrenBounds = this->CalculateChildBounds();
			}
			~OctreeBoundsNode() = default;

			// COPY
			OctreeBoundsNode(const OctreeBoundsNode&) = default;
			OctreeBoundsNode& operator=(const OctreeBoundsNode&) = default;

			// MOVE
			OctreeBoundsNode(OctreeBoundsNode&&) = default;
			OctreeBoundsNode& operator=(OctreeBoundsNode&&) = default;

			/// <summary>
			/// This will attempt to insert a data source within the Octree.
			/// </summary>
			bool Insert(OctreeData data, size_t node_data_start_position, bool already_checked_contains = false) {

				// 1. Set the position of the Node's data source start index
				m_DataSourceIndex = node_data_start_position;

				// 2. Have we already checked if this node contains the data source?
				if (!already_checked_contains) {
					// No we haven't so we will perform the check now!
					if (m_NodeBounds.Contains(data->Bounds, m_Octree->m_Config.Looseness) != BoundsContainResult::Contains) {
						// The data does not fit in the current node, return it to the caller :'( OR :D ... depends what node you're in ;)
						return false;
					}
				}

				// 3. First Insert Check: check if the current node has less than the suggested max data_sources
				// or pass this check if the node has already been split.
				if (m_DataSourceSize < m_Octree->m_Config.PreferredDataSourceLimit && !IsNodeSplit()) {
					
					InsertDataToNode(data);
					return true;
				}

				// 4. Second Insert Check: are we able to split the node? 
				
				// If the children nodes will be smaller than the defined minimum size, we 
				// will simply add to this node and return true.
				glm::vec3 child_bounds_size = m_ChildrenBounds[0].Size();
				if (child_bounds_size.x < m_Octree->m_Config.MinNodeSize || child_bounds_size.y < m_Octree->m_Config.MinNodeSize || child_bounds_size.z < m_Octree->m_Config.MinNodeSize) {

					InsertDataToNode(data);
					return true;
				}

				// Split the node as we should not place into the current node YET
				if(!IsNodeSplit())
					SplitNode();

				// 5. Third Insert Check: Attempt insert into child node
				size_t child_data_start = m_DataSourceIndex + m_DataSourceSize;
				for (int i = 0; i < m_ChildrenBounds.size(); i++) {

					if (m_ChildrenBounds[i].Contains(data->Bounds, m_Octree->m_Config.Looseness) == BoundsContainResult::Contains) {

						if (!m_ChildrenNodes[i]) {
							m_ChildrenNodes[i] = std::make_shared<OctreeBoundsNode<DataType>>(m_ChildrenBounds[i], m_Octree);
							m_IsNodeSplit = true;
						}

						if (m_ChildrenNodes[i]->Insert(data, child_data_start, true)) {
							m_TotalNodeDataSourceSize++;
							return true;
						}
						else { 
							break; 
						}

					}

					if(m_ChildrenNodes[i])
						child_data_start += m_ChildrenNodes[i]->TotalCount();
				}

				// 6. Fourth Insert Check: If the data_source could not be inserted into a child
				// or for some reason there was an error, we will just insert into this node.

				InsertDataToNode(data);
				return true;
			}

			/// <summary>
			/// This will find the node that holds this data_position, and 
			/// remove this data source from the Octree Storage, and update
			/// the node size and all parent total node size.
			/// </summary>
			bool Remove(size_t data_position) {

				// 1. Check if data position out of TOTAL node data bounds
				size_t end = m_TotalNodeDataSourceSize > 0 ? m_DataSourceIndex + m_TotalNodeDataSourceSize - 1 : m_DataSourceIndex;
				if (data_position < m_DataSourceIndex || data_position > end || m_TotalNodeDataSourceSize == 0)
					return false;

				// 2. Check if data position out of CURRENT node data bounds, we need to check children
				end = m_DataSourceSize > 0 ? m_DataSourceIndex + m_DataSourceSize - 1 : 0;
				if (data_position > end || m_DataSourceSize == 0) {

					for (int i = 0; i < m_ChildrenNodes.size(); i++) {
						if (m_ChildrenNodes[i]) {
							if (m_ChildrenNodes[i]->Remove(data_position)) {
								m_TotalNodeDataSourceSize--;
								return true;
							}
						}
					}

					L_CORE_WARN("Octree - Data Misalignment: Data Source Could Not Be Removed From Children.");
					return false; // If this could not be removed from children, what the heck has happened here?
				}

				if (!m_Octree)
					return false;

				m_Octree->m_DataSources.erase(m_Octree->m_DataSources.begin() + data_position);
				
				if (m_DataSourceSize > 0)
					m_DataSourceSize--;
				
				if (m_TotalNodeDataSourceSize > 0) 
					m_TotalNodeDataSourceSize--;
				
				return true;
			}

			/// <summary>
			/// Query a particular region and return a result.
			/// </summary>
			void Query(const Bounds_AABB& query_bounds, std::vector<OctreeData>& result, bool& should_delete_node) {

				// If there are no data sources in itself or children 
				// why bother testing this node?
				if (TotalCount() == 0) {
					should_delete_node = CheckShouldDeleteNode();
					return;
				}

				switch (query_bounds.Contains(m_NodeBounds)) {

				case BoundsContainResult::Contains: {

					// Mmmm grab all of our contiguous data in one fell swoop, yummy
					result.insert(
						result.end(),
						m_Octree->m_DataSources.begin() + m_DataSourceIndex,
						m_Octree->m_DataSources.begin() + m_DataSourceIndex + TotalCount()
					);
					break;
				}

				case BoundsContainResult::Intersects: {

					// Test intersection of each data source pertaining to this node
					for (int i = 0; i < m_DataSourceSize; i++) {
						try {
							const auto& data = m_Octree->m_DataSources.at(m_DataSourceIndex + i);
							if (query_bounds.Contains(data->Bounds) != BoundsContainResult::DoesNotContain)
								result.push_back(data);
						}
						catch (const std::out_of_range& ex) {
							L_CORE_ERROR("Octree Query - Data Out of Range Could Not Test Intersection.");
							L_CORE_ERROR(ex.what());
						}
					}

					// If Node is not split, just break
					if (!IsNodeSplit())
						break;

					// Query children
					for (int i = 0; i < m_ChildrenNodes.size(); i++) {

						if (m_ChildrenNodes[i])
						{
							bool should_delete = false;
							m_ChildrenNodes[i]->Query(query_bounds, result, should_delete);
							if (should_delete && !m_ChildrenNodes[i]->IsRootNode()) {
								m_ChildrenNodes[i]->Clear();
								m_ChildrenNodes[i].reset();
								m_ChildrenNodes[i] = nullptr;
							}
						}

					}

					break;
				}

				}
			}

			/// <summary>
			/// Query a particular region and return a result.
			/// </summary>
			void Query(const Bounds_Sphere& query_bounds, std::vector<OctreeData>& result, bool& should_delete_node) {

				// If there are no data sources in itself or children 
				// why bother testing this node?
				if (TotalCount() == 0) {
					should_delete_node = CheckShouldDeleteNode();
					return;
				}

				switch (query_bounds.Contains(m_NodeBounds)) {

				case BoundsContainResult::Contains: {

					// Mmmm grab all of our contiguous data in one fell swoop, yummy
					result.insert(
						result.end(),
						m_Octree->m_DataSources.begin() + m_DataSourceIndex,
						m_Octree->m_DataSources.begin() + m_DataSourceIndex + TotalCount()
					);
					break;
				}

				case BoundsContainResult::Intersects: {

					// Test intersection of each data source pertaining to this node
					for (int i = 0; i < m_DataSourceSize; i++) {
						try {
							const auto& data = m_Octree->m_DataSources.at(m_DataSourceIndex + i);
							if (data && query_bounds.Contains(data->Bounds) != BoundsContainResult::DoesNotContain)
								result.push_back(data);
						}
						catch (const std::out_of_range& ex) {
							L_CORE_ERROR("Octree Query - Data Out of Range Could Not Test Intersection.");
							L_CORE_ERROR(ex.what());
						}
					}

					// If Node is not split, just break
					if (!IsNodeSplit())
						break;

					// Query children
					for (int i = 0; i < m_ChildrenNodes.size(); i++) {

						if (m_ChildrenNodes[i])
						{
							bool should_delete = false;
							m_ChildrenNodes[i]->Query(query_bounds, result, should_delete);
							if (should_delete && !m_ChildrenNodes[i]->IsRootNode()) {
								m_ChildrenNodes[i]->Clear();
								m_ChildrenNodes[i].reset();
								m_ChildrenNodes[i] = nullptr;
							}
						}

					}

					break;
				}

				}
			}

			/// <summary>
			/// Query a particular region and return a result.
			/// </summary>
			void Query(const Frustum& frustum, std::vector<OctreeData>& result, bool& should_delete_node) {

				// If there are no data sources in itself or children 
				// why bother testing this node?
				if (TotalCount() == 0) {
					should_delete_node = CheckShouldDeleteNode();
					return;
				}
				
				switch (frustum.Contains(m_NodeBounds * m_Octree->m_Config.Looseness)) {

					case FrustumContainResult::Contains: {
						
						// Mmmm grab all of our contiguous data in one fell swoop, yummy
						result.insert(
							result.end(),
							m_Octree->m_DataSources.begin() + m_DataSourceIndex,
							m_Octree->m_DataSources.begin() + m_DataSourceIndex + TotalCount()
						);
						break;
					}

					case FrustumContainResult::Intersects: {

						// Test intersection of each data source pertaining to this node
						for (int i = 0; i < m_DataSourceSize; i++) {
							try {
								const auto& data = m_Octree->m_DataSources.at(m_DataSourceIndex + i);
								if (frustum.Contains(data->Bounds) != FrustumContainResult::DoesNotContain)
									result.push_back(data);
							}
							catch (const std::out_of_range& ex) {
								L_CORE_ERROR("Octree Query - Data Out of Range Could Not Test Intersection.");
								L_CORE_ERROR(ex.what());
							}
						}

						// If Node is not split, just break
						if (!IsNodeSplit())
							break;

						// Query children
						for (int i = 0; i < m_ChildrenNodes.size(); i++) {

							if (m_ChildrenNodes[i])
							{
								bool should_delete = false;
								m_ChildrenNodes[i]->Query(frustum, result, should_delete);
								if (should_delete) {
									m_ChildrenNodes[i].reset();
									m_ChildrenNodes[i] = nullptr;
								}
							}

						}

						break;
					}

				}
			}

			/// <summary>
			/// This should be called any time we make any changes 
			/// to the Octree Data Storage to ensure all nodes point
			/// to the valid index of where their data starts.
			/// </summary>
			void RevalidateNodes(size_t start_index) {

				L_CORE_ASSERT(start_index + m_DataSourceSize <= m_Octree->m_DataSources.size(), "Octree - Index Out Of Bounds!");

				m_DataSourceIndex = start_index;

				if (!IsNodeSplit()) {
					return;
				}

				start_index += m_DataSourceSize;

				for (int i = 0; i < m_ChildrenNodes.size(); i++) {
					if (!m_ChildrenNodes[i])
						continue;

					m_ChildrenNodes[i]->RevalidateNodes(start_index);
					start_index += m_ChildrenNodes[i]->TotalCount();
				}

				m_TotalNodeDataSourceSize = start_index - m_DataSourceIndex;
			}

			/// <summary>
			/// Get count of node's data.
			/// </summary>
			/// <returns></returns>
			const size_t& Count() const {
				return m_DataSourceSize;
			}

			/// <summary>
			/// Get count of node's data including children
			/// data sources.
			/// </summary>
			size_t TotalCount() const {
				return m_TotalNodeDataSourceSize;
			}

			/// <summary>
			/// Manual recursive approach to getting the true
			/// size of self and all child nodes data count.
			/// </summary>
			size_t ForceTotalCount() {

				size_t count = m_DataSourceSize;

				for (const auto& child_node : m_ChildrenNodes)
					if (child_node)
						count += child_node->ForceTotalCount();

				m_TotalNodeDataSourceSize = count;

				return count;
			}

			/// <summary>
			/// This will clear all data sources in this and all children nodes.
			/// </summary>
			void Clear() {
				// Clear data sources
				m_Octree->m_DataSources.erase(m_Octree->m_DataSources.begin() + m_DataSourceIndex, m_Octree->m_DataSources.begin() + m_DataSourceIndex + TotalCount());
				m_DataSourceSize = 0;
				m_TotalNodeDataSourceSize = 0;

				// Recursively clear child nodes
				for (auto& child_node : m_ChildrenNodes) {
					if (child_node) {
						child_node.reset();
						child_node = nullptr;
					}
				}

				m_Octree->m_RootNode->RevalidateNodes(0);

				// Reset split status
				m_IsNodeSplit = false;
			}

			/// <summary>
			/// This will tell us if our node has been split into children.
			/// </summary>
			bool IsNodeSplit() const {
				return m_IsNodeSplit;
			}

			/// <summary>
			/// This will tell us if we are in the root node or not.
			/// </summary>
			bool IsRootNode() const {

				if (!m_Octree) {
					L_CORE_ERROR("Octree - Node Does Not Contain Valid Reference to Octree.");
					return false;
				}

				if (m_Octree->GetRootNode().get() == this)
					return true;
				return false;
			}

			/// <summary>
			/// This assigns a child node to this parent node.
			/// </summary>
			bool SetChildNode(int index, OctreeNode node) {
				if (index >= 8 || index < 0)
					return false;

				m_ChildrenNodes[index] = node;
				m_TotalNodeDataSourceSize += node->m_TotalNodeDataSourceSize;
				m_IsNodeSplit = true;
				return true;
			}
			
			/// <summary>
			/// This finds which child node a particular point would fit in best.
			/// </summary>
			int BestFitChild(const glm::vec3& point) {
				for (int i = 0; i < m_ChildrenBounds.size(); i++) {
					if (m_ChildrenBounds[i].Contains(point) == BoundsContainResult::Contains)
						return i;
				}
				return -1; // Unsuccessful
			}

			/// <summary>
			/// This will get the node bounds of the children directly relative to 
			/// this node only, and not children of children.
			/// </summary>
			const std::array<Bounds_AABB, 8>& GetNodeChildBounds() const { return m_ChildrenBounds; }

			/// <summary>
			/// This will get the node bounds for this and all child nodes.
			/// </summary>
			std::vector<Bounds_AABB> GetRecursiveChildBounds() {
				std::vector<Bounds_AABB> bounds_vector;
				bounds_vector.push_back(GetNodeBounds());
				
				// Base case: if no children, return this nodes bounds
				if (!IsNodeSplit()) {
					return bounds_vector;
				}

				// Recursively gather bounds from child nodes
				for (const auto& child : m_ChildrenNodes) {
					if (child) {

						// Recursively call this on children
						if(std::vector<Bounds_AABB> child_bounds_vector = child->GetRecursiveChildBounds(); !child_bounds_vector.empty()) {
							bounds_vector.insert(bounds_vector.end(), child_bounds_vector.begin(), child_bounds_vector.end());
						}
					}
				}

				return bounds_vector;
			}

			/// <summary>
			/// This will get the node bounds glm::mat4 for this and all child nodes.
			/// </summary>
			std::vector<glm::mat4> GetRecursiveChildBoundsMat4() {
				std::vector<glm::mat4> transforms_vector;
				transforms_vector.push_back(GetNodeBoundsMat4());

				// Base case: if no children, return this nodes mat4
				if (!IsNodeSplit()) {
					return transforms_vector;
				}

				// Recursively gather transforms from child nodes
				for (const auto& child : m_ChildrenNodes) {
					if (child) {

						// Recursively call this on children
						if (std::vector<glm::mat4> child_transforms_vector = child->GetRecursiveChildBoundsMat4(); !child_transforms_vector.empty()) {
							transforms_vector.insert(transforms_vector.end(), child_transforms_vector.begin(), child_transforms_vector.end());
						}

					}
				}

				return transforms_vector;
			}

			/// <summary>
			/// This will get the node bounds for this node only.
			/// </summary>
			const Bounds_AABB& GetNodeBounds() { return m_NodeBounds; }

			/// <summary>
			/// This will get the node bounds glm::mat4 for this node only.
			/// </summary>
			const glm::mat4& GetNodeBoundsMat4() { return m_NodeMat4; }

		private:

			/// <summary>
			/// This will split the node and distribute it's children into
			/// child nodes.
			/// </summary>
			void SplitNode() {

				if (m_DataSourceSize == 0)
					return;

				std::vector<OctreeData> data_sources{};
				data_sources.reserve(m_DataSourceSize);

				// Move from global Octree and put into temporary storage to distribute data_sources
				data_sources.insert(data_sources.begin(), m_Octree->m_DataSources.begin() + m_DataSourceIndex, m_Octree->m_DataSources.begin() + m_DataSourceIndex + m_DataSourceSize);
				m_Octree->m_DataSources.erase(m_Octree->m_DataSources.begin() + m_DataSourceIndex, m_Octree->m_DataSources.begin() + m_DataSourceIndex + m_DataSourceSize);

				m_DataSourceSize = 0;

				size_t child_data_start = m_DataSourceIndex;

				// Iterate through m_ChildrenBounds and see if can be placed in different children
				for (int i = 0; i < m_ChildrenBounds.size(); i++) {

					auto it = data_sources.begin();
					while (it != data_sources.end()) {

						if (m_ChildrenBounds[i].Contains((*it)->Bounds, m_Octree->m_Config.Looseness) == BoundsContainResult::Contains) {

							if (!m_ChildrenNodes[i]) {
								m_ChildrenNodes[i] = std::make_shared<OctreeBoundsNode<DataType>>(m_ChildrenBounds[i], m_Octree);
								m_ChildrenNodes[i]->m_DataSourceIndex = child_data_start;
								m_IsNodeSplit = true;
							}
							m_ChildrenNodes[i]->m_DataSourceSize++;
							m_ChildrenNodes[i]->m_TotalNodeDataSourceSize++;

							m_Octree->m_DataSources.insert(m_Octree->m_DataSources.begin() + child_data_start, *it);
							child_data_start++;

							// Erase data from temporary vector, and ensure the while loop 
							// iterator is no invalidated
							it = data_sources.erase(it);
							continue;
						}
						++it; // Continue to next element
					}

				}

				for (auto& data_source : data_sources) {
					size_t insert_position = m_DataSourceIndex + m_DataSourceSize;
					m_Octree->m_DataSources.insert(m_Octree->m_DataSources.begin() + insert_position, data_source);
					m_DataSourceSize++;
				}

				// Revalidate child node start positions
				for (int i = 0; i < m_ChildrenNodes.size(); i++) {
					if (m_ChildrenNodes[i]) {
						m_ChildrenNodes[i]->m_DataSourceIndex += m_DataSourceSize;
					}
				}
			}

			/// <summary>
			/// This checks if this node should be deleted by its parent 
			/// during a query.
			/// </summary>
			bool CheckShouldDeleteNode() {

				// Set a roof of 64 queries it can be empty for before deletion
				if (m_LifeCount < 64)
					m_LifeCount++;

				return (m_LifeCount > m_LifeMax);
			}

			/// <summary>
			/// This will actually insert the data into the Octree Data Storage
			/// when we have determined if something fits inside this node.
			/// </summary>
			void InsertDataToNode(OctreeData data) {

				if (m_LifeCount > 0 || TotalCount() == 0) {
					m_LifeCount = 0;
					m_LifeMax = glm::min(m_LifeMax * 2, 64);
				}

				// 1. Determine the insert location for the data source.
				size_t insert_position = m_DataSourceIndex + m_DataSourceSize;

				// 2. Insert the new data source into the specified location, the global vector
				// has already checked if there is enough room to insert, and allocated sufficient
				// so storage we are safe to raw dog the insertion.
				m_Octree->m_DataSources.insert(m_Octree->m_DataSources.begin() + insert_position, data);

				// 3. Increment the current nodes data size by 1 because we successfully placed 
				// this data in this node's memory location!
				m_DataSourceSize++;
				m_TotalNodeDataSourceSize++;
			}

			/// <summary>
			/// This function will calculate the bounding regions for 8 children.
			/// </summary>
			/// <returns></returns>
			std::array<Bounds_AABB, 8> CalculateChildBounds() const {
				std::array<Bounds_AABB, 8> childBounds{};

				// Calculate the size of each child node's bounding box (half the size of the current node)
				glm::vec3 halfSize = m_NodeBounds.Size() * 0.5f;

				// Calculate the center of the current node
				glm::vec3 center = m_NodeBounds.Center();

				// Generate the eight child bounding boxes
				for (int i = 0; i < 8; ++i) {
					glm::vec3 offset(
						(i & 1 ? 0.5f : -0.5f) * halfSize.x,
						(i & 2 ? 0.5f : -0.5f) * halfSize.y,
						(i & 4 ? 0.5f : -0.5f) * halfSize.z
					);

					glm::vec3 childCenter = center + offset;

					childBounds[i].BoundsMin = childCenter - halfSize * 0.5f;
					childBounds[i].BoundsMax = childCenter + halfSize * 0.5f;
				}

				return childBounds;
			}

			/// <summary>
			/// This is the index of the start of the DataSourceIndex in 
			/// the Octree Data storage.
			/// </summary>
			size_t m_DataSourceIndex = 0;

			/// <summary>
			/// This is the size of how many DataSources are contained in 
			/// this Node at the DataSourceIndex in the Octree Data Storage.
			/// </summary>
			size_t m_DataSourceSize = 0;

			/// <summary>
			/// The current size of self including all child data sources.
			/// </summary>
			size_t m_TotalNodeDataSourceSize = 0;

			/// <summary>
			/// Array of pointers to 8 (oct) child nodes of this node.
			/// </summary>
			std::array<OctreeNode, 8> m_ChildrenNodes{ nullptr };

			/// <summary>
			/// Array of 8 (oct) child nodes bounding regions for this node.
			/// </summary>
			std::array<Bounds_AABB, 8> m_ChildrenBounds{};

			/// <summary>
			/// This tells us if this node has been split into child nodes or not.
			/// </summary>
			bool m_IsNodeSplit = false;

			/// <summary>
			/// The actual bounds of the node unaffected by the looseness.
			/// 
			/// Please note, this is not varied when looseness changes. This 
			/// will always be the actual bounds of the current node.
			/// 
			/// The looseness will ONLY be taken into account when attempting 
			/// to insert a Data Source, or querying a particular node.
			/// </summary>
			Bounds_AABB m_NodeBounds;

			/// <summary>
			/// This is the mat4 in worldspace of this current node. This is
			/// useful for when we want to draw the debug version of the octree
			/// and to save on performance, we calculate this once when the 
			/// node is created, opposed to every frame.
			/// </summary>
			glm::mat4 m_NodeMat4;

			/// <summary>
			/// Pointer to the Octree holding class that contains the root node.
			/// </summary>
			OctreeBounds<DataType>* m_Octree;

			/// <summary>
			/// This is the max amount of times a node can be queried whilst itself 
			/// and its children have no data sources.
			/// </summary>
			uint8_t m_LifeMax;

			/// <summary>
			/// This is the death counter, it will start at 0 and work its way up to 
			/// Life Max each time this node is queried. If it reaches LifeMax it 
			/// will delete the current node and all its children. If the node is 
			/// saved and something is placed in it, then the LifeMax of the node 
			/// is doubled.
			/// </summary>
			uint8_t m_LifeCount = 0;
		};
		using OctreeNode = std::shared_ptr<OctreeBoundsNode<DataType>>;

	public:

		using OctreeData = std::shared_ptr<OctreeDataSource<DataType>>;

	public:

		OctreeBounds() {
			BuildOctree({});
		}

		OctreeBounds(const OctreeBoundsConfig& config) : m_Config(config) {
			BuildOctree({});
		};

		OctreeBounds(const OctreeBoundsConfig& config, std::vector<OctreeData> data_sources) : m_Config(config) {
			BuildOctree(data_sources);
		};

		~OctreeBounds() {
			m_DataSources.clear();
			m_RootNode.reset();
			m_RootNode = nullptr;
		};
		
		OctreeBounds(const OctreeBounds& other) = default;
		OctreeBounds& operator=(const OctreeBounds& other) = default;

		OctreeBounds(OctreeBounds&& other) = default;
		OctreeBounds& operator=(OctreeBounds&& other) = default;

		/// <summary>
		/// This will attempt to insert a data source which has  been
		/// created into a shared_ptr already.
		/// </summary>
		bool Insert(OctreeData data_source) {

			if (!m_RootNode) {
				L_CORE_ERROR("Octree - Root Node Invalid or Octree Not Built - Cannot Insert Data Source.");
				return false;
			}

			int octree_growth_attempts = 0;
			const int max_attempts = 20; // We are expanding to infinity if we even get close to this number....
			std::array<Bounds_AABB, max_attempts> parent_node_bounding_region{};

			OctreeNode old_root_node_for_if_fail = m_RootNode;
			while (!m_RootNode->Insert(data_source, 0, false)) {

				if (octree_growth_attempts < max_attempts) {
					GrowOctree(data_source->Bounds.Center() - m_RootNode->GetNodeBounds().Center());
				}
				else {
					L_CORE_WARN("Octree - Could Not Grow the Octree to Fit Data Source.");
					m_RootNode = old_root_node_for_if_fail;
					return false;
				}

				octree_growth_attempts++;
			}

			// Revalidate the entire tree as we have successfully inserted the data source, this
			// is because when we insert something, all following node data source index start
			// positions will need to be shifted by 1
			m_RootNode->RevalidateNodes(0);

			return true;
		}

		/// <summary>
		/// This will attempt to insert a data source which has not been
		/// created into a shared_ptr yet.
		/// </summary>
		bool Insert(const DataType& data, const Bounds_AABB& bounds) {

			if (m_DataSources.size() + 1 >= m_DataSources.capacity())
				m_DataSources.reserve(m_DataSources.capacity() * 2);

			auto data_source = std::make_shared<OctreeDataSource<DataType>>(data, bounds);

			if (!Insert(data_source)) {
				data_source.reset();
				data_source = nullptr;
				return false;
			}
			return true;
		}

		/// <summary>
		/// This will attempt to insert an entire vector of data sources 
		/// into the Octree that have not been made into shared_ptr's yet.
		/// </summary>
		/// <returns>
		/// If anything is returned, these are the data sources
		/// that could not be inserted into the Octree. Deal with these
		/// remaining data sources as you see fit.
		/// </returns>
		std::vector<OctreeDataSource<DataType>> InsertVector(const std::vector<OctreeDataSource<DataType>>& data_sources) {

			// These are data sources that are out of bounds
			std::vector<OctreeDataSource<DataType>> remaining_data_sources{};
			remaining_data_sources.reserve(data_sources.size());

			if (m_DataSources.size() + data_sources.size() >= m_DataSources.capacity())
				m_DataSources.reserve(m_DataSources.capacity() * 2);

			for (const auto& data : data_sources) {
				auto data_shared = std::make_shared<OctreeDataSource<DataType>>(data);
				if (!Insert(data_shared)) {
					remaining_data_sources.push_back(data);
					data_shared.reset();
					data_shared = nullptr;
				}
			}

			return remaining_data_sources;
		}

		/// <summary>
		/// This will attempt to insert an entire vector of data source 
		/// to the Octree.
		/// </summary>
		/// <returns>
		/// If anything is returned, these are the data sources
		/// that could not be inserted into the Octree. Deal with these
		/// remaining data sources as you see fit.
		/// </returns>
		std::vector<OctreeData> InsertVector(const std::vector<OctreeData>& data_sources) {

			// These are data sources that are out of bounds
			std::vector<OctreeData> remaining_data_sources{};
			remaining_data_sources.reserve(data_sources.size());

			if (m_DataSources.size() + data_sources.size() >= m_DataSources.capacity())
				m_DataSources.reserve(m_DataSources.capacity() * 2);
			
			for (const auto& data : data_sources) {
				if(!Insert(data))
					remaining_data_sources.push_back(data);
			}

			return remaining_data_sources;
		}

		/// <summary>
		/// This will attempt to remove the data source from the Octree.
		/// </summary>
		bool Remove(const DataType& data) {

			if (!m_RootNode)
				return false;

			auto it = std::find_if(m_DataSources.begin(), m_DataSources.end(),
				[&data](const OctreeData& storedData) {
					return storedData->Data == data;
			});
			
			if (it == m_DataSources.end()) {
				L_CORE_WARN("Octree - Data Not Found in Data Sources.");
				return false;
			}

			// Remove the data from the octree node
			size_t position = std::distance(m_DataSources.begin(), it);
			
			if (!m_RootNode->Remove(position)) {
				L_CORE_ERROR("Octree - Data Not Actually Contained Within Octree.");
				return false;
			}

			m_RootNode->RevalidateNodes(0);
			return true;
		}

		/// <summary>
		/// This will update a data source within the current Octree. It will first
		/// remove it from the Octree, then Insert into the Octree.
		/// 
		/// If the data source is not within the current Octree, it will just insert
		/// it into the Octree.
		/// </summary>
		bool Update(const DataType& data, const Bounds_AABB& bounds) {

			if (!m_RootNode)
				return false;

			Remove(data);
				
			return Insert(data, bounds);
		}

		/// <summary>
		/// This will query the octree and return a const reference to a static vector.
		/// 
		/// If you do not process or copy the data from this result before the next query, 
		/// the next query will overwrite all data returned from the first query call.
		/// 
		/// Make sure that you copy the results, or you process them before querying the 
		/// Octree again.
		/// </summary>
		/// <param name="frustum">Bounds AABB Range to Query</param>
		const std::vector<OctreeData>& Query(const Bounds_AABB& bounds) {

			if (m_QueryReturnVectors[0].capacity() == 0 || m_QueryReturnVectors[0].capacity() < m_DataSources.size()) {
				m_QueryReturnVectors[0].reserve(m_DataSources.empty() ? 1024 : m_DataSources.size());
			}

			m_QueryReturnVectors[0].clear();

			if (m_DataSources.empty())
				return m_QueryReturnVectors[0];

			if (m_RootNode) {
				bool should_delete = false;
				m_RootNode->Query(bounds, m_QueryReturnVectors[0], should_delete);
			}
			return m_QueryReturnVectors[0];
		}

		/// <summary>
		/// This will query the octree and return a const reference to a static vector.
		/// 
		/// If you do not process or copy the data from this result before the next query, 
		/// the next query will overwrite all data returned from the first query call.
		/// 
		/// Make sure that you copy the results, or you process them before querying the 
		/// Octree again.
		/// </summary>
		/// <param name="frustum">Bounds Sphere Range to Query</param>
		const std::vector<OctreeData>& Query(const Bounds_Sphere& bounds) {

			if (m_QueryReturnVectors[1].capacity() == 0 || m_QueryReturnVectors[1].capacity() < m_DataSources.size()) {
				m_QueryReturnVectors[1].reserve(m_DataSources.empty() ? 1024 : m_DataSources.size());
			}

			m_QueryReturnVectors[1].clear();

			if (m_DataSources.empty())
				return m_QueryReturnVectors[1];

			if (m_RootNode) {
				bool should_delete = false;
				m_RootNode->Query(bounds, m_QueryReturnVectors[1], should_delete);
			}
			return m_QueryReturnVectors[1];
		}

		/// <summary>
		/// This will query the octree and return a const reference to a static vector.
		/// 
		/// If you do not process or copy the data from this result before the next query, 
		/// the next query will overwrite all data returned from the first query call.
		/// 
		/// Make sure that you copy the results, or you process them before querying the 
		/// Octree again.
		/// </summary>
		/// <param name="frustum">Camera Frustum to Query</param>
		const std::vector<OctreeData>& Query(const Frustum& frustum) {

			if (m_QueryReturnVectors[2].capacity() == 0 || m_QueryReturnVectors[2].capacity() < m_DataSources.size()) {
				m_QueryReturnVectors[2].reserve(m_DataSources.empty() ? 1024 : m_DataSources.size());
			}

			m_QueryReturnVectors[2].clear();

			if (m_DataSources.empty())
				return m_QueryReturnVectors[2];

			if (m_RootNode) {
				bool should_delete = false;
				m_RootNode->Query(frustum, m_QueryReturnVectors[2], should_delete);
			}
			return m_QueryReturnVectors[2];
		}

		/// <summary>
		/// This will rebuild the octree with all its current data sources.
		/// </summary>
		void RebuildOctree() {
			std::unique_lock lock(m_OctreeMutex);
			BuildOctree();
		}

		/// <summary>
		/// This will rebuild the octree with all its current data sources, 
		/// but with a new configuration for the Octree.
		/// </summary>
		void RebuildOctree(const OctreeBoundsConfig& new_config) {
			std::unique_lock lock(m_OctreeMutex);
			m_Config = new_config;
			BuildOctree();
		}

		/// <summary>
		/// This will return the data sources contained ONLY in the root node.
		/// </summary>
		/// <returns></returns>
		size_t Count() const { 
			return m_RootNode ? m_RootNode->Count() : 0; 
		}

		/// <summary>
		/// This will return the 'lazy' approach count which tracks the
		/// total count of its data sources and sub nodes data sources.
		/// </summary>
		size_t TotalCount() const {
			return m_RootNode ? m_RootNode->TotalCount() : 0;
		}

		/// <summary>
		/// This will force a manual recursive count of all the Octree's node
		/// data sources to ensure accuracy.
		/// </summary>
		size_t ForceTotalCount() const {
			return m_RootNode ? m_RootNode->ForceTotalCount() : 0;
		}

		void Clear() { 
			if (m_RootNode) 
				m_RootNode->Clear(); 
		}

		bool IsEmpty() const { 
			return TotalCount() == 0; 
		}

		OctreeNode GetRootNode() const { 
			return m_RootNode; 
		}

		/// <summary>
		/// Calling this function will try to shrink the Root Node if there is 
		/// only one octant which contains data.
		/// </summary>
		void TryShrinkOctree() {
			if(auto node = ShrinkOctree(); node && m_RootNode != node) {
				m_RootNode = node;
				m_RootNode->RevalidateNodes(0);
			}
		}

		void SetConfig(const OctreeBoundsConfig& config) { m_Config = config; }
		const OctreeBoundsConfig& GetConfig() const { return m_Config; }

		/// <summary>
		/// Use this to lock the octree when doing any updates in a separate thread.
		/// </summary>
		std::mutex& GetOctreeMutex() { return m_OctreeMutex; }

		const std::vector<OctreeData>& GetAllOctreeDataSources() const {
			return m_DataSources;
		}

		/// <summary>
		/// This will return a vector of AABB bounds of the entire Octree's node's recursively.
		/// </summary>
		/// <returns></returns>
		std::vector<Bounds_AABB> GetAllOctreeBounds() const {
			return m_RootNode ? m_RootNode->GetRecursiveChildBounds() : std::vector<Bounds_AABB>();
		}

		/// <summary>
		/// This will return a vector of glm::mat4 matricies of the entire Octree's node bounds recursively.
		/// </summary>
		std::vector<glm::mat4> GetAllOctreeBoundsMat4() const {
			return m_RootNode ? m_RootNode->GetRecursiveChildBoundsMat4() : std::vector<glm::mat4>();
		}

	private:

		/// <summary>
		/// This will build the octree provided the data in the constructor.
		/// </summary>
		/// 
		/// <param name="data_sources">
		/// This will be the new octree's data. This 
		/// function will not add this data to previous data, it will start from 
		/// scratch with the new data provided
		/// </param>
		void BuildOctree(std::vector<OctreeData> data_sources = {}) {

			// 1. Delete old Octree
			if (m_RootNode) {

				if (data_sources.empty() && !m_DataSources.empty())
					data_sources = m_DataSources;

				m_RootNode.reset();
				m_RootNode = nullptr;
			}

			m_DataSources.clear();

			// 2. Calculate Initial Bounds of New Octree
			if (!data_sources.empty()) {

				m_Config.InitialBounds.BoundsMin = glm::vec3(FLT_MAX);
				m_Config.InitialBounds.BoundsMax = glm::vec3(-FLT_MAX);

				for (const auto& data : data_sources) {
					if (data) {
						m_Config.InitialBounds.BoundsMin = glm::min(data->Bounds.BoundsMin, m_Config.InitialBounds.BoundsMin);
						m_Config.InitialBounds.BoundsMax = glm::max(data->Bounds.BoundsMax, m_Config.InitialBounds.BoundsMax);
					}
				}

				// Step 1: Calculate the initial center and extent
				glm::vec3 center = m_Config.InitialBounds.Center();
				glm::vec3 extent = m_Config.InitialBounds.BoundsMax - m_Config.InitialBounds.BoundsMin;

				// Step 2: Find the largest extent to ensure the bounding box is a cube
				float maxExtent = glm::compMax(extent);

				// Step 3: Create a cubic bounding box centered on the original center
				glm::vec3 halfExtent = glm::vec3(maxExtent) * 0.5f;

				// Step 4: Adjust the bounds to make them uniform and cubic
				m_Config.InitialBounds.BoundsMin = center - halfExtent;
				m_Config.InitialBounds.BoundsMax = center + halfExtent;

				// Optional: Apply a scaling factor to add some padding around the entire scene
				float scaleFactor = 1.1f;
				m_Config.InitialBounds.BoundsMin *= scaleFactor;
				m_Config.InitialBounds.BoundsMax *= scaleFactor;

			}
			else {
				m_Config.InitialBounds.BoundsMin = glm::vec3(-1000.0f);
				m_Config.InitialBounds.BoundsMax = glm::vec3(1000.0f);
			}

			// 3. Pre-allocate the global data vector
			size_t dataSourceCount = data_sources.size();
			size_t initialCapacity = static_cast<size_t>(std::max(1024.0, std::pow(2.0, std::ceil(std::log2(dataSourceCount)))));
			m_DataSources.reserve(initialCapacity);

			// 4. Create Root Octree Node
			m_RootNode = std::make_shared<OctreeBoundsNode<DataType>>(m_Config.InitialBounds, this);

			if (!data_sources.empty())
				InsertVector(data_sources);
		}

		/// <summary>
		/// This will attempt to grow the octree
		/// </summary>
		/// <param name="direction">Direction from centre of current root node you want to grow to.</param>
		bool GrowOctree(const glm::vec3& direction) {

			// 1. Determine the direction in each axis.
			int xDirection = direction.x >= 0 ? 1 : -1;
			int yDirection = direction.y >= 0 ? 1 : -1;
			int zDirection = direction.z >= 0 ? 1 : -1;

			// 2. Calculate the half size of the current root node.
			glm::vec3 halfSize = m_RootNode->GetNodeBounds().Size() * 0.5f;

			// 3. Calculate the new center for the expanded root node.
			glm::vec3 newCenter = m_RootNode->GetNodeBounds().Center() + glm::vec3(
				xDirection * halfSize.x,
				yDirection * halfSize.y,
				zDirection * halfSize.z
			);

			// 4. Calculate the new size (double the current size).
			glm::vec3 newHalfSize = halfSize * 2.0f;

			// 5. Create new bounding region based on the new center and new size.
			Bounds_AABB new_bounds;
			new_bounds.BoundsMin = newCenter - newHalfSize;
			new_bounds.BoundsMax = newCenter + newHalfSize;

			// 6. Store old root node and create new root node
			OctreeNode old_root_node = m_RootNode;
			OctreeNode new_root_node = std::make_shared<OctreeBoundsNode<DataType>>(new_bounds, this);

			// 7. Get the node index of the new root node which holds the old root node
			int child_index = new_root_node->BestFitChild(m_RootNode->GetNodeBounds().Center());
			if (child_index == -1) {
				L_CORE_ERROR("Octree - Could Not Determine Child Region for Old Root Node.");
				return false;
			}

			// 8. Set the root node to the new root node, and set the child at the appropriate index to the old root node
			if (new_root_node->SetChildNode(child_index, old_root_node)) {
				m_RootNode = new_root_node;
			}
			else {
				L_CORE_ERROR("Octree - Could Not Set Old Root Node as Child to New Root Node - Invalid Index.");
				return false;
			}
			return true;
		}

		/// <summary>
		/// This checks if the root node should be shrunk
		/// </summary>
		bool ShouldShrink() {

			if (!m_RootNode->IsNodeSplit())
				return false;

			if (m_RootNode->m_DataSourceSize > 0)
				return false;

			bool found_one_child_with_data = false;
			for (const auto& child : m_RootNode->m_ChildrenNodes) {

				if (!child)
					continue;

				if (child->TotalCount() == 0)
					continue;

				if (found_one_child_with_data)
					return false;

				found_one_child_with_data = true;
			}

			return true;
		}

		/// <summary>
		/// This will return the most appropriate root node
		/// </summary>
		OctreeNode ShrinkOctree() {

			if (!m_RootNode)
				return nullptr;

			if (!ShouldShrink())
				return m_RootNode;

			for (int i = 0; i < m_RootNode->m_ChildrenNodes.size(); i++) {
				if (m_RootNode->m_ChildrenNodes[i] && m_RootNode->m_ChildrenNodes[i]->TotalCount() > 0)
					return m_RootNode->m_ChildrenNodes[i];
			}

			return m_RootNode;
		}

		OctreeBoundsConfig m_Config{};

		OctreeNode m_RootNode;

		std::vector<OctreeData> m_DataSources{};

		std::array<std::vector<OctreeData>, 3> m_QueryReturnVectors{};

		std::mutex m_OctreeMutex{};

	};
}