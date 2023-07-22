#include "level.h"
#include "raymath.h"

#include <string>

void TRex::sense(Level* level)
{
	auto tree = level->find_tree();
	if(tree)
	{
		tree_position = tree->position;
		tree_in_sight = true;
	} else {
		tree_in_sight = false;
	}
}

void TRex::decide()
{
	if(tree_in_sight)
	{
		target_position = tree_position;
		target_aquired = true;
	}

	if(energy < 5 && Vector2Distance(tree_position, position) < 60)
	{
		should_eat = true;
	}
}

void TRex::act(Level* level)
{
	if(target_aquired)
	{
		Vector2 tree_direction = Vector2Normalize(Vector2Subtract(tree_position, position));

		position = Vector2Add(position, Vector2Scale(tree_direction, SPEED * GetFrameTime()));
	}

	if(should_eat)
	{
		auto closest_agent = level->find_tree(); // BAD
		if(closest_agent)
		{
			energy += closest_agent->eat_me(this);
		}

		should_eat = false;
	}
}

void TRex::draw()
{
	DrawCircle(position.x, position.y, 20, BLUE);
	DrawCircle(position.x, position.y, energy / MAX_ENERGY * 10 + 2, YELLOW);
}

float TRex::eat_me(Agent* eater)
{
	return 0;
}

void Tree::sense(Level* level)
{
}

void Tree::decide()
{
}

void Tree::act(Level* level)
{
	energy += GetFrameTime(); // From the sun
}

void Tree::draw()
{
	DrawCircle(position.x, position.y, 40, DARKGREEN);
	
	std::string text = "E: " + std::to_string(energy);
	DrawText(text.c_str(), position.x, position.y - 60, 20, BLACK);
}

float Tree::eat_me(Agent* eater)
{
	float result = energy / 2;
	energy -= result;

	return result;
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

Agent* Level::find_tree()
{
	if(tree_agents.size())
	{
		return &tree_agents.back();
	}
	return nullptr;
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
	// @AddMoreHere
	trex_agents.remove_if([](TRex& a){ return a.dead; });
	tree_agents.remove_if([](Tree& a){ return a.dead; });


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

Agent* Level::spawn_agent(TRex agent)
{
	Agent* result = nullptr;

	trex_agents.push_back(agent);
	result = &trex_agents.back();
	
	pending_agents.push_back(result); // Enqueue it so that it can be added to the level at the beginning of the next frame

	return result;
}

Agent* Level::spawn_agent(Tree agent)
{
	Agent* result = nullptr;

	tree_agents.push_back(agent);
	result = &tree_agents.back();
	
	pending_agents.push_back(result); // Enqueue it so that it can be added to the level at the beginning of the next frame

	return result;
}

void Level::reset()
{
	// TODO: Implement this yourself, clear all lists and vectors, after that spawn agents

    // this is here just as an example.
    // You should also replace "SillyAgent", that is also just an example.

	auto tree = spawn_agent(Tree());
    tree->position = {100,200};
    
	auto trex = spawn_agent(TRex());
    trex->position = {300,100};
}

void Level::update()
{
	remove_dead_and_add_pending_agents();

	for(auto& agent : all_agents)
	{
		if(agent->energy > 0)
		{
			agent->energy -= GetFrameTime();
		} else {
			agent->dead = true;
		}
		// TODO: This piece of code needs to be changed to make sure that sense, decide, act, happen at different frequencies.
		agent->sense(this);
		agent->decide();
		agent->act(this);
	}
}

void Level::draw()
{
	for(auto& agent : all_agents)
	{
		agent->draw();
	}
}