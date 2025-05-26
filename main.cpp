#include <SFML/Graphics.hpp>
#include <vector> // Required for std::vector
#include <iostream> // Required for std::cout
#include <cstdlib>  // For std::rand and std::srand
#include <ctime>    // For std::time
#include <string>   // Required for std::to_string
#include <SFML/System/Clock.hpp> // Required for sf::Clock

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

const float PLAYER_WIDTH = 50.f;
const float PLAYER_HEIGHT = 50.f;

const float TREE_SEGMENT_WIDTH = 100.f;
const float TREE_SEGMENT_HEIGHT = 100.f;

// Enum for player side
enum class PlayerSide { LEFT, RIGHT };

// Enum for branch side
enum class BranchSide { NONE, LEFT_BRANCH, RIGHT_BRANCH };

// Struct for tree segments
struct TreeSegment {
    sf::Sprite trunkSprite;
    sf::Sprite branchSprite; // Sprite for the branch itself
    BranchSide branchSide; // NONE, LEFT_BRANCH, RIGHT_BRANCH (used for logic)
    bool hasBranch; // To easily know if we need to draw branchSprite
};

// Function to update player's X position based on side
void updatePlayerXPosition(PlayerSide side, sf::Sprite& playerSprite) { // Changed to sf::Sprite
    float newX;
    // Assuming player sprite origin is top-left. Adjust if necessary.
    // These calculations might need tweaking based on player sprite's actual width
    if (side == PlayerSide::LEFT) {
        newX = (WINDOW_WIDTH / 2.f) - (TREE_SEGMENT_WIDTH / 2.f) - playerSprite.getGlobalBounds().width - 5.f;
    } else { // PlayerSide::RIGHT
        newX = (WINDOW_WIDTH / 2.f) + (TREE_SEGMENT_WIDTH / 2.f) + 5.f;
    }
    playerSprite.setPosition(newX, playerSprite.getPosition().y);
}

