#pragma once
#include <string>
#include <array>
#include <cstddef>
#include <unordered_map>
#include <chrono>
#include <stack>
#include <sstream>
#define FIXED_FLOAT(x) std::fixed << std::setprecision(2) << (x)

//****************************** Output Formatting ******************************//

const std::string RESET = "\033[0m";
const std::string BOLD = "\033[1m";
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN = "\033[36m";
const std::string WHITE = "\033[37m";

//******************* (Currently Deprecated) CFR Parameters *******************//

static const size_t STRATEGY_ARRAY_SIZE = 500007;

static const float DCFR_ALPHA = 1.0f;
static const float DCFR_BETA = 1.0f;
static const float DCFR_GAMMA = 1.0f;

static const int FLOP_BUCKETS = 100;
static const int TURN_BUCKETS = 100;
static const int RIVR_BUCKETS = 100;
static const std::array<float, FLOP_BUCKETS> FLOP_BUCKETS_ARR = {0.212586f, 0.236864f, 0.251572f, 0.265294f, 0.276199f, 0.287481f, 0.295916f, 0.303057f, 0.308807f, 0.317067f, 0.324064f, 0.329401f, 0.335334f, 0.339599f, 0.343856f, 0.348642f, 0.35223f, 0.35735f, 0.361062f, 0.365889f, 0.370129f, 0.375378f, 0.378068f, 0.382191f, 0.385437f, 0.389338f, 0.393295f, 0.398017f, 0.402075f, 0.406405f, 0.409611f, 0.412195f, 0.414821f, 0.417412f, 0.421604f, 0.425537f, 0.428851f, 0.43166f, 0.435117f, 0.438867f, 0.441477f, 0.443548f, 0.447451f, 0.450983f, 0.454905f, 0.457864f, 0.460301f, 0.462896f, 0.465963f, 0.468883f, 0.471795f, 0.474216f, 0.477504f, 0.481153f, 0.484019f, 0.488389f, 0.492385f, 0.49641f, 0.500369f, 0.503798f, 0.508693f, 0.51303f, 0.518054f, 0.525526f, 0.532714f, 0.54161f, 0.547067f, 0.554007f, 0.562147f, 0.568448f, 0.574701f, 0.585489f, 0.593985f, 0.601386f, 0.606892f, 0.612711f, 0.618749f, 0.625165f, 0.633094f, 0.642679f, 0.647095f, 0.652945f, 0.662958f, 0.670666f, 0.675811f, 0.681646f, 0.689697f, 0.697487f, 0.708952f, 0.717659f, 0.73385f, 0.745911f, 0.76757f, 0.79292f, 0.820826f, 0.841296f, 0.861318f, 0.876335f, 0.89612f, 1.0f};
static const std::array<float, TURN_BUCKETS> TURN_BUCKETS_ARR = {0.15792f, 0.181003f, 0.194257f, 0.211569f, 0.222222f, 0.230939f, 0.238199f, 0.245676f, 0.252257f, 0.258352f, 0.262909f, 0.268748f, 0.2743f, 0.279793f, 0.284725f, 0.289351f, 0.295454f, 0.300089f, 0.305063f, 0.30929f, 0.312945f, 0.319309f, 0.324026f, 0.328403f, 0.332377f, 0.336509f, 0.340345f, 0.344653f, 0.348964f, 0.353541f, 0.35736f, 0.362574f, 0.367175f, 0.371891f, 0.37583f, 0.380299f, 0.384952f, 0.389689f, 0.395024f, 0.400592f, 0.406025f, 0.410353f, 0.417159f, 0.423963f, 0.429535f, 0.436939f, 0.441231f, 0.447428f, 0.453016f, 0.458613f, 0.465429f, 0.474227f, 0.483467f, 0.490076f, 0.497443f, 0.50498f, 0.513411f, 0.522663f, 0.532198f, 0.541255f, 0.54766f, 0.552462f, 0.558618f, 0.568591f, 0.577642f, 0.584744f, 0.591177f, 0.600037f, 0.606827f, 0.615556f, 0.623263f, 0.629845f, 0.638078f, 0.64654f, 0.652692f, 0.661541f, 0.67396f, 0.68085f, 0.689577f, 0.697372f, 0.706259f, 0.714556f, 0.725507f, 0.741262f, 0.757957f, 0.768685f, 0.780041f, 0.793564f, 0.800697f, 0.816833f, 0.828207f, 0.838228f, 0.852835f, 0.862706f, 0.869278f, 0.886263f, 0.892009f, 0.946426f, 0.964123f, 1.0f};
static const std::array<float, RIVR_BUCKETS> RIVR_BUCKETS_ARR = {0.0107905f, 0.0201254f, 0.0299482f, 0.0395268f, 0.0485912f, 0.0545718f, 0.0677025f, 0.0765087f, 0.0888627f, 0.0969953f, 0.106989f, 0.118112f, 0.130495f, 0.137547f, 0.150257f, 0.162407f, 0.173653f, 0.182596f, 0.190947f, 0.200835f, 0.210978f, 0.219748f, 0.225634f, 0.233613f, 0.243688f, 0.252463f, 0.2599f, 0.275133f, 0.294297f, 0.311714f, 0.326977f, 0.33757f, 0.347697f, 0.356147f, 0.363269f, 0.372073f, 0.382862f, 0.393029f, 0.4009f, 0.40907f, 0.415611f, 0.421848f, 0.431871f, 0.43986f, 0.44975f, 0.46054f, 0.471763f, 0.48081f, 0.4942f, 0.506483f, 0.513647f, 0.525564f, 0.53884f, 0.552616f, 0.56375f, 0.572733f, 0.580203f, 0.587843f, 0.603704f, 0.61399f, 0.621144f, 0.633477f, 0.64604f, 0.656881f, 0.665088f, 0.673133f, 0.681325f, 0.691016f, 0.699762f, 0.70702f, 0.716738f, 0.721527f, 0.731067f, 0.745019f, 0.756321f, 0.76648f, 0.772362f, 0.772366f, 0.79141f, 0.798767f, 0.798779f, 0.810884f, 0.816545f, 0.81938f, 0.825674f, 0.832786f, 0.838951f, 0.852901f, 0.856927f, 0.862471f, 0.868417f, 0.883682f, 0.889364f, 0.901519f, 0.945114f, 0.951334f, 0.960516f, 0.964913f, 0.974751f, 1.0f};

