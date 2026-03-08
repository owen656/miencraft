#include <iostream>
#include <cmath>

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3_image/SDL_image.h>
#include <list>
#include <unordered_map>

float playerX=0;
float playerY=0;
float playerZ=3;

float yaw=0;
float pitch=0;

using namespace std;

list<tuple<string, string, string>> TextureAtlas = {
    tuple<string, string, string>("grass_top.png", "grass.png", "dirt.png"),
    tuple<string, string, string>("dirt.png","dirt.png","dirt.png")
};
unordered_map<std::string, int> KeyMapper = {
    {"grass" , 1},
    {"dirt", 2}
};

GLuint LoadTexture(const char* file)
{
    SDL_Surface* raw = IMG_Load(file);
    if (!raw) {
        std::cout << "Failed to load " << file << ": " << SDL_GetError() << std::endl;
        return 0;
    }

    // Convert to RGBA32 → OpenGL expects this byte order reliably
    SDL_Surface* surface = SDL_ConvertSurface(raw, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(raw);
    if (!surface) {
        std::cout << "Conversion failed for " << file << ": " << SDL_GetError() << std::endl;
        return 0;
    }

    // Optional: print to confirm
    std::cout << "Loaded " << file << " → format: " << SDL_GetPixelFormatName(surface->format)
              << " (" << surface->w << "x" << surface->h << ")" << std::endl;

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,                // internal format
        surface->w,
        surface->h,
        0,
        GL_RGBA,                // data format — matches RGBA32 bytes
        GL_UNSIGNED_BYTE,
        surface->pixels
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    SDL_DestroySurface(surface);

    return texture;
}

void InitOpenGL(int width, int height)
{
    glViewport(0,0,width,height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspect = (float)width/(float)height;
    glFrustum(-aspect, aspect, -1, 1, 0.1, 100);   // or even 0.05

    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
}

void DrawCube(GLuint top, GLuint side, GLuint bottom, float x,float y,float z)
{
    glPushMatrix();
    glTranslatef(x,y,z);

    glBegin(GL_QUADS);

    glBindTexture(GL_TEXTURE_2D,side);

    // Front
    glTexCoord2f(0,1); glVertex3f(-0.5,-0.5, 0.5);
    glTexCoord2f(1,1); glVertex3f( 0.5,-0.5, 0.5);
    glTexCoord2f(1,0); glVertex3f( 0.5, 0.5, 0.5);
    glTexCoord2f(0,0); glVertex3f(-0.5, 0.5, 0.5);

    // Back
    glTexCoord2f(0,1); glVertex3f( 0.5,-0.5,-0.5);
    glTexCoord2f(1,1); glVertex3f(-0.5,-0.5,-0.5);
    glTexCoord2f(1,0); glVertex3f(-0.5, 0.5,-0.5);
    glTexCoord2f(0,0); glVertex3f( 0.5, 0.5,-0.5);

    // Left
    glTexCoord2f(0,1); glVertex3f(-0.5,-0.5,-0.5);
    glTexCoord2f(1,1); glVertex3f(-0.5,-0.5, 0.5);
    glTexCoord2f(1,0); glVertex3f(-0.5, 0.5, 0.5);
    glTexCoord2f(0,0); glVertex3f(-0.5, 0.5,-0.5);

    // Right
    glTexCoord2f(0,1); glVertex3f(0.5,-0.5, 0.5);
    glTexCoord2f(1,1); glVertex3f(0.5,-0.5,-0.5);
    glTexCoord2f(1,0); glVertex3f(0.5, 0.5,-0.5);
    glTexCoord2f(0,0); glVertex3f(0.5, 0.5, 0.5);

    glEnd();

    glBindTexture(GL_TEXTURE_2D,top);

    glBegin(GL_QUADS);

    // Top
    glTexCoord2f(0,1); glVertex3f(-0.5,0.5, 0.5);
    glTexCoord2f(1,1); glVertex3f( 0.5,0.5, 0.5);
    glTexCoord2f(1,0); glVertex3f( 0.5,0.5,-0.5);
    glTexCoord2f(0,0); glVertex3f(-0.5,0.5,-0.5);

    glEnd();

    glBindTexture(GL_TEXTURE_2D,bottom);

    glBegin(GL_QUADS);

    // Bottom
    glTexCoord2f(0,1); glVertex3f(-0.5,-0.5,-0.5);
    glTexCoord2f(1,1); glVertex3f( 0.5,-0.5,-0.5);
    glTexCoord2f(1,0); glVertex3f( 0.5,-0.5, 0.5);
    glTexCoord2f(0,0); glVertex3f(-0.5,-0.5, 0.5);

    glEnd();

    glPopMatrix();
}

std::tuple<string, string, string> Find_tuple(string Index_name) {
    int indexVal = 1;
    int Index_value = KeyMapper[Index_name];
    for (const auto& index_tuple : TextureAtlas){
        if (indexVal == Index_value){
            return(index_tuple);
        }
        indexVal++;
    }
}

void RenderCube(float x, float y, float z, std::string type){
        glRotatef(-pitch,1,0,0);
        glRotatef(-yaw,0,1,0);
        glTranslatef(-playerX,-playerY,-playerZ);

        tuple<string, string, string> typetuple = Find_tuple(type);

        string top_ = get<0>(typetuple);
        string side_ = get<1>(typetuple);
        string bottom_ = get<2>(typetuple);
        GLuint Top = LoadTexture(top_.c_str());
        GLuint Side = LoadTexture(side_.c_str());
        GLuint Bottom = LoadTexture(bottom_.c_str());

        DrawCube(Top,Side,Bottom,0,0,0);
}

int main(int argc,char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "Minecraft",
        1280,
        720,
        SDL_WINDOW_OPENGL
    );

    SDL_SetWindowRelativeMouseMode(window, true);

    SDL_GLContext context = SDL_GL_CreateContext(window);

    InitOpenGL(1280,720);

    GLuint grassTop = LoadTexture("grass_top.png");
    GLuint grassSide = LoadTexture("grass.png");
    GLuint dirt = LoadTexture("dirt.png");

    playerX=0;
    playerY=0;
    playerZ=0;

    yaw=0;
    pitch=0;

    float speed=10.0f;
    float damper = 0.4f;

    Uint64 NOW = SDL_GetPerformanceCounter();
    Uint64 LAST = 0;
    double deltaTime = 0; // Stored in seconds

    bool running=true;
    SDL_Event event;

    while(running)
    {
        LAST = NOW;
        NOW = SDL_GetPerformanceCounter();
        deltaTime = (double)((NOW - LAST) / (double)SDL_GetPerformanceFrequency());
        while(SDL_PollEvent(&event))
        {
            if(event.type==SDL_EVENT_QUIT)
                running=false;

            if(event.type==SDL_EVENT_KEY_DOWN)
            {
                if (event.key.key==SDLK_ESCAPE){
                    SDL_SetWindowRelativeMouseMode(window, false);
                }
            }
            if (event.type == SDL_EVENT_MOUSE_MOTION) {
                yaw -= event.motion.xrel * damper;
                pitch -= event.motion.yrel * damper;
                if (pitch > 89.0f)  pitch = 89.0f;
                if (pitch < -89.0f) pitch = -89.0f;
            }
        }

        const bool* state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_W]) {
            playerX -= sin(yaw * M_PI / 180.0) * speed * deltaTime;
            playerZ -= cos(yaw * M_PI / 180.0) * speed * deltaTime;   // ← changed - to +
        }
        if (state[SDL_SCANCODE_SPACE]){
            playerY += speed*deltaTime;
        }
        if (state[SDL_SCANCODE_LSHIFT]){
            playerY -= speed*deltaTime;
        }
        if (state[SDL_SCANCODE_S]) {
            playerX += sin(yaw * M_PI / 180.0) * speed * deltaTime;
            playerZ += cos(yaw * M_PI / 180.0) * speed * deltaTime;   // ← changed + to -
        }
        if (state[SDL_SCANCODE_A]) {
            playerX-=cos(yaw*M_PI/180)*speed*deltaTime;
            playerZ-=sin(yaw*M_PI/180)*speed*deltaTime;
        }
        if (state[SDL_SCANCODE_D]) {
            playerX+=cos(yaw*M_PI/180)*speed*deltaTime;
            playerZ+=sin(yaw*M_PI/180)*speed*deltaTime;
        }

        glClearColor(0.5f,0.7f,1.0f,1);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        RenderCube(0.0f, 0.0f, 0.0f, "grass");
        RenderCube(50.0f,0.0f,0.0f,"dirt");

        SDL_GL_SwapWindow(window);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}