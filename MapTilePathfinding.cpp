#include "MapTilePathfinding.h"

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

bool MapTilePathfinder::IsTileInGrid(const MapTile* tile)
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
