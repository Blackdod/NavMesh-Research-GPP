#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\Movement\SteeringBehaviors\SteeringAgent.h"

using namespace Elite;
// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors(0)
	,m_NeighborhoodCells(rows * cols)
{
	m_CellWidth = m_SpaceWidth / m_NrOfCols;
	m_CellHeight = m_SpaceHeight / m_NrOfRows;

	for (int col{}; col < m_NrOfCols; ++col)
	{
		for (int row{}; row < m_NrOfRows; ++row)
		{
			m_Cells.push_back(Cell{col * m_CellWidth, row * m_CellHeight, m_CellWidth, m_CellHeight});
		}
	}
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	m_Cells[PositionToIndex(agent->GetPosition())].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, Elite::Vector2 oldPos)
{
	const int oldIdx = PositionToIndex(oldPos);
	const int newIdx = PositionToIndex(agent->GetPosition());

	if (oldIdx == newIdx)
	{
		return;
	}

	m_Cells[oldIdx].agents.remove(agent);
	m_Cells[newIdx].agents.push_back(agent);
}

void CellSpace::RegisterNeighbors(SteeringAgent* agent, float queryRadius)
{
	const Elite::Vector2 rectBottomLeft{ agent->GetPosition().x - queryRadius, agent->GetPosition().y - queryRadius };
	const Elite::Vector2 rectTopRight{ agent->GetPosition().x + queryRadius, agent->GetPosition().y + queryRadius };
	const float rectWidth{ queryRadius * 2 };
	const float rectHeight{ queryRadius * 2 };
	const Elite::Rect queryRect{ rectBottomLeft, rectWidth, rectHeight };

	const int minIdxCol{ Elite::Clamp(int(rectBottomLeft.x / m_CellWidth), 0, m_NrOfCols)  };
	const int minIdxRow{ Elite::Clamp(int(rectBottomLeft.y / m_CellHeight), 0, m_NrOfRows) };
	const int maxIdxCol{ int(rectTopRight.x / m_CellWidth) % m_NrOfCols + 1};
	const int maxIdxRow{ int(rectTopRight.y / m_CellHeight) % m_NrOfRows + 1};
	const int amountOfHorizontalIdx{int( rectWidth / m_CellWidth) + 1};
	const int amountOfVerticalIdx{ int(rectHeight / m_CellHeight) + 1};

	int neighborhoodIdx{ 0 };

	m_NrOfNeighbors = 0;

	for (int col{}; col < amountOfHorizontalIdx; ++col)
	{
		for (int row{}; row < amountOfVerticalIdx; ++row)
		{
			const int worldIdx{ (((minIdxCol + col) % m_NrOfCols) * m_NrOfRows + (minIdxRow + row) % m_NrOfRows) };
			m_NeighborhoodCells[neighborhoodIdx] = &m_Cells[worldIdx];

			for(const auto& pAgent : m_NeighborhoodCells[neighborhoodIdx]->agents)
			{
				if (pAgent != agent)
				{
					if (queryRadius * queryRadius >= DistanceSquared(agent->GetPosition(), pAgent->GetPosition()))
					{

						m_Neighbors[m_NrOfNeighbors] = pAgent;
						++m_NrOfNeighbors;
					}
				}
			}
			
			++neighborhoodIdx;
		}
	}
}

void CellSpace::EmptyCells()
{
	for (Cell& c : m_Cells)
		c.agents.clear();
}

void CellSpace::RenderCells() const
{
	for (const Cell& pCell : m_Cells)
	{
		Elite::Polygon cellPoints{ pCell.GetRectPoints()};
		DEBUGRENDERER2D->DrawPolygon(&cellPoints, {1,0,0}, 0.4f);

		int amountOfAgents{ static_cast<int>(pCell.agents.size()) };
		std::string text = std::to_string(amountOfAgents);

		const Elite::Vector2 textPos = Vector2{ pCell.boundingBox.bottomLeft.x + pCell.boundingBox.width / 2, pCell.boundingBox.bottomLeft.y + pCell.boundingBox.height / 2};

		DEBUGRENDERER2D->DrawString(textPos, text.c_str());
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	int col = static_cast<int>(pos.x / m_CellWidth);
	int row = static_cast<int>(pos.y / m_CellHeight);

	col = Elite::Clamp(col, 0, m_NrOfCols - 1);
	row = Elite::Clamp(row, 0, m_NrOfRows - 1);

	const int returnIndex = col * m_NrOfRows + row;
	return returnIndex;
}