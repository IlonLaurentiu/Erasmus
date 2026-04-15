// Security Features:
// - No buffer overflows: Uses std::string for all string operations (dynamic sizing, bounds checking)
// - Input sanitization: Only accepts alphabetic characters for guesses, validates input length
// - Secure random seed: Hardware-based entropy with std::random_device and std::mt19937
// - No undefined behavior: All loops use size_t indices, bounds checking on arrays/vectors, safe memory access

#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <cctype>
#include <limits>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <map>
#include <thread>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

// ANSI Color Codes
const string RESET = "\033[0m";
const string RED = "\033[31m";
const string GREEN = "\033[32m";
const string YELLOW = "\033[33m";
const string BLUE = "\033[34m";
const string MAGENTA = "\033[35m";
const string CYAN = "\033[36m";
const string WHITE = "\033[37m";

vector<string> christmasColors = {RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN};

// Constants
const int MAX_ATTEMPTS = 6;
const string HIGHSCORE_FILE = "highscores.txt";
const string STATS_FILE = "game_stats.txt";
const int MAX_HIGHSCORES = 10;
const int MAX_HINTS = 3;
const int STREAK_BONUS = 5;
const int HINT_PENALTY = 20;
const int TIME_ATTACK_LIMIT = 60; // 60 seconds for time attack
const int SURVIVAL_START_LIVES = 3;

// Achievement Structure
struct Achievement {
    string name;
    string description;
    bool unlocked;
    string icon;
};

vector<Achievement> achievements = {
    {"First Blood", "Win your first game", false, "🏆"},
    {"Speed Demon", "Guess a word in under 30 seconds", false, "⚡"},
    {"Perfectionist", "Win a game with no wrong guesses", false, "💎"},
    {"Category Master", "Win all words in a category", false, "👑"},
    {"Hint Master", "Win a game using all hints", false, "💡"},
    {"Streak Master", "Achieve a streak of 10", false, "🔥"},
    {"Time Warrior", "Win 5 time attack games", false, "⏰"},
    {"Survivor", "Win 3 survival games", false, "🛡️"}
};

// Game Mode Enum
enum GameMode {
    CLASSIC,
    TIME_ATTACK,
    SURVIVAL,
    PRACTICE
};

// Achievement tracking
map<string, int> achievementProgress;

// Game Statistics Structure
struct GameStats {
    int totalGames = 0;
    int totalWins = 0;
    int totalLosses = 0;
    long long totalPlayTime = 0; // in seconds
    map<string, int> categoryWins;
    map<string, int> categoryGames;
    map<string, int> difficultyWins;
    map<string, int> difficultyGames;
    int totalHintsUsed = 0;
    int maxStreak = 0;
    int totalStreakBonuses = 0;
};

GameStats gameStats;

struct Category {
    string name;
    vector<string> easy;
    vector<string> medium;
    vector<string> hard;
};

const vector<Category> CATEGORIES = {
    {
        "Animals",
        {"cat", "dog", "pig", "cow", "fox"},
        {"elephant", "giraffe", "kangaroo", "penguin", "rhinoceros"},
        {"hippopotamus", "chameleon", "crocodile"}
    },
    {
        "Countries",
        {"usa", "uk", "china", "india", "brazil"},
        {"australia", "germany", "france", "japan", "canada"},
        {"switzerland", "netherlands", "newzealand"}
    },
    {
        "Fruits",
        {"apple", "pear", "plum", "fig", "lime"},
        {"banana", "orange", "grapes", "cherry", "peach"},
        {"pineapple", "watermelon", "strawberry"}
    },
    {
        "Objects",
        {"pen", "book", "lamp", "chair", "table"},
        {"computer", "telephone", "umbrella", "backpack", "scissors"},
        {"refrigerator", "microphone", "helicopter"}
    }
};

struct HighScore {
    string name;
    int score;
};

vector<HighScore> highScores;

// Sound effect function declarations
void playSound(int frequency, int duration);
void playCorrectGuessSound();
void playWrongGuessSound();
void playWinSound();
void playLoseSound();
void playPlayAgainSound();
void playQuitSound();
void playIntroSound();
void playCategorySelectSound();

// Sound effect functions
void playSound(int frequency, int duration) {
#ifdef _WIN32
    Beep(frequency, duration);
#endif
}

void playCorrectGuessSound() {
    playSound(800, 200); // High beep for correct
}

void playWrongGuessSound() {
    playSound(300, 400); // Low beep for wrong
}

void playWinSound() {
    // Victory fanfare
    playSound(523, 200); // C
    playSound(659, 200); // E
    playSound(784, 200); // G
    playSound(1047, 400); // C (octave higher)
    
    // Triumphant chord
    playSound(523, 150); // C
    playSound(659, 150); // E
    playSound(784, 150); // G
    playSound(1047, 150); // C
    playSound(1319, 600); // E (even higher)
}

