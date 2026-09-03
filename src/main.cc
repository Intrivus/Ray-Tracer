#include "app.hpp"

int main()
{
    double aspect_ratio = 16.0 / 9.0;
    int window_width = 400;
    App app(aspect_ratio, window_width);

    app.init();
    app.update();
    app.shutdown();
}