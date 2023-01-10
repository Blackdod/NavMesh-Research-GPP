#include "stdafx.h"
#include "Flock.h"

#include "../SteeringAgent.h"
#include "../Steering/SteeringBehaviors.h"
#include "../CombinedSteering/CombinedSteeringBehaviors.h"

using namespace Elite;

//Constructor & Destructor
Flock::Flock(
	int flockSize /*= 50*/, 
	float worldSize /*= 100.f*/, 
	SteeringAgent* pAgentToEvade /*= nullptr*/, 
	bool trimWorld /*= false*/)

	: m_WorldSize{ worldSize }
	, m_FlockSize{ flockSize }
	, m_TrimWorld { trimWorld }
	, m_pAgentToEvade{pAgentToEvade}
	, m_NeighborhoodRadius{ 15.f }
	, m_NrOfNeighbors{0}
{
	// TODO: initialize the flock and the memory pool

	m_Agents.resize(m_FlockSize);

	m_Neighbors.resize(m_FlockSize);

	m_OldPositions.resize(m_FlockSize);
	
	m_pCohesionBehavior = new Cohesion(this);
	m_pSeparationBehavior = new Separation(this);
	m_pVelMatchBehavior = new VelocityMatch(this);
	m_pSeekBehavior = new Seek();
	m_pWanderBehavior = new Wander();
	m_pEvadeBehavior = new Evade();

	m_pBlendedSteering = new BlendedSteering({  {m_pSeekBehavior, 1.f} ,
															{m_pWanderBehavior, 1.f},
															{m_pSeparationBehavior, 0.1f},
															{m_pCohesionBehavior, 0.8f},
															{m_pVelMatchBehavior, 0.2f} });
	m_pPrioritySteering = new PrioritySteering({ m_pEvadeBehavior, m_pBlendedSteering });


	for (int idx{}; idx < m_Agents.size(); ++idx)
	{
		m_Agents[idx] = new SteeringAgent();
		m_Agents[idx]->SetPosition(Vector2{ randomFloat(0, worldSize), randomFloat(0, worldSize) });
		m_Agents[idx]->SetSteeringBehavior(m_pPrioritySteering);
		m_Agents[idx]->SetAutoOrient(true);
		m_Agents[idx]->SetMaxLinearSpeed(25.f);
		m_Agents[idx]->SetMass(1.f);

		m_Agents[idx]->SetLinearVelocity(Vector2{ randomFloat(-m_Agents[idx]->GetMaxLinearSpeed(), m_Agents[idx]->GetMaxLinearSpeed()), randomFloat(-m_Agents[idx]->GetMaxLinearSpeed(), m_Agents[idx]->GetMaxLinearSpeed()) });

		m_OldPositions[idx] = Vector2{ m_Agents[idx]->GetPosition() };
	}
	//m_Agents[m_FlockSize - 1]->SetRenderBehavior(true);
	
	//CellSpace

	m_pCellSpace = new CellSpace(m_WorldSize, m_WorldSize, 15, 15, m_FlockSize);

	for (const auto& pAgent : m_Agents)
	{
		m_pCellSpace->AddAgent(pAgent);
	}
}

Flock::~Flock()
{
	// TODO: clean up any additional data

	SAFE_DELETE(m_pBlendedSteering);
	SAFE_DELETE(m_pPrioritySteering);
	SAFE_DELETE(m_pAgentToEvade);

	for(auto pAgent: m_Agents)
	{
		SAFE_DELETE(pAgent);
	}
	m_Agents.clear();

	//for (auto pAgent : m_Neighbors)
	//{
	//	SAFE_DELETE(pAgent);
	//}
	m_Neighbors.clear();

	//for (auto pOldPos : m_OldPositions)
	//{
	//	SAFE_DELETE(pOldPos);
	//}
	m_OldPositions.clear();

	SAFE_DELETE(m_pCohesionBehavior);
	SAFE_DELETE(m_pSeparationBehavior);
	SAFE_DELETE(m_pVelMatchBehavior);
	SAFE_DELETE(m_pSeekBehavior);
	SAFE_DELETE(m_pWanderBehavior);
	SAFE_DELETE(m_pEvadeBehavior);

	SAFE_DELETE(m_pCellSpace);
}

