#include "stdafx.h"
#include "StatesAndTransitions.h"

using namespace Elite;
using namespace FSMStates;

void WanderState::OnEnter(Blackboard* pBlackboard)
{
	//std::cout << "Wandering\n";
	AgarioAgent* pAgent;

	bool isValid = pBlackboard->GetData("Agent", pAgent);

	if(isValid == false || pAgent == nullptr)
	{
		return;
	}
	pAgent->SetToWander();

	const Vector2 center = { pAgent->m_WorldSize / 2, pAgent->m_WorldSize / 2 };
	pAgent->GetSteeringBehavior()->SetTarget(center);
}

void FSMStates::SeekFoodState::OnEnter(Elite::Blackboard* pBlackboard)
{
	//std::cout << "Getting food\n";
	AgarioAgent* pAgent;
	AgarioFood* pFood;

	bool isValid = pBlackboard->GetData("Agent", pAgent);

	if (isValid == false || pAgent == nullptr)
	{
		return;
	}

	pBlackboard->GetData("FoodNearbyPtr", pFood);
	pAgent->SetToSeek(pFood->GetPosition());
}

void FSMStates::FleeEnemeyState::OnEnter(Elite::Blackboard* pBlackboard)
{
	//std::cout << "Fleeing from other agent\n";
	AgarioAgent* pAgent;
	std::vector<AgarioAgent*>* pOtherAgents;

	if (pBlackboard->GetData("Agent", pAgent) == false)
	{
		return;
	}
	const float fleeRadius{ pAgent->GetRadius() * 2.f + 20.f };

	if (pBlackboard->GetData("EnemyAgents", pOtherAgents) == false)
	{
		return;
	}

	if (pAgent == nullptr || pOtherAgents == nullptr)
	{
		return;
	}

	//lamda stuff
	Vector2 agentPos{ pAgent->GetPosition() };

	auto elementdist = [agentPos](AgarioAgent* pOther1, AgarioAgent* pOther2)
	{
		float dist1 = agentPos.DistanceSquared(pOther1->GetPosition());
		float dist2 = agentPos.DistanceSquared(pOther2->GetPosition());
		return dist1 < dist2;
	};
	auto closestOtherIt = std::min_element(pOtherAgents->begin(), pOtherAgents->end(), elementdist);

	AgarioAgent* pOther = *closestOtherIt;
	
	//All other checks were already done in evaluate, if false there, this wouldn't fire
	pAgent->SetToFlee(pOther->GetPosition(), fleeRadius);
}

void FSMStates::SeekTargetState::OnEnter(Elite::Blackboard* pBlackboard)
{
	//std::cout << "Getting other agent\n";
	AgarioAgent* pAgent;
	std::vector<AgarioAgent*>* pOtherAgents;

	if (pBlackboard->GetData("Agent", pAgent) == false)
	{
		return;
	}
	const float fleeRadius{ pAgent->GetRadius() * 1.5f };

	if (pBlackboard->GetData("EnemyAgents", pOtherAgents) == false)
	{
		return;
	}

	if (pAgent == nullptr || pOtherAgents == nullptr)
	{
		return;
	}

	//lamda stuff
	Vector2 agentPos{ pAgent->GetPosition() };

	auto elementdist = [agentPos](AgarioAgent* pOther1, AgarioAgent* pOther2)
	{
		float dist1 = agentPos.DistanceSquared(pOther1->GetPosition());
		float dist2 = agentPos.DistanceSquared(pOther2->GetPosition());
		return dist1 < dist2;
	};
	auto closestOtherIt = std::min_element(pOtherAgents->begin(), pOtherAgents->end(), elementdist);

	AgarioAgent* pOther = *closestOtherIt;

	//All other checks were already done in evaluate, if false there, this wouldn't fire
	pAgent->SetToSeek(pOther->GetPosition());
}

