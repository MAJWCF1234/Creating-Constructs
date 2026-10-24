// Geometry matches IDs 28-40 in the current HTML defs array.
case 28:spherePart({0,-.5f,0},.5f,0,true,24,12);break;
case 29:cylinder(.5f,1,0,0,24);break;
case 30:roundVault();break;
case 31:
 box(1,.04f,.04f,0,.45f,0,3);box(1,.04f,.04f,0,-.45f,0,3);
 box(.04f,1,.04f,-.48f,0,0,3);box(.04f,1,.04f,.48f,0,0,3);
 for(float x:{-.3f,-.1f,.1f,.3f})box(.015f,.9f,.015f,x,0,0,3);
 for(float y:{-.3f,-.1f,.1f,.3f})box(.9f,.015f,.015f,0,y,0,3);break;
case 32:box(.4f,1,.3f,-.3f);box(.4f,1,.3f,.3f);box(.2f,.2f,.3f,0,.4f);box(.2f,.2f,.3f,0,-.4f);break;
case 33:box(1,.35f,.3f,0,-.325f);box(1,.35f,.3f,0,.325f);box(1,.3f,.04f,0,0,0,2);break;
case 34:box(1,.15f,1,0,.425f);break;
case 35:
 for(float x:{-.4f,-.13f,.13f,.4f})box(.1f,.12f,1,x,.44f,0,5);
 for(float z:{-.35f,0.f,.35f})box(1,.06f,.08f,0,.5f,z,5);break;
case 36:for(int i=0;i<8;i++)box(1,.125f*(i+1),.125f,0,-.5f+.0625f*(i+1),.4375f-.125f*i);stairHandrails(stairSideBlocked(-1)?-999.f:-.42f,stairSideBlocked(1)?-999.f:.42f,8);break;
case 37:cylinder(.01f,.5f,.25f,3);spherePart({0,-.08f,0},.12f,4,false,10,8);break;
case 38:taperedObelisk();break;
case 39:box(.9f,.08f,.36f,0,-.18f,0,5);box(.08f,.32f,.3f,-.38f,-.34f,0,5);box(.08f,.32f,.3f,.38f,-.34f,0,5);break;
case 40:waterSurface();break;
case 41:natureBirch();break;
case 42:naturePine();break;
// Catalog thumbs for face-mounted pipe tools (actual routing is the pipe overlay).
case 43:cylinder(.05f,.9f,0,8,12);cylinder(.08f,.04f,.4f,3,10);cylinder(.08f,.04f,-.4f,3,10);break;
case 44:cylinder(.05f,.9f,0,8,12);box(.14f,.12f,.12f,0,0,0,3);cylinder(.07f,.03f,.22f,7,10);break;
case 45:cylinder(.05f,.9f,0,8,12);cylinder(.08f,.05f,.22f,3,12);cylinder(.06f,.02f,.28f,8,12);break;
case 46:cylinder(.05f,1,0,8,12);cylinder(.09f,.08f,-.42f,3,12);cylinder(.11f,.03f,-.38f,8,12);break;
// Brutalism first wave
case 47:box(1,1,1,0,0,0,18);break; // board-form concrete material
case 48:box(1,.28f,1.45f,0,.36f,.225f);break; // cantilever soffit
case 49: // battered retaining mass
 box(1,.28f,.95f,0,-.36f,.025f);box(1,.28f,.8f,0,-.08f,.1f);box(1,.28f,.65f,0,.2f,.175f);box(1,.16f,.5f,0,.42f,.25f);break;
case 50: // waffle / coffered underside
 box(1,.14f,1,0,.43f);box(1,.08f,.08f,0,.34f);box(.08f,.08f,1,0,.34f);
 for(float x:{-.33f,0.f,.33f})for(float z:{-.33f,0.f,.33f})box(.28f,.1f,.28f,x,.36f,z);break;
case 51: // deep portal — 2×2, jambs recessed/thick toward interior (-Z)
 box(.32f,2,.75f,-.34f,.5f,-.25f);box(.32f,2,.75f,1.34f,.5f,-.25f);box(1.36f,.2f,.75f,.5f,1.4f,-.25f);break;
