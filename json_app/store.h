#pragma once
#include "task.h"
#include "json.h"
#include <string>
#include <vector>

class Store {
public:
    explicit Store(const std::string& path);

    Task              add(const std::string& title, const std::string& tag = "");
    std::vector<Task> list() const;
    Task              get(int id) const;
    void              mark_done(int id, bool done);
    void              update(int id, const std::string* title, const std::string* tag);
    void              remove(int id);

private:
    std::string path_;
    json::Value data_;

    void        flush();
    Task        to_task(const json::Value& v) const;
    int         find_idx(int id) const;
};