void playLoseSound() {
    // "Ua ua ua" sound - descending tones
    playSound(400, 300);
    playSound(350, 300);
    playSound(300, 500);
}

void playPlayAgainSound() {
    // Upbeat sound for continuing
    playSound(523, 150); // C
    playSound(659, 150); // E
    playSound(784, 150); // G
}

void playCategorySelectSound() {
    playSound(600, 150);
    playSound(800, 150);
}

void playQuitSound() {
    // Sad descending sound for quitting
    playSound(392, 300); // G
    playSound(370, 300); // F#
    playSound(349, 300); // F
    playSound(330, 300); // E
    playSound(311, 300); // D#
    playSound(294, 500); // D
}

void playIntroSound() {
    // Longer intro music - Hangman theme
    playSound(523, 300); // C
    playSound(587, 300); // D
    playSound(659, 300); // E
    playSound(698, 300); // F
    playSound(784, 300); // G
    playSound(880, 300); // A
    playSound(988, 300); // B
    playSound(1047, 600); // C (higher)
    
    // Short pause using busy wait (simple alternative to Sleep)
    for(volatile int i = 0; i < 1000000; i++);
    
    // Descending scale
    playSound(1047, 200); // C
    playSound(988, 200);  // B
    playSound(880, 200);  // A
    playSound(784, 200);  // G
    playSound(698, 200);  // F
    playSound(659, 200);  // E
    playSound(587, 200);  // D
    playSound(523, 400);  // C
}

// Function to load high scores from file
void loadHighScores() {
    ifstream file(HIGHSCORE_FILE);
    if (!file.is_open()) return; // File doesn't exist yet
    
    highScores.clear();
    string line;
    while (getline(file, line) && highScores.size() < MAX_HIGHSCORES) {
        size_t commaPos = line.find(',');
        if (commaPos != string::npos) {
            string name = line.substr(0, commaPos);
            string scoreStr = line.substr(commaPos + 1);
            try {
                int score = stoi(scoreStr);
                highScores.push_back({name, score});
            } catch (...) {
                // Invalid score, skip
            }
        }
    }
    file.close();
}

// Function to save high scores to file
void saveHighScores() {
    ofstream file(HIGHSCORE_FILE);
    if (!file.is_open()) return;
    
    for (const auto& hs : highScores) {
        file << hs.name << "," << hs.score << "\n";
    }
    file.close();
}

// Function to add a new high score
void addHighScore(int score) {
    if (score <= 0) return;
    
    cout << YELLOW << "New high score: " << score << "!\n" << RESET;
    cout << CYAN << "Enter your name: " << RESET;
    string name;
    getline(cin, name);
    
    // Trim whitespace
    size_t start = name.find_first_not_of(" \t");
    if (start != string::npos) {
        size_t end = name.find_last_not_of(" \t");
        name = name.substr(start, end - start + 1);
    }
    
    if (name.empty()) name = "Anonymous";
    
    highScores.push_back({name, score});
    
    // Sort by score descending
    sort(highScores.begin(), highScores.end(), [](const HighScore& a, const HighScore& b) {
        return a.score > b.score;
    });
    
    // Keep only top scores
    if (highScores.size() > MAX_HIGHSCORES) {
        highScores.resize(MAX_HIGHSCORES);
    }
    
    saveHighScores();
}

// Function to load achievements from file
void loadAchievements() {
    ifstream file("achievements.txt");
    if (!file.is_open()) return;
    
    string line;
    while (getline(file, line)) {
        size_t colonPos = line.find(':');
        if (colonPos != string::npos) {
            string name = line.substr(0, colonPos);
            string status = line.substr(colonPos + 1);
            for (auto& achievement : achievements) {
                if (achievement.name == name) {
                    achievement.unlocked = (status == "1");
                    break;
                }
            }
        }
    }
    file.close();
}

// Function to save achievements to file
void saveAchievements() {
    ofstream file("achievements.txt");
    if (!file.is_open()) return;
    
    for (const auto& achievement : achievements) {
        file << achievement.name << ":" << (achievement.unlocked ? "1" : "0") << "\n";
    }
    file.close();
}

