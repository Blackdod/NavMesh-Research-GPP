#pragma once

#include "framework/EliteAI/EliteGraphs/EGraph2D.h"
#include "framework/EliteAI/EliteGraphs/EGraphConnectionTypes.h"
#include "framework/EliteAI/EliteGraphs/EGraphNodeTypes.h"

namespace Elite
{
	class NavGraph final: public Graph2D<NavGraphNode, GraphConnection2D>
	{
	public:
		NavGraph(const Polygon& baseMesh, float playerRadius );
		~NavGraph();

		int GetNodeIdxFromLineIdx(int lineIdx) const;
		Polygon* GetNavMeshPolygon() const;

		//Update graph
		void UpdateGraph( float playerRadius, Elite::Polygon& wall);

	private:
		//--- Datamembers ---
		Polygon* m_pNavMeshPolygon = nullptr; //Polygon that represents navigation mesh

		void CreateNavigationGraph();

		//Update graph
		const Polygon m_ContourMesh{};
		std::vector<Elite::Polygon> m_Walls{};

	private:
		NavGraph(const NavGraph& other) = delete;
		NavGraph& operator=(const NavGraph& other) = delete;
		NavGraph(NavGraph&& other) = delete;
		NavGraph& operator=(NavGraph&& other) = delete;
		
	};
}

