#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	const Elite::Vector2 targetPos{ m_pFlock->GetAverageNeighborPos() };
	SetTarget(targetPos);
	SteeringOutput steering{ Seek::CalculateSteering(deltaT, pAgent) };

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), pAgent->GetDirection(), 7, { 0, 1, 1 }, 0.40f);
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), 15.f, { 0,1,0 }, 0.40f);
		//for(const auto& pNeighbor : m_pFlock->GetNeighbors())
		//{
		//	DEBUGRENDERER2D->DrawCircle(pNeighbor->GetPosition(), 1.f, { 1,0,0 }, 0.40f);
		//}
		//DEBUGRENDERER2D->DrawCircle(m_pFlock->GetAverageNeighborPos(), 1.f, { 1,0,0 }, 0.40f);
	}

	return steering;
}

//*********************
//SEPARATION (FLOCKING)

SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	const std::vector<SteeringAgent*> m_pNeighbors = m_pFlock->GetNeighbors();
	for (int i{}; i < m_pFlock->GetNrOfNeighbors(); ++i)
	{
		SteeringOutput tempSteering{};
		const Elite::Vector2 targetPos{ m_pNeighbors[i]->GetPosition() };
		SetTarget(targetPos);
		const Elite::Vector2 vectorBetweenAgentAndNeighbor{ pAgent->GetPosition() - m_pNeighbors[i]->GetPosition() };
		const float magnitudeSquared = vectorBetweenAgentAndNeighbor.MagnitudeSquared();
		tempSteering = Flee::CalculateSteering(deltaT, pAgent);
		tempSteering.LinearVelocity /= magnitudeSquared;
		steering.LinearVelocity += tempSteering.LinearVelocity;
		steering.AngularVelocity += tempSteering.AngularVelocity;
	}

	//Vector from every neighbor, get magnitudes
	//Divide vector by magnitudeSquared -> impact stuff with distance

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 7, { 0, 1, 1 }, 0.40f);
		//DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), 15.f, { 0,1,0 }, 0.40f);
	}

	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	return steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)

SteeringOutput VelocityMatch::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	const Elite::Vector2 averageVel{ m_pFlock->GetAverageNeighborVelocity() };
	const Elite::Vector2 targetPos{ pAgent->GetPosition() };
	SetTarget(targetPos + averageVel);
	SteeringOutput steering{ Seek::CalculateSteering(deltaT, pAgent) };

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 7, { 0, 1, 1 }, 0.40f);
		//DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), 15.f, { 0,1,0 }, 0.40f);
	}

	return steering;
}