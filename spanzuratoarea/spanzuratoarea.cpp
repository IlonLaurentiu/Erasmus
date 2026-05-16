// Security Features:
// - Safe std::string usage
// - Input sanitization for guesses
// - Secure random generation
// - Thread-safe audio control

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
#include <atomic>
#include <mutex>
#include <sstream>
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
const vector<string> RETRO_COLORS = {GREEN, CYAN, YELLOW, MAGENTA, BLUE, WHITE};

const int MAX_ATTEMPTS = 6;
const int MAX_HINTS = 3;
const int TIME_ATTACK_LIMIT = 60;
const int SURVIVAL_START_LIVES = 3;
const int MAX_LEADERBOARD = 10;
const string LEADERBOARD_FILE = "leaderboard.txt";
const string STATS_FILE = "game_stats.txt";
const string ACHIEVEMENT_FILE = "achievements.txt";

enum GameMode {
    CLASSIC,
    TIME_ATTACK,
    SURVIVAL,
    PRACTICE,
    MENU,
    LOADING
};

struct Achievement {
    string name;
    string description;
    string icon;
    bool unlocked;
};

struct GameStats {
    int totalGames = 0;
    int totalWins = 0;
    int totalLosses = 0;
    long long totalPlayTime = 0;
    map<string, int> categoryWins;
    map<string, int> categoryGames;
    map<string, int> difficultyWins;
    map<string, int> difficultyGames;
    int totalHintsUsed = 0;
    int maxStreak = 0;
    int totalStreakBonuses = 0;
};

struct Category {
    string name;
    vector<string> easy;
    vector<string> medium;
    vector<string> hard;
};

struct LeaderboardEntry {
    string name;
    int score;
    string category;
    string difficulty;
};

vector<LeaderboardEntry> leaderboard;
vector<Achievement> achievements = {
    {"First Blood", "Win your first game.", "[*]", false},
    {"Speed Demon", "Finish any game under 30 seconds.", "[~]", false},
    {"Perfectionist", "Win without a wrong guess.", "[!]", false},
    {"Category Master", "Win a game in every category.", "[#]", false},
    {"Hint Master", "Use all hints in one game.", "[@]", false},
    {"Streak Master", "Reach a streak of 10 wins.", "[%]", false},
    {"Time Warrior", "Win 5 time attack rounds.", "[=]", false},
    {"Survivor", "Win 3 survival runs.", "[&]", false}
};

GameStats gameStats;

