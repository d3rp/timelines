#include "entities.h"
#include "calendars.h"

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

Entities* Entities::instance = nullptr;

Entity::Entity(std::string _name)
    : name(std::move(_name))
    , id(ids++)
{
    std::cout << "Creating " << name << ", id: " << ids - 1 << "\n";
}

Entity::Entity(std::string _name, int _start, int _end)
    : name(std::move(_name))
    , startYear(_start)
    , endYear(_end)
    , id(ids++)
{
    // birth before death, right?
    assert(startYear <= endYear);
}

Entity::~Entity()
{
    std::cout << "DTOR '" << name <<"'\n";
}

Entities*
Entities::getInstance()
{
    if (instance == nullptr)
        instance = new Entities();

    return instance;
}

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

Entity& operator""_e(const char* text, size_t)
{
    Entity* e = new Entity{ text };
    e->properties |= Entity::property::hasNameAndId;
    return *e;
}

Entity&
operator|(Entity& e, int year)
{
    if ((bool) (e.properties & Entity::property::hasStartYear))
    {
        // end year should be after start year
        assert(year > e.startYear);

        // year should remain within set limits
        std::cout << yearLimits(year) << "\n";
        assert(yearLimits(year) == year);

        e.endYear = year;
        e.properties |= Entity::property::hasEndYear;
        std::cout << "emplacing (" << e.name << ", " << e.startYear << ", " << e.endYear << ")\n";
        Entities::getInstance()->data.emplace_back(&e);
//        Years::getInstance()->insert(&e);
        return e;
    }
    else if ((bool) (e.properties & Entity::property::hasEndYear))
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

ScopedEntities::ScopedEntities()
  : _entities{ Entities::getInstance() }
  , _years{ Years::getInstance() }
{}