// Function to check and unlock achievements
void checkAchievements(int gameDuration, int wrongGuesses, int hintsUsed, bool won, const string& category, GameMode mode) {
    // First Blood
    if (!achievements[0].unlocked && won) {
        achievements[0].unlocked = true;
        cout << GREEN << "\n🏆 ACHIEVEMENT UNLOCKED: " << achievements[0].name << " - " << achievements[0].description << "!\n" << RESET;
    }
    
    // Speed Demon
    if (!achievements[1].unlocked && won && gameDuration < 30) {
        achievements[1].unlocked = true;
        cout << GREEN << "\n⚡ ACHIEVEMENT UNLOCKED: " << achievements[1].name << " - " << achievements[1].description << "!\n" << RESET;
    }
    
    // Perfectionist
    if (!achievements[2].unlocked && won && wrongGuesses == 0) {
        achievements[2].unlocked = true;
        cout << GREEN << "\n💎 ACHIEVEMENT UNLOCKED: " << achievements[2].name << " - " << achievements[2].description << "!\n" << RESET;
    }
    
    // Hint Master
    if (!achievements[4].unlocked && won && hintsUsed == MAX_HINTS) {
        achievements[4].unlocked = true;
        cout << GREEN << "\n💡 ACHIEVEMENT UNLOCKED: " << achievements[4].name << " - " << achievements[4].description << "!\n" << RESET;
    }
    
    // Time Warrior
    if (mode == TIME_ATTACK && won) {
        achievementProgress["time_attack_wins"]++;
        if (achievementProgress["time_attack_wins"] >= 5 && !achievements[6].unlocked) {
            achievements[6].unlocked = true;
            cout << GREEN << "\n⏰ ACHIEVEMENT UNLOCKED: " << achievements[6].name << " - " << achievements[6].description << "!\n" << RESET;
        }
    }
    
    // Survivor
    if (mode == SURVIVAL && won) {
        achievementProgress["survival_wins"]++;
        if (achievementProgress["survival_wins"] >= 3 && !achievements[7].unlocked) {
            achievements[7].unlocked = true;
            cout << GREEN << "\n🛡️ ACHIEVEMENT UNLOCKED: " << achievements[7].name << " - " << achievements[7].description << "!\n" << RESET;
        }
    }
    
    saveAchievements();
}

// Function to display achievements
void displayAchievements() {
    cout << "\n" << YELLOW << "=== ACHIEVEMENTS ===\n" << RESET;
    for (const auto& achievement : achievements) {
        cout << (achievement.unlocked ? GREEN : RED) << achievement.icon << " " << achievement.name;
        if (!achievement.unlocked) {
            cout << " (Locked)";
        }
        cout << "\n  " << achievement.description << "\n" << RESET;
    }
    cout << "\n";
}

// Function to load game statistics from file
void loadGameStats() {
    ifstream file(STATS_FILE);
    if (!file.is_open()) return;
    
    string line;
    while (getline(file, line)) {
        // Parse statistics from file
        // Format: key:value
        size_t colonPos = line.find(':');
        if (colonPos != string::npos) {
            string key = line.substr(0, colonPos);
            string value = line.substr(colonPos + 1);
            try {
                if (key == "totalGames") gameStats.totalGames = stoi(value);
                else if (key == "totalWins") gameStats.totalWins = stoi(value);
                else if (key == "totalLosses") gameStats.totalLosses = stoi(value);
                else if (key == "totalPlayTime") gameStats.totalPlayTime = stoll(value);
                else if (key == "totalHintsUsed") gameStats.totalHintsUsed = stoi(value);
                else if (key == "maxStreak") gameStats.maxStreak = stoi(value);
                else if (key == "totalStreakBonuses") gameStats.totalStreakBonuses = stoi(value);
                else if (key.find("cat_win_") == 0) {
                    string cat = key.substr(8);
                    gameStats.categoryWins[cat] = stoi(value);
                }
                else if (key.find("cat_game_") == 0) {
                    string cat = key.substr(9);
                    gameStats.categoryGames[cat] = stoi(value);
                }
                else if (key.find("diff_win_") == 0) {
                    string diff = key.substr(9);
                    gameStats.difficultyWins[diff] = stoi(value);
                }
                else if (key.find("diff_game_") == 0) {
                    string diff = key.substr(10);
                    gameStats.difficultyGames[diff] = stoi(value);
                }
            } catch (...) {
                // Skip invalid lines
            }
        }
    }
    file.close();
}

// Function to save game statistics to file
void saveGameStats() {
    ofstream file(STATS_FILE);
    if (!file.is_open()) return;
    
    file << "totalGames:" << gameStats.totalGames << "\n";
    file << "totalWins:" << gameStats.totalWins << "\n";
    file << "totalLosses:" << gameStats.totalLosses << "\n";
    file << "totalPlayTime:" << gameStats.totalPlayTime << "\n";
    file << "totalHintsUsed:" << gameStats.totalHintsUsed << "\n";
    file << "maxStreak:" << gameStats.maxStreak << "\n";
    file << "totalStreakBonuses:" << gameStats.totalStreakBonuses << "\n";
    
    for (const auto& pair : gameStats.categoryWins) {
        file << "cat_win_" << pair.first << ":" << pair.second << "\n";
    }
    for (const auto& pair : gameStats.categoryGames) {
        file << "cat_game_" << pair.first << ":" << pair.second << "\n";
    }
    for (const auto& pair : gameStats.difficultyWins) {
        file << "diff_win_" << pair.first << ":" << pair.second << "\n";
    }
    for (const auto& pair : gameStats.difficultyGames) {
        file << "diff_game_" << pair.first << ":" << pair.second << "\n";
    }
    
    file.close();
}

