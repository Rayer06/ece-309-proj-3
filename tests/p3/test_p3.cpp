// tests/p3/test_p3.cpp — YOUR P3 TEST SUITE.
//
// The five tests below are worked examples. Keep them, and add your own
// until you have AT LEAST 15 (spec §6 lists what must be covered).
// Name every test <area>_<what>: reg_ for the registry, ws_ for the
// workspace, calc_ for the calculator.
//
// Build & run:   cmake --build build && ./build/test_p3
// One area:      ./build/test_p3 reg_        (names starting with "reg_")
// One test:      ./build/test_p3 =reg_add_and_find

#include "../mini_test.h"
#include "../test_helpers.h"     // TempDir, write_text, args1, ...

#include "tools/calculator.h"
#include "tools/tool_registry.h"
#include "tools/workspace.h"

#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

// A tool whose name you choose — use it to fill the registry in tests.
class NamedTool : public Tool {
public:
    explicit NamedTool(std::string n) : n_(std::move(n)), schema_(JsonValue::object()) {}
    std::string name() const override { return n_; }
    std::string description() const override { return "test tool"; }
    const JsonValue& schema() const override { return schema_; }
    ToolResult invoke(const JsonValue&, ToolContext&) override { return ToolResult::success(n_); }
private:
    std::string n_;
    JsonValue   schema_;   // an empty JSON object: {}
};

// Tokenize with the provided tokenizer, then run YOUR calc_evaluate.
bool eval(const std::string& expr, double* value, std::string* error) {
    std::vector<CalcToken> tokens;
    if (!calc_tokenize(expr, &tokens, error)) return false;
    return calc_evaluate(tokens, value, error);
}

}  // namespace

// ---------------------------------------------------------------- examples

TEST(reg_add_and_find) {
    ToolRegistry r;
    auto t = std::make_unique<NamedTool>("alpha");
    Tool* raw = t.get();                        // remember the address before moving
    REQUIRE(r.add(std::move(t)));
    CHECK(r.size() == 1);
    CHECK(r.find("alpha") == raw);              // same object, not a copy
    CHECK(r.find("beta") == nullptr);
}

TEST(reg_duplicate_keeps_original) {
    ToolRegistry r;
    auto first = std::make_unique<NamedTool>("alpha");
    Tool* original = first.get();
    REQUIRE(r.add(std::move(first)));
    CHECK(!r.add(std::make_unique<NamedTool>("alpha")));
    CHECK(r.find("alpha") == original);
    CHECK(r.size() == 1);
}

TEST(ws_rejects_parent_escape) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("notes/todo.txt", &out, &reason));   // sanity: accepts *something*
    CHECK(out == ws.root() / "notes" / "todo.txt");
    CHECK(!ws.resolve("../etc/passwd", &out, &reason));
    CHECK(!reason.empty());
}

TEST(calc_precedence) {
    double v = 0;
    std::string err;
    REQUIRE(eval("2 + 3 * 4", &v, &err));
    CHECK(v == 14);
}

TEST(calc_division_by_zero) {
    double v = 0;
    std::string err;
    CHECK(!eval("1 / 0", &v, &err));
    CHECK(err == "division by zero");
}

// ------------------------------------------------------------- your tests
//
// Still to write (spec §6 has the full list). Suggested names:
//   reg_null_rejected, reg_for_each_visits_each_once
//   reg_10000_tools               REQUIRED: print bucket_count / load_factor /
//                                 max_probe_length, check the bound
//   reg_alternative_hash          REQUIRED: longest chain of a weak hash
//   ws_<case>                     REQUIRED: every row of the table in spec §4.4
//   calc_<case>                   left-to-right, parentheses, negatives

TEST(reg_null_rejected) {
    ToolRegistry r;
    REQUIRE(!r.add(nullptr));
    CHECK(r.size() == 0);
}

TEST(reg_find_empty) {
    ToolRegistry r;
    auto t = std::make_unique<NamedTool>("tool_1");
    REQUIRE(r.add(std::move(t)));
    REQUIRE(!r.find("tool_0"));
    CHECK(r.size() == 1);
}

TEST(reg_for_each_visits_each_once) {
    ToolRegistry r;
    r.add(std::make_unique<NamedTool>("tool_1"));
    r.add(std::make_unique<NamedTool>("tool_2"));
    r.add(std::make_unique<NamedTool>("tool_3"));
    r.add(std::make_unique<NamedTool>("tool_4"));
    r.add(std::make_unique<NamedTool>("tool_5"));

    int count = 0;
    r.for_each([&](const Tool&) {
        ++count;
    });
    REQUIRE(count == 5);
    CHECK(r.size() == 5);
}

