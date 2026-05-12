#include <SFML/Graphics.hpp>

#include "game_state.hpp"
#include "game_state_editor.hpp"
#include "map.hpp"

void GameStateEditor::draw(const float dt)
{
    this->game->getWindow().clear(sf::Color::Black);

    this->game->getWindow().setView(this->guiView);
    this->game->getWindow().draw(this->game->background);

    this->game->getWindow().setView(this->gameView);
    this->city.map.draw(this->game->getWindow(), dt);

    this->game->getWindow().setView(this->guiView);
    for (auto gui : this->guiSystem) this->game->getWindow().draw(gui.second);

    return;
}

void GameStateEditor::update(const float dt)
{
    this->city.update(dt);

    /* Update the info bar at the bottom of the screen */
    this->guiSystem.at("infoBar").setEntryText(0, "Day: " + std::to_string(this->city.day));
    this->guiSystem.at("infoBar").setEntryText(1, "$" + std::to_string(long(this->city.funds)));
    this->guiSystem.at("infoBar").setEntryText(2, std::to_string(long(this->city.population)) + " (" + std::to_string(long(this->city.getHomeless())) + ")");
    this->guiSystem.at("infoBar").setEntryText(3, std::to_string(long(this->city.employable)) + " (" + std::to_string(long(this->city.getUnemployed())) + ")");
    this->guiSystem.at("infoBar").setEntryText(4, tileTypeToStr(currentTile->tileType));

    /* Highlight entries of the right click context menu */
    this->guiSystem.at("rightClickMenu").highlight(this->guiSystem.at("rightClickMenu").getEntry(this->game->getWindow().mapPixelToCoords(sf::Mouse::getPosition(this->game->getWindow()), this->guiView)));

    return;
}

