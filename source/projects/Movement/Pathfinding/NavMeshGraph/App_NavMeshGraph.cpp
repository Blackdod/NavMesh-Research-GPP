//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"
using namespace Elite;

//Includes
#include "App_NavMeshGraph.h"
#include "projects/Shared/NavigationColliderElement.h"

#include "projects/Movement/SteeringBehaviors/SteeringAgent.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"


#include "framework\EliteAI\EliteNavigation\Algorithms\ENavGraphPathfinding.h"

//Statics
bool App_NavMeshGraph::sShowPolygon = true;
bool App_NavMeshGraph::sShowGraph = false;
bool App_NavMeshGraph::sDrawPortals = false;
bool App_NavMeshGraph::sDrawFinalPath = true;
bool App_NavMeshGraph::sDrawNonOptimisedPath = false;

//Destructor
App_NavMeshGraph::~App_NavMeshGraph()
{
	for (auto pNC : m_vNavigationColliders)
		SAFE_DELETE(pNC);
	m_vNavigationColliders.clear();

	SAFE_DELETE(m_pNavGraph);
	SAFE_DELETE(m_pSeekBehavior);
	SAFE_DELETE(m_pArriveBehavior);
	SAFE_DELETE(m_pAgent);
}

//Functions
void App_NavMeshGraph::Start()
{
	//Initialization of your application. 
	//----------- CAMERA ------------
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(36.782f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(12.9361f, 0.2661f));

	//----------- NAVMESH  ------------
	std::list<Elite::Vector2> baseBox
	{ { -60, 30 },{ -60, -30 },{ 60, -30 },{ 60, 30 } };
	m_BaseBox = Elite::Polygon{ baseBox };

	m_pNavGraph = new Elite::NavGraph(Elite::Polygon(baseBox), m_AgentRadius);

	//----------- AGENT ------------
	m_pSeekBehavior = new Seek();
	m_pArriveBehavior = new Arrive();
	m_pArriveBehavior->SetSlowRadius(3.0f);
	m_pArriveBehavior->SetTargetRadius(1.0f);
	m_Target = TargetData(Elite::ZeroVector2);
	m_pAgent = new SteeringAgent();
	m_pAgent->SetSteeringBehavior(m_pSeekBehavior);
	m_pAgent->SetMaxLinearSpeed(m_AgentSpeed);
	m_pAgent->SetAutoOrient(true);
	m_pAgent->SetMass(0.1f);

}

void App_NavMeshGraph::Update(float deltaTime)
{
	//Update target/path based on input
	if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eMiddle))
	{
		auto mouseData = INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eMiddle);
		Elite::Vector2 mouseTarget = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld(
			Elite::Vector2((float)mouseData.X, (float)mouseData.Y));
		m_vPath = NavMeshPathfinding::FindPath(m_pAgent->GetPosition(), mouseTarget, m_pNavGraph, m_DebugNodePositions, m_Portals);
		m_Target.Position = mouseTarget;
		//std::cout << mouseTarget.x << ", " << mouseTarget.y << '\n';
	}
	else if (INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eLeft))
	{
		auto mouseData = INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
		Elite::Vector2 mouseTarget = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld(
			Elite::Vector2((float)mouseData.X, (float)mouseData.Y));

		Elite::Polygon newWall = CreateWall(mouseTarget, m_Width, m_Height);
		if (newWall.GetCenterPoint() != Elite::Vector2(0, 0)) //placed a wall somewhere wrong
		{
			m_pNavGraph->UpdateGraph(m_pAgent->GetRadius(), newWall);
		}
	}

	if (didNavMeshChange)
	{
		m_vPath = NavMeshPathfinding::FindPath(m_pAgent->GetPosition(), m_Target.Position, m_pNavGraph, m_DebugNodePositions, m_Portals);
	}

	//Check if a path exist and move to the following point
	if (m_vPath.size() > 0)
	{
		if (m_vPath.size() == 1)
		{
			//We have reached the last node
			m_pAgent->SetSteeringBehavior(m_pArriveBehavior);
			m_pArriveBehavior->SetTarget(m_vPath[0]);
		}
		else
		{
			//Move to the next node
			m_pAgent->SetSteeringBehavior(m_pSeekBehavior);
			m_pSeekBehavior->SetTarget(m_vPath[0]);
		}

		if (Elite::DistanceSquared(m_pAgent->GetPosition(), m_vPath[0]) < m_AgentRadius * m_AgentRadius)
		{
			//If we reached the next point of the path. Remove it 
			m_vPath.erase(std::remove(m_vPath.begin(), m_vPath.end(), m_vPath[0]));
		}
	}
	
	UpdateImGui();
	m_pAgent->Update(deltaTime);
}

