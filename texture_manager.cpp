#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <memory>
#include <iostream>
#include "texture_manager.hpp"

sf::Texture& TextureManager::getNullTexture() {
    static sf::Texture nullTex;
    return nullTex;
}

void TextureManager::loadTexture(const std::string& name, const std::string& filename)
{
    auto tex = std::make_unique<sf::Texture>();
    if (!tex->loadFromFile(filename))
    {
        std::cout << "Failed to load: " << filename << std::endl;
        return;
    }
    this->textures[name] = std::move(tex); // Move into the map
}

sf::Texture& TextureManager::getRef(const std::string& texture)
{
    // Return the actual texture object by dereferencing the unique_ptr
    return *this->textures.at(texture);
}