const vector<Category> CATEGORIES = {
    {
        "Animals",
        {"cat","dog","pig","cow","fox","hare","wolf","toad","seal","mole","goat","swan","crab","goose","pony","llama","puma","otter","yak","eagle","heron","ram","mare","elk","boar","hawk","panda","horse","deer","crow","toad","duck","squir","shrew","crow","deer","lion","mouse","beetle"},
        {"elephant","giraffe","kangaroo","penguin","rhinoceros","alligator","dolphin","squirrel","caterpillar","ostrich","crocodile","buffalo","porpoise","armadillo","flamingo","panther","jaguar","platypus","seahorse","hedgehog","walrus","falcon","woodpecker","anteater","badger","macaw","pelican","toucan","cheetah","lemming","heron","octopus","salamander","warthog","marmoset","eland","antelope","mole","badger","ermine","caribou"},
        {"hippopotamus","chameleon","orangutan","caterpillar","chimpanzee","rattlesnake","butterfly","porcupine","tarantula","armadillo","salamander","flamenco","tarantula","anteater","hippopotamus","ankylosaurus","archaeopteryx","barracuda","praying mantis","sea cucumber","komododragon","glossy ibis","sabretooth","axolotl","ichthyosaur","pangolin","manatee","scorpion","eagle ray","narwhal","shoebill","velociraptor","mantis shrimp","golden eagle","woodpecker","thunderbird","swordfish","flyingfish","cuttlefish","albatross"}
    },
    {
        "Countries",
        {"usa","uk","china","india","brazil","spain","italy","japan","canada","egypt","gabon","laos","mali","qatar","peru","nepal","iran","oman","fiji","chile","togo","cuba","samoa","tonga","sudan","ghana","kenya","czech","brunei","bahamas","bhutan","latvia","palau","serbia","haiti","benin","ireland","belgium","poland","norway"},
        {"australia","germany","france","portugal","ukraine","denmark","belgium","ecuador","venezuela","mongolia","cambodia","albania","belarus","slovakia","romania","iceland","netherlands","austria","bulgaria","croatia","hungary","malaysia","pakistan","singapore","mexico","argentina","finland","greece","poland","turkey","chile","nigeria","serbia","lebanon","algeria","romania","estonia","latvia","malta","nicaragua","uganda"},
        {"switzerland","netherlands","newzealand","kazakhstan","uzbekistan","tajikistan","afghanistan","bangladesh","mozambique","madagascar","philippines","colombia","slovenia","mauritius","eswatini","singapore","jamaica","trinidad","seychelles","azerbaijan","montenegro","macedonia","kyrgyzstan","burkinafaso","bosniaherzegovina","cotedivoire","srilanka","costarica","dominica","honduras","mauritania","myanmar","saotome","saintlucia","turkmenistan","vaticancity","svalbard","capeverde","newcaledonia","saintkitts","antiguabarbuda"}
    },
    {
        "Fruits",
        {"apple","pear","plum","fig","lime","kiwi","grape","melon","lemon","olive","date","guava","papaya","lychee","berry","coconut","apricot","orange","mango","banana","peach","nectarine","quince","currant","raisin","avocado","mandarin","tamarind","loquat","persimmon","ugli","persimmon","berry","cherry","passion","pear","apple","lemon","mango","apple"},
        {"banana","orange","grapes","cherry","peach","blueberry","raspberry","pineapple","cantaloupe","watermelon","strawberry","blackberry","cranberry","plantain","kumquat","passionfruit","tangerine","pomegranate","nectarine","clementine","boysenberry","elderberry","gooseberry","honeydew","starfruit","jackfruit","durian","kiwano","sapodilla","dragonfruit","mangosteen","longan","physalis","mulberry","bilberry","cherimoya","soursop","custardapple","breadfruit","pineberry","feijoa","ambarella"},
        {"pineapple","watermelon","strawberry","pomegranate","persimmon","passionfruit","blackcurrant","gooseberry","elderberry","boysenberry","jackfruit","cantaloupe","honeydew","clementine","tangerine","dragonfruit","bloodorange","starfruit","breadfruit","kiwano","longan","guava","mangosteen","cherimoya","soursop","rambutan","mirabelle","physalis","cloudberry","ackee","durian","pineberry","feijoa","jabuticaba","honeycrisp","galaapple","redcurrant","yellowpassion","saskatoon","ambarella","palmfruit"}
    },
    {
        "Objects",
        {"pen","book","lamp","chair","table","knife","spoon","fork","cup","mug","bowl","bag","key","rope","shoe","coat","ring","bell","hat","desk","door","glass","brick","watch","phone","plate","brush","comb","crate","drum","glove","nail","torch","file","scarf","purse","broom","tape","needle"},
        {"computer","telephone","umbrella","backpack","scissors","refrigerator","microphone","helicopter","binoculars","calculator","typewriter","printer","keyboard","scanner","projector","headphones","flashlight","cushion","satellite","thermometer","microscope","telescope","armchair","wardrobe","cabinet","motorbike","briefcase","pajamas","trampoline","sunshade","pendulum","hologram","sideboard","stepladder","periscope","airplane","generator","blueprint","cushion"},
        {"submarine","espressomachine","typewriter","maracas","stethoscope","vacuumcleaner","smartphone","metronome","soundboard","dishwasher","oscilloscope","thermostat","percolator","turntable","stereoscope","gyroscope","headquarters","candelabra","microscope","centrifuge","polaroid","philharmonic","saxophone","photocopier","lightningrod","workstation","polygraph","flightsimulator","seismograph","liquidnitrogen","photojournal","windtunnel","kaleidoscope","roomba","hovercraft","smartwatch","audiobook","camcorder","nebulizer"}
    },
    {
        "Space",
        {"moon","star","mars","venus","orbit","comet","meteor","solar","nova","apollo","probe","spaceship","rocket","gravity","crater","eclipse","saturn","jupiter","astro","lunar","nebula","quasar","planet","asteroid","galaxy","telescope","cosmic","aurora","pulsar","martian","binary","cosmos","capsule","lander","astral","meteor","nebula","rocket","satellite"},
        {"satellite","asteroid","nebula","meteorite","spacewalk","launchpad","cosmonaut","exoplanet","radiation","gravity","magnetar","quasar","pulsar","telescope","orbital","vacuum","trajectory","payload","mission","interstellar","heliosphere","spacecraft","moonscape","starfield","captain","habitable","cosmonaut","megastructure","libration","sunspot","equinox","perihelion","aphelion","warpdrive","spaceport","starchart","spaceport","jetstream","ionosphere"},
        {"interstellar","extraterrestrial","microgravity","cosmology","gravitational","astrobiology","spectroscopy","magnetosphere","astrophysics","extraplanetary","micrometeorite","transmission","thermonuclear","supernovae","quasistellar","photometry","parallax","singularity","circumstellar","cosmonautics","astrogeology","planetarium","heliostatic","astrometry","spaceflight","protoplanetary","magnetohydrodynamics","astrochemistry","cryovolcano","planetesimal","geostationary","microsatellite","lunarmodule","deepfield","nebulous","exoatmosphere","coronalmass","spectrograph","gravitational","illumination"}
    },
    {
        "Sport",
        {"ball","goal","race","team","kick","run","jump","tennis","golf","ski","box","gym","pool","ref","net","club","court","hike","ride","sled","surf","swim","duel","drill","pad","ring","bike","punt","drum","rope","slam","chase","score","punt","toss","swim","shoot","rink","arch"},
        {"soccer","football","baseball","cricket","lacrosse","cycling","boxing","rowing","skating","hockey","wrestling","volleyball","rugby","fencing","handball","triathlon","marathon","archery","motorsport","skiing","surfing","curling","badminton","sailing","gymnastics","climbing","polo","flagfootball","snowboard","skateboard","equestrian","waterpolo","archery","shooting","yachting","bobsled","squash","kayaking","kayak"},
        {"weightlifting","pentathlon","decathlon","speedskating","windsurfing","mountaineering","ultramarathon","synchronised","downhill","slalom","sprintrelay","rugbyunion","cleanandjerk","polevaulting","racewalking","cricketworldcup","mountainbiking","biathlon","rallycross","handballmatch","hurdlesprint","synchronous","crosscountry","downhillbike","pentathlon","ultrahelium","triathlon","sporterformance","sprintsquad","gridiron","kiteboarding","skeletonrace","equestrianism","mudrun","motorcycling","aerobatics","difficulty","tournament","championship","sportsmanship","showjumping"}
    },
    {
        "Literature",
        {"poem","novel","story","book","verse","plot","hero","epic","essay","fable","prose","fairy","drama","scene","narrate","title","poet","diary","letter","genre","chapter","order","quote","muse","scene","symbol","style","tragedy","comedy","myth","lore","canon","theme","rhythm","imagery","voice","memoir","preface","review"},
        {"fiction","nonfiction","novella","trilogy","biography","autobiography","mystery","fantasy","romance","satire","allegory","parody","gothic","historical","literary","narrative","epistolary","existential","metaphor","prologue","epilogue","sonnet","ballad","chronicle","verseplay","dramatist","chapter","poetry","annotation","translation","manuscript","critique","catharsis","dialogue","dialect","imagistic","novelist","voiceover","publication"},
        {"streamofconsciousness","postmodernism","bildungsroman","autofiction","magicalrealism","metafiction","intertextuality","psychologicalthriller","epistolarynovel","contemporaryfiction","speculativefiction","historicalfiction","philosophicalnovel","streamofconsciousness","epistolary","nonlinearplot","postcolonialism","transcendentalism","existentialism","autoethnography","literarycriticism","metanarrative","romanticism","naturalism","symbolism","modernism","surrealism","expressionism","postmodern","minimalism","metaphysical","realism","avantgarde","paranormal","multiverse","sciencefiction","mythopoeia"}
    },
    {
        "Fantasy",
        {"elf","mage","yarn","wand","myth","dwarf","castle","fury","neon","gale","rift","ogre","drake","hero","spell","relic","tribe","torch","runic","sword","shield","quest","realm","goblin","spirit","siren","rune","demon","crypt","crown","coven","fable","tunic","spear","gnome","raven","cloak","staff","grail"},
        {"dragon","wizard","throne","kingdom","mythical","sorcery","unicorn","spellbook","enchanted","mystical","amulet","orcish","paladin","druidic","shadowfax","griffin","centaur","golem","talisman","werewolf","ghostly","phoenix","vampire","enchanted","crystal","valkyrie","runestone","fairyland","darkwood","astral","etherial","moonstone","manticore","typhoon","seeress","runeberg","arcane","goblinking"},
        {"spellcasting","elementalism","chronomancer","necromancer","dragonrider","mythopoeia","storytelling","shapeshifter","philosopherstone","argonautica","shadowrealm","eldritchness","beastmastery","mermaidkin","cosmicmystic","transmutation","soulforge","lightbringer","aetherblade","planeshifter","sorceress","enchantment","spellweaving","starcaller","moonshadow","stormcaller","bloodmage","timeweaver","dreamwalker","faequeen","spiritguide","runebound","arcaneartistry","elderscroll","shadowlands","wyrmfire","soulsword"}
    },
    {
        "Technology",
        {"chip","code","byte","data","laser","modem","robot","drone","pixel","codec","array","cloud","email","logic","fiber","cable","input","output","cache","token","bitmap","proxy","query","script","debug","forum","driver","widget","router","socket","printer","scanner","mouse","flash","debug","touch","sensor","cable","micro"},
        {"processor","network","database","algorithm","firewall","compiler","interface","bandwidth","encryption","hardware","software","virtual","protocol","debugger","middleware","bluetooth","megabyte","gigabyte","terabyte","firmware","password","simulation","repository","framework","router","quantum","automation","machine","workflow","endpoint","analytics","desktop","server","browser","kernel","scanner","syntax","compiler","patch"},
        {"microprocessor","microcontroller","cryptography","telemetry","nanotechnology","semiconductor","supercomputer","virtualization","authentication","decentralized","multithreading","synchronization","biomechatronics","holographic","neuromorphic","cryptanalysis","biocomputing","cybersecurity","artificialintelligence","bioinformatics","microarchitecture","photolithography","programmable","robotics","transistor","quantization","ultrasonic","teleconference","thermodynamics","electromagnetism","microfabrication","biometric","telepresence","neuralnet","augmentedreality","blockchain","quantumlogic","digitaltwin"}
    },
    {
        "Cryptography",
        {"key","hash","seed","code","cipher","swap","mask","salt","byte","lock","gate","bit","prime","data","sbox","xor","rsa","aes","ssh","ssl","otp","pki","hash","mask","keystore","mod","pad","dsgn","edge","sign","seal","hash","nonce","pigeon","wheel","crib","tube"},
        {"ciphertext","plaintext","signature","encryption","decryption","blockchain","keyexchange","authentication","diffie","hellman","elliptic","curve","rsaalgorithm","desmode","sha256","sha512","publickey","privatekey","certified","securehash","oneway","algorithm","entropy","xorstream","steganography","cryptanalysis","bruteforce","padlock","hashing","passphrase","salted","noncevalue","phrasebook","fingerprint","keystream","macvalue","timestamp","keystore"},
        {"asymmetrickey","symmetrickey","ellipticcurve","quantumresistant","postquantum","polynomialhash","homomorphic","zeroknowledge","obfuscation","cryptanalysis","sidechannel","ratchetprotocol","hashfunction","schnorrsignature","sslcertificate","publickeyinfrastructure","fingerprinthash","onewayfunction","keyderivation","entropysource","securemultiparty","authentication","dataprivacy","digitalenvelope","messageauthentication","keyagreement","authenticatedencryption","nostrildamus","scrambler","cryptoengine"}
    },
    {
        "History",
        {"rome","egypt","greece","china","aztec","maya","inca","viking","spain","france","britain","india","persia","japan","cuba","russia","poland","latin","sword","crown","castle","fort","helm","tunic","scroll","tablet","coin","trade","guild","quest","epoch","mason","pearl","toga","arch","steam","steam"},
        {"renaissance","medieval","imperial","colonial","baroque","victorian","republican","byzantine","ottoman","revolution","crusades","explorer","navigator","expedition","artifact","archaeology","dynasty","monarchy","empire","plebeian","patrician","colossus","tribute","plague","feudalism","sacrament","armistice","cartography","hieroglyph","pharaoh","festival","coliseum","constable","dynasty","philosophy","claudius","caesar","treaty","upheaval"},
        {"industrialization","enlightenment","reformation","colonization","tribalism","imperialism","prohibition","mercantilism","historiography","chivalry","constitutionalism","absolutism","bureaucracy","totalitarianism","nationalism","sovereignty","manifesto","archipelago","autocracy","theocracy","hegemony","oligarchy","serfdom","tributary","republicanism","postcolonial","neoimperialism","decolonization","antebellum","reconstruction","bureaucratic","geopolitics","microhistory","historiography","philology","paleography","suffragette","antiquarian","archaeological","etiquette"}
    }
};

