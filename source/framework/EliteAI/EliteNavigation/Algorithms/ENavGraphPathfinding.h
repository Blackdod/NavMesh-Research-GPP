#pragma once
#include <vector>
#include <iostream>
#include "framework/EliteMath/EMath.h"
#include "framework\EliteAI\EliteGraphs\ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

namespace Elite
{
	class NavMeshPathfinding
	{
	public:
		static std::vector<Vector2> FindPath(Vector2 startPos, Vector2 endPos, NavGraph* pNavGraph, std::vector<Vector2>& debugNodePositions, std::vector<Portal>& debugPortals)
		{
			//Create the path to return
			std::vector<Vector2> finalPath{};

			//Get the start and endTriangle

			const Triangle* startTriangle =  pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos);
			const Triangle* endTriangle =  pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos);
		
			//We have valid start/end triangles and they are not the same
			if(startTriangle == nullptr || endTriangle == nullptr)
			{
				return finalPath;
			}
			if(startTriangle == endTriangle)
			{
				return finalPath;
			}
			//=> Start looking for a path
			//Copy the graph

			auto tempGraph = pNavGraph;
			
			//Create extra node for the Start Node (Agent's position)

			const auto startNode = new NavGraphNode{ tempGraph->GetNextFreeNodeIndex(), -1, startPos };
			tempGraph->AddNode(startNode);

			for(const auto& lineIdx : startTriangle->metaData.IndexLines)
			{
				const int toIdx = tempGraph->GetNodeIdxFromLineIdx(lineIdx);
				if(toIdx >= 0)
				{
					const int lastIdx = (int)tempGraph->GetAllNodes().size() - 1;
					tempGraph->AddConnection(new GraphConnection2D{ lastIdx, toIdx, Distance(startPos, tempGraph->GetNodeWorldPos(toIdx)) });
				}
			}
			
			//Create extra node for the endNode

			const auto endNode = new NavGraphNode{ tempGraph->GetNextFreeNodeIndex(), endPos };
			tempGraph->AddNode(endNode);

			for (const auto& lineIdx : endTriangle->metaData.IndexLines)
			{
				const int toIdx = tempGraph->GetNodeIdxFromLineIdx(lineIdx);
				if (toIdx >= 0)
				{
					const int lastIdx = (int)tempGraph->GetAllNodes().size() - 1;
					tempGraph->AddConnection(new GraphConnection2D{ lastIdx, toIdx, Distance(endPos, tempGraph->GetNodeWorldPos(toIdx)) });
				}
			}

			//Run A star on new graph

			auto aStar = AStar<NavGraphNode, GraphConnection2D>(tempGraph, HeuristicFunctions::Manhattan);
			auto nodes = aStar.FindPath(startNode, endNode);

			for(const auto node : nodes)
			{
				finalPath.push_back(node->GetPosition());

				//OPTIONAL BUT ADVICED: Debug Visualisation
				debugNodePositions.push_back(node->GetPosition());
			}
			

			//Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
			auto m_Portals = SSFA::FindPortals(nodes, pNavGraph->GetNavMeshPolygon());
			finalPath = SSFA::OptimizePortals(m_Portals);

			return finalPath;
		}
	};
}
