// Exact texture recipes from monolith.html canvas code (lines 48-64, 55-62)
static uint32_t rngState=1;
static float frand(){rngState=rngState*1664525u+1013904223u;return(rngState>>8)*(1.f/16777216.f);}
static void putPx(unsigned char* img,int x,int y,int r,int g,int b,int a=255){x&=127;y&=127;int i=(y*128+x)*4;img[i]=r;img[i+1]=g;img[i+2]=b;img[i+3]=a;}
static void getPx(const unsigned char* img,int x,int y,int& r,int& g,int& b){x&=127;y&=127;int i=(y*128+x)*4;r=img[i];g=img[i+1];b=img[i+2];}
static void blendPx(unsigned char* img,int x,int y,int r,int g,int b,float a){int R,G,B;getPx(img,x,y,R,G,B);putPx(img,x,y,(int)(R*(1-a)+r*a),(int)(G*(1-a)+g*a),(int)(B*(1-a)+b*a));}
static void hsl(float h,float s,float l,int& r,int& g,int& b){h=fmodf(h,360.f)/360.f;s/=100;l/=100;auto hue2rgb=[](float p,float q,float t){if(t<0)t+=1;if(t>1)t-=1;if(t<1.f/6)return p+(q-p)*6*t;if(t<.5f)return q;if(t<2.f/3)return p+(q-p)*(2.f/3-t)*6;return p;};
float q=l<.5f?l*(1+s):l+s-l*s,p=2*l-q;r=(int)(hue2rgb(p,q,h+1.f/3)*255);g=(int)(hue2rgb(p,q,h)*255);b=(int)(hue2rgb(p,q,h-1.f/3)*255);}
static void bakeConcrete(unsigned char* img){rngState=1;for(int i=0;i<128*128;i++){img[i*4]=0x9d;img[i*4+1]=0x9b;img[i*4+2]=0x95;img[i*4+3]=255;}
for(int i=0;i<900;i++){int v=150+(int)(frand()*60);float a=.08f;int x=(int)(frand()*128),y=(int)(frand()*128);blendPx(img,x,y,v,v,v-4,a);}
for(int i=0;i<14;i++){float x0=frand()*128,w=1+frand()*3;for(int y=0;y<128;y++){float a=.22f*(y/127.f);for(int dx=0;dx<(int)w+1;dx++)blendPx(img,(int)x0+dx,y,40,38,34,a);}}
for(int i=0;i<128;i++)for(int t=0;t<2;t++){blendPx(img,i,t,60,58,52,.55f);blendPx(img,i,127-t,60,58,52,.55f);blendPx(img,t,i,60,58,52,.55f);blendPx(img,127-t,i,60,58,52,.55f);}
{int holes[4][2]={{28,28},{100,28},{28,100},{100,100}};for(int h=0;h<4;h++)for(int y=-4;y<=4;y++)for(int x=-4;x<=4;x++)if(x*x+y*y<=13)putPx(img,holes[h][0]+x,holes[h][1]+y,15,15,14);}
for(int i=0;i<40;i++){float x0=frand()*128,y0=frand()*128,rr=14+frand()*30;bool dark=frand()<.5f;for(int y=0;y<128;y++)for(int x=0;x<128;x++){float d=hypotf(x-x0,y-y0)/rr;if(d<1){float a=(1-d)*(dark?.10f:.09f);if(dark)blendPx(img,x,y,40,40,38,a);else blendPx(img,x,y,220,220,214,a);}}}
for(int i=0;i<140;i++){int x=(int)(frand()*128),y=(int)(frand()*128);float rad=.5f+frand()*1.2f,a=.2f+frand()*.4f;for(int dy=-(int)rad-1;dy<=(int)rad+1;dy++)for(int dx=-(int)rad-1;dx<=(int)rad+1;dx++)if(dx*dx+dy*dy<=rad*rad)blendPx(img,x+dx,y+dy,30,30,28,a);}}
static void bakeLeaf(unsigned char* img,bool alpha){rngState=alpha?7:3;for(int i=0;i<128*128;i++){if(alpha){img[i*4]=img[i*4+1]=img[i*4+2]=img[i*4+3]=0;}else{img[i*4]=0x2b;img[i*4+1]=0x44;img[i*4+2]=0x21;img[i*4+3]=255;}}
int n=alpha?400:460;for(int i=0;i<n;i++){int r,g,b;hsl(80+frand()*35,40+frand()*30,24+frand()*34,r,g,b);float x=frand()*128,y=frand()*128,rx=2.5f+frand()*4,ry=1.5f+frand()*2.5f,ang=frand()*6.3f,ca=cosf(ang),sa=sinf(ang);
for(int yy=-(int)ry-2;yy<=(int)ry+2;yy++)for(int xx=-(int)rx-2;xx<=(int)rx+2;xx++){float u=xx*ca+yy*sa,v=-xx*sa+yy*ca;if((u*u)/(rx*rx)+(v*v)/(ry*ry)<=1){int px=(int)x+xx,py=(int)y+yy;if(alpha)putPx(img,px,py,r,g,b,255);else blendPx(img,px,py,r,g,b,.85f);}}}}
static void bakeBrick(unsigned char* img){rngState=11;for(int i=0;i<128*128;i++){img[i*4]=0x3e;img[i*4+1]=0x31;img[i*4+2]=0x2b;img[i*4+3]=255;}
for(int row=0;row<8;row++)for(int i=-1;i<4;i++){int ox=(row%2)*16,r,g,b;hsl(10+frand()*10,28+frand()*14,36+frand()*24,r,g,b);for(int y=row*16+1;y<row*16+15;y++)for(int x=i*32+ox+1;x<i*32+ox+31;x++)putPx(img,x,y,r,g,b);}
for(int i=0;i<500;i++)blendPx(img,(int)(frand()*128),(int)(frand()*128),0,0,0,.08f);}
static void bakeWood(unsigned char* img){rngState=13;for(int i=0;i<128*128;i++){img[i*4]=0x6b;img[i*4+1]=0x4a;img[i*4+2]=0x2e;img[i*4+3]=255;}
for(int y=0;y<128;y++){bool dark=frand()<.5f;float a=.04f+frand()*.08f;for(int x=0;x<128;x++){if(dark)blendPx(img,x,y,30,18,8,a);else blendPx(img,x,y,210,150,90,a);}}
for(int i=0;i<5;i++)for(int x=0;x<128;x++)blendPx(img,x,i*26,20,10,4,.6f);}
static void bakeSteel(unsigned char* img){rngState=17;for(int i=0;i<128*128;i++){img[i*4]=0x58;img[i*4+1]=0x5d;img[i*4+2]=0x60;img[i*4+3]=255;}
for(int i=0;i<16;i++)for(int y=0;y<128;y++)for(int x=i*8;x<i*8+4&&x<128;x++){if(i%2)blendPx(img,x,y,255,255,255,.12f);else blendPx(img,x,y,0,0,0,.2f);}
for(int i=0;i<300;i++)blendPx(img,(int)(frand()*128),(int)(frand()*128),120,70,40,.15f);}
static void bakeBoardForm(unsigned char* img){rngState=29;for(int i=0;i<128*128;i++){img[i*4]=0x8e;img[i*4+1]=0x8a;img[i*4+2]=0x82;img[i*4+3]=255;}
for(int i=0;i<22;i++){float x0=frand()*128,w=3+frand()*5;for(int y=0;y<128;y++){float a=.18f+.12f*(y/127.f);for(int dx=0;dx<(int)w+1;dx++)blendPx(img,(int)x0+dx,y,55,52,48,a);}}
for(int i=0;i<18;i++){float y0=frand()*128,h=1+frand()*2;for(int x=0;x<128;x++)for(int dy=0;dy<(int)h+1;dy++)blendPx(img,x,(int)y0+dy,70,68,62,.12f);}
for(int i=0;i<700;i++){int v=120+(int)(frand()*50);blendPx(img,(int)(frand()*128),(int)(frand()*128),v,v-2,v-6,.07f);}
for(int gy=0;gy<4;gy++)for(int gx=0;gx<4;gx++){int hx=16+gx*32,hy=16+gy*32;for(int y=-5;y<=5;y++)for(int x=-5;x<=5;x++){int d=x*x+y*y;if(d<=20)putPx(img,hx+x,hy+y,22,22,20);if(d<=6)putPx(img,hx+x,hy+y,12,12,11);}
for(int a=0;a<12;a++){float t=a/12.f*2*PI;blendPx(img,hx+(int)(cosf(t)*7),hy+(int)(sinf(t)*7),90,88,82,.55f);}}
for(int i=0;i<80;i++)blendPx(img,(int)(frand()*128),(int)(frand()*128),30,30,28,.35f);}
// HTML grass canvas gk 64→128: stroked blades, alphaTest .45, color 0xb4c89a
static void bakeGrass(unsigned char* img){rngState=23;for(int i=0;i<128*128;i++){img[i*4]=img[i*4+1]=img[i*4+2]=img[i*4+3]=0;}
for(int i=0;i<20;i++){float x0=12+frand()*104,lean=(frand()-.5f)*52;int r,g,b;hsl(85+frand()*30,40+frand()*25,28+frand()*30,r,g,b);float lw=2+frand()*2;
for(int s=0;s<64;s++){float t=s/63.f;float x=x0+lean*.4f*t*(2-t)+lean*t*t,y=128-t*120;for(int dy=-1;dy<=1;dy++)for(int dx=-(int)lw;dx<=(int)lw;dx++){int px=(int)x+dx,py=(int)y+dy;if(px>=0&&px<128&&py>=0&&py<128){float a=1.f-fabsf(dx)/lw;if(a>0){int o=(py*128+px)*4;img[o]=(unsigned char)(r*a+img[o]*(1-a));img[o+1]=(unsigned char)(g*a+img[o+1]*(1-a));img[o+2]=(unsigned char)(b*a+img[o+2]*(1-a));img[o+3]=(unsigned char)std::max((int)img[o+3],(int)(a*255));}}}}}
}