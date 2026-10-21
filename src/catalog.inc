// Generated from Creating Constructs.html by tools/sync_catalog.py.
constexpr int PieceCount=68,CategoryCount=7,PieceEncodingStride=128;
const char* names[]={"Block","Slab","Pillar","Glass","Slope","Moss","Stairs","Column","Window","Panel","Arch","Railing","Beam","Light","Tree","Ivy","Cable","Planter","Brick","Timber","Steel","Fins","Cobogó","Doorway","Pool","Ladder","Bollard","Sconce","Dome","Silo","Vault","Fence","Slit","Ribbon","Floor","Pergola","Fine stairs","Pendant","Obelisk","Bench","Water","Birch","Pine","Pipe","Valve","Gauge","Outlet","Boardform","Cantilever","Batter","Waffle","Portal","Clerestory","Plinth","Chase","Spout","Pilotis","Megastep","Lightwell","Bridge","Core","Aggregate","Corbel","Stone","Boulder","Log","Stump","Mushroom"};
const char* cats[]={"Structure","Shapes","Openings","Detail","Nature","Material","Pipes"};
constexpr int CategorySlots=13;
const int catTypes[CategoryCount][CategorySlots]={
{0,1,34,48,49,50,56,57,58,59,60,2,-1},
{4,36,28,29,30,38,12,7,35,37,6,9,-1},
{3,51,52,8,23,22,21,32,33,31,10,-1,-1},
{53,54,55,62,11,25,13,27,26,39,16,-1,-1},
{5,14,15,17,24,40,41,42,63,64,65,66,67},
{47,61,18,19,20,0,-1,-1,-1,-1,-1,-1,-1},
{43,44,45,46,-1,-1,-1,-1,-1,-1,-1,-1,-1},
};
static_assert(PieceCount<=PieceEncodingStride,"GPU piece encoding must fit all types");
