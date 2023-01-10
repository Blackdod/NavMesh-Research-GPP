#include "stdafx.h"
#include "ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr),
	m_ContourMesh(contourMesh)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon; 
	m_pNavMeshPolygon = nullptr;
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void Elite::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigationmesh and create nodes
	std::vector<NavGraphNode*> newNodes{};

	for(const auto& edge : m_pNavMeshPolygon->GetLines())
	{
		if(m_pNavMeshPolygon->GetTrianglesFromLineIndex(edge->index).size() > 1)
		{
			const Vector2 edgeCenter = Vector2{ (edge->p1.x + edge->p2.x) / 2, (edge->p1.y + edge->p2.y) / 2 };
			NavGraphNode* newNode = new NavGraphNode{ this->GetNextFreeNodeIndex(),edge->index, edgeCenter };
			this->AddNode(newNode);
		}
	}
	
	//2. Create connections now that every node is created


	std::vector<int> validIdx{};
	for(const auto& triangle:m_pNavMeshPolygon->GetTriangles())
	{
		for(const auto& lineIdx:triangle->metaData.IndexLines)
		{
			if(GetNodeIdxFromLineIdx(lineIdx) >= 0)
			{
				validIdx.push_back(GetNodeIdxFromLineIdx(lineIdx));
			}
		}

		if (validIdx.size() == 2)
		{
			if (this->GetConnection(validIdx[0], validIdx[1]) == nullptr)
			{
				this->AddConnection(new GraphConnection2D(validIdx[0], validIdx[1]));
			}
		}
		else if(validIdx.size() == 3)
		{
			if (this->GetConnection(validIdx[0], validIdx[1]) == nullptr && this->GetNode(validIdx[0]) != this->GetNode(validIdx[1]))
			{
				this->AddConnection(new GraphConnection2D(validIdx[0], validIdx[1]));
			}
			
			if (this->GetConnection(validIdx[1], validIdx[2]) == nullptr && this->GetNode(validIdx[1]) != this->GetNode(validIdx[2]))
			{
				this->AddConnection(new GraphConnection2D(validIdx[1], validIdx[2]));
			}
			
			if (this->GetConnection(validIdx[2], validIdx[0]) == nullptr && this->GetNode(validIdx[2]) != this->GetNode(validIdx[0]))
			{
				this->AddConnection(new GraphConnection2D(validIdx[2], validIdx[0]));
			}
		}
		validIdx.clear();
	}
	
	//3. Set the connections cost to the actual distance

	SetConnectionCostsToDistance();
}

void Elite::NavGraph::UpdateGraph(float playerRadius, Elite::Polygon& wall)
{
	delete m_pNavMeshPolygon;
	m_pNavMeshPolygon = nullptr;
	
	this->Clear();

	m_pNavMeshPolygon = new Polygon(m_ContourMesh);
	//We don't make a copy of the wall since we don't actually change the original, the polygon isn't the navmesh collider

	m_Walls.push_back(wall);

	for (auto currentWall : m_Walls)
	{
		currentWall.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(currentWall);
	}


	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}