atomic<bool> musicActive(false);
atomic<int> musicModeValue(0);
atomic<int> musicCategoryIndex(-1);
atomic<int> musicTimeLeft(0);
atomic<int> musicDanger(0);
thread musicThread;
mutex musicMutex;
mt19937 rng((random_device())());

void sleepMs(int ms);
void pauseForEnter();
void animateWordReveal(const string& word);
void displayHighScores();
void displayAchievements();

void clearScreen() {
    cout << "\033[2J\033[H";
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode;
        if (GetConsoleMode(hOut, &dwMode)) {
            SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
#endif
}

void flickerTransition(int flashes = 3) {
    for (int i = 0; i < flashes; ++i) {
        clearScreen();
        cout << WHITE << string(80, '=') << RESET << "\n";
        sleepMs(80);
        clearScreen();
        sleepMs(120);
    }
}

void sleepMs(int ms) {
    this_thread::sleep_for(chrono::milliseconds(ms));
}

void playSound(int frequency, int duration) {
#ifdef _WIN32
    if (frequency > 0 && duration > 0) {
        Beep(frequency, duration);
    } else if (duration > 0) {
        sleepMs(duration);
    }
#else
    if (duration > 0) sleepMs(duration);
#endif
}

void typewriterPrint(const string& text, int delay = 22, bool sound = false) {
    for (char c : text) {
        cout << c << flush;
        if (sound && c != ' ' && c != '\n') {
            playSound(800, 20);
        }
        sleepMs(delay);
    }
    cout << "\n";
}

string spinnerFrame(int index) {
    static const vector<string> frames = {"[|]", "[/]", "[-]", "[\\]"};
    return frames[index % frames.size()];
}

void printMatrixBackground(int cycles = 40) {
    const string charset = "01abcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < cycles; ++i) {
        clearScreen();
        for (int row = 0; row < 16; ++row) {
            string line;
            for (int col = 0; col < 60; ++col) {
                char c = charset[rng() % charset.size()];
                if (rng() % 7 == 0) line += c;
                else line += ' ';
            }
            cout << GREEN << line << RESET << "\n";
        }
        sleepMs(60);
    }
}

