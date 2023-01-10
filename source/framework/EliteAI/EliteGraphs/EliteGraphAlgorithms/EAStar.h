#pragma once
#include "framework/EliteAI/EliteNavigation/ENavigation.h"

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}

	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		std::vector<T_NodeType*> path{};
		std::vector<NodeRecord> openList{};
		std::vector<NodeRecord> closedList{};
		NodeRecord currentRecord{};

		NodeRecord startRecord{ pStartNode, nullptr, 0, GetHeuristicCost(pStartNode, pGoalNode) };
		openList.push_back(startRecord);

		while(openList.empty() == false)
		{
			currentRecord = *std::min_element(openList.begin(), openList.end());

			if(currentRecord.pNode == pGoalNode)
			{
				break;
			}

			for (const auto& pConnection : m_pGraph->GetNodeConnections(currentRecord.pNode))
			{
				T_NodeType* pNextNode = m_pGraph->GetNode(pConnection->GetTo());
				const float costSoFar{ currentRecord.costSoFar + pConnection->GetCost()};

				bool isCheaper{ true };

				//Remove connections that lead to previously checked nodes if the new connection is cheaper
				for(const auto& closedRecord : closedList)
				{
					if (pConnection->GetTo() == closedRecord.pNode->GetIndex())
					{
						if(closedRecord.pConnection != nullptr && closedRecord.costSoFar > costSoFar)
						{
							closedList.erase(std::remove(closedList.begin(), closedList.end(), closedRecord));
							//std::cout << closedRecord.costSoFar << " > " << costSoFar << '\n';
						}
						else
						{
							isCheaper = false;
						}
						
					}
				}

				//Same as above but for openList?
				for (const auto& openRecord : openList)
				{
					if (pConnection->GetTo() == openRecord.pNode->GetIndex())
					{
						if (openRecord.pConnection != nullptr && openRecord.costSoFar > costSoFar)
						{
							openList.erase(std::remove(openList.begin(), openList.end(), openRecord));
						}
						else
						{
							isCheaper = false;
						}
					}
				}

				if(isCheaper)
				{				
					NodeRecord newNodeRecord{ pNextNode, pConnection, costSoFar, costSoFar + GetHeuristicCost(pNextNode, pGoalNode) };
					openList.push_back(newNodeRecord);
				}
			}

			openList.erase(std::remove(openList.begin(), openList.end(), currentRecord));
			closedList.push_back(currentRecord);
		}

		if (currentRecord.pNode == pGoalNode)
		{
			while (currentRecord.pNode != pStartNode)
			{
				path.push_back(currentRecord.pNode);

				const int searchedIdx = currentRecord.pConnection->GetFrom();

				for (const auto& closedRecord : closedList)
				{
					if (closedRecord.pNode->GetIndex() == searchedIdx)
					{
						currentRecord = closedRecord;
						break;
					}
				}
			}
			path.push_back(pStartNode);
			std::reverse(path.begin(), path.end());
		}

		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}