#include "level.h"
#include "raymath.h"
#include <string>
#include <iostream>


void PowerUpSqaure::setupBehaviourTree(Level* level)
{
}

void PowerUpSqaure::sense(Level* level)
{
}

void PowerUpSqaure::decide()
{
}

void PowerUpSqaure::act(Level* level)
{
	if (collected)
	{
		std::cout << "power up is dead" << std::endl;
		dead = true;
	}
}

void PowerUpSqaure::applyEffect()
{
}

void PowerUpSqaure::endEffect()
{
}

void PowerUpSqaure::draw()
{
	DrawRectangle((int)position1.x, (int)position1.y, (int)size.x, (int)size.y, ORANGE);
}


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
	DrawRectangle((int)position1.x, (int)position1.y, (int)size.x, (int)size.y, BLACK);
}

void CollectableSquare::applyEffect()
{
}

void CollectableSquare::endEffect()
{
}

void CollectorTriangle::setupBehaviourTree(Level* level)
{
	SelectorNode* rootNode = new SelectorNode();
	
	// Search and move to power up
	SequenceNode* searchAndMoveToPowerUpSequence = new SequenceNode;

	ActionNode* findPowerUp = new ActionNode([this, level](Agent* agent) {
		return this->searchForPowerUp(agent, level);
		});

	ActionNode* moveToPowerUp = new ActionNode([this](Agent* agent){
		return this->moveToPowerUp(this);
		});

	searchAndMoveToPowerUpSequence->addChild(findPowerUp);
	searchAndMoveToPowerUpSequence->addChild(moveToPowerUp);

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

	ActionNode* FindItem = new ActionNode([this, level](Agent* agent) {
		return this->searchForItem(agent, level);
		});

	ActionNode* moveToItem = new ActionNode([this](Agent* agent) {
		return this->moveToItem(agent);
		});

	searchAndMoveToItemSequence->addChild(FindItem);
	searchAndMoveToItemSequence->addChild(moveToItem);
	
	// Adding root order

	rootNode->addChild(searchAndMoveToPowerUpSequence);
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
			std::cout << "collected score sqaure" << std::endl;
			//score++;
		}
	}

	// Store closest square
	this->itemPosition = closestSquare ? closestSquare->position1 : Vector2{};
	this->itemInSight = closestSquare != nullptr;

	// Find powerUp
	for (auto& powerUp : level->powerUp_agents) {
		float distance = Vector2Distance(center, powerUp.position1);
		if (distance < 100.0f) { // This value determines how close the power-up should be to prioritize it
			powerUpPosition = powerUp.position1;
			powerUpSqaure = &powerUp;
		}

		if (Vector2Distance(this->position1, powerUp.position1) < 50.0) {
			powerUp.collected = true;
			std::cout << "collected powerUp" << std::endl;
			//score++;
		}
	}
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
	else if(!targetAquired)
	{
		evadeDistractor(this);
	}

	if(isPowerUpCollected && powerUpSqaure)
	{
		powerUpSqaure->collected = true;
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

void CollectorTriangle::applyEffect()
{
	speed = 150.0f;
}

void CollectorTriangle::endEffect()
{
	speed = defualtSpeed;
}

bool CollectorTriangle::searchForItem(Agent* agent, Level* level)
{

	if (itemInSight && shouldCollect) {

		float distanceToDistractor = Vector2Distance(itemPosition, distractorPosition);

		if(distanceToDistractor < 100.0f)
		{
			targetPosition = findNewTarget(this, level);
		} 
		else 
		{
			targetPosition = itemPosition;
		}
		targetAquired = true;
	}
	else 
	{
		targetAquired = false;
	}
	
	return itemInSight;
}

Vector2 CollectorTriangle::findNewTarget(Agent* agent, Level* level)
{
	Vector2 newTarget = itemPosition;
	float maxDistance = Vector2Distance(itemPosition, distractorPosition);

	for(auto& square : level->collectableSquare_agents)
	{
		float distance = Vector2Distance(square.position1, distractorPosition);
		if(distance > maxDistance)
		{
			maxDistance = distance;
			newTarget = square.position1;
		}
	}

	return newTarget;
}

bool CollectorTriangle::moveToItem(Agent* agent)
{
	if (targetAquired) {
		Vector2 direction = Vector2Subtract(targetPosition, center);
		direction = Vector2Normalize(direction);
		Vector2 newPosition = Vector2Add(center, Vector2Scale(direction, speed * GetFrameTime()));

		// bounds checking here
		if (newPosition.x < 0) newPosition.x = 0;
		else if (newPosition.x > (float)GetScreenWidth()) newPosition.x = (float)GetScreenWidth();

		if (newPosition.y < 0) newPosition.y = 0;
		else if (newPosition.y > (float)GetScreenHeight()) newPosition.y = (float)GetScreenHeight();

		center = newPosition;

		// update vertices after center
		position1 = { center.x, center.y - size.y / 2.0f };
		position2 = { center.x - size.x / 2.0f, center.y + size.y / 2.0f };
		position3 = { center.x + size.x / 2.0f, center.y + size.y / 2.0f };
	}
	return targetAquired;
}

bool CollectorTriangle::detectDistractor(Agent* agent, Level* level)
{
	bool distractorNearby = false;

	for (auto& circle : level->circle_agents) {
		float distance = Vector2Distance(center, circle.position1);
		if (distance < 75.0f) { // Change this value to adjust the detection radius
			distractorPosition = circle.position1;
			distractorNearby = true;
			break;
		}
	}

	if(distractorNearby)
	{
		targetAquired = false;
		shouldCollect = false;
	} 
	else 
	{
		targetAquired = true;
		shouldCollect = true;
	}

	return distractorNearby;
}

bool CollectorTriangle::evadeDistractor(Agent* agent)
{
	if (Vector2Length(Vector2Subtract(center, distractorPosition)) < 200.0f)
	{
		Vector2 evadeDirection = Vector2Normalize(Vector2Subtract(center, distractorPosition));
		Vector2 newPosition = Vector2Add(center, Vector2Scale(evadeDirection, speed * GetFrameTime()));
		center = newPosition;
		return true;
	}
	return false;
}

bool CollectorTriangle::searchForPowerUp(Agent* agent, Level* level)
{
	for (auto& powerUp : level->powerUp_agents) {
		float distance = Vector2Distance(center, powerUp.position1);
		if (distance < 200.0f) { // This value determines how close the power-up should be to prioritize it
			powerUpPosition = powerUp.position1;
			powerUpSqaure = &powerUp;
			targetAquired = false;
			return true; // Power-up found and within range
		}
	}
	return false; // No power-up in range
}

bool CollectorTriangle::moveToPowerUp(Agent* agent)
{
	Vector2 direction = Vector2Subtract(powerUpPosition, center);
	direction = Vector2Normalize(direction);
	Vector2 newPosition = Vector2Add(center, Vector2Scale(direction, speed * GetFrameTime()));

	// Check if the agent reached the power-up
	if (Vector2Distance(newPosition, powerUpPosition) < 10.0f) { // 10.0f can be adjusted based on desired precision
		isPowerUpCollected = true;
		applyEffect(); // Apply the effect once the power-up is collected
		return true; // Power-up successfully collected
	}

	// Update agent's position
	center = newPosition;

	return false; // Power-up not yet collected
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
		return this->detectDistractorCloseToCollector(agent, level);
		});

	ActionNode* chaseAwayDistractorNode = new ActionNode([this](Agent* agent) {
		return this->chaseAwayDistractor(agent);
		});

	protectCollectorSequence->addChild(detectDistractorNode);
	protectCollectorSequence->addChild(chaseAwayDistractorNode);


	SequenceNode* patrolSequence = new SequenceNode();

	ActionNode* findPatrolPointNode = new ActionNode([this, level ](Agent* agent) {
		return this->findPatrolPoint(agent, level);
		});
	ActionNode* moveToPatrolPointNode = new ActionNode([this, level](Agent* agent) {
		return this->moveToPatrolPoint(agent, level);
		});

	patrolSequence->addChild(findPatrolPointNode);
	patrolSequence->addChild(moveToPatrolPointNode);

	rootNode->addChild(protectCollectorSequence);
	rootNode->addChild(patrolSequence);

	this->behaviorTree = rootNode;
}

