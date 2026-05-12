#ifndef TILE_HPP
#define TILE_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

#include "animation_handler.hpp"

enum class TileType { VOID, GRASS, FOREST, WATER, RESIDENTIAL, COMMERCIAL, INDUSTRIAL, ROAD };

std::string tileTypeToStr(TileType type);

class Tile
{
public:
    AnimationHandler animHandler;

    // Using a pointer allows us to have a Tile without a Sprite immediately
    std::unique_ptr<sf::Sprite> sprite;

    TileType tileType;
    int tileVariant;
    unsigned int cost;
    unsigned int maxLevels;
    float population;
    float maxPopPerLevel;
    unsigned int production;
    unsigned int storedGoods;
    int regions[1];

    void update();
    void draw(sf::RenderWindow& window, float dt);

    /* Fixed Default Constructor */
    Tile() :
        sprite(nullptr),
        tileType(TileType::VOID),
        tileVariant(0),
        cost(0),
        population(0.0f),
        maxPopPerLevel(0.0f),
        maxLevels(0),
        production(0),
        storedGoods(0)
    {
        this->regions[0] = 0;
    }

    /* Manual Copy Constructor to handle unique_ptr */
    Tile(const Tile& other) :
        animHandler(other.animHandler),
        tileType(other.tileType),
        tileVariant(other.tileVariant),
        cost(other.cost),
        maxLevels(other.maxLevels),
        population(other.population),
        maxPopPerLevel(other.maxPopPerLevel),
        production(other.production),
        storedGoods(other.storedGoods)
    {
        this->regions[0] = other.regions[0];
        // Create a NEW sprite for the copy rather than trying to copy the pointer
        if (other.sprite) {
            this->sprite = std::make_unique<sf::Sprite>(*other.sprite);
        }
        else {
            this->sprite = nullptr;
        }
    }

    /* Manual Copy Assignment Operator to handle unique_ptr */
    Tile& operator=(const Tile& other) {
        if (this == &other) return *this;

        this->animHandler = other.animHandler;
        this->tileType = other.tileType;
        this->tileVariant = other.tileVariant;
        this->cost = other.cost;
        this->maxLevels = other.maxLevels;
        this->population = other.population;
        this->maxPopPerLevel = other.maxPopPerLevel;
        this->production = other.production;
        this->storedGoods = other.storedGoods;
        this->regions[0] = other.regions[0];

        if (other.sprite) {
            this->sprite = std::make_unique<sf::Sprite>(*other.sprite);
        }
        else {
            this->sprite = nullptr;
        }
        return *this;
    }

    /* SFML 3.1 Updated Main Constructor */
    Tile(const unsigned int tileSize, const unsigned int height, sf::Texture& texture,
        const std::vector<Animation>& animations,
        const TileType tileType, const unsigned int cost, const unsigned int maxPopPerLevel,
        const unsigned int maxLevels) :
        sprite(std::make_unique<sf::Sprite>(texture)), // Create the sprite here
        tileType(tileType),
        tileVariant(0),
        cost(cost),
        population(0.0f),
        maxPopPerLevel(static_cast<float>(maxPopPerLevel)),
        maxLevels(maxLevels),
        production(0),
        storedGoods(0)
    {
        this->regions[0] = 0;

        // Use -> because sprite is now a pointer
        this->sprite->setOrigin({ 0.0f, static_cast<float>(tileSize * (height - 1)) });

        // SFML 3 Rect: {position}, {size}
        this->animHandler.frameSize = sf::IntRect(
            { 0, 0 },
            { static_cast<int>(tileSize * 2), static_cast<int>(tileSize * height) }
        );

        for (auto& animation : const_cast<std::vector<Animation>&>(animations))
        {
            this->animHandler.addAnim(animation);
        }
        this->animHandler.update(0.0f);
    }

    std::string getCost()
    {
        return std::to_string(this->cost);
    }
};

#endif /* TILE_HPP */