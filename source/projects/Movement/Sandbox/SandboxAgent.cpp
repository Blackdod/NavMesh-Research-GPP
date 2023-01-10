#include "stdafx.h"
#include "SandboxAgent.h"

using namespace Elite;

SandboxAgent::SandboxAgent(): BaseAgent()
{
	m_Target = GetPosition();
}

void SandboxAgent::Update(float dt)
{

	const float maxSpeed{ 50.f };
	const float arrivalRadius{ 1.f }; //Stops trying to reach target --> no twitching
	const float slowRadius{ 10.f };

	Elite::Vector2 toTarget = m_Target - GetPosition();
	float distanceSquared = toTarget.MagnitudeSquared();

	if(distanceSquared < arrivalRadius * arrivalRadius)
	{
		SetLinearVelocity({ 0.f,0.f });
		return; 
	}


	auto velocity = toTarget;
	velocity.Normalize();
	if(distanceSquared < slowRadius * slowRadius)
	{
		velocity *= maxSpeed * distanceSquared / (slowRadius * slowRadius); 
	}
	else
	{
		velocity *= maxSpeed;
	}

	SetLinearVelocity(velocity);

	//Orientation
	AutoOrient();
}

void SandboxAgent::Render(float dt)
{
	BaseAgent::Render(dt); //Default Agent Rendering
}

void SandboxAgent::AutoOrient()
{
	//Determine angle based on direction
	Vector2 velocity = GetLinearVelocity();
	if (velocity.Magnitude() > 0)
	{
		velocity.Normalize();
		SetRotation(atan2(velocity.y, velocity.x) + E_PI_2);
	}

	SetRotation(GetRotation() + E_PI_2);
}