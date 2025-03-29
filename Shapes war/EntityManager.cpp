#include "EntityManager.h"
#include <algorithm>

EntityManager::EntityManager(){

}

void EntityManager::update(){

    for(auto& e: m_entitiesToAdd){
        m_entities.push_back(e);            // adds entity(in waitng room) to the vector of all entities
        m_entityMap[e->tag()].push_back(e); // adds entity(in waiting room) to the vector inside the map, with the tag as key
    }

    m_entitiesToAdd.clear();                // clear "waiting room"

    // remove dead entities from each vcector in the entity map
    for(auto& [tag, EntityVec]: m_entityMap){
        removeDeadEntities(EntityVec);
    }

    // remove dead entities from the vector of all entities
    removeDeadEntities(m_entities);
}

void EntityManager::removeDeadEntities(EntityVec& vec) {
    vec.erase(
        std::remove_if(vec.begin(), vec.end(), 
            [](const std::shared_ptr<Entity>& e) {
                return !e->isActive(); // Remove entities that are inactive
        }),
        vec.end()
    );
}


std::shared_ptr<Entity> EntityManager:: addEntity(const std::string& tag){

    // this creates the entity shared pointer
    auto entity = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));

    m_entitiesToAdd.push_back(entity);

    return entity;
}

const EntityVec& EntityManager::getEntities(){
    return m_entities;
}

const EntityVec& EntityManager::getEntities(const std::string& tag){
    return m_entityMap[tag];
}