bool FSMConditions::FoodNearbyCondition::Evaluate(Blackboard* pBlackboard) const
{
	AgarioAgent* pAgent;
	std::vector<AgarioFood*>* pFoodVec;

	if (pBlackboard->GetData("Agent", pAgent) == false || pAgent == nullptr)
	{
		return false;
	}

	const float smellRadius{ pAgent->GetRadius() * 2.f + 20.f };

	if (pBlackboard->GetData("FoodVecPtr", pFoodVec) == false || pFoodVec == nullptr)
	{
		return false;
	}

	//lamda stuff
	Vector2 agentPos{ pAgent->GetPosition() };

	auto elementdist = [agentPos](AgarioFood* pFood1, AgarioFood* pFood2)
	{
		float dist1 = agentPos.DistanceSquared(pFood1->GetPosition());
		float dist2 = agentPos.DistanceSquared(pFood2->GetPosition());
		return dist1 < dist2;
	};
	auto closestFoodIt = std::min_element(pFoodVec->begin(), pFoodVec->end(), elementdist);
	 
	if (closestFoodIt != pFoodVec->end())
	{
		AgarioFood* pFood = *closestFoodIt;

		if (agentPos.DistanceSquared(pFood->GetPosition()) < smellRadius * smellRadius)
		{
			pBlackboard->ChangeData("FoodNearbyPtr", pFood);
			return true;
		}
	}

	return false;
}

bool FSMConditions::FoodEaten::Evaluate(Elite::Blackboard* pBlackboard) const
{
	std::vector<AgarioFood*>* pFoodVec;
	AgarioFood* pFood;

	if (pBlackboard->GetData("FoodVecPtr", pFoodVec) == false)
	{
		return false;
	}

	if (pBlackboard->GetData("FoodNearbyPtr", pFood) == false)
	{
		return false;
	}

	if (pFoodVec == nullptr || pFood == nullptr)
	{
		return false;
	}

	if (std::find(pFoodVec->begin(), pFoodVec->end(), pFood) == pFoodVec->end())
	{
		return true;
	}
	return false;
}

bool FSMConditions::BigEnemyNearby::Evaluate(Elite::Blackboard* pBlackboard) const
{
	std::vector<AgarioAgent*>* pAgentVec;
	AgarioAgent* pMainAgent;

	if (pBlackboard->GetData("Agent", pMainAgent) == false)
	{
		return false;
	}

	if (pBlackboard->GetData("EnemyAgents", pAgentVec) == false)
	{
		return false;
	}

	if (pMainAgent == nullptr || pAgentVec == nullptr)
	{
		return false;
	}

	const float fleeRadius{ pMainAgent->GetRadius() * 2.f + 20.f };
	DEBUGRENDERER2D->DrawCircle(pMainAgent->GetPosition(), fleeRadius,  Color{1,0,0}, 0);

	AgarioAgent* pClosestLarge{};
	Vector2 agentPos{ pMainAgent->GetPosition() };
	float smallestDistanceSquare{ FLT_MAX };

	for (const auto& pOther : *pAgentVec)
	{
		if (pOther->GetRadius() > pMainAgent->GetRadius())
		{
			const float currentDistance = agentPos.DistanceSquared(pOther->GetPosition());
			if (currentDistance < smallestDistanceSquare)
			{
				smallestDistanceSquare = currentDistance;
				pClosestLarge = pOther;
			}
		}
	}

	if (pClosestLarge == nullptr)
	{
		return false;
	}

	if (agentPos.DistanceSquared(pClosestLarge->GetPosition()) < fleeRadius * fleeRadius)
	{
		DEBUGRENDERER2D->DrawCircle(pClosestLarge->GetPosition(), pClosestLarge->GetRadius() + 1, Color{ 1,0,0 }, 0.1);
		return true;
	}
	return false;
}

bool FSMConditions::NoBigEnemyNearby::Evaluate(Elite::Blackboard* pBlackboard) const
{
	std::vector<AgarioAgent*>* pAgentVec;
	AgarioAgent* pMainAgent;

	if (pBlackboard->GetData("Agent", pMainAgent) == false)
	{
		return true;
	}

	if (pBlackboard->GetData("EnemyAgents", pAgentVec) == false)
	{
		return true;
	}

	if (pMainAgent == nullptr || pAgentVec == nullptr)
	{
		return true;
	}

	const float fleeRadius{ pMainAgent->GetRadius() * 2.f + 20.f };
	DEBUGRENDERER2D->DrawCircle(pMainAgent->GetPosition(), fleeRadius, Color{ 1,0,0 }, 0);

	//lamda stuff
	AgarioAgent* pClosestLarge{};
	Vector2 agentPos{ pMainAgent->GetPosition() };
	float smallestDistanceSquare{ FLT_MAX };

	for (const auto& pOther : *pAgentVec)
	{
		if (pOther->GetRadius() > pMainAgent->GetRadius())
		{
			const float currentDistance = agentPos.DistanceSquared(pOther->GetPosition());
			if (currentDistance < smallestDistanceSquare)
			{
				smallestDistanceSquare = currentDistance;
				pClosestLarge = pOther;
			}
		}
	}

	if (pClosestLarge == nullptr)
	{
		return true;
	}

	if (agentPos.DistanceSquared(pClosestLarge->GetPosition()) < fleeRadius * fleeRadius)
	{			
		return false;			
	}
	
	return true;
}

