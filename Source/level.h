#pragma once
#include "raylib.h"
#include <vector>
#include <unordered_map>

class Level;

class Agent
{
public:
	static constexpr float MAX_ENERGY = 10;
	float energy = MAX_ENERGY;

	int id           = 0;
	Vector2 position = {};
	bool dead        = false;

	virtual void sense(Level* level)  = 0;
	virtual void decide() = 0;
	virtual void act(Level* level)    = 0;

	virtual void draw()   = 0;

	// ----

	virtual float eat_me(Agent* eater) = 0; // Returns the energy eater gains after eating

	virtual ~Agent() = default;
};

// This is just an example
class TRex : public Agent
{
	static constexpr float SPEED = 100.f;

	Vector2 tree_position;
	bool tree_in_sight;

	int tree;

	Vector2 target_position;
	bool target_aquired;

	bool should_eat;
public:
	void sense(Level* level) override;
	void decide() override;
	void act(Level* level) override;
	void draw() override;

	float eat_me(Agent* eater) override;

	virtual ~TRex() = default;
};

// This is just an example
class Tree : public Agent
{
public:
	void sense(Level* level) override;
	void decide() override;
	void act(Level* level) override;
	void draw() override;

	float eat_me(Agent* eater) override;

	virtual ~Tree() = default;
};
	
	
class Level 
{
	int last_id = 0;

	//NOTE(Filippo): Using a list here is not the best idea, ideally you should store agents in some other data structure that keeps them close to each other while being pointer-stable.
	std::list<TRex> trex_agents;
	std::list<Tree> tree_agents;
	// @AddMoreHere

	std::unordered_map<int, Agent*> id_to_agent;
	std::vector<Agent*> all_agents;

	std::vector<Agent*> pending_agents; // Agents that will be added at the beginning of the next frame

public:
	Agent* get_agent(int id);
	Agent* find_tree();

	Agent* spawn_agent(TRex agent);
	Agent* spawn_agent(Tree agent);

	void reset();
	void update();
	void draw();

private:
	void remove_dead_and_add_pending_agents();
	// Remember, if you add more lists (see @AddMoreHere), edit this function so that dead agents are removed correctly without leaking memory
};