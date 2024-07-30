#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
#include <ctime>

class Item {
public:
    std::string name;
    std::string description;

    Item(std::string n, std::string d) : name(n), description(d) {}
};

class Enemy {
public:
    std::string type;
    int health;
    int damage;

    Enemy() : type(""), health(0), damage(0) {}
    Enemy(std::string t, int h, int d) : type(t), health(h), damage(d) {}
};

class Player {
public:
    int health;
    int armor;
    std::vector<Item> inventory;

    Player() : health(100), armor(0) {}

    void addItem(Item item) {
        inventory.push_back(item);
    }

    bool hasItem(std::string itemName) {
        for (auto& item : inventory) {
            if (item.name == itemName) {
                return true;
            }
        }
        return false;
    }

    void removeItem(std::string itemName) {
        inventory.erase(std::remove_if(inventory.begin(), inventory.end(),
            [&itemName](Item& item) { return item.name == itemName; }),
            inventory.end());
    }

    bool useHealingPotion() {
        if (hasItem("potion")) {
            removeItem("potion");
            health += 20;
            if (!hasItem("shield")) {
                health -= 10; // Take damage while healing if not carrying a shield
                std::cout << "You took 10 damage while healing because you don't have a shield.\n";
            }
            return true;
        }
        return false;
    }

    bool hasAllItems() {
        return hasItem("sword") && hasItem("shield") && hasItem("armor");
    }

    void takeDamage(int damage) {
        health -= std::max(0, damage - armor); // Reduce damage by armor value, but not below zero
    }
};

class Game {
public:
    Player player;
    std::map<std::string, std::string> rooms;
    std::map<std::string, std::vector<std::string>> roomConnections;
    std::map<std::string, std::vector<Item>> initialRoomItems;
    std::map<std::string, Enemy> initialRoomEnemies;
    std::map<std::string, std::vector<Item>> roomItems;
    std::map<std::string, Enemy> roomEnemies;
    std::string currentRoom;
    bool hasWon = false;

    Game() {
        // Define rooms
        rooms["start"] = "You are in a dark room. There is a door to the north and south.";
        rooms["hallway"] = "You are in a hallway. There is a locked door to the north, a door to the south, a door to the east, and a door to the west.";
        rooms["hidden room"] = "You found a hidden room with some items. You can go west to return to the start.";
        rooms["boss room"] = "You are in the boss room. There is an enemy here.";
        rooms["storage room"] = "You are in a storage room. There are some useful items here. You can go west to return to the hallway.";
        rooms["trap room"] = "You are in a trap room. Every action you take here deals 20 damage to you. The only way out is west.";
        rooms["library"] = "You are in a library filled with old books. You can go east to return to the hallway.";
        rooms["armory"] = "You are in an armory with various weapons and armor. You can go west to return to the garden.";
        rooms["garden"] = "You are in a peaceful garden. You can go north to the troll's lair, south to the hallway, east to the trap room, or west to the armory.";
        rooms["dungeon"] = "You are in a dark, damp dungeon. You can go east to return to the library.";
        rooms["goblin room"] = "You are in a room with a goblin! You can go north to return to the start.";
        rooms["troll's lair"] = "You are in the troll's lair! The troll attacks you on sight!";
        rooms["throne room"] = "You have entered the throne room, filled with riches beyond imagining.";

        // Define room connections
        roomConnections["start"] = { "north", "south" };
        roomConnections["hallway"] = { "south", "east", "west", "north" };
        roomConnections["hidden room"] = { "west" }; // Can go back to start
        roomConnections["boss room"] = { "west" };
        roomConnections["storage room"] = { "west" };
        roomConnections["trap room"] = { "west" };
        roomConnections["library"] = { "east" };
        roomConnections["armory"] = { "west" };
        roomConnections["garden"] = { "north", "south", "east", "west" };
        roomConnections["dungeon"] = { "east" };
        roomConnections["goblin room"] = { "north" };
        roomConnections["troll's lair"] = { "south", "north", "east" };
        roomConnections["throne room"] = {};
        roomConnections["secret boss room"] = { "west" };

        // Define initial room items
        initialRoomItems["start"].push_back(Item("torch", "A small torch to light your way."));
        initialRoomItems["hallway"].push_back(Item("potion", "A healing potion."));
        initialRoomItems["hidden room"].push_back(Item("shield", "A shield to protect yourself."));
        initialRoomItems["storage room"].push_back(Item("sword", "A sharp sword to fight enemies."));
        initialRoomItems["library"].push_back(Item("book", "An old book with strange symbols."));
        initialRoomItems["armory"].push_back(Item("armor", "A sturdy set of armor."));
        initialRoomItems["garden"].push_back(Item("potion", "A healing potion."));
        initialRoomItems["trap room"].push_back(Item("potion", "A healing potion."));
        initialRoomItems["dungeon"].push_back(Item("lockpick", "A lockpick for unlocking doors."));
        initialRoomItems["goblin room"].push_back(Item("key", "A small rusty key."));

        // Define initial room enemies
        initialRoomEnemies["boss room"] = Enemy("Secret Boss", 80, 20);
        initialRoomEnemies["goblin room"] = Enemy("Goblin", 30, 10);
        initialRoomEnemies["troll's lair"] = Enemy("Troll", 100, 20);

        // Copy initial state to current state
        resetGameWorld();
    }