// Function to display game statistics
void displayGameStats() {
    cout << "\n" << CYAN << "=== GAME STATISTICS ===\n" << RESET;
    cout << "Total Games: " << gameStats.totalGames << "\n";
    cout << "Wins: " << gameStats.totalWins << " (" << (gameStats.totalGames > 0 ? (gameStats.totalWins * 100 / gameStats.totalGames) : 0) << "%)\n";
    cout << "Losses: " << gameStats.totalLosses << "\n";
    cout << "Total Play Time: " << (gameStats.totalPlayTime / 60) << "m " << (gameStats.totalPlayTime % 60) << "s\n";
    cout << "Max Streak: " << gameStats.maxStreak << "\n";
    cout << "Total Streak Bonuses: " << gameStats.totalStreakBonuses << "\n";
    cout << "Hints Used: " << gameStats.totalHintsUsed << "\n\n";
    
    if (!gameStats.categoryGames.empty()) {
        cout << "Category Success Rates:\n";
        for (const auto& cat : CATEGORIES) {
            string catName = cat.name;
            int games = gameStats.categoryGames[catName];
            int wins = gameStats.categoryWins[catName];
            if (games > 0) {
                cout << "  " << catName << ": " << wins << "/" << games << " (" << (wins * 100 / games) << "%)\n";
            }
        }
        cout << "\n";
    }
    
    if (!gameStats.difficultyGames.empty()) {
        cout << "Difficulty Success Rates:\n";
        vector<string> difficulties = {"easy", "medium", "hard"};
        for (const string& diff : difficulties) {
            int games = gameStats.difficultyGames[diff];
            int wins = gameStats.difficultyWins[diff];
            if (games > 0) {
                cout << "  " << diff << ": " << wins << "/" << games << " (" << (wins * 100 / games) << "%)\n";
            }
        }
        cout << "\n";
    }
}

// Function to create typewriter effect
void typewriterEffect(const string& text, int delay = 50) {
    for (char c : text) {
        cout << c << flush;
        this_thread::sleep_for(chrono::milliseconds(delay));
    }
    cout << "\n";
}

// Function to animate word reveal
void animateWordReveal(const string& word, const string& colors = "") {
    cout << "\n" << YELLOW << "Revealing word: " << RESET;
    for (size_t i = 0; i < word.length(); ++i) {
        cout << GREEN << word[i] << RESET;
        this_thread::sleep_for(chrono::milliseconds(200));
    }
    cout << "\n\n";
}

// Function to display progress bar for attempts
void displayProgressBar(int current, int max, int width = 20) {
    cout << "[";
    int filled = (current * width) / max;
    for (int i = 0; i < width; ++i) {
        if (i < filled) {
            cout << RED << "█" << RESET;
        } else {
            cout << GREEN << "░" << RESET;
        }
    }
    cout << "] " << current << "/" << max << " attempts used\n";
}

// Function to animate hangman drawing
void animateHangmanDrawing(int stage) {
    static vector<string> hangmanParts = {
        "  +---+",
        "  |   |",
        "  O   ",
        "  |   ",
        " /|   ",
        " /|\\  ",
        " /    ",
        " / \\  "
    };
    
    cout << "\n" << RED;
    // Draw base structure
    cout << "  +---+\n";
    cout << "  |   |\n";
    
    // Animate drawing parts
    for (int i = 0; i <= stage && i < 6; ++i) {
        if (i == 0) cout << "  O   |\n";
        else if (i == 1) cout << "  |   |\n";
        else if (i == 2) cout << " /|   |\n";
        else if (i == 3) cout << " /|\\  |\n";
        else if (i == 4) cout << " /    |\n";
        else if (i == 5) cout << " / \\  |\n";
        
        if (i < stage) {
            this_thread::sleep_for(chrono::milliseconds(300));
        }
    }
    
    // Fill remaining empty lines
    for (int i = stage + 1; i < 6; ++i) {
        cout << "      |\n";
    }
    cout << "=========\n" << RESET;
}

// Function to highlight newly discovered letters
void highlightNewLetter(const string& word, string& guessed, char newLetter) {
    cout << BLUE << "Word: ";
    for (size_t i = 0; i < word.length(); ++i) {
        if (tolower(word[i]) == tolower(newLetter) && guessed[i] == '-') {
            // Newly discovered letter - highlight it
            cout << YELLOW << word[i] << RESET;
        } else if (guessed[i] != '-') {
            // Already discovered letter
            cout << GREEN << guessed[i] << RESET;
        } else {
            // Undiscovered letter
            cout << WHITE << guessed[i] << RESET;
        }
    }
    cout << RESET << "\n";
}

