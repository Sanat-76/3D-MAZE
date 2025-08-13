#define _USE_MATH_DEFINES
#include <cmath>
#include <GL/glew.h>
#include <GL/glut.h>
#include <iostream>
#include <set>
#include <chrono>
#include <vector>
#include <random>

// Maze size
const int WIDTH = 10, HEIGHT = 6;

// Float version of PI to avoid narrowing conversion warnings
const float PI_F = static_cast<float>(M_PI);

// Maze cell structure
struct Cell {
    bool north, south, east, west;
};

Cell maze[WIDTH][HEIGHT];

// Camera parameters
float cameraX = 10.0f, cameraY = 25.0f, cameraZ = 6.0f;
float cameraYaw = 0.0f;
float cameraPitch = -70.0f;
float moveSpeed = 3.0f;
float turnSpeed = 90.0f;

// Animation state
bool isTransitioning = true;
float transitionTime = 0.0f;
const float TRANSITION_DURATION = 5.0f;

// Target positions for transition
float startX = 10.0f, startY = 25.0f, startZ = 6.0f;
float startYaw = 0.0f, startPitch = -70.0f;
float endX = -4.0f, endY = 1.0f, endZ = 6.0f;
float endYaw = 0.0f, endPitch = 0.0f;

// Input state
std::set<unsigned char> pressedKeys;

// Timing
auto lastTime = std::chrono::high_resolution_clock::now();

// Night scene parameters
float moonPhase = 0.0f;
float starTwinkle = 0.0f;
float windTime = 0.0f;
float rainTime = 0.0f;
float torchFlickerTime = 0.0f;

// Rain particle system
struct RainDrop {
    float x, y, z;
    float speed;
    float life;
};

std::vector<RainDrop> rainDrops;
const int MAX_RAIN_DROPS = 300;

// Lamp post system
struct LampPost {
    float x, y, z;
    float flickerPhase;
    float lightIntensity;
};

std::vector<LampPost> lampPosts;

// Realistic grass system
struct GrassBlade {
    float x, z;
    float height;
    float bendAngle;
    float windPhase;
    int segments;
};

std::vector<GrassBlade> grassBlades;
const int MAX_GRASS_BLADES = 800;

// Random number generator
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dis(0.0f, 1.0f);

