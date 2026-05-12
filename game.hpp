#ifndef GAME_HPP
#define GAME_HPP

#include <stack>
#include <SFML/Graphics.hpp>
#include <memory>
#include <map>
#include <string>

#include "texture_manager.hpp"
#include "tile.hpp"
#include "gui.hpp"

class GameState;

class Game
{
private:

    sf::RenderWindow window;
    TextureManager texmgr;
    void loadTextures();
    void loadTiles();
    void loadStylesheets();
    void loadFonts();

public:

    const static int tileSize = 8;

    std::stack<std::unique_ptr<GameState>> states;

    sf::RenderWindow& getWindow() { return this->window; }
    TextureManager& getTextureManager() { return this->texmgr; }
    sf::Sprite background;

    std::map<std::string, Tile> tileAtlas;
    std::map<std::string, sf::Font> fonts;
    std::map<std::string, GuiStyle> stylesheets;

    void pushState(std::unique_ptr<GameState> state);
    void popState();
    void changeState(std::unique_ptr<GameState> state);
    GameState* peekState();

    void gameLoop();

    Game();
    ~Game();
};

#endif /* GAME_HPP */