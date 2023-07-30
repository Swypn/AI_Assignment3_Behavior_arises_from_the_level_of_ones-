#include "level.h"
#include "raymath.h"
#include <string>
#include <iostream>

void CollectableSquare::setupBehaviourTree(Level* level)
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
	if(collected)
	{
		level->score++;
		dead = true;
	}
}

void CollectableSquare::draw()
{
	DrawRectangle(position1.x, position1.y, size.x, size.y, BLACK);
}


void CollectorTriangle::setupBehaviourTree(Level* level)
{
	SelectorNode* rootNode = new SelectorNode();
	//Evade distractor sequence

	SequenceNode* evadeDistractorSequence = new SequenceNode();
	ActionNode* detectDistractorNode = new ActionNode([this, level](Agent* agent) {
		return this->detectDistractor(agent, level);
		});

	ActionNode* evadeDistractorNode = new ActionNode([this](Agent* agent) {
		return this->evadeDistractor(agent);
		});

	evadeDistractorSequence->addChild(detectDistractorNode);
	evadeDistractorSequence->addChild(evadeDistractorNode);
	
	// Search and move to item sequence
	SequenceNode* searchAndMoveToItemSequence = new SequenceNode();

	ActionNode* FindItem = new ActionNode([this](Agent* agent) {
		return this->searchForItem(agent);
		});

	ActionNode* moveToItem = new ActionNode([this](Agent* agent) {
		return this->moveToItem(agent);
		});

	searchAndMoveToItemSequence->addChild(FindItem);
	searchAndMoveToItemSequence->addChild(moveToItem);
	
	rootNode->addChild(evadeDistractorSequence);
	rootNode->addChild(searchAndMoveToItemSequence);
	
	this->behaviorTree = rootNode;
}

void CollectorTriangle::sense(Level* level)
{
}

