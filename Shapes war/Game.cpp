#include "Game.h"
#include <iostream>
#include <fstream>
#include <cstdlib>  // For rand() and srand()
#include <ctime>    // For time()
#include <sstream>


Game::Game(const std:: string& config)
{
    init(config);
}
void Game:: init(const std::string& path){

    unsigned int  W, H;
    int FL;
    bool FS;  

    std::ifstream file(path);
    std::string line, first;
    while(std::getline(file, line)){  
        std::stringstream ss(line);     //reads through each string on each line  
    
        // passes each string to first
        ss >> first;

        if (first == "Window"){
            ss >> W >> H >> FL >> FS;
        }
        
        
        if (first == "Player"){     // holds Player
            ss >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR
            >> m_playerConfig.FG >> m_playerConfig.FB >> m_playerConfig.OR >> m_playerConfig.OG
            >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V;
        }
        
        if (first == "Enemy"){   // holds Enemy
            ss >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX 
            >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >> m_enemyConfig.OT 
            >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
        }
        
        if (first == "Bullet"){     // holds Bullet
        ss >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S 
            >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB 
            >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB 
            >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
        }
    }
    file.close();
     
 

    // set up default window parameters;

    sf::State state = FS ? sf::State::Fullscreen : sf::State::Windowed;

    m_window.create(sf::VideoMode({W, H}), "Assignment 2", state);
    m_window.setFramerateLimit(FL);

    spawnPlayer();
}

void Game::run(){
    
    srand(time(0)); // Seed with current time

    while(m_running){

        m_entities.update();

        if(!m_paused){
            sEnemySpawner();
            sMovement();
            sCollision();
            sLifespan();
        }
        sRender();
        sUserInput();

        // increement the current frame
        // may need to be moved
        m_currentFrame++;
    }
}

void Game::setPaused(bool paused){
    m_paused = paused;
}

void Game::spawnPlayer(){
    
    // we create every entity by calling EntityManager.addEntity(tag)
    // this returns a std::shared_ptr<Entity>, so we use 'auto' to save typing
    auto entity = m_entities.addEntity("Player");

    // respawn the player in the middle of the screen
    float mx = m_window.getSize().x / 2.0f;
    float my = m_window.getSize().y / 2.0f;

    entity->cTransform = std::make_shared<CTransform>(Vec2(mx, my), Vec2(0.0f, 0.0f), 0.0f);

    // the entity's shape will have radius 32, 8 side, dark grey, fill, and red outline of thickness 4
    entity->cShape = std::make_shared<CShape>(m_playerConfig.SR, m_playerConfig.V, 
                     sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB), 
                     sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), m_playerConfig.OT);

    // add an input component to the player so that we can use inputs
    entity->cInput = std:: make_shared<CInput>();

    // set players collision radius
    entity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);

    // set our Game's player variable to be this Entity
    m_player = entity;
}

// spawn an enemy at a random position
void Game:: spawnEnemy(){

    auto entity = m_entities.addEntity("Enemy"); //tag

    // random position inside the bounds of the window
    float ex = rand() % (m_window.getSize().x - 2 * m_enemyConfig.SR) + m_enemyConfig.SR;
    float ey = rand() % (m_window.getSize().y - 2 * m_enemyConfig.SR) + m_enemyConfig.SR;

    // random speed, range specified in config.txt
    float sx = m_enemyConfig.SMIN + static_cast<float>(rand()) / RAND_MAX * (m_enemyConfig.SMAX - m_enemyConfig.SMIN);
    float sy = m_enemyConfig.SMIN + static_cast<float>(rand()) / RAND_MAX * (m_enemyConfig.SMAX - m_enemyConfig.SMIN);

    // random number of vertices (from 3 - 8)
    int ev = m_enemyConfig.VMIN + (rand() % (1 + m_enemyConfig.VMAX - m_enemyConfig.VMIN));

    // random RGB
    float R = rand() % 256;
    float G = rand() % 256;
    float B = rand() % 256;


    // set entity's postion, speed, and angle 
    entity->cTransform = std::make_shared<CTransform>(Vec2(ex, ey), Vec2(sx, sy), 0.0f);

    // set enemy entity config
    entity->cShape = std::make_shared<CShape> (m_enemyConfig.SR, ev, sf::Color (R, G, B),
    sf::Color (m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB), m_enemyConfig.OT);

    entity->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);

    entity->cScore = std::make_shared<CScore>(ev * 100);

    // record when the most recent enemy was spawned
    m_lastEnemySpawnTime = m_currentFrame;
}

