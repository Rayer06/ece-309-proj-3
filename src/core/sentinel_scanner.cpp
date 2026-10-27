#include "core/sentinel_scanner.h"
#include <string>

SentinelScanner::SentinelScanner(std::string sentinel) {
    sentinel_ = sentinel;
    pending_ = "";
}

// Feed the next chunk. Returns text guaranteed NOT to be part of
// the sentinel (safe to print immediately) and whether the
// sentinel has now been fully seen.
SentinelScanner::Out SentinelScanner::feed(std::string_view chunk) {
    bool sentinel_start = false;
    std::size_t last_match = 0;
    int where_match = 0;
    std::size_t i;

    Out put; // create new output container
    put.safe_text = "";
    put.sentinel_found = false;

    // string to search
    std::string tmp = "";
    tmp = tmp.append(pending_);
    tmp = tmp.append(chunk);

    std::size_t search_start = std::max<signed long>(static_cast<signed long>(tmp.size()) - static_cast<signed long>(sentinel_.size()), 0);

    // new logic:
    // search string for "<", "<|", "<|e", etc in sequence
    // if no sequence returns match, all text is safe text
    // if every iteration returns match, sentinel found, return only text excluded in sentinel
    // if only some iterations return match:
    // if last matched iteration hit the end of the string, sentinel not found,
    // but this text excluded from safe text
    // if last matched iteration does not hit end of string, sentinel not found,
    // all text is safe text

    for (i = 0; i < sentinel_.size(); i++) {
        if (tmp.find(sentinel_.substr(0, i+1), search_start) != std::string::npos) {
            where_match = tmp.find(sentinel_.substr(0, i+1));
            sentinel_start = true;
            last_match = i;
        }
    }
    // no portion of sentinel found
    if (sentinel_start == false) {
        put.safe_text.append(tmp);
        put.sentinel_found = false;
    }
    // Whole sentinel found
    if ((sentinel_start == true) && (last_match == sentinel_.size()-1)) {
        put.safe_text.append(tmp.substr(0, where_match));
        put.sentinel_found = true;
        pending_.clear();
    }
    // Part of sentinel or false match
    if ((sentinel_start == true) && (last_match != sentinel_.size()-1)) {
        if (where_match + last_match == tmp.size()-1) { // part of sentinel (hits end)
            put.safe_text.append(tmp.substr(0, where_match));
            pending_.append(tmp.substr(where_match + pending_.size()));
        } else {
            put.safe_text.append(tmp); // false match (does not hit end)
            pending_.clear();
        }
        put.sentinel_found = false;
    }
    if (pending_.size() > sentinel_.size()-1) {
        std::size_t charsToRemove = pending_.size() - (sentinel_.size() - 1);
        pending_.erase(0, charsToRemove); // if too long, remove characters
    }

    return put;
}

// Call once, after the stream ends, to release any text still
// being held back.
SentinelScanner::Out SentinelScanner::flush() {
    Out put;
    put.safe_text = std::move(pending_);
    put.sentinel_found = false;
    return put;
}