void GuardianRectangle::sense(Level* level)
{

}

void GuardianRectangle::sense(Level* level, std::list<CollectableSquare>& squares)
{
	for (auto& circle : level->circle_agents) {
		float distance = Vector2Distance(position1, circle.position1);
		if (distance < maxDetectionDistance) { // Change this value to adjust the detection radius
			distractorPosition = circle.position1;
			intruderInSight = true;
			return;
		}
	}
	intruderInSight = false;
	//targetAquired = false;

	// If no distractor is near, find a patrol point
	if (!intruderInSight && !targetAquired) {
		findPatrolPoint(this, level);
	}
}

void GuardianRectangle::decide()
{
	behaviorTree->execute(this);
}

void GuardianRectangle::act(Level* level)
{
	if(intruderInSight)
	{
		chaseAwayDistractor(this);
	} 
	else if(targetAquired)
	{
		moveToPatrolPoint(this, level);
	}
}

void GuardianRectangle::draw()
{
	DrawRectangle((int)position1.x, (int)position1.y, (int)size.x, (int)size.y, BLUE);
}

void GuardianRectangle::applyEffect()
{
	size = {100, 100};
}

void GuardianRectangle::endEffect()
{
	size = { 50, 50 };
}

bool GuardianRectangle::findPatrolPoint(Agent* agent, Level* level)
{
	if (level->collectableSquare_agents.empty()) {
		targetAquired = false;
		return false;
	}

	if(!targetAquired)
	{
		randomIndexNumber = rand() % level->collectableSquare_agents.size();
	}

	auto it = std::next(level->collectableSquare_agents.begin(), randomIndexNumber);

	targetPosition = it->position1;
	targetAquired = true;

	return true;
}

