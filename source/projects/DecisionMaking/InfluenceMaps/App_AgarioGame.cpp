#include "stdafx.h"
#include "App_AgarioGame.h"
#include "StatesAndTransitions.h"

//AgarioIncludes
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioContactListener.h"


using namespace Elite;
using namespace FSMStates;
using namespace FSMConditions;

App_AgarioGame::App_AgarioGame()
{
}

App_AgarioGame::~App_AgarioGame()
{
	for (auto& f : m_pFoodVec)
	{
		SAFE_DELETE(f);
	}
	m_pFoodVec.clear();

	for (auto& a : m_pAgentVec)
	{
		SAFE_DELETE(a);
	}
	m_pAgentVec.clear();

	SAFE_DELETE(m_pContactListener);
	SAFE_DELETE(m_pSmartAgent);
	for (auto& s : m_pStates)
	{
		SAFE_DELETE(s);
	}

	for (auto& t : m_pConditions)
	{
		SAFE_DELETE(t);
	}

	//==============================================================
	//Influence
	//==============================================================

	SAFE_DELETE(m_pInfluenceGrid);
}

void App_AgarioGame::Start()
{
	//Creating the world contact listener that informs us of collisions
	m_pContactListener = new AgarioContactListener();

	//Create food items
	m_pFoodVec.reserve(m_AmountOfFood);
	for (int i = 0; i < m_AmountOfFood; i++)
	{
		Elite::Vector2 randomPos = randomVector2(0, m_TrimWorldSize);
		m_pFoodVec.push_back(new AgarioFood(randomPos));
	}

	//Create default agents
	m_pAgentVec.reserve(m_AmountOfAgents);

	WanderState* pWanderState = new WanderState();
	m_pStates.push_back(pWanderState); //don't forget to add, otherwise memory leaks

	for (int i = 0; i < m_AmountOfAgents; i++)
	{
		Elite::Vector2 randomPos = randomVector2(0, m_TrimWorldSize * (2.0f / 3));
		AgarioAgent* newAgent = new AgarioAgent(randomPos, m_TrimWorldSize);

		Blackboard* pBlackBoard = CreateBlackboard(newAgent, m_pInfluenceGrid);

		FiniteStateMachine* pStateMachine = new FiniteStateMachine(pWanderState, pBlackBoard);
		newAgent->SetDecisionMaking(pStateMachine);

		m_pAgentVec.push_back(newAgent);
	}
	

	//-------------------
	//Create Custom Agent
	//-------------------
	Elite::Vector2 randomPos = randomVector2(0, m_TrimWorldSize * (2.0f / 3));
	Color customColor = Color{ 0.0f, 1.0f, 0.0f };
	m_pSmartAgent = new AgarioAgent(randomPos, customColor, m_TrimWorldSize);

	//1. Create and add the necessary blackboard data

	Blackboard* pBlackBoard = CreateBlackboard(m_pSmartAgent, m_pInfluenceGrid);

	//2. Create the different agent states

	SeekFoodState* pSeekFoodState = new SeekFoodState();
	m_pStates.push_back(pSeekFoodState);

	FleeEnemeyState* pFleeEnemyState = new FleeEnemeyState();
	m_pStates.push_back(pFleeEnemyState);

	SeekTargetState* pSeekTargetState = new SeekTargetState();
	m_pStates.push_back(pSeekTargetState);

	//3. Create the conditions beetween those states
	FoodNearbyCondition* pFoodNearbyCondition = new FoodNearbyCondition();
	m_pConditions.push_back(pFoodNearbyCondition);
	FoodEaten* pFoodEatenState = new FoodEaten();
	m_pConditions.push_back(pFoodEatenState);

	BigEnemyNearby* pEnemyNearbyCondition = new BigEnemyNearby();
	m_pConditions.push_back(pEnemyNearbyCondition);
	NoBigEnemyNearby* pNoEnemyNearbyCondition = new NoBigEnemyNearby();
	m_pConditions.push_back(pNoEnemyNearbyCondition);

	SmallEnemyNearby* pTargetNearbyCondition = new SmallEnemyNearby();
	m_pConditions.push_back(pTargetNearbyCondition);
	NoSmallEnemyNearby* pNoTargetNearbyCondition = new NoSmallEnemyNearby();
	m_pConditions.push_back(pNoTargetNearbyCondition);
	

	//4. Create the finite state machine with a starting state and the blackboard
	FiniteStateMachine* pStateMachine = new FiniteStateMachine(pWanderState, pBlackBoard);

	//5. Add the transitions for the states to the state machine
	// stateMachine->AddTransition(startState, toState, condition)
	// startState: active state for which the transition will be checked
	// condition: if the Evaluate function returns true => transition will fire and move to the toState
	// toState: end state where the agent will move to if the transition fires

	pStateMachine->AddTransition(pWanderState, pFleeEnemyState, pEnemyNearbyCondition);
	pStateMachine->AddTransition(pSeekFoodState, pFleeEnemyState, pEnemyNearbyCondition);
	pStateMachine->AddTransition(pSeekTargetState, pFleeEnemyState, pEnemyNearbyCondition);
	pStateMachine->AddTransition(pFleeEnemyState, pFleeEnemyState, pEnemyNearbyCondition);
	pStateMachine->AddTransition(pFleeEnemyState, pWanderState, pNoEnemyNearbyCondition);

	pStateMachine->AddTransition(pWanderState, pSeekTargetState, pTargetNearbyCondition);
	pStateMachine->AddTransition(pSeekFoodState, pSeekTargetState, pTargetNearbyCondition);
	pStateMachine->AddTransition(pSeekTargetState, pSeekTargetState, pTargetNearbyCondition);
	pStateMachine->AddTransition(pSeekTargetState, pSeekFoodState, pNoTargetNearbyCondition);
	pStateMachine->AddTransition(pSeekTargetState, pWanderState, pNoTargetNearbyCondition);

	pStateMachine->AddTransition(pWanderState, pSeekFoodState, pFoodNearbyCondition);
	pStateMachine->AddTransition(pSeekFoodState, pWanderState, pFoodEatenState);
	pStateMachine->AddTransition(pSeekFoodState, pSeekFoodState, pFoodNearbyCondition);
	
	//6. Activate the decision making stucture on the custom agent by calling the SetDecisionMaking function
	m_pSmartAgent->SetDecisionMaking(pStateMachine);

	//===================================================
	//Influence
	//====================================================

	m_pInfluenceGrid = new Elite::InfluenceMap<InfluenceGrid>(false);
	m_pInfluenceGrid->InitializeGrid(20, 20, 5, false, true);
	m_pInfluenceGrid->InitializeBuffer();

	m_GraphRenderer.SetNumberPrintPrecision(0);
}