int main()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr))); // Seed random number generator

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Timberman");

    // Asset Paths
    std::string backgroundTexturePath = "assets/background.png";
    std::string playerTexturePath = "assets/player_sheet.png";
    std::string treeTrunkTexturePath = "assets/tree_trunk.png";
    std::string branchTexturePath = "assets/branch.png";
    std::string cloudTexturePath = "assets/cloud.png";
    std::string ripTexturePath = "assets/rip.png";
    std::string axeTexturePath = "assets/axe.png";

    // Load Textures
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile(backgroundTexturePath)) { std::cerr << "Error loading " << backgroundTexturePath << std::endl; }
    sf::Texture playerTexture;
    if (!playerTexture.loadFromFile(playerTexturePath)) { std::cerr << "Error loading " << playerTexturePath << std::endl; }
    sf::Texture treeTrunkTexture;
    if (!treeTrunkTexture.loadFromFile(treeTrunkTexturePath)) { std::cerr << "Error loading " << treeTrunkTexturePath << std::endl; }
    sf::Texture branchTexture;
    if (!branchTexture.loadFromFile(branchTexturePath)) { std::cerr << "Error loading " << branchTexturePath << std::endl; }
    sf::Texture cloudTexture;
    if (!cloudTexture.loadFromFile(cloudTexturePath)) { std::cerr << "Error loading " << cloudTexturePath << std::endl; }
    sf::Texture ripTexture;
    if (!ripTexture.loadFromFile(ripTexturePath)) { std::cerr << "Error loading " << ripTexturePath << std::endl; }
    sf::Texture axeTexture;
    if (!axeTexture.loadFromFile(axeTexturePath)) { std::cerr << "Error loading " << axeTexturePath << std::endl; }

    // Create Sprites
    sf::Sprite backgroundSprite;
    backgroundSprite.setTexture(backgroundTexture);
    // backgroundSprite.setPosition(0,0); // Usually default is fine for full background

    sf::Sprite playerSprite;
    playerSprite.setTexture(playerTexture);
    // playerSprite.setTextureRect(sf::IntRect(0,0,width,height)); // For sprite sheet, define the part to use
    // For now, assume playerTexture is a single frame or first frame is okay
    // Adjust origin if needed, e.g. playerSprite.setOrigin(width/2, height/2);
    
    PlayerSide currentPlayerSide = PlayerSide::LEFT;
    // Set initial player Y position (X will be set by updatePlayerXPosition)
    // Need to consider player sprite's height for ground positioning.
    // For now, using PLAYER_HEIGHT constant. This may need adjustment.
    playerSprite.setPosition(0, WINDOW_HEIGHT - PLAYER_HEIGHT - 10.f); 
    updatePlayerXPosition(currentPlayerSide, playerSprite);

    sf::Sprite axeSprite;
    axeSprite.setTexture(axeTexture);

    sf::Sprite cloudSprite1, cloudSprite2;
    cloudSprite1.setTexture(cloudTexture);
    cloudSprite2.setTexture(cloudTexture);
    cloudSprite1.setPosition(100.f, 50.f);
    cloudSprite2.setPosition(500.f, 80.f);
    // cloudSprite1.setScale(0.5f, 0.5f); // Example scale
    // cloudSprite2.setScale(0.6f, 0.6f); // Example scale


    std::vector<TreeSegment> treeSegments;
    int initialSegments = WINDOW_HEIGHT / static_cast<int>(TREE_SEGMENT_HEIGHT); // Assuming TREE_SEGMENT_HEIGHT is still relevant for number
    for (int i = 0; i < initialSegments; ++i) {
        TreeSegment ts;
        ts.trunkSprite.setTexture(treeTrunkTexture);
        // Assuming TREE_SEGMENT_WIDTH is still relevant for positioning
        ts.trunkSprite.setPosition(
            (WINDOW_WIDTH / 2.f) - (TREE_SEGMENT_WIDTH / 2.f), // Center the trunk
            WINDOW_HEIGHT - (i + 1) * TREE_SEGMENT_HEIGHT // Stack from bottom
        );
        ts.branchSide = BranchSide::NONE; // Initial segments have no branches
        ts.hasBranch = false;
        treeSegments.push_back(ts);
    }
    
    sf::Sprite ripSprite;
    ripSprite.setTexture(ripTexture);
    // Center RIP sprite - this needs texture size, so set it after loading if possible
    // ripSprite.setOrigin(ripTexture.getSize().x / 2.f, ripTexture.getSize().y / 2.f);
    ripSprite.setPosition(WINDOW_WIDTH / 2.f - ripTexture.getSize().x / 2.f, WINDOW_HEIGHT / 2.f - ripTexture.getSize().y / 2.f);


    bool isGameOver = false;
    int score = 0;

    // Time management
    sf::Clock gameClock;
    float timeRemaining = 6.f;
    const float timeBarStartWidth = 400.f;
    const float timeBarHeight = 20.f;
    float timePerChop = 0.5f; 
    const float maxTime = 10.f;
    float depletionRate = 1.0f;

    sf::Font font; // Keep font for score and potentially other UI text
    bool fontLoaded = font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
    if (!fontLoaded) {
        std::cout << "Error: Font DejaVuSans.ttf not found. Game Over text will not be displayed." << std::endl;
    }

    // sf::Text gameOverText; // Removed as per instruction to use ripSprite
    // if (fontLoaded) {
    //     gameOverText.setFont(font);
    //     gameOverText.setString("Game Over!");
    //     gameOverText.setCharacterSize(50);
    //     gameOverText.setFillColor(sf::Color::White);
    //     sf::FloatRect textRect = gameOverText.getLocalBounds();
    //     gameOverText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    //     gameOverText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
    // }

    sf::Text scoreText; // Keep score text
    if (fontLoaded) {
        scoreText.setFont(font);
        scoreText.setString("Score: 0");
        scoreText.setCharacterSize(30);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(10.f, 10.f); // Top-left corner
    }

    sf::RectangleShape timeBar;
    timeBar.setSize(sf::Vector2f(timeBarStartWidth, timeBarHeight));
    timeBar.setFillColor(sf::Color::Yellow);
    timeBar.setPosition((WINDOW_WIDTH - timeBarStartWidth) / 2.f, 50.f);


    while (window.isOpen())
    {
        sf::Time dt = gameClock.restart(); // Restart clock at the beginning of the loop for dt

        if (!isGameOver) { // Update time related logic only if game is not over
            timeRemaining -= dt.asSeconds() * depletionRate;
            if (timeRemaining <= 0.f) {
                timeRemaining = 0.f;
                isGameOver = true;
                std::cout << "Game Over! Out of time." << std::endl;
            }

            float timeBarCurrentWidth = (timeRemaining / maxTime) * timeBarStartWidth;
            if(maxTime == 0) timeBarCurrentWidth = 0; // Avoid division by zero if maxTime could be 0
            timeBar.setSize(sf::Vector2f(timeBarCurrentWidth, timeBarHeight));
        }

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (!isGameOver) { 
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Left) {
                        currentPlayerSide = PlayerSide::LEFT;
                        updatePlayerXPosition(currentPlayerSide, playerSprite);
                    } else if (event.key.code == sf::Keyboard::Right) {
                        currentPlayerSide = PlayerSide::RIGHT;
                        updatePlayerXPosition(currentPlayerSide, playerSprite);
                    } else if (event.key.code == sf::Keyboard::Space) {
                        // Chop action
                        if (!treeSegments.empty()) {
                            treeSegments.erase(treeSegments.begin()); 

                            for (size_t i = 0; i < treeSegments.size(); ++i) {
                                // Update Y position of existing trunk sprites
                                treeSegments[i].trunkSprite.setPosition(
                                    treeSegments[i].trunkSprite.getPosition().x,
                                    WINDOW_HEIGHT - (i + 1) * TREE_SEGMENT_HEIGHT
                                );
                                // And branch sprites if they exist
                                if (treeSegments[i].hasBranch) {
                                    treeSegments[i].branchSprite.setPosition(
                                        treeSegments[i].trunkSprite.getPosition().x + (treeSegments[i].branchSide == BranchSide::LEFT_BRANCH ? -TREE_SEGMENT_WIDTH / 2.f - branchTexture.getSize().x /2.f : TREE_SEGMENT_WIDTH / 2.f + branchTexture.getSize().x /2.f), // Adjust X
                                        treeSegments[i].trunkSprite.getPosition().y // Y should align with trunk
                                    );
                                }
                            }

                            TreeSegment newSegment;
                            newSegment.trunkSprite.setTexture(treeTrunkTexture);
                            newSegment.trunkSprite.setPosition(
                                (WINDOW_WIDTH / 2.f) - (TREE_SEGMENT_WIDTH / 2.f),
                                WINDOW_HEIGHT - (treeSegments.size() + 1) * TREE_SEGMENT_HEIGHT 
                            );
                            newSegment.hasBranch = false; // Default

                            BranchSide previousBranchSide = BranchSide::NONE;
                            if (!treeSegments.empty()) {
                               previousBranchSide = treeSegments.back().branchSide; 
                            }
                            
                            int randVal = std::rand() % 3; 
                            if (randVal == 0) {
                                newSegment.branchSide = BranchSide::NONE;
                            } else if (randVal == 1) { // Attempt LEFT_BRANCH
                                if (previousBranchSide != BranchSide::LEFT_BRANCH) {
                                    newSegment.branchSide = BranchSide::LEFT_BRANCH;
                                    newSegment.hasBranch = true;
                                } else {
                                    newSegment.branchSide = BranchSide::NONE; 
                                }
                            } else { // Attempt RIGHT_BRANCH
                                if (previousBranchSide != BranchSide::RIGHT_BRANCH) {
                                    newSegment.branchSide = BranchSide::RIGHT_BRANCH;
                                    newSegment.hasBranch = true;
                                } else {
                                    newSegment.branchSide = BranchSide::NONE; 
                                }
                            }
                            
                            if (newSegment.hasBranch) {
                                newSegment.branchSprite.setTexture(branchTexture);
                                float branchX = newSegment.trunkSprite.getPosition().x;
                                if (newSegment.branchSide == BranchSide::LEFT_BRANCH) {
                                    // Position branch to the left of trunk. Adjust based on origin and texture size
                                    // Example: branchX -= newSegment.trunkSprite.getGlobalBounds().width / 2 + newSegment.branchSprite.getGlobalBounds().width / 2;
                                    branchX -= branchTexture.getSize().x; // Simplified: assumes branch origin top-left, places it left of trunk
                                    newSegment.branchSprite.setScale(1.f, 1.f); // Normal scale for left
                                } else { // RIGHT_BRANCH
                                    // Position branch to the right. Example:
                                    // branchX += newSegment.trunkSprite.getGlobalBounds().width / 2 + newSegment.branchSprite.getGlobalBounds().width / 2;
                                    branchX += TREE_SEGMENT_WIDTH; // Simplified: assumes branch origin top-left, places it right of trunk
                                    newSegment.branchSprite.setScale(1.f, 1.f); // Might need -1.f on X if asset faces one way
                                }
                                newSegment.branchSprite.setPosition(branchX, newSegment.trunkSprite.getPosition().y);
                            }
                            treeSegments.push_back(newSegment);

                            score++;
                            if(fontLoaded) {
                                scoreText.setString("Score: " + std::to_string(score));
                            }
                            
                            timeRemaining += timePerChop;
                            if (timeRemaining > maxTime) {
                                timeRemaining = maxTime;
                            }
                            depletionRate = 1.0f + (score / 50.f); // Update depletion rate

                            if (!treeSegments.empty()) {
                                BranchSide currentSegmentBranchSide = treeSegments[0].branchSide; // Check branchSide
                                if ((currentPlayerSide == PlayerSide::LEFT && currentSegmentBranchSide == BranchSide::LEFT_BRANCH) ||
                                    (currentPlayerSide == PlayerSide::RIGHT && currentSegmentBranchSide == BranchSide::RIGHT_BRANCH)) {
                                    isGameOver = true;
                                    std::cout << "Game Over! Collision." << std::endl;
                                }
                            }
                        }
                    }
                }
            }
        }

        window.clear(isGameOver ? sf::Color::Red : sf::Color::Black); // Keep red screen on game over for now

        window.draw(backgroundSprite);
        window.draw(cloudSprite1);
        window.draw(cloudSprite2);

        if (!isGameOver) {
            for (const auto& segment : treeSegments) { 
                window.draw(segment.trunkSprite);
                if (segment.hasBranch) {
                    window.draw(segment.branchSprite);
                }
            }
            window.draw(playerSprite);

            // Update axe position based on player
            float axeX = playerSprite.getPosition().x;
            float axeY = playerSprite.getPosition().y + PLAYER_HEIGHT * 0.3f; // Adjust Y based on player sprite
            if (currentPlayerSide == PlayerSide::LEFT) {
                axeX += playerSprite.getGlobalBounds().width * 0.8f; // Axe to the right of player
                axeSprite.setScale(1.f, 1.f);
            } else { // PlayerSide::RIGHT
                axeX -= axeSprite.getGlobalBounds().width * 0.8f; // Axe to the left of player, adjust for axe width
                axeSprite.setScale(-1.f, 1.f); // Flip axe
            }
            axeSprite.setPosition(axeX, axeY);
            window.draw(axeSprite);


            if (fontLoaded) {
                window.draw(scoreText);
            }
            window.draw(timeBar); // Draw the time bar
        } else {
            // If font loaded and ripTexture loaded, draw ripSprite
            // if (ripTexture.getSize().x > 0) // Check if texture is valid
            window.draw(ripSprite); 
            // else if (fontLoaded) { // Fallback to text if rip sprite fails
            //    window.draw(gameOverText); // gameOverText was removed, so this won't compile.
            // }
             if (fontLoaded) { // Keep score visible on game over for now
                // Optionally recenter score text or make it more prominent
                window.draw(scoreText);
             }
        }
        window.display();
    }

    return 0;
}
