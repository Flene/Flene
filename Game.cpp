#include <iostream>
#include <string>

class Game {
public:
    Game(const std::string& name): gameName(name) {} 
    void start() {
        std::cout << "Starting the game: " << gameName << std::endl;
    }
    void end() {
        std::cout << "Ending the game: " << gameName << std::endl;
    }
private:
    std::string gameName;
};

int main() {
    Game myGame("Flene Adventure");
    myGame.start();
    // Game logic goes here
    myGame.end();
    return 0;
}