// Smooth interpolation function
float smoothstep(float t) {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

// Initialize lamp posts around maze perimeter - FIXED VERSION
void initLampPosts() {
    lampPosts.clear();
    
    // Place lamp posts strategically around the maze perimeter
    float cellSize = 2.0f;
    float mazeWidth = WIDTH * cellSize;
    float mazeHeight = HEIGHT * cellSize;
    
    // Corner lamp posts - Fixed with float PI
    lampPosts.push_back({-6.0f, 3.0f, -6.0f, dis(gen) * 2.0f * PI_F, 0.8f + dis(gen) * 0.2f});
    lampPosts.push_back({mazeWidth + 6.0f, 3.0f, -6.0f, dis(gen) * 2.0f * PI_F, 0.8f + dis(gen) * 0.2f});
    lampPosts.push_back({-6.0f, 3.0f, mazeHeight + 6.0f, dis(gen) * 2.0f * PI_F, 0.8f + dis(gen) * 0.2f});
    lampPosts.push_back({mazeWidth + 6.0f, 3.0f, mazeHeight + 6.0f, dis(gen) * 2.0f * PI_F, 0.8f + dis(gen) * 0.2f});
    
    // Side lamp posts - Fixed with float PI
    lampPosts.push_back({-6.0f, 3.0f, mazeHeight/2.0f, dis(gen) * 2.0f * PI_F, 0.8f + dis(gen) * 0.2f});
    lampPosts.push_back({mazeWidth + 6.0f, 3.0f, mazeHeight/2.0f, dis(gen) * 2.0f * PI_F, 0.8f + dis(gen) * 0.2f});
    lampPosts.push_back({mazeWidth/2.0f, 3.0f, -6.0f, dis(gen) * 2.0f * PI_F, 0.8f + dis(gen) * 0.2f});
    lampPosts.push_back({mazeWidth/2.0f, 3.0f, mazeHeight + 6.0f, dis(gen) * 2.0f * PI_F, 0.8f + dis(gen) * 0.2f});
    
    // Additional atmospheric lamp posts - Fixed with float PI
    lampPosts.push_back({-10.0f, 3.0f, 2.0f, dis(gen) * 2.0f * PI_F, 0.7f + dis(gen) * 0.3f});
    lampPosts.push_back({mazeWidth + 10.0f, 3.0f, mazeHeight - 2.0f, dis(gen) * 2.0f * PI_F, 0.7f + dis(gen) * 0.3f});
}

// Initialize realistic grass system
void initRealisticGrass() {
    grassBlades.clear();
    
    for (int i = 0; i < MAX_GRASS_BLADES; i++) {
        GrassBlade blade;
        
        // Distribute grass around maze perimeter and open areas
        float angle = dis(gen) * 2.0f * PI_F;
        float radius = 8.0f + dis(gen) * 15.0f;
        blade.x = WIDTH + cos(angle) * radius;
        blade.z = HEIGHT + sin(angle) * radius;
        
        // Skip if too close to maze
        if (blade.x > -1.0f && blade.x < WIDTH*2.0f + 1.0f && 
            blade.z > -1.0f && blade.z < HEIGHT*2.0f + 1.0f) {
            i--; // Retry this blade
            continue;
        }
        
        blade.height = 0.2f + dis(gen) * 0.6f;
        blade.bendAngle = 0.0f;
        blade.windPhase = dis(gen) * 2.0f * PI_F;
        blade.segments = 3 + (int)(dis(gen) * 4); // 3-6 segments per blade
        
        grassBlades.push_back(blade);
    }
}

// Initialize rain system
void initRain() {
    rainDrops.clear();
    for (int i = 0; i < MAX_RAIN_DROPS; i++) {
        RainDrop drop;
        drop.x = -20.0f + dis(gen) * 60.0f;
        drop.y = 15.0f + dis(gen) * 15.0f;
        drop.z = -20.0f + dis(gen) * 60.0f;
        drop.speed = 6.0f + dis(gen) * 3.0f;
        drop.life = 1.0f;
        rainDrops.push_back(drop);
    }
}

// Update rain particles
void updateRain(float deltaTime) {
    for (auto& drop : rainDrops) {
        drop.y -= drop.speed * deltaTime;
        drop.x += sin(windTime * 1.5f + drop.z * 0.05f) * 0.3f * deltaTime;
        
        if (drop.y < 0.0f || drop.x < -30.0f || drop.x > 50.0f || drop.z < -30.0f || drop.z > 50.0f) {
            drop.x = -20.0f + dis(gen) * 60.0f;
            drop.y = 15.0f + dis(gen) * 15.0f;
            drop.z = -20.0f + dis(gen) * 60.0f;
            drop.speed = 6.0f + dis(gen) * 3.0f;
            drop.life = 1.0f;
        }
        
        if (drop.y < 1.5f) {
            drop.life = drop.y / 1.5f;
        }
    }
}

// Update lamp posts
void updateLampPosts(float deltaTime) {
    for (auto& lamp : lampPosts) {
        lamp.flickerPhase += deltaTime * (2.0f + dis(gen) * 1.0f);
        // Subtle intensity variation
        lamp.lightIntensity = 0.7f + 0.3f * sin(lamp.flickerPhase);
    }
}

// Draw lamp posts with realistic design
void drawLampPosts() {
    for (const auto& lamp : lampPosts) {
        // Lamp post pole
        glColor3f(0.2f, 0.2f, 0.2f); // Dark metal
        glPushMatrix();
        glTranslatef(lamp.x, lamp.y/2.0f, lamp.z);
        glScalef(0.1f, lamp.y, 0.1f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        // Lamp base
        glColor3f(0.15f, 0.15f, 0.15f);
        glPushMatrix();
        glTranslatef(lamp.x, 0.2f, lamp.z);
        glScalef(0.3f, 0.4f, 0.3f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        // Lamp housing
        glColor3f(0.1f, 0.1f, 0.1f);
        glPushMatrix();
        glTranslatef(lamp.x, lamp.y, lamp.z);
        glScalef(0.4f, 0.3f, 0.4f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        // Lamp light source with flicker
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        float intensity = lamp.lightIntensity;
        glColor4f(1.0f, 0.9f, 0.6f, 0.8f * intensity);
        glPushMatrix();
        glTranslatef(lamp.x, lamp.y, lamp.z);
        glScalef(0.3f, 0.2f, 0.3f);
        glutSolidSphere(1.0f, 8, 8);
        glPopMatrix();
        
        // Lamp glow
        glColor4f(1.0f, 0.9f, 0.6f, 0.3f * intensity);
        glPushMatrix();
        glTranslatef(lamp.x, lamp.y, lamp.z);
        glScalef(0.8f, 0.5f, 0.8f);
        glutSolidSphere(1.0f, 8, 8);
        glPopMatrix();
        
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    }
}

// Draw realistic rain
void drawRain() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    
    for (const auto& drop : rainDrops) {
        float alpha = drop.life * 0.4f;
        glColor4f(0.6f, 0.7f, 0.9f, alpha);
        
        glVertex3f(drop.x, drop.y, drop.z);
        glVertex3f(drop.x, drop.y - 0.2f, drop.z);
    }
    
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// Draw realistic grass blades
void drawRealisticGrass() {
    for (const auto& blade : grassBlades) {
        // Calculate wind effect
        float windBend = sin(windTime * 1.3f + blade.windPhase) * 0.4f;
        float windSway = cos(windTime * 0.8f + blade.windPhase * 0.7f) * 0.2f;
        
        // Grass color variation
        float colorVar = 0.8f + 0.4f * sin(blade.x * 0.1f + blade.z * 0.1f);
        glColor3f(0.1f * colorVar, 0.3f * colorVar, 0.1f * colorVar);
        
        // Draw grass blade as connected segments
        float segmentHeight = blade.height / blade.segments;
        
        for (int seg = 0; seg < blade.segments; seg++) {
            float segmentRatio = (float)seg / blade.segments;
            float nextSegmentRatio = (float)(seg + 1) / blade.segments;
            
            // Calculate positions with wind bending
            float currentBend = windBend * segmentRatio * segmentRatio;
            float nextBend = windBend * nextSegmentRatio * nextSegmentRatio;
            
            float x1 = blade.x + currentBend + windSway * segmentRatio;
            float y1 = segmentRatio * blade.height;
            float z1 = blade.z;
            
            float x2 = blade.x + nextBend + windSway * nextSegmentRatio;
            float y2 = nextSegmentRatio * blade.height;
            float z2 = blade.z;
            
            // Draw segment as a thin line
            glLineWidth(2.0f);
            glBegin(GL_LINES);
            glVertex3f(x1, y1, z1);
            glVertex3f(x2, y2, z2);
            glEnd();
            
            // Add width with small quads for thicker appearance
            if (seg < blade.segments - 1) {
                float width = 0.02f * (1.0f - segmentRatio * 0.5f);
                
                glBegin(GL_QUADS);
                glVertex3f(x1 - width, y1, z1);
                glVertex3f(x1 + width, y1, z1);
                glVertex3f(x2 + width, y2, z2);
                glVertex3f(x2 - width, y2, z2);
                glEnd();
            }
        }
    }
}

// Setup enhanced realistic lighting system
void setupRealisticLighting() {
    if (isTransitioning) {
        // During transition: moderate moonlight for overview
        glEnable(GL_LIGHT0);
        GLfloat moonlight_pos[] = {30.0f, 35.0f, -40.0f, 1.0f};
        GLfloat moonlight_ambient[] = {0.2f, 0.2f, 0.25f, 1.0f};
        GLfloat moonlight_diffuse[] = {0.4f, 0.4f, 0.5f, 1.0f};
        GLfloat moonlight_specular[] = {0.3f, 0.3f, 0.4f, 1.0f};
        
        glLightfv(GL_LIGHT0, GL_POSITION, moonlight_pos);
        glLightfv(GL_LIGHT0, GL_AMBIENT, moonlight_ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, moonlight_diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, moonlight_specular);
        
        // Disable other lights during transition
        glDisable(GL_LIGHT1);
        glDisable(GL_LIGHT2);
        glDisable(GL_LIGHT3);
        
        GLfloat global_ambient[] = {0.15f, 0.15f, 0.2f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
    } else {
        // After transition: very dim moonlight + lamp posts + player torch
        
        // Very dim moonlight for subtle outdoor illumination
        glEnable(GL_LIGHT0);
        GLfloat moonlight_pos[] = {30.0f, 35.0f, -40.0f, 1.0f};
        GLfloat moonlight_ambient[] = {0.05f, 0.05f, 0.08f, 1.0f};
        GLfloat moonlight_diffuse[] = {0.1f, 0.1f, 0.15f, 1.0f};
        GLfloat moonlight_specular[] = {0.05f, 0.05f, 0.1f, 1.0f};
        
        glLightfv(GL_LIGHT0, GL_POSITION, moonlight_pos);
        glLightfv(GL_LIGHT0, GL_AMBIENT, moonlight_ambient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, moonlight_diffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, moonlight_specular);
        
        // Player's enhanced realistic torch light
        glEnable(GL_LIGHT1);
        
        // Calculate torch direction and position more realistically
        float rad = cameraYaw * PI_F / 180.0f;
        float pitchRad = cameraPitch * PI_F / 180.0f;
        
        float torchDirX = cos(rad) * cos(pitchRad);
        float torchDirY = sin(pitchRad) - 0.1f; // Slight downward angle
        float torchDirZ = sin(rad) * cos(pitchRad);
        
        // Position torch more realistically - slightly forward and to the side
        float torchX = cameraX + cos(rad - 0.2f) * 0.4f;
        float torchY = cameraY - 0.3f;
        float torchZ = cameraZ + sin(rad - 0.2f) * 0.4f;
        
        // Enhanced torch flicker
        float torchFlicker = 0.85f + 0.15f * sin(torchFlickerTime * 10.0f) * cos(torchFlickerTime * 7.3f);
        
        GLfloat torch_pos[] = {torchX, torchY, torchZ, 1.0f};
        GLfloat torch_dir[] = {torchDirX, torchDirY, torchDirZ};
        GLfloat torch_ambient[] = {0.15f * torchFlicker, 0.08f * torchFlicker, 0.02f * torchFlicker, 1.0f};
        GLfloat torch_diffuse[] = {1.2f * torchFlicker, 0.8f * torchFlicker, 0.4f * torchFlicker, 1.0f};
        GLfloat torch_specular[] = {1.0f * torchFlicker, 0.7f * torchFlicker, 0.3f * torchFlicker, 1.0f};
        
        glLightfv(GL_LIGHT1, GL_POSITION, torch_pos);
        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, torch_dir);
        glLightfv(GL_LIGHT1, GL_AMBIENT, torch_ambient);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, torch_diffuse);
        glLightfv(GL_LIGHT1, GL_SPECULAR, torch_specular);
        
        // Enhanced torch spotlight properties for more realistic beam
        glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 35.0f); // Narrower beam
        glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 3.0f); // More focused center
        glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
        glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.6f); // Faster falloff
        glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.3f);
        
        // Lamp post lighting (use multiple lights for different lamp posts)
        if (lampPosts.size() > 0) {
            glEnable(GL_LIGHT2);
            float lampIntensity = lampPosts[0].lightIntensity;
            
            GLfloat lamp_pos[] = {lampPosts[0].x, lampPosts[0].y, lampPosts[0].z, 1.0f};
            GLfloat lamp_ambient[] = {0.1f * lampIntensity, 0.08f * lampIntensity, 0.05f * lampIntensity, 1.0f};
            GLfloat lamp_diffuse[] = {0.6f * lampIntensity, 0.5f * lampIntensity, 0.3f * lampIntensity, 1.0f};
            GLfloat lamp_specular[] = {0.4f * lampIntensity, 0.3f * lampIntensity, 0.2f * lampIntensity, 1.0f};
            
            glLightfv(GL_LIGHT2, GL_POSITION, lamp_pos);
            glLightfv(GL_LIGHT2, GL_AMBIENT, lamp_ambient);
            glLightfv(GL_LIGHT2, GL_DIFFUSE, lamp_diffuse);
            glLightfv(GL_LIGHT2, GL_SPECULAR, lamp_specular);
            
            // Lamp post attenuation
            glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 1.0f);
            glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.2f);
            glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.05f);
        }
        
        if (lampPosts.size() > 1) {
            glEnable(GL_LIGHT3);
            float lampIntensity = lampPosts[1].lightIntensity;
            
            GLfloat lamp_pos[] = {lampPosts[1].x, lampPosts[1].y, lampPosts[1].z, 1.0f};
            GLfloat lamp_ambient[] = {0.1f * lampIntensity, 0.08f * lampIntensity, 0.05f * lampIntensity, 1.0f};
            GLfloat lamp_diffuse[] = {0.6f * lampIntensity, 0.5f * lampIntensity, 0.3f * lampIntensity, 1.0f};
            GLfloat lamp_specular[] = {0.4f * lampIntensity, 0.3f * lampIntensity, 0.2f * lampIntensity, 1.0f};
            
            glLightfv(GL_LIGHT3, GL_POSITION, lamp_pos);
            glLightfv(GL_LIGHT3, GL_AMBIENT, lamp_ambient);
            glLightfv(GL_LIGHT3, GL_DIFFUSE, lamp_diffuse);
            glLightfv(GL_LIGHT3, GL_SPECULAR, lamp_specular);
            
            // Lamp post attenuation
            glLightf(GL_LIGHT3, GL_CONSTANT_ATTENUATION, 1.0f);
            glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, 0.2f);
            glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, 0.05f);
        }
        
        // Very dark global ambient for realistic cave-like maze interior
        GLfloat global_ambient[] = {0.02f, 0.02f, 0.03f, 1.0f};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
    }
}