case 52: // clerestory band
 box(1,.55f,.35f,0,-.225f);box(.08f,.3f,.35f,-.46f,.35f);box(.08f,.3f,.35f,.46f,.35f);box(1,.08f,.35f,0,.49f);box(.84f,.22f,.04f,0,.35f,0,2);break;
case 53:box(1,.22f,.9f,0,-.39f);break; // plinth
case 54: // service chase
 box(.18f,1,.45f,-.41f,0,-.275f);box(.18f,1,.45f,.41f,0,-.275f);box(1,1,.18f,0,0,-.41f);break;
case 55: // rain scupper / spout
 box(.4f,.14f,.55f,0,.38f,-.1f);box(.16f,.1f,.4f,0,.3f,-.42f);box(.22f,.08f,.18f,0,.26f,-.55f);break;
// Brutalism wave 2
case 56: // pilotis — raised capital, walk under
 for(float x:{-.36f,.36f})for(float z:{-.36f,.36f})box(.11f,1.72f,.11f,x,.36f,z);box(.85f,.28f,.85f,0,1.36f);break;
case 57: // megastep — 2-wide stair mass
 box(2,.5f,1,.5f,-.25f);break;
case 58: // lightwell — 2×2 plan rim, 2 tall
 box(.18f,2,2,-.41f,.5f,.5f);box(.18f,2,2,1.41f,.5f,.5f);box(2,2,.18f,.5f,.5f,-.41f);box(2,2,.18f,.5f,.5f,1.41f);break;
case 59: // inhabited bridge — 2-wide × 2-tall walk tunnel (fits 1.6m player)
 box(2,.12f,1,.5f,-.44f);box(2,1.9f,.12f,.5f,.5f,-.44f);box(2,1.9f,.12f,.5f,.5f,.44f);box(2,.12f,1,.5f,1.44f);break;
case 60: // tower core — 1×2 walk-in shaft, -Z open, fits standing player
 box(.14f,2,1,-.43f,.5f);box(.14f,2,1,.43f,.5f);box(1,2,.14f,0,.5f,.43f);box(1,.14f,1,0,1.43f);break;
case 61:box(1,1,1,0,0,0,19);break; // exposed aggregate
case 62: // corbel bracket
 box(.55f,.12f,.5f,0,.38f,-.05f);box(.4f,.12f,.4f,0,.24f);box(.26f,.12f,.3f,0,.1f,.05f);break;
// Nature wave — stone/boulder via rock surface nets; timber/fungi from PSX pack
case 63:case 64:box(1,1,1,0,0,0,38);break;
case 65:natureLog();break;
case 66:natureStump();break;
case 67:natureMushroom();break;
// Brutalism wave 3 — concept corridor braces
case 68:{ // Strut — 2-tall diagonal in XY (portal face). r=0 = brace across a Z-facing opening.
 // Right-low → left-high; pads seat on +X jamb and under soffit.
 V a{.58f,-.58f,0.f},b{-.58f,1.58f,0.f},d=norm(b-a),u{0,0,1},v=norm(cross(d,u));
 float hw=.32f,ht=.26f;auto at=[&](float t,float su,float sv){return a+(b-a)*t+u*(su*hw)+v*(sv*ht);};
 quad(at(0,-1,-1),at(0,1,-1),at(0,1,1),at(0,-1,1),18);
 quad(at(1,-1,1),at(1,1,1),at(1,1,-1),at(1,-1,-1),18);
 quad(at(0,-1,-1),at(0,-1,1),at(1,-1,1),at(1,-1,-1),18);
 quad(at(0,1,1),at(0,1,-1),at(1,1,-1),at(1,1,1),18);
 quad(at(0,-1,1),at(0,1,1),at(1,1,1),at(1,-1,1),18);
 quad(at(0,1,-1),at(0,-1,-1),at(1,-1,-1),at(1,1,-1),18);
 box(.14f,.55f,.62f,.46f,-.22f,0.f,18); // +X jamb pad
 box(.55f,.14f,.62f,-.22f,1.46f,0.f,18); // soffit pad
 break;}
case 69:box(2,.55f,.72f,.5f,.12f,0,18);break; // Girder — 2-wide thick board-form span
case 70: // Niche — deep square alcove (stack into pierced piers); open −Z
 box(.18f,1,.9f,-.41f,0,.05f,18);box(.18f,1,.9f,.41f,0,.05f,18);
 box(.9f,.18f,.9f,0,.41f,.05f,18);box(.9f,.18f,.9f,0,-.41f,.05f,18);
 box(1,1,.18f,0,0,.41f,18);break;