// spawns the small enemies when a big one (input entity e) explodes
void Game::spawnSmallEnemies(std::shared_ptr<Entity> e)
{
    int numVertices = e->cShape->circle.getPointCount();
    float originalRadius = e->cShape->circle.getRadius();

    for (int i = 0; i < numVertices; ++i)
    {
        auto small = m_entities.addEntity("Small");

        // Slightly different direction per small enemy
        float angle = i * (2 * M_PI / numVertices);
        Vec2 direction(std::cos(angle), std::sin(angle));
        Vec2 velocity = direction * m_enemyConfig.SMAX;  

        small->cTransform = std::make_shared<CTransform>(
            e->cTransform->pos,
            velocity,
            0.0f
        );

        small->cShape = std::make_shared<CShape>(
            originalRadius / 2.0f,
            numVertices,
            e->cShape->circle.getFillColor(),
            e->cShape->circle.getOutlineColor(),
            e->cShape->circle.getOutlineThickness()
        );

        small->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);
        small->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR / 2.0f);
        small->cScore = std::make_shared<CScore>(numVertices * 200); // double points
    }
}


// spawns bullet from a given entity to a target location
void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2& target){

    auto b = m_entities.addEntity("Bullet");

    // set bullet config
    b->cShape = std::make_shared<CShape>(m_bulletConfig.SR, m_bulletConfig.V, 
                    sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB), 
                    sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB), m_bulletConfig.OT);

    Vec2 diff(target - entity->cTransform->pos);
    diff.normalize();
    Vec2 velocity = diff * m_bulletConfig.S;
    b->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, velocity, 0.0f);

    b->cCollision = std::make_shared<CCollision>(m_bulletConfig.SR);

    b->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);
}

void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity) {
    if (!entity || !entity->cTransform) {
        std::cerr << "Invalid entity passed to spawnSpecialWeapon!" << std::endl;
        return;
    }
    auto laser = m_entities.addEntity("Laser");

    float laserWidth = 20.0f;
    float laserLength = m_window.getSize().y;

    Vec2 pos(entity->cTransform->pos.x, 0); // Starts at top of screen
    Vec2 velocity(0, 0);

    laser->cTransform = std::make_shared<CTransform>(pos, velocity, 0.0f);

    // New: use rectangle shape component
    laser->cRectShape = std::make_shared<CRectShape>(
        sf::Vector2f(laserWidth, laserLength),
        sf::Color::Red,
        sf::Color::White,
        2.0f
    );

    laser->cCollision = std::make_shared<CCollision>(laserWidth / 2.0f);
    laser->cLifespan = std::make_shared<CLifespan>(15);
}



void Game::sMovement(){
 
    // reads player input and sets direction and velocity
    m_player->cTransform->velocity = {0, 0};
    
    if(m_player->cInput-> up){
        m_player->cTransform->velocity.y = - m_playerConfig.S;
    }

    if(m_player->cInput-> down){
        m_player->cTransform->velocity.y = m_playerConfig.S;
    }

    if(m_player->cInput-> left){
        m_player->cTransform->velocity.x = - m_playerConfig.S;
    }

    if(m_player->cInput-> right){
        m_player->cTransform->velocity.x = m_playerConfig.S;
    }

    // Sample movement speed update
    m_player->cTransform->pos.x += m_player->cTransform->velocity.x;
    m_player->cTransform->pos.y += m_player->cTransform->velocity.y;

    float radius = m_player->cShape->circle.getRadius();
    float winW = m_window.getSize().x;
    float winH = m_window.getSize().y;

    // Clamp X
    if (m_player->cTransform->pos.x - radius < 0) {
        m_player->cTransform->pos.x = radius;
    }
    if (m_player->cTransform->pos.x + radius > winW) {
        m_player->cTransform->pos.x = winW - radius;
    }

    // Clamp Y
    if (m_player->cTransform->pos.y - radius < 0) {
        m_player->cTransform->pos.y = radius;
    }
    if (m_player->cTransform->pos.y + radius > winH) {
        m_player->cTransform->pos.y = winH - radius;
    }


    // all entities movement
    for(auto& e: m_entities.getEntities()){
        e->cTransform->pos.x += e->cTransform->velocity.x;
        e->cTransform->pos.y += e->cTransform->velocity.y;
    }

    for(auto& e: m_entities.getEntities("Enemy")){
        if (e->cTransform-> pos.x - e->cShape->circle.getRadius() <= 0 || e->cTransform-> pos.x + e->cShape->circle.getRadius() >= m_window.getSize().x)
        {
            e->cTransform->velocity.x = - e->cTransform->velocity.x;
        }

        if (e->cTransform-> pos.y - e->cShape->circle.getRadius() <= 0 || e->cTransform-> pos.y + e->cShape->circle.getRadius() >= m_window.getSize().y)
        {
            e->cTransform->velocity.y = - e->cTransform->velocity.y;
        }
    }

    for(auto& e: m_entities.getEntities("Small")){
        if (e->cTransform-> pos.x - e->cShape->circle.getRadius() <= 0 || e->cTransform-> pos.x + e->cShape->circle.getRadius() >= m_window.getSize().x)
        {
            e->cTransform->velocity.x = - e->cTransform->velocity.x;
        }

        if (e->cTransform-> pos.y - e->cShape->circle.getRadius() <= 0 || e->cTransform-> pos.y + e->cShape->circle.getRadius() >= m_window.getSize().y)
        {
            e->cTransform->velocity.y = - e->cTransform->velocity.y;
        }
    }

}