// Draw realistic night sky with dimmed moon
void drawNightSky() {
    glDisable(GL_LIGHTING);
    
    // Darker night sky gradient
    glBegin(GL_QUADS);
    glColor3f(0.01f, 0.01f, 0.08f);
    glVertex3f(-100.0f, 50.0f, -100.0f);
    glVertex3f(100.0f, 50.0f, -100.0f);
    glVertex3f(100.0f, 50.0f, 100.0f);
    glVertex3f(-100.0f, 50.0f, 100.0f);
    
    glColor3f(0.02f, 0.02f, 0.1f);
    glVertex3f(-100.0f, 0.0f, -100.0f);
    glVertex3f(100.0f, 0.0f, -100.0f);
    glVertex3f(100.0f, 0.0f, 100.0f);
    glVertex3f(-100.0f, 0.0f, 100.0f);
    glEnd();
    
    // Dimmer twinkling stars
    glPointSize(1.5f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 200; i++) {
        float x = -80.0f + (i * 13.7f) - (int)(i * 13.7f / 160.0f) * 160.0f;
        float z = -80.0f + (i * 17.3f) - (int)(i * 17.3f / 160.0f) * 160.0f;
        float y = 20.0f + (i % 20) * 1.0f;
        
        float brightness = 0.3f + 0.3f * sin(starTwinkle + i * 0.3f);
        glColor3f(brightness, brightness, brightness * 1.1f);
        glVertex3f(x, y, z);
    }
    glEnd();
    
    glEnable(GL_LIGHTING);
}

