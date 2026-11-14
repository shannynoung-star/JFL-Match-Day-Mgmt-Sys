// recordfile.h - RANDOM ACCESS file of fixed-size binary records.
//
// Every record takes exactly the same number of bytes (sizeof(Record)), so record number N starts at
// byte N * sizeof(Record). We can jump straight to any record with seekg/seekp - no need to read the
// records before it. That is what makes the file "random access" (sequential files must be read in order).
#pragma once
#include <cstring>
#include <fstream>
#include "common.h"

namespace kpfl {

const int TAG_LEN = 16;      // room for a record type such as "TEAM" or "PLAYER"
const int MAX_FIELDS = 12;   // most fields any record type needs
const int FIELD_LEN = 40;    // 39 characters + 1 for the end-of-text marker '\0'

// A plain fixed-size struct: safe to write to a file as raw bytes.
struct Record {
    char tag[TAG_LEN];
    int fieldCount;
    char field[MAX_FIELDS][FIELD_LEN];
};
const streamoff RECORD_SIZE = sizeof(Record);

// Builds a Record from a tag and a list of text fields.
inline Record makeRecord(const string& tag, const vector<string>& fields) {
    if (tag.size() >= static_cast<size_t>(TAG_LEN)) throw KpflException("Record tag too long: " + tag);
    if (fields.size() > static_cast<size_t>(MAX_FIELDS)) throw KpflException("Too many fields in " + tag + " record.");
    Record r;
    memset(&r, 0, sizeof r);                       // start with all zero bytes
    memcpy(r.tag, tag.c_str(), tag.size());
    r.fieldCount = static_cast<int>(fields.size());
    for (size_t i = 0; i < fields.size(); i++) {
        if (fields[i].size() >= static_cast<size_t>(FIELD_LEN))
            throw InvalidDataException("Value too long to store: " + fields[i]);
        memcpy(r.field[i], fields[i].c_str(), fields[i].size());
    }
    return r;
}

// Turns a Record back into {tag, field1, field2, ...}.
inline vector<string> recordToStrings(const Record& r) {
    if (r.fieldCount < 0 || r.fieldCount > MAX_FIELDS) throw KpflException("Damaged record (bad field count).");
    vector<string> out;
    out.push_back(string(r.tag, strnlen(r.tag, TAG_LEN)));
    for (int i = 0; i < r.fieldCount; i++) out.push_back(string(r.field[i], strnlen(r.field[i], FIELD_LEN)));
    return out;
}

class RecordFile {
public:
    enum Mode { READ, CREATE };   // READ: open existing file.  CREATE: make a new (empty) file.
private:
    fstream file;
public:
    RecordFile(const string& path, Mode mode) {
        if (mode == CREATE) file.open(path, ios::in | ios::out | ios::binary | ios::trunc);
        else file.open(path, ios::in | ios::binary);
        if (!file) throw KpflException(mode == READ ? "Data file not found: " + path
                                                    : "Could not open " + path + " for writing.");
    }
    // How many records are in the file: jump to the end, ask the position, divide by the record size.
    long recordCount() {
        file.clear();
        file.seekg(0, ios::end);
        streamoff bytes = file.tellg();
        if (bytes < 0 || bytes % RECORD_SIZE != 0) throw KpflException("Data file is damaged (incomplete record).");
        return static_cast<long>(bytes / RECORD_SIZE);
    }
    // RANDOM READ: jump to record n and read exactly one record.
    Record read(long n) {
        file.clear();
        file.seekg(n * RECORD_SIZE, ios::beg);
        Record r;
        file.read(reinterpret_cast<char*>(&r), sizeof r);
        if (!file) throw KpflException("Could not read record " + to_string(n) + ".");
        return r;
    }
    // RANDOM WRITE: jump to record n and overwrite exactly that record.
    void write(long n, const Record& r) {
        file.clear();
        file.seekp(n * RECORD_SIZE, ios::beg);
        file.write(reinterpret_cast<const char*>(&r), sizeof r);
        if (!file) throw KpflException("Could not write record " + to_string(n) + ".");
    }
    void flush() {
        file.flush();
        if (!file) throw KpflException("Error while saving the data file.");
    }
};

}  // namespace kpfl