TEST(reg_10000_tools) {
    ToolRegistry r;
    Tool* first = nullptr;
    for (int i = 1; i <= 10000; i++) {
        std::string name = "tool_" + std::to_string(i);
        auto t = std::make_unique<NamedTool>(name);
        if (i == 1) {
            first = t.get();
        }
        r.add(std::move(t));
        REQUIRE(r.load_factor() <= 1.0);
    }
    for (int i = 1; i <= 10000; i++) {
        REQUIRE(r.find("tool_" + std::to_string(i)));
    }
    std::cout << "Bucket count: " << r.bucket_count() << std::endl;
    std::cout << "Load factor: " << r.load_factor() << std::endl;
    std::cout << "Max probe length: " << r.max_probe_length() << std::endl;
    REQUIRE(r.find("tool_1") == first);
    REQUIRE(r.max_probe_length() <= 64);

}

TEST(reg_alternative_hash) {
    ToolRegistry r;
    std::vector<int> weak_hash_count;
    int max_chain = 0;
    for (int i = 1; i <= 10000; i++) {
        std::string name = "tool_" + std::to_string(i);
        auto t = std::make_unique<NamedTool>(name);
        r.add(std::move(t));
    }
    for (size_t i = 0; i < r.bucket_count(); i++) {
        weak_hash_count.push_back(0);
    }
    for (int i = 1; i <= 10000; i++) {
        std::string s = "tool_" + std::to_string(i);
        unsigned int weak_hash = s[0] + s.size();
        size_t index = weak_hash & (r.bucket_count()-1);
        weak_hash_count[index]++;
    }
    for (size_t i = 0; i < r.bucket_count(); i++) {
        if (weak_hash_count[i] > max_chain) {
            max_chain = weak_hash_count[i];
        }
    }
    std::cout << "Max chain for weak hash: " << max_chain << std::endl;
}

TEST(ws_acceptable_paths) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("./notes//todo.txt", &out, &reason));
    CHECK(out == ws.root() / "notes" / "todo.txt");
    CHECK(ws.resolve(".", &out, &reason));
    CHECK(out == ws.root());
    CHECK(ws.resolve("a/../b", &out, &reason));
    CHECK(out == ws.root() / "b");
    CHECK(ws.resolve("a/..", &out, &reason));
    CHECK(out == ws.root());
    CHECK(ws.resolve("new/dir/file.txt", &out, &reason));
    CHECK(out == ws.root() / "new" / "dir" / "file.txt");
    CHECK(ws.resolve("my..notes.txt", &out, &reason));
    CHECK(out == ws.root() / "my..notes.txt");
}

TEST(ws_rejects_empty_string) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(!ws.resolve("", &out, &reason));
    CHECK(!reason.empty());
}

TEST(ws_rejects_null) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(!ws.resolve(std::string("notes") + '\0' + "/x", &out, &reason));
    CHECK(!reason.empty());
}

TEST(ws_rejects_absolute_non_ws_path) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(!ws.resolve("/etc/passwd", &out, &reason));
    CHECK(!reason.empty());
}

TEST(ws_rejects_additional_excaping_paths) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(!ws.resolve("..", &out, &reason));
    CHECK(!reason.empty());
    CHECK(!ws.resolve("a/../../b", &out, &reason));
    CHECK(!reason.empty());
    CHECK(!ws.resolve("notes/../../ws/notes/todo.txt", &out, &reason));
    CHECK(!reason.empty());
}

TEST(calc_various_expressions) {
    double v = 0;
    std::string err;
    REQUIRE(eval("10 - 4 - 3", &v, &err));
    CHECK(v == 3);
    REQUIRE(eval("100 / 10 / 5", &v, &err));
    CHECK(v == 2);
    REQUIRE(eval("(2 + 3) * 4", &v, &err));
    CHECK(v == 20);
    REQUIRE(eval("2 * (3 + 4) - 5", &v, &err));
    CHECK(v == 9);
    REQUIRE(eval("-3 * -3", &v, &err));
    CHECK(v == 9);
    REQUIRE(eval("7 / 2", &v, &err));
    CHECK(v == 3.5);
    REQUIRE(!eval("1 / (2 - 2)", &v, &err));
    CHECK(err == "division by zero");
    REQUIRE(eval("4 * (2 * 3)", &v, &err));
    CHECK(v == 24);
    REQUIRE(eval("1 + 2 - 3", &v, &err));
    CHECK(v == 0);
}


int main(int argc, char** argv) { return mt::run_all(argc, argv); }
