#include <SFML/Graphics.hpp>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <algorithm>

#include "map.hpp"
#include "tile.hpp"

/* Load map from disk */
void Map::load(const std::string& filename, unsigned int width, unsigned int height,
    std::map<std::string, Tile>& tileAtlas)
{
    std::ifstream inputFile;
    inputFile.open(filename, std::ios::in | std::ios::binary);

    this->width = width;
    this->height = height;

    for (unsigned int pos = 0; pos < this->width * this->height; ++pos)
    {
        this->resources.push_back(255);
        this->selected.push_back(0);

        TileType tileType;
        inputFile.read((char*)&tileType, sizeof(int));
        switch (tileType)
        {
        default:
        case TileType::VOID:
        case TileType::GRASS:
            this->tiles.push_back(tileAtlas.at("grass"));
            break;
        case TileType::FOREST:
            this->tiles.push_back(tileAtlas.at("forest"));
            break;
        case TileType::WATER:
            this->tiles.push_back(tileAtlas.at("water"));
            break;
        case TileType::RESIDENTIAL:
            this->tiles.push_back(tileAtlas.at("residential"));
            break;
        case TileType::COMMERCIAL:
            this->tiles.push_back(tileAtlas.at("commercial"));
            break;
        case TileType::INDUSTRIAL:
            this->tiles.push_back(tileAtlas.at("industrial"));
            break;
        case TileType::ROAD:
            this->tiles.push_back(tileAtlas.at("road"));
            break;
        }
        Tile& tile = this->tiles.back();
        inputFile.read((char*)&tile.tileVariant, sizeof(int));
        inputFile.read((char*)&tile.regions, sizeof(int) * 1);
        inputFile.read((char*)&tile.population, sizeof(double));
        inputFile.read((char*)&tile.storedGoods, sizeof(float));
    }

    inputFile.close();

    return;
}

void Map::save(const std::string& filename)
{
    std::ofstream outputFile;
    outputFile.open(filename, std::ios::out | std::ios::binary);

    for (auto tile : this->tiles)
    {
        outputFile.write((char*)&tile.tileType, sizeof(int));
        outputFile.write((char*)&tile.tileVariant, sizeof(int));
        outputFile.write((char*)&tile.regions, sizeof(int) * 3);
        outputFile.write((char*)&tile.population, sizeof(double));
        outputFile.write((char*)&tile.storedGoods, sizeof(float));
    }

    outputFile.close();

    return;
}

void Map::draw(sf::RenderWindow& window, float dt)
{
    for (unsigned int y = 0; y < this->height; ++y)
    {
        for (unsigned int x = 0; x < this->width; ++x)
        {
            sf::Vector2f pos;
            pos.x = (static_cast<float>(x) - static_cast<float>(y)) * this->tileSize + this->width * this->tileSize;
            pos.y = (static_cast<float>(x) + static_cast<float>(y)) * this->tileSize * 0.5f;
            unsigned int index = y * this->width + x;

            this->tiles[index].sprite->setPosition(pos);

            if (this->selected[index] > 0)
            {
                // If selected (1) or invalid (2), apply a grey tint
                this->tiles[index].sprite->setColor(sf::Color(0x7d, 0x7d, 0x7d));
            }
            else
            {
                // Reset to white (no tint)
                this->tiles[index].sprite->setColor(sf::Color::White);
            }

            this->tiles[index].draw(window, dt);
        }
    }
}

void Map::updateDirection(TileType tileType)
{
    for (unsigned int y = 0; y < this->height; ++y)
    {
        for (unsigned int x = 0; x < this->width; ++x)
        {
            unsigned int pos = y * this->width + x;
            if (this->tiles[pos].tileType != tileType) continue;

            // Correct 3x3 array size
            bool adj[3][3] = { {0,0,0}, {0,0,0}, {0,0,0} };

            if (x > 0 && y > 0) adj[0][0] = (this->tiles[(y - 1) * width + (x - 1)].tileType == tileType);
            if (y > 0)          adj[0][1] = (this->tiles[(y - 1) * width + (x)].tileType == tileType);
            if (x < width - 1 && y > 0) adj[0][2] = (this->tiles[(y - 1) * width + (x + 1)].tileType == tileType);
            if (x > 0)          adj[1][0] = (this->tiles[(y)*width + (x - 1)].tileType == tileType);
            if (x < width - 1)    adj[1][2] = (this->tiles[(y)*width + (x + 1)].tileType == tileType);
            if (x > 0 && y < height - 1) adj[2][0] = (this->tiles[(y + 1) * width + (x - 1)].tileType == tileType);
            if (y < height - 1)   adj[2][1] = (this->tiles[(y + 1) * width + (x)].tileType == tileType);
            if (x < width - 1 && y < height - 1) adj[2][2] = (this->tiles[(y + 1) * width + (x + 1)].tileType == tileType);

            // Updated logic for 3x3 indices
            if (adj[1][0] && adj[1][2] && adj[0][1] && adj[2][1]) this->tiles[pos].tileVariant = 2;
            else if (adj[1][0] && adj[1][2] && adj[0][1]) this->tiles[pos].tileVariant = 7;
            else if (adj[1][0] && adj[1][2] && adj[2][1]) this->tiles[pos].tileVariant = 8;
            else if (adj[0][1] && adj[2][1] && adj[1][0]) this->tiles[pos].tileVariant = 9;
            else if (adj[0][1] && adj[2][1] && adj[1][2]) this->tiles[pos].tileVariant = 10;
            else if (adj[1][0] && adj[1][2]) this->tiles[pos].tileVariant = 0;
            else if (adj[0][1] && adj[2][1]) this->tiles[pos].tileVariant = 1;
            else if (adj[2][1] && adj[1][0]) this->tiles[pos].tileVariant = 3;
            else if (adj[0][1] && adj[1][2]) this->tiles[pos].tileVariant = 4;
            else if (adj[1][0] && adj[0][1]) this->tiles[pos].tileVariant = 5;
            else if (adj[2][1] && adj[1][2]) this->tiles[pos].tileVariant = 6;
            else if (adj[1][0] || adj[1][2]) this->tiles[pos].tileVariant = 0;
            else if (adj[0][1] || adj[2][1]) this->tiles[pos].tileVariant = 1;
        }
    }
}