void Flock::Update(float deltaT)
{
	// TODO: update the flock
	// loop over all the agents
	for(int idx{}; idx < m_Agents.size(); ++idx)
	{
		// register its neighbors	(-> memory pool is filled with neighbors of the currently evaluated agent)
		if(m_UsingSpacialPartitioning)
		{
			m_pCellSpace->RegisterNeighbors(m_Agents[idx], m_NeighborhoodRadius);
			m_Neighbors = m_pCellSpace->GetNeighbors();
			m_NrOfNeighbors = m_pCellSpace->GetNrOfNeighbors();
		}
		else
		{
			RegisterNeighbors(m_Agents[idx]);
		}
		// update it				(-> the behaviors can use the neighbors stored in the pool, next iteration they will be the next agent's neighbors)
		m_Agents[idx]->Update(deltaT);
		// trim it to the world
		m_Agents[idx]->TrimToWorld(m_WorldSize, true);

		if (m_UsingSpacialPartitioning)
		{
			m_pCellSpace->UpdateAgentCell(m_Agents[idx], m_OldPositions[idx]);

			m_OldPositions[idx] = m_Agents[idx]->GetPosition();
		}
	}

	if (m_Agents[0]->GetSteeringBehavior() == m_pEvadeBehavior || m_Agents[0]->GetSteeringBehavior() == m_pPrioritySteering)
	{
		m_pEvadeBehavior->SetTarget(m_pAgentToEvade->GetPosition());
	}
	m_pAgentToEvade->Update(deltaT);
	m_pAgentToEvade->TrimToWorld(m_WorldSize, true);
}

void Flock::Render(float deltaT)
{
	// TODO: render the flock
	for (auto& pAgent : m_Agents)
	{
		pAgent->Render(deltaT);

		if(pAgent->CanRenderBehavior())
		{
			for (int idx{}; idx < m_NrOfNeighbors; ++idx)
			{
				DEBUGRENDERER2D->DrawCircle(m_Neighbors[idx]->GetPosition(), m_Neighbors[idx]->GetRadius(), {0,0,1}, 0.f);
			}
			//Draw neighborhood cells
			if(m_UsingSpacialPartitioning)
			{
				for(const auto& pCell : m_pCellSpace->m_NeighborhoodCells)
				{
					if(pCell == nullptr)
					{
						continue;
					}
					Elite::Polygon cellPoints{ pCell->GetRectPoints() };
					DEBUGRENDERER2D->DrawPolygon(&cellPoints, { 0,0,1 }, 0.f);
				}
			}
		}
	}

	if (m_UsingSpacialPartitioning)
	{
		m_pCellSpace->RenderCells();

		for (auto& pAgent : m_Agents)
		{
			if (pAgent->CanRenderBehavior())
			{
				const Elite::Vector2 rectBottomLeft{ pAgent->GetPosition().x - m_NeighborhoodRadius, pAgent->GetPosition().y - m_NeighborhoodRadius };
				const float rectWidth{ m_NeighborhoodRadius * 2 };
				const float rectHeight{ m_NeighborhoodRadius * 2 };
				const Elite::Rect queryRect{ rectBottomLeft, rectWidth, rectHeight };

				std::vector<Elite::Vector2> rectPoints =
				{
					{ rectBottomLeft },
					{ rectBottomLeft.x , rectBottomLeft.y + rectHeight },
					{ rectBottomLeft.x + rectWidth , rectBottomLeft.y + rectHeight },
					{ rectBottomLeft.x + rectWidth , rectBottomLeft.y },
				};
				Elite::Polygon rectPolygon{ rectPoints };

				DEBUGRENDERER2D->DrawPolygon(&rectPolygon, { 0,0,1 });
			}
		}
	}
}