// Draw dimmed moon
void drawMoon() {
    glDisable(GL_LIGHTING);
    
    float moonX = 30.0f;
    float moonY = 35.0f;
    float moonZ = -40.0f;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Dimmed moon glow
    glColor4f(0.6f, 0.6f, 0.5f, 0.1f);
    glPushMatrix();
    glTranslatef(moonX, moonY, moonZ);
    glutSolidSphere(3.0f, 20, 20);
    glPopMatrix();
    
    // Dimmed main moon body
    glColor4f(0.7f, 0.7f, 0.6f, 0.8f);
    glPushMatrix();
    glTranslatef(moonX, moonY, moonZ);
    glutSolidSphere(1.8f, 25, 25);
    glPopMatrix();
    
    // Dimmed moon craters
    glColor4f(0.5f, 0.5f, 0.4f, 0.8f);
    glPushMatrix();
    glTranslatef(moonX - 0.3f, moonY + 0.2f, moonZ + 1.7f);
    glutSolidSphere(0.25f, 12, 12);
    glPopMatrix();
    
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

// Draw a proper cube wall at specific position
void drawWallCube(float x, float y, float z, float width, float height, float depth) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(width, height, depth);
    glutSolidCube(1.0f);
    glPopMatrix();
}

// Draw walls using properly positioned cubes
void drawMazeWalls() {
    glColor3f(0.3f, 0.25f, 0.2f); // Darker stone for night
    float wallHeight = 2.0f;
    float wallThickness = 0.1f;
    float cellSize = 2.0f;
    
    for (int i = 0; i < WIDTH; ++i) {
        for (int j = 0; j < HEIGHT; ++j) {
            float cellX = i * cellSize;
            float cellZ = j * cellSize;
            
            if (maze[i][j].north) {
                drawWallCube(cellX + cellSize/2.0f, wallHeight/2.0f, cellZ,
                           cellSize + wallThickness, wallHeight, wallThickness);
            }
            
            if (maze[i][j].south) {
                drawWallCube(cellX + cellSize/2.0f, wallHeight/2.0f, cellZ + cellSize,
                           cellSize + wallThickness, wallHeight, wallThickness);
            }
            
            if (maze[i][j].west) {
                drawWallCube(cellX, wallHeight/2.0f, cellZ + cellSize/2.0f,
                           wallThickness, wallHeight, cellSize + wallThickness);
            }
            
            if (maze[i][j].east) {
                drawWallCube(cellX + cellSize, wallHeight/2.0f, cellZ + cellSize/2.0f,
                           wallThickness, wallHeight, cellSize + wallThickness);
            }
        }
    }
}