void Map::depthfirstsearch(std::vector<TileType>& whitelist,
    sf::Vector2i pos, int label, int regionType)
{
    if (pos.x < 0 || static_cast<unsigned int>(pos.x) >= this->width) return;
    if (pos.y < 0 || static_cast<unsigned int>(pos.y) >= this->height) return;
    if (this->tiles[pos.y * this->width + pos.x].regions[regionType] != 0) return;

    bool found = false;
    for (auto type : whitelist)
    {
        if (type == this->tiles[pos.y * this->width + pos.x].tileType)
        {
            found = true;
            break;
        }
    }
    if (!found) return;

    this->tiles[pos.y * this->width + pos.x].regions[regionType] = label;

    depthfirstsearch(whitelist, pos + sf::Vector2i(-1, 0), label, regionType);
    depthfirstsearch(whitelist, pos + sf::Vector2i(0, 1), label, regionType);
    depthfirstsearch(whitelist, pos + sf::Vector2i(1, 0), label, regionType);
    depthfirstsearch(whitelist, pos + sf::Vector2i(0, -1), label, regionType);

    return;
}

void Map::findConnectedRegions(std::vector<TileType> whitelist, int regionType)
{
    int regions = 1;

    for (auto& tile : this->tiles) tile.regions[regionType] = 0;

    for (unsigned int y = 0; y < this->height; ++y)
    {
        for (unsigned int x = 0; x < this->width; ++x)
        {
            bool found = false;
            for (auto type : whitelist)
            {
                if (type == this->tiles[y * this->width + x].tileType)
                {
                    found = true;
                    break;
                }
            }
            if (this->tiles[y * this->width + x].regions[regionType] == 0 && found)
            {
                depthfirstsearch(whitelist, sf::Vector2i(static_cast<int>(x), static_cast<int>(y)), regions++, regionType);
            }
        }
    }
    this->numRegions[regionType] = static_cast<unsigned int>(regions);
}

void Map::clearSelected()
{
    for (auto& tile : this->selected) tile = 0;

    this->numSelected = 0;

    return;
}

void Map::select(sf::Vector2i start, sf::Vector2i end, std::vector<TileType> blacklist)
{
    /* Swap coordinates if necessary */
    if (end.y < start.y) std::swap(start.y, end.y);
    if (end.x < start.x) std::swap(start.x, end.x);

    /* Clamp in range using static_cast to resolve signed/unsigned mismatch */
    if (start.x < 0) start.x = 0;
    if (start.y < 0) start.y = 0;
    if (static_cast<unsigned int>(end.x) >= this->width)  end.x = static_cast<int>(this->width - 1);
    if (static_cast<unsigned int>(end.y) >= this->height) end.y = static_cast<int>(this->height - 1);

    // Reset selection before starting new selection
    this->numSelected = 0;
    // Note: Ensure this->selected is resized to width * height elsewhere (like in load)
    std::fill(this->selected.begin(), this->selected.end(), 0);

    for (int y = start.y; y <= end.y; ++y)
    {
        for (int x = start.x; x <= end.x; ++x)
        {
            unsigned int index = static_cast<unsigned int>(y) * this->width + static_cast<unsigned int>(x);

            /* Check if the tile type is in the blacklist. 1 = Selected, 2 = Invalid */
            this->selected[index] = 1;
            ++this->numSelected;

            for (auto type : blacklist)
            {
                if (this->tiles[index].tileType == type)
                {
                    this->selected[index] = 2;
                    --this->numSelected;
                    break;
                }
            }
        }
    }

    return;
}