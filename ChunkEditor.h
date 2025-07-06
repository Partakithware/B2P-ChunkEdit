// ChunkEditor.h
#pragma once
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <cstdint>

class ChunkEditor {
public:
    ChunkEditor(const std::string& path, size_t chunkSize = 64);

    std::vector<uint8_t> read_chunk(size_t index);
    void write_chunk(size_t index, const std::vector<uint8_t>& data);
    void save_as(const std::string& outPath);
    void set_chunk_size(size_t size);
    size_t get_total_chunks() const;
    size_t get_chunk_size() const;
    size_t get_file_size() const;

private:
    std::ifstream input;
    std::string original_path;
    std::map<size_t, std::vector<uint8_t>> dirty_chunks;
    size_t chunk_size;
    size_t file_size;
};

