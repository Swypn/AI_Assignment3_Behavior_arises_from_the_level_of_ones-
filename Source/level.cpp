#include "level.h"
#include "raymath.h"
#include <string>


void CollectableSquare::setupBehaviourTree()
{
}

void CollectableSquare::sense(Level* level)
{
}

void CollectableSquare::decide()
{
}

void CollectableSquare::act(Level* level)
{
}

void CollectableSquare::draw()
{
	DrawRectangle(position1.x, position1.y, size.x, size.y, BLACK);
}


void CollectorTriangle::setupBehaviourTree()
{
	SequenceNode* searchAndMoveToItem = new SequenceNode();


	ActionNode* FindItem = new ActionNode([this](Agent* agent) {
		return this->searchForItem(agent);
		});

	ActionNode* moveToItem = new ActionNode([this](Agent* agent) {
		return this->moveToItem(agent);
		});

	searchAndMoveToItem->addChild(FindItem);
	searchAndMoveToItem->addChild(moveToItem);

	this->behaviorTree = searchAndMoveToItem;
}

void CollectorTriangle::sense(Level* level)
{
}

void CollectorTriangle::sense(Level* level, std::list<CollectableSquare>& squares)
{
	// Reset closest square distance and reference
	float closestSquareDist = std::numeric_limits<float>::max();
	CollectableSquare* closestSquare = nullptr;

	// Find closest square
	for (CollectableSquare& square : squares)
	{
		float dist = Vector2Distance(this->position1, square.position1);
		if (dist < closestSquareDist)
		{
			closestSquareDist = dist;
			closestSquare = &square;
		}
	}

	// Store closest square
	this->itemPosition = closestSquare ? closestSquare->position1 : Vector2{};
	this->itemInSight = closestSquare != nullptr;
}

void CollectorTriangle::decide()
{
	behaviorTree->execute(this);
}

void CollectorTriangle::act(Level* level)
{
}

void CollectorTriangle::draw()
{
	position1 = { center.x, center.y - size.y / 2 };
	position2 = { center.x - size.x / 2, center.y + size.y / 2 };
	position3 = { center.x + size.x / 2, center.y + size.y / 2 };

	// If you want to rotate the triangle according to the orientation:
	position1 = Vector2Rotate(position1, orientation);
	position2 = Vector2Rotate(position2, orientation);
	position3 = Vector2Rotate(position3, orientation);

	DrawTriangle(position1, position2, position3, GOLD);
}

bool CollectorTriangle::searchForItem(Agent* agent)
{

	if (itemInSight) {
		targetPosition = itemPosition;
		targetAquired = true;
	}
	return itemInSight;
}

bool CollectorTriangle::moveToItem(Agent* agent)
{
	if (targetAquired) {
		Vector2 direction = Vector2Subtract(targetPosition, center);
		direction = Vector2Normalize(direction);
		Vector2 lerpTarget = Vector2Lerp(center, targetPosition, speed * GetFrameTime());
		Vector2 newPosition = Vector2Add(center, Vector2Scale(direction, Vector2Distance(center, lerpTarget)));

		// bounds checking here
		if (newPosition.x < 0) newPosition.x = 0;
		else if (newPosition.x > GetScreenWidth()) newPosition.x = GetScreenWidth();

		if (newPosition.y < 0) newPosition.y = 0;
		else if (newPosition.y > GetScreenHeight()) newPosition.y = GetScreenHeight();

		center = newPosition;

		// update vertices after center
		position1 = { center.x, center.y - size.y / 2 };
		position2 = { center.x - size.x / 2, center.y + size.y / 2 };
		position3 = { center.x + size.x / 2, center.y + size.y / 2 };
	}
	return targetAquired;
}

Vector2 CollectorTriangle::Vector2Rotate(Vector2 point, float rad)
{
	float s = sin(rad);
	float c = cos(rad);

	// Translate point back to origin
	point.x -= center.x;
	point.y -= center.y;

	// Rotate point
	float xnew = point.x * c - point.y * s;
	float ynew = point.x * s + point.y * c;

	// Translate point back
	Vector2 newPoint = { xnew + center.x, ynew + center.y };
	return newPoint;
}

Vector2 CollectorTriangle::Vector2Lerp(Vector2 a, Vector2 b, float t)
{
	Vector2 Lerp = { a.x + t * (b.x - a.x), a.y + t * (b.y - a.y) };
	return Lerp;
}

void GuardianRectangle::setupBehaviourTree()
{
}

void GuardianRectangle::sense(Level* level)
{
}

void GuardianRectangle::decide()
{
	//behaviorTree->execute(this);
}

void GuardianRectangle::act(Level* level)
{
}

void GuardianRectangle::draw()
{
	DrawRectangle(position1.x, position1.y, size.x, size.y, BLUE);
}

void DistractorCircle::setupBehaviourTree()
{
}

void DistractorCircle::sense(Level* level)
{
}

void DistractorCircle::decide()
{
	//behaviorTree->execute(this);
}

void DistractorCircle::act(Level* level)
{
}

void DistractorCircle::draw()
{
	DrawCircle(position1.x, position1.y, radius, RED);
}


Agent* Level::get_agent(int id)
{
	auto agent_iterator = id_to_agent.find(id);
	if(agent_iterator != id_to_agent.end())
	{
		return agent_iterator->second;
	}

	return nullptr;
}

Agent* Level::spawn_agent(CollectableSquare agent)
{
	Agent* result = nullptr;

	square_agents.push_back(agent);
	result = &square_agents.back();

	pending_agents.push_back(result);
	
	return result;
}

