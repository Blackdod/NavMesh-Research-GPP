/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// StatesAndTransitions.h: Implementation of the state/transition classes
/*=============================================================================*/
#ifndef ELITE_APPLICATION_FSM_STATES_TRANSITIONS
#define ELITE_APPLICATION_FSM_STATES_TRANSITIONS

#include "projects/Shared/Agario/AgarioAgent.h"
#include "projects/Shared/Agario/AgarioFood.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
#include "framework/EliteAI/EliteData/EBlackboard.h"

//------------
//---STATES---
//------------
namespace FSMStates
{
	class WanderState : public Elite::FSMState
	{
	public:
		WanderState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class SeekFoodState : public Elite::FSMState
	{
	public:
		SeekFoodState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class FleeEnemeyState : public Elite::FSMState
	{
	public:
		FleeEnemeyState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};

	class SeekTargetState : public Elite::FSMState
	{
	public:
		SeekTargetState() : FSMState() {};
		virtual void OnEnter(Elite::Blackboard* pBlackboard) override;
	};
}



//-----------------
//---TRANSITIONS---
//-----------------

namespace FSMConditions
{
	class FoodNearbyCondition : public Elite::FSMCondition
	{
	public:
		FoodNearbyCondition() : FSMCondition() {};


		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;

	};

	class FoodEaten : public Elite::FSMCondition
	{
	public:
		FoodEaten() : FSMCondition() {};


		// Inherited via FSMCondition
		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;

	};

	class BigEnemyNearby : public Elite::FSMCondition
	{
	public:
		BigEnemyNearby() : FSMCondition() {};

		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class NoBigEnemyNearby : public Elite::FSMCondition
	{
	public:
		NoBigEnemyNearby() : FSMCondition() {};

		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class SmallEnemyNearby : public Elite::FSMCondition
	{
	public:
		SmallEnemyNearby() : FSMCondition() {};

		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};

	class NoSmallEnemyNearby : public Elite::FSMCondition
	{
	public:
		NoSmallEnemyNearby() : FSMCondition() {};

		virtual bool Evaluate(Elite::Blackboard* pBlackboard) const override;
	};
}
#endif