void App_AgarioGame::Update(float deltaTime)
{
	UpdateImGui();

	//Check if agent is still alive
	if (m_pSmartAgent->CanBeDestroyed())
	{
		m_GameOver = true;

		//Update the other agents and food
		UpdateAgarioEntities(m_pFoodVec, deltaTime);
		UpdateAgarioEntities(m_pAgentVec, deltaTime);
		return;
	}
	//Update the custom agent
	m_pSmartAgent->Update(deltaTime);
	m_pSmartAgent->TrimToWorld(m_TrimWorldSize, false);

	//Update the other agents and food
	UpdateAgarioEntities(m_pFoodVec, deltaTime);
	UpdateAgarioEntities(m_pAgentVec, deltaTime);

	
	//Check if we need to spawn new food
	m_TimeSinceLastFoodSpawn += deltaTime;
	if (m_TimeSinceLastFoodSpawn > m_FoodSpawnDelay)
	{
		m_TimeSinceLastFoodSpawn = 0.f;
		m_pFoodVec.push_back(new AgarioFood(randomVector2(0, m_TrimWorldSize)));
	}

	//===========================================================
	//Influence
	//===========================================================
	const float foodInfluence{ 20.f };
	const float targetInfluence{ 75.f };
	const float enemyInfluence{ -100.f };
	for (const auto& other : m_pAgentVec)
	{
		if (other->GetRadius() + 1.f < m_pSmartAgent->GetRadius())
		{
			m_pInfluenceGrid->SetInfluenceAtPosition(other->GetPosition(), targetInfluence);
		}
		else
		{
			m_pInfluenceGrid->SetInfluenceAtPosition(other->GetPosition(), enemyInfluence);
		}
	}
	for (const auto& food : m_pFoodVec)
	{
		const float currentNodeInfluence = m_pInfluenceGrid->GetNodeAtWorldPos(food->GetPosition())->GetInfluence();
		m_pInfluenceGrid->SetInfluenceAtPosition( food->GetPosition(), foodInfluence);
	}
	m_pInfluenceGrid->PropagateInfluence(deltaTime);
}