    void play() {
        do {
            resetGame();
            gameLoop();
        } while (askToPlayAgain());
    }

private:
    void gameLoop() {
        printInstructions();
        std::string command;
        while (!hasWon && player.health > 0) {
            if (!command.empty()) {
                std::cout << std::endl;
            }
            printRoomDescription();
            printExits();
            checkForEnemy();
            if (!hasWon && player.health > 0) {
                std::cout << "Enter command: ";
                std::getline(std::cin, command);
                handleCommand(toLower(trim(command)));
            }
        }

        if (hasWon) {
            std::cout << "You win!\n";
            std::cout << "You have entered the throne room, filled with riches beyond imagining.\n";
        }
        else {
            std::cout << "You lose!\n";
        }
    }

    void resetGame() {
        player.health = 100;
        player.armor = 0;
        player.inventory.clear();
        currentRoom = "start";
        hasWon = false;
        resetGameWorld();
    }

    void resetGameWorld() {
        roomItems = initialRoomItems;
        roomEnemies = initialRoomEnemies;
        roomConnections["start"] = { "north", "south" }; // Reset hidden room connection
    }

    bool askToPlayAgain() {
        std::string input;
        std::cout << "Would you like to try again? (yes/no): ";
        std::getline(std::cin, input);
        return toLower(trim(input)) == "yes";
    }

    void printInstructions() {
        std::cout << "Welcome to the Text Adventure Game!\n";
        std::cout << "Available commands: north, south, east, west, search, attack, flee, heal\n";
        std::cout << "Your objective is to find the key, unlock the door, and defeat the troll.\n";
        std::cout << "Good luck!\n\n";
    }

    void printRoomDescription() {
        static std::string lastRoom = "";
        if (currentRoom != lastRoom) {
            std::cout << rooms[currentRoom] << std::endl;
            lastRoom = currentRoom;
        }
    }

    void printExits() {
        std::cout << "Exits: ";
        for (const auto& exit : roomConnections[currentRoom]) {
            std::cout << exit << " ";
        }
        std::cout << std::endl;
    }

    void checkForEnemy() {
        if (roomEnemies.find(currentRoom) != roomEnemies.end()) {
            encounterEnemy(roomEnemies[currentRoom]);
        }
    }

    void handleCommand(std::string command) {
        if (currentRoom == "trap room") {
            player.takeDamage(20);
            std::cout << "You take 20 damage from the traps! Your health is now " << player.health << ".\n";
            if (player.health <= 0) {
                std::cout << "You have succumbed to the traps.\n";
                return;
            }
        }

        if (command == "north") {
            move("north");
        }
        else if (command == "south") {
            move("south");
        }
        else if (command == "east") {
            move("east");
        }
        else if (command == "west") {
            move("west");
        }
        else if (command == "search") {
            searchRoom();
        }
        else if (command == "attack") {
            std::cout << "There's nothing to attack here." << std::endl;
        }
        else if (command == "heal") {
            if (player.useHealingPotion()) {
                std::cout << "You used a healing potion. Current health: " << player.health << std::endl;
            }
            else {
                std::cout << "You don't have any healing potions." << std::endl;
            }
        }
        else if (command == "flee") {
            std::cout << "There's nothing to flee from here." << std::endl;
        }
        else {
            std::cout << "Unknown command." << std::endl;
        }
    }