void Flock::UpdateAndRenderUI()
{
	//Setup
	int menuWidth = 235;
	int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
	int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
	bool windowActive = true;
	ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
	ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
	ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::PushAllowKeyboardFocus(false);

	//Elements
	ImGui::Text("CONTROLS");
	ImGui::Indent();
	ImGui::Text("LMB: place target");
	ImGui::Text("RMB: move cam.");
	ImGui::Text("Scrollwheel: zoom cam.");
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::Text("STATS");
	ImGui::Indent();
	ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
	ImGui::Unindent();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::Text("Flocking");
	ImGui::Spacing();

	// TODO: Implement checkboxes for debug rendering and weight sliders here

	ImGui::Checkbox("Space Partitioning", &m_UsingSpacialPartitioning);

	auto& m_pBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
	if (m_pPrioritySteering)
	{
		ImGui::SliderFloat("Seek", &m_pBehaviors[0].weight, 0.f, 1.f, "%.1");
		ImGui::SliderFloat("Wander", &m_pBehaviors[1].weight, 0.f, 1.f, "%.1");
		ImGui::SliderFloat("Separation", &m_pBehaviors[2].weight, 0.f, 1.f, "%.1");
		ImGui::SliderFloat("Cohesion", &m_pBehaviors[3].weight, 0.f, 1.f, "%.1");
		ImGui::SliderFloat("VelMatch", &m_pBehaviors[4].weight, 0.f, 1.f, "%.1");
	}
	ImGui::Spacing();

	//End
	ImGui::PopAllowKeyboardFocus();
	ImGui::End();
	
}

void Flock::RegisterNeighbors(SteeringAgent* pAgent)
{
	// TODO: Implement
	m_NrOfNeighbors = 0;
	for (const auto& pNeighbor : m_Agents)
	{
		if (pNeighbor != pAgent)
		{
			if (m_NeighborhoodRadius * m_NeighborhoodRadius >= DistanceSquared(pAgent->GetPosition(), pNeighbor->GetPosition()))
			{

				m_Neighbors[m_NrOfNeighbors] = pNeighbor;
				++m_NrOfNeighbors;
			}
		}
	}
}

Elite::Vector2 Flock::GetAverageNeighborPos() const
{
	// TODO: Implement
	Vector2 averagePos{};

	for(int currentNeighborNr{}; currentNeighborNr < m_NrOfNeighbors; ++currentNeighborNr)
	{
		averagePos += m_Neighbors[currentNeighborNr]->GetPosition();
	}

	averagePos /= static_cast<float>(m_NrOfNeighbors);

	return averagePos;
}

Elite::Vector2 Flock::GetAverageNeighborVelocity() const
{
	// TODO: Implement
	Vector2 averageVelocity{};

	for (int currentNeighborNr{}; currentNeighborNr < m_NrOfNeighbors; ++currentNeighborNr)
	{
		averageVelocity += m_Neighbors[currentNeighborNr]->GetLinearVelocity();
	}

	averageVelocity /= static_cast<float>(m_NrOfNeighbors);

	return averageVelocity;
}

void Flock::SetTarget_Seek(TargetData target)
{
	// TODO: Set target for seek behavior
	auto const mouseData = INPUTMANAGER->GetMouseData(InputType::eMouseButton, InputMouseButton::eLeft);
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft))
	{
		m_SeekTarget.Position = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y) });
		//m_pSeekBehavior->SetTarget(DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y) }));
		m_pSeekBehavior->SetTarget(m_SeekTarget);
	}
}


float* Flock::GetWeight(ISteeringBehavior* pBehavior) 
{
	if (m_pBlendedSteering)
	{
		auto& weightedBehaviors = m_pBlendedSteering->GetWeightedBehaviorsRef();
		auto it = find_if(weightedBehaviors.begin(),
			weightedBehaviors.end(),
			[pBehavior](BlendedSteering::WeightedBehavior el)
			{
				return el.pBehavior == pBehavior;
			}
		);

		if(it!= weightedBehaviors.end())
			return &it->weight;
	}

	return nullptr;
}
