#pragma once

#include <algorithm>
#include <iostream>
#include <memory>
#include <set>
#include <utility>
#include <vector>

// Windows
//#ifndef __PRETTY_FUNCTION__
//#define __PRETTY_FUNCTION__ __FUNCSIG__
//#endif

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

  Dimensions bounds;
  Entity* parent = nullptr;
};
///////////////////////////
class EntitiesSingleton
{
  EntitiesSingleton(){};

  static EntitiesSingleton* instance;

public:
  ~EntitiesSingleton() { std::cout << "DTOR " << __PRETTY_FUNCTION__ << "\n"; }

  static EntitiesSingleton*
  getInstance();

  std::vector<std::unique_ptr<Entity>> data;
};

///////////////////////////
Entity::property
operator|(Entity::property lhs, Entity::property rhs);

Entity::property
operator&(Entity::property lhs, Entity::property rhs);

Entity::property
operator|=(Entity::property& lhs, Entity::property rhs);

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
Entity&
operator|(Entity& e, int year);

struct Years;

struct Entities
{
  /**
   * RAII for Singleton to invoke its child objects' DTORs upon exiting
   */
  Entities();

  ~Entities() = default;

  std::unique_ptr<EntitiesSingleton> entities;
  std::unique_ptr<Years> years;
};

template<typename>
void
populateEntitiesTest()
{
  auto& philip = "Philip II of Macedon"_e | -382 | -336;
  "Alexander the Great"_e | -356 | -323;
  "Aristoteles"_e | -384 | -322;
  "Plato"_e | -428 | -348;
  "Macedonia"_e | -808 | -168;
  "Minna the Great"_e | -3200 | 2019;

  //    for (auto& e : EntitiesSingleton::getInstance()->data)
  //        years.insert(e.get());
}