void GameStateEditor::handleInput()
{
    sf::Vector2f guiPos = this->game->getWindow().mapPixelToCoords(sf::Mouse::getPosition(this->game->getWindow()), this->guiView);
    sf::Vector2f gamePos = this->game->getWindow().mapPixelToCoords(sf::Mouse::getPosition(this->game->getWindow()), this->gameView);
    // SFML 3: pollEvent returns a std::optional<sf::Event>
    while (const std::optional event = this->game->getWindow().pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            game->getWindow().close();
        }
        else if (const auto* resized = event->getIf<sf::Event::Resized>())
        {
            sf::Vector2f newSize{ static_cast<float>(resized->size.x), static_cast<float>(resized->size.y) };

            this->gameView.setSize(newSize * zoomLevel);
            this->guiView.setSize(newSize);

            this->guiSystem.at("infoBar").setDimensions(sf::Vector2f(newSize.x / this->guiSystem.at("infoBar").entries.size(), 16));
            this->guiSystem.at("infoBar").setPosition(this->game->getWindow().mapPixelToCoords(sf::Vector2i(0, newSize.y - 16), this->guiView));
            this->guiSystem.at("infoBar").show();
            this->game->background.setPosition(this->game->getWindow().mapPixelToCoords(sf::Vector2i(0, 0), this->guiView));
            sf::Vector2u texSize = this->game->background.getTexture().getSize();
            this->game->background.setScale({ newSize.x / static_cast<float>(texSize.x), newSize.y / static_cast<float>(texSize.y) });
        }
        else if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>())
        {
            if (this->actionState == ActionState::PANNING)
            {
                // Access window through getter and calculate pan
                sf::Vector2i mousePos = sf::Mouse::getPosition(this->game->getWindow());
                sf::Vector2f pos = sf::Vector2f(mousePos - this->panningAnchor);

                gameView.move(-1.0f * pos * this->zoomLevel);
                panningAnchor = mousePos;
            }
            else if (actionState == ActionState::SELECTING)
            {
                sf::Vector2f pos = this->game->getWindow().mapPixelToCoords(sf::Mouse::getPosition(this->game->getWindow()), this->gameView);
                selectionEnd.x = pos.y / (this->city.map.tileSize) + pos.x / (2 * this->city.map.tileSize) - this->city.map.width * 0.5 - 0.5;
                selectionEnd.y = pos.y / (this->city.map.tileSize) - pos.x / (2 * this->city.map.tileSize) + this->city.map.width * 0.5 + 0.5;

                this->city.map.clearSelected();
                if (this->currentTile->tileType == TileType::GRASS)
                {
                    this->city.map.select(selectionStart, selectionEnd, { this->currentTile->tileType, TileType::WATER });
                }
                else
                {
                    this->city.map.select(selectionStart, selectionEnd,
                        {
                            this->currentTile->tileType,    TileType::FOREST,
                            TileType::WATER,                TileType::ROAD,
                            TileType::RESIDENTIAL,          TileType::COMMERCIAL,
                            TileType::INDUSTRIAL
                        });
                }

                this->guiSystem.at("selectionCostText").setEntryText(0, "$" + std::to_string(this->currentTile->cost * this->city.map.numSelected));
                if (this->city.funds <= this->city.map.numSelected * this->currentTile->cost)
                    this->guiSystem.at("selectionCostText").highlight(0);
                else
                    this->guiSystem.at("selectionCostText").highlight(-1);
                this->guiSystem.at("selectionCostText").setPosition(guiPos + sf::Vector2f(16, -16));
                this->guiSystem.at("selectionCostText").show();
            }
            /* Highlight entries of the right click context menu */
            this->guiSystem.at("rightClickMenu").highlight(this->guiSystem.at("rightClickMenu").getEntry(guiPos));

        }
        else if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
        {
            if (mousePressed->button == sf::Mouse::Button::Middle)
            {
                this->guiSystem.at("rightClickMenu").hide();
                this->guiSystem.at("selectionCostText").hide();
                this->actionState = ActionState::PANNING;
                this->panningAnchor = sf::Mouse::getPosition(this->game->getWindow());
            }
            else if (mousePressed->button == sf::Mouse::Button::Left)
            {
                /* Select a context menu entry. */
                if (this->guiSystem.at("rightClickMenu").visible == true)
                {
                    std::string msg = this->guiSystem.at("rightClickMenu").activate(guiPos);
                    if (msg != "null") this->currentTile = &this->game->tileAtlas.at(msg);

                    this->guiSystem.at("rightClickMenu").hide();
                }
                /* Select map tile. */
                else
                {
                    this->actionState = ActionState::SELECTING;
                    sf::Vector2f pos = this->game->getWindow().mapPixelToCoords(sf::Mouse::getPosition(this->game->getWindow()), this->gameView);
                    selectionStart.x = pos.y / (this->city.map.tileSize) + pos.x / (2 * this->city.map.tileSize) - this->city.map.width * 0.5 - 0.5;
                    selectionStart.y = pos.y / (this->city.map.tileSize) - pos.x / (2 * this->city.map.tileSize) + this->city.map.width * 0.5 + 0.5;
                }
            }
            else if (mousePressed->button == sf::Mouse::Button::Right)
            {
                /* Stop selecting. */
                if (this->actionState == ActionState::SELECTING)
                {
                    this->actionState = ActionState::NONE;
                    this->guiSystem.at("selectionCostText").hide();
                    this->city.map.clearSelected();
                }
                else
                {
                    /* Open the tile select menu. */
                    sf::Vector2f pos = guiPos;

                    if (pos.x > this->game->getWindow().getSize().x - this->guiSystem.at("rightClickMenu").getSize().x)
                    {
                        pos -= sf::Vector2f(this->guiSystem.at("rightClickMenu").getSize().x, 0);
                    }
                    if (pos.y > this->game->getWindow().getSize().y - this->guiSystem.at("rightClickMenu").getSize().y)
                    {
                        pos -= sf::Vector2f(0, this->guiSystem.at("rightClickMenu").getSize().y);
                    }
                    this->guiSystem.at("rightClickMenu").setPosition(pos);
                    this->guiSystem.at("rightClickMenu").show();
                }
            }
        }
        else if (const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>())
        {
            /* Stop panning */
            if (mouseReleased->button == sf::Mouse::Button::Middle)
            {
                this->actionState = ActionState::NONE;
            }
            /* Stop selecting */
            else if (mouseReleased->button == sf::Mouse::Button::Left)
            {
                if (this->actionState == ActionState::SELECTING)
                {
                    /* Replace tiles if enough funds and a tile is selected */
                    if (this->currentTile != nullptr)
                    {
                        unsigned int cost = this->currentTile->cost * this->city.map.numSelected;
                        if (this->city.funds >= cost)
                        {
                            this->city.bulldoze(*this->currentTile);
                            this->city.funds -= this->currentTile->cost * this->city.map.numSelected;
                            this->city.tileChanged();
                        }
                    }
                    this->guiSystem.at("selectionCostText").hide();
                    this->actionState = ActionState::NONE;
                    this->city.map.clearSelected();
                }
            }
        }
        else if (const auto* wheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>())
        {
            // SFML 3 uses MouseWheelScrolled instead of MouseWheelMoved
            float zoom = (wheelScrolled->delta < 0) ? 2.0f : 0.5f;
            gameView.zoom(zoom);
            zoomLevel *= zoom;
        }
    }
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

    this->city = City("city", this->game->tileSize, this->game->tileAtlas);
    this->city.shuffleTiles();

    /* Create gui elements */
    this->guiSystem.emplace("rightClickMenu", Gui(sf::Vector2f(196, 16), 2, false, this->game->stylesheets.at("button"),
        {
            std::make_pair("Flatten $" + this->game->tileAtlas["grass"].getCost(), "grass"),
            std::make_pair("Forest $" + this->game->tileAtlas["forest"].getCost(), "forest"),
            std::make_pair("Residential Zone $" + this->game->tileAtlas["residential"].getCost(), "residential"),
            std::make_pair("Commercial Zone $" + this->game->tileAtlas["commercial"].getCost(), "commercial"),
            std::make_pair("Industrial Zone $" + this->game->tileAtlas["industrial"].getCost(), "industrial"),
            std::make_pair("Road $" + this->game->tileAtlas["road"].getCost(), "road")
        }));

    this->guiSystem.emplace("selectionCostText", Gui(sf::Vector2f(196, 16), 0, false, this->game->stylesheets.at("text"),
        { std::make_pair("", "") }));

    this->guiSystem.emplace("infoBar", Gui(sf::Vector2f(this->game->getWindow().getSize().x / 5, 16), 2, true, this->game->stylesheets.at("button"),
        { std::make_pair("time", "time"),
        std::make_pair("funds", "funds"),
        std::make_pair("population", "population"),
        std::make_pair("employment", "employment"),
        std::make_pair("current tile", "tile") }));
    this->guiSystem.at("infoBar").setPosition(sf::Vector2f(0, this->game->getWindow().getSize().y - 16));
    this->guiSystem.at("infoBar").show();

    this->zoomLevel = 1.0f;

    /* Centre the camera on the map */
    sf::Vector2f centre(static_cast<float>(this->city.map.width), static_cast<float>(this->city.map.height) * 0.5f);
    centre *= static_cast<float>(this->city.map.tileSize);
    gameView.setCenter(centre);

    this->selectionStart = sf::Vector2i(0, 0);
    this->selectionEnd = sf::Vector2i(0, 0);

    this->currentTile = &this->game->tileAtlas.at("grass");
    this->actionState = ActionState::NONE;
}