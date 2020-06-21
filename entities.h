//
// Created by j on 1/17/20.
//
#pragma once

#include <algorithm>
#include <iostream>
#include <memory>
#include <set>
#include <utility>
#include <vector>

static size_t ids = 0;

struct Entity
{
    Entity(std::string _name);

    Entity(std::string _name, int _start, int _end);

    ~Entity();


    enum class property : unsigned
    {
        none = 0x0,
        hasNameAndId = 0x1,
        hasStartYear = 0x2,
        hasEndYear = 0x4
    };

    const std::string name;
    const size_t id;
    int startYear = 0;
    int endYear = 0;
    property properties = property::none;
};
///////////////////////////
class Entities
{
    Entities() {};

    static Entities* instance;

public:
    ~Entities() { std::cout << "DTOR " << __PRETTY_FUNCTION__ << "\n"; }

    static Entities* getInstance();

    std::vector<std::unique_ptr<Entity>> data;
};

///////////////////////////
Entity::property operator|(Entity::property lhs, Entity::property rhs);

Entity::property operator&(Entity::property lhs, Entity::property rhs);

Entity::property operator|=(Entity::property& lhs, Entity::property rhs);

/**
 * Creates an Entity object in the heap (new Entity) and
 * returns a reference to it. This can be then used with the
 * '|' operator:
 *   Entity& e = "Historical figure"_e | <birth year> | <year of demise>
 *
 * @param text
 * @return Reference to Entity allocated in heap
 */
Entity& operator""_e(const char* text, size_t);

/**
 * First usage will default to passing <start year> and second
 * to <end year>. When passing the end year, the pointer is emplaced
 * in a container passing the ownership to it.
 *
 * @param e
 * @param year
 * @return
 */
Entity& operator|(Entity& e, int year);