void introCinematic() {
    flickerTransition(4);
    printMatrixBackground(32);
    clearScreen();
    vector<string> lines = {
        "========================================",
        "   RETRO TERMINAL OPERATING SYSTEM 7.3",
        "========================================",
        "INITIALIZING ARCADE HANGMAN EXPERIENCE",
        "SCANNING WORD DATABASE...",
        "LOADING NEON DISPLAY MODULES...",
        "SYNCHRONIZING CHIPTUNE SOUNDTRACK...",
        "CALIBRATING CRT FLICKER EFFECT..."
    };
    for (const string& line : lines) {
        typewriterPrint(line, 18, true);
        sleepMs(160);
    }
    cout << "\n";
    typewriterPrint("Press ENTER to enter the arena.", 25, true);
    string dummy;
    getline(cin, dummy);
    flickerTransition(2);
}

void pulseTitle(const string& title) {
    vector<string> pulseColors = {CYAN, WHITE, BLUE, MAGENTA};
    for (int i = 0; i < 4; ++i) {
        clearScreen();
        cout << pulseColors[i] << "========================================" << RESET << "\n";
        cout << pulseColors[i] << "   " << title << "   " << RESET << "\n";
        cout << pulseColors[i] << "========================================" << RESET << "\n";
        sleepMs(120);
    }
}

void scanRevealEffect(const string& message) {
    cout << CYAN << message << RESET << "\n";
    playSound(1000, 40);
    playSound(1200, 30);
    sleepMs(120);
}

void loadingScreen(const string& context) {
    vector<string> messages = {
        "Decrypting word database...",
        "Loading retro sound engine...",
        "Generating challenge...",
        "Calibrating difficulty...",
        "Preparing survival systems...",
        "Loading arcade assets...",
        "Synthesizing background tones...",
        "Warming up display grid..."
    };
    uniform_int_distribution<int> msgDist(0, static_cast<int>(messages.size()) - 1);
    string selected = messages[msgDist(rng)];
    int totalSteps = 24;
    for (int step = 0; step <= totalSteps; ++step) {
        clearScreen();
        cout << MAGENTA << "=== " << context << " ===" << RESET << "\n";
        cout << CYAN << selected << RESET << "\n\n";
        int percent = (step * 100) / totalSteps;
        cout << GREEN << "[";
        int filled = (percent * 30) / 100;
        for (int i = 0; i < 30; ++i) {
            cout << (i < filled ? "#" : "-");
        }
        cout << "] " << percent << "% " << spinnerFrame(step) << RESET << "\n";
        string timeline;
        if (percent < 30) timeline = "Boot sequence initializing...";
        else if (percent < 60) timeline = "Injecting retro graphics...";
        else if (percent < 90) timeline = "Synchronizing arcade beat...";
        else timeline = "Ready to launch...";
        cout << YELLOW << timeline << RESET << "\n";
        playSound(440, 30);
        sleepMs(100 + (step % 3) * 20);
    }
    cout << GREEN << "Loading complete." << RESET << "\n";
    sleepMs(450);
}

void loadLeaderboard() {
    leaderboard.clear();
    ifstream file(LEADERBOARD_FILE);
    if (!file.is_open()) return;
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string name, scoreStr, category, difficulty;
        if (!getline(ss, name, ',')) continue;
        if (!getline(ss, scoreStr, ',')) continue;
        if (!getline(ss, category, ',')) continue;
        if (!getline(ss, difficulty, ',')) continue;
        try {
            int score = stoi(scoreStr);
            leaderboard.push_back({name, score, category, difficulty});
        } catch (...) {
        }
    }
}

void saveLeaderboard() {
    ofstream file(LEADERBOARD_FILE);
    if (!file.is_open()) return;
    for (const auto& entry : leaderboard) {
        file << entry.name << "," << entry.score << "," << entry.category << "," << entry.difficulty << "\n";
    }
}

void addLeaderboardScore(int score, const string& category, const string& difficulty) {
    string name;
    cout << CYAN << "Enter your name for the leaderboard: " << RESET;
    getline(cin, name);
    size_t start = name.find_first_not_of(" \t");
    if (start != string::npos) {
        size_t end = name.find_last_not_of(" \t");
        name = name.substr(start, end - start + 1);
    }
    if (name.empty()) {
        name = "Player";
    }
    leaderboard.push_back({name, score, category, difficulty});
    sort(leaderboard.begin(), leaderboard.end(), [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
        return a.score > b.score;
    });
    if (leaderboard.size() > MAX_LEADERBOARD) {
        leaderboard.resize(MAX_LEADERBOARD);
    }
    saveLeaderboard();
}

void displayHighScores() {
    cout << MAGENTA << "=== GLOBAL LEADERBOARD ===" << RESET << "\n";
    if (leaderboard.empty()) {
        cout << "No scores yet. Become the first legend!\n";
    } else {
        for (size_t i = 0; i < leaderboard.size(); ++i) {
            cout << (i + 1) << ". " << leaderboard[i].name << " - " << leaderboard[i].score
                 << " [" << leaderboard[i].category << ", " << leaderboard[i].difficulty << "]\n";
        }
    }
    cout << "\n";
}

void displayCategoryLeaderboard(const string& category) {
    cout << MAGENTA << "=== LEADERBOARD: " << category << " ===" << RESET << "\n";
    vector<LeaderboardEntry> entries;
    for (const auto& entry : leaderboard) {
        if (entry.category == category) {
            entries.push_back(entry);
        }
    }
    if (entries.empty()) {
        cout << "No scores yet for this category.\n";
    } else {
        for (size_t i = 0; i < entries.size() && i < 10; ++i) {
            cout << (i + 1) << ". " << entries[i].name << " - " << entries[i].score
                 << " [" << entries[i].difficulty << "]\n";
        }
    }
    cout << "\n";
}

void loadAchievements() {
    ifstream file(ACHIEVEMENT_FILE);
    if (!file.is_open()) return;
    string line;
    while (getline(file, line)) {
        size_t pos = line.find(':');
        if (pos == string::npos) continue;
        string name = line.substr(0, pos);
        string state = line.substr(pos + 1);
        for (auto& achievement : achievements) {
            if (achievement.name == name) {
                achievement.unlocked = (state == "1");
                break;
            }
        }
    }
}

