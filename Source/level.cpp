#include "level.h"
#include "raymath.h"
#include <string>

void CollectorTriangle::setupCollectorTriangleBehaviorTree(Agent* agent)
{
}

void CollectorTriangle::sense(Level* level)
{
}

void CollectorTriangle::decide()
{
}

void CollectorTriangle::act(Level* level)
{
}

void CollectorTriangle::draw()
{
	DrawTriangle(position1, position2, position3, GOLD);
}


void GuardianRectangle::setupGuardianRectangleBehaviorTree(Agent* agent)
{
}

void GuardianRectangle::sense(Level* level)
{
}

void GuardianRectangle::decide()
{
}

void GuardianRectangle::act(Level* level)
{
}

void GuardianRectangle::draw()
{
	DrawRectangle(position1.x, position1.y, size.x, size.y, BLUE);
}


void DistractorCircle::setupDistractorCircleBehaviorTree(Agent* agent)
{
}

void DistractorCircle::sense(Level* level)
{
}

void DistractorCircle::decide()
{
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

	auto triangle = spawn_agent(CollectorTriangle());
	triangle->position1 = { 100, 200 };
	triangle->position2 = {150, 200};
	triangle->position3 = { 125, 150 };

	auto rectangle = spawn_agent(GuardianRectangle());
	rectangle->position1 = { 200, 200 };
	rectangle->size = { 50, 50 };

	auto circle = spawn_agent(DistractorCircle());
	circle->position1 = {300, 200};
	circle->radius = 25.0f;
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


