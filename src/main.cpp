#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

const char* ssid = "ESP32-Stream";
const char* password = "12345678";

WebServer server(80);
WebSocketsServer webSocket(81);

// --- Gioco ---
struct Bird { int x=50; float y=50; float dy=0; };
struct Pipe { float x; float y; bool passed=false; };

Bird bird;
#define MAX_PIPES 3
Pipe pipes[MAX_PIPES];

bool gameOver=false;
bool gamePass=false;
int gap=40;
float gravity=0.25;
float jump=-3.0;
int score=0;

#define W 160
#define H 120
uint8_t frame[W*H]; // 0=sky,1=bird,2=pipe,3=text

// --- FONT 5x5 ---
const uint8_t font5x5[36][5] = {
  {0x1F,0x11,0x11,0x11,0x1F},{0x04,0x0C,0x04,0x04,0x0E},{0x1F,0x01,0x1F,0x10,0x1F},{0x1F,0x01,0x1F,0x01,0x1F},{0x11,0x11,0x1F,0x01,0x01},
  {0x1F,0x10,0x1F,0x01,0x1F},{0x1F,0x10,0x1F,0x11,0x1F},{0x1F,0x01,0x02,0x04,0x04},{0x1F,0x11,0x1F,0x11,0x1F},{0x1F,0x11,0x1F,0x01,0x1F},
  {0x0E,0x11,0x1F,0x11,0x11},{0x1E,0x11,0x1E,0x11,0x1E},{0x0E,0x11,0x10,0x11,0x0E},{0x1E,0x11,0x11,0x11,0x1E},{0x1F,0x10,0x1E,0x10,0x1F},
  {0x1F,0x10,0x1E,0x10,0x10},{0x0F,0x10,0x17,0x11,0x0F},{0x11,0x11,0x1F,0x11,0x11},{0x0E,0x04,0x04,0x04,0x0E},{0x07,0x02,0x02,0x12,0x0C},
  {0x11,0x12,0x1C,0x12,0x11},{0x10,0x10,0x10,0x10,0x1F},{0x11,0x1B,0x15,0x11,0x11},{0x11,0x19,0x15,0x13,0x11},{0x0E,0x11,0x11,0x11,0x0E},
  {0x1E,0x11,0x1E,0x10,0x10},{0x0E,0x11,0x11,0x15,0x0F},{0x1E,0x11,0x1E,0x12,0x11},{0x0F,0x10,0x0E,0x01,0x1E},{0x1F,0x04,0x04,0x04,0x04},
  {0x11,0x11,0x11,0x11,0x0E},{0x11,0x11,0x0A,0x0A,0x04},{0x11,0x11,0x15,0x1B,0x11},{0x11,0x0A,0x04,0x0A,0x11},{0x1F,0x02,0x04,0x08,0x1F}
};

// --- paint Character ---
void drawChar(char c,int x,int y,uint8_t color){
    int index=-1;
    if(c>='0' && c<='9') index=c-'0';
    else if(c>='A' && c<='Z') index=c-'A'+10;
    else return;
    for(int row=0;row<5;row++){
        for(int col=0;col<5;col++){
            if((font5x5[index][row]>>(4-col)) & 0x01){
                int fx=x+col;
                int fy=y+row;
                if(fx<W && fy<H) frame[fy*W + fx]=color;
            }
        }
    }
}

// --- paunt string ---
void drawText(const char* txt,int x,int y,uint8_t color){
    for(int i=0;txt[i];i++){
        drawChar(txt[i],x+i*6,y,color);
    }
}

// --- paint number ---
void drawNumber(int n,int x,int y,uint8_t color){
    char buf[4];
    snprintf(buf,sizeof(buf),"%d",n);
    drawText(buf,x,y,color);
}

// --- Initialize pipe ---
void initPipes(){
    for(int i=0;i<MAX_PIPES;i++){
        pipes[i].x = W + i*60 + random(0,20);
        pipes[i].y = random(10,H-gap-10);
        pipes[i].passed=false;
    }
}

