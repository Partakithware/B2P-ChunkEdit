// MainWindow.h
#pragma once
#include <gtkmm.h>
#include "ChunkEditor.h"

    enum class ViewMode {
    Grid,
    Raw
    };

class MainWindow : public Gtk::Window {
public:
    MainWindow();
    virtual ~MainWindow();

private:
    void on_open_file();
    void on_save_file();
    void load_chunk(size_t index);
    void on_prev_chunk();
    void on_next_chunk();
    void on_byte_edited(size_t byte_index);

    void update_chunk_info_label();
    void on_chunk_size_changed();
    void save_raw_view(const std::string& save_path);
    void on_view_mode_toggled();
    void sync_from_view();
    

    Gtk::Box vbox_main;
    Gtk::ScrolledWindow scroll;
    Gtk::Grid byte_grid;
    Gtk::Button btn_prev, btn_next, btn_save;
    Gtk::FileChooserDialog* file_chooser = nullptr;
    Gtk::Label label_chunk_info;          // "Chunk: X (Offset: 0x... / ... dec)"
    Gtk::SpinButton spin_chunk_size;      // For chunk size selection

    ViewMode current_view_mode = ViewMode::Grid;
    Gtk::Box bottom_bar; // For toggle buttons
    Gtk::ToggleButton btn_view_grid{"Grid View"};
    Gtk::ToggleButton btn_view_raw{"Raw View"};

    Gtk::ScrolledWindow scroll_raw;
    Gtk::TextView raw_text_view;

    std::unique_ptr<ChunkEditor> editor;
    std::vector<Gtk::Entry*> byte_entries;
    std::vector<uint8_t> current_chunk_data;
    
    // State to track chunk size (default 512)
    size_t current_chunk_size = 512;

    size_t current_chunk_index = 0;
};