case 71: // Hood — continuous niche jambs + outer lip past −Z (no floating frame)
 box(.16f,1.f,1.2f,-.42f,0.f,-.1f,18);box(.16f,1.f,1.2f,.42f,0.f,-.1f,18);
 box(.84f,.16f,1.2f,0.f,.42f,-.1f,18);box(.84f,.16f,1.2f,0.f,-.42f,-.1f,18);
 box(1.f,1.f,.16f,0.f,0.f,.42f,18);
 box(.12f,.92f,.32f,-.44f,0.f,-.66f,18);box(.12f,.92f,.32f,.44f,0.f,-.66f,18);
 box(.92f,.12f,.32f,0.f,.44f,-.66f,18);box(.92f,.12f,.32f,0.f,-.44f,-.66f,18);break;
case 72:{ // True circular oculus; intentionally a small opening, not a doorway.
 for(int i=0;i<32;i++){
  float a=2*PI*i/32,b=2*PI*(i+1)/32;
  V u{cosf(a),sinf(a),0},v{cosf(b),sinf(b),0};
  V A=u*(.5f/std::max(fabsf(u.x),fabsf(u.y))),B=v*(.5f/std::max(fabsf(v.x),fabsf(v.y)));
  V U=u*.32f,W=v*.32f;
  A.z=B.z=U.z=W.z=.18f;quad(A,U,W,B,18);
  V C=A,D=B,X=U,Y=W;C.z=D.z=X.z=Y.z=.5f;quad(C,D,Y,X,18);quad(U,X,Y,W,18);quad(A,B,D,C,18);
 }break;}
case 73: // Stripe — yellow hazard bands on a board-form face
 box(1,1,1,0,0,0,18);box(.92f,.11f,.05f,0,.14f,-.48f,41);box(.92f,.11f,.05f,0,-.14f,-.48f,41);break;
case 74: // Half — half-depth wall slab (front of cell, open toward −Z approach)
 box(1,1,.5f,0,0,-.25f,18);break;
case 75:{ // Lilypad — real pack pad mesh over a thin water disc
 quad({-.4f,-.42f,.4f},{.4f,-.42f,.4f},{.4f,-.42f,-.4f},{-.4f,-.42f,-.4f},9);
 IvyRandom padRng{natureSeed()+211u};
 natureModel(nature_clover,{padRng.range(-.12f,.12f),-.40f,padRng.range(-.12f,.12f)},padRng.range(1.05f,1.35f),padRng.range(0,2*PI),false);
 break;}
case 76:{ // Grass — meadow / tallgrass cluster (not PSX atlas-on-box)
 natureGrass((int)(natureSeed()%6),{0,-.42f,0},1.05f,0.f,false);
 natureGrass((int)((natureSeed()>>3)%6),{.22f,-.40f,-.18f},.95f,1.1f,false);
 natureGrass((int)((natureSeed()>>7)%6),{-.2f,-.40f,.2f},1.1f,2.2f,true);
 break;}
case 77:natureCascade();break;
case 78: // Flight — continuous 2-wide stair run + side and center rails
 for(int i=0;i<4;i++)box(2,.25f*(i+1),.25f,.5f,-.5f+.125f*(i+1),.375f-.25f*i);
 stairHandrails(stairSideBlocked(-1)?-999.f:-.42f,stairSideBlocked(2)?-999.f:1.42f,4,.5f);break;
case 79: // Amber lamp
 box(.6f,.07f,.07f,0,.4f,0,42);box(.66f,.04f,.14f,0,.46f,0,3);break;
case 80: // Cool lamp
 box(.6f,.07f,.07f,0,.4f,0,43);box(.66f,.04f,.14f,0,.46f,0,3);break;
case 81: // Ribbed — vertical board ribs on board-form
 box(1,1,1,0,0,0,18);for(int i=0;i<5;i++)box(.08f,.98f,.06f,-.4f+i*.2f,0,-.48f,18);break;