void saveAchievements() {
    ofstream file(ACHIEVEMENT_FILE);
    if (!file.is_open()) return;
    for (const auto& achievement : achievements) {
        file << achievement.name << ":" << (achievement.unlocked ? "1" : "0") << "\n";
    }
}

void checkAchievements(int gameDuration, int wrongGuesses, int hintsUsed, bool won, const string& category, GameMode mode) {
    if (!achievements[0].unlocked && won) {
        achievements[0].unlocked = true;
        cout << GREEN << "=== ACHIEVEMENT UNLOCKED ===" << RESET << "\n";
        cout << GREEN << achievements[0].name << ": " << achievements[0].description << RESET << "\n\n";
    }
    if (!achievements[1].unlocked && won && gameDuration < 30) {
        achievements[1].unlocked = true;
        cout << GREEN << "=== ACHIEVEMENT UNLOCKED ===" << RESET << "\n";
        cout << GREEN << achievements[1].name << ": " << achievements[1].description << RESET << "\n\n";
    }
    if (!achievements[2].unlocked && won && wrongGuesses == 0) {
        achievements[2].unlocked = true;
        cout << GREEN << "=== ACHIEVEMENT UNLOCKED ===" << RESET << "\n";
        cout << GREEN << achievements[2].name << ": " << achievements[2].description << RESET << "\n\n";
    }
    if (!achievements[4].unlocked && won && hintsUsed == MAX_HINTS) {
        achievements[4].unlocked = true;
        cout << GREEN << "=== ACHIEVEMENT UNLOCKED ===" << RESET << "\n";
        cout << GREEN << achievements[4].name << ": " << achievements[4].description << RESET << "\n\n";
    }
    if (mode == TIME_ATTACK && won) {
        static int timeWins = 0;
        timeWins++;
        if (timeWins >= 5 && !achievements[6].unlocked) {
            achievements[6].unlocked = true;
            cout << GREEN << "=== ACHIEVEMENT UNLOCKED ===" << RESET << "\n";
            cout << GREEN << achievements[6].name << ": " << achievements[6].description << RESET << "\n\n";
        }
    }
    if (mode == SURVIVAL && won) {
        static int survivalWins = 0;
        survivalWins++;
        if (survivalWins >= 3 && !achievements[7].unlocked) {
            achievements[7].unlocked = true;
            cout << GREEN << "=== ACHIEVEMENT UNLOCKED ===" << RESET << "\n";
            cout << GREEN << achievements[7].name << ": " << achievements[7].description << RESET << "\n\n";
        }
    }
    saveAchievements();
}

void displayAchievements() {
    cout << YELLOW << "=== ACHIEVEMENTS & TROPHIES ===" << RESET << "\n";
    for (const auto& achievement : achievements) {
        cout << (achievement.unlocked ? GREEN : RED) << achievement.icon << " " << achievement.name;
        if (!achievement.unlocked) cout << " (locked)";
        cout << RESET << "\n  " << achievement.description << "\n";
    }
    cout << "\n";
}

void loadGameStats() {
    ifstream file(STATS_FILE);
    if (!file.is_open()) return;
    string line;
    while (getline(file, line)) {
        size_t pos = line.find(':');
        if (pos == string::npos) continue;
        string key = line.substr(0, pos);
        string value = line.substr(pos + 1);
        try {
            if (key == "totalGames") gameStats.totalGames = stoi(value);
            else if (key == "totalWins") gameStats.totalWins = stoi(value);
            else if (key == "totalLosses") gameStats.totalLosses = stoi(value);
            else if (key == "totalPlayTime") gameStats.totalPlayTime = stoll(value);
            else if (key == "totalHintsUsed") gameStats.totalHintsUsed = stoi(value);
            else if (key == "maxStreak") gameStats.maxStreak = stoi(value);
            else if (key == "totalStreakBonuses") gameStats.totalStreakBonuses = stoi(value);
            else if (key.rfind("cat_win_", 0) == 0) gameStats.categoryWins[key.substr(8)] = stoi(value);
            else if (key.rfind("cat_game_", 0) == 0) gameStats.categoryGames[key.substr(9)] = stoi(value);
            else if (key.rfind("diff_win_", 0) == 0) gameStats.difficultyWins[key.substr(9)] = stoi(value);
            else if (key.rfind("diff_game_", 0) == 0) gameStats.difficultyGames[key.substr(10)] = stoi(value);
        } catch (...) {
        }
    }
}

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
    for (const auto& pair : gameStats.categoryWins) file << "cat_win_" << pair.first << ":" << pair.second << "\n";
    for (const auto& pair : gameStats.categoryGames) file << "cat_game_" << pair.first << ":" << pair.second << "\n";
    for (const auto& pair : gameStats.difficultyWins) file << "diff_win_" << pair.first << ":" << pair.second << "\n";
    for (const auto& pair : gameStats.difficultyGames) file << "diff_game_" << pair.first << ":" << pair.second << "\n";
}

void displayGameStats() {
    cout << CYAN << "=== GAME STATISTICS ===" << RESET << "\n";
    cout << "Total games: " << gameStats.totalGames << "\n";
    cout << "Wins: " << gameStats.totalWins << "\n";
    cout << "Losses: " << gameStats.totalLosses << "\n";
    cout << "Total play time: " << (gameStats.totalPlayTime / 60) << "m " << (gameStats.totalPlayTime % 60) << "s\n";
    cout << "Max streak: " << gameStats.maxStreak << "\n";
    cout << "Hints used: " << gameStats.totalHintsUsed << "\n\n";
    if (!gameStats.categoryGames.empty()) {
        cout << "Category success rates:\n";
        for (const auto& cat : CATEGORIES) {
            int games = gameStats.categoryGames[cat.name];
            int wins = gameStats.categoryWins[cat.name];
            if (games > 0) cout << "  " << cat.name << ": " << wins << "/" << games << "\n";
        }
        cout << "\n";
    }
    if (!gameStats.difficultyGames.empty()) {
        cout << "Difficulty success rates:\n";
        for (const string& diff : {"easy", "medium", "hard"}) {
            int games = gameStats.difficultyGames[diff];
            int wins = gameStats.difficultyWins[diff];
            if (games > 0) cout << "  " << diff << ": " << wins << "/" << games << "\n";
        }
        cout << "\n";
    }
}