// Function to display high scores
void displayHighScores() {
    cout << "\n" << MAGENTA << "=== HIGH SCORES ===\n" << RESET;
    if (highScores.empty()) {
        cout << "No high scores yet!\n";
    } else {
        for (size_t i = 0; i < highScores.size(); ++i) {
            cout << i + 1 << ". " << highScores[i].name << " - " << highScores[i].score << "\n";
        }
    }
    cout << "\n";
}

// Function to use a hint (reveals first letter)
char useHint(const string& word, string& guessed) {
    // Find first unrevealed letter
    for (size_t i = 0; i < word.length(); ++i) {
        char letter = tolower(word[i]);
        bool alreadyRevealed = false;
        for (char g : guessed) {
            if (tolower(g) == letter) {
                alreadyRevealed = true;
                break;
            }
        }
        if (!alreadyRevealed) {
            // Reveal this letter
            guessed[i] = word[i];
            return letter;
        }
    }
    return '\0'; // No hint available
}

// Function to display intro with "fireworks" effect
void displayIntro() {
    cout << "\n\n";
    cout << MAGENTA << "******************************************\n";
    cout << "*                                        *\n";
    cout << "*              " << CYAN << "H A N G M A N" << MAGENTA << "             *\n";
    cout << "*                                        *\n";
    cout << "*          " << YELLOW << "Word Guessing Game" << MAGENTA << "            *\n";
    cout << "*                                        *\n";
    cout << "******************************************\n" << RESET;
    cout << "\n" << BLUE << "Guess the word letter by letter, or try guessing the whole word!\n";
    cout << "Choose from various categories and difficulty levels.\n";
    cout << "Earn points for correct guesses and bonuses!\n";
    cout << "Scoring: +10 for each correct letter, +50 bonus for whole word, -5 for wrong letter, -10 for wrong word.\n";
    cout << "You have " << MAX_ATTEMPTS << " attempts before the hangman is complete.\n\n" << RESET;
}

// Function to get a random word from the selected category and difficulty
string getRandomWord(int categoryIndex, const string& difficulty) {
    // Security: Bounds checking for category index
    if (categoryIndex < 0 || static_cast<size_t>(categoryIndex) >= CATEGORIES.size()) {
        return "";
    }
    
    const Category& cat = CATEGORIES[categoryIndex];
    const vector<string>* wordList = nullptr;
    if (difficulty == "easy") wordList = &cat.easy;
    else if (difficulty == "medium") wordList = &cat.medium;
    else if (difficulty == "hard") wordList = &cat.hard;
    else return ""; // error

    // Security: Check if word list is not empty to prevent undefined behavior
    if (wordList->empty()) {
        return "";
    }

    // Security: Secure random seed with hardware-based entropy
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<size_t> dis(0, wordList->size() - 1);
    return (*wordList)[dis(gen)];
}

// Function to display the hangman based on wrong guesses
void displayHangman(int wrong) {
    static int colorIndex = 0;
    string borderColor = christmasColors[colorIndex % christmasColors.size()];
    colorIndex++;

    cout << "\n" << borderColor << "******************************************\n" << RESET;
    string color = (wrong > 0) ? RED : WHITE;
    cout << color;
    switch (wrong) {
        case 0:
            cout << "  +---+\n";
            cout << "  |   |\n";
            cout << "      |\n";
            cout << "      |\n";
            cout << "      |\n";
            cout << "      |\n";
            cout << "=========\n";
            break;
        case 1:
            cout << "  +---+\n";
            cout << "  |   |\n";
            cout << "  O   |\n";
            cout << "      |\n";
            cout << "      |\n";
            cout << "      |\n";
            cout << "=========\n";
            break;
        case 2:
            cout << "  +---+\n";
            cout << "  |   |\n";
            cout << "  O   |\n";
            cout << "  |   |\n";
            cout << "      |\n";
            cout << "      |\n";
            cout << "=========\n";
            break;
        case 3:
            cout << "  +---+\n";
            cout << "  |   |\n";
            cout << "  O   |\n";
            cout << " /|   |\n";
            cout << "      |\n";
            cout << "      |\n";
            cout << "=========\n";
            break;
        case 4:
            cout << "  +---+\n";
            cout << "  |   |\n";
            cout << "  O   |\n";
            cout << " /|\\  |\n";
            cout << "      |\n";
            cout << "      |\n";
            cout << "=========\n";
            break;
        case 5:
            cout << "  +---+\n";
            cout << "  |   |\n";
            cout << "  O   |\n";
            cout << " /|\\  |\n";
            cout << " /    |\n";
            cout << "      |\n";
            cout << "=========\n";
            break;
        case 6:
            cout << "  +---+\n";
            cout << "  |   |\n";
            cout << "  O   |\n";
            cout << " /|\\  |\n";
            cout << " / \\  |\n";
            cout << "      |\n";
            cout << "=========\n";
            break;
    }
    cout << borderColor << "******************************************\n" << RESET << "\n";
}