case 82:natureGreenWall();break;
case 83:{ // Lattice — inset −Z grille; bars run to cell edges so tiling merges the carved field
 bool nL=faceNeighbor(83,-1,0),nR=faceNeighbor(83,1,0),nD=faceNeighbor(83,0,-1),nU=faceNeighbor(83,0,1);
 const float z=-.38f,d=.28f,t=.07f;
 box(1.f,1.f,.05f,0,0,-.48f,18); // back plate inset in wall
 for(int i=0;i<5;i++){
  float u=-.4f+i*.2f;
  box(t,1.f,d,u,0,z);
  box(1.f,t,d,0,u,z);
 }
 // Outer lip only where the lattice meets non-lattice wall
 const float lip=.05f;
 if(!nU)box(1.f,lip,d,0,.5f-lip*.5f,z,18);
 if(!nD)box(1.f,lip,d,0,-.5f+lip*.5f,z,18);
 if(!nL)box(lip,1.f,d,-.5f+lip*.5f,0,z,18);
 if(!nR)box(lip,1.f,d,.5f-lip*.5f,0,z,18);
 break;}
case 84:{ // Glassblock — 3×3 translucent blocks, seated on −Z face
 for(int iy=0;iy<3;iy++)for(int ix=0;ix<3;ix++)box(.28f,.28f,.22f,-.3f+ix*.3f,-.3f+iy*.3f,-.35f,2);
 break;}
case 85: // Embrasure — deep splayed recess, open −Z
 box(.16f,1,.75f,-.42f,0,.05f,18);box(.16f,1,.75f,.42f,0,.05f,18);
 box(.68f,.16f,.75f,0,.42f,.05f,18);box(.68f,.16f,.75f,0,-.42f,.05f,18);
 box(1,1,.14f,0,0,.43f,18);
 box(.12f,.9f,.28f,-.46f,0,-.48f,18);box(.12f,.9f,.28f,.46f,0,-.48f,18);
 box(.9f,.12f,.28f,0,.46f,-.48f,18);box(.9f,.12f,.28f,0,-.46f,-.48f,18);break;
case 86:{ // Real branching sumac bush; three stable placement variants.
 IvyRandom rng{natureSeed()^0x62u};V root{0,-.49f,0};float a=rng.range(0,2*PI),scale=rng.range(.95f,1.15f);
 switch(natureSeed()%3){case 0:natureModel(nature_shrub_1,root,scale,a,false);break;case 1:natureModel(nature_shrub_2,root,scale,a,false);break;default:natureModel(nature_shrub_3,root,scale,a,false);}
 break;}
case 87:natureHanging(87);break;
case 88:natureSurfaceCover(88);break;
case 89:natureSurfaceCover(89);break;
case 90:natureHanging(90);break;
case 91:natureSurfaceCover(91);break;
case 92:natureHanging(92);break;
case 93: // Opposite slope: high at +Z; normals and collision agree.
 quad({.5f,-.5f,-.5f},{-.5f,-.5f,-.5f},{-.5f,.5f,.5f},{.5f,.5f,.5f},0);quad({-.5f,-.5f,.5f},{.5f,-.5f,.5f},{.5f,.5f,.5f},{-.5f,.5f,.5f},0);tri({.5f,-.5f,.5f},{.5f,-.5f,-.5f},{.5f,.5f,.5f},0);tri({-.5f,-.5f,-.5f},{-.5f,-.5f,.5f},{-.5f,.5f,.5f},0);quad({.5f,-.5f,.5f},{-.5f,-.5f,.5f},{-.5f,-.5f,-.5f},{.5f,-.5f,-.5f},0);break;
case 94: // Quadpier — four slender square pillars packed into one cell
 for(V c:{V{-.2f,0,-.2f},V{.2f,0,-.2f},V{-.2f,0,.2f},V{.2f,0,.2f}})
  box(.18f,1.f,.18f,c.x,0,c.z);
 break;
case 95: // Angle — wall cube flush −Z, luminous aperture aimed down the face
 box(.34f,.34f,.05f,0,.08f,-.475f,3); // backplate
 box(.28f,.28f,.22f,0,.08f,-.38f,3);  // housing
 box(.2f,.03f,.16f,0,-.06f,-.36f,4);  // down-facing emitter
 break;

case 96: // Two-metre interactive hinged service door.
 serviceDoorFrame();if(!doorFramePass){int index=idx((int)floorf(origin.x),(int)floorf(origin.y),(int)floorf(origin.z));serviceDoorLeaf(doorAmount(index));}break;
