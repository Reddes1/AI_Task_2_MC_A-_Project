#include "MapTilePathfinding.h"
#include "TextureEnums.h"

using namespace DirectX;

MapTilePathfinder::MapTilePathfinder()
{
	
}

void MapTilePathfinder::GenerateTileGrid(std::vector<MapTile*>& tileContainer, UnitEntity* unit, int mapRowLength)
{

	//
	//Setup
	//

	//Create two holding containers
	std::set<Node> newNodes;
	std::set<Node> tempNodes;

	//Grab unit coordinates for brevity
	XMINT2& coords = unit->GetMapCoordinates();

	//Add Starting root node and setup
	Node rootNode;
	rootNode.tile = tileContainer.at((coords.x + coords.y) + coords.y * mapRowLength);
	rootNode.remainingCost = unit->GetClassTotals().TotalMovespeed;

	//
	//Algorithm Preamble
	//

	//Check neighbours of the initial node and run algorithm to for starting chain
	for (int i(0); i < NUM_OF_NEIGHBOURS; ++i)
	{
		if (rootNode.tile->GetNeighbourAtIndex(i))
		{
			if (MoveValidationPolicy(rootNode, rootNode.tile->GetNeighbourAtIndex(i), unit))
			{
				NodeValidation(rootNode, newNodes, i);
			}
		}
	}

	//Add the starting node to the manifest (we intend to remove this later, but prevents needless insertions and odd behaviours)
	m_GridManifest.insert(rootNode.tile);
	//Add starting nodes to manifest
	for (auto& t : newNodes)
		m_GridManifest.insert(t.tile);


	//
	//Main Algorithm
	//

	//Using starting nodes, run algorithm to explore and evaluate new nodes till all possible nodes have been found
	while (newNodes.size() != 0)
	{
		//For each unresolved Tile, check its neighbouring nodes for existance & then if traversable
		for (auto& node : newNodes)
		{
			for (int i(0); i < NUM_OF_NEIGHBOURS; ++i)
			{
				if (node.tile->GetNeighbourAtIndex(i))
				{
					if (MoveValidationPolicy(node, node.tile->GetNeighbourAtIndex(i), unit))
					{
						NodeValidation(node, tempNodes, i);
					}
				}
			}
		}

		//Store the new tiles for the next cycle
		newNodes = tempNodes;
		//Clear for next cycle
		tempNodes.clear();
		//Extract the addresses from the newly added nodes into a manifest
		for (auto& t : newNodes)
			m_GridManifest.insert(t.tile);
	}

	//
	//Post Cleanup
	//

	//No longer need the original tile in the manifest, so remove it
	m_GridManifest.erase(rootNode.tile);

}

bool MapTilePathfinder::IsTileInGrid(MapTile* tile)
{
	//Attempt to find a match for the tile
	if (std::find(m_GridManifest.begin(), m_GridManifest.end(), tile) != m_GridManifest.end())
		return true;
	else
		return false;
}

void MapTilePathfinder::ReleaseManifest()
{
	for (auto& m : m_GridManifest)
	{
		//Turn off/disable grid
		m->GetParentTile() = nullptr;
		RemoveGridEffect(m);
	}

	m_GridManifest.clear();
}

bool MapTilePathfinder::MoveValidationPolicy(const Node& node, MapTile* neighbour, UnitEntity* unit)
{
	return ((node.remainingCost - neighbour->GetTileProperties().moveCost >= 0) &&					//Is there enough cost left?
			(neighbour->GetTileProperties().occupied == false) &&									//Is the tile occupied?
			(neighbour->GetTileProperties().impassable == false) &&									//Is the tile impassable?
			(static_cast<int>(unit->GetUnitType()) == neighbour->GetTileProperties().terrainTypeID) //Can the unit navigate to the tile type?
	);
}

void MapTilePathfinder::NodeValidation(const Node& node, std::set<Node>& container, int index)
{
	//Start with a new node object
	Node newNode;

	//Do the minimum required work for algorithm (as node may be discarded)
	newNode.remainingCost = node.remainingCost - node.tile->GetNeighbourAtIndex(index)->GetTileProperties().moveCost;
	newNode.tile = node.tile->GetNeighbourAtIndex(index);

	/*
		As the algorithm doesn't care on how its finds the tile, only that it does, it might reach a tile and
		have provide it a less than optimal value for the remaining moves as another route could reach it better.
		So we check this new node against another node of the same coordinates. If a match is found, we then evaluate
		the better option of the two by comparing remaining costs. If the existing nodes cost is better (higher remaining),
		then we discard the new node as it evaluated poorly. If the new node is better, we erase the old node and insert the
		new one.

		If no node is found at all, then this new entry is inserted into the container as usual.
	*/
	std::set<Node>::iterator it = container.find(newNode);
	if (it != container.end())	//Found match
	{
		//Is the new node better? If not do nothing.
		if (it->remainingCost < newNode.remainingCost)
		{
			//Discard the existing, worse node
			container.erase(it);

			//Apply/Enable grid effect
			ApplyGridEffect(newNode.tile);

			//Insert new node
			container.insert(newNode);
		}
	}
	//No match meaning this is first time this tile has been checked.
	else	
	{
		//Apply/Enable grid effect
		ApplyGridEffect(newNode.tile);

		//Insert new node
		container.insert(newNode);
	}
}

