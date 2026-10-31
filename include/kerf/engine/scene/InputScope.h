#pragma once

#include <unordered_set>


namespace kerf {

// forward declarations
class Node;

// ------------------------------------------------
// InputScope
// ------------------------------------------------

/**
 * @brief A set of root nodes that may receive mouse input
 *
 * remove a node before deleting it.
 */
class InputScope {
private:
    std::unordered_set<const Node*> roots;

public:
    void add(const Node* node);
    void remove(const Node* node);
    void clear();

    // true if node or any of its parents is a root of this scope
    bool contains(const Node* node) const;
};

} // namespace kerf