    void move(std::string direction) {
        if (std::find(roomConnections[currentRoom].begin(), roomConnections[currentRoom].end(), direction) != roomConnections[currentRoom].end()) {
            if (currentRoom == "hallway" && direction == "north" && !player.hasItem("key")) {
                std::cout << "The door is locked. You need a key." << std::endl;
            }
            else {
                if (currentRoom == "start" && direction == "north") {
                    currentRoom = "hallway";
                }
                else if (currentRoom == "start" && direction == "south") {
                    currentRoom = "goblin room";
                }
                else if (currentRoom == "start" && direction == "east") {
                    if (std::find(roomConnections["start"].begin(), roomConnections["start"].end(), "east") != roomConnections["start"].end()) {
                        currentRoom = "hidden room";
                    }
                    else {
                        std::cout << "You can't go east from here." << std::endl;
                        return;
                    }
                }
                else if (currentRoom == "hallway" && direction == "south") {
                    currentRoom = "start";
                }
                else if (currentRoom == "hidden room" && direction == "west") {
                    currentRoom = "start";
                }
                else if (currentRoom == "trap room" && direction == "west") {
                    currentRoom = "garden";
                }
                else if (currentRoom == "boss room" && direction == "west") {
                    currentRoom = "troll's lair";
                }
                else if (currentRoom == "hallway" && direction == "east") {
                    currentRoom = "storage room";
                }
                else if (currentRoom == "hallway" && direction == "west") {
                    currentRoom = "library";
                }
                else if (currentRoom == "storage room" && direction == "west") {
                    currentRoom = "hallway";
                }
                else if (currentRoom == "library" && direction == "east") {
                    currentRoom = "hallway";
                }
                else if (currentRoom == "dungeon" && direction == "east") {
                    currentRoom = "library";
                }
                else if (currentRoom == "hallway" && direction == "north") {
                    currentRoom = "garden";
                }
                else if (currentRoom == "garden" && direction == "north") {
                    currentRoom = "troll's lair";
                }
                else if (currentRoom == "garden" && direction == "south") {
                    currentRoom = "hallway";
                }
                else if (currentRoom == "garden" && direction == "east") {
                    currentRoom = "trap room";
                }
                else if (currentRoom == "garden" && direction == "west") {
                    currentRoom = "armory";
                }
                else if (currentRoom == "armory" && direction == "west") {
                    currentRoom = "garden";
                }
                else if (currentRoom == "goblin room" && direction == "north") {
                    currentRoom = "start";
                }
                else if (currentRoom == "troll's lair" && direction == "south") {
                    currentRoom = "garden";
                }
                else if (currentRoom == "troll's lair" && direction == "north") {
                    currentRoom = "throne room";
                    hasWon = true;
                }
                else if (currentRoom == "troll's lair" && direction == "east") {
                    currentRoom = "boss room";
                }
                else {
                    std::cout << "You can't go " << direction << " from here." << std::endl;
                }
            }
        }
        else {
            std::cout << "You can't go " << direction << " from here." << std::endl;
        }
    }

    void searchRoom() {
        if (!roomItems[currentRoom].empty()) {
            std::cout << "You found the following items:\n";
            for (const auto& item : roomItems[currentRoom]) {
                std::cout << "- " << item.name << ": " << item.description << std::endl;
                player.addItem(item);
                if (item.name == "book") {
                    roomConnections["start"].push_back("east");
                    std::cout << "The book has revealed a hidden passage to the east in the start room!\n";
                }
                else if (item.name == "armor") {
                    player.armor = 5;
                    std::cout << "You equipped the armor. Damage taken is reduced by 5.\n";
                }
            }
            roomItems[currentRoom].clear(); // Remove items after searching
        }
        else {
            std::cout << "There is nothing to search here." << std::endl;
        }
    }

    void encounterEnemy(Enemy& enemy) {
        std::string action;

        while (enemy.health > 0 && player.health > 0) {
            std::cout << "You encounter a " << enemy.type << "! Health: " << enemy.health << std::endl;
            std::cout << "Enter action (attack/flee/heal): ";
            std::getline(std::cin, action);

            std::string lowerAction = toLower(trim(action));

            if (lowerAction == "attack") {
                int damage = player.hasItem("sword") ? 20 : 5; // 5 damage without sword, 20 with sword
                enemy.health -= damage;
                std::cout << "You attack the " << enemy.type << "! Its health is now " << enemy.health << std::endl;
                if (enemy.health > 0) {
                    player.takeDamage(enemy.damage);
                    std::cout << "The " << enemy.type << " attacks you! Your health is now " << player.health << std::endl;
                }
            }
            else if (lowerAction == "flee") {
                currentRoom = "garden";
                std::cout << "You fled back to the garden." << std::endl;
                break;
            }
            else if (lowerAction == "heal") {
                if (player.useHealingPotion()) {
                    std::cout << "You used a healing potion. Current health: " << player.health << std::endl;
                }
                else {
                    std::cout << "You don't have any healing potions." << std::endl;
                }
            }
            else {
                std::cout << "Unknown action." << std::endl;
            }
        }

        if (enemy.health <= 0) {
            std::cout << "You defeated the " << enemy.type << "!" << std::endl;
            roomEnemies.erase(currentRoom); // Remove enemy from the room after defeat
            if (enemy.type == "Troll") {
                currentRoom = "throne room";
                hasWon = true;
            }
        }

        if (player.health <= 0) {
            std::cout << "You have been defeated by the " << enemy.type << "." << std::endl;
        }
    }

    std::string toLower(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
            [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    std::string trim(const std::string& str) {
        const char* whitespace = " \t\n\r\f\v";
        std::string result = str;
        result.erase(result.find_last_not_of(whitespace) + 1);
        result.erase(0, result.find_first_not_of(whitespace));
        return result;
    }
};

int main() {
    srand(static_cast<unsigned int>(time(nullptr))); // Initialize random seed
    Game game;
    game.play();
    return 0;
}