case 97: // Wall ventilation grille with visible louvres.
 box(.86f,.70f,.055f,0,0,-.46f,8);box(.74f,.58f,.02f,0,0,-.42f,3);
 for(int i=0;i<7;i++)box(.70f,.035f,.075f,0,-.25f+i*.08f,-.38f,8);break;
case 98: // Open ladder cable tray, aligned to merge across neighboring cells.
 box(1.f,.08f,.035f,0,.3f,-.23f,8);box(1.f,.08f,.035f,0,.3f,.23f,8);
 for(int i=0;i<5;i++)box(.035f,.035f,.46f,-.4f+i*.2f,.27f,0,8);
 for(float z:{-.13f,0.f,.13f})box(1.f,.025f,.025f,0,.30f,z,13);break;
case 99: // Recessed fluorescent ceiling panel.
 box(.94f,.10f,.94f,0,.43f,0,8);box(.86f,.025f,.86f,0,.367f,0,3);
 for(float z:{-.25f,0.f,.25f})box(.77f,.028f,.10f,0,.345f,z,4);break;
case 100: // Green exit pictogram in a wall-mounted housing.
 box(.88f,.32f,.10f,0,.22f,-.43f,3);box(.78f,.25f,.015f,0,.22f,-.37f,51);
 box(.36f,.045f,.014f,0,.22f,-.355f,4);tri({.25f,.22f,-.345f},{.10f,.32f,-.345f},{.10f,.12f,-.345f},4);break;
case 101:case 102: // Cyan / rose neon, with visible supports and dark backing.
 box(.92f,.32f,.05f,0,.1f,-.46f,3);
 for(float x:{-.38f,.38f})box(.04f,.24f,.09f,x,.1f,-.40f,8);
 box(.82f,.035f,.035f,0,.18f,-.34f,t==101?48:49);box(.82f,.035f,.035f,0,.02f,-.34f,t==101?48:49);break;
case 103: // Industrial terminal, inset screen and keyboard shelf.
 box(.72f,.65f,.16f,0,.06f,-.41f,8);box(.60f,.43f,.016f,0,.12f,-.32f,3);box(.52f,.34f,.012f,0,.12f,-.304f,50);
 for(int i=0;i<4;i++)box(.32f-i*.045f,.012f,.007f,-.05f,.23f-i*.065f,-.293f,48);
 box(.68f,.06f,.30f,0,-.32f,-.30f,8);for(int i=0;i<8;i++)box(.045f,.015f,.10f,-.24f+i*.068f,-.28f,-.23f,3);break;
// Content update: Cyber (104-114) and Liminal (115-123) parts. Box-built so HTML defs mirror them exactly.
case 104: // Neon violet strip pair on a dark backing.
 box(.92f,.32f,.05f,0,.1f,-.46f,3);for(float x:{-.38f,.38f})box(.04f,.24f,.09f,x,.1f,-.40f,8);box(.82f,.035f,.035f,0,.18f,-.34f,53);box(.82f,.035f,.035f,0,.02f,-.34f,53);break;
case 105: // Holo sign: vertical cyan panel on a wall bracket with dark glyph bars.
 box(.1f,.08f,.12f,0,.42f,-.44f,8);box(.5f,.9f,.06f,0,0,-.44f,3);box(.42f,.8f,.015f,0,0,-.40f,48);for(float y:{.25f,0.f,-.25f})box(.3f,.05f,.01f,0,y,-.391f,50);break;
case 106: // Server rack with LED rows.
 box(.8f,1,.8f,0,0,.05f,58);box(.72f,.9f,.02f,0,0,-.36f,3);for(int i=0;i<6;i++){box(.5f,.02f,.01f,-.08f,-.38f+i*.15f,-.372f,48);box(.06f,.02f,.01f,.28f,-.38f+i*.15f,-.372f,49);}break;
case 107: // Vending machine: glass front, lit header, keypad and slot.
 box(.8f,1,.6f,0,0,.1f,58);box(.56f,.55f,.02f,-.08f,.12f,-.2f,2);box(.72f,.16f,.02f,0,.4f,-.2f,4);box(.12f,.4f,.02f,.3f,.05f,-.2f,3);box(.5f,.12f,.02f,-.08f,-.34f,-.2f,13);break;
