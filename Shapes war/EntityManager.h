#pragma once

#include "Entity.h"
#include <vector>
#include <map>

typedef std::vector<std::shared_ptr<Entity>> EntityVec;  // EntityVec means  std::vector<std::shared_ptr<Entity>>
typedef std::map<std::string, EntityVec>     EntityMap;  // map from tag to the vector<Entity> having that tag

class EntityManager{

        EntityVec m_entities;
        EntityVec m_entitiesToAdd;          // to avoid iterator invalidation
        EntityMap m_entityMap;
        size_t    m_totalEntities = 0;      // total number ever created in game (for unique id)

        void removeDeadEntities(EntityVec& vec);

    public:
        EntityManager();

        void update();
        
        std:: shared_ptr<Entity> addEntity(const std::string& tag);

        const EntityVec& getEntities();
        const EntityVec& getEntities(const std::string& tag);

};