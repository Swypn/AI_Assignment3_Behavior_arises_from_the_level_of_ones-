#pragma once
#include "raylib.h"
#include <vector>
#include <unordered_map>

class Level;
class Agent;


// Base class for behavior tree nodes
class BehaviorNode {
public:
	virtual ~BehaviorNode() {}
	virtual bool execute(Agent* agent) = 0;
};

// Selector node - runs its children until one succeeds
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

private:
	std::vector<BehaviorNode*> children;
};

// Sequence node - runs its children until one fails
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

// Action node - represents an action for the agent to perform
class ActionNode : public BehaviorNode {
public:
	using ActionFunction = bool (*)(Agent*);

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
	float radius;
	bool dead        = false;

	virtual void sense(Level* level)  = 0;
	virtual void decide() = 0;
	virtual void act(Level* level)    = 0;

	virtual void draw()   = 0;

	virtual ~Agent() = default;
};

class CollectorTriangle : public Agent 
{
	// Collector
	float speed = 100.0f;
	Vector2 itemPosition;
	bool itemInSight;
	int item;
	Vector2 targetPosition;
	bool targetAquired;

	bool shouldCollect;
public:
	void setupCollectorTriangleBehaviorTree(Agent* agent);
	void sense(Level* level) override;
	void decide() override;
	void act(Level* level);
	void draw() override;

};

class GuardianRectangle : public Agent
{
	// Guardian
	float speed = 100.0f;

	Vector2 patrolArea;
	bool intruderInSight;

	int intruder;

	Vector2 targetPosition;
	bool targetAquired;

	bool shouldChase;
public:
	void setupGuardianRectangleBehaviorTree(Agent* agent);
	void sense(Level* level) override;
	void decide() override;
	void act(Level* level);
	void draw() override;

};

class DistractorCircle : public Agent
{
	// Distractor
	float speed = 100.0f;

	Vector2 distractionTarget;
	bool agentInSight;

	int agent;

	Vector2 targetPosition;
	bool targetAquired;

	bool shouldDistract;
public:
	void setupDistractorCircleBehaviorTree(Agent* agent);
	void sense(Level* level) override;
	void decide() override;
	void act(Level* level);
	void draw() override;

};

class Level 
{
	int last_id = 0;

	//NOTE(Filippo): Using a list here is not the best idea, ideally you should store agents in some other data structure that keeps them close to each other while being pointer-stable.
	//std::list<TRex> trex_agents;
	//std::list<Tree> tree_agents;
	std::list<CollectorTriangle> triangle_agents;
	std::list<GuardianRectangle> rectangle_agents;
	std::list<DistractorCircle> circle_agents;
	// @AddMoreHere

	std::unordered_map<int, Agent*> id_to_agent;
	std::vector<Agent*> all_agents;

	std::vector<Agent*> pending_agents; // Agents that will be added at the beginning of the next frame

public:
	Agent* get_agent(int id);

	//Agent* spawn_agent(TRex agent);
	//Agent* spawn_agent(Tree agent);
	Agent* spawn_agent(CollectorTriangle agent);
	Agent* spawn_agent(GuardianRectangle agent);
	Agent* spawn_agent(DistractorCircle agent);

	void reset();
	void update();
	void draw();

private:
	void remove_dead_and_add_pending_agents();
	// Remember, if you add more lists (see @AddMoreHere), edit this function so that dead agents are removed correctly without leaking memory
};