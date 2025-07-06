// main.cpp
#include <gtkmm/application.h>
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create("com.example.chunkedit");

    app->signal_activate().connect([&app]() {
        auto window = new MainWindow();
        window->set_application(app);
        window->present();
        // app will keep the window alive once added
    });

    return app->run(argc, argv);
}