bool GuardianRectangle::moveToPatrolPoint(Agent* agent, Level* level)
{
	if (!targetAquired || randomIndexNumber >= level->collectableSquare_agents.size()) {
		targetAquired = false;
		return false;
	}

	if (Vector2Length(Vector2Subtract(position1, targetPosition)) < 10.0f) {
		targetAquired = false;
		return false;
	}

	Vector2 direction = Vector2Normalize(Vector2Subtract(targetPosition, position1));
	Vector2 newPosition = Vector2Add(position1, Vector2Scale(direction, speed * GetFrameTime()));
	position1 = newPosition;

	return true;
}

bool GuardianRectangle::detectDistractorCloseToCollector(Agent* agent, Level* level)
{

	for (auto& circle : level->circle_agents) {
		for(auto& triangle : level->triangle_agents)
		{
			float distance = Vector2Distance(triangle.position1, circle.position1);
			if (distance < maxDetectionDistance) { // Change this value to adjust the detection radius
				distractorPosition = circle.position1;
				intruderInSight = true;
				speed = 150.0f;
				return true;
			}
		}
	}
	speed = 75.0f;
	intruderInSight = false;
	isChasing = false;

	return false;
}

bool GuardianRectangle::chaseAwayDistractor(Agent* agent)
{
	if (Vector2Length(Vector2Subtract(position1, distractorPosition)))
	{
		Vector2 chaseDirection = Vector2Normalize(Vector2Subtract(distractorPosition, position1));
		Vector2 newPosition = Vector2Add(position1, Vector2Scale(chaseDirection, speed * GetFrameTime()));
		position1 = newPosition;
		targetAquired = false;
		return true;
	}
	return false;
}

