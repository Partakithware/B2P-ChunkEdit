// ChunkEditor.cpp
#include "ChunkEditor.h"
#include <stdexcept>
#include <cstring>
#include <cstdint>

ChunkEditor::ChunkEditor(const std::string& path, size_t chunkSize)
    : original_path(path), chunk_size(chunkSize) {
    input.open(path, std::ios::binary | std::ios::ate);
    if (!input)
        throw std::runtime_error("Failed to open file");

    file_size = static_cast<size_t>(input.tellg());
    input.seekg(0, std::ios::beg);
}

std::vector<uint8_t> ChunkEditor::read_chunk(size_t index) {
    size_t offset = index * chunk_size;
    if (offset >= file_size)
        return {};

    input.seekg(offset, std::ios::beg);
    size_t bytes_to_read = std::min(chunk_size, file_size - offset);

    std::vector<uint8_t> buffer(bytes_to_read);
    input.read(reinterpret_cast<char*>(buffer.data()), bytes_to_read);

    return buffer;
}

void ChunkEditor::write_chunk(size_t index, const std::vector<uint8_t>& data) {
    dirty_chunks[index] = data;
}

void ChunkEditor::set_chunk_size(size_t size) {
    chunk_size = size;
}

/*void ChunkEditor::save_as(const std::string& outPath) {
    std::ifstream in(original_path, std::ios::binary);
    std::ofstream out(outPath, std::ios::binary | std::ios::trunc);

    if (!in || !out)
        throw std::runtime_error("Failed to open files for saving");

    std::vector<uint8_t> buffer(chunk_size);
    size_t total_chunks = get_total_chunks();

    for (size_t i = 0; i < total_chunks; ++i) {
        if (dirty_chunks.count(i)) {
            out.write(reinterpret_cast<const char*>(dirty_chunks[i].data()),
                      dirty_chunks[i].size());
        } else {
            in.read(reinterpret_cast<char*>(buffer.data()), chunk_size);
            out.write(reinterpret_cast<char*>(buffer.data()), in.gcount());
        }
    }

    // Write trailing bytes if any
    size_t trailing = file_size % chunk_size;
    if (trailing > 0) {
        std::vector<uint8_t> tail(trailing);
        in.read(reinterpret_cast<char*>(tail.data()), trailing);
        out.write(reinterpret_cast<char*>(tail.data()), trailing);
    }
} */

void ChunkEditor::save_as(const std::string& outPath) {
    std::ifstream in(original_path, std::ios::binary);
    std::ofstream out(outPath, std::ios::binary | std::ios::trunc);

    if (!in || !out)
        throw std::runtime_error("Failed to open files for saving");

    std::vector<uint8_t> buffer(chunk_size);
    size_t total_chunks = get_total_chunks();

    for (size_t i = 0; i < total_chunks; ++i) {
        if (dirty_chunks.count(i)) {
            // Write edited chunk
            out.write(reinterpret_cast<const char*>(dirty_chunks[i].data()),
                      dirty_chunks[i].size());

            // Advance input stream to skip original chunk bytes
            in.seekg(chunk_size, std::ios::cur);
        } else {
            // Read chunk from original file and write
            in.read(reinterpret_cast<char*>(buffer.data()), chunk_size);
            out.write(reinterpret_cast<char*>(buffer.data()), in.gcount());
        }
    }

    // Write trailing bytes if any
    size_t trailing = file_size % chunk_size;
    if (trailing > 0) {
        std::vector<uint8_t> tail(trailing);
        in.read(reinterpret_cast<char*>(tail.data()), trailing);
        out.write(reinterpret_cast<char*>(tail.data()), trailing);
    }
}

size_t ChunkEditor::get_total_chunks() const {
    return (file_size + chunk_size - 1) / chunk_size;
}

size_t ChunkEditor::get_chunk_size() const {
    return chunk_size;
}

size_t ChunkEditor::get_file_size() const {
    return file_size;
}