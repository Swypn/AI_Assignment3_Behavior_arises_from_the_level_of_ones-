#include "level.h"
#include "raymath.h"

#include <string>

void Triangle::sense(Level* level)
{
}

void Triangle::decide()
{
}

void Triangle::act(Level* level)
{
}

void Triangle::draw()
{
}

float Triangle::eat_me(Agent* eater)
{
	return 0.0f;
}

void Rectangle::sense(Level* level)
{
}

void Rectangle::decide()
{
}

void Rectangle::act(Level* level)
{
}

void Rectangle::draw()
{
}

float Rectangle::eat_me(Agent* eater)
{
	return 0.0f;
}

void Circle::sense(Level* level)
{
}

void Circle::decide()
{
}

void Circle::act(Level* level)
{
}

void Circle::draw()
{
}

float Circle::eat_me(Agent* eater)
{
	return 0.0f;
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

Agent* Level::spawn_agent(Triangle agent)
{
	Agent* result = nullptr;

	triangle_agents.push_back(agent);
	result = &triangle_agents.back();

	pending_agents.push_back(result); // Enqueue it so that it can be added to the level at the beginning of the next frame

	return result;
}

Agent* Level::spawn_agent(Rectangle agent)
{
	Agent* result = nullptr;

	rectangle_agents.push_back(agent);
	result = &rectangle_agents.back();

	pending_agents.push_back(result); // Enqueue it so that it can be added to the level at the beginning of the next frame

	return result;
}

Agent* Level::spawn_agent(Circle agent)
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
	triangle_agents.remove_if([](Triangle& a) {return a.dead; });
	rectangle_agents.remove_if([](Rectangle& a) {return a.dead; });
	circle_agents.remove_if([](Circle& a) {return a.dead;  });
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

	auto triangle = spawn_agent(Triangle());
	triangle->position = { 100, 200 };

	auto rectangle = spawn_agent(Rectangle());
	rectangle->position = { 200, 200 };

	auto circle = spawn_agent(Circle());
	circle->position = {300, 200};
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


