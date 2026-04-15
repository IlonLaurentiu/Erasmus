# HANGMAN GAME - Diploma Project

## Description
A complete, secure implementation of the classic Hangman word-guessing game in C++. This project demonstrates modern C++ practices, input validation, random number generation, and modular code design suitable for university-level coursework.

## Features
- **Secure Randomization**: Uses `<random>` library with `std::mt19937` and `std::random_device` for true randomness
- **Input Validation**: Rejects non-letter inputs and handles edge cases
- **Modular Design**: Separated functions for clarity and maintainability
- **User-Friendly Interface**: Clear displays, attempt counters, and restart functionality
- **Error Handling**: Robust input buffer management
- **Difficulty Levels**: Easy (short words), Medium (medium words), Hard (long words)
- **Word Categories**: Animals, Countries, Fruits, Objects
- **Scoring System**: Points for correct guesses, penalties for wrong ones
- **Expanded Database**: 50+ words across multiple categories and difficulties
- **Sound Effects**: Audio feedback for correct/wrong guesses, win/lose, and game events (Windows only)
- **High Scores**: Persistent leaderboard with top 10 scores saved to file
- **Colorful Interface**: ANSI color codes for enhanced visual experience
- **Christmas Theme**: Colorful hangman borders and word displays
- **Hint System**: Limited hints that reveal letters (with penalty)
- **Streak Bonus**: Extra points for consecutive correct guesses
- **Speed Bonus**: Bonus for fast perfect guesses
- **Game Statistics**: Detailed analytics on performance by category and difficulty
- **Word Reveal Animation**: Gradual animated reveal of the word at game end
- **Typewriter Effects**: Animated text display for win/lose messages
- **Progress Bar**: Visual progress bar showing remaining attempts
- **Highlight Effects**: Newly discovered letters are highlighted
- **Animated Hangman**: Step-by-step drawing animation of hangman parts
- **Game Modes**: Classic, Time Attack, Survival, and Practice modes
- **Achievements System**: Unlockable achievements with progress tracking

## Technical Details
- **Language**: C++11 or higher
- **Libraries Used**:
  - `<iostream>`: Console I/O
  - `<string>`: String manipulation
  - `<vector>`: Word storage
  - `<random>`: Secure random number generation
  - `<cctype>`: Character classification and conversion
  - `<limits>`: Input buffer management
  - `<fstream>`: File I/O for high scores and statistics
  - `<algorithm>`: Sorting high scores
  - `<chrono>`: Time tracking for game duration and speed bonuses
  - `<map>`: Statistics storage by category and difficulty
  - `<thread>`: Animation delays and timing
  - `<windows.h>`: Sound effects (Windows only)
- **Compilation**: `g++ -Wall -Wextra -std=c++11 spanzuratoarea.cpp -o output/spanzuratoarea.exe`
- **Execution**: `./output/spanzuratoarea.exe`
- **Platform**: Windows (sound effects require Windows API)
- **Data Files**: `highscores.txt`, `game_stats.txt`, `achievements.txt`

## Game Rules
1. Player selects a category and difficulty level
2. Computer selects a random word from the chosen category and difficulty
3. Player guesses letters one by one or the whole word
4. Correct letter guesses reveal letters in the word (+10 points)
5. Wrong letter guesses draw parts of the hangman (-5 points)
6. Wrong whole word guesses cost attempts (-10 points)
7. Guessing the whole word correctly gives bonus (+50 points)
8. Game ends when word is guessed or hangman is complete (6 attempts)
9. Player can choose to play again, with cumulative scoring

## Game Modes
- **Classic Mode**: Standard hangman gameplay with scoring
- **Time Attack**: 60-second time limit per word - lose if time expires
- **Survival Mode**: 3 lives - lose a life per wrong word, continue until all lives lost
- **Practice Mode**: No penalties or scoring - perfect for learning

## Achievements System
- **First Blood**: Win your first game
- **Speed Demon**: Guess a word in under 30 seconds
- **Perfectionist**: Win a game with no wrong guesses
- **Category Master**: Win all words in a category
- **Hint Master**: Win a game using all hints
- **Streak Master**: Achieve a streak of 10 wins
- **Time Warrior**: Win 5 time attack games
- **Survivor**: Win 3 survival games
- **Persistent**: Achievements saved to `achievements.txt`