//****************************** Game Constants ******************************//

static const float SMALL_BLIND = 1.0f;
static const float BIG_BLIND = 2.0f;

static const int NUM_ROUNDS = 3;
static const int NUM_SUITS = 4;
static const int NUM_RANKS = 9;
static const int NUM_CARDS = 36;
static const int NUM_POCKET_PAIRS = 81;
static const int MAX_ACTIONS = 7;

static const std::array<std::string,  4> SUIT_NAMES = {"♠", "♥", "♦", "♣"};
static const std::array<std::string, 13> CARD_NAMES = {"6", "7", "8", "9", "T", "J", "Q", "K", "A"};

static const std::array<int, 6> STRAIGHT_MASKS = {0b111110000, 0b011111000, 0b001111100, 0b000111110, 0b000011111, 0b100001111};
static const std::array<int, 9> SINGLE_MASKS = {0b100000000, 0b010000000, 0b001000000, 0b000100000, 0b000010000, 0b000001000, 0b000000100, 0b000000010, 0b000000001};
static const std::array<int, 9> OCTAL_MASKS = {0b111, 0b111000, 0b111000000, 0b111000000000, 0b111000000000000, 0b111000000000000000};
static const std::unordered_map<std::string, int> STRING_TO_ACTION = {{"fold", 0}, {"check", 0}, {"call", 1}, {"bet", 1}, {"raise", 2}};
static const std::array<std::string, 7> BET_ACTION_NAMES = {"Check", "Bet", "Bet", "Bet", "Bet", "All in", "Call"};
static const std::array<std::string, 7> RAISE_ACTION_NAMES = {"Fold", "Raise", "Raise", "Raise", "Raise", "All in", "Call"};

extern std::array<float, 4> BET_SIZES;
extern std::array<float, 4> RAISE_SIZES;
extern float STACK_SIZE;

template<typename T>
std::string stack_to_string(std::stack<T> s) {
    std::stringstream ss;
    std::vector<T> temp;
    
    while (!s.empty()) {
        temp.push_back(s.top());
        s.pop();
    }
    
    for (int i=0; i<temp.size(); i++) {
        ss << temp[i];
        if (i != temp.size()-1) ss << ", ";
    }

    return ss.str();
};