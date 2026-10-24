// Generated from Creating Constructs.html by tools/sync_catalog.py.
constexpr int PieceCount=128,CategoryCount=9,PieceEncodingStride=128;
const char* names[]={"Block","Slab","Pillar","Glass","Slope","Moss","Stairs","Column","Window","Panel","Arch","Railing","Beam","Light","Tree","Ivy","Cable","Planter","Brick","Timber","Steel","Fins","Cobogó","Doorway","Pool","Ladder","Bollard","Sconce","Dome","Silo","Vault","Fence","Slit","Ribbon","Floor","Pergola","Fine stairs","Pendant","Obelisk","Bench","Water","Birch","Pine","Pipe","Valve","Gauge","Outlet","Boardform","Cantilever","Batter","Waffle","Portal","Clerestory","Plinth","Chase","Spout","Pilotis","Megastep","Lightwell","Bridge","Core","Aggregate","Corbel","Stone","Boulder","Log","Stump","Mushroom","Strut","Girder","Niche","Hood","Roundel","Stripe","Half","Lilypad","Grass","Cascade","Flight","Amber","Cool","Ribbed","Green wall","Lattice","Glassblock","Embrasure","Shrub","Hang","Slopegrass","Captop","Soffitveg","Deckgrass","Dropveg","Downslope","Quadpier","Angle","Service entry","Air vent","Cable tray","Ceiling light","Exit sign","Neon cyan","Neon rose","Terminal","Neon violet","Wall screen","Server cabinet","Trash can","Grate floor","Barrier","Duct","Fan vent","Street light","Light strip","Grid floor","Bollard light","Carpet","Yellow wall","Ceiling tile","Tube light","Pool tile","Office door","Partition","Water cooler","Desk","Office chair","File cabinet","Painted pillar"};
const char* cats[]={"Structure","Shapes","Openings","Detail","Nature","Material","Pipes","Cyber","Liminal"};
constexpr int CategorySlots=24;
const int catTypes[CategoryCount][CategorySlots]={
{0,1,34,48,49,50,56,57,58,59,60,69,74,78,93,94,2,-1,-1,-1,-1,-1,-1,-1},
{4,93,36,28,29,30,38,12,7,35,37,6,9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{3,51,52,8,23,22,83,21,32,33,84,85,31,10,68,70,71,72,-1,-1,-1,-1,-1,-1},
{53,54,55,62,11,25,13,27,26,39,16,73,79,80,95,96,97,98,99,100,101,102,103,-1},
{5,14,15,86,76,77,87,82,88,89,90,91,92,17,24,40,41,42,63,64,65,66,67,75},
{47,61,18,19,20,0,81,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{43,44,45,46,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{104,105,106,107,108,109,110,111,112,113,114,115,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
{116,117,118,119,120,121,122,123,124,125,126,127,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
};
static_assert(PieceCount<=PieceEncodingStride,"GPU piece encoding must fit all types");
