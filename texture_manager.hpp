#ifndef TEXTURE_MANAGER_HPP
#define TEXTURE_MANAGER_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <map>
#include <memory> // Required for unique_ptr

class TextureManager
{
private:
    // Change to unique_ptr to keep memory addresses stable
    std::map<std::string, std::unique_ptr<sf::Texture>> textures;

public:
    static sf::Texture& getNullTexture();
    void loadTexture(const std::string& name, const std::string& filename);
    sf::Texture& getRef(const std::string& texture);
    // ...
};

#endif