void MapTilePathfinder::ApplyGridEffect(MapTile* tile)
{
	tile->SetDrawGridFlag(true);
	tile->GetGridSprite().SetFrame(21);
}

void MapTilePathfinder::RemoveGridEffect(MapTile* tile)
{
	tile->SetDrawGridFlag(false);
}

void MapTilePathfinder::ResetGridEffectToDefault()
{
	for (auto& a : m_GridManifest)
	{
		a->GetGridSprite().SetFrame(21);
		a->GetParentTile() = nullptr;
		a->SetDrawGridFlag(true);
	}
}

void MapTilePathfinder::RunAStarAlgorithm(MapTile* startingTile, MapTile* targetTile)
{
	//This container holds nodes that need to be evaluated
	std::vector<MapTile*> openNodes;
	//This container holds the nodes that have been evaluated
	std::vector<MapTile*> closedNodes;

	//Setup starting node
	startingTile->CalculateCosts(startingTile->GetMapCoordinates(), targetTile->GetMapCoordinates());

	//Push starting node into open nodes
	openNodes.push_back(startingTile);

	while (openNodes.size() > 0)
	{
		
		MapTile* currentNode = nullptr;

		//Find the lowest F Cost node in open nodes
		FindLowestFCostNode(currentNode, openNodes);
		RemoveNodeFromContainer(currentNode, openNodes);
		closedNodes.push_back(currentNode);


		//Check if the current node is the target node break out
		if ((currentNode->GetMapCoordinates().x == targetTile->GetMapCoordinates().x) &&
			(currentNode->GetMapCoordinates().y == targetTile->GetMapCoordinates().y))

		{
			//Enable effect on each tile in path
			EnablePath(currentNode);
			return;
		}

		//Start looking at the neighbours

		for (int i(0); i < NUM_OF_NEIGHBOURS; ++i)
		{
			//Validate if this neighbour needs evaluating or not
			if (currentNode->GetNeighbourAtIndex(i) &&
				IsTileInGrid(currentNode->GetNeighbourAtIndex(i)) &&
				!currentNode->GetNeighbourAtIndex(i)->GetTileProperties().impassable &&
				!IsNodeInContainer(currentNode->GetNeighbourAtIndex(i), closedNodes))
			{
				//Create new node and calculate information about the node
				MapTile* newNode = currentNode->GetNeighbourAtIndex(i);
				newNode->CalculateCosts(startingTile->GetMapCoordinates(), targetTile->GetMapCoordinates());

				//Check if H is lower OR not inside the open nodes container
				if ((newNode->GetHCost() < currentNode->GetHCost()) || !IsNodeInContainer(newNode, openNodes))
				{
					//Set the new nodes parent at the currently examined tile
					newNode->GetParentTile() = currentNode;

					//If the node isnt in the container
					if (!IsNodeInContainer(newNode, openNodes))
					{
						openNodes.push_back(newNode);
					}
				}
			}
		}

	};
}

void MapTilePathfinder::FindLowestFCostNode(MapTile*& node, std::vector<MapTile*>& nodes)
{
	//Set node to first node
	node = nodes[0];

	for (int i(0); i < nodes.size(); ++i)
	{
		//If the checked node is lower than first held node, replace it
		if ((nodes[i]->GetFCost() < node->GetFCost()) || (nodes[i]->GetFCost() == node->GetFCost() && nodes[i]->GetHCost() < node->GetHCost()))
			node = nodes[i];
	}
}

void MapTilePathfinder::RemoveNodeFromContainer(MapTile*& node, std::vector<MapTile*>& nodes)
{
	for (int i(0); i < nodes.size(); ++i)
	{
		if (node == nodes[i])
		{
			nodes.erase(nodes.begin() + i);
		}
	}
}

bool MapTilePathfinder::IsNodeInContainer(MapTile* node, std::vector<MapTile*>& nodes)
{
	for (int i(0); i < nodes.size(); ++i)
	{
		//If the same tile is found, return true
		if (node == nodes[i])
			return true;
	}

	//Else no tile found, return false
	return false;
}

void MapTilePathfinder::EnablePath(MapTile*& endpoint)
{
	bool pathDone = false;
	
	MapTile* currentNode = endpoint;

	while (!pathDone)
	{
		currentNode->GetGridSprite().SetFrame(UI_ATLAS_01_FRAMES::ATTACK_TILE_HIGHLIGHT);
		
		//If the node has a parent 
		if (currentNode->GetParentTile())
		{
			currentNode = currentNode->GetParentTile();
		}
		//This must be the origin point, so end drawing
		else
		{
			pathDone = true;
		}

	}
}