Agent* Level::spawn_agent(CollectorTriangle agent)
{
	Agent* result = nullptr;

	triangle_agents.push_back(agent);
	result = &triangle_agents.back();

	pending_agents.push_back(result); // Enqueue it so that it can be added to the level at the beginning of the next frame

	return result;
}

Agent* Level::spawn_agent(GuardianRectangle agent)
{
	Agent* result = nullptr;

	rectangle_agents.push_back(agent);
	result = &rectangle_agents.back();

	pending_agents.push_back(result); // Enqueue it so that it can be added to the level at the beginning of the next frame

	return result;
}

Agent* Level::spawn_agent(DistractorCircle agent)
{
	Agent* result = nullptr;

	circle_agents.push_back(agent);
	result = &circle_agents.back();

	pending_agents.push_back(result); // Enqueue it so that it can be added to the level at the beginning of the next frame

	return result;
}

//Agent* Level::spawn_agent(Tree agent)
//{
//	Agent* result = nullptr;
//
//	tree_agents.push_back(agent);
//	result = &tree_agents.back();
//
//	pending_agents.push_back(result); // Enqueue it so that it can be added to the level at the beginning of the next frame
//
//	return result;
//}

void Level::remove_dead_and_add_pending_agents()
{
	auto agent_iterator = all_agents.begin();
	while(agent_iterator != all_agents.end())
	{
		if((*agent_iterator)->dead)
		{
			id_to_agent.erase((*agent_iterator)->id);
			agent_iterator = all_agents.erase(agent_iterator);
		} else {
			agent_iterator++;
		}
	}

	// This must happen _after_ we remove agents from the vector 'all_agents'.
	// @AddMoreHere
	square_agents.remove_if([](CollectableSquare& a) {return a.dead; });
	triangle_agents.remove_if([](CollectorTriangle& a) {return a.dead; });
	rectangle_agents.remove_if([](GuardianRectangle& a) {return a.dead; });
	circle_agents.remove_if([](DistractorCircle& a) {return a.dead;  });

	// Add all pending agents
	for(Agent* agent: pending_agents)
	{
		last_id += 1;
		agent->id = last_id;

		all_agents.push_back(agent);
		id_to_agent.insert({agent->id, agent});
	}

	pending_agents.clear(); // Important that we clear this, otherwise we'll add the same agents every frame.
}

void Level::reset()
{
	// TODO: Implement this yourself, clear all lists and vectors, after that spawn agents

    // this is here just as an example.
    // You should also replace "SillyAgent", that is also just an example.
	int squareCount = 10;

	setupCollectableSquares(squareCount);

	auto triangle = spawn_agent(CollectorTriangle());
	triangle->position1 = { 100, 200 };
	triangle->position2 = {150, 200};
	triangle->position3 = { 125, 150 };
	triangle->size = {50, 50};
	triangle->setupBehaviourTree();

	auto rectangle = spawn_agent(GuardianRectangle());
	rectangle->position1 = { 200, 200 };
	rectangle->size = { 50, 50 };
	rectangle->setupBehaviourTree();

	auto circle = spawn_agent(DistractorCircle());
	circle->position1 = {300, 200};
	circle->radius = 25.0f;
	circle->setupBehaviourTree();


}

void Level::update()
{
	remove_dead_and_add_pending_agents();
	currentTime += GetFrameTime();

	if (currentTime > tickTimer) 
	{
		for (auto& collectableSquare : square_agents)
		{
			collectableSquare.sense(this);
			collectableSquare.decide();
		}

		for (auto& collectorTriangle : triangle_agents)
		{
			collectorTriangle.sense(this, square_agents);
			collectorTriangle.decide();
		}

		for (auto& guardianRectangle : rectangle_agents)
		{
			guardianRectangle.sense(this);
			guardianRectangle.decide();
		}

		for (auto& distractorCircle : circle_agents)
		{
			distractorCircle.sense(this);
			distractorCircle.decide();
		}
		currentTime = 0.0f;
	}

	for (auto& collectableSquare : square_agents)
	{
		collectableSquare.act(this);
	}

	for (auto& collectorTriangle : triangle_agents)
	{
		collectorTriangle.act(this);
	}

	for (auto& guardianRectangle : rectangle_agents)
	{
		guardianRectangle.act(this);
	}

	for (auto& distractorCircle : circle_agents)
	{
		distractorCircle.act(this);
	}
	
	//for(auto& agent : all_agents)
	//{
	//	if(agent->energy > 0)
	//	{
	//		agent->energy -= GetFrameTime();
	//	} else {
	//		agent->dead = true;
	//	}
	//	// TODO: This piece of code needs to be changed to make sure that sense, decide, act, happen at different frequencies.
	//	agent->sense(this);
	//	agent->decide();
	//	agent->act(this);
	//}
}

void Level::draw()
{
	for (auto& collectableSquare : square_agents)
	{
		collectableSquare.draw();
	}

	for(auto& collectorTriangle : triangle_agents)
	{
		collectorTriangle.draw();
	}

	for (auto&	guardianRectangle : rectangle_agents)
	{
		guardianRectangle.draw();
	}
	
	for (auto& distractorCircle : circle_agents)
	{
		distractorCircle.draw();
	}

}

void Level::setupCollectableSquares(int count)
{
	std::srand(std::time(nullptr));

	for (int i = 0; i < count; ++i) {
		CollectableSquare square;
		square.position1 = { static_cast<float>(std::rand() % GetScreenWidth()), static_cast<float>(std::rand() % GetScreenHeight()) }; // Random position
		square.size = { 25, 25 }; // Fixed size for example, you can randomize this as well
		spawn_agent(square);
	}
}