void App_NavMeshGraph::Render(float deltaTime) const
{
	if (sShowGraph)
	{
		m_GraphRenderer.RenderGraph(m_pNavGraph, true, true);
	}

	if (sShowPolygon)
	{
		DEBUGRENDERER2D->DrawPolygon(m_pNavGraph->GetNavMeshPolygon(),
			Color(0.1f, 0.1f, 0.1f));
		DEBUGRENDERER2D->DrawSolidPolygon(m_pNavGraph->GetNavMeshPolygon(),
			Color(0.0f, 0.5f, 0.1f, 0.05f), 0.4f);
	}

	if (sDrawPortals)
	{
		for (const auto &portal : m_Portals)
		{
			DEBUGRENDERER2D->DrawSegment(portal.Line.p1, portal.Line.p2, Color(1.f, .5f, 0.f), -0.1f);
			//Draw just p1 p2
			std::string p1{ "p1" };
			std::string p2{ "p2" };
			//Add the positions to the debugdrawing
			//p1 +=" x:" + std::to_string(portal.Line.p1.x) + ", y: " + std::to_string(portal.Line.p1.y);
			//p2 +=" x:" + std::to_string(portal.Line.p2.x) + ", y: " + std::to_string(portal.Line.p2.y);
			DEBUGRENDERER2D->DrawString(portal.Line.p1, p1.c_str(), Color(1.f, .5f, 0.f), -0.1f);
			DEBUGRENDERER2D->DrawString(portal.Line.p2, p2.c_str(), Color(1.f, .5f, 0.f), -0.1f);

		}
	}

	if (sDrawNonOptimisedPath)
	{
		for (auto pathNode : m_DebugNodePositions)
			DEBUGRENDERER2D->DrawCircle(pathNode, 2.0f, Color(0.f, 0.f, 1.f), -0.45f);
	}

	if (sDrawFinalPath && m_vPath.size() > 0)
	{

		for (auto pathPoint : m_vPath)
			DEBUGRENDERER2D->DrawCircle(pathPoint, 2.0f, Color(1.f, 0.f, 0.f), -0.2f);

		DEBUGRENDERER2D->DrawSegment(m_pAgent->GetPosition(), m_vPath[0], Color(1.f, 0.0f, 0.0f), -0.2f);
		for (size_t i = 0; i < m_vPath.size() - 1; i++)
		{
			float g = float(i) / m_vPath.size();
			DEBUGRENDERER2D->DrawSegment(m_vPath[i], m_vPath[i+1], Color(1.f, g, g), -0.2f);
		}
			
	}


}

void App_NavMeshGraph::UpdateImGui()
{
	//------- UI --------
#ifdef PLATFORM_WINDOWS
#pragma region UI
	{
		//Setup
		int menuWidth = 150;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 90));
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);
		ImGui::SetWindowFocus();
		ImGui::PushItemWidth(70);
		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
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
		ImGui::Spacing();

		ImGui::Checkbox("Show Polygon", &sShowPolygon);
		ImGui::Checkbox("Show Graph", &sShowGraph);
		ImGui::Checkbox("Show Portals", &sDrawPortals);
		ImGui::Checkbox("Show Path Nodes", &sDrawNonOptimisedPath);
		ImGui::Checkbox("Show Final Path", &sDrawFinalPath);
		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::SliderFloat("AgentSpeed", &m_AgentSpeed, 0.0f, 22.0f))
		{
			m_pAgent->SetMaxLinearSpeed(m_AgentSpeed);
		}
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("Walls (research)");
		ImGui::Spacing(); 

		if (ImGui::SliderFloat("WallWidth", &m_Width, 1.f, 50.f))
		{
			m_pAgent->SetMaxLinearSpeed(m_AgentSpeed);
		}
		ImGui::Spacing();

		if (ImGui::SliderFloat("WallHeight", &m_Height, 1.f, 50.f))
		{
			m_pAgent->SetMaxLinearSpeed(m_AgentSpeed);
		}

		
		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
}

Elite::Polygon App_NavMeshGraph::CreateWall(Elite::Vector2 position, float width, float height)
{
	for (const auto& collider : m_vNavigationColliders)
	{
		Elite::Vector2 colBottomLeft{ collider->m_Position.x - collider->m_Width / 2, collider->m_Position.y - collider->m_Height / 2 };
		Elite::Vector2 colTopRight{ collider->m_Position.x + collider->m_Width / 2 , collider->m_Position.y + collider->m_Height / 2 };

		if (position.x > colBottomLeft.x - width && position.x < colTopRight.x + width)
		{
			if (position.y > colBottomLeft.y - height && position.y < colTopRight.y + height )
			{
				std::cout << "Can't place wall here!\n";
				return Elite::Polygon{};
			}
		}
	}

	Elite::Vector2 baseBottomLeft{ m_BaseBox.GetPosVertMinXPos(), m_BaseBox.GetPosVertMinYPos() };
	Elite::Vector2 baseTopRight{ m_BaseBox.GetPosVertMaxXPos(), m_BaseBox.GetPosVertMaxYPos() };

	if (position.x < baseBottomLeft.x + width / 2 || position.x > baseTopRight.x - width / 2)
	{
		std::cout << "Can't place wall here!\n";
		return Elite::Polygon{};
	}
	else if (position.y < baseBottomLeft.y + height / 2 || position.y > baseTopRight.y - height / 2)
	{
		std::cout << "Can't place wall here!\n";
		return Elite::Polygon{};
	}

	m_vNavigationColliders.push_back(new NavigationColliderElement(position, width, height));

	didNavMeshChange = true;

	Elite::Vector2 bottomLeft{ position.x - width / 2, position.y - height / 2 };
	Elite::Vector2 bottomRight{ position.x + width / 2, position.y - height / 2 };
	Elite::Vector2 topLeft{ position.x - width / 2, position.y + height / 2 };
	Elite::Vector2 topRight{ position.x + width / 2 , position.y + height / 2  };

	std::list<Elite::Vector2> wallBox
	{ topLeft, topRight, bottomRight, bottomLeft };

	return Elite::Polygon{ wallBox };
}