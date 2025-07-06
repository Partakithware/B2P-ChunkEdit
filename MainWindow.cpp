// MainWindow.cpp
#include "MainWindow.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <gtkmm/filedialog.h>
#include <giomm/asyncresult.h>
#include <giomm/file.h>

MainWindow::MainWindow()
: vbox_main(Gtk::Orientation::VERTICAL),
  btn_prev("Prev"),
  btn_next("Next"),
  btn_save("Save"),
  spin_chunk_size(Gtk::Adjustment::create(128, 1, 4096, 1, 64)) // default 512, min 1, max 4096
{
    set_title("ChunkEdit");
    set_default_size(700, 400);  // widen a bit for new widgets

    auto btn_open = Gtk::make_managed<Gtk::Button>("Open File");
    btn_open->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_open_file));

    btn_prev.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_prev_chunk));
    btn_next.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_next_chunk));
    btn_save.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_save_file));

    spin_chunk_size.signal_value_changed().connect(sigc::mem_fun(*this, &MainWindow::on_chunk_size_changed));

    // Horizontal box for controls + info
    auto control_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
    control_box->append(*btn_open);
    control_box->append(btn_prev);
    control_box->append(btn_next);
    control_box->append(btn_save);

    // Chunk info label
    control_box->append(label_chunk_info);
    

    // Spacer
    auto spacer = Gtk::make_managed<Gtk::Separator>(Gtk::Orientation::VERTICAL);
    control_box->append(*spacer);

    // Chunk size spinner label + widget
    auto size_label = Gtk::make_managed<Gtk::Label>("Chunk Size:");
    control_box->append(*size_label);
    control_box->append(spin_chunk_size);

    vbox_main.append(*control_box);
    scroll.set_child(byte_grid);
    scroll.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC); // Horizontal: never, Vertical: scroll
    scroll.set_expand(true);
    vbox_main.append(scroll);

    // Setup view toggle buttons
    btn_view_grid.set_active(true); // Default
    btn_view_raw.set_group(btn_view_grid);

    btn_view_grid.signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::on_view_mode_toggled));
    btn_view_raw.signal_toggled().connect(sigc::mem_fun(*this, &MainWindow::on_view_mode_toggled));

    bottom_bar.set_orientation(Gtk::Orientation::HORIZONTAL);
    bottom_bar.set_spacing(10);
    bottom_bar.set_margin(5);
    bottom_bar.append(btn_view_grid);
    bottom_bar.append(btn_view_raw);
    vbox_main.append(bottom_bar);


    // Setup raw view
    scroll_raw.set_child(raw_text_view);
    scroll_raw.set_policy(Gtk::PolicyType::AUTOMATIC, Gtk::PolicyType::AUTOMATIC);
    scroll_raw.set_expand(true);
    raw_text_view.set_monospace(true);
        vbox_main.append(scroll_raw);

    // Initialize label text
    update_chunk_info_label();

    set_child(vbox_main);
}

MainWindow::~MainWindow() {}

void MainWindow::on_open_file() {
    auto file_dialog = Gtk::FileDialog::create();
    file_dialog->set_title("Open Binary File");

    file_dialog->open(*this, [this, file_dialog](const Glib::RefPtr<Gio::AsyncResult>& result) {
        try {
            auto file = file_dialog->open_finish(result);
            if (!file)
                return;

            std::string filename = file->get_path();
            editor = std::make_unique<ChunkEditor>(filename);
            current_chunk_index = 0;
            load_chunk(current_chunk_index);
        } catch (const Glib::Error& ex) {
            std::cerr << "Open dialog error: " << ex.what() << std::endl;
        }
    });
}

void MainWindow::load_chunk(size_t index) {
    if (!editor) return;

    current_chunk_size = spin_chunk_size.get_value_as_int();
    editor->set_chunk_size(current_chunk_size);

    // 🔥 MOVE THIS TO TOP — Save changes BEFORE overwriting data
    if (index != current_chunk_index) {
        sync_from_view();  // Save edits from old chunk
    }

    current_chunk_data = editor->read_chunk(index); // Now safe to load new
    current_chunk_index = index;
    current_chunk_size = current_chunk_data.size();

    update_chunk_info_label();

    // GUI setup...
    for (auto* entry : byte_entries) {
        byte_grid.remove(*entry);
        delete entry;
    }
    byte_entries.clear();
    raw_text_view.get_buffer()->set_text("");
    scroll.set_visible(false);
    scroll_raw.set_visible(false);

    if (current_view_mode == ViewMode::Grid) {
        const int columns = 8;
        for (size_t i = 0; i < current_chunk_data.size(); ++i) {
            auto* entry = new Gtk::Entry();
            std::stringstream ss;
            ss.imbue(std::locale::classic());
            ss << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)current_chunk_data[i];
            entry->set_text(ss.str());
            entry->set_width_chars(3);

            size_t absolute_offset = (current_chunk_index * current_chunk_size) + i;
            std::stringstream tooltip;
            tooltip.imbue(std::locale::classic());
            tooltip << "Offset: 0x" << std::hex << std::uppercase << absolute_offset
                    << " / " << std::dec << absolute_offset << " dec";
            entry->set_tooltip_text(tooltip.str());

            entry->signal_changed().connect([=]() {
                on_byte_edited(i);
            });

            int row = i / columns;
            int col = i % columns;
            byte_grid.attach(*entry, col, row);
            byte_entries.push_back(entry);
        }
        scroll.set_visible(true);
    } else {
        std::stringstream raw;
        for (uint8_t byte : current_chunk_data) {
            raw << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
        }
        raw_text_view.get_buffer()->set_text(raw.str());
        scroll_raw.set_visible(true);
    }
}