// Draw corner posts
void drawCornerPosts() {
    glColor3f(0.25f, 0.2f, 0.15f);
    float wallHeight = 2.0f;
    float postSize = 0.1f;
    float cellSize = 2.0f;
    
    for (int i = 0; i <= WIDTH; ++i) {
        for (int j = 0; j <= HEIGHT; ++j) {
            float x = i * cellSize;
            float z = j * cellSize;
            drawWallCube(x, wallHeight/2.0f, z, postSize, wallHeight, postSize);
        }
    }
}

// Draw entrance
void drawEntrance() {
    glColor3f(0.15f, 0.4f, 0.15f);
    glPushMatrix();
    glTranslatef(-0.3f, 0.1f, 6.0f);
    glScalef(0.2f, 0.2f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    glColor3f(0.2f, 0.15f, 0.1f);
    glPushMatrix();
    glTranslatef(-0.1f, 1.0f, 6.0f);
    glScalef(0.2f, 2.0f, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-0.1f, 1.0f, 5.0f);
    glScalef(0.2f, 2.0f, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-0.1f, 1.0f, 7.0f);
    glScalef(0.2f, 2.0f, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-0.1f, 2.0f, 6.0f);
    glScalef(0.2f, 0.1f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();
}

// Render the complete realistic dark maze
void drawMaze() {
    drawNightSky();
    drawMoon();
    
    // Darker ground
    glColor3f(0.04f, 0.1f, 0.04f);
    glBegin(GL_QUADS);
    glVertex3f(-50.0f, 0.0f, -50.0f);
    glVertex3f(WIDTH*2.0f+50.0f, 0.0f, -50.0f);
    glVertex3f(WIDTH*2.0f+50.0f, 0.0f, HEIGHT*2.0f+50.0f);
    glVertex3f(-50.0f, 0.0f, HEIGHT*2.0f+50.0f);
    glEnd();
    
    // Maze floor - very dark
    glColor3f(0.06f, 0.12f, 0.06f);
    glBegin(GL_QUADS);
    glVertex3f(0.0f, 0.005f, 0.0f);
    glVertex3f(WIDTH*2.0f, 0.005f, 0.0f);
    glVertex3f(WIDTH*2.0f, 0.005f, HEIGHT*2.0f);
    glVertex3f(0.0f, 0.005f, HEIGHT*2.0f);
    glEnd();
    
    // Draw lamp posts
    drawLampPosts();
    
    // Draw realistic grass
    drawRealisticGrass();
    
    // Draw walls and posts
    drawMazeWalls();
    drawCornerPosts();
    
    // Draw entrance
    drawEntrance();
    
    // Draw exit marker
    glColor3f(0.4f, 0.1f, 0.1f);
    glPushMatrix();
    glTranslatef(WIDTH*2.0f + 0.3f, 0.1f, (HEIGHT-1)*2.0f + 1.0f);
    glScalef(0.2f, 0.2f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();
    
    // Draw rain
    drawRain();
    
    // Add distant trees
    glColor3f(0.03f, 0.08f, 0.03f);
    for (int i = 0; i < 8; ++i) {
        float angle = i * 45.0f * PI_F / 180.0f;
        float radius = 25.0f + (i % 2) * 8.0f;
        float x = WIDTH + cos(angle) * radius;
        float z = HEIGHT + sin(angle) * radius;
        
        float treeSway = sin(windTime * 0.6f + i * 0.8f) * 0.2f;
        
        glPushMatrix();
        glTranslatef(x + treeSway, 3.0f, z);
        glRotatef(treeSway * 8.0f, 0.0f, 0.0f, 1.0f);
        glScalef(0.6f, 6.0f, 0.6f);
        glutSolidCube(1.0f);
        glPopMatrix();
        
        glPushMatrix();
        glTranslatef(x + treeSway * 1.2f, 5.5f, z);
        glRotatef(treeSway * 12.0f, 0.0f, 0.0f, 1.0f);
        glScalef(2.5f, 2.5f, 2.5f);
        glutSolidSphere(1.0f, 8, 8);
        glPopMatrix();
    }
}

// Update movement
void updateMovement(float deltaTime) {
    if (isTransitioning) return;
    
    float rad = cameraYaw * PI_F / 180.0f;
    float dirX = cos(rad), dirZ = sin(rad);
    float moveDistance = moveSpeed * deltaTime;
    float turnAmount = turnSpeed * deltaTime;
    
    if (pressedKeys.count('w') || pressedKeys.count('W')) {
        cameraX += moveDistance * dirX;
        cameraZ += moveDistance * dirZ;
    }
    
    if (pressedKeys.count('s') || pressedKeys.count('S')) {
        cameraX -= moveDistance * dirX;
        cameraZ -= moveDistance * dirZ;
    }
    
    if (pressedKeys.count('a') || pressedKeys.count('A')) {
        cameraX += moveDistance * dirZ;
        cameraZ -= moveDistance * dirX;
    }
    
    if (pressedKeys.count('d') || pressedKeys.count('D')) {
        cameraX -= moveDistance * dirZ;
        cameraZ += moveDistance * dirX;
    }
    
    if (pressedKeys.count('q') || pressedKeys.count('Q')) {
        cameraYaw -= turnAmount;
    }
    
    if (pressedKeys.count('e') || pressedKeys.count('E')) {
        cameraYaw += turnAmount;
    }
    
    if (pressedKeys.count('r') || pressedKeys.count('R')) {
        cameraX = -4.0f;
        cameraY = 1.0f;
        cameraZ = 6.0f;
        cameraYaw = 0.0f;
        cameraPitch = 0.0f;
        printf("Reset to entrance position\n");
    }
    
    while (cameraYaw < 0) cameraYaw += 360.0f;
    while (cameraYaw >= 360.0f) cameraYaw -= 360.0f;
}

// Update transition animation
void updateTransition(float deltaTime) {
    if (!isTransitioning) return;
    
    transitionTime += deltaTime;
    float t = transitionTime / TRANSITION_DURATION;
    
    if (t >= 1.0f) {
        isTransitioning = false;
        cameraX = endX;
        cameraY = endY;
        cameraZ = endZ;
        cameraYaw = endYaw;
        cameraPitch = endPitch;
        printf("Transition complete! The maze is now completely dark.\n");
        printf("Use your torch to navigate - lamp posts light the surrounding area.\n");
    } else {
        float smoothT = smoothstep(t);
        cameraX = startX + (endX - startX) * smoothT;
        cameraY = startY + (endY - startY) * smoothT;
        cameraZ = startZ + (endZ - startZ) * smoothT;
        cameraYaw = startYaw + (endYaw - startYaw) * smoothT;
        cameraPitch = startPitch + (endPitch - startPitch) * smoothT;
    }
}

// Camera setup
void updateCamera() {
    float rad = cameraYaw * PI_F / 180.0f;
    float pitchRad = cameraPitch * PI_F / 180.0f;
    
    float dirX = cos(rad) * cos(pitchRad);
    float dirY = sin(pitchRad);
    float dirZ = sin(rad) * cos(pitchRad);
    
    gluLookAt(cameraX, cameraY, cameraZ,
              cameraX + dirX, cameraY + dirY, cameraZ + dirZ,
              0.0f, 1.0f, 0.0f);
}

// GLUT display callback
void display() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastTime);
    float deltaTime = duration.count() / 1000000.0f;
    lastTime = currentTime;
    
    // Update animations
    moonPhase += deltaTime * 0.1f;
    starTwinkle += deltaTime * 1.5f;
    windTime += deltaTime;
    rainTime += deltaTime;
    torchFlickerTime += deltaTime;
    
    updateTransition(deltaTime);
    updateMovement(deltaTime);
    updateRain(deltaTime);
    updateLampPosts(deltaTime);
    
    // Setup realistic lighting
    setupRealisticLighting();
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    updateCamera();
    drawMaze();
    
    glutSwapBuffers();
}

// Keyboard functions
void keyboardDown(unsigned char key, int x, int y) {
    pressedKeys.insert(key);
    if (key == 27) exit(0);
    
    if (key == ' ' && isTransitioning) {
        isTransitioning = false;
        cameraX = endX;
        cameraY = endY;
        cameraZ = endZ;
        cameraYaw = endYaw;
        cameraPitch = endPitch;
        printf("Transition skipped! The maze is now completely dark.\n");
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    pressedKeys.erase(key);
}

void idle() {
    glutPostRedisplay();
}

// Initialize maze
void initMaze() {
    for (int i = 0; i < WIDTH; ++i)
        for (int j = 0; j < HEIGHT; ++j)
            maze[i][j] = {true, true, true, true};
    
    maze[0][3].west = false;
    maze[WIDTH-1][HEIGHT-1].east = false;
    
    // Main solution path
    maze[0][3].east = false; maze[1][3].west = false;
    maze[1][3].east = false; maze[2][3].west = false;
    maze[2][3].east = false; maze[3][3].west = false;
    maze[3][3].east = false; maze[4][3].west = false;
    maze[4][3].east = false; maze[5][3].west = false;
    maze[5][3].east = false; maze[6][3].west = false;
    maze[6][3].east = false; maze[7][3].west = false;
    
    maze[7][3].north = false; maze[7][2].south = false;
    maze[7][2].north = false; maze[7][1].south = false;
    
    maze[7][1].east = false; maze[8][1].west = false;
    maze[8][1].east = false; maze[9][1].west = false;
    
    maze[9][1].south = false; maze[9][2].north = false;
    maze[9][2].south = false; maze[9][3].north = false;
    maze[9][3].south = false; maze[9][4].north = false;
    maze[9][4].south = false; maze[9][5].north = false;
    
    // Add dead ends and alternate paths
    maze[0][3].south = false; maze[0][4].north = false;
    maze[0][4].south = false; maze[0][5].north = false;
    maze[0][5].east = false; maze[1][5].west = false;
    
    maze[2][3].south = false; maze[2][4].north = false;
    maze[2][4].south = false; maze[2][5].north = false;
    
    maze[4][3].south = false; maze[4][4].north = false;
    maze[4][4].south = false; maze[4][5].north = false;
    maze[4][5].east = false; maze[5][5].west = false;
    maze[5][5].east = false; maze[6][5].west = false;
    maze[6][5].east = false; maze[7][5].west = false;
    maze[7][5].east = false; maze[8][5].west = false;
    maze[8][5].east = false; maze[9][5].west = false;
    
    // Top row connections
    maze[0][0].east = false; maze[1][0].west = false;
    maze[1][0].east = false; maze[2][0].west = false;
    maze[2][0].east = false; maze[3][0].west = false;
    maze[3][0].south = false; maze[3][1].north = false;
    maze[3][1].south = false; maze[3][2].north = false;
    maze[3][2].east = false; maze[4][2].west = false;
    maze[4][2].east = false; maze[5][2].west = false;
    maze[5][2].east = false; maze[6][2].west = false;
    maze[6][2].north = false; maze[6][1].south = false;
    maze[6][1].north = false; maze[6][0].south = false;
    maze[6][0].east = false; maze[7][0].west = false;
    maze[7][0].east = false; maze[8][0].west = false;
    maze[8][0].east = false; maze[9][0].west = false;
    maze[9][0].south = false; maze[9][1].north = false;
}

// Window resize
void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)width / (double)height, 0.1, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

// OpenGL/GLUT setup
void initGL() {
    if (glewInit() != GLEW_OK) {
        printf("Error: Failed to initialize GLEW\n");
        exit(1);
    }
    
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.01f, 0.01f, 0.05f, 1.0f);
    
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    printf("OpenGL initialized successfully!\n");
    printf("GLEW version: %s\n", glewGetString(GLEW_VERSION));
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(900, 700);
    glutCreateWindow("3D Dark Maze - Realistic Torch Navigation");
    
    initGL();
    initMaze();
    initRain();
    initRealisticGrass();
    initLampPosts();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutIdleFunc(idle);
    
    printf("\t \t \t 3D Dark Maze \n");
    printf("Experience true darkness with realistic torch-only navigation!\n");
    printf("Lamp posts illuminate the surrounding area, but the maze is pitch black...\n\n");
    printf("Controls (after transition):\n");
    printf("WASD - Navigate through complete darkness\n");
    printf("Q/E - Turn left/right (torch follows your view)\n");
    printf("R - Reset to entrance\n");
    printf("SPACE - Skip transition\n");
    printf("ESC - Exit\n\n");
    // printf(" Dark Maze Features:\n");
    // printf("- Dimmed moonlight for subtle atmosphere\n");
    // printf("- Strategic lamp posts around maze perimeter\n");
    // printf("- Completely dark maze interior\n");
    // printf("- Enhanced realistic torch with flicker effects\n");
    // printf("- Focused torch beam for precise navigation\n");
    // printf("- Atmospheric rain and wind effects\n\n");
    // printf("Starting descent into darkness...\n");
    
    glutMainLoop();
    return 0;
}
