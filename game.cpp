#include <stack>

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include "game.hpp"
#include "game_state.hpp"
#include "texture_manager.hpp"
#include <iostream>
#include "animation_handler.hpp"

void Game::loadTextures()
{
    texmgr.loadTexture("grass", "grass.png");
    texmgr.loadTexture("forest", "forest.png");
    texmgr.loadTexture("water", "water.png");
    texmgr.loadTexture("residential", "residential.png");
    texmgr.loadTexture("commercial", "commercial.png");
    texmgr.loadTexture("industrial", "industrial.png");
    texmgr.loadTexture("road", "road.png");

    texmgr.loadTexture("background", "background.png");
}

void Game::pushState(std::unique_ptr<GameState> state)
{
    // Use std::move because unique_ptr cannot be copied, only moved
    this->states.push(std::move(state));
}

void Game::popState()
{
    // NO 'delete' here! 
    // When you pop a unique_ptr, it automatically deletes the object it points to.
    this->states.pop();
}

void Game::changeState(std::unique_ptr<GameState> state)
{
    if (!this->states.empty())
        this->popState();

    this->pushState(std::move(state));
}

GameState* Game::peekState()
{
    if (this->states.empty())
        return nullptr;

    // .get() returns the raw pointer (GameState*) 
    // while the unique_ptr keeps ownership in the stack.
    return this->states.top().get();
}

void Game::gameLoop()
{
    sf::Clock clock;

    while (this->window.isOpen())
    {
        sf::Time elapsed = clock.restart();
        float dt = elapsed.asSeconds();

        if (peekState() == nullptr) continue;
        peekState()->handleInput();
        peekState()->update(dt);
        this->window.clear(sf::Color::Black);
        peekState()->draw(dt);
        this->window.display();
    }
}

void Game::loadTiles()
{
    Animation staticAnim(0, 0, 1.0f);
    this->tileAtlas["grass"] =
        Tile(this->tileSize, 1, texmgr.getRef("grass"),
            { staticAnim },
            TileType::GRASS, 50, 0, 1);
    tileAtlas["forest"] =
        Tile(this->tileSize, 1, texmgr.getRef("forest"),
            { staticAnim },
            TileType::FOREST, 100, 0, 1);
    tileAtlas["water"] =
        Tile(this->tileSize, 1, texmgr.getRef("water"),
            { Animation(0, 3, 0.5f),
            Animation(0, 3, 0.5f),
            Animation(0, 3, 0.5f) },
            TileType::WATER, 0, 0, 1);
    tileAtlas["residential"] =
        Tile(this->tileSize, 2, texmgr.getRef("residential"),
            { staticAnim, staticAnim, staticAnim,
            staticAnim, staticAnim, staticAnim },
            TileType::RESIDENTIAL, 300, 50, 6);
    tileAtlas["commercial"] =
        Tile(this->tileSize, 2, texmgr.getRef("commercial"),
            { staticAnim, staticAnim, staticAnim, staticAnim },
            TileType::COMMERCIAL, 300, 50, 4);
    tileAtlas["industrial"] =
        Tile(this->tileSize, 2, texmgr.getRef("industrial"),
            { staticAnim, staticAnim, staticAnim,
            staticAnim },
            TileType::INDUSTRIAL, 300, 50, 4);
    tileAtlas["road"] =
        Tile(this->tileSize, 1, texmgr.getRef("road"),
            { staticAnim, staticAnim, staticAnim,
            staticAnim, staticAnim, staticAnim,
            staticAnim, staticAnim, staticAnim,
            staticAnim, staticAnim },
            TileType::ROAD, 100, 0, 1);

    return;
}

Game::Game() : background(TextureManager::getNullTexture())
{
    // 1. Create Window
    this->window.create(sf::VideoMode({ 800, 600 }), "City Builder");
    this->window.setFramerateLimit(60);

    // 2. Load Textures
    this->loadTextures();
    this->loadTiles();

    // 3. Link Texture
    sf::Texture& bgTex = this->texmgr.getRef("background");
    this->background.setTexture(bgTex);

    // 4. THE FIX: Force the Sprite to look at the whole image
    sf::Vector2u size = bgTex.getSize();
    this->background.setTextureRect(sf::IntRect({ 0, 0 }, { static_cast<int>(size.x), static_cast<int>(size.y) }));

    // 5. Default position/scale
    this->background.setPosition({ 0.f, 0.f });
    this->background.setScale({ 1.f, 1.f });

    //std::cout << "Background size: " << size.x << "x" << size.y << std::endl; //Test to make sure code ran to this point
}

Game::~Game()
{
    while (!this->states.empty()) popState();
}