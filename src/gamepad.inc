// Basic XInput support — optional; ignored when no pad is connected.
struct GamepadState{bool connected=false;XINPUT_STATE cur{},prev{};float lx=0,ly=0,rx=0,ry=0;bool a=false,b=false,x=false,y=false,start=false,lb=false,rb=false;
 bool pressed(WORD mask)const{return (cur.Gamepad.wButtons&mask)&&!(prev.Gamepad.wButtons&mask);}
} pad;
float padAxis(SHORT v,SHORT dead=7849){if(v>-dead&&v<dead)return 0;float n=v/32767.f;return n;}
void pollGamepad(float dt){
 pad.prev=pad.cur;pad.connected=XInputGetState(0,&pad.cur)==ERROR_SUCCESS;if(!pad.connected)return;
 pad.lx=padAxis(pad.cur.Gamepad.sThumbLX);pad.ly=padAxis(pad.cur.Gamepad.sThumbLY);
 pad.rx=padAxis(pad.cur.Gamepad.sThumbRX);pad.ry=padAxis(pad.cur.Gamepad.sThumbRY);
 if(pad.pressed(XINPUT_GAMEPAD_START)){if(menuActive)action(60);else action(65);return;}
 if(menuActive){if(pad.pressed(XINPUT_GAMEPAD_B))action(22);if(uiModal==6){if(pad.pressed(XINPUT_GAMEPAD_DPAD_UP))menuSelection=(menuSelection+4)%5;if(pad.pressed(XINPUT_GAMEPAD_DPAD_DOWN))menuSelection=(menuSelection+1)%5;if(pad.pressed(XINPUT_GAMEPAD_A))action(60+menuSelection);invalidate();}return;}
 if(uiModal)return;
 if(pad.pressed(XINPUT_GAMEPAD_X)){if(walk)interactDoor();else undo();}
 if(pad.pressed(XINPUT_GAMEPAD_Y))redo();
 if(pad.pressed(XINPUT_GAMEPAD_B)){erasing=!erasing;invalidate();}
 if(pad.pressed(XINPUT_GAMEPAD_LEFT_SHOULDER)){selectType((selected+PieceCount-1)%PieceCount);}
 if(pad.pressed(XINPUT_GAMEPAD_RIGHT_SHOULDER)){selectType((selected+1)%PieceCount);}
 if(walk){
  yaw-=pad.rx*1.6f*dt*mouseSens;pitch=std::clamp(pitch+pad.ry*1.2f*dt*mouseSens,-1.5f,1.5f);
  keys['W']=pad.ly>.35f;keys['S']=pad.ly<-.35f;keys['A']=pad.lx<-.35f;keys['D']=pad.lx>.35f;
  keys[VK_SHIFT]=pad.cur.Gamepad.bLeftTrigger>140;keys[' ']=pad.cur.Gamepad.bRightTrigger>140;
 }else{
  theta-=pad.rx*1.4f*dt*mouseSens;phi=std::clamp(phi-pad.ry*1.1f*dt*mouseSens,.15f,1.5f);
  V side{cosf(theta),0,-sinf(theta)};target=target+V{-sinf(theta),0,-cosf(theta)}*(pad.ly*6.f*dt)+side*(pad.lx*6.f*dt);
  target.y=std::max(0.f,target.y+(pad.cur.Gamepad.bRightTrigger-pad.cur.Gamepad.bLeftTrigger)/255.f*4.f*dt);
  if(pad.pressed(XINPUT_GAMEPAD_A)){pick();remember();std::fill(std::begin(placed),std::end(placed),false);edit();invalidate();}
 }
}