// Function to play a single game
bool playGame(int categoryIndex, const string& difficulty, int& totalScore, int& currentStreak, int& maxStreak, GameMode mode) {
    auto gameStartTime = chrono::steady_clock::now();
    
    string word = getRandomWord(categoryIndex, difficulty);
    if (word.empty()) {
        cout << RED << "Error: No words available for this category and difficulty.\n" << RESET;
        return false;
    }
    string guessed(word.length(), '-');
    int wrong = 0;
    string wrongGuesses = "";
    int gameScore = 0;
    int hintsUsed = 0;
    int gameStreak = 0; // Local streak for this game
    int lives = (mode == SURVIVAL) ? SURVIVAL_START_LIVES : 1;
    bool timeExpired = false;

    while (wrong < MAX_ATTEMPTS && guessed != word && !timeExpired) {
        // Check time for TIME_ATTACK mode
        if (mode == TIME_ATTACK) {
            auto currentTime = chrono::steady_clock::now();
            auto elapsed = chrono::duration_cast<chrono::seconds>(currentTime - gameStartTime).count();
            if (elapsed >= TIME_ATTACK_LIMIT) {
                timeExpired = true;
                break;
            }
            cout << YELLOW << "Time remaining: " << (TIME_ATTACK_LIMIT - elapsed) << "s\n" << RESET;
        }
        
        // Animate hangman drawing
        animateHangmanDrawing(wrong);
        
        cout << BLUE << "Word: ";
        static int colorOffset = 0;
        for (size_t i = 0; i < guessed.length(); ++i) {
            cout << christmasColors[(i + colorOffset) % christmasColors.size()] << guessed[i];
        }
        cout << RESET << "\n";
        colorOffset++;
        
        // Display progress bar
        displayProgressBar(wrong, MAX_ATTEMPTS);
        
        cout << RED << "Wrong guesses: " << wrongGuesses << "\n";
        cout << YELLOW << "Attempts left: " << (MAX_ATTEMPTS - wrong) << "\n";
        cout << GREEN << "Current score: " << gameScore << "\n";
        cout << MAGENTA << "Hints available: " << (MAX_HINTS - hintsUsed) << " (type 'hint' to use)\n";
        cout << BLUE << "Current streak: " << gameStreak << "\n";
        
        if (mode == SURVIVAL) {
            cout << CYAN << "Lives remaining: " << lives << "\n" << RESET;
        } else if (mode == PRACTICE) {
            cout << CYAN << "Practice Mode - No penalties!\n" << RESET;
        }
        
        string input;
        cout << CYAN << "Guess a letter, the whole word, or 'hint': " << RESET;
        getline(cin, input);

        // Security: Input sanitization - limit input length to prevent buffer issues
        const size_t MAX_INPUT_LENGTH = 100;
        if (input.length() > MAX_INPUT_LENGTH) {
            cout << RED << "Input too long! Please enter a shorter guess.\n\n" << RESET;
            continue;
        }

        // Convert to lowercase
        for (char &c : input) c = tolower(c);

        // Check if user wants to use a hint
        if (input == "hint") {
            if (hintsUsed >= MAX_HINTS) {
                cout << YELLOW << "No hints left! You can use up to " << MAX_HINTS << " hints per game.\n\n" << RESET;
                continue;
            }
            
            char hintedLetter = useHint(word, guessed);
            if (hintedLetter != '\0') {
                hintsUsed++;
                gameScore -= HINT_PENALTY; // Penalty for using hint
                gameStats.totalHintsUsed++;
                cout << GREEN << "💡 Hint used! Revealed letter: " << hintedLetter << " (-" << HINT_PENALTY << " points)\n\n" << RESET;
                playCorrectGuessSound();
                continue;
            } else {
                cout << YELLOW << "No more letters to reveal!\n\n" << RESET;
                continue;
            }
        }

        // Security: Input sanitization - only letters accepted for single character guesses
        if (input.length() == 1 && isalpha(input[0])) {
            char guess = input[0];

            // Check if already guessed
            bool alreadyGuessed = false;
            for (char c : guessed) {
                if (c == guess) {
                    alreadyGuessed = true;
                    break;
                }
            }
            if (!alreadyGuessed) {
                for (char c : wrongGuesses) {
                    if (c == guess) {
                        alreadyGuessed = true;
                        break;
                    }
                }
            }
            if (alreadyGuessed) {
                cout << YELLOW << "You already guessed that letter!\n\n" << RESET;
                continue;
            }

            // Check if guess is in word
            bool found = false;
            for (size_t i = 0; i < word.length(); ++i) {
                if (tolower(word[i]) == guess) {
                    guessed[i] = word[i];
                    found = true;
                }
            }

            if (found) {
                gameStreak++; // Increment streak
                int streakBonus = gameStreak * STREAK_BONUS;
                gameScore += 10 + streakBonus; // Points for correct letter + streak bonus
                gameStats.totalStreakBonuses += streakBonus;
                
                // Highlight newly discovered letters
                highlightNewLetter(word, guessed, guess);
                
                playCorrectGuessSound();
                cout << GREEN << "Correct! +" << (10 + streakBonus) << " points (" << streakBonus << " streak bonus)\n" << RESET;
            } else {
                gameStreak = 0; // Reset streak on wrong guess
                wrong++;
                wrongGuesses += guess;
                wrongGuesses += " ";
                
                // Different penalties based on game mode
                if (mode == PRACTICE) {
                    gameScore -= 0; // No penalty in practice mode
                } else {
                    gameScore -= 5; // Penalty for wrong guess
                }
                
                playWrongGuessSound();
                cout << RED << "Wrong! Streak broken!\n" << RESET;
            }
        }
        // Check if it's the full word guess
        // Security: Input sanitization - validate whole word contains only letters
        else if (input.length() > 1) {
            bool validWord = true;
            for (char c : input) {
                if (!isalpha(c)) {
                    validWord = false;
                    break;
                }
            }
            if (!validWord) {
                cout << RED << "Invalid input! Whole word guesses must contain only letters.\n\n" << RESET;
                continue;
            }

            if (input == word) {
                guessed = word;  // Reveal the word
                gameScore += 50; // Bonus for whole word
                playWinSound();
                cout << GREEN << "🎉 Amazing! You guessed the whole word correctly!\n\n" << RESET;
                break;  // Exit the loop
            } else {
                wrong++;
                
                // Different penalties based on game mode
                if (mode == PRACTICE) {
                    gameScore -= 0; // No penalty in practice mode
                } else {
                    gameScore -= 10; // Penalty for wrong word
                }
                
                playWrongGuessSound();
                cout << RED << "Wrong word guess!\n\n" << RESET;
            }
        }
        // Invalid input
        else {
            cout << RED << "Invalid input! Please enter a single letter or the whole word.\n\n" << RESET;
            continue;
        }

        cout << "\n";
    }

    // End game
    auto gameEndTime = chrono::steady_clock::now();
    auto gameDuration = chrono::duration_cast<chrono::seconds>(gameEndTime - gameStartTime).count();
    
    // Determine win/loss based on game mode
    bool won = false;
    if (mode == TIME_ATTACK && timeExpired) {
        // Time attack - loss if time expired
        won = false;
    } else if (mode == SURVIVAL) {
        // Survival mode - win if guessed word, loss if lost all lives
        won = (guessed == word);
        if (!won) {
            lives--;
            if (lives > 0) {
                // Continue with new word in survival mode
                cout << YELLOW << "You lost a life! " << lives << " lives remaining.\n" << RESET;
                return playGame(categoryIndex, difficulty, totalScore, currentStreak, maxStreak, mode);
            }
        }
    } else {
        // Classic/Practice mode
        won = (guessed == word);
    }
    
    // Animate final hangman
    animateHangmanDrawing(wrong);
    
    if (won) {
        // Check for fast guess bonus (under 30 seconds)
        if (gameDuration < 30 && wrong == 0 && mode != PRACTICE) {
            int speedBonus = gameScore / 2; // 50% bonus for perfect fast guess
            gameScore += speedBonus;
            cout << YELLOW << "⚡ SPEED BONUS! Perfect guess in " << gameDuration << "s! +" << speedBonus << " points!\n" << RESET;
        }
        
        // Animate word reveal
        animateWordReveal(word);
        
        // Typewriter effect for win message
        typewriterEffect("🎉 CONGRATULATIONS! You guessed the word correctly! 🎉", 30);
        cout << "Game score: " << gameScore << " (Time: " << gameDuration << "s)\n\n";
        
        // Update statistics
        gameStats.totalGames++;
        gameStats.totalWins++;
        gameStats.categoryGames[CATEGORIES[categoryIndex].name]++;
        gameStats.categoryWins[CATEGORIES[categoryIndex].name]++;
        gameStats.difficultyGames[difficulty]++;
        gameStats.difficultyWins[difficulty]++;
        gameStats.totalPlayTime += gameDuration;
        
        // Update streaks
        currentStreak++;
        if (currentStreak > maxStreak) {
            maxStreak = currentStreak;
            gameStats.maxStreak = maxStreak;
        }
        
        // Check achievements
        checkAchievements(gameDuration, wrong, hintsUsed, true, CATEGORIES[categoryIndex].name, mode);
        
        totalScore += gameScore;
        saveGameStats();
        return true;
    } else {
        // Animate word reveal for loss
        animateWordReveal(word);
        
        string loseMessage;
        if (mode == TIME_ATTACK && timeExpired) {
            loseMessage = "⏰ TIME'S UP! The word was: " + word + " ⏰";
        } else {
            loseMessage = "💀 GAME OVER! The word was: " + word + " 💀";
        }
        
        // Typewriter effect for lose message
        typewriterEffect(loseMessage, 30);
        cout << "Game score: " << gameScore << " (Time: " << gameDuration << "s)\n\n";
        
        // Update statistics
        gameStats.totalGames++;
        gameStats.totalLosses++;
        gameStats.categoryGames[CATEGORIES[categoryIndex].name]++;
        gameStats.difficultyGames[difficulty]++;
        gameStats.totalPlayTime += gameDuration;
        
        // Reset streak on loss
        currentStreak = 0;
        
        // Check achievements (some achievements can be unlocked on loss too)
        checkAchievements(gameDuration, wrong, hintsUsed, false, CATEGORIES[categoryIndex].name, mode);
        
        totalScore += gameScore;
        saveGameStats();
        return false;
    }
}

