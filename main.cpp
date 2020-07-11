#include "sdl_handling.h"
#include "csv/csv.h"
#include <filesystem>

int
main()
{
    ScopedEntities sc_e;
    ScopedGraphics sc_g;

    // path to csv
    auto basepath = std::filesystem::path(std::string(SDL_GetBasePath()));
    auto project_path =  basepath.parent_path().parent_path();
    auto example_csv = project_path / "example.csv";

    io::CSVReader<3> in(example_csv.string());

    // populate entities from csv
    in.read_header(io::ignore_extra_column, "name", "start", "end");
    std::string name = ""; int startYear = 0; int endYear = 0;
    while(in.read_row(name, startYear, endYear)){
        std::cout << "name: " << name << " s: " << startYear << " e: " << endYear << "\n";
        auto* e = new Entity(name);
        *e | startYear | endYear;
    }

    RenderingController controller;

   
    Renderer* r1 = new Horizontal();
    r1->yearRange.start_ = -430;
    r1->yearRange.end_ = 0;
    r1->renderRange(Entities::getInstance()->data, &r1->yearRange);
    controller.rendererContainer_.emplace_back(r1);

    Renderer* r0 = new Vertical();
    r0->yearRange.start_ = -430;
    r0->yearRange.end_ = 0;
    r0->renderRange(Entities::getInstance()->data, &r0->yearRange);
    controller.rendererContainer_.emplace_back(r0);
 
    controller.renderer_ = controller.rendererContainer_.back().get();

    EventHandler evh{&controller};
    evh.handleEvents();
 
    return EXIT_SUCCESS;
}