void GuardianRectangle::isStunned()
{
}

void GuardianRectangle::setPosition(float positionX, float positionY)
{
	position1.x = positionX;
	position1.y = positionY;
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

	// move between sqaure and triangle
	SequenceNode* moveBetweenSequence = new SequenceNode();

	ActionNode* checkCollectorNearSquareNode = new ActionNode([this, level](Agent* agent) {
		return this->isCollectorNearSquare(agent, level);
		});

	ActionNode* moveBetweenCollectorAndSquareNode = new ActionNode([this](Agent* agent) {
		return this->moveBetweenCollectorAndSquare(agent);
		});

	moveBetweenSequence->addChild(checkCollectorNearSquareNode);
	moveBetweenSequence->addChild(moveBetweenCollectorAndSquareNode);

	// chase triangle
	SequenceNode* chaseCollectorSequence = new SequenceNode();

	ActionNode* checkCollectorCloseNode = new ActionNode([this, level](Agent* agent)
		{
			return this->shouldChaseCollector(agent, level);
		});

	ActionNode* chaseCollectorNode = new ActionNode([this](Agent* agent)
		{
			return this->chaseCollector(agent);
		});

	chaseCollectorSequence->addChild(checkCollectorCloseNode);
	chaseCollectorSequence->addChild(chaseCollectorNode);

	RandomSelectorNode* chaseOrFlankSelector = new RandomSelectorNode(5.0);
	
	chaseOrFlankSelector->addChild(moveBetweenSequence);
	chaseOrFlankSelector->addChild(chaseCollectorSequence);
	
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
	if(targetAquired && isFlanking)
	{
		moveBetweenCollectorAndSquare(this);
	}
	else if(targetAquired && isChasing)
	{
		chaseCollector(this);
	}
	else if (!targetAquired)
	{
		evadeGuardian(this);
	}
}

void DistractorCircle::draw()
{
	DrawCircle((int)position1.x, (int)position1.y, radius, RED);
}

void DistractorCircle::applyEffect()
{
	// stun guardian
}

void DistractorCircle::endEffect()
{

}

