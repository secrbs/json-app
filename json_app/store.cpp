#include "store.h"
#include <ctime>
#include <stdexcept>
#include <string>

static std::string today() {
    std::time_t t = std::time(nullptr);
    std::tm tm{};
    localtime_s(&tm, &t);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
    return buf;
}

Store::Store(const std::string& path) : path_(path) {
    try {
        data_ = json::load(path);
    } catch (const std::exception&) {
        data_["next_id"] = 1;
        data_["tasks"]   = json::Value::Array{};
    }
}

Task Store::add(const std::string& title, const std::string& tag) {
    int id = data_["next_id"].as_int();
    data_["next_id"] = id + 1;

    json::Value entry;
    entry["id"]         = id;
    entry["title"]      = title;
    entry["tag"]        = tag;
    entry["done"]       = false;
    entry["created_at"] = today();

    data_["tasks"].push_back(entry);
    flush();
    return to_task(entry);
}

std::vector<Task> Store::list() const {
    std::vector<Task> result;
    if (!data_.has_key("tasks")) return result;
    for (const auto& v : data_["tasks"])
        result.push_back(to_task(v));
    return result;
}

Task Store::get(int id) const {
    int idx = find_idx(id);
    if (idx < 0)
        throw std::runtime_error("task #" + std::to_string(id) + " not found");
    return to_task(data_["tasks"][static_cast<size_t>(idx)]);
}

void Store::mark_done(int id, bool done) {
    int idx = find_idx(id);
    if (idx < 0)
        throw std::runtime_error("task #" + std::to_string(id) + " not found");
    data_["tasks"][static_cast<size_t>(idx)]["done"] = done;
    flush();
}

void Store::update(int id, const std::string* title, const std::string* tag) {
    int idx = find_idx(id);
    if (idx < 0)
        throw std::runtime_error("task #" + std::to_string(id) + " not found");
    auto si = static_cast<size_t>(idx);
    if (title) data_["tasks"][si]["title"] = *title;
    if (tag)   data_["tasks"][si]["tag"]   = *tag;
    flush();
}

void Store::remove(int id) {
    int idx = find_idx(id);
    if (idx < 0)
        throw std::runtime_error("task #" + std::to_string(id) + " not found");

    json::Value::Array new_arr;
    {
        const auto& tasks = data_["tasks"].as_array();
        for (const auto& v : tasks)
            if (v["id"].as_int() != id)
                new_arr.push_back(v);
    }
    data_["tasks"] = json::Value{std::move(new_arr)};
    flush();
}

void Store::flush() {
    json::save(path_, data_, 2);
}

Task Store::to_task(const json::Value& v) const {
    Task t;
    t.id         = v["id"].as_int();
    t.title      = v["title"].as_string();
    t.tag        = (v.has_key("tag") && v["tag"].is_string()) ? v["tag"].as_string() : "";
    t.done       = (v.has_key("done") && v["done"].is_bool()) ? v["done"].as_bool() : false;
    t.created_at = (v.has_key("created_at") && v["created_at"].is_string()) ? v["created_at"].as_string() : "";
    return t;
}

int Store::find_idx(int id) const {
    if (!data_.has_key("tasks")) return -1;
    const auto& arr = data_["tasks"].as_array();
    for (size_t i = 0; i < arr.size(); ++i)
        if (arr[i]["id"].as_int() == id)
            return static_cast<int>(i);
    return -1;
}