void updateMusicState(int wrong, int timeLeft, int categoryIndex) {
    musicDanger.store(wrong);
    musicTimeLeft.store(timeLeft);
    musicCategoryIndex.store(categoryIndex);
}

void playBackgroundMusic(GameMode mode, int categoryIndex = -1) {
    lock_guard<mutex> guard(musicMutex);
    if (musicActive.load()) return;
    musicActive.store(true);
    musicModeValue.store(static_cast<int>(mode));
    musicCategoryIndex.store(categoryIndex);
    musicThread = thread([mode, categoryIndex]() {
        auto selectPattern = [&](GameMode mode, int category)->vector<pair<int,int>> {
            vector<pair<int,int>> base;
            int baseTone = 440 + (category >= 0 ? (category * 12 % 60) : 0);
            switch (mode) {
                case MENU:
                    base = {{baseTone,120},{0,40},{baseTone+62,120},{0,40},{baseTone-48,120},{0,80}};
                    break;
                case LOADING:
                    base = {{330,100},{0,30},{392,100},{0,30},{330,100},{0,60}};
                    break;
                case CLASSIC:
                    base = {{baseTone,90},{baseTone+30,100},{baseTone+12,90},{0,50}};
                    break;
                case TIME_ATTACK:
                    base = {{baseTone+80,70},{baseTone+140,70},{baseTone+200,70},{baseTone+260,70},{0,30}};
                    break;
                case SURVIVAL:
                    base = {{baseTone-120,110},{baseTone-20,110},{baseTone-80,100},{0,50}};
                    break;
                case PRACTICE:
                    base = {{baseTone+10,140},{0,50},{baseTone+22,140},{0,50}};
                    break;
            }
            return base;
        };
        size_t idx = 0;
        while (musicActive.load()) {
            int currentMode = musicModeValue.load();
            int currCat = musicCategoryIndex.load();
            int danger = musicDanger.load();
            int left = musicTimeLeft.load();
            GameMode gm = static_cast<GameMode>(currentMode);
            auto pattern = selectPattern(gm, currCat);
            if (gm == TIME_ATTACK && left <= 15) {
                pattern = {{880,60},{784,60},{932,60},{0,30}};
            }
            if (danger >= MAX_ATTEMPTS - 1) {
                pattern = {{220,180},{0,80},{220,180},{0,120}};
            }
            if (pattern.empty()) {
                sleepMs(100);
                continue;
            }
            const auto& note = pattern[idx % pattern.size()];
            playSound(note.first, note.second);
            idx++;
            sleepMs(50);
        }
    });
}

void stopMusic() {
    {
        lock_guard<mutex> guard(musicMutex);
        if (!musicActive.load()) return;
        musicActive.store(false);
    }
    if (musicThread.joinable()) musicThread.join();
}

void playEventSound(const string& event) {
    if (event == "correct") {
        playSound(880, 70);
        playSound(1047, 50);
    } else if (event == "wrong") {
        playSound(220, 120);
    } else if (event == "win") {
        playSound(523, 100);
        playSound(659, 90);
        playSound(784, 90);
        playSound(1047, 150);
    } else if (event == "lose") {
        playSound(330, 150);
        playSound(294, 130);
        playSound(262, 170);
    } else if (event == "select") {
        playSound(600, 60);
        playSound(800, 50);
    } else if (event == "scan") {
        playSound(900, 30);
    } else if (event == "type") {
        playSound(780, 18);
    }
}

string getRandomWord(int categoryIndex, const string& difficulty) {
    if (categoryIndex < 0 || categoryIndex >= static_cast<int>(CATEGORIES.size())) return "";
    const Category& cat = CATEGORIES[categoryIndex];
    const vector<string>* list = nullptr;
    if (difficulty == "easy") list = &cat.easy;
    else if (difficulty == "medium") list = &cat.medium;
    else if (difficulty == "hard") list = &cat.hard;
    if (!list || list->empty()) return "";
    uniform_int_distribution<size_t> dist(0, list->size() - 1);
    return (*list)[dist(rng)];
}

void renderHangman(int wrong) {
    vector<string> frame = {
        "  +---+",
        "  |   |",
        "      |",
        "      |",
        "      |",
        "      |",
        "=========",
    };
    if (wrong >= 1) frame[2] = "  O   |";
    if (wrong >= 2) frame[3] = "  |   |";
    if (wrong >= 3) frame[3] = " /|   |";
    if (wrong >= 4) frame[3] = " /|\\  |";
    if (wrong >= 5) frame[4] = " /    |";
    if (wrong >= 6) frame[4] = " / \\  |";
    cout << MAGENTA << "=== HANGMAN STATUS ===" << RESET << "\n";
    for (const string& line : frame) cout << RED << line << RESET << "\n";
    cout << "\n";
}

void renderHUD(const string& guessed, const string& wrongGuesses, int wrong, int hintsUsed, int gameScore, int gameStreak, GameMode mode, int timeLeft, int lives) {
    static int flash = 0;
    flash = (flash + 1) % RETRO_COLORS.size();
    cout << "Word length: " << guessed.size() << " letters\n";
    cout << "Word: ";
    for (char c : guessed) {
        cout << RETRO_COLORS[flash] << c << RESET;
    }
    cout << "\n";
    int filled = (wrong * 20) / MAX_ATTEMPTS;
    cout << GREEN << "Attempts:[";
    for (int i = 0; i < 20; ++i) cout << (i < filled ? "#" : "-");
    cout << "] " << (MAX_ATTEMPTS - wrong) << " left" << RESET << "\n";
    cout << YELLOW << "Wrong letters: " << (wrongGuesses.empty() ? "none" : wrongGuesses) << RESET << "\n";
    cout << BLUE << "Hints: " << (MAX_HINTS - hintsUsed) << " / " << MAX_HINTS << RESET << "\n";
    cout << CYAN << "Score: " << gameScore << "   Streak: " << gameStreak << RESET << "\n";
    if (mode == TIME_ATTACK) cout << MAGENTA << "Time remaining: " << timeLeft << "s" << RESET << "\n";
    if (mode == SURVIVAL) cout << MAGENTA << "Lives: " << lives << RESET << "\n";
    cout << "\n";
}

char useHint(const string& word, string& guessed) {
    for (size_t i = 0; i < word.size(); ++i) {
        if (guessed[i] == '-') {
            guessed[i] = word[i];
            return word[i];
        }
    }
    return '\0';
}

