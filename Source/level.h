#pragma once
#include "raylib.h"
#include <vector>
#include <unordered_map>
#include <functional>
#include <ctime>
#include <random>
class Level;
class Agent;


// Base class for behavior tree nodes
class BehaviorNode {
public:
	virtual ~BehaviorNode() {}
	virtual bool execute(Agent* agent) = 0;
};

// Selector node, runs its children until one succeeds
class SelectorNode : public BehaviorNode {
public:
	virtual bool execute(Agent* agent) override {
		for (BehaviorNode* child : children) {
			if (child->execute(agent)) {
				return true;
			}
		}
		return false;
	}

	void addChild(BehaviorNode* child) {
		children.push_back(child);
	}

protected:
	std::vector<BehaviorNode*> children;
};

// Sequence node, runs its children until one fails
class SequenceNode : public BehaviorNode {
public:
	virtual bool execute(Agent* agent) override {
		for (BehaviorNode* child : children) {
			if (!child->execute(agent)) {
				return false;
			}
		}
		return true;
	}

	void addChild(BehaviorNode* child) {
		children.push_back(child);
	}

private:
	std::vector<BehaviorNode*> children;
};

class RandomSelectorNode : public SelectorNode {
public:
	RandomSelectorNode(double delayInSeconds)
		: delay(delayInSeconds),
		accumulatedTime(0.0) {}

	virtual bool execute(Agent* agent) override {
		accumulatedTime += GetFrameTime();
		if (accumulatedTime >= delay) {
			accumulatedTime = 0.0;
			currentChild = rand() % children.size();
		}
		return children[currentChild]->execute(agent);
	}

private:
	double delay;
	double accumulatedTime;
	size_t currentChild = 0;
};

// Action node, it represents an action for the agent to perform
class ActionNode : public BehaviorNode {
public:
	using ActionFunction = std::function<bool(Agent*)>;

	ActionNode(ActionFunction actionFunc) : actionFunction(actionFunc) {}

	virtual bool execute(Agent* agent) override {
		return actionFunction(agent);
	}

private:
	ActionFunction actionFunction;
};

class Agent
{
public:
	static constexpr float MAX_ENERGY = 10;
	float energy = MAX_ENERGY;
	BehaviorNode* behaviorTree;
	int id           = 0;
	Vector2 position1 = {};
	Vector2 position2 = {};
	Vector2 position3 = {};
	Vector2 size = {};
	Vector2 center = {0, 0};
	float radius;
	bool dead        = false;

	virtual void sense(Level* level)  = 0;
	virtual void decide() = 0;
	virtual void act(Level* level)    = 0;
	virtual void setupBehaviourTree(Level* level) = 0;
	virtual void draw()   = 0;

	virtual ~Agent() = default;
};

class CollectableSquare : public Agent
{
public:
	bool collected = false;

	void setupBehaviourTree(Level* level) override;
	void sense(Level* level) override;
	void decide() override;
	void act(Level* level) override;
	void draw() override;
};

class CollectorTriangle : public Agent 
{
	// Collector
	float speed = 100.0f;
	float orientation = 0.0f;
	Vector2 itemPosition;
	bool itemInSight;
	int item;
	Vector2 targetPosition;
	bool targetAquired;
	bool shouldCollect;
	Vector2 distractorPosition;
public:
	void setupBehaviourTree(Level* level) override;
	void sense(Level* level) override;
	void sense(Level* level, std::list<CollectableSquare>& squares);
	void decide() override;
	void act(Level* level);
	void draw() override;
	bool searchForItem(Agent* agent, Level* level); 
	Vector2 findNewTarget(Agent* agent, Level* level);
	bool moveToItem(Agent* agent);
	bool detectDistractor(Agent* agent, Level* level);
	bool evadeDistractor(Agent* agent);
	Vector2 Vector2Rotate(Vector2 point, float rad);
	Vector2 Vector2Lerp(Vector2 a, Vector2 b, float t);
};

class GuardianRectangle : public Agent
{
	// Guardian
	float speed = 50.0f;
	float maxDetectionDistance = 100.0f;
	Vector2 patrolArea;
	bool intruderInSight;

	int intruder;

	Vector2 targetPosition;
	bool targetAquired;
	int randomIndexNumber;

	Vector2 distractorPosition;
public:
	void setupBehaviourTree(Level* level) override;
	void sense(Level* level) override;
	void sense(Level* level, std::list<CollectableSquare>& squares);
	void decide() override;
	void act(Level* level);
	void draw() override;
	bool findPatrolPoint(Agent* agent, Level* level);
	bool moveToPatrolPoint(Agent* agent, Level* level);
	bool detectDistractorCloseToCollector(Agent* agent, Level* level);
	bool chaseAwayDistractor(Agent* agent);
};

class DistractorCircle : public Agent
{
	// Distractor
	float speed = 50.0f;
	
	CollectableSquare* sqaureTarget;
	CollectorTriangle* triangleTarget;

	Vector2 distractionTarget;
	bool agentInSight;
	int agent;
	Vector2 targetPosition;
	bool targetAquired;
	bool isChasing;
	bool isFlanking;
	Vector2 guardianPosition;
public:
	void setupBehaviourTree(Level* level) override;
	void sense(Level* level) override;
	void decide() override;
	void act(Level* level);
	void draw() override;
	bool isCollectorNearSquare(Agent* agent, Level* level);
	bool moveBetweenCollectorAndSquare(Agent* agent);
	bool shouldChaseCollector(Agent* agent, Level* level);
	bool chaseCollector(Agent* agent);
	bool detectGuardian(Agent* agent, Level* level);
	bool evadeGuardian(Agent* agent);
};

class Level 
{
	int last_id = 0;
	float tickTimer = 0.2f;
	float currentTime = 0.0f;
	
	int uncollectedSquareCount = 0;
	
	std::unordered_map<int, Agent*> id_to_agent;
	std::vector<Agent*> all_agents;

	std::vector<Agent*> pending_agents; // Agents that will be added at the beginning of the next frame

public:
	//NOTE(Filippo): Using a list here is not the best idea, ideally you should store agents in some other data structure that keeps them close to each other while being pointer-stable.
	std::list<CollectableSquare> square_agents;
	std::list<CollectorTriangle> triangle_agents;
	std::list<GuardianRectangle> rectangle_agents;
	std::list<DistractorCircle> circle_agents;
	int score;
	Agent* get_agent(int id);

	Agent* spawn_agent(CollectableSquare agent);
	Agent* spawn_agent(CollectorTriangle agent);
	Agent* spawn_agent(GuardianRectangle agent);
	Agent* spawn_agent(DistractorCircle agent);

	void reset();
	void update();
	void draw();

	void setupCollectableSquares(int count);
private:
	void remove_dead_and_add_pending_agents();
	// Remember, if you add more lists (see @AddMoreHere), edit this function so that dead agents are removed correctly without leaking memory
};