// --- paint frame ---
void drawFrame(){
    for(int i=0;i<W*H;i++) frame[i]=0; // blue sky

    // Bird yellow
    int bx=bird.x;
    int by=(int)bird.y;
    for(int dx=0;dx<5;dx++)
        for(int dy=0;dy<5;dy++)
            if(bx+dx<W && by+dy<H) frame[(by+dy)*W + bx+dx]=1;

    // Pipes green
    for(int i=0;i<MAX_PIPES;i++){
        int px=(int)pipes[i].x;
        int py=(int)pipes[i].y;
        for(int dx=0;dx<10;dx++){
            for(int dy=0;dy<py;dy++){
                if(px+dx<W && dy<H) frame[dy*W + px+dx]=2;
            }
            for(int dy=py+gap;dy<H;dy++){
                if(px+dx<W) frame[dy*W + px+dx]=2;
            }
        }
    }

    drawNumber(score,2,2,3); // Score up-left

    if(gameOver) drawText("GAME OVER",40,50,3);
    if(gamePass) drawText("GAME PASS",40,50,3);
}

// --- update logic ---
void updateGame(){
    if(gameOver || gamePass) return;

    bird.dy += gravity;
    bird.y += bird.dy;

    for(int i=0;i<MAX_PIPES;i++){
        pipes[i].x -= 2.0;

        if(!pipes[i].passed && pipes[i].x+10 < bird.x){
            pipes[i].passed=true;
            score++;
            if(score>=10) gamePass=true;
        }

        if(pipes[i].x+10<0){
            pipes[i].x=W + random(0,20);
            pipes[i].y=random(10,H-gap-10);
            pipes[i].passed=false;
        }

        if(bird.x+5>pipes[i].x && bird.x<pipes[i].x+10){
            if(bird.y<pipes[i].y || bird.y+5>pipes[i].y+gap) gameOver=true;
        }
    }

    if(bird.y<0 || bird.y+5>H) gameOver=true;
}

// --- send frame ---
void sendFrame(){
    drawFrame();
    webSocket.broadcastBIN(frame,W*H);
}

// --- Events WebSocket ---
void webSocketEvent(uint8_t num,WStype_t type,uint8_t * payload,size_t length){
    if(type==WStype_TEXT){
        if(strcmp((char*)payload,"JUMP")==0){
            if(!gameOver && !gamePass) bird.dy=jump;
            else { bird.y=50; bird.dy=0; initPipes(); score=0; gameOver=false; gamePass=false; }
        }
    }
}

// --- Page HTML ---
const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP32 Game</title>
<style>
body { margin:0; display:flex; justify-content:center; align-items:center; height:100vh; background:#000; }
canvas { image-rendering: pixelated; }
button { position:absolute; bottom:10px; left:50%; transform:translateX(-50%); padding:10px 20px; font-size:18px; background:#222;color:#fff;border:none;border-radius:5px;}
</style>
</head>
<body>
<canvas id="game" width="160" height="120"></canvas>
<button onclick="jump()">JUMP</button>
<script>
let canvas=document.getElementById('game');
let ctx=canvas.getContext('2d');
let ws=new WebSocket("ws://"+location.hostname+":81");
ws.binaryType="arraybuffer";

ws.onmessage=function(event){
    let data=new Uint8Array(event.data);
    let img=ctx.createImageData(160,120);
    for(let i=0;i<data.length;i++){
        let v=data[i];
        if(v==0){img.data[i*4+0]=135; img.data[i*4+1]=206; img.data[i*4+2]=235;} // cielo
        if(v==1){img.data[i*4+0]=255; img.data[i*4+1]=255; img.data[i*4+2]=0;}   // bird
        if(v==2){img.data[i*4+0]=0; img.data[i*4+1]=255; img.data[i*4+2]=0;}     // pipe
        if(v==3){img.data[i*4+0]=255; img.data[i*4+1]=255; img.data[i*4+2]=255;} // testo
        img.data[i*4+3]=255;
    }
    ctx.putImageData(img,0,0);
}

function jump(){ ws.send("JUMP"); }
</script>
</body>
</html>
)rawliteral";

// --- Server ---
void handleRoot(){ server.send(200,"text/html",htmlPage); }

void setup(){
    Serial.begin(115200);
    WiFi.softAP(ssid,password);
    Serial.println(WiFi.softAPIP());
    server.on("/",handleRoot);
    server.begin();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    initPipes();
}

void loop(){
    webSocket.loop();
    server.handleClient();
    updateGame();
    sendFrame();
    delay(30);
}