int askMenuChoice(int min, int max, const string& prompt) {
    while (true) {
        cout << CYAN << prompt << RESET;
        string input;
        getline(cin, input);
        for (char& c : input) if (isalpha(static_cast<unsigned char>(c))) playEventSound("type");
        if (input.empty()) continue;
        bool valid = true;
        for (char c : input) if (!isdigit(static_cast<unsigned char>(c))) { valid = false; break; }
        if (!valid) {
            cout << RED << "Please enter a numeric choice." << RESET << "\n";
            continue;
        }
        int value = stoi(input);
        if (value < min || value > max) {
            cout << RED << "Choice must be between " << min << " and " << max << "." << RESET << "\n";
            continue;
        }
        return value;
    }
}

void displayLeaderboardSummary() {
    displayHighScores();
    cout << CYAN << "Challenge: Beat my score and own the leaderboard!" << RESET << "\n\n";
}

void displayIntroScreen() {
    introCinematic();
    pulseTitle("HANGMAN RETRO ARENA");
    displayLeaderboardSummary();
    pauseForEnter();
}

void pauseForEnter() {
    cout << CYAN << "Press ENTER to continue..." << RESET;
    string dummy;
    getline(cin, dummy);
}

bool qualifiesForLeaderboard(int score) {
    if (leaderboard.size() < MAX_LEADERBOARD) return true;
    return score > leaderboard.back().score;
}

void addLeaderboard(int score, const string& category, const string& difficulty) {
    if (!qualifiesForLeaderboard(score)) return;
    addLeaderboardScore(score, category, difficulty);
}

void displayPrompt(const string& prompt) {
    typewriterPrint(prompt, 16, true);
}

void displayAchievementBanner(const string& text) {
    cout << GREEN << "=== " << text << " ===" << RESET << "\n";
}

void checkAndDisplayMedal(int score, int streak, const string& category, const string& difficulty) {
    string medal = "Bronze";
    if (score >= 200) medal = "Gold";
    else if (score >= 120) medal = "Silver";
    else if (score >= 60) medal = "Bronze";
    cout << MAGENTA << "Achievement medal: " << medal << " for " << category << " / " << difficulty << RESET << "\n";
    if (streak >= 5) cout << YELLOW << "Streak bonus medal unlocked!" << RESET << "\n";
}

bool playGame(int categoryIndex, const string& difficulty, int& totalScore, int& currentStreak, int& maxStreak, GameMode mode) {
    string word = getRandomWord(categoryIndex, difficulty);
    if (word.empty()) {
        cout << RED << "Error: No word available." << RESET << "\n";
        return false;
    }
    string guessed(word.size(), '-');
    int wrong = 0;
    string wrongGuesses;
    int hintsUsed = 0;
    int gameScore = 0;
    int gameStreak = 0;
    int lives = (mode == SURVIVAL) ? SURVIVAL_START_LIVES : 1;
    bool timeExpired = false;
    auto start = chrono::steady_clock::now();

    while (true) {
        int timeLeft = 0;
        if (mode == TIME_ATTACK) {
            auto now = chrono::steady_clock::now();
            int elapsed = static_cast<int>(chrono::duration_cast<chrono::seconds>(now - start).count());
            timeLeft = TIME_ATTACK_LIMIT - elapsed;
            if (timeLeft <= 0) { timeExpired = true; break; }
        }
        updateMusicState(wrong, timeLeft, categoryIndex);
        clearScreen();
        renderHangman(wrong);
        renderHUD(guessed, wrongGuesses, wrong, hintsUsed, gameScore, gameStreak, mode, timeLeft, lives);
        if (guessed == word) break;
        if (wrong >= MAX_ATTEMPTS) {
            if (mode == SURVIVAL && lives > 1) {
                lives -= 1;
                cout << YELLOW << "Life lost! Remaining lives: " << lives << RESET << "\n";
                playEventSound("wrong");
                sleepMs(900);
                wrong = 0;
                wrongGuesses.clear();
                hintsUsed = 0;
                word = getRandomWord(categoryIndex, difficulty);
                guessed.assign(word.size(), '-');
                continue;
            }
            break;
        }

        displayPrompt("Guess a letter, full word, or type 'hint': ");
        string input;
        getline(cin, input);
        for (char& c : input) {
            c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
            playEventSound("type");
        }
        if (input.length() > 100) {
            cout << RED << "Input too long." << RESET << "\n";
            sleepMs(300);
            continue;
        }
        if (input == "hint") {
            if (hintsUsed >= MAX_HINTS) {
                cout << YELLOW << "No hints left." << RESET << "\n";
                sleepMs(400);
                continue;
            }
            char hint = useHint(word, guessed);
            if (hint != '\0') {
                hintsUsed++;
                gameScore -= 10;
                gameStats.totalHintsUsed++;
                scanRevealEffect("Scanning letter...");
                cout << GREEN << "Hint: " << hint << RESET << "\n";
                playEventSound("correct");
                sleepMs(500);
            } else {
                cout << YELLOW << "No letters to reveal." << RESET << "\n";
                sleepMs(500);
            }
            continue;
        }
        if (input.empty()) continue;
        bool single = (input.size() == 1 && isalpha(static_cast<unsigned char>(input[0])));
        bool whole = input.size() > 1;
        if (single) {
            char guess = input[0];
            if (guessed.find(guess) != string::npos || wrongGuesses.find(guess) != string::npos) {
                cout << YELLOW << "Letter already guessed." << RESET << "\n";
                sleepMs(300);
                continue;
            }
            bool found = false;
            for (size_t i = 0; i < word.size(); ++i) {
                if (tolower(word[i]) == guess) {
                    guessed[i] = word[i];
                    found = true;
                }
            }
            if (found) {
                gameStreak++;
                int bonus = gameStreak * 5;
                gameScore += 10 + bonus;
                gameStats.totalStreakBonuses += bonus;
                scanRevealEffect("Letter discovered...");
                playEventSound("correct");
                cout << GREEN << "Correct! +" << (10 + bonus) << " points." << RESET << "\n";
                sleepMs(400);
            } else {
                gameStreak = 0;
                wrong++;
                wrongGuesses += guess;
                wrongGuesses += ' ';
                if (mode != PRACTICE) gameScore -= 5;
                playEventSound("wrong");
                cout << RED << "Wrong letter." << RESET << "\n";
                sleepMs(400);
            }
            continue;
        }
        if (whole) {
            bool valid = true;
            for (char c : input) if (!isalpha(static_cast<unsigned char>(c))) valid = false;
            if (!valid) {
                cout << RED << "Invalid word guess." << RESET << "\n";
                sleepMs(300);
                continue;
            }
            if (input == word) {
                guessed = word;
                gameScore += 50;
                playEventSound("win");
                break;
            }
            wrong++;
            if (mode == SURVIVAL) lives -= 1;
            if (mode != PRACTICE) gameScore -= 10;
            playEventSound("wrong");
            cout << RED << "Wrong full word." << RESET << "\n";
            sleepMs(400);
            continue;
        }
        cout << RED << "Invalid entry." << RESET << "\n";
        sleepMs(300);
    }

    auto end = chrono::steady_clock::now();
    int duration = static_cast<int>(chrono::duration_cast<chrono::seconds>(end - start).count());
    bool won = (guessed == word && !timeExpired);
    clearScreen();
    renderHangman(wrong);
    if (won) {
        cout << GREEN << "=== CONGRATULATIONS ===" << RESET << "\n";
        animateWordReveal(word);
        cout << GREEN << "You solved this " << difficulty << " challenge in " << duration << " seconds." << RESET << "\n";
        playEventSound("win");
        gameStats.totalGames++;
        gameStats.totalWins++;
        gameStats.categoryGames[CATEGORIES[categoryIndex].name]++;
        gameStats.categoryWins[CATEGORIES[categoryIndex].name]++;
        gameStats.difficultyGames[difficulty]++;
        gameStats.difficultyWins[difficulty]++;
        gameStats.totalPlayTime += duration;
        if (currentStreak + 1 > maxStreak) maxStreak = ++currentStreak;
        else currentStreak++;
        checkAchievements(duration, wrong, hintsUsed, true, CATEGORIES[categoryIndex].name, mode);
        checkAndDisplayMedal(gameScore, currentStreak, CATEGORIES[categoryIndex].name, difficulty);
        addLeaderboard(gameScore, CATEGORIES[categoryIndex].name, difficulty);
        saveGameStats();
        return true;
    }
    animateWordReveal(word);
    cout << RED << "=== GAME OVER ===" << RESET << "\n";
    if (timeExpired) cout << RED << "Time expired." << RESET << "\n";
    else cout << RED << "Hangman complete." << RESET << "\n";
    playEventSound("lose");
    gameStats.totalGames++;
    gameStats.totalLosses++;
    gameStats.categoryGames[CATEGORIES[categoryIndex].name]++;
    gameStats.difficultyGames[difficulty]++;
    gameStats.totalPlayTime += duration;
    currentStreak = 0;
    checkAchievements(duration, wrong, hintsUsed, false, CATEGORIES[categoryIndex].name, mode);
    addLeaderboard(gameScore, CATEGORIES[categoryIndex].name, difficulty);
    saveGameStats();
    return false;
}