case 108: // Steel grate floor with kerb strips.
 box(1,.04f,1,0,-.46f,0,8);for(int i=0;i<8;i++)box(1,.03f,.04f,0,-.43f,-.44f+i*.125f,3);box(1,.08f,.06f,0,-.44f,-.47f,8);box(1,.08f,.06f,0,-.44f,.47f,8);break;
case 109: // Jersey barrier with hazard bands both sides.
 box(1,.3f,.6f,0,-.35f,0,0);box(1,.3f,.36f,0,-.05f,0,0);box(1,.1f,.24f,0,.15f,0,0);box(1,.08f,.012f,0,-.3f,-.306f,41);box(1,.08f,.012f,0,-.3f,.306f,41);break;
case 110: // Square ventilation duct with end flanges.
 box(1,.5f,.5f,0,.25f,0,8);box(.06f,.56f,.56f,-.44f,.25f,0,3);box(.06f,.56f,.56f,.44f,.25f,0,3);break;
case 111: // Wall fan: frame, dark throat, cross blades and hub.
 box(.9f,.9f,.08f,0,0,-.46f,8);box(.78f,.78f,.02f,0,0,-.43f,13);box(.64f,.1f,.02f,0,0,-.41f,3);box(.1f,.64f,.02f,0,0,-.41f,3);box(.14f,.14f,.04f,0,0,-.40f,8);break;
case 112: // Antenna mast with twin dishes and a rose tip light.
 cylinder(.03f,1,0,3,8);box(.5f,.03f,.03f,0,.3f,0,3);box(.16f,.16f,.03f,.2f,.3f,0,8);box(.16f,.16f,.03f,-.2f,.3f,0,8);box(.04f,.04f,.04f,0,.5f,0,49);break;
case 113: // Floor-level cyan light strip along a wall base.
 box(1,.1f,.08f,0,-.45f,-.46f,3);box(1,.04f,.03f,0,-.41f,-.44f,48);break;
case 114: // Grid floor: dark plate with glowing cyan lines.
 box(1,.1f,1,0,-.45f,0,13);for(int i=0;i<4;i++){box(1,.012f,.02f,0,-.395f,-.375f+i*.25f,48);box(.02f,.012f,1,-.375f+i*.25f,-.395f,0,48);}break;
case 115:box(1,1,1,0,0,0,54);break; // Carpet cube
case 116:box(1,1,1,0,0,0,55);box(1.004f,.05f,1.004f,0,-.12f,0,15);break; // Yellow wall with a dark dado stripe
case 117: // Drop-ceiling tile with T-bar edges.
 box(1,.12f,1,0,.44f,0,56);box(1,.015f,.04f,0,.375f,-.48f,3);box(1,.015f,.04f,0,.375f,.48f,3);box(.04f,.015f,1,-.48f,.375f,0,3);box(.04f,.015f,1,.48f,.375f,0,3);break;
case 118: // Bare twin fluorescent tubes.
 box(1,.06f,.22f,0,.44f,0,3);box(.9f,.04f,.04f,0,.405f,-.06f,4);box(.9f,.04f,.04f,0,.405f,.06f,4);break;
case 119:box(1,1,1,0,0,0,57);break; // Pool tile cube
case 120: // Blank painted door in a dark frame, flush on the wall.
 box(.08f,1,.14f,-.46f,0,-.43f,3);box(.08f,1,.14f,.46f,0,-.43f,3);box(1,.06f,.14f,0,.47f,-.43f,3);box(.84f,.94f,.05f,0,-.03f,-.44f,5);box(.04f,.04f,.06f,.28f,-.05f,-.49f,8);break;
case 121: // Office partition with top rail and feet.
 box(1,.62f,.06f,0,-.19f,0,54);box(1,.03f,.09f,0,.13f,0,8);box(.06f,.03f,.3f,-.4f,-.485f,0,3);box(.06f,.03f,.3f,.4f,-.485f,0,3);break;
case 122: // Water cooler: white body, glass jug, tap.
 box(.3f,.62f,.3f,0,-.19f,0,56);cylinder(.14f,.3f,.34f,2,12);box(.06f,.04f,.08f,0,-.1f,-.19f,3);break;
case 123:box(.3f,1,.3f,0,0,0,55);break; // Painted square pillar