void MainWindow::update_chunk_info_label() {
    size_t offset_start = current_chunk_index * current_chunk_size;
    size_t offset_end = offset_start + current_chunk_size - 1;

    std::stringstream ss;
    ss.imbue(std::locale::classic());
    ss << "Chunk: " << current_chunk_index
       << " (Offset: 0x" << std::uppercase << std::hex << offset_start
       << "–0x" << offset_end
       << " / " << std::dec << offset_start
       << "–" << offset_end << " dec)"
       << "    Chunk Size: " << current_chunk_size;

    label_chunk_info.set_text(ss.str());
}

void MainWindow::on_chunk_size_changed() {
    size_t new_chunk_size = spin_chunk_size.get_value_as_int();
    if (new_chunk_size == current_chunk_size)
        return;

    current_chunk_size = new_chunk_size;

    if (!editor) return;

    editor->set_chunk_size(current_chunk_size);

    // Clamp current chunk index to valid range after resizing
    size_t total_chunks = editor->get_total_chunks();
    if (current_chunk_index >= total_chunks)
        current_chunk_index = total_chunks ? total_chunks - 1 : 0;

    load_chunk(current_chunk_index);
}

void MainWindow::on_byte_edited(size_t byte_index) {
    if (!editor || byte_index >= current_chunk_size) return;

    auto text = byte_entries[byte_index]->get_text();
    try {
        uint8_t value = std::stoul(text, nullptr, 16);
        //auto chunk = editor->read_chunk(current_chunk_index);
        //chunk[byte_index] = value;
        if (byte_index < current_chunk_data.size()) {
        current_chunk_data[byte_index] = value;
        }
        //editor->write_chunk(current_chunk_index, chunk);
    } catch (...) {
        // Invalid hex input; ignore for now
    }
}

void MainWindow::on_prev_chunk() {
    if (!editor || current_chunk_index == 0) return;
    sync_from_view(); // <-- Capture edits from current view before changing
    --current_chunk_index;
    load_chunk(current_chunk_index);
}

void MainWindow::on_next_chunk() {
    if (!editor) return;
    sync_from_view(); // <-- Capture edits from current view before changing
    if (current_chunk_index + 1 >= editor->get_total_chunks()) return;
    ++current_chunk_index;
    load_chunk(current_chunk_index);
}


void MainWindow::sync_from_view() {
    if (!editor) return;

    if (current_view_mode == ViewMode::Grid) {
        for (size_t i = 0; i < byte_entries.size(); ++i) {
            auto text = byte_entries[i]->get_text();
            try {
                uint8_t value = std::stoul(text, nullptr, 16);
                if (i < current_chunk_data.size())
                    current_chunk_data[i] = value;
            } catch (...) {
                // Ignore invalid hex input
            }
        }
    } else {
        auto text = raw_text_view.get_buffer()->get_text();
        std::vector<uint8_t> new_data;
        for (size_t i = 0; i + 1 < text.size(); i += 2) {
            std::string hex_byte = text.substr(i, 2);
            try {
                uint8_t byte = std::stoul(hex_byte, nullptr, 16);
                new_data.push_back(byte);
            } catch (...) {
                std::cerr << "Invalid hex byte: " << hex_byte << "\n";
            }
        }
        current_chunk_data = std::move(new_data);
    }

    // 🔥 THIS IS THE MISSING STEP 🔥
    editor->write_chunk(current_chunk_index, current_chunk_data);
}

void MainWindow::on_view_mode_toggled() {
    sync_from_view(); // <-- Capture edits from current view before changing
    if (btn_view_grid.get_active()) {
        current_view_mode = ViewMode::Grid;
    } else if (btn_view_raw.get_active()) {
        current_view_mode = ViewMode::Raw;
    }

    load_chunk(current_chunk_index); // Reload current chunk in new mode
}

void MainWindow::save_raw_view(const std::string& save_path) {
    if (!editor) return;

    auto text = raw_text_view.get_buffer()->get_text();

    std::vector<uint8_t> data;
    for (size_t i = 0; i + 1 < text.size(); i += 2) {
        std::string hex_byte = text.substr(i, 2);
        try {
            uint8_t byte = std::stoul(hex_byte, nullptr, 16);
            data.push_back(byte);
        } catch (...) {
            std::cerr << "Invalid hex byte: " << hex_byte << "\n";
        }
    }

    current_chunk_data = std::move(data);

    // 🔥 Mark it dirty — or NOTHING gets saved
    editor->write_chunk(current_chunk_index, current_chunk_data);

    editor->save_as(save_path);
}



void MainWindow::on_save_file() {
    auto file_dialog = Gtk::FileDialog::create();
    file_dialog->set_title("Save As");

    file_dialog->save(*this, [this, file_dialog](const Glib::RefPtr<Gio::AsyncResult>& result) {
        try {
            auto file = file_dialog->save_finish(result);
            if (!file)
                return;

            std::string save_path = file->get_path();

            if (current_view_mode == ViewMode::Grid) {
                editor->write_chunk(current_chunk_index, current_chunk_data);
                editor->save_as(save_path);
            } else {
                save_raw_view(save_path);
            }
        } catch (const Glib::Error& ex) {
            std::cerr << "Save dialog error: " << ex.what() << std::endl;
        }
    });
}