bool askPlayAgain() {
    while (true) {
        cout << CYAN << "Play again? (y/n): " << RESET;
        string input;
        getline(cin, input);
        if (!input.empty()) {
            char choice = static_cast<char>(tolower(static_cast<unsigned char>(input[0])));
            if (choice == 'y') {
                playEventSound("select");
                return true;
            }
            if (choice == 'n') {
                playEventSound("lose");
                return false;
            }
        }
        cout << YELLOW << "Please enter y or n." << RESET << "\n";
    }
}

void animateWordReveal(const string& word) {
    cout << YELLOW << "Revealing word: " << RESET;
    for (char c : word) {
        cout << GREEN << c << RESET << flush;
        playSound(900, 30);
        sleepMs(140);
    }
    cout << "\n\n";
}

int main() {
    loadLeaderboard();
    loadGameStats();
    loadAchievements();
    displayIntroScreen();
    playBackgroundMusic(MENU);
    flickerTransition(2);
    while (true) {
        clearScreen();
        pulseTitle("HANGMAN RETRO ARENA");
        displayHighScores();
        displayAchievements();
        cout << YELLOW << "Choose a game mode:" << RESET << "\n";
        cout << "1. Classic Mode\n";
        cout << "2. Time Attack\n";
        cout << "3. Survival Mode\n";
        cout << "4. Practice Mode\n";
        int modeChoice = askMenuChoice(1, 4, "Mode number: ");
        GameMode selectedMode = CLASSIC;
        switch (modeChoice) {
            case 2: selectedMode = TIME_ATTACK; break;
            case 3: selectedMode = SURVIVAL; break;
            case 4: selectedMode = PRACTICE; break;
            default: selectedMode = CLASSIC; break;
        }
        playEventSound("select");
        clearScreen();
        cout << YELLOW << "Choose a category:" << RESET << "\n";
        for (size_t i = 0; i < CATEGORIES.size(); ++i) {
            cout << (i + 1) << ". " << CATEGORIES[i].name << "\n";
        }
        int categoryChoice = askMenuChoice(1, static_cast<int>(CATEGORIES.size()), "Category number: ");
        playEventSound("select");
        clearScreen();
        cout << YELLOW << "Choose difficulty:" << RESET << "\n";
        cout << "1. Easy\n";
        cout << "2. Medium\n";
        cout << "3. Hard\n";
        int difficultyChoice = askMenuChoice(1, 3, "Difficulty number: ");
        string difficulty = (difficultyChoice == 1 ? "easy" : difficultyChoice == 2 ? "medium" : "hard");
        playEventSound("select");
        stopMusic();
        playBackgroundMusic(LOADING, categoryChoice - 1);
        loadingScreen("SYSTEM INITIALIZATION");
        stopMusic();
        playBackgroundMusic(selectedMode, categoryChoice - 1);
        flickerTransition(2);
        int totalScore = 0;
        static int currentStreak = 0;
        static int maxStreak = gameStats.maxStreak;
        bool won = playGame(categoryChoice - 1, difficulty, totalScore, currentStreak, maxStreak, selectedMode);
        stopMusic();
        cout << BLUE << "TOTAL SCORE: " << totalScore << RESET << "\n";
        cout << GREEN << "STREAK: " << currentStreak << RESET << "\n";
        if (qualifiesForLeaderboard(totalScore)) {
            addLeaderboard(totalScore, CATEGORIES[categoryChoice - 1].name, difficulty);
            displayHighScores();
        }
        if (!askPlayAgain()) break;
    }
    playBackgroundMusic(MENU);
    clearScreen();
    cout << MAGENTA << "=== THANK YOU FOR PLAYING ===" << RESET << "\n";
    displayHighScores();
    displayAchievements();
    stopMusic();
    return 0;
}
