//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"

#include <iostream>

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if(pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, {0.f,1,0});
	}

	return steering;
}

SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	const Elite::Vector2 toTarget{ m_Target.Position - pAgent->GetPosition() };
	const float distanceSquared{ toTarget.MagnitudeSquared() };

	if(distanceSquared < m_FleeRadius * m_FleeRadius)
	{
		steering.LinearVelocity = pAgent->GetPosition() - m_Target.Position;
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	}

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0.f,1,0 });
		DEBUGRENDERER2D->DrawCircle(m_Target.Position, m_FleeRadius, { 0,1,0 },0);
	}

	return steering;
}

SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();

	const Elite::Vector2 toTarget{ m_Target.Position - pAgent->GetPosition() };
	const float distanceSquared{ toTarget.MagnitudeSquared() };

	if (distanceSquared < m_StopRadius * m_StopRadius)
	{
		steering.LinearVelocity = Elite::Vector2{ 0.f,0.f };
		return steering;
	}

	if(distanceSquared < m_SlowRadius * m_SlowRadius)
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() * distanceSquared / (m_SlowRadius * m_SlowRadius);
	}
	else
	{
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	}

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0.f,1,0 });
	}

	return steering;
}

SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	pAgent->SetAutoOrient(false);

	const Elite::Vector2 desiredVector = Elite::Vector2( m_Target.Position - pAgent->GetPosition() );
	const Elite::Vector2 lookVector{ std::cosf(pAgent->GetRotation()),std::sinf(pAgent->GetRotation()) };

	float angle{ Elite::AngleBetween(desiredVector, lookVector) };

	if (!(fabsf(Elite::AngleBetween(lookVector, desiredVector)) < 0.1f))
	{
		if (Elite::AngleBetween(lookVector, desiredVector) > 0)
		{
			steering.AngularVelocity = pAgent->GetMaxAngularSpeed();
		}
		else
		{
			steering.AngularVelocity = -pAgent->GetMaxAngularSpeed();
		}
		
	}

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), lookVector, 5, {0.f,1,0});
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), desiredVector, 5, { 0.f,1,0 });
	}

	return steering;
}

SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	b2CircleShape wanderCircle{};
	//const Elite::Vector2 directionVector{ pAgent->GetPosition() - m_Target.Position}; //desired direction
	wanderCircle.m_p = pAgent->GetDirection().GetNormalized() * m_OffsetDistance + pAgent->GetPosition(); //circle center
	wanderCircle.m_radius = m_Radius;

	const float addedAngle{ Elite::randomFloat(-m_MaxAngleChange, m_MaxAngleChange) }; //random angle withing maxAngle interval

	m_WanderAngle += addedAngle; //add direction change
	//std::cout << m_WanderAngle << '\n';

	const Elite::Vector2 targetPoint{ wanderCircle.m_p + Elite::Vector2{cosf(m_WanderAngle), sinf(m_WanderAngle)}.GetNormalized() * m_Radius}; //calculate target point

	m_Target = targetPoint;
	steering = Seek::CalculateSteering(deltaT, pAgent);

	if (pAgent->CanRenderBehavior())
	{
		//DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), directionVector, 5, { 0.f,1,0 });
		DEBUGRENDERER2D->DrawPoint(targetPoint, 5, { 0.f,1,0 });
		DEBUGRENDERER2D->DrawCircle(Elite::Vector2{ wanderCircle.m_p }, m_Radius, {0,0,1}, 0);
	}
	return steering;
}

SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	const Elite::Vector2 nextTargetPos{ m_Target.Position + m_Target.LinearVelocity * deltaT };
	const float speedDiff = m_Target.LinearVelocity.Magnitude() / pAgent->GetMaxLinearSpeed(); //not used

	Seek::SetTarget(nextTargetPos);
	steering = Seek::CalculateSteering(deltaT, pAgent);
	

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0.f,1,0 });
	}

	return steering;
}

SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	const Elite::Vector2 nextTargetPos{ m_Target.Position + m_Target.LinearVelocity * deltaT };
	const Elite::Vector2 toTarget = nextTargetPos - pAgent->GetPosition();
	float distanceSquared = toTarget.MagnitudeSquared();
	SteeringOutput steering = {};

	if(distanceSquared < m_FleeRadius * m_FleeRadius)
	{
		const Elite::Vector2 speedDiff = m_Target.LinearVelocity / pAgent->GetMaxLinearSpeed(); //not used

		SetTarget(nextTargetPos);
		steering = Flee::CalculateSteering(deltaT, pAgent);
		steering.IsValid = true;
	}
	else
	{
		steering.IsValid = false;
	}

	if (pAgent->CanRenderBehavior())
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, 5, { 0.f,1,0 });
		//DEBUGRENDERER2D->DrawCircle(nextTargetPos, m_FleeRadius, { 1,0,0 }, 0);
	}

	return steering;
}