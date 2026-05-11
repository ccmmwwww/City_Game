#include <SFML/Graphics.hpp>

#include "game_state.hpp"
#include "game_state_editor.hpp"

void GameStateEditor::draw(const float dt)
{
    this->game->getWindow().setView(this->gameView);
    this->game->getWindow().draw(this->game->background);

    return;
}

void GameStateEditor::update(const float dt)
{
    return;
}

void GameStateEditor::handleInput()
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
            this->gameView.setSize({ static_cast<float>(resized->size.x), static_cast<float>(resized->size.y) });
            this->guiView.setSize({ static_cast<float>(resized->size.x), static_cast<float>(resized->size.y) });
            this->game->background.setPosition(this->game->getWindow().mapPixelToCoords({0, 0}));

            auto textureSize = this->game->background.getTexture().getSize();
            this->game->background.setScale({
                static_cast<float>(resized->size.x) / textureSize.x,
                static_cast<float>(resized->size.y) / textureSize.y
                });
        }
    }

    return;
}

GameStateEditor::GameStateEditor(Game* game)
{
    this->game = game;
    sf::Vector2f pos = sf::Vector2f(this->game->getWindow().getSize());
    this->guiView.setSize(pos);
    this->gameView.setSize(pos);
    pos *= 0.5f;
    this->guiView.setCenter(pos);
    this->gameView.setCenter(pos);
}