#include "entities.hpp"
#include "resource_mgr.hpp"

#if 0
#include "doctest/doctest.h"

int factorial(int number) { return number <= 1 ? number : factorial(number - 1) * number; }


TEST_CASE("testing the factorial function") {
    CHECK(factorial(1) == 1);
    CHECK(factorial(2) == 2);
    CHECK(factorial(3) == 6);
    CHECK(factorial(10) == 3628800);
}

TEST_CASE("testing entities")
{
    Entity e("foo", 0, 1);
    CHECK(e.name == "foo");
}
#endif

namespace tl
{

Entity::Entity(std::string _name)
  : name(std::move(_name))
  , id(ids++)
{
    std::cout << "Creating " << name << ", id: " << ids - 1 << "\n";
}

Entity::Entity(std::string _name, int _start, int _end)
  : name(std::move(_name))
  , start_year(_start)
  , end_year(_end)
  , id(ids++)
{
    // birth before death, right?
    assert(start_year <= end_year);
}

Entity::~Entity() { std::cout << "DTOR '" << name << "'\n"; }

Entity::Property
operator|(Entity::Property lhs, Entity::Property rhs)
{
    return static_cast<Entity::Property>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs));
}

Entity::Property
operator&(Entity::Property lhs, Entity::Property rhs)
{
    return static_cast<Entity::Property>(
      static_cast<std::underlying_type<Entity::Property>::type>(lhs) &
      static_cast<std::underlying_type<Entity::Property>::type>(rhs));
}

Entity::Property
operator|=(Entity::Property& lhs, Entity::Property rhs)
{
    lhs = static_cast<Entity::Property>(static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs));
    return lhs;
}

Entity&
operator""_e(const char* text, size_t)
{
    Entity* e = new Entity{ text };
    e->properties |= Entity::Property::Has_name_and_id;
    return *e;
}

Entity&
operator|(Entity& e, int year)
{
    if ((bool)(e.properties & Entity::Property::Has_start_year))
    {
        // end year should be after start year
        assert(year > e.start_year);

        // year should remain within set limits
        std::cout << year_limits(year) << "\n";
        assert(year_limits(year) == year);

        e.end_year = year;
        e.properties |= Entity::Property::Has_end_year;
        std::cout << "emplacing (" << e.name << ", " << e.start_year << ", " << e.end_year << ")\n";
        Global::instance().data.emplace_back(&e);
        //        Years::instance()->insert(&e);
        return e;
    }
    else if ((bool)(e.properties & Entity::Property::Has_end_year))
    {
        std::cout << "warning: tried to set even though all parameters are set\n";
        return e;
    }
    else
    {
        e.start_year = year;
        e.properties |= Entity::Property::Has_start_year;
        return e;
    }
}

Global&
Global::instance()
{
    static Global instance;
    return instance;
}
}