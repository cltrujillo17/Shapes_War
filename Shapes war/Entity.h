#pragma once

#include "Components.h"
#include <memory>
#include <string>


class Entity{

    friend class EntityManager;

    bool m_active = true;
    size_t m_id = 0;
    std::string m_tag = "default";

    //constructor and destructorx

    Entity(const size_t id, const std::string& tag);

public:

    //component pointer
    std::shared_ptr<CTransform> cTransform;
    std::shared_ptr<CShape> cShape;
    std::shared_ptr<CRectShape> cRectShape;     // added component
    std::shared_ptr<CCollision> cCollision;
    std::shared_ptr<CInput> cInput;
    std::shared_ptr<CScore> cScore;
    std::shared_ptr<CLifespan> cLifespan;

    //private member access functions
    bool isActive() const;
    const std:: string & tag() const;
    const size_t id() const;
    void destroy();

};