bool DistractorCircle::isCollectorNearSquare(Agent* agent, Level* level)
{
	for (auto& collectorTriangle : level->triangle_agents)
	{
		for(auto& collectableSquare : level->collectableSquare_agents)
		{
			if(!collectableSquare.collected && Vector2Distance(collectorTriangle.position1, collectableSquare.position1) < 100.0f)
			{
				sqaureTarget = &collectableSquare;
				triangleTarget = &collectorTriangle;
				targetAquired = true;
				isFlanking = true;
				isChasing = false;
				return true;
			}
		}
	}
	targetAquired = false;
	isFlanking = false;
	isChasing = false;
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

bool DistractorCircle::shouldChaseCollector(Agent* agent, Level* level)
{

	float closestTriangleDistance = std::numeric_limits<float>::max();
	CollectorTriangle* closestTriangle = nullptr;
	for (CollectorTriangle& triangle : level->triangle_agents)
	{
		float distance = Vector2Distance(this->position1, triangle.position1);
		if (distance < closestTriangleDistance)
		{
			closestTriangleDistance = distance;
			closestTriangle = &triangle;

		}
	}
	this->triangleTarget = closestTriangle ? closestTriangle : nullptr;
	// Start chasing if a CollectorTriangle is within a certain distance
	return closestTriangleDistance < 100.0f; // Adjust this value to your needs
}

bool DistractorCircle::chaseCollector(Agent* agent)
{
	if (triangleTarget) {
		Vector2 direction = Vector2Normalize(Vector2Subtract(triangleTarget->position1, position1));
		position1 = Vector2Add(position1, Vector2Scale(direction, speed * GetFrameTime()));
		isChasing = true;
		isFlanking = false;
		return true;
	}
	isFlanking = false;
	isChasing = false;
	return false;
}

bool DistractorCircle::detectGuardian(Agent* agent, Level* level)
{
	for (auto& rectangle : level->rectangle_agents) {
		float distance = Vector2Distance(position1, rectangle.position1);
		if (distance < 200.0f) { // Change this value to adjust the detection radius
			guardianPosition = rectangle.position1;
			targetAquired = false;
			speed = 150.0f;
			return true;
		}
	}
	speed = 100.0f;
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

Agent* Level::spawn_agent(PowerUpSqaure agent)
{
	Agent* result = nullptr;

	powerUp_agents.push_back(agent);
	result = &powerUp_agents.back();

	pending_agents.push_back(result);

	return result;
}

Agent* Level::spawn_agent(CollectableSquare agent)
{
	Agent* result = nullptr;

	collectableSquare_agents.push_back(agent);
	result = &collectableSquare_agents.back();

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

	collectableSquare_agents.remove_if([](CollectableSquare& a) {return a.dead; });
	powerUp_agents.remove_if([](PowerUpSqaure& a) {return a.dead; });
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
	spawnCollectableSquares(squareCount, this);
	
	int powerUpCount = 1;
	spawnPowerUps(powerUpCount, this);

	auto triangle = spawn_agent(CollectorTriangle());
	triangle->position1 = { 100, 200 };
	triangle->position2 = {150, 200};
	triangle->position3 = { 125, 150 };
	triangle->center = { 600, 200 };
	triangle->size = {50, 50};
	triangle->setupBehaviourTree(this);

	auto rectangle = spawn_agent(GuardianRectangle());
	rectangle->position1 = { 10, 300 };
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
		for (auto& collectableSquare : collectableSquare_agents)
		{
			collectableSquare.sense(this);
			collectableSquare.decide();
		}

		for(auto& powerUp : powerUp_agents)
		{
			powerUp.sense(this);
			powerUp.decide();
		}

		for (auto& collectorTriangle : triangle_agents)
		{
			collectorTriangle.sense(this, collectableSquare_agents);
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

	for (auto& collectableSquare : collectableSquare_agents)
	{
		collectableSquare.act(this);
	}

	for (auto& powerUp : powerUp_agents)
	{
		powerUp.act(this);
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
	for (auto& square : collectableSquare_agents) {
		if (!square.collected) {
			allCollected = false;
			break;
		}
	}

	if (allCollected) {
		for (auto& square : collectableSquare_agents) {
			square.collected = false;
			
			// reset position or any other properties
		}
		int squareCount = 10;
		spawnCollectableSquares(squareCount, this);
	}
}

void Level::draw()
{
	for (auto& collectableSquare : collectableSquare_agents)
	{
		collectableSquare.draw();
	}

	for(auto& powerUp : powerUp_agents)
	{
		powerUp.draw();
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

void Level::spawnCollectableSquares(int count, Level* level)
{
	std::srand(std::time(nullptr));

	for (int i = 0; i < count; ++i) {
		CollectableSquare square;
		square.position1 = { static_cast<float>(std::rand() % GetScreenWidth()), static_cast<float>(std::rand() % GetScreenHeight()) }; // Random position
		square.size = { 25, 25 }; // Fixed size for example, you can randomize this as well
		square.setupBehaviourTree(level);
		spawn_agent(square);
	}
}

void Level::spawnPowerUps(int count, Level* level)
{
	std::srand(std::time(nullptr));

	for(int i = 0; i < count; ++i)
	{
		PowerUpSqaure powerUp;
		powerUp.position1 = { static_cast<float>(std::rand() % GetScreenWidth()), static_cast<float>(std::rand() % GetScreenHeight()) };
		powerUp.size = {25, 25};
		powerUp.setupBehaviourTree(level);
		spawn_agent(powerUp);
	}
}
