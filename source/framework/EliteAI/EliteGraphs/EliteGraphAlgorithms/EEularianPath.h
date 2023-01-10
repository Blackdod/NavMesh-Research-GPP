#pragma once
#include <stack>

namespace Elite
{
	enum class Eulerianity
	{
		notEulerian,
		semiEulerian,
		eulerian,
	};

	template <class T_NodeType, class T_ConnectionType>
	class EulerianPath
	{
	public:

		EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph);

		Eulerianity IsEulerian() const;
		std::vector<T_NodeType*> FindPath(Eulerianity& eulerianity) const;

	private:
		void VisitAllNodesDFS(int startIdx, std::vector<bool>& visited) const;
		bool IsConnected() const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
	};

	template<class T_NodeType, class T_ConnectionType>
	inline EulerianPath<T_NodeType, T_ConnectionType>::EulerianPath(IGraph<T_NodeType, T_ConnectionType>* pGraph)
		: m_pGraph(pGraph)
	{
	}

	template<class T_NodeType, class T_ConnectionType>
	inline Eulerianity EulerianPath<T_NodeType, T_ConnectionType>::IsEulerian() const
	{

		// If the graph is not connected, there can be no Eulerian Trail
		if(IsConnected() == false)
		{
			return Eulerianity::notEulerian;
		}

		// Count nodes with odd degree 
		const auto nodes = m_pGraph->GetAllNodes();
		int oddCount = 0;
		for(const auto& n : nodes)
		{
			const auto connections = m_pGraph->GetNodeConnections(n);
			if(connections.size() & 1) //bitwise 1, check if odd. 0101 & 0001 == true
			{
				++oddCount;
			}
		}


		// A connected graph with more than 2 nodes with an odd degree (an odd amount of connections) is not Eulerian
		if(oddCount > 2)
		{
			return Eulerianity::notEulerian;
		}

		// A connected graph with exactly 2 nodes with an odd degree is Semi-Eulerian (unless there are only 2 nodes)
		// An Euler trail can be made, but only starting and ending in these 2 nodes
		if(oddCount == 2 && nodes.size() != 2)
		{
			return Eulerianity::semiEulerian;
		}

		// A connected graph with no odd nodes is Eulerian
		return  Eulerianity::eulerian;
	}

	template<class T_NodeType, class T_ConnectionType>
	inline std::vector<T_NodeType*> EulerianPath<T_NodeType, T_ConnectionType>::FindPath(Eulerianity& eulerianity) const
	{
		// Get a copy of the graph because this algorithm involves removing edges
		auto graphCopy = m_pGraph->Clone();
		auto path = std::vector<T_NodeType*>();
		int nrOfNodes = graphCopy->GetNrOfNodes();

		// Check if there can be an Euler path
		// If this graph is not eulerian, return the empty path
		if(eulerianity == Eulerianity::notEulerian)
		{
			std::cout << "No path found\n";
			for (int i{}; i < nrOfNodes; ++i)
			{
				m_pGraph->GetNode(i)->SetColor({0,0,1});
			}
			return {};
		}
		// Else we need to find a valid starting index for the algorithm

		T_NodeType* startingNode{};
		if(eulerianity == Eulerianity::semiEulerian) //2 odd nodes
		{
			for(const auto& n : graphCopy->GetAllNodes())
			{
				const auto connections = graphCopy->GetNodeConnections(n);
				if (connections.size() & 1) //bitwise 1, check if odd. 0101 & 0001 == true
				{
					startingNode = n;
					break;
				}
			}
			//break jumps here
		}
		else
		{
			startingNode = graphCopy->GetNode(0);
		}
		
		// Start algorithm loop
		std::stack<T_NodeType*> nodeStack;
		T_NodeType* currentNode{ startingNode };
		const Color baseColor{ 0,1,0 }; //start at green
		const Color colorChange{ 1, -1, 0 }; //go to red {0,1,0} + {1,-1,0} = {1,0,0}

		while(graphCopy->GetAllConnections().size() != 0 || nodeStack.empty() == false)
		{
			auto connections = graphCopy->GetNodeConnections(currentNode);
			if(connections.size() > 0)
			{
				nodeStack.push(currentNode);

				currentNode = graphCopy->GetNode(connections.back()->GetTo());

				graphCopy->RemoveConnection(connections.back());
			}
			else
			{
				if(currentNode != nullptr)
				{
					path.push_back(m_pGraph->GetNode(currentNode->GetIndex()));

					if (nodeStack.size() > 0)
					{
						currentNode = nodeStack.top();
						nodeStack.pop();
					}
					else
					{
						break;
					}
				}
			}
		}

		std::reverse(path.begin(), path.end()); // reverses order of the path

		std::string printString{};
		for(size_t i{}; i < path.size(); ++i)
		{
			if(i == 0)
			{
				printString += std::to_string(path[i]->GetIndex());
			}
			else
			{
				printString += " -> " + std::to_string(path[i]->GetIndex());
			}

			if(eulerianity == Eulerianity::eulerian)
			{
				m_pGraph->GetNode(path[i]->GetIndex())->SetColor(DEFAULT_NODE_COLOR);
			}
			else
			{
				const float colorScalair{ i / float(nrOfNodes) };
				const Color currentColor = { baseColor.r + colorChange.r * colorScalair, baseColor.g + colorChange.g * colorScalair, baseColor.b + colorChange.b * colorScalair };
				m_pGraph->GetNode(path[i]->GetIndex())->SetColor(currentColor);
			}
		}
		std::cout << printString << '\n';

		return path;
	}

	template<class T_NodeType, class T_ConnectionType>
	inline void EulerianPath<T_NodeType, T_ConnectionType>::VisitAllNodesDFS(int startIdx, std::vector<bool>& visited) const
	{
		// mark the visited node
		visited[startIdx] = true;

		// recursively visit any valid connected nodes that were not visited before
		for(const T_ConnectionType* connection : m_pGraph->GetNodeConnections(startIdx))
		{
			if(visited[connection->GetTo()] == false)
			{
				VisitAllNodesDFS(connection->GetTo(), visited);
			}
		}
	}

	template<class T_NodeType, class T_ConnectionType>
	inline bool EulerianPath<T_NodeType, T_ConnectionType>::IsConnected() const
	{
		const auto nodes = m_pGraph->GetAllNodes();
		vector<bool> visited(m_pGraph->GetNrOfNodes(), false);

		// find a valid starting node that has connections
		// depth first search, go as deep as possible
		int connextedIdx = invalid_node_index;

		if (nodes.size() > 1 && m_pGraph->GetAllConnections().size() == 0)
		{
			return false;
		}

		for (const auto& n : nodes)
		{
			const auto connections = m_pGraph->GetNodeConnections(n);
			if(connections.size() != 0)
			{
				connextedIdx = n->GetIndex();
				break;
			}
		}
		
		// if no valid node could be found, return false
		if(connextedIdx == invalid_node_index)
		{
			return false;
		}

		// start a depth-first-search traversal from the node that has at least one connection
		VisitAllNodesDFS(connextedIdx, visited);

		// if a node was never visited, this graph is not connected
		for (const auto& n:nodes)
		{
			if(visited[n->GetIndex()] == false)
			{
				return false;
			}
		}

		return true;
	}

}