// Shared player-sized collision and structure shadow proxies.
switch(type){case 1:return f.y<.5;case 2:case 7:case 14:case 41:case 42:return PART_ABS(f.x-.5f)<.2f&&PART_ABS(f.z-.5f)<.2f;case 43:case 44:case 45:case 46:return false;case 4:return f.y<1-f.z;case 6:return f.y<PART_CEIL((1-f.z)*4)/4;case 8:return PART_ABS(f.z-.5f)<.15f&&!(f.x>.15&&f.x<.85&&f.y>.25&&f.y<.8);case 9:return PART_ABS(f.z-.5f)<.15f;case 10:return !(PART_ABS(f.x-.5f)<.25f&&(f.y<.5f||(f.x-.5f)*(f.x-.5f)+(f.y-.5f)*(f.y-.5f)<.0625f));case 11:return f.z<.15;case 12:return f.y>.7&&PART_ABS(f.z-.5f)<.15;case 13:case 15:case 16:case 25:case 27:case 95:return false;case 17:return f.y<.5;case 21:return PART_ABS(f.z-.5f)<.25;case 22:return f.z<.48f; // inset cobogó slab + outward diamonds
case 83:return f.z<.42f; // inset lattice
case 23:return f.z>.02f&&f.z<.72f&&!(f.x>.3f&&f.x<1.7f&&f.y<1.82f); // deep interior-biased collider
case 24:return f.y<.1||(f.y<.5&&PART_MAX(PART_ABS(f.x-.5f),PART_ABS(f.z-.5f))>.4);case 26:return PART_ABS(f.x-.5f)<.12&&PART_ABS(f.z-.5f)<.12&&f.y<.8;
case 28:return (f.x-.5f)*(f.x-.5f)+(f.z-.5f)*(f.z-.5f)+f.y*f.y<.25f;
case 29:return (f.x-.5f)*(f.x-.5f)+(f.z-.5f)*(f.z-.5f)<.25f;
case 30:return f.y<.5f+PART_SQRT(PART_MAX(0.f,.25f-(f.x-.5f)*(f.x-.5f)));
case 31:return PART_ABS(f.z-.5f)<.05f;case 32:case 33:return PART_ABS(f.z-.5f)<.15f;
case 34:return f.y>.85f;case 35:case 37:case 40:return false;
case 36:return f.y<PART_CEIL((1-f.z)*8)/8;
case 38:return PART_ABS(f.x-.5f)<.2f&&PART_ABS(f.z-.5f)<.2f;
case 39:return f.y<.35f&&PART_ABS(f.z-.5f)<.2f&&PART_ABS(f.x-.5f)<.45f;
case 48:return f.y>.72f;case 49:return f.z>=(f.y<.28f?.05f:f.y<.56f?.2f:f.y<.84f?.35f:.5f);case 50:return f.y>.86f;
case 51:return f.z>-.05f&&f.z<.7f&&!(f.x>.32f&&f.x<1.68f&&f.y<1.8f);
case 52:return PART_ABS(f.z-.5f)<.175f&&!(f.x>.1f&&f.x<.9f&&f.y>.55f&&f.y<.85f);
case 53:return f.y<.22f;case 54:return f.z<.18f||PART_ABS(f.x-.5f)>.32f;case 55:return f.y>.65f&&PART_ABS(f.z-.5f)<.35f;
case 56:return (f.y>1.72f&&PART_ABS(f.x-.5f)<.425f&&PART_ABS(f.z-.5f)<.425f)||(f.y<=1.72f&&PART_ABS(PART_ABS(f.x-.5f)-.36f)<.055f&&PART_ABS(PART_ABS(f.z-.5f)-.36f)<.055f);
case 57:return f.y<.5f;
case 58:return f.x<.18f||f.x>1.82f||f.z<.18f||f.z>1.82f;
case 59:return f.y<.12f||PART_ABS(f.z-.5f)>.38f||f.y>1.85f; // standing clearance inside tunnel
case 60:return PART_ABS(f.x-.5f)>.36f||f.z>.86f||f.y>1.85f; // walk-in shaft for 1.6m player
case 62:return f.y>.05f&&f.z<.55f-.35f*f.y&&PART_ABS(f.x-.5f)<.3f;
case 63:return (f.x-.5f)*(f.x-.5f)+(f.z-.5f)*(f.z-.5f)+(f.y-.5f)*(f.y-.5f)<.22f; // stone
case 64:return (f.x-.5f)*(f.x-.5f)+(f.z-.5f)*(f.z-.5f)+(f.y-.5f)*(f.y-.5f)<.28f; // boulder
case 65:return f.y<.42f&&PART_ABS(f.z-.5f)<.28f; // fallen log
case 66:return PART_ABS(f.x-.5f)<.28f&&PART_ABS(f.z-.5f)<.28f&&f.y<.85f; // stump
case 67:return false; // mushroom
case 68:{ // strut XY diagonal (y≈2(1-x)) + +X/soffit pads
 bool diag=PART_ABS(f.z-.5f)<.32f&&PART_ABS(f.y-2.f*(1.f-f.x))<.42f&&f.y>=-.08f&&f.y<=2.08f;
 bool padX=f.x>.38f&&f.y<.4f&&PART_ABS(f.z-.5f)<.32f;
 bool padY=f.y>1.35f&&f.x<.4f&&PART_ABS(f.z-.5f)<.32f;
 return diag||padX||padY;}
case 69:return f.y>.345f&&f.y<.895f&&PART_ABS(f.z-.5f)<.36f; // girder slab
case 70:return PART_ABS(f.x-.5f)>.32f||PART_ABS(f.y-.5f)>.32f||f.z>.32f; // niche frame + back
case 71:if(f.z>=0.f)return PART_ABS(f.x-.5f)>.32f||PART_ABS(f.y-.5f)>.32f||f.z>.32f;return PART_ABS(f.x-.5f)>.30f||PART_ABS(f.y-.5f)>.30f;
case 72:{float u=f.x-.5f,v=f.y-.5f;return f.z>.68f&&(u*u+v*v)>.1024f;} // roundel ring
case 73:return true; // stripe sits on a solid board-form cell
case 74:return f.z<.5f; // half-depth wall (front half of cell)
case 75:case 76:case 77:return false; // lilypad / grass / cascade are nonblocking accents
case 78:return f.y<PART_CEIL((1.f-f.z)*4)/4; // 2-wide flight — same rise as Stairs
case 79:case 80:return false; // colored lamps
case 81:return true; // ribbed concrete mass
case 82:return false; // green-wall panel on −Z
case 84:return f.z<.4f; // glassblock flush slab
case 85:return PART_ABS(f.x-.5f)>.28f||PART_ABS(f.y-.5f)>.28f||f.z>.28f; // embrasure frame + back
case 86:case 87:case 88:case 89:case 90:case 91:case 92:return false; // nature accents
case 93:return f.y<f.z; // downslope — high −Z, low +Z
case 94:{bool lx=PART_ABS(f.x-.3f)<.1f||PART_ABS(f.x-.7f)<.1f,lz=PART_ABS(f.z-.3f)<.1f||PART_ABS(f.z-.7f)<.1f;return lx&&lz;}
case 96:return f.z>.03f&&f.z<.25f&&(PART_ABS(f.x-.5f)>.42f||f.y>1.9f); // frame; moving leaf tested separately
case 97:return f.z<.12f&&PART_ABS(f.x-.5f)<.43f&&PART_ABS(f.y-.5f)<.35f;
case 98:return f.y>.75f&&f.y<.85f&&PART_ABS(f.z-.5f)<.25f;
case 99:return f.y>.80f;
case 100:case 101:case 102:return false;
case 103:return f.z<.26f&&PART_ABS(f.x-.5f)<.36f&&f.y>.15f&&f.y<.90f;
case 104:case 105:case 113:case 118:return false; // neon, holo sign, light strip, tube light
case 106:return PART_ABS(f.x-.5f)<.4f&&f.z>.12f&&f.z<.95f; // server rack
case 107:return PART_ABS(f.x-.5f)<.4f&&f.z>.3f&&f.z<.92f; // vending machine
case 108:return f.y<.08f; // grate floor
case 109:return f.y<.55f&&PART_ABS(f.z-.5f)<.3f; // jersey barrier
case 110:return f.y>.5f&&PART_ABS(f.z-.5f)<.25f; // duct
case 111:return f.z<.12f; // wall fan
case 112:return PART_ABS(f.x-.5f)<.05f&&PART_ABS(f.z-.5f)<.05f; // antenna mast
case 114:return f.y<.1f; // grid floor
case 115:case 116:case 119:return true; // carpet, yellow wall, pool tile
case 117:return f.y>.88f; // ceiling tile
case 120:return f.z<.1f; // door blank
case 121:return f.y<.63f&&PART_ABS(f.z-.5f)<.06f; // partition
case 122:return PART_ABS(f.x-.5f)<.16f&&PART_ABS(f.z-.5f)<.16f&&f.y<.95f; // water cooler
case 123:return PART_ABS(f.x-.5f)<.15f&&PART_ABS(f.z-.5f)<.15f; // painted pillar
default:return true;}
