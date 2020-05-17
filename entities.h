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
    Entity(std::string _name)
      : name(std::move(_name))
      , id(ids++)
    {
        std::cout << ids << "\n";
    }

    ~Entity() = default;

    Entity(std::string _name, int _start, int _end)
      : name(std::move(_name))
      , startYear(_start)
      , endYear(_end)
      , id(ids++)
    {
        // birth before death, right?
        assert(startYear <= endYear);
    }

    enum class property : unsigned
    {
        none         = 0x0,
        hasNameAndId = 0x1,
        hasStartYear = 0x2,
        hasEndYear   = 0x4
    };

    const std::string name;
    int               startYear = 0;
    int               endYear   = 0;
    const size_t      id;
    property          properties = property::none;
};
///////////////////////////
std::vector<std::unique_ptr<Entity>> entities;
///////////////////////////
Entity::property
operator|(Entity::property lhs, Entity::property rhs)
{
    return static_cast<Entity::property>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs));
}
Entity::property operator&(Entity::property lhs, Entity::property rhs)
{
    return static_cast<Entity::property>(static_cast<std::underlying_type<Entity::property>::type>(lhs) &
                                         static_cast<std::underlying_type<Entity::property>::type>(rhs));
}

Entity::property
operator|=(Entity::property& lhs, Entity::property rhs)
{
    lhs = static_cast<Entity::property>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs));
    return lhs;
}

Entity
operator""_e(const char* text, size_t)
{
    Entity e{ text };
    e.properties |= Entity::property::hasNameAndId;
    return e;
}

Entity
operator|(Entity e, int year)
{
    if ((bool)(e.properties & Entity::property::hasStartYear))
    {
        e.endYear = year;
        e.properties |= Entity::property::hasEndYear;
        entities.emplace_back(std::move(&e));
        return e;
    }
    else if ((bool)(e.properties & Entity::property::hasEndYear))
    {
        std::cout << "warning: tried to set even though all parameters are set\n";
        return e;
    }
    else
    {
        e.startYear = year;
        e.properties |= Entity::property::hasStartYear;
        return e;
    }
}