void CollectorTriangle::sense(Level* level, std::list<CollectableSquare>& squares)
{
	// Reset closest square distance and reference
	float closestSquareDistance = std::numeric_limits<float>::max();
	CollectableSquare* closestSquare = nullptr;

	// Find closest square
	for (CollectableSquare& square : squares)
	{
		float distance = Vector2Distance(this->position1, square.position1);
		if (distance < closestSquareDistance)
		{
			closestSquareDistance = distance;
			closestSquare = &square;
		}

		if (Vector2Distance(this->position1, square.position1) < 50.0) {
			square.collected = true;
			std::cout << "collected true" << std::endl;
			//score++;
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
	if (targetAquired) {
		moveToItem(this);
		
	}
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
		Vector2 newPosition = Vector2Add(center, Vector2Scale(direction, speed * GetFrameTime()));

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

bool CollectorTriangle::detectDistractor(Agent* agent, Level* level)
{
	for (auto& circle : level->circle_agents) {
		float distance = Vector2Distance(center, circle.position1);
		if (distance < 100.0f) { // Change this value to adjust the detection radius
			distractorPosition = circle.position1;
			return true;
		}
	}
	return false;
}

bool CollectorTriangle::evadeDistractor(Agent* agent)
{
	if (Vector2Length(Vector2Subtract(position1, distractorPosition)) < 100.0f)
	{
		Vector2 evadeDirection = Vector2Normalize(Vector2Subtract(position1, distractorPosition));
		Vector2 newPosition = Vector2Add(position1, Vector2Scale(evadeDirection, speed * GetFrameTime()));
		position1 = newPosition;
		return true;
	}
	return false;
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

void GuardianRectangle::setupBehaviourTree(Level* level)
{
	SelectorNode* rootNode = new SelectorNode();

	SequenceNode* protectCollectorSequence = new SequenceNode();

	ActionNode* detectDistractorNode = new ActionNode([this, level](Agent* agent) {
		return this->detectDistractor(agent, level);
		});

	ActionNode* chaseAwayDistractorNode = new ActionNode([this](Agent* agent) {
		return this->chaseAwayDistractor(agent);
		});

	protectCollectorSequence->addChild(detectDistractorNode);
	protectCollectorSequence->addChild(chaseAwayDistractorNode);

	rootNode->addChild(protectCollectorSequence);
	
	this->behaviorTree = rootNode;
}

void GuardianRectangle::sense(Level* level)
{

}

void GuardianRectangle::decide()
{
	behaviorTree->execute(this);
}

void GuardianRectangle::act(Level* level)
{
	if(targetAquired)
	{
		chaseAwayDistractor(this);
	}
}

void GuardianRectangle::draw()
{
	DrawRectangle(position1.x, position1.y, size.x, size.y, BLUE);
}

bool GuardianRectangle::detectDistractor(Agent* agent, Level* level)
{
	for (auto& circle : level->circle_agents) {
		float distance = Vector2Distance(position1, circle.position1);
		if (distance < 1000.0f) { // Change this value to adjust the detection radius
			distractorPosition = circle.position1;
			targetAquired = true;
			return true;
		}
	}
	targetAquired = false;
	return false;
}

bool GuardianRectangle::chaseAwayDistractor(Agent* agent)
{
	if (Vector2Length(Vector2Subtract(position1, distractorPosition)) < 1000.0f)
	{
		Vector2 chaseDirection = Vector2Normalize(Vector2Subtract(distractorPosition, position1));
		Vector2 newPosition = Vector2Add(position1, Vector2Scale(chaseDirection, speed * GetFrameTime()));
		position1 = newPosition;
		return true;
	}
	return false;
}

void DistractorCircle::setupBehaviourTree(Level* level)
{
	SelectorNode* rootNode = new SelectorNode();

	// avoid guardian
	SequenceNode* avoidGuardianSequence = new SequenceNode();

	ActionNode* checkGuardianNode = new ActionNode([this, level](Agent* agent)
		{
			return this->detectGuardian(this, level);
		});

	ActionNode* avoidGuardianNode = new ActionNode([this](Agent* agent)
		{
			return this->evadeGuardian(this);
		});

	avoidGuardianSequence->addChild(checkGuardianNode);
	avoidGuardianSequence->addChild(avoidGuardianNode);

	// move between sqaure and rectangle
	SequenceNode* moveBetweenSequence = new SequenceNode();

	ActionNode* checkCollectorNearSquareNode = new ActionNode([this, level](Agent* agent) {
		return this->isCollectorNearSquare(agent, level);
		});

	ActionNode* moveBetweenCollectorAndSquareNode = new ActionNode([this](Agent* agent) {
		return this->moveBetweenCollectorAndSquare(agent);
		});

	moveBetweenSequence->addChild(checkCollectorNearSquareNode);
	moveBetweenSequence->addChild(moveBetweenCollectorAndSquareNode);

	rootNode->addChild(avoidGuardianSequence);
	rootNode->addChild(moveBetweenSequence);

	this->behaviorTree = rootNode;
}

void DistractorCircle::sense(Level* level)
{
}

void DistractorCircle::decide()
{
	behaviorTree->execute(this);
}

void DistractorCircle::act(Level* level)
{
	if(targetAquired)
	{
		moveBetweenCollectorAndSquare(this);
	}
}

void DistractorCircle::draw()
{
	DrawCircle(position1.x, position1.y, radius, RED);
}

bool DistractorCircle::isCollectorNearSquare(Agent* agent, Level* level)
{
	for (auto& collectorTriangle : level->triangle_agents)
	{
		for(auto& collectableSquare : level->square_agents)
		{
			if(!collectableSquare.collected && Vector2Distance(collectorTriangle.position1, collectableSquare.position1) < 100.0f)
			{
				sqaureTarget = &collectableSquare;
				triangleTarget = &collectorTriangle;
				targetAquired = true;
				return true;
			}
		}
	}
	targetAquired = false;
	return false;
}

bool DistractorCircle::moveBetweenCollectorAndSquare(Agent* agent)
{
	if(triangleTarget && sqaureTarget)
	{
		Vector2 targetPosition = Vector2Scale(Vector2Add(triangleTarget->position1, sqaureTarget->position1), 0.5f);
		Vector2 direction = Vector2Normalize(Vector2Subtract(targetPosition, position1));
		position1 = Vector2Add(position1, Vector2Scale(direction, speed * GetFrameTime()));
		return true;
	}
	return false;
}

bool DistractorCircle::detectGuardian(Agent* agent, Level* level)
{
	for (auto& rectangle : level->rectangle_agents) {
		float distance = Vector2Distance(position1, rectangle.position1);
		if (distance < 100.0f) { // Change this value to adjust the detection radius
			guardianPosition = rectangle.position1;
			return true;
		}
	}
	return false;
}

bool DistractorCircle::evadeGuardian(Agent* agent)
{
	if (Vector2Length(Vector2Subtract(position1, guardianPosition)) < 100.0f)
	{
		Vector2 evadeDirection = Vector2Normalize(Vector2Subtract(position1, guardianPosition));
		Vector2 newPosition = Vector2Add(position1, Vector2Scale(evadeDirection, speed * GetFrameTime()));
		position1 = newPosition;
		return true;
	}
	return false;
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
	int squareCount = 10;

	setupCollectableSquares(squareCount);

	auto triangle = spawn_agent(CollectorTriangle());
	triangle->position1 = { 100, 200 };
	triangle->position2 = {150, 200};
	triangle->position3 = { 125, 150 };
	triangle->size = {50, 50};
	triangle->setupBehaviourTree(this);

	auto rectangle = spawn_agent(GuardianRectangle());
	rectangle->position1 = { 200, 200 };
	rectangle->size = { 50, 50 };
	rectangle->setupBehaviourTree(this);

	auto circle = spawn_agent(DistractorCircle());
	circle->position1 = {300, 200};
	circle->radius = 25.0f;
	circle->setupBehaviourTree(this);

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
	
	bool allCollected = true;
	for (auto& square : square_agents) {
		if (!square.collected) {
			allCollected = false;
			break;
		}
	}

	if (allCollected) {
		for (auto& square : square_agents) {
			square.collected = false;
			
			// reset position or any other properties
		}
		int squareCount = 10;
		setupCollectableSquares(squareCount);
	}
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


	std::string scoreString = "Score: " + std::to_string(score);

	DrawText(scoreString.c_str(), 10, 10, 20, PURPLE);
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
