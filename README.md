# B2P - ChunkEdit

**B2P - ChunkEdit** is a simple, GTK-based binary file viewer and editor built using C++. It allows you to inspect and modify files in fixed-size chunks, either in a structured grid layout or as raw hexadecimal text.

This tool is part of a planned broader **Bin2Play (B2P)** toolkit. (This is just the first concept)

---

## Features

- ✅ Open and read any binary file
- ✅ View file contents in chunked format (custom chunk size)
- ✅ Edit binary data in two modes:
  - **Grid view** — edit each byte individually with a tooltip showing the file offset
  - **Raw hex view** — edit the full chunk as a hex string
- ✅ Navigate between chunks with Prev/Next buttons
- ✅ Adjustable chunk size (1 to 4096 bytes)
- ✅ Save changes to a new file via "Save As"
- ✅ Chunk-level tracking to ensure only modified parts are written (Flawed in several ways)
  Flaw 1 - First off if you edit a chunk, you have to save the file while in that chunk, this is not how it's intended to be.
  Flaw 2 - When switching between RAW/GRID it will reset what you changed, not intended to be this way, which means if you edit in RAW - Save new file before switching, same with GRID.

This is an early concept, so not all has been done/updated.
---

## Screenshots

Grid view:

[![Grid view](https://i.ibb.co/xSRC2PWH/Screenshot-from-2025-07-06-11-02-22.png)](https://ibb.co/1fwv0h16)

Raw view:

[![Raw view](https://i.ibb.co/tPbrL5NR/Screenshot-from-2025-07-06-11-06-54.png)](https://ibb.co/cXNnYdpq)

Chunk navigation and editing:

[![Editing](https://i.ibb.co/8L8Zy1b0/Screenshot-from-2025-07-06-11-07-07.png)](https://ibb.co/4nNrbh27)

---

## Requirements

- GTK4
- gtkmm-4
- C++17 or later

---

## Build Instructions

```bash
meson setup build
meson compile -C build
./build/chunkedit
