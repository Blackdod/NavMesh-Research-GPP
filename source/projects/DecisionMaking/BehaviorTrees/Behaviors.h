/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "framework/EliteMath/EMath.h"
#include "framework/EliteAI/EliteDecisionMaking/EliteBehaviorTree/EBehaviorTree.h"
#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------

namespace BT_Actions
{
	Elite::BehaviorState ChangeToWander(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		pAgent->SetToWander();
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToSeek(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		Elite::Vector2 targetPos{};
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}
		
		if (!pBlackboard->GetData("Target", targetPos))
		{
			return Elite::BehaviorState::Failure;
		}

		pAgent->SetToSeek(targetPos);
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToFlee(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		std::vector<AgarioAgent*>* pOtherAgents;

		if (pBlackboard->GetData("Agent", pAgent) == false)
		{
			return Elite::BehaviorState::Failure;
		}
		const float fleeRadius{ pAgent->GetRadius() * 2.f + 20.f };

		if (pBlackboard->GetData("AgentsVec", pOtherAgents) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		if (pAgent == nullptr || pOtherAgents == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		//lamda stuff
		Elite::Vector2 agentPos{ pAgent->GetPosition() };

		auto elementdist = [agentPos](AgarioAgent* pOther1, AgarioAgent* pOther2)
		{
			float dist1 = agentPos.DistanceSquared(pOther1->GetPosition());
			float dist2 = agentPos.DistanceSquared(pOther2->GetPosition());
			return dist1 < dist2;
		};
		auto closestOtherIt = std::min_element(pOtherAgents->begin(), pOtherAgents->end(), elementdist);

		AgarioAgent* pOther = *closestOtherIt;

		//All other checks were already done in conditional before, if false there, this wouldn't fire
		const TargetData data{pOther->GetPosition(), pOther->GetRotation(), pOther->GetLinearVelocity(), pOther->GetAngularVelocity()};
		pAgent->SetToEvade(data, fleeRadius);
		return Elite::BehaviorState::Success;
	}

	Elite::BehaviorState ChangeToSeekEnemy(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		std::vector<AgarioAgent*>* pOtherAgents;

		if (pBlackboard->GetData("Agent", pAgent) == false)
		{
			return Elite::BehaviorState::Failure;
		}
		const float fleeRadius{ pAgent->GetRadius() * 1.5f };

		if (pBlackboard->GetData("AgentsVec", pOtherAgents) == false)
		{
			return Elite::BehaviorState::Failure;
		}

		if (pAgent == nullptr || pOtherAgents == nullptr)
		{
			return Elite::BehaviorState::Failure;
		}

		//lamda stuff
		Elite::Vector2 agentPos{ pAgent->GetPosition() };

		auto elementdist = [agentPos](AgarioAgent* pOther1, AgarioAgent* pOther2)
		{
			float dist1 = agentPos.DistanceSquared(pOther1->GetPosition());
			float dist2 = agentPos.DistanceSquared(pOther2->GetPosition());
			return dist1 < dist2;
		};
		auto closestOtherIt = std::min_element(pOtherAgents->begin(), pOtherAgents->end(), elementdist);

		AgarioAgent* pOther = *closestOtherIt;

		//All other checks were already done in conditional before, if false there, this wouldn't fire
		const TargetData data{ pOther->GetPosition(), pOther->GetRotation(), pOther->GetLinearVelocity(), pOther->GetAngularVelocity() };
		pAgent->SetToPursuit(data);
		return Elite::BehaviorState::Success;
	}
}

namespace BT_Conditions
{
	bool IsFoodNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		std::vector<AgarioFood*>* pFoodVec;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return false;
		}
		const float searchRadius{ pAgent->GetRadius() * 2.f + 20.f};

		if (!pBlackboard->GetData("FoodVec", pFoodVec) || pFoodVec == nullptr)
		{
			return false;
		}

		AgarioFood* pClosestFood = nullptr;
		float closestDistSq{ searchRadius * searchRadius };

		Elite::Vector2 agentPos = pAgent->GetPosition();
		//TODO: Debug rendering
		DEBUGRENDERER2D->DrawCircle(agentPos, searchRadius, { 0,0,1 }, 0);

		for (const auto& pFood : *pFoodVec)
		{
			const float distSq = pFood->GetPosition().DistanceSquared(agentPos);
			if (distSq < closestDistSq)
			{
				pClosestFood = pFood;
				closestDistSq = distSq;
			}
		}


		if (pClosestFood != nullptr)
		{
			//std::cout << pClosestFood->GetPosition() << '\n';
			pBlackboard->ChangeData("Target", pClosestFood->GetPosition());
			return true;
		}

		return false;
	}

	bool IsLargeEnemyNearby(Elite::Blackboard* pBlackboard)
	{
		AgarioAgent* pAgent;
		std::vector<AgarioAgent*>* pOthers;
		if (!pBlackboard->GetData("Agent", pAgent) || pAgent == nullptr)
		{
			return false;
		}
		const float fleeRadius{ pAgent->GetRadius() * 2.f + 30.f };

		if (!pBlackboard->GetData("AgentsVec", pOthers) || pOthers == nullptr)
		{
			return false;
		}

		AgarioAgent* pClosestEnemy = nullptr;
		float closestDistSq{ fleeRadius * fleeRadius };

		Elite::Vector2 agentPos = pAgent->GetPosition();
		//TODO: Debug rendering
		DEBUGRENDERER2D->DrawCircle(agentPos, fleeRadius, { 1,0,0 }, 0);

		for (const auto& pOther : *pOthers)
		{
			if (pOther->GetRadius() > pAgent->GetRadius())
			{
				const float currentDistance = agentPos.DistanceSquared(pOther->GetPosition());
				if (currentDistance < closestDistSq)
				{
					closestDistSq = currentDistance;
					pClosestEnemy = pOther;
				}
			}
		}

		if (pClosestEnemy == nullptr)
		{
			return false;
		}

		if (agentPos.DistanceSquared(pClosestEnemy->GetPosition()) < fleeRadius * fleeRadius)
		{
			DEBUGRENDERER2D->DrawCircle(pClosestEnemy->GetPosition(), pClosestEnemy->GetRadius() + 1, Elite::Color{ 1,0,0 }, 0.1f);
			return true;
		}
		return false;
	}

	bool IsSmallEnemyNearby(Elite::Blackboard* pBlackboard)
	{
		std::vector<AgarioAgent*>* pAgentVec;
		AgarioAgent* pMainAgent;

		if (pBlackboard->GetData("Agent", pMainAgent) == false)
		{
			return false;
		}

		if (pBlackboard->GetData("AgentsVec", pAgentVec) == false)
		{
			return false;
		}

		if (pMainAgent == nullptr || pAgentVec == nullptr)
		{
			return false;
		}

		const float searchRadius{ pMainAgent->GetRadius() * 2.f + 25.f };
		DEBUGRENDERER2D->DrawCircle(pMainAgent->GetPosition(), searchRadius, Elite::Color{ 0,1,0 }, 0);

		//lamda stuff
		Elite::Vector2 agentPos{ pMainAgent->GetPosition() };

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
}

#endif