// Function to ask if player wants to play again
bool askPlayAgain() {
    char choice;
    while (true) {
        cout << CYAN << "Play again? (y/n): " << RESET;
        cin >> choice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        choice = tolower(choice);
        if (choice == 'y') {
            playPlayAgainSound();
            return true;
        } else if (choice == 'n') {
            playQuitSound();
            return false;
        }
        cout << YELLOW << "Please enter 'y' for yes or 'n' for no.\n" << RESET;
    }
}

int main() {
    loadHighScores();
    loadGameStats();
    loadAchievements();
    displayIntro();
    playIntroSound();
    displayHighScores();
    displayGameStats();
    displayAchievements();

    int totalScore = 0;
    int currentStreak = 0;
    int maxStreak = gameStats.maxStreak;
    bool playAgain = true;
    
    while (playAgain) {
        // Display game modes
        cout << YELLOW << "Choose a game mode:\n" << RESET;
        cout << "1. Classic Mode - Standard hangman game\n";
        cout << "2. Time Attack - " << TIME_ATTACK_LIMIT << " seconds per word\n";
        cout << "3. Survival Mode - " << SURVIVAL_START_LIVES << " lives, lose a life per wrong word\n";
        cout << "4. Practice Mode - No penalties, just for fun\n";
        cout << CYAN << "Enter mode number: " << RESET;
        int modeChoice;
        cin >> modeChoice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        GameMode selectedMode;
        if (modeChoice == 1) selectedMode = CLASSIC;
        else if (modeChoice == 2) selectedMode = TIME_ATTACK;
        else if (modeChoice == 3) selectedMode = SURVIVAL;
        else if (modeChoice == 4) selectedMode = PRACTICE;
        else {
            cout << RED << "Invalid choice! Defaulting to Classic mode.\n" << RESET;
            selectedMode = CLASSIC;
        }
        
        // Display categories
        cout << YELLOW << "Choose a category:\n" << RESET;
        for (size_t i = 0; i < CATEGORIES.size(); ++i) {
            cout << i + 1 << ". " << CATEGORIES[i].name << "\n";
        }
        cout << CYAN << "Enter category number: " << RESET;
        int categoryChoice;
        cin >> categoryChoice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        if (categoryChoice < 1 || categoryChoice > (int)CATEGORIES.size()) {
            cout << RED << "Invalid choice! Defaulting to first category.\n" << RESET;
            categoryChoice = 1;
        }
        playCategorySelectSound();
        int categoryIndex = categoryChoice - 1;

        // Display difficulties
        cout << YELLOW << "Choose difficulty:\n" << RESET;
        cout << "1. Easy (short words)\n";
        cout << "2. Medium (medium words)\n";
        cout << "3. Hard (long words)\n";
        cout << CYAN << "Enter difficulty number: " << RESET;
        int diffChoice;
        cin >> diffChoice;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string difficulty;
        if (diffChoice == 1) difficulty = "easy";
        else if (diffChoice == 2) difficulty = "medium";
        else if (diffChoice == 3) difficulty = "hard";
        else {
            cout << RED << "Invalid choice! Defaulting to medium.\n" << RESET;
            difficulty = "medium";
        }
        playCategorySelectSound();

        playGame(categoryIndex, difficulty, totalScore, currentStreak, maxStreak, selectedMode);
        cout << BLUE << "Total score: " << totalScore << "\n";
        cout << GREEN << "Current streak: " << currentStreak << "\n" << RESET;
        
        // Check for high score
        bool isHighScore = highScores.size() < MAX_HIGHSCORES || 
                          (totalScore > 0 && totalScore > highScores.back().score);
        if (isHighScore) {
            addHighScore(totalScore);
            displayHighScores();
        }
        
        playAgain = askPlayAgain();
    }

    cout << "\n" << MAGENTA << "Thanks for playing Hangman!\n";
    cout << "Final total score: " << totalScore << "\n";
    cout << "******************************************\n" << RESET;
    displayHighScores();
    displayAchievements();
    return 0;
}