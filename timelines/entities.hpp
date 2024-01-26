#pragma once

#include <algorithm>
#include <iostream>
#include <memory>
#include <set>
#include <utility>
#include <vector>

// Windows
// #ifndef __PRETTY_FUNCTION__
// #define __PRETTY_FUNCTION__ __FUNCSIG__
// #endif
namespace tl
{
static size_t ids = 0;

struct Dimensions
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
};

struct Entity
{
    explicit Entity(std::string _name);

    Entity(std::string _name, int _start, int _end);

    ~Entity();

    enum class Property : unsigned
    {
        None = 0x0,
        Has_name_and_id = 0x1,
        Has_start_year = 0x2,
        Has_end_year = 0x4
    };

    const std::string name;
    const size_t id;
    int start_year = 0;
    int end_year = 0;
    Property properties = Property::None;

    Dimensions bounds;
    Entity* parent = nullptr;
};

///////////////////////////
Entity::Property operator|(Entity::Property lhs, Entity::Property rhs);

Entity::Property operator&(Entity::Property lhs, Entity::Property rhs);

Entity::Property operator|=(Entity::Property& lhs, Entity::Property rhs);

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

template<typename>
void
populate_entities_test()
{
    auto& philip = "Philip II of Macedon"_e | -382 | -336;
    "Alexander the Great"_e | -356 | -323;
    "Aristoteles"_e | -384 | -322;
    "Plato"_e | -428 | -348;
    "Macedonia"_e | -808 | -168;
    "Minna the Great"_e | -3200 | 2019;

    //    for (auto& e : Global::instance()->data)
    //        years.insert(e.get());
}
}