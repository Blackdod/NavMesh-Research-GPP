/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
// Authors: Andries Geens
/*=============================================================================*/
// AgarioAgent.h: Agent owned and updated by the agariogame. 
//Inherits from steeringagent
//Use SetDecisionMaking to set a specific behavior for the agent
/*=============================================================================*/
#ifndef ELITE_AGARIO_AGENT
#define ELITE_AGARIO_AGENT

#include "projects/Movement/SteeringBehaviors/SteeringAgent.h"
#include "projects/Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"

class AgarioAgent : public SteeringAgent
{
public:
	//--- Constructor & Destructor ---
	AgarioAgent(Elite::Vector2 pos, float worldSize);
	AgarioAgent(Elite::Vector2 pos, Elite::Color color, float worldSize);
	AgarioAgent(Elite::Vector2 pos, Elite::Color color);
	AgarioAgent(Elite::Vector2 pos);
	virtual ~AgarioAgent();

	//--- WorldSize ---
	const float m_WorldSize{};

	//--- Agent Functions ---
	virtual void Update(float dt) override;
	virtual void Render(float dt) override;

	//-- Agario Functions --
	void MarkForUpgrade(float amountOfFood  = 1.0f );
	void MarkForDestroy();
	bool CanBeDestroyed();
	void SetDecisionMaking(Elite::IDecisionMaking* decisionMakingStructure);
	
	void SetToWander();
	void SetToSeek(Elite::Vector2 seekPos);
	void SetToFlee(Elite::Vector2 seekPos);
	void SetToFlee(Elite::Vector2 seekPos, float fleeRadius);
	void SetToEvade(const TargetData& target);
	void SetToEvade(const TargetData& target, float fleeRadius);
	void SetToPursuit(const TargetData& target);

private:
	Elite::IDecisionMaking* m_DecisionMaking = nullptr;
	float m_ToUpgrade = 0.0f;
	bool m_ToDestroy = false;
	float m_SpeedBase = 25.f;

	ISteeringBehavior* m_pWander = nullptr;
	ISteeringBehavior* m_pSeek = nullptr;
	ISteeringBehavior* m_pFlee = nullptr;
	Pursuit* m_pPursuit = nullptr;
	Evade* m_pEvade = nullptr;
	
private:
	void OnUpgrade(float amountOfFood);

private:
	//C++ make the class non-copyable
	AgarioAgent(const AgarioAgent&) {};
	AgarioAgent& operator=(const AgarioAgent&) {};
};
#endif

