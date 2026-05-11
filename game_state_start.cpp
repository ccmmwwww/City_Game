#include <SFML/Graphics.hpp>

#include "game_state_start.hpp"
#include "game_state_editor.hpp"
#include "game_state.hpp"

void GameStateStart::draw(const float dt)
{
    // Force SFML to use the standard 800x600 window coordinates
    this->game->getWindow().setView(this->game->getWindow().getDefaultView());

    // Force the background to the top-left corner
    this->game->background.setPosition({ 0.f, 0.f });

    // Ensure the scale is 1:1 (so it's not size 0)
    this->game->background.setScale({ 1.f, 1.f });

    this->game->getWindow().draw(this->game->background);
}

void GameStateStart::update(const float dt)
{
}

void GameStateStart::handleInput()
{
    // SFML 3: pollEvent now returns a std::optional<sf::Event>
    while (const std::optional event = this->game->getWindow().pollEvent())
    {
        // 1. Close the window
        if (event->is<sf::Event::Closed>())
        {
            game->getWindow().close();
        }
        // 2. Resize the window
        else if (const auto* resized = event->getIf<sf::Event::Resized>())
        {
            // Data is accessed via the 'resized' pointer, not event.size
            this->view.setSize({ static_cast<float>(resized->size.x), static_cast<float>(resized->size.y) });
            this->game->background.setPosition(this->game->getWindow().mapPixelToCoords({0, 0}));

            auto textureSize = this->game->background.getTexture().getSize();
            this->game->background.setScale({
                static_cast<float>(resized->size.x) / textureSize.x,
                static_cast<float>(resized->size.y) / textureSize.y
                });
        }
        // 3. Key Pressed
        else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            if (keyPressed->code == sf::Keyboard::Key::Escape)
                this->game->getWindow().close();
            else if (keyPressed->code == sf::Keyboard::Key::Space)
                this->loadgame();
        }
    }
}

void GameStateStart::loadgame()
{
    this->game->pushState(std::make_unique<GameStateEditor>(this->game));
    return;
}

GameStateStart::GameStateStart(Game* game)
{
    this->game = game;
    sf::Vector2f size = sf::Vector2f(this->game->getWindow().getSize());
    this->view.setSize(size);
    this->view.setCenter(size * 0.5f);
}