void App_AgarioGame::Render(float deltaTime) const
{
	RenderWorldBounds(m_TrimWorldSize);

	for (AgarioFood* f : m_pFoodVec)
	{
		f->Render(deltaTime);
	}

	for (AgarioAgent* a : m_pAgentVec)
	{
		a->Render(deltaTime);
	}

	m_pSmartAgent->Render(deltaTime);

	//============================================================
	//Influence
	//============================================================
	m_pInfluenceGrid->SetNodeColorsBasedOnInfluence();
	m_GraphRenderer.RenderGraph(m_pInfluenceGrid, true, false, false, true);
}

Blackboard* App_AgarioGame::CreateBlackboard(AgarioAgent* a, Elite::InfluenceMap<InfluenceGrid>* influenceMap)
{
	Blackboard* pBlackboard = new Blackboard();
	pBlackboard->AddData("Agent", a);
	pBlackboard->AddData("FoodVecPtr", &m_pFoodVec);
	pBlackboard->AddData("FoodNearbyPtr", static_cast<AgarioFood*>(nullptr)); //type needs to be known at compile time
	pBlackboard->AddData("EnemyAgents", &m_pAgentVec);
	//...

	pBlackboard->AddData("InfluenceMap", influenceMap);

	return pBlackboard;
}

void App_AgarioGame::UpdateImGui()
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
		ImGui::Begin("Agario", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
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
		
		ImGui::Text("Agent Info");
		ImGui::Text("Radius: %.1f",m_pSmartAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());

		auto momentum = m_pInfluenceGrid->GetMomentum();
		auto decay = m_pInfluenceGrid->GetDecay();
		auto propagationInterval = m_pInfluenceGrid->GetPropagationInterval();
		//
		ImGui::SliderFloat("Momentum", &momentum, 0.0f, 1.f, "%.2");
		ImGui::SliderFloat("Decay", &decay, 0.f, 1.f, "%.2");
		ImGui::SliderFloat("Propagation Interval", &propagationInterval, 0.f, 2.f, "%.2");
		ImGui::Spacing();

		//Set data
		m_pInfluenceGrid->SetMomentum(momentum);
		m_pInfluenceGrid->SetDecay(decay);
		m_pInfluenceGrid->SetPropagationInterval(propagationInterval);
		
		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
	if(m_GameOver)
	{
		//Setup
		int menuWidth = 300;
		int menuHeight = 100;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2(width/2.0f- menuWidth, height/2.0f - menuHeight));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)menuHeight));
		ImGui::Begin("Game Over", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		ImGui::Text("Final Agent Info");
		ImGui::Text("Radius: %.1f", m_pSmartAgent->GetRadius());
		ImGui::Text("Survive Time: %.1f", TIMER->GetTotal());
		ImGui::End();
	}
#pragma endregion
#endif

}
