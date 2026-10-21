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
case 36:for(int i=0;i<8;i++)box(1,.125f*(i+1),.125f,0,-.5f+.0625f*(i+1),.4375f-.125f*i);break;
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
 cylinder(.26f,.34f,1.83f);box(.85f,.12f,.85f,0,1.94f);break;
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
