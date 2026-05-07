#include "store.h"
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

static const char* DATA_FILE = "tasks.json";

// ---------- arg helpers ----------

static std::string opt(const std::vector<std::string>& args, const std::string& flag) {
    for (size_t i = 0; i + 1 < args.size(); ++i)
        if (args[i] == flag) return args[i + 1];
    return "";
}

static bool has(const std::vector<std::string>& args, const std::string& flag) {
    return std::any_of(args.begin(), args.end(),
                       [&](const std::string& a) { return a == flag; });
}

static int parse_id(const std::string& s) {
    try { return std::stoi(s); }
    catch (...) { throw std::runtime_error("invalid id: " + s); }
}

// ---------- display ----------

static void print_help() {
    puts(
        "Usage: json_app <command> [args]\n"
        "\n"
        "  add  <title> [--tag TAG]              Add a new task\n"
        "  list [--done|--pending] [--tag TAG]   List tasks\n"
        "  show <id>                             Show task detail\n"
        "  done <id>                             Mark task as done\n"
        "  undo <id>                             Mark task as pending\n"
        "  edit <id> [--title T] [--tag TAG]     Update a task\n"
        "  rm   <id>                             Delete a task\n"
        "  help                                  Show this help\n"
        "\n"
        "Data file: tasks.json (current directory)"
    );
}

static void print_table_header() {
    printf("%-4s  %-5s  %-12s  %s\n", "ID", "DONE", "TAG", "TITLE");
    printf("%.4s  %.5s  %.12s  %.40s\n",
           "----", "-----", "------------", "----------------------------------------");
}

static void print_row(const Task& t) {
    std::string tag = t.tag.size() > 12 ? t.tag.substr(0, 12) : t.tag;
    printf("%-4d  %-5s  %-12s  %s\n",
           t.id, t.done ? "[x]" : "[ ]", tag.c_str(), t.title.c_str());
}

static void print_detail(const Task& t) {
    printf("ID      : %d\n",  t.id);
    printf("Title   : %s\n",  t.title.c_str());
    printf("Tag     : %s\n",  t.tag.empty() ? "(none)" : t.tag.c_str());
    printf("Done    : %s\n",  t.done ? "yes" : "no");
    printf("Created : %s\n",  t.created_at.c_str());
}

// ---------- commands ----------

static void cmd_add(Store& s, const std::vector<std::string>& args) {
    if (args.empty()) { std::cerr << "Usage: add <title> [--tag TAG]\n"; return; }
    Task t = s.add(args[0], opt(args, "--tag"));
    printf("Added: #%d  %s\n", t.id, t.title.c_str());
}

static void cmd_list(Store& s, const std::vector<std::string>& args) {
    bool done_only = has(args, "--done");
    bool pend_only = has(args, "--pending");
    std::string tf = opt(args, "--tag");

    std::vector<Task> tasks = s.list();
    std::vector<Task> filtered;
    for (const auto& t : tasks) {
        if (done_only && !t.done)          continue;
        if (pend_only &&  t.done)          continue;
        if (!tf.empty() && t.tag != tf)    continue;
        filtered.push_back(t);
    }

    if (filtered.empty()) { puts("No tasks found."); return; }

    print_table_header();
    for (const auto& t : filtered) print_row(t);
    printf("  %zu task(s)\n", filtered.size());
}

static void cmd_show(Store& s, const std::vector<std::string>& args) {
    if (args.empty()) { std::cerr << "Usage: show <id>\n"; return; }
    print_detail(s.get(parse_id(args[0])));
}

static void cmd_done(Store& s, const std::vector<std::string>& args, bool done) {
    if (args.empty()) { std::cerr << "Usage: done|undo <id>\n"; return; }
    int id = parse_id(args[0]);
    s.mark_done(id, done);
    printf("Task #%d marked as %s\n", id, done ? "done" : "pending");
}

static void cmd_edit(Store& s, const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cerr << "Usage: edit <id> [--title T] [--tag TAG]\n";
        return;
    }
    int id = parse_id(args[0]);

    bool        has_title = has(args, "--title");
    bool        has_tag   = has(args, "--tag");
    std::string new_title = opt(args, "--title");
    std::string new_tag   = opt(args, "--tag");

    if (!has_title && !has_tag) {
        std::cerr << "Nothing to update. Use --title or --tag.\n";
        return;
    }

    s.update(id,
             (has_title && !new_title.empty()) ? &new_title : nullptr,
             has_tag ? &new_tag : nullptr);
    printf("Updated task #%d\n", id);
}

static void cmd_rm(Store& s, const std::vector<std::string>& args) {
    if (args.empty()) { std::cerr << "Usage: rm <id>\n"; return; }
    int id = parse_id(args[0]);
    s.remove(id);
    printf("Deleted task #%d\n", id);
}

// ---------- main ----------

int main(int argc, char* argv[]) {
    if (argc < 2) { print_help(); return 0; }

    std::string cmd = argv[1];
    std::vector<std::string> args;
    for (int i = 2; i < argc; ++i) args.push_back(argv[i]);

    try {
        Store store(DATA_FILE);

        if      (cmd == "add")  cmd_add(store, args);
        else if (cmd == "list") cmd_list(store, args);
        else if (cmd == "show") cmd_show(store, args);
        else if (cmd == "done") cmd_done(store, args, true);
        else if (cmd == "undo") cmd_done(store, args, false);
        else if (cmd == "edit") cmd_edit(store, args);
        else if (cmd == "rm")   cmd_rm(store, args);
        else if (cmd == "help") print_help();
        else {
            std::cerr << "Unknown command: " << cmd << "\n";
            print_help();
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