bool FSMConditions::SmallEnemyNearby::Evaluate(Elite::Blackboard* pBlackboard) const
{
	std::vector<AgarioAgent*>* pAgentVec;
	AgarioAgent* pMainAgent;

	if (pBlackboard->GetData("Agent", pMainAgent) == false)
	{
		return false;
	}

	if (pBlackboard->GetData("EnemyAgents", pAgentVec) == false)
	{
		return false;
	}

	if (pMainAgent == nullptr || pAgentVec == nullptr)
	{
		return false;
	}

	const float searchRadius{ pMainAgent->GetRadius() * 2.f + 20.f };
	DEBUGRENDERER2D->DrawCircle(pMainAgent->GetPosition(), searchRadius, Color{ 1,0,0 }, 0);

	//lamda stuff
	Vector2 agentPos{ pMainAgent->GetPosition() };

	auto elementdist = [agentPos](AgarioAgent* pOther1, AgarioAgent* pOther2)
	{
		float dist1 = agentPos.DistanceSquared(pOther1->GetPosition());
		float dist2 = agentPos.DistanceSquared(pOther2->GetPosition());
		return dist1 < dist2;
	};
	auto closestOtherIt = std::min_element(pAgentVec->begin(), pAgentVec->end(), elementdist);

	if (closestOtherIt != pAgentVec->end())
	{
		AgarioAgent* pOther = *closestOtherIt;

		if (agentPos.DistanceSquared(pOther->GetPosition()) < searchRadius * searchRadius)
		{
			if (pOther->GetRadius() + 1.f < pMainAgent->GetRadius()) //small offset because sometimes it won't eat
			{
				return true;
			}
		}
	}
	return false;
}

bool FSMConditions::NoSmallEnemyNearby::Evaluate(Elite::Blackboard* pBlackboard) const
{
	std::vector<AgarioAgent*>* pAgentVec;
	AgarioAgent* pMainAgent;

	if (pBlackboard->GetData("Agent", pMainAgent) == false)
	{
		return true;
	}

	if (pBlackboard->GetData("EnemyAgents", pAgentVec) == false)
	{
		return true;
	}

	if (pMainAgent == nullptr || pAgentVec == nullptr)
	{
		return true;
	}

	const float searchRadius{ pMainAgent->GetRadius() * 2.f + 20.f };
	DEBUGRENDERER2D->DrawCircle(pMainAgent->GetPosition(), searchRadius, Color{ 1,0,0 }, 0);

	//lamda stuff
	Vector2 agentPos{ pMainAgent->GetPosition() };

	auto elementdist = [agentPos](AgarioAgent* pOther1, AgarioAgent* pOther2)
	{
		float dist1 = agentPos.DistanceSquared(pOther1->GetPosition());
		float dist2 = agentPos.DistanceSquared(pOther2->GetPosition());
		return dist1 < dist2;
	};
	auto closestOtherIt = std::min_element(pAgentVec->begin(), pAgentVec->end(), elementdist);

	if (closestOtherIt != pAgentVec->end())
	{
		AgarioAgent* pOther = *closestOtherIt;

		if (agentPos.DistanceSquared(pOther->GetPosition()) < searchRadius * searchRadius)
		{
			if (pOther->GetRadius() + 1.f < pMainAgent->GetRadius())
			{
				return false;
			}
		}
	}
	return true;
}

//Influence

void FSMStates::InfluenceMovement::OnEnter(Elite::Blackboard* pBlackboard)
{
	//std::cout << "Getting food\n";
	AgarioAgent* pAgent;
	AgarioFood* pFood;

	bool isValid = pBlackboard->GetData("Agent", pAgent);

	if (isValid == false || pAgent == nullptr)
	{
		return;
	}

	pBlackboard->GetData("FoodNearbyPtr", pFood);
	pAgent->SetToSeek(pFood->GetPosition());
}