## Visual Effects & Animations
- **Word Reveal Animation**: Gradual letter-by-letter reveal at game end
- **Typewriter Effects**: Animated text display for messages (30ms delay)
- **Progress Bar**: Visual bar showing attempts used vs remaining
- **Letter Highlighting**: Newly discovered letters flash in yellow
- **Animated Hangman**: Step-by-step drawing of hangman parts with delays
- **Color Transitions**: Smooth color changes between game states

## Hint System
- **3 hints** available per game
- **Reveal first unrevealed letter** in the word
- **-20 point penalty** for each hint used
- Type 'hint' during gameplay to use a hint

## Streak & Speed Bonuses
- **Streak Bonus**: Extra points for consecutive correct letter guesses
- **Combo Breaker**: Streak resets on wrong guesses
- **Speed Bonus**: 50% score bonus for perfect games completed in under 30 seconds
- **Streak Tracking**: Current and maximum streaks displayed

## High Scores
- Top 10 scores are saved to `highscores.txt`
- Scores persist between game sessions
- New high scores prompt for player name
- Leaderboard displayed at game start and end

## Game Statistics
- **Persistent Statistics**: Saved to `game_stats.txt`
- **Performance Tracking**: Win/loss ratios by category and difficulty
- **Time Analytics**: Total play time and average game duration
- **Streak Records**: Maximum consecutive wins
- **Hint Usage**: Total hints used across all games
- **Success Rates**: Detailed breakdown by category and difficulty level

## Sound Effects (Windows Only)
- **Correct Guess**: High beep (800Hz, 200ms)
- **Wrong Guess**: Low beep (300Hz, 400ms)
- **Win**: Victory fanfare melody
- **Lose**: Descending "ua ua ua" tones
- **Game Events**: Various sound cues for menu navigation and game flow

## Categories and Words
- **Animals**: cat, dog, elephant, hippopotamus, etc.
- **Countries**: usa, australia, switzerland, etc.
- **Fruits**: apple, pineapple, strawberry, etc.
- **Objects**: pen, computer, refrigerator, etc.

## Code Structure
- `displayIntro()`: Shows game title, instructions, and scoring rules
- `getRandomWord(int categoryIndex, const string& difficulty)`: Selects random word from specific category and difficulty
- `displayHangman(int wrong)`: Legacy hangman display (replaced by animated version)
- `animateHangmanDrawing(int stage)`: Animated step-by-step hangman drawing
- `playGame(int categoryIndex, const string& difficulty, int& totalScore, int& currentStreak, int& maxStreak, GameMode mode)`: Main game logic with all features
- `useHint(const string& word, string& guessed)`: Reveals first unrevealed letter as hint
- `typewriterEffect(const string& text, int delay)`: Animated text display
- `animateWordReveal(const string& word)`: Gradual word reveal animation
- `displayProgressBar(int current, int max)`: Visual progress bar for attempts
- `highlightNewLetter(const string& word, string& guessed, char newLetter)`: Highlights newly discovered letters
- `loadGameStats()` / `saveGameStats()`: Persistent statistics management
- `displayGameStats()`: Shows detailed performance analytics
- `loadAchievements()` / `saveAchievements()`: Achievement persistence
- `displayAchievements()`: Shows achievement progress
- `checkAchievements(...)`: Evaluates and unlocks achievements
- `askPlayAgain()`: Handles restart functionality
- `main()`: Program entry point with game mode selection

## Security Features
- **No Buffer Overflows**: Uses `std::string` for all string operations with dynamic sizing and bounds checking
- **Input Sanitization**: Only accepts alphabetic characters for guesses, validates input length (max 100 chars)
- **Secure Random Seed**: Hardware-based entropy with `std::random_device` and `std::mt19937`
- **No Undefined Behavior**: All loops use `size_t` indices, bounds checking on arrays/vectors, safe memory access
- **Input Validation**: Rejects non-letter inputs, handles edge cases, and prevents invalid category/difficulty selections

## Future Enhancements
- **Graphical Version**: Create GUI version with SFML or Qt libraries
- **Multiplayer Mode**: Add local multiplayer with turn-based gameplay
- **Cross-Platform Sound**: Implement sound effects for Linux/macOS using different APIs
- **Custom Word Lists**: Allow players to add their own words and categories
- **Statistics Tracking**: Detailed game statistics (win rate, average score, favorite categories)
- **Themes**: Multiple visual themes beyond Christmas colors
- **Difficulty Scaling**: Dynamic difficulty adjustment based on player performance
- **Time Challenges**: Add time limits for faster-paced gameplay
- **Network Multiplayer**: Online multiplayer with matchmaking

## Author
AI Assistant - 2026

## License
Educational use only.