void Game:: sLifespan(){

    for(auto& e: m_entities.getEntities()){
        if(!e->cLifespan) continue;

        // decrement lifespan of entity
        if(e->cLifespan->remaining > 0){
            e->cLifespan->remaining -= 1;
        }

        if(e->cLifespan->remaining > 0 && e->isActive()){

            // calculate alpha based on % of lifespan remaining
            float alphaRatio = static_cast<float>(e->cLifespan->remaining) / e->cLifespan->total;
            float alpha = 255.0f * alphaRatio;
            
             // Circle-based entities
             if (e->cShape) {
                auto fill = e->cShape->circle.getFillColor();
                auto outline = e->cShape->circle.getOutlineColor();
                e->cShape->circle.setFillColor(sf::Color(fill.r, fill.g, fill.b, alpha));
                e->cShape->circle.setOutlineColor(sf::Color(outline.r, outline.g, outline.b, alpha));
            }
            // Rectangle-based entities (like laser)
            else if (e->cRectShape) {
                auto fill = e->cRectShape->rect.getFillColor();
                auto outline = e->cRectShape->rect.getOutlineColor();
                e->cRectShape->rect.setFillColor(sf::Color(fill.r, fill.g, fill.b, alpha));
                e->cRectShape->rect.setOutlineColor(sf::Color(outline.r, outline.g, outline.b, alpha));
            }
        }

        if(e->cLifespan->remaining == 0){
            e->destroy();
        }
    }
}

void Game::sCollision() {
    // Bullet vs Enemy & Small
    for (auto& b : m_entities.getEntities("Bullet")) {
        if (!b->isActive() || !b->cTransform || !b->cCollision) continue;

        for (auto& e : m_entities.getEntities("Enemy")) {
            if (!e->isActive() || !e->cTransform || !e->cCollision || !e->cScore) continue;

            if (b->cTransform->pos.dist(e->cTransform->pos) < b->cCollision->radius + e->cCollision->radius) {
                b->destroy();
                e->destroy();
                spawnSmallEnemies(e);
                m_score += e->cScore->score;
                break;
            }
        }

        if (!b->isActive()) continue;

        for (auto& e : m_entities.getEntities("Small")) {
            if (!e->isActive() || !e->cTransform || !e->cCollision || !e->cScore) continue;

            if (b->cTransform->pos.dist(e->cTransform->pos) < b->cCollision->radius + e->cCollision->radius) {
                b->destroy();
                e->destroy();
                m_score += e->cScore->score;
                break;
            }
        }
    }

    // Player vs Enemy
    if (m_player && m_player->isActive() && m_player->cTransform && m_player->cCollision) {
        for (auto& e : m_entities.getEntities("Enemy")) {
            
            if (!e->isActive() || !e->cTransform || !e->cCollision) continue;

            if (m_player->cTransform->pos.dist(e->cTransform->pos) < m_player->cCollision->radius + e->cCollision->radius) {
                e->destroy();
                m_player->destroy();
                m_score -= 5000;
                spawnPlayer();
                break;
            }
        }

        // Player vs small enemy    
        for (auto& e : m_entities.getEntities("Small")) {
            if (!e->isActive() || !e->cTransform || !e->cCollision) continue;

            if (m_player->cTransform->pos.dist(e->cTransform->pos) < m_player->cCollision->radius + e->cCollision->radius) {
                e->destroy();
                m_player->destroy();
                m_score -= 1000;
                spawnPlayer();
                break;
            }
        }

        // Laser vs Enemy 
        for (auto& laser : m_entities.getEntities("Laser")) {
            if (!laser->isActive() || !laser->cTransform || !laser->cRectShape) continue;
        
            float laserX = laser->cTransform->pos.x;
            float laserHalfWidth = laser->cRectShape->rect.getSize().x / 2.0f;
        
            for (auto& e : m_entities.getEntities("Enemy")) {
                if (!e->isActive() || !e->cTransform || !e->cCollision) continue;
        
                float enemyX = e->cTransform->pos.x;
                float enemyRadius = e->cCollision->radius;
        
                if (std::abs(laserX - enemyX) < laserHalfWidth + enemyRadius) {
                    laser->destroy();
                    e->destroy();
                    spawnSmallEnemies(e);
                    m_score += e->cScore->score;
                    break;
                }
            }
        
            // Laser vs Small
            for (auto& e : m_entities.getEntities("Small")) {
                if (!e->isActive() || !e->cTransform || !e->cCollision) continue;
        
                float smallX = e->cTransform->pos.x;
                float smallRadius = e->cCollision->radius;
        
                if (std::abs(laserX - smallX) < laserHalfWidth + smallRadius) {
                    laser->destroy();
                    e->destroy();
                    m_score += e->cScore->score;
                    break;
                }
            }
        }
        
    }
}


void Game::sEnemySpawner(){

    if (m_currentFrame - m_lastEnemySpawnTime > 120){
        spawnEnemy();
        m_lastEnemySpawnTime = m_currentFrame;
    }
    
}

void Game::sRender()
{

    m_window.clear(sf::Color::Black);
  
    for (auto& e : m_entities.getEntities()) {
        if (e->cRectShape) {
            e->cRectShape->rect.setPosition({e->cTransform->pos.x, e->cTransform->pos.y});
            e->cRectShape->rect.setRotation(sf::degrees(e->cTransform->angle));
            m_window.draw(e->cRectShape->rect);
        }
        else if (e->cShape) {
            e->cShape->circle.setPosition({e->cTransform->pos.x, e->cTransform->pos.y});
            e->cTransform->angle += 1.0f;
            e->cShape->circle.setRotation(sf::degrees(e->cTransform->angle));
            m_window.draw(e->cShape->circle);
        }
    }
    

    // draw score
    if(m_score < 0){m_score = 0;}
    std::string score = "Score: " + std::to_string (m_score);
    m_text.setString(score);
    m_text.setPosition({10.0f, 10.0f});
    m_window.draw(m_text);

    m_window.display(); 
}

void Game::sUserInput()
{   
    while (const std::optional<sf::Event> event = m_window.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
        {
            m_running = false;
            m_window.close();
        }
        //this event is triggered when key is pressed
        
        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            switch (keyPressed->scancode)
            {
                case sf::Keyboard::Scancode::W:
                    m_player->cInput->up = true;
                    break;
                case sf::Keyboard::Scancode::A:
                    m_player->cInput->left = true;
                    break;
                case sf::Keyboard::Scancode::S:
                    m_player->cInput->down = true;
                    break;
                case sf::Keyboard::Scancode::D:
                    m_player->cInput->right = true;
                    break;
                case sf::Keyboard::Scancode::Escape:
                    m_window.close();
                    m_running = false;
                    break;
                case sf::Keyboard::Scancode::Space:
                    setPaused(!m_paused);
                    
                    break;
                default:
                    break;
            }
        }

        // this event triggers when a key is released
        if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
        {
            switch (keyReleased->scancode)
            {
                case sf::Keyboard::Scancode::W:
                    m_player->cInput->up = false;
                    break;
                case sf::Keyboard::Scancode::A:
                    m_player->cInput->left = false;
                    break;
                case sf::Keyboard::Scancode::S:
                    m_player->cInput->down = false;
                    break;
                case sf::Keyboard::Scancode::D:
                    m_player->cInput->right = false;
                    break;
                default:
                    break;
            }
        }

        // this event is triggered by left clicking or right clicking
        if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
        {
            switch (mousePressed->button)
            {
                case sf::Mouse::Button::Left:
                    if (!m_paused){
                        spawnBullet(m_player, Vec2(sf::Mouse::getPosition(m_window).x, sf::Mouse::getPosition(m_window).y));
                    }
                    break;
                case sf::Mouse::Button::Right:
                    if (!m_paused) {
                        spawnSpecialWeapon(m_player);
                    }
                    break;
                
                default:
